#include "../kaba.h"
#include "../../file/file.h"
#include "../../base/set.h"
#include <stdio.h>
#include <functional>


#if defined(OS_LINUX)// || defined(OS_MINGW)
	#include <sys/mman.h>
	#if (!defined(__x86_64__)) && (!defined(__amd64__))
		#define MAP_32BIT		0
	#endif
#endif
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	#include <windows.h>
#endif
#include <errno.h>

namespace Kaba{


int LocalOffset,LocalOffsetMax;

/*int get_func_temp_size(Function *f)
{
}*/

inline int add_temp_var(int size)
{
	LocalOffset += size;
	if (LocalOffset > LocalOffsetMax)
		LocalOffsetMax = LocalOffset;
	return LocalOffset;
}


int TaskReturnOffset;


#define CallRel32OCSize			5
#define AfterWaitOCSize			10

void AddEspAdd(Asm::InstructionWithParamsList *list,int d)
{
	if (d > 0){
		if (d > 120)
			list->add2(Asm::INST_ADD, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(d, 4));
		else
			list->add2(Asm::INST_ADD, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(d, 1));
	}else if (d < 0){
		if (d < -120)
			list->add2(Asm::INST_SUB, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(-d, 4));
		else
			list->add2(Asm::INST_SUB, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(-d, 1));
	}
}

void try_init_global_var(Class *type, char* g_var)
{
	if (type->is_array){
		for (int i=0;i<type->array_length;i++)
			try_init_global_var(type->parent, g_var + i * type->parent->size);
		return;
	}
	ClassFunction *cf = type->get_default_constructor();
	if (!cf)
		return;
	typedef void init_func(void *);
	//msg_write("global init: " + v.type->name);
	init_func *ff = (init_func*)cf->script->func[cf->nr];
	if (ff)
		ff(g_var);
}

void init_all_global_objects(SyntaxTree *ps, Array<char*> &g_var)
{
	foreachi(Variable &v, ps->root_of_all_evil.var, i)
		if (!v.is_extern)
			try_init_global_var(v.type, g_var[i]);
}

static int64 _opcode_rand_state_ = 10000;

void* get_nice_random_addr()
{
	int64 p = ((int_p)&Init) & 0xfffffffffffff000;
	_opcode_rand_state_ = (_opcode_rand_state_ * 1664525 + 1013904223);
	p += (int64)(_opcode_rand_state_ & 0x3fff) * 4096;
	return (void*)p;

}

void* get_nice_memory(long size, bool executable)
{
	void *mem = nullptr;
	size = mem_align(size, 4096);
	if (config.verbose)
		msg_write("get nice...");

#if defined(OS_WINDOWS) || defined(OS_MINGW)
	mem = (char*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else

	// try in 32bit distance from current opcode
	for (int i=0; i<100; i++){
		void *addr0 = get_nice_random_addr();
		//opcode = (char*)mmap(addr0, max_opcode, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, -1, 0);
		mem = (char*)mmap(addr0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_EXECUTABLE, -1, 0);
		if (config.verbose)
			printf("%d  %p  ->  %p\n", i, addr0, mem);
		if ((int_p)mem != -1){
			if (labs((int_p)mem - (int_p)addr0) < 1000000000)
				return mem;
			else
				munmap(mem, size);
			if (config.verbose)
				msg_write("...try again");
		}
	}

	// no?...ok, try anywhere
	mem = (char*)mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_EXECUTABLE, -1, 0);
	if ((int_p)mem == -1)
		mem = nullptr;
#endif

	// failed...
	if (!mem){
		msg_error(string("Script:  could not allocate executable memory: ") + strerror(errno));
		mem = new char[size];
	}

	return mem;
}

void Script::AllocateMemory()
{
	// get memory size needed
	memory_size = 0;
	for (int i=0;i<syntax->root_of_all_evil.var.num;i++)
		if (!syntax->root_of_all_evil.var[i].is_extern)
			memory_size += mem_align(syntax->root_of_all_evil.var[i].type->size, 4);

	// constants
	foreachi(Constant *c, syntax->constants, i)
		memory_size += mem_align(c->mapping_size(), 4);

	// vtables
	for (Class *t: syntax->classes)
		if (t->vtable.num > 0)
			memory_size += config.pointer_size;

	// allocate
	if (memory_size > 0){
		memory = (char*)get_nice_memory(memory_size, false);
		if (config.verbose)
			msg_write("memory:  " + p2s(memory));
	}
}

void Script::AllocateStack()
{
	// use your own stack if needed
	//   wait() used -> needs to switch stacks ("tasks")
	__stack = nullptr;
	/*for (Command *cmd: syntax->commands){
		if (cmd->kind == KIND_COMPILER_FUNCTION)
			if ((cmd->link_no == COMMAND_WAIT) or (cmd->link_no == COMMAND_WAIT_RT) or (cmd->link_no == COMMAND_WAIT_ONE_FRAME)){
				__stack = new char[config.stack_size];
				break;
			}
	}*/
}

void Script::AllocateOpcode()
{
	int max_opcode = MAX_OPCODE;
	if (config.compile_os)
		max_opcode *= 10;

	opcode = (char*)get_nice_memory(max_opcode, true);
	if (config.verbose)
		msg_write("opcode:  " + p2s(opcode));

	if (config.override_code_origin)
		syntax->asm_meta_info->code_origin = config.code_origin;
	else
		syntax->asm_meta_info->code_origin = (int_p)opcode;
	opcode_size = 0;
}

void Script::MapConstantsToMemory()
{
	// constants -> Memory
	cnst.resize(syntax->constants.num);
	foreachi(Constant *c, syntax->constants, i){
		cnst[i] = &memory[memory_size];
		c->map_into(cnst[i], cnst[i]);
		memory_size += mem_align(c->mapping_size(), 4);
	}
}

void Script::MapGlobalVariablesToMemory()
{
	// global variables -> into Memory
	g_var.resize(syntax->root_of_all_evil.var.num);
	foreachi(Variable &v, syntax->root_of_all_evil.var, i){
		if (v.is_extern){
			g_var[i] = (char*)GetExternalLink(v.name);
			if (!g_var[i])
				DoErrorLink("external variable " + v.name + " was not linked");
		}else{
			if (config.override_variables_offset)
				g_var[i] = (char*)(int_p)(memory_size + config.variables_offset);
			else
				g_var[i] = &memory[memory_size];
			memory_size += mem_align(v.type->size, 4);
		}
	}
	memset(memory, 0, memory_size); // reset all global variables to 0
}

void Script::AlignOpcode()
{
	int ocs_new = mem_align(opcode_size, config.function_align);
	for (int i=opcode_size;i<ocs_new;i++)
		opcode[i] = 0x90;
	opcode_size = ocs_new;
}

static int OCORA;
void Script::CompileOsEntryPoint()
{
	int nf=-1;
	foreachi(Function *ff, syntax->functions, index)
		if (ff->name == "main")
			nf = index;
	// call
	if (nf>=0)
		Asm::AddInstruction(opcode, opcode_size, Asm::INST_CALL, Asm::param_imm(0, 4));
	TaskReturnOffset=opcode_size;
	OCORA = Asm::OCParam;
	AlignOpcode();
}

void apply_recursive(Node *n, std::function<void(Node*)> f)
{
	f(n);

	for (Node *p: n->params)
		apply_recursive(p, f);

	if (n->instance)
		apply_recursive(n->instance, f);

	if (n->kind == KIND_BLOCK)
		for (Node *child: n->as_block()->nodes)
			apply_recursive(child, f);
}

void Script::MapConstantsToOpcode()
{
	cnst.resize(syntax->constants.num);

	// vtables -> no data yet...
	for (Class *t: syntax->classes)
		if (t->vtable.num > 0){
			t->_vtable_location_compiler_ = &opcode[opcode_size];
			t->_vtable_location_target_ = (void*)(syntax->asm_meta_info->code_origin + opcode_size);
			opcode_size += config.pointer_size * t->vtable.num;
			for (Constant *c: syntax->constants)
				if ((c->type == TypePointer) and (*(int*)c->value.data == (int)(int_p)t->vtable.data))
					memcpy(c->value.data, &t->_vtable_location_target_, config.pointer_size);
		}

	Array<bool> used;
	used.resize(syntax->constants.num);
	for (Function *f: syntax->functions)
		for (Node *n: f->block->nodes)
			apply_recursive(n, [&](Node *n){ if (n->kind == KIND_CONSTANT){ used[n->link_no] = true; if (n->script != syntax->script) msg_error("evil const " + n->as_const()->name); } });

	/*int n = 0;
	for (bool b: used)
		if (b)
			n ++;
	msg_write(format("     USED:    %d / %d", n, used.num));
	int size0 = opcode_size;

	foreachi(Constant *c, syntax->constants, i){
		cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
		c->map_into(&opcode[opcode_size], cnst[i]);
		opcode_size += mem_align(c->mapping_size(), 4);
	}
	int uncompressed = opcode_size - size0;
	opcode_size = size0;*/


	foreachi(Constant *c, syntax->constants, i)
		if (used[i]){
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += mem_align(c->mapping_size(), 4);
		}

	/*foreachi(Constant *c, syntax->constants, i)
		if ((c->mapping_size() > 1) and used[i]){
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += mem_align(c->mapping_size(), 4);
		}
	foreachi(Constant *c, syntax->constants, i)
		if ((c->mapping_size() == 1) and used[i]){
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += 1;
		}*/

	//msg_write(format("    compressed:  %d  ->  %d", uncompressed, opcode_size - size0));

	AlignOpcode();
}

void Script::LinkOsEntryPoint()
{
	int nf = -1;
	foreachi(Function *ff, syntax->functions, index)
		if (ff->name == "main")
			nf = index;
	if (nf >= 0){
		int lll = (int_p)func[nf] - syntax->asm_meta_info->code_origin - TaskReturnOffset;
		//printf("insert   %d  an %d\n", lll, OCORA);
		//msg_write(lll);
		//msg_write(d2h(&lll,4,false));
		*(int*)&opcode[OCORA] = lll;
	}
}

void Script::CompileTaskEntryPoint()
{
	// "stack" usage for waiting:
	//  -4| -8 - ebp (before execution)
	//  -8|-16 - ebp (script continue)
	// -12|-24 - esp (script continue)
	// -16|-32 - eip (script continue)
	// -20|-40 - script stack...

	// call
	void *_main_ = MatchFunction("main", "void", 0);

	if ((!__stack) or (!_main_)){
		__first_execution = (t_func*)_main_;
		__continue_execution = nullptr;
		return;
	}

	Asm::InstructionWithParamsList *list = new Asm::InstructionWithParamsList(0);

	int label_first = list->add_label("_first_execution");

	__first_execution = (t_func*)&__thread_opcode[__thread_opcode_size];
	// intro
	list->add2(Asm::INST_PUSH, Asm::param_reg(Asm::REG_EBP)); // within the actual program
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EBP), Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_ESP), Asm::param_deref_imm((int_p)&__stack[config.stack_size], 4)); // start of the script stack
	list->add2(Asm::INST_PUSH, Asm::param_reg(Asm::REG_EBP)); // address of the old stack
	AddEspAdd(list, -12); // space for wait() task data
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EBP), Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EAX), Asm::param_imm(WAITING_MODE_NONE, 4)); // "reset"
	list->add2(Asm::INST_MOV, Asm::param_deref_imm((int_p)&__waiting_mode, 4), Asm::param_reg(Asm::REG_EAX));

	// call main()
	list->add2(Asm::INST_CALL, Asm::param_imm((int_p)_main_, 4));

	// outro
	AddEspAdd(list, 12); // make space for wait() task data
	list->add2(Asm::INST_POP, Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EBP), Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_LEAVE);
	list->add2(Asm::INST_RET);

	// "task" for execution after some wait()
	int label_cont = list->add_label("_continue_execution");

	// Intro
	list->add2(Asm::INST_PUSH, Asm::param_reg(Asm::REG_EBP)); // within the external program
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EBP), Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_MOV, Asm::param_deref_imm((int_p)&__stack[config.stack_size - 4], 4), Asm::param_reg(Asm::REG_EBP)); // save the external ebp
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_ESP), Asm::param_deref_imm((int_p)&__stack[config.stack_size - 16], 4)); // to the eIP of the script
	list->add2(Asm::INST_POP, Asm::param_reg(Asm::REG_EAX));
	list->add2(Asm::INST_ADD, Asm::param_reg(Asm::REG_EAX), Asm::param_imm(AfterWaitOCSize, 4));
	list->add2(Asm::INST_JMP, Asm::param_reg(Asm::REG_EAX));
	//list->add2(Asm::inst_leave);
	//list->add2(Asm::inst_ret);
	/*OCAddChar(0x90);
	OCAddChar(0x90);
	OCAddChar(0x90);*/

	list->Compile(__thread_opcode, __thread_opcode_size);

	__first_execution = (t_func*)(int_p)list->label[label_first].value;
	__continue_execution = (t_func*)(int_p)list->label[label_cont].value;

	delete(list);
}

