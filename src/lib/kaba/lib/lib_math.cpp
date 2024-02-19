#include "../../os/msg.h"
#include "../../math/math.h"
#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "../../math/complex.h"
#include "../../math/quaternion.h"
#include "../../math/mat4.h"
#include "../../math/mat3.h"
#include "../../math/plane.h"
#include "../../math/ray.h"
#include "../../math/rect.h"
#include "../../math/interpolation.h"
#include "../../math/random.h"
#include "../../image/color.h"
#include "../../base/map.h"
#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "list.h"
#include "dict.h"
#include "optional.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"

#if __has_include("../../algebra/algebra.h")
	#include "../../algebra/algebra.h"
	#define HAS_ALGEBRA
#else
	typedef int vli;
	typedef int Crypto;
#endif

#if __has_include("../../any/any.h")
	#include "../../any/any.h"
	#define HAS_ANY
#else
	typedef int Any;
	#error("no any.h ... we're screwed")
#endif

#if __has_include("../../fft/fft.h")
	#include "../../fft/fft.h"
	#define HAS_FFT
#else
#endif

namespace kaba {

#ifdef HAS_ALGEBRA
	#define algebra_p(p)		p
#else
	#define algebra_p(p)		nullptr
#endif

#ifdef HAS_ANY
	#define any_p(p)		p
#else
	#define any_p(p)		nullptr
#endif

#ifdef HAS_FFT
	#define fft_p(p)		p
#else
	#define fft_p(p)		nullptr
#endif

// we're always using math types
#define type_p(p)			p

extern const Class *TypeStringList;
extern const Class *TypeComplexList;
extern const Class *TypeFloatList;
extern const Class *TypeFloat64List;
extern const Class *TypeVec3List;
extern const Class *TypeVec2;
extern const Class *TypeVec2List;
extern const Class *TypeMat4;
extern const Class *TypePlane;
extern const Class *TypePlaneList;
extern const Class *TypeColorList;
extern const Class *TypeMat3;
extern const Class *TypeIntList;
extern const Class *TypeBoolList;
extern const Class *TypeAny;
extern const Class *TypeAnyList;
extern const Class *TypeAnyDict;


float _cdecl f_sqr(float f) {
	return f*f;
}

template<class T>
class VectorList : public Array<T> {
public:
	static T _cdecl sum(const Array<T> &list) {
		T r = T::ZERO;
		for (auto &v: list)
			r += v;
		return r;
	}
	static float sum_sqr(const Array<T> &list) {
		float r = 0;
		for (auto &v: list) {
			if constexpr (std::is_same<T, complex>::value)
				r += v.abs_sqr();
			else
				r += v.length_sqr();
		}
		return r;
	}
	
	// a += b
	void _cdecl iadd_values(VectorList<T> &b)	IMPLEMENT_IOP_LIST(+=, T)
	void _cdecl isub_values(VectorList<T> &b)	IMPLEMENT_IOP_LIST(-=, T)
	void _cdecl imul_values(VectorList<T> &b)	IMPLEMENT_IOP_LIST(*=, T)
	void _cdecl idiv_values(VectorList<T> &b)	IMPLEMENT_IOP_LIST(/=, T)

	// a = b + c
	Array<T> _cdecl add_values(VectorList<T> &b)	IMPLEMENT_OP_LIST(+, T, T)
	Array<T> _cdecl sub_values(VectorList<T> &b)	IMPLEMENT_OP_LIST(-, T, T)
	Array<T> _cdecl mul_values(VectorList<T> &b)	IMPLEMENT_OP_LIST(*, T, T)
	Array<T> _cdecl div_values(VectorList<T> &b)	IMPLEMENT_OP_LIST(/, T, T)

