#include "../script.h"
#include "../../file/file.h"
#include "../../base/set.h"

#ifdef OS_LINUX
	#include <sys/mman.h>
	#if (!defined(__x86_64__)) && (!defined(__amd64__))
		#define MAP_32BIT		0
	#endif
#endif
#ifdef OS_WINDOWS
	#include "windows.h"
#endif
#include <errno.h>

namespace Script{


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

void try_init_global_var(Type *type, char* g_var)
{
	if (type->is_array){
		for (int i=0;i<type->array_length;i++)
			try_init_global_var(type->parent, g_var + i * type->parent->size);
		return;
	}
	ClassFunction *cf = type->GetDefaultConstructor();
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
		try_init_global_var(v.type, g_var[i]);
}

void Script::AllocateMemory()
{
	// get memory size needed
	memory_size = 0;
	for (int i=0;i<syntax->root_of_all_evil.var.num;i++)
		if (!syntax->root_of_all_evil.var[i].is_extern)
			memory_size += mem_align(syntax->root_of_all_evil.var[i].type->size, 4);

	// constants
	foreachi(Constant &c, syntax->constants, i){
		int s = c.type->size;
		if (c.type == TypeString){
			// const string -> variable length   (+ super array frame)
			s = c.value.num + config.super_array_size;
		}
		memory_size += mem_align(s, 4);
	}
	foreach(Type *t, syntax->types)
		if (t->vtable.num > 0)
			memory_size += config.pointer_size;

	// allocate
	if (memory_size > 0){
#ifdef OS_WINDOWS
		memory = (char*)VirtualAlloc(NULL, memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else
		//Memory = (char*)mmap(0, MemorySize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS /*| MAP_EXECUTABLE*/ | MAP_32BIT, -1, 0);
		memory = (char*)mmap(0, mem_align(memory_size, 4096), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS /*| MAP_EXECUTABLE*/ | MAP_32BIT, -1, 0);
		if (memory == (char*)-1)
			memory = new char[memory_size];
			//DoErrorInternal(format("can not allocate memory, (%d) ", errno) + strerror(errno));
#endif
		//Memory = new char[MemorySize];
	}
}

void Script::AllocateStack()
{
	// use your own stack if needed
	//   wait() used -> needs to switch stacks ("tasks")
	__stack = NULL;
	foreach(Command *cmd, syntax->commands){
		if (cmd->kind == KIND_COMPILER_FUNCTION)
			if ((cmd->link_no == COMMAND_WAIT) or (cmd->link_no == COMMAND_WAIT_RT) or (cmd->link_no == COMMAND_WAIT_ONE_FRAME)){
				__stack = new char[config.stack_size];
				break;
			}
	}
}

void Script::AllocateOpcode()
{
	int max_opcode = SCRIPT_MAX_OPCODE;
	if (config.compile_os)
		max_opcode *= 10;
	// allocate some memory for the opcode......    has to be executable!!!   (important on amd64)
#ifdef OS_WINDOWS
	opcode=(char*)VirtualAlloc(NULL,max_opcode,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
#else
	opcode = (char*)mmap(0, max_opcode, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, -1, 0);
#endif
	if ((long)opcode == -1){
		//DoErrorInternal(string("Script:  could not allocate executable memory: ") + strerror(errno));
		msg_error(string("Script:  could not allocate executable memory: ") + strerror(errno));
		opcode = new char[max_opcode];
	}
	if (config.override_code_origin)
		syntax->asm_meta_info->code_origin = config.code_origin;
	else
		syntax->asm_meta_info->code_origin = (long)opcode;
	opcode_size = 0;
}

void Script::MapConstantsToMemory()
{
	// constants -> Memory
	cnst.resize(syntax->constants.num);
	foreachi(Constant &c, syntax->constants, i){
		cnst[i] = &memory[memory_size];
		int s = c.type->size;
		if (c.type == TypeString){
			// const string -> variable length
			s = syntax->constants[i].value.num;

			*(void**)&memory[memory_size] = &memory[memory_size + config.super_array_size]; // .data
			*(int*)&memory[memory_size + config.pointer_size    ] = s; // .num
			*(int*)&memory[memory_size + config.pointer_size + 4] = 0; // .reserved
			*(int*)&memory[memory_size + config.pointer_size + 8] = 1; // .item_size
			memory_size += config.super_array_size;
		}
		memcpy(&memory[memory_size], (void*)c.value.data, s);
		memory_size += mem_align(s, 4);
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
				g_var[i] = (char*)(long)(memory_size + config.variables_offset);
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
}

void Script::MapConstantsToOpcode()
{
	cnst.resize(syntax->constants.num);

	// vtables -> no data yet...
	foreach(Type *t, syntax->types)
		if (t->vtable.num > 0){
			t->_vtable_location_compiler_ = &opcode[opcode_size];
			t->_vtable_location_target_ = (void*)(opcode_size + syntax->asm_meta_info->code_origin);
			opcode_size += config.pointer_size * t->vtable.num;
			foreach(Constant &c, syntax->constants)
				if ((c.type == TypePointer) and (*(int*)c.value.data == (int)(long)t->vtable.data))
					memcpy(c.value.data, &t->_vtable_location_target_, config.pointer_size);
		}

	// put all constants into Opcode!
	foreachi(Constant &c, syntax->constants, i){
		if (config.compile_os){// && (c.type == TypeCString)){
			cnst[i] = (char*)(opcode_size + syntax->asm_meta_info->code_origin);
			int s = c.type->size;
			if (c.type == TypeString){
				// const string -> variable length
				s = syntax->constants[i].value .num;

				*(void**)&opcode[opcode_size] = (char*)(opcode_size + syntax->asm_meta_info->code_origin + config.super_array_size); // .data
				*(int*)&opcode[opcode_size + config.pointer_size    ] = s; // .num
				*(int*)&opcode[opcode_size + config.pointer_size + 4] = 0; // .reserved
				*(int*)&opcode[opcode_size + config.pointer_size + 8] = 1; // .item_size
				opcode_size += config.super_array_size;
			}else if (c.type == TypeCString){
				s = syntax->constants[i].value .num;
			}
			memcpy(&opcode[opcode_size], (void*)c.value.data, s);
			opcode_size += s;

			// cstring -> 0 terminated
			if (c.type == TypeCString)
				opcode[opcode_size ++] = 0;
		}
	}

	AlignOpcode();
}

void Script::LinkOsEntryPoint()
{
	int nf = -1;
	foreachi(Function *ff, syntax->functions, index)
		if (ff->name == "main")
			nf = index;
	if (nf >= 0){
		int lll = (long)func[nf] - syntax->asm_meta_info->code_origin - TaskReturnOffset;
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
		__continue_execution = NULL;
		return;
	}

	Asm::InstructionWithParamsList *list = new Asm::InstructionWithParamsList(0);

	int label_first = list->add_label("_first_execution");

	__first_execution = (t_func*)&__thread_opcode[__thread_opcode_size];
	// intro
	list->add2(Asm::INST_PUSH, Asm::param_reg(Asm::REG_EBP)); // within the actual program
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EBP), Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_ESP), Asm::param_deref_imm((long)&__stack[config.stack_size], 4)); // start of the script stack
	list->add2(Asm::INST_PUSH, Asm::param_reg(Asm::REG_EBP)); // address of the old stack
	AddEspAdd(list, -12); // space for wait() task data
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EBP), Asm::param_reg(Asm::REG_ESP));
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_EAX), Asm::param_imm(WAITING_MODE_NONE, 4)); // "reset"
	list->add2(Asm::INST_MOV, Asm::param_deref_imm((long)&__waiting_mode, 4), Asm::param_reg(Asm::REG_EAX));

	// call main()
	list->add2(Asm::INST_CALL, Asm::param_imm((long)_main_, 4));

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
	list->add2(Asm::INST_MOV, Asm::param_deref_imm((long)&__stack[config.stack_size - 4], 4), Asm::param_reg(Asm::REG_EBP)); // save the external ebp
	list->add2(Asm::INST_MOV, Asm::param_reg(Asm::REG_ESP), Asm::param_deref_imm((long)&__stack[config.stack_size - 16], 4)); // to the eIP of the script
	list->add2(Asm::INST_POP, Asm::param_reg(Asm::REG_EAX));
	list->add2(Asm::INST_ADD, Asm::param_reg(Asm::REG_EAX), Asm::param_imm(AfterWaitOCSize, 4));
	list->add2(Asm::INST_JMP, Asm::param_reg(Asm::REG_EAX));
	//list->add2(Asm::inst_leave);
	//list->add2(Asm::inst_ret);
	/*OCAddChar(0x90);
	OCAddChar(0x90);
	OCAddChar(0x90);*/

	list->Compile(__thread_opcode, __thread_opcode_size);

	__first_execution = (t_func*)(long)list->label[label_first].value;
	__continue_execution = (t_func*)(long)list->label[label_cont].value;

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

