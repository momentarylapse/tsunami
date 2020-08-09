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
#include "lib/common.h"
#include <cassert>

#include "../config.h"
#ifdef _X_ALLOW_X_
	#include "../../meta.h"
#endif

#ifdef OS_LINUX
	#include <sys/mman.h>
#endif
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	#include <windows.h>
#endif

namespace Kaba {

string Version = "0.19.-1.0";

//#define ScriptDebug


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Script *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	text +=  ", " + s->filename.str();
}

Exception::Exception(const Asm::Exception &e, Script *s, Function *f) :
	Asm::Exception(e)
{
	text = format("assembler: %s, %s: %s", message(), f->long_name(), s->filename);
}


Array<Script*> _public_scripts_;
Array<Script*> _dead_scripts_;





Script *Load(const Path &filename, bool just_analyse) {
	//msg_write(string("Lade ",filename));
	Script *s = nullptr;

	// already loaded?
	for (Script *ps: _public_scripts_)
		if (ps->filename == filename)
			return ps;
	
	// load
	s = new Script();
	s->syntax->base_class->name = filename.basename().replace(".kaba", "");
	try {
		s->load(filename, just_analyse);
	} catch(const Exception &e) {
		delete(s);
		throw e;
	}

	// store script in database
	_public_scripts_.add(s);
	return s;
}

Script *CreateForSource(const string &buffer, bool just_analyse) {
	Script *s = new Script;
	s->just_analyse = just_analyse;
	auto parser = s->syntax->parser = new Parser(s->syntax);
	try {
		parser->parse_buffer(buffer, just_analyse);

		if (!just_analyse)
			s->compile();
	} catch(const Exception &e) {
		delete(s);
		throw e;
	}
	return s;
}

void Remove(Script *s)
{
	// remove references
	for (Script *i: s->syntax->includes)
		i->reference_counter --;

	// put on to-delete-list
	_dead_scripts_.add(s);

	// remove from normal list
	for (int i=0;i<_public_scripts_.num;i++)
		if (_public_scripts_[i] == s)
			_public_scripts_.erase(i);

	// delete all deletables
	for (int i=_dead_scripts_.num-1;i>=0;i--)
		if (_dead_scripts_[i]->reference_counter <= 0){
			delete(_dead_scripts_[i]);
			_dead_scripts_.erase(i);
		}
}

void DeleteAllScripts(bool even_immortal, bool force)
{
	// try to erase them...
	auto to_del = _public_scripts_;
	foreachb(Script *s, to_del)
		if ((!s->syntax->flag_immortal) or even_immortal)
			Remove(s);

	// undead... really KILL!
	if (force){
		foreachb(Script *s, _dead_scripts_)
			delete(s);
		_dead_scripts_.clear();
	}

	//ScriptResetSemiExternalData();

	
	/*msg_write("------------------------------------------------------------------");
	msg_write(mem_used);
	for (int i=0;i<num_ps;i++)
		msg_write(string2("  fehlt:   %s  %p  (%d)",ppn[i],ppp[i],pps[i]));
	*/
}

VirtualTable* get_vtable(const VirtualBase *p) {
	return *(VirtualTable**)p;
}

const Class *_dyn_type_in_namespace(const VirtualTable *p, const Class *ns) {
	for (auto *c: ns->classes) {
		if (c->_vtable_location_target_ == p)
			return c;
		auto t = _dyn_type_in_namespace(p, c);
		if (t)
			return t;
	}
	return nullptr;
}

// TODO...namespace
const Class *GetDynamicType(const VirtualBase *p) {
	auto *pp = get_vtable(p);
	for (Script *s: _public_scripts_) {
		auto t = _dyn_type_in_namespace(pp, s->syntax->base_class);
		if (t)
			return t;
	}
	return nullptr;
}

// wrapper preventing compiler complaints for Array<Script*>.pop()
struct LoadingScript {
	Script *s = nullptr;
	LoadingScript() {}
	LoadingScript(Script *_s) { s = _s; }
};
Array<LoadingScript> loading_script_stack;


void Script::load(const Path &_filename, bool _just_analyse) {
	loading_script_stack.add(this);
	just_analyse = _just_analyse;


	if (_filename.is_relative())
		filename = (config.directory << _filename).absolute().canonical();
	else
		filename = _filename.absolute().canonical();
	auto parser = syntax->parser = new Parser(syntax);

	try {

	// read file
		string buffer = FileReadText(filename);
		parser->parse_buffer(buffer, just_analyse);


		if (!just_analyse)
			compile();
		/*if (pre_script->FlagShow)
			pre_script->Show();*/
		if ((!just_analyse) and (config.verbose)){
			msg_write(format("Opcode: %d bytes", opcode_size));
			if (config.allow_output_stage("dasm"))
				msg_write(Asm::disassemble(opcode, opcode_size));
		}

	} catch(FileError &e) {
		loading_script_stack.pop();
		do_error("script file not loadable: " + filename.str());
	} catch(Exception &e) {
		loading_script_stack.pop();
		throw e;
	}
	loading_script_stack.pop();
}

