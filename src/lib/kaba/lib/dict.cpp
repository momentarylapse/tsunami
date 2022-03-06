
#include "../kaba.h"
#include "lib.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"
#include "../../base/map.h"

namespace kaba {

extern const Class *TypeDictBase;
extern const Class *TypeIntDict;
extern const Class *TypeFloatDict;
extern const Class *TypeStringDict;

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

class IntDict : public Map<string,int> {
public:
	void set_int(const string &k, int v)
	{ set(k, v); }
	int get_int(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return 0; }
	void assign(const IntDict &o)
	{ *this = o; }
	string str()
	{ return var2str(this, TypeIntDict); }
};

class FloatDict : public Map<string,float> {
public:
	void set_float(const string &k, float v)
	{ set(k, v); }
	float get_float(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return 0.0f; }
	string str()
	{ return var2str(this, TypeFloatDict); }
};

class StringDict : public Map<string,string> {
public:
	string get_string(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return ""; }
	void assign(const StringDict &o)
	{ *this = o; }
	string str()
	{ return var2str(this, TypeStringDict); }
};
#pragma GCC pop_options

void kaba_make_dict(Class *t, SyntaxTree *ps) {
	const Class *p = t->param[0];
	t->derive_from(TypeDictBase, false);
	t->param[0] = p;
	add_class(t);

	if (p->can_memcpy()) {
		// elements don't need a destructor
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &Map<string,int>::clear);
		class_add_func("clear", TypeVoid, &Map<string,int>::clear);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &IntDict::assign);
			func_add_param("other", t);
	}

	if (p == TypeInt) {
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,int>::__init__);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &IntDict::set_int);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_func(IDENTIFIER_FUNC_GET, p, &IntDict::get_int, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &IntDict::str, Flags::PURE);
	} else if (p == TypeFloat32) {
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,float>::__init__);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &FloatDict::set_float);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_func(IDENTIFIER_FUNC_GET, p, &FloatDict::get_float, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &FloatDict::str, Flags::PURE);
	} else if (p == TypeString) {
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,string>::__init__);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &Map<string,string>::set);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_func(IDENTIFIER_FUNC_GET, p, &StringDict::get_string, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &Map<string,string>::clear);
		class_add_func("clear", TypeVoid, &Map<string,string>::clear);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &StringDict::assign);
			func_add_param("other", t);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &StringDict::str, Flags::PURE);
	}
}



}
