#include "../file/file.h"

#ifndef __ANY_INCLUDED__
#define __ANY_INCLUDED__

enum {
	TYPE_NONE,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_BOOL,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_HASH,
	TYPE_POINTER
};

class AnyMap;

class Any {
public:
	Any();
	Any(const Any &a);
	Any(int i);
	Any(float f);
	Any(bool b);
	Any(const string &s);
	Any(const Array<Any> &a);
	Any(const AnyMap &m);
	Any(const void *p);
	~Any();
	void _cdecl clear();
	void create_type(int type);
	string _cdecl str() const;
	string _cdecl repr() const;
	int _cdecl _int() const;
	float _cdecl _float() const;
	bool _cdecl _bool() const;
	Any &_cdecl operator = (const Any &a);
	Any _cdecl operator + (const Any &a) const;
	Any _cdecl operator - (const Any &a) const;
	void _cdecl operator += (const Any &a);

	// array
	void _cdecl add(const Any &a);
	void _cdecl append(const Any &a);
	const Any &operator[] (int index) const;
	Any &operator[] (int index);
	Any &_cdecl back();
	int length();
	
	int* as_int() const;
	float* as_float() const;
	bool* as_bool() const;
	string* as_string() const;
	Array<Any>* as_array() const;
	AnyMap* as_map() const;
	const void** as_pointer() const;

	// hash map
	const Any &operator[] (const string &key) const;
	Any &operator[] (const string &key);
	Array<string> keys() const;

	// data
	int type;
	void *data;

	// kaba
	void _cdecl __init__();
	void _cdecl set(const Any &a){	*this = a;	}
	/*void _cdecl set_int(int i){	*this = i;	}
	void _cdecl set_float(float f){	*this = f;	}
	void _cdecl set_bool(bool b){	*this = b;	}
	void _cdecl set_str(const string &s){	*this = s;	}
	void _cdecl set_array(const Array<Any> &a){	*this = a;	}
	void _cdecl set_map(const AnyMap &m){	*this = m;	}*/
	void _cdecl _add(const Any &a){	Any b = *this + a;	*this = b;	}
	void _cdecl _sub(const Any &a){	Any b = *this - a;	*this = b;	}
	Any _cdecl array_get(int i) const;
	void _cdecl array_set(int i, const Any &value);
	Any _cdecl map_get(const string &key) const;
	void _cdecl map_set(const string &key, const Any &value);
};


extern Any EmptyVar;
extern Any EmptyHash;
extern Any EmptyArray;

void print(const Any &a);



#endif

