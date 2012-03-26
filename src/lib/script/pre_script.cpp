#include "script.h"
#include "../file/file.h"
#include <stdio.h>

//#define ScriptDebug


/*#define PRESCRIPT_DB_LEVEL	2
#define db_r(msg,level)		msg_db_r(msg,level+PRESCRIPT_DB_LEVEL)*/

static int PreConstantNr, NamedConstantNr;

inline bool type_match(sType *type, bool is_class, sType *wanted);
inline bool type_match_with_cast(sType *type, bool is_class, bool is_modifiable, sType *wanted, int &penalty, int &cast);
inline void CommandMakeOperator(sCommand *cmd, sCommand *p1, sCommand *p2, int op);

char cstr_temp[256];
const char *cstr2(const char *a, const char *b)
{
	strcpy(cstr_temp, a);
	strcat(cstr_temp, b);
	return cstr_temp;
}
 
#define GetPointerType(sub)	CreateNewType(cstr2(sub->Name, "*"), PointerSize, true, false, false, 0, sub)
#define GetReferenceType(sub)	CreateNewType(cstr2(sub->Name, "*"), PointerSize, true, true, false, 0, sub) 

inline sCommand *cp_command(CPreScript *ps, sCommand *c)
{
	sCommand *cmd = ps->AddCommand();
	*cmd = *c;
	return cmd;
}

inline void command_make_ref(CPreScript *ps, sCommand *c, sCommand *param)
{
	c->Kind = KindReference;
	c->NumParams = 1;
	c->Param[0] = param;
	c->Type = ps->GetPointerType(param->Type);
}

inline void ref_command(CPreScript *ps, sCommand *c)
{
	sCommand *t = cp_command(ps, c);
	command_make_ref(ps, c, t);
}

inline void command_make_deref(CPreScript *ps, sCommand *c, sCommand *param)
{
	c->Kind = KindDereference;
	c->NumParams = 1;
	c->Param[0] = param;
	c->Type = param->Type->SubType;
}

inline void deref_command(CPreScript *ps, sCommand *c)
{
	sCommand *t = cp_command(ps, c);
	command_make_deref(ps, c, t);
}

void reset_pre_script(CPreScript *ps)
{
	msg_db_r("reset_pre_script", 2);
	//memset(ps, 0, sizeof(CPreScript));
	ps->Filename = "";
	ps->Error = false;
	ps->ErrorMsg = "";
	ps->ErrorMsgExt[0] = "";
	ps->ErrorMsgExt[1] = "";
	ps->ErrorLine = 0;
	ps->ErrorColumn = 0;
	ps->IncludeLinkerError = false;
	ps->Buffer = "";
	ps->Exp.buffer = NULL;
	ps->FlagShow = false;
	ps->FlagShowPrae = false;
	ps->FlagDisassemble = false;
	ps->FlagCompileOS = false;
	ps->FlagCompileInitialRealMode = false;
	ps->FlagOverwriteVariablesOffset = false;
	ps->FlagImmortal = false;
	ps->FlagNoExecution = false;
	ps->AsmMetaInfo = NULL;
	strcpy(ps->RootOfAllEvil.Name, "RootOfAllEvil");
	ps->RootOfAllEvil.NumParams = 0;
	ps->RootOfAllEvil.Type = TypeVoid;

	// "include" default stuff
	ps->NumOwnTypes = 0;
	for (int i=0;i<PreType.num;i++)
		ps->Type.add(PreType[i]);
	msg_db_l(2);	
}

CPreScript::CPreScript()
{
	reset_pre_script(this);
}


CPreScript::CPreScript(const string &filename,bool just_analyse)
{
	msg_db_r("CPreScript",4);
	reset_pre_script(this);
	
	Filename = SysFileName(filename);

	Error = !LoadToBuffer(ScriptDirectory + Filename, just_analyse);

	
	if (!Error)
		PreCompiler(just_analyse);

	if (!Error)
		Parser();
	
	if ((!Error) && (FlagShowPrae))
		Show();

	if (!Error)
		ConvertCallByReference();

	/*if ((!Error) && (FlagShow))
		Show();*/

	/*if (!Error)
		BreakDownComplicatedCommands();*/

	if (!Error)
		Simplify();
	
	if (!Error)
		PreProcessor(NULL);

	if ((!Error) && (FlagShow))
		Show();

	clear_exp_buffer(&Exp);
	msg_db_l(4);
}



// ################################################################################################
//                                        Syntax-Analyse
// ################################################################################################

int indent_0;
bool indented, unindented;
inline void test_indent(int i)
{
	indented = (i > indent_0);
	unindented = (i < indent_0);
	indent_0 = i;
		
}

inline void reset_indent()
{
	indented = unindented = false;
	indent_0 = 0;
}

void line_out(CPreScript *ps)
{
	char str[1024];
	strcpy(str, ps->Exp.cur_line->exp[0].name);
	for (int i=1;ps->Exp.cur_line->exp.num;i++){
		strcat(str, "  ");
		strcat(str, ps->Exp.cur_line->exp[i].name);
	}
}


char Temp[1024];

static int shift_right=0;

static void stringout(const string &str)
{
	msg_write(str);
}

static void so(const char *str)
{
#ifdef ScriptDebug
	/*if (strlen(str)>256)
		str[256]=0;*/
	msg_write(str);
#endif
}

static void so(const string &str)
{
#ifdef ScriptDebug
	/*if (strlen(str)>256)
		str[256]=0;*/
	msg_write(str);
#endif
}

static void so(int i)
{
#ifdef ScriptDebug
	msg_write(i);
#endif
}

static void right()
{
#ifdef ScriptDebug
	msg_right();
	shift_right+=2;
#endif
}

static void left()
{
#ifdef ScriptDebug
	msg_left();
	shift_right-=2;
#endif
}

int s2i2(const char *str)
{
	if ((str[0]=='0')&&(str[1]=='x')){
		int r=0;
		str+=2;
		while(str[0]!=0){ 
			r*=16;
			if ((str[0]>='0')&&(str[0]<='9'))
				r+=str[0]-48;
			if ((str[0]>='a')&&(str[0]<='f'))
				r+=str[0]-'a'+10;
			if ((str[0]>='A')&&(str[0]<='F'))
				r+=str[0]-'A'+10;
			str++;
		}
		return r;
	}else
		return	s2i(str);
}


string Type2Str(CPreScript *s, sType *type)
{
	string str;
	if (type)
		str = format("UNKNOWN TYPE (%s)", type->Name);
	else
		str = "UNKNOWN TYPE (-nil-)";
	for (int i=0;i<s->Type.num;i++)
		if (type == s->Type[i])
			str = s->Type[i]->Name;
	if (type == TypeUnknown)
		str = "[deliberately unknown type]";
	return str;
}

string Kind2Str(int kind)
{
	if (kind == KindVarLocal)			return "local variable";
	if (kind == KindVarGlobal)			return "global variable";
	if (kind == KindVarFunction)		return "function as variable";
	if (kind == KindVarExternal)		return "external program-variable";
	if (kind == KindConstant)			return "constant";
	if (kind == KindRefToConst)			return "reference to const";
	if (kind == KindFunction)			return "function";
	if (kind == KindCompilerFunction)	return "compiler function";
	if (kind == KindOperator)			return "operator";
	if (kind == KindPrimitiveOperator)	return "PRIMITIVE operator";
	if (kind == KindBlock)				return "command block";
	if (kind == KindAddressShift)		return "address shift";
	if (kind == KindArray)				return "array element";
	if (kind == KindPointerAsArray)		return "pointer as array element";
	if (kind == KindReference)			return "address operator";
	if (kind == KindDereference)		return "dereferencing";
	if (kind == KindDerefAddressShift)	return "deref address shift";
	if (kind == KindType)				return "type";
	if (kind == KindVarTemp)			return "temp";
	if (kind == KindDerefVarTemp)		return "deref temp";
	if (kind == KindRegister)			return "register";
	if (kind == KindAddress)			return "address";
	if (kind == KindMemory)				return "memory";
	if (kind == KindLocalAddress)		return "local address";
	if (kind == KindLocalMemory)		return "local memory";
	if (kind == KindDerefRegister)		return "deref register";
	if (kind == KindMarker)				return "marker";
	if (kind == KindAsmBlock)			return "assembler block";
	if (kind == KindRefToLocal)			return "ref to local";
	if (kind == KindRefToGlobal)		return "ref to global";
	if (kind == KindRefToConst)			return "ref to const";
	if (kind == KindDerefVarLocal)		return "deref local";
	return format("UNKNOWN KIND: %d", kind);
}

string Operator2Str(CPreScript *s,int cmd)
{
	//strcpy(str,string("UNBEKANNTER OPERATOR: ",i2s(cmd)));
	return "(" + Type2Str(s,PreOperator[cmd].ParamType1) + format(") %s (", PrimitiveOperator[PreOperator[cmd].PrimitiveID].Name)
		+ Type2Str(s,PreOperator[cmd].ParamType2) + ")";
}

string PrimitiveOperator2Str(int cmd)
{
	//strcpy(str,string("UNBEKANNTER PRIMITIVER OPERATOR: ",i2s(cmd)));
	return PrimitiveOperator[cmd].Name;
}

string LinkNr2Str(CPreScript *s,int kind,int nr)
{
	if (kind==KindVarLocal)			return i2s(nr);
	if (kind==KindVarGlobal)		return i2s(nr);
	if (kind==KindVarFunction)		return i2s(nr);
	if (kind==KindVarExternal)		return PreExternalVar[nr].Name;
	if (kind==KindConstant)			return i2s(nr);
	if (kind==KindFunction)			return s->Function[nr].Name;
	if (kind==KindCompilerFunction)	return PreCommand[nr].Name;
	if (kind==KindOperator)			return Operator2Str(s,nr);
	if (kind==KindPrimitiveOperator)return PrimitiveOperator2Str(nr);
	if (kind==KindBlock)			return i2s(nr);
	if (kind==KindAddressShift)		return i2s(nr);
	if (kind==KindArray)			return "(no LinkNr)";
	if (kind==KindPointerAsArray)	return "(no LinkNr)";
	if (kind==KindReference)		return "(no LinkNr)";
	if (kind==KindDereference)		return "(no LinkNr)";
	if (kind==KindDerefAddressShift)return i2s(nr);
	if (kind==KindType)				return s->Type[nr]->Name;
	if (kind==KindAddress)			return d2h(&nr, 4);
	if (kind==KindMemory)			return d2h(&nr, 4);
	if (kind==KindLocalAddress)		return d2h(&nr, 4);
	if (kind==KindLocalMemory)		return d2h(&nr, 4);
	return "UNUSABLE KIND";
}

void CPreScript::DoError(const string &str, int overwrite_line)
{
	if (Error)
		return;
	stringout("\n\n\n");
	stringout("------------------------       Error       -----------------------");	
	Error = true;

	// what data do we have?
	int line = -1;
	int pos = 0;
	bool exp_known = false;
	if (Exp.cur_line){
		line = Exp.cur_line->physical_line;
		if (Exp.cur_exp >= 0){
			exp_known = true;
			pos = Exp.cur_line->exp[Exp.cur_exp].pos;
		}
	}
	if (overwrite_line >= 0){
		line = overwrite_line;
		pos = 0;
	}

	if (exp_known){
		// full data
		ErrorMsg = format("\"%s\" %s", cur_name, str.c_str());
		ErrorMsgExt[0] = str;
		ErrorMsgExt[1] = format("\"%s\" , line %d:%d", cur_name, line + 1, pos + 1);
		ErrorLine = line;
		ErrorColumn = pos;
	}else{
		ErrorMsg = str;
		ErrorMsgExt[0] = str;
		if (line >= 0)
			ErrorMsgExt[1] = format("line %d", line + 1);
		else
			ErrorMsgExt[1] = "";
		ErrorLine = (line >= 0) ? line : 0;
		ErrorColumn = 0;
	}
	stringout(str);
	stringout(ErrorMsgExt[1]);
	stringout("------------------------------------------------------------------");
	stringout(Filename);
	stringout("\n\n\n");
}


bool IsIfDefed(int &num_ifdefs,bool *defed)
{
	for (int i=0;i<num_ifdefs;i++)
		if (!defed[i])
			return false;
	return true;
}

CScript *cur_script;
void CreateAsmMetaInfo(CPreScript* ps)
{
	msg_db_r("CreateAsmMetaInfo",5);
	//msg_error("zu coden: CreateAsmMetaInfo");
	sAsmMetaInfo *m=(sAsmMetaInfo*)ps->AsmMetaInfo;
	if (!m){
		m=new sAsmMetaInfo;
		ps->AsmMetaInfo = (char*)m;
		m->Mode16 = ps->FlagCompileInitialRealMode;
		m->CodeOrigin=0; // FIXME:  &Opcode[0] ????
	}
	m->Opcode=cur_script->Opcode;
	m->GlobalVar.clear();
	for (int i=0;i<ps->RootOfAllEvil.Var.num;i++){
		sAsmGlobalVar v;
		v.Name = ps->RootOfAllEvil.Var[i].Name;
		v.Pos = cur_script->g_var[i];
		m->GlobalVar.add(v);
	}
	msg_db_l(5);
}


