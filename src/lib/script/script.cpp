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
#ifdef _X_ALLOW_META_
	#include "../x/x.h"
#endif

#ifdef OS_LINUX
	#include <sys/mman.h>
#endif
#ifdef OS_WINDOWS
	#include "windows.h"
#endif

namespace Script{

string Version = "0.10.11.0";

//#define ScriptDebug

int GlobalWaitingMode;
float GlobalTimeToWait;

bool CompileSilently = false;
bool ShowCompilerStats = true;


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Script *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	message +=  ", " + s->pre_script->Filename;
}

Exception::Exception(const Asm::Exception &e, Script *s) :
	Asm::Exception(e)
{
	message = "assembler: " + message + ", " + s->pre_script->Filename;
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
Array<Script*> PrivateScript;
Array<Script*> DeadScript;


string Directory = "";




Script *Load(const string &filename, bool is_public, bool just_analyse)
{
	//msg_write(string("Lade ",filename));
	Script *s = NULL;

	// public und private aus dem Speicher versuchen zu laden
	if (is_public){
		for (int i=0;i<PublicScript.num;i++)
			if (PublicScript[i]->pre_script->Filename == filename.sys_filename())
				return PublicScript[i];
	}
#if 0
	int ae=-1;
	for (int i=0;i<NumPublicScripts;i++)
		if (strcmp(PublicScript[i].filename,SysFileName(filename))==0)
			ae=i;
	if (ae>=0){
		if (is_public){
			s=PublicScript[ae].script;
			//so("...pointer");
		}else{
			s=new Script();
			memcpy(s,PublicScript[ae].script,sizeof(Script));
			s->WaitingMode=WaitingModeNone;
			s->isCopy=true;
			s->OpcodeSize=0;
			s->Compiler();
			s->isPrivate=!is_public;
			s->ThisObject=-1;
			//so("...kopiert (private)");
			//msg_error(string("Script existiert schon!!! ",filename));
		}
		return s;
	}
#endif

	
	s = new Script();
	try{
		s->Load(filename, just_analyse);
	}catch(const Exception &e){
		delete(s);
		throw e;
	}
	s->isPrivate = !is_public;

	// store script in database
	if (is_public){
		//so("...neu (public)");
		PublicScript.add(s);
	}else{
		//so("...neu (private)");
		PrivateScript.add(s);
	}
	//msg_error(i2s(NumPublicScripts));
	return s;
}

#if 0
Script *LoadAsInclude(char *filename, bool just_analyse)
{
	msg_db_f("LoadAsInclude",4);
	//so(string("Include ",filename));
	// aus dem Speicher versuchen zu laden
	for (int i=0;i<ublicScript.size();i++)
		if (strcmp(PublicScript[i].filename, SysFileName(filename)) == 0){
			//so("...pointer");
			return PublicScript[i].script;
		}

	//so("nnneu");
	Script *s = new Script(filename, just_analyse);
	so("geladen....");
	//msg_write("...neu");
	s->isPrivate = false;

	// als public speichern
	PublicScript[NumPublicScripts].filename=new char[strlen(filename)+1];
	strcpy(PublicScript[NumPublicScripts].filename,SysFileName(filename));
	PublicScript[NumPublicScripts++].script=s;

	return s;
}
#endif

void ExecuteAllScripts()
{
	for (int i=0;i<PrivateScript.num;i++)
		PrivateScript[i]->Execute();
	
	for (int i=0;i<PublicScript.num;i++)
		PublicScript[i]->Execute();
}

void Remove(Script *s)
{
	msg_db_f("RemoveScript", 1);
	// remove references
	for (int i=0;i<s->pre_script->Includes.num;i++)
		s->pre_script->Includes[i]->ReferenceCounter --;

	// put on to-delete-list
	DeadScript.add(s);

	// remove from normal list
	if (s->isPrivate){
		for (int i=0;i<PrivateScript.num;i++)
			if (PrivateScript[i] == s)
				PrivateScript.erase(i);
	}else{
		for (int i=0;i<PublicScript.num;i++)
			if (PublicScript[i] == s)
				PublicScript.erase(i);
	}

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
		if ((!s->pre_script->FlagImmortal) || (even_immortal))
			Remove(s);
	foreachb(Script *s, PrivateScript)
		if ((!s->pre_script->FlagImmortal) || (even_immortal))
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

void reset_script(Script *s)
{
	s->ReferenceCounter = 0;
	s->isCopy = false;
	s->isPrivate = false;
	
	s->cur_func = NULL;
	s->WaitingMode = 0;
	s->TimeToWait = 0;
	s->ShowCompilerStats = (!CompileSilently) && ShowCompilerStats;
	
	s->pre_script = NULL;

	s->Opcode = NULL;
	s->OpcodeSize = 0;
	s->ThreadOpcode = NULL;
	s->ThreadOpcodeSize = 0;
	s->Memory = NULL;
	s->MemorySize = 0;
	s->MemoryUsed = 0;
	s->Stack = NULL;

	//func.clear();
	//g_var.clear();
	//cnst.clear();
}

void Script::Load(const string &filename, bool just_analyse)
{
	msg_db_f("loading script", 1);
	JustAnalyse = just_analyse;
	pre_script->LoadAndParseFile(filename, just_analyse);

	if (!JustAnalyse)
		Compiler();
	/*if (pre_script->FlagShow)
		pre_script->Show();*/
	if ((!JustAnalyse) && (pre_script->FlagDisassemble)){
		msg_write("disasm");
		msg_write(OpcodeSize);
		msg_write(Asm::Disassemble(ThreadOpcode,ThreadOpcodeSize));
		msg_write("\n\n");
		//printf("%s\n\n", Asm::Disassemble(Opcode,OpcodeSize));
		msg_write(Asm::Disassemble(Opcode,OpcodeSize));
	}
}

void Script::DoError(const string &str, int overwrite_line)
{
	pre_script->DoError(str, overwrite_line);
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
	for (int i=0;i<pre_script->RootOfAllEvil.var.num;i++)
		if (pre_script->RootOfAllEvil.var[i].name == name){
			/*msg_write("var");
			msg_write(pre_script->RootOfAllEvil.Var[i].Type->Size);
			msg_write((int)g_var[i]);*/
			memcpy(g_var[i], data, pre_script->RootOfAllEvil.var[i].type->size);
			return;
		}
	msg_error("CScript.SetVariable: variable " + name + " not found");
}

Script::Script()
{
	so("creating empty script (for console)");
	right();
	reset_script(this);
	WaitingMode = WaitingModeFirst;

	pre_script = new PreScript(this);
	
	pre_script->Filename = "-console script-";

	so("-ok");
	left();
}

Script::~Script()
{
	msg_db_f("~CScript", 4);
	if ((Memory) && (!JustAnalyse)){
		delete[](Memory);
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
	delete(pre_script);
}



static string single_command;


// bad:  should clean up in case of errors!
void ExecuteSingleScriptCommand(const string &cmd)
{
	if (cmd.num < 1)
		return;
	msg_db_f("ExecuteSingleScriptCmd", 2);
	single_command = cmd;
	msg_write("script command: " + single_command);

	// empty script
	Script *s = new Script();
	PreScript *ps = s->pre_script;

	try{

// find expressions
	ps->Analyse(single_command.c_str(), false);
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
	ps->Exp.cur_line = &ps->Exp.line[0];
	ps->Exp.cur_exp = 0;
	ps->Exp._cur_ = ps->Exp.cur_line->exp[ps->Exp.cur_exp].name;
	ps->GetCompleteCommand(func->block, func);
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
	foreachi(Function *f, pre_script->Functions, i)
		if ((f->name == name) && (f->literal_return_type->name == return_type) && (num_params == f->num_params)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->name != param_type[j])
				if (f->literal_param_type[j]->name != param_type[j])
					params_ok = false;
			if (params_ok){
				if (func.num > 0)
					return (void*)func[i];
				else
					return (void*)0xdeadbeaf; // when just analyzing...
			}
		}

	return NULL;
}

void Script::ShowVars(bool include_consts)
{	
/*	int ss=0;
	int i;
	string name;
	Type *t;
	int n=pre_script->RootOfAllEvil.Var.num;
	if (include_consts)
		n+=pre_script->Constant.num;
	for (i=0;i<n;i++){
		char *add=(char*)&Stack[ss];
		if (i<pre_script->RootOfAllEvil.Var.num){
			name = pre_script->RootOfAllEvil.Var[i].Name;
			t=pre_script->RootOfAllEvil.Var[i].Type;
		}else{
			name = "---const---";
			t=pre_script->Constant[i-pre_script->RootOfAllEvil.Var.num].type;
		}
		if (t == TypeInt)
			msg_write(format("%p: %s = %d", &add, name.c_str(), *(int*)&Stack[ss]));
		else if (t==TypeFloat)
			msg_write(format("%p: %s = %.3f", &add, name.c_str(), *(float*)&Stack[ss]));
		else if (t==TypeBool)
			msg_write(format("%p: %s = (bool) %d", &add, name.c_str(), *(int*)&Stack[ss]));
		else if (t==TypeVector)
			msg_write(string(d2h((char*)&add,4,false),":  ",name,"  =  (",string(f2s(*(float*)&Stack[ss],3)," , ",f2s(*(float*)&Stack[ss+4],3)," , ",f2s(*(float*)&Stack[ss+8],3),")")));
		else if ((t==TypeColor)||(t==TypeRect)||(t==TypeQuaternion))
			msg_write(string(d2h((char*)&add,4,false),":  ",name,"  =  (",string(f2s(*(float*)&Stack[ss],3)," , ",f2s(*(float*)&Stack[ss+4],3),string(" , ",f2s(*(float*)&Stack[ss+8],3)," , ",f2s(*(float*)&Stack[ss+12],3),")"))));
		else if (t->IsPointer)
			msg_write(format("%p: %s = %p", &add, name.c_str(), Stack[ss]));
		else if (t==TypeString)
			msg_write(format("%p: %s = \"...\"", &add, name.c_str()));
		else
			msg_write(string(d2h((char*)&add,4,false),":  ",name,"  =  ??? (unbekannter Typ)"));
		ss+=t->Size;
	}*/
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
	msg_db_f(pre_script->Filename.c_str(),1);

	// handle wait-commands
	if (WaitingMode==WaitingModeFirst){
		GlobalWaitingMode=WaitingModeNone;
		msg_db_f("->First",1);
		//msg_right();
		first_execution();
		//msg_left();
	}else{
#ifdef _X_ALLOW_META_
		if (WaitingMode==WaitingModeRT)
			TimeToWait-=ElapsedRT;
		else
			TimeToWait-=Elapsed;
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
