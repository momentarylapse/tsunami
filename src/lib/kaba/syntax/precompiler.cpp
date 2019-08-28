#include "../kaba.h"
#include "../../file/file.h"

namespace Kaba{


#define MAX_DEFINE_RECURSIONS	128

//#define ScriptDebug


void SetImmortal(SyntaxTree *ps)
{
	ps->flag_immortal = true;
	for (Script *i: ps->includes)
		SetImmortal(i->syntax);
}

// import data from an included script file
void SyntaxTree::add_include_data(Script *s)
{
	for (Script *i: includes)
		if (i == s)
			return;

	SyntaxTree *ps = s->syntax;
	if (flag_immortal)
		SetImmortal(ps);

	flag_string_const_as_cstring |= ps->flag_string_const_as_cstring;

	// defines
	defines.append(ps->defines);


	/*if (FlagCompileOS){
		import_deep(this, ps);
	}else{*/
		includes.add(s);
		s->reference_counter ++;
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

	for (Operator *op: ps->operators)
		if (op->owner == ps)
			operators.add(op);
}

enum{
	MacroDefine,
	MacroImmortal,
	NumMacroNames
};

string MacroName[NumMacroNames] =
{
	"#define",
	"#immortal",
};

void SyntaxTree::handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse)
{
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
			d.source = Exp.cur;
			// dests
			while(true){
				Exp.next();
				if (Exp.end_of_line())
					break;
				d.dest.add(Exp.cur);
			}

			// special defines?
			if ((d.source.num > 4) && (d.source.head(2) == "__") && (d.source.tail(2) == "__")){
				if (d.source == "__OS__"){
					do_error("#define __OS__ deprecated");
				}else if (d.source == "__STRING_CONST_AS_CSTRING__"){
					flag_string_const_as_cstring = true;
				}else if (d.source == "__NO_FUNCTION_FRAME__"){
					do_error("#define __NO_FUNCTION_FRAME__ deprecated");
				}else if (d.source == "__ADD_ENTRY_POINT__"){
					do_error("#define __ADD_ENTRY_POINT__ deprecated");
				}else if (d.source == "__VARIABLE_OFFSET__"){
					do_error("#define __VARIABLE_OFFSET__ deprecated");
				}else if (d.source == "__CODE_ORIGIN__"){
					do_error("#define __CODE_ORIGING__ deprecated");
				}else
					do_error("unknown compiler flag (define starting and ending with \"__\"): " + d.source);
			}else
				// normal define
				defines.add(d);
			break;
		case MacroImmortal:
			SetImmortal(this);
			//FlagImmortal=true;
			break;
		default:
			do_error("unknown makro after \"#\"");
			break;
	}

	// remove macro line
	Exp.line[line_no].exp.clear();
	Exp.line.erase(line_no);
	line_no --;
}

// ... maybe some time later
void SyntaxTree::pre_compiler(bool just_analyse)
{
	int NumIfDefs = 0;
	bool IfDefed[1024];
	
	for (int i=0;i<Exp.line.num-1;i++){
		Exp.set(0, i);
		if (Exp.cur[0] == '#'){
			handle_macro(i, NumIfDefs, IfDefed, just_analyse);
		}else if (Exp.line[i].exp[0].name == IDENTIFIER_USE){
			parse_import();
			Exp.line.erase(i);
			i --;
		}else{
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

			// replace by definition?
			int num_defs_inserted = 0;
			while(!Exp.end_of_line()){
				foreachi(Define &d, defines, j){
					if (Exp.cur == d.source){
						int pos = Exp.cur_line->exp[Exp.cur_exp].pos;
						Exp.remove(Exp.cur_exp);
						for (int k=0;k<d.dest.num;k++){
							Exp.insert(d.dest[k].c_str(), pos, Exp.cur_exp);
							Exp.next();
						}
						Exp.set(Exp.cur_exp - d.dest.num);
						num_defs_inserted ++;
						if (num_defs_inserted > MAX_DEFINE_RECURSIONS)
							do_error("recursion in #define macros");
						break;
					}
				}
				Exp.next();
			}

			// "-" in front of numbers (after ( , : [ = < >)
			Exp.set(1);
			while(!Exp.end_of_line()){
				if (Exp.cur == "-"){
					string last = Exp.get_name(Exp.cur_exp - 1);
					if ((last == "(") or
						(last == ",") or
						(last == ":") or
						(last == "[") or
						(last == "=") or
						(last == "<") or
						(last == ">")){
						if (is_number(last[0])){
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
