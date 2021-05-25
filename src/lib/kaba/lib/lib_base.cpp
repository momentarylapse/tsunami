#include "../kaba.h"
#include "lib.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"
#include "../../file/file.h"
#include <algorithm>
#include <math.h>
#include <cstdio>


namespace kaba {

extern const Class *TypeDynamicArray;
extern const Class *TypeSharedPointer;
const Class *TypeAbstractList;
const Class *TypeAbstractDict;
const Class *TypeAbstractTuple;
extern const Class *TypeStringAutoCast;
extern const Class *TypeDictBase;
extern const Class *TypeFloat;
extern const Class *TypePointerList;
extern const Class *TypeObject;
extern const Class *TypeObjectP;
extern const Class *TypeBoolPs;
extern const Class *TypeBoolList;
extern const Class *TypeIntPs;
extern const Class *TypeIntP;
extern const Class *TypeIntList;
extern const Class *TypeIntArray;
extern const Class *TypeFloatP;
extern const Class *TypeFloatPs;
extern const Class *TypeFloatArray;
extern const Class *TypeFloatArrayP;
extern const Class *TypeFloatList;
extern const Class *TypeCharPs;
extern const Class *TypeStringList;
extern const Class *TypeIntDict;
extern const Class *TypeFloatDict;
extern const Class *TypeStringDict;
extern const Class *TypeAny;


static string kaba_print_postfix = "\n";
void _cdecl kaba_print(const string &str)
{	printf("%s%s", str.c_str(), kaba_print_postfix.c_str()); fflush(stdout);	}
void _cdecl kaba_cstringout(char *str)
{	kaba_print(str);	}
bytes _cdecl kaba_binary(const char *p, int length)
{	return bytes(p, length);	}
int _cdecl _Float2Int(float f)
{	return (int)f;	}
double _cdecl _Float2Float64(float f)
{	return (double)f;	}
float _cdecl _Float642Float(double f)
{	return (float)f;	}
float _cdecl _Int2Float(int i)
{	return (float)i;	}
int _cdecl _Int642Int(int64 i)
{	return (int)i;	}
int64 _cdecl _Int2Int64(int i)
{	return (int64)i;	}
char _cdecl _Int2Char(int i)
{	return (char)i;	}
int _cdecl _Char2Int(char c)
{	return (int)c;	}
bool _cdecl _Pointer2Bool(void *p)
{	return (p != nullptr);	}



static void kaba_int_out(int a) {
	msg_write("out: " + i2s(a));
}

static void kaba_float_out(float a) {
	msg_write("float out..." + d2h(&a, 4));
	msg_write("out: " + f2s(a, 6));
}

static float kaba_float_ret() {
	return 13.0f;
}

static void _x_call_float() {
	kaba_float_out(13);
}

static int kaba_int_ret() {
	return 2001;
}

static void kaba_xxx(int a, int b, int c, int d, int e, int f, int g, int h) {
	msg_write(format("xxx  %d %d %d %d %d %d %d %d", a, b, c, d, e, f, g, h));
}

static int extern_variable1 = 13;




MAKE_OP_FOR(int)
MAKE_OP_FOR(float)
MAKE_OP_FOR(int64)
MAKE_OP_FOR(double)

int op_int_mod(int a, int b) { return a % b; }
int op_int_shr(int a, int b) { return a >> b; }
int op_int_shl(int a, int b) { return a << b; }
int64 op_int64_mod(int64 a, int64 b) { return a % b; }
int64 op_int64_shr(int64 a, int64 b) { return a >> b; }
int64 op_int64_shl(int64 a, int64 b) { return a << b; }
int64 op_int64_add_int(int64 a, int b) { return a + b; }


class StringList : public Array<string> {
public:
	void _cdecl assign(StringList &s) {
		*this = s;
	}
	string _cdecl join(const string &glue) {
		return implode(*this, glue);
	}
	bool __contains__(const string &s) {
		return this->find(s) >= 0;
	}
	bool __eq__(const StringList &s) {
		return *this == s;
	}
	bool __neq__(const StringList &s) {
		return *this != s;
	}
	Array<string> __add__(const Array<string> &o) {
		return *this + o;
	}
	void __adds__(const Array<string> &o) {
		append(o);
	}
	string str() const {
		return sa2s(*this);
	}
};

string kaba_int_format(int i, const string &fmt) {
	try {
		if (fmt.tail(1) == "x")
			return _xf_str_<int>(fmt, i);
		return _xf_str_<int>(fmt + "d", i);
	} catch(::Exception &e) {
		return "{ERROR: " + e.message() + "}";
	}
}

string kaba_float2str(float f) {
	return f2s(f, 6);
}

string kaba_float_format(float f, const string &fmt) {
	try {
		return _xf_str_<float>(fmt + "f", f);
	} catch(::Exception &e) {
		return "{ERROR: " + e.message() + "}";
	}
}

string kaba_float642str(double f) {
	return f642s(f, 6);
}

string kaba_char2str(char c) {
	return string(&c, 1);
}

string kaba_char_repr(char c) {
	return "'" + string(&c, 1).escape() + "'";
}


class KabaString : public string {
public:
	string format(const string& fmt) const {
		try {
			return _xf_str_<const string&>(fmt + "s", *this);
		} catch (::Exception& e) {
			return "{ERROR: " + e.message() + "}";
		}
	}
	bool contains_s(const string &s) const {
		return find(s, 0) >= 0;
	}
	bool contains_c(char c) const {
		for (auto &cc: *this)
			if (cc == c)
				return true;
		return false;
	}
};



int xop_int_exp(int a, int b) {
	return (int)pow((double)a, (double)b);
}
float xop_float_exp(float a, float b) {
	return pow(a, b);
}


class _VirtualBase : public VirtualBase {
public:
	void __init__() {
		new(this) _VirtualBase();
	}
};


class BoolList : public Array<bool> {
public:
	// a = b + c
	Array<bool> _cdecl _and(BoolList &b)	IMPLEMENT_OP(and, bool, bool)
	Array<bool> _cdecl _or(BoolList &b)	IMPLEMENT_OP(or, bool, bool)
	Array<bool> _cdecl eq(BoolList &b)	IMPLEMENT_OP(==, bool, bool)
	Array<bool> _cdecl ne(BoolList &b)	IMPLEMENT_OP(!=, bool, bool)
	// a = b + x
	Array<bool> _cdecl _and2(bool x)	IMPLEMENT_OP2(and, bool, bool)
	Array<bool> _cdecl _or2(bool x)	IMPLEMENT_OP2(or, bool, bool)
	Array<bool> _cdecl eq2(bool x)	IMPLEMENT_OP2(==, bool, bool)
	Array<bool> _cdecl ne2(bool x)	IMPLEMENT_OP2(!=, bool, bool)

