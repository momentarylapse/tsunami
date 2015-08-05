/*----------------------------------------------------------------------------*\
| CScript                                                                      |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../file/file.h"
#include <stdarg.h>
#include "script.h"

#include "../config.h"
#ifdef _X_ALLOW_X_
	#include "../../meta.h"
#endif

#ifdef OS_LINUX
	#include <sys/mman.h>
#endif
#ifdef OS_WINDOWS
	#include "windows.h"
#endif

namespace Script{

string Version = "0.14.3.-1";

//#define ScriptDebug

int GlobalWaitingMode;
float GlobalTimeToWait;


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Script *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	message +=  ", " + s->filename;
}

Exception::Exception(const Asm::Exception &e, Script *s) :
	Asm::Exception(e)
{
	message = "assembler: " + message + ", " + s->filename;
}


static int shift_right=0;

Array<Script*> PublicScript;
Array<Script*> DeadScript;





Script *Load(const string &filename, bool just_analyse)
{
	//msg_write(string("Lade ",filename));
	Script *s = NULL;

	// already loaded?
	for (int i=0;i<PublicScript.num;i++)
		if (PublicScript[i]->filename == filename.sys_filename())
			return PublicScript[i];
	
	// load
	s = new Script();
	try{
		s->Load(filename, just_analyse);
	}catch(const Exception &e){
		delete(s);
		throw e;
	}

	// store script in database
	PublicScript.add(s);
	return s;
}

Script *CreateForSource(const string &buffer, bool just_analyse)
{
	Script *s = new Script;
	s->just_analyse = just_analyse;
	try{
		s->syntax->ParseBuffer(buffer, just_analyse);

		if (!just_analyse)
			s->Compiler();
	}catch(const Exception &e){
		delete(s);
		throw e;
	}
	return s;
}

void Remove(Script *s)
{
	msg_db_f("RemoveScript", 1);
	// remove references
	for (int i=0;i<s->syntax->includes.num;i++)
		s->syntax->includes[i]->reference_counter --;

	// put on to-delete-list
	DeadScript.add(s);

	// remove from normal list
	for (int i=0;i<PublicScript.num;i++)
		if (PublicScript[i] == s)
			PublicScript.erase(i);

	// delete all deletables
	for (int i=DeadScript.num-1;i>=0;i--)
		if (DeadScript[i]->reference_counter <= 0){
			delete(DeadScript[i]);
			DeadScript.erase(i);
		}
}

void DeleteAllScripts(bool even_immortal, bool force)
{
	msg_db_f("DeleteAllScripts", 1);

	// try to erase them...
	foreachb(Script *s, PublicScript)
		if ((!s->syntax->flag_immortal) || (even_immortal))
			Remove(s);

	// undead... really KILL!
	if (force){
		foreachb(Script *s, DeadScript)
			delete(s);
		DeadScript.clear();
	}

	//ScriptResetSemiExternalData();

	
	/*msg_write("------------------------------------------------------------------");
	msg_write(mem_used);
	for (int i=0;i<num_ps;i++)
		msg_write(string2("  fehlt:   %s  %p  (%d)",ppn[i],ppp[i],pps[i]));
	*/
}


extern Array<Script*> PublicScript;

Type *GetDynamicType(void *p)
{
	VirtualTable *pp = *(VirtualTable**)p;
	foreach(Script *s, PublicScript){
		foreach(Type *t, s->syntax->types){
			if (t->vtable.data == pp)
				return t;
		}
	}
	return NULL;
}

Array<Script*> loading_script_stack;

void Script::Load(const string &_filename, bool _just_analyse)
{
	msg_db_f("loading script", 1);
	loading_script_stack.add(this);
	just_analyse = _just_analyse;
	filename = _filename.sys_filename();

	try{

	// read file
	File *f = FileOpen(config.directory + filename);
	if (!f)
		DoError("script file not loadable");
	string buffer = f->ReadComplete();
	delete(f);
	syntax->ParseBuffer(buffer, just_analyse);


	if (!just_analyse)
		Compiler();
	/*if (pre_script->FlagShow)
		pre_script->Show();*/
	if ((!just_analyse) and (config.verbose)){
		msg_write(format("Opcode: %d bytes", opcode_size));
		msg_write(Asm::Disassemble(opcode, opcode_size));
	}

	}catch(Exception &e){
		loading_script_stack.pop();
		throw e;
	}
	loading_script_stack.pop();
}

void Script::DoError(const string &str, int overwrite_line)
{
	syntax->DoError(str, overwrite_line);
}

void Script::DoErrorInternal(const string &str)
{
	DoError("internal compiler error (Call Michi!): " + str, 0);
}

void Script::DoErrorLink(const string &str)
{
	DoError(str);
}

void Script::SetVariable(const string &name, void *data)
{
	msg_db_f("SetVariable", 4);
	//msg_write(name);
	for (int i=0;i<syntax->root_of_all_evil.var.num;i++)
		if (syntax->root_of_all_evil.var[i].name == name){
			/*msg_write("var");
			msg_write(pre_script->RootOfAllEvil.Var[i].Type->Size);
			msg_write((int)g_var[i]);*/
			memcpy(g_var[i], data, syntax->root_of_all_evil.var[i].type->size);
			return;
		}
	msg_error("CScript.SetVariable: variable " + name + " not found");
}

