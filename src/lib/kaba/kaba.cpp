/*----------------------------------------------------------------------------*\
| Kaba                                                                         |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../file/file.h"
#include <stdarg.h>
#include "kaba.h"

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

namespace Kaba{

string Version = "0.17.-1.6";

//#define ScriptDebug

int GlobalWaitingMode;
float GlobalTimeToWait;


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Script *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	text +=  ", " + s->filename;
}

Exception::Exception(const Asm::Exception &e, Script *s) :
	Asm::Exception(e)
{
	text = "assembler: " + message() + ", " + s->filename;
}


Array<Script*> _public_scripts_;
Array<Script*> _dead_scripts_;





Script *Load(const string &filename, bool just_analyse)
{
	//msg_write(string("Lade ",filename));
	Script *s = nullptr;

	// already loaded?
	for (Script *ps: _public_scripts_)
		if (ps->filename == filename.sys_filename())
			return ps;
	
	// load
	s = new Script();
	try{
		s->Load(filename, just_analyse);
	}catch(const Exception &e){
		delete(s);
		throw e;
	}

	// store script in database
	_public_scripts_.add(s);
	return s;
}

Script *CreateForSource(const string &buffer, bool just_analyse)
{
	Script *s = new Script;
	s->just_analyse = just_analyse;
	try{
		s->syntax->parse_buffer(buffer, just_analyse);

		if (!just_analyse)
			s->compile();
	}catch(const Exception &e){
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


const Class *GetDynamicType(const void *p)
{
	VirtualTable *pp = *(VirtualTable**)p;
	for (Script *s: _public_scripts_){
		for (auto *t: s->syntax->classes){
			if (t->vtable.data == pp)
				return t;
		}
	}
	return nullptr;
}

Array<Script*> loading_script_stack;

void Script::Load(const string &_filename, bool _just_analyse)
{
	loading_script_stack.add(this);
	just_analyse = _just_analyse;
	filename = _filename.sys_filename();

	try{

	// read file
		string buffer = FileReadText(config.directory + filename);
		syntax->parse_buffer(buffer, just_analyse);


		if (!just_analyse)
			compile();
		/*if (pre_script->FlagShow)
			pre_script->Show();*/
		if ((!just_analyse) and (config.verbose)){
			msg_write(format("Opcode: %d bytes", opcode_size));
			if (config.allow_output_stage("dasm"))
				msg_write(Asm::Disassemble(opcode, opcode_size));
		}

	}catch(FileError &e){
		loading_script_stack.pop();
		do_error("script file not loadable: " + filename);
	}catch(Exception &e){
		loading_script_stack.pop();
		throw e;
	}
	loading_script_stack.pop();
}

void Script::do_error(const string &str, int override_line)
{
	syntax->do_error(str, 0, override_line);
}

void Script::do_error_internal(const string &str)
{
	do_error("internal compiler error: " + str, 0);
}

void Script::do_error_link(const string &str)
{
	do_error(str, 0);
}

void Script::SetVariable(const string &name, void *data)
{
	//msg_write(name);
	for (auto *v: syntax->root_of_all_evil.var)
		if (v->name == name){
			memcpy(v->memory, data, v->type->size);
			return;
		}
	msg_error("CScript.SetVariable: variable " + name + " not found");
}

Script::Script()
{
	filename = "-empty script-";

	reference_counter = 0;

	cur_func = nullptr;
	show_compiler_stats = !config.compile_silently;

	just_analyse = false;

	opcode = nullptr;
	opcode_size = 0;

	syntax = new SyntaxTree(this);
}

Script::~Script()
{
	if (opcode){
		#if defined(OS_WINDOWS) || defined(OS_MINGW)
			VirtualFree(opcode, 0, MEM_RELEASE);
		#else
			int r = munmap(opcode, MAX_OPCODE);
		#endif
	}
	//msg_write(string2("-----------            Memory:         %p",Memory));
	delete(syntax);
}


// bad:  should clean up in case of errors!
void ExecuteSingleScriptCommand(const string &cmd)
{
	if (cmd.num < 1)
		return;
	msg_write("script command: " + cmd);

	// empty script
	Script *s = new Script();
	SyntaxTree *ps = s->syntax;

	try{

// find expressions
	ps->Exp.Analyse(ps, cmd + string("\0", 1));
	if (ps->Exp.line[0].exp.num < 1){
		//clear_exp_buffer(&ps->Exp);
		delete(s);
		return;
	}

// analyse syntax

	// create a main() function
	Function *func = ps->add_function("--command-func--", TypeVoid);
	func->_var_size = 0; // set to -1...

	// parse
	ps->Exp.reset_parser();
	ps->parse_complete_command(func->block);
	//pre_script->GetCompleteCommand((pre_script->Exp->ExpNr,0,0,&func);

	ps->ConvertCallByReference();

// compile
	s->compile();

	/*if (true){
		printf("%s\n\n", Opcode2Asm(s->ThreadOpcode,s->ThreadOpcodeSize));
		printf("%s\n\n", Opcode2Asm(s->Opcode,s->OpcodeSize));
		//msg_write(Opcode2Asm(Opcode,OpcodeSize));
	}*/
// execute
	typedef void void_func();
	void_func *f = (void_func*)s->MatchFunction("--command-func--", "void", 0);
	if (f)
		f();

	}catch(const Exception &e){
		e.print();
	}

	delete(s);
}

void *Script::MatchFunction(const string &name, const string &return_type, int num_params, ...)
{
	// process argument list
	va_list marker;
	va_start(marker, num_params);
	Array<string> param_type;
	for (int p=0;p<num_params;p++)
		param_type.add(string(va_arg(marker, char*)));
	va_end(marker);

	// match
	for (Function *f: syntax->functions)
		if (f->long_name.match(name) and (f->literal_return_type->name == return_type) and (num_params == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_type[j])
					params_ok = false;
			if (params_ok){
				if (just_analyse)
					return (void*)0xdeadbeaf;
				else
					return f->address;
			}
		}

	return nullptr;
}

void *Script::MatchClassFunction(const string &_class, bool allow_derived, const string &name, const string &return_type, int num_params, ...)
{
	// process argument list
	va_list marker;
	va_start(marker, num_params);
	Array<string> param_type;
	for (int p=0;p<num_params;p++)
		param_type.add(string(va_arg(marker, char*)));
	va_end(marker);

	const Class *root_type = syntax->find_type_by_name(_class);
	if (!root_type)
		return nullptr;

	// match
	for (Function *f: syntax->functions){
		if (!f->_class)
			continue;
		if (!f->_class->is_derived_from(root_type))
			continue;
		if ((f->name == name) and (f->literal_return_type->name == return_type) and (num_params == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_type[j])
					params_ok = false;
			if (params_ok){
				if (just_analyse)
					return (void*)0xdeadbeaf;
				else
					return f->address;
			}
		}
	}

	return nullptr;
}

void print_var(void *p, const string &name, const Class *t)
{
	msg_write(t->name + " " + name + " = " + t->var2str(p));
}

void Script::ShowVars(bool include_consts)
{
	for (auto *v: syntax->root_of_all_evil.var)
		print_var(v->memory, v->name, v->type);
	/*if (include_consts)
		foreachi(LocalVariable &c, pre_script->Constant, i)
			print_var((void*)g_var[i], c.name, c.type);*/
}

};
