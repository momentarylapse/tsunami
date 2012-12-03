/*----------------------------------------------------------------------------*\
| CScript                                                                      |
| -> C-like scripting system                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../file/file.h"
#ifdef OS_LINUX
	#include <sys/mman.h>
#endif
#ifdef OS_WINDOWS
	#include "windows.h"
#endif
#include <stdarg.h>
#include "script.h"

#include "../00_config.h"
#ifdef _X_ALLOW_META_
	#include "../x/x.h"
#endif

string ScriptVersion = "0.10.2.1";

//#define ScriptDebug

int GlobalWaitingMode;
float GlobalTimeToWait;

bool ScriptCompileSilently = false;
bool ScriptShowCompilerStats = true;

static Array<CScript*> cur_script_stack;
inline void push_cur_script(CScript *s)
{
	cur_script_stack.add(s);
	cur_script = s;
}
inline void pop_cur_script()
{
	cur_script_stack.resize(cur_script_stack.num - 1);
	if (cur_script_stack.num >= 1)
		cur_script = cur_script_stack.back();
	else
		cur_script = NULL;
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



Array<CScript*> PublicScript;
Array<CScript*> PrivateScript;
Array<CScript*> DeadScript;


string ScriptDirectory = "";




CScript *LoadScript(const string &filename, bool is_public, bool just_analyse)
{
	//msg_write(string("Lade ",filename));
	CScript *s = NULL;

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
			s=new CScript();
			memcpy(s,PublicScript[ae].script,sizeof(CScript));
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

	
	s = new CScript(filename, just_analyse);
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
CScript *LoadScriptAsInclude(char *filename, bool just_analyse)
{
	msg_db_r("LoadAsInclude",4);
	//so(string("Include ",filename));
	// aus dem Speicher versuchen zu laden
	for (int i=0;i<ublicScript.size();i++)
		if (strcmp(PublicScript[i].filename, SysFileName(filename)) == 0){
			//so("...pointer");
			msg_db_l(4);
			return PublicScript[i].script;
		}

	//so("nnneu");
	CScript *s = new CScript(filename, just_analyse);
	so("geladen....");
	//msg_write("...neu");
	s->isPrivate = false;

	// als public speichern
	PublicScript[NumPublicScripts].filename=new char[strlen(filename)+1];
	strcpy(PublicScript[NumPublicScripts].filename,SysFileName(filename));
	PublicScript[NumPublicScripts++].script=s;

	msg_db_l(4);
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

void RemoveScript(CScript *s)
{
	msg_db_r("RemoveScript", 1);
	// remove references
	for (int i=0;i<s->pre_script->Include.num;i++)
		s->pre_script->Include[i]->ReferenceCounter --;

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
	msg_db_l(1);
}

void DeleteAllScripts(bool even_immortal, bool force)
{
	msg_db_r("DeleteAllScripts", 1);

	// try to erase them...
	foreachb(CScript *s, PublicScript)
		if ((!s->pre_script->FlagImmortal) || (even_immortal))
			RemoveScript(s);
	foreachb(CScript *s, PrivateScript)
		if ((!s->pre_script->FlagImmortal) || (even_immortal))
			RemoveScript(s);

	// undead... really KILL!
	if (force){
		foreachb(CScript *s, DeadScript)
			delete(s);
		DeadScript.clear();
	}

	//ScriptResetSemiExternalData();

	
	/*msg_write("------------------------------------------------------------------");
	msg_write(mem_used);
	for (int i=0;i<num_ps;i++)
		msg_write(string2("  fehlt:   %s  %p  (%d)",ppn[i],ppp[i],pps[i]));
	*/
	msg_db_l(1);
}

