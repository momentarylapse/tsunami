#include "../../file/file.h"
#include "../../math/math.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"
#include "exception.h"

#ifdef _X_USE_ALGEBRA_
	#include "../../algebra/algebra.h"
#else
		typedef int vli;
		typedef int Crypto;
#endif

#ifdef _X_USE_ANY_
	#include "../../any/any.h"
#else
		typedef int Any;
#endif

namespace Kaba{

#ifdef _X_USE_ALGEBRA_
	#define algebra_p(p)		(void*)p
#else
	#define algebra_p(p)		nullptr
#endif

#ifdef _X_USE_ANY_
	#define any_p(p)		(void*)p
#else
	#define any_p(p)		nullptr
#endif

// we're always using math types
#define type_p(p)			(void*)p

extern const Class *TypeStringList;
extern const Class *TypeComplexList;
extern const Class *TypeFloatList;
extern const Class *TypeVectorList;
extern const Class *TypeMatrix;
extern const Class *TypePlane;
extern const Class *TypePlaneList;
extern const Class *TypeColorList;
extern const Class *TypeMatrix3;
extern const Class *TypeIntList;
extern const Class *TypeBoolList;
extern const Class *TypeFloatPs;
extern const Class *TypeAny;


float _cdecl f_sqr(float f){	return f*f;	}

class ComplexList : public Array<complex> {
public:
	complex _cdecl sum() {
		complex r = complex(0, 0);
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	float _cdecl sum2() {
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i].abs_sqr();
		return r;
	}
	
	// a += b
	void _cdecl iadd(ComplexList &b)	IMPLEMENT_IOP(+=, complex)
	void _cdecl isub(ComplexList &b)	IMPLEMENT_IOP(-=, complex)
	void _cdecl imul(ComplexList &b)	IMPLEMENT_IOP(*=, complex)
	void _cdecl idiv(ComplexList &b)	IMPLEMENT_IOP(/=, complex)

	// a = b + c
	Array<complex> _cdecl add(ComplexList &b)	IMPLEMENT_OP(+, complex, complex)
	Array<complex> _cdecl sub(ComplexList &b)	IMPLEMENT_OP(-, complex, complex)
	Array<complex> _cdecl mul(ComplexList &b)	IMPLEMENT_OP(*, complex, complex)
	Array<complex> _cdecl div(ComplexList &b)	IMPLEMENT_OP(/, complex, complex)

	// a += x
	void _cdecl iadd2(complex x)	IMPLEMENT_IOP2(+=, complex)
	void _cdecl isub2(complex x)	IMPLEMENT_IOP2(-=, complex)
	void _cdecl imul2(complex x)	IMPLEMENT_IOP2(*=, complex)
	void _cdecl idiv2(complex x)	IMPLEMENT_IOP2(/=, complex)
	void _cdecl imul2f(float x)	IMPLEMENT_IOP2(*=, complex)
	void _cdecl idiv2f(float x)	IMPLEMENT_IOP2(/=, complex)
	void _cdecl assign_complex(complex x)	IMPLEMENT_IOP2(=, complex)
};

Array<int> _cdecl int_range(int start, int end) {
	Array<int> a;
	//a.__init__(); // done by kaba-constructors for temp variables
	for (int i=start; i<end; i++)
		a.add(i);
	return a;
}

Array<float> _cdecl float_range(float start, float end, float step) {
	Array<float> a;
	for (float f=start; f<end; f+=step)
		a.add(f);
	return a;
}

float _cdecl maxf(float a, float b)
{	return (a > b) ? a : b;	}

float _cdecl minf(float a, float b)
{	return (a < b) ? a : b;	}

vector _quat_vec_mul(quaternion &a, vector &b)
{	return a * b;	}
color _col_mul_c(color &a, color &b)
{	return a * b;	}
color _col_mul_f(color &a, float b)
{	return a * b;	}


complex __complex_set(float x, float y)
{ return complex(x, y); }
color __color_set(float a, float r, float g, float b)
{ return color(a, r, g, b); }
vector __vector_set(float x, float y, float z)
{ return vector(x, y, z); }
rect __rect_set(float x1, float x2, float y1, float y2)
{ return rect(x1, x2, y1, y2); }



complex op_complex_add(complex &a, complex &b) { return a + b; }
complex op_complex_sub(complex &a, complex &b) { return a - b; }
complex op_complex_mul(complex &a, complex &b) { return a * b; }
complex op_complex_div(complex &a, complex &b) { return a / b; }



#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


void kaba_array_resize(void *p, const Class *type, int num);


class KabaAny : public Any {
public:
	Any _cdecl _map_get(const string &key)
	{ KABA_EXCEPTION_WRAPPER(return map_get(key)); return Any(); }
	void _cdecl _map_set(const string &key, Any &a)
	{ KABA_EXCEPTION_WRAPPER(map_set(key, a)); }
	Any _cdecl _array_get(int i)
	{ KABA_EXCEPTION_WRAPPER(return array_get(i)); return Any(); }
	void _cdecl _array_set(int i, Any &a)
	{ KABA_EXCEPTION_WRAPPER(array_set(i, a)); }
	void _cdecl _array_add(Any &a)
	{ KABA_EXCEPTION_WRAPPER(add(a)); }
	