#if 0
void CPreScript::MakeExps(char *Buffer,bool just_ana				)
{
	msg_db_r("MakeExps",4);
	int i,NumIfDefs=0,ln;
	bool IfDefed[1024];
	Exp=new exp_buffer;
	Exp->BufferUsed=0;
	BufferPos=0;
	Exp->TempLine=1;
	Exp->TempColumn=0;
	Exp->NumExps=0;
	char filename[256];
	CScript *include;
	
	while(true){
		int l=(Exp->NumExps==0)?0:Exp->TempLine;
		NextExp(Buffer);
		if (Error)	return;
		if (Temp[0]==0)
			break;

		if ((Exp->TempLine>l)&&(strcmp(Temp,"#")==0)){
			msg_db_m("makro",4);
			l=Exp->TempLine;
			so("# -Makro");
			NextExp(Buffer);

			int macro_no=-1;
			for (i=0;i<NumMacroNames;i++)
				if (strcmp(Temp,MacroName[i])==0)
					macro_no=i;

			switch(macro_no){
				case MacroInclude:
					NextExp(Buffer);
					if (!IsIfDefed(NumIfDefs,IfDefed))
						continue;
					strcpy(filename,dir_from_filename(Filename));
					strcat(filename,&Temp[1]);
					filename[strlen(filename)-1]=0; // remove "
					strcpy(filename,filename_no_recursion(filename));

					so("lade Include-Datei");
					right();

					include=LoadScriptAsInclude(filename,just_analyse);

					left();
					if ((!include)||(include->Error)){
						IncludeLinkerError|=include->LinkerError;
						DoError(string2("error in inluded file \"%s\":\n[ %s (line %d:) ]",filename,include->ErrorMsg,include->ErrorLine,include->ErrorColumn),Exp->ExpNr);
						return;
					}
					AddIncludeData(include);
					Exp->ExpNr++;
					break;
				case MacroDefine:
					Define[NumDefines]=new sDefine;
					Define[NumDefines]->Owner=this;
					// Source
					NextExp(Buffer);
					strcpy(Define[NumDefines]->Source,Temp);
					Define[NumDefines]->NumDests=0;
					// Dests
					int t;
					for (i=0;i<SCRIPT_MAX_DEFINE_DESTS;i++){
						t=BufferPos;
						NextExp(Buffer);
						if (Exp->TempLine>l){
							BufferPos=t;
							break;
						}
						strcpy(Define[NumDefines]->Dest[Define[NumDefines]->NumDests],Temp);
						Define[NumDefines]->NumDests++;
					}
					Exp->TempLine=l;
					NumDefines++;
					break;
				case MacroIfdef:
					NextExp(Buffer);
					IfDefed[NumIfDefs]=false;
					for (i=0;i<NumDefines;i++)
						if (strcmp(Temp,Define[i]->Source)==0){
							IfDefed[NumIfDefs]=true;
							break;
						}
					NumIfDefs++;
					break;
				case MacroIfndef:
					NextExp(Buffer);
					IfDefed[NumIfDefs]=true;
					for (i=0;i<NumDefines;i++)
						if (strcmp(Temp,Define[i]->Source)==0){
							IfDefed[NumIfDefs]=false;
							break;
						}
					NumIfDefs++;
					break;
				case MacroElse:
					if (NumIfDefs<1){
						strcpy(Exp->Name[Exp->NumExps],Temp);
						Exp->Line[Exp->NumExps]=Exp->TempLine;
						Exp->Column[Exp->NumExps]=Exp->TempColumn;
						DoError("\"#else\" found but no matching \"#ifdef\"",Exp->NumExps);
						return;
					}
					IfDefed[NumIfDefs-1]=!IfDefed[NumIfDefs-1];
					break;
				case MacroEndif:
					if (NumIfDefs<1){
						strcpy(Exp->Name[Exp->NumExps],Temp);
						Exp->Line[Exp->NumExps]=Exp->TempLine;
						Exp->Column[Exp->NumExps]=Exp->TempColumn;
						DoError("\"#endif\" found but no matching \"#ifdef\"",Exp->NumExps);
						return;
					}
					NumIfDefs--;
					break;
				case MacroRule:
					NextExp(Buffer);
					ln=-1;
					for (i=0;i<NumScriptLocations;i++)
						if (strcmp(ScriptLocation[i].Name,Temp)==0)
							ln=i;
					if (ln<0){
						strcpy(Exp->Name[Exp->NumExps],Temp);
						Exp->Line[Exp->NumExps]=Exp->TempLine;
						Exp->Column[Exp->NumExps]=Exp->TempColumn;
						DoError("unknown location in script rule",Exp->NumExps);
						return;
					}
					PreScriptRule[NumPreScriptRules]=new sPreScriptRule;
					PreScriptRule[NumPreScriptRules]->Location=ScriptLocation[ln].Location;
					NextExp(Buffer);
					PreScriptRule[NumPreScriptRules]->Level=s2i(Temp);
					NextExp(Buffer);
					Temp[strlen(Temp)-1]=0;
					strcpy(PreScriptRule[NumPreScriptRules]->Name,&Temp[1]);
					NumPreScriptRules++;
					break;
				case MacroDisasm:
					FlagDisassemble=true;
					break;
				case MacroShow:
					FlagShow=true;
					break;
				case MacroImmortal:
					FlagImmortal=true;
					break;
				case MacroOs:
					FlagCompileOS=true;
					break;
				case MacroInitialRealmode:
					FlagCompileInitialRealMode=true;
					break;
				case MacroVariablesOffset:
					FlagOverwriteVariablesOffset=true;
					NextExp(Buffer);
					VariablesOffset=s2i2(Temp);
					break;
				case MacroCodeOrigin:
					NextExp(Buffer);
					CreateAsmMetaInfo(this);
					((sAsmMetaInfo*)AsmMetaInfo)->CodeOrigin=s2i2(Temp);
					break;
				default:
					strcpy(Exp->Name[Exp->NumExps],Temp);
					Exp->Line[Exp->NumExps]=Exp->TempLine;
					Exp->Column[Exp->NumExps]=Exp->TempColumn;
					DoError("unknown makro atfer \"#\"",Exp->NumExps);
					return;
			}
			continue;
		}
	//msg_db_m("def",4);

		bool defed=false;
		for (i=0;i<NumDefines;i++)
			if (strcmp(Temp,Define[i]->Source)==0){
				defed=true;
				for (int j=0;j<Define[i]->NumDests;j++){
					strcpy(Exp->Name[Exp->NumExps],Define[i]->Dest[j]);
					Exp->BufferUsed+=strlen(Define[i]->Dest[j])+1;
					Exp->Name[Exp->NumExps+1]=&Exp->Buffer[Exp->BufferUsed];
					Exp->Line[Exp->NumExps]=Exp->TempLine;
					Exp->Column[Exp->NumExps]=Exp->TempColumn;
					Exp->NumExps++;
				}
				break;
			}
			
	//msg_db_m("postdef",4);
		if (defed)
			continue;

		if (!IsIfDefed(NumIfDefs,IfDefed))
			continue;
	//msg_db_m("zzz",4);

		strcpy(Exp->Name[Exp->NumExps],Temp);
		Exp->Line[Exp->NumExps]=Exp->TempLine;
		Exp->Column[Exp->NumExps]=Exp->TempColumn;
		Exp->NumExps++;
	}
	if (NumIfDefs>0){
		DoError("\"#ifdef\" found but no matching \"#endif\"",Exp->NumExps);
		return;
	}
	Exp->ExpNr=0;
	msg_db_l(4);
}
#endif

bool next_extern = false;
bool next_const = false;

int CPreScript::AddVar(const char *name, sType *type, sFunction *f)
{
/*	// "extern" variable -> link to main program
	if ((next_extern) && (f == &RootOfAllEvil)){
		for (int i=NumTruePreExternalVars;i<PreGlobalVar.num;i++)
			if (strcmp(name, PreGlobalVar[i].Name) == 0){
				f->Var[i].Type = type;
				return i;
			}
	}
should be done somwhere else (ParseVariableDefSingle) */
	so("                                AddVar");
	sLocalVariable v;
	strcpy(v.Name, name);
	v.Type = type;
	f->Var.add(v);
	return f->Var.num - 1;
}

// constants

int CPreScript::AddConstant(sType *type)
{
	so("                                AddConstant");
	Constant.resize(Constant.num + 1);
	sConstant *c = &Constant.back();
	strcpy(c->name, "-none-");
	c->type = type;
	int s = (type->Size > (int)PointerSize) ? type->Size : PointerSize;
	if (type == TypeString)
		s = 256;
	c->data = new char[s];
	return Constant.num - 1;
}

sBlock *CPreScript::AddBlock()
{
	so("AddBlock");
	sBlock *b = new sBlock;
	b->Index = Block.num;
	b->Root = -1;
	Block.add(b);
	return b;
}

// functions

int CPreScript::AddFunction(const char *name, sType *type)
{
	so("AddFunction");
	Function.resize(Function.num + 1);
	sFunction *f = &Function.back();
	strcpy(f->Name, name);
	f->Block = AddBlock();
	if (Error)
		return -1;
	f->NumParams = 0;
	f->Var.clear();
	f->Type = type;
	f->LiteralType = type;
	return Function.num - 1;
}
sCommand *CPreScript::AddCommand()
{
	so("AddCommand");
	sCommand *c = new sCommand;
	Command.add(c);
	c->Type = TypeVoid;
	c->Kind = KindUnknown;
	c->NumParams = 0;
	c->Instance = NULL;
	return c;
}


inline sCommand *add_command_compilerfunc(CPreScript *ps, int cf)
{
	sCommand *c = ps->AddCommand();
	ps->CommandSetCompilerFunction(cf, c);
	return c;
}

inline void CommandSetConst(CPreScript *ps, sCommand *c, int nc)
{
	c->Kind = KindConstant;
	c->LinkNr = nc;
	c->Type = ps->Constant[nc].type;
	c->NumParams = 0;
}

inline sCommand *add_command_const(CPreScript *ps, int nc)
{
	sCommand *c = ps->AddCommand();
	CommandSetConst(ps, c, nc);
	return c;
}

int CPreScript::WhichPrimitiveOperator(const char *name)
{
	for (int i=0;i<NumPrimitiveOperators;i++)
		if (strcmp(name, PrimitiveOperator[i].Name) == 0)
			return i;
	return -1;
}

int CPreScript::WhichExternalVariable(const char *name)
{
	// wrong order -> "extern" varbiables are dominant...
	for (int i=PreExternalVar.num-1;i>=0;i--)
		if (strcmp(name, PreExternalVar[i].Name) == 0)
			return i;

	return -1;
}

int CPreScript::WhichType(const char *name)
{
	for (int i=0;i<Type.num;i++)
		if (strcmp(name, Type[i]->Name) == 0)
			return i;

	return -1;
}

Array<int> MultipleFunctionList;

int CPreScript::WhichCompilerFunction(const char *name)
{
	MultipleFunctionList.clear();
	for (int i=0;i<PreCommand.num;i++)
		if (strcmp(name, PreCommand[i].Name) == 0)
			MultipleFunctionList.add(i);
			//return i;
	if (MultipleFunctionList.num > 0)
		return MultipleFunctionList[0];
	return -1;
}

inline void exlink_make_var_local(CPreScript *ps, sType *t, int var_no)
{
	ps->GetExistenceLink.Type = t;
	ps->GetExistenceLink.LinkNr = var_no;
	ps->GetExistenceLink.Kind = KindVarLocal;
	ps->GetExistenceLink.NumParams = 0;
	ps->GetExistenceLink.script = NULL;
	ps->GetExistenceLink.Instance = NULL;
}