void reset_script(CScript *s)
{
	s->ReferenceCounter = 0;
	s->Error = false;
	s->ParserError = false;
	s->LinkerError = false;
	s->isCopy = false;
	s->isPrivate = false;
	
	s->ErrorLine = 0;
	s->ErrorColumn = 0;
	s->cur_func = NULL;
	s->WaitingMode = 0;
	s->TimeToWait = 0;
	s->ShowCompilerStats = (!ScriptCompileSilently) && ScriptShowCompilerStats;
	
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

CScript::CScript(const string &filename, bool just_analyse)
{
	msg_db_r("loading script", 1);
	reset_script(this);
	push_cur_script(this);
	JustAnalyse = just_analyse;

	WaitingMode = WaitingModeFirst;
	ShowCompilerStats = (!ScriptCompileSilently) && ScriptShowCompilerStats;

	pre_script = new CPreScript(this);
	pre_script->LoadAndParseFile(filename, just_analyse);
	ParserError = Error = pre_script->Error;
	LinkerError = pre_script->IncludeLinkerError;
	ErrorLine = pre_script->ErrorLine;
	ErrorColumn = pre_script->ErrorColumn;
	ErrorMsg = pre_script->ErrorMsg;
	ErrorMsgExt[0] = pre_script->ErrorMsgExt[0];
	ErrorMsgExt[1] = pre_script->ErrorMsgExt[1];

	if ((!Error) && (!JustAnalyse))
		Compiler();
	/*if (pre_script->FlagShow)
		pre_script->Show();*/
	if ((!Error) && (!JustAnalyse) && (pre_script->FlagDisassemble)){
		msg_write("disasm");
		msg_write(OpcodeSize);
		msg_write(Opcode2Asm(ThreadOpcode,ThreadOpcodeSize));
		msg_write("\n\n");
		//printf("%s\n\n", Opcode2Asm(Opcode,OpcodeSize));
		msg_write(Opcode2Asm(Opcode,OpcodeSize));
	}

	pop_cur_script();
	msg_db_l(1);
}

void CScript::DoError(const string &str, int overwrite_line)
{
	pre_script->DoError(str, overwrite_line);
	Error = true;
	ErrorLine = pre_script->ErrorLine;
	ErrorColumn = pre_script->ErrorColumn;
	ErrorMsgExt[0] = pre_script->ErrorMsgExt[0];
	ErrorMsgExt[1] = pre_script->ErrorMsgExt[1];
	ErrorMsg = pre_script->ErrorMsg;
}

void CScript::DoErrorInternal(const string &str)
{
	if (Error)
		return;
	msg_write("\n\n\n");
	msg_write("------------------------       Error       -----------------------");	
	Error = true;
	
	ErrorMsg = "internal compiler error (Call Michi!): " + str;
	if (cur_func)
		ErrorMsg += " (in function '" + cur_func->Name  + "')";
	ErrorMsgExt[0] = ErrorMsg;
	ErrorMsgExt[1] = "";
	ErrorLine = 0;
	ErrorColumn = 0;
	msg_write(ErrorMsg);
	msg_write("------------------------------------------------------------------");
	msg_write(pre_script->Filename);
	msg_write("\n\n\n");
}

void CScript::DoErrorLink(const string &str)
{
	DoError(str);
	LinkerError = true;
}

void CScript::SetVariable(const string &name, void *data)
{
	msg_db_r("SetVariable", 4);
	//msg_write(name);
	for (int i=0;i<pre_script->RootOfAllEvil.Var.num;i++)
		if (pre_script->RootOfAllEvil.Var[i].Name == name){
			/*msg_write("var");
			msg_write(pre_script->RootOfAllEvil.Var[i].Type->Size);
			msg_write((int)g_var[i]);*/
			memcpy(g_var[i], data, pre_script->RootOfAllEvil.Var[i].Type->Size);
			msg_db_l(4);
			return;
		}
	msg_error("CScript.SetVariable: variable " + name + " not found");
	msg_db_l(4);
}

int LocalOffset,LocalOffsetMax;

/*int get_func_temp_size(sFunction *f)
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

enum{
	inNop,
	inPushEbp,
	inMovEbpEsp,
	inMovEspM,
	inMovEdxpi8Eax,
	inLeave,
	inRet,
	inRet4,
	inMovEaxM,
	inMovMEax,
	inMovEdxM,
	inMovMEdx,
	inMovAlM8,
	inMovAhM8,
	inMovBlM8,
	inMovBhM8,
	inMovClM8,
	inMovM8Al,
	inMovMEbp,
	inMovMEsp,
	inLeaEaxM,
	inLeaEdxM,
	inPushM,
	inPushEax,
	inPushEdx,
	inPopEax,
	inPopEsp,
	inAndEaxM,
	inOrEaxM,
	inXorEaxM,
	inAddEaxM,
	inAddEdxM,
	inAddMEax,
	inSubEaxM,
	inSubMEax,
	inMulEaxM,
	inDivEaxM,
	inDivEaxEbx,
	inCmpEaxM,
	inCmpAlM8,
	inCmpM80,
	inSetzAl,
	inSetnzAl,
	inSetnleAl,
	inSetnlAl,
	inSetlAl,
	inSetleAl,
	inAndAlM8,
	inOrAlM8,
	inXorAlM8,
	inAddAlM8,
	inAddM8Al,
	inSubAlM8,
	inSubM8Al,
	inCallRel32,
	inJmpEax,
	inJmpC32,
	inJzC8,
	inJzC32,
	inLoadfM,
	inSavefM,
	inLoadfiM,
	inAddfM,
	inSubfM,
	inMulfM,
	inDivfM,
	inShrEaxCl,
	inShlEaxCl,
	NumAsmInstructions
};


#define CallRel32OCSize			5
#define AfterWaitOCSize			10



inline void OCAddChar(char *oc,int &ocs,int c)
{	oc[ocs]=(char)c;	ocs++;	}

inline void OCAddWord(char *oc,int &ocs,int i)
{	*(short*)&oc[ocs]=i;	ocs+=2;	}

inline void OCAddInt(char *oc,int &ocs,int i)
{	*(int*)&oc[ocs]=i;	ocs+=4;	}

int OCOParam;

// offset: used to shift addresses   (i.e. mov iteratively to a big local variable)
void OCAddInstruction(char *oc, int &ocs, int inst, int kind, void *param = NULL, int offset = 0)
{
	int code = 0;
	int pk[2] = {PKNone, PKNone};
	void *p[2] = {NULL, NULL};
	int m = -1;
	int size = 32;
	//msg_write(offset);

// corrections
	// lea
	if ((inst == inLeaEaxM) && (kind == KindRefToConst)){
		OCAddInstruction(oc, ocs, inst, KindVarGlobal, param, offset);
		return;
	}

	
	switch(inst){
		case inNop:			code = inst_nop;	break;
		case inPushEbp:		code = inst_push;	pk[0] = PKRegister;	p[0] = (void*)RegEbp;	break;
		case inMovEbpEsp:	code = inst_mov;	pk[0] = PKRegister;	p[0] = (void*)RegEbp;	pk[1] = PKRegister;	p[1] = (void*)RegEsp;	break;
		case inMovEspM:		code = inst_mov;	pk[0] = PKRegister;	p[0] = (void*)RegEsp;	m = 1;	break;
		case inMovEdxpi8Eax:code = inst_mov;	pk[0] = PKEdxRel;	p[0] = param;	pk[1] = PKRegister;	p[1] = (void*)RegEax;	break;
		case inLeave:		code = inst_leave;	break;
		case inRet:			code = inst_ret;	break;
		case inRet4:		code = inst_ret;	pk[0] = PKConstant16;	p[0] = (void*)4;	break;
		case inMovEaxM:		code = inst_mov;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inMovMEax:		code = inst_mov;	pk[1] = PKRegister;	p[1] = (void*)RegEax;	m = 0;	break;
		case inMovEdxM:		code = inst_mov;	pk[0] = PKRegister;	p[0] = (void*)RegEdx;	m = 1;	break;
		case inMovMEdx:		code = inst_mov;	pk[1] = PKRegister;	p[1] = (void*)RegEdx;	m = 0;	break;
		case inMovAlM8:		code = inst_mov_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inMovAhM8:		code = inst_mov_b;	pk[0] = PKRegister;	p[0] = (void*)RegAh;	m = 1;	size = 8;	break;
		case inMovBlM8:		code = inst_mov_b;	pk[0] = PKRegister;	p[0] = (void*)RegBl;	m = 1;	size = 8;	break;
		case inMovBhM8:		code = inst_mov_b;	pk[0] = PKRegister;	p[0] = (void*)RegBh;	m = 1;	size = 8;	break;
		case inMovClM8:		code = inst_mov_b;	pk[0] = PKRegister;	p[0] = (void*)RegCl;	m = 1;	size = 8;	break;
		case inMovM8Al:		code = inst_mov_b;	pk[1] = PKRegister;	p[1] = (void*)RegAl;	m = 0;	size = 8;	break;
		case inMovMEbp:		code = inst_mov;	pk[1] = PKRegister;	p[1] = (void*)RegEbp;	m = 0;	break;
		case inMovMEsp:		code = inst_mov;	pk[1] = PKRegister;	p[1] = (void*)RegEsp;	m = 0;	break;
		case inLeaEaxM:		code = inst_lea;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inLeaEdxM:		code = inst_lea;	pk[0] = PKRegister;	p[0] = (void*)RegEdx;	m = 1;	break;
		case inPushM:		code = inst_push;	m = 0;	break;
		case inPushEax:		code = inst_push;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	break;
		case inPushEdx:		code = inst_push;	pk[0] = PKRegister;	p[0] = (void*)RegEdx;	break;
		case inPopEax:		code = inst_pop;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	break;
		case inPopEsp:		code = inst_pop;	pk[0] = PKRegister;	p[0] = (void*)RegEsp;	break;
		case inAndEaxM:		code = inst_and;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inOrEaxM:		code = inst_or;		pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inXorEaxM:		code = inst_xor;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inAddEaxM:		code = inst_add;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inAddEdxM:		code = inst_add;	pk[0] = PKRegister;	p[0] = (void*)RegEdx;	m = 1;	break;
		case inAddMEax:		code = inst_add;	pk[1] = PKRegister;	p[1] = (void*)RegEax;	m = 0;	break;
		case inSubEaxM:		code = inst_sub;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inSubMEax:		code = inst_sub;	pk[1] = PKRegister;	p[1] = (void*)RegEax;	m = 0;	break;
		case inMulEaxM:		code = inst_imul;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inDivEaxM:		code = inst_div;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inDivEaxEbx:	code = inst_div;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	pk[1] = PKRegister;	p[1] = (void*)RegEbx;	break;
		case inCmpEaxM:		code = inst_cmp;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	m = 1;	break;
		case inCmpAlM8:		code = inst_cmp_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inCmpM80:		code = inst_cmp_b;	pk[1] = PKConstant8;	p[1] = NULL;	m = 0;	size = 8;	break;
		case inSetzAl:		code = inst_setz_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	break;
		case inSetnzAl:		code = inst_setnz_b;pk[0] = PKRegister;	p[0] = (void*)RegAl;	break;
		case inSetnleAl:	code = inst_setnle_b;pk[0] = PKRegister;	p[0] = (void*)RegAl;	break;
		case inSetnlAl:		code = inst_setnl_b;pk[0] = PKRegister;	p[0] = (void*)RegAl;	break;
		case inSetlAl:		code = inst_setl_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	break;
		case inSetleAl:		code = inst_setle_b;pk[0] = PKRegister;	p[0] = (void*)RegAl;	break;
		case inAndAlM8:		code = inst_and_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inOrAlM8:		code = inst_or_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inXorAlM8:		code = inst_xor_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inAddAlM8:		code = inst_add_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inAddM8Al:		code = inst_add_b;	pk[1] = PKRegister;	p[1] = (void*)RegAl;	m = 0;	size = 8;	break;
		case inSubAlM8:		code = inst_sub_b;	pk[0] = PKRegister;	p[0] = (void*)RegAl;	m = 1;	size = 8;	break;
		case inSubM8Al:		code = inst_sub_b;	pk[1] = PKRegister;	p[1] = (void*)RegAl;	m = 0;	size = 8;	break;
		case inCallRel32:	code = inst_call;	m = 0;	break;
		case inJmpEax:		code = inst_jmp;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	break;
		case inJmpC32:		code = inst_jmp;	m = 0;	break;
		case inJzC8:		code = inst_jz_b;	m = 0;	size = 8;	break;
		case inJzC32:		code = inst_jz;		m = 0;	size = 8;	break;
		case inLoadfM:		code = inst_fld;	m = 0;	break;
		case inSavefM:		code = inst_fstp;	m = 0;	break;
		case inLoadfiM:		code = inst_fild;	m = 0;	break;
		case inAddfM:		code = inst_fadd;	m = 0;	break;
		case inSubfM:		code = inst_fsub;	m = 0;	break;
		case inMulfM:		code = inst_fmul;	m = 0;	break;
		case inDivfM:		code = inst_fdiv;	m = 0;	break;
		case inShrEaxCl:	code = inst_shr;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	pk[1] = PKRegister;	p[1] = (void*)RegCl;	break;
		case inShlEaxCl:	code = inst_shl;	pk[0] = PKRegister;	p[0] = (void*)RegEax;	pk[1] = PKRegister;	p[1] = (void*)RegCl;	break;
		default:
			msg_todo(format("unhandled instruction: %d", inst));
			cur_script->DoErrorInternal("asm error");
			return;
	}

	
	// const as global var?
	if (kind == KindRefToConst){
		if (!AsmImmediateAllowed(code)){
			//msg_write("evil");
			kind = KindVarGlobal;
		}

		if (inst == inCmpM80){
			kind = KindVarGlobal;
		}
	}
	

	// parameters
	if ((m >= 0) && (kind >= 0)){
	
		if (kind == KindVarLocal){
			pk[m] = PKLocal;
			p[m] = (void*)((long)param + offset);
		}else if (kind == KindVarGlobal){
			pk[m] = PKDerefConstant;
			p[m] = (void*)((long)param + offset);
		}else if (kind == KindConstant){
			pk[m] = (size == 8) ? PKConstant8 : PKConstant32;
			p[m] = param;
		}else if (kind == KindRefToConst){
			kind = KindConstant;
			pk[m] = (size == 8) ? PKConstant8 : PKConstant32;
			p[m] = *(void**)((long)param + offset);
		}else if ((kind == KindRefToLocal) || (kind == KindRefToGlobal)){
			if (kind == KindRefToLocal)
				OCAddInstruction(oc, ocs, inMovEdxM, KindVarLocal, param);
			else if (kind == KindRefToGlobal)
				OCAddInstruction(oc, ocs, inMovEdxM, KindVarGlobal, param);
			if (offset != 0)
				OCAddInstruction(oc, ocs, inAddEdxM, KindConstant, (char*)(long)offset);
			pk[m] = PKDerefRegister;
			p[m] = (void*)RegEdx;
		}else{
			msg_error("kind unhandled");
			msg_write(kind);
		}
	}
	
	// compile
	if (!AsmAddInstruction(oc, ocs, code, pk[0], p[0], pk[1], p[1]))
		cur_script->DoErrorInternal("asm error");

	OCOParam = AsmOCParam;
}

/*enum{
	PKInvalid,
	PKNone,
	PKRegister,			// eAX
	PKDerefRegister,	// [eAX]
	PKLocal,			// [ebp + 0x0000]
	PKStackRel,			// [esp + 0x0000]
	PKConstant32,		// 0x00000000
	PKConstant16,		// 0x0000
	PKConstant8,		// 0x00
	PKConstantDouble,   // 0x00:0x0000   ...
	PKDerefConstant		// [0x0000]
};*/
//bool AsmAddInstruction(char *oc, int &ocs, int inst, int param1_type, void *param1, int param2_type, void *param2, int offset = 0, int insert_at = -1);

void OCAddEspAdd(char *oc,int &ocs,int d)
{
	if (d>0){
		if (d>120)
			AsmAddInstruction(oc, ocs, inst_add, PKRegister, (void*)RegEsp, PKConstant32, (void*)(long)d);
		else
			AsmAddInstruction(oc, ocs, inst_add_b, PKRegister, (void*)RegEsp, PKConstant8, (void*)(long)d);
	}else if (d<0){
		if (d<-120)
			AsmAddInstruction(oc, ocs, inst_sub, PKRegister, (void*)RegEsp, PKConstant32, (void*)(long)(-d));
		else
			AsmAddInstruction(oc, ocs, inst_sub_b, PKRegister, (void*)RegEsp, PKConstant8, (void*)(long)(-d));
	}
}

void init_sub_super_array(CPreScript *ps, sFunction *f, sType *t, char* g_var, int offset)
{
	// direct
	if (t->IsSuperArray){
		if (g_var)
			((DynamicArray*)(g_var + offset))->init(t->SubType->Size);
		if (f){}
	}

	// indirect
	if (t->IsArray)
		for (int i=0;i<t->ArrayLength;i++)
			init_sub_super_array(ps, f, t->SubType, g_var, offset + i * t->SubType->Size);
	for (int i=0;i<t->Element.num;i++)
		init_sub_super_array(ps, f, t->Element[i].Type, g_var, offset + t->Element[i].Offset);
}

void find_all_super_arrays(CPreScript *ps, sFunction *f, Array<char*> &g_var)
{
	for (int i=0;i<f->Var.num;i++)
		init_sub_super_array(ps, f, f->Var[i].Type, g_var[i], 0);
}

void CScript::AllocateMemory()
{
	// get memory size needed
	MemorySize = 0;
	for (int i=0;i<pre_script->RootOfAllEvil.Var.num;i++)
		MemorySize += mem_align(pre_script->RootOfAllEvil.Var[i].Type->Size);
	foreachi(sConstant &c, pre_script->Constant, i){
		int s = c.type->Size;
		if (c.type == TypeString){
			// const string -> variable length   (+ super array frame)
			s = strlen(c.data) + 1 + 16;
		}
		MemorySize += mem_align(s);
	}
	if (MemorySize > 0)
		Memory = new char[MemorySize];
}

void CScript::AllocateStack()
{
	// use your own stack if needed
	//   wait() used -> needs to switch stacks ("tasks")
	Stack = NULL;
	foreach(sCommand *cmd, pre_script->Command){
		if (cmd->Kind == KindCompilerFunction)
			if ((cmd->LinkNr == CommandWait) || (cmd->LinkNr == CommandWaitRT) || (cmd->LinkNr == CommandWaitOneFrame)){
				Stack = new char[ScriptStackSize];
				break;
			}
	}
}

void CScript::AllocateOpcode()
{
	// allocate some memory for the opcode......    has to be executable!!!   (important on amd64)
#ifdef OS_WINDOWS
	Opcode=(char*)VirtualAlloc(NULL,SCRIPT_MAX_OPCODE,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
	ThreadOpcode=(char*)VirtualAlloc(NULL,SCRIPT_MAX_THREAD_OPCODE,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
#else
	Opcode = (char*)mmap(0, SCRIPT_MAX_OPCODE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE, 0, 0);
	ThreadOpcode = (char*)mmap(0, SCRIPT_MAX_THREAD_OPCODE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | MAP_EXECUTABLE, 0, 0);
#endif
	if (((long)Opcode==-1)||((long)ThreadOpcode==-1))
		_do_error_int_("CScript:  could not allocate executable memory", 2,);
	OpcodeSize=0;
	ThreadOpcodeSize=0;
}

void CScript::MapConstantsToMemory()
{
	// constants -> Memory
	so("Konstanten");
	cnst.resize(pre_script->Constant.num);
	foreachi(sConstant &c, pre_script->Constant, i){
		cnst[i] = &Memory[MemorySize];
		int s = c.type->Size;
		if (c.type == TypeString){
			// const string -> variable Laenge
			s = strlen(pre_script->Constant[i].data) + 1;

			*(void**)&Memory[MemorySize] = &Memory[MemorySize + 16]; // .data
			*(int*)&Memory[MemorySize + 4] = s - 1; // .num
			*(int*)&Memory[MemorySize + 8] = 0; // .reserved
			*(int*)&Memory[MemorySize + 12] = 1; // .item_size
			MemorySize += 16;
		}
		memcpy(&Memory[MemorySize], (void*)c.data, s);
		MemorySize += mem_align(s);
	}
}

void CScript::MapGlobalVariablesToMemory()
{
	// global variables -> into Memory
	so("glob.Var.");
	g_var.resize(pre_script->RootOfAllEvil.Var.num);
	for (int i=0;i<pre_script->RootOfAllEvil.Var.num;i++){
		if (pre_script->FlagOverwriteVariablesOffset)
			g_var[i] = (char*)(long)(MemorySize + pre_script->VariablesOffset);
		else
			g_var[i] = &Memory[MemorySize];
		so(format("%d: %s", MemorySize, pre_script->RootOfAllEvil.Var[i].Name.c_str()));
		MemorySize += mem_align(pre_script->RootOfAllEvil.Var[i].Type->Size);
	}
	memset(Memory, 0, MemorySize); // reset all global variables to 0
	// initialize global super arrays
	find_all_super_arrays(pre_script, &pre_script->RootOfAllEvil, g_var);
}

static int OCORA;
void CScript::CompileOsEntryPoint()
{
	int nf=-1;
	foreachi(sFunction *ff, pre_script->Function, index)
		if (ff->Name == "main")
			nf = index;
	// call
	if (nf>=0)
		OCAddInstruction(Opcode,OpcodeSize,inCallRel32,KindConstant,NULL);
	TaskReturnOffset=OpcodeSize;
	OCORA=OCOParam;

	// put strings into Opcode!
	foreachi(sConstant &c, pre_script->Constant, i){
		if ((pre_script->FlagCompileOS) || (c.type == TypeString)){
			int offset = 0;
			if (pre_script->AsmMetaInfo)
				offset = ((sAsmMetaInfo*)pre_script->AsmMetaInfo)->CodeOrigin;
			cnst[i] = (char*)(long)(OpcodeSize + offset);
			int s = c.type->Size;
			if (c.type == TypeString)
				s = strlen(c.data) + 1;
			memcpy(&Opcode[OpcodeSize], (void*)c.data, s);
			OpcodeSize += s;
		}
	}
}

void CScript::LinkOsEntryPoint()
{
	int nf=-1;
	foreachi(sFunction *ff, pre_script->Function, index)
		if (ff->Name == "main")
			nf = index;
	if (nf>=0){
		int lll=((long)func[nf]-(long)&Opcode[TaskReturnOffset]);
		if (pre_script->FlagCompileInitialRealMode)
			lll+=5;
		else
			lll+=3;
		//printf("insert   %d  an %d\n", lll, OCORA);
		//msg_write(lll);
		//msg_write(d2h(&lll,4,false));
		*(int*)&Opcode[OCORA]=lll;
	}
}

void CScript::CompileTaskEntryPoint()
{
	// "stack" usage for waiting:
	//  -4 - ebp (before execution)
	//  -8 - ebp (script continue)
	// -12 - esp (script continue)
	// -16 - eip (script continue)
	// -20 - script stack...

	first_execution=(t_func*)&ThreadOpcode[ThreadOpcodeSize];
	// intro
	OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inPushEbp,-1); // within the actual program
	OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEbpEsp,-1);
	if (Stack){
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEspM,KindConstant,(char*)&Stack[ScriptStackSize]); // zum Anfang des Script-Stacks
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inPushEbp,-1); // adress of the old stack
		OCAddEspAdd(ThreadOpcode,ThreadOpcodeSize,-12); // space for wait() task data
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEbpEsp,-1);
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEaxM,KindConstant,(char*)WaitingModeNone); // "reset"
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovMEax,KindVarGlobal,(char*)&WaitingMode);
	}
	// call
	int nf = -1;
	foreachi(sFunction *ff, pre_script->Function, index){
		if (ff->Name == "main")
			if (ff->NumParams == 0)
				nf = index;
	}
	if (nf >= 0){
		// call main() ...correct adress will be put here later!
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inCallRel32,KindConstant,NULL);
		*(int*)&ThreadOpcode[OCOParam]=((long)func[nf]-(long)&ThreadOpcode[ThreadOpcodeSize]);
	}
	// outro
	if (Stack){
		OCAddEspAdd(ThreadOpcode,ThreadOpcodeSize,12); // make space for wait() task data
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inPopEsp,-1);
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEbpEsp,-1);
	}
	OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inLeave,-1);
	OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inRet,-1);

