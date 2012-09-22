#if !defined(STRINGS_H__INCLUDED_)
#define STRINGS_H__INCLUDED_

#include "array.h"


#define REGEX_MAX_MATCHES			128

//--------------------------------------------------------------
// cool string class

class string : public DynamicArray
{
	public:

	// constructors
	string();
	string(const string &s);
	string(const char *str);
	string(const char *str, int l);
	void __init__();
	~string();

	// functions
	void add(char c)
	{	append_1_single(c);	}
	void insert(int pos, char c)
	{	resize(num + 1);	for (int i=num-2;i>=pos;i--) (*this)[i+1] = (*this)[i];	(*this)[pos] = c;	}
	void erase(int index)
	{	delete_single(index);	}
	int find(const string &s, int start = 0) const;
	int rfind(const string &s, int start = -1) const;
	string substr(int start, int length) const;
	string head(int size) const;
	string tail(int size) const;
	int compare(const string &s) const;
	int icompare(const string &s) const;
	void replace0(int start, int length, const string &s);
	string _replace(const string &sub, const string &by) const;
	string _reverse() const;
	string trim() const;
	Array<string> explode(const string &s) const;
	string lower() const;
	string upper() const;
	string hex(bool inverted = false) const;
	string unhex() const;
	int hash() const;
	int _int() const;
	float _float() const;
	const char *c_str() const;

	// for paths
	string sys_filename() const;
	string dirname() const;
	string basename() const;
	void dir_ensure_ending();
	string no_recursion() const;
	string extension() const;

	// operators
	void operator = (const string &s)
	//{	printf("= assign %p = %p", data, s.data);	assign(&s);	printf(" /=  '%s\n", c_str());}
	{	assign(&s);	}
	void operator += (const string &s)
	{	append(&s);	}
	string operator + (const string &s) const
	{	string r = *this;	r += s;	return r;	}
	friend string operator + (const char *s1, const string &s2)
	{	return string(s1) + s2;	}
	bool operator == (const string &s) const
	{
		if (num != s.num)
			return false;
		char *a = (char*)data;
		char *b = (char*)s.data;
		for (int i=0;i<num;i++){
			if (*a != *b)
				return false;
			a ++;
			b ++;
		}
		return true;
	}
	bool operator != (const string &s) const
	{	return !(*this == s);	}
	bool operator < (const string &s) const
	{	return compare(s) < 0;	}
	bool operator > (const string &s) const
	{	return compare(s) > 0;	}
	bool operator <= (const string &s) const
	{	return compare(s) <= 0;	}
	bool operator >= (const string &s) const
	{	return compare(s) >= 0;	}
	char operator[] (int index) const
	{	return ((char*)data)[index];	}
	char &operator[] (int index)
	{	return ((char*)data)[index];	}
	char &back()
	{	return (*this)[num - 1];	}
	char back() const
	{	return (*this)[num - 1];	}
};


//--------------------------------------------------------------
// string operations


string format(const string s, ...);
string i2s(int i);
string i2s2(int i, int l);
string f2s(float f, int dez);
string f2sf(float f);
string b2s(bool b);
string p2s(void *p);
string ia2s(const Array<int> &a);
string fa2s(const Array<float> &a);
string ba2s(const Array<bool> &a);
string sa2s(const Array<string> &a);
int s2i(const string &s);
float s2f(const string &s);

string d2h(const void *data, int bytes, bool inverted = true);
string h2d(const string &hex_str, int bytes);

string implode(const Array<string> &a, const string &glue);

//--------------------------------------------------------------
// regular expressions

/*extern char *regex_out_match[REGEX_MAX_MATCHES];
extern int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];


int regex_match(char *rex,char *str,int max_matches=1);
char *regex_replace(char *rex,char *str,int max_matches=0);*/


#endif
