#include "../../base/base.h"
#include "../../os/msg.h"
#include "lexical.h"
#include "../syntax/SyntaxTree.h"
#include "../Context.h"
#include <stdio.h>

namespace kaba {

//#define ScriptDebug

static const int MAX_STRING_CONST_LENGTH = 65536;
static char Temp[MAX_STRING_CONST_LENGTH];

static const char str_eol[] = "<eol>";

static const int SCRIPT_MAX_NAME = 256;

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
}

string ExpressionBuffer::peek_next() const {
	return cur_line->tokens[_cur_exp + 1].name;
}

int ExpressionBuffer::cur_token() const {
	return cur_line->token_ids[_cur_exp];
}

int ExpressionBuffer::consume_token() {
	if (end_of_line())
		do_error_endl();
	int token = cur_token();
	next();
	return token;
}

string ExpressionBuffer::get_token(int id) const {
	if (auto l = token_logical_line(id))
		return l->tokens[id - l->first_token_id].name;
	return "";
}

ExpressionBuffer::Line *ExpressionBuffer::token_logical_line(int id) const {
	int i0 = 0;
	int i1 = lines.num - 1;
	while (i0 < i1) {
		int m = (i0 + i1) >> 1;
		if (id < lines[m].first_token_id)
			i1 = m - 1;
		else if (id > lines[m].last_token_id)
			i0 = m + 1;
		else
			return &lines[m];
	}
	return &lines[i0];
}

int ExpressionBuffer::token_physical_line_no(int id) const {
	if (auto l = token_logical_line(id))
		return l->physical_line;
	return -1;
}

int ExpressionBuffer::token_line_offset(int id) const {
	if (auto l = token_logical_line(id))
		return l->tokens[id - l->first_token_id].pos;
	return -1;
}

int ExpressionBuffer::token_index_in_line(int id) const {
	if (auto l = token_logical_line(id))
		return id - l->first_token_id;
	return -1;
}

void ExpressionBuffer::next() {
	_cur_exp ++;

	if (_cur_exp < cur_line->tokens.num)
		cur = cur_line->tokens[_cur_exp].name;
	//else
	//	cur = "<eol>";
}

string ExpressionBuffer::consume() {
	if (end_of_line())
		do_error_endl();
	auto s = cur;
	next();
	return s;
}

void ExpressionBuffer::rewind() {
	jump(cur_token() - 1);
}

bool ExpressionBuffer::end_of_line() const {
	return (_cur_exp >= cur_line->tokens.num - 1); // the last entry is "<eol>"
}

bool ExpressionBuffer::almost_end_of_line() const {
	return (_cur_exp >= cur_line->tokens.num - 2); // the last entry is "-eol-"#
}

bool ExpressionBuffer::past_end_of_line() const {
	return (_cur_exp >= cur_line->tokens.num);
}

void ExpressionBuffer::next_line() {
	cur_line ++;
	_cur_exp = 0;
	cur = cur_line->tokens[_cur_exp].name;
}

void ExpressionBuffer::jump(int token_id) {
	cur_line = token_logical_line(token_id);
	_cur_exp = token_index_in_line(token_id);
	cur = cur_line->tokens[_cur_exp].name;
}

bool ExpressionBuffer::end_of_file() const {
	return (int_p)cur_line >= (int_p)&lines.back(); // last line = "-eol-"*/
}

void ExpressionBuffer::reset_walker() {
	cur_line = &lines[0];
	_cur_exp = 0;
	cur = cur_line->tokens[_cur_exp].name;
}

bool ExpressionBuffer::empty() const {
	return lines[0].tokens.num <= 1;
}

void ExpressionBuffer::clear() {
	cur_line = nullptr;
	lines.clear();
	cur_line = &temp_line;
	_cur_exp = -1;
}

void ExpressionBuffer::add_line() {
	Line l;
	lines.add(l);
	cur_line = &lines.back();
}

void ExpressionBuffer::insert(const char *_name, int pos, int index) {
	Token e;
	e.name = _name;
	e.pos = pos;
	if (index < 0)
		// at the end...
		cur_line->tokens.add(e);
	else
		cur_line->tokens.insert(e, index);
}

void ExpressionBuffer::remove(int index) {
	cur_line->tokens.erase(index);
}

