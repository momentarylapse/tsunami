#include "../../base/base.h"
#include "../../file/file.h"
#include "lexical.h"
#include "syntax_tree.h"
#include <stdio.h>

namespace Kaba{

//#define ScriptDebug

static char Temp[MAX_STRING_CONST_LENGTH];
static int ExpKind;

char str_eol[] = "-eol-";

#define SCRIPT_MAX_NAME	256

// type of expression (syntax)
enum
{
	ExpKindNumber,
	ExpKindLetter,
	ExpKindSpacing,
	ExpKindSign
};


ExpressionBuffer::ExpressionBuffer():
	cur(dummy)
{
	clear();
}

string ExpressionBuffer::get_name(int n)
{
	return cur_line->exp[n].name;
}

int ExpressionBuffer::get_line_no()
{
	foreachi(Line &l, line, i)
		if (cur_line == &l)
			return i;
	return -1;
}

void ExpressionBuffer::next()
{
	cur_exp ++;
	cur = cur_line->exp[cur_exp].name;
}

void ExpressionBuffer::rewind()
{
	cur_exp --;
	cur = cur_line->exp[cur_exp].name;
}

bool ExpressionBuffer::end_of_line()
{
	return (cur_exp >= cur_line->exp.num - 1); // the last entry is "-eol-"#
}

bool ExpressionBuffer::past_end_of_line()
{
	return (cur_exp >= cur_line->exp.num);
}

void ExpressionBuffer::next_line()
{
	cur_line ++;
	cur_exp = 0;
	test_indent(cur_line->indent);
	cur = cur_line->exp[cur_exp].name;
}

void ExpressionBuffer::set(int exp_no, int line_no)
{
	if (line_no < 0)
		line_no = get_line_no();
	cur_line = &line[line_no];
	cur_exp = exp_no;
	cur = cur_line->exp[cur_exp].name;
}

bool ExpressionBuffer::end_of_file()
{
	return ((long)cur_line >= (long)&line[line.num - 1]); // last line = "-eol-"*/
}

void ExpressionBuffer::reset_parser()
{
	cur_line = &line[0];
	cur_exp = 0;
	cur = cur_line->exp[cur_exp].name;
	reset_indent();
}

void ExpressionBuffer::clear()
{
	cur_line = NULL;
	line.clear();
	cur_line = &temp_line;
	cur_exp = -1;
	comment_level = 0;
}

void ExpressionBuffer::add_line()
{
	Line l;
	line.add(l);
	cur_line = &line.back();
}

void ExpressionBuffer::insert(const char *_name, int pos, int index)
{
	Expression e;
	e.name = _name;
	e.pos = pos;
	if (index < 0)
		// at the end...
		cur_line->exp.add(e);
	else
		cur_line->exp.insert(e, index);
}

void ExpressionBuffer::remove(int index)
{
	cur_line->exp.erase(index);
}

void ExpressionBuffer::test_indent(int i)
{
	indented = (i > indent_0);
	unindented = (i < indent_0);
	indent_0 = i;

}

void ExpressionBuffer::reset_indent()
{
	indented = unindented = false;
	indent_0 = 0;
}

int GetKind(char c)
{
	if (isNumber(c))
		return ExpKindNumber;
	/*else if (isLetter(c))
		return ExpKindLetter;*/
	else if (isSpacing(c))
		return ExpKindSpacing;
	else if (isSign(c))
		return ExpKindSign;
	else if (c == 0)
		return -1;
	// allow all other characters as letters
	return ExpKindLetter;
}

void ExpressionBuffer::Analyse(SyntaxTree *ps, const string &source)
{
	syntax = ps;
	clear();

	// scan all lines
	const char *buf = (char*)source.data;
	for (int i=0;true;i++){
		//exp_add_line(&Exp);
		cur_line->physical_line = i;
		if (AnalyseLine(buf, cur_line, i))
			break;
		buf += cur_line->length + 1;
	}

	// glue together lines ending with a "\" or ","
	for (int i=0;i<(int)line.num-1;i++){
		if ((line[i].exp.back().name == "\\") || (line[i].exp.back().name == ",")){
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
	for (int i=0;i<line.num;i++){
		ExpressionBuffer::Expression e;
		e.name = str_eol;
		e.pos = line[i].length;
		line[i].exp.add(e);
	}
}

void ExpressionBuffer::show()
{
	for (int i=0;i<line.num;i++){
		msg_write("--------------------");
		msg_write(line[i].indent);
		for (int j=0;j<line[i].exp.num;j++)
			msg_write(line[i].exp[j].name);
	}
}

// scan one line
//   true -> end of file
bool ExpressionBuffer::AnalyseLine(const char *source, ExpressionBuffer::Line *l, int &line_no)
{
	int pos = 0;
	l->indent = 0;
	l->length = 0;
	l->exp.clear();

	for (int i=0;true;i++){
		if (AnalyseExpression(source, pos, l, line_no))
			break;
	}
	l->length = pos;
	if (l->exp.num > 0)
		line.add(*l);
	return source[pos] == 0;
}

// reads at most one line
//   returns true if end_of_line is reached
bool ExpressionBuffer::DoMultiLineComment(const char *source, int &pos)
{
	while(true){
		if (source[pos] == '\n')
			return true;
		if ((source[pos] == '/') && (source[pos + 1] == '*')){
			pos ++;
			comment_level ++;
		}else if ((source[pos] == '*') && (source[pos + 1] == '/')){
			comment_level --;
			pos ++;
			if (comment_level == 0){
				pos ++;
				return false;
			}
		}else if (source[pos] == 0){
			syntax->DoError("comment exceeds end of file");
		}
		pos ++;
	}
	//ExpKind = ExpKindSpacing;
	return false;
}

void ExpressionBuffer::DoAsmBlock(const char *source, int &pos, int &line_no)
{
	int line_breaks = 0;
	// find beginning
	for (int i=0;i<1024;i++){
		if (source[pos] == '{')
			break;
		if ((source[pos] != ' ') && (source[pos] != '\t') && (source[pos] != '\n'))
			syntax->DoError("'{' expected after \"asm\"");
		if (source[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	pos ++;
	int asm_start = pos;
	
	// find end
	for (int i=0;i<65536;i++){
		if (source[pos] == '}')
			break;
		if (source[pos] == 0)
			syntax->DoError("'}' expected to end \"asm\"");
		if (source[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	int asm_end = pos - 1;
	pos ++;

	AsmBlock a;
	a.line = cur_line->physical_line;
	a.block = string(&source[asm_start], asm_end - asm_start);
	syntax->asm_blocks.add(a);

	line_no += line_breaks;
}

// scan one line
// starts with <pos> and sets <pos> to first character after the expression
//   true -> end of line (<pos> is on newline)
bool ExpressionBuffer::AnalyseExpression(const char *source, int &pos, ExpressionBuffer::Line *l, int &line_no)
{
	// skip whitespace and other "invisible" stuff to find the first interesting character
	if (comment_level > 0)
		if (DoMultiLineComment(source, pos))
			return true;

	for (int i=0;true;i++){
		// end of file
		if (source[pos] == 0){
			strcpy(Temp, "");
			return true;
		}else if (source[pos]=='\n'){ // line break
			return true;
		}else if (source[pos]=='\t'){ // tab
			if (l->exp.num == 0)
				l->indent ++;
		}else if ((source[pos] == '/') && (source[pos + 1]=='/')){ // single-line comment
			// skip to end of line
			while (true){
				pos ++;
				if ((source[pos] == '\n') || (source[pos] == 0))
					return true;
			}
		}else if ((source[pos] == '/') && (source[pos + 1] == '*')){ // multi-line comment
			if (DoMultiLineComment(source, pos))
				return true;
			ExpKind = ExpKindSpacing;
			continue;
		}else if ((source[pos] == 'a') && (source[pos + 1] == 's') && (source[pos + 2] == 'm')){ // asm block
			int pos0 = pos;
			pos += 3;
			DoAsmBlock(source, pos, line_no);
			insert("-asm-", pos0);
			return true;
		}
		ExpKind = GetKind(source[pos]);
		if (ExpKind != ExpKindSpacing)
			break;
		pos ++;
	}
	int TempLength = 0;
	//int ExpStart=BufferPos;

	// string
	if (source[pos] == '\"'){
		msg_db_m("string", 1);
		for (int i=0;true;i++){
			char c = Temp[TempLength ++] = source[pos ++];
			// end of string?
			if ((c == '\"') && (i > 0)){
				break;
			}else if (c == 0){
				syntax->DoError("string exceeds file");
			}else if (c == '\n'){
				line_no ++;
				//syntax->DoError("string exceeds line");
			}else{
				// escape sequence
				if (c == '\\'){
					if (source[pos] == '\\')
						Temp[TempLength - 1] = '\\';
					else if (source[pos] == '\"')
						Temp[TempLength - 1] = '\"';
					else if (source[pos] == 'n')
						Temp[TempLength - 1] = '\n';
					else if (source[pos] == 'r')
						Temp[TempLength - 1] = '\r';
					else if (source[pos] == 't')
						Temp[TempLength - 1] = '\t';
					else if (source[pos] == '0')
						Temp[TempLength - 1] = '\0';
					else
						syntax->DoError("unknown escape in string");
					pos ++;
				}
				continue;
			}
		}

	// macro
	}else if ((source[pos] == '#') && (GetKind(source[pos + 1]) == ExpKindLetter)){
		msg_db_m("macro", 4);
		for (int i=0;i<SCRIPT_MAX_NAME;i++){
			int kind = GetKind(source[pos]);
			// may contain letters and numbers
			if ((i > 0) && (kind != ExpKindLetter) && (kind != ExpKindNumber))
				break;
			Temp[TempLength ++] = source[pos ++];
		}

	// character
	}else if (source[pos] == '\''){
		msg_db_m("char", 4);
		Temp[TempLength ++] = source[pos ++];
		Temp[TempLength ++] = source[pos ++];
		Temp[TempLength ++] = source[pos ++];
		if (Temp[TempLength - 1] != '\'')
			syntax->DoError("character constant should end with '''");

	// word
	}else if (ExpKind == ExpKindLetter){
		msg_db_m("word", 4);
		for (int i=0;i<SCRIPT_MAX_NAME;i++){
			int kind = GetKind(source[pos]);
			// may contain letters and numbers
			if ((kind != ExpKindLetter) && (kind != ExpKindNumber))
				break;
			Temp[TempLength ++] = source[pos ++];
		}

	// number
	}else if (ExpKind == ExpKindNumber){
		msg_db_m("num", 4);
		bool hex = false;
		char last = 0;
		for (int i=0;true;i++){
			char c = Temp[TempLength] = source[pos];
			// "0x..." -> hexadecimal
			if ((i == 1) && (Temp[0] == '0') && (Temp[1] == 'x'))
				hex = true;
			int kind = GetKind(c);
			if (hex){
				if ((i > 1) && (kind != ExpKindNumber) && ((c < 'a') || (c > 'f')))
					break;
			}else{
				// may contain numbers and '.' or 'e'/'E'
				if ((kind != ExpKindNumber) and (c != '.')){
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
	}else if (ExpKind == ExpKindSign){
		msg_db_m("sym", 4);
		// mostly single-character symbols
		char c = Temp[TempLength ++] = source[pos ++];
		// double-character symbol
		if (((c == '=') && (source[pos] == '=')) || // ==
			((c == '!') && (source[pos] == '=')) || // !=
			((c == '<') && (source[pos] == '=')) || // <=
			((c == '>') && (source[pos] == '=')) || // >=
			((c == '+') && (source[pos] == '=')) || // +=
			((c == '-') && (source[pos] == '=')) || // -=
			((c == '*') && (source[pos] == '=')) || // *=
			((c == '/') && (source[pos] == '=')) || // /=
			((c == '+') && (source[pos] == '+')) || // ++
			((c == '-') && (source[pos] == '-')) || // --
			((c == '&') && (source[pos] == '&')) || // &&
			((c == '|') && (source[pos] == '|')) || // ||
			((c == '<') && (source[pos] == '<')) || // <<
			((c == '>') && (source[pos] == '>')) || // >>
			((c == '+') && (source[pos] == '+')) || // ++
			((c == '-') && (source[pos] == '-')) || // --
			((c == '-') && (source[pos] == '>'))) // ->
				Temp[TempLength ++] = source[pos ++];
	}

	Temp[TempLength] = 0;
	insert(Temp, pos - TempLength);

	return (source[pos] == '\n');
}

};
