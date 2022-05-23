/*----------------------------------------------------------------------------*\
| Kaba                                                                         |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../file/file.h"
#include "kaba.h"
#include "syntax/Parser.h"
#include "Interpreter.h"
#include <cassert>

#include "../config.h"

#ifdef OS_LINUX
	#include <sys/mman.h>
#endif
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	#include <windows.h>
#endif

namespace kaba {

string Version = "0.19.19.0";

//#define ScriptDebug


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Module *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	filename = s->filename;
}

Exception::Exception(const Asm::Exception &e, Module *s, Function *f) :
	Asm::Exception(e)
{
	filename = s->filename;
	text = format("assembler: %s, %s", message(), f->long_name());
}

string Exception::message() const {
	return format("%s, %s", Asm::Exception::message(), filename);
}


shared_array<Module> public_modules;




Path absolute_module_path(const Path &filename) {
	if (filename.is_relative())
		return (config.directory << filename).absolute().canonical();
	else
		return filename.absolute().canonical();
}


shared<Module> load(const Path &filename, bool just_analyse) {
	//msg_write("loading " + filename.str());

	auto _filename = absolute_module_path(filename);

	// already loaded?
	for (auto ps: public_modules)
		if (ps->filename == _filename)
			return ps;
	
	// load
	shared<Module> s = new Module();
	s->load(filename, just_analyse);

	// store module in database
	public_modules.add(s);
	return s;
}

shared<Module> create_for_source(const string &buffer, bool just_analyse) {
	shared<Module> s = new Module;
	s->just_analyse = just_analyse;
	s->syntax->parser = new Parser(s->syntax);
	s->syntax->default_import();
	s->syntax->parser->parse_buffer(buffer, just_analyse);

	if (!just_analyse)
		s->compile();
	return s;
}

void remove_module(Module *s) {

	// remove from normal list
	for (int i=0;i<public_modules.num;i++)
		if (public_modules[i] == s)
			public_modules.erase(i);
}

void delete_all_modules(bool even_immortal, bool force) {
	public_modules.clear();

#if 0
	// try to erase them...
	auto to_del = public_modules;
	foreachb(Module *s, to_del)
		if ((!s->syntax->flag_immortal) or even_immortal)
			Remove(s);

	// undead... really KILL!
	if (force){
		foreachb(Module *s, _dead_scripts_)
			delete(s);
		_dead_scripts_.clear();
	}

	//ScriptResetSemiExternalData();

	
	/*msg_write("------------------------------------------------------------------");
	msg_write(mem_used);
	for (int i=0;i<num_ps;i++)
		msg_write(string2("  fehlt:   %s  %p  (%d)",ppn[i],ppp[i],pps[i]));
	*/
#endif
}

VirtualTable* get_vtable(const VirtualBase *p) {
	return *(VirtualTable**)p;
}

const Class *_dyn_type_in_namespace(const VirtualTable *p, const Class *ns) {
	for (auto *c: weak(ns->classes)) {
		if (c->_vtable_location_target_ == p)
			return c;
		auto t = _dyn_type_in_namespace(p, c);
		if (t)
			return t;
	}
	return nullptr;
}

// TODO...namespace
const Class *get_dynamic_type(const VirtualBase *p) {
	auto *pp = get_vtable(p);
	for (auto s: public_modules) {
		auto t = _dyn_type_in_namespace(pp, s->syntax->base_class);
		if (t)
			return t;
	}
	return nullptr;
}

Array<shared<Module>> loading_module_stack;


