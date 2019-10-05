
#include "../kaba.h"
#include "common.h"
#include "exception.h"
#include "dynamic.h"
#include "../../base/map.h"

namespace Kaba {

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

void script_make_dict(Class *t, SyntaxTree *ps) {
	const Class *p = t->param;
	t->derive_from(TypeDictBase, false);
	t->param = p;
	add_class(t);

	if (p->is_simple_class()) {
		// elements don't need a destructor
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Map<string,int>::clear);
		class_add_funcx("clear", TypeVoid, &Map<string,int>::clear);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &IntDict::assign);
			func_add_param("other", t);
	}

	if (p == TypeInt) {
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,int>::__init__);
		class_add_funcx("set", TypeVoid, &IntDict::set_int);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_funcx("__get__", p, &IntDict::get_int, FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_funcx("str", TypeString, &IntDict::str);
	} else if (p == TypeFloat32) {
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,float>::__init__);
		class_add_funcx("set", TypeVoid, &FloatDict::set_float);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_funcx("__get__", p, &FloatDict::get_float, FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_funcx("str", TypeString, &FloatDict::str);
	} else if (p == TypeString) {
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,string>::__init__);
		class_add_funcx("set", TypeVoid, &Map<string,string>::set);
			func_add_param("key", TypeString);
			func_add_param("x", p);
		class_add_funcx("__get__", p, &StringDict::get_string, FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Map<string,string>::clear);
		class_add_funcx("clear", TypeVoid, &Map<string,string>::clear);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &StringDict::assign);
			func_add_param("other", t);
		class_add_funcx("str", TypeString, &StringDict::str);
	}
}



}