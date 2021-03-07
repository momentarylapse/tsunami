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

namespace kaba {


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
			list->add2(Asm::INST_ADD, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(d, 4));
		else
			list->add2(Asm::INST_ADD, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(d, 1));
	} else if (d < 0) {
		if (d < -120)
			list->add2(Asm::INST_SUB, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(-d, 4));
		else
			list->add2(Asm::INST_SUB, Asm::param_reg(Asm::REG_ESP), Asm::param_imm(-d, 1));
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
	init_func *ff = (init_func*)cf->address;
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

static int64 _opcode_rand_state_ = 10000;

// randomly pick a page address +/- 1gb around &kaba::init
void* get_nice_random_addr() {
	int64 p = ((int_p)&init) & 0xfffffffffffff000;
	_opcode_rand_state_ = (_opcode_rand_state_ * 1664525 + 1013904223);
	//printf("%p      %04x\n", p, (int)_opcode_rand_state_ & 0x3ffff);
	if (_opcode_rand_state_ & 0x40000)
		p += (int64)(_opcode_rand_state_ & 0x3ffff) * 4096;
	else
		p -= (int64)(_opcode_rand_state_ & 0x3ffff) * 4096;
	return (void*)p;

}

void* get_nice_memory(int64 size, bool executable, Script *script) {
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
	int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE;
	if (executable) {
		prot |= PROT_EXEC;
		flags |= MAP_EXECUTABLE;
	}

	// try in 32bit distance from current opcode
	for (int i=0; i<10000; i++) {
		void *addr0 = get_nice_random_addr();
		//opcode = (char*)mmap(addr0, max_opcode, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE | MAP_32BIT, -1, 0);
		mem = (char*)mmap(addr0, size, prot, flags, -1, 0);
		if ((int_p)mem != -1) {
			if (config.verbose)
				printf("%d  %p  ->  %p\n", i, addr0, mem);
			if (labs((int_p)mem - (int_p)addr0) < 2000000000)
				return mem;
			//munmap(mem, size);
			if (config.verbose)
				msg_write("...try again");
		}
		if (i > 5000) {
			prot |= PROT_EXEC;
			flags |= MAP_EXECUTABLE | MAP_FIXED;
		}
	}
	//script->do_error(format("och, can't allocate %dkb memory in a usable address range", size/1024));

	// no?...ok, try anywhere
	mem = (char*)mmap(nullptr, size, prot, flags, -1, 0);
	if ((int_p)mem == -1)
		mem = nullptr;
#endif

	// failed...
	if (!mem) {
		script->do_error(format("could not allocate executable memory: %s", strerror(errno)));
		mem = new char[size];
	}

	return mem;
}


void Script::allocate_opcode() {
	int max_opcode = MAX_OPCODE;
	if (config.compile_os)
		max_opcode *= 10;

	opcode = (char*)get_nice_memory(max_opcode, true, this);
	if (config.verbose)
		msg_write("opcode:  " + p2s(opcode));

	if (config.override_code_origin)
		syntax->asm_meta_info->code_origin = config.code_origin;
	else
		syntax->asm_meta_info->code_origin = (int_p)opcode;
	opcode_size = 0;
}


int mem_size_needed(const Class *c) {
	int memory_size = 0;
	if (c->vtable.num > 0)
		memory_size += config.pointer_size * c->vtable.num;
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

void Script::allocate_memory() {
	memory_size = mem_size_needed_total(syntax->base_class);

	memory = (char*)get_nice_memory(memory_size, false, this);
	if (config.verbose)
		msg_write("memory:  " + p2s(memory));
	memory_size = 0;
}

void _update_const_locations(const Class *ns) {
	for (auto *c: weak(ns->constants)) {
		c->address = c->p();
	}
	for (auto *c: weak(ns->classes))
		_update_const_locations(c);
}

// DEPRECATED???
void Script::update_constant_locations() {
	_update_const_locations(syntax->base_class);
}

void Script::_map_global_variables_to_memory(char *mem, int &offset, char *address, const Class *name_space) {
	for (auto *v: weak(name_space->static_variables)) {
		if (v->is_extern()) {
			v->memory = get_external_link(v->cname(name_space, name_space->owner->base_class));
			if (!v->memory)
				do_error_link(format("external variable '%s' was not linked", v->cname(name_space, name_space->owner->base_class)));
		} else {
			int size_aligned = mem_align(v->type->size, 4);
			v->memory = &address[offset];
			offset += size_aligned;
			//memset(v->memory, 0, v->type->size); // reset all global variables to 0
		}
	}
	for (auto *cc: weak(name_space->classes))
		_map_global_variables_to_memory(mem, offset, address, cc);
}

void Script::map_global_variables_to_memory() {
	// global variables -> into Memory
	int override_offset = 0;
	if (config.override_variables_offset)
		_map_global_variables_to_memory(memory, override_offset, (char*)config.variables_offset, syntax->base_class);
	else
		_map_global_variables_to_memory(memory, memory_size, memory, syntax->base_class);
}

void Script::align_opcode() {
	int ocs_new = mem_align(opcode_size, config.function_align);
	for (int i=opcode_size;i<ocs_new;i++)
		opcode[i] = 0x90;
	opcode_size = ocs_new;
}

static int OCORA;
void Script::CompileOsEntryPoint() {
	if (!base_class()->get_func("main", TypeVoid, {}))
		if (!syntax->imported_symbols->get_func("main", TypeVoid, {}))
			do_error("os entry point: no 'void main()' found");

	// call
	Asm::add_instruction(opcode, opcode_size, Asm::INST_CALL, Asm::param_imm(0, 4));
	TaskReturnOffset = opcode_size;
	OCORA = Asm::OCParam;
	align_opcode();
}

shared<Node> check_const_used(shared<Node> n, Script *me) {
	if (n->kind == NodeKind::CONSTANT) {
		n->as_const()->used = true;
		/*if (n->as_const()->owner != me->syntax)
			msg_error("evil const " + n->as_const()->name);*/
	}
	return n;
}

void remap_virtual_tables(Script *s, char *mem, int &offset, char *address, const Class *ct) {
	// vtables -> no data yet...
	if (ct->vtable.num > 0) {
		Class *t = const_cast<Class*>(ct);
		t->_vtable_location_compiler_ = &mem[offset];
		t->_vtable_location_target_ = &address[offset];
		offset += config.pointer_size * t->vtable.num;
		for (Constant *c: weak(s->syntax->base_class->constants))
			if ((c->type == TypePointer) and (c->as_int64() == (int_p)t->vtable.data)) {
				c->as_int64() = (int_p)t->_vtable_location_target_;
			}
	}

	for (auto *c: weak(ct->classes))
		remap_virtual_tables(s, mem, offset, address, c);
}


void _map_constants_to_memory(char *mem, int &offset, char *address, const Class *ns) {

	// also allow named constants... might be imported by other scripts!
	for (Constant *c: weak(ns->constants))
		if (c->used or ((c->name[0] != '-') and !config.compile_os)) {
			c->address = (void*)(address + offset);//ns->owner->asm_meta_info->code_origin + offset);
		//	c->address = &mem[offset];
			c->map_into(&mem[offset], (char*)c->address);
			offset += mem_align(c->mapping_size(), 4);
		}

	/*foreachi(Constant *c, syntax->constants, i)
		if ((c->mapping_size() > 1) and used[i]) {
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += mem_align(c->mapping_size(), 4);
		}
	foreachi(Constant *c, syntax->constants, i)
		if ((c->mapping_size() == 1) and used[i]) {
			cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
			c->map_into(&opcode[opcode_size], cnst[i]);
			opcode_size += 1;
		}*/

	for (auto *c: weak(ns->classes))
		_map_constants_to_memory(mem, offset, address, c);
}

void Script::map_constants_to_memory(char *mem, int &offset, char *address) {

	remap_virtual_tables(this, mem, offset, address, syntax->base_class);


	syntax->transform([&](shared<Node> n) { return check_const_used(n, this); });

	/*int n = 0;
	for (bool b: used)
		if (b)
			n ++;
	msg_write(format("     USED:    %d / %d", n, used.num));
	int size0 = opcode_size;

	foreachi(Constant *c, syntax->constants, i) {
		cnst[i] = (char*)(syntax->asm_meta_info->code_origin + opcode_size);
		c->map_into(&opcode[opcode_size], cnst[i]);
		opcode_size += mem_align(c->mapping_size(), 4);
	}
	int uncompressed = opcode_size - size0;
	opcode_size = size0;*/


	_map_constants_to_memory(mem, offset, address, syntax->base_class);

	//msg_write(format("    compressed:  %d  ->  %d", uncompressed, opcode_size - size0));
}

void Script::map_constants_to_opcode() {
	map_constants_to_memory(opcode, opcode_size, (char*)(int_p)syntax->asm_meta_info->code_origin);
	align_opcode();
}

void Script::LinkOsEntryPoint() {
	auto *f = base_class()->get_func("main", TypeVoid, {});
	if (!f)
		f = syntax->imported_symbols->get_func("main", TypeVoid, {});
	if (!f)
		do_error_internal("os entry point missing...");

	int lll = (int_p)f->address - syntax->asm_meta_info->code_origin - TaskReturnOffset;
	//printf("insert   %d  an %d\n", lll, OCORA);
	//msg_write(lll);
	//msg_write(i2h(lll,4));
	*(int*)&opcode[OCORA] = lll;
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
	if (source->script->filename.basename().find(".kaba") < 0)
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

void find_all_includes_rec(Script *s, Set<Script*> &includes) {
	for (Script *i: weak(s->syntax->includes)) {
		//if (i->filename.find(".kaba") < 0)
		//	continue;
		includes.add(i);
		find_all_includes_rec(i, includes);
	}
}

// only for "os"
void import_includes(Script *s) {
	Set<Script*> includes;
	find_all_includes_rec(s, includes);

	for (Script *i: includes)
		import_deep(s->syntax, i->syntax);
}

void Script::link_functions() {
	for (Asm::WantedLabel &l: functions_to_link) {
		string name = l.name.substr(10, -1);
		bool found = false;
		for (Function *f: syntax->functions)
			if (f->name == name) {
				*(int*)&opcode[l.pos] = (int_p)f->address - (syntax->asm_meta_info->code_origin + l.pos + 4);
				found = true;
				break;
			}
		if (!found)
			do_error_link("could not link function: " + name);
	}
	for (int n: function_vars_to_link) {
		int64 p = (n + 0xefef0000);
		int64 q = (int_p)syntax->functions[n]->address;
		if (!find_and_replace(opcode, opcode_size, (char*)&p, config.pointer_size, (char*)&q))
			do_error_link("could not link function as variable: " + syntax->functions[n]->signature());
	}


}

void Script::link_virtual_functions_into_vtable(const Class *c) {
	Class *t = const_cast<Class*>(c);
	t->link_virtual_table();

	/*if (config.compile_os)*/{
		for (int i=0; i<t->vtable.num; i++) {
			memcpy((char*)t->_vtable_location_compiler_ + i*config.pointer_size, &t->vtable[i], config.pointer_size);
		}
	}

	for (auto *cc: weak(c->classes))
		link_virtual_functions_into_vtable(cc);
}

struct DynamicLibraryImport {
	string filename;
	void *handle;
	void *get_symbol(const string &name, Script *s) {
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
DynamicLibraryImport *get_dynamic_lib(const string &filename, Script *s) {
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
	for (auto *c: weak(s->base_class->constants))
		if (c->name == "KABA_LINK" and c->type == TypeString) {
			DynamicLibraryImport *d = nullptr;
			auto xx = c->as_string().explode("\n");
			for (string &x: xx) {
				if (x.num == 0)
					continue;
				if (x[0] == '\t') {
					if (d and x.find(":")) {
						auto y = x.substr(1, -1).explode(":");
						link_external(y[0], d->get_symbol(y[1], s->script));
					}
				} else {
					d = get_dynamic_lib(x, s->script);
				}
			}
		}

}

// generate opcode
void Script::compile() {
	Asm::CurrentMetaInfo = syntax->asm_meta_info.get();

	if (config.compile_os)
		import_includes(this);

	parse_magic_linker_string(syntax);
	
	syntax->digest();

	allocate_memory();
	map_global_variables_to_memory();

	// useful because some constants might have ended up outside of RIP-relative addressing!
	if (!config.compile_os)
		map_constants_to_memory(memory, memory_size, memory);

	allocate_opcode();



// compiling an operating system?
//   -> create an entry point for execution... so we can just call Opcode like a function
	if (config.add_entry_point)
		CompileOsEntryPoint();

	if (config.compile_os)
		map_constants_to_opcode();



	if (config.verbose)
		syntax->show("compile:b");

	syntax->pre_processor_addresses();

	if (config.verbose)
		syntax->show("compile:eval-addr");


// compile functions into Opcode
	compile_functions(opcode, opcode_size);

// link functions
	link_functions();
	link_virtual_functions_into_vtable(syntax->base_class);
	if (config.compile_os)
		link_virtual_functions_into_vtable(syntax->imported_symbols.get());





	if (config.add_entry_point)
		LinkOsEntryPoint();


	// initialize global objects
	if (!config.compile_os)
		init_all_global_objects(syntax, syntax->base_class);

	//_expand(Opcode,OpcodeSize);

	if (show_compiler_stats) {
		msg_write("--------------------------------");
		msg_write(format("Opcode: %db, Memory: %db", opcode_size, memory_size));
	}
	if (config.allow_output_stage("dasm"))
		msg_write(Asm::disassemble(opcode, opcode_size));
}

};