	//Array<bool> _cdecl _not(BoolList &b)	IMPLEMENT_OP(!, bool, bool)

	bool all() const {
		for (auto &b: *this)
			if (!b)
				return false;
		return true;
	}
	bool any() const {
		for (auto &b: *this)
			if (b)
				return true;
		return false;
	}

	string str() const {
		return ba2s(*this);
	}
};

template<class T>
T list_min(const Array<T> &a) {
	if (a.num == 0)
		return 0;
	T r = a[0];
	for (int i=1; i<a.num; i++)
		r = min(r, a[i]);
	return r;
}
template<class T>
T list_max(const Array<T> &a) {
	if (a.num == 0)
		return 0;
	T r = a[0];
	for (int i=1; i<a.num; i++)
		r = max(r, a[i]);
	return r;
}
template<class T>
T list_sum(const Array<T> &a) {
	T r = 0;
	for (int i=0; i<a.num; i++)
		r += a[i];
	return r;
}

class IntList : public Array<int> {
public:
	void _cdecl sort() {
		std::sort((int*)data, (int*)data + num);
	}
	void _cdecl unique() {
		int ndiff = 0;
		int i0 = 1;
		while(((int*)data)[i0] != ((int*)data)[i0-1])
			i0 ++;
		for (int i=i0;i<num;i++) {
			if (((int*)data)[i] == ((int*)data)[i-1])
				ndiff ++;
			else
				((int*)data)[i - ndiff] = ((int*)data)[i];
		}
		resize(num - ndiff);
	}
	bool __contains__(int v) {
		for (int i=0;i<num;i++)
			if ((*this)[i] == v)
				return true;
		return false;
	}
	string str() const {
		return ia2s(*this);
	}
	
	// a += b
	void _cdecl iadd(IntList &b)	IMPLEMENT_IOP(+=, int)
	void _cdecl isub(IntList &b)	IMPLEMENT_IOP(-=, int)
	void _cdecl imul(IntList &b)	IMPLEMENT_IOP(*=, int)
	void _cdecl idiv(IntList &b)	IMPLEMENT_IOP(/=, int)

	// a = b + c
	Array<int> _cdecl add(IntList &b)	IMPLEMENT_OP(+, int, int)
	Array<int> _cdecl sub(IntList &b)	IMPLEMENT_OP(-, int, int)
	Array<int> _cdecl mul(IntList &b)	IMPLEMENT_OP(*, int, int)
	Array<int> _cdecl div(IntList &b)	IMPLEMENT_OP(/, int, int)
	Array<int> _cdecl exp(IntList &b)	IMPLEMENT_OPF(xop_int_exp, int, int)

	// a += x
	void _cdecl iadd2(int x)	IMPLEMENT_IOP2(+=, int)
	void _cdecl isub2(int x)	IMPLEMENT_IOP2(-=, int)
	void _cdecl imul2(int x)	IMPLEMENT_IOP2(*=, int)
	void _cdecl idiv2(int x)	IMPLEMENT_IOP2(/=, int)
	void _cdecl assign_int(int x)	IMPLEMENT_IOP2(=, int)
	