bool find_and_replace(char *opcode, int opcode_size, char *pattern, int size, char *insert)
{
	for (int i=0;i<opcode_size - size;i++){
		bool match = true;
		for (int j=0;j<size;j++)
			if (pattern[j] != opcode[i + j]){
				match = false;
				break;
			}
		if (match){
			for (int j=0;j<size;j++)
				opcode[i + j] = insert[j];
			return true;
		}
	}
	return false;
}

struct IncludeTranslationData
{
	int const_off;
	int func_off;
	int var_off;
	Script *source;
};

void relink_calls(Script *s, Script *target, IncludeTranslationData &d)
{
	//msg_write("relink ----" + s->filename + " : " + d.source->filename + " -> " + target->filename + "  ---------");
	for (Node *c: s->syntax->nodes){
		// keep commands... just redirect var/const/func
		if (c->script != d.source)
			continue;

		if (c->kind != KIND_CONSTANT)
			if (c->script->filename.find(".kaba") < 0)
				continue;

		//msg_write(p2s(c->script));
		if (c->kind == KIND_VAR_GLOBAL){
			c->link_no += d.var_off;
			c->script = target;
		}else if (c->kind == KIND_CONSTANT){
			c->link_no += d.const_off;
			c->script = target;
		}else if ((c->kind == KIND_FUNCTION) or (c->kind == KIND_VAR_FUNCTION)){
			c->link_no += d.func_off;
			c->script = target;
		}
	}

	// we might need some constructors later on...
	for (Class *t: s->syntax->classes)
		for (ClassFunction &f: t->functions)
			if (f.script == d.source){
				if (f.script->filename.find(".kaba") < 0)
					continue;
				f.script = target;
				f.nr += d.func_off;
			}
}

