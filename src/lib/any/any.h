#include "../base/base.h"

#ifndef __ANY_INCLUDED__
#define __ANY_INCLUDED__


class AnyMap;

class Any {
public:

	enum {
		TYPE_NONE,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_STRING,
		TYPE_ARRAY,
		TYPE_MAP,
		TYPE_POINTER
	};

	Any();
	Any(const Any &a);
	Any(int i);
	Any(float f);
	Any(bool b);
	Any(const string &s);
	Any(const char *s);
	Any(const void *p);
	Any(const Array<Any> &a);
	Any(const Array<int> &a);
	Any(const AnyMap &m);
	//Any(const void *p);
	~Any();
	void _cdecl clear();
	Any ref();
	void sync_to_parent();
	void sync_from_parent();
	void create_type(int type);

	bool is_empty() const;
	bool is_string() const;
	bool is_int() const;
	bool is_float() const;
	bool is_bool() const;
	bool is_pointer() const;
	bool is_array() const;
	bool is_map() const;

	string _cdecl str() const;
	string _cdecl repr() const;

	static Any parse(const string &s);


	int _cdecl _int() const;
	float _cdecl _float() const;
	bool _cdecl _bool() const;
	void _cdecl operator = (const Any &a);
	Any _cdecl operator + (const Any &a) const;
	Any _cdecl operator - (const Any &a) const;
	void _cdecl operator += (const Any &a);
	bool operator==(const Any& other) const;
	bool operator!=(const Any& other) const;

	// array
	void _cdecl add(const Any &a);
	void _cdecl append(const Any &a);
	const Any &operator[] (int index) const;
	Any &operator[] (int index);
	Any &_cdecl back();
	int length() const;
	
	int& as_int() const;
	float& as_float() const;
	bool& as_bool() const;
	string& as_string() const;
	Array<Any>& as_array() const;
	AnyMap& as_map() const;
	const void*& as_pointer() const;

	// map/dict
	const Any &operator[] (const string &key) const;
	Any &operator[] (const string &key);
	Array<string> keys() const;
	bool has(const string &key) const;

	// data
	int type;
	void *data;
	Any *parent;
	const void *_class;

	// kaba
	void _cdecl __init__();
	void _cdecl __delete__();
	void _cdecl set(const Any &a){	*this = a;	}
	/*void _cdecl set_int(int i){	*this = i;	}
	void _cdecl set_float(float f){	*this = f;	}
	void _cdecl set_bool(bool b){	*this = b;	}
	void _cdecl set_str(const string &s){	*this = s;	}
	void _cdecl set_array(const Array<Any> &a){	*this = a;	}
	void _cdecl set_map(const AnyMap &m){	*this = m;	}*/
	void _cdecl _add(const Any &a){	Any b = *this + a;	*this = b;	}
	void _cdecl _sub(const Any &a){	Any b = *this - a;	*this = b;	}
	Any _cdecl array_get(int i);
	void _cdecl array_set(int i, const Any &value);
	Any _cdecl map_get(const string &key);
	void _cdecl map_set(const string &key, const Any &value);
	void _cdecl map_drop(const string &key);


	static Any EmptyMap;
	static Any EmptyArray;
	static bool allow_simple_output;
};

template<> string repr(const Any& a);




#endif

