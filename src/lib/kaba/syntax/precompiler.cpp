#include "../kaba.h"
#include "../lib/common.h"
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

static bool _class_contains(const Class *c, const string &name) {
	for (auto *cc: weak(c->classes))
		if (cc->name == name)
			return true;
	for (auto *f: weak(c->functions))
		if (f->name == name)
			return true;
	for (auto *cc: weak(c->constants))
		if (cc->name == name)
			return true;
	return false;
}

// import data from an included script file
void SyntaxTree::add_include_data(shared<Script> s, bool indirect) {
	for (auto i: weak(includes))
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
	if (indirect) {
		imported_symbols->classes.add(ps->base_class);
	} else {
		for (auto *c: weak(ps->base_class->classes))
			imported_symbols->classes.add(c);
		for (auto *f: weak(ps->base_class->functions))
			imported_symbols->functions.add(f);
		for (auto *v: weak(ps->base_class->static_variables))
			imported_symbols->static_variables.add(v);
		for (auto *c: weak(ps->base_class->constants))
			imported_symbols->constants.add(c);
		if (s->filename.basename().find(".kaba") < 0)
			if (!_class_contains(imported_symbols.get(), ps->base_class->name)) {
				imported_symbols->classes.add(ps->base_class);
			}
	}
	includes.add(s);
	//}

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

void Parser::handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse) {
	string filename;
	Define d;


	int macro_no = -1;
	for (int i=0; i<NumMacroNames; i++)
		if (Exp.cur == MacroName[i])
			macro_no = i;
	
	switch(macro_no) {
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
			if ((d.source.num > 4) and (d.source.head(2) == "__") and (d.source.tail(2) == "__")) {
				if (d.source == "__OS__"){
					do_error("#define __OS__ deprecated");
				}else if (d.source == "__STRING_CONST_AS_CSTRING__"){
					tree->flag_string_const_as_cstring = true;
				}else if (d.source == "__FUNCTION_POINTER_AS_CODE__"){
					tree->flag_function_pointer_as_code = true;
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
			} else {
				// normal define
				tree->defines.add(d);
			}
			break;
		case MacroImmortal:
			SetImmortal(tree);
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
void Parser::pre_compiler(bool just_analyse)
{
	int NumIfDefs = 0;
	bool IfDefed[1024];
	
	for (int i=0;i<Exp.line.num-1;i++){
		Exp.set(0, i);
		if (Exp.cur[0] == '#'){
			handle_macro(i, NumIfDefs, IfDefed, just_analyse);
		}else if ((Exp.line[i].exp[0].name == IDENTIFIER_USE) or (Exp.line[i].exp[0].name == IDENTIFIER_IMPORT)) {
			parse_import();
			Exp.line.erase(i);
			i --;
		}else{
			Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

			// replace by definition?
			int num_defs_inserted = 0;
			while(!Exp.end_of_line()){
				foreachi(Define &d, tree->defines, j){
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
