
#ifndef LEXICAL_H_
#define LEXICAL_H_

namespace Kaba{

class SyntaxTree;

// character buffer and expressions (syntax analysis)

class ExpressionBuffer {
public:
	ExpressionBuffer();

	struct Expression {
		string name;
		int pos;
	};

	struct Line {
		int physical_line, length, indent;
		Array<Expression> exp;
	};

	Array<Line> line;
	Line temp_line;
	Line *cur_line;
	int cur_exp;
	string dummy;
	string &cur;
	SyntaxTree *syntax;

	string get_name(int n);
	int get_line_no();
	void next();
	void rewind();
	bool end_of_line();
	bool past_end_of_line();
	void next_line();
	bool end_of_file();
	void set(int exp_no, int line = -1);

	void clear();
	void reset_parser();

	void add_line();
	void insert(const char *name, int pos, int index = -1);
	void remove(int index);

	int indent_0;
	bool indented, unindented;
	void test_indent(int i);
	void reset_indent();

	void show();

	void analyse(SyntaxTree *ps, const string &source);
	bool analyse_expression(const char *source, int &pos, ExpressionBuffer::Line *l, int &line_no);
	bool analyse_line(const char *source, ExpressionBuffer::Line *l, int &line_no);
	void analyse_logical_line(const char *source, ExpressionBuffer::Line *l, int &line_no);
	void do_asm_block(const char *source, int &pos, int &line_no);
};


inline bool is_number(char c) {
	if ((c >= 48) and (c <= 57))
		return true;
	return false;
}

inline bool is_letter(char c) {
	if ((c >= 'a') and (c <= 'z'))
		return true;
	if ((c >= 'A') and (c <= 'Z'))
		return true;
	if ((c == '_'))
		return true;
	// Umlaute
#ifdef OS_WINDOWS
	// Windows-Zeichensatz
	if ((c==-28) or (c==-10) or (c==-4) or (c==-33) or (c==-60) or (c==-42) or (c==-36))
		return true;
#endif
#ifdef OS_LINUX
	// Linux-Zeichensatz??? testen!!!!
#endif
	return false;
}

inline bool is_spacing(char c) {
	if ((c == ' ') or (c == '\t') or (c == '\n'))
		return true;
	return false;
}

inline bool is_sign(char c) {
	if ((c=='.') or (c==':') or (c==',') or (c==';') or (c=='+') or (c=='-') or (c=='*') or (c=='%') or (c=='/') or (c=='=') or (c=='<') or (c=='>') or (c=='^') or (c=='\''))
		return true;
	if ((c=='(') or (c==')') or (c=='{') or (c=='}') or (c=='&') or (c=='|') or (c=='!') or (c=='[') or (c==']') or (c=='\"') or (c=='\\') or (c=='#') or (c=='?') or (c=='$'))
		return true;
	return false;
}

};

#endif /* LEXICAL_H_ */
