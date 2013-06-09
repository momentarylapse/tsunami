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

string Version = "0.11.9.1";

//#define ScriptDebug

int GlobalWaitingMode;
float GlobalTimeToWait;


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Script *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	message +=  ", " + s->Filename;
}

Exception::Exception(const Asm::Exception &e, Script *s) :
	Asm::Exception(e)
{
	message = "assembler: " + message + ", " + s->Filename;
}


static int shift_right=0;

void script_db_out(const string &str)
{
#ifdef ScriptDebug
	/*if (str.num > 256)
		((char*)str)[256]=0;*/
	msg_write(str);
#endif
}

void script_db_out(int i)
{
#ifdef ScriptDebug
	msg_write(i);
#endif
}

void script_db_right()
{
#ifdef ScriptDebug
	msg_right();
	shift_right+=2;
#endif
}

void script_db_left()
{
#ifdef ScriptDebug
	msg_left();
	shift_right-=2;
#endif
}

#define so		script_db_out
#define right	script_db_right
#define left	script_db_left



Array<Script*> PublicScript;
Array<Script*> DeadScript;





Script *Load(const string &filename, bool just_analyse)
{
	//msg_write(string("Lade ",filename));
	Script *s = NULL;

	// already loaded?
	for (int i=0;i<PublicScript.num;i++)
		if (PublicScript[i]->Filename == filename.sys_filename())
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

void Remove(Script *s)
{
	msg_db_f("RemoveScript", 1);
	// remove references
	for (int i=0;i<s->syntax->Includes.num;i++)
		s->syntax->Includes[i]->ReferenceCounter --;

	// put on to-delete-list
	DeadScript.add(s);

	// remove from normal list
	for (int i=0;i<PublicScript.num;i++)
		if (PublicScript[i] == s)
			PublicScript.erase(i);

	// delete all deletables
	for (int i=DeadScript.num-1;i>=0;i--)
		if (DeadScript[i]->ReferenceCounter <= 0){
			delete(DeadScript[i]);
			DeadScript.erase(i);
		}
}

void DeleteAllScripts(bool even_immortal, bool force)
{
	msg_db_f("DeleteAllScripts", 1);

	// try to erase them...
	foreachb(Script *s, PublicScript)
		if ((!s->syntax->FlagImmortal) || (even_immortal))
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

void Script::Load(const string &filename, bool just_analyse)
{
	msg_db_f("loading script", 1);
	JustAnalyse = just_analyse;
	Filename = filename.sys_filename();
	syntax->LoadAndParseFile(filename, just_analyse);


	if (!JustAnalyse)
		Compiler();
	/*if (pre_script->FlagShow)
		pre_script->Show();*/
	if ((!JustAnalyse) && (syntax->FlagDisassemble)){
		if (ThreadOpcodeSize > 0){
			msg_write(format("ThreadOpcode: %d bytes", ThreadOpcodeSize));
			msg_write(Asm::Disassemble(ThreadOpcode, ThreadOpcodeSize));
			msg_write("\n\n");
		}
		msg_write(format("Opcode: %d bytes", OpcodeSize));
		msg_write(Asm::Disassemble(Opcode, OpcodeSize));
	}
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
	for (int i=0;i<syntax->RootOfAllEvil.var.num;i++)
		if (syntax->RootOfAllEvil.var[i].name == name){
			/*msg_write("var");
			msg_write(pre_script->RootOfAllEvil.Var[i].Type->Size);
			msg_write((int)g_var[i]);*/
			memcpy(g_var[i], data, syntax->RootOfAllEvil.var[i].type->size);
			return;
		}
	msg_error("CScript.SetVariable: variable " + name + " not found");
}

Script::Script()
{
	Filename = "-empty script-";

	ReferenceCounter = 0;

	cur_func = NULL;
	WaitingMode = WaitingModeFirst;
	TimeToWait = 0;
	ShowCompilerStats = !config.CompileSilently;

	Opcode = NULL;
	OpcodeSize = 0;
	ThreadOpcode = NULL;
	ThreadOpcodeSize = 0;
	Memory = NULL;
	MemorySize = 0;
	MemoryUsed = 0;
	Stack = NULL;

	syntax = new SyntaxTree(this);
}

Script::~Script()
{
	msg_db_f("~CScript", 4);
	if ((Memory) && (!JustAnalyse)){
		//delete[](Memory);
		int r=munmap(Memory,MemorySize);
	}
	if (Opcode){
		#ifdef OS_WINDOWS
			VirtualFree(Opcode,0,MEM_RELEASE);
		#else
			int r=munmap(Opcode,SCRIPT_MAX_OPCODE);
		#endif
	}
	if (ThreadOpcode){
		#ifdef OS_WINDOWS
			VirtualFree(ThreadOpcode,0,MEM_RELEASE);
		#else
			int r=munmap(ThreadOpcode,SCRIPT_MAX_THREAD_OPCODE);
		#endif
	}
	if (Stack)
		delete[](Stack);
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
	ps->ParseCompleteCommand(func->block, func);
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
	string param_type[SCRIPT_MAX_PARAMS];
	for (int p=0;p<num_params;p++)
		param_type[p] = string(va_arg(marker, char*));
	va_end(marker);

	// match
	foreachi(Function *f, syntax->Functions, i)
		if ((f->name == name) && (f->literal_return_type->name == return_type) && (num_params == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_type[j])
					params_ok = false;
			if (params_ok){
				if (JustAnalyse)
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
	string param_type[SCRIPT_MAX_PARAMS];
	for (int p=0;p<num_params;p++)
		param_type[p] = string(va_arg(marker, char*));
	va_end(marker);

	Type *root_type = syntax->FindType(_class);
	if (!root_type)
		return NULL;

	// match
	foreachi(Function *f, syntax->Functions, i){
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
				if (JustAnalyse)
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
	foreachi(Variable &v, syntax->RootOfAllEvil.var, i)
		print_var((void*)g_var[i], v.name, v.type);
	/*if (include_consts)
		foreachi(LocalVariable &c, pre_script->Constant, i)
			print_var((void*)g_var[i], c.name, c.type);*/
}

void Script::Execute()
{
	if (WaitingMode==WaitingModeNone)	return;
	#ifdef ScriptDebug
		//so("\n\n\n################### fuehre aus ######################\n\n\n");
	#endif
	shift_right=0;
	//msg_db_f(string("Execute ",pre_script->Filename),1);
	msg_db_f("Execute", 1);{
	msg_db_f(Filename.c_str(),1);

	// handle wait-commands
	if (WaitingMode==WaitingModeFirst){
		GlobalWaitingMode=WaitingModeNone;
		msg_db_f("->First",1);
		//msg_right();
		first_execution();
		//msg_left();
	}else{
#ifdef _X_ALLOW_X_
		if (WaitingMode==WaitingModeRT)
			TimeToWait -= Engine.ElapsedRT;
		else
			TimeToWait -= Engine.Elapsed;
		if (TimeToWait>0){
			return;
		}
#endif
		GlobalWaitingMode=WaitingModeNone;
		//msg_write(ThisObject);
		msg_db_f("->Continue",1);
		//msg_write(">---");
		//msg_right();
		continue_execution();
		//msg_write("---<");
		//msg_write("ok");
		//msg_left();
	}
	WaitingMode=GlobalWaitingMode;
	TimeToWait=GlobalTimeToWait;
	}
}

};
