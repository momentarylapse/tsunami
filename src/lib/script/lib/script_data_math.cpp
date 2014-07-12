#include <algorithm>
#include "../../file/file.h"
#include "../../math/math.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

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

namespace Script{

#ifdef _X_USE_ALGEBRA_
	#define algebra_p(p)		(void*)p
#else
	#define algebra_p(p)		NULL
#endif

#ifdef _X_USE_ANY_
	#define any_p(p)		(void*)p
#else
	#define any_p(p)		NULL
#endif

// we're always using math types
#define type_p(p)			(void*)p

extern Type *TypeComplexList;
extern Type *TypeFloatList;
extern Type *TypeVectorList;
extern Type *TypeMatrix;
extern Type *TypePlane;
extern Type *TypePlaneList;
extern Type *TypeMatrix3;
extern Type *TypeIntList;
extern Type *TypeFloatPs;


float _cdecl f_sqr(float f){	return f*f;	}


#define IMPLEMENT_IOP(OP, TYPE) \
{ \
	int n = min(num, b.num); \
	TYPE *pa = (TYPE*)data; \
	TYPE *pb = (TYPE*)b.data; \
	for (int i=0;i<n;i++) \
		*(pa ++) OP *(pb ++); \
}

#define IMPLEMENT_IOP2(OP, TYPE) \
{ \
	TYPE *pa = (TYPE*)data; \
	for (int i=0;i<num;i++) \
		*(pa ++) OP x; \
}

#define IMPLEMENT_OP(OP, TYPE, LISTTYPE) \
{ \
	int n = min(num, b.num); \
	LISTTYPE r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)data; \
	TYPE *pb = (TYPE*)b.data; \
	TYPE *pr = (TYPE*)r.data; \
	for (int i=0;i<n;i++) \
		*(pr ++) = *(pa ++) OP *(pb ++); \
	return r; \
}

class IntList : public Array<int>
{
public:
	int _cdecl sum()
	{
		int r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	void _cdecl sort()
	{	std::sort((int*)data, (int*)data + num);	}
	void _cdecl unique()
	{
		int ndiff = 0;
		int i0 = 1;
		while(((int*)data)[i0] != ((int*)data)[i0-1])
			i0 ++;
		for (int i=i0;i<num;i++){
			if (((int*)data)[i] == ((int*)data)[i-1])
				ndiff ++;
			else
				((int*)data)[i - ndiff] = ((int*)data)[i];
		}
		resize(num - ndiff);
	}
	
	// a += b
	void _cdecl iadd(IntList &b)	IMPLEMENT_IOP(+=, int)
	void _cdecl isub(IntList &b)	IMPLEMENT_IOP(-=, int)
	void _cdecl imul(IntList &b)	IMPLEMENT_IOP(*=, int)
	void _cdecl idiv(IntList &b)	IMPLEMENT_IOP(/=, int)

	// a = b + c
	IntList _cdecl add(IntList &b)	IMPLEMENT_OP(+, int, IntList)
	IntList _cdecl sub(IntList &b)	IMPLEMENT_OP(-, int, IntList)
	IntList _cdecl mul(IntList &b)	IMPLEMENT_OP(*, int, IntList)
	IntList _cdecl div(IntList &b)	IMPLEMENT_OP(/, int, IntList)

	// a += x
	void _cdecl add2(int x)	IMPLEMENT_IOP2(+=, int)
	void _cdecl sub2(int x)	IMPLEMENT_IOP2(-=, int)
	void _cdecl mul2(int x)	IMPLEMENT_IOP2(*=, int)
	void _cdecl div2(int x)	IMPLEMENT_IOP2(/=, int)
	void _cdecl assign_int(int x)	IMPLEMENT_IOP2(=, int)
};




void _cdecl super_array_add_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) += *(pb ++);	}
void _cdecl super_array_sub_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) -= *(pb ++);	}
void _cdecl super_array_mul_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) *= *(pb ++);	}
void _cdecl super_array_div_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) /= *(pb ++);	}

void _cdecl super_array_add_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) += x;	}
void _cdecl super_array_sub_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) -= x;	}
void _cdecl super_array_mul_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}
void _cdecl super_array_div_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) /= x;	}
void _cdecl super_array_mul_s_com_float(DynamicArray *a, float x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}