int ExpressionBuffer::next_line_indent() const {
	return (cur_line + 1)->indent;
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

void ExpressionBuffer::erase_logical_line(int line_no) {
	lines.erase(line_no);
	update_meta_data();
}

void ExpressionBuffer::update_meta_data() {
	int id0 = 0;
	for (auto &l: lines) {
		l.first_token_id = id0;
		l.last_token_id = id0 + l.tokens.num - 1;
		l.token_ids.clear();
		for (int i=0; i<l.tokens.num; i++)
			l.token_ids.add(id0 + i);
		id0 += l.tokens.num;
	}

}

void ExpressionBuffer::merge_logical_lines() {
	// glue together lines ending with a "\" or ","
	auto should_glue = [] (const Line &cur, const Line& next) {
		if ((cur.tokens.back().name == "\\") or (cur.tokens.back().name == ","))
			return true;
		if (next.tokens[0].name == "|>")
			return true;
		return false;
	};

	for (int i=0; i<(int)lines.num-1; i++) {
		if (should_glue(lines[i], lines[i+1])) {
			// glue... (without \\ but with ,)
			if (lines[i].tokens.back().name == "\\")
				lines[i].tokens.pop();
			lines[i].tokens.append(lines[i + 1].tokens);
			// remove line
			lines.erase(i + 1);
			i --;

		}
	}
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

	merge_logical_lines();

	//show();

	
	// safety
	temp_line.tokens.clear();
	temp_line.indent = 0;
	lines.add(temp_line);
	for (auto &l: lines) {
		Token e;
		e.name = str_eol;
		e.pos = l.length;
		l.tokens.add(e);
	}

	update_meta_data();
}

string ExpressionBuffer::line_str(Line *l) const {
	string s;
	for (auto &t: l->tokens) {
		if (s != "")
			s += "  ";
		s += t.name;
	}
	return format("%d  %d   ", l->physical_line, l->indent) + s;
}

void ExpressionBuffer::show() {
	for (auto &l: lines)
		msg_write(line_str(&l));
}

// scan one line
//   true -> end of file
bool ExpressionBuffer::analyse_line(const char *source, ExpressionBuffer::Line *l, int &line_no) {
	int pos = 0;
	l->indent = 0;
	l->length = 0;
	l->tokens.clear();

	for (int i=0;true;i++) {
		if (analyse_expression(source, pos, l, line_no))
			break;
	}
	l->length = pos;
	if (l->tokens.num > 0)
		lines.add(*l);
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
			do_error_analyse("'{' expected after 'asm'", pos, line_no);
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
			do_error_analyse("'}' expected to end \"asm\"", pos, line_no);
		if (source[pos] == '\n')
			line_breaks ++;
		pos ++;
	}
	int asm_end = pos - 1;
	pos ++;

	AsmBlock a;
	static int next_asm_uuid = 0;
	a.uuid = next_asm_uuid ++;
	a.line = cur_line->physical_line;
	a.block = strip_comments(string(&source[asm_start], asm_end - asm_start));
	syntax->asm_blocks.add(a);

	line_no += line_breaks;
}

void ExpressionBuffer::do_error_analyse(const string& msg, int pos, int line_no) {
	throw Exception(msg, "?", line_no, pos, syntax->module);
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
			if (l->tokens.num == 0)
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
				do_error_analyse("string exceeds file", pos, line_no);
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
								do_error_analyse("string interpolation exceeds file", pos, line_no);
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
			do_error_analyse("character constant should end with '''", pos, line_no);

	// word
	} else if (exp_kind == ExpKind::LETTER) {
		for (int i=0;i<SCRIPT_MAX_NAME;i++) {
			auto kind = GetKind(source[pos]);
			// may contain letters and numbers and '!'
			if ((kind != ExpKind::LETTER) and (kind != ExpKind::NUMBER)) {
				if (source[pos] != '!')
					break;
				// allow '!' only in some special words
				string t = string(Temp, TempLength);
				if (t != "shared" and t != "owned" and t != "xfer")
					break;
			}
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
			((c == '|') and (source[pos] == '>')) or // |>
			((c == '=') and (source[pos] == '>')) or // =>
			((c == '-') and (source[pos] == '>')) or // ->
			((c == '&') and (source[pos] == '!')) or // &!
			((c == ':') and (source[pos] == '=')))   // :=
				Temp[TempLength ++] = source[pos ++];
	}

	Temp[TempLength] = 0;
	insert(Temp, pos - TempLength);

	return (source[pos] == '\n');
}

};
