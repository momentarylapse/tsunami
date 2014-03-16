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
	SyntaxTree *ps = s->syntax;
	if (FlagImmortal)
		SetImmortal(ps);

	FlagCompileOS |= ps->FlagCompileOS;
	FlagAddEntryPoint |= ps->FlagAddEntryPoint;
	FlagNoFunctionFrame |= ps->FlagNoFunctionFrame;
	FlagOverwriteVariablesOffset |= ps->FlagOverwriteVariablesOffset;
	if (ps->FlagOverwriteVariablesOffset)
		VariablesOffset = ps->VariablesOffset;
	FlagStringConstAsCString |= ps->FlagStringConstAsCString;
	if (ps->AsmMetaInfo->CodeOrigin != (long)ps->script->Opcode)
		AsmMetaInfo->CodeOrigin = ps->AsmMetaInfo->CodeOrigin;

	// defines
	Defines.append(ps->Defines);


	/*if (FlagCompileOS){
		import_deep(this, ps);
	}else{*/
		Includes.add(s);
		s->ReferenceCounter ++;
	//}

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

			// special defines?
			if ((d.Source.num > 4) && (d.Source.head(2) == "__") && (d.Source.tail(2) == "__")){
				if (d.Source == "__OS__"){
					FlagCompileOS = true;
				}else if (d.Source == "__STRING_CONST_AS_CSTRING__"){
					FlagStringConstAsCString = true;
				}else if (d.Source == "__NO_FUNCTION_FRAME__"){
					FlagNoFunctionFrame = true;
				}else if (d.Source == "__ADD_ENTRY_POINT__"){
					FlagAddEntryPoint = true;
				}else if (d.Source == "__VARIABLE_OFFSET__"){
					FlagOverwriteVariablesOffset = true;
					if (d.Dest.num != 1)
						DoError("offset value expected after __VARIABLE_OFFSET__");
					VariablesOffset = s2i2(d.Dest[0]);
				}else if (d.Source == "__CODE_ORIGIN__"){
					if (d.Dest.num != 1)
						DoError("offset value expected after __CODE_ORIGIN__");
					AsmMetaInfo->CodeOrigin = s2i2(d.Dest[0]);
				}else
					DoError("unknown compiler flag (define starting and ending with \"__\"): " + d.Source);
			}else
				// normal define
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
