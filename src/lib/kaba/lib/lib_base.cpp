#include "../kaba.h"
#include "common.h"
#include "exception.h"
#include "dynamic.h"
#include "../../file/file.h"
#include <algorithm>
#include <cstdio>
#include <math.h>


namespace Kaba {

extern const Class *TypeDynamicArray;
const Class *TypeAbstractList;
const Class *TypeAbstractDict;
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


#ifndef OS_WINDOWS
	//#define _cdecl
	#include <stdlib.h>
#endif

void _cdecl _print(const string &str)
{	printf("%s\n", str.c_str());	}
void _cdecl _cstringout(char *str)
{	_print(str);	}
string _cdecl _binary(const char *p, int length)
{	return string(p, length);	}
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
	Array<string> __add__(const Array<string> &o) {
		return *this + o;
	}
	void __adds__(const Array<string> &o) {
		append(o);
	}
};

string i2s_zero_padded(int i, int n) {
	string s = i2s(i);
	while (s.num < n)
		s = "0" + s;
	return s;
}

string kaba_int_format(int i, const string &fmt) {
	if (fmt.head(1) == "0" and fmt.num >= 2)
		return i2s_zero_padded(i, fmt[1] - 48);
	return i2s(i);
}

string kaba_float2str(float f) {
	return f2s(f, 6);
}

string kaba_float_format(float f, const string &fmt) {
	if (fmt.head(1) == "." and fmt.num >= 2)
		return f2s(f, fmt[1] - 48);
	return f2s(f, 6);
}

string kaba_float642str(float f) {
	return f642s(f, 6);
}

string kaba_char2str(char c) {
	return string(&c, 1);
}



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
};

