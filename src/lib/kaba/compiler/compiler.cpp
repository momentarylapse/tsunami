#include "../kaba.h"
#include "../../file/file.h"
#include "../../base/set.h"
#include <stdio.h>
#include <functional>
#if HAS_LIB_DL
#include <dlfcn.h>
#endif


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

void add_esp_add(Asm::InstructionWithParamsList *list,int d)
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

void try_init_global_var(const Class *type, char* g_var)
{
	if (type->is_array()){
		for (int i=0;i<type->array_length;i++)
			try_init_global_var(type->parent, g_var + i * type->parent->size);
		return;
	}
	ClassFunction *cf = type->get_default_constructor();
	if (!cf)
		return;
	typedef void init_func(void *);
	//msg_write("global init: " + v.type->name);
	init_func *ff = (init_func*)cf->func->address;
	if (ff)
		ff(g_var);
}

void init_all_global_objects(SyntaxTree *ps)
{
	for (Variable *v: ps->root_of_all_evil.var)
		if (!v->is_extern)
			try_init_global_var(v->type, (char*)v->memory);
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
	if (size == 0)
		return nullptr;
	void *mem = nullptr;
	size = mem_align(size, 4096);
	if (config.verbose)
		msg_write("get nice...");

#if defined(OS_WINDOWS) || defined(OS_MINGW)
	mem = (char*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else

	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	if (executable){
		prot |= PROT_EXEC;
		flags |= MAP_EXECUTABLE;
	}

	// try in 32bit distance from current opcode
	for (int i=0; i<100; i++){
		void *addr0 = get_nice_random_addr();
		//opcode = (char*)mmap(addr0, max_opcode, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, -1, 0);
		mem = (char*)mmap(addr0, size, prot, flags, -1, 0);
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
	mem = (char*)mmap(nullptr, size, prot, flags, -1, 0);
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


void Script::allocate_opcode()
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

void Script::allocate_memory()
{
	memory_size = 0;
	for (auto *t: syntax->classes)
		if (t->vtable.num > 0)
			memory_size += config.pointer_size * t->vtable.num;
	for (auto *v: syntax->root_of_all_evil.var)
		memory_size += mem_align(v->type->size, 4);
	for (auto *c: syntax->constants)
		memory_size += c->mapping_size();

	memory = (char*)get_nice_memory(memory_size, false);
	if (config.verbose)
		msg_write("memory:  " + p2s(memory));
	memory_size = 0;
}

void Script::update_constant_locations()
{
	for (auto *c: syntax->constants)
		c->address = c->p();
}

void Script::map_global_variables_to_memory()
{
	// global variables -> into Memory
	int override_offset = 0;
	for (Variable *v: syntax->root_of_all_evil.var){
		if (v->is_extern){
			v->memory = GetExternalLink(v->name);
			if (!v->memory)
				do_error_link("external variable " + v->name + " was not linked");
		}else{
			int size_aligned = mem_align(v->type->size, 4);
			if (config.override_variables_offset){
				v->memory = (char*)(int_p)(config.variables_offset + override_offset);
				override_offset += size_aligned;
			}else{
				v->memory = &memory[memory_size];
				memory_size += size_aligned;
				//v->memory_owner = true;
			}
			//memset(v->memory, 0, v->type->size); // reset all global variables to 0
		}
	}
}

void Script::align_opcode()
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
		if (ff->long_name == "main")
			nf = index;
	// call
	if (nf>=0)
		Asm::AddInstruction(opcode, opcode_size, Asm::INST_CALL, Asm::param_imm(0, 4));
	TaskReturnOffset=opcode_size;
	OCORA = Asm::OCParam;
	align_opcode();
}

Node *check_const_used(Node *n, Script *me)
{
	if (n->kind == KIND_CONSTANT){
		n->as_const()->used = true;
		if (n->as_const()->owner != me->syntax)
			msg_error("evil const " + n->as_const()->name);
	}
	return n;
}

void remap_virtual_tables(Script *s, char *mem, int &offset)
{
	// vtables -> no data yet...
	for (auto *ct: s->syntax->classes)
		if (ct->vtable.num > 0){
			Class *t = const_cast<Class*>(ct);
			t->_vtable_location_compiler_ = &mem[offset];
			t->_vtable_location_target_ = &mem[offset];//(void*)(s->syntax->asm_meta_info->code_origin + opcode_size);
			offset += config.pointer_size * t->vtable.num;
			for (Constant *c: s->syntax->constants)
				if ((c->type == TypePointer) and (c->as_int64() == (int_p)t->vtable.data)){
					c->as_int64() = (int_p)t->_vtable_location_target_;
				}
		}
}

void Script::map_constants_to_memory(char *mem, int &offset)
{
	//update_constant_locations();
	//return;

	remap_virtual_tables(this, mem, offset);


//	syntax->transform([&](Node* n){ return check_const_used(n, this); });

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


	for (Constant *c: syntax->constants)
		if (/*c->used*/ true){
//			c->address = (void*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->address = &mem[offset];
			c->map_into(&mem[offset], (char*)c->address);
			offset += mem_align(c->mapping_size(), 4);
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
}

void Script::map_constants_to_opcode()
{
	map_constants_to_memory(opcode, opcode_size);
	align_opcode();
}

void Script::LinkOsEntryPoint()
{
	Function *f = nullptr;
	for (Function *ff: syntax->functions)
		if (ff->long_name == "main")
			f = ff;
	if (f){
		int lll = (int_p)f->address - syntax->asm_meta_info->code_origin - TaskReturnOffset;
		//printf("insert   %d  an %d\n", lll, OCORA);
		//msg_write(lll);
		//msg_write(d2h(&lll,4,false));
		*(int*)&opcode[OCORA] = lll;
	}
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

Node *conv_relink_calls(Node *c, Script *s, Script *target, IncludeTranslationData &d)
{

#if 0
	// keep commands... just redirect var/const/func
	if (c->script != d.source)
		return c;

	if (c->kind != KIND_CONSTANT)
		if (c->script->filename.find(".kaba") < 0)
			return c;

	//msg_write(p2s(c->script));
	if (c->kind == KIND_VAR_GLOBAL){
		c->script = target;
	}else if (c->kind == KIND_CONSTANT){
		c->script = target;
	}else if ((c->kind == KIND_FUNCTION_CALL) or (c->kind == KIND_FUNCTION_POINTER)){
		c->script = target;
	}
#endif
	return c;
}

void relink_calls(Script *s, Script *target, IncludeTranslationData &d)
{
	//msg_write("relink ----" + s->filename + " : " + d.source->filename + " -> " + target->filename + "  ---------");
	s->syntax->transform([&](Node *n){ return conv_relink_calls(n, s, target, d); });

	// we might need some constructors later on...
	for (const Class *t: s->syntax->classes)
		for (ClassFunction &f: t->functions)
			if (f.script == d.source){
				if (f.script->filename.find(".kaba") < 0)
					continue;
				f.script = target;
			}
}

IncludeTranslationData import_deep(SyntaxTree *dest, SyntaxTree *source)
{
	// ksdjfhksjdhfkjsdhfj
	IncludeTranslationData d;
	d.const_off = dest->constants.num;
	d.var_off = dest->root_of_all_evil.var.num;
	d.func_off = dest->functions.num;
	d.source = source->script;

	dest->constants.append(source->constants);
	source->constants.clear();

	// don't fully include internal libraries
	if (source->script->filename.find(".kaba") < 0)
		return d;

	dest->root_of_all_evil.var.append(source->root_of_all_evil.var);

	for (Function *f: source->functions){
		Function *ff = dest->add_function(f->name, f->return_type);
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
	s->do_error_internal("deep import is currently not supported, can't compile OS... sorry. Complain to Michi");
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

void Script::link_functions()
{
	for (Asm::WantedLabel &l: functions_to_link){
		string name = l.name.substr(10, -1);
		bool found = false;
		for (Function *f: syntax->functions)
			if (f->name == name){
				*(int*)&opcode[l.pos] = (int_p)f->address - (syntax->asm_meta_info->code_origin + l.pos + 4);
				found = true;
				break;
			}
		if (!found)
			do_error_link("could not link function: " + name);
	}
	for (int n: function_vars_to_link){
		int64 p = (n + 0xefef0000);
		int64 q = (int_p)syntax->functions[n]->address;
		if (!find_and_replace(opcode, opcode_size, (char*)&p, config.pointer_size, (char*)&q))
			do_error_link("could not link function as variable: " + syntax->functions[n]->signature());
	}


	// link virtual functions into vtables
	for (const Class *ct: syntax->classes){
		Class *t = const_cast<Class*>(ct);
		t->link_virtual_table();

		/*if (config.compile_os)*/{
			for (int i=0; i<t->vtable.num; i++)
				memcpy((char*)t->_vtable_location_compiler_ + i*config.pointer_size, &t->vtable[i], config.pointer_size);
		}
	}
}

struct DynamicLibraryImport
{
	string filename;
	void *handle;
	void *get_symbol(const string &name, Script *s)
	{
#if HAS_LIB_DL
		if (!handle)
			return nullptr;
		void *p = dlsym(handle, name.c_str());
		if (!p)
			s->do_error_link("can't load symbol '" + name + "' from library " + filename);
		return p;
#else
		return nullptr;
#endif
	}
};
static Array<DynamicLibraryImport*> dynamic_libs;
DynamicLibraryImport *get_dynamic_lib(const string &filename, Script *s)
{
#if HAS_LIB_DL
	for (auto &d: dynamic_libs)
		if (d->filename == filename)
			return d;
	DynamicLibraryImport *d = new DynamicLibraryImport;
	d->filename = filename;
	d->handle = dlopen(filename.c_str(), RTLD_NOW);
	if (!d->handle)
		s->do_error_link("can't load external library " + filename + ": " + dlerror());
	dynamic_libs.add(d);
	return d;
#else
	s->do_error_link("can't load dynamic lib, program is compiled without support for dl library");
#endif
	return nullptr;
}

void parse_magic_linker_string(SyntaxTree *s)
{
	for (auto *c: s->constants)
		if (c->name == "KABA_LINK" and c->type == TypeString){
			DynamicLibraryImport *d = nullptr;
			auto xx = c->as_string().explode("\n");
			for (string &x: xx){
				if (x.num == 0)
					continue;
				if (x[0] == '\t'){
					if (d and x.find(":")){
						auto y = x.substr(1, -1).explode(":");
						LinkExternal(y[0], d->get_symbol(y[1], s->script));
					}
				}else{
					d = get_dynamic_lib(x, s->script);
				}
			}
		}

}

// generate opcode
void Script::compile()
{
	Asm::CurrentMetaInfo = syntax->asm_meta_info;

	if (config.compile_os)
		import_includes(this);

	parse_magic_linker_string(syntax);

	syntax->MapLocalVariablesToStack();

	syntax->BreakDownComplicatedCommands();

	syntax->SimplifyRefDeref();
	syntax->SimplifyShiftDeref();

	syntax->PreProcessor();
	syntax->MakeFunctionsInline();

	if (config.verbose)
		syntax->Show("comp:a");

	allocate_memory();
	map_global_variables_to_memory();
	if (!config.compile_os)
		map_constants_to_memory(memory, memory_size);

	allocate_opcode();



// compiling an operating system?
//   -> create an entry point for execution... so we can just call Opcode like a function
	if (config.add_entry_point)
		CompileOsEntryPoint();

	if (config.compile_os)
		map_constants_to_opcode();



	syntax->PreProcessorAddresses();

	if (config.verbose)
		syntax->Show("comp:b");


// compile functions into Opcode
	compile_functions(opcode, opcode_size);

// link functions
	link_functions();





	if (config.add_entry_point)
		LinkOsEntryPoint();


	// initialize global objects
	if (!config.compile_os)
		init_all_global_objects(syntax);

	//_expand(Opcode,OpcodeSize);

	if (show_compiler_stats){
		msg_write("--------------------------------");
		msg_write(format("Opcode: %db, Memory: %db", opcode_size, memory_size));
	}
}

};



