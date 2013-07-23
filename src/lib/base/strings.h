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
	void _cdecl __init__();
	~string();

	// functions
	void _cdecl add(char c)
	{	append_1_single(c);	}
	void _cdecl insert(int pos, char c)
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
	int _cdecl hash() const;
	int _cdecl _int() const;
	float _cdecl _float() const;
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
	{	assign(&s);	}
	void _cdecl operator += (const string &s)
	{	append(&s);	}
	string _cdecl operator + (const string &s) const
	{	string r = *this;	r += s;	return r;	}
	friend string _cdecl operator + (const char *s1, const string &s2)
	{	return string(s1) + s2;	}
	bool _cdecl operator == (const string &s) const
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
	char operator[] (int index) const
	{	return ((char*)data)[index];	}
	char &operator[] (int index)
	{	return ((char*)data)[index];	}
	char &_cdecl back()
	{	return (*this)[num - 1];	}
	char _cdecl back() const
	{	return (*this)[num - 1];	}
};


//--------------------------------------------------------------
// string operations


string _cdecl format(const string s, ...);
string _cdecl i2s(int i);
string _cdecl i2s2(int i, int l);
string _cdecl f2s(float f, int dez);
string _cdecl f2sf(float f);
string _cdecl b2s(bool b);
string _cdecl p2s(void *p);
string _cdecl ia2s(const Array<int> &a);
string _cdecl fa2s(const Array<float> &a);
string _cdecl ba2s(const Array<bool> &a);
string _cdecl sa2s(const Array<string> &a);
int _cdecl s2i(const string &s);
float _cdecl s2f(const string &s);

string _cdecl d2h(const void *data, int bytes, bool inverted = true);
string _cdecl h2d(const string &hex_str, int bytes);

string _cdecl implode(const Array<string> &a, const string &glue);

//--------------------------------------------------------------
// regular expressions

/*extern char *regex_out_match[REGEX_MAX_MATCHES];
extern int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];


int regex_match(char *rex,char *str,int max_matches=1);
char *regex_replace(char *rex,char *str,int max_matches=0);*/


#endif