	// a += x
	void _cdecl iadd_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(+=, T)
	void _cdecl isub_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(-=, T)
	void _cdecl imul_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(*=, T)
	void _cdecl idiv_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(/=, T)
	void _cdecl imul_values_scalar_f(float x)	IMPLEMENT_IOP_LIST_SCALAR(*=, T)
	void _cdecl idiv_values_scalar_f(float x)	IMPLEMENT_IOP_LIST_SCALAR(/=, T)
	void _cdecl assign_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(=, T)
};

template<class T>
Array<T> kaba_range(T start, T end, T step) {
	if (end == DynamicArray::MAGIC_END_INDEX) {
		end = start;
		start = 0;
	}
	Array<T> a;
	for (T v=start; v<end; v+=step)
		a.add(v);
	return a;
}


class KabaQuaternion : public quaternion {
public:
	vec3 mulv(const vec3& v) {
		return *this * v;
	}
};



template<class T>
class KabaVector : public T {
public:
	void assign(const T &o) {
		*(T*)this = o;
	}
	T negate() const {
		return -(*(T*)this);
	}
	float mul_vv(const T &v) const {
		return T::dot(*(T*)this, v);
	}
	T mul_vf(float f) const {
		return *(T*)this * f;
	}
	static T mul_fv(float f, const T &v) {
		return f * v;
	}
	T div_f(float f) const {
		return *(T*)this / f;
	}
	void init3(float x, float y, float z) {
		*(T*)this = T(x,y,z);
	}
	void init2(float x, float y) {
		*(T*)this = T(x,y);
	}
	static T set3(float x, float y, float z) {
		return T(x, y, z);
	}
	static T set2(float x, float y) {
		return T(x, y);
	}
	static float abs(const T &v) {
		return v.abs();
	}
};

template<class T>
class KabaMatrix : public T {
public:
	void _cdecl imul(const T &m) {
		*(T*)this *= m;
	}
	T _cdecl mul(const T &m) {
		return *(T*)this * m;
	}
	template<class V>
	V _cdecl mul_v(const V &v) {
		return *(T*)this * v;
	}
	static T _cdecl rotation_v(const vec3& v) {
		return T::rotation(v);
	}
	static T _cdecl rotation_q(const quaternion& q) {
		return T::rotation(q);
	}
	static T _cdecl scale_f(float x, float y, float z) {
		return T::scale(x, y, z);
	}
	static T _cdecl scale_v(const vec3& v) {
		return T::scale(v);
	}
};

class KabaRect : public rect {
public:
	void assign(const rect& o) {
		*(rect*)this = o;
	}
	void init(float x1, float x2, float y1, float y2) {
		*(rect*)this = rect(x1, x2, y1, y2);
	}
	static rect set(float x1, float x2, float y1, float y2) {
		return rect(x1, x2, y1, y2);
	}
};


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")



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
		} else if (type->is_pointer_raw()) {
			*(const void**)var = aa.as_pointer();
		} else if (type->is_list() and (aa.type == TYPE_ARRAY)) {
			auto *t_el = type->get_array_element();
			auto *a = (DynamicArray*)var;
			auto &b = aa.as_array();
			int n = b.num;
			array_resize(var, type, n);
			for (int i=0; i<n; i++)
				unwrap(aa[i], (char*)a->data + i * t_el->size, t_el);
		} else if (type->is_array() and (aa.type == TYPE_ARRAY)) {
			auto *t_el = type->get_array_element();
			auto &b = aa.as_array();
			int n = min(type->array_length, b.num);
			for (int i=0; i<n; i++)
				unwrap(b[i], (char*)var + i*t_el->size, t_el);
		} else if (aa.type == TYPE_MAP) {
			[[maybe_unused]] auto &map = aa.as_map();
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

Any int2any(int i) {
	return Any(i);
}
Any float2any(float f) {
	return Any(f);
}
Any bool2any(bool b) {
	return Any(b);
}
Any str2any(const string &str) {
	return Any(str);
}
Any pointer2any(const void *p) {
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
	static color set(float r, float g, float b, float a) {
		return color(a, r, g, b);
	}
};

class KabaRay : public Ray {
public:
	void init() {
		new(this) Ray();
	}
	void init_ex(const vec3& u, const vec3& v) {
		new(this) Ray(u, v);
	}
};

void SIAddPackageMath(Context *c) {
	add_package(c, "math", Flags::AUTO_IMPORT);

	// types
	TypeComplex = add_type("complex", sizeof(complex));
	TypeComplexList = add_type_list(TypeComplex);
	TypeVec2 = add_type("vec2", sizeof(vec2));
	TypeVec2List = add_type_list(TypeVec2);
	TypeVec3 = add_type("vec3", sizeof(vec3));
	TypeVec3List = add_type_list(TypeVec3);
	TypeRect = add_type("rect", sizeof(rect));
	TypeMat4 = add_type("mat4", sizeof(mat4));
	TypeQuaternion = add_type("quaternion", sizeof(quaternion));
	TypePlane = add_type("plane", sizeof(plane));
	TypePlaneList = add_type_list(TypePlane);
	TypeColor = add_type("color", sizeof(color));
	TypeColorList = add_type_list(TypeColor);
	auto TypeRay = add_type("ray", sizeof(Ray));
	TypeMat3 = add_type("mat3", sizeof(mat3));
	auto TypeFloatArray3 = add_type_array(TypeFloat32, 3);
	auto TypeFloatArray4 = add_type_array(TypeFloat32, 4);
	auto TypeFloatArray4x4 = add_type_array(TypeFloatArray4, 4);
	auto TypeFloatArray16 = add_type_array(TypeFloat32, 16);
	auto TypeFloatArray3x3 = add_type_array(TypeFloatArray3, 3);
	auto TypeFloatArray9 = add_type_array(TypeFloat32, 9);
	auto TypeVli = add_type("vli", sizeof(vli));
	auto TypeCrypto = add_type("Crypto", sizeof(Crypto));
	TypeAny = add_type("any", sizeof(Any));
	auto TypeFloatInterpolator = add_type("FloatInterpolator", sizeof(Interpolator<float>));
	auto TypeVectorInterpolator = add_type("VectorInterpolator", sizeof(Interpolator<vec3>));
	auto TypeRandom = add_type("Random", sizeof(Random));
	auto TypeFFT = add_type("fft", 0);
	const_cast<Class*>(TypeFFT)->type = Class::Type::NAMESPACE;

	auto TypeVec3Optional = add_type_optional(TypeVec3);
	
	// dirty hack :P
	/*if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64)*/ {
		flags_set(((Class*)TypeFloat32)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
		flags_set(((Class*)TypeFloat64)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
		if (config.target.abi == Abi::AMD64_GNU) {
			// not on windows!
			flags_set(((Class*)TypeComplex)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeVec2)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeQuaternion)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeVec3)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeColor)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypePlane)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
			flags_set(((Class*)TypeRect)->flags, Flags::AMD64_ALLOW_PASS_IN_XMM);
		}
	}

	lib_create_list<complex>(TypeComplexList);
	lib_create_list<vec2>(TypeVec2List);
	lib_create_list<vec3>(TypeVec3List);
	lib_create_list<plane>(TypePlaneList);
	lib_create_list<color>(TypeColorList);

	lib_create_optional<vec3>(TypeVec3Optional);

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
		class_add_func(Identifier::Func::STR, TypeString, &complex::str, Flags::PURE);
		class_add_const("I", TypeComplex, &complex::I);
		class_add_func("_create", TypeComplex, &KabaVector<complex>::set2, Flags::STATIC | Flags::PURE);
			func_set_inline(InlineID::COMPLEX_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaVector<complex>::init2, Flags::MUTABLE);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);	
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeComplex, TypeComplex, InlineID::CHUNK_ASSIGN, &KabaVector<complex>::assign);
		add_operator(OperatorID::ADD, TypeComplex, TypeComplex, TypeComplex, InlineID::VEC2_ADD, &complex::operator+);
		add_operator(OperatorID::SUBTRACT, TypeComplex, TypeComplex, TypeComplex, InlineID::VEC2_SUBTRACT, (decltype(&complex::operator+)) &complex::operator-);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY, (decltype(&complex::operator+)) &complex::operator*);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeFloat32, TypeComplex, InlineID::VEC2_MULTIPLY_SV);
		add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeFloat32, InlineID::VEC2_MULTIPLY_VS);
		add_operator(OperatorID::DIVIDE, TypeComplex, TypeComplex, TypeComplex, InlineID::NONE /*InlineID::COMPLEX_DIVIDE*/, (decltype(&complex::operator+)) &complex::operator/);
		add_operator(OperatorID::ADDS, TypeVoid, TypeComplex, TypeComplex, InlineID::VEC2_ADD_ASSIGN, &complex::operator+=);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeComplex, TypeComplex, InlineID::VEC2_SUBTARCT_ASSIGN, &complex::operator-=);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY_ASSIGN, (decltype(&complex::operator+=)) &complex::operator*=);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_DIVIDE_ASSIGN, (decltype(&complex::operator+=)) &complex::operator/=);
		add_operator(OperatorID::EQUAL, TypeBool, TypeComplex, TypeComplex, InlineID::CHUNK_EQUAL, &complex::operator==);
		add_operator(OperatorID::NEGATIVE, TypeComplex, nullptr, TypeComplex, InlineID::VEC2_NEGATIVE, &KabaVector<complex>::negate);

	add_class(TypeComplexList);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XList<complex>::__init__, Flags::MUTABLE);
		add_operator(OperatorID::ADD, TypeComplexList, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::add_values);
		add_operator(OperatorID::SUBTRACT, TypeComplexList, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::sub_values);
		add_operator(OperatorID::MULTIPLY, TypeComplexList, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::mul_values);
		add_operator(OperatorID::DIVIDE, TypeComplexList, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::div_values);
		add_operator(OperatorID::ADDS, TypeVoid, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::iadd_values);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::isub_values);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::imul_values);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplexList, TypeComplexList, InlineID::NONE, &VectorList<complex>::idiv_values);
		add_operator(OperatorID::ADDS, TypeVoid, TypeComplexList, TypeComplex, InlineID::NONE, &VectorList<complex>::iadd_values_scalar);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeComplexList, TypeComplex, InlineID::NONE, &VectorList<complex>::isub_values_scalar);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplexList, TypeComplex, InlineID::NONE, &VectorList<complex>::imul_values_scalar);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplexList, TypeComplex, InlineID::NONE, &VectorList<complex>::idiv_values_scalar);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplexList, TypeFloat32, InlineID::NONE, &VectorList<complex>::imul_values_scalar_f);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplexList, TypeFloat32, InlineID::NONE, &VectorList<complex>::idiv_values_scalar_f);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeComplexList, TypeComplex, InlineID::NONE, &VectorList<complex>::assign_values_scalar);


	add_class(TypeVec2);
		class_add_element("x", TypeFloat32, &vec2::x);
		class_add_element("y", TypeFloat32, &vec2::y);
		//class_add_element("_e", TypeFloatArray2, 0);
		class_add_func(Identifier::Func::LENGTH, TypeFloat32, type_p(&vec2::length), Flags::PURE);
		class_add_func("length", TypeFloat32, type_p(&vec2::length), Flags::PURE);
		//class_add_func("length_sqr", TypeFloat32, type_p(&vec2::length_sqr), Flags::PURE);
		//class_add_func("length_fuzzy", TypeFloat32, type_p(&vec2::length_fuzzy), Flags::PURE);
		class_add_func("normalized", TypeVec2, &vec2::normalized, Flags::PURE);
		class_add_func(Identifier::Func::STR, TypeString, &vec2::str, Flags::PURE);
		class_add_func("dot", TypeFloat32, &vec2::dot, Flags::STATIC | Flags::PURE);
			func_add_param("v1", TypeVec2);
			func_add_param("v2", TypeVec2);
		class_add_func("_create", TypeVec2, &KabaVector<vec2>::set2, Flags::STATIC | Flags::PURE);
			func_set_inline(InlineID::COMPLEX_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		// ignored, but useful for docu
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaVector<vec2>::init2, Flags::MUTABLE);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
		class_add_const("0", TypeVec2, &vec2::ZERO);
		class_add_const("O", TypeVec2, &vec2::ZERO);
		class_add_const("EX", TypeVec2, &vec2::EX);
		class_add_const("EY", TypeVec2, &vec2::EY);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeVec2, TypeVec2, InlineID::CHUNK_ASSIGN, &KabaVector<vec2>::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeVec2, TypeVec2, InlineID::CHUNK_EQUAL, &vec2::operator==);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeVec2, TypeVec2, InlineID::CHUNK_NOT_EQUAL, &vec2::operator!=);
		add_operator(OperatorID::ADD, TypeVec2, TypeVec2, TypeVec2, InlineID::VEC2_ADD, &vec2::operator+);
		add_operator(OperatorID::SUBTRACT, TypeVec2, TypeVec2, TypeVec2, InlineID::VEC2_SUBTRACT, (decltype(&vec2::operator+)) &vec2::operator-);