// "task" for execution after some wait()
	continue_execution=(t_func*)&ThreadOpcode[ThreadOpcodeSize];
	// Intro
	if (Stack){
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inPushEbp,-1); // within the external program
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEbpEsp,-1);
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovMEbp,KindVarGlobal,&Stack[ScriptStackSize-4]); // save the external ebp
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inMovEspM,KindConstant,&Stack[ScriptStackSize-16]); // to the eIP of the script
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inPopEax,-1);
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inAddEaxM,KindConstant,(char*)AfterWaitOCSize);
		OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inJmpEax,-1);
		//OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inLeave,-1);
		//OCAddInstruction(ThreadOpcode,ThreadOpcodeSize,inRet,-1);
		/*OCAddChar(0x90);
		OCAddChar(0x90);
		OCAddChar(0x90);*/
	}
}

// Opcode generieren
void CScript::Compiler()
{
	if (Error)	return;
	msg_db_r("Compiler",2);

	pre_script->MapLocalVariablesToStack();
	
	if (!Error)
		pre_script->BreakDownComplicatedCommands();
#ifdef ScriptDebug
	pre_script->Show();
#endif
	
	if (!Error)
		pre_script->Simplify();
	if (!Error)
		pre_script->PreProcessor(this);


	AllocateMemory();
	AllocateStack();

	MemorySize = 0;
	MapGlobalVariablesToMemory();
	MapConstantsToMemory();

	AllocateOpcode();

	
	if (!Error)
		pre_script->PreProcessorAddresses(this);


	
// compiling an operating system?
//   -> create an entry point for execution... so we can just call Opcode like a function
	if ((pre_script->FlagCompileOS)||(pre_script->FlagCompileInitialRealMode))
		CompileOsEntryPoint();


// compile functions into Opcode
	so("Funktionen");
	func.resize(pre_script->Function.num);
	foreachi(sFunction *f, pre_script->Function, i){
		right();
		func[i] = (t_func*)&Opcode[OpcodeSize];
		CompileFunction(f, Opcode, OpcodeSize);
		left();

		if (!Error)
			if (pre_script->AsmMetaInfo)
				if (((sAsmMetaInfo*)pre_script->AsmMetaInfo)->WantedLabel.num > 0)
					_do_error_(format("unknown name in assembler code:  \"%s\"", ((sAsmMetaInfo*)pre_script->AsmMetaInfo)->WantedLabel[0].Name.c_str()), 2,);
	}


// "task" for the first execution of main() -> ThreadOpcode
	if (!pre_script->FlagCompileOS)
		CompileTaskEntryPoint();




	if (pre_script->FlagCompileOS)
		LinkOsEntryPoint();

	//msg_db_out(1,GetAsm(Opcode,OpcodeSize));

	//_expand(Opcode,OpcodeSize);

	WaitingMode = WaitingModeFirst;
	
	if (ShowCompilerStats){
		msg_write("--------------------------------");
		msg_write(format("Opcode: %db, Memory: %db",OpcodeSize,MemorySize));
	}
	msg_db_l(2);
}