	static void unwrap(Any &aa, void *var, const Class *type) {
		if (type == TypeInt) {
			*(int*)var = aa._int();
		} else if (type == TypeFloat32) {
			*(float*)var = aa._float();
		} else if (type == TypeBool) {
			*(bool*)var = aa._bool();
		} else if (type == TypeString) {
			*(string*)var = aa.str();
		} else if (type->is_pointer()) {
			*(const void**)var = *aa.as_pointer();
		} else if (type->is_super_array() and (aa.type == TYPE_ARRAY)) {
			auto *t_el = type->get_array_element();
			auto *a = (DynamicArray*)var;
			auto *b = aa.as_array();
			int n = b->num;
			kaba_array_resize(var, type, n);
			for (int i=0; i<n; i++)
				unwrap(aa[i], (char*)a->data + i * t_el->size, t_el);
		} else if (type->is_array() and (aa.type == TYPE_ARRAY)) {
			auto *t_el = type->get_array_element();
			auto *b = aa.as_array();
			int n = min(type->array_length, b->num);
			for (int i=0; i<n; i++)
				unwrap((*b)[i], (char*)var + i*t_el->size, t_el);
		} else if (aa.type == TYPE_HASH) {
			auto *map = aa.as_map();
			auto keys = aa.keys();
			for (auto &e: type->elements)
				for (string &k: keys)
					if (e.name == k)
						unwrap(aa[k], (char*)var + e.offset, e.type);
		} else {
			msg_error("unwrap... "  + aa.str() + " -> " + type->long_name());
		}
	}
};

Any kaba_int2any(int i) {
	return Any(i);
}
Any kaba_float2any(float f) {
	return Any(f);
}
Any kaba_bool2any(bool b) {
	return Any(b);
}
Any kaba_str2any(const string &str) {
	return Any(str);
}
Any kaba_pointer2any(const void *p) {
	return Any(p);
}

#pragma GCC pop_options


template<int N>
class FloatN {
public:
	float a[N];
	void __assign__(FloatN<N> &o) {
		for (int i=0; i<N; i++)
			a[i] = o.a[i];
	}
};


void SIAddPackageMath() {
	add_package("math", true);

	// types
	TypeComplex = add_type("complex", sizeof(complex));
	TypeComplexList = add_type_l(TypeComplex);
	TypeVector = add_type("vector", sizeof(vector));
	TypeVectorList = add_type_l(TypeVector);
	TypeRect = add_type("rect", sizeof(rect));
	TypeMatrix = add_type("matrix", sizeof(matrix));
	TypeQuaternion = add_type("quaternion", sizeof(quaternion));
	TypePlane = add_type("plane", sizeof(plane));
	TypePlaneList = add_type_l(TypePlane);
	TypeColor = add_type("color", sizeof(color));
	TypeColorList = add_type_l(TypeColor);
	TypeMatrix3 = add_type("matrix3", sizeof(matrix3));
	const Class *TypeFloatArray3 = add_type_a(TypeFloat32, 3);
	const Class *TypeFloatArray4 = add_type_a(TypeFloat32, 4);
	const Class *TypeFloatArray4x4 = add_type_a(TypeFloatArray4, 4);
	const Class *TypeFloatArray16 = add_type_a(TypeFloat32, 16);
	const Class *TypeFloatArray3x3 = add_type_a(TypeFloatArray3, 3);
	const Class *TypeFloatArray9 = add_type_a(TypeFloat32, 9);
	const Class *TypeVli = add_type("vli", sizeof(vli));
	const Class *TypeCrypto = add_type("Crypto", sizeof(Crypto));
	TypeAny = add_type("any", sizeof(Any));
	const Class *TypeFloatInterpolator = add_type("FloatInterpolator", sizeof(Interpolator<float>));
	const Class *TypeVectorInterpolator = add_type("VectorInterpolator", sizeof(Interpolator<vector>));
	const Class *TypeRandom = add_type("Random", sizeof(Random));
	
	// dirty hack :P
	/*if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64)*/ {
		((Class*)TypeFloat32)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypeFloat64)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypeComplex)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypeQuaternion)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypeVector)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypeColor)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypePlane)->_amd64_allow_pass_in_xmm = true;
		((Class*)TypeRect)->_amd64_allow_pass_in_xmm = true;
	}


	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray3, TypeFloatArray3, InlineID::CHUNK_ASSIGN, mf(&FloatN<3>::__assign__));
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray4, TypeFloatArray4, InlineID::CHUNK_ASSIGN, mf(&FloatN<4>::__assign__));
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray9, TypeFloatArray9, InlineID::CHUNK_ASSIGN, mf(&FloatN<9>::__assign__));
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray3x3, TypeFloatArray3x3, InlineID::CHUNK_ASSIGN, mf(&FloatN<9>::__assign__));
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray16, TypeFloatArray16, InlineID::CHUNK_ASSIGN, mf(&FloatN<16>::__assign__));
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray4x4, TypeFloatArray4x4, InlineID::CHUNK_ASSIGN, mf(&FloatN<16>::__assign__));


	add_class(TypeComplex);
		class_add_elementx("x", TypeFloat32, &complex::x);
		class_add_elementx("y", TypeFloat32, &complex::y);
		class_add_func("abs", TypeFloat32, mf(&complex::abs), FLAG_PURE);
		class_add_func("abs_sqr", TypeFloat32, mf(&complex::abs_sqr), FLAG_PURE);
		class_add_func("bar", TypeComplex, 		mf(&complex::bar), FLAG_PURE);
		class_add_func("str", TypeString, mf(&complex::str), FLAG_PURE);
		class_add_const("I", TypeComplex, &complex::I);
		class_add_func("create", TypeComplex, (void*)__complex_set, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_set_inline(InlineID::COMPLEX_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, (void*)__complex_set);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);	
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeComplex, TypeComplex, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::ADD, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_ADD, (void*)op_complex_add);
		add_operator(OperatorID::SUBTRACT, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_SUBTRACT, (void*)op_complex_sub);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY, (void*)op_complex_mul);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeFloat32, TypeComplex, InlineID::COMPLEX_MULTIPLY_FC);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeFloat32, InlineID::COMPLEX_MULTIPLY_CF);
		add_operator(OperatorID::DIVIDE, TypeComplex, TypeComplex, TypeComplex, InlineID::NONE /*InlineID::COMPLEX_DIVIDE*/, (void*)op_complex_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_SUBTARCT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeComplex, TypeComplex, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NEGATIVE, TypeComplex, nullptr, TypeComplex, InlineID::COMPLEX_NEGATE);

	add_class(TypeComplexList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&ComplexList::__init__));
		class_add_func("sum", TypeComplex, mf(&ComplexList::sum), FLAG_PURE);
		class_add_func("sum2", TypeFloat32, mf(&ComplexList::sum2), FLAG_PURE);
		class_add_func("__iadd__", TypeVoid, mf(&ComplexList::iadd));
			func_add_param("other", TypeComplexList);
		class_add_func("__isub__", TypeVoid, mf(&ComplexList::isub));
			func_add_param("other", TypeComplexList);
		class_add_func("__imul__", TypeVoid, mf(&ComplexList::imul));
			func_add_param("other", TypeComplexList);
		class_add_func("__idiv__", TypeVoid, mf(&ComplexList::idiv));
			func_add_param("other", TypeComplexList);
		class_add_func("__add__", TypeComplexList, mf(&ComplexList::add), FLAG_PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__sub__", TypeComplexList, mf(&ComplexList::sub), FLAG_PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__mul__", TypeComplexList, mf(&ComplexList::mul), FLAG_PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__div__", TypeComplexList, mf(&ComplexList::div), FLAG_PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__iadd__", TypeVoid, mf(&ComplexList::iadd2));
			func_add_param("other", TypeComplex);
		class_add_func("__isub__", TypeVoid, mf(&ComplexList::isub2));
			func_add_param("other", TypeComplex);
		class_add_func("__imul__", TypeVoid, mf(&ComplexList::imul2));
			func_add_param("other", TypeComplex);
		class_add_func("__idiv__", TypeVoid, mf(&ComplexList::idiv2));
			func_add_param("other", TypeComplex);
		class_add_func("__imul__", TypeVoid, mf(&ComplexList::imul2f));
			func_add_param("other", TypeFloat32);
		class_add_func("__idiv__", TypeVoid, mf(&ComplexList::idiv2f));
			func_add_param("other", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&ComplexList::assign_complex));
			func_add_param("other", TypeComplex);

	
	add_class(TypeVector);
		class_add_elementx("x", TypeFloat32, &vector::x);
		class_add_elementx("y", TypeFloat32, &vector::y);
		class_add_elementx("z", TypeFloat32, &vector::z);
		class_add_element("_e", TypeFloatArray3, 0);
		class_add_func("length", TypeFloat32, type_p(mf(&vector::length)), FLAG_PURE);
		class_add_func("length_sqr", TypeFloat32, type_p(mf(&vector::length_sqr)), FLAG_PURE);
		class_add_func("length_fuzzy", TypeFloat32, type_p(mf(&vector::length_fuzzy)), FLAG_PURE);
		class_add_func("normalized", TypeVector, mf(&vector::normalized), FLAG_PURE);
		class_add_func("dir2ang", TypeVector, mf(&vector::dir2ang), FLAG_PURE);
		class_add_func("dir2ang2", TypeVector, mf(&vector::dir2ang2), FLAG_PURE);
			func_add_param("up", TypeVector);
		class_add_func("ang2dir", TypeVector, mf(&vector::ang2dir), FLAG_PURE);
		class_add_func("rotate", TypeVector, mf(&vector::rotate), FLAG_PURE);
			func_add_param("ang", TypeVector);