class IntList : public Array<int> {
public:
	int _cdecl sum() {
		int r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	void _cdecl sort()
	{	std::sort((int*)data, (int*)data + num);	}
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
	float _cdecl _max() {
		float max = 0;
		if (num > 0)
			max = (*this)[0];
		for (int i=1;i<num;i++)
			if ((*this)[i] > max)
				max = (*this)[i];
		return max;
	}
	float _cdecl _min() {
		float min = 0;
		if (num > 0)
			min = (*this)[0];
		for (int i=1;i<num;i++)
			if ((*this)[i] < min)
				min = (*this)[i];
		return min;
	}
	float _cdecl sum() {
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	float _cdecl sum2() {
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i] * (*this)[i];
		return r;
	}

	void _cdecl sort()
	{	std::sort((float*)data, (float*)data + num);	}
	
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



#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


string _cdecl kaba_shell_execute(const string &cmd) {
	try {
		return shell_execute(cmd);
	} catch(::Exception &e) {
		kaba_raise_exception(new KabaException(e.message()));
	}
	return "";
}


#pragma GCC pop_options


void SIAddXCommands() {


	add_func("@sorted", TypeDynamicArray, (void*)&kaba_array_sort, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("list", TypePointer);
		func_add_param("class", TypeClassP);
		func_add_param("by", TypeString);
	add_func("@var2str", TypeString, (void*)var2str, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("@var_repr", TypeString, (void*)var_repr, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("@map", TypeDynamicArray, (void*)kaba_map, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("func", TypeFunctionP);
		func_add_param("array", TypePointer);
	add_func("@dyn", TypeAny, (void*)kaba_dyn, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);

	add_func("@call0", TypeVoid, (void*)&kaba_call0, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
	add_func("@call1", TypeVoid, (void*)&kaba_call1, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
		func_add_param("p1", TypePointer);
	add_func("@call2", TypeVoid, (void*)&kaba_call2, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
		func_add_param("p1", TypePointer);
		func_add_param("p2", TypePointer);
	add_func("@call3", TypeVoid, (void*)&kaba_call3, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("f", TypeFunctionP);
		func_add_param("p1", TypePointer);
		func_add_param("p2", TypePointer);
		func_add_param("p3", TypePointer);
}


void SIAddPackageBase() {
	add_package("base", true);

	// internal
	TypeUnknown			= add_type  ("-unknown-", 0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type  ("-reg128-", 16, Flags::CALL_BY_VALUE);
	TypeReg64			= add_type  ("-reg64-", 8, Flags::CALL_BY_VALUE);
	TypeReg32			= add_type  ("-reg32-", 4, Flags::CALL_BY_VALUE);
	TypeReg16			= add_type  ("-reg16-", 2, Flags::CALL_BY_VALUE);
	TypeReg8			= add_type  ("-reg8-", 1, Flags::CALL_BY_VALUE);
	TypeObject			= add_type  ("Object", sizeof(VirtualBase)); // base for most virtual classes
	TypeObjectP			= add_type_p(TypeObject);

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
	TypeDictBase		= add_type  ("@DictBase",   config.super_array_size);

	TypeException		= add_type  ("Exception", sizeof(KabaException));
	TypeExceptionP		= add_type_p(TypeException);


	// select default float type
	TypeFloat = TypeFloat32;
	(const_cast<Class*>(TypeFloat))->name = "float";


	add_class(TypeObject);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &_VirtualBase::__init__);
		class_add_func_virtualx(IDENTIFIER_FUNC_DELETE, TypeVoid, &VirtualBase::__delete__);
		class_set_vtable(VirtualBase);

	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt, config.pointer_size);
		class_add_funcx("swap", TypeVoid, &DynamicArray::simple_swap);
			func_add_param("i1", TypeInt);
			func_add_param("i2", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, &DynamicArray::ref_subarray, Flags::SELFREF);
			func_add_param("start", TypeInt);
			func_add_param("end", TypeInt);
		// low level operations
		class_add_funcx("__mem_init__", TypeVoid, &DynamicArray::init);
			func_add_param("element_size", TypeInt);
		class_add_funcx("__mem_clear__", TypeVoid, &DynamicArray::simple_clear);
		class_add_funcx("__mem_resize__", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("size", TypeInt);
		class_add_funcx("__mem_remove__", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt, config.pointer_size);
		// low level operations
		class_add_funcx("__mem_init__", TypeVoid, &DynamicArray::init);
			func_add_param("element_size", TypeInt);
		class_add_funcx("__mem_clear__", TypeVoid, &DynamicArray::simple_clear);
		class_add_funcx("__mem_resize__", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("size", TypeInt);
		class_add_funcx("__mem_remove__", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);

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
	TypeStringList  = add_type_l(TypeString);

	TypeIntDict     = add_type_d(TypeInt);
	TypeFloatDict   = add_type_d(TypeFloat);
	TypeStringDict  = add_type_d(TypeString);
	


	add_funcx("p2b", TypeBool, &_Pointer2Bool, Flags::_STATIC__PURE);
		func_set_inline(InlineID::POINTER_TO_BOOL);
		func_add_param("p", TypePointer);
	


	add_class(TypePointer);
		class_add_funcx("str", TypeString, &p2s, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePointer, TypePointer, InlineID::POINTER_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_EQUAL);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_NOT_EQUAL);


	add_class(TypeInt);
		class_add_funcx("str", TypeString, &i2s, Flags::PURE);
		class_add_funcx("format", TypeString, &kaba_int_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_funcx("float", TypeFloat32, &_Int2Float, Flags::PURE);
			func_set_inline(InlineID::INT_TO_FLOAT);
		class_add_funcx("char", TypeChar, &_Int2Char, Flags::PURE);
			func_set_inline(InlineID::INT_TO_CHAR);
		class_add_funcx("int64", TypeInt64, &_Int2Int64, Flags::PURE);
			func_set_inline(InlineID::INT_TO_INT64);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt, TypeInt, InlineID::INT_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt, TypeInt, TypeInt, InlineID::INT_ADD, (void*)op_int_add);
		add_operator(OperatorID::SUBTRACT, TypeInt, TypeInt, TypeInt, InlineID::INT_SUBTRACT, (void*)op_int_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt, TypeInt, TypeInt, InlineID::INT_MULTIPLY, (void*)op_int_mul);
		add_operator(OperatorID::DIVIDE, TypeInt, TypeInt, TypeInt, InlineID::INT_DIVIDE, (void*)op_int_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt, TypeInt, InlineID::INT_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt, TypeInt, InlineID::INT_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt, TypeInt, InlineID::INT_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt, TypeInt, InlineID::INT_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt, TypeInt, TypeInt, InlineID::INT_MODULO, (void*)op_int_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_EQUAL, (void*)op_int_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_NOT_EQUAL, (void*)op_int_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt, TypeInt, InlineID::INT_GREATER, (void*)op_int_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_GREATER_EQUAL, (void*)op_int_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt, TypeInt, InlineID::INT_SMALLER, (void*)op_int_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_SMALLER_EQUAL, (void*)op_int_le);
		add_operator(OperatorID::BIT_AND, TypeInt, TypeInt, TypeInt, InlineID::INT_AND);
		add_operator(OperatorID::BIT_OR, TypeInt, TypeInt, TypeInt, InlineID::INT_OR);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt, TypeInt, TypeInt, InlineID::INT_SHIFT_RIGHT, (void*)op_int_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt, TypeInt, TypeInt, InlineID::INT_SHIFT_LEFT, (void*)op_int_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt, nullptr, TypeInt, InlineID::INT_NEGATE, (void*)op_int_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt, nullptr, InlineID::INT_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt, nullptr, InlineID::INT_DECREASE);
		class_add_funcx("__exp__", TypeInt, &xop_int_exp, Flags::PURE);
			func_add_param("b", TypeInt);

	add_class(TypeInt64);
		class_add_funcx("str", TypeString, &i642s, Flags::PURE);
		class_add_funcx("int", TypeInt, &_Int642Int, Flags::PURE);
			func_set_inline(InlineID::INT64_TO_INT);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_ADD, (void*)op_int64_add);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt, InlineID::INT64_ADD_INT, (void*)op_int64_add_int); // needed by internal address calculations!
		add_operator(OperatorID::SUBTRACT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT, (void*)op_int64_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY, (void*)op_int64_mul);
		add_operator(OperatorID::DIVIDE, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE, (void*)op_int64_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MODULO, (void*)op_int64_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_EQUAL, (void*)op_int64_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_NOT_EQUAL, (void*)op_int64_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER, (void*)op_int64_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER_EQUAL, (void*)op_int64_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER, (void*)op_int64_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER_EQUAL, (void*)op_int64_le);
		add_operator(OperatorID::BIT_AND, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_AND);
		add_operator(OperatorID::BIT_OR, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_OR);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_RIGHT, (void*)op_int64_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_LEFT, (void*)op_int64_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt64, nullptr, TypeInt64, InlineID::INT64_NEGATE, (void*)op_int64_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_DECREASE);


	add_class(TypeFloat32);
		class_add_funcx("str", TypeString, &kaba_float2str, Flags::PURE);
		class_add_funcx("str2", TypeString, &f2s, Flags::PURE);
			func_add_param("decimals", TypeInt);
		class_add_funcx("format", TypeString, &kaba_float_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_funcx("int", TypeInt, &_Float2Int, Flags::PURE);
			func_set_inline(InlineID::FLOAT_TO_INT);    // sometimes causes floating point exceptions...
		class_add_funcx("float64", TypeFloat64, &_Float2Float64, Flags::PURE);
			func_set_inline(InlineID::FLOAT_TO_FLOAT64);
		class_add_funcx("__exp__", TypeFloat32, &xop_float_exp, Flags::PURE);
			func_add_param("b", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_ASSIGN);
		add_operator(OperatorID::ADD, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_ADD, (void*)op_float_add);
		add_operator(OperatorID::SUBTRACT, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_SUBTARCT, (void*)op_float_sub);
		add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_MULTIPLY, (void*)op_float_mul);
		add_operator(OperatorID::DIVIDE, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_DIVIDE, (void*)op_float_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_EQUAL, (void*)op_float_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_NOT_EQUAL, (void*)op_float_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_GREATER, (void*)op_float_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_GREATER_EQUAL, (void*)op_float_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_SMALLER, (void*)op_float_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_SMALLER_EQUAL, (void*)op_float_le);
		add_operator(OperatorID::NEGATIVE, TypeFloat32, nullptr, TypeFloat32, InlineID::FLOAT_NEGATE, (void*)op_float_neg);


	add_class(TypeFloat64);
		class_add_funcx("str", TypeString, &kaba_float642str, Flags::PURE);
		class_add_funcx("str2", TypeString, &f642s, Flags::PURE);
			func_add_param("decimals", TypeInt);
		class_add_funcx("float", TypeFloat32, &_Float642Float, Flags::PURE);
			func_set_inline(InlineID::FLOAT64_TO_FLOAT);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD, (void*)op_double_add);
		add_operator(OperatorID::SUBTRACT, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT, (void*)op_double_sub);
		add_operator(OperatorID::MULTIPLY, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY, (void*)op_double_mul);
		add_operator(OperatorID::DIVIDE, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE, (void*)op_double_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_EQUAL, (void*)op_double_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_NOT_EQUAL, (void*)op_double_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER, (void*)op_double_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER_EQUAL, (void*)op_double_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER, (void*)op_double_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER_EQUAL, (void*)op_double_le);
		add_operator(OperatorID::NEGATIVE, TypeFloat32, nullptr, TypeFloat64, InlineID::FLOAT64_NEGATE, (void*)op_double_neg);


	add_class(TypeBool);
		class_add_funcx("str", TypeString, &b2s, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeBool, TypeBool, InlineID::BOOL_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_EQUAL);
		add_operator(OperatorID::NOTEQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_NOT_EQUAL);
		add_operator(OperatorID::AND, TypeBool, TypeBool, TypeBool, InlineID::BOOL_AND);
		add_operator(OperatorID::OR, TypeBool, TypeBool, TypeBool, InlineID::BOOL_OR);
		add_operator(OperatorID::NEGATE, TypeBool, nullptr, TypeBool, InlineID::BOOL_NEGATE);

	add_class(TypeChar);
		class_add_funcx("str", TypeString, &kaba_char2str, Flags::PURE);
		class_add_funcx("int", TypeInt, &_Char2Int, Flags::PURE);
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
		class_add_funcx("__iadd__", TypeVoid, &string::operator+=);
			func_add_param("x", TypeString);
		class_add_funcx("__add__", TypeString, &string::operator+, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__eq__", TypeBool, &string::operator==, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__ne__", TypeBool, &string::operator!=, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__lt__", TypeBool, &string::operator<, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__gt__", TypeBool, &string::operator>, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__le__", TypeBool, &string::operator<=, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__ge__", TypeBool, &string::operator>=, Flags::PURE);
			func_add_param("x", TypeString);
		class_add_funcx("head", TypeString, &string::head, Flags::PURE);
			func_add_param("size", TypeInt);
		class_add_funcx("tail", TypeString, &string::tail, Flags::PURE);
			func_add_param("size", TypeInt);
		class_add_funcx("find", TypeInt, &string::find, Flags::PURE);
			func_add_param("str", TypeString);
			func_add_param("start", TypeInt);
		class_add_funcx("compare", TypeInt, &string::compare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_funcx("icompare", TypeInt, &string::icompare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_funcx("replace", TypeString, &string::replace, Flags::PURE);
			func_add_param("sub", TypeString);
			func_add_param("by", TypeString);
		class_add_funcx("explode", TypeStringList, &string::explode, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_funcx("lower", TypeString, &string::lower, Flags::PURE);
		class_add_funcx("upper", TypeString, &string::upper, Flags::PURE);
		class_add_funcx("reverse", TypeString, &string::reverse, Flags::PURE);
		class_add_funcx("hash", TypeInt, &string::hash, Flags::PURE);
		class_add_funcx("md5", TypeString, &string::md5, Flags::PURE);
		class_add_funcx("hex", TypeString, &string::hex, Flags::PURE);
			func_add_param("inverted", TypeBool);
		class_add_funcx("unhex", TypeString, &string::unhex, Flags::PURE);
		class_add_funcx("match", TypeBool, &string::match, Flags::PURE);
			func_add_param("glob", TypeString);
		class_add_funcx("int", TypeInt, &string::_int, Flags::PURE);
		class_add_funcx("int64", TypeInt64, &string::i64, Flags::PURE);
		class_add_funcx("float", TypeFloat32, &string::_float, Flags::PURE);
		class_add_funcx("float64", TypeFloat64, &string::f64, Flags::PURE);
		class_add_funcx("trim", TypeString, &string::trim, Flags::PURE);
		class_add_funcx("dirname", TypeString, &string::dirname, Flags::PURE);
		class_add_funcx("basename", TypeString, &string::basename, Flags::PURE);
		class_add_funcx("extension", TypeString, &string::extension, Flags::PURE);
		class_add_funcx("escape", TypeString, &str_escape, Flags::PURE);
		class_add_funcx("unescape", TypeString, &str_unescape, Flags::PURE);
		class_add_funcx("repr", TypeString, &string::repr, Flags::PURE);



	add_class(TypeBoolList);
		class_add_funcx("str", TypeString, &ba2s, Flags::PURE);
		class_add_func("__and__", TypeBoolList, mf(&BoolList::_and), Flags::PURE);
			func_add_param("other", TypeBoolList);
		class_add_func("__or__", TypeBoolList, mf(&BoolList::_or), Flags::PURE);
			func_add_param("other", TypeBoolList);
		class_add_func("__eq__", TypeBoolList, mf(&BoolList::eq), Flags::PURE);
			func_add_param("other", TypeBoolList);
		class_add_func("__ne__", TypeBoolList, mf(&BoolList::ne), Flags::PURE);
			func_add_param("other", TypeBoolList);
		class_add_func("__and__", TypeBoolList, mf(&BoolList::_and2), Flags::PURE);
			func_add_param("other", TypeBool);
		class_add_func("__or__", TypeBoolList, mf(&BoolList::_or2), Flags::PURE);
			func_add_param("other", TypeBool);
		class_add_func("__eq__", TypeBoolList, mf(&BoolList::eq2), Flags::PURE);
			func_add_param("other", TypeBool);
		class_add_func("__ne__", TypeBoolList, mf(&BoolList::ne2), Flags::PURE);
			func_add_param("other", TypeBool);

	
	
	add_class(TypeIntList);
		class_add_funcx("str", TypeString, &ia2s, Flags::PURE);
		class_add_func("sort", TypeVoid, mf(&IntList::sort));
		class_add_func("unique", TypeVoid, mf(&IntList::unique));
		class_add_func("sum", TypeInt, mf(&IntList::sum), Flags::PURE);
		class_add_func("__iadd__", TypeVoid, mf(&IntList::iadd));
			func_add_param("other", TypeIntList);
		class_add_func("__isub__", TypeVoid, mf(&IntList::isub));
			func_add_param("other", TypeIntList);
		class_add_func("__imul__", TypeVoid, mf(&IntList::imul));
			func_add_param("other", TypeIntList);
		class_add_func("__idiv__", TypeVoid, mf(&IntList::idiv));
			func_add_param("other", TypeIntList);
		class_add_func("__add__", TypeIntList, mf(&IntList::add), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__sub__", TypeIntList, mf(&IntList::sub), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__mul__", TypeIntList, mf(&IntList::mul), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__div__", TypeIntList, mf(&IntList::div), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__exp__", TypeIntList, mf(&IntList::exp), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__add__", TypeIntList, mf(&IntList::add2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__sub__", TypeIntList, mf(&IntList::sub2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__mul__", TypeIntList, mf(&IntList::mul2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__div__", TypeIntList, mf(&IntList::div2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__exp__", TypeIntList, mf(&IntList::exp2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&IntList::assign_int));
			func_add_param("other", TypeInt);
		class_add_funcx("__contains__", TypeBool, &IntList::__contains__, Flags::PURE);
			func_add_param("i", TypeInt);
		class_add_func("__lt__", TypeBoolList, mf(&IntList::lt), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__le__", TypeBoolList, mf(&IntList::le), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__gt__", TypeBoolList, mf(&IntList::gt), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__ge__", TypeBoolList, mf(&IntList::ge), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__eq__", TypeBoolList, mf(&IntList::eq), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__ne__", TypeBoolList, mf(&IntList::ne), Flags::PURE);
			func_add_param("other", TypeIntList);
		class_add_func("__lt__", TypeBoolList, mf(&IntList::lt2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__le__", TypeBoolList, mf(&IntList::le2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__gt__", TypeBoolList, mf(&IntList::gt2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__ge__", TypeBoolList, mf(&IntList::ge2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__eq__", TypeBoolList, mf(&IntList::eq2), Flags::PURE);
			func_add_param("other", TypeInt);
		class_add_func("__ne__", TypeBoolList, mf(&IntList::ne2), Flags::PURE);
			func_add_param("other", TypeInt);

	add_class(TypeFloatList);
		class_add_funcx("str", TypeString, &fa2s, Flags::PURE);
		class_add_func("sort", TypeVoid, mf(&FloatList::sort));
		class_add_func("sum", TypeFloat32, mf(&FloatList::sum), Flags::PURE);
		class_add_func("sum2", TypeFloat32, mf(&FloatList::sum2), Flags::PURE);
		class_add_func("max", TypeFloat32, mf(&FloatList::_max), Flags::PURE);
		class_add_func("min", TypeFloat32, mf(&FloatList::_min), Flags::PURE);
		class_add_func("__iadd__", TypeVoid, mf(&FloatList::iadd));
			func_add_param("other", TypeFloatList);
		class_add_func("__isub__", TypeVoid, mf(&FloatList::isub));
			func_add_param("other", TypeFloatList);
		class_add_func("__imul__", TypeVoid, mf(&FloatList::imul));
			func_add_param("other", TypeFloatList);
		class_add_func("__idiv__", TypeVoid, mf(&FloatList::idiv));
			func_add_param("other", TypeFloatList);
		class_add_func("__add__", TypeFloatList, mf(&FloatList::add), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__sub__", TypeFloatList, mf(&FloatList::sub), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__mul__", TypeFloatList, mf(&FloatList::mul), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__div__", TypeFloatList, mf(&FloatList::div), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__exp__", TypeFloatList, mf(&FloatList::exp), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__add__", TypeFloatList, mf(&FloatList::add2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__sub__", TypeFloatList, mf(&FloatList::sub2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__mul__", TypeFloatList, mf(&FloatList::mul2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__div__", TypeFloatList, mf(&FloatList::div2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__exp__", TypeFloatList, mf(&FloatList::exp2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__iadd__", TypeVoid, mf(&FloatList::iadd2));
			func_add_param("other", TypeFloat32);
		class_add_func("__isub__", TypeVoid, mf(&FloatList::isub2));
			func_add_param("other", TypeFloat32);
		class_add_func("__imul__", TypeVoid, mf(&FloatList::imul2));
			func_add_param("other", TypeFloat32);
		class_add_func("__idiv__", TypeVoid, mf(&FloatList::idiv2));
			func_add_param("other", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&FloatList::assign_float));
			func_add_param("other", TypeFloat32);
		class_add_func("__lt__", TypeBoolList, mf(&FloatList::lt), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__le__", TypeBoolList, mf(&FloatList::le), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__gt__", TypeBoolList, mf(&FloatList::gt), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__ge__", TypeBoolList, mf(&FloatList::ge), Flags::PURE);
			func_add_param("other", TypeFloatList);
		class_add_func("__lt__", TypeBoolList, mf(&FloatList::lt2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__le__", TypeBoolList, mf(&FloatList::le2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__gt__", TypeBoolList, mf(&FloatList::gt2), Flags::PURE);
			func_add_param("other", TypeFloat32);
		class_add_func("__ge__", TypeBoolList, mf(&FloatList::ge2), Flags::PURE);
			func_add_param("other", TypeFloat32);



	add_class(TypeStringList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &StringList::__init__);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &StringList::clear);
		class_add_funcx("add", TypeVoid, &StringList::add);
			func_add_param("x", TypeString);
		class_add_funcx("clear", TypeVoid, &StringList::clear);
		class_add_funcx("remove", TypeVoid, &StringList::erase);
			func_add_param("index", TypeInt);
		class_add_funcx("resize", TypeVoid, &StringList::resize);
			func_add_param("num", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &StringList::assign);
			func_add_param("other", TypeStringList);
		class_add_funcx("join", TypeString, &StringList::join, Flags::PURE);
			func_add_param("glue", TypeString);
		class_add_funcx("__contains__", TypeBool, &StringList::__contains__, Flags::PURE);
			func_add_param("s", TypeString);
		class_add_funcx("__add__", TypeStringList, &StringList::__add__, Flags::PURE);
			func_add_param("o", TypeStringList);
		class_add_funcx("__adds__", TypeVoid, &StringList::__adds__);
			func_add_param("o", TypeStringList);
		class_add_funcx("str", TypeString, &sa2s, Flags::PURE);


	// constants
	void *kaba_nil = nullptr;
	bool kaba_true = true;
	bool kaba_false = false;
	add_const("nil", TypePointer, (void*)&kaba_nil);
	add_const("false", TypeBool, (void*)&kaba_false);
	add_const("true",  TypeBool, (void*)&kaba_true);


	add_class(TypeException);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaException::__init__);
			func_add_param("message", TypeString);
		class_add_func_virtualx(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaException::__delete__);
		class_add_func_virtualx("message", TypeString, &KabaException::message);
		class_add_element("text", TypeString, config.pointer_size);
		class_set_vtable(KabaException);

	add_funcx(IDENTIFIER_RAISE, TypeVoid, &kaba_raise_exception, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("e", TypeExceptionP);
		
		
	// type casting
	add_func("p2s", TypeString, (void*)&p2s, Flags::_STATIC__PURE);
		func_add_param("p", TypePointer);
	// debug output
	/*add_func("cprint", TypeVoid, (void*)&_cstringout, Flags::STATIC);
		func_add_param("str", TypeCString);*/
	add_func("print", TypeVoid, (void*)&_print, Flags::STATIC);
		func_add_param("str", TypeString);
	add_func("binary", TypeString, (void*)&_binary, Flags::STATIC);
		func_add_param("p", TypePointer);
		func_add_param("length", TypeInt);
	// memory
	add_func("@malloc", TypePointer, (void*)&malloc, Flags::STATIC);
		func_add_param("size", TypeInt);
	add_func("@free", TypeVoid, (void*)&free, Flags::STATIC);
		func_add_param("p", TypePointer);
	// system
	add_func("shell_execute", TypeString, (void*)&kaba_shell_execute, Flags::_STATIC__RAISES_EXCEPTIONS);
		func_add_param("cmd", TypeString);
}



}
