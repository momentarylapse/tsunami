#include "script.h"
#include "../file/file.h"
#include <stdio.h>

//#define ScriptDebug

static char Temp[1024];
static int ExpKind;

char str_eol[] = "-eol-";

#define SCRIPT_MAX_NAME	256

void clear_exp_buffer(ps_exp_buffer_t *e)
{
	e->cur_line = NULL;
	for (int i=0;i<e->line.num;i++)
		e->line[i].exp.clear();
	e->line.clear();
	if (e->buffer){
		delete[]e->buffer;
		e->buffer = NULL;
	}
	e->cur_line = &e->temp_line;
	e->cur_exp = -1;
	e->comment_level = 0;
}

inline void exp_add_line(ps_exp_buffer_t *e)
{
	ps_line_t l;
	e->line.add(l);
	e->cur_line = &e->line.back();
}

inline void insert_into_buffer(CPreScript *ps, const char *name, int pos, int index = -1)
{
	ps_exp_t e;
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

inline void remove_from_buffer(CPreScript *ps, int index)
{
	ps->Exp.cur_line->exp.erase(index);
}

int CPreScript::GetKind(char c)
{
	if (isNumber(c))
		return ExpKindNumber;
	else if (isLetter(c))
		return ExpKindLetter;
	else if (isSpacing(c))
		return ExpKindSpacing;
	else if (isSign(c))
		return ExpKindSign;
	else if (c==0)
		return -1;

/*	msg_write("evil char");
	insert_into_buffer(this, format("   '%c' (%2x)   ", c, c).c_str(), 0);
	msg_write(Exp.cur_exp);
	msg_write(p2s(cur_name));
	msg_write(cur_name);
	//Exp.cur_exp = Exp.cur_line->exp.num;
	//sprintf(Exp.cur_line->exp[Exp.cur_exp].name, "   '%c' (%2x)   ", c, c);
	//sprintf(cur_name, "   '%c' (%2x)   ", c, c);
	DoError("evil character found!");
	return -1;*/
	return ExpKindLetter;
}

void CPreScript::Analyse(const char *buffer, bool just_analyse)
{
	msg_db_r("Analyse", 4);
	clear_exp_buffer(&Exp);
	Exp.buffer = new char[strlen(buffer)*2];
	Exp.buf_cur = Exp.buffer;

	// scan all lines
	const char *buf = buffer;
	for (int i=0;true;i++){
		//exp_add_line(&Exp);
		Exp.cur_line->physical_line = i;
		if (AnalyseLine(buf, Exp.cur_line, i, just_analyse))
			break;
		if (Error){
			msg_db_l(4);
			return;
		}
		buf += Exp.cur_line->length + 1;
	}

	// glue together lines ending with a "\" or ","
	for (int i=0;i<(int)Exp.line.num-1;i++){
		if ((strcmp(Exp.line[i].exp.back().name, "\\") == 0) || (strcmp(Exp.line[i].exp.back().name, ",") == 0)){
			int d = (strcmp(Exp.line[i].exp.back().name, "\\") == 0) ? 1 : 0;
			// glue... (without \\ but with ,)
			for (int j=d;j<Exp.line[i + 1].exp.num;j++){
				ps_exp_t e;
				e.name = Exp.line[i + 1].exp[j].name;
				e.pos = 0; // Exp.line[i + 1].exp[j].name;
				Exp.line[i].exp.add(e);
			}
			// remove line
			Exp.line.erase(i + 1);
			i --;
			
		}
	}

	/*for (int i=0;i<Exp.line.num;i++){
		msg_write("--------------------");
		msg_write(Exp.line[i].indent);
		for (int j=0;j<Exp.line[i].exp.num;j++)
			msg_write(Exp.line[i].exp[j].name);
	}*/

	
	// safety
	Exp.temp_line.exp.clear();
	Exp.line.add(Exp.temp_line);
	for (int i=0;i<Exp.line.num;i++){
		ps_exp_t e;
		e.name = str_eol;
		e.pos = Exp.line[i].length;
		Exp.line[i].exp.add(e);
	}
	
	msg_db_l(4);
}

// scan one line
//   true -> end of file
bool CPreScript::AnalyseLine(const char *buffer, ps_line_t *l, int &line_no, bool just_analyse)
{
	msg_db_r("AnalyseLine", 4);
	int pos = 0;
	l->indent = 0;
	l->length = 0;
	l->exp.clear();

	for (int i=0;true;i++){
		if (AnalyseExpression(buffer, pos, l, line_no, just_analyse))
			break;
		if (Error){
			msg_db_l(4);
			return false;
		}
	}
	l->length = pos;
	if (l->exp.num > 0)
		Exp.line.add(*l);
	msg_db_l(4);
	return (buffer[pos] == 0);
}

// reads at most one line
//   returns true if end_of_line is reached
bool DoMultiLineComment(CPreScript *ps, const char *buffer, int &pos)
{
	while(true){
		if (buffer[pos] == '\n')
			return true;
		if ((buffer[pos] == '/') && (buffer[pos + 1] == '*')){
			pos ++;
			ps->Exp.comment_level ++;
		}else if ((buffer[pos] == '*') && (buffer[pos + 1] == '/')){
			ps->Exp.comment_level --;
			pos ++;
			if (ps->Exp.comment_level == 0){
				pos ++;
				return false;
			}
		}else if ((buffer[pos] == 0)){// || (BufferPos>=BufferLength)){
			ps->DoError("comment exceeds end of file");
			return true;
		}
		pos ++;
	}
	//ExpKind = ExpKindSpacing;
}

void DoAsmBlock(CPreScript *ps, const char *buffer, int &pos, int &line_no)
{
	int line_breaks = 0;
	// find beginning
	for (int i=0;i<1024;i++){
		if (buffer[pos] == '{')
			break;
		if ((buffer[pos] != ' ') && (buffer[pos] != '\t') && (buffer[pos] != '\n')){
			ps->DoError("'{' expected after \"asm\"");
			return;
		}
		if (buffer[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	pos ++;
	int asm_start = pos;
	
	// find end
	for (int i=0;i<65536;i++){
		if (buffer[pos] == '}')
			break;
		if (buffer[pos] == 0){
			ps->DoError("'}' expected to end \"asm\"");
			return;
		}
		if (buffer[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	int asm_end = pos - 1;
	pos ++;

	sAsmBlock a;
	a.Line = ps->Exp.cur_line->physical_line;
	a.block = new char[asm_end - asm_start + 1];
	memcpy(a.block, &buffer[asm_start], asm_end - asm_start);
	a.block[asm_end - asm_start] = 0;
	ps->AsmBlock.add(a);

	line_no += line_breaks;
}

// scan one line
// starts with <pos> and sets <pos> to first character after the expression
//   true -> end of line (<pos> is on newline)
bool CPreScript::AnalyseExpression(const char *buffer, int &pos, ps_line_t *l, int &line_no, bool just_analyse)
{
	msg_db_r("AnalyseExpression", 4);
	// skip whitespace and other "invisible" stuff to find the first interesting character
	if (Exp.comment_level > 0)
		if (DoMultiLineComment(this, buffer, pos))
			_return_(4, true);

	for (int i=0;true;i++){
		// end of file
		if (buffer[pos] == 0){
			strcpy(Temp, "");
			_return_(4, true);
		}else if (buffer[pos]=='\n'){ // line break
			_return_(4, true);
		}else if (buffer[pos]=='\t'){ // tab
			if (l->exp.num == 0)
				l->indent ++;
		}else if ((buffer[pos] == '/') && (buffer[pos + 1]=='/')){ // single-line comment
			// skip to end of line
			while (true){
				pos ++;
				if ((buffer[pos] == '\n') || (buffer[pos] == 0))
					_return_(4, true);
			}
		}else if ((buffer[pos] == '/') && (buffer[pos + 1] == '*')){ // multi-line comment
			if (DoMultiLineComment(this, buffer, pos))
				_return_(4, true);
			ExpKind = ExpKindSpacing;
			continue;
		}else if ((buffer[pos] == 'a') && (buffer[pos + 1] == 's') && (buffer[pos + 2] == 'm')){ // asm block
			int pos0 = pos;
			pos += 3;
			DoAsmBlock(this, buffer, pos, line_no);
			if (Error)	_return_(4, true);
			insert_into_buffer(this, "-asm-", pos0);
			_return_(4, true);
		}
		ExpKind = GetKind(buffer[pos]);
		if (Error)
			_return_(4, false);
		if (ExpKind != ExpKindSpacing)
			break;
		pos ++;
	}
	int TempLength = 0;
	//int ExpStart=BufferPos;

	// string
	if (buffer[pos] == '\"'){
		msg_db_m("string", 1);
		for (int i=0;true;i++){
			char c = Temp[TempLength ++] = buffer[pos ++];
			// end of string?
			if ((c == '\"') && (i > 0))
				break;
			else if ((c == '\n') || (c == 0)){
				_do_error_("string exceeds line", 4, false);
			}else{
				// escape sequence
				if (c == '\\'){
					if (buffer[pos] == '\\')
						Temp[TempLength - 1] = '\\';
					else if (buffer[pos] == '\"')
						Temp[TempLength - 1] = '\"';
					else if (buffer[pos] == 'n')
						Temp[TempLength - 1] = '\n';
					else if (buffer[pos] == 'r')
						Temp[TempLength - 1] = '\r';
					else if (buffer[pos] == 't')
						Temp[TempLength - 1] = '\t';
					else if (buffer[pos] == '0')
						Temp[TempLength - 1] = '\0';
					else
						_do_error_("unknown escape in string", 4, false);
					pos ++;
				}
				continue;
			}
		}

	// macro
	}else if ((buffer[pos] == '#') && (GetKind(buffer[pos + 1]) == ExpKindLetter)){
		msg_db_m("macro", 4);
		for (int i=0;i<SCRIPT_MAX_NAME;i++){
			int kind = GetKind(buffer[pos]);
			// may contain letters and numbers
			if ((i > 0) && (kind != ExpKindLetter) && (kind != ExpKindNumber))
				break;
			Temp[TempLength ++] = buffer[pos ++];
		}

	// character
	}else if (buffer[pos] == '\''){
		msg_db_m("char", 4);
		Temp[TempLength ++] = buffer[pos ++];
		Temp[TempLength ++] = buffer[pos ++];
		Temp[TempLength ++] = buffer[pos ++];
		if (Temp[TempLength - 1] != '\'')
			_do_error_("character constant should end with '''", 4, false);

	// word
	}else if (ExpKind == ExpKindLetter){
		msg_db_m("word", 4);
		for (int i=0;i<SCRIPT_MAX_NAME;i++){
			int kind = GetKind(buffer[pos]);
			// may contain letters and numbers
			if ((kind != ExpKindLetter) && (kind != ExpKindNumber))
				break;
			Temp[TempLength ++] = buffer[pos ++];
		}

	// number
	}else if (ExpKind == ExpKindNumber){
		msg_db_m("num", 4);
		bool hex = false;
		for (int i=0;true;i++){
			char c = Temp[TempLength] = buffer[pos];
			// "0x..." -> hexadecimal
			if ((i == 1) && (Temp[0] == '0') && (Temp[1] == 'x'))
				hex = true;
			int kind = GetKind(c);
			if (hex){
				if ((i > 1) && (kind != ExpKindNumber) && ((c < 'a') || (c > 'f')))
					break;
			}else{
				// may contain numbers and '.' or 'f'
				if ((kind != ExpKindNumber) && (c != '.'))// && (c != 'f'))
					break;
			}
			TempLength ++;
			pos ++;
		}

	// symbol
	}else if (ExpKind == ExpKindSign){
		msg_db_m("sym", 4);
		// mostly single-character symbols
		char c = Temp[TempLength ++] = buffer[pos ++];
		// double-character symbol
		if (((c == '=') && (buffer[pos] == '=')) || // ==
			((c == '!') && (buffer[pos] == '=')) || // !=
			((c == '<') && (buffer[pos] == '=')) || // <=
			((c == '>') && (buffer[pos] == '=')) || // >=
			((c == '+') && (buffer[pos] == '=')) || // +=
			((c == '-') && (buffer[pos] == '=')) || // -=
			((c == '*') && (buffer[pos] == '=')) || // *=
			((c == '/') && (buffer[pos] == '=')) || // /=
			((c == '+') && (buffer[pos] == '+')) || // ++
			((c == '-') && (buffer[pos] == '-')) || // --
			((c == '&') && (buffer[pos] == '&')) || // &&
			((c == '|') && (buffer[pos] == '|')) || // ||
			((c == '<') && (buffer[pos] == '<')) || // <<
			((c == '>') && (buffer[pos] == '>')) || // >>
			((c == '+') && (buffer[pos] == '+')) || // ++
			((c == '-') && (buffer[pos] == '-')) || // --
			((c == '-') && (buffer[pos] == '>'))) // ->
				Temp[TempLength ++] = buffer[pos ++];
	}

	Temp[TempLength] = 0;
	insert_into_buffer(this, Temp, pos - TempLength);

	msg_db_l(4);
	return (buffer[pos] == '\n');
}
