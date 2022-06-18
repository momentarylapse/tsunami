#include "../kaba.h"
#include "Parser.h"
#include "../../file/file.h"

namespace kaba {


#define MAX_DEFINE_RECURSIONS	128

//#define ScriptDebug


void SetImmortal(SyntaxTree *ps) {
	ps->flag_immortal = true;
	for (auto i: ps->includes)
		SetImmortal(i->syntax);
}


enum {
	MacroDefine,
	MacroImmortal,
	NumMacroNames
};

string MacroName[NumMacroNames] =
{
	"#define",
	"#immortal",
};

void Parser::handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse) {
	string filename;
	string source;


	int macro_no = -1;
	for (int i=0; i<NumMacroNames; i++)
		if (Exp.cur == MacroName[i])
			macro_no = i;
	
	switch (macro_no) {
		case MacroDefine:
			// source
			Exp.next();
			source = Exp.cur;
			// dests
			while (true) {
				Exp.next();
				if (Exp.end_of_line())
					break;
				//d.dest.add(Exp.cur);
			}

			// special defines?
			if ((source.num > 4) and (source.head(2) == "__") and (source.tail(2) == "__")) {
				if (source == "__OS__") {
					do_error_exp("#define __OS__ deprecated");
				} else if (source == "__STRING_CONST_AS_CSTRING__") {
					tree->flag_string_const_as_cstring = true;
				} else if (source == "__FUNCTION_POINTER_AS_CODE__") {
					tree->flag_function_pointer_as_code = true;
				} else if (source == "__NO_FUNCTION_FRAME__") {
					do_error_exp("#define __NO_FUNCTION_FRAME__ deprecated");
				} else if (source == "__ADD_ENTRY_POINT__") {
					do_error_exp("#define __ADD_ENTRY_POINT__ deprecated");
				} else if (source == "__VARIABLE_OFFSET__") {
					do_error_exp("#define __VARIABLE_OFFSET__ deprecated");
				} else if (source == "__CODE_ORIGIN__") {
					do_error_exp("#define __CODE_ORIGING__ deprecated");
				} else {
					do_error_exp("unknown compiler flag (define starting and ending with \"__\"): " + source);
				}
			} else {
				do_error_exp("#define deprecated");
				// normal define
			}
			break;
		case MacroImmortal:
			SetImmortal(tree);
			//FlagImmortal=true;
			break;
		default:
			do_error_exp("unknown macro after \"#\"");
			break;
	}

	// remove macro line
	Exp.erase_logical_line(line_no);
	line_no --;
}

// ... maybe some time later
void Parser::parse_macros(bool just_analyse) {
	int NumIfDefs = 0;
	bool IfDefed[1024];
	
	for (int i=0; i<Exp.lines.num-1; i++) {
		Exp.jump(Exp.lines[i].token_ids[0]);

		if (Exp.cur[0] == '#') {
			handle_macro(i, NumIfDefs, IfDefed, just_analyse);
		} else {
#if 0
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

			// replace by definition?
			int num_defs_inserted = 0;
			while (!Exp.end_of_line()) {
				foreachi(Define &d, tree->defines, j) {
					if (Exp.cur == d.source) {
						int pos = Exp.cur_line->exp[Exp.cur_exp].pos;
						Exp.remove(Exp.cur_exp);
						for (int k=0;k<d.dest.num;k++) {
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
			while (!Exp.end_of_line()) {
				if (Exp.cur == "-") {
					string last = Exp.get_name(Exp.cur_exp - 1);
					if ((last == "(") or
						(last == ",") or
						(last == ":") or
						(last == "[") or
						(last == "=") or
						(last == "<") or
						(last == ">")) {
						if (is_number(last[0])) {
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
#endif
		}
	}
}

};
