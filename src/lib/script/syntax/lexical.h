
#ifndef LEXICAL_H_
#define LEXICAL_H_

namespace Script{

class SyntaxTree;

// character buffer and expressions (syntax analysis)

struct ExpressionBuffer
{
	ExpressionBuffer();

	struct Expression
	{
		string name;
		int pos;
	};

	struct Line
	{
		int physical_line, length, indent;
		Array<Expression> exp;
	};

	Array<Line> line;
	Line temp_line;
	Line *cur_line;
	int cur_exp;
	int comment_level;
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

	void Analyse(SyntaxTree *ps, const string &source);
	bool AnalyseExpression(const char *source, int &pos, ExpressionBuffer::Line *l, int &line_no);
	bool AnalyseLine(const char *source, ExpressionBuffer::Line *l, int &line_no);
	void AnalyseLogicalLine(const char *source, ExpressionBuffer::Line *l, int &line_no);
	void DoAsmBlock(const char *source, int &pos, int &line_no);
	bool DoMultiLineComment(const char *source, int &pos);
};


inline bool isNumber(char c)
{
	if ((c>=48)&&(c<=57))
		return true;
	return false;
}

inline bool isLetter(char c)
{
	if ((c>='a')&&(c<='z'))
		return true;
	if ((c>='A')&&(c<='Z'))
		return true;
	if ((c=='_'))
		return true;
	// Umlaute
#ifdef OS_WINDOWS
	// Windows-Zeichensatz
	if ((c==-28)||(c==-10)||(c==-4)||(c==-33)||(c==-60)||(c==-42)||(c==-36))
		return true;
#endif
#ifdef OS_LINUX
	// Linux-Zeichensatz??? testen!!!!
#endif
	return false;
}

inline bool isSpacing(char c)
{
	if ((c==' ')||(c=='\t')||(c=='\n'))
		return true;
	return false;
}

inline bool isSign(char c)
{
	if ((c=='.')||(c==':')||(c==',')||(c==';')||(c=='+')||(c=='-')||(c=='*')||(c=='%')||(c=='/')||(c=='=')||(c=='<')||(c=='>')||(c=='\''))
		return true;
	if ((c=='(')||(c==')')||(c=='{')||(c=='}')||(c=='&')||(c=='|')||(c=='!')||(c=='[')||(c==']')||(c=='\"')||(c=='\\')||(c=='#')||(c=='?')||(c=='$'))
		return true;
	return false;
}

};

#endif /* LEXICAL_H_ */
