#if !defined(STRINGS_H__INCLUDED_)
#define STRINGS_H__INCLUDED_

#include "array.h"


#define REGEX_MAX_MATCHES			128

//--------------------------------------------------------------
// cool string class

class string : public Array<char> {
	public:

	// constructors
	string();
	string(const string &s);
	string(string &&s);
	string(const char *str);
	string(const void *str, int l);
	void _cdecl __init__();
	~string();

	// functions
	void _cdecl add(unsigned char c)
	{	append_1_single(c);	}
	void _cdecl insert(int pos, unsigned char c)
	{	resize(num + 1);	for (int i=num-2;i>=pos;i--) (*this)[i+1] = (*this)[i];	(*this)[pos] = c;	}
	void _cdecl erase(int index)
	{	delete_single(index);	}
	int _cdecl find(const string &s, int start = 0) const;
	int _cdecl rfind(const string &s, int start = -1) const;
	string _cdecl substr(int start, int length) const;
	string _cdecl head(int size) const;
	string _cdecl tail(int size) const;
	int _cdecl compare(const string &s) const;
	int _cdecl icompare(const string &s) const;
	void _cdecl replace0(int start, int length, const string &s);
	string _cdecl replace(const string &sub, const string &by) const;
	string _cdecl reverse() const;
	string _cdecl trim() const;
	Array<string> _cdecl explode(const string &s) const;
	string _cdecl lower() const;
	string _cdecl upper() const;
	string _cdecl hex(bool inverted = false) const;
	string _cdecl unhex() const;
	bool _cdecl match(const string &glob) const;
	string repr() const;
	string escape() const;
	string unescape() const;
	int _cdecl utf8len() const;
	string utf16_to_utf8() const;
	string latin_to_utf8() const;
	Array<int> utf8_to_utf32() const;
	Array<int> utf16_to_utf32() const;
	int _cdecl hash() const;
	string _cdecl md5() const;
	int _cdecl _int() const;
	long long _cdecl i64() const;
	float _cdecl _float() const;
	double _cdecl f64() const;
	bool _cdecl _bool() const;
	const char *c_str() const;

	// for paths
	string _cdecl sys_filename() const;
	string _cdecl dirname() const;
	string _cdecl basename() const;
	void _cdecl dir_ensure_ending();
	string _cdecl no_recursion() const;
	string _cdecl extension() const;

	// operators
	void _cdecl operator = (const string &s)
	//{	printf("= assign %p = %p", data, s.data);	assign(&s);	printf(" /=  '%s\n", c_str());}
	{	simple_assign(&s);	}
	void _cdecl operator = (string &&s)
	{	exchange(s);	}
	void _cdecl operator += (const string &s)
	{	simple_append(&s);	}
	string _cdecl operator + (const string &s) const
	{	string r = *this;	r += s;	return r;	}
	friend string _cdecl operator + (const char *s1, const string &s2)
	{	return string(s1) + s2;	}
	bool _cdecl operator == (const string &s) const
	{
		if (num != s.num)
			return false;
		unsigned char *a = (unsigned char*)data;
		unsigned char *b = (unsigned char*)s.data;
		for (int i=0;i<num;i++){
			if (*a != *b)
				return false;
			a ++;
			b ++;
		}
		return true;
	}
	bool _cdecl operator != (const string &s) const
	{	return !(*this == s);	}
	bool _cdecl operator < (const string &s) const
	{	return compare(s) < 0;	}
	bool _cdecl operator > (const string &s) const
	{	return compare(s) > 0;	}
	bool _cdecl operator <= (const string &s) const
	{	return compare(s) <= 0;	}
	bool _cdecl operator >= (const string &s) const
	{	return compare(s) >= 0;	}
	unsigned char operator[] (int index) const
	{	return ((unsigned char*)data)[index];	}
	unsigned char &operator[] (int index)
	{	return ((unsigned char*)data)[index];	}
	unsigned char &_cdecl back()
	{	return (*this)[num - 1];	}
	unsigned char _cdecl back() const
	{	return (*this)[num - 1];	}
};


//--------------------------------------------------------------
// string operations


string _cdecl format(const string &s, ...);
string _cdecl i2s(int i);
string _cdecl i642s(long long i);
string _cdecl i2s2(int i, int l);
string _cdecl f2s(float f, int dez);
string _cdecl f642s(double f, int dez);
string _cdecl f2sf(float f);
string _cdecl f642sf(double f);
string _cdecl b2s(bool b);
string _cdecl p2s(const void *p);
string _cdecl ia2s(const Array<int> &a);
string _cdecl fa2s(const Array<float> &a);
string _cdecl ba2s(const Array<bool> &a);
string _cdecl sa2s(const Array<string> &a);
int _cdecl s2i(const string &s);
bool _cdecl s2b(const string &s);
float _cdecl s2f(const string &s);
double _cdecl s2f64(const string &s);

string _cdecl d2h(const void *data, int bytes, bool inverted = true);
string _cdecl h2d(const string &hex_str, int bytes);

string _cdecl implode(const Array<string> &a, const string &glue);
string _cdecl utf32_to_utf8(const Array<int> &s);



string _cdecl str_unescape(const string &str);
string _cdecl str_escape(const string &str);

string _cdecl str_m_to_utf8(const string &str);
string _cdecl str_utf8_to_m(const string &str);

bool _cdecl sa_contains(const Array<string> &a, const string &s);

//--------------------------------------------------------------
// regular expressions

/*extern char *regex_out_match[REGEX_MAX_MATCHES];
extern int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];


int regex_match(char *rex,char *str,int max_matches=1);
char *regex_replace(char *rex,char *str,int max_matches=0);*/


#endif