CScript::CScript()
{
	so("creating empty script (for console)");
	right();
	reset_script(this);
	WaitingMode = WaitingModeFirst;

	pre_script = new CPreScript(this);
	
	pre_script->Filename = "-console script-";

	so("-ok");
	left();
}

CScript::~CScript()
{
	msg_db_r("~CScript", 4);
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
	msg_db_l(4);
}



static string single_command;


// bad:  should clean up in case of errors!
void ExecuteSingleScriptCommand(const string &cmd)
{
	if (cmd.num < 1)
		return;
	msg_db_r("ExecuteSingleScriptCmd", 2);
	single_command = cmd;
	msg_write("script command: " + single_command);

	// empty script
	CScript *s = new CScript();
	CPreScript *ps = s->pre_script;

// find expressions
	ps->Analyse(single_command.c_str(), false);
	if (ps->Exp.line[0].exp.num < 1){
		//clear_exp_buffer(&ps->Exp);
		delete(s);
		msg_db_l(2);
		return;
	}

// analyse syntax

	// create a main() function
	sFunction *f = ps->AddFunction("--command-func--", TypeVoid);
	f->_VarSize = 0; // set to -1...

	// parse
	ps->Exp.cur_line = &ps->Exp.line[0];
	ps->Exp.cur_exp = 0;
	ps->Exp._cur_ = ps->Exp.cur_line->exp[ps->Exp.cur_exp].name;
	ps->GetCompleteCommand(f->Block, f);
	//pre_script->GetCompleteCommand((pre_script->Exp->ExpNr,0,0,&f);
	s->Error |= ps->Error;

	if (!s->Error)
		ps->ConvertCallByReference();

// compile
	if (!s->Error)
		s->Compiler();

	/*if (true){
		printf("%s\n\n", Opcode2Asm(s->ThreadOpcode,s->ThreadOpcodeSize));
		printf("%s\n\n", Opcode2Asm(s->Opcode,s->OpcodeSize));
		//msg_write(Opcode2Asm(Opcode,OpcodeSize));
	}*/
// execute
	if (!s->Error){
		typedef void void_func();
		void_func *f = (void_func*)s->MatchFunction("--command-func--", "void", 0);
		if (f)
			f();
	}

	delete(s);
	msg_db_l(2);
}