class FloatList : public Array<float>
{
public:
	float _cdecl _max()
	{
		float max = 0;
		if (num > 0)
			max = (*this)[0];
		for (int i=1;i<num;i++)
			if ((*this)[i] > max)
				max = (*this)[i];
		return max;
	}
	float _cdecl _min()
	{
		float min = 0;
		if (num > 0)
			min = (*this)[0];
		for (int i=1;i<num;i++)
			if ((*this)[i] < min)
				min = (*this)[i];
		return min;
	}
	float _cdecl sum()
	{
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	float _cdecl sum2()
	{
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
	FloatList _cdecl add(FloatList &b)	IMPLEMENT_OP(+, float, FloatList)
	FloatList _cdecl sub(FloatList &b)	IMPLEMENT_OP(-, float, FloatList)
	FloatList _cdecl mul(FloatList &b)	IMPLEMENT_OP(*, float, FloatList)
	FloatList _cdecl div(FloatList &b)	IMPLEMENT_OP(/, float, FloatList)

	// a += x
	void _cdecl iadd2(float x)	IMPLEMENT_IOP2(+=, float)
	void _cdecl isub2(float x)	IMPLEMENT_IOP2(-=, float)
	void _cdecl imul2(float x)	IMPLEMENT_IOP2(*=, float)
	void _cdecl idiv2(float x)	IMPLEMENT_IOP2(/=, float)
	void _cdecl assign_float(float x)	IMPLEMENT_IOP2(=, float)
};

class ComplexList : public Array<complex>
{
public:
	complex _cdecl sum()
	{
		complex r = complex(0, 0);
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	float _cdecl sum2()
	{
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
	ComplexList _cdecl add(ComplexList &b)	IMPLEMENT_OP(+, complex, ComplexList)
	ComplexList _cdecl sub(ComplexList &b)	IMPLEMENT_OP(-, complex, ComplexList)
	ComplexList _cdecl mul(ComplexList &b)	IMPLEMENT_OP(*, complex, ComplexList)
	ComplexList _cdecl div(ComplexList &b)	IMPLEMENT_OP(/, complex, ComplexList)

	// a += x
	void _cdecl iadd2(complex x)	IMPLEMENT_IOP2(+=, complex)
	void _cdecl isub2(complex x)	IMPLEMENT_IOP2(-=, complex)
	void _cdecl imul2(complex x)	IMPLEMENT_IOP2(*=, complex)
	void _cdecl idiv2(complex x)	IMPLEMENT_IOP2(/=, complex)
	void _cdecl imul2f(float x)	IMPLEMENT_IOP2(*=, complex)
	void _cdecl idiv2f(float x)	IMPLEMENT_IOP2(/=, complex)
	void _cdecl assign_complex(complex x)	IMPLEMENT_IOP2(=, complex)
};

Array<int> _cdecl int_range(int start, int end)
{
	Array<int> a;
	//a.__init__(); // done by kaba-constructors for temp variables
	for (int i=start;i<end;i++)
		a.add(i);
	return a;
}

Array<float> _cdecl float_range(float start, float end, float step)
{
	Array<float> a;
	//a.__init__(); // done by kaba-constructors for temp variables
	//msg_write(a.element_size);
	for (float f=start;f<end;f+=step)
		a.add(f);
	return a;
}

float _cdecl maxf(float a, float b)
{	return (a > b) ? a : b;	}

float _cdecl minf(float a, float b)
{	return (a < b) ? a : b;	}

string _cdecl ff2s(complex &x){	return x.str();	}
string _cdecl fff2s(vector &x){	return x.str();	}
string _cdecl ffff2s(quaternion &x){	return x.str();	}



string CastVector2StringP(string &s)
{
	return ((vector*)s.data)->str();
}
string CastFFFF2StringP(string &s)
{
	return ((quaternion*)s.data)->str();
}
string CastComplex2StringP(string &s)
{
	return ((complex*)s.data)->str();
}

// amd64 complex return wrappers
void amd64_comlist_sum(complex &r, ComplexList &l)
{	r = l.sum();	}
void amd64_com_bar(complex &r, complex &c)
{	r = c.bar();	}

// amd64 vector return wrappers
void amd64_vec_dir2ang(vector &r, vector &v)
{	r = v.dir2ang();	}
void amd64_vec_dir2ang2(vector &r, vector &v, vector &u)
{	r = v.dir2ang2(u);	}
void amd64_vec_ang2dir(vector &r, vector &v)
{	r = v.ang2dir();	}
void amd64_vec_rotate(vector &r, vector &v, vector &a)
{	r = v.rotate(a);	}
void amd64_vec_transform(vector &r, vector &v, matrix &m)
{	r = v.transform(m);	}
void amd64_vec_transform_normal(vector &r, vector &v, matrix &m)
{	r = v.transform_normal(m);	}
void amd64_vec_untransform(vector &r, vector &v, matrix &m)
{	r = v.untransform(m);	}
void amd64_vec_ortho(vector &r, vector &v)
{	r = v.ortho();	}
void amd64_quat_get_angles(vector &r, quaternion &q)
{	r = q.get_angles();	}
void amd64_vec_inter_get(vector &r, Interpolator<vector> &inter, float t)
{	r = inter.get(t);	}
void amd64_vec_inter_get_tang(vector &r, Interpolator<vector> &inter, float t)
{	r = inter.getTang(t);	}
void amd64_mat_vec_mul(vector &r, matrix &m, vector &v)
{	r = m * v;	}
void amd64_vec_ang_add(vector &r, vector &a, vector &b)
{	r = VecAngAdd(a, b);	}
void amd64_vec_ang_interpolate(vector &r, vector &a, vector &b, float t)
{	r = VecAngInterpolate(a, b, t);	}
void amd64_vec_cross_product(vector &r, vector &a, vector &b)
{	r = VecCrossProduct(a, b);	}

void amd64_vec_rand_dir(vector &v, Random &r)
{	v = r.dir();	}
void amd64_vec_rand_in_ball(vector &v, Random &r, float rad)
{	v = r.in_ball(rad);	}


// amd64 quaternion return wrappers
void amd64_quat_mul(quaternion &r, quaternion &a, quaternion&b)
{	r = a * b;	}
void amd64_quat_vec_mul(vector &r, quaternion &a, vector &b)
{	r = a * b;	}
void amd64_quat_bar(quaternion &r, quaternion &a)
{	r = a.bar();	}

// amd64 color return wrappers
void amd64_col_hsb(color &r, float a, float h, float s, float b)
{	r = SetColorHSB(a, h, s, b);	}
void amd64_col_interpolate(color &r, color &a, color &b, float t)
{	r = ColorInterpolate(a, b, t);	}
void amd64_col_add(color &r, color &a, color &b)
{	r = a + b;	}
void amd64_col_sub(color &r, color &a, color &b)
{	r = a - b;	}
void amd64_col_mul_c(color &r, color &a, color &b)
{	r = a * b;	}
void amd64_col_mul_f(color &r, color &a, float b)
{	r = a * b;	}

#define amd64_wrap(orig, wrap)	((config.instruction_set == Asm::InstructionSetAMD64) ? ((void*)(wrap)) : ((void*)(orig)))


void __complex_set(complex &r, float x, float y)
{	r = complex(x, y);	}
void __color_set(color &_r, float a, float r, float g, float b)
{	_r = color(a, r, g, b);	}
void __vector_set(vector &r, float x, float y, float z)
{	r = vector(x, y, z);	}
void __rect_set(rect &r, float x1, float x2, float y1, float y2)
{	r = rect(x1, x2, y1, y2);	}

void SIAddPackageMath()
{
	msg_db_f("SIAddPackageMath", 3);

	add_package("math", false);

	// types
	TypeComplex		= add_type  ("complex",		sizeof(float) * 2);
	TypeComplexList	= add_type_a("complex[]",	TypeComplex, -1);
	TypeVector		= add_type  ("vector",		sizeof(vector));
	TypeVectorList	= add_type_a("vector[]",	TypeVector, -1);
	TypeRect		= add_type  ("rect",		sizeof(rect));
	TypeMatrix		= add_type  ("matrix",		sizeof(matrix));
	TypeQuaternion	= add_type  ("quaternion",	sizeof(quaternion));
	TypePlane		= add_type  ("plane",		sizeof(plane));
	TypePlaneList	= add_type_a("plane[]",		TypePlane, -1);
	TypeColor		= add_type  ("color",		sizeof(color));
	TypeMatrix3		= add_type  ("matrix3",		sizeof(matrix3));
	Type*
	TypeFloatArray3		= add_type_a("float[3]",	TypeFloat32, 3);
	Type*
	TypeFloatArray4		= add_type_a("float[4]",	TypeFloat32, 4);
	Type*
	TypeFloatArray4x4	= add_type_a("float[4][4]",	TypeFloatArray4, 4);
	Type*
	TypeFloatArray16	= add_type_a("float[16]",	TypeFloat32, 16);
	Type*
	TypeFloatArray3x3	= add_type_a("float[3][3]",	TypeFloatArray3, 3);
	Type*
	TypeFloatArray9		= add_type_a("float[9]",	TypeFloat32, 9);
	Type*
	TypeVli			= add_type  ("vli",		sizeof(vli));
	Type*
	TypeCrypto		= add_type  ("Crypto",	sizeof(Crypto));
	Type*
	TypeAny			= add_type  ("any",		sizeof(Any));
	Type*
	TypeFloatInterpolator	= add_type  ("FloatInterpolator",		sizeof(Interpolator<float>));
	Type*
	TypeVectorInterpolator	= add_type  ("VectorInterpolator",		sizeof(Interpolator<vector>));
	Type*
	TypeRandom		= add_type  ("Random",	sizeof(Random));
	
	
	add_class(TypeIntList);
		class_add_func("sort", TypeVoid, mf(&IntList::sort));
		class_add_func("unique", TypeVoid, mf(&IntList::unique));
		class_add_func("sum", TypeInt, mf(&IntList::sum), FLAG_PURE);
		class_add_func("__iadd__", TypeVoid, mf(&IntList::iadd));
			func_add_param("other",	TypeIntList);
		class_add_func("__isub__", TypeVoid, mf(&IntList::isub));
			func_add_param("other",	TypeIntList);
		class_add_func("__imul__", TypeVoid, mf(&IntList::imul));
			func_add_param("other",	TypeIntList);
		class_add_func("__idiv__", TypeVoid, mf(&IntList::idiv));
			func_add_param("other",	TypeIntList);
		class_add_func("__add__", TypeIntList, mf(&IntList::add), FLAG_PURE);
			func_add_param("other",	TypeIntList);
		class_add_func("__sub__", TypeIntList, mf(&IntList::sub), FLAG_PURE);
			func_add_param("other",	TypeIntList);
		class_add_func("__mul__", TypeIntList, mf(&IntList::mul), FLAG_PURE);
			func_add_param("other",	TypeIntList);
		class_add_func("__div__", TypeIntList, mf(&IntList::div), FLAG_PURE);
			func_add_param("other",	TypeIntList);
		class_add_func("__assign__", TypeVoid, mf(&IntList::assign_int));
			func_add_param("other",	TypeInt);

	add_class(TypeFloatList);
		class_add_func("sort", TypeVoid, mf(&FloatList::sort));
		class_add_func("sum", TypeFloat32, mf(&FloatList::sum), FLAG_PURE);
		class_add_func("sum2", TypeFloat32, mf(&FloatList::sum2), FLAG_PURE);
		class_add_func("max", TypeFloat32, mf(&FloatList::_max), FLAG_PURE);
		class_add_func("min", TypeFloat32, mf(&FloatList::_min), FLAG_PURE);
		class_add_func("__iadd__", TypeVoid, mf(&FloatList::iadd));
			func_add_param("other",	TypeFloatList);
		class_add_func("__isub__", TypeVoid, mf(&FloatList::isub));
			func_add_param("other",	TypeFloatList);
		class_add_func("__imul__", TypeVoid, mf(&FloatList::imul));
			func_add_param("other",	TypeFloatList);
		class_add_func("__idiv__", TypeVoid, mf(&FloatList::idiv));
			func_add_param("other",	TypeFloatList);
		class_add_func("__add__", TypeFloatList, mf(&FloatList::add), FLAG_PURE);
			func_add_param("other",	TypeFloatList);
		class_add_func("__sub__", TypeFloatList, mf(&FloatList::sub), FLAG_PURE);
			func_add_param("other",	TypeFloatList);
		class_add_func("__mul__", TypeFloatList, mf(&FloatList::mul), FLAG_PURE);
			func_add_param("other",	TypeFloatList);
		class_add_func("__div__", TypeFloatList, mf(&FloatList::div), FLAG_PURE);
			func_add_param("other",	TypeFloatList);
		class_add_func("__iadd__", TypeVoid, mf(&FloatList::iadd2));
			func_add_param("other",	TypeFloat32);
		class_add_func("__isub__", TypeVoid, mf(&FloatList::isub2));
			func_add_param("other",	TypeFloat32);
		class_add_func("__imul__", TypeVoid, mf(&FloatList::imul2));
			func_add_param("other",	TypeFloat32);
		class_add_func("__idiv__", TypeVoid, mf(&FloatList::idiv2));
			func_add_param("other",	TypeFloat32);
		class_add_func("__assign__", TypeVoid, mf(&FloatList::assign_float));
			func_add_param("other",	TypeFloat32);

	add_class(TypeComplexList);
		class_add_func("__init__",	TypeVoid, mf(&ComplexList::__init__));
		class_add_func("sum",	TypeComplex, amd64_wrap(mf(&ComplexList::sum), &amd64_comlist_sum), FLAG_PURE);
		class_add_func("sum2",	TypeFloat32, mf(&ComplexList::sum2), FLAG_PURE);
		class_add_func("__iadd__", TypeVoid, mf(&ComplexList::iadd));
			func_add_param("other",	TypeComplexList);
		class_add_func("__isub__", TypeVoid, mf(&ComplexList::isub));
			func_add_param("other",	TypeComplexList);
		class_add_func("__imul__", TypeVoid, mf(&ComplexList::imul));
			func_add_param("other",	TypeComplexList);
		class_add_func("__idiv__", TypeVoid, mf(&ComplexList::idiv));
			func_add_param("other",	TypeComplexList);
		class_add_func("__add__", TypeComplexList, mf(&ComplexList::add), FLAG_PURE);
			func_add_param("other",	TypeComplexList);
		class_add_func("__sub__", TypeComplexList, mf(&ComplexList::sub), FLAG_PURE);
			func_add_param("other",	TypeComplexList);
		class_add_func("__mul__", TypeComplexList, mf(&ComplexList::mul), FLAG_PURE);
			func_add_param("other",	TypeComplexList);
		class_add_func("__div__", TypeComplexList, mf(&ComplexList::div), FLAG_PURE);
			func_add_param("other",	TypeComplexList);
		class_add_func("__iadd__", TypeVoid, mf(&ComplexList::iadd2));
			func_add_param("other",	TypeComplex);
		class_add_func("__isub__", TypeVoid, mf(&ComplexList::isub2));
			func_add_param("other",	TypeComplex);
		class_add_func("__imul__", TypeVoid, mf(&ComplexList::imul2));
			func_add_param("other",	TypeComplex);
		class_add_func("__idiv__", TypeVoid, mf(&ComplexList::idiv2));
			func_add_param("other",	TypeComplex);
		class_add_func("__imul__", TypeVoid, mf(&ComplexList::imul2f));
			func_add_param("other",	TypeFloat32);
		class_add_func("__idiv__", TypeVoid, mf(&ComplexList::idiv2f));
			func_add_param("other",	TypeFloat32);
		class_add_func("__assign__", TypeVoid, mf(&ComplexList::assign_complex));
			func_add_param("other",	TypeComplex);
	
	add_class(TypeVectorList);
		class_add_func("__init__",	TypeVoid, mf(&Array<vector>::__init__));
	add_class(TypePlaneList);
		class_add_func("__init__",	TypeVoid, mf(&Array<plane>::__init__));

	
	add_class(TypeComplex);
		class_add_element("x",		TypeFloat32,	0);
		class_add_element("y",		TypeFloat32,	4);
		class_add_func("abs",		TypeFloat32,			mf(&complex::abs), FLAG_PURE);
		class_add_func("absSqr",	TypeFloat32,			mf(&complex::abs_sqr), FLAG_PURE);
		class_add_func("bar",		TypeComplex, 		amd64_wrap(mf(&complex::bar), &amd64_com_bar), FLAG_PURE);
		class_add_func("str",		TypeString,			mf(&complex::str), FLAG_PURE);
	
	add_class(TypeVector);
		class_add_element("x",		TypeFloat32,	0);
		class_add_element("y",		TypeFloat32,	4);
		class_add_element("z",		TypeFloat32,	8);
		class_add_element("e",		TypeFloatArray3,	0, FLAG_HIDDEN);
		class_add_func("length",			TypeFloat32,	type_p(mf(&vector::length)), FLAG_PURE);
		class_add_func("lengthSqr",		TypeFloat32,	type_p(mf(&vector::length_sqr)), FLAG_PURE);
		class_add_func("lengthFuzzy",		TypeFloat32,	type_p(mf(&vector::length_fuzzy)), FLAG_PURE);
		class_add_func("normalize",		TypeVoid,	type_p(mf(&vector::normalize)));
		class_add_func("dir2ang",			TypeVector,	amd64_wrap(mf(&vector::dir2ang), &amd64_vec_dir2ang), FLAG_PURE);
		class_add_func("dir2ang2",			TypeVector,	amd64_wrap(mf(&vector::dir2ang2), &amd64_vec_dir2ang2), FLAG_PURE);
			func_add_param("up",		TypeVector);
		class_add_func("ang2dir",			TypeVector,	amd64_wrap(mf(&vector::ang2dir), &amd64_vec_ang2dir), FLAG_PURE);
		class_add_func("rotate",			TypeVector,	amd64_wrap(mf(&vector::rotate), &amd64_vec_rotate), FLAG_PURE);
			func_add_param("ang",		TypeVector);
		class_add_func("transform",		TypeVector,	amd64_wrap(mf(&vector::transform), &amd64_vec_transform), FLAG_PURE);
			func_add_param("m",			TypeMatrix);
		class_add_func("transformNormal",	TypeVector,	amd64_wrap(mf(&vector::transform_normal), &amd64_vec_transform_normal), FLAG_PURE);
			func_add_param("m",			TypeMatrix);
		class_add_func("untransform",		TypeVector,	amd64_wrap(mf(&vector::untransform), &amd64_vec_untransform), FLAG_PURE);
			func_add_param("m",			TypeMatrix);
		class_add_func("__div__",		TypeVector,	amd64_wrap(mf(&vector::untransform), &amd64_vec_untransform), FLAG_PURE);
			func_add_param("m",			TypeMatrix);
		class_add_func("ortho",			TypeVector,	amd64_wrap(mf(&vector::ortho), &amd64_vec_ortho), FLAG_PURE);
		class_add_func("str",		TypeString,			mf(&vector::str), FLAG_PURE);
	
	add_class(TypeQuaternion);
		class_add_element("x",		TypeFloat32,	0);
		class_add_element("y",		TypeFloat32,	4);
		class_add_element("z",		TypeFloat32,	8);
		class_add_element("w",		TypeFloat32,	12);
		class_add_func("__mul__", TypeQuaternion, amd64_wrap(mf(&quaternion::mul), &amd64_quat_mul), FLAG_PURE);
			func_add_param("other",	TypeQuaternion);
		class_add_func("__mul__", TypeVector, (void*)&amd64_quat_vec_mul, FLAG_PURE);
			func_add_param("other",	TypeVector);
		class_add_func("__imul__", TypeVoid, mf(&quaternion::imul));
			func_add_param("other",	TypeQuaternion);
		class_add_func("invert",	TypeVoid,	mf(&quaternion::invert));
		class_add_func("bar",		TypeQuaternion,	amd64_wrap(mf(&quaternion::bar), &amd64_quat_bar), FLAG_PURE);
		class_add_func("normalize",	TypeVoid,	mf(&quaternion::normalize));
		class_add_func("getAngles",	TypeVector,	amd64_wrap(mf(&quaternion::get_angles), &amd64_quat_get_angles), FLAG_PURE);
		class_add_func("str",		TypeString,			mf(&quaternion::str), FLAG_PURE);
	
	add_class(TypeRect);
		class_add_element("x1",	TypeFloat32,	0);
		class_add_element("x2",	TypeFloat32,	4);
		class_add_element("y1",	TypeFloat32,	8);
		class_add_element("y2",	TypeFloat32,	12);
		class_add_func("width",		TypeFloat32,			mf(&rect::width), FLAG_PURE);
		class_add_func("height",	TypeFloat32,			mf(&rect::height), FLAG_PURE);
		class_add_func("area",		TypeFloat32,			mf(&rect::area), FLAG_PURE);
		class_add_func("inside",	TypeBool,			mf(&rect::inside), FLAG_PURE);
			func_add_param("x",	TypeFloat32);
			func_add_param("y",	TypeFloat32);
		class_add_func("str",		TypeString,			mf(&rect::str), FLAG_PURE);
	
	add_class(TypeColor);
		class_add_element("a",		TypeFloat32,	12);
		class_add_element("r",		TypeFloat32,	0);
		class_add_element("g",		TypeFloat32,	4);
		class_add_element("b",		TypeFloat32,	8);
		class_add_func("str",		TypeString,			mf(&color::str), FLAG_PURE);
		class_add_func("__add__",	TypeColor,	amd64_wrap(mf(&color::operator+), &amd64_col_add), FLAG_PURE);
			func_add_param("o",	TypeColor);
		class_add_func("__adds__",	TypeVoid,	mf(&color::operator+=));
			func_add_param("o",	TypeColor);
		class_add_func("__sub__",	TypeColor,	amd64_wrap(mf(&color::operator-), &amd64_col_sub), FLAG_PURE);
			func_add_param("o",	TypeColor);
		class_add_func("__subs__",	TypeVoid,	mf(&color::operator-=));
			func_add_param("o",	TypeColor);
		class_add_func("__mul__",	TypeColor,	(void*)&amd64_col_mul_f, FLAG_PURE);
			func_add_param("f",	TypeFloat32);
		class_add_func("__mul__",	TypeColor,	(void*)&amd64_col_mul_c, FLAG_PURE);
			func_add_param("c",	TypeColor);
	
	add_class(TypePlane);
		class_add_element("a",		TypeFloat32,	0);
		class_add_element("b",		TypeFloat32,	4);
		class_add_element("c",		TypeFloat32,	8);
		class_add_element("d",		TypeFloat32,	12);
		class_add_element("n",		TypeVector,	0);
		class_add_func("intersectLine",	TypeBool,	mf(&plane::intersect_line), FLAG_PURE);
			func_add_param("l1",		TypeVector);
			func_add_param("l2",		TypeVector);
			func_add_param("inter",		TypeVector);
		class_add_func("inverse",	TypeVoid,	mf(&plane::inverse), FLAG_PURE);
		class_add_func("distance",	TypeFloat32,	mf(&plane::distance), FLAG_PURE);
			func_add_param("p",		TypeVector);
		class_add_func("str",		TypeString,			mf(&plane::str), FLAG_PURE);
	
	add_class(TypeMatrix);
		class_add_element("_00",	TypeFloat32,	0);
		class_add_element("_10",	TypeFloat32,	4);
		class_add_element("_20",	TypeFloat32,	8);
		class_add_element("_30",	TypeFloat32,	12);
		class_add_element("_01",	TypeFloat32,	16);
		class_add_element("_11",	TypeFloat32,	20);
		class_add_element("_21",	TypeFloat32,	24);
		class_add_element("_31",	TypeFloat32,	28);
		class_add_element("_02",	TypeFloat32,	32);
		class_add_element("_12",	TypeFloat32,	36);
		class_add_element("_22",	TypeFloat32,	40);
		class_add_element("_32",	TypeFloat32,	44);
		class_add_element("_03",	TypeFloat32,	48);
		class_add_element("_13",	TypeFloat32,	52);
		class_add_element("_23",	TypeFloat32,	56);
		class_add_element("_33",	TypeFloat32,	60);
		class_add_element("e",		TypeFloatArray4x4,	0, FLAG_HIDDEN);
		class_add_element("_e",		TypeFloatArray16,	0, FLAG_HIDDEN);
		class_add_func("__imul__",	TypeVoid,	mf(&matrix::imul));
			func_add_param("other",	TypeMatrix);
		class_add_func("__mul__",	TypeMatrix,	mf(&matrix::mul), FLAG_PURE);
			func_add_param("other",	TypeMatrix);
		class_add_func("__mul__",	TypeVector,	amd64_wrap(mf(&matrix::mul_v), &amd64_mat_vec_mul), FLAG_PURE);
			func_add_param("other",	TypeVector);
		class_add_func("str",		TypeString,			mf(&matrix::str), FLAG_PURE);
	
	add_class(TypeMatrix3);
		class_add_element("_11",	TypeFloat32,	0);
		class_add_element("_21",	TypeFloat32,	4);
		class_add_element("_31",	TypeFloat32,	8);
		class_add_element("_12",	TypeFloat32,	12);
		class_add_element("_22",	TypeFloat32,	16);
		class_add_element("_32",	TypeFloat32,	20);
		class_add_element("_13",	TypeFloat32,	24);
		class_add_element("_23",	TypeFloat32,	28);
		class_add_element("_33",	TypeFloat32,	32);
		class_add_element("e",		TypeFloatArray3x3,	0, FLAG_HIDDEN);
		class_add_element("_e",		TypeFloatArray9,	0, FLAG_HIDDEN);
		class_add_func("str",		TypeString,			mf(&matrix3::str), FLAG_PURE);
	
	add_class(TypeVli);
		class_add_element("sign",	TypeBool,	0);
		class_add_element("data",	TypeIntList,	4);
		class_add_func("__init__",	TypeVoid, algebra_p(mf(&vli::__init__)));
		class_add_func("__delete__",	TypeVoid, algebra_p(mf(&vli::__delete__)));
		class_add_func("__assign__",			TypeVoid,			algebra_p(mf(&vli::set_vli)));
			func_add_param("v",			TypeVli);
		class_add_func("__assign__",			TypeVoid,			algebra_p(mf(&vli::set_str)));
			func_add_param("s",			TypeString);
		class_add_func("__assign__",			TypeVoid,			algebra_p(mf(&vli::set_int)));
			func_add_param("i",			TypeInt);
		class_add_func("str",		TypeString,			algebra_p(mf(&vli::to_string)), FLAG_PURE);
		class_add_func("compare",			TypeInt,			algebra_p(mf(&vli::compare)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__eq__",			TypeBool,			algebra_p(mf(&vli::operator==)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__ne__",			TypeBool,			algebra_p(mf(&vli::operator!=)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__lt__",			TypeBool,			algebra_p(mf(&vli::operator<)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__gt__",			TypeBool,			algebra_p(mf(&vli::operator>)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__le__",			TypeBool,			algebra_p(mf(&vli::operator<=)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__ge__",			TypeBool,			algebra_p(mf(&vli::operator>=)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__iadd__",			TypeVoid,			algebra_p(mf(&vli::operator+=)));
			func_add_param("v",			TypeVli);
		class_add_func("__isub__",			TypeVoid,			algebra_p(mf(&vli::operator-=)));
			func_add_param("v",			TypeVli);
		class_add_func("__imul__",			TypeVoid,			algebra_p(mf(&vli::operator*=)));
			func_add_param("v",			TypeVli);
		class_add_func("__add__",			TypeVli,			algebra_p(mf(&vli::operator+)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__sub__",			TypeVli,			algebra_p(mf(&vli::operator-)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("__mul__",			TypeVli,			algebra_p(mf(&vli::operator*)), FLAG_PURE);
			func_add_param("v",			TypeVli);
		class_add_func("idiv",			TypeVoid,			algebra_p(mf(&vli::idiv)));
			func_add_param("div",			TypeVli);
			func_add_param("rem",			TypeVli);
		class_add_func("div",			TypeVli,			algebra_p(mf(&vli::_div)), FLAG_PURE);
			func_add_param("div",			TypeVli);
			func_add_param("rem",			TypeVli);
		class_add_func("pow",			TypeVli,			algebra_p(mf(&vli::pow)), FLAG_PURE);
			func_add_param("exp",			TypeVli);
		class_add_func("powMod",		TypeVli,			algebra_p(mf(&vli::pow_mod)), FLAG_PURE);
			func_add_param("exp",			TypeVli);
			func_add_param("mod",		TypeVli);
		class_add_func("gcd",			TypeVli,			algebra_p(mf(&vli::gcd)), FLAG_PURE);
			func_add_param("v",			TypeVli);
	
	add_class(TypeAny);
		class_add_element("type",	TypeInt, 0);
		class_add_element("data",	TypePointer, 4);
		class_add_func("__init__",	TypeVoid, any_p(mf(&Any::__init__)));
		class_add_func("__delete__",	TypeVoid, any_p(mf(&Any::clear)));
		class_add_func("__assign__",			TypeVoid,			any_p(mf(&Any::set)));
			func_add_param("a",			TypeAny);
		class_add_func("__assign__",			TypeVoid,			any_p(mf(&Any::set_str)));
			func_add_param("s",			TypeString);
		class_add_func("__assign__",			TypeVoid,			any_p(mf(&Any::set_int)));
			func_add_param("i",			TypeInt);
		class_add_func("__assign__",			TypeVoid,			any_p(mf(&Any::set_float)));
			func_add_param("f",			TypeFloat32);
		class_add_func("__assign__",			TypeVoid,			any_p(mf(&Any::set_bool)));
			func_add_param("b",			TypeBool);
		class_add_func("__iadd__",			TypeVoid,			any_p(mf(&Any::_add)));
			func_add_param("a",			TypeAny);
		class_add_func("__isub__",			TypeVoid,			any_p(mf(&Any::_sub)));
			func_add_param("a",			TypeAny);
		class_add_func("clear",	TypeVoid, any_p(mf(&Any::clear)));
		class_add_func("get",			TypeAny,			any_p(mf(&Any::get)));
			func_add_param("key",			TypeString);
		class_add_func("hset",			TypeVoid,			any_p(mf(&Any::hset)));
			func_add_param("key",			TypeString);
			func_add_param("value",			TypeAny);
		class_add_func("at",			TypeAny,			any_p(mf(&Any::at)));
			func_add_param("index",			TypeInt);
		class_add_func("aset",			TypeVoid,			any_p(mf(&Any::aset)));
			func_add_param("index",			TypeString);
			func_add_param("value",			TypeAny);
		class_add_func("bool",		TypeBool,			any_p(mf(&Any::_bool)));
		class_add_func("int",		TypeInt,			any_p(mf(&Any::_int)));
		class_add_func("float",		TypeFloat32,			any_p(mf(&Any::_float)));
		class_add_func("str",		TypeString,			any_p(mf(&Any::str)));


	add_class(TypeCrypto);
		class_add_element("n",	TypeVli, 0);
		class_add_element("k",	TypeVli, sizeof(vli));
		class_add_func("__init__",	TypeVoid, algebra_p(mf(&Crypto::__init__)));
		class_add_func("str",		TypeString, algebra_p(mf(&Crypto::str)));
		class_add_func("fromStr",	TypeVoid, algebra_p(mf(&Crypto::from_str)));
			func_add_param("str",		TypeString);
		class_add_func("encrypt",	TypeString, algebra_p(mf(&Crypto::Encrypt)));
			func_add_param("str",		TypeString);
		class_add_func("decrypt",	TypeString, algebra_p(mf(&Crypto::Decrypt)));
			func_add_param("str",		TypeString);
			func_add_param("cut",		TypeBool);

	add_class(TypeRandom);
		class_add_element("n",	TypeRandom, 0);
		class_add_func("seed",		TypeVoid, mf(&Random::seed));
			func_add_param("str",		TypeString);
		class_add_func("geti",		TypeInt, mf(&Random::geti));
			func_add_param("max",		TypeInt);
		class_add_func("getu",		TypeFloat32, mf(&Random::getu));
		class_add_func("getf",		TypeFloat32, mf(&Random::getf));
			func_add_param("min",		TypeFloat32);
			func_add_param("max",		TypeFloat32);
		class_add_func("inBall",		TypeVector, amd64_wrap(mf(&Random::in_ball), &amd64_vec_rand_in_ball));
			func_add_param("r",		TypeFloat32);
		class_add_func("dir",		TypeVector, amd64_wrap(mf(&Random::dir), &amd64_vec_rand_dir));
	
	add_func("complex",		TypeComplex,	(void*)__complex_set, FLAG_PURE);
		func_set_inline(CommandInlineComplexSet);
		func_add_param("x",		TypeFloat32);
		func_add_param("y",		TypeFloat32);
	add_func("rect",		TypeRect,	(void*)__rect_set, FLAG_PURE);
		func_set_inline(CommandInlineRectSet);
		func_add_param("x1",	TypeFloat32);
		func_add_param("x2",	TypeFloat32);
		func_add_param("y1",	TypeFloat32);
		func_add_param("y2",	TypeFloat32);

	
	add_class(TypeFloatInterpolator);
		class_add_element("type",	TypeInt, 0);
		class_add_func("__init__",	TypeVoid, mf(&Interpolator<float>::__init__));
		class_add_func("clear",	TypeVoid, mf(&Interpolator<float>::clear));
		class_add_func("setType",	TypeVoid, mf(&Interpolator<float>::setType));
			func_add_param("type",	TypeString);
		class_add_func("add",	TypeVoid, mf(&Interpolator<float>::addv));
			func_add_param("p",	TypeFloat32);
			func_add_param("dt",	TypeFloat32);
		class_add_func("add2",	TypeVoid, mf(&Interpolator<float>::add2v));
			func_add_param("p",	TypeFloat32);
			func_add_param("v",	TypeFloat32);
			func_add_param("dt",	TypeFloat32);
		class_add_func("add3",	TypeVoid, mf(&Interpolator<float>::add3v));
			func_add_param("p",	TypeFloat32);
			func_add_param("v",	TypeFloat32);
			func_add_param("w",	TypeFloat32);
			func_add_param("dt",	TypeFloat32);
		class_add_func("jump",	TypeVoid, mf(&Interpolator<float>::jumpv));
			func_add_param("p",	TypeFloat32);
			func_add_param("v",	TypeFloat32);
		class_add_func("normalize",	TypeVoid, mf(&Interpolator<float>::normalize));
		class_add_func("get",	TypeFloat32, mf(&Interpolator<float>::get));
			func_add_param("t",	TypeFloat32);
		class_add_func("getTang",	TypeFloat32, mf(&Interpolator<float>::getTang));
			func_add_param("t",	TypeFloat32);
		class_add_func("getList",	TypeFloatList, mf(&Interpolator<float>::getList));
			func_add_param("t",	TypeFloatList);

	
	add_class(TypeVectorInterpolator);
		class_add_element("type",	TypeInt, 0);
		class_add_func("__init__",	TypeVoid, mf(&Interpolator<vector>::__init__));
		class_add_func("clear",	TypeVoid, mf(&Interpolator<vector>::clear));
		class_add_func("setType",	TypeVoid, mf(&Interpolator<vector>::setType));
			func_add_param("type",	TypeString);
		class_add_func("add",	TypeVoid, mf(&Interpolator<vector>::add));
			func_add_param("p",	TypeVector);
			func_add_param("dt",	TypeFloat32);
		class_add_func("add2",	TypeVoid, mf(&Interpolator<vector>::add2));
			func_add_param("p",	TypeVector);
			func_add_param("v",	TypeVector);
			func_add_param("dt",	TypeFloat32);
		class_add_func("add3",	TypeVoid, mf(&Interpolator<vector>::add3));
			func_add_param("p",	TypeVector);
			func_add_param("v",	TypeVector);
			func_add_param("w",	TypeFloat32);
			func_add_param("dt",	TypeFloat32);
		class_add_func("jump",	TypeVoid, mf(&Interpolator<vector>::jump));
			func_add_param("p",	TypeVector);
			func_add_param("v",	TypeVector);
		class_add_func("normalize",	TypeVoid, mf(&Interpolator<vector>::normalize));
		class_add_func("get",	TypeVector, amd64_wrap(mf(&Interpolator<vector>::get), &amd64_vec_inter_get));
			func_add_param("t",	TypeFloat32);
		class_add_func("getTang",	TypeVector, amd64_wrap(mf(&Interpolator<vector>::getTang), &amd64_vec_inter_get_tang));
			func_add_param("t",	TypeFloat32);
		class_add_func("getList",	TypeVectorList, mf(&Interpolator<vector>::getList));
			func_add_param("t",	TypeFloatList);

	// mathematical
	add_func("sin",			TypeFloat32,	(void*)&sinf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("cos",			TypeFloat32,	(void*)&cosf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("tan",			TypeFloat32,	(void*)&tanf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("asin",		TypeFloat32,	(void*)&asinf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("acos",		TypeFloat32,	(void*)&acosf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("atan",		TypeFloat32,	(void*)&atanf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("atan2",		TypeFloat32,	(void*)&atan2f, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
		func_add_param("y",		TypeFloat32);
	add_func("sqrt",		TypeFloat32,	(void*)&sqrtf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("sqr",			TypeFloat32,		(void*)&f_sqr, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("exp",			TypeFloat32,		(void*)&expf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("log",			TypeFloat32,		(void*)&logf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
	add_func("pow",			TypeFloat32,		(void*)&powf, FLAG_PURE);
		func_add_param("x",		TypeFloat32);
		func_add_param("exp",	TypeFloat32);
	add_func("clamp",		TypeFloat32,		(void*)&clampf, FLAG_PURE);
		func_add_param("f",		TypeFloat32);
		func_add_param("min",	TypeFloat32);
		func_add_param("max",	TypeFloat32);
	add_func("loop",		TypeFloat32,		(void*)&loopf, FLAG_PURE);
		func_add_param("f",		TypeFloat32);
		func_add_param("min",	TypeFloat32);
		func_add_param("max",	TypeFloat32);
	add_func("abs",			TypeFloat32,		(void*)&fabsf, FLAG_PURE);
		func_add_param("f",		TypeFloat32);
	add_func("min",			TypeFloat32,		(void*)&minf, FLAG_PURE);
		func_add_param("a",		TypeFloat32);
		func_add_param("b",		TypeFloat32);
	add_func("max",			TypeFloat32,		(void*)&maxf, FLAG_PURE);
		func_add_param("a",		TypeFloat32);
		func_add_param("b",		TypeFloat32);
	// int
	add_func("clampi",		TypeInt,		(void*)&clampi, FLAG_PURE);
		func_add_param("i",		TypeInt);
		func_add_param("min",	TypeInt);
		func_add_param("max",	TypeInt);
	add_func("loopi",		TypeInt,		(void*)&loopi, FLAG_PURE);
		func_add_param("i",		TypeInt);
		func_add_param("min",	TypeInt);
		func_add_param("max",	TypeInt);
	// lists
	add_func("range",		TypeIntList,	(void*)&int_range, FLAG_PURE);
		func_add_param("start",		TypeInt);
		func_add_param("end",		TypeInt);
	add_func("rangef",		TypeFloatList,	(void*)&float_range, FLAG_PURE);
		func_add_param("start",		TypeFloat32);
		func_add_param("end",		TypeFloat32);
		func_add_param("step",		TypeFloat32);
	// vectors
	add_func("vector",		TypeVector,	(void*)&__vector_set, FLAG_PURE);
		func_set_inline(CommandInlineVectorSet);
		func_add_param("x",		TypeFloat32);
		func_add_param("y",		TypeFloat32);
		func_add_param("z",		TypeFloat32);
	add_func("VecAngAdd",			TypeVector,	amd64_wrap(&VecAngAdd, &amd64_vec_ang_add), FLAG_PURE);
		func_add_param("ang1",		TypeVector);
		func_add_param("ang2",		TypeVector);
	add_func("VecAngInterpolate",	TypeVector,	amd64_wrap(&VecAngInterpolate, &amd64_vec_ang_interpolate), FLAG_PURE);
		func_add_param("ang1",		TypeVector);
		func_add_param("ang2",		TypeVector);
		func_add_param("t",			TypeFloat32);
	add_func("VecDotProduct",		TypeFloat32,	(void*)&VecDotProduct, FLAG_PURE);
		func_add_param("v1",		TypeVector);
		func_add_param("v2",		TypeVector);
	add_func("VecCrossProduct",		TypeVector,	amd64_wrap(&VecCrossProduct, &amd64_vec_cross_product), FLAG_PURE);
		func_add_param("v1",		TypeVector);
		func_add_param("v2",		TypeVector);
	// matrices
	add_func("MatrixIdentity",		TypeVoid,	(void*)&MatrixIdentity, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
	add_func("MatrixTranslation",	TypeVoid,	(void*)&MatrixTranslation, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("trans",		TypeVector);
	add_func("MatrixRotation",		TypeVoid,	(void*)&MatrixRotation, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeVector);
	add_func("MatrixRotationX",		TypeVoid,	(void*)&MatrixRotationX, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeFloat32);
	add_func("MatrixRotationY",		TypeVoid,	(void*)&MatrixRotationY, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeFloat32);
	add_func("MatrixRotationZ",		TypeVoid,	(void*)&MatrixRotationZ, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeFloat32);
	add_func("MatrixRotationQ",		TypeVoid,	(void*)&MatrixRotationQ, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeQuaternion);
	add_func("MatrixRotationView",	TypeVoid,	(void*)&MatrixRotationView, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeVector);
	add_func("MatrixScale",			TypeVoid,	(void*)&MatrixScale, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("s_x",		TypeFloat32);
		func_add_param("s_y",		TypeFloat32);
		func_add_param("s_z",		TypeFloat32);
	add_func("MatrixMultiply",		TypeVoid,	(void*)&MatrixMultiply, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("m2",		TypeMatrix);
		func_add_param("m1",		TypeMatrix);
	add_func("MatrixInverse",		TypeVoid,	(void*)&MatrixInverse, FLAG_PURE);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("m_in",		TypeMatrix);
	// quaternions
	add_func("QuaternionRotationV",	TypeVoid,	(void*)&QuaternionRotationV, FLAG_PURE);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("ang",		TypeVector);
	add_func("QuaternionRotationA",	TypeVoid,	(void*)&QuaternionRotationA, FLAG_PURE);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("axis",		TypeVector);
		func_add_param("angle",		TypeFloat32);
	add_func("QuaternionInterpolate",		TypeVoid,	(void*)(void(*)(quaternion&, const quaternion&, const quaternion&, float))&QuaternionInterpolate, FLAG_PURE);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("q_0",		TypeQuaternion);
		func_add_param("q_1",		TypeQuaternion);
		func_add_param("t",		TypeFloat32);
	add_func("QuaternionDrag",	TypeVoid,	(void*)&QuaternionDrag, FLAG_PURE);
		func_add_param("q",		TypeQuaternion);
		func_add_param("up",		TypeVector);
		func_add_param("dang",		TypeVector);
		func_add_param("reset_z",		TypeBool);
	// plane
	add_func("PlaneFromPoints",	TypeVoid,	(void*)&PlaneFromPoints, FLAG_PURE);
		func_add_param("pl",		TypePlane);
		func_add_param("a",		TypeVector);
		func_add_param("b",		TypeVector);
		func_add_param("c",		TypeVector);
	add_func("PlaneFromPointNormal",	TypeVoid,	(void*)&PlaneFromPointNormal, FLAG_PURE);
		func_add_param("pl",		TypePlane);
		func_add_param("p",		TypeVector);
		func_add_param("n",		TypeVector);
	add_func("PlaneTransform",	TypeVoid,	(void*)&PlaneTransform, FLAG_PURE);
		func_add_param("pl_out",		TypePlane);
		func_add_param("m",		TypeMatrix);
		func_add_param("pl_in",		TypePlane);
	// other types
	add_func("GetBaryCentric",	TypeVoid,	(void*)&GetBaryCentric, FLAG_PURE);
		func_add_param("p",		TypeVector);
		func_add_param("a",		TypeVector);
		func_add_param("b",		TypeVector);
		func_add_param("c",		TypeVector);
		func_add_param("f",		TypeFloatPs);
		func_add_param("g",		TypeFloatPs);
	add_func("color",		TypeColor,	(void*)&__color_set, FLAG_PURE);
		func_set_inline(CommandInlineColorSet);
		func_add_param("a",		TypeFloat32);
		func_add_param("r",		TypeFloat32);
		func_add_param("g",		TypeFloat32);
		func_add_param("b",		TypeFloat32);
	add_func("ColorSetHSB",			TypeColor,		amd64_wrap(&SetColorHSB, &amd64_col_hsb), FLAG_PURE);
		func_add_param("a",		TypeFloat32);
		func_add_param("h",		TypeFloat32);
		func_add_param("s",		TypeFloat32);
		func_add_param("b",		TypeFloat32);
	add_func("ColorInterpolate",	TypeColor,		amd64_wrap(&ColorInterpolate, &amd64_col_interpolate), FLAG_PURE);
		func_add_param("c1",		TypeColor);
		func_add_param("c2",		TypeColor);
		func_add_param("t",		TypeFloat32);
	add_func("CryptoCreateKeys",	TypeVoid,		algebra_p(&CryptoCreateKeys));
		func_add_param("c1",		TypeCrypto);
		func_add_param("c2",		TypeCrypto);
		func_add_param("type",		TypeString);
		func_add_param("bits",		TypeInt);
	// random numbers
	add_func("randi",			TypeInt,		(void*)&randi);
		func_add_param("max",	TypeInt);
	add_func("rand",			TypeFloat32,		(void*)&randf);
		func_add_param("max",	TypeFloat32);
	add_func("rand_seed",		TypeVoid,		(void*)&srand);
		func_add_param("seed",	TypeInt);

	
	// float
	add_const("pi",  TypeFloat32, *(void**)&pi);
	// complex
	add_const("c_i", TypeComplex, (void**)&c_i);
	// vector
	add_const("v_0", TypeVector, (void*)&v_0);
	add_const("e_x", TypeVector, (void*)&e_x);
	add_const("e_y", TypeVector, (void*)&e_y);
	add_const("e_z", TypeVector, (void*)&e_z);
	// matrix
	add_const("m_id", TypeMatrix, (void*)&m_id);
	// quaternion
	add_const("q_id", TypeQuaternion, (void*)&q_id);
	// color
	add_const("White",  TypeColor, (void*)&White);
	add_const("Black",  TypeColor, (void*)&Black);
	add_const("Gray",   TypeColor, (void*)&Gray);
	add_const("Red",    TypeColor, (void*)&Red);
	add_const("Green",  TypeColor, (void*)&Green);
	add_const("Blue",   TypeColor, (void*)&Blue);
	add_const("Yellow", TypeColor, (void*)&Yellow);
	add_const("Orange", TypeColor, (void*)&Orange);
	// rect
	add_const("r_id", TypeRect, (void*)&r_id);



	// internal type casts
	add_func("-v2s-",				TypeString,	(void*)&fff2s, FLAG_PURE);
		func_add_param("v",		TypeVector);
	add_func("-complex2s-",		TypeString,	(void*)&ff2s, FLAG_PURE);
		func_add_param("z",		TypeComplex);
	add_func("-quaternion2s-",	TypeString,	(void*)&ffff2s, FLAG_PURE);
		func_add_param("q",		TypeQuaternion);
	add_func("-plane2s-",			TypeString,	(void*)&ffff2s, FLAG_PURE);
		func_add_param("p",		TypePlane);
	add_func("-color2s-",			TypeString,	(void*)&ffff2s, FLAG_PURE);
		func_add_param("c",		TypeColor);
	add_func("-rect2s-",			TypeString,	(void*)&ffff2s, FLAG_PURE);
		func_add_param("r",		TypeRect);
	add_type_cast(50,	TypeVector,		TypeString,	"-v2s-",	(void*)&CastVector2StringP);
	add_type_cast(50,	TypeComplex,	TypeString,	"-complex2s-",	(void*)&CastComplex2StringP);
	add_type_cast(50,	TypeColor,		TypeString,	"-color2s-",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeQuaternion,	TypeString,	"-quaternion2s-",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypePlane,		TypeString,	"-plane2s-",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeRect,		TypeString,	"-rect2s-",	(void*)&CastFFFF2StringP);
}

};