bool CPreScript::GetExistence(const char *name, sFunction *f)
{
	msg_db_r("GetExistence", 3);
	MultipleFunctionList.clear();
	sFunction *lf=f;
	GetExistenceLink.Type = TypeUnknown;
	GetExistenceLink.NumParams = 0;
	GetExistenceLink.script = NULL;
	GetExistenceLink.Instance = NULL;

	// first test local variables
	if (lf){
		foreachi(f->Var, v, i){
			if (strcmp(v.Name, name) == 0){
				exlink_make_var_local(this, v.Type, i);
				msg_db_l(3);
				return true;
			}
		}
	}

	// then global variables (=local variables in "RootOfAllEvil")
	lf = &RootOfAllEvil;
	for (int i=0;i<lf->Var.num;i++)
		if (strcmp(lf->Var[i].Name, name)==0){
			GetExistenceLink.Type = lf->Var[i].Type;
			GetExistenceLink.LinkNr = i;
			GetExistenceLink.Kind = KindVarGlobal;
			msg_db_l(3);
			return true;
		}

	// at last the external variables
	int w = WhichExternalVariable(name);
	if (w >= 0){
		SetExternalVariable(w, &GetExistenceLink);
		msg_db_l(3);
		return true;
	}

	// then the (self-coded) functions
	for (int i=0;i<Function.num;i++)
		if (strcmp(Function[i].Name, name) == 0){
			GetExistenceLink.Kind = KindFunction;
			GetExistenceLink.LinkNr = i;
			GetExistenceLink.Type = Function[i].LiteralType;
			GetExistenceLink.NumParams = Function[i].NumParams;
			msg_db_l(3);
			return true;
		}

	// then the compiler functions
	w = WhichCompilerFunction(name);
	if (w >= 0){
		GetExistenceLink.Kind = KindCompilerFunction;
		GetExistenceLink.LinkNr = w;
		GetExistenceLink.Type = PreCommand[w].ReturnType;
		GetExistenceLink.NumParams = PreCommand[w].Param.num;
		msg_db_l(3);
		return true;
	}

	// operators
	w = WhichPrimitiveOperator(name);
	if (w >= 0){
		GetExistenceLink.Kind = KindPrimitiveOperator;
		GetExistenceLink.LinkNr = w;
		msg_db_l(3);
		return true;
	}

	// types
	w = WhichType(name);
	if (w >= 0){
		GetExistenceLink.Kind = KindType;
		GetExistenceLink.LinkNr = w;
		msg_db_l(3);
		return true;
	}

	// in include files (only global)...
	for (int i=0;i<Include.num;i++){
		if (Include[i]->pre_script->GetExistence(name, NULL)){
			if (Include[i]->pre_script->GetExistenceLink.script) // nicht rekursiv!!!
				continue;
			memcpy(&GetExistenceLink, &(Include[i]->pre_script->GetExistenceLink), sizeof(sCommand));
			GetExistenceLink.script = Include[i];
			//msg_error(string2("\"%s\" in Include gefunden!  %s", name, GetExistenceLink.Type->Name));
			msg_db_l(3);
			return true;
		}
	}

	// ...unknown
	GetExistenceLink.Type = TypeUnknown;
	GetExistenceLink.Kind = KindUnknown;
	GetExistenceLink.LinkNr = 0;
	msg_db_l(3);
	return false;
}

void CPreScript::CommandSetCompilerFunction(int CF, sCommand *Com)
{
	msg_db_r("CommandSetCompilerFunction", 4);
	if (FlagCompileOS)
		if (!PreCommand[CF].IsSpecial){
			DoError(format("external function call (%s) not allowed with #os", PreCommand[CF].Name));
			return;
		}
	
// a function the compiler knows
	Com->Kind = KindCompilerFunction;
	Com->LinkNr = CF;

	Com->NumParams = PreCommand[CF].Param.num;
	for (int p=0;p<Com->NumParams;p++){
		Com->Param[p] = AddCommand(); // temporary...
		Com->Param[p]->Type = PreCommand[CF].Param[p].Type;
	}
	Com->Type = PreCommand[CF].ReturnType;
			
	msg_db_l(4);
}

#define is_variable(kind)	(((kind) == KindVarLocal) || ((kind) == KindVarGlobal) || ((kind) == KindVarExternal))

void CPreScript::SetExternalVariable(int gv, sCommand *c)
{
	c->NumParams = 0;
	c->Kind = KindVarExternal;
	c->LinkNr = gv;
	c->Type = PreExternalVar[gv].Type;
}

// find the type of a (potential) constant
//  "1.2" -> float
sType *CPreScript::GetConstantType()
{
	msg_db_r("GetConstantType", 4);
	PreConstantNr = -1;
	NamedConstantNr = -1;

	// predefined constants
	foreachi(PreConstant, c, i)
		if (strcmp(cur_name, c.Name) == 0){
			PreConstantNr = i;
			_return_(4, c.Type);
		}

	// named constants
	foreachi(Constant, c, i)
		if (strcmp(cur_name, c.name) == 0){
			NamedConstantNr = i;
			_return_(4, c.type);
		}

	// character "..."
	if ((cur_name[0] == '\'') && (cur_name[strlen(cur_name)-1] == '\''))
		_return_(4, TypeChar);

	// string "..."
	if ((cur_name[0] == '"') && (cur_name[strlen(cur_name)-1] == '"'))
		_return_(4, (FlagCompileOS ? TypeCString : TypeString));

	// numerical (int/float)
	sType *type = TypeInt;
	bool hex = (cur_name[0] == '0') && (cur_name[1] == 'x');
	for (unsigned int c=0;c<strlen(cur_name);c++)
		if ((cur_name[c] < '0') || (cur_name[c] > '9')){
			if (hex){
				if ((c >= 2) && (cur_name[c] < 'a') && (cur_name[c] > 'f'))
					_return_(4, TypeUnknown);
			}else if (cur_name[c] == '.'){
				type = TypeFloat;
			}else{
				//if ((type != TypeFloat) || (cur_name[c] != 'f')) // f in floats erlauben
					if ((c != 0) || (cur_name[c] != '-')) // Vorzeichen erlauben
						_return_(4, TypeUnknown);
			}
		}

	// super array [...]
	if (strcmp(cur_name, "[") == 0){
		//msg_error("super array constant");
		DoError("super array constant");
		_return_(4, TypeUnknown);
	}
	_return_(4, type);
}

static int _some_int_;
static float _some_float_;
static char _some_string_[2048];

bool pre_const_is_ref(sType *type)
{
	return ((type->Size > 4) && (!type->IsPointer));
}

void *CPreScript::GetConstantValue()
{
	sType *type = GetConstantType();
// named constants
	if (PreConstantNr >= 0){
		if (pre_const_is_ref(type))
			return PreConstant[PreConstantNr].Value;
		else
			return &PreConstant[PreConstantNr].Value;
	}
	if (NamedConstantNr >= 0)
		return Constant[NamedConstantNr].data;
// literal
	if (type == TypeChar){
		_some_int_ = cur_name[1];
		return &_some_int_;
	}
	if ((type == TypeString) || (type == TypeCString)){
		for (unsigned int ui=0;ui<strlen(cur_name) - 2;ui++)
			_some_string_[ui] = cur_name[ui+1];
		_some_string_[strlen(cur_name) - 2] = 0;
		return _some_string_;
	}
	if (type == TypeInt){
		_some_int_ = s2i2(cur_name);
		return &_some_int_;
	}
	if (type == TypeFloat){
		_some_float_ = s2f(cur_name);
		return &_some_float_;
	}
	return NULL;
}

// expression naming a type
sType *CPreScript::GetType(int &ie,bool force)
{
	sType *type=NULL;
	for (int i=0;i<Type.num;i++)
		if (strcmp(get_name(ie), Type[i]->Name)==0)
			type=Type[i];
	if (force){
		if (!type){
			Exp.cur_exp = ie;
			DoError("unknown type");
		}
	}
	if (type)
		ie++;
	return type;
}

// create a new type?
void CPreScript::AddType(sType **type)
{
	for (int i=0;i<Type.num;i++)
		if (strcmp((*type)->Name,Type[i]->Name)==0){
			(*type)=Type[i];
			return;
		}
	sType *t = new sType;
	(*t) = (**type);
	t->Owner = this;
	strcpy(t->Name, (*type)->Name);
	so(format("AddType: %s",t->Name));
	(*type) = t;
	Type.add(t);
	NumOwnTypes ++;

	if (t->IsSuperArray)
		script_make_super_array(t, this);
}

sType *CPreScript::CreateNewType(const char *name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, sType *sub)
{
	sType nt, *pt = &nt;
	nt.IsArray = is_array && (array_size >= 0);
	nt.IsSuperArray = is_array && (array_size < 0);
	nt.ArrayLength = max(array_size, 0);
	nt.IsPointer = is_pointer;
	nt.IsSilent = is_silent;
	strcpy(nt.Name, name);
	nt.Size = size;
	nt.SubType = sub;
	AddType(&pt);
	return pt;
}


void DoClassFunction(CPreScript *ps, sCommand *Operand, sType *t, int f_no, sFunction *f)
{
	msg_db_r("DoClassFunc", 1);
#if 0
	switch(Operand->Kind){
		case KindVarLocal:
		case KindVarGlobal:
		case KindVarExternal:
		case KindConstant:
		case KindPointerShift:
		case KindArray:
		case KindDerefPointerShift:
		/*case KindRefToLocal:
		case KindRefToGlobal:
		case KindRefToConst:*/
			break;
		default:
			ps->DoError(string("class functions only allowed for object variables, not for: ", Kind2Str(Operand->Kind)));
			_return_(1,);
	}
#endif

	// create a command for the object
	sCommand *ob = cp_command(ps, Operand);

	//msg_write(LinkNr2Str(ps, Operand->Kind, Operand->Nr));

	// the function
	Operand->script = NULL;
    Operand->Kind = t->Function[f_no].Kind;
	Operand->LinkNr = t->Function[f_no].Nr;
	if (t->Function[f_no].Kind == KindCompilerFunction){
		Operand->Type = PreCommand[t->Function[f_no].Nr].ReturnType;
		Operand->NumParams = PreCommand[t->Function[f_no].Nr].Param.num;
		ps->GetFunctionCall(PreCommand[t->Function[f_no].Nr].Name, Operand, f);
	}else if (t->Function[f_no].Kind == KindFunction){
		Operand->Type = ps->Function[t->Function[f_no].Nr].LiteralType;
		Operand->NumParams = ps->Function[t->Function[f_no].Nr].NumParams;
		ps->GetFunctionCall(ps->Function[t->Function[f_no].Nr].Name, Operand, f);
		//ps->DoError("script member function call not implemented");
	}
	Operand->Instance = ob;

	
	msg_db_l(1);
}

// find any ".", "->", or "[...]"'s    or operators?
void CPreScript::GetOperandExtension(sCommand *Operand, sFunction *f)
{
	msg_db_r("GetOperandExtension", 4);

	// nothing?
	int op = WhichPrimitiveOperator(cur_name);
	if ((strcmp(cur_name, ".") != 0) && (strcmp(cur_name, "[") !=0 ) && (strcmp(cur_name, "->") != 0) && (op < 0)){
		msg_db_l(4);
		return;
	}
	//sLinkData link, temp;

	// class element?
	if ((strcmp(cur_name, ".") == 0) || (strcmp(cur_name, "->") == 0)){
		so("->Klasse");
		next_exp();
		sType *type = Operand->Type;

		// pointer -> dereference
		bool deref = false;
		if (type->IsPointer){
			type = type->SubType;
			deref = true;
		}

		if (strcmp(get_name(Exp.cur_exp-1), "->") == 0){
			DoError("\"->\" deprecated,  use \".\" instead");
			msg_db_l(4);
			return;
		}

		// find element
		bool ok = false;
		for (int e=0;e<type->Element.num;e++)
			if (strcmp(cur_name, type->Element[e].Name) == 0){
				sCommand *t = cp_command(this, Operand);
				Operand->Kind = deref ? KindDerefAddressShift : KindAddressShift;
				Operand->LinkNr = type->Element[e].Offset;
				Operand->Type = type->Element[e].Type;
				Operand->NumParams = 1;
				Operand->Param[0] = t;
				ok = true;
				break;
			}
		
		if (!ok){

			// class function?
			for (int e=0;e<type->Function.num;e++)
				if (strcmp(cur_name, type->Function[e].Name) == 0){
					if (!deref){
						so("ref object");
						ref_command(this, Operand);
					}
					next_exp();
					DoClassFunction(this, Operand, type, e, f);
					//DoError(string("class functions not implemented yet  ...",Type2Str(this,type)));
					msg_db_l(4);
					return;
				}
			
			DoError("unknown element of " + Type2Str(this,type));
			msg_db_l(4);
			return;
		}

		next_exp();

	// array?
	}else if (strcmp(cur_name, "[") == 0){
		so("->Array");

		// allowed?
		bool allowed = ((Operand->Type->IsArray) || (Operand->Type->IsSuperArray));
		bool pparray = false;
		if (!allowed)
			if (Operand->Type->IsPointer){
				if ((Operand->Type->SubType->IsArray) || (Operand->Type->SubType->IsSuperArray)){
					allowed = true;
					pparray = (Operand->Type->SubType->IsSuperArray);
				}else{
					DoError(format("using pointer type \"%s\" as an array (like in C) is not allowed any more", Operand->Type->Name));
					msg_db_l(4);
					return;
				}
			}
		if (!allowed){
			DoError(format("type \"%s\" is neither an array nor a pointer to an array", Operand->Type->Name));
			msg_db_l(4);
			return;
		}
		next_exp();

		sCommand *t = cp_command(this, Operand);
		Operand->NumParams = 2;
		Operand->Param[0] = t;
		sCommand *array = Operand;

		// pointer?
		so(Operand->Type->Name);
		if (pparray){
			so("  ->Pointer-Pointer-Array");
			//array = cp_command(this, Operand);
			Operand->Kind = KindPointerAsArray;
			Operand->Type = t->Type->SubType;
			deref_command(this, Operand);
			array = Operand->Param[0];
		}else if ((Operand->Type->IsPointer) || (Operand->Type->IsSuperArray)){
			Operand->Kind = KindPointerAsArray;
			if (Operand->Type->IsPointer)
				Operand->Type = t->Type->SubType->SubType;
			else
				Operand->Type = t->Type->SubType;
			so("  ->Pointer-Array");
		}else{
			Operand->Kind = KindArray;
			Operand->Type = t->Type->SubType;
		}

		// array index...
		sCommand *index = GetCommand(f);
		if (Error){
			msg_db_l(4);
			return;
		}
		array->Param[1] = index;
		if (index->Type != TypeInt){
			Exp.cur_exp --;
			DoError(format("type of index for an array needs to be (int), not (%s)", index->Type->Name));
			msg_db_l(4);
			return;
		}
		if (strcmp(cur_name, "]") != 0){
			DoError("\"]\" expected after array index");
			msg_db_l(4);
			return;
		}
		next_exp();

	// unary operator?
	}else if (op >= 0){
		for (int i=0;i<PreOperator.num;i++)
			if (PreOperator[i].PrimitiveID == op)
				if ((PreOperator[i].ParamType1 == Operand->Type) && (PreOperator[i].ParamType2 == TypeVoid)){
					so("  => unaerer Operator");
					so(LinkNr2Str(this,KindOperator,i));
					sCommand *t = cp_command(this, Operand);
					CommandMakeOperator(Operand, t, NULL, i);
					next_exp();
					msg_db_l(4);
					return;
				}
		msg_db_l(4);
		return;
	}

	// recursion
	GetOperandExtension(Operand, f);
	msg_db_l(4);
}