void relink_calls(Script *s, Script *a, IncludeTranslationData &d)
{
	foreach(Command *c, s->syntax->commands){
		// keep commands... just redirect var/const/func
		//msg_write(p2s(c->script));
		if (c->script != d.source)
			continue;
		if (c->kind == KIND_VAR_GLOBAL){
			c->link_no += d.var_off;
			c->script = a;
		}else if (c->kind == KIND_CONSTANT){
			c->link_no += d.const_off;
			c->script = a;
		}else if ((c->kind == KIND_FUNCTION) or (c->kind == KIND_VAR_FUNCTION)){
			c->link_no += d.func_off;
			c->script = a;
		}
	}

	// we might need some constructors later on...
	foreach(Type *t, s->syntax->types)
		foreach(ClassFunction &f, t->function)
			if (f.script == d.source){
				f.script = a;
				f.nr += d.func_off;
			}
}

IncludeTranslationData import_deep(SyntaxTree *a, SyntaxTree *b)
{
	IncludeTranslationData d;
	d.const_off = a->constants.num;
	d.var_off = a->root_of_all_evil.var.num;
	d.func_off = a->functions.num;
	d.source = b->script;

	a->constants.append(b->constants);

	a->root_of_all_evil.var.append(b->root_of_all_evil.var);

	foreach(Function *f, b->functions){
		Function *ff = a->AddFunction(f->name, f->return_type);
		*ff = *f;
		// keep block pointing to include file...
	}
	a->types.append(b->types);

	//int asm_off = a->AsmBlocks.num;
	foreach(AsmBlock &ab, b->asm_blocks){
		a->asm_blocks.add(ab);
	}

	return d;
}