IncludeTranslationData import_deep(SyntaxTree *dest, SyntaxTree *source)
{
	IncludeTranslationData d;
	d.const_off = dest->constants.num;
	d.var_off = dest->root_of_all_evil.var.num;
	d.func_off = dest->functions.num;
	d.source = source->script;

	for (Constant *c: source->constants){
		Constant *cc = new Constant(c->type);
		cc->name = c->name;
		cc->set(*c);
		dest->constants.add(cc);
	}

	// don't fully include internal libraries
	if (source->script->filename.find(".kaba") < 0)
		return d;

	dest->root_of_all_evil.var.append(source->root_of_all_evil.var);

	for (Function *f: source->functions){
		Function *ff = dest->AddFunction(f->name, f->return_type);
		*ff = *f;
		// keep block pointing to include file...
	}
	dest->classes.append(source->classes);

	//int asm_off = a->AsmBlocks.num;
	for (AsmBlock &ab: source->asm_blocks){
		dest->asm_blocks.add(ab);
	}

	return d;
}

void find_all_includes_rec(Script *s, Set<Script*> &includes)
{
	for (Script *i: s->syntax->includes){
		//if (i->filename.find(".kaba") < 0)
		//	continue;
		includes.add(i);
		find_all_includes_rec(i, includes);
	}
}