inline bool direct_type_match(sType *a, sType *b)
{
	return ( (a==b) || ( (a->IsPointer) && (b->IsPointer) ) );
}

bool CPreScript::GetSpecialFunctionCall(const char *f_name, sCommand *Operand, sFunction *f)
{
	msg_db_r("GetSpecialFuncCall", 4);

	// sizeof
	if ((Operand->Kind == KindCompilerFunction) && (Operand->LinkNr == CommandSizeof)){

		so("sizeof");
		next_exp();
		int nc = AddConstant(TypeInt);
		CommandSetConst(this, Operand, nc);
		
		int nt = WhichType(cur_name);
		sType *type;
		if (nt >= 0)
			(*(int*)(Constant[nc].data)) = Type[nt]->Size;
		else if ((GetExistence(cur_name, f)) && ((GetExistenceLink.Kind == KindVarGlobal) || (GetExistenceLink.Kind == KindVarLocal) || (GetExistenceLink.Kind == KindVarExternal)))
			(*(int*)(Constant[nc].data)) = GetExistenceLink.Type->Size;
		else if (type == GetConstantType())
			(*(int*)(Constant[nc].data)) = type->Size;
		else{
			DoError("type-name or variable name expected in sizeof(...)");
			msg_db_l(4);
			return false;
		}
		next_exp();
		if (strcmp(cur_name, ")") != 0){
			DoError("\")\" expected after parameter list");
			msg_db_l(4);
			return false;
		}
		next_exp();
		
		so(*(int*)(Constant[nc].data));
		msg_db_l(4);
		return true;
	}

	// sizeof
	if ((Operand->Kind == KindCompilerFunction) && (Operand->LinkNr == CommandReturn)){
		DoError("return");
	}
	
	msg_db_l(4);
	return false;
}


// cmd needs to have Param[]'s existing with correct Type!
void CPreScript::FindFunctionSingleParameter(int p, sType **WantedType, sFunction *f, sCommand *cmd)
{
	msg_db_r("FindFuncSingleParam", 4);
	sCommand *Param = GetCommand(f);
	if (Error)
		_return_(4,);

	WantedType[p] = TypeUnknown;
	if (cmd->Kind == KindFunction){
		if (cmd->script)
			WantedType[p] = cmd->script->pre_script->Function[cmd->LinkNr].LiteralParamType[p];
		else
			WantedType[p] = Function[cmd->LinkNr].LiteralParamType[p];
	}
	if (cmd->Kind == KindCompilerFunction)
		WantedType[p] = PreCommand[cmd->LinkNr].Param[p].Type;
	// link parameters
	cmd->Param[p] = Param;
	msg_db_l(4);
}

void CPreScript::FindFunctionParameters(int &np, sType **WantedType, sFunction *f, sCommand *cmd)
{
	if (strcmp(cur_name, "(") != 0){
		DoError("\"(\" expected in front of function parameter list");
		return;
	}
	msg_db_r("FindFunctionParameters", 4);
	next_exp();
		    
	// list of parameters
	np = 0;
	for (int p=0;p<SCRIPT_MAX_PARAMS;p++){
		if (strcmp(cur_name, ")") == 0)
			break;
		np ++;
		// find parameter

		FindFunctionSingleParameter(p, WantedType, f, cmd);
		if (Error)
			_return_(4,);

		if (strcmp(cur_name, ",") != 0){
			if (strcmp(cur_name, ")") == 0)
				break;
			DoError("\",\" or \")\" expected after parameter for function");
			_return_(4,);
		}
		next_exp();
	}
	next_exp(); // ')'
	msg_db_l(4);
}

void apply_type_cast(CPreScript *ps, int tc, sCommand *param);


// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
void CPreScript::CheckParamLink(sCommand *link, sType *type, const char *f_name, int param_no)
{
	msg_db_r("CheckParamLink", 4);
	// type cast needed and possible?
	sType *pt = link->Type;
	sType *wt = type;

	// "silent" pointer (&)?
	if ((wt->IsPointer) && (wt->IsSilent)){
		if (direct_type_match(pt, wt->SubType)){
			so("<silent Ref &>");

			ref_command(this, link);
		}else if ((pt->IsPointer) && (direct_type_match(pt->SubType, wt->SubType))){
			so("<silent Ref & of *>");

			// no need to do anything...
		}else{
			Exp.cur_exp --;
			DoError(format("(c) parameter %d for function \"%s\" has type (%s), (%s) expected", param_no + 1, f_name, pt->Name, wt->Name));
			_return_(4,);
		}

	// normal type cast
	}else if (!direct_type_match(pt, wt)){
		int tc = -1;
		for (int i=0;i<TypeCast.num;i++)
			if ((direct_type_match(TypeCast[i].Source, pt)) && (direct_type_match(TypeCast[i].Dest, wt)))
				tc = i;

		if (tc >= 0){
			so("TypeCast");
			apply_type_cast(this, tc, link);
		}else{
			DoError(format("(a) parameter %d for function \"%s\" has type (%s), (%s) expected", param_no + 1, f_name, pt->Name, wt->Name));
			_return_(4,);
		}
	}
	msg_db_l(4);
}

// creates <Operand> to be the function call
//  on entry <Operand> only contains information from GetExistence (Kind, Nr, Type, NumParams)
void CPreScript::GetFunctionCall(const char *f_name, sCommand *Operand, sFunction *f)
{
	msg_db_r("GetFunctionCall", 4);
	
	// function as a variable?
	if (Exp.cur_exp >= 2)
	if ((strcmp(get_name(Exp.cur_exp - 2), "&") == 0) && (strcmp(cur_name, "(") != 0)){
		if (Operand->Kind == KindFunction){
			so("Funktion als Variable!");
			Operand->Kind = KindVarFunction;
			Operand->Type = TypePointer;
			Operand->NumParams = 0;
		}else{
			Exp.cur_exp --;
			//DoError("\"(\" expected in front of parameter list");
			DoError("only script functions can be referenced");
		}
		_return_(4,);
	}

	
	// "special" functions
    if (Operand->Kind == KindCompilerFunction)
	    if (Operand->LinkNr == CommandSizeof){
			GetSpecialFunctionCall(f_name, Operand, f);
			_return_(4,);
		}

	so(Type2Str(this, Operand->Type));
	// link operand onto this command
//	so(cmd->NumParams);


	
	// find (and provisorically link) the parameters in the source
	int np;
	sType *WantedType[SCRIPT_MAX_PARAMS];
	
	bool needs_brackets = ((Operand->Type != TypeVoid) || (Operand->NumParams != 1));
	if (needs_brackets){
		FindFunctionParameters(np, WantedType, f, Operand);
		
	}else{
		np = 1;
		FindFunctionSingleParameter(0, WantedType, f, Operand);
	}
	if (Error){
		_return_(4,);
	}
	

	// return: parameter type by function
	if ((Operand->Kind == KindCompilerFunction) && (Operand->LinkNr == CommandReturn))
		WantedType[0] = f->LiteralType;

	// test compatibility
	if (np != Operand->NumParams){
		Exp.cur_exp --;
		DoError(format("function \"%s\" expects %d parameters, %d were found",f_name, Operand->NumParams, np));
		_return_(4,);
	}
	for (int p=0;p<np;p++){

		CheckParamLink(Operand->Param[p], WantedType[p], f_name, p);
		if (Error){
			_return_(4,);
		}
	}
	msg_db_l(4);
}

sCommand *CPreScript::GetOperand(sFunction *f)
{
	msg_db_r("GetOperand", 4);
	sCommand *Operand = NULL;
	so(cur_name);

	// ( -> one level down and combine commands
	if (strcmp(cur_name, "(") == 0){
		next_exp();
		Operand = GetCommand(f);
		if (strcmp(cur_name, ")") != 0)
			_do_error_("\")\" expected", 4, Operand);
		next_exp();
	}else if (strcmp(cur_name, "&") == 0){ // & -> address operator
		so("<Adress-Operator &>");
		next_exp();
		Operand = GetOperand(f);
		if (Error)
			_return_(4, Operand);
		ref_command(this, Operand);
	}else if (strcmp(cur_name, "*") == 0){ // * -> dereference
		so("<Dereferenzierung *>");
		next_exp();
		Operand = GetOperand(f);
		if (Error)
			_return_(4, Operand);
		if (!Operand->Type->IsPointer){
			Exp.cur_exp --;
			_do_error_("only pointers can be dereferenced using \"*\"", 4, Operand);
		}
		deref_command(this, Operand);
	}else{
		// direct operand
		if (GetExistence(cur_name, f)){
			Operand = cp_command(this, &GetExistenceLink);
			char f_name[SCRIPT_MAX_NAME * 2];
			strcpy(f_name, cur_name);
			so("=> " + Kind2Str(Operand->Kind));
			next_exp();
			// variables get linked directly...

			// operand is executable
			if ((Operand->Kind == KindFunction) || (Operand->Kind == KindCompilerFunction)){
				GetFunctionCall(f_name, Operand, f);
				
			}else if (Operand->Kind == KindPrimitiveOperator){
				// unary operator
				int _ie=Exp.cur_exp-1;
				so("  => unaerer Operator");
				int po = Operand->LinkNr, o=-1;
				sCommand *sub_command = GetOperand(f);
				if (Error)
					_return_(4, Operand);
				sType *r = TypeVoid;
				sType *p2 = sub_command->Type;

				// exact match?
				bool ok=false;
				for (int i=0;i<PreOperator.num;i++)
					if ((unsigned)po == PreOperator[i].PrimitiveID)
						if ((PreOperator[i].ParamType1 == TypeVoid) && (type_match(p2, false, PreOperator[i].ParamType2))){
							o = i;
							r = PreOperator[i].ReturnType;
							ok = true;
							break;
						}


				// needs type casting?
				if (!ok){
					int pen2;
					int c2, c2_best;
					int pen_min = 100;
					for (int i=0;i<PreOperator.num;i++)
						if (po == PreOperator[i].PrimitiveID)
							if ((PreOperator[i].ParamType1 == TypeVoid) && (type_match_with_cast(p2, false, false, PreOperator[i].ParamType2, pen2, c2))){
								ok = true;
								if (pen2 < pen_min){
									r = PreOperator[i].ReturnType;
									o = i;
									pen_min = pen2;
									c2_best = c2;
								}
						}
					// cast
					if (ok){
						apply_type_cast(this, c2_best, sub_command);
						if (Error)
							_return_(4, Operand);
					}
				}


				if (!ok){
					Exp.cur_exp = _ie;
					_do_error_(format("unknown unitary operator  %s", p2->Name), 4, Operand);
				}
				CommandMakeOperator(Operand, sub_command, NULL, o);
				so(Operator2Str(this,o));
				_return_(4, Operand);
			}
		}else{
			sType *t = GetConstantType();
			if (Error)	_return_(4, Operand);
			if (t != TypeUnknown){
				so("=> Konstante");
				Operand = AddCommand();
				Operand->Kind = KindConstant;
				// constant for parameter (via variable)
				Operand->Type = t;
				Operand->LinkNr = AddConstant(t);
				int size = t->Size;
				if (t == TypeString)
					size = 256;
				memcpy(Constant[Operand->LinkNr].data, GetConstantValue(), size);
				next_exp();
			}else{
				//Operand.Kind=0;
				_do_error_("unknown operand", 4, Operand);
			}
		}

	}
	if (Error)
			_return_(4, Operand);

	// Arrays, Strukturen aufloessen...
	GetOperandExtension(Operand,f);

	so(format("Operand endet mit %s", get_name(Exp.cur_exp - 1)));
	_return_(4, Operand);
}

