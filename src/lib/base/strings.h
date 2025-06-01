#if !defined(STRINGS_H__INCLUDED_)
#define STRINGS_H__INCLUDED_

#include "array.h"

class Exception;


#define REGEX_MAX_MATCHES			128

struct string;

struct bytes : Array<char> {

	// constructors
	bytes();
	bytes(const bytes &s);
	bytes(bytes &&s) noexcept;
	bytes(const char *str);
	bytes(const void *str, int l);
	void __init__();
	~bytes();

	// functions
	void add(unsigned char c)
	{	append_1_single(c);	}
	void insert(int pos, unsigned char c)
	{	resize(num + 1);	for (int i=num-2;i>=pos;i--) (*this)[i+1] = (*this)[i];	(*this)[pos] = c;	}
	void erase(int index)
	{	delete_single(index);	}
	string hex() const;
	int hash() const;
	string md5() const;
	int compare(const bytes &s) const;
	bytes repeat(int n) const;
	bytes sub(int start, int end = MAGIC_END_INDEX) const;
	bytes sub_ref(int start, int end = MAGIC_END_INDEX) const;

	// operators
	void operator = (const bytes &s)
	{	simple_assign(&s);	}
	void operator = (bytes &&s);
	void operator += (const bytes &s)
	{	simple_append(&s);	}
	bytes operator + (const bytes &s) const
	{	bytes r = *this;	r += s;	return r;	}
	friend bytes operator + (const char *s1, const bytes &s2)
	{	return bytes(s1) + s2;	}
	bool operator == (const bytes &s) const;
	bool operator != (const bytes &s) const
	{	return !(*this == s);	}
	bool operator < (const bytes &s) const
	{	return compare(s) < 0;	}
	bool operator > (const bytes &s) const
	{	return compare(s) > 0;	}
	bool operator <= (const bytes &s) const
	{	return compare(s) <= 0;	}
	bool operator >= (const bytes &s) const
	{	return compare(s) >= 0;	}
	unsigned char operator[] (int index) const
	{	return ((unsigned char*)data)[index];	}
	unsigned char &operator[] (int index)
	{	return ((unsigned char*)data)[index];	}
	unsigned char &_cdecl back()
	{	return (*this)[num - 1];	}
	unsigned char back() const
	{	return (*this)[num - 1];	}
};

//--------------------------------------------------------------
// cool string class

struct string : bytes {

	// constructors
	string();
	string(const bytes &s);
	string(const string &s);
	string(bytes &&s);
	string(string &&s);
	string(const char *str);
	string(const void *str, int l);
#if __cpp_char8_t
	string(const char8_t *str);
#endif

	// functions
	string repeat(int n) const;
	int find(const string &s, int start = 0) const;
	int rfind(const string &s, int start = -1) const;
	bool has_char(char c) const;
	string sub(int start, int end = MAGIC_END_INDEX) const;
	string sub_ref(int start, int end = MAGIC_END_INDEX) const;
	string head(int size) const;
	string tail(int size) const;
	int icompare(const string &s) const;
	void _replace0(int start, int length, const string &s);
	string replace(const string &sub, const string &by) const;
	string reverse() const;
	string trim() const;
	Array<string> explode(const string &s) const;
	Array<string> split_any(const string &splitters) const;
	Array<string> parse_tokens(const string &splitters = "") const;
	string lower() const;
	string upper() const;
	string unhex() const;
	bool match(const string &glob) const;
	string repr() const;
	string escape() const;
	string unescape() const;
	int utf8len() const;
	string utf16_to_utf8() const;
	string latin_to_utf8() const;
	Array<int> utf8_to_utf32() const;
	Array<int> utf16_to_utf32() const;
	int _int() const;
	int64 i64() const;
	float _float() const;
	double f64() const;
	bool _bool() const;
	const char *c_str() const;


	// operators
	void operator = (const string &s)
	{	simple_assign(&s);	}
	void operator = (string &&s)
	{	exchange(s);	}
	void operator += (const string &s)
	{	simple_append(&s);	}
	string operator + (const string &s) const
	{	string r = *this;	r += s;	return r;	}
	friend string operator + (const char *s1, const string &s2)
	{	return string(s1) + s2;	}
#if __cpp_char8_t
	friend string operator + (const char8_t *s1, const string &s2)
	{	return string(s1) + s2;	}
#endif
	bool operator == (const string &s) const;
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


string i2s(int i);
string i642s(int64 i);
string i2s2(int i, int l);
string f2s(float f, int dez);
string f642s(double f, int dez);
string f2sf(float f);
string f642sf(double f);
string b2s(bool b);
string p2s(const void *p);

template<class T>
string str(const T &t) {
	return t.str();
}
template<> string str(const int& i);
template<> string str(const unsigned int& i);
template<> string str(const int64& i);
template<> string str(const float& f);
template<> string str(const double& d);
template<> string str(const bool& b);
template<> string str(const string& s);


template<class T>
string repr(const T &t) {
	if constexpr (std::is_same_v<T, string>)
		return t.repr();
	else
		return str(t);
}

template<> string repr(const string& s);

template<class T>
string str(const Array<T> &a) {
	string r;
	for (int i=0; i<a.num; i++) {
		if (i > 0)
			r += ", ";
		r += repr(a[i]);
	}
	return "[" + r + "]";
}


int s2i(const string &s);
bool s2b(const string &s);
float s2f(const string &s);
double s2f64(const string &s);

bool str_is_integer(const string &s);
bool str_is_float(const string &s);

string d2h(const void *data, int bytes);
string i2h(int64, int bytes);
string h2d(const string &hex_str, int bytes);

string implode(const Array<string> &a, const string &glue);
string utf32_to_utf8(const Array<int> &s);


bool sa_contains(const Array<string> &a, const string &s);

//--------------------------------------------------------------
// regular expressions

/*extern char *regex_out_match[REGEX_MAX_MATCHES];
extern int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];


int regex_match(char *rex,char *str,int max_matches=1);
char *regex_replace(char *rex,char *str,int max_matches=0);*/


#endif