// only for "os"
void import_includes(Script *s)
{
	Set<Script*> includes;
	find_all_includes_rec(s, includes);
	Array<IncludeTranslationData> da;
	for (Script *i: includes)
		da.add(import_deep(s->syntax, i->syntax));


	for (IncludeTranslationData &d: da)
		relink_calls(s, s, d);

	// we need to also correct the includes, since we kept the blocks/commands there
	for (Script *i: includes)
		for (IncludeTranslationData &d: da)
			relink_calls(i, s, d);
}

void Script::LinkFunctions()
{
	for (Asm::WantedLabel &l: functions_to_link){
		string name = l.name.substr(10, -1);
		bool found = false;
		foreachi(Function *f, syntax->functions, i)
			if (f->name == name){
				*(int*)&opcode[l.pos] = (int_p)func[i] - (syntax->asm_meta_info->code_origin + l.pos + 4);
				found = true;
				break;
			}
		if (!found)
			DoErrorLink("could not link function: " + name);
	}
	for (int n: function_vars_to_link){
		int64 p = (n + 0xefef0000);
		int64 q = (int_p)func[n];
		if (!find_and_replace(opcode, opcode_size, (char*)&p, config.pointer_size, (char*)&q))
			DoErrorLink("could not link function as variable: " + syntax->functions[n]->name);
	}


	// link virtual functions into vtables
	for (Class *t: syntax->classes){
		t->link_virtual_table();

		if (config.compile_os){
			for (int i=0; i<t->vtable.num; i++)
				memcpy((char*)t->_vtable_location_compiler_ + i*config.pointer_size, &t->vtable[i], config.pointer_size);
		}
	}
}

