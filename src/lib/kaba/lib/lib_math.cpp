#include "../../os/msg.h"
#include "../../math/math.h"
#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "../../math/complex.h"
#include "../../math/quaternion.h"
#include "../../math/mat4.h"
#include "../../math/mat3.h"
#include "../../math/plane.h"
#include "../../math/Box.h"
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

#if __has_include("../../any/any.h")
	#include "../../any/any.h"
	#define HAS_ANY
#else
	typedef int Any;
	#error("no any.h ... we're screwed")
#endif

namespace kaba {

#ifdef HAS_ANY
	#define any_p(p)		p
#else
	#define any_p(p)		nullptr
#endif


// we're always using math types
#define type_p(p)			p


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
	static rect set(float x1, float x2, float y1, float y2) {
		return rect(x1, x2, y1, y2);
	}
	static rect set2(const vec2& p00, const vec2& p11) {
		return rect(p00, p11);
	}
};


KABA_LINK_GROUP_BEGIN


class KabaAny : public Any {
public:
	const Class* _get_class() {
		if (type == Any::Type::Int)
			return common_types.i32; // legacy...
		if (type == Any::Type::Float)
			return common_types.f32; // legacy...
		if (type == Any::Type::Bool)
			return common_types._bool;
		if (type == Any::Type::String)
			return common_types.string;
		if (type == Any::Type::List)
			return common_types.any_list;
		if (type == Any::Type::Dict)
			return common_types.any_dict;
		return common_types._void;
	}

	Array<Any>* _as_list() {
		if (type != Type::List)
			return nullptr;
		return &as_list();
	}
	Array<int>* _as_dict() { // FAKE TYPE!!!
		if (type != Type::Dict)
			return nullptr;
		return (Array<int>*)&as_dict();
	}