// only "primitive" operator -> no type information
sCommand *CPreScript::GetOperator(sFunction *f)
{
	msg_db_r("GetOperator",4);
	so(cur_name);
	int op = WhichPrimitiveOperator(cur_name);
	if (op >= 0){

		// command from operator
		sCommand *cmd = AddCommand();
		cmd->Kind = KindPrimitiveOperator;
		cmd->LinkNr = op;
		// only provisorically (only operator sign, parameters and their types by GetCommand!!!)

		next_exp();
		msg_db_l(4);
		return cmd;
	}
	msg_db_l(4);
	return NULL;
}

inline void CommandMakeOperator(sCommand *cmd, sCommand *p1, sCommand *p2, int op)
{
	cmd->Kind = KindOperator;
	cmd->LinkNr = op;
	cmd->NumParams = ((PreOperator[op].ParamType1 == TypeVoid) || (PreOperator[op].ParamType2 == TypeVoid)) ? 1 : 2; // unary / binary
	cmd->Param[0] = p1;
	cmd->Param[1] = p2;
	cmd->Type = PreOperator[op].ReturnType;
}

/*inline int find_operator(int primitive_id, sType *param_type1, sType *param_type2)
{
	for (int i=0;i<PreOperator.num;i++)
		if (PreOperator[i].PrimitiveID == primitive_id)
			if ((PreOperator[i].ParamType1 == param_type1) && (PreOperator[i].ParamType2 == param_type2))
				return i;
	//_do_error_("");
	return 0;
}*/

// both operand types have to match the operator's types
//   (operater wants a pointer -> all pointers are allowed!!!)
//   (same for classes of same type...)
inline bool type_match(sType *type, bool is_class, sType *wanted)
{
	if (type == wanted)
		return true;
	if ((type->IsPointer) && (wanted == TypePointer))
		return true;
	if ((is_class) && (wanted == TypeClass))
		return true;
	if ((type->IsSuperArray) && (wanted == TypeSuperArray))
		return true;
	return false;
}

inline bool type_match_with_cast(sType *type, bool is_class, bool is_modifiable, sType *wanted, int &penalty, int &cast)
{
	penalty = 0;
	cast = -1;
	if (type_match(type, is_class, wanted))
	    return true;
	if (is_modifiable) // is a variable getting assigned.... better not cast
		return false;
	for (int i=0;i<TypeCast.num;i++)
		if ((direct_type_match(TypeCast[i].Source, type)) && (direct_type_match(TypeCast[i].Dest, wanted))){ // type_match()?
			penalty = TypeCast[i].Penalty;
			cast = i;
			return true;
		}
	return false;
}

void apply_type_cast(CPreScript *ps, int tc, sCommand *param)
{
	if (tc < 0)
		return;
	so(format("Benoetige automatischen TypeCast: %s -> %s", TypeCast[tc].Source->Name, TypeCast[tc].Dest->Name));
	if (param->Kind == KindConstant){
		char *data_old = ps->Constant[param->LinkNr].data;
		char *data_new = (char*)TypeCast[tc].Func(data_old);
		if ((TypeCast[tc].Dest->IsArray) || (TypeCast[tc].Dest->IsSuperArray)){
			// arrays as return value -> reference!
			delete[] data_old;
			ps->Constant[param->LinkNr].data = new char[TypeCast[tc].Dest->Size];
			data_new = *(char**)data_new;
			memcpy(ps->Constant[param->LinkNr].data, data_new, TypeCast[tc].Dest->Size);
		}else
			memcpy(ps->Constant[param->LinkNr].data, data_new, TypeCast[tc].Dest->Size);
		ps->Constant[param->LinkNr].type = TypeCast[tc].Dest;
		param->Type = TypeCast[tc].Dest;
		so("  ...Konstante wurde direkt gewandelt!");
	}else{
		sCommand *sub_cmd = cp_command(ps, param);
		ps->CommandSetCompilerFunction(TypeCast[tc].Command, param);
		param->Param[0] = sub_cmd;
		so("  ...keine Konstante: Wandel-Befehl wurde hinzugefuegt!");
	}
}

void CPreScript::LinkMostImportantOperator(int &NumOperators, sCommand **Operand, sCommand **Operator, int *op_exp)
{
	msg_db_r("LinkMostImpOp",4);
// find the most important operator (mio)
	int mio=0;
	for (int i=0;i<NumOperators;i++){
		so(format("%d %d", Operator[i]->LinkNr, Operator[i]->LinkNr));
		if (PrimitiveOperator[Operator[i]->LinkNr].Level > PrimitiveOperator[Operator[mio]->LinkNr].Level)
			mio=i;
	}
	so(mio);

// link it
	sCommand *param1 = Operand[mio];
	sCommand *param2 = Operand[mio + 1];
	bool left_modifiable = PrimitiveOperator[Operator[mio]->LinkNr].LeftModifiable;
	
	int po = Operator[mio]->LinkNr, o = -1;
	sType *p1 = Operand[mio]->Type;
	sType *p2 = Operand[mio+1]->Type;
	bool equal_classes = false;
	if (p1 == p2)
		if (!p1->IsSuperArray)
			if (p1->Element.num > 0)
				equal_classes = true;


	// exact match?
	bool ok = false;
	for (int i=0;i<PreOperator.num;i++)
		if (po == PreOperator[i].PrimitiveID)
			if (type_match(p1, equal_classes, PreOperator[i].ParamType1) && type_match(p2, equal_classes, PreOperator[i].ParamType2)){
				o = i;
				ok = true;
				break;
			}


	// needs type casting?
	if (!ok){
		int pen1, pen2;
		int c1, c2, c1_best, c2_best;
		int pen_min = 200;
		for (int i=0;i<PreOperator.num;i++)
			if ((unsigned)po == PreOperator[i].PrimitiveID)
				if (type_match_with_cast(p1, equal_classes, left_modifiable, PreOperator[i].ParamType1, pen1, c1) && type_match_with_cast(p2, equal_classes, false, PreOperator[i].ParamType2, pen2, c2)){
					ok = true;
					if (pen1 + pen2 < pen_min){
						o = i;
						pen_min = pen1 + pen2;
						c1_best = c1;
						c2_best = c2;
					}
			}
		// cast
		if (ok){
			apply_type_cast(this, c1_best, param1);
			apply_type_cast(this, c2_best, param2);
			if (Error)
				_return_(4,);
		}
	}

	if (ok){
		CommandMakeOperator(Operator[mio], param1, param2, o);
	}else{
		Exp.cur_exp = op_exp[mio];
		_do_error_(format("no operator found: (%s) %s (%s)", Type2Str(this,p1).c_str(), PrimitiveOperator2Str(po).c_str(), Type2Str(this,p2).c_str()), 4,);
	}

// ihn aus der Liste herauskuerzen
	Operand[mio]=Operator[mio];
	for (int i=mio;i<NumOperators-1;i++){
		Operator[i]=Operator[i+1];
		Operand[i+1]=Operand[i+2];
		op_exp[i] = op_exp[i+1];
	}
	NumOperators--;
	msg_db_l(4);
}

sCommand *CPreScript::GetCommand(sFunction *f)
{
	msg_db_r("GetCommand", 4);
	int NumOperands = 0;
	Array<sCommand*> Operand;
	Array<sCommand*> Operator;
	Array<int> op_exp;

	// find the first operand
	Operand.add(GetOperand(f));
	if (Error){
		msg_db_l(4);
		return Operand[0];
	}
	NumOperands ++;

	// je einen Operator und einen Operanden finden
	for (int i=0;true;i++){
		op_exp.add(Exp.cur_exp);
		sCommand *op = GetOperator(f);
		if (op){
			Operator.add(op);
			if (end_of_line()){
				//Exp.cur_exp --;
				_do_error_("unexpected end of line after operator", 4, NULL);
			}
			Operand.add(GetOperand(f));
			if (Error){
				msg_db_l(4);
				return NULL;
			}
			NumOperands++;
		}else{
			if (Error){
				msg_db_l(4);
				return NULL;
			}
			so("(kein weiterer Operator)");
			so(cur_name);
			break;
		}
	}


	// in jedem Schritt den wichtigsten Operator finden und herauskuerzen
	int NumOperators=NumOperands-1;
	for (int i=0;i<NumOperands-1;i++){
		LinkMostImportantOperator(NumOperators, &Operand[0], &Operator[0], &op_exp[0]);
		if (Error){
			msg_db_l(4);
			return Operand[0];
		}
	}

	sCommand *ret = Operand[0];
	Operand.clear();
	Operator.clear();
	op_exp.clear();

	// der gesammte Befehl hat sich dann im Operand[0] gesammelt
	// (ohne Operator war Operand[0] schon das einzig wichtige)

	so("-fertig");
	so(format("Command endet mit %s", get_name(Exp.cur_exp - 1)));
	msg_db_l(4);
	return ret;
}

void conv_cbr(CPreScript *ps, sCommand *&c, int var);