// generate opcode
void Script::Compiler()
{
	Asm::CurrentMetaInfo = syntax->asm_meta_info;

	if (config.compile_os)
		import_includes(this);

	syntax->MapLocalVariablesToStack();

	syntax->BreakDownComplicatedCommands();
#ifdef ScriptDebug
	syntax->Show();
#endif


	syntax->SimplifyRefDeref();
	syntax->SimplifyShiftDeref();

	syntax->PreProcessor();

	if (config.verbose)
		syntax->Show();

	AllocateMemory();
	AllocateStack();

	memory_size = 0;
	MapGlobalVariablesToMemory();
	if (!config.compile_os)
		MapConstantsToMemory();

	AllocateOpcode();



// compiling an operating system?
//   -> create an entry point for execution... so we can just call Opcode like a function
	if (config.add_entry_point)
		CompileOsEntryPoint();

	if (config.compile_os)
		MapConstantsToOpcode();



	syntax->PreProcessorAddresses();

	if (config.verbose)
		syntax->Show();


// compile functions into Opcode
	CompileFunctions(opcode, opcode_size);

// link functions
	LinkFunctions();


// "task" for the first execution of main() -> ThreadOpcode
	if (!config.compile_os)
		CompileTaskEntryPoint();




	if (config.add_entry_point)
		LinkOsEntryPoint();


	// initialize global objects
	if (!config.compile_os)
		init_all_global_objects(syntax, g_var);

	//_expand(Opcode,OpcodeSize);

	if (__first_execution)
		__waiting_mode = WAITING_MODE_FIRST;
	else
		__waiting_mode = WAITING_MODE_NONE;

	if (show_compiler_stats){
		msg_write("--------------------------------");
		msg_write(format("Opcode: %db, Memory: %db",opcode_size,memory_size));
	}
}

};