	void set(const Any &a) {	*static_cast<Any*>(this) = a;	}
	void _add(const Any &a) {	Any b = *this + a;	*static_cast<Any*>(this) = b;	}
	void _sub(const Any &a) {	Any b = *this - a;	*static_cast<Any*>(this) = b;	}

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


KABA_LINK_GROUP_END


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

struct KabaBox : public Box {
	KabaBox(const vec3& min, const vec3& max) {
		this->min = min;
		this->max = max;
	}
};

template<class T>
T kaba_xor(T a, T b) {
	return a ^ b;
}

void SIAddPackageMath(Context *c) {
	add_internal_package(c, "math", "1", Flags::AutoImport);

	// types
	common_types.complex = add_type("Complex", sizeof(complex));
	common_types.complex_list = add_type_list(common_types.complex);
	common_types.vec2 = add_type("vec2", sizeof(vec2));
	common_types.vec2_list = add_type_list(common_types.vec2);
	common_types.vec3 = add_type("vec3", sizeof(vec3));
	common_types.vec3_list = add_type_list(common_types.vec3);
	common_types.rect = add_type("Rect", sizeof(rect));
	auto TypeBox = add_type("Box", sizeof(Box));
	common_types.mat4 = add_type("mat4", sizeof(mat4));
	common_types.quaternion = add_type("Quaternion", sizeof(quaternion));
	common_types.plane = add_type("Plane", sizeof(plane));
	common_types.plane_list = add_type_list(common_types.plane);
	common_types.color = add_type("Color", sizeof(color));
	common_types.color_list = add_type_list(common_types.color);
	auto TypeRay = add_type("Ray", sizeof(Ray));
	common_types.mat3 = add_type("mat3", sizeof(mat3));
	auto TypeFloatArray3 = add_type_array(common_types.f32, 3);
	auto TypeFloatArray4 = add_type_array(common_types.f32, 4);
	auto TypeFloatArray4x4 = add_type_array(TypeFloatArray4, 4);
	auto TypeFloatArray16 = add_type_array(common_types.f32, 16);
	auto TypeFloatArray3x3 = add_type_array(TypeFloatArray3, 3);
	auto TypeFloatArray9 = add_type_array(common_types.f32, 9);
	common_types.any = add_type("Any", sizeof(Any));
	auto TypeFloatInterpolator = add_type("FloatInterpolator", sizeof(Interpolator<float>));
	auto TypeVectorInterpolator = add_type("VectorInterpolator", sizeof(Interpolator<vec3>));
	auto TypeRandom = add_type("Random", sizeof(Random));

	const_cast<Class*>(common_types.vec3)->alignment = 4; // would be updated too late, otherwise...
	auto TypeVec3Optional = add_type_optional(common_types.vec3);

	auto TypeAnyP = add_type_p_raw(common_types.any);
	
	// dirty hack :P
	/*if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64)*/ {
		flags_set(((Class*)common_types.f32)->flags, Flags::ReturnInFloatRegisters);
		flags_set(((Class*)common_types.f64)->flags, Flags::ReturnInFloatRegisters);
		if (config.target.abi == Abi::AMD64_GNU or config.target.abi == Abi::ARM64_GNU) {
			// not on windows!
			flags_set(((Class*)common_types.complex)->flags, Flags::ReturnInFloatRegisters);
			flags_set(((Class*)common_types.vec2)->flags, Flags::ReturnInFloatRegisters);
			flags_set(((Class*)common_types.quaternion)->flags, Flags::ReturnInFloatRegisters);
			flags_set(((Class*)common_types.vec3)->flags, Flags::ReturnInFloatRegisters);
			flags_set(((Class*)common_types.color)->flags, Flags::ReturnInFloatRegisters);
			flags_set(((Class*)common_types.plane)->flags, Flags::ReturnInFloatRegisters);
			flags_set(((Class*)common_types.rect)->flags, Flags::ReturnInFloatRegisters);
		}
	}

	lib_create_list<complex>(common_types.complex_list);
	lib_create_list<vec2>(common_types.vec2_list);
	lib_create_list<vec3>(common_types.vec3_list);
	lib_create_list<plane>(common_types.plane_list);
	lib_create_list<color>(common_types.color_list);

	lib_create_optional<vec3>(TypeVec3Optional);

	add_operator(OperatorID::Assign, common_types._void, TypeFloatArray3, TypeFloatArray3, InlineID::ChunkAssign, &FloatN<3>::__assign__);
	add_operator(OperatorID::Assign, common_types._void, TypeFloatArray4, TypeFloatArray4, InlineID::ChunkAssign, &FloatN<4>::__assign__);
	add_operator(OperatorID::Assign, common_types._void, TypeFloatArray9, TypeFloatArray9, InlineID::ChunkAssign, &FloatN<9>::__assign__);
	add_operator(OperatorID::Assign, common_types._void, TypeFloatArray3x3, TypeFloatArray3x3, InlineID::ChunkAssign, &FloatN<9>::__assign__);
	add_operator(OperatorID::Assign, common_types._void, TypeFloatArray16, TypeFloatArray16, InlineID::ChunkAssign, &FloatN<16>::__assign__);
	add_operator(OperatorID::Assign, common_types._void, TypeFloatArray4x4, TypeFloatArray4x4, InlineID::ChunkAssign, &FloatN<16>::__assign__);


	add_class(common_types.complex);
		class_add_element("x", common_types.f32, &complex::x);
		class_add_element("y", common_types.f32, &complex::y);
		class_add_func("abs", common_types.f32, &complex::abs, Flags::Pure);
		class_add_func("abs_sqr", common_types.f32, &complex::abs_sqr, Flags::Pure);
		class_add_func("bar", common_types.complex, &complex::bar, Flags::Pure);
		class_add_func(Identifier::func::Str, common_types.string, &complex::str, Flags::Pure);
		class_add_const("I", common_types.complex, &complex::I);
		class_add_func("_create", common_types.complex, &KabaVector<complex>::set2, Flags::Static | Flags::Pure);
			func_set_inline(InlineID::ComplexSet);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
		class_add_func(Identifier::func::Init, common_types._void, &KabaVector<complex>::init2, Flags::Mutable);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
		add_operator(OperatorID::Assign, common_types._void, common_types.complex, common_types.complex, InlineID::ChunkAssign, &KabaVector<complex>::assign);
		add_operator(OperatorID::Add, common_types.complex, common_types.complex, common_types.complex, InlineID::Vec2Add, &complex::operator+);
		add_operator(OperatorID::Subtract, common_types.complex, common_types.complex, common_types.complex, InlineID::Vec2Subtract, (decltype(&complex::operator+)) &complex::operator-);
		add_operator(OperatorID::Multiply, common_types.complex, common_types.complex, common_types.complex, InlineID::ComplexMultiply, (decltype(&complex::operator+)) &complex::operator*);
		add_operator(OperatorID::Multiply, common_types.complex, common_types.f32, common_types.complex, InlineID::Vec2MultiplySV);
		add_operator(OperatorID::Multiply, common_types.complex, common_types.complex, common_types.f32, InlineID::Vec2MultiplyVS);
		add_operator(OperatorID::Divide, common_types.complex, common_types.complex, common_types.complex, InlineID::None /*InlineID::COMPLEX_DIVIDE*/, (decltype(&complex::operator+)) &complex::operator/);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.complex, common_types.complex, InlineID::Vec2AddAssign, &complex::operator+=);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.complex, common_types.complex, InlineID::Vec2SubtarctAssign, &complex::operator-=);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.complex, common_types.complex, InlineID::ComplexMultiplyAssign, (decltype(&complex::operator+=)) &complex::operator*=);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.complex, common_types.complex, InlineID::ComplexDivideAssign, (decltype(&complex::operator+=)) &complex::operator/=);
		add_operator(OperatorID::Equal, common_types._bool, common_types.complex, common_types.complex, InlineID::ChunkEqual, &complex::operator==);
		add_operator(OperatorID::Negative, common_types.complex, nullptr, common_types.complex, InlineID::Vec2Negative, &KabaVector<complex>::negate);

	add_class(common_types.complex_list);
		add_operator(OperatorID::Add, common_types.complex_list, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::add_values);
		add_operator(OperatorID::Subtract, common_types.complex_list, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::sub_values);
		add_operator(OperatorID::Multiply, common_types.complex_list, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::mul_values);
		add_operator(OperatorID::Divide, common_types.complex_list, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::div_values);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::iadd_values);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::isub_values);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::imul_values);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.complex_list, common_types.complex_list, InlineID::None, &VectorList<complex>::idiv_values);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.complex_list, common_types.complex, InlineID::None, &VectorList<complex>::iadd_values_scalar);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.complex_list, common_types.complex, InlineID::None, &VectorList<complex>::isub_values_scalar);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.complex_list, common_types.complex, InlineID::None, &VectorList<complex>::imul_values_scalar);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.complex_list, common_types.complex, InlineID::None, &VectorList<complex>::idiv_values_scalar);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.complex_list, common_types.f32, InlineID::None, &VectorList<complex>::imul_values_scalar_f);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.complex_list, common_types.f32, InlineID::None, &VectorList<complex>::idiv_values_scalar_f);
		add_operator(OperatorID::Assign, common_types._void, common_types.complex_list, common_types.complex, InlineID::None, &VectorList<complex>::assign_values_scalar);


	add_class(common_types.vec2);
		class_add_element("x", common_types.f32, &vec2::x);
		class_add_element("y", common_types.f32, &vec2::y);
		//class_add_element("_e", TypeFloatArray2, 0);
		class_add_func(Identifier::func::Length, common_types.f32, type_p(&vec2::length), Flags::Pure);
		class_add_func("length", common_types.f32, type_p(&vec2::length), Flags::Pure);
		//class_add_func("length_sqr", common_types.f32, type_p(&vec2::length_sqr), Flags::PURE);
		//class_add_func("length_fuzzy", common_types.f32, type_p(&vec2::length_fuzzy), Flags::PURE);
		class_add_func("normalized", common_types.vec2, &vec2::normalized, Flags::Pure);
		class_add_func(Identifier::func::Str, common_types.string, &vec2::str, Flags::Pure);
		class_add_func("dot", common_types.f32, &vec2::dot, Flags::Static | Flags::Pure);
			func_add_param("v1", common_types.vec2);
			func_add_param("v2", common_types.vec2);
		class_add_func("_create", common_types.vec2, &KabaVector<vec2>::set2, Flags::Static | Flags::Pure);
			func_set_inline(InlineID::ComplexSet);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
		// ignored, but useful for docu
		class_add_func(Identifier::func::Init, common_types._void, &KabaVector<vec2>::init2, Flags::Mutable);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
		class_add_const("0", common_types.vec2, &vec2::ZERO);
		class_add_const("O", common_types.vec2, &vec2::ZERO);
		class_add_const("EX", common_types.vec2, &vec2::EX);
		class_add_const("EY", common_types.vec2, &vec2::EY);
		add_operator(OperatorID::Assign, common_types._void, common_types.vec2, common_types.vec2, InlineID::ChunkAssign, &KabaVector<vec2>::assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.vec2, common_types.vec2, InlineID::ChunkEqual, &vec2::operator==);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.vec2, common_types.vec2, InlineID::ChunkNotEqual, &vec2::operator!=);
		add_operator(OperatorID::Add, common_types.vec2, common_types.vec2, common_types.vec2, InlineID::Vec2Add, &vec2::operator+);
		add_operator(OperatorID::Subtract, common_types.vec2, common_types.vec2, common_types.vec2, InlineID::Vec2Subtract, (decltype(&vec2::operator+)) &vec2::operator-);
//		add_operator(OperatorID::MULTIPLY, common_types.f32, common_types.vec2, common_types.vec2, InlineID::COMPLEX_MULTIPLY_VV, &KabaVector<vec2>::mul_vv);
		add_operator(OperatorID::Multiply, common_types.vec2, common_types.vec2, common_types.f32, InlineID::Vec2MultiplyVS, &KabaVector<vec2>::mul_vf);
		add_operator(OperatorID::Multiply, common_types.vec2, common_types.f32, common_types.vec2, InlineID::Vec2MultiplySV, &KabaVector<vec2>::mul_fv);
		add_operator(OperatorID::Divide, common_types.vec2, common_types.vec2, common_types.f32, InlineID::Vec2DivideVs, &KabaVector<vec2>::div_f);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.vec2, common_types.vec2, InlineID::Vec2AddAssign, &vec2::operator+=);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.vec2, common_types.vec2, InlineID::Vec2SubtarctAssign, &vec2::operator-=);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.vec2, common_types.f32, InlineID::Vec2MultiplyAssign, &vec2::operator*=);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.vec2, common_types.f32, InlineID::Vec2DivideAssign, &vec2::operator/=);
		add_operator(OperatorID::Negative, common_types.vec2, nullptr, common_types.vec2, InlineID::Vec2Negative, &KabaVector<vec2>::negate);


	add_class(common_types.vec3);
		class_add_element("x", common_types.f32, &vec3::x);
		class_add_element("y", common_types.f32, &vec3::y);
		class_add_element("z", common_types.f32, &vec3::z);
		class_add_element("_e", TypeFloatArray3, &vec3::x);
		class_add_element("_xy", common_types.vec2, &vec3::x);
		class_add_func(Identifier::func::Length, common_types.f32, type_p(&vec3::length), Flags::Pure);
		class_add_func("length", common_types.f32, type_p(&vec3::length), Flags::Pure);
		class_add_func("length_sqr", common_types.f32, type_p(&vec3::length_sqr), Flags::Pure);
		class_add_func("length_fuzzy", common_types.f32, type_p(&vec3::length_fuzzy), Flags::Pure);
		class_add_func("normalized", common_types.vec3, &vec3::normalized, Flags::Pure);
		class_add_func("dir2ang", common_types.vec3, &vec3::dir2ang, Flags::Pure);
		class_add_func("dir2ang2", common_types.vec3, &vec3::dir2ang2, Flags::Pure);
			func_add_param("up", common_types.vec3);
		class_add_func("ang2dir", common_types.vec3, &vec3::ang2dir, Flags::Pure);