void CPreScript::GetSpecialCommand(sBlock *block, sFunction *f)
{
	msg_db_r("GetSpecialCommand", 4);

	// special commands...
	if (strcmp(cur_name, "for") == 0){
		// variable
		next_exp();
		sCommand *for_var;
		// internally declared?
		bool internally = false;
		if ((strcmp(cur_name, "int") == 0) || (strcmp(cur_name, "float") == 0)){
			sType *t = (strcmp(cur_name, "int") == 0) ? TypeInt : TypeFloat;
			internally = true;
			next_exp();
			int var_no = AddVar(cur_name, t, f);
			exlink_make_var_local(this, t, var_no);
 			for_var = cp_command(this, &GetExistenceLink);
		}else{
			GetExistence(cur_name, f);
 			for_var = cp_command(this, &GetExistenceLink);
			if (Error)	_return_(4,);
			if ((!is_variable(for_var->Kind)) || ((for_var->Type != TypeInt) && (for_var->Type != TypeFloat)))
				_do_error_("int or float variable expected after \"for\"", 4,);
		}
		next_exp();

		// first value
		if (strcmp(cur_name, ",") != 0)
			_do_error_("\",\" expected after variable in for", 4,);
		next_exp();
		sCommand *val0 = GetCommand(f);
		if (Error)	_return_(4,);
		if (val0->Type != for_var->Type){
			Exp.cur_exp --;
			_do_error_(format("%s expected as first value of for", for_var->Type->Name), 4,);
		}

		// last value
		if (strcmp(cur_name, ",") != 0)
			_do_error_("\",\" expected after variable in for", 4,);
		next_exp();
		sCommand *val1 = GetCommand(f);
		if (Error)	_return_(4,);
		if (val1->Type != for_var->Type){
			Exp.cur_exp --;
			_do_error_(format("%s expected as last value of for", for_var->Type->Name), 4,);
		}

		// implement
		// for_var = val0
		sCommand *cmd_assign = AddCommand();
		CommandMakeOperator(cmd_assign, for_var, val0, OperatorIntAssign);
		block->Command.add(cmd_assign);
			
		// while(for_var < val1)
		sCommand *cmd_cmp = AddCommand();
		CommandMakeOperator(cmd_cmp, for_var, val1, OperatorIntSmaller);
			
		sCommand *cmd_while = add_command_compilerfunc(this, CommandFor);
		cmd_while->Param[0] = cmd_cmp;
		block->Command.add(cmd_while);
		if (ExpectNewline())
			_return_(4,);
		// ...block
		next_line();
		if (ExpectIndent())
			_return_(4,);
		int loop_block_no = Block.num; // should get created...soon
		GetCompleteCommand(block, f);
			
		// ...for_var += 1
		sCommand *cmd_inc = AddCommand();
		if (for_var->Type == TypeInt){
			CommandMakeOperator(cmd_inc, for_var, val1 /*dummy*/, OperatorIntIncrease);
		}else{
			int nc = AddConstant(TypeFloat);
			*(float*)Constant[nc].data = 1.0;
			sCommand *val_add = add_command_const(this, nc);
			CommandMakeOperator(cmd_inc, for_var, val_add, OperatorFloatAddS);
		}
		sBlock *loop_block = Block[loop_block_no];
		loop_block->Command.add(cmd_inc); // add to loop-block

		// <for_var> declared internally?
		// -> force it out of scope...
		if (internally)
			strcpy(f->Var[for_var->LinkNr].Name, "-out-of-scope-");

	}else if (strcmp(cur_name, "forall") == 0){
		// for index
		int var_no_index = AddVar("-for_index-", TypeInt, f);
		exlink_make_var_local(this, TypeInt, var_no_index);
 		sCommand *for_index = cp_command(this, &GetExistenceLink);
		
		// variable
		next_exp();
		char var_name[SCRIPT_MAX_NAME];
		strcpy(var_name, cur_name);
		next_exp();

		// super array
		if (strcmp(cur_name, "in") != 0)
			_do_error_("\"in\" expected after variable in forall", 4,);
		next_exp();
		GetExistence(cur_name, f);
		sCommand *for_array = cp_command(this, &GetExistenceLink);
		if (Error)	_return_(4,);
		if ((!is_variable(for_array->Kind)) || (!for_array->Type->IsSuperArray))
			_do_error_("list variable expected as second parameter in \"forall\"", 4,);
		next_exp();

		// variable...
		sType *var_type = for_array->Type->SubType;
		int var_no = AddVar(var_name, var_type, f);
		exlink_make_var_local(this, var_type, var_no);
 		sCommand *for_var = cp_command(this, &GetExistenceLink);

		// 0
		int nc = AddConstant(TypeInt);
		*(int*)Constant[nc].data = 0;
		sCommand *val0 = add_command_const(this, nc);

		// implement
		// for_index = 0
		sCommand *cmd_assign = AddCommand();
		CommandMakeOperator(cmd_assign, for_index, val0, OperatorIntAssign);
		block->Command.add(cmd_assign);

		// array.num
		sCommand *val1 = AddCommand();
		val1->Kind = KindAddressShift;
		val1->LinkNr = PointerSize;
		val1->Type = TypeInt;
		val1->NumParams = 1;
		val1->Param[0] = for_array;
			
		// while(for_index < val1)
		sCommand *cmd_cmp = AddCommand();
		CommandMakeOperator(cmd_cmp, for_index, val1, OperatorIntSmaller);
			
		sCommand *cmd_while = add_command_compilerfunc(this, CommandFor);
		cmd_while->Param[0] = cmd_cmp;
		block->Command.add(cmd_while);
		if (ExpectNewline())
			_return_(4,);
		// ...block
		next_line();
		if (ExpectIndent())
			_return_(4,);
		int loop_block_no = Block.num; // should get created...soon
		GetCompleteCommand(block, f);
			
		// ...for_index += 1
		sCommand *cmd_inc = AddCommand();
		CommandMakeOperator(cmd_inc, for_index, val1 /*dummy*/, OperatorIntIncrease);
		sBlock *loop_block = Block[loop_block_no];
		loop_block->Command.add(cmd_inc); // add to loop-block

		// &for_var
		sCommand *for_var_ref = AddCommand();
		command_make_ref(this, for_var_ref, for_var);

		// &array[for_index]
		sCommand *array_el = AddCommand();
		array_el->Kind = KindPointerAsArray;
		array_el->NumParams = 2;
		array_el->Param[0] = for_array;
		array_el->Param[1] = for_index;
		array_el->Type = var_type;
		sCommand *array_el_ref = AddCommand();
		command_make_ref(this, array_el_ref, array_el);

		// &for_var = &array[for_index]
		sCommand *cmd_var_assign = AddCommand();
		CommandMakeOperator(cmd_var_assign, for_var_ref, array_el_ref, OperatorPointerAssign);
		loop_block->Command.insert(cmd_var_assign, 0);

		// ref...
		f->Var[var_no].Type = GetPointerType(var_type);
		foreach(loop_block->Command, c)
			conv_cbr(this, c, var_no);

		// force for_var out of scope...
		strcpy(f->Var[for_var->LinkNr].Name, "-out-of-scope-");
		strcpy(f->Var[for_index->LinkNr].Name, "-out-of-scope-");
		
	}else if (strcmp(cur_name, "while") == 0){
		next_exp();
		sCommand *cmd_cmp = GetCommand(f);
		if (Error)	_return_(4,);
		CheckParamLink(cmd_cmp, TypeBool, "while", 0);
		if (Error)	_return_(4,);
		if (ExpectNewline())
			_return_(4,);
			
		sCommand *cmd_while = add_command_compilerfunc(this, CommandWhile);
		cmd_while->Param[0] = cmd_cmp;
		block->Command.add(cmd_while);
		// ...block
		next_line();
		if (ExpectIndent())
			_return_(4,);
		GetCompleteCommand(block, f);
 	}else if (strcmp(cur_name, "break") == 0){
		next_exp();
		sCommand *cmd = add_command_compilerfunc(this, CommandBreak);
		block->Command.add(cmd);
	}else if (strcmp(cur_name, "continue") == 0){
		next_exp();
		sCommand *cmd = add_command_compilerfunc(this, CommandContinue);
		block->Command.add(cmd);
	}else if (strcmp(cur_name, "if") == 0){
		int ind = Exp.cur_line->indent;
		next_exp();
		sCommand *cmd_cmp = GetCommand(f);
		if (Error)	_return_(4,);
		CheckParamLink(cmd_cmp, TypeBool, "if", 0);
		if (Error)	_return_(4,);
		if (ExpectNewline())
			_return_(4,);
			
		sCommand *cmd_if = add_command_compilerfunc(this, CommandIf);
		cmd_if->Param[0] = cmd_cmp;
		block->Command.add(cmd_if);
		// ...block
		next_line();
		if (ExpectIndent())
			_return_(4,);
		GetCompleteCommand(block, f);
		next_line();

		// else?
		if ((!end_of_file()) && (strcmp(cur_name, "else") == 0) && (Exp.cur_line->indent >= ind)){
			cmd_if->LinkNr = CommandIfElse;
			next_exp();
			// iterative if
			if (strcmp(cur_name, "if") == 0){
				// sub-if's in a new block
				sBlock *new_block = AddBlock();
				if (Error)
					_return_(4,);
				// parse the next if
				GetCompleteCommand(new_block, f);
				// command for the found block
				sCommand *cmd_block = AddCommand();
				cmd_block->Kind = KindBlock;
				cmd_block->LinkNr = new_block->Index;
				// ...
				block->Command.add(cmd_block);
				_return_(4,);
			}
			if (ExpectNewline())
				_return_(4,);
			// ...block
			next_line();
			if (ExpectIndent())
				_return_(4,);
			GetCompleteCommand(block, f);
			//next_line();
		}else{
			Exp.cur_line --;
			Exp.cur_exp = Exp.cur_line->exp.num - 1;
		}
	}
	
	msg_db_l(4);
}

/*void ParseBlock(sBlock *block, sFunction *f)
{
}*/

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void CPreScript::GetCompleteCommand(sBlock *block, sFunction *f)
{
	msg_db_r("GetCompleteCommand", 4);
	// cur_exp = 0!

	sType *tType = GetType(Exp.cur_exp, false);
	int last_indent = indent_0;

	// block?  <- indent
	if (indented){
		indented = false;
		Exp.cur_exp = 0; // bad hack...
		msg_db_r("Block", 4);
		sBlock *new_block = AddBlock();
		if (Error){
			msg_db_l(4);
			_return_(4,);
		}
		new_block->Root = block->Index;

		sCommand *c = AddCommand();
		c->Kind = KindBlock;
		c->LinkNr = new_block->Index;
		block->Command.add(c);

		for (int i=0;true;i++){
			if (((i > 0) && (Exp.cur_line->indent < last_indent)) || (end_of_file()))
				break;

			GetCompleteCommand(new_block, f);
			if (Error){
				msg_db_l(4);
				_return_(4,);
			}
			next_line();
		}
		Exp.cur_line --;
		indent_0 = Exp.cur_line->indent;
		indented = false;
		Exp.cur_exp = Exp.cur_line->exp.num - 1;
		msg_db_l(4);

	// assembler block
	}else if (strcmp(cur_name, "-asm-") == 0){
		next_exp();
		so("<Asm-Block>");
		sCommand *c = add_command_compilerfunc(this, CommandAsm);
		block->Command.add(c);

	// local (variable) definitions...
	// type of variable
	}else if (tType){
		for (int l=0;!end_of_line();l++){
			ParseVariableDefSingle(tType, f);

			// assignment?
			if (strcmp(cur_name, "=") == 0){
				Exp.cur_exp --;
				sCommand *c = GetCommand(f);
				if (Error)
					_return_(4,);
				block->Command.add(c);
			}
			if (end_of_line())
				break;
			if ((strcmp(cur_name, ",") != 0) && (!end_of_line()))
				_do_error_("\",\", \"=\" or newline expected after definition of local variable", 4,);
			next_exp();
		}
		_return_(4,);
	}else{

		
	// commands (the actual code!)
		if ((strcmp(cur_name, "for") == 0) || (strcmp(cur_name, "forall") == 0) || (strcmp(cur_name, "while") == 0) || (strcmp(cur_name, "break") == 0) || (strcmp(cur_name, "continue") == 0) || (strcmp(cur_name, "if") == 0)){
			GetSpecialCommand(block, f);

		}else{

			// normal commands
			sCommand *c = GetCommand(f);
			if (Error)
				_return_(4,);

			// link
			block->Command.add(c);
		}
	}

	if (ExpectNewline())
		_return_(4,);
	msg_db_l(4);
}

// look for array definitions and correct pointers
void CPreScript::TestArrayDefinition(sType **type, bool is_pointer)
{
	msg_db_r("TestArrayDef", 4);
	if (is_pointer){
		(*type) = GetPointerType((*type));
	}
	if (strcmp(cur_name, "[") == 0){
		sType nt;
		int array_size;
		char or_name[SCRIPT_MAX_NAME];
		strcpy(or_name, (*type)->Name);
		int or_name_length = strlen(or_name);
		so("-Array-");
		next_exp();

		// no index -> super array
		if (strcmp(cur_name, "]") == 0){
			array_size = -1;
			
		}else{

			// find array index
			sCommand *c = GetCommand(&RootOfAllEvil);
			if (Error)	_return_(4,);
			PreProcessCommand(NULL, c);

			if ((c->Kind != KindConstant) || (c->Type != TypeInt)){
				DoError("only constants of type \"int\" allowed for size of arrays");
				msg_db_l(4);
				return;
			}
			array_size = *(int*)Constant[c->LinkNr].data;
			//next_exp();
			if (strcmp(cur_name, "]") != 0){
				DoError("\"]\" expected after array size");
				msg_db_l(4);
				return;
			}
		}
		next_exp();
		// recursion
		TestArrayDefinition(type, false); // is_pointer=false, since pointers have been handled

		// create array       (complicated name necessary to get correct ordering   int a[2][4] = (int[4])[2])
		if (array_size < 0){
			(*type) = CreateNewType(	format("%s[]%s", or_name, &(*type)->Name[or_name_length]).c_str(),
			                        	SuperArraySize, false, false, true, array_size, (*type));
		}else{
			(*type) = CreateNewType(	format("%s[%d]%s", or_name, array_size, &(*type)->Name[or_name_length]).c_str(),
			                        	(*type)->Size * array_size, false, false, true, array_size, (*type));
		}
		if (strcmp(cur_name, "*") == 0){
			so("nachtraeglich Pointer");
			next_exp();
			TestArrayDefinition(type, true);
		}
	}
	msg_db_l(4);
}


// Datei auslesen (und Kommentare auslesen)
bool CPreScript::LoadToBuffer(const string &filename,bool just_analyse)
{
	msg_db_r("LoadToBuffer",4);

// read file
	CFile *f = OpenFile(filename);
	if (!f){
		Exp.cur_line = NULL;
		DoError("script file not loadable");
		msg_db_l(4);
		return false;
	}
	Buffer = f->ReadComplete();
	FileClose(f);

	Analyse(Buffer.c_str(), just_analyse);


	Buffer.clear();

	msg_db_l(4);
	return !Error;
}


