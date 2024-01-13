/*
 * Compiler.cpp
 *
 *  Created on: 27 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "Compiler.h"
#include "BackendAmd64.h"
#include "BackendX86.h"
#include "BackendARM.h"
#include "Serializer.h"
#include "../Interpreter.h"
#include "../asm/asm.h"
#include "../../base/set.h"
#include "../../base/iter.h"
#include "../../os/msg.h"
#include <stdio.h>
#include <functional>
#if HAS_LIB_DL
#include <dlfcn.h>
#endif


#if defined(OS_LINUX) || defined(OS_MAC) // || defined(OS_MINGW)
	#include <sys/mman.h>
	#if (!defined(__x86_64__)) && (!defined(__amd64__))
		#define MAP_32BIT		0
	#endif
#endif
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	#include <windows.h>
	#include "../../base/iter.h"
	#ifdef DELETE
	#undef DELETE
	#endif
#endif
#include <errno.h>

namespace kaba {

Compiler::Compiler(Module *m) {
	module = m;
	tree = m->tree.get();
	context = m->context;
}

Compiler::~Compiler() {
}

int LocalOffset,LocalOffsetMax;

/*int get_func_temp_size(Function *f) {
}*/

inline int add_temp_var(int size) {
	LocalOffset += size;
	if (LocalOffset > LocalOffsetMax)
		LocalOffsetMax = LocalOffset;
	return LocalOffset;
}


int TaskReturnOffset;


#define CallRel32OCSize			5
#define AfterWaitOCSize			10

void add_esp_add(Asm::InstructionWithParamsList *list,int d) {
	if (d > 0) {
		if (d > 120)
			list->add2(Asm::InstID::ADD, Asm::param_reg(Asm::RegID::ESP), Asm::param_imm(d, 4));
		else
			list->add2(Asm::InstID::ADD, Asm::param_reg(Asm::RegID::ESP), Asm::param_imm(d, 1));
	} else if (d < 0) {
		if (d < -120)
			list->add2(Asm::InstID::SUB, Asm::param_reg(Asm::RegID::ESP), Asm::param_imm(-d, 4));
		else
			list->add2(Asm::InstID::SUB, Asm::param_reg(Asm::RegID::ESP), Asm::param_imm(-d, 1));
	}
}

void try_init_global_var(const Class *type, char* g_var, SyntaxTree *ps) {
	if (type->is_array()) {
		for (int i=0;i<type->array_length;i++)
			try_init_global_var(type->param[0], g_var + i * type->param[0]->size, ps);
		return;
	}
	Function *cf = type->get_default_constructor();
	if (!cf) {
		if (type->needs_constructor())
			ps->do_error("static variable without default constructor...");
		return;
	}
	typedef void init_func(void *);
	//msg_write("global init: " + v.type->name);
	auto ff = (init_func*)(int_p)cf->address;
	if (ff)
		ff(g_var);
}

void init_all_global_objects(SyntaxTree *ps, const Class *c) {
	for (auto *v: weak(c->static_variables))
		if (!v->is_extern())
			try_init_global_var(v->type, (char*)v->memory, ps);
	for (auto *cc: weak(c->classes))
		init_all_global_objects(ps, cc);
}

// randomly pick a page address +/- 1gb around &kaba::init
void* get_nice_random_addr() {
	static int64 _opcode_rand_state_ = 10000;

	int64 p = ((int_p)&init) & 0xfffffffffffff000;
	_opcode_rand_state_ = (_opcode_rand_state_ * 1664525 + 1013904223);
	//printf("%p      %04x\n", p, (int)_opcode_rand_state_ & 0x3ffff);
	if (_opcode_rand_state_ & 0x40000)
		p += (int64)(_opcode_rand_state_ & 0x3ffff) * 4096;
	else
		p -= (int64)(_opcode_rand_state_ & 0x3ffff) * 4096;
	return (void*)p;
}