//		add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeVec2, TypeVec2, InlineID::COMPLEX_MULTIPLY_VV, &KabaVector<vec2>::mul_vv);
		add_operator(OperatorID::MULTIPLY, TypeVec2, TypeVec2, TypeFloat32, InlineID::VEC2_MULTIPLY_VS, &KabaVector<vec2>::mul_vf);
		add_operator(OperatorID::MULTIPLY, TypeVec2, TypeFloat32, TypeVec2, InlineID::VEC2_MULTIPLY_SV, &KabaVector<vec2>::mul_fv);
		add_operator(OperatorID::DIVIDE, TypeVec2, TypeVec2, TypeFloat32, InlineID::VEC2_DIVIDE_VS, &KabaVector<vec2>::div_f);
		add_operator(OperatorID::ADDS, TypeVoid, TypeVec2, TypeVec2, InlineID::VEC2_ADD_ASSIGN, &vec2::operator+=);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeVec2, TypeVec2, InlineID::VEC2_SUBTARCT_ASSIGN, &vec2::operator-=);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeVec2, TypeFloat32, InlineID::VEC2_MULTIPLY_ASSIGN, &vec2::operator*=);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeVec2, TypeFloat32, InlineID::VEC2_DIVIDE_ASSIGN, &vec2::operator/=);
		add_operator(OperatorID::NEGATIVE, TypeVec2, nullptr, TypeVec2, InlineID::VEC2_NEGATIVE, &KabaVector<vec2>::negate);

	add_class(TypeVec2List);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XList<vec2>::__init__, Flags::MUTABLE);

	
	add_class(TypeVec3);
		class_add_element("x", TypeFloat32, &vec3::x);
		class_add_element("y", TypeFloat32, &vec3::y);
		class_add_element("z", TypeFloat32, &vec3::z);
		class_add_element("_e", TypeFloatArray3, &vec3::x);
		class_add_element("_xy", TypeVec2, &vec3::x);
		class_add_func(Identifier::Func::LENGTH, TypeFloat32, type_p(&vec3::length), Flags::PURE);
		class_add_func("length", TypeFloat32, type_p(&vec3::length), Flags::PURE);
		class_add_func("length_sqr", TypeFloat32, type_p(&vec3::length_sqr), Flags::PURE);
		class_add_func("length_fuzzy", TypeFloat32, type_p(&vec3::length_fuzzy), Flags::PURE);
		class_add_func("normalized", TypeVec3, &vec3::normalized, Flags::PURE);
		class_add_func("dir2ang", TypeVec3, &vec3::dir2ang, Flags::PURE);
		class_add_func("dir2ang2", TypeVec3, &vec3::dir2ang2, Flags::PURE);
			func_add_param("up", TypeVec3);
		class_add_func("ang2dir", TypeVec3, &vec3::ang2dir, Flags::PURE);
//		class_add_func("rotate", TypeVector, &vector::rotate, Flags::PURE);
//			func_add_param("ang", TypeVector);
//		class_add_func("__div__", TypeVector, vector::untransform), Flags::PURE);
//			func_add_param("m", TypeMatrix);
		class_add_func("ortho", TypeVec3, &vec3::ortho, Flags::PURE);
		class_add_func(Identifier::Func::STR, TypeString, &vec3::str, Flags::PURE);
		class_add_func("dot", TypeFloat32, &vec3::dot, Flags::STATIC | Flags::PURE);
			func_add_param("v1", TypeVec3);
			func_add_param("v2", TypeVec3);
		class_add_func("cross", TypeVec3, &vec3::cross, Flags::STATIC | Flags::PURE);
			func_add_param("v1", TypeVec3);
			func_add_param("v2", TypeVec3);
		class_add_func("_create", TypeVec3, &KabaVector<vec3>::set3, Flags::STATIC | Flags::PURE);
			func_set_inline(InlineID::VECTOR_SET);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
			func_add_param("z", TypeFloat32);
		// ignored, but useful for docu
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaVector<vec3>::init3, Flags::MUTABLE);
			func_add_param("x", TypeFloat32);
			func_add_param("y", TypeFloat32);
			func_add_param("z", TypeFloat32);