	// a = b + x
	Array<int> _cdecl add2(int x)	IMPLEMENT_OP2(+, int, int)
	Array<int> _cdecl sub2(int x)	IMPLEMENT_OP2(-, int, int)
	Array<int> _cdecl mul2(int x)	IMPLEMENT_OP2(*, int, int)
	Array<int> _cdecl div2(int x)	IMPLEMENT_OP2(/, int, int)
	Array<int> _cdecl exp2(int x)	IMPLEMENT_OPF2(xop_int_exp, int, int)
	
	// a <= b
	Array<bool> _cdecl lt(IntList &b) IMPLEMENT_OP(<, int, bool)
	Array<bool> _cdecl le(IntList &b) IMPLEMENT_OP(<=, int, bool)
	Array<bool> _cdecl gt(IntList &b) IMPLEMENT_OP(>, int, bool)
	Array<bool> _cdecl ge(IntList &b) IMPLEMENT_OP(>=, int, bool)
	Array<bool> _cdecl eq(IntList &b) IMPLEMENT_OP(==, int, bool)
	Array<bool> _cdecl ne(IntList &b) IMPLEMENT_OP(!=, int, bool)

	// a <= x
	Array<bool> _cdecl lt2(int x) IMPLEMENT_OP2(<, int, bool)
	Array<bool> _cdecl le2(int x) IMPLEMENT_OP2(<=, int, bool)
	Array<bool> _cdecl gt2(int x) IMPLEMENT_OP2(>, int, bool)
	Array<bool> _cdecl ge2(int x) IMPLEMENT_OP2(>=, int, bool)
	Array<bool> _cdecl eq2(int x) IMPLEMENT_OP2(==, int, bool)
	Array<bool> _cdecl ne2(int x) IMPLEMENT_OP2(!=, int, bool)
};

class FloatList : public Array<float> {
public:
	float _cdecl sum2() {
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i] * (*this)[i];
		return r;
	}

	void _cdecl sort() {
		std::sort((float*)data, (float*)data + num);
	}

	string str() const {
		return fa2s(*this);
	}
	
	// a += b
	void _cdecl iadd(FloatList &b)	IMPLEMENT_IOP(+=, float)
	void _cdecl isub(FloatList &b)	IMPLEMENT_IOP(-=, float)
	void _cdecl imul(FloatList &b)	IMPLEMENT_IOP(*=, float)
	void _cdecl idiv(FloatList &b)	IMPLEMENT_IOP(/=, float)

	// a = b + c
	Array<float> _cdecl add(FloatList &b)	IMPLEMENT_OP(+, float, float)
	Array<float> _cdecl sub(FloatList &b)	IMPLEMENT_OP(-, float, float)
	Array<float> _cdecl mul(FloatList &b)	IMPLEMENT_OP(*, float, float)
	Array<float> _cdecl div(FloatList &b)	IMPLEMENT_OP(/, float, float)
	Array<float> _cdecl exp(FloatList &b)	IMPLEMENT_OPF(xop_float_exp, float, float)

	// a += x
	void _cdecl iadd2(float x)	IMPLEMENT_IOP2(+=, float)
	void _cdecl isub2(float x)	IMPLEMENT_IOP2(-=, float)
	void _cdecl imul2(float x)	IMPLEMENT_IOP2(*=, float)
	void _cdecl idiv2(float x)	IMPLEMENT_IOP2(/=, float)
	void _cdecl assign_float(float x)	IMPLEMENT_IOP2(=, float)
	
	// a = b + x
	Array<float> _cdecl add2(float x)	IMPLEMENT_OP2(+, float, float)
	Array<float> _cdecl sub2(float x)	IMPLEMENT_OP2(-, float, float)
	Array<float> _cdecl mul2(float x)	IMPLEMENT_OP2(*, float, float)
	Array<float> _cdecl div2(float x)	IMPLEMENT_OP2(/, float, float)
	Array<float> _cdecl exp2(float x)	IMPLEMENT_OPF2(xop_float_exp, float, float)
	
	// a <= b
	Array<bool> _cdecl lt(FloatList &b) IMPLEMENT_OP(<, float, bool)
	Array<bool> _cdecl le(FloatList &b) IMPLEMENT_OP(<=, float, bool)
	Array<bool> _cdecl gt(FloatList &b) IMPLEMENT_OP(>, float, bool)
	Array<bool> _cdecl ge(FloatList &b) IMPLEMENT_OP(>=, float, bool)
	Array<bool> _cdecl eq(FloatList &b) IMPLEMENT_OP(==, float, bool)
	Array<bool> _cdecl ne(FloatList &b) IMPLEMENT_OP(!=, float, bool)

	// a <= x
	Array<bool> _cdecl lt2(float x) IMPLEMENT_OP2(<, float, bool)
	Array<bool> _cdecl le2(float x) IMPLEMENT_OP2(<=, float, bool)
	Array<bool> _cdecl gt2(float x) IMPLEMENT_OP2(>, float, bool)
	Array<bool> _cdecl ge2(float x) IMPLEMENT_OP2(>=, float, bool)
	Array<bool> _cdecl eq2(float x) IMPLEMENT_OP2(==, float, bool)
	Array<bool> _cdecl ne2(float x) IMPLEMENT_OP2(!=, float, bool)
};