void CPreScript::ParseEnum()
{
	msg_db_r("ParseEnum", 4);
	next_exp(); // 'enum'
	if (ExpectNewline())
		_return_(4,);
	int value = 0;
	next_line();
	if (ExpectIndent())
		_return_(4,);
	for (int i=0;!end_of_file();i++){
		for (int j=0;!end_of_line();j++){
			int nc = AddConstant(TypeInt);
			sConstant *c = &Constant[nc];
			strcpy(c->name, cur_name);
			next_exp();

			// explicit value
			if (strcmp(cur_name, ":") == 0){
				next_exp();
				if (ExpectNoNewline())
					_return_(4,);
				sType *type = GetConstantType();
				if (type == TypeInt)
					value = *(int*)GetConstantValue();
				else
					_do_error_("integer constant expected after \":\" for explicit value of enum", 4,);
				next_exp();
			}
			*(int*)c->data = value ++;
			
			if (end_of_line())
				break;
			if ((strcmp(cur_name, ",") != 0))
				_do_error_("\",\" or newline expected after enum definition", 4,);
			next_exp();
			if (ExpectNoNewline())
				_return_(4,);
		}
		next_line();
		if (unindented)
			break;
	}
	Exp.cur_line --;
	msg_db_l(4);
}

void ParseClassFunction(CPreScript *ps, sType *t)
{
	ps->ParseFunction(ps->GetPointerType(t));

	// convert name to Class.Function
	sFunction *f = &ps->Function.back();
	sprintf(f->Name, "%s.%s", t->Name, f->Name);

	// remove instance parameter....
	//f->Var.erase(f->Var.begin());
	f->NumParams --;
	msg_todo("Kaba: Class Function parameters...");
}

inline bool type_needs_alignment(sType *t)
{
	if (t->IsArray)
		return type_needs_alignment(t->SubType);
	return (t->Size >= 4);
}

void CPreScript::ParseClass()
{
	msg_db_r("ParseClass", 4);

	int indent0 = Exp.cur_line->indent;
	int _offset = 0;
	next_exp(); // 'class'
	char name[SCRIPT_MAX_NAME * 2];
	strcpy(name, cur_name);
	next_exp();

	// create class and type
	sType *t = CreateNewType(name, 0, false, false, false, 0, NULL);

	// parent class
	if (strcmp(cur_name, ":") == 0){
		so("vererbung der struktur");
		next_exp();
		sType *ancestor = GetType(Exp.cur_exp, true);
		if (Error){
			msg_db_l(4);
			return;
		}
		bool found = false;
		if (ancestor->Element.num > 0){
			// inheritance of elements
			t->Element.assign(&ancestor->Element);
			_offset = ancestor->Size;
			found = true;
		}
		if (!found){
			DoError(format("parental type in class definition after \":\" has to be a class, but (%s) is not", ancestor->Name));
			msg_db_l(4);
			return;
		}
	}
	if (ExpectNewline()){
		msg_db_l(4);
		return;
	}

	// elements
	for (int num=0;true;num++){
		next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
		if (end_of_file())
			break;
		int ie = Exp.cur_exp;
		sType *tType = GetType(Exp.cur_exp, true);
		if (Error){
			msg_db_l(4);
			return;
		}
		for (int j=0;!end_of_line();j++){
			//int indent = Exp.cur_line->indent;
			
			sClassElement el;
			bool is_pointer = false;
			sType *type = tType;
			if (strcmp(cur_name, "*") == 0){
				next_exp();
				is_pointer = true;
			}
			strcpy(el.Name, cur_name);
			next_exp();
			TestArrayDefinition(&type, is_pointer);
			el.Type = type;

			// is a function?
			bool is_function = false;
			if (strcmp(cur_name, "(") == 0)
			    is_function = true;
			if (is_function){
				Exp.cur_exp = ie;
				ParseClassFunction(this, t);
				sClassFunction f;
				strcpy(f.Name, el.Name);
				f.Kind = KindFunction;
				f.Nr = Function.num - 1;
				t->Function.add(f);
				
				break;
			}

			
			if (type_needs_alignment(type))
				_offset = mem_align(_offset);
			so(format("Class-Element: %s %s  Offset: %d", type->Name, el.Name, _offset));
			if ((strcmp(cur_name, ",") != 0) && (!end_of_line())){
				DoError("\",\" or newline expected after class element");
				msg_db_l(4);
				return;
			}
			el.Offset = _offset;
			_offset += type->Size;
			t->Element.add(el);
			if (end_of_line())
				break;
			next_exp();
		}
	}
	foreach(t->Element, e)
		if (type_needs_alignment(e.Type))
			_offset = mem_align(_offset);
	t->Size = _offset;

	Exp.cur_line --;
	msg_db_l(4);
}

bool CPreScript::ExpectNoNewline()
{
	if (end_of_line()){
		DoError("unexpected newline");
		return true;
	}
	return false;
}

bool CPreScript::ExpectNewline()
{
	if (!end_of_line()){
		DoError("newline expected");
		return true;
	}
	return false;
}

bool CPreScript::ExpectIndent()
{
	if (!indented){
		DoError("additional indent expected");
		return true;
	}
	return false;
}

void AddExternalVar(char *name, sType *type)
{
	so("extern");
	// already existing?
	bool found = false;
	for (int i=0;i<PreExternalVar.num;i++)
		if (PreExternalVar[i].IsSemiExternal)
			if (strcmp(PreExternalVar[i].Name, name) == 0){
				PreExternalVar[i].Type = type;
				found = true;
				break;
			}

		// not found -> create provisorium (not linkable.... but parsable)
		if (!found){
			// ScriptLinkSemiExternalVar()
			sPreExternalVar v;
			v.Name = new char[strlen(name) + 1];
			strcpy((char*)v.Name, name);
			v.Pointer = NULL;
			v.Type = type;
			v.IsSemiExternal = true;
			PreExternalVar.add(v);
		}
}

void CPreScript::ParseGlobalConst(const char *name, sType *type)
{
	msg_db_r("ParseGlobalConst", 6);
	if (strcmp(cur_name, "=") != 0)
		_do_error_("\"=\" expected after const name", 6, );
	next_exp();

	// find const value
	sCommand *cv = GetCommand(&RootOfAllEvil);
	if (Error)	_return_(6,);
	PreProcessCommand(NULL, cv);
	if (Error)	_return_(6,);

	if ((cv->Kind != KindConstant) || (cv->Type != type)){
		DoError(format("only constants of type \"%s\" allowed as value for this constant", type->Name));
		msg_db_l(6);
		return;
	}

	// give our const the name
	sConstant *c = &Constant[cv->LinkNr];
	strcpy(c->name, name);
	
	msg_db_l(6);
}

sType *CPreScript::ParseVariableDefSingle(sType *type, sFunction *f, bool as_param)
{
	msg_db_r("ParseVariableDefSingle", 6);
	
	bool is_pointer = false;
	char name[SCRIPT_MAX_NAME * 2];

	// pointer?
	if (strcmp(cur_name, "*") == 0){
		next_exp();
		is_pointer = true;
	}

	// name
	strcpy(name, cur_name);
	next_exp();
	so(format("Variable: %s", name));

	// array?
	TestArrayDefinition(&type, is_pointer);
/*	if ((as_param) && ((type->IsArray) || (type->IsSuperArray))){
		// function parameter:  array -> pointer

		type = GetReferenceType(type);
		so("C-Standart:   Array wurde in Referenz umgewandelt!!!!");
	}*/

	// add
	if (next_extern)
		AddExternalVar(name, type);
	else if (next_const){
		ParseGlobalConst(name, type);
	}else
		AddVar(name, type, f);
	msg_db_l(6);
	return type;
}

void CPreScript::ParseVariableDef(bool single, sFunction *f)
{
	msg_db_r("ParseVariableDef", 4);
	sType *type = GetType(Exp.cur_exp, true);
	if (Error){
		msg_db_l(4);
		return;
	}
	
	for (int j=0;true;j++){
		if (ExpectNoNewline())
			break;

		ParseVariableDefSingle(type, f);
		if (Error)
			break;
		
		if ((strcmp(cur_name, ",") != 0) && (!end_of_line())){
			DoError("\",\" or newline expected after definition of a global variable");
			break;
		}

		// last one?
		if (end_of_line())
			break;
		
		next_exp(); // ','
	}
	msg_db_l(4);
}

void CopyFuncDataToExternal(sFunction *f, sPreCommand *c)
{
	c->ReturnType = f->Type;
	for (int j=0;j<f->NumParams;j++){
		sPreCommandParam p;
		p.Name = new char[strlen(f->Var[j].Name) + 1];
		strcpy((char*)p.Name, f->Var[j].Name);
		p.Type = f->Var[j].Type;
		c->Param.add(p);
	}
}

void AddExternalFunc(CPreScript *ps, sFunction *f)
{
	so("extern");
	
	// already existing?
	bool found = false;
	for (int i=0;i<PreCommand.num;i++)
		if (PreCommand[i].IsSemiExternal)
			if (strcmp(PreCommand[i].Name, f->Name) == 0){
				CopyFuncDataToExternal(f, &PreCommand[i]);
				found = true;
				break;
			}
	
	// not found -> create provisorium (not linkable.... but parsable)
	if (!found){
		sPreCommand c;
		c.Name = new char[strlen(f->Name) + 1];
		strcpy((char*)c.Name, f->Name);
		c.Func = NULL;
		CopyFuncDataToExternal(f, &c);
		c.IsClassFunction = false;
		c.IsSemiExternal = true;
		PreCommand.add(c);
	}
	f->Var.clear();
	ps->Function.erase(ps->Function.num - 1);
}

void CPreScript::ParseFunction(sType *class_type)
{
	msg_db_r("ParseFunction", 4);
	
// return type
	sType *type = GetType(Exp.cur_exp, true);
	if (Error){
		msg_db_l(4);
		return;
	}

	// pointer?
	if (strcmp(cur_name, "*") == 0){
		next_exp();
		type = GetPointerType(type);
	}

	so(cur_name);
	int function = AddFunction(cur_name, type);
	if (Error){
		msg_db_l(4);
		return;
	}
	bool func_extern = next_extern;
	next_extern = false;
	sFunction *f = &Function[function];
	next_exp();
	next_exp(); // '('

// parameter list
	if (class_type){
		f->NumParams ++;
		AddVar("self", class_type, f);
	}
	
	if (strcmp(cur_name, ")") != 0)
		for (int k=0;k<SCRIPT_MAX_PARAMS;k++){
			// like variable definitions

			f->NumParams ++;

			// type of parameter variable
			sType *param_type = GetType(Exp.cur_exp,true);
			if (Error){
				msg_db_l(4);
				return;
			}
			sType *pt = ParseVariableDefSingle(param_type, f, true);
			f->Var.back().Type = pt;

			if (strcmp(cur_name, ")") == 0)
				break;

			if (strcmp(cur_name, ",") != 0){
				DoError("\",\" or \")\" expected after parameter");
				msg_db_l(4);
				return;
			}
			next_exp(); // ','
		}
	next_exp(); // ')'

	// save "original" param types (Var[].Type gets altered for call by reference)
	for (int i=0;i<f->NumParams;i++)
		f->LiteralParamType[i] = f->Var[i].Type;

	if (!end_of_line()){
		DoError("newline expected after parameter list");
		msg_db_l(4);
		return;
	}

	if (func_extern){
		AddExternalFunc(this, f);
		msg_db_l(4);
		return;
	}

	ps_line_t *this_line = Exp.cur_line;
	

// instructions
	while(true){
		next_line();
		indented = false;

		// end of file
		if (end_of_file())
			break;

		// end of function
		if (Exp.cur_line->indent <= this_line->indent)
			break;

		// command or local definition
		GetCompleteCommand(f->Block, f);
		if (Error){
			msg_db_l(4);
			return;
		}
	}

	Exp.cur_line --;
	msg_db_l(4);
}

// convert text into script data
void CPreScript::Parser()
{
	if (Error)	return;
	msg_db_r("Parser", 4);

	strcpy(RootOfAllEvil.Name, "RootOfAllEvil");

	// syntax analysis
	Error = false;
	shift_right = 0;

	Exp.cur_line = &Exp.line[0];
	Exp.cur_exp = 0;
	reset_indent();

	// global definitions (enum, class, variables and functions)
	while (!end_of_file()){
		if (Error)
			return;
		next_extern = false;
		next_const = false;

		// extern?
		if (strcmp(cur_name, "extern") == 0){
			next_extern = true;
			next_exp();
		}

		// const?
		if (strcmp(cur_name, "const") == 0){
			next_const = true;
			next_exp();
		}

		// enum
		if (strcmp(cur_name, "enum") == 0){
			ParseEnum();

		// class
		}else if ((strcmp(cur_name, "struct") == 0) || (strcmp(cur_name, "class") == 0)){
			ParseClass();
			
		}else{

			// type of definition
			GetType(Exp.cur_exp, true);
			if (Error){
				msg_db_l(4);
				return;
			}
			Exp.cur_exp --;
			bool is_function = false;
			for (int j=1;j<Exp.cur_line->exp.num-1;j++)
				if (strcmp(Exp.cur_line->exp[j].name, "(") == 0)
				    is_function = true;

			// own function?
			if (is_function){
				ParseFunction();
				
			// global variables
			}else{
				ParseVariableDef(false, &RootOfAllEvil);
			}
		}
		next_line();
	}

	msg_db_l(4);
}