//		class_add_func("rotate", TypeVector, &vector::rotate, Flags::PURE);
//			func_add_param("ang", TypeVector);
//		class_add_func("__div__", TypeVector, vector::untransform), Flags::PURE);
//			func_add_param("m", TypeMatrix);
		class_add_func("ortho", common_types.vec3, &vec3::ortho, Flags::Pure);
		class_add_func(Identifier::func::Str, common_types.string, &vec3::str, Flags::Pure);
		class_add_func("dot", common_types.f32, &vec3::dot, Flags::Static | Flags::Pure);
			func_add_param("v1", common_types.vec3);
			func_add_param("v2", common_types.vec3);
		class_add_func("cross", common_types.vec3, &vec3::cross, Flags::Static | Flags::Pure);
			func_add_param("v1", common_types.vec3);
			func_add_param("v2", common_types.vec3);
		class_add_func("_create", common_types.vec3, &KabaVector<vec3>::set3, Flags::Static | Flags::Pure);
			func_set_inline(InlineID::VectorSet);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
			func_add_param("z", common_types.f32);
		// ignored, but useful for docu
		class_add_func(Identifier::func::Init, common_types._void, &KabaVector<vec3>::init3, Flags::Mutable);
			func_add_param("x", common_types.f32);
			func_add_param("y", common_types.f32);
			func_add_param("z", common_types.f32);
