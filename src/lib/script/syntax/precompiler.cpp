#include "../script.h"
#include "../../file/file.h"

namespace Script{


#define SCRIPT_MAX_DEFINE_RECURSIONS	128

//#define ScriptDebug

extern int s2i2(const string &str);


void SetImmortal(SyntaxTree *ps)
{
	ps->FlagImmortal = true;
	for (int i=0;i<ps->Includes.num;i++)
		SetImmortal(ps->Includes[i]->syntax);
}

// import data from an included script file
void SyntaxTree::AddIncludeData(Script *s)
{
	foreach(Script *i, Includes)
		if (i == s)
			return;
	msg_db_f("AddIncludeData",5);
	Includes.add(s);
	SyntaxTree *ps = s->syntax;
	s->ReferenceCounter ++;
	if (FlagImmortal)
		SetImmortal(ps);

	// defines
	Defines.append(ps->Defines);

	/*ExpressionBuffer::Line *cur_line = Exp.cur_line;
	PreCompiler(script->JustAnalyse);
	Exp.cur_line = cur_line;
	Exp.cur_exp = 0;*/


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
	string filename;
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
			DoError("unknown makro after \"#\"");
			break;
	}

	// remove macro line
	Exp.line[line_no].exp.clear();
	Exp.line.erase(line_no);
	line_no --;
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
		}else if (Exp.line[i].exp[0].name == "use"){
			ParseImport();
			Exp.line.erase(i);
			i --;
		}else{
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

			// replace by definition?
			int num_defs_inserted = 0;
			while(!Exp.end_of_line()){
				foreachi(Define &d, Defines, j){
					if (Exp.cur == d.Source){
						int pos = Exp.cur_line->exp[Exp.cur_exp].pos;
						Exp.remove(Exp.cur_exp);
						for (int k=0;k<d.Dest.num;k++){
							Exp.insert(d.Dest[k].c_str(), pos, Exp.cur_exp);
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
							string name = "-" + Exp.get_name(Exp.cur_exp + 1);
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
}

};