//		class_add_func("__div__", TypeVector, mf(&vector::untransform), FLAG_PURE);
//			func_add_param("m", TypeMatrix);
		class_add_func("ortho", TypeVector, mf(&vector::ortho), FLAG_PURE);
		class_add_func("str", TypeString, mf(&vector::str), FLAG_PURE);
		class_add_func("dot", TypeFloat32, (void*)&vector::dot, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("v1", TypeVector);
			func_add_param("v2", TypeVector);
		class_add_funcx("cross", TypeVector, &vector::cross, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("v1", TypeVector);
			func_add_param("v2", TypeVector);
		class_add_func("create", TypeVector, (void*)&__vector_set, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_set_inline(InlineID::VECTOR_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
			func_add_param("z", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, (void*)&__vector_set);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
			func_add_param("z", TypeFloat32);
		class_add_func("ang_add", TypeVector, mf(&VecAngAdd), ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
		class_add_func("ang_interpolate", TypeVector, mf(&VecAngInterpolate), ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
			func_add_param("t", TypeFloat32);
		class_add_const("0", TypeVector, (void*)&vector::ZERO);
		class_add_const("O", TypeVector, (void*)&vector::ZERO);
		class_add_const("EX", TypeVector, (void*)&vector::EX);
		class_add_const("EY", TypeVector, (void*)&vector::EY);
		class_add_const("EZ", TypeVector, (void*)&vector::EZ);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeVector, TypeVector, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeVector, TypeVector, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::ADD, TypeVector, TypeVector, TypeVector, InlineID::VECTOR_ADD);
		add_operator(OperatorID::SUBTRACT, TypeVector, TypeVector, TypeVector, InlineID::VECTOR_SUBTRACT);
		add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeVector, TypeVector, InlineID::VECTOR_MULTIPLY_VV);
		add_operator(OperatorID::MULTIPLY, TypeVector, TypeVector, TypeFloat32, InlineID::VECTOR_MULTIPLY_VF);
		add_operator(OperatorID::MULTIPLY, TypeVector, TypeFloat32, TypeVector, InlineID::VECTOR_MULTIPLY_FV);
		add_operator(OperatorID::DIVIDE, TypeVector, TypeVector, TypeFloat32, InlineID::VECTOR_DIVIDE_VF);
		add_operator(OperatorID::ADDS, TypeVoid, TypeVector, TypeVector, InlineID::VECTOR_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeVector, TypeVector, InlineID::VECTOR_SUBTARCT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeVector, TypeFloat32, InlineID::VECTOR_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeVector, TypeFloat32, InlineID::VECTOR_DIVIDE_ASSIGN);
		add_operator(OperatorID::NEGATIVE, TypeVector, nullptr, TypeVector, InlineID::VECTOR_NEGATE);
	
	add_class(TypeVectorList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&Array<vector>::__init__));

	
	add_class(TypeQuaternion);
		class_add_elementx("x", TypeFloat32, &quaternion::x);
		class_add_elementx("y", TypeFloat32, &quaternion::y);
		class_add_elementx("z", TypeFloat32, &quaternion::z);
		class_add_elementx("w", TypeFloat32, &quaternion::w);
		class_add_func("__mul__", TypeQuaternion, mf(&quaternion::mul), FLAG_PURE);
			func_add_param("other", TypeQuaternion);
		class_add_func("__mul__", TypeVector, (void*)&_quat_vec_mul, FLAG_PURE);
			func_add_param("other", TypeVector);
		class_add_func("__imul__", TypeVoid, mf(&quaternion::imul));
			func_add_param("other", TypeQuaternion);
		class_add_func("bar", TypeQuaternion, mf(&quaternion::bar), FLAG_PURE);
		class_add_func("normalize", TypeVoid, mf(&quaternion::normalize));
		class_add_func("angles", TypeVector, mf(&quaternion::get_angles), FLAG_PURE);
		class_add_func("str", TypeString, mf(&quaternion::str), FLAG_PURE);
		class_add_func("rotation", TypeQuaternion, (void*)&quaternion::rotation_v, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang", TypeVector);
		class_add_func("rotation", TypeQuaternion, (void*)&quaternion::rotation_a, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("axis", TypeVector);
			func_add_param("angle", TypeFloat32);
		class_add_func("rotation", TypeQuaternion, (void*)&quaternion::rotation_m, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("m_in", TypeMatrix);
		class_add_func("interpolate", TypeQuaternion, (void*)(quaternion(*)(const quaternion&, const quaternion&, float))&quaternion::interpolate, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("q_0", TypeQuaternion);
			func_add_param("q_1", TypeQuaternion);
			func_add_param("t", TypeFloat32);
		class_add_func("drag", TypeQuaternion, (void*)&quaternion::drag, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("up", TypeVector);
			func_add_param("dang", TypeVector);
			func_add_param("reset_z", TypeBool);
		class_add_const("ID", TypeQuaternion, (void*)&quaternion::ID);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_EQUAL);
	
	add_class(TypeRect);
		class_add_element("x1", TypeFloat32, 0);
		class_add_element("x2", TypeFloat32, 4);
		class_add_element("y1", TypeFloat32, 8);
		class_add_element("y2", TypeFloat32, 12);
		class_add_func("width", TypeFloat32, mf(&rect::width), FLAG_PURE);
		class_add_func("height", TypeFloat32, mf(&rect::height), FLAG_PURE);
		class_add_func("area", TypeFloat32, mf(&rect::area), FLAG_PURE);
		class_add_func("inside", TypeBool, mf(&rect::inside), FLAG_PURE);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_func("str", TypeString, mf(&rect::str), FLAG_PURE);
		class_add_const("ID", TypeRect, (void*)&rect::ID);
		class_add_func("create", TypeRect, (void*)__rect_set, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_set_inline(InlineID::RECT_SET);
			func_add_param("x1", TypeFloat32);
			func_add_param("x2", TypeFloat32);
			func_add_param("y1", TypeFloat32);
			func_add_param("y2", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, (void*)__rect_set);
			func_add_param("x1", TypeFloat32);
			func_add_param("x2", TypeFloat32);
			func_add_param("y1", TypeFloat32);
			func_add_param("y2", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeRect, TypeRect, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeRect, TypeRect, InlineID::CHUNK_EQUAL);
	
	add_class(TypeColor);
		class_add_element("a", TypeFloat32, 12);
		class_add_element("r", TypeFloat32, 0);
		class_add_element("g", TypeFloat32, 4);
		class_add_element("b", TypeFloat32, 8);
		class_add_func("str", TypeString, mf(&color::str), FLAG_PURE);
		class_add_func("__add__", TypeColor, mf(&color::operator+), FLAG_PURE);
			func_add_param("o", TypeColor);
		class_add_func("__adds__", TypeVoid, mf(&color::operator+=));
			func_add_param("o", TypeColor);
		class_add_func("__sub__", TypeColor, mf(&color::operator-), FLAG_PURE);
			func_add_param("o", TypeColor);
		class_add_func("__subs__", TypeVoid, mf(&color::operator-=));
			func_add_param("o", TypeColor);
		class_add_func("__mul__", TypeColor, (void*)&_col_mul_f, FLAG_PURE);
			func_add_param("f", TypeFloat32);
		class_add_func("__mul__", TypeColor, (void*)&_col_mul_c, FLAG_PURE);
			func_add_param("c", TypeColor);
		class_add_funcx("hsb", TypeColor, &SetColorHSB, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("a", TypeFloat32);
			func_add_param("h", TypeFloat32);
			func_add_param("s", TypeFloat32);
			func_add_param("b", TypeFloat32);
		class_add_funcx("interpolate", TypeColor, &ColorInterpolate, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("c1", TypeColor);
			func_add_param("c2", TypeColor);
			func_add_param("t", TypeFloat32);
		class_add_func("create", TypeColor, (void*)&__color_set, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_set_inline(InlineID::COLOR_SET);
			func_add_param("a", TypeFloat32);
			func_add_param("r", TypeFloat32);
			func_add_param("g", TypeFloat32);
			func_add_param("b", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, (void*)&__color_set);
			func_add_param("a", TypeFloat32);
			func_add_param("r", TypeFloat32);
			func_add_param("g", TypeFloat32);
			func_add_param("b", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeColor, TypeColor, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeColor, TypeColor, InlineID::CHUNK_EQUAL);

	add_class(TypeColorList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&Array<color>::__init__));

	add_class(TypePlane);
		class_add_element("a", TypeFloat32, 0);
		class_add_element("b", TypeFloat32, 4);
		class_add_element("c", TypeFloat32, 8);
		class_add_element("d", TypeFloat32, 12);
		class_add_element("n", TypeVector, 0);
		class_add_func("intersect_line", TypeBool, mf(&plane::intersect_line), FLAG_PURE);
			func_add_param("l1", TypeVector);
			func_add_param("l2", TypeVector);
			func_add_param("inter", TypeVector);
		class_add_func("inverse", TypeVoid, mf(&plane::inverse), FLAG_PURE);
		class_add_func("distance", TypeFloat32, mf(&plane::distance), FLAG_PURE);
			func_add_param("p", TypeVector);
		class_add_func("str", TypeString, mf(&plane::str), FLAG_PURE);
		class_add_func("transform", TypePlane, mf(&plane::transform), FLAG_PURE);
			func_add_param("m", TypeMatrix);
		class_add_func("from_points", TypePlane, (void*)&plane::from_points, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("a", TypeVector);
			func_add_param("b", TypeVector);
			func_add_param("c", TypeVector);
		class_add_func("from_point_normal", TypePlane, (void*)&plane::from_point_normal, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("p", TypeVector);
			func_add_param("n", TypeVector);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePlane, TypePlane, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypePlane, TypePlane, InlineID::CHUNK_EQUAL);
	
	add_class(TypePlaneList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&Array<plane>::__init__));
	
	add_class(TypeMatrix);
		class_add_element("_00", TypeFloat32, 0);
		class_add_element("_10", TypeFloat32, 4);
		class_add_element("_20", TypeFloat32, 8);
		class_add_element("_30", TypeFloat32, 12);
		class_add_element("_01", TypeFloat32, 16);
		class_add_element("_11", TypeFloat32, 20);
		class_add_element("_21", TypeFloat32, 24);
		class_add_element("_31", TypeFloat32, 28);
		class_add_element("_02", TypeFloat32, 32);
		class_add_element("_12", TypeFloat32, 36);
		class_add_element("_22", TypeFloat32, 40);
		class_add_element("_32", TypeFloat32, 44);
		class_add_element("_03", TypeFloat32, 48);
		class_add_element("_13", TypeFloat32, 52);
		class_add_element("_23", TypeFloat32, 56);
		class_add_element("_33", TypeFloat32, 60);
		class_add_element("e", TypeFloatArray4x4, 0);
		class_add_element("_e", TypeFloatArray16, 0);
		class_add_func("__imul__", TypeVoid, mf(&matrix::imul));
			func_add_param("other", TypeMatrix);
		class_add_func("__mul__", TypeMatrix, mf(&matrix::mul), FLAG_PURE);
			func_add_param("other", TypeMatrix);
		class_add_func("__mul__", TypeVector, mf(&matrix::mul_v), FLAG_PURE);
			func_add_param("other", TypeVector);
		class_add_func("str", TypeString, mf(&matrix::str), FLAG_PURE);
		class_add_func("transform", TypeVector, mf(&matrix::transform), FLAG_PURE);
			func_add_param("v", TypeVector);
		class_add_func("transform_normal", TypeVector, mf(&matrix::transform_normal), FLAG_PURE);
			func_add_param("v", TypeVector);
		class_add_func("untransform", TypeVector, mf(&matrix::untransform), FLAG_PURE);
			func_add_param("v", TypeVector);
		class_add_func("project", TypeVector, mf(&matrix::project), FLAG_PURE);
			func_add_param("v", TypeVector);
		class_add_func("unproject", TypeVector, mf(&matrix::unproject), FLAG_PURE);
			func_add_param("v", TypeVector);
		class_add_func("inverse", TypeMatrix, mf(&matrix::inverse), FLAG_PURE);
		class_add_func("transpose", TypeMatrix, mf(&matrix::transpose), FLAG_PURE);
		class_add_func("translation", TypeMatrix, (void*)&matrix::translation, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("trans", TypeVector);
		class_add_func("rotation", TypeMatrix, (void*)&matrix::rotation_v, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang", TypeVector);
		class_add_func("rotation_x", TypeMatrix, (void*)&matrix::rotation_x, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation_y", TypeMatrix, (void*)&matrix::rotation_y, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation_z", TypeMatrix, (void*)&matrix::rotation_z, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation", TypeMatrix, (void*)&matrix::rotation_q, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("ang", TypeQuaternion);
		class_add_func("scale", TypeMatrix, (void*)&matrix::scale, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("s_x", TypeFloat32);
			func_add_param("s_y", TypeFloat32);
			func_add_param("s_z", TypeFloat32);
		class_add_func("perspective", TypeMatrix, (void*)&matrix::perspective, ScriptFlag(FLAG_PURE | FLAG_STATIC));
			func_add_param("fovy", TypeFloat32);
			func_add_param("aspect", TypeFloat32);
			func_add_param("z_near", TypeFloat32);
			func_add_param("z_far", TypeFloat32);
		class_add_const("ID", TypeMatrix, (void*)&matrix::ID);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeMatrix, TypeMatrix, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeMatrix, TypeMatrix, InlineID::CHUNK_EQUAL);
	
	add_class(TypeMatrix3);
		class_add_element("_11", TypeFloat32, 0);
		class_add_element("_21", TypeFloat32, 4);
		class_add_element("_31", TypeFloat32, 8);
		class_add_element("_12", TypeFloat32, 12);
		class_add_element("_22", TypeFloat32, 16);
		class_add_element("_32", TypeFloat32, 20);
		class_add_element("_13", TypeFloat32, 24);
		class_add_element("_23", TypeFloat32, 28);
		class_add_element("_33", TypeFloat32, 32);
		class_add_element("e", TypeFloatArray3x3, 0);
		class_add_element("_e", TypeFloatArray9, 0);
		class_add_func("__mul__", TypeMatrix3, mf(&matrix3::mul), FLAG_PURE);
			func_add_param("other", TypeMatrix3);
		class_add_func("__mul__", TypeVector, mf(&matrix3::mul_v), FLAG_PURE);
			func_add_param("other", TypeVector);
		class_add_func("str", TypeString, mf(&matrix3::str), FLAG_PURE);
		class_add_func("inverse", TypeMatrix3, mf(&matrix3::inverse), FLAG_PURE);
		class_add_const("ID", TypeMatrix3, (void*)&matrix3::ID);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeMatrix3, TypeMatrix3, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeMatrix3, TypeMatrix3, InlineID::CHUNK_EQUAL);
	
	add_class(TypeVli);
		class_add_element("sign", TypeBool, 0);
		class_add_element("data", TypeIntList, 4);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, algebra_p(mf(&vli::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, algebra_p(mf(&vli::__delete__)));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, algebra_p(mf(&vli::set_vli)));
			func_add_param("v", TypeVli);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, algebra_p(mf(&vli::set_str)));
			func_add_param("s", TypeString);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, algebra_p(mf(&vli::set_int)));
			func_add_param("i", TypeInt);
		class_add_func("str", TypeString, algebra_p(mf(&vli::to_string)), FLAG_PURE);
		class_add_func("compare", TypeInt, algebra_p(mf(&vli::compare)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__eq__", TypeBool, algebra_p(mf(&vli::operator==)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__ne__", TypeBool, algebra_p(mf(&vli::operator!=)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__lt__", TypeBool, algebra_p(mf(&vli::operator<)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__gt__", TypeBool, algebra_p(mf(&vli::operator>)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__le__", TypeBool, algebra_p(mf(&vli::operator<=)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__ge__", TypeBool, algebra_p(mf(&vli::operator>=)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__iadd__", TypeVoid, algebra_p(mf(&vli::operator+=)));
			func_add_param("v", TypeVli);
		class_add_func("__isub__", TypeVoid, algebra_p(mf(&vli::operator-=)));
			func_add_param("v", TypeVli);
		class_add_func("__imul__", TypeVoid, algebra_p(mf(&vli::operator*=)));
			func_add_param("v", TypeVli);
		class_add_func("__add__", TypeVli, algebra_p(mf(&vli::operator+)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__sub__", TypeVli, algebra_p(mf(&vli::operator-)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("__mul__", TypeVli, algebra_p(mf(&vli::operator*)), FLAG_PURE);
			func_add_param("v", TypeVli);
		class_add_func("idiv", TypeVoid, algebra_p(mf(&vli::idiv)));
			func_add_param("div", TypeVli);
			func_add_param("rem", TypeVli);
		class_add_func("div", TypeVli, algebra_p(mf(&vli::_div)), FLAG_PURE);
			func_add_param("div", TypeVli);
			func_add_param("rem", TypeVli);
		class_add_func("pow", TypeVli, algebra_p(mf(&vli::pow)), FLAG_PURE);
			func_add_param("exp", TypeVli);
		class_add_func("pow_mod", TypeVli, algebra_p(mf(&vli::pow_mod)), FLAG_PURE);
			func_add_param("exp", TypeVli);
			func_add_param("mod", TypeVli);
		class_add_func("gcd", TypeVli, algebra_p(mf(&vli::gcd)), FLAG_PURE);
			func_add_param("v", TypeVli);
	
	add_class(TypeAny);
		class_add_element("type", TypeInt, 0);
		class_add_element("data", TypePointer, 4);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, any_p(mf(&Any::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, any_p(mf(&Any::clear)));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, any_p(mf(&Any::set)));
			func_add_param("a", TypeAny);
		class_add_func("__iadd__", TypeVoid, any_p(mf(&Any::_add)));
			func_add_param("a", TypeAny);
		class_add_func("__isub__", TypeVoid, any_p(mf(&Any::_sub)));
			func_add_param("a", TypeAny);
		class_add_func("clear", TypeVoid, any_p(mf(&Any::clear)));
		class_add_func("length", TypeInt, any_p(mf(&Any::length)));
		class_add_func("__get__", TypeAny, any_p(mf(&KabaAny::_map_get)), FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func("set", TypeVoid, any_p(mf(&KabaAny::_map_set)), FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
			func_add_param("value", TypeAny);
		class_add_func("__get__", TypeAny, any_p(mf(&KabaAny::_array_get)), FLAG_RAISES_EXCEPTIONS);
			func_add_param("index", TypeInt);
		class_add_func("set", TypeVoid, any_p(mf(&KabaAny::_array_set)), FLAG_RAISES_EXCEPTIONS);
			func_add_param("index", TypeInt);
			func_add_param("value", TypeAny);
		class_add_func("add", TypeVoid, any_p(mf(&KabaAny::_array_add)), FLAG_RAISES_EXCEPTIONS);
			func_add_param("a", TypeAny);
		class_add_func("keys", TypeStringList, any_p(mf(&Any::keys)));//, FLAG_RAISES_EXCEPTIONS);
		class_add_func("bool", TypeBool, any_p(mf(&Any::_bool)));
		class_add_func("int", TypeInt, any_p(mf(&Any::_int)));
		class_add_func("float", TypeFloat32, any_p(mf(&Any::_float)));
		class_add_func("str", TypeString, any_p(mf(&Any::str)));
		class_add_func("repr", TypeString, any_p(mf(&Any::repr)));
		class_add_func("unwrap", TypeVoid, (void*)&KabaAny::unwrap, FLAG_RAISES_EXCEPTIONS);
			func_add_param("var", TypePointer);
			func_add_param("type", TypeClassP);


	add_funcx("@int2any", TypeAny, &kaba_int2any, FLAG_STATIC);
		func_add_param("i", TypeInt);
	add_funcx("@float2any", TypeAny, &kaba_float2any, FLAG_STATIC);
		func_add_param("i", TypeFloat32);
	add_funcx("@bool2any", TypeAny, &kaba_bool2any, FLAG_STATIC);
		func_add_param("i", TypeBool);
	add_funcx("@str2any", TypeAny, &kaba_str2any, FLAG_STATIC);
		func_add_param("s", TypeString);
	add_funcx("@pointer2any", TypeAny, &kaba_pointer2any, FLAG_STATIC);
		func_add_param("p", TypePointer);


	add_class(TypeCrypto);
		class_add_element("n", TypeVli, 0);
		class_add_element("k", TypeVli, sizeof(vli));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, algebra_p(mf(&Crypto::__init__)));
		class_add_func("str", TypeString, algebra_p(mf(&Crypto::str)));
		class_add_func("from_str", TypeVoid, algebra_p(mf(&Crypto::from_str)));
			func_add_param("str", TypeString);
		class_add_func("encrypt", TypeString, algebra_p(mf(&Crypto::Encrypt)));
			func_add_param("str", TypeString);
		class_add_func("decrypt", TypeString, algebra_p(mf(&Crypto::Decrypt)));
			func_add_param("str", TypeString);
			func_add_param("cut", TypeBool);
		class_add_func("create_keys", TypeVoid, algebra_p(&CryptoCreateKeys), FLAG_STATIC);
			func_add_param("c1", TypeCrypto);
			func_add_param("c2", TypeCrypto);
			func_add_param("type", TypeString);
			func_add_param("bits", TypeInt);

	add_class(TypeRandom);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&Random::__init__));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&Random::__assign__));
			func_add_param("o", TypeRandom);
		//class_add_element("n", TypeRandom, 0);
		class_add_func("seed", TypeVoid, mf(&Random::seed));
			func_add_param("str", TypeString);
		class_add_func("int", TypeInt, mf(&Random::_int));
			func_add_param("max", TypeInt);
		class_add_func("uniform01", TypeFloat32, mf(&Random::uniform01));
		class_add_func("uniform", TypeFloat32, mf(&Random::uniform));
			func_add_param("min", TypeFloat32);
			func_add_param("max", TypeFloat32);
		class_add_func("normal", TypeFloat32, mf(&Random::normal));
			func_add_param("mean", TypeFloat32);
			func_add_param("stddev", TypeFloat32);
		class_add_func("in_ball", TypeVector, mf(&Random::in_ball));
			func_add_param("r", TypeFloat32);
		class_add_func("dir", TypeVector, mf(&Random::dir));
	
	
	add_class(TypeFloatInterpolator);
		class_add_element("type", TypeInt, 0);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&Interpolator<float>::__init__));
		class_add_func("clear", TypeVoid, mf(&Interpolator<float>::clear));
		class_add_func("set_type", TypeVoid, mf(&Interpolator<float>::setType));
			func_add_param("type", TypeString);
		class_add_func("add", TypeVoid, mf(&Interpolator<float>::addv));
			func_add_param("p", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("add2", TypeVoid, mf(&Interpolator<float>::add2v));
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("add3", TypeVoid, mf(&Interpolator<float>::add3v));
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
			func_add_param("w", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("jump", TypeVoid, mf(&Interpolator<float>::jumpv));
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
		class_add_func("normalize", TypeVoid, mf(&Interpolator<float>::normalize));
		class_add_func("get", TypeFloat32, mf(&Interpolator<float>::get));
			func_add_param("t", TypeFloat32);
		class_add_func("get_tang", TypeFloat32, mf(&Interpolator<float>::getTang));
			func_add_param("t", TypeFloat32);
		class_add_func("get_list", TypeFloatList, mf(&Interpolator<float>::getList));
			func_add_param("t", TypeFloatList);

	
	add_class(TypeVectorInterpolator);
		class_add_element("type", TypeInt, 0);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&Interpolator<vector>::__init__));
		class_add_func("clear", TypeVoid, mf(&Interpolator<vector>::clear));
		class_add_func("set_type", TypeVoid, mf(&Interpolator<vector>::setType));
			func_add_param("type", TypeString);
		class_add_func("add", TypeVoid, mf(&Interpolator<vector>::add));
			func_add_param("p", TypeVector);
			func_add_param("dt", TypeFloat32);
		class_add_func("add2", TypeVoid, mf(&Interpolator<vector>::add2));
			func_add_param("p", TypeVector);
			func_add_param("v", TypeVector);
			func_add_param("dt", TypeFloat32);
		class_add_func("add3", TypeVoid, mf(&Interpolator<vector>::add3));
			func_add_param("p", TypeVector);
			func_add_param("v", TypeVector);
			func_add_param("w", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("jump", TypeVoid, mf(&Interpolator<vector>::jump));
			func_add_param("p", TypeVector);
			func_add_param("v", TypeVector);
		class_add_func("normalize", TypeVoid, mf(&Interpolator<vector>::normalize));
		class_add_func("get", TypeVector, mf(&Interpolator<vector>::get));
			func_add_param("t", TypeFloat32);
		class_add_func("get_tang", TypeVector, mf(&Interpolator<vector>::getTang));
			func_add_param("t", TypeFloat32);
		class_add_func("get_list", TypeVectorList, mf(&Interpolator<vector>::getList));
			func_add_param("t", TypeFloatList);

	// mathematical
	add_func("sin", TypeFloat32, (void*)&sinf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("cos", TypeFloat32, (void*)&cosf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("tan", TypeFloat32, (void*)&tanf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("asin", TypeFloat32, (void*)&asinf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("acos", TypeFloat32, (void*)&acosf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("atan", TypeFloat32, (void*)&atanf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("atan2", TypeFloat32, (void*)&atan2f, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
		func_add_param("y", TypeFloat32);
	add_func("sqrt", TypeFloat32, (void*)&sqrtf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("sqr", TypeFloat32, (void*)&f_sqr, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("exp", TypeFloat32, (void*)&expf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("log", TypeFloat32, (void*)&logf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
	add_func("pow", TypeFloat32, (void*)&powf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("x", TypeFloat32);
		func_add_param("exp", TypeFloat32);
	add_func("clamp", TypeFloat32, (void*)&clampf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("f", TypeFloat32);
		func_add_param("min", TypeFloat32);
		func_add_param("max", TypeFloat32);
	add_func("loop", TypeFloat32, (void*)&loopf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("f", TypeFloat32);
		func_add_param("min", TypeFloat32);
		func_add_param("max", TypeFloat32);
	add_func("abs", TypeFloat32, (void*)&fabsf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("f", TypeFloat32);
	add_func("min", TypeFloat32, (void*)&minf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("a", TypeFloat32);
		func_add_param("b", TypeFloat32);
	add_func("max", TypeFloat32, (void*)&maxf, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("a", TypeFloat32);
		func_add_param("b", TypeFloat32);
	// int
	add_func("clampi", TypeInt, (void*)&clampi, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("i", TypeInt);
		func_add_param("min", TypeInt);
		func_add_param("max", TypeInt);
	add_func("loopi", TypeInt, (void*)&loopi, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("i", TypeInt);
		func_add_param("min", TypeInt);
		func_add_param("max", TypeInt);
	// lists
	add_func("range", TypeIntList, (void*)&int_range, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("start", TypeInt);
		func_add_param("end", TypeInt);
	add_func("rangef", TypeFloatList, (void*)&float_range, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("start", TypeFloat32);
		func_add_param("end", TypeFloat32);
		func_add_param("step", TypeFloat32);
	// other types
	add_func("bary_centric", TypeVoid, (void*)&GetBaryCentric, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_add_param("p", TypeVector);
		func_add_param("a", TypeVector);
		func_add_param("b", TypeVector);
		func_add_param("c", TypeVector);
		func_add_param("f", TypeFloatPs);
		func_add_param("g", TypeFloatPs);
	// random numbers
	add_func("randi", TypeInt, (void*)&randi, FLAG_STATIC);
		func_add_param("max", TypeInt);
	add_func("rand", TypeFloat32, (void*)&randf, FLAG_STATIC);
		func_add_param("max", TypeFloat32);
	add_func("rand_seed", TypeVoid, (void*)&srand, FLAG_STATIC);
		func_add_param("seed", TypeInt);

	
	// float
	add_const("pi",  TypeFloat32, (void*)&pi);
	// color
	add_const("White",  TypeColor, (void*)&White);
	add_const("Black",  TypeColor, (void*)&Black);
	add_const("Gray",   TypeColor, (void*)&Gray);
	add_const("Red",    TypeColor, (void*)&Red);
	add_const("Green",  TypeColor, (void*)&Green);
	add_const("Blue",   TypeColor, (void*)&Blue);
	add_const("Yellow", TypeColor, (void*)&Yellow);
	add_const("Orange", TypeColor, (void*)&Orange);
}

};