/*		class_add_func("ang_add", TypeVector, &VecAngAdd, Flags::STATIC | Flags::PURE);
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
		class_add_func("ang_interpolate", TypeVector, &VecAngInterpolate, Flags::STATIC | Flags::PURE);
			func_add_param("ang1", TypeVector);
			func_add_param("ang2", TypeVector);
			func_add_param("t", common_types.f32);*/
		class_add_const("0", common_types.vec3, &vec3::ZERO);
		class_add_const("O", common_types.vec3, &vec3::ZERO);
		class_add_const("EX", common_types.vec3, &vec3::EX);
		class_add_const("EY", common_types.vec3, &vec3::EY);
		class_add_const("EZ", common_types.vec3, &vec3::EZ);
		add_operator(OperatorID::Assign, common_types._void, common_types.vec3, common_types.vec3, InlineID::ChunkAssign, &KabaVector<vec3>::assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.vec3, common_types.vec3, InlineID::ChunkEqual, &vec3::operator==);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.vec3, common_types.vec3, InlineID::ChunkNotEqual, &vec3::operator!=);
		add_operator(OperatorID::Add, common_types.vec3, common_types.vec3, common_types.vec3, InlineID::Vec3Add, &vec3::operator+);
		add_operator(OperatorID::Subtract, common_types.vec3, common_types.vec3, common_types.vec3, InlineID::Vec3Subtract, (decltype(&vec3::operator+)) &vec3::operator-);
		add_operator(OperatorID::Multiply, common_types.f32, common_types.vec3, common_types.vec3, InlineID::Vec3MultiplyVV, &KabaVector<vec3>::mul_vv);
		add_operator(OperatorID::Multiply, common_types.vec3, common_types.vec3, common_types.f32, InlineID::Vec3MultiplyVF, &KabaVector<vec3>::mul_vf);
		add_operator(OperatorID::Multiply, common_types.vec3, common_types.f32, common_types.vec3, InlineID::Vec3MultiplyFV, &KabaVector<vec3>::mul_fv);
		add_operator(OperatorID::Divide, common_types.vec3, common_types.vec3, common_types.f32, InlineID::Vec3DivideVF, &KabaVector<vec3>::div_f);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.vec3, common_types.vec3, InlineID::Vec3AddAssign, &vec3::operator+=);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.vec3, common_types.vec3, InlineID::Vec3SubtarctAssign, &vec3::operator-=);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.vec3, common_types.f32, InlineID::Vec3MultiplyAssign, &vec3::operator*=);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.vec3, common_types.f32, InlineID::Vec3DivideAssign, &vec3::operator/=);
		add_operator(OperatorID::Negative, common_types.vec3, nullptr, common_types.vec3, InlineID::Vec3Negative, &KabaVector<vec3>::negate);


	add_class(common_types.quaternion);
		class_add_element("x", common_types.f32, &quaternion::x);
		class_add_element("y", common_types.f32, &quaternion::y);
		class_add_element("z", common_types.f32, &quaternion::z);
		class_add_element("w", common_types.f32, &quaternion::w);
		class_add_func("bar", common_types.quaternion, &quaternion::bar, Flags::Pure);
		class_add_func("normalize", common_types._void, &quaternion::normalize, Flags::Mutable);
		class_add_func("angles", common_types.vec3, &quaternion::get_angles, Flags::Pure);
		class_add_func(Identifier::func::Str, common_types.string, &quaternion::str, Flags::Pure);
		class_add_func(Identifier::func::Init, common_types._void, nullptr, Flags::Mutable);
			func_add_param("ang", common_types.vec3);
		class_add_func(Identifier::func::Init, common_types._void, nullptr, Flags::Mutable);
			func_add_param("axis", common_types.vec3);
			func_add_param("angle", common_types.f32);
		class_add_func(Identifier::func::Init, common_types._void, nullptr, Flags::Mutable);
			func_add_param("m", common_types.mat4);
		class_add_func("_rotation_v", common_types.quaternion, &quaternion::rotation_v, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.vec3);
		class_add_func("_rotation_a", common_types.quaternion, &quaternion::rotation_a, Flags::Static | Flags::Pure);
			func_add_param("axis", common_types.vec3);
			func_add_param("angle", common_types.f32);
		class_add_func("_rotation_m", common_types.quaternion, &quaternion::rotation_m, Flags::Static | Flags::Pure);
			func_add_param("m", common_types.mat4);
		class_add_func("interpolate", common_types.quaternion, (quaternion(*)(const quaternion&, const quaternion&, float))&quaternion::interpolate, Flags::Static | Flags::Pure);
			func_add_param("q0", common_types.quaternion);
			func_add_param("q1", common_types.quaternion);
			func_add_param("t", common_types.f32);
		class_add_func("drag", common_types.quaternion, &quaternion::drag, Flags::Static | Flags::Pure);
			func_add_param("up", common_types.vec3);
			func_add_param("dang", common_types.vec3);
			func_add_param("reset_z", common_types._bool);
		class_add_const("ID", common_types.quaternion, &quaternion::ID);
		add_operator(OperatorID::Assign, common_types._void, common_types.quaternion, common_types.quaternion, InlineID::ChunkAssign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.quaternion, common_types.quaternion, InlineID::ChunkEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.quaternion, common_types.quaternion, InlineID::ChunkNotEqual);
		add_operator(OperatorID::Multiply, common_types.quaternion, common_types.quaternion, common_types.quaternion, InlineID::None, &quaternion::mul);
		add_operator(OperatorID::Multiply, common_types.vec3, common_types.quaternion, common_types.vec3, InlineID::None, &KabaQuaternion::mulv);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.quaternion, common_types.quaternion, InlineID::None, &quaternion::imul);

	add_class(common_types.rect);
		class_add_element("x1", common_types.f32, &rect::x1);
		class_add_element("x2", common_types.f32, &rect::x2);
		class_add_element("y1", common_types.f32, &rect::y1);
		class_add_element("y2", common_types.f32, &rect::y2);
		class_add_const("ID", common_types.rect, &rect::ID);
		class_add_const("ID_SYM", common_types.rect, &rect::ID_SYM);
		class_add_const("EMPTY", common_types.rect, &rect::EMPTY);
		class_add_func("width", common_types.f32, &rect::width, Flags::Pure);
		class_add_func("height", common_types.f32, &rect::height, Flags::Pure);
		class_add_func("area", common_types.f32, &rect::area, Flags::Pure);
		class_add_func("center", common_types.vec2, &rect::center, Flags::Pure);
		class_add_func("p00", common_types.vec2, &rect::p00, Flags::Pure);
		class_add_func("p11", common_types.vec2, &rect::p11, Flags::Pure);
		class_add_func("size", common_types.vec2, &rect::size, Flags::Pure);
		class_add_func("inside", common_types._bool, &rect::inside, Flags::Pure);
			func_add_param("p", common_types.vec2);
		class_add_func(Identifier::func::Str, common_types.string, &rect::str, Flags::Pure);
		class_add_func("_create", common_types.rect, &KabaRect::set, Flags::Static | Flags::Pure);
			func_set_inline(InlineID::RectSet);
			func_add_param("x1", common_types.f32);
			func_add_param("x2", common_types.f32);
			func_add_param("y1", common_types.f32);
			func_add_param("y2", common_types.f32);
		/*class_add_func("_create", common_types.rect, &KabaRect::set2, Flags::Static | Flags::Pure);
			func_add_param("p00", common_types.vec2);
			func_add_param("p11", common_types.vec2);*/
		class_add_func(Identifier::func::Init, common_types._void, &generic_init_ext<rect, float, float, float, float>, Flags::Mutable);
			func_add_param("x1", common_types.f32);
			func_add_param("x2", common_types.f32);
			func_add_param("y1", common_types.f32);
			func_add_param("y2", common_types.f32);
		/*class_add_func(Identifier::func::Init, common_types._void, &KabaRect::init2, Flags::Mutable);
			func_add_param("p00", common_types.vec2);
			func_add_param("p11", common_types.vec2);*/
		add_operator(OperatorID::Assign, common_types._void, common_types.rect, common_types.rect, InlineID::ChunkAssign, &generic_assign<rect>);
		add_operator(OperatorID::Equal, common_types._bool, common_types.rect, common_types.rect, InlineID::ChunkEqual, &rect::operator==);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.rect, common_types.rect, InlineID::ChunkNotEqual, &rect::operator!=);


	add_class(TypeBox);
		class_add_element("min", common_types.vec3, &Box::min);
		class_add_element("max", common_types.vec3, &Box::max);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init<Box>, Flags::Mutable);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init_ext<KabaBox, const vec3&, const vec3&>, Flags::Mutable);
			func_add_param("min", common_types.vec3);
			func_add_param("max", common_types.vec3);
		class_add_func("size", common_types.vec3, &Box::size, Flags::Pure);
			class_add_func("center", common_types.vec3, &Box::center, Flags::Pure);
		class_add_func("is_inside", common_types._bool, &Box::is_inside, Flags::Pure);
			func_add_param("p", common_types.vec3);
		class_add_func("to_relative", common_types.vec3, &Box::to_relative, Flags::Pure);
			func_add_param("p", common_types.vec3);
		class_add_func("to_absolute", common_types.vec3, &Box::to_absolute, Flags::Pure);
			func_add_param("p", common_types.vec3);
		class_add_func(Identifier::func::Str, common_types.string, &Box::str, Flags::Pure);
		class_add_const("ID",  TypeBox, &Box::ID);
		class_add_const("ID_SYM",  TypeBox, &Box::ID_SYM);
		add_operator(OperatorID::Assign, common_types._void, TypeBox, TypeBox, InlineID::ChunkAssign, &generic_assign<Box>);


	add_class(common_types.color);
		class_add_element("r", common_types.f32, &color::r);
		class_add_element("g", common_types.f32, &color::g);
		class_add_element("b", common_types.f32, &color::b);
		class_add_element("a", common_types.f32, &color::a);
		class_add_func(Identifier::func::Str, common_types.string, &color::str, Flags::Pure);
		class_add_func("hex", common_types.string, &color::hex, Flags::Pure);
		class_add_func("with_alpha", common_types.color, &color::with_alpha, Flags::Pure);
			func_add_param("a", common_types.f32);
		class_add_func("hsb", common_types.color, &color::from_hsb, Flags::Static | Flags::Pure);
			func_add_param("h", common_types.f32);
			func_add_param("s", common_types.f32);
			func_add_param("b", common_types.f32);
			func_add_param_def("a", common_types.f32, 1.0f);
		class_add_func("mix", common_types.color, &color::mix, Flags::Static | Flags::Pure);
			func_add_param("c1", common_types.color);
			func_add_param("c2", common_types.color);
			func_add_param("t", common_types.f32);
		class_add_func("_create", common_types.color, &color::from_rgba, Flags::Static | Flags::Pure);
			func_set_inline(InlineID::ColorSet);
			func_add_param("r", common_types.f32);
			func_add_param("g", common_types.f32);
			func_add_param("b", common_types.f32);
			func_add_param("a", common_types.f32);
		class_add_func(Identifier::func::Init, common_types._void, &KabaColor::init, Flags::Mutable);
			func_add_param("r", common_types.f32);
			func_add_param("g", common_types.f32);
			func_add_param("b", common_types.f32);
			func_add_param_def("a", common_types.f32, 1.0f);
		add_operator(OperatorID::Assign, common_types._void, common_types.color, common_types.color, InlineID::ChunkAssign, &KabaColor::assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.color, common_types.color, InlineID::ChunkEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.color, common_types.color, InlineID::ChunkNotEqual);
		add_operator(OperatorID::Add, common_types.color, common_types.color, common_types.color, InlineID::None, &color::operator+);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.color, common_types.color, InlineID::None, &color::operator+=);
		add_operator(OperatorID::Subtract, common_types.color, common_types.color, common_types.color, InlineID::None, &color::operator-);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.color, common_types.color, InlineID::None, &color::operator-=);
		add_operator(OperatorID::Multiply, common_types.color, common_types.color, common_types.f32, InlineID::None, &KabaColor::mul_f);
		add_operator(OperatorID::Multiply, common_types.color, common_types.color, common_types.color, InlineID::None, &KabaColor::mul_c);
		// color
		class_add_const("WHITE",  common_types.color, &White);
		class_add_const("BLACK",  common_types.color, &Black);
		class_add_const("GRAY",   common_types.color, &Gray);
		class_add_const("RED",    common_types.color, &Red);
		class_add_const("GREEN",  common_types.color, &Green);
		class_add_const("BLUE",   common_types.color, &Blue);
		class_add_const("YELLOW", common_types.color, &Yellow);
		class_add_const("ORANGE", common_types.color, &Orange);
		class_add_const("PURPLE", common_types.color, &Purple);


	add_class(common_types.plane);
		class_add_element("_a", common_types.f32, 0);
		class_add_element("_b", common_types.f32, 4);
		class_add_element("_c", common_types.f32, 8);
		class_add_element("d", common_types.f32, &plane::d);
		class_add_element("n", common_types.vec3, &plane::n);
		class_add_func("intersect_line", common_types._bool, &plane::intersect_line, Flags::Pure);
			func_add_param("l1", common_types.vec3);
			func_add_param("l2", common_types.vec3);
			func_add_param("inter", common_types.vec3);
		class_add_func("inverse", common_types.plane, &plane::inverse, Flags::Pure);
		class_add_func("distance", common_types.f32, &plane::distance, Flags::Pure);
			func_add_param("p", common_types.vec3);
		class_add_func(Identifier::func::Str, common_types.string, &plane::str, Flags::Pure);
		class_add_func("transform", common_types.plane, &plane::transform, Flags::Pure);
			func_add_param("m", common_types.mat4);
		class_add_func("from_points", common_types.plane, &plane::from_points, Flags::Static | Flags::Pure);
			func_add_param("a", common_types.vec3);
			func_add_param("b", common_types.vec3);
			func_add_param("c", common_types.vec3);
		class_add_func("from_point_normal", common_types.plane, &plane::from_point_normal, Flags::Static | Flags::Pure);
			func_add_param("p", common_types.vec3);
			func_add_param("n", common_types.vec3);
		add_operator(OperatorID::Assign, common_types._void, common_types.plane, common_types.plane, InlineID::ChunkAssign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.plane, common_types.plane, InlineID::ChunkEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.plane, common_types.plane, InlineID::ChunkNotEqual);


	add_class(TypeRay);
		class_add_element("u", common_types.vec3, &Ray::u);
		class_add_element("v", common_types.vec3, &Ray::v);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init<Ray>, Flags::Mutable);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init_ext<Ray, const vec3&, const vec3&>, Flags::Mutable);
			func_add_param("a", common_types.vec3);
			func_add_param("b", common_types.vec3);
		class_add_func("dot", common_types.f32, &Ray::dot, Flags::Static | Flags::Pure);
			func_add_param("r1", TypeRay);
			func_add_param("r2", TypeRay);
		class_add_func("intersect_plane", TypeVec3Optional, &Ray::intersect_plane, Flags::Pure);
			func_add_param("pl", common_types.plane);
		add_operator(OperatorID::Assign, common_types._void, TypeRay, TypeRay, InlineID::ChunkAssign);
		add_operator(OperatorID::Equal, common_types._bool, TypeRay, TypeRay, InlineID::ChunkEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, TypeRay, TypeRay, InlineID::ChunkNotEqual);


	add_class(common_types.mat4);
		class_add_element("_00", common_types.f32, 0);
		class_add_element("_10", common_types.f32, 4);
		class_add_element("_20", common_types.f32, 8);
		class_add_element("_30", common_types.f32, 12);
		class_add_element("_01", common_types.f32, 16);
		class_add_element("_11", common_types.f32, 20);
		class_add_element("_21", common_types.f32, 24);
		class_add_element("_31", common_types.f32, 28);
		class_add_element("_02", common_types.f32, 32);
		class_add_element("_12", common_types.f32, 36);
		class_add_element("_22", common_types.f32, 40);
		class_add_element("_32", common_types.f32, 44);
		class_add_element("_03", common_types.f32, 48);
		class_add_element("_13", common_types.f32, 52);
		class_add_element("_23", common_types.f32, 56);
		class_add_element("_33", common_types.f32, 60);
		class_add_element("e", TypeFloatArray4x4, 0);
		class_add_element("_e", TypeFloatArray16, 0);
		class_add_const("ID", common_types.mat4, &mat4::ID);
		class_add_func(Identifier::func::Str, common_types.string, &mat4::str, Flags::Pure);
		class_add_func("transform", common_types.vec3, &mat4::transform, Flags::Pure);
			func_add_param("v", common_types.vec3);
		class_add_func("transform_normal", common_types.vec3, &mat4::transform_normal, Flags::Pure);
			func_add_param("v", common_types.vec3);
		class_add_func("untransform", common_types.vec3, &mat4::untransform, Flags::Pure);
			func_add_param("v", common_types.vec3);
		class_add_func("project", common_types.vec3, &mat4::project, Flags::Pure);
			func_add_param("v", common_types.vec3);
		class_add_func("unproject", common_types.vec3, &mat4::unproject, Flags::Pure);
			func_add_param("v", common_types.vec3);
		class_add_func("inverse", common_types.mat4, &mat4::inverse, Flags::Pure);
		class_add_func("transpose", common_types.mat4, &mat4::transpose, Flags::Pure);
		class_add_func("translation", common_types.mat4, &mat4::translation, Flags::Static | Flags::Pure);
			func_add_param("trans", common_types.vec3);
		class_add_func("rotation", common_types.mat4, &KabaMatrix<mat4>::rotation_v, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.vec3);
		class_add_func("rotation_x", common_types.mat4, &mat4::rotation_x, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.f32);
		class_add_func("rotation_y", common_types.mat4, &mat4::rotation_y, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.f32);
		class_add_func("rotation_z", common_types.mat4, &mat4::rotation_z, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.f32);
		class_add_func("rotation", common_types.mat4, &KabaMatrix<mat4>::rotation_q, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.quaternion);
		class_add_func("scale", common_types.mat4, &KabaMatrix<mat4>::scale_f, Flags::Static | Flags::Pure);
			func_add_param("s_x", common_types.f32);
			func_add_param("s_y", common_types.f32);
			func_add_param("s_z", common_types.f32);
		class_add_func("scale", common_types.mat4, &KabaMatrix<mat4>::scale_v, Flags::Static | Flags::Pure);
			func_add_param("s", common_types.vec3);
		class_add_func("perspective", common_types.mat4, &mat4::perspective, Flags::Static | Flags::Pure);
			func_add_param("fovy", common_types.f32);
			func_add_param("aspect", common_types.f32);
			func_add_param("z_near", common_types.f32);
			func_add_param("z_far", common_types.f32);
			func_add_param("z_sym", common_types._bool);
		add_operator(OperatorID::Assign, common_types._void, common_types.mat4, common_types.mat4, InlineID::ChunkAssign, &KabaVector<mat4>::assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.mat4, common_types.mat4, InlineID::ChunkEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.mat4, common_types.mat4, InlineID::ChunkNotEqual);
		add_operator(OperatorID::Multiply, common_types.mat4, common_types.mat4, common_types.mat4, InlineID::None, &KabaMatrix<mat4>::mul);
		add_operator(OperatorID::Multiply, common_types.vec3, common_types.mat4, common_types.vec3, InlineID::None, &KabaMatrix<mat4>::mul_v<vec3>);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.mat4, common_types.mat4, InlineID::None, &KabaMatrix<mat4>::imul);

	add_class(common_types.mat3);
		class_add_element("_11", common_types.f32, 0);
		class_add_element("_21", common_types.f32, 4);
		class_add_element("_31", common_types.f32, 8);
		class_add_element("_12", common_types.f32, 12);
		class_add_element("_22", common_types.f32, 16);
		class_add_element("_32", common_types.f32, 20);
		class_add_element("_13", common_types.f32, 24);
		class_add_element("_23", common_types.f32, 28);
		class_add_element("_33", common_types.f32, 32);
		class_add_element("e", TypeFloatArray3x3, 0);
		class_add_element("_e", TypeFloatArray9, 0);
		class_add_const("ID", common_types.mat3, &mat3::ID);
		class_add_const("0", common_types.mat3, &mat3::ZERO);
		class_add_func(Identifier::func::Str, common_types.string, &mat3::str, Flags::Pure);
		class_add_func("inverse", common_types.mat3, &mat3::inverse, Flags::Pure);
		class_add_func("rotation", common_types.mat3, &KabaMatrix<mat3>::rotation_v, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.vec3);
		class_add_func("rotation", common_types.mat3, &KabaMatrix<mat3>::rotation_q, Flags::Static | Flags::Pure);
			func_add_param("ang", common_types.quaternion);
		class_add_func("scale", common_types.mat3, &KabaMatrix<mat3>::scale_f, Flags::Static | Flags::Pure);
			func_add_param("s_x", common_types.f32);
			func_add_param("s_y", common_types.f32);
			func_add_param("s_z", common_types.f32);
		class_add_func("scale", common_types.mat3, &KabaMatrix<mat3>::scale_v, Flags::Static | Flags::Pure);
			func_add_param("s", common_types.vec3);
		add_operator(OperatorID::Assign, common_types._void, common_types.mat3, common_types.mat3, InlineID::ChunkAssign, &KabaVector<mat3>::assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.mat3, common_types.mat3, InlineID::ChunkEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.mat3, common_types.mat3, InlineID::ChunkNotEqual);
		add_operator(OperatorID::Multiply, common_types.mat3, common_types.mat3, common_types.mat3, InlineID::None, &KabaMatrix<mat3>::mul);
		add_operator(OperatorID::Multiply, common_types.vec3, common_types.mat3, common_types.vec3, InlineID::None, &KabaMatrix<mat3>::mul_v<vec3>);


	add_class(common_types.any);
		class_add_element("data", common_types.pointer, &Any::data);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init<Any>, Flags::Mutable);
		class_add_func(Identifier::func::Delete, common_types._void, &generic_delete<Any>, Flags::Mutable);
		class_add_func(Identifier::func::Assign, common_types._void, &KabaAny::set, Flags::Mutable);
			func_add_param("a", common_types.any);
		class_add_func("type", common_types.class_ref, &KabaAny::_get_class);
		class_add_func("clear", common_types._void, &Any::clear, Flags::Mutable);
		class_add_func(Identifier::func::Length, common_types.i32, &Any::length, Flags::Pure);
		class_add_func(Identifier::func::Get, TypeAnyP, &Any::dict_get, Flags::Ref);
			func_add_param("key", common_types.string);
		class_add_func(Identifier::func::Set, common_types._void, &Any::dict_set, Flags::Mutable);
			func_add_param("key", common_types.string);
			func_add_param("value", common_types.any);
		class_add_func(Identifier::func::Get, TypeAnyP, &Any::list_get, Flags::Ref);
			func_add_param("index", common_types.i32);
		class_add_func(Identifier::func::Set, common_types._void, &Any::list_set, Flags::Mutable);
			func_add_param("index", common_types.i32);
			func_add_param("value", common_types.any);
		class_add_func("is_empty", common_types._bool, &Any::is_empty, Flags::Pure);
		class_add_func("has", common_types._bool, &Any::has, Flags::Pure);
			func_add_param("key", common_types.string);
		class_add_func("add", common_types._void, &KabaAny::add, Flags::Mutable);
			func_add_param("a", common_types.any);
		class_add_func("drop", common_types._void, &Any::dict_drop, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("key", common_types.string);
		class_add_func("keys", common_types.string_list, &Any::keys, Flags::Pure);//, Flags::RAISES_EXCEPTIONS);
		class_add_func("__bool__", common_types._bool, &Any::to_bool, Flags::Pure);
		class_add_func("__i32__", common_types.i32, &Any::to_i32, Flags::Pure);
		class_add_func("__i64__", common_types.i32, &Any::to_i64, Flags::Pure);
		class_add_func("__f32__", common_types.f32, &Any::to_f32, Flags::Pure);
		class_add_func("__f64__", common_types.f64, &Any::to_f64, Flags::Pure);
		class_add_func(Identifier::func::Str, common_types.string, &Any::str, Flags::Pure);
		class_add_func(Identifier::func::Repr, common_types.string, &Any::repr, Flags::Pure);
		class_add_func("unwrap", common_types._void, &unwrap_any);//, Flags::RaisesExceptions);
			func_add_param("var", common_types.reference);
			func_add_param("type", common_types.class_ref);
		class_add_func("parse", common_types.any, &KabaAny::parse, Flags::Static | Flags::RaisesExceptions);
			func_add_param("s", common_types.string);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.any, common_types.any, InlineID::None, &KabaAny::_add);// operator+=);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.any, common_types.any, InlineID::None, &KabaAny::_sub);// operator-);


	add_func("@int2any", common_types.any, &int2any, Flags::Static);
		func_add_param("i", common_types.i32);
	add_func("@float2any", common_types.any, &float2any, Flags::Static);
		func_add_param("i", common_types.f32);
	add_func("@bool2any", common_types.any, &bool2any, Flags::Static);
		func_add_param("i", common_types._bool);
	add_func("@str2any", common_types.any, &str2any, Flags::Static);
		func_add_param("s", common_types.string);
	add_func("@pointer2any", common_types.any, &pointer2any, Flags::Static);
		func_add_param("p", common_types.pointer);


	add_class(TypeRandom);
		class_add_func(Identifier::func::Init, common_types._void, &generic_init<Random>, Flags::Mutable);
		class_add_func(Identifier::func::Assign, common_types._void, &Random::__assign__, Flags::Mutable);
			func_add_param("o", TypeRandom);
		//class_add_element("n", TypeRandom, 0);
		class_add_func("seed", common_types._void, &Random::seed, Flags::Mutable);
			func_add_param("str", common_types.string);
		class_add_func("int", common_types.i32, &Random::_int, Flags::Mutable);
			func_add_param("max", common_types.i32);
		class_add_func("uniform01", common_types.f32, &Random::uniform01, Flags::Mutable);
		class_add_func("uniform", common_types.f32, &Random::uniform, Flags::Mutable);
			func_add_param("min", common_types.f32);
			func_add_param("max", common_types.f32);
		class_add_func("normal", common_types.f32, &Random::normal, Flags::Mutable);
			func_add_param("mean", common_types.f32);
			func_add_param("stddev", common_types.f32);
		class_add_func("in_ball", common_types.vec3, &Random::in_ball, Flags::Mutable);
			func_add_param("r", common_types.f32);
		class_add_func("dir", common_types.vec3, &Random::dir, Flags::Mutable);


	add_class(TypeFloatInterpolator);
		class_add_element("type", common_types.i32, 0);
		class_add_func(Identifier::func::Init, common_types._void, &Interpolator<float>::__init__, Flags::Mutable);
		class_add_func("clear", common_types._void, &Interpolator<float>::clear, Flags::Mutable);
		class_add_func("set_type", common_types._void, &Interpolator<float>::setType, Flags::Mutable);
			func_add_param("type", common_types.string);
		class_add_func("add", common_types._void, &Interpolator<float>::addv, Flags::Mutable);
			func_add_param("p", common_types.f32);
			func_add_param("dt", common_types.f32);
		class_add_func("add2", common_types._void, &Interpolator<float>::add2v, Flags::Mutable);
			func_add_param("p", common_types.f32);
			func_add_param("v", common_types.f32);
			func_add_param("dt", common_types.f32);
		class_add_func("add3", common_types._void, &Interpolator<float>::add3v, Flags::Mutable);
			func_add_param("p", common_types.f32);
			func_add_param("v", common_types.f32);
			func_add_param("w", common_types.f32);
			func_add_param("dt", common_types.f32);
		class_add_func("jump", common_types._void, &Interpolator<float>::jumpv, Flags::Mutable);
			func_add_param("p", common_types.f32);
			func_add_param("v", common_types.f32);
		class_add_func("normalize", common_types._void, &Interpolator<float>::normalize, Flags::Mutable);
		class_add_func("get", common_types.f32, &Interpolator<float>::get, Flags::Pure);
			func_add_param("t", common_types.f32);
		class_add_func("get_derivative", common_types.f32, &Interpolator<float>::get_derivative, Flags::Pure);
			func_add_param("t", common_types.f32);
		class_add_func("get_list", common_types.f32_list, &Interpolator<float>::get_list, Flags::Pure);
			func_add_param("t", common_types.f32_list);


	add_class(TypeVectorInterpolator);
		class_add_element("type", common_types.i32, 0);
		class_add_func(Identifier::func::Init, common_types._void, &Interpolator<vec3>::__init__, Flags::Mutable);
		class_add_func("clear", common_types._void, &Interpolator<vec3>::clear, Flags::Mutable);
		class_add_func("set_type", common_types._void, &Interpolator<vec3>::setType, Flags::Mutable);
			func_add_param("type", common_types.string);
		class_add_func("add", common_types._void, &Interpolator<vec3>::add, Flags::Mutable);
			func_add_param("p", common_types.vec3);
			func_add_param("dt", common_types.f32);
		class_add_func("add2", common_types._void, &Interpolator<vec3>::add2, Flags::Mutable);
			func_add_param("p", common_types.vec3);
			func_add_param("v", common_types.vec3);
			func_add_param("dt", common_types.f32);
		class_add_func("add3", common_types._void, &Interpolator<vec3>::add3, Flags::Mutable);
			func_add_param("p", common_types.vec3);
			func_add_param("v", common_types.vec3);
			func_add_param("w", common_types.f32);
			func_add_param("dt", common_types.f32);
		class_add_func("jump", common_types._void, &Interpolator<vec3>::jump, Flags::Mutable);
			func_add_param("p", common_types.vec3);
			func_add_param("v", common_types.vec3);
		class_add_func("normalize", common_types._void, &Interpolator<vec3>::normalize, Flags::Mutable);
		class_add_func("get", common_types.vec3, &Interpolator<vec3>::get, Flags::Pure);
			func_add_param("t", common_types.f32);
		class_add_func("get_tang", common_types.vec3, &Interpolator<vec3>::get_derivative, Flags::Pure);
			func_add_param("t", common_types.f32);
		class_add_func("get_list", common_types.vec3_list, &Interpolator<vec3>::get_list, Flags::Pure);
			func_add_param("t", common_types.f32_list);


	// i32
	add_func("clamp", common_types.i32, &clamp<int>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i32);
		func_add_param("min", common_types.i32);
		func_add_param("max", common_types.i32);
	add_func("loop", common_types.i32, &loop<int>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i32);
		func_add_param("min", common_types.i32);
		func_add_param("max", common_types.i32);
	add_func("abs", common_types.i32, &abs<int>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i32);
	add_func("sign", common_types.i32, &sign<int>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i32);
	add_func("min", common_types.i32, &min<int>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.i32);
		func_add_param("b", common_types.i32);
	add_func("max", common_types.i32, &max<int>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.i32);
		func_add_param("b", common_types.i32);
	add_func("xor", common_types.i32, &kaba_xor<int>, Flags::Static | Flags::Pure);
		func_set_inline(InlineID::Int32BitXOr);
		func_add_param("a", common_types.i32);
		func_add_param("b", common_types.i32);

	// i64
	add_func("clamp", common_types.i64, &clamp<int64>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i64);
		func_add_param("min", common_types.i64);
		func_add_param("max", common_types.i64);
	add_func("abs", common_types.i64, &abs<int64>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i64);
	add_func("sign", common_types.i64, &sign<int64>, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i64);
	add_func("min", common_types.i64, &min<int64>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.i64);
		func_add_param("b", common_types.i64);
	add_func("max", common_types.i64, &max<int64>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.i64);
		func_add_param("b", common_types.i64);
	add_func("xor", common_types.i64, &kaba_xor<int64>, Flags::Static | Flags::Pure);
		func_set_inline(InlineID::Int64BitXOr);
		func_add_param("a", common_types.i64);
		func_add_param("b", common_types.i64);

	// f32
	add_func("sin", common_types.f32, &sinf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("cos", common_types.f32, &cosf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("tan", common_types.f32, &tanf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("asin", common_types.f32, &asinf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("acos", common_types.f32, &acosf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("atan", common_types.f32, &atanf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("atan2", common_types.f32, &atan2f, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
		func_add_param("y", common_types.f32);
	add_func("sqrt", common_types.f32, &sqrtf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("sqr", common_types.f32, &sqr<float>, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("exp", common_types.f32, &expf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("log", common_types.f32, &logf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
	add_func("pow", common_types.f32, &powf, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f32);
		func_add_param("exp", common_types.f32);
	add_func("clamp", common_types.f32, &clamp<float>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f32);
		func_add_param("min", common_types.f32);
		func_add_param("max", common_types.f32);
	add_func("loop", common_types.f32, &loop<float>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f32);
		func_add_param("min", common_types.f32);
		func_add_param("max", common_types.f32);
	add_func("abs", common_types.f32, &abs<float>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f32);
	add_func("sign", common_types.f32, &sign<float>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f32);
	add_func("min", common_types.f32, &min<float>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.f32);
		func_add_param("b", common_types.f32);
	add_func("max", common_types.f32, &max<float>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.f32);
		func_add_param("b", common_types.f32);

	// f64
	add_func("sin", common_types.f64, (double(*)(double))&sin, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("cos", common_types.f64, (double(*)(double))&cos, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("tan", common_types.f64, (double(*)(double))&tan, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("asin", common_types.f64, (double(*)(double))&asin, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("acos", common_types.f64, (double(*)(double))&acos, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("atan", common_types.f64, (double(*)(double))&atan, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("atan2", common_types.f64, (double(*)(double,double))&atan2, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
		func_add_param("y", common_types.f64);
	add_func("sqrt", common_types.f64, (double(*)(double))&sqrt, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("sqr", common_types.f64, &sqr<double>, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("exp", common_types.f64, (double(*)(double))&exp, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("log", common_types.f64, (double(*)(double))&log, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
	add_func("pow", common_types.f64, (double(*)(double,double))&pow, Flags::Static | Flags::Pure);
		func_add_param("x", common_types.f64);
		func_add_param("exp", common_types.f64);
	add_func("clamp", common_types.f64, &clamp<double>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f64);
		func_add_param("min", common_types.f64);
		func_add_param("max", common_types.f64);
	add_func("abs", common_types.f64, &abs<double>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f64);
	add_func("sign", common_types.f64, &sign<double>, Flags::Static | Flags::Pure);
		func_add_param("f", common_types.f64);
	add_func("min", common_types.f64, &min<double>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.f64);
		func_add_param("b", common_types.f64);
	add_func("max", common_types.f64, &max<double>, Flags::Static | Flags::Pure);
		func_add_param("a", common_types.f64);
		func_add_param("b", common_types.f64);

	// complex
	add_func("abs", common_types.f32, &KabaVector<complex>::abs, Flags::Static | Flags::Pure);
		func_add_param("z", common_types.complex);

	// i32[]
	add_func("sum", common_types.i32, &XList<int>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("sum_sqr", common_types.i32, &XList<int>::sum_sqr, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("min", common_types.i32, &XList<int>::min, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("max", common_types.i32, &XList<int>::max, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("argmin", common_types.i32, &XList<int>::argmin, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("argmax", common_types.i32, &XList<int>::argmax, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("unique", common_types.i32_list, &XList<int>::unique, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.i32_list);
	add_func("range", common_types.i32_list, (void*)&kaba_range<int>, Flags::Static | Flags::Pure);
		func_add_param("start", common_types.i32);
		func_add_param_def("end", common_types.i32, DynamicArray::MAGIC_END_INDEX);
		func_add_param_def("step", common_types.i32, 1);

	// f32[]
	add_func("sum", common_types.f32, &XList<float>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("sum_sqr", common_types.f32, &XList<float>::sum_sqr, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("min", common_types.f32, &XList<float>::min, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("max", common_types.f32, &XList<float>::max, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("argmin", common_types.i32, &XList<float>::argmin, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("argmax", common_types.i32, &XList<float>::argmax, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("unique", common_types.f32_list, &XList<float>::unique, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f32_list);
	add_func("range", common_types.f32_list, (void*)&kaba_range<float>, Flags::Static | Flags::Pure);
		func_add_param("start", common_types.f32);
		func_add_param_def("end", common_types.f32, (float)DynamicArray::MAGIC_END_INDEX);
		func_add_param_def("step", common_types.f32, 1.0f);
	add_func("cubic_spline", common_types.f32, &cubic_spline<float>, Flags::Static | Flags::Pure);
		func_add_param("points", common_types.f32_list);
		func_add_param("t", common_types.f32);
	add_func("cubic_spline_d", common_types.f32, &cubic_spline_d<float>, Flags::Static | Flags::Pure);
		func_add_param("points", common_types.f32_list);
		func_add_param("t", common_types.f32);

	// float64[]
	add_func("sum", common_types.f64, &XList<double>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f64_list);
	add_func("sum_sqr", common_types.f64, &XList<double>::sum_sqr, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f64_list);
	add_func("min", common_types.f64, &XList<double>::min, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f64_list);
	add_func("max", common_types.f64, &XList<double>::max, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f64_list);
	add_func("argmin", common_types.i32, &XList<double>::argmin, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f64_list);
	add_func("argmax", common_types.i32, &XList<double>::argmax, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.f64_list);

	// vec2[]
	add_func("sum", common_types.vec2, &VectorList<vec2>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.vec2_list);
	add_func("sum_sqr", common_types.f32, &VectorList<vec2>::sum_sqr, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.vec2_list);
	add_func("cubic_spline", common_types.vec2, &cubic_spline<vec2>, Flags::Static | Flags::Pure);
		func_add_param("points", common_types.vec2_list);
		func_add_param("t", common_types.f32);
	add_func("cubic_spline_d", common_types.vec2, &cubic_spline_d<vec2>, Flags::Static | Flags::Pure);
		func_add_param("points", common_types.vec2_list);
		func_add_param("t", common_types.f32);

	// vec3[]
	add_func("sum", common_types.vec3, &VectorList<vec3>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.vec3_list);
	add_func("sum_sqr", common_types.f32, &VectorList<vec3>::sum_sqr, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.vec3_list);
	add_func("cubic_spline", common_types.vec3, &cubic_spline<vec3>, Flags::Static | Flags::Pure);
		func_add_param("points", common_types.vec3_list);
		func_add_param("t", common_types.f32);
	add_func("cubic_spline_d", common_types.vec3, &cubic_spline_d<vec3>, Flags::Static | Flags::Pure);
		func_add_param("points", common_types.vec3_list);
		func_add_param("t", common_types.f32);

	// complex[]
	add_func("sum", common_types.complex, &VectorList<complex>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.complex_list);
	add_func("sum_sqr", common_types.f32, &VectorList<complex>::sum_sqr, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.complex_list);

	// string[]
	add_func("sum", common_types.string, &XList<string>::sum, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.string_list);
	add_func("unique", common_types.string_list, &XList<string>::unique, Flags::Static | Flags::Pure);
		func_add_param("list", common_types.string_list);

	// other types
	add_func("bary_centric", common_types.vec2, (void*)&bary_centric, Flags::Static | Flags::Pure);
		func_add_param("p", common_types.vec3);
		func_add_param("a", common_types.vec3);
		func_add_param("b", common_types.vec3);
		func_add_param("c", common_types.vec3);

	// random numbers
	add_func("rand", common_types.i32, &randi, Flags::Static);
		func_add_param("max", common_types.i32);
	add_func("rand", common_types.f32, &randf, Flags::Static);
		func_add_param("max", common_types.f32);
	add_func("rand_seed", common_types._void, &srand, Flags::Static);
		func_add_param("seed", common_types.i32);

	add_ext_var("_any_allow_simple_output", common_types._bool, (void*)&Any::allow_simple_output);
	
	// float
	add_const("pi",  common_types.f32, &pi);


	// needs to be defined after any
	common_types.any_list = add_type_list(common_types.any);
	lib_create_list<Any>(common_types.any_list);
	auto TypeAnyListP = add_type_p_raw(common_types.any_list);

	common_types.any_dict = add_type_dict(common_types.any);
	lib_create_dict<Any>(common_types.any_dict, TypeAnyP);
	auto TypeAnyDictP = add_type_p_raw(common_types.any_dict);


	add_class(common_types.any);
		class_add_func("as_list", TypeAnyListP, &KabaAny::_as_list, Flags::Ref);
		class_add_func("as_dict", TypeAnyDictP, &KabaAny::_as_dict, Flags::Ref);


	add_type_cast(50, common_types.i32, common_types.any, "math.@int2any");
	add_type_cast(50, common_types.f32, common_types.any, "math.@float2any");
	add_type_cast(50, common_types._bool, common_types.any, "math.@bool2any");
	add_type_cast(50, common_types.string, common_types.any, "math.@str2any");
	add_type_cast(50, common_types.pointer, common_types.any, "math.@pointer2any");
}

};
