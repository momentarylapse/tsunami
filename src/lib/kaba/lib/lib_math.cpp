#include "../../file/file.h"
#include "../../math/math.h"
#include "../../base/map.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "../dynamic/exception.h"

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

namespace kaba {

#ifdef _X_USE_ALGEBRA_
	#define algebra_p(p)		p
#else
	#define algebra_p(p)		nullptr
#endif

#ifdef _X_USE_ANY_
	#define any_p(p)		p
#else
	#define any_p(p)		nullptr
#endif

// we're always using math types
#define type_p(p)			p

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
extern const Class *TypeAnyList;
extern const Class *TypeAnyDict;


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

class AnyList : public Array<Any> {
public:
	void __delete__() {
		this->~AnyList();
	}
	void assign(const AnyList &o) {
		*this = o;
	}
};

class AnyDict : public Map<string,Any> {
public:
	void __delete__() {
		this->~AnyDict();
	}
	void assign(AnyDict &o) {
		*this = o;
	}
	Any get_item(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return Any(); }
	string str()
	{ return var2str(this, TypeAnyDict); }
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

template<class T>
T _cdecl x_max(T a, T b) {
	return max(a,b);
}

template<class T>
T _cdecl x_min(T a, T b) {
	return min(a, b);
}

template<class T>
T _cdecl x_abs(T x) {
	return abs(x);
}

vector _quat_vec_mul(quaternion &a, vector &b)
{	return a * b;	}


complex __complex_set(float x, float y)
{ return complex(x, y); }
color __color_set(float r, float g, float b, float a)
{ return color(a, r, g, b); }
vector __vector_set(float x, float y, float z)
{ return vector(x, y, z); }
rect __rect_set(float x1, float x2, float y1, float y2)
{ return rect(x1, x2, y1, y2); }



complex op_complex_add(complex &a, complex &b) { return a + b; }
complex op_complex_sub(complex &a, complex &b) { return a - b; }
complex op_complex_mul(complex &a, complex &b) { return a * b; }
complex op_complex_div(complex &a, complex &b) { return a / b; }

class KabaComplex : public complex {
public:
	void assign(const complex &o) {
		*(complex*)this = o;
	}
};

class KabaVector : public vector {
public:
	void assign(const vector &o) {
		*(vector*)this = o;
	}
};

class KabaRect : public rect{
public:
	void assign(const rect& o) {
		*(rect*)this = o;
	}
};


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


void kaba_array_resize(void *p, const Class *type, int num);


class KabaAny : public Any {
public:
	Any _cdecl _map_get(const string &key)
	//{ return map_get(key); }
	{ KABA_EXCEPTION_WRAPPER(return map_get(key)); return Any(); }
	void _cdecl _map_set(const string &key, Any &a)
	{ KABA_EXCEPTION_WRAPPER(map_set(key, a)); }
	Any _cdecl _array_get(int i)
	{ KABA_EXCEPTION_WRAPPER(return array_get(i)); return Any(); }
	void _cdecl _array_set(int i, Any &a)
	{ KABA_EXCEPTION_WRAPPER(array_set(i, a)); }
	void _cdecl _array_add(Any &a)
	{ KABA_EXCEPTION_WRAPPER(add(a)); }
	Array<Any> _as_array() {
		if (type != TYPE_ARRAY)
			kaba_raise_exception(new KabaException("not an array"));
		Array<Any> r;
		r.set_ref(as_array());
		return r;
	}
	Array<int> _as_map() { // FAKE TYPE!!!
		if (type != TYPE_MAP)
			kaba_raise_exception(new KabaException("not a map"));
		Array<int> r;
		r.set_ref((Array<int>&)as_map());
		return r;
	}
	