void SIAddXCommands() {


	add_func("@sorted", TypeDynamicArray, &kaba_array_sort, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("list", TypePointer);
		func_add_param("class", TypeClassP);
		func_add_param("by", TypeString);
	add_func("@var2str", TypeString, &var2str, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("@var_repr", TypeString, &var_repr, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("@map", TypeDynamicArray, &kaba_map, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("func", TypeFunctionP);
		func_add_param("array", TypePointer);
	add_func("@dyn", TypeAny, &kaba_dyn, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("@xmap", TypeDynamicArray, &kaba_xmap, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("func", TypeFunctionP);
		func_add_param("array", TypeDynamic);

	add_func("@call0", TypeVoid, &kaba_call0, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
	add_func("@call1", TypeVoid, &kaba_call1, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
		func_add_param("p1", TypePointer);
	add_func("@call2", TypeVoid, &kaba_call2, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
		func_add_param("p1", TypePointer);
		func_add_param("p2", TypePointer);
	add_func("@call3", TypeVoid, &kaba_call3, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
		func_add_param("p1", TypePointer);
		func_add_param("p2", TypePointer);
		func_add_param("p3", TypePointer);
}


void SIAddPackageBase() {
	add_package("base", Flags::AUTO_IMPORT);

	// internal
	TypeUnknown			= add_type  ("-unknown-", 0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type  ("-reg128-", 16, Flags::CALL_BY_VALUE);
	TypeReg64			= add_type  ("-reg64-", 8, Flags::CALL_BY_VALUE);
	TypeReg32			= add_type  ("-reg32-", 4, Flags::CALL_BY_VALUE);
	TypeReg16			= add_type  ("-reg16-", 2, Flags::CALL_BY_VALUE);
	TypeReg8			= add_type  ("-reg8-", 1, Flags::CALL_BY_VALUE);
	TypeObject			= add_type  ("Object", sizeof(VirtualBase)); // base for most virtual classes
	TypeObjectP			= add_type_p(TypeObject);
	TypeDynamic			= add_type  ("-dynamic-", 0);

	// "real"
	TypeVoid			= add_type  ("void", 0, Flags::CALL_BY_VALUE);
	TypeBool			= add_type  ("bool", sizeof(bool), Flags::CALL_BY_VALUE);
	TypeInt				= add_type  ("int", sizeof(int), Flags::CALL_BY_VALUE);
	TypeInt64			= add_type  ("int64", sizeof(int64), Flags::CALL_BY_VALUE);
	TypeFloat32			= add_type  ("float32", sizeof(float), Flags::CALL_BY_VALUE);
	TypeFloat64			= add_type  ("float64", sizeof(double), Flags::CALL_BY_VALUE);
	TypeChar			= add_type  ("char", sizeof(char), Flags::CALL_BY_VALUE);
	TypeDynamicArray	= add_type  ("@DynamicArray", config.super_array_size);
	TypeAbstractList	= add_type  ("-abstract-list-", config.super_array_size);
	TypeAbstractDict	= add_type  ("-abstract-dict-", config.super_array_size);
	TypeAbstractTuple	= add_type  ("-abstract-tuple-", config.super_array_size);
	TypeDictBase		= add_type  ("@DictBase",   config.super_array_size);
	TypeSharedPointer	= add_type  ("@SharedPointer", config.pointer_size);

	TypeException		= add_type  ("Exception", sizeof(KabaException));
	TypeExceptionP		= add_type_p(TypeException);


	// select default float type
	TypeFloat = TypeFloat32;
	(const_cast<Class*>(TypeFloat))->name = "float";


	add_class(TypeObject);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &_VirtualBase::__init__);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, &VirtualBase::__delete__);
		class_set_vtable(VirtualBase);

	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt, config.pointer_size);
		class_add_func("swap", TypeVoid, &DynamicArray::simple_swap);
			func_add_param("i1", TypeInt);
			func_add_param("i2", TypeInt);
		class_add_func(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, &DynamicArray::ref_subarray, Flags::SELFREF);
			func_add_param("start", TypeInt);
			func_add_param("end", TypeInt);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init);
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt, config.pointer_size);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init);
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);

	add_class(TypeSharedPointer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nullptr);
			func_set_inline(InlineID::SHARED_POINTER_INIT);

	// derived   (must be defined after the primitive types and the bases!)
	TypePointer     = add_type_p(TypeVoid, Flags::CALL_BY_VALUE); // substitute for all pointer types
	TypePointerList = add_type_l(TypePointer);
	TypeBoolPs      = add_type_p(TypeBool, Flags::SILENT);
	TypeBoolList    = add_type_l(TypeBool);
	TypeIntPs       = add_type_p(TypeInt, Flags::SILENT);
	TypeIntP        = add_type_p(TypeInt);
	TypeIntList     = add_type_l(TypeInt);
	TypeIntArray    = add_type_a(TypeInt, 1, "int[?]");
	TypeFloatP      = add_type_p(TypeFloat);
	TypeFloatPs     = add_type_p(TypeFloat, Flags::SILENT);
	TypeFloatArray  = add_type_a(TypeFloat, 1, "float[?]");
	TypeFloatArrayP = add_type_p(TypeFloatArray);
	TypeFloatList   = add_type_l(TypeFloat);
	TypeCharPs      = add_type_p(TypeChar, Flags::SILENT);
	TypeCString     = add_type_a(TypeChar, 256, "cstring");	// cstring := char[256]
	TypeString      = add_type_l(TypeChar, "string");	// string := char[]
	TypeStringAutoCast = add_type("-string-auto-cast-", config.super_array_size);	// string := char[]
	TypeStringList  = add_type_l(TypeString);

	TypeIntDict     = add_type_d(TypeInt);
	TypeFloatDict   = add_type_d(TypeFloat);
	TypeStringDict  = add_type_d(TypeString);
	


	add_func("p2b", TypeBool, &_Pointer2Bool, Flags::_STATIC__PURE);
		func_set_inline(InlineID::POINTER_TO_BOOL);
		func_add_param("p", TypePointer);
	


	add_class(TypePointer);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &p2s, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePointer, TypePointer, InlineID::POINTER_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_EQUAL);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_NOT_EQUAL);


	add_class(TypeInt);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &i2s, Flags::PURE);
		class_add_func("format", TypeString, &kaba_int_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__float__", TypeFloat32, &_Int2Float, Flags::PURE);
			func_set_inline(InlineID::INT_TO_FLOAT);
		class_add_func("__char__", TypeChar, &_Int2Char, Flags::PURE);
			func_set_inline(InlineID::INT_TO_CHAR);
		class_add_func("__int64__", TypeInt64, &_Int2Int64, Flags::PURE);
			func_set_inline(InlineID::INT_TO_INT64);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt, TypeInt, InlineID::INT_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt, TypeInt, TypeInt, InlineID::INT_ADD, &op_int_add);
		add_operator(OperatorID::SUBTRACT, TypeInt, TypeInt, TypeInt, InlineID::INT_SUBTRACT, &op_int_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt, TypeInt, TypeInt, InlineID::INT_MULTIPLY, &op_int_mul);
		add_operator(OperatorID::DIVIDE, TypeInt, TypeInt, TypeInt, InlineID::INT_DIVIDE, &op_int_div);
		add_operator(OperatorID::EXPONENT, TypeInt, TypeInt, TypeInt, InlineID::NONE, &xop_int_exp);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt, TypeInt, InlineID::INT_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt, TypeInt, InlineID::INT_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt, TypeInt, InlineID::INT_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt, TypeInt, InlineID::INT_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt, TypeInt, TypeInt, InlineID::INT_MODULO, &op_int_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_EQUAL, &op_int_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_NOT_EQUAL, &op_int_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt, TypeInt, InlineID::INT_GREATER, &op_int_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_GREATER_EQUAL, &op_int_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt, TypeInt, InlineID::INT_SMALLER, &op_int_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_SMALLER_EQUAL, &op_int_le);
		add_operator(OperatorID::BIT_AND, TypeInt, TypeInt, TypeInt, InlineID::INT_AND);
		add_operator(OperatorID::BIT_OR, TypeInt, TypeInt, TypeInt, InlineID::INT_OR);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt, TypeInt, TypeInt, InlineID::INT_SHIFT_RIGHT, &op_int_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt, TypeInt, TypeInt, InlineID::INT_SHIFT_LEFT, &op_int_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt, nullptr, TypeInt, InlineID::INT_NEGATE, &op_int_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt, nullptr, InlineID::INT_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt, nullptr, InlineID::INT_DECREASE);

	add_class(TypeInt64);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &i642s, Flags::PURE);
		class_add_func("__int__", TypeInt, &_Int642Int, Flags::PURE);
			func_set_inline(InlineID::INT64_TO_INT);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_ADD, &op_int64_add);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt, InlineID::INT64_ADD_INT, &op_int64_add_int); // needed by internal address calculations!
		add_operator(OperatorID::SUBTRACT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT, &op_int64_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY, &op_int64_mul);
		add_operator(OperatorID::DIVIDE, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE, &op_int64_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MODULO, &op_int64_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_EQUAL, &op_int64_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_NOT_EQUAL, &op_int64_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER, &op_int64_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER_EQUAL, &op_int64_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER, &op_int64_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER_EQUAL, &op_int64_le);
		add_operator(OperatorID::BIT_AND, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_AND);
		add_operator(OperatorID::BIT_OR, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_OR);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_RIGHT, &op_int64_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_LEFT, &op_int64_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt64, nullptr, TypeInt64, InlineID::INT64_NEGATE, &op_int64_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_DECREASE);


	add_class(TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &kaba_float2str, Flags::PURE);
		class_add_func("str2", TypeString, &f2s, Flags::PURE);
			func_add_param("decimals", TypeInt);
		class_add_func("format", TypeString, &kaba_float_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__int__", TypeInt, &_Float2Int, Flags::PURE);
			func_set_inline(InlineID::FLOAT_TO_INT);    // sometimes causes floating point exceptions...
		class_add_func("__float64__", TypeFloat64, &_Float2Float64, Flags::PURE);
			func_set_inline(InlineID::FLOAT_TO_FLOAT64);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_ASSIGN);
		add_operator(OperatorID::ADD, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_ADD, &op_float_add);
		add_operator(OperatorID::SUBTRACT, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_SUBTARCT, &op_float_sub);
		add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_MULTIPLY, &op_float_mul);
		add_operator(OperatorID::DIVIDE, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_DIVIDE, &op_float_div);
		add_operator(OperatorID::EXPONENT, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::NONE, &xop_float_exp);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_EQUAL, &op_float_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_NOT_EQUAL, &op_float_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_GREATER, &op_float_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_GREATER_EQUAL, &op_float_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_SMALLER, &op_float_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_SMALLER_EQUAL, &op_float_le);
		add_operator(OperatorID::NEGATIVE, TypeFloat32, nullptr, TypeFloat32, InlineID::FLOAT_NEGATE, &op_float_neg);


	add_class(TypeFloat64);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &kaba_float642str, Flags::PURE);
		class_add_func("__float__", TypeFloat32, &_Float642Float, Flags::PURE);
			func_set_inline(InlineID::FLOAT64_TO_FLOAT);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD, &op_double_add);
		add_operator(OperatorID::SUBTRACT, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT, &op_double_sub);
		add_operator(OperatorID::MULTIPLY, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY, &op_double_mul);
		add_operator(OperatorID::DIVIDE, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE, &op_double_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_EQUAL, &op_double_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_NOT_EQUAL, &op_double_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER, &op_double_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER_EQUAL, &op_double_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER, &op_double_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER_EQUAL, &op_double_le);
		add_operator(OperatorID::NEGATIVE, TypeFloat32, nullptr, TypeFloat64, InlineID::FLOAT64_NEGATE, &op_double_neg);


	add_class(TypeBool);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &b2s, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeBool, TypeBool, InlineID::BOOL_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_EQUAL);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_NOT_EQUAL);
		add_operator(OperatorID::AND, TypeBool, TypeBool, TypeBool, InlineID::BOOL_AND);
		add_operator(OperatorID::OR, TypeBool, TypeBool, TypeBool, InlineID::BOOL_OR);
		add_operator(OperatorID::NEGATE, TypeBool, nullptr, TypeBool, InlineID::BOOL_NEGATE);

	add_class(TypeChar);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &kaba_char2str, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_REPR, TypeString, &kaba_char_repr, Flags::PURE);
		class_add_func("__int__", TypeInt, &_Char2Int, Flags::PURE);
			func_set_inline(InlineID::CHAR_TO_INT);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeChar, TypeChar, InlineID::CHAR_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_EQUAL);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_NOT_EQUAL);
		add_operator(OperatorID::GREATER, TypeBool, TypeChar, TypeChar, InlineID::CHAR_GREATER);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_GREATER_EQUAL);
		add_operator(OperatorID::SMALLER, TypeBool, TypeChar, TypeChar, InlineID::CHAR_SMALLER);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_SMALLER_EQUAL);
		add_operator(OperatorID::ADD, TypeChar, TypeChar, TypeChar, InlineID::CHAR_ADD);
		add_operator(OperatorID::SUBTRACTS, TypeChar, TypeChar, TypeChar, InlineID::CHAR_SUBTRACT_ASSIGN);
		add_operator(OperatorID::ADDS, TypeChar, TypeChar, TypeChar, InlineID::CHAR_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACT, TypeChar, TypeChar, TypeChar, InlineID::CHAR_SUBTRACT);
		add_operator(OperatorID::BIT_AND, TypeChar, TypeChar, TypeChar, InlineID::CHAR_AND);
		add_operator(OperatorID::BIT_OR, TypeChar, TypeChar, TypeChar, InlineID::CHAR_OR);
		add_operator(OperatorID::NEGATE, TypeChar, nullptr, TypeChar, InlineID::CHAR_NEGATE);


	add_class(TypeString);
		add_operator(OperatorID::ADDS, TypeVoid, TypeString, TypeString, InlineID::NONE, &string::operator+=);
		add_operator(OperatorID::ADD, TypeString, TypeString, TypeString, InlineID::NONE, &string::operator+);
		add_operator(OperatorID::EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator==);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator!=);
		add_operator(OperatorID::SMALLER, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator<);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator<=);
		add_operator(OperatorID::GREATER, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator>);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator>=);
		class_add_func("head", TypeString, &string::head, Flags::PURE);
			func_add_param("size", TypeInt);
		class_add_func("tail", TypeString, &string::tail, Flags::PURE);
			func_add_param("size", TypeInt);
		class_add_func("find", TypeInt, &string::find, Flags::PURE);
			func_add_param("str", TypeString);
			func_add_param("start", TypeInt);
		class_add_func("compare", TypeInt, &string::compare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("icompare", TypeInt, &string::icompare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("replace", TypeString, &string::replace, Flags::PURE);
			func_add_param("sub", TypeString);
			func_add_param("by", TypeString);
		class_add_func("explode", TypeStringList, &string::explode, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("parse_tokens", TypeStringList, &string::parse_tokens, Flags::PURE);
			func_add_param("splitters", TypeString);
		class_add_func("repeat", TypeString, &string::repeat, Flags::PURE);
			func_add_param("n", TypeInt);
		class_add_func("lower", TypeString, &string::lower, Flags::PURE);
		class_add_func("upper", TypeString, &string::upper, Flags::PURE);
		class_add_func("reverse", TypeString, &string::reverse, Flags::PURE);
		class_add_func("hash", TypeInt, &string::hash, Flags::PURE);
		class_add_func("md5", TypeString, &string::md5, Flags::PURE);
		class_add_func("hex", TypeString, &string::hex, Flags::PURE);
		class_add_func("unhex", TypeString, &string::unhex, Flags::PURE);
		class_add_func("match", TypeBool, &string::match, Flags::PURE);
			func_add_param("glob", TypeString);
		class_add_func("__int__", TypeInt, &string::_int, Flags::PURE);
		class_add_func("__int64__", TypeInt64, &string::i64, Flags::PURE);
		class_add_func("__float__", TypeFloat32, &string::_float, Flags::PURE);
		class_add_func("__float64__", TypeFloat64, &string::f64, Flags::PURE);
		class_add_func("trim", TypeString, &string::trim, Flags::PURE);
		class_add_func("escape", TypeString, &string::escape, Flags::PURE);
		class_add_func("unescape", TypeString, &string::unescape, Flags::PURE);
		class_add_func("utf8_to_utf32", TypeIntList, &string::utf8_to_utf32, Flags::PURE);
		class_add_func("utf8_length", TypeInt, &string::utf8len, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_REPR, TypeString, &string::repr, Flags::PURE);
		class_add_func("format", TypeString, &KabaString::format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__contains__", TypeBool, &KabaString::contains_s, Flags::PURE);
			func_add_param("s", TypeString);
		class_add_func("__contains__", TypeBool, &KabaString::contains_c, Flags::PURE);
			func_add_param("c", TypeChar);



	add_class(TypeBoolList);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &BoolList::str, Flags::PURE);
		class_add_func("all", TypeBool, &BoolList::all, Flags::PURE);
		class_add_func("any", TypeBool, &BoolList::any, Flags::PURE);
		class_add_func("__bool__", TypeBool, &BoolList::all, Flags::PURE);
		add_operator(OperatorID::AND, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::_and);
		add_operator(OperatorID::OR, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::_or);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::eq);
		add_operator(OperatorID::NOTEQUAL, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::ne);
		add_operator(OperatorID::AND, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::_and2);
		add_operator(OperatorID::OR, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::_or2);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::eq2);
		add_operator(OperatorID::NOTEQUAL, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::ne2);

	
	
	add_class(TypeIntList);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &IntList::str, Flags::PURE);
		class_add_func("sort", TypeVoid, &IntList::sort);
		class_add_func("unique", TypeVoid, &IntList::unique);
		class_add_func("sum", TypeInt, &list_sum<int>, Flags::PURE);
		class_add_func("min", TypeInt, &list_min<int>, Flags::PURE);
		class_add_func("max", TypeInt, &list_max<int>, Flags::PURE);
		add_operator(OperatorID::ADDS, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &IntList::iadd);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &IntList::isub);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &IntList::imul);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &IntList::idiv);
		add_operator(OperatorID::ADD, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::add);
		add_operator(OperatorID::SUBTRACT, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::sub);
		add_operator(OperatorID::MULTIPLY, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::mul);
		add_operator(OperatorID::DIVIDE, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::div);
		add_operator(OperatorID::EXPONENT, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::exp);
		add_operator(OperatorID::ADD, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &IntList::add2);
		add_operator(OperatorID::SUBTRACT, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &IntList::sub2);
		add_operator(OperatorID::MULTIPLY, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &IntList::mul2);
		add_operator(OperatorID::DIVIDE, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &IntList::div2);
		add_operator(OperatorID::EXPONENT, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &IntList::exp2);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeIntList, TypeInt, InlineID::NONE, &IntList::assign_int);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::lt);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::le);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::gt);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::ge);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::eq);
		add_operator(OperatorID::NOTEQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &IntList::ne);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &IntList::lt2);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &IntList::le2);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &IntList::gt2);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &IntList::ge2);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &IntList::eq2);
		add_operator(OperatorID::NOTEQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &IntList::ne2);
		class_add_func("__contains__", TypeBool, &IntList::__contains__, Flags::PURE);
			func_add_param("i", TypeInt);

	add_class(TypeFloatList);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &FloatList::str, Flags::PURE);
		class_add_func("sort", TypeVoid, &FloatList::sort);
		class_add_func("sum", TypeFloat32, &list_sum<float>, Flags::PURE);
		class_add_func("sum2", TypeFloat32, &FloatList::sum2, Flags::PURE);
		class_add_func("max", TypeFloat32, &list_max<float>, Flags::PURE);
		class_add_func("min", TypeFloat32, &list_min<float>, Flags::PURE);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::iadd);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::isub);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::imul);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::idiv);
		add_operator(OperatorID::ADD, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::add);
		add_operator(OperatorID::SUBTRACT, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::sub);
		add_operator(OperatorID::MULTIPLY, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::mul);
		add_operator(OperatorID::DIVIDE, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::div);
		add_operator(OperatorID::EXPONENT, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::exp);
		add_operator(OperatorID::ADD, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::add2);
		add_operator(OperatorID::SUBTRACT, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::sub2);
		add_operator(OperatorID::MULTIPLY, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::mul2);
		add_operator(OperatorID::DIVIDE, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::div2);
		add_operator(OperatorID::EXPONENT, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::exp2);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::iadd2);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::isub2);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::imul2);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::idiv2);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::assign_float);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::lt);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::le);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::gt);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &FloatList::ge);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::lt2);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::le2);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::gt2);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &FloatList::ge2);



	add_class(TypeStringList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &StringList::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &StringList::clear);
		class_add_func("add", TypeVoid, &StringList::add);
			func_add_param("x", TypeString);
		class_add_func("clear", TypeVoid, &StringList::clear);
		class_add_func("remove", TypeVoid, &StringList::erase);
			func_add_param("index", TypeInt);
		class_add_func("resize", TypeVoid, &StringList::resize);
			func_add_param("num", TypeInt);
		class_add_func("join", TypeString, &StringList::join, Flags::PURE);
			func_add_param("glue", TypeString);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &StringList::str, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeStringList, TypeStringList, InlineID::NONE, &StringList::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__eq__);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__neq__);
		add_operator(OperatorID::ADD, TypeStringList, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__add__);
		add_operator(OperatorID::ADDS, TypeVoid, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__adds__);
		class_add_func("__contains__", TypeBool, &StringList::__contains__, Flags::PURE);
			func_add_param("s", TypeString);


	// constants
	void *kaba_nil = nullptr;
	bool kaba_true = true;
	bool kaba_false = false;
	add_const("nil", TypePointer, &kaba_nil);
	add_const("false", TypeBool, &kaba_false);
	add_const("true",  TypeBool, &kaba_true);


	add_class(TypeException);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaException::__init__);
			func_add_param("message", TypeString);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaException::__delete__);
		class_add_func_virtual(IDENTIFIER_FUNC_STR, TypeString, &KabaException::message);
		class_add_element("_text", TypeString, config.pointer_size);
		class_set_vtable(KabaException);

	add_func(IDENTIFIER_RAISE, TypeVoid, &kaba_raise_exception, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("e", TypeExceptionP);
		
		
	// type casting
	add_func("p2s", TypeString, &p2s, Flags::_STATIC__PURE);
		func_add_param("p", TypePointer);
	// debug output
	/*add_func("cprint", TypeVoid, &_cstringout, Flags::STATIC);
		func_add_param("str", TypeCString);*/
	add_func("print", TypeVoid, &kaba_print, Flags::STATIC);
		func_add_param("str", TypeStringAutoCast);//, (Flags)((int)Flags::CONST | (int)Flags::AUTO_CAST));
	add_ext_var("_print_postfix", TypeString, &kaba_print_postfix);
	add_func("binary", TypeString, &kaba_binary, Flags::STATIC);
		func_add_param("p", TypePointer);
		func_add_param("length", TypeInt);
	// memory
	add_func("@malloc", TypePointer, &malloc, Flags::STATIC);
		func_add_param("size", TypeInt);
	add_func("@free", TypeVoid, &free, Flags::STATIC);
		func_add_param("p", TypePointer);

	// basic testing
	add_func("_int_out", TypeVoid, &kaba_int_out, Flags::STATIC);
		func_add_param("i", TypeInt);
	add_func("_float_out", TypeVoid, &kaba_float_out, Flags::STATIC);
		func_add_param("f", TypeFloat32);
	add_func("_call_float", TypeVoid, &_x_call_float, Flags::STATIC);
	add_func("_float_ret", TypeFloat32, &kaba_float_ret, Flags::STATIC);
	add_func("_int_ret", TypeInt, &kaba_int_ret, Flags::STATIC);
	add_func("_xxx", TypeVoid, &kaba_xxx, Flags::STATIC);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);
		func_add_param("c", TypeInt);
		func_add_param("d", TypeInt);
		func_add_param("e", TypeInt);
		func_add_param("f", TypeInt);
		func_add_param("g", TypeInt);
		func_add_param("h", TypeInt);
	add_ext_var("_extern_variable", TypeInt, &extern_variable1);
}



}
