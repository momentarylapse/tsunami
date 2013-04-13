#include "../script.h"
#include "../../file/file.h"

namespace Script{


#define SCRIPT_MAX_DEFINE_RECURSIONS	128

//#define ScriptDebug

extern int s2i2(const string &str);

static int shift_right=0;

static void stringout(const char *str)
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

void SetImmortal(SyntaxTree *ps)
{
	ps->FlagImmortal = true;
	for (int i=0;i<ps->Includes.num;i++)
		SetImmortal(ps->Includes[i]->syntax);
}

// import data from an included script file
void SyntaxTree::AddIncludeData(Script *s)
{
	msg_db_f("AddIncludeData",5);
	Includes.add(s);
	SyntaxTree *ps = s->syntax;
	s->ReferenceCounter ++;
	if (FlagImmortal)
		SetImmortal(ps);

	// defines
	for (int i=0;i<ps->Defines.num;i++)
		Defines.add(ps->Defines[i]);

	ExpressionBuffer::Line *cur_line = Exp.cur_line;
	PreCompiler(script->JustAnalyse);
	Exp.cur_line = cur_line;
	Exp.cur_exp = 0;


	// types
//	Type.insert(Type.begin(), ps->Type.begin(), ps->Type.end()); // make sure foreign types precede the "own" types!
/*	for (int i=0;i<ps->Types.num;i++)
		Types.insert(ps->Types[i], i);
		//Type.insert(ps->Type[i + PreType.num], i);

	// constants
	foreach(Constant &c, ps->Constants)
		if (c.name[0] != '-')
			Constants.add(c);*/
	// TODO... ownership of "big" constants
}

enum{
	MacroDefine,
	MacroDisasm,
	MacroNoExec,
	MacroShow,
	MacroShowPrae,
	MacroImmortal,
	MacroOs,
	MacroInitialRealmode,
	MacroVariablesOffset,
	MacroCodeOrigin,
	NumMacroNames
};

string MacroName[NumMacroNames] =
{
	"#define",
	"#disasm",
	"#noexec",
	"#show",
	"#show_prae",
	"#immortal",
	"#os",
	"#initial_realmode",
	"#variables_offset",
	"#code_origin"
};

void SyntaxTree::HandleMacro(ExpressionBuffer::Line *l, int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse)
{
	msg_db_f("HandleMacro", 4);
	Exp.cur_line = l;
	Exp.cur_exp = 0;
	Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
	int ln;
	string filename;
	Script *include;
	Define d;


	int macro_no=-1;
	for (int i=0;i<NumMacroNames;i++)
		if (Exp.cur == MacroName[i])
			macro_no = i;
	
	switch(macro_no){
		case MacroDefine:
			// source
			Exp.next();
			d.Source = Exp.cur;
			// dests
			while(true){
				Exp.next();
				if (Exp.end_of_line())
					break;
				d.Dest.add(Exp.cur);
			}
			Defines.add(d);
			break;
		case MacroDisasm:
			FlagDisassemble=true;
			break;
		case MacroShow:
			FlagShow=true;
			break;
		case MacroShowPrae:
			FlagShowPrae=true;
			break;
		case MacroNoExec:
			FlagNoExecution=true;
			break;
		case MacroImmortal:
			SetImmortal(this);
			//FlagImmortal=true;
			break;
		case MacroOs:
			FlagCompileOS=true;
			break;
		case MacroInitialRealmode:
			FlagCompileInitialRealMode=true;
			break;
		case MacroVariablesOffset:
			FlagOverwriteVariablesOffset=true;
			Exp.next();
			VariablesOffset=s2i2(Exp.cur);
			break;
		case MacroCodeOrigin:
			Exp.next();
			CreateAsmMetaInfo();
			((Asm::MetaInfo*)AsmMetaInfo)->CodeOrigin = s2i2(Exp.cur);
			break;
		default:
			DoError("unknown makro atfer \"#\"");
	}

	// remove macro line
	Exp.line[line_no].exp.clear();
	Exp.line.erase(line_no);
	line_no --;
}

inline void insert_into_buffer(SyntaxTree *ps, const char *name, int pos, int index = -1)
{
	ExpressionBuffer::Expression e;
	e.name = ps->Exp.buf_cur;
	ps->Exp.buf_cur += strlen(name) + 1;
	strcpy(e.name, name);
	e.pos = pos;
	if (index < 0)
		// at the end...
		ps->Exp.cur_line->exp.add(e);
	else
		ps->Exp.cur_line->exp.insert(e, index);
}

inline void remove_from_buffer(SyntaxTree *ps, int index)
{
	ps->Exp.cur_line->exp.erase(index);
}

// ... maybe some time later
void SyntaxTree::PreCompiler(bool just_analyse)
{
	msg_db_f("PreCompiler", 4);

	int NumIfDefs = 0;
	bool IfDefed[1024];
	
	for (int i=0;i<Exp.line.num-1;i++){
		Exp.cur_exp = 0;
		Exp.cur_line = &Exp.line[i];
		if (Exp.line[i].exp[0].name[0] == '#'){
			HandleMacro(Exp.cur_line, i, NumIfDefs, IfDefed, just_analyse);
		}else{
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

			// replace by definition?
			int num_defs_inserted = 0;
			while(!Exp.end_of_line()){
				foreachi(Define &d, Defines, j){
					if (Exp.cur == d.Source){
						int pos = Exp.cur_line->exp[Exp.cur_exp].pos;
						remove_from_buffer(this, Exp.cur_exp);
						for (int k=0;k<d.Dest.num;k++){
							insert_into_buffer(this, d.Dest[k].c_str(), pos, Exp.cur_exp);
							Exp.next();
						}
						Exp.cur_exp -= d.Dest.num;
						Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
						num_defs_inserted ++;
						if (num_defs_inserted > SCRIPT_MAX_DEFINE_RECURSIONS)
							DoError("recursion in #define macros");
						break;
					}
				}
				Exp.next();
			}

			// "-" in front of numbers (after ( , : [ = < >)
			Exp.cur_exp = 1;
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
			while(!Exp.end_of_line()){
				if (Exp.cur == "-"){
					string last = Exp.get_name(Exp.cur_exp - 1);
					if ((last == "(") ||
						(last == ",") ||
						(last == ":") ||
						(last == "[") ||
						(last == "=") ||
						(last == "<") ||
						(last == ">")){
						if (isNumber(last[0])){
							string name = string("-") + Exp.get_name(Exp.cur_exp + 1);
							int pos = Exp.cur_line->exp[Exp.cur_exp].pos;
							Exp.remove(Exp.cur_exp);
							Exp.remove(Exp.cur_exp);
							Exp.insert(name.c_str(), pos, Exp.cur_exp);
						}
					}
				}
				Exp.next();
			}
		}
	}

	

	/*msg_db_f("MakeExps",4);
	int i,NumIfDefs=0,ln;
	bool IfDefed[1024];
	Exp=new exp_buffer;
	am("exp_buffer",sizeof(exp_buffer),Exp);
	Exp->BufferUsed=0;
	BufferPos=0;
	Exp->TempLine=1;
	Exp->TempColumn=0;
	Exp->NumExps=0;
	char filename[256];
	Script *include;
	
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
					Define[NumDefines]=new Define;
					am("Define",sizeof(Define),Define[NumDefines]);
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
					am("PreScriptRule",sizeof(sPreScriptRule),PreScriptRule[NumPreScriptRules]);
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
	Exp->ExpNr=0;*/
}

};