	static void unwrap(Any &aa, void *var, const Class *type) {
		if (type == TypeInt) {
			*(int*)var = aa.as_int();
		} else if (type == TypeFloat32) {
			*(float*)var = aa.as_float();
		} else if (type == TypeBool) {
			*(bool*)var = aa.as_bool();
		} else if (type == TypeString) {
			*(string*)var = aa.as_string();
		} else if (type->is_pointer()) {
			*(const void**)var = aa.as_pointer();
		} else if (type->is_super_array() and (aa.type == TYPE_ARRAY)) {
			auto *t_el = type->get_array_element();
			auto *a = (DynamicArray*)var;
			auto &b = aa.as_array();
			int n = b.num;
			kaba_array_resize(var, type, n);
			for (int i=0; i<n; i++)
				unwrap(aa[i], (char*)a->data + i * t_el->size, t_el);
		} else if (type->is_array() and (aa.type == TYPE_ARRAY)) {
			auto *t_el = type->get_array_element();
			auto &b = aa.as_array();
			int n = min(type->array_length, b.num);
			for (int i=0; i<n; i++)
				unwrap(b[i], (char*)var + i*t_el->size, t_el);
		} else if (aa.type == TYPE_MAP) {
			auto &map = aa.as_map();
			auto keys = aa.keys();
			for (auto &e: type->elements)
				for (string &k: keys)
					if (e.name == k)
						unwrap(aa[k], (char*)var + e.offset, e.type);
		} else {
			msg_error("unwrap... "  + aa.str() + " -> " + type->long_name());
		}
	}
	static Any _cdecl parse(const string &s)
	{ KABA_EXCEPTION_WRAPPER(return Any::parse(s)); return Any(); }
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

class KabaColor : public color {
public:
	color mul_f(float f) const {
		return *this * f;
	}
	color mul_c(const color &c) const {
		return *this * c;
	}
	void init(float r, float g, float b, float a) {
		*(color*)this = color(a, r, g, b);
	}
	void assign(const color &o) {
		*(color*)this = o;
	}
};


void SIAddPackageMath() {
	add_package("math", Flags::AUTO_IMPORT);

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
	auto TypeFloatArray3 = add_type_a(TypeFloat32, 3);
	auto TypeFloatArray4 = add_type_a(TypeFloat32, 4);
	auto TypeFloatArray4x4 = add_type_a(TypeFloatArray4, 4);
	auto TypeFloatArray16 = add_type_a(TypeFloat32, 16);
	auto TypeFloatArray3x3 = add_type_a(TypeFloatArray3, 3);
	auto TypeFloatArray9 = add_type_a(TypeFloat32, 9);
	auto TypeVli = add_type("vli", sizeof(vli));
	auto TypeCrypto = add_type("Crypto", sizeof(Crypto));
	TypeAny = add_type("any", sizeof(Any));
	auto TypeFloatInterpolator = add_type("FloatInterpolator", sizeof(Interpolator<float>));
	auto TypeVectorInterpolator = add_type("VectorInterpolator", sizeof(Interpolator<vector>));
	auto TypeRandom = add_type("Random", sizeof(Random));
	
	// dirty hack :P
	/*if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64)*/ {
		flags_set(((Class*)TypeFloat32)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
		flags_set(((Class*)TypeFloat64)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
		if (config.abi == Abi::AMD64_GNU) {
			// not on windows!
			flags_set(((Class*)TypeComplex)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeQuaternion)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeVector)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeColor)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypePlane)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeRect)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
		}
	}


	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray3, TypeFloatArray3, InlineID::CHUNK_ASSIGN, &FloatN<3>::__assign__);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray4, TypeFloatArray4, InlineID::CHUNK_ASSIGN, &FloatN<4>::__assign__);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray9, TypeFloatArray9, InlineID::CHUNK_ASSIGN, &FloatN<9>::__assign__);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray3x3, TypeFloatArray3x3, InlineID::CHUNK_ASSIGN, &FloatN<9>::__assign__);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray16, TypeFloatArray16, InlineID::CHUNK_ASSIGN, &FloatN<16>::__assign__);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatArray4x4, TypeFloatArray4x4, InlineID::CHUNK_ASSIGN, &FloatN<16>::__assign__);


	add_class(TypeComplex);
		class_add_element("x", TypeFloat32, &complex::x);
		class_add_element("y", TypeFloat32, &complex::y);
		class_add_func("abs", TypeFloat32, &complex::abs, Flags::PURE);
		class_add_func("abs_sqr", TypeFloat32, &complex::abs_sqr, Flags::PURE);
		class_add_func("bar", TypeComplex, &complex::bar, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &complex::str, Flags::PURE);
		class_add_const("I", TypeComplex, &complex::I);
		class_add_func("_create", TypeComplex, &__complex_set, Flags::_STATIC__PURE);
			func_set_inline(InlineID::COMPLEX_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &__complex_set);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);	
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeComplex, TypeComplex, InlineID::CHUNK_ASSIGN, &KabaComplex::assign);
		add_operator(OperatorID::ADD, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_ADD, &op_complex_add);
		add_operator(OperatorID::SUBTRACT, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_SUBTRACT, &op_complex_sub);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY, &op_complex_mul);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeFloat32, TypeComplex, InlineID::COMPLEX_MULTIPLY_FC);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeFloat32, InlineID::COMPLEX_MULTIPLY_CF);
		add_operator(OperatorID::DIVIDE, TypeComplex, TypeComplex, TypeComplex, InlineID::NONE /*InlineID::COMPLEX_DIVIDE*/, op_complex_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_SUBTARCT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeComplex, TypeComplex, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NEGATIVE, TypeComplex, nullptr, TypeComplex, InlineID::COMPLEX_NEGATE);

	add_class(TypeComplexList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &ComplexList::__init__);
		class_add_func("sum", TypeComplex, &ComplexList::sum, Flags::PURE);
		class_add_func("sum2", TypeFloat32, &ComplexList::sum2, Flags::PURE);
		class_add_func("__iadd__", TypeVoid, &ComplexList::iadd);
			func_add_param("other", TypeComplexList);
		class_add_func("__isub__", TypeVoid, &ComplexList::isub);
			func_add_param("other", TypeComplexList);
		class_add_func("__imul__", TypeVoid, &ComplexList::imul);
			func_add_param("other", TypeComplexList);
		class_add_func("__idiv__", TypeVoid, &ComplexList::idiv);
			func_add_param("other", TypeComplexList);
		class_add_func("__add__", TypeComplexList, &ComplexList::add, Flags::PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__sub__", TypeComplexList, &ComplexList::sub, Flags::PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__mul__", TypeComplexList, &ComplexList::mul, Flags::PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__div__", TypeComplexList, &ComplexList::div, Flags::PURE);
			func_add_param("other", TypeComplexList);
		class_add_func("__iadd__", TypeVoid, &ComplexList::iadd2);
			func_add_param("other", TypeComplex);
		class_add_func("__isub__", TypeVoid, &ComplexList::isub2);
			func_add_param("other", TypeComplex);
		class_add_func("__imul__", TypeVoid, &ComplexList::imul2);
			func_add_param("other", TypeComplex);
		class_add_func("__idiv__", TypeVoid, &ComplexList::idiv2);
			func_add_param("other", TypeComplex);
		class_add_func("__imul__", TypeVoid, &ComplexList::imul2f);
			func_add_param("other", TypeFloat32);
		class_add_func("__idiv__", TypeVoid, &ComplexList::idiv2f);
			func_add_param("other", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &ComplexList::assign_complex);
			func_add_param("other", TypeComplex);

	
	add_class(TypeVector);
		class_add_element("x", TypeFloat32, &vector::x);
		class_add_element("y", TypeFloat32, &vector::y);
		class_add_element("z", TypeFloat32, &vector::z);
		class_add_element("_e", TypeFloatArray3, 0);
		class_add_func(IDENTIFIER_FUNC_LENGTH, TypeFloat32, type_p(&vector::length), Flags::PURE);
		class_add_func("length", TypeFloat32, type_p(&vector::length), Flags::PURE);
		class_add_func("length_sqr", TypeFloat32, type_p(&vector::length_sqr), Flags::PURE);
		class_add_func("length_fuzzy", TypeFloat32, type_p(&vector::length_fuzzy), Flags::PURE);
		class_add_func("normalized", TypeVector, &vector::normalized, Flags::PURE);
		class_add_func("dir2ang", TypeVector, &vector::dir2ang, Flags::PURE);
		class_add_func("dir2ang2", TypeVector, &vector::dir2ang2, Flags::PURE);
			func_add_param("up", TypeVector);
		class_add_func("ang2dir", TypeVector, &vector::ang2dir, Flags::PURE);
		class_add_func("rotate", TypeVector, &vector::rotate, Flags::PURE);
			func_add_param("ang", TypeVector);
//		class_add_func("__div__", TypeVector, vector::untransform), Flags::PURE);
//			func_add_param("m", TypeMatrix);
		class_add_func("ortho", TypeVector, &vector::ortho, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &vector::str, Flags::PURE);
		class_add_func("dot", TypeFloat32, &vector::dot, Flags::_STATIC__PURE);
			func_add_param("v1", TypeVector);
			func_add_param("v2", TypeVector);
		class_add_func("cross", TypeVector, &vector::cross, Flags::_STATIC__PURE);
			func_add_param("v1", TypeVector);
			func_add_param("v2", TypeVector);
		class_add_func("_create", TypeVector, &__vector_set, Flags::_STATIC__PURE);
			func_set_inline(InlineID::VECTOR_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
			func_add_param("z", TypeFloat32);
		// ignored, but useful for docu
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &__vector_set);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
			func_add_param("z", TypeFloat32);
		class_add_func("ang_add", TypeVector, &VecAngAdd, Flags::_STATIC__PURE);
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
		class_add_func("ang_interpolate", TypeVector, &VecAngInterpolate, Flags::_STATIC__PURE);
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
			func_add_param("t", TypeFloat32);
		class_add_const("0", TypeVector, &vector::ZERO);
		class_add_const("O", TypeVector, &vector::ZERO);
		class_add_const("EX", TypeVector, &vector::EX);
		class_add_const("EY", TypeVector, &vector::EY);
		class_add_const("EZ", TypeVector, &vector::EZ);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeVector, TypeVector, InlineID::CHUNK_ASSIGN, &KabaVector::assign);
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
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<vector>::__init__);

	
	add_class(TypeQuaternion);
		class_add_element("x", TypeFloat32, &quaternion::x);
		class_add_element("y", TypeFloat32, &quaternion::y);
		class_add_element("z", TypeFloat32, &quaternion::z);
		class_add_element("w", TypeFloat32, &quaternion::w);
		class_add_func("__mul__", TypeQuaternion, &quaternion::mul, Flags::PURE);
			func_add_param("other", TypeQuaternion);
		class_add_func("__mul__", TypeVector, &_quat_vec_mul, Flags::PURE);
			func_add_param("other", TypeVector);
		class_add_func("__imul__", TypeVoid, &quaternion::imul);
			func_add_param("other", TypeQuaternion);
		class_add_func("bar", TypeQuaternion, &quaternion::bar, Flags::PURE);
		class_add_func("normalize", TypeVoid, &quaternion::normalize);
		class_add_func("angles", TypeVector, &quaternion::get_angles, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &quaternion::str, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nullptr);
			func_add_param("ang", TypeVector);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nullptr);
			func_add_param("axis", TypeVector);
			func_add_param("angle", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, nullptr);
			func_add_param("m", TypeMatrix);
		class_add_func("_rotation_v", TypeQuaternion, &quaternion::rotation_v, Flags::_STATIC__PURE);
			func_add_param("ang", TypeVector);
		class_add_func("_rotation_a", TypeQuaternion, &quaternion::rotation_a, Flags::_STATIC__PURE);
			func_add_param("axis", TypeVector);
			func_add_param("angle", TypeFloat32);
		class_add_func("_rotation_m", TypeQuaternion, &quaternion::rotation_m, Flags::_STATIC__PURE);
			func_add_param("m", TypeMatrix);
		class_add_func("interpolate", TypeQuaternion, (quaternion(*)(const quaternion&, const quaternion&, float))&quaternion::interpolate, Flags::_STATIC__PURE);
			func_add_param("q0", TypeQuaternion);
			func_add_param("q1", TypeQuaternion);
			func_add_param("t", TypeFloat32);
		class_add_func("drag", TypeQuaternion, &quaternion::drag, Flags::_STATIC__PURE);
			func_add_param("up", TypeVector);
			func_add_param("dang", TypeVector);
			func_add_param("reset_z", TypeBool);
		class_add_const("ID", TypeQuaternion, &quaternion::ID);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_EQUAL);
	
	add_class(TypeRect);
		class_add_element("x1", TypeFloat32, 0);
		class_add_element("x2", TypeFloat32, 4);
		class_add_element("y1", TypeFloat32, 8);
		class_add_element("y2", TypeFloat32, 12);
		class_add_const("ID", TypeRect, &rect::ID);
		class_add_func("width", TypeFloat32, &rect::width, Flags::PURE);
		class_add_func("height", TypeFloat32, &rect::height, Flags::PURE);
		class_add_func("area", TypeFloat32, &rect::area, Flags::PURE);
		class_add_func("inside", TypeBool, &rect::inside, Flags::PURE);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &rect::str, Flags::PURE);
		class_add_func("_create", TypeRect, &__rect_set, Flags::_STATIC__PURE);
			func_set_inline(InlineID::RECT_SET);
			func_add_param("x1", TypeFloat32);
			func_add_param("x2", TypeFloat32);
			func_add_param("y1", TypeFloat32);
			func_add_param("y2", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &__rect_set);
			func_add_param("x1", TypeFloat32);
			func_add_param("x2", TypeFloat32);
			func_add_param("y1", TypeFloat32);
			func_add_param("y2", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeRect, TypeRect, InlineID::CHUNK_ASSIGN, &KabaRect::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeRect, TypeRect, InlineID::CHUNK_EQUAL);
	
	add_class(TypeColor);
		class_add_element("r", TypeFloat32, 0);
		class_add_element("g", TypeFloat32, 4);
		class_add_element("b", TypeFloat32, 8);
		class_add_element("a", TypeFloat32, 12);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &color::str, Flags::PURE);
		class_add_func("hsb", TypeColor, &color::hsb, Flags::_STATIC__PURE);
			func_add_param("h", TypeFloat32);
			func_add_param("s", TypeFloat32);
			func_add_param("b", TypeFloat32);
			func_add_param("a", TypeFloat32);
		class_add_func("interpolate", TypeColor, &color::interpolate, Flags::_STATIC__PURE);
			func_add_param("c1", TypeColor);
			func_add_param("c2", TypeColor);
			func_add_param("t", TypeFloat32);
		class_add_func("_create", TypeColor, &__color_set, Flags::_STATIC__PURE);
			func_set_inline(InlineID::COLOR_SET);
			func_add_param("r", TypeFloat32);
			func_add_param("g", TypeFloat32);
			func_add_param("b", TypeFloat32);
			func_add_param("a", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &__color_set);
			func_add_param("r", TypeFloat32);
			func_add_param("g", TypeFloat32);
			func_add_param("b", TypeFloat32);
			func_add_param("a", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeColor, TypeColor, InlineID::CHUNK_ASSIGN, &KabaColor::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeColor, TypeColor, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::ADD, TypeColor, TypeColor, TypeColor, InlineID::NONE, &color::operator+);
		add_operator(OperatorID::ADDS, TypeVoid, TypeColor, TypeColor, InlineID::NONE, &color::operator+=);
		add_operator(OperatorID::SUBTRACT, TypeColor, TypeColor, TypeColor, InlineID::NONE, &color::operator-);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeColor, TypeColor, InlineID::NONE, &color::operator-=);
		add_operator(OperatorID::MULTIPLY, TypeColor, TypeColor, TypeFloat32, InlineID::NONE, &KabaColor::mul_f);
		add_operator(OperatorID::MULTIPLY, TypeColor, TypeColor, TypeColor, InlineID::NONE, &KabaColor::mul_c);
		// color
		class_add_const("WHITE",  TypeColor, &White);
		class_add_const("BLACK",  TypeColor, &Black);
		class_add_const("GRAY",   TypeColor, &Gray);
		class_add_const("RED",    TypeColor, &Red);
		class_add_const("GREEN",  TypeColor, &Green);
		class_add_const("BLUE",   TypeColor, &Blue);
		class_add_const("YELLOW", TypeColor, &Yellow);
		class_add_const("ORANGE", TypeColor, &Orange);
		class_add_const("PURPLE", TypeColor, &Purple);

	add_class(TypeColorList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<color>::__init__);

	add_class(TypePlane);
		class_add_element("a", TypeFloat32, 0);
		class_add_element("b", TypeFloat32, 4);
		class_add_element("c", TypeFloat32, 8);
		class_add_element("d", TypeFloat32, 12);
		class_add_element("n", TypeVector, 0);
		class_add_func("intersect_line", TypeBool, &plane::intersect_line, Flags::PURE);
			func_add_param("l1", TypeVector);
			func_add_param("l2", TypeVector);
			func_add_param("inter", TypeVector);
		class_add_func("inverse", TypePlane, &plane::inverse, Flags::PURE);
		class_add_func("distance", TypeFloat32, &plane::distance, Flags::PURE);
			func_add_param("p", TypeVector);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &plane::str, Flags::PURE);
		class_add_func("transform", TypePlane, &plane::transform, Flags::PURE);
			func_add_param("m", TypeMatrix);
		class_add_func("from_points", TypePlane, &plane::from_points, Flags::_STATIC__PURE);
			func_add_param("a", TypeVector);
			func_add_param("b", TypeVector);
			func_add_param("c", TypeVector);
		class_add_func("from_point_normal", TypePlane, &plane::from_point_normal, Flags::_STATIC__PURE);
			func_add_param("p", TypeVector);
			func_add_param("n", TypeVector);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePlane, TypePlane, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypePlane, TypePlane, InlineID::CHUNK_EQUAL);
	
	add_class(TypePlaneList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<plane>::__init__);
	
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
		class_add_const("ID", TypeMatrix, &matrix::ID);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &matrix::str, Flags::PURE);
		class_add_func("transform", TypeVector, &matrix::transform, Flags::PURE);
			func_add_param("v", TypeVector);
		class_add_func("transform_normal", TypeVector, &matrix::transform_normal, Flags::PURE);
			func_add_param("v", TypeVector);
		class_add_func("untransform", TypeVector, &matrix::untransform, Flags::PURE);
			func_add_param("v", TypeVector);
		class_add_func("project", TypeVector, &matrix::project, Flags::PURE);
			func_add_param("v", TypeVector);
		class_add_func("unproject", TypeVector, &matrix::unproject, Flags::PURE);
			func_add_param("v", TypeVector);
		class_add_func("inverse", TypeMatrix, &matrix::inverse, Flags::PURE);
		class_add_func("transpose", TypeMatrix, &matrix::transpose, Flags::PURE);
		class_add_func("translation", TypeMatrix, &matrix::translation, Flags::_STATIC__PURE);
			func_add_param("trans", TypeVector);
		class_add_func("rotation", TypeMatrix, &matrix::rotation_v, Flags::_STATIC__PURE);
			func_add_param("ang", TypeVector);
		class_add_func("rotation_x", TypeMatrix, &matrix::rotation_x, Flags::_STATIC__PURE);
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation_y", TypeMatrix, &matrix::rotation_y, Flags::_STATIC__PURE);
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation_z", TypeMatrix, &matrix::rotation_z, Flags::_STATIC__PURE);
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation", TypeMatrix, &matrix::rotation_q, Flags::_STATIC__PURE);
			func_add_param("ang", TypeQuaternion);
		class_add_func("scale", TypeMatrix, &matrix::scale, Flags::_STATIC__PURE);
			func_add_param("s_x", TypeFloat32);
			func_add_param("s_y", TypeFloat32);
			func_add_param("s_z", TypeFloat32);
		class_add_func("perspective", TypeMatrix, &matrix::perspective, Flags::_STATIC__PURE);
			func_add_param("fovy", TypeFloat32);
			func_add_param("aspect", TypeFloat32);
			func_add_param("z_near", TypeFloat32);
			func_add_param("z_far", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeMatrix, TypeMatrix, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeMatrix, TypeMatrix, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::MULTIPLY, TypeMatrix, TypeMatrix, TypeMatrix, InlineID::NONE, &matrix::mul);
		add_operator(OperatorID::MULTIPLY, TypeVector, TypeMatrix, TypeVector, InlineID::NONE, &matrix::mul_v);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeMatrix, TypeMatrix, InlineID::NONE, &matrix::imul);
	
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
		class_add_const("ID", TypeMatrix3, &matrix3::ID);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &matrix3::str, Flags::PURE);
		class_add_func("inverse", TypeMatrix3, &matrix3::inverse, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeMatrix3, TypeMatrix3, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeMatrix3, TypeMatrix3, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::MULTIPLY, TypeMatrix3, TypeMatrix3, TypeMatrix3, InlineID::NONE, &matrix3::mul);
		add_operator(OperatorID::MULTIPLY, TypeVector, TypeMatrix3, TypeVector, InlineID::NONE, &matrix3::mul_v);
	
	add_class(TypeVli);
		class_add_element("sign", TypeBool, 0);
		class_add_element("data", TypeIntList, 4);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, algebra_p(&vli::__init__));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, algebra_p(&vli::__delete__));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, algebra_p(&vli::set_vli));
			func_add_param("v", TypeVli);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, algebra_p(&vli::set_str));
			func_add_param("s", TypeString);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, algebra_p(&vli::set_int));
			func_add_param("i", TypeInt);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, algebra_p(&vli::to_string), Flags::PURE);
		class_add_func("compare", TypeInt, algebra_p(&vli::compare), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__eq__", TypeBool, algebra_p(&vli::operator==), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__ne__", TypeBool, algebra_p(&vli::operator!=), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__lt__", TypeBool, algebra_p(&vli::operator<), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__gt__", TypeBool, algebra_p(&vli::operator>), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__le__", TypeBool, algebra_p(&vli::operator<=), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__ge__", TypeBool, algebra_p(&vli::operator>=), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__iadd__", TypeVoid, algebra_p(&vli::operator+=));
			func_add_param("v", TypeVli);
		class_add_func("__isub__", TypeVoid, algebra_p(&vli::operator-=));
			func_add_param("v", TypeVli);
		class_add_func("__imul__", TypeVoid, algebra_p(&vli::operator*=));
			func_add_param("v", TypeVli);
		class_add_func("__add__", TypeVli, algebra_p(&vli::operator+), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__sub__", TypeVli, algebra_p(&vli::operator-), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("__mul__", TypeVli, algebra_p(&vli::operator*), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("idiv", TypeVoid, algebra_p(&vli::idiv));
			func_add_param("div", TypeVli);
			func_add_param("rem", TypeVli);
		class_add_func("div", TypeVli, algebra_p(&vli::_div), Flags::PURE);
			func_add_param("div", TypeVli);
			func_add_param("rem", TypeVli);
		class_add_func("pow", TypeVli, algebra_p(&vli::pow), Flags::PURE);
			func_add_param("exp", TypeVli);
		class_add_func("pow_mod", TypeVli, algebra_p(&vli::pow_mod), Flags::PURE);
			func_add_param("exp", TypeVli);
			func_add_param("mod", TypeVli);
		class_add_func("gcd", TypeVli, algebra_p(&vli::gcd), Flags::PURE);
			func_add_param("v", TypeVli);
	
	add_class(TypeAny);
		class_add_element("type", TypeClassP, &Any::_class);
		class_add_element("data", TypePointer, &Any::data);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Any::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &Any::__delete__);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &Any::set);
			func_add_param("a", TypeAny);
		class_add_func("__iadd__", TypeVoid, &Any::_add);
			func_add_param("a", TypeAny);
		class_add_func("__isub__", TypeVoid, &Any::_sub);
			func_add_param("a", TypeAny);
		class_add_func("clear", TypeVoid, &Any::clear);
		class_add_func(IDENTIFIER_FUNC_LENGTH, TypeInt, &Any::length, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_GET, TypeAny, &KabaAny::_map_get, Flags::_SELFREF__RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &KabaAny::_map_set, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
			func_add_param("value", TypeAny);
		class_add_func(IDENTIFIER_FUNC_GET, TypeAny, &KabaAny::_array_get, Flags::_SELFREF__RAISES_EXCEPTIONS);
			func_add_param("index", TypeInt);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &KabaAny::_array_set, Flags::RAISES_EXCEPTIONS);
			func_add_param("index", TypeInt);
			func_add_param("value", TypeAny);
		class_add_func("is_empty", TypeBool, &Any::is_empty, Flags::PURE);
		class_add_func("has", TypeBool, &Any::has, Flags::PURE);
			func_add_param("key", TypeString);
		class_add_func("add", TypeVoid, &KabaAny::_array_add, Flags::RAISES_EXCEPTIONS);
			func_add_param("a", TypeAny);
		class_add_func("drop", TypeVoid, &Any::map_drop, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func("keys", TypeStringList, &Any::keys, Flags::PURE);//, Flags::RAISES_EXCEPTIONS);
		class_add_func("__bool__", TypeBool, &Any::_bool, Flags::PURE);
		class_add_func("__int__", TypeInt, &Any::_int, Flags::PURE);
		class_add_func("__float__", TypeFloat32, &Any::_float, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &Any::str, Flags::PURE);
		class_add_func(IDENTIFIER_FUNC_REPR, TypeString, &Any::repr, Flags::PURE);
		class_add_func("unwrap", TypeVoid, &KabaAny::unwrap, Flags::RAISES_EXCEPTIONS);
			func_add_param("var", TypePointer);
			func_add_param("type", TypeClassP);
		class_add_func("parse", TypeAny, &KabaAny::parse, Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);

	add_func("@int2any", TypeAny, &kaba_int2any, Flags::STATIC);
		func_add_param("i", TypeInt);
	add_func("@float2any", TypeAny, &kaba_float2any, Flags::STATIC);
		func_add_param("i", TypeFloat32);
	add_func("@bool2any", TypeAny, &kaba_bool2any, Flags::STATIC);
		func_add_param("i", TypeBool);
	add_func("@str2any", TypeAny, &kaba_str2any, Flags::STATIC);
		func_add_param("s", TypeString);
	add_func("@pointer2any", TypeAny, &kaba_pointer2any, Flags::STATIC);
		func_add_param("p", TypePointer);


	add_class(TypeCrypto);
		class_add_element("n", TypeVli, 0);
		class_add_element("k", TypeVli, sizeof(vli));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, algebra_p(&Crypto::__init__));
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, algebra_p(&Crypto::str), Flags::PURE);
		class_add_func("from_str", TypeVoid, algebra_p(&Crypto::from_str));
			func_add_param("str", TypeString);
		class_add_func("encrypt", TypeString, algebra_p(&Crypto::Encrypt), Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("decrypt", TypeString, algebra_p(&Crypto::Decrypt), Flags::PURE);
			func_add_param("str", TypeString);
			func_add_param("cut", TypeBool);
		class_add_func("create_keys", TypeVoid, algebra_p(&CryptoCreateKeys), Flags::STATIC);
			func_add_param("c1", TypeCrypto);
			func_add_param("c2", TypeCrypto);
			func_add_param("type", TypeString);
			func_add_param("bits", TypeInt);

	add_class(TypeRandom);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Random::__init__);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &Random::__assign__);
			func_add_param("o", TypeRandom);
		//class_add_element("n", TypeRandom, 0);
		class_add_func("seed", TypeVoid, &Random::seed);
			func_add_param("str", TypeString);
		class_add_func("int", TypeInt, &Random::_int);
			func_add_param("max", TypeInt);
		class_add_func("uniform01", TypeFloat32, &Random::uniform01);
		class_add_func("uniform", TypeFloat32, &Random::uniform);
			func_add_param("min", TypeFloat32);
			func_add_param("max", TypeFloat32);
		class_add_func("normal", TypeFloat32, &Random::normal);
			func_add_param("mean", TypeFloat32);
			func_add_param("stddev", TypeFloat32);
		class_add_func("in_ball", TypeVector, &Random::in_ball);
			func_add_param("r", TypeFloat32);
		class_add_func("dir", TypeVector, &Random::dir);
	
	
	add_class(TypeFloatInterpolator);
		class_add_element("type", TypeInt, 0);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Interpolator<float>::__init__);
		class_add_func("clear", TypeVoid, &Interpolator<float>::clear);
		class_add_func("set_type", TypeVoid, &Interpolator<float>::setType);
			func_add_param("type", TypeString);
		class_add_func("add", TypeVoid, &Interpolator<float>::addv);
			func_add_param("p", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("add2", TypeVoid, &Interpolator<float>::add2v);
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("add3", TypeVoid, &Interpolator<float>::add3v);
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
			func_add_param("w", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("jump", TypeVoid, &Interpolator<float>::jumpv);
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
		class_add_func("normalize", TypeVoid, &Interpolator<float>::normalize);
		class_add_func("get", TypeFloat32, &Interpolator<float>::get, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_tang", TypeFloat32, &Interpolator<float>::getTang, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_list", TypeFloatList, &Interpolator<float>::getList, Flags::PURE);
			func_add_param("t", TypeFloatList);

	
	add_class(TypeVectorInterpolator);
		class_add_element("type", TypeInt, 0);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Interpolator<vector>::__init__);
		class_add_func("clear", TypeVoid, &Interpolator<vector>::clear);
		class_add_func("set_type", TypeVoid, &Interpolator<vector>::setType);
			func_add_param("type", TypeString);
		class_add_func("add", TypeVoid, &Interpolator<vector>::add);
			func_add_param("p", TypeVector);
			func_add_param("dt", TypeFloat32);
		class_add_func("add2", TypeVoid, &Interpolator<vector>::add2);
			func_add_param("p", TypeVector);
			func_add_param("v", TypeVector);
			func_add_param("dt", TypeFloat32);
		class_add_func("add3", TypeVoid, &Interpolator<vector>::add3);
			func_add_param("p", TypeVector);
			func_add_param("v", TypeVector);
			func_add_param("w", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("jump", TypeVoid, &Interpolator<vector>::jump);
			func_add_param("p", TypeVector);
			func_add_param("v", TypeVector);
		class_add_func("normalize", TypeVoid, &Interpolator<vector>::normalize);
		class_add_func("get", TypeVector, &Interpolator<vector>::get, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_tang", TypeVector, &Interpolator<vector>::getTang, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_list", TypeVectorList, &Interpolator<vector>::getList, Flags::PURE);
			func_add_param("t", TypeFloatList);

	// int
	add_func("clamp", TypeInt, &clamp<int>, Flags::_STATIC__PURE);
		func_add_param("i", TypeInt);
		func_add_param("min", TypeInt);
		func_add_param("max", TypeInt);
	add_func("loop", TypeInt, &loop<int>, Flags::_STATIC__PURE);
		func_add_param("i", TypeInt);
		func_add_param("min", TypeInt);
		func_add_param("max", TypeInt);
	add_func("abs", TypeInt, &x_abs<int>, Flags::_STATIC__PURE);
		func_add_param("i", TypeInt);
	add_func("sign", TypeInt, &sign<int>, Flags::_STATIC__PURE);
		func_add_param("i", TypeInt);
	add_func("min", TypeInt, &x_min<int>, Flags::_STATIC__PURE);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);
	add_func("max", TypeInt, &x_max<int>, Flags::_STATIC__PURE);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);
	// mathematical
	add_func("sin", TypeFloat32, &sinf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("cos", TypeFloat32, &cosf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("tan", TypeFloat32, &tanf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("asin", TypeFloat32, &asinf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("acos", TypeFloat32, &acosf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("atan", TypeFloat32, &atanf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("atan2", TypeFloat32, &atan2f, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
		func_add_param("y", TypeFloat32);
	add_func("sqrt", TypeFloat32, &sqrtf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("sqr", TypeFloat32, &f_sqr, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("exp", TypeFloat32, &expf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("log", TypeFloat32, &logf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
	add_func("pow", TypeFloat32, &powf, Flags::_STATIC__PURE);
		func_add_param("x", TypeFloat32);
		func_add_param("exp", TypeFloat32);
	add_func("clamp", TypeFloat32, &clamp<float>, Flags::_STATIC__PURE);
		func_add_param("f", TypeFloat32);
		func_add_param("min", TypeFloat32);
		func_add_param("max", TypeFloat32);
	add_func("loop", TypeFloat32, &loop<float>, Flags::_STATIC__PURE);
		func_add_param("f", TypeFloat32);
		func_add_param("min", TypeFloat32);
		func_add_param("max", TypeFloat32);
	add_func("abs", TypeFloat32, &abs<float>, Flags::_STATIC__PURE);
		func_add_param("f", TypeFloat32);
	add_func("sign", TypeFloat32, &sign<float>, Flags::_STATIC__PURE);
		func_add_param("f", TypeFloat32);
	add_func("min", TypeFloat32, &x_min<float>, Flags::_STATIC__PURE);
		func_add_param("a", TypeFloat32);
		func_add_param("b", TypeFloat32);
	add_func("max", TypeFloat32, &x_max<float>, Flags::_STATIC__PURE);
		func_add_param("a", TypeFloat32);
		func_add_param("b", TypeFloat32);
	// lists
	add_func("range", TypeIntList, (void*)&int_range, Flags::_STATIC__PURE);
		func_add_param("start", TypeInt);
		func_add_param("end", TypeInt);
	add_func("rangef", TypeFloatList, (void*)&float_range, Flags::_STATIC__PURE);
		func_add_param("start", TypeFloat32);
		func_add_param("end", TypeFloat32);
		func_add_param("step", TypeFloat32);
	// other types
	add_func("bary_centric", TypeVoid, (void*)&GetBaryCentric, Flags::_STATIC__PURE);
		func_add_param("p", TypeVector);
		func_add_param("a", TypeVector);
		func_add_param("b", TypeVector);
		func_add_param("c", TypeVector);
		func_add_param("f", TypeFloatPs);
		func_add_param("g", TypeFloatPs);
	// random numbers
	add_func("randi", TypeInt, &randi, Flags::STATIC);
		func_add_param("max", TypeInt);
	add_func("rand", TypeFloat32, &randf, Flags::STATIC);
		func_add_param("max", TypeFloat32);
	add_func("rand_seed", TypeVoid, &srand, Flags::STATIC);
		func_add_param("seed", TypeInt);

	add_ext_var("_any_allow_simple_output", TypeBool, (void*)&Any::allow_simple_output);
	
	// float
	add_const("pi",  TypeFloat32, &pi);


	// needs to be defined after any
	TypeAnyList = add_type_l(TypeAny);
	add_class(TypeAnyList);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &AnyList::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &AnyList::__delete__);
		class_add_func("add", TypeVoid, &AnyList::add);
			func_add_param("a", TypeAny);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &AnyList::assign);
			func_add_param("other", TypeAnyList);

	TypeAnyDict = add_type_d(TypeAny);
	add_class(TypeAnyDict);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &AnyDict::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &AnyDict::__delete__);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &AnyDict::set);
			func_add_param("key", TypeString);
			func_add_param("x", TypeAny);
		class_add_func(IDENTIFIER_FUNC_GET, TypeAny, &AnyDict::get_item, Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func("clear", TypeVoid, &AnyDict::clear);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &AnyDict::assign);
			func_add_param("other", TypeAny);
		class_add_func(IDENTIFIER_FUNC_STR, TypeString, &AnyDict::str);


	add_class(TypeAny);
		class_add_func("as_array", TypeAnyList, &KabaAny::_as_array, Flags::_SELFREF__RAISES_EXCEPTIONS);
		class_add_func("as_map", TypeAnyDict, &KabaAny::_as_map, Flags::_SELFREF__RAISES_EXCEPTIONS);
}

};
