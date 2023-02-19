#include "Module.h"
#include "Context.h"
#include "Interpreter.h"
#include "CompilerConfiguration.h"
#include "syntax/SyntaxTree.h"
#include "parser/Parser.h"
#include "compiler/Compiler.h"
#include "dynamic/dynamic.h"
#include "lib/lib.h"
#include "../os/filesystem.h"
#include "../os/file.h"
#include "../os/msg.h"

#ifdef OS_LINUX
	#include <sys/mman.h>
#endif
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	#include <windows.h>
#endif


namespace kaba {

Path absolute_module_path(const Path &filename);


Array<shared<Module>> loading_module_stack;


void Module::load(const Path &_filename, bool _just_analyse) {
	loading_module_stack.add(this);
	just_analyse = _just_analyse;

	filename = absolute_module_path(_filename);

	tree->base_class->name = filename.basename().replace(".kaba", "");

	auto parser = new Parser(tree.get());
	tree->parser = parser;

	try {
		tree->default_import();

	// read file
		string buffer = os::fs::read_text(filename);
		parser->parse_buffer(buffer, just_analyse);


		if (!just_analyse)
			Compiler::compile(this);

	} catch (os::fs::FileError &e) {
		loading_module_stack.pop();
		do_error("module file not loadable: " + str(filename));
	} catch (Exception &e) {
		loading_module_stack.pop();
		throw e;
	}
	loading_module_stack.pop();
}

void Module::do_error(const string &str, int override_token) {
#ifdef CPU_ARM
	msg_error(str);
#endif
	tree->do_error(str, override_token);
}

void Module::do_error_internal(const string &str) {
	do_error("internal compiler error: " + str, 0);
}

void Module::do_error_link(const string &str) {
	do_error(str, 0);
}

void Module::set_variable(const string &name, void *data) {
	//msg_write(name);
	for (auto *v: weak(tree->base_class->static_variables))
		if (v->name == name) {
			memcpy(v->memory, data, v->type->size);
			return;
		}
	msg_error("Module.set_variable: variable " + name + " not found");
}

Module::Module(Context *c, const Path &_filename) {
    context = c;
	filename = _filename;
	used_by_default = false;

	show_compiler_stats = !config.compile_silently;

	just_analyse = false;

	opcode = nullptr;
	opcode_size = 0;

	memory = nullptr;
	memory_size = 0;

	tree = new SyntaxTree(this);
}

Module::~Module() {
	int r = 0;
	if (opcode) {
		#if defined(OS_WINDOWS) || defined(OS_MINGW)
			VirtualFree(opcode, 0, MEM_RELEASE);
		#else
			r = munmap(opcode, MAX_OPCODE);
		#endif
	}
	if (r != 0)
		msg_error("munmap...op");
	if (memory and memory_size > 0) {
		#if defined(OS_WINDOWS) || defined(OS_MINGW)
			VirtualFree(memory, 0, MEM_RELEASE);
		#else
			r = munmap(memory, memory_size);
		#endif
	}
	if (r != 0)
		msg_error("munmap...mem");
	//msg_write(string2("-----------            Memory:         %p",Memory));
}

void *Module::match_function(const string &name, const string &return_type, const Array<string> &param_types) {
	auto ns = base_class();
	// match
	for (Function *f: tree->functions)
		if (f->cname(ns).match(name) and (f->literal_return_type->cname(ns) == return_type) and (param_types.num == f->num_params)) {

			bool params_ok = true;
			for (int j=0;j<param_types.num;j++)
				if (f->literal_param_type[j]->cname(ns) != param_types[j])
					params_ok = false;
			if (params_ok) {
				if (just_analyse)
					return (void*)(int_p)0xdeadbeaf;
				else
					return (void*)(int_p)f->address;
			}
		}

	return nullptr;
}

void print_var(void *p, const string &name, const Class *t) {
	msg_write(t->name + " " + name + " = " + var2str(p, t));
}

void Module::show_vars(bool include_consts) {
	for (auto *v: weak(tree->base_class->static_variables))
		print_var(v->memory, v->name, v->type);
	/*if (include_consts)
		for ([i,c]: pre_script->Constant)
			print_var((void*)g_var[i], c.name, c.type);*/
}

Array<const Class*> Module::classes() {
	return weak(tree->base_class->classes);
}

Array<Function*> Module::functions() {
	return tree->functions;
}

Array<Variable*> Module::variables() {
	return weak(tree->base_class->static_variables);
}

Array<Constant*> Module::constants() {
	return weak(tree->base_class->constants);
}

const Class *Module::base_class() {
	return tree->base_class;
}


}