void Script::do_error(const string &str, int override_line)
{
#ifdef CPU_ARM
	msg_error(str);
#endif
	syntax->do_error(str, 0, override_line);
}

void Script::do_error_internal(const string &str) {
	do_error("internal compiler error: " + str, 0);
}

void Script::do_error_link(const string &str) {
	do_error(str, 0);
}

void Script::set_variable(const string &name, void *data) {
	//msg_write(name);
	for (auto *v: syntax->base_class->static_variables)
		if (v->name == name) {
			memcpy(v->memory, data, v->type->size);
			return;
		}
	msg_error("Script.set_variable: variable " + name + " not found");
}

Script::Script() {
	filename = "-empty script-";
	used_by_default = false;

	reference_counter = 0;

	show_compiler_stats = !config.compile_silently;

	just_analyse = false;

	opcode = nullptr;
	opcode_size = 0;

	memory = nullptr;
	memory_size = 0;

	syntax = new SyntaxTree(this);
}

Script::~Script()
{
	int r = 0;
	if (opcode){
		#if defined(OS_WINDOWS) || defined(OS_MINGW)
			VirtualFree(opcode, 0, MEM_RELEASE);
		#else
			r = munmap(opcode, MAX_OPCODE);
		#endif
	}
	if (r != 0)
		msg_error("munmap...op");
	if (memory and memory_size > 0){
		#if defined(OS_WINDOWS) || defined(OS_MINGW)
			VirtualFree(memory, 0, MEM_RELEASE);
		#else
			r = munmap(memory, memory_size);
		#endif
	}
	if (r != 0)
		msg_error("munmap...mem");
	//msg_write(string2("-----------            Memory:         %p",Memory));
	delete(syntax);
}


// bad:  should clean up in case of errors!
void ExecuteSingleScriptCommand(const string &cmd)
{
	if (cmd.num < 1)
		return;
	//msg_write("script command: " + cmd);

	// empty script
	Script *s = new Script();
	s->filename = "-command line-";
	SyntaxTree *ps = s->syntax;
	auto parser = ps->parser = new Parser(ps);

	try {

// find expressions
	parser->Exp.analyse(ps, cmd);
	if (parser->Exp.line[0].exp.num < 1){
		//clear_exp_buffer(&ps->Exp);
		delete(s);
		return;
	}
	
	for (auto *p: packages)
		if (!p->used_by_default and (p->filename != "x"))
			ps->add_include_data(p, true);

// analyse syntax

	// create a main() function
	Function *func = ps->add_function("--command-func--", TypeVoid, ps->base_class, Flags::STATIC);
	func->_var_size = 0; // set to -1...

	parser->Exp.reset_parser();

	// parse
	parser->parse_complete_command(func->block);
	
	// implicit print(...)?
	if (func->block->params.num > 0 and func->block->params[0]->type != TypeVoid) {
		auto *n = parser->add_converter_str(func->block->params[0], true);
		
		auto links = ps->get_existence("print", nullptr, nullptr, false);
		Function *f = links[0]->as_func();

		Node *cmd = ps->add_node_call(f);
		cmd->set_param(0, n);
		func->block->params[0] = cmd;
	}
	for (auto *c: ps->owned_classes)
		parser->auto_implement_functions(c);
	//ps->show("aaaa");

// compile
	s->compile();

// execute
	typedef void void_func();
	void_func *f = (void_func*)func->address;
	if (f)
		f();

	}catch(const Exception &e){
		e.print();
	}

	delete(s);
}

void *Script::match_function(const string &name, const string &return_type, const Array<string> &param_types) {
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
					return f->address;
			}
		}

	return nullptr;
}

// DEPRECATED?
void *Script::match_class_function(const string &_class, bool allow_derived, const string &name, const string &return_type, const Array<string> &param_types)
{
	const Class *root_type = syntax->find_root_type_by_name(_class, syntax->base_class, false);
	if (!root_type)
		return nullptr;

	// match
	for (Function *f: syntax->functions){
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
					return f->address;
			}
		}
	}

	return nullptr;
}

void print_var(void *p, const string &name, const Class *t) {
	msg_write(t->name + " " + name + " = " + var2str(p, t));
}

void Script::show_vars(bool include_consts) {
	for (auto *v: syntax->base_class->static_variables)
		print_var(v->memory, v->name, v->type);
	/*if (include_consts)
		foreachi(LocalVariable &c, pre_script->Constant, i)
			print_var((void*)g_var[i], c.name, c.type);*/
}

Array<const Class*> Script::classes() {
	return syntax->base_class->classes;
}

Array<Function*> Script::functions() {
	return syntax->functions;
}

Array<Variable*> Script::variables() {
	return syntax->base_class->static_variables;
}

Array<Constant*> Script::constants() {
	return syntax->base_class->constants;
}

const Class *Script::base_class() {
	return syntax->base_class;
}

};
