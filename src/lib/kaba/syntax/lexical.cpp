#include "../../base/base.h"
#include "../../file/file.h"
#include "lexical.h"
#include <stdio.h>
#include "SyntaxTree.h"

namespace kaba {

//#define ScriptDebug

#define MAX_STRING_CONST_LENGTH	65536
static char Temp[MAX_STRING_CONST_LENGTH];

char str_eol[] = "-eol-";

#define SCRIPT_MAX_NAME	256

// type of expression (syntax)
enum class ExpKind {
	NUMBER,
	LETTER,
	SPACING,
	SIGN,
	UNKNOWN = -1
};
static ExpKind exp_kind;


ExpressionBuffer::ExpressionBuffer() : cur(dummy) {
	clear();
	reset_indent();
}

string ExpressionBuffer::get_name(int n) {
	return cur_line->exp[n].name;
}

int ExpressionBuffer::get_line_no() {
	foreachi(Line &l, line, i)
		if (cur_line == &l)
			return i;
	return -1;
}

void ExpressionBuffer::next() {
	cur_exp ++;
	cur = cur_line->exp[cur_exp].name;
}

void ExpressionBuffer::rewind() {
	cur_exp --;
	cur = cur_line->exp[cur_exp].name;
}

bool ExpressionBuffer::end_of_line() {
	return (cur_exp >= cur_line->exp.num - 1); // the last entry is "-eol-"#
}

bool ExpressionBuffer::past_end_of_line() {
	return (cur_exp >= cur_line->exp.num);
}

void ExpressionBuffer::next_line() {
	cur_line ++;
	cur_exp = 0;
	test_indent(cur_line->indent);
	cur = cur_line->exp[cur_exp].name;
}

void ExpressionBuffer::set(int exp_no, int line_no) {
	if (line_no < 0)
		line_no = get_line_no();
	cur_line = &line[line_no];
	cur_exp = exp_no;
	cur = cur_line->exp[cur_exp].name;
}

bool ExpressionBuffer::end_of_file() {
	return ((int_p)cur_line >= (int_p)&line[line.num - 1]); // last line = "-eol-"*/
}

void ExpressionBuffer::reset_parser() {
	cur_line = &line[0];
	cur_exp = 0;
	cur = cur_line->exp[cur_exp].name;
	reset_indent();
}

void ExpressionBuffer::clear() {
	cur_line = nullptr;
	line.clear();
	cur_line = &temp_line;
	cur_exp = -1;
}

void ExpressionBuffer::add_line() {
	Line l;
	line.add(l);
	cur_line = &line.back();
}

void ExpressionBuffer::insert(const char *_name, int pos, int index) {
	Expression e;
	e.name = _name;
	e.pos = pos;
	if (index < 0)
		// at the end...
		cur_line->exp.add(e);
	else
		cur_line->exp.insert(e, index);
}

void ExpressionBuffer::remove(int index) {
	cur_line->exp.erase(index);
}

void ExpressionBuffer::test_indent(int i) {
	indented = (i > indent_0);
	unindented = (i < indent_0);
	indent_0 = i;

}

void ExpressionBuffer::reset_indent() {
	indented = unindented = false;
	indent_0 = 0;
}

ExpKind GetKind(char c) {
	if (is_number(c))
		return ExpKind::NUMBER;
	/*else if (is_letter(c))
		return ExpKind::LETTER;*/
	else if (is_spacing(c))
		return ExpKind::SPACING;
	else if (is_sign(c))
		return ExpKind::SIGN;
	else if (c == 0)
		return ExpKind::UNKNOWN;
	// allow all other characters as letters
	return ExpKind::LETTER;
}

void ExpressionBuffer::analyse(SyntaxTree *ps, const string &_source) {
	syntax = ps;
	string source = _source + string("\0", 1); // :P
	clear();

	// scan all lines
	const char *buf = (char*)source.data;
	for (int i=0;true;i++) {
		//exp_add_line(&Exp);
		cur_line->physical_line = i;
		if (analyse_line(buf, cur_line, i))
			break;
		buf += cur_line->length + 1;
	}

	// glue together lines ending with a "\" or ","
	for (int i=0;i<(int)line.num-1;i++) {
		if ((line[i].exp.back().name == "\\") or (line[i].exp.back().name == ",")) {
			// glue... (without \\ but with ,)
			if (line[i].exp.back().name == "\\")
				line[i].exp.pop();
			line[i].exp.append(line[i + 1].exp);
			// remove line
			line.erase(i + 1);
			i --;
			
		}
	}

	//show();

	
	// safety
	temp_line.exp.clear();
	line.add(temp_line);
	for (int i=0;i<line.num;i++) {
		ExpressionBuffer::Expression e;
		e.name = str_eol;
		e.pos = line[i].length;
		line[i].exp.add(e);
	}
}

void ExpressionBuffer::show() {
	for (int i=0;i<line.num;i++) {
		msg_write("--------------------");
		msg_write(line[i].indent);
		for (int j=0;j<line[i].exp.num;j++)
			msg_write(line[i].exp[j].name);
	}
}

// scan one line
//   true -> end of file
bool ExpressionBuffer::analyse_line(const char *source, ExpressionBuffer::Line *l, int &line_no) {
	int pos = 0;
	l->indent = 0;
	l->length = 0;
	l->exp.clear();

	for (int i=0;true;i++) {
		if (analyse_expression(source, pos, l, line_no))
			break;
	}
	l->length = pos;
	if (l->exp.num > 0)
		line.add(*l);
	return source[pos] == 0;
}

string strip_comments(const string &source) {
	auto lines = source.explode("\n");
	for (auto &l: lines) {
		if (l.find("#") >= 0)
			l = l.head(l.find("#"));
	}
	return implode(lines, "\n");
}

void ExpressionBuffer::do_asm_block(const char *source, int &pos, int &line_no) {
	int line_breaks = 0;
	// find beginning
	for (int i=0;i<1024;i++) {
		if (source[pos] == '{')
			break;
		if ((source[pos] != ' ') and (source[pos] != '\t') and (source[pos] != '\n'))
			syntax->do_error("'{' expected after 'asm'");
		if (source[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	pos ++;
	int asm_start = pos;
	
	// find end
	for (int i=0;i<65536;i++) {
		if (source[pos] == '}')
			break;
		if (source[pos] == 0)
			syntax->do_error("'}' expected to end \"asm\"");
		if (source[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	int asm_end = pos - 1;
	pos ++;

	AsmBlock a;
	a.line = cur_line->physical_line;
	a.block = strip_comments(string(&source[asm_start], asm_end - asm_start));
	syntax->asm_blocks.add(a);

	line_no += line_breaks;
}

// scan one line
// starts with <pos> and sets <pos> to first character after the expression
//   true -> end of line (<pos> is on newline)
bool ExpressionBuffer::analyse_expression(const char *source, int &pos, ExpressionBuffer::Line *l, int &line_no) {
	// skip whitespace and other "invisible" stuff to find the first interesting character

	for (int i=0;true;i++) {
		// end of file
		if (source[pos] == 0) {
			strcpy(Temp, "");
			return true;
		} else if (source[pos]=='\n') { // line break
			return true;
		} else if (source[pos]=='\t') { // tab
			if (l->exp.num == 0)
				l->indent ++;
		} else if (source[pos] == '#') { // single-line comment

			// dirty macro hack
			if ((source[pos+1] == 'd') and (source[pos+2] == 'e') and (source[pos+3] == 'f') and (source[pos+4] == 'i') and (source[pos+5] == 'n') and (source[pos+6] == 'e'))
				break;
			if ((source[pos+1] == 'i') and (source[pos+2] == 'm') and (source[pos+3] == 'm') and (source[pos+4] == 'o') and (source[pos+5] == 'r') and (source[pos+6] == 't'))
				break;

			// skip to end of line
			while (true) {
				pos ++;
				if ((source[pos] == '\n') or (source[pos] == 0))
					return true;
			}
		} else if ((source[pos] == 'a') and (source[pos + 1] == 's') and (source[pos + 2] == 'm')) { // asm block
			int pos0 = pos;
			pos += 3;
			do_asm_block(source, pos, line_no);
			insert("-asm-", pos0);
			return true;
		}
		exp_kind = GetKind(source[pos]);
		if (exp_kind != ExpKind::SPACING)
			break;
		pos ++;
	}
	int TempLength = 0;
	//int ExpStart=BufferPos;

	// string
	if (source[pos] == '\"') {
		for (int i=0; true; i++) {
			char c = Temp[TempLength ++] = source[pos ++];
			// end of string?
			if ((c == '\"') and (i > 0)) {
				break;
			} else if (c == 0) {
				syntax->do_error("string exceeds file");
			} else if (c == '\n') {
				line_no ++;
				//syntax->DoError("string exceeds line");
			} else {
				if (c == '{') {
					if (source[pos] == '{') {
						// string interpolation {{..}}
						for (int j=0; true; j++) {
							c = Temp[TempLength ++] = source[pos ++];
							if (c == 0)
								syntax->do_error("string interpolation exceeds file");
							if ((c == '}') and (Temp[TempLength-2] == '}'))
								break;
						}
						continue;
					}
				}

				// escape sequence
				if (c == '\\') {
					Temp[TempLength ++] = source[pos ++];
				}
				continue;
			}
		}

	// macro
	} else if ((source[pos] == '#') and (GetKind(source[pos + 1]) == ExpKind::LETTER)) {
		for (int i=0;i<SCRIPT_MAX_NAME;i++) {
			auto kind = GetKind(source[pos]);
			// may contain letters and numbers
			if ((i > 0) and (kind != ExpKind::LETTER) and (kind != ExpKind::NUMBER))
				break;
			Temp[TempLength ++] = source[pos ++];
		}

	// character
	} else if (source[pos] == '\'') {
		Temp[TempLength ++] = source[pos ++];
		if (source[pos] == '\\') {
			Temp[TempLength ++] = source[pos ++];
			Temp[TempLength ++] = source[pos ++];
		} else {
			Temp[TempLength ++] = source[pos ++];
		}
		Temp[TempLength ++] = source[pos ++];
		if (Temp[TempLength - 1] != '\'')
			syntax->do_error("character constant should end with '''");

	// word
	} else if (exp_kind == ExpKind::LETTER) {
		for (int i=0;i<SCRIPT_MAX_NAME;i++) {
			auto kind = GetKind(source[pos]);
			// may contain letters and numbers
			if ((kind != ExpKind::LETTER) and (kind != ExpKind::NUMBER))
				break;
			Temp[TempLength ++] = source[pos ++];
		}

	// number
	} else if (exp_kind == ExpKind::NUMBER) {
		bool hex = false;
		char last = 0;
		for (int i=0;true;i++) {
			char c = Temp[TempLength] = source[pos];
			// "0x..." -> hexadecimal
			if ((i == 1) and (Temp[0] == '0') and (Temp[1] == 'x'))
				hex = true;
			auto kind = GetKind(c);
			if (hex) {
				if ((i > 1) and (kind != ExpKind::NUMBER) and ((c < 'a') or (c > 'f')))
					break;
			} else {
				// may contain numbers and '.' or 'e'/'E'
				if ((kind != ExpKind::NUMBER) and (c != '.')) {
					if ((c != 'e') and (c != 'E'))
						if (((c != '-') and (c != '+')) or ((last != 'e') and (last != 'E')))
							break;
				}
			}
			TempLength ++;
			pos ++;
			last = c;
		}

	// symbol
	} else if (exp_kind == ExpKind::SIGN) {
		// mostly single-character symbols
		char c = Temp[TempLength ++] = source[pos ++];
		// double-character symbol
		if (((c == '=') and (source[pos] == '=')) or // ==
			((c == '!') and (source[pos] == '=')) or // !=
			((c == '<') and (source[pos] == '=')) or // <=
			((c == '>') and (source[pos] == '=')) or // >=
			((c == '+') and (source[pos] == '=')) or // +=
			((c == '-') and (source[pos] == '=')) or // -=
			((c == '*') and (source[pos] == '=')) or // *=
			((c == '/') and (source[pos] == '=')) or // /=
			((c == '+') and (source[pos] == '+')) or // ++
			((c == '-') and (source[pos] == '-')) or // --
			((c == '<') and (source[pos] == '<')) or // <<
			((c == '>') and (source[pos] == '>')) or // >>
			((c == '+') and (source[pos] == '+')) or // ++
			((c == '-') and (source[pos] == '-')) or // --
			((c == '-') and (source[pos] == '>')))   // ->
				Temp[TempLength ++] = source[pos ++];
	}

	Temp[TempLength] = 0;
	insert(Temp, pos - TempLength);

	return (source[pos] == '\n');
}

};
