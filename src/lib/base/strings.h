#if !defined(STRINGS_H__INCLUDED_)
#define STRINGS_H__INCLUDED_

#include "array.h"

class Exception;


#define REGEX_MAX_MATCHES			128

class string;

class bytes : public Array<char> {
	public:

	// constructors
	bytes();
	bytes(const bytes &s);
	bytes(bytes &&s);
	bytes(const char *str);
	bytes(const void *str, int l);
	void _cdecl __init__();
	~bytes();

	// functions
	void _cdecl add(unsigned char c)
	{	append_1_single(c);	}
	void _cdecl insert(int pos, unsigned char c)
	{	resize(num + 1);	for (int i=num-2;i>=pos;i--) (*this)[i+1] = (*this)[i];	(*this)[pos] = c;	}
	void _cdecl erase(int index)
	{	delete_single(index);	}
	string _cdecl hex() const;
	int _cdecl hash() const;
	string _cdecl md5() const;
	int _cdecl compare(const bytes &s) const;
	bytes _cdecl repeat(int n) const;
	bytes _cdecl sub(int start, int end = MAGIC_END_INDEX) const;
	bytes _cdecl sub_ref(int start, int end = MAGIC_END_INDEX) const;

	// operators
	void _cdecl operator = (const bytes &s)
	{	simple_assign(&s);	}
	void _cdecl operator = (bytes &&s)
	{	exchange(s);	}
	void _cdecl operator += (const bytes &s)
	{	simple_append(&s);	}
	bytes _cdecl operator + (const bytes &s) const
	{	bytes r = *this;	r += s;	return r;	}
	friend bytes _cdecl operator + (const char *s1, const bytes &s2)
	{	return bytes(s1) + s2;	}
	bool _cdecl operator == (const bytes &s) const;
	bool _cdecl operator != (const bytes &s) const
	{	return !(*this == s);	}
	bool _cdecl operator < (const bytes &s) const
	{	return compare(s) < 0;	}
	bool _cdecl operator > (const bytes &s) const
	{	return compare(s) > 0;	}
	bool _cdecl operator <= (const bytes &s) const
	{	return compare(s) <= 0;	}
	bool _cdecl operator >= (const bytes &s) const
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
// cool string class

class string : public bytes {
	public:

	// constructors
	string();
	string(const bytes &s);
	string(const string &s);
	string(bytes &&s);
	string(string &&s);
	string(const char *str);
	string(const void *str, int l);

	// functions
	string _cdecl repeat(int n) const;
	int _cdecl find(const string &s, int start = 0) const;
	int _cdecl rfind(const string &s, int start = -1) const;
	bool has_char(char c) const;
	string _cdecl sub(int start, int end = MAGIC_END_INDEX) const;
	string _cdecl sub_ref(int start, int end = MAGIC_END_INDEX) const;
	string _cdecl head(int size) const;
	string _cdecl tail(int size) const;
	int _cdecl icompare(const string &s) const;
	void _cdecl _replace0(int start, int length, const string &s);
	string _cdecl replace(const string &sub, const string &by) const;
	string _cdecl reverse() const;
	string _cdecl trim() const;
	Array<string> _cdecl explode(const string &s) const;
	Array<string> _cdecl split_any(const string &splitters) const;
	Array<string> _cdecl parse_tokens(const string &splitters = "") const;
	string _cdecl lower() const;
	string _cdecl upper() const;
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
	int _cdecl _int() const;
	long long _cdecl i64() const;
	float _cdecl _float() const;
	double _cdecl f64() const;
	bool _cdecl _bool() const;
	const char *c_str() const;


	// operators
	void _cdecl operator = (const string &s)
	{	simple_assign(&s);	}
	void _cdecl operator = (string &&s)
	{	exchange(s);	}
	void _cdecl operator += (const string &s)
	{	simple_append(&s);	}
	string _cdecl operator + (const string &s) const
	{	string r = *this;	r += s;	return r;	}
	friend string _cdecl operator + (const char *s1, const string &s2)
	{	return string(s1) + s2;	}
	bool _cdecl operator == (const string &s) const;
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
};


//--------------------------------------------------------------
// string operations


template<typename T>
string _xf_str_(const string &f, const T value);

bool _xf_split_first_(const string &s, string &pre, string &f, string &post);

template<typename T, typename... Args>
string format(const string &s, T value, Args... args) {
	string pre, f, post;
	if (_xf_split_first_(s, pre, f, post)) {
		string t = _xf_str_(f, value);
		return pre + t + format(post, args...);
	}
	return pre;
}

string format(const string &s);


string _cdecl i2s(int i);
string _cdecl i642s(int64 i);
string _cdecl i2s2(int i, int l);
string _cdecl f2s(float f, int dez);
string _cdecl f642s(double f, int dez);
string _cdecl f2sf(float f);
string _cdecl f642sf(double f);
string _cdecl b2s(bool b);
string _cdecl p2s(const void *p);
string _cdecl ia2s(const Array<int> &a);
string _cdecl fa2s(const Array<float> &a);
string _cdecl f64a2s(const Array<double> &a);
string _cdecl ba2s(const Array<bool> &a);
string _cdecl sa2s(const Array<string> &a);
int _cdecl s2i(const string &s);
bool _cdecl s2b(const string &s);
float _cdecl s2f(const string &s);
double _cdecl s2f64(const string &s);

string _cdecl d2h(const void *data, int bytes);
string _cdecl i2h(int64, int bytes);
string _cdecl h2d(const string &hex_str, int bytes);

string _cdecl implode(const Array<string> &a, const string &glue);
string _cdecl utf32_to_utf8(const Array<int> &s);


bool _cdecl sa_contains(const Array<string> &a, const string &s);

//--------------------------------------------------------------
// regular expressions

/*extern char *regex_out_match[REGEX_MAX_MATCHES];
extern int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];


int regex_match(char *rex,char *str,int max_matches=1);
char *regex_replace(char *rex,char *str,int max_matches=0);*/


#endif