Script::Script()
{
	filename = "-empty script-";

	reference_counter = 0;

	cur_func = NULL;
	__first_execution = NULL;
	__waiting_mode = WAITING_MODE_FIRST;
	__time_to_wait = 0;
	show_compiler_stats = !config.compile_silently;

	opcode = NULL;
	opcode_size = 0;
	memory = NULL;
	memory_size = 0;
	memory_used = 0;
	__stack = NULL;

	syntax = new SyntaxTree(this);
}

Script::~Script()
{
	msg_db_f("~CScript", 4);
	if ((memory) && (!just_analyse)){
		//delete[](Memory);
		#ifdef OS_WINDOWS
			VirtualFree(memory, 0, memory_size);
		#else
			int r = munmap(memory, memory_size);
		#endif
	}
	if (opcode){
		#ifdef OS_WINDOWS
			VirtualFree(opcode, 0, MEM_RELEASE);
		#else
			int r = munmap(opcode, SCRIPT_MAX_OPCODE);
		#endif
	}
	if (__stack)
		delete[](__stack);
	//msg_write(string2("-----------            Memory:         %p",Memory));
	delete(syntax);
}


// bad:  should clean up in case of errors!
void ExecuteSingleScriptCommand(const string &cmd)
{
	if (cmd.num < 1)
		return;
	msg_db_f("ExecuteSingleScriptCmd", 2);
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
	Function *func = ps->AddFunction("--command-func--", TypeVoid);
	func->_var_size = 0; // set to -1...

	// parse
	ps->Exp.reset_parser();
	ps->ParseCompleteCommand(func->block);
	//pre_script->GetCompleteCommand((pre_script->Exp->ExpNr,0,0,&func);

	ps->ConvertCallByReference();

// compile
	s->Compiler();

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
	msg_db_f("MatchFunction", 2);
	
	// process argument list
	va_list marker;
	va_start(marker, num_params);
	Array<string> param_type;
	for (int p=0;p<num_params;p++)
		param_type.add(string(va_arg(marker, char*)));
	va_end(marker);

	// match
	foreachi(Function *f, syntax->functions, i)
		if (((f->name == name) or (name == "*")) and (f->literal_return_type->name == return_type) and (num_params == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_type[j])
					params_ok = false;
			if (params_ok){
				if (just_analyse)
					return (void*)0xdeadbeaf;
				else
					return (void*)func[i];
			}
		}

	return NULL;
}

void *Script::MatchClassFunction(const string &_class, bool allow_derived, const string &name, const string &return_type, int num_params, ...)
{
	msg_db_f("MatchClassFunction", 2);

	// process argument list
	va_list marker;
	va_start(marker, num_params);
	Array<string> param_type;
	for (int p=0;p<num_params;p++)
		param_type.add(string(va_arg(marker, char*)));
	va_end(marker);

	Type *root_type = syntax->FindType(_class);
	if (!root_type)
		return NULL;

	// match
	foreachi(Function *f, syntax->functions, i){
		if (!f->_class)
			continue;
		if (!f->_class->IsDerivedFrom(root_type))
			continue;
		if ((f->name.match("*." + name)) && (f->literal_return_type->name == return_type) && (num_params == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_type[j])
					params_ok = false;
			if (params_ok){
				if (just_analyse)
					return (void*)0xdeadbeaf;
				else
					return (void*)func[i];
			}
		}
	}

	return NULL;
}

void print_var(void *p, const string &name, Type *t)
{
	msg_write(t->name + " " + name + " = " + t->var2str(p));
}

void Script::ShowVars(bool include_consts)
{
	foreachi(Variable &v, syntax->root_of_all_evil.var, i)
		print_var((void*)g_var[i], v.name, v.type);
	/*if (include_consts)
		foreachi(LocalVariable &c, pre_script->Constant, i)
			print_var((void*)g_var[i], c.name, c.type);*/
}

// REALLY DEPRECATED!
void Script::__Execute()
{
	return;
	if (__waiting_mode == WAITING_MODE_NONE)
		return;
	shift_right=0;
	//msg_db_f(string("Execute ",pre_script->Filename),1);
	msg_db_f("Execute", 1);{
	msg_db_f(filename.c_str(),1);

	// handle wait-commands
	if (__waiting_mode == WAITING_MODE_FIRST){
		GlobalWaitingMode = WAITING_MODE_NONE;
		msg_db_f("->First",1);
		//msg_right();
		__first_execution();
		//msg_left();
	}else{
#ifdef _X_ALLOW_X_
		if (__waiting_mode==WaitingModeRT)
			__time_to_wait -= Engine.ElapsedRT;
		else
			__time_to_wait -= Engine.Elapsed;
		if (__time_to_wait>0){
			return;
		}
#endif
		GlobalWaitingMode=WAITING_MODE_NONE;
		//msg_write(ThisObject);
		msg_db_f("->Continue",1);
		//msg_write(">---");
		//msg_right();
		__continue_execution();
		//msg_write("---<");
		//msg_write("ok");
		//msg_left();
	}
	__waiting_mode=GlobalWaitingMode;
	__time_to_wait=GlobalTimeToWait;
	}
}

};