/*		class_add_func("ang_add", TypeVector, &VecAngAdd, Flags::STATIC | Flags::PURE);
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
		class_add_func("ang_interpolate", TypeVector, &VecAngInterpolate, Flags::STATIC | Flags::PURE);
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
			func_add_param("t", TypeFloat32);*/
		class_add_const("0", TypeVec3, &vec3::ZERO);
		class_add_const("O", TypeVec3, &vec3::ZERO);
		class_add_const("EX", TypeVec3, &vec3::EX);
		class_add_const("EY", TypeVec3, &vec3::EY);
		class_add_const("EZ", TypeVec3, &vec3::EZ);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeVec3, TypeVec3, InlineID::CHUNK_ASSIGN, &KabaVector<vec3>::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeVec3, TypeVec3, InlineID::CHUNK_EQUAL, &vec3::operator==);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeVec3, TypeVec3, InlineID::CHUNK_NOT_EQUAL, &vec3::operator!=);
		add_operator(OperatorID::ADD, TypeVec3, TypeVec3, TypeVec3, InlineID::VEC3_ADD, &vec3::operator+);
		add_operator(OperatorID::SUBTRACT, TypeVec3, TypeVec3, TypeVec3, InlineID::VEC3_SUBTRACT, (decltype(&vec3::operator+)) &vec3::operator-);
		add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeVec3, TypeVec3, InlineID::VEC3_MULTIPLY_VV, &KabaVector<vec3>::mul_vv);
		add_operator(OperatorID::MULTIPLY, TypeVec3, TypeVec3, TypeFloat32, InlineID::VEC3_MULTIPLY_VF, &KabaVector<vec3>::mul_vf);
		add_operator(OperatorID::MULTIPLY, TypeVec3, TypeFloat32, TypeVec3, InlineID::VEC3_MULTIPLY_FV, &KabaVector<vec3>::mul_fv);
		add_operator(OperatorID::DIVIDE, TypeVec3, TypeVec3, TypeFloat32, InlineID::VEC3_DIVIDE_VF, &KabaVector<vec3>::div_f);
		add_operator(OperatorID::ADDS, TypeVoid, TypeVec3, TypeVec3, InlineID::VEC3_ADD_ASSIGN, &vec3::operator+=);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeVec3, TypeVec3, InlineID::VEC3_SUBTARCT_ASSIGN, &vec3::operator-=);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeVec3, TypeFloat32, InlineID::VEC3_MULTIPLY_ASSIGN, &vec3::operator*=);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeVec3, TypeFloat32, InlineID::VEC3_DIVIDE_ASSIGN, &vec3::operator/=);
		add_operator(OperatorID::NEGATIVE, TypeVec3, nullptr, TypeVec3, InlineID::VEC3_NEGATIVE, &KabaVector<vec3>::negate);
	
	add_class(TypeVec3List);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XList<vec3>::__init__, Flags::MUTABLE);

	
	add_class(TypeQuaternion);
		class_add_element("x", TypeFloat32, &quaternion::x);
		class_add_element("y", TypeFloat32, &quaternion::y);
		class_add_element("z", TypeFloat32, &quaternion::z);
		class_add_element("w", TypeFloat32, &quaternion::w);
		class_add_func("bar", TypeQuaternion, &quaternion::bar, Flags::PURE);
		class_add_func("normalize", TypeVoid, &quaternion::normalize, Flags::MUTABLE);
		class_add_func("angles", TypeVec3, &quaternion::get_angles, Flags::PURE);
		class_add_func(Identifier::Func::STR, TypeString, &quaternion::str, Flags::PURE);
		class_add_func(Identifier::Func::INIT, TypeVoid, nullptr, Flags::MUTABLE);
			func_add_param("ang", TypeVec3);
		class_add_func(Identifier::Func::INIT, TypeVoid, nullptr, Flags::MUTABLE);
			func_add_param("axis", TypeVec3);
			func_add_param("angle", TypeFloat32);
		class_add_func(Identifier::Func::INIT, TypeVoid, nullptr, Flags::MUTABLE);
			func_add_param("m", TypeMat4);
		class_add_func("_rotation_v", TypeQuaternion, &quaternion::rotation_v, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeVec3);
		class_add_func("_rotation_a", TypeQuaternion, &quaternion::rotation_a, Flags::STATIC | Flags::PURE);
			func_add_param("axis", TypeVec3);
			func_add_param("angle", TypeFloat32);
		class_add_func("_rotation_m", TypeQuaternion, &quaternion::rotation_m, Flags::STATIC | Flags::PURE);
			func_add_param("m", TypeMat4);
		class_add_func("interpolate", TypeQuaternion, (quaternion(*)(const quaternion&, const quaternion&, float))&quaternion::interpolate, Flags::STATIC | Flags::PURE);
			func_add_param("q0", TypeQuaternion);
			func_add_param("q1", TypeQuaternion);
			func_add_param("t", TypeFloat32);
		class_add_func("drag", TypeQuaternion, &quaternion::drag, Flags::STATIC | Flags::PURE);
			func_add_param("up", TypeVec3);
			func_add_param("dang", TypeVec3);
			func_add_param("reset_z", TypeBool);
		class_add_const("ID", TypeQuaternion, &quaternion::ID);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeQuaternion, TypeQuaternion, InlineID::CHUNK_NOT_EQUAL);
		add_operator(OperatorID::MULTIPLY, TypeQuaternion, TypeQuaternion, TypeQuaternion, InlineID::NONE, &quaternion::mul);
		add_operator(OperatorID::MULTIPLY, TypeVec3, TypeQuaternion, TypeVec3, InlineID::NONE, &KabaQuaternion::mulv);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeQuaternion, TypeQuaternion, InlineID::NONE, &quaternion::imul);
	
	add_class(TypeRect);
		class_add_element("x1", TypeFloat32, &rect::x1);
		class_add_element("x2", TypeFloat32, &rect::x2);
		class_add_element("y1", TypeFloat32, &rect::y1);
		class_add_element("y2", TypeFloat32, &rect::y2);
		class_add_const("ID", TypeRect, &rect::ID);
		class_add_const("ID_SYM", TypeRect, &rect::ID_SYM);
		class_add_const("EMPTY", TypeRect, &rect::EMPTY);
		class_add_func("width", TypeFloat32, &rect::width, Flags::PURE);
		class_add_func("height", TypeFloat32, &rect::height, Flags::PURE);
		class_add_func("area", TypeFloat32, &rect::area, Flags::PURE);
		class_add_func("center", TypeVec2, &rect::center, Flags::PURE);
		class_add_func("size", TypeVec2, &rect::size, Flags::PURE);
		class_add_func("inside", TypeBool, &rect::inside, Flags::PURE);
			func_add_param("p", TypeVec2);
		class_add_func(Identifier::Func::STR, TypeString, &rect::str, Flags::PURE);
		class_add_func("_create", TypeRect, &KabaRect::set, Flags::STATIC | Flags::PURE);
			func_set_inline(InlineID::RECT_SET);
			func_add_param("x1", TypeFloat32);
			func_add_param("x2", TypeFloat32);
			func_add_param("y1", TypeFloat32);
			func_add_param("y2", TypeFloat32);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaRect::init, Flags::MUTABLE);
			func_add_param("x1", TypeFloat32);
			func_add_param("x2", TypeFloat32);
			func_add_param("y1", TypeFloat32);
			func_add_param("y2", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeRect, TypeRect, InlineID::CHUNK_ASSIGN, &KabaRect::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeRect, TypeRect, InlineID::CHUNK_EQUAL, &rect::operator==);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeRect, TypeRect, InlineID::CHUNK_NOT_EQUAL, &rect::operator!=);
	
	add_class(TypeColor);
		class_add_element("r", TypeFloat32, &color::r);
		class_add_element("g", TypeFloat32, &color::g);
		class_add_element("b", TypeFloat32, &color::b);
		class_add_element("a", TypeFloat32, &color::a);
		class_add_func(Identifier::Func::STR, TypeString, &color::str, Flags::PURE);
		class_add_func("hex", TypeString, &color::hex, Flags::PURE);
		class_add_func("with_alpha", TypeColor, &color::with_alpha, Flags::PURE);
			func_add_param("a", TypeFloat32);
		class_add_func("hsb", TypeColor, &color::hsb, Flags::STATIC | Flags::PURE);
			func_add_param("h", TypeFloat32);
			func_add_param("s", TypeFloat32);
			func_add_param("b", TypeFloat32);
			func_add_param("a", TypeFloat32);
		class_add_func("interpolate", TypeColor, &color::interpolate, Flags::STATIC | Flags::PURE);
			func_add_param("c1", TypeColor);
			func_add_param("c2", TypeColor);
			func_add_param("t", TypeFloat32);
		class_add_func("_create", TypeColor, &KabaColor::set, Flags::STATIC | Flags::PURE);
			func_set_inline(InlineID::COLOR_SET);
			func_add_param("r", TypeFloat32);
			func_add_param("g", TypeFloat32);
			func_add_param("b", TypeFloat32);
			func_add_param("a", TypeFloat32);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaColor::init, Flags::MUTABLE);
			func_add_param("r", TypeFloat32);
			func_add_param("g", TypeFloat32);
			func_add_param("b", TypeFloat32);
			func_add_param("a", TypeFloat32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeColor, TypeColor, InlineID::CHUNK_ASSIGN, &KabaColor::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeColor, TypeColor, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeColor, TypeColor, InlineID::CHUNK_NOT_EQUAL);
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
		class_add_func(Identifier::Func::INIT, TypeVoid, &XList<color>::__init__, Flags::MUTABLE);

	add_class(TypePlane);
		class_add_element("_a", TypeFloat32, 0);
		class_add_element("_b", TypeFloat32, 4);
		class_add_element("_c", TypeFloat32, 8);
		class_add_element("d", TypeFloat32, &plane::d);
		class_add_element("n", TypeVec3, &plane::n);
		class_add_func("intersect_line", TypeBool, &plane::intersect_line, Flags::PURE);
			func_add_param("l1", TypeVec3);
			func_add_param("l2", TypeVec3);
			func_add_param("inter", TypeVec3);
		class_add_func("inverse", TypePlane, &plane::inverse, Flags::PURE);
		class_add_func("distance", TypeFloat32, &plane::distance, Flags::PURE);
			func_add_param("p", TypeVec3);
		class_add_func(Identifier::Func::STR, TypeString, &plane::str, Flags::PURE);
		class_add_func("transform", TypePlane, &plane::transform, Flags::PURE);
			func_add_param("m", TypeMat4);
		class_add_func("from_points", TypePlane, &plane::from_points, Flags::STATIC | Flags::PURE);
			func_add_param("a", TypeVec3);
			func_add_param("b", TypeVec3);
			func_add_param("c", TypeVec3);
		class_add_func("from_point_normal", TypePlane, &plane::from_point_normal, Flags::STATIC | Flags::PURE);
			func_add_param("p", TypeVec3);
			func_add_param("n", TypeVec3);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePlane, TypePlane, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypePlane, TypePlane, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypePlane, TypePlane, InlineID::CHUNK_NOT_EQUAL);
	
	add_class(TypePlaneList);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XList<plane>::__init__, Flags::MUTABLE);


	add_class(TypeRay);
		class_add_element("u", TypeVec3, &Ray::u);
		class_add_element("v", TypeVec3, &Ray::v);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaRay::init);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaRay::init_ex);
			func_add_param("a", TypeVec3);
			func_add_param("b", TypeVec3);
		class_add_func("dot", TypeFloat32, &Ray::dot, Flags::STATIC | Flags::PURE);
			func_add_param("r1", TypeRay);
			func_add_param("r2", TypeRay);
		class_add_func("intersect_plane", TypeVec3Optional, &Ray::intersect_plane, Flags::PURE);
			func_add_param("pl", TypePlane);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeRay, TypeRay, InlineID::CHUNK_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeRay, TypeRay, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeRay, TypeRay, InlineID::CHUNK_NOT_EQUAL);


	add_class(TypeMat4);
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
		class_add_const("ID", TypeMat4, &mat4::ID);
		class_add_func(Identifier::Func::STR, TypeString, &mat4::str, Flags::PURE);
		class_add_func("transform", TypeVec3, &mat4::transform, Flags::PURE);
			func_add_param("v", TypeVec3);
		class_add_func("transform_normal", TypeVec3, &mat4::transform_normal, Flags::PURE);
			func_add_param("v", TypeVec3);
		class_add_func("untransform", TypeVec3, &mat4::untransform, Flags::PURE);
			func_add_param("v", TypeVec3);
		class_add_func("project", TypeVec3, &mat4::project, Flags::PURE);
			func_add_param("v", TypeVec3);
		class_add_func("unproject", TypeVec3, &mat4::unproject, Flags::PURE);
			func_add_param("v", TypeVec3);
		class_add_func("inverse", TypeMat4, &mat4::inverse, Flags::PURE);
		class_add_func("transpose", TypeMat4, &mat4::transpose, Flags::PURE);
		class_add_func("translation", TypeMat4, &mat4::translation, Flags::STATIC | Flags::PURE);
			func_add_param("trans", TypeVec3);
		class_add_func("rotation", TypeMat4, &KabaMatrix<mat4>::rotation_v, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeVec3);
		class_add_func("rotation_x", TypeMat4, &mat4::rotation_x, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation_y", TypeMat4, &mat4::rotation_y, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation_z", TypeMat4, &mat4::rotation_z, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeFloat32);
		class_add_func("rotation", TypeMat4, &KabaMatrix<mat4>::rotation_q, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeQuaternion);
		class_add_func("scale", TypeMat4, &KabaMatrix<mat4>::scale_f, Flags::STATIC | Flags::PURE);
			func_add_param("s_x", TypeFloat32);
			func_add_param("s_y", TypeFloat32);
			func_add_param("s_z", TypeFloat32);
		class_add_func("scale", TypeMat4, &KabaMatrix<mat4>::scale_v, Flags::STATIC | Flags::PURE);
			func_add_param("s", TypeVec3);
		class_add_func("perspective", TypeMat4, &mat4::perspective, Flags::STATIC | Flags::PURE);
			func_add_param("fovy", TypeFloat32);
			func_add_param("aspect", TypeFloat32);
			func_add_param("z_near", TypeFloat32);
			func_add_param("z_far", TypeFloat32);
			func_add_param("z_sym", TypeBool);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeMat4, TypeMat4, InlineID::CHUNK_ASSIGN, &KabaVector<mat4>::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeMat4, TypeMat4, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeMat4, TypeMat4, InlineID::CHUNK_NOT_EQUAL);
		add_operator(OperatorID::MULTIPLY, TypeMat4, TypeMat4, TypeMat4, InlineID::NONE, &KabaMatrix<mat4>::mul);
		add_operator(OperatorID::MULTIPLY, TypeVec3, TypeMat4, TypeVec3, InlineID::NONE, &KabaMatrix<mat4>::mul_v<vec3>);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeMat4, TypeMat4, InlineID::NONE, &KabaMatrix<mat4>::imul);

	add_class(TypeMat3);
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
		class_add_const("ID", TypeMat3, &mat3::ID);
		class_add_const("0", TypeMat3, &mat3::ZERO);
		class_add_func(Identifier::Func::STR, TypeString, &mat3::str, Flags::PURE);
		class_add_func("inverse", TypeMat3, &mat3::inverse, Flags::PURE);
		class_add_func("rotation", TypeMat3, &KabaMatrix<mat3>::rotation_v, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeVec3);
		class_add_func("rotation", TypeMat3, &KabaMatrix<mat3>::rotation_q, Flags::STATIC | Flags::PURE);
			func_add_param("ang", TypeQuaternion);
		class_add_func("scale", TypeMat3, &KabaMatrix<mat3>::scale_f, Flags::STATIC | Flags::PURE);
			func_add_param("s_x", TypeFloat32);
			func_add_param("s_y", TypeFloat32);
			func_add_param("s_z", TypeFloat32);
		class_add_func("scale", TypeMat3, &KabaMatrix<mat3>::scale_v, Flags::STATIC | Flags::PURE);
			func_add_param("s", TypeVec3);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeMat3, TypeMat3, InlineID::CHUNK_ASSIGN, &KabaVector<mat3>::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeMat3, TypeMat3, InlineID::CHUNK_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeMat3, TypeMat3, InlineID::CHUNK_NOT_EQUAL);
		add_operator(OperatorID::MULTIPLY, TypeMat3, TypeMat3, TypeMat3, InlineID::NONE, &KabaMatrix<mat3>::mul);
		add_operator(OperatorID::MULTIPLY, TypeVec3, TypeMat3, TypeVec3, InlineID::NONE, &KabaMatrix<mat3>::mul_v<vec3>);
	
	add_class(TypeVli);
		class_add_element("sign", TypeBool, 0);
		class_add_element("data", TypeIntList, 4);
		class_add_func(Identifier::Func::INIT, TypeVoid, algebra_p(&vli::__init__), Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, algebra_p(&vli::__delete__), Flags::MUTABLE);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, algebra_p(&vli::set_vli), Flags::MUTABLE);
			func_add_param("v", TypeVli);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, algebra_p(&vli::set_str), Flags::MUTABLE);
			func_add_param("s", TypeString);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, algebra_p(&vli::set_int), Flags::MUTABLE);
			func_add_param("i", TypeInt);
		class_add_func(Identifier::Func::STR, TypeString, algebra_p(&vli::to_string), Flags::PURE);
		class_add_func("compare", TypeInt, algebra_p(&vli::compare), Flags::PURE);
			func_add_param("v", TypeVli);
		class_add_func("idiv", TypeVoid, algebra_p(&vli::idiv), Flags::MUTABLE);
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
		add_operator(OperatorID::EQUAL, TypeBool, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator==));
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator!=));
		add_operator(OperatorID::GREATER, TypeBool, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator<));
		add_operator(OperatorID::GREATER, TypeBool, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator>));
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator<=));
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator>=));
		add_operator(OperatorID::ADD, TypeVli, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator+));
		add_operator(OperatorID::SUBTRACT, TypeVli, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator-));
		add_operator(OperatorID::MULTIPLY, TypeVli, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator*));
		add_operator(OperatorID::ADDS, TypeVoid, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator+=));
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator-=));
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeVli, TypeVli, InlineID::NONE, algebra_p(&vli::operator*=));
	
	add_class(TypeAny);
		class_add_element("type", TypeClassRef, &Any::_class);
		class_add_element("data", TypePointer, &Any::data);
		class_add_func(Identifier::Func::INIT, TypeVoid, &Any::__init__, Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &Any::__delete__, Flags::MUTABLE);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &Any::set, Flags::MUTABLE);
			func_add_param("a", TypeAny);
		class_add_func("clear", TypeVoid, &Any::clear, Flags::MUTABLE);
		class_add_func(Identifier::Func::LENGTH, TypeInt, &Any::length, Flags::PURE);
		class_add_func(Identifier::Func::GET, TypeAny, &KabaAny::_map_get, Flags::REF | Flags::RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_func(Identifier::Func::SET, TypeVoid, &KabaAny::_map_set, Flags::RAISES_EXCEPTIONS | Flags::MUTABLE);
			func_add_param("key", TypeString);
			func_add_param("value", TypeAny);
		class_add_func(Identifier::Func::GET, TypeAny, &KabaAny::_array_get, Flags::REF | Flags::RAISES_EXCEPTIONS);
			func_add_param("index", TypeInt);
		class_add_func(Identifier::Func::SET, TypeVoid, &KabaAny::_array_set, Flags::RAISES_EXCEPTIONS | Flags::MUTABLE);
			func_add_param("index", TypeInt);
			func_add_param("value", TypeAny);
		class_add_func("is_empty", TypeBool, &Any::is_empty, Flags::PURE);
		class_add_func("has", TypeBool, &Any::has, Flags::PURE);
			func_add_param("key", TypeString);
		class_add_func("add", TypeVoid, &KabaAny::_array_add, Flags::RAISES_EXCEPTIONS | Flags::MUTABLE);
			func_add_param("a", TypeAny);
		class_add_func("drop", TypeVoid, &Any::map_drop, Flags::RAISES_EXCEPTIONS | Flags::MUTABLE);
			func_add_param("key", TypeString);
		class_add_func("keys", TypeStringList, &Any::keys, Flags::PURE);//, Flags::RAISES_EXCEPTIONS);
		class_add_func("__bool__", TypeBool, &Any::_bool, Flags::PURE);
		class_add_func("__int__", TypeInt, &Any::_int, Flags::PURE);
		class_add_func("__float__", TypeFloat32, &Any::_float, Flags::PURE);
		class_add_func(Identifier::Func::STR, TypeString, &Any::str, Flags::PURE);
		class_add_func(Identifier::Func::REPR, TypeString, &Any::repr, Flags::PURE);
		class_add_func("unwrap", TypeVoid, &KabaAny::unwrap, Flags::RAISES_EXCEPTIONS);
			func_add_param("var", TypeReference);
			func_add_param("type", TypeClassRef);
		class_add_func("parse", TypeAny, &KabaAny::parse, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("s", TypeString);
		add_operator(OperatorID::ADDS, TypeVoid, TypeAny, TypeAny, InlineID::NONE, &Any::_add);// operator+=);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeAny, TypeAny, InlineID::NONE, &Any::_sub);// operator-);

	add_func("@int2any", TypeAny, &int2any, Flags::STATIC);
		func_add_param("i", TypeInt);
	add_func("@float2any", TypeAny, &float2any, Flags::STATIC);
		func_add_param("i", TypeFloat32);
	add_func("@bool2any", TypeAny, &bool2any, Flags::STATIC);
		func_add_param("i", TypeBool);
	add_func("@str2any", TypeAny, &str2any, Flags::STATIC);
		func_add_param("s", TypeString);
	add_func("@pointer2any", TypeAny, &pointer2any, Flags::STATIC);
		func_add_param("p", TypePointer);


	add_class(TypeCrypto);
		class_add_element("n", TypeVli, 0);
		class_add_element("k", TypeVli, sizeof(vli));
		class_add_func(Identifier::Func::INIT, TypeVoid, algebra_p(&Crypto::__init__), Flags::MUTABLE);
		class_add_func(Identifier::Func::STR, TypeString, algebra_p(&Crypto::str), Flags::PURE);
		class_add_func("from_str", TypeVoid, algebra_p(&Crypto::from_str), Flags::MUTABLE);
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
		class_add_func(Identifier::Func::INIT, TypeVoid, &Random::__init__, Flags::MUTABLE);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &Random::__assign__, Flags::MUTABLE);
			func_add_param("o", TypeRandom);
		//class_add_element("n", TypeRandom, 0);
		class_add_func("seed", TypeVoid, &Random::seed, Flags::MUTABLE);
			func_add_param("str", TypeString);
		class_add_func("int", TypeInt, &Random::_int, Flags::MUTABLE);
			func_add_param("max", TypeInt);
		class_add_func("uniform01", TypeFloat32, &Random::uniform01, Flags::MUTABLE);
		class_add_func("uniform", TypeFloat32, &Random::uniform, Flags::MUTABLE);
			func_add_param("min", TypeFloat32);
			func_add_param("max", TypeFloat32);
		class_add_func("normal", TypeFloat32, &Random::normal, Flags::MUTABLE);
			func_add_param("mean", TypeFloat32);
			func_add_param("stddev", TypeFloat32);
		class_add_func("in_ball", TypeVec3, &Random::in_ball, Flags::MUTABLE);
			func_add_param("r", TypeFloat32);
		class_add_func("dir", TypeVec3, &Random::dir, Flags::MUTABLE);
	
	
	add_class(TypeFloatInterpolator);
		class_add_element("type", TypeInt, 0);
		class_add_func(Identifier::Func::INIT, TypeVoid, &Interpolator<float>::__init__, Flags::MUTABLE);
		class_add_func("clear", TypeVoid, &Interpolator<float>::clear, Flags::MUTABLE);
		class_add_func("set_type", TypeVoid, &Interpolator<float>::setType, Flags::MUTABLE);
			func_add_param("type", TypeString);
		class_add_func("add", TypeVoid, &Interpolator<float>::addv, Flags::MUTABLE);
			func_add_param("p", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("add2", TypeVoid, &Interpolator<float>::add2v, Flags::MUTABLE);
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("add3", TypeVoid, &Interpolator<float>::add3v, Flags::MUTABLE);
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
			func_add_param("w", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("jump", TypeVoid, &Interpolator<float>::jumpv, Flags::MUTABLE);
			func_add_param("p", TypeFloat32);
			func_add_param("v", TypeFloat32);
		class_add_func("normalize", TypeVoid, &Interpolator<float>::normalize, Flags::MUTABLE);
		class_add_func("get", TypeFloat32, &Interpolator<float>::get, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_tang", TypeFloat32, &Interpolator<float>::getTang, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_list", TypeFloatList, &Interpolator<float>::getList, Flags::PURE);
			func_add_param("t", TypeFloatList);

	
	add_class(TypeVectorInterpolator);
		class_add_element("type", TypeInt, 0);
		class_add_func(Identifier::Func::INIT, TypeVoid, &Interpolator<vec3>::__init__, Flags::MUTABLE);
		class_add_func("clear", TypeVoid, &Interpolator<vec3>::clear, Flags::MUTABLE);
		class_add_func("set_type", TypeVoid, &Interpolator<vec3>::setType, Flags::MUTABLE);
			func_add_param("type", TypeString);
		class_add_func("add", TypeVoid, &Interpolator<vec3>::add, Flags::MUTABLE);
			func_add_param("p", TypeVec3);
			func_add_param("dt", TypeFloat32);
		class_add_func("add2", TypeVoid, &Interpolator<vec3>::add2, Flags::MUTABLE);
			func_add_param("p", TypeVec3);
			func_add_param("v", TypeVec3);
			func_add_param("dt", TypeFloat32);
		class_add_func("add3", TypeVoid, &Interpolator<vec3>::add3, Flags::MUTABLE);
			func_add_param("p", TypeVec3);
			func_add_param("v", TypeVec3);
			func_add_param("w", TypeFloat32);
			func_add_param("dt", TypeFloat32);
		class_add_func("jump", TypeVoid, &Interpolator<vec3>::jump, Flags::MUTABLE);
			func_add_param("p", TypeVec3);
			func_add_param("v", TypeVec3);
		class_add_func("normalize", TypeVoid, &Interpolator<vec3>::normalize, Flags::MUTABLE);
		class_add_func("get", TypeVec3, &Interpolator<vec3>::get, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_tang", TypeVec3, &Interpolator<vec3>::getTang, Flags::PURE);
			func_add_param("t", TypeFloat32);
		class_add_func("get_list", TypeVec3List, &Interpolator<vec3>::getList, Flags::PURE);
			func_add_param("t", TypeFloatList);

	add_class(TypeFFT);
		class_add_func("c2c", TypeVoid, fft_p(&fft::c2c), Flags::STATIC | Flags::PURE);
			func_add_param("in", TypeComplexList);
			func_add_param("out", TypeComplexList, Flags::OUT);
			func_add_param("invers", TypeBool);
		class_add_func("r2c", TypeVoid, fft_p(&fft::r2c), Flags::STATIC | Flags::PURE);
			func_add_param("in", TypeFloatList);
			func_add_param("out", TypeComplexList, Flags::OUT);
		class_add_func("c2r_inv", TypeVoid, fft_p(&fft::c2r_inv), Flags::STATIC | Flags::PURE);
			func_add_param("in", TypeComplexList);
			func_add_param("out", TypeFloatList, Flags::OUT);
		class_add_func("c2c_2d", TypeVoid, fft_p(&fft::c2c_2d), Flags::STATIC | Flags::PURE);
			func_add_param("in", TypeComplexList);
			func_add_param("out", TypeComplexList, Flags::OUT);
			func_add_param("n", TypeInt);
			func_add_param("invers", TypeBool);


	// int
	add_func("clamp", TypeInt, &clamp<int>, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt);
		func_add_param("min", TypeInt);
		func_add_param("max", TypeInt);
	add_func("loop", TypeInt, &loop<int>, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt);
		func_add_param("min", TypeInt);
		func_add_param("max", TypeInt);
	add_func("abs", TypeInt, &abs<int>, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt);
	add_func("sign", TypeInt, &sign<int>, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt);
	add_func("min", TypeInt, &min<int>, Flags::STATIC | Flags::PURE);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);
	add_func("max", TypeInt, &max<int>, Flags::STATIC | Flags::PURE);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);

	// float
	add_func("sin", TypeFloat32, &sinf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("cos", TypeFloat32, &cosf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("tan", TypeFloat32, &tanf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("asin", TypeFloat32, &asinf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("acos", TypeFloat32, &acosf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("atan", TypeFloat32, &atanf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("atan2", TypeFloat32, &atan2f, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
		func_add_param("y", TypeFloat32);
	add_func("sqrt", TypeFloat32, &sqrtf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("sqr", TypeFloat32, &f_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("exp", TypeFloat32, &expf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("log", TypeFloat32, &logf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
	add_func("pow", TypeFloat32, &powf, Flags::STATIC | Flags::PURE);
		func_add_param("x", TypeFloat32);
		func_add_param("exp", TypeFloat32);
	add_func("clamp", TypeFloat32, &clamp<float>, Flags::STATIC | Flags::PURE);
		func_add_param("f", TypeFloat32);
		func_add_param("min", TypeFloat32);
		func_add_param("max", TypeFloat32);
	add_func("loop", TypeFloat32, &loop<float>, Flags::STATIC | Flags::PURE);
		func_add_param("f", TypeFloat32);
		func_add_param("min", TypeFloat32);
		func_add_param("max", TypeFloat32);
	add_func("abs", TypeFloat32, &abs<float>, Flags::STATIC | Flags::PURE);
		func_add_param("f", TypeFloat32);
	add_func("sign", TypeFloat32, &sign<float>, Flags::STATIC | Flags::PURE);
		func_add_param("f", TypeFloat32);
	add_func("min", TypeFloat32, &min<float>, Flags::STATIC | Flags::PURE);
		func_add_param("a", TypeFloat32);
		func_add_param("b", TypeFloat32);
	add_func("max", TypeFloat32, &max<float>, Flags::STATIC | Flags::PURE);
		func_add_param("a", TypeFloat32);
		func_add_param("b", TypeFloat32);

	// complex
	add_func("abs", TypeFloat32, &KabaVector<complex>::abs, Flags::STATIC | Flags::PURE);
		func_add_param("z", TypeComplex);

	// int[]
	add_func("sum", TypeInt, &XList<int>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("sum_sqr", TypeInt, &XList<int>::sum_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("min", TypeInt, &XList<int>::min, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("max", TypeInt, &XList<int>::max, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("argmin", TypeInt, &XList<int>::argmin, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("argmax", TypeInt, &XList<int>::argmax, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("unique", TypeIntList, &XList<int>::unique, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeIntList);
	add_func("range", TypeIntList, (void*)&kaba_range<int>, Flags::STATIC | Flags::PURE);
		func_add_param("start", TypeInt);
		func_add_param_def("end", TypeInt, DynamicArray::MAGIC_END_INDEX);
		func_add_param_def("step", TypeInt, 1);

	// float[]
	add_func("sum", TypeFloat32, &XList<float>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloatList);
	add_func("sum_sqr", TypeFloat32, &XList<float>::sum_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloatList);
	add_func("min", TypeFloat32, &XList<float>::min, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloatList);
	add_func("max", TypeFloat32, &XList<float>::max, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloatList);
	add_func("argmin", TypeInt, &XList<float>::argmin, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloatList);
	add_func("argmax", TypeInt, &XList<float>::argmax, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloatList);
	add_func("range", TypeFloatList, (void*)&kaba_range<float>, Flags::STATIC | Flags::PURE);
		func_add_param("start", TypeFloat32);
		func_add_param_def("end", TypeFloat32, (float)DynamicArray::MAGIC_END_INDEX);
		func_add_param_def("step", TypeFloat32, 1.0f);

	// float64[]
	add_func("sum", TypeFloat64, &XList<double>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloat64List);
	add_func("sum_sqr", TypeFloat64, &XList<double>::sum_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloat64List);
	add_func("min", TypeFloat64, &XList<double>::min, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloat64List);
	add_func("max", TypeFloat64, &XList<double>::max, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloat64List);
	add_func("argmin", TypeInt, &XList<double>::argmin, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloat64List);
	add_func("argmax", TypeInt, &XList<double>::argmax, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeFloat64List);

	// vec2[]
	add_func("sum", TypeVec2, &VectorList<vec2>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeVec2List);
	add_func("sum_sqr", TypeFloat32, &VectorList<vec2>::sum_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeVec2List);

	// vec3[]
	add_func("sum", TypeVec3, &VectorList<vec3>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeVec3List);
	add_func("sum_sqr", TypeFloat32, &VectorList<vec3>::sum_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeVec3List);

	// complex[]
	add_func("sum", TypeComplex, &VectorList<complex>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeComplexList);
	add_func("sum_sqr", TypeFloat32, &VectorList<complex>::sum_sqr, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeComplexList);

	// string[]
	add_func("sum", TypeString, &XList<string>::sum, Flags::STATIC | Flags::PURE);
		func_add_param("list", TypeStringList);

	// other types
	add_func("bary_centric", TypeVec2, (void*)&bary_centric, Flags::STATIC | Flags::PURE);
		func_add_param("p", TypeVec3);
		func_add_param("a", TypeVec3);
		func_add_param("b", TypeVec3);
		func_add_param("c", TypeVec3);

	// random numbers
	add_func("rand", TypeInt, &randi, Flags::STATIC);
		func_add_param("max", TypeInt);
	add_func("rand", TypeFloat32, &randf, Flags::STATIC);
		func_add_param("max", TypeFloat32);
	add_func("rand_seed", TypeVoid, &srand, Flags::STATIC);
		func_add_param("seed", TypeInt);

	add_ext_var("_any_allow_simple_output", TypeBool, (void*)&Any::allow_simple_output);
	
	// float
	add_const("pi",  TypeFloat32, &pi);


	// needs to be defined after any
	TypeAnyList = add_type_list(TypeAny);
	lib_create_list<Any>(TypeAnyList);

	TypeAnyDict = add_type_dict(TypeAny);
	lib_create_dict<Any>(TypeAnyDict);


	add_class(TypeAny);
		class_add_func("as_array", TypeAnyList, &KabaAny::_as_array, Flags::REF | Flags::RAISES_EXCEPTIONS);
		class_add_func("as_map", TypeAnyDict, &KabaAny::_as_map, Flags::REF | Flags::RAISES_EXCEPTIONS);


	add_type_cast(50, TypeInt, TypeAny, "math.@int2any");
	add_type_cast(50, TypeFloat32, TypeAny, "math.@float2any");
	add_type_cast(50, TypeBool, TypeAny, "math.@bool2any");
	add_type_cast(50, TypeString, TypeAny, "math.@str2any");
	add_type_cast(50, TypePointer, TypeAny, "math.@pointer2any");
}

};