void* get_nice_memory(int64 size, bool executable, Module *module) {
	if (size == 0)
		return nullptr;
	void *mem = nullptr;
	size = mem_align(size, 4096);
	if (config.verbose)
		msg_write("get nice...");

#if defined(OS_WINDOWS) || defined(OS_MINGW)
	int prot = PAGE_READWRITE;
	if (executable) {
		prot = PAGE_EXECUTE_READWRITE;
	}
#elif defined(OS_MAC)
	msg_error("FIXME: kaba compiler - mmap() parameters for MacOS");
	int prot = 0;//PROT_READ | PROT_WRITE;
	int flags = 0;//MAP_PRIVATE | MAP_ANON | MAP_FIXED_NOREPLACE;
	//if (executable) {
	//	prot |= PROT_EXEC;
#else // OS_LINUX
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED_NOREPLACE;
	if (executable) {
		prot |= PROT_EXEC;
		flags |= MAP_EXECUTABLE;
	}
#endif

	// try in 32bit distance from current opcode
	for (int i=0; i<10000; i++) {
		void *addr0 = get_nice_random_addr();
#if defined(OS_WINDOWS) || defined(OS_MINGW)
		mem = (char *)VirtualAlloc(addr0, size, MEM_COMMIT | MEM_RESERVE, prot);
#else
		//opcode = (char*)mmap(addr0, max_opcode, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, -1, 0);
		mem = (char*)mmap(addr0, size, prot, flags, -1, 0);
		if ((int_p)mem == -1)
			mem = nullptr;
#endif

		if (mem) {
			if (config.verbose)
				printf("%d  %p  ->  %p\n", i, addr0, mem);
			if (labs((int_p)mem - (int_p)addr0) < 2000000000)
				return mem;
			//munmap(mem, size);
			//if (config.verbose)
				msg_write("...try again");
		}
		if (i > 5000) {
#if defined(OS_WINDOWS) || defined(OS_MINGW)
#elif defined(OS_MAC)
			// FIXME
			//prot |= PROT_EXEC;
			//flags |= MAP_FIXED;
#else
			prot |= PROT_EXEC;
			flags |= MAP_EXECUTABLE | MAP_FIXED;
#endif
		}
	}
	//module->do_error(format("och, can't allocate %dkb memory in a usable address range", size/1024));

	// no?...ok, try anywhere
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	mem = (char*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else
	mem = (char*)mmap(nullptr, size, prot, flags, -1, 0);
	if ((int_p)mem == -1)
		mem = nullptr;
#endif

	// failed...
	if (!mem) {
		module->do_error(format("could not allocate executable memory: %s", strerror(errno)));
		mem = new char[size];
	}

	return mem;
}


void Compiler::allocate_opcode() {
	int max_opcode = MAX_OPCODE;
	if (config.fully_linear_output)
		max_opcode *= 10;

	module->opcode = (char*)get_nice_memory(max_opcode, true, module);
	if (config.verbose)
		msg_write("opcode:  " + p2s(module->opcode));

	if (config.override_code_origin)
		tree->asm_meta_info->code_origin = config.code_origin;
	else
		tree->asm_meta_info->code_origin = (int_p)module->opcode;
	module->opcode_size = 0;
}


int mem_size_needed(const Class *c) {
	int memory_size = 0;
	if (c->vtable.num > 0)
		memory_size += config.target.pointer_size * c->vtable.num;
	for (auto *v: weak(c->static_variables))
		memory_size += mem_align(v->type->size, 4);
	for (auto *cc: weak(c->constants))
		memory_size += mem_align(cc->mapping_size(), 4);

	for (auto *t: weak(c->classes))
		memory_size += mem_size_needed(t);
	return memory_size;
}

int mem_size_needed_total(const Class *c) {
	int size = mem_size_needed(c);
	for (auto *v: weak(c->static_variables))
		size += mem_align(v->type->size, 4);
	for (auto *cc: weak(c->classes))
		size += mem_size_needed_total(cc);
	return size;
}

void Compiler::allocate_memory() {
	module->memory_size = mem_size_needed_total(tree->base_class);

	module->memory = (char*)get_nice_memory(module->memory_size, false, module);
	if (config.verbose)
		msg_write("memory:  " + p2s(module->memory));
	module->memory_size = 0;
}

void Compiler::_map_global_variables_to_memory(char *mem, int &offset, char *address, const Class *name_space) {
	auto external = context->external.get();
	for (auto *v: weak(name_space->static_variables)) {
		if (v->is_extern()) {
			auto m = external->get_link(v->cname(name_space, name_space->owner->base_class));
			if (!v->memory or m)
				v->memory = m;
			if (!v->memory)
				module->do_error_link(format("external variable '%s' was not linked", v->cname(name_space, name_space->owner->base_class)));
		} else if (!v->memory) {
			int size_aligned = mem_align(v->type->size, 4);
			v->memory = &address[offset];
			offset += size_aligned;
			//memset(v->memory, 0, v->type->size); // reset all global variables to 0
		}
	}
	for (auto *cc: weak(name_space->classes))
		_map_global_variables_to_memory(mem, offset, address, cc);
}

void Compiler::map_global_variables_to_memory() {
	// global variables -> into Memory
	int override_offset = 0;
	if (config.override_variables_offset)
		_map_global_variables_to_memory(module->memory, override_offset, (char*)config.variables_offset, tree->base_class);
	else
		_map_global_variables_to_memory(module->memory, module->memory_size, module->memory, tree->base_class);
}

void Compiler::align_opcode() {
	int ocs_new = mem_align(module->opcode_size, config.target.function_align);
	for (int i=module->opcode_size;i<ocs_new;i++)
		module->opcode[i] = 0x90;
	module->opcode_size = ocs_new;
}

Function *get_entry_point(SyntaxTree *tree) {
	if (auto f = tree->base_class->get_func("main", TypeVoid, {}))
		return f;
	auto rmain = tree->global_scope.find("main", -1);
	if (rmain.num > 0)
		if (rmain[0]->kind == NodeKind::FUNCTION)
			return rmain[0]->as_func();
	tree->do_error("no entry point 'func main()' found");
	return nullptr;
}

static int OCORA;
void Compiler::compile_os_entry_point() {
	// call main()  (linked later)
	Asm::add_instruction(module->opcode, module->opcode_size, Asm::InstID::CALL, Asm::param_imm(0, 4));
	TaskReturnOffset = module->opcode_size;
	OCORA = Asm::OCParam;
	align_opcode();
}

shared<Node> check_const_used(shared<Node> n, Module *me) {
	if (n->kind == NodeKind::CONSTANT) {
		n->as_const()->used = true;
		/*if (n->as_const()->owner != me->syntax)
			msg_error("evil const " + n->as_const()->name);*/
	}
	return n;
}

void remap_virtual_tables(Module *s, char *mem, int &offset, char *address, const Class *ct) {
	// vtables -> no data yet...
	if (ct->vtable.num > 0) {
		Class *t = const_cast<Class*>(ct);
		t->_vtable_location_compiler_ = &mem[offset];
		t->_vtable_location_target_ = &address[offset];
		offset += config.target.pointer_size * t->vtable.num;
		for (Constant *c: weak(s->tree->base_class->constants))
			if ((c->type == TypePointer) and (c->as_int64() == (int_p)t->vtable.data))
				c->as_int64() = (int_p)t->_vtable_location_target_;
	}

	for (auto *c: weak(ct->classes))
		remap_virtual_tables(s, mem, offset, address, c);
}


void _map_constants_to_memory(char *mem, int &offset, char *address, const Class *ns) {

	// also allow named constants... might be imported by other modules!
	for (Constant *c: weak(ns->constants))
		if (c->used or ((c->name[0] != '-') and !config.fully_linear_output)) {
			c->address_runtime = (void*)(address + offset);//ns->owner->asm_meta_info->code_origin + offset);
			c->address_compiler = &mem[offset];
			c->map_into(&mem[offset], (char*)c->address_runtime);
			offset += mem_align(c->mapping_size(), 4);
		}

	/*for (auto&& [i,c]: enumerate(syntax->constants))
		if ((c->mapping_size() > 1) and used[i]) {
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += mem_align(c->mapping_size(), 4);
		}
	for (auto&& [i,c]: enumerate(syntax->constants))
		if ((c->mapping_size() == 1) and used[i]) {
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += 1;
		}*/

	for (auto *c: weak(ns->classes))
		_map_constants_to_memory(mem, offset, address, c);
}

void Compiler::map_constants_to_memory(char *mem, int &offset, char *address) {

	remap_virtual_tables(module, mem, offset, address, tree->base_class);


	tree->transform([this] (shared<Node> n) {
		return check_const_used(n, module);
	});

	/*int n = 0;
	for (bool b: used)
		if (b)
			n ++;
	msg_write(format("     USED:    %d / %d", n, used.num));
	int size0 = opcode_size;

	for (auto&& [i,c]: enumerate(syntax->constants)) {
		cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
		c->map_into(&opcode[opcode_size], cnst[i]);
		opcode_size += mem_align(c->mapping_size(), 4);
	}
	int uncompressed = opcode_size - size0;
	opcode_size = size0;*/


	_map_constants_to_memory(mem, offset, address, tree->base_class);

	//msg_write(format("    compressed:  %d  ->  %d", uncompressed, opcode_size - size0));
}

void Compiler::map_constants_to_opcode() {
    //remap_virtual_tables(this, opcode, opcode_size, (char*)(int_p)syntax->asm_meta_info->code_origin, syntax->base_class);

	map_constants_to_memory(module->opcode, module->opcode_size, (char*)(int_p)tree->asm_meta_info->code_origin);
	//align_opcode();
}



void Compiler::map_address_constants_to_opcode() {
	for (auto f: module->functions()) {
		tree->transform_block(f->block.get(), [this] (shared<Node> n) {
			if (n->kind == NodeKind::ADDRESS) {
				//msg_write(format("ADDRESS   %x", n->link_no));
				memcpy(&module->opcode[module->opcode_size], &n->link_no, config.target.pointer_size);
				n->link_no = config.code_origin + module->opcode_size;
				module->opcode_size += config.target.pointer_size;
				//msg_write(format("  %x", n->link_no));
			}
			return n;
		});
	}
	align_opcode();
}

void Compiler::link_os_entry_point() {
	auto f = get_entry_point(tree);

	int64 lll = f->address - tree->asm_meta_info->code_origin - TaskReturnOffset;
	//printf("insert   %d  an %d\n", lll, OCORA);
	//msg_write(lll);
	//msg_write(i2h(lll,4));
	*(int*)&module->opcode[OCORA] = (int)lll;
}

bool find_and_replace(char *opcode, int opcode_size, char *pattern, int size, char *insert) {
	for (int i=0;i<opcode_size - size;i++) {
		bool match = true;
		for (int j=0;j<size;j++)
			if (pattern[j] != opcode[i + j]) {
				match = false;
				break;
			}
		if (match) {
			for (int j=0;j<size;j++)
				opcode[i + j] = insert[j];
			return true;
		}
	}
	return false;
}


void import_deep(SyntaxTree *dest, SyntaxTree *source) {
	dest->base_class->constants.append(source->base_class->constants);
	source->base_class->constants.clear();

	// don't fully include internal libraries
	if (source->module->filename.basename().find(".kaba") < 0)
		return;

	dest->base_class->static_variables.append(source->base_class->static_variables);
	source->base_class->static_variables.clear();
	dest->functions.append(source->functions);
	source->functions.clear();
	dest->base_class->classes.append(source->base_class->classes);
	source->base_class->classes.clear();

	//int asm_off = a->AsmBlocks.num;
	for (AsmBlock &ab: source->asm_blocks) {
		dest->asm_blocks.add(ab);
	}
}

void find_all_includes_rec(Module *s, base::set<Module*> &includes) {
	for (Module *i: weak(s->tree->includes)) {
		//if (i->filename.find(".kaba") < 0)
		//	continue;
		includes.add(i);
		find_all_includes_rec(i, includes);
	}
}

// only for "os"
void import_includes(Module *s) {
	base::set<Module*> includes;
	find_all_includes_rec(s, includes);

	for (Module *i: includes)
		import_deep(s->tree.get(), i->tree.get());
}

void link_raw_function_pointers(Module *m) {
	for (auto &c: m->constants())
		if (c->type == TypeFunctionCodeRef) {
			auto f = (Function*)(int_p)c->as_int64();
			c->as_int64() = f->address;

			// remap
			memcpy(c->address_compiler, &c->as_int64(), config.target.pointer_size);
		}
}

void Compiler::link_functions() {
	for (Asm::WantedLabel &l: module->functions_to_link) {
		string name = l.name.sub(10);
		bool found = false;
		for (Function *f: tree->functions)
			if (f->name == name) {
				*(int*)&module->opcode[l.pos] = (int)(f->address - (tree->asm_meta_info->code_origin + l.pos + 4));
				found = true;
				break;
			}
		if (!found)
			module->do_error_link("could not link function: " + name);
	}

	link_raw_function_pointers(module);

	// unused?
	for (int n: module->function_vars_to_link) {
		int64 p = (n + 0xefef0000);
		int64 q = tree->functions[n]->address;
		if (!find_and_replace(module->opcode, module->opcode_size, (char*)&p, config.target.pointer_size, (char*)&q))
			module->do_error_link("could not link function as variable: " + tree->functions[n]->signature());
	}


}

void Compiler::link_virtual_functions_into_vtable(const Class *c) {
	Class *t = const_cast<Class*>(c);
	t->link_virtual_table();

	for (int i=0; i<t->vtable.num; i++)
		memcpy((char*)t->_vtable_location_compiler_ + i*config.target.pointer_size, &t->vtable[i], config.target.pointer_size);

	for (auto *cc: weak(c->classes))
		link_virtual_functions_into_vtable(cc);
}

struct DynamicLibraryImport {
	string filename;
	void *handle;
	void *get_symbol(const string &name, Module *s) {
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
DynamicLibraryImport *get_dynamic_lib(const string &filename, Module *s) {
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

void parse_magic_linker_string(SyntaxTree *s) {
	auto external = s->module->context->external.get();
	for (auto *c: weak(s->base_class->constants))
		if (c->name == "KABA_LINK" and c->type == TypeString) {
			DynamicLibraryImport *d = nullptr;
			auto xx = c->as_string().explode("\n");
			for (string &x: xx) {
				if (x.num == 0)
					continue;
				if (x[0] == '\t') {
					if (d and x.find(":")) {
						auto y = x.sub(1).explode(":");
						external->link(y[0], d->get_symbol(y[1], s->module));
					}
				} else {
					d = get_dynamic_lib(x, s->module);
				}
			}
		}

}



#ifdef OS_WINDOWS

// https://pmeerw.net/blog/programming/RtlAddFunctionTable.html

typedef union _UNWIND_CODE {
	struct {
		uint8_t CodeOffset;
		uint8_t UnwindOp : 4;
		uint8_t OpInfo : 4;
	};
	USHORT FrameOffset;
} UNWIND_CODE;

typedef struct _UNWIND_INFO {
	uint8_t Version : 3;
	uint8_t Flags : 5;
	uint8_t SizeOfProlog;
	uint8_t CountOfCodes;
	uint8_t FrameRegister : 4;
	uint8_t FrameOffset : 4;
	UNWIND_CODE UnwindCode[1];
	/*	UNWIND_CODE MoreUnwindCode[((CountOfCodes + 1) & ~1) - 1];
	 *	OPTIONAL ULONG ExceptionHandler;
	 *	OPTIONAL ULONG ExceptionData[]; */
} UNWIND_INFO;

// very experimental
void register_functions(Module* s) {
	int nf = s->functions().num;
	RUNTIME_FUNCTION* function_table = new RUNTIME_FUNCTION[nf];
	UNWIND_INFO* unwind_info = new UNWIND_INFO[nf];
	for (auto&& [i,f]: enumerate(s->functions())) {
		unwind_info[i].Version = 1;
		unwind_info[i].Flags = UNW_FLAG_EHANDLER;
		unwind_info[i].SizeOfProlog = 0;
		unwind_info[i].CountOfCodes = 0;
		unwind_info[i].FrameRegister = 0;
		unwind_info[i].FrameOffset = 0;
		*(DWORD*)&unwind_info[i].UnwindCode = 0;

		function_table[i].BeginAddress = (int_p)f->block->_start - (int_p)s->opcode;
		function_table[i].EndAddress = (int_p)f->block->_end - (int_p)s->opcode;
		function_table[i].UnwindInfoAddress = (int_p)&unwind_info[i] - (int_p)s->opcode;
	}
	RtlAddFunctionTable(function_table, nf, (int_p)s->opcode);


	if (false) {
		DWORD64 dyn_base = 0;
		auto q = RtlLookupFunctionEntry((DWORD64)&register_functions, &dyn_base, NULL);
		printf("lookup 'reg_func' %p %llx\n", q, dyn_base); // no function table entry
		q = RtlLookupFunctionEntry(s->functions()[0]->address, &dyn_base, NULL);
		printf("lookup 'f[0]' %p %llx\n", q, dyn_base); // no function table entry
		fflush(stdout);
	}
}
#endif

void Compiler::compile(Module *m) {
	Compiler c(m);
	c._compile();
}

// generate opcode
void Compiler::_compile() {
	Asm::CurrentMetaInfo = tree->asm_meta_info.get();

	if (config.fully_linear_output)
		import_includes(module);

	parse_magic_linker_string(tree);

	tree->digest();

	allocate_memory();
	map_global_variables_to_memory();

	// useful because some constants might have ended up outside of RIP-relative addressing!
	if (!config.fully_linear_output)
		map_constants_to_memory(module->memory, module->memory_size, module->memory);

	allocate_opcode();



// compiling an operating system?
//   -> create an entry point for execution... so we can just call Opcode like a function
	if (config.add_entry_point)
		compile_os_entry_point();

	if (config.fully_linear_output)
		map_constants_to_opcode();



	if (config.verbose)
		tree->show("compile:b");

	tree->pre_processor_addresses();

	if (config.fully_linear_output)
		map_address_constants_to_opcode();

	if (config.verbose)
		tree->show("compile:eval-addr");


// compile functions into Opcode
	compile_functions(module->opcode, module->opcode_size);

// link functions
	link_functions();
	link_virtual_functions_into_vtable(tree->base_class);
	link_virtual_functions_into_vtable(tree->implicit_symbols.get());
	if (config.fully_linear_output) {
		//tree->do_error("FIXME: compile os... vtable in imported?");
		msg_write("   WARNING: compile os... vtable in imported?");
		//link_virtual_functions_into_vtable(tree->imported_symbols.get());
	}





	if (config.add_entry_point)
		link_os_entry_point();


	// initialize global objects
	if (!config.fully_linear_output)
		init_all_global_objects(tree, tree->base_class);

	//_expand(Opcode,OpcodeSize);

	if (module->show_compiler_stats) {
		msg_write("--------------------------------");
		msg_write(format("Opcode: %db, Memory: %db", module->opcode_size, module->memory_size));
	}
	if (config.allow_output_stage("dasm"))
		msg_write(Asm::disassemble(module->opcode, module->opcode_size));


#ifdef OS_WINDOWS
	register_functions(module);
#endif
}

bool is_func(shared<Node> n) {
	return (n->kind == NodeKind::CALL_FUNCTION or n->kind == NodeKind::CALL_VIRTUAL or n->kind == NodeKind::FUNCTION);
}

int check_needed(SyntaxTree *tree, Function *f) {
	int ref_count = 0;
	tree->transform([&ref_count, f](shared<Node> n) {
		if (is_func(n) and n->as_func() == f)
			ref_count ++;
		return n;
	});

	if (f->is_static() and f->name == "main")
		ref_count ++;
	if (f->virtual_index >= 0)
		ref_count ++;
	// well, for now, only allow these functions:
	if (f->name != Identifier::Func::ASSIGN and f->name != Identifier::Func::DELETE and f->name != Identifier::Func::INIT)
		ref_count ++;

	return ref_count;
}

Backend *create_backend(Serializer *s) {
	if (config.target.instruction_set == Asm::InstructionSet::AMD64)
		return new BackendAmd64(s);
	if (config.target.instruction_set == Asm::InstructionSet::X86)
		return new BackendX86(s);
	if (config.target.is_arm())
		return new BackendARM(s);
	s->module->do_error("unable to create a backend for the architecture");
	return nullptr;
}

void Compiler::assemble_function(int index, Function *f, Asm::InstructionWithParamsList *list) {
	if (config.verbose and config.allow_output(f, "asm"))
		msg_write("serializing " + f->long_name() + " -------------------");
	f->show("asm");


	// skip unused functions?
	if (config.remove_unused)
		if (check_needed(tree, f) == 0)
			return;

	if (config.verbose and config.allow_output(f, "ser:0"))
		f->block->show(TypeVoid);

	if (config.target.interpreted) {
		auto x = new Serializer(module, list);
		x->cur_func_index = index;
		x->serialize_function(f);
		x->fix_return_by_ref();
		if (!module->interpreter)
			module->interpreter = new Interpreter(module);
		module->interpreter->add_function(f, x);
		return;
	}


	auto x = new Serializer(module, list);
	x->cur_func_index = index;
	x->serialize_function(f);
	x->fix_return_by_ref();
	auto be = create_backend(x);

	try {
		be->process(f, index);
		be->assemble();
	} catch (Exception &e) {
		throw e;
	} catch (Asm::Exception &e) {
		throw Exception(e, module, f);
	}
	module->functions_to_link.append(be->list->wanted_label);
	delete be;
	delete x;

}


string function_link_name(Function *f);

void function_update_address(Function *f, Asm::InstructionWithParamsList *list) {
	f->address = list->_label_value(f->_label);
	for (Block *b: f->all_blocks()) {
		b->_start = (void*)list->_label_value(b->_label_start);
		b->_end = (void*)list->_label_value(b->_label_end);
	}
}

void Compiler::compile_functions(char *oc, int &ocs) {
	auto *list = new Asm::InstructionWithParamsList(0);
	Array<int> func_offset;

	auto external = context->external.get();

	// link external functions
	int func_no = 0;
	for (Function *f: tree->functions) {
		if (f->is_template() or  f->is_macro()) {
			//msg_write("SKIP COMPILE " + f->signature());
		} else if (f->is_extern()) {
			string name = function_link_name(f);
			f->address = (int_p)external->get_link(name);
			if (f->address == 0)
				f->address = (int_p)external->get_link(f->cname(f->owner()->base_class));
			if (f->address == 0)
				module->do_error_link(format("external function '%s' not linkable", name));
		} else {
			f->_label = list->create_label("_FUNC_" + i2s(func_no ++));
		}
	}

	// create assembler
	for (auto&& [i,f]: enumerate(tree->functions)) {
		func_offset.add(list->num);
		if (!f->is_extern() and !f->is_template() and !f->is_macro()) {
			assemble_function(i, f, list);
		}
	}
	func_offset.add(list->num);


	//if (config.verbose and config.allow_output(cur_func, "comp:x"))
	//	list->show();

	// assemble into opcode
	try {
		list->optimize(oc, ocs);
		list->compile(oc, ocs);
	} catch(Asm::Exception &e) {
		list->show();
		Function *f = nullptr;
		for (int i=0; i<func_offset.num; i++)
			if (e.line >= func_offset[i] and e.line < func_offset[i+1]) {
				f = tree->functions[i];
				break;
			}
		msg_write(f->long_name());
		throw Exception(e, module, f);
	}


	// get function addresses
	for (auto *f: tree->functions)
		if (!f->is_extern() and !f->is_template() and !f->is_macro())
			function_update_address(f, list);

	if (!config.target.interpreted)
		delete list;
}

};


