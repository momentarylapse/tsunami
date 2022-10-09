
#ifndef SRC_LIB_KABA_PARSER_LEXICAL_H_
#define SRC_LIB_KABA_PARSER_LEXICAL_H_

#include "../../base/base.h"

namespace kaba {

class SyntaxTree;

// character buffer and expressions (syntax analysis)

class ExpressionBuffer {
public:
	ExpressionBuffer();

	struct Token {
		string name;
		int pos;
	};

	struct Line {
		int physical_line, length, indent;
		Array<Token> tokens;
		Array<int> token_ids;
	};

	Array<Line> lines;

	Line temp_line;
	string dummy;
	SyntaxTree *syntax;


	string get_token(int id) const;
	Line *token_logical_line(int id) const;
	int token_physical_line_no(int id) const;
	int token_line_offset(int id) const;
	int token_index_in_line(int id) const;

	void clear();
	bool empty() const;

	void add_line();
	void insert(const char *name, int pos, int index = -1);
	void remove(int index);
	void erase_logical_line(int line_no);

	string line_str(Line *l) const;
	void show();

	void analyse(SyntaxTree *ps, const string &source);
	bool analyse_expression(const char *source, int &pos, ExpressionBuffer::Line *l, int &line_no);
	bool analyse_line(const char *source, ExpressionBuffer::Line *l, int &line_no);
	void analyse_logical_line(const char *source, ExpressionBuffer::Line *l, int &line_no);
	void do_asm_block(const char *source, int &pos, int &line_no);

	void merge_logical_lines();
	void update_meta_data();

	// walker
	Line *cur_line;
	int _cur_exp;
	string &cur;
	string peek_next() const;

	void reset_walker();
	int next_line_indent() const;

	int cur_token() const;
	int consume_token();
	void next();
	string consume();
	void rewind();
	bool end_of_line() const;
	bool almost_end_of_line() const;
	bool past_end_of_line() const;
	void next_line();
	bool end_of_file() const;
	void jump(int token_id);

	std::function<void()> do_error_endl;
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
	if (c == '_')
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

#endif /* SRC_LIB_KABA_PARSER_LEXICAL_H_ */