void find_all_includes_rec(Script *s, Set<Script*> &includes)
{
	foreach(Script *i, s->syntax->includes){
		if (i->filename.find(".kaba") < 0)
			continue;
		includes.add(i);
		find_all_includes_rec(i, includes);
	}
}

void import_includes(Script *s)
{
	Set<Script*> includes;
	find_all_includes_rec(s, includes);
	Array<IncludeTranslationData> da;
	foreach(Script *i, includes)
		da.add(import_deep(s->syntax, i->syntax));

	// we need to also correct the includes, since we kept the blocks/commands there
	foreach(Script *i, includes){
		foreach(IncludeTranslationData &d, da){
			relink_calls(s, s, d);
			relink_calls(i, s, d);
		}
	}
}

// generate opcode
void Script::Compiler()
{
	msg_db_f("Compiler",2);
	Asm::CurrentMetaInfo = syntax->asm_meta_info;

	if (config.compile_os)
		import_includes(this);

	syntax->MapLocalVariablesToStack();

	syntax->BreakDownComplicatedCommands();
#ifdef ScriptDebug
	syntax->Show();
#endif

	syntax->Simplify();
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
	foreach(Asm::WantedLabel &l, functions_to_link){
		string name = l.name.substr(10, -1);
		bool found = false;
		foreachi(Function *f, syntax->functions, i)
			if (f->name == name){
				*(int*)&opcode[l.pos] = (long)func[i] - (syntax->asm_meta_info->code_origin + l.pos + 4);
				found = true;
				break;
			}
		if (!found)
			DoErrorLink("could not link function: " + name);
	}
	foreach(int n, function_vars_to_link){
		long p = (n + 0xefef0000);
		long q = (long)func[n];
		if (!find_and_replace(opcode, opcode_size, (char*)&p, config.pointer_size, (char*)&q))
			DoErrorLink("could not link function as variable: " + syntax->functions[n]->name);
	}

// link virtual functions into vtables
	foreach(Type *t, syntax->types){
		t->LinkVirtualTable();

		if (config.compile_os){
			for (int i=0; i<t->vtable.num; i++)
				memcpy((char*)t->_vtable_location_compiler_ + i*config.pointer_size, &t->vtable[i], config.pointer_size);
		}
	}


// "task" for the first execution of main() -> ThreadOpcode
	if (!config.compile_os)
		CompileTaskEntryPoint();




	if (config.add_entry_point)
		LinkOsEntryPoint();


	// initialize global objects
	if (!config.compile_os)
		init_all_global_objects(syntax, g_var);

	//msg_db_out(1,GetAsm(Opcode,OpcodeSize));

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