void *CScript::MatchFunction(const string &name, const string &return_type, int num_params, ...)
{
	msg_db_r("MatchFunction", 2);
	
	// process argument list
	va_list marker;
	va_start(marker, num_params);
	string param_type[SCRIPT_MAX_PARAMS];
	for (int p=0;p<num_params;p++)
		param_type[p] = string(va_arg(marker, char*));
	va_end(marker);

	// match
	foreachi(sFunction *f, pre_script->Function, i)
		if ((f->Name == name) && (f->LiteralType->Name == return_type) && (num_params == f->NumParams)){

			bool params_ok = true;
			for (int j=0;j<num_params;j++)
				//if ((*f)->Var[j].Type->Name != param_type[j])
				if (f->LiteralParamType[j]->Name != param_type[j])
					params_ok = false;
			if (params_ok){
				msg_db_l(2);
				if (func.num > 0)
					return (void*)func[i];
				else
					return (void*)0xdeadbeaf; // when just analyzing...
			}
		}

	msg_db_l(2);
	return NULL;
}

void CScript::ShowVars(bool include_consts)
{	
/*	int ss=0;
	int i;
	string name;
	sType *t;
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

void CScript::Execute()
{
	if (Error)	return;
	if (WaitingMode==WaitingModeNone)	return;
	#ifdef ScriptDebug
		//so("\n\n\n################### fuehre aus ######################\n\n\n");
	#endif
	shift_right=0;
	//msg_db_r(string("Execute ",pre_script->Filename),1);
	msg_db_r("Execute", 1);
	msg_db_r(pre_script->Filename.c_str(),1);

	// handle wait-commands
	if (WaitingMode==WaitingModeFirst){
		GlobalWaitingMode=WaitingModeNone;
		msg_db_r("->First",1);
		//msg_right();
		first_execution();
		msg_db_l(1);
		//msg_left();
	}else{
#ifdef _X_ALLOW_META_
		if (WaitingMode==WaitingModeRT)
			TimeToWait-=ElapsedRT;
		else
			TimeToWait-=Elapsed;
		if (TimeToWait>0){
			msg_db_l(1);
			msg_db_l(1);
			return;
		}
#endif
		GlobalWaitingMode=WaitingModeNone;
		//msg_write(ThisObject);
		msg_db_r("->Continue",1);
		//msg_write(">---");
		//msg_right();
		continue_execution();
		//msg_write("---<");
		msg_db_l(1);
		//msg_write("ok");
		//msg_left();
	}
	WaitingMode=GlobalWaitingMode;
	TimeToWait=GlobalTimeToWait;

	msg_db_l(1);
	msg_db_l(1);
}