void conv_cbr(CPreScript *ps, sCommand *&c, int var)
{
	msg_db_r("conv_cbr", 4);
	//so(Kind2Str(c->Kind));
	
	// recursion...
	so(c->NumParams);
	for (int i=0;i<c->NumParams;i++)
		conv_cbr(ps, c->Param[i], var);
	if (c->Kind == KindBlock)
		foreach(ps->Block[c->LinkNr]->Command, cc)
			conv_cbr(ps, cc, var);
	if (c->Instance)
		conv_cbr(ps, c->Instance, var);
	so("a");

	// convert
	if ((c->Kind == KindVarLocal) && (c->LinkNr == var)){
		so("conv");
		c = cp_command(ps, c);
		c->Type = ps->GetPointerType(c->Type);
		deref_command(ps, c);
	}
	msg_db_l(4);
}

#if 0
void conv_return(CPreScript *ps, sCommand *c)
{
	// recursion...
	for (int i=0;i<c->NumParams;i++)
		conv_return(ps, c->Param[i]);
	
	if ((c->Kind == KindCompilerFunction) && (c->LinkNr == CommandReturn)){
		msg_write("conv ret");
		ref_command(ps, c);
	}
}
#endif


// remove &*x and (*x)[] and (*x).y
void easyfy(CPreScript *ps, sCommand *c, int l)
{
	msg_db_r("easyfy", 4);
	//msg_write(l);
	//msg_write("a");
	
	// recursion...
	for (int i=0;i<c->NumParams;i++)
		easyfy(ps, c->Param[i], l+1);
	if (c->Kind == KindBlock)
		for (int i=0;i<ps->Block[c->LinkNr]->Command.num;i++)
			easyfy(ps, ps->Block[c->LinkNr]->Command[i], l+1);
	if (c->Instance)
		easyfy(ps, c->Instance, l+1);
	
	//msg_write("b");


	// convert
	if (c->Kind == KindReference){
		if (c->Param[0]->Kind == KindDereference){
			so("rem 2");
			// remove 2 knots...
			sCommand *t = c->Param[0]->Param[0];
			*c = *t;
		}
	}else if ((c->Kind == KindAddressShift) || (c->Kind == KindArray)){
		if (c->Param[0]->Kind == KindDereference){
			so("rem 1 (unify)");
			// unify 2 knots (remove 1)
			sCommand *t = c->Param[0]->Param[0];
			c->Kind = (c->Kind == KindAddressShift) ? KindDerefAddressShift : KindPointerAsArray;
			c->Param[0] = t;
		}
	}
	//msg_write("ok");
	msg_db_l(4);
}

// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void CPreScript::ConvertCallByReference()
{
	msg_db_r("ConvertCallByReference", 2);


	// convert functions
	foreach(Function, f){
		
		// parameter: array/class as refrerence
		for (int j=0;j<f.NumParams;j++)
			if ((f.Var[j].Type->IsArray) || (f.Var[j].Type->IsSuperArray) || (f.Var[j].Type->Element.num > 0)){
				f.Var[j].Type = GetPointerType(f.Var[j].Type);

				// internal usage...
				foreach(f.Block->Command, c)
					conv_cbr(this, c, j);
			}

		// return: array as reference
		if ((f.Type->IsArray) /*|| (f.Type->IsSuperArray)*/){
			f.Type = GetPointerType(f.Type);
			/*for (int k=0;k<f.Block->Command.num;k++)
				conv_return(this, f.Block->Command[k]);*/
			// no need... return gets converted automatically (all calls...)
		}
	}

	msg_db_m("a", 3);

	// convert function calls
	foreach(Command, ccc){
		// Command array might be reallocated in the loop!
		sCommand *c = ccc;

		if (c->Kind == KindCompilerFunction)
			if (c->LinkNr == CommandReturn){
				if ((c->Param[0]->Type->IsArray) /*|| (c->Param[j]->Type->IsSuperArray)*/){
					so("conv param (return)");
					so(c->Param[0]->Type->Name);
					ref_command(this, c->Param[0]);
				}
				continue;
			}
		
		if ((c->Kind == KindFunction)|| (c->Kind == KindCompilerFunction)){
			// parameters: array/class as reference
			for (int j=0;j<c->NumParams;j++)
				if ((c->Param[j]->Type->IsArray) || (c->Param[j]->Type->IsSuperArray) || (c->Param[j]->Type->Element.num > 0)){
					so("conv param");
					so(c->Param[j]->Type->Name);
					ref_command(this, c->Param[j]);
				}

			// return: array reference (-> dereference)
			if ((c->Type->IsArray) /*|| (c->Type->IsSuperArray)*/){
				so("conv ret");
				so(c->Type->Name);
				c->Type = GetPointerType(c->Type);
				deref_command(this, c);
			}	
		}

		// special string / list operators
		if (c->Kind == KindOperator){
			// parameters: super array as reference
			for (int j=0;j<c->NumParams;j++)
				if ((c->Param[j]->Type->IsArray) || (c->Param[j]->Type->IsSuperArray)){
					so("conv param (op)");
					so(c->Param[j]->Type->Name);
					ref_command(this, c->Param[j]);
				}
		}
	}
	msg_db_l(2);
}


void CPreScript::Simplify()
{
	msg_db_r("Simplify", 2);
	
	// remove &*
	for (int i=0;i<Function.num;i++){
		sFunction *f = &Function[i];
		for (int k=0;k<f->Block->Command.num;k++)
			easyfy(this, f->Block->Command[k],0);
	}
	

	
	msg_db_l(2);
}

void CPreScript::BreakDownHighLevelOperators()
{
	msg_db_r("BreakDownHighLevelOperators", 4);
	
	for (int i=0;i<Command.num;i++){
		sCommand *c = Command[i];
		if (c->Kind != KindOperator)
			continue;
		/*switch(c->LinkNr){
			case OperatorClassAssign:
				for (int i=0;i<signed(c->Param[0]->Type->Size)/4;i++){
				OCAddInstruction(Opcode,OpcodeSize,inMovEaxM,pk[1],param[1],i*4);
				OCAddInstruction(Opcode,OpcodeSize,inMovMEax,pk[0],param[0],i*4);
			}
			for (int i=4*signed(com->Param[0]->Type->Size/4);i<signed(com->Param[0]->Type->Size);i++){
				OCAddInstruction(Opcode,OpcodeSize,inMovAlM8,pk[1],param[1],i);
				OCAddInstruction(Opcode,OpcodeSize,inMovM8Al,pk[0],param[0],i);
			}
			break;
			}*/
	}
	msg_db_l(4);
}

// split arrays and address shifts into simpler commands...
void CPreScript::BreakDownComplicatedCommands()
{
	msg_db_r("BreakDownComplicatedCommands", 4);

	BreakDownHighLevelOperators();
	
	for (int i=0;i<Command.num;i++){
		sCommand *c = Command[i];
		if (c->Kind == KindArray){
			so("array");

			sType *el_type = c->Type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

			sCommand *c_index = c->Param[1];
			// & array
			sCommand *c_ref_array = cp_command(this, c->Param[0]);
			ref_command(this, c_ref_array);
			// create command for size constant
			int nc = AddConstant(TypeInt);
			*(int*)Constant[nc].data = el_type->Size;
			sCommand *c_size = add_command_const(this, nc);
			// offset = size * index
			sCommand *c_offset = AddCommand();
			CommandMakeOperator(c_offset, c_index, c_size, OperatorIntMultiply);
			c_offset->Type = TypeInt;//TypePointer;
			// address = &array + offset
			sCommand *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_array, c_offset, OperatorIntAdd);
			c_address->Type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->Type = el_type;
		}else if (c->Kind == KindPointerAsArray){
			so("array");

			sType *el_type = c->Type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

			sCommand *c_index = c->Param[1];
			sCommand *c_ref_array = c->Param[0];
			// create command for size constant
			int nc = AddConstant(TypeInt);
			*(int*)Constant[nc].data = el_type->Size;
			sCommand *c_size = add_command_const(this, nc);
			// offset = size * index
			sCommand *c_offset = AddCommand();
			CommandMakeOperator(c_offset, c_index, c_size, OperatorIntMultiply);
			c_offset->Type = TypeInt;
			// address = &array + offset
			sCommand *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_array, c_offset, OperatorIntAdd);
			c_address->Type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->Type = el_type;
		}else if (c->Kind == KindAddressShift){
			so("address shift");

			sType *el_type = c->Type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

			// & struct
			sCommand *c_ref_struct = cp_command(this, c->Param[0]);
			ref_command(this, c_ref_struct);
			// create command for shift constant
			int nc = AddConstant(TypeInt);
			*(int*)Constant[nc].data = c->LinkNr;
			sCommand *c_shift = add_command_const(this, nc);
			// address = &struct + shift
			sCommand *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_struct, c_shift, OperatorIntAdd);
			c_address->Type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->Type = el_type;
		}else if (c->Kind == KindDerefAddressShift){
			so("deref address shift");

			sType *el_type = c->Type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

			sCommand *c_ref_struct = c->Param[0];
			// create command for shift constant
			int nc = AddConstant(TypeInt);
			*(int*)Constant[nc].data = c->LinkNr;
			sCommand *c_shift = add_command_const(this, nc);
			// address = &struct + shift
			sCommand *c_address = AddCommand();
			CommandMakeOperator(c_address, c_ref_struct, c_shift, OperatorIntAdd);
			c_address->Type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->Type = el_type;			
		}			
	}
	msg_db_l(4);
}

// no included scripts may be deleted before us!!!
CPreScript::~CPreScript()
{
	msg_db_r("~CPreScript", 4);
	
	clear_exp_buffer(&Exp);

	// delete all types created by this script
	for (int i=Type.num-NumOwnTypes;i<Type.num;i++)
		if (Type[i]->Owner == this){ // redundant...
			Type[i]->Element.clear();
			Type[i]->Function.clear();
			delete(Type[i]);
		}
	Type.clear();
	
	for (int i=0;i<Define.num;i++){
		for (int j=0;j<Define[i].Dest.num;j++)
			Define[i].Dest[j].clear();
		Define[i].Dest.clear();
	}
	Define.clear();


	PreScriptRule.clear();

	
	msg_db_m("asm", 8);
	if (AsmMetaInfo)
		delete(AsmMetaInfo);
	for (int i=0;i<AsmBlock.num;i++)
		delete[](AsmBlock[i].block);
	AsmBlock.clear();
	
	msg_db_m("const", 8);
	foreach(Constant, c)
		if (c.owner == this)
			delete[](c.data);
	Constant.clear();

	
	msg_db_m("cmd", 8);
	for (int i=0;i<Command.num;i++)
		delete(Command[i]);
	Command.clear();

	msg_db_m("rest", 8);

	for (int i=0;i<Block.num;i++)
		delete(Block[i]);
	Block.clear();
	for (int i=0;i<Function.num;i++)
		Function[i].Var.clear();
	Function.clear();
	
	msg_db_l(4);
}

void CPreScript::ShowCommand(sCommand *c)
{
	msg_write(format("Command: %s, %s", Kind2Str(c->Kind).c_str(), LinkNr2Str(this,c->Kind,c->LinkNr).c_str()));
	msg_right();
	msg_write("Type: " + Type2Str(this,c->Type));
	for (int p=0;p<c->NumParams;p++){
		msg_write("Parameter");
		ShowCommand(c->Param[p]);
	}
	if (c->Instance){
		msg_write("Object:");
		ShowCommand(c->Instance);
	}
	msg_left();
	msg_write("");
}

void CPreScript::ShowBlock(sBlock *b)
{
	msg_write("b");
	msg_right();
	for (int c=0;c<b->Command.num;c++){
		if (b->Command[c]->Kind == KindBlock)
			ShowBlock(Block[b->Command[c]->LinkNr]);
		else
			ShowCommand(b->Command[c]);
	}
	msg_left();
	msg_write("/b");
}

void CPreScript::ShowFunction(int f)
{
	msg_write(format("%d: %s --------------------------", f, Function[f].Name));
	ShowBlock(Function[f].Block);
}

void CPreScript::Show()
{
	//if (Error)	return;
	msg_write("\n\n\n################### Representation ######################\n\n\n");
	msg_write(Filename);
	/*msg_write("Befehle:\n");
	msg_right();
	for (int c=0;c<NumCommands;c++)
		ShowCommand(c);
	msg_left();*/
	msg_write("\nFunctions:\n");
	msg_right();
	for (int f=0;f<Function.num;f++)
		ShowFunction(f);
	msg_left();
	msg_write("\n\n");
}