void Module::load(const Path &_filename, bool _just_analyse) {
	loading_module_stack.add(this);
	just_analyse = _just_analyse;

	filename = absolute_module_path(_filename);

	syntax->base_class->name = filename.basename().replace(".kaba", "");

	auto parser = new Parser(syntax);
	syntax->parser = parser;

	try {
		syntax->default_import();

	// read file
		string buffer = FileReadText(filename);
		parser->parse_buffer(buffer, just_analyse);


		if (!just_analyse)
			compile();

	} catch (FileError &e) {
		loading_module_stack.pop();
		do_error("module file not loadable: " + filename.str());
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
	syntax->do_error(str, override_token);
}

void Module::do_error_internal(const string &str) {
	do_error("internal compiler error: " + str, 0);
}

void Module::do_error_link(const string &str) {
	do_error(str, 0);
}

void Module::set_variable(const string &name, void *data) {
	//msg_write(name);
	for (auto *v: weak(syntax->base_class->static_variables))
		if (v->name == name) {
			memcpy(v->memory, data, v->type->size);
			return;
		}
	msg_error("Module.set_variable: variable " + name + " not found");
}

Module::Module() {
	filename = "-empty module-";
	used_by_default = false;

	show_compiler_stats = !config.compile_silently;

	just_analyse = false;

	opcode = nullptr;
	opcode_size = 0;

	memory = nullptr;
	memory_size = 0;

	syntax = new SyntaxTree(this);
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
	delete syntax;
}


// bad:  should clean up in case of errors!
void execute_single_command(const string &cmd) {
	if (cmd.num < 1)
		return;
	//msg_write("command: " + cmd);

	// empty module
	shared<Module> s = new Module();
	s->filename = "-command line-";
	auto tree = s->syntax;
	tree->default_import();
	auto parser = new Parser(tree);
	tree->parser = parser;

	try {

// find expressions
	parser->Exp.analyse(tree, cmd);
	if (parser->Exp.empty()) {
		//clear_exp_buffer(&ps->Exp);
		return;
	}
	
	for (auto p: packages)
		if (!p->used_by_default)
			tree->import_data(p, true, "");

// analyse syntax

	// create a main() function
	Function *func = tree->add_function("--command-func--", TypeVoid, tree->base_class, Flags::STATIC);
	func->_var_size = 0; // set to -1...

	parser->Exp.reset_walker();

	// parse
	func->block->type = TypeUnknown;
	parser->parse_abstract_complete_command(func->block.get());
	if (config.verbose) {
		msg_write("ABSTRACT SINGLE:");
		func->block->show();
	}
	parser->concretify_node(func->block.get(), func->block.get(), func->name_space);
	
	// implicit print(...)?
	if (func->block->params.num > 0 and func->block->params[0]->type != TypeVoid) {
		auto n = parser->add_converter_str(func->block->params[0], true);
		
		auto f = tree->required_func_global("print");

		auto cmd = tree->add_node_call(f);
		cmd->set_param(0, n);
		func->block->params[0] = cmd;
	}
	for (auto *c: tree->owned_classes)
		parser->auto_implement_functions(c);
	//ps->show("aaaa");


	if (config.verbose)
		tree->show("parse:a");

// compile
	s->compile();


	if (config.interpreted) {
		s->interpreter->run("--command-func--");
		return;
	}

// execute
	if (config.abi == config.native_abi) {
		typedef void void_func();
		void_func *f = (void_func*)func->address;
		if (f)
			f();
	}

	} catch(const Exception &e) {
		e.print();
	}
}

void *Module::match_function(const string &name, const string &return_type, const Array<string> &param_types) {
	auto ns = base_class();
	// match
	for (Function *f: syntax->functions)
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

// DEPRECATED?
void *Module::match_class_function(const string &_class, bool allow_derived, const string &name, const string &return_type, const Array<string> &param_types)
{
	const Class *root_type = syntax->find_root_type_by_name(_class, syntax->base_class, false);
	if (!root_type)
		return nullptr;

	// match
	for (auto *f: syntax->functions){
		if (!f->name_space)
			continue;
		if (!f->name_space->is_derived_from(root_type))
			continue;
		if ((f->name == name) and (f->literal_return_type->name == return_type) and (param_types.num == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<param_types.num;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_types[j])
					params_ok = false;
			if (params_ok){
				if (just_analyse)
					return (void*)(int_p)0xdeadbeaf;
				else
					return (void*)(int_p)f->address;
			}
		}
	}

	return nullptr;
}

void print_var(void *p, const string &name, const Class *t) {
	msg_write(t->name + " " + name + " = " + var2str(p, t));
}

void Module::show_vars(bool include_consts) {
	for (auto *v: weak(syntax->base_class->static_variables))
		print_var(v->memory, v->name, v->type);
	/*if (include_consts)
		foreachi(LocalVariable &c, pre_script->Constant, i)
			print_var((void*)g_var[i], c.name, c.type);*/
}

Array<const Class*> Module::classes() {
	return weak(syntax->base_class->classes);
}

Array<Function*> Module::functions() {
	return syntax->functions;
}

Array<Variable*> Module::variables() {
	return weak(syntax->base_class->static_variables);
}

Array<Constant*> Module::constants() {
	return weak(syntax->base_class->constants);
}

const Class *Module::base_class() {
	return syntax->base_class;
}

};
