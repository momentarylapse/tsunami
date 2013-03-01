#include <algorithm>
#include "../file/file.h"
#include "script.h"
#include "../00_config.h"
#include "script_data_common.h"

#ifdef _X_USE_ALGEBRA_
	#include "../algebra/algebra.h"
#else
		typedef int vli;
		typedef int Crypto;
#endif

#ifdef _X_USE_ANY_
	#include "../any/any.h"
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
	int sum()
	{
		int r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	void sort()
	{	std::sort((int*)data, (int*)data + num);	}
	void unique()
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
	void iadd(IntList &b)	IMPLEMENT_IOP(+=, int)
	void isub(IntList &b)	IMPLEMENT_IOP(-=, int)
	void imul(IntList &b)	IMPLEMENT_IOP(*=, int)
	void idiv(IntList &b)	IMPLEMENT_IOP(/=, int)

	// a = b + c
	IntList add(IntList &b)	IMPLEMENT_OP(+, int, IntList)
	IntList sub(IntList &b)	IMPLEMENT_OP(-, int, IntList)
	IntList mul(IntList &b)	IMPLEMENT_OP(*, int, IntList)
	IntList div(IntList &b)	IMPLEMENT_OP(/, int, IntList)

	// a += x
	void add2(int x)	IMPLEMENT_IOP2(+=, int)
	void sub2(int x)	IMPLEMENT_IOP2(-=, int)
	void mul2(int x)	IMPLEMENT_IOP2(*=, int)
	void div2(int x)	IMPLEMENT_IOP2(/=, int)
	void assign_int(int x)	IMPLEMENT_IOP2(=, int)
};




void super_array_add_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) += *(pb ++);	}
void super_array_sub_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) -= *(pb ++);	}
void super_array_mul_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) *= *(pb ++);	}
void super_array_div_s_com(DynamicArray *a, DynamicArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) /= *(pb ++);	}

void super_array_add_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) += x;	}
void super_array_sub_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) -= x;	}
void super_array_mul_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}
void super_array_div_s_com_com(DynamicArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) /= x;	}
void super_array_mul_s_com_float(DynamicArray *a, float x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}

class FloatList : public Array<float>
{
public:
	float _max()
	{
		float max = 0;
		if (num > 0)
			max = (*this)[0];
		for (int i=1;i<num;i++)
			if ((*this)[i] > max)
				max = (*this)[i];
		return max;
	}
	float _min()
	{
		float min = 0;
		if (num > 0)
			min = (*this)[0];
		for (int i=1;i<num;i++)
			if ((*this)[i] < min)
				min = (*this)[i];
		return min;
	}
	float sum()
	{
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	float sum2()
	{
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i] * (*this)[i];
		return r;
	}

	void sort()
	{	std::sort((float*)data, (float*)data + num);	}
	
	// a += b
	void iadd(FloatList &b)	IMPLEMENT_IOP(+=, float)
	void isub(FloatList &b)	IMPLEMENT_IOP(-=, float)
	void imul(FloatList &b)	IMPLEMENT_IOP(*=, float)
	void idiv(FloatList &b)	IMPLEMENT_IOP(/=, float)

	// a = b + c
	FloatList add(FloatList &b)	IMPLEMENT_OP(+, float, FloatList)
	FloatList sub(FloatList &b)	IMPLEMENT_OP(-, float, FloatList)
	FloatList mul(FloatList &b)	IMPLEMENT_OP(*, float, FloatList)
	FloatList div(FloatList &b)	IMPLEMENT_OP(/, float, FloatList)

	// a += x
	void iadd2(float x)	IMPLEMENT_IOP2(+=, float)
	void isub2(float x)	IMPLEMENT_IOP2(-=, float)
	void imul2(float x)	IMPLEMENT_IOP2(*=, float)
	void idiv2(float x)	IMPLEMENT_IOP2(/=, float)
	void assign_float(float x)	IMPLEMENT_IOP2(=, float)
};

class ComplexList : public Array<complex>
{
public:
	complex sum()
	{
		complex r = complex(0, 0);
		for (int i=0;i<num;i++)
			r += (*this)[i];
		return r;
	}
	float sum2()
	{
		float r = 0;
		for (int i=0;i<num;i++)
			r += (*this)[i].abs_sqr();
		return r;
	}
	
	// a += b
	void iadd(ComplexList &b)	IMPLEMENT_IOP(+=, complex)
	void isub(ComplexList &b)	IMPLEMENT_IOP(-=, complex)
	void imul(ComplexList &b)	IMPLEMENT_IOP(*=, complex)
	void idiv(ComplexList &b)	IMPLEMENT_IOP(/=, complex)

	// a = b + c
	ComplexList add(ComplexList &b)	IMPLEMENT_OP(+, complex, ComplexList)
	ComplexList sub(ComplexList &b)	IMPLEMENT_OP(-, complex, ComplexList)
	ComplexList mul(ComplexList &b)	IMPLEMENT_OP(*, complex, ComplexList)
	ComplexList div(ComplexList &b)	IMPLEMENT_OP(/, complex, ComplexList)

	// a += x
	void iadd2(complex x)	IMPLEMENT_IOP2(+=, complex)
	void isub2(complex x)	IMPLEMENT_IOP2(-=, complex)
	void imul2(complex x)	IMPLEMENT_IOP2(*=, complex)
	void idiv2(complex x)	IMPLEMENT_IOP2(/=, complex)
	void imul2f(float x)	IMPLEMENT_IOP2(*=, complex)
	void idiv2f(float x)	IMPLEMENT_IOP2(/=, complex)
	void assign_complex(complex x)	IMPLEMENT_IOP2(=, complex)
};

Array<int> int_range(int start, int end)
{
	Array<int> a;
	//a.__init__(); // done by kaba-constructors for temp variables
	for (int i=start;i<end;i++)
		a.add(i);
	return a;
}

Array<float> float_range(float start, float end, float step)
{
	Array<float> a;
	//a.__init__(); // done by kaba-constructors for temp variables
	//msg_write(a.element_size);
	for (float f=start;f<end;f+=step)
		a.add(f);
	return a;
}

float maxf(float a, float b)
{	return (a > b) ? a : b;	}

float minf(float a, float b)
{	return (a < b) ? a : b;	}

void SIAddPackageMath()
{
	msg_db_r("SIAddPackageMath", 3);

	set_cur_package("math");

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
	TypeFloatArray3		= add_type_a("float[3]",	TypeFloat, 3);
	Type*
	TypeFloatArray4		= add_type_a("float[4]",	TypeFloat, 4);
	Type*
	TypeFloatArray4x4	= add_type_a("float[4][4]",	TypeFloatArray4, 4);
	Type*
	TypeFloatArray16	= add_type_a("float[16]",	TypeFloat, 16);
	Type*
	TypeFloatArray3x3	= add_type_a("float[3][3]",	TypeFloatArray3, 3);
	Type*
	TypeFloatArray9		= add_type_a("float[9]",	TypeFloat, 9);
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
	
	
	add_class(TypeIntList);
		class_add_func("sort", TypeVoid, mf((tmf)&IntList::sort));
		class_add_func("unique", TypeVoid, mf((tmf)&IntList::unique));
		class_add_func("sum", TypeInt, mf((tmf)&IntList::sum));
		class_add_func("__iadd__", TypeVoid, mf((tmf)&IntList::iadd));
			func_add_param("other",	TypeIntList);
		class_add_func("__isub__", TypeVoid, mf((tmf)&IntList::isub));
			func_add_param("other",	TypeIntList);
		class_add_func("__imul__", TypeVoid, mf((tmf)&IntList::imul));
			func_add_param("other",	TypeIntList);
		class_add_func("__idiv__", TypeVoid, mf((tmf)&IntList::idiv));
			func_add_param("other",	TypeIntList);
		class_add_func("__add__", TypeIntList, mf((tmf)&IntList::add));
			func_add_param("other",	TypeIntList);
		class_add_func("__sub__", TypeIntList, mf((tmf)&IntList::sub));
			func_add_param("other",	TypeIntList);
		class_add_func("__mul__", TypeIntList, mf((tmf)&IntList::mul));
			func_add_param("other",	TypeIntList);
		class_add_func("__div__", TypeIntList, mf((tmf)&IntList::div));
			func_add_param("other",	TypeIntList);
		class_add_func("__assign__", TypeVoid, mf((tmf)&IntList::assign_int));
			func_add_param("other",	TypeInt);

	add_class(TypeFloatList);
		class_add_func("sort", TypeVoid, mf((tmf)&FloatList::sort));
		class_add_func("sum", TypeFloat, mf((tmf)&FloatList::sum));
		class_add_func("sum2", TypeFloat, mf((tmf)&FloatList::sum2));
		class_add_func("max", TypeFloat, mf((tmf)&FloatList::_max));
		class_add_func("min", TypeFloat, mf((tmf)&FloatList::_min));
		class_add_func("__iadd__", TypeVoid, mf((tmf)&FloatList::iadd));
			func_add_param("other",	TypeFloatList);
		class_add_func("__isub__", TypeVoid, mf((tmf)&FloatList::isub));
			func_add_param("other",	TypeFloatList);
		class_add_func("__imul__", TypeVoid, mf((tmf)&FloatList::imul));
			func_add_param("other",	TypeFloatList);
		class_add_func("__idiv__", TypeVoid, mf((tmf)&FloatList::idiv));
			func_add_param("other",	TypeFloatList);
		class_add_func("__add__", TypeFloatList, mf((tmf)&FloatList::add));
			func_add_param("other",	TypeFloatList);
		class_add_func("__sub__", TypeFloatList, mf((tmf)&FloatList::sub));
			func_add_param("other",	TypeFloatList);
		class_add_func("__mul__", TypeFloatList, mf((tmf)&FloatList::mul));
			func_add_param("other",	TypeFloatList);
		class_add_func("__div__", TypeFloatList, mf((tmf)&FloatList::div));
			func_add_param("other",	TypeFloatList);
		class_add_func("__iadd__", TypeVoid, mf((tmf)&FloatList::iadd2));
			func_add_param("other",	TypeFloat);
		class_add_func("__isub__", TypeVoid, mf((tmf)&FloatList::isub2));
			func_add_param("other",	TypeFloat);
		class_add_func("__imul__", TypeVoid, mf((tmf)&FloatList::imul2));
			func_add_param("other",	TypeFloat);
		class_add_func("__idiv__", TypeVoid, mf((tmf)&FloatList::idiv2));
			func_add_param("other",	TypeFloat);
		class_add_func("__assign__", TypeVoid, mf((tmf)&FloatList::assign_float));
			func_add_param("other",	TypeFloat);

	add_class(TypeComplexList);
		class_add_func("__init__",	TypeVoid, mf((tmf)&ComplexList::__init__));
		class_add_func("sum",	TypeComplex, mf((tmf)&ComplexList::sum));
		class_add_func("sum2",	TypeFloat, mf((tmf)&ComplexList::sum2));
		class_add_func("__iadd__", TypeVoid, mf((tmf)&ComplexList::iadd));
			func_add_param("other",	TypeComplexList);
		class_add_func("__isub__", TypeVoid, mf((tmf)&ComplexList::isub));
			func_add_param("other",	TypeComplexList);
		class_add_func("__imul__", TypeVoid, mf((tmf)&ComplexList::imul));
			func_add_param("other",	TypeComplexList);
		class_add_func("__idiv__", TypeVoid, mf((tmf)&ComplexList::idiv));
			func_add_param("other",	TypeComplexList);
		class_add_func("__add__", TypeComplexList, mf((tmf)&ComplexList::add));
			func_add_param("other",	TypeComplexList);
		class_add_func("__sub__", TypeComplexList, mf((tmf)&ComplexList::sub));
			func_add_param("other",	TypeComplexList);
		class_add_func("__mul__", TypeComplexList, mf((tmf)&ComplexList::mul));
			func_add_param("other",	TypeComplexList);
		class_add_func("__div__", TypeComplexList, mf((tmf)&ComplexList::div));
			func_add_param("other",	TypeComplexList);
		class_add_func("__iadd__", TypeVoid, mf((tmf)&ComplexList::iadd2));
			func_add_param("other",	TypeComplex);
		class_add_func("__isub__", TypeVoid, mf((tmf)&ComplexList::isub2));
			func_add_param("other",	TypeComplex);
		class_add_func("__imul__", TypeVoid, mf((tmf)&ComplexList::imul2));
			func_add_param("other",	TypeComplex);
		class_add_func("__idiv__", TypeVoid, mf((tmf)&ComplexList::idiv2));
			func_add_param("other",	TypeComplex);
		class_add_func("__imul__", TypeVoid, mf((tmf)&ComplexList::imul2f));
			func_add_param("other",	TypeFloat);
		class_add_func("__idiv__", TypeVoid, mf((tmf)&ComplexList::idiv2f));
			func_add_param("other",	TypeFloat);
		class_add_func("__assign__", TypeVoid, mf((tmf)&ComplexList::assign_complex));
			func_add_param("other",	TypeComplex);
	
	add_class(TypeVectorList);
		class_add_func("__init__",	TypeVoid, mf((tmf)&Array<vector>::__init__));
	add_class(TypePlaneList);
		class_add_func("__init__",	TypeVoid, mf((tmf)&Array<plane>::__init__));

	
	add_class(TypeComplex);
		class_add_element("x",		TypeFloat,	0);
		class_add_element("y",		TypeFloat,	4);
		class_add_func("abs",		TypeFloat,			mf((tmf)&complex::abs));
		class_add_func("abs_sqr",	TypeFloat,			mf((tmf)&complex::abs_sqr));
		class_add_func("str",		TypeString,			mf((tmf)&complex::str));
	
	add_class(TypeVector);
		class_add_element("x",		TypeFloat,	0);
		class_add_element("y",		TypeFloat,	4);
		class_add_element("z",		TypeFloat,	8);
		class_add_func("length",			TypeFloat,	type_p(mf((tmf)&vector::length)));
		class_add_func("length_sqr",		TypeFloat,	type_p(mf((tmf)&vector::length_sqr)));
		class_add_func("length_fuzzy",		TypeFloat,	type_p(mf((tmf)&vector::length_fuzzy)));
		class_add_func("normalize",		TypeVoid,	type_p(mf((tmf)&vector::normalize)));
		class_add_func("dir2ang",			TypeVector,	type_p(mf((tmf)&vector::dir2ang)));
		class_add_func("dir2ang2",			TypeVector,	type_p(mf((tmf)&vector::dir2ang2)));
			func_add_param("up",		TypeVector);
		class_add_func("ang2dir",			TypeVector,	type_p(mf((tmf)&vector::ang2dir)));
		class_add_func("rotate",			TypeVector,	type_p(mf((tmf)&vector::rotate)));
			func_add_param("ang",		TypeVector);
		class_add_func("transform",		TypeVector,	type_p(mf((tmf)&vector::transform)));
			func_add_param("m",			TypeMatrix);
		class_add_func("transform_normal",	TypeVector,	type_p(mf((tmf)&vector::transform_normal)));
			func_add_param("m",			TypeMatrix);
		class_add_func("ortho",			TypeVector,	type_p(mf((tmf)&vector::ortho)));
		class_add_func("str",		TypeString,			type_p(mf((tmf)&vector::str)));
	
	add_class(TypeQuaternion);
		class_add_element("x",		TypeFloat,	0);
		class_add_element("y",		TypeFloat,	4);
		class_add_element("z",		TypeFloat,	8);
		class_add_element("w",		TypeFloat,	12);
		class_add_func("__mul__", TypeQuaternion, mf((tmf)&quaternion::mul));
			func_add_param("other",	TypeQuaternion);
		class_add_func("__imul__", TypeVoid, mf((tmf)&quaternion::imul));
			func_add_param("other",	TypeQuaternion);
		class_add_func("inverse",	TypeVoid,	mf((tmf)&quaternion::inverse));
		class_add_func("normalize",	TypeVoid,	mf((tmf)&quaternion::normalize));
		class_add_func("get_angles",	TypeVector,	mf((tmf)&quaternion::get_angles));
		class_add_func("str",		TypeString,			mf((tmf)&quaternion::str));
	
	add_class(TypeRect);
		class_add_element("x1",	TypeFloat,	0);
		class_add_element("x2",	TypeFloat,	4);
		class_add_element("y1",	TypeFloat,	8);
		class_add_element("y2",	TypeFloat,	12);
		class_add_func("width",		TypeFloat,			mf((tmf)&rect::width));
		class_add_func("height",	TypeFloat,			mf((tmf)&rect::height));
		class_add_func("inside",	TypeBool,			mf((tmf)&rect::inside));
			func_add_param("x",	TypeFloat);
			func_add_param("y",	TypeFloat);
		class_add_func("str",		TypeString,			mf((tmf)&rect::str));
	
	add_class(TypeColor);
		class_add_element("a",		TypeFloat,	12);
		class_add_element("r",		TypeFloat,	0);
		class_add_element("g",		TypeFloat,	4);
		class_add_element("b",		TypeFloat,	8);
		class_add_func("str",		TypeString,			mf((tmf)&color::str));
	
	add_class(TypePlane);
		class_add_element("a",		TypeFloat,	0);
		class_add_element("b",		TypeFloat,	4);
		class_add_element("c",		TypeFloat,	8);
		class_add_element("d",		TypeFloat,	12);
		class_add_element("n",		TypeVector,	0);
		class_add_func("intersect_line",	TypeBool,	mf((tmf)&plane::intersect_line));
			func_add_param("l1",		TypeVector);
			func_add_param("l2",		TypeVector);
			func_add_param("inter",		TypeVector);
		class_add_func("inverse",	TypeVoid,	mf((tmf)&plane::inverse));
		class_add_func("distance",	TypeFloat,	mf((tmf)&plane::distance));
			func_add_param("p",		TypeVector);
		class_add_func("str",		TypeString,			mf((tmf)&plane::str));
	
	add_class(TypeMatrix);
		class_add_element("_00",	TypeFloat,	0);
		class_add_element("_10",	TypeFloat,	4);
		class_add_element("_20",	TypeFloat,	8);
		class_add_element("_30",	TypeFloat,	12);
		class_add_element("_01",	TypeFloat,	16);
		class_add_element("_11",	TypeFloat,	20);
		class_add_element("_21",	TypeFloat,	24);
		class_add_element("_31",	TypeFloat,	28);
		class_add_element("_02",	TypeFloat,	32);
		class_add_element("_12",	TypeFloat,	36);
		class_add_element("_22",	TypeFloat,	40);
		class_add_element("_32",	TypeFloat,	44);
		class_add_element("_03",	TypeFloat,	48);
		class_add_element("_13",	TypeFloat,	52);
		class_add_element("_23",	TypeFloat,	56);
		class_add_element("_33",	TypeFloat,	60);
		class_add_element("e",		TypeFloatArray4x4,	0);
		class_add_element("_e",		TypeFloatArray16,	0);
		class_add_func("__imul__",	TypeVoid,	mf((tmf)&matrix::imul));
			func_add_param("other",	TypeMatrix);
		class_add_func("__mul__",	TypeMatrix,	mf((tmf)&matrix::mul));
			func_add_param("other",	TypeMatrix);
		class_add_func("__mul__",	TypeVector,	mf((tmf)&matrix::mul_v));
			func_add_param("other",	TypeVector);
		class_add_func("str",		TypeString,			mf((tmf)&matrix::str));
	
	add_class(TypeMatrix3);
		class_add_element("_11",	TypeFloat,	0);
		class_add_element("_21",	TypeFloat,	4);
		class_add_element("_31",	TypeFloat,	8);
		class_add_element("_12",	TypeFloat,	12);
		class_add_element("_22",	TypeFloat,	16);
		class_add_element("_32",	TypeFloat,	20);
		class_add_element("_13",	TypeFloat,	24);
		class_add_element("_23",	TypeFloat,	28);
		class_add_element("_33",	TypeFloat,	32);
		class_add_element("e",		TypeFloatArray3x3,	0);
		class_add_element("_e",		TypeFloatArray9,	0);
		class_add_func("str",		TypeString,			mf((tmf)&matrix3::str));
	
	add_class(TypeVli);
		class_add_element("sign",	TypeBool,	0);
		class_add_element("data",	TypeIntList,	4);
		class_add_func("__init__",	TypeVoid, algebra_p(mf((tmf)&vli::__init__)));
		class_add_func("__delete__",	TypeVoid, algebra_p(mf((tmf)&vli::__delete__)));
		class_add_func("__assign__",			TypeVoid,			algebra_p(mf((tmf)&vli::set_vli)));
			func_add_param("v",			TypeVli);
		class_add_func("__assign__",			TypeVoid,			algebra_p(mf((tmf)&vli::set_str)));
			func_add_param("s",			TypeString);
		class_add_func("__assign__",			TypeVoid,			algebra_p(mf((tmf)&vli::set_int)));
			func_add_param("i",			TypeInt);
		class_add_func("str",		TypeString,			algebra_p(mf((tmf)&vli::to_string)));
		class_add_func("compare",			TypeInt,			algebra_p(mf((tmf)&vli::compare)));
			func_add_param("v",			TypeVli);
		class_add_func("__eq__",			TypeBool,			algebra_p(mf((tmf)&vli::operator==)));
			func_add_param("v",			TypeVli);
		class_add_func("__ne__",			TypeBool,			algebra_p(mf((tmf)&vli::operator!=)));
			func_add_param("v",			TypeVli);
		class_add_func("__lt__",			TypeBool,			algebra_p(mf((tmf)&vli::operator<)));
			func_add_param("v",			TypeVli);
		class_add_func("__gt__",			TypeBool,			algebra_p(mf((tmf)&vli::operator>)));
			func_add_param("v",			TypeVli);
		class_add_func("__le__",			TypeBool,			algebra_p(mf((tmf)&vli::operator<=)));
			func_add_param("v",			TypeVli);
		class_add_func("__ge__",			TypeBool,			algebra_p(mf((tmf)&vli::operator>=)));
			func_add_param("v",			TypeVli);
		class_add_func("__iadd__",			TypeVoid,			algebra_p(mf((tmf)&vli::operator+=)));
			func_add_param("v",			TypeVli);
		class_add_func("__isub__",			TypeVoid,			algebra_p(mf((tmf)&vli::operator-=)));
			func_add_param("v",			TypeVli);
		class_add_func("__imul__",			TypeVoid,			algebra_p(mf((tmf)&vli::operator*=)));
			func_add_param("v",			TypeVli);
		class_add_func("__add__",			TypeVli,			algebra_p(mf((tmf)&vli::operator+)));
			func_add_param("v",			TypeVli);
		class_add_func("__sub__",			TypeVli,			algebra_p(mf((tmf)&vli::operator-)));
			func_add_param("v",			TypeVli);
		class_add_func("__mul__",			TypeVli,			algebra_p(mf((tmf)&vli::operator*)));
			func_add_param("v",			TypeVli);
		class_add_func("idiv",			TypeVoid,			algebra_p(mf((tmf)&vli::idiv)));
			func_add_param("div",			TypeVli);
			func_add_param("rem",			TypeVli);
		class_add_func("div",			TypeVli,			algebra_p(mf((tmf)&vli::_div)));
			func_add_param("div",			TypeVli);
			func_add_param("rem",			TypeVli);
		class_add_func("pow",			TypeVli,			algebra_p(mf((tmf)&vli::pow)));
			func_add_param("exp",			TypeVli);
		class_add_func("pow_mod",		TypeVli,			algebra_p(mf((tmf)&vli::pow_mod)));
			func_add_param("exp",			TypeVli);
			func_add_param("mod",		TypeVli);
		class_add_func("gcd",			TypeVli,			algebra_p(mf((tmf)&vli::gcd)));
			func_add_param("v",			TypeVli);
	
	add_class(TypeAny);
		class_add_element("type",	TypeInt, 0);
		class_add_element("data",	TypePointer, 4);
		class_add_func("__init__",	TypeVoid, any_p(mf((tmf)&Any::__init__)));
		class_add_func("__delete__",	TypeVoid, any_p(mf((tmf)&Any::clear)));
		class_add_func("__assign__",			TypeVoid,			any_p(mf((tmf)&Any::set)));
			func_add_param("a",			TypeAny);
		class_add_func("__assign__",			TypeVoid,			any_p(mf((tmf)&Any::set_str)));
			func_add_param("s",			TypeString);
		class_add_func("__assign__",			TypeVoid,			any_p(mf((tmf)&Any::set_int)));
			func_add_param("i",			TypeInt);
		class_add_func("__assign__",			TypeVoid,			any_p(mf((tmf)&Any::set_float)));
			func_add_param("f",			TypeFloat);
		class_add_func("__assign__",			TypeVoid,			any_p(mf((tmf)&Any::set_bool)));
			func_add_param("b",			TypeBool);
		class_add_func("__iadd__",			TypeVoid,			any_p(mf((tmf)&Any::_add)));
			func_add_param("a",			TypeAny);
		class_add_func("__isub__",			TypeVoid,			any_p(mf((tmf)&Any::_sub)));
			func_add_param("a",			TypeAny);
		class_add_func("clear",	TypeVoid, any_p(mf((tmf)&Any::clear)));
		class_add_func("get",			TypeAny,			any_p(mf((tmf)&Any::get)));
			func_add_param("key",			TypeString);
		class_add_func("hset",			TypeVoid,			any_p(mf((tmf)&Any::hset)));
			func_add_param("key",			TypeString);
			func_add_param("value",			TypeAny);
		class_add_func("at",			TypeAny,			any_p(mf((tmf)&Any::at)));
			func_add_param("index",			TypeInt);
		class_add_func("aset",			TypeVoid,			any_p(mf((tmf)&Any::aset)));
			func_add_param("index",			TypeString);
			func_add_param("value",			TypeAny);
		class_add_func("bool",		TypeBool,			any_p(mf((tmf)&Any::_bool)));
		class_add_func("int",		TypeInt,			any_p(mf((tmf)&Any::_int)));
		class_add_func("float",		TypeFloat,			any_p(mf((tmf)&Any::_float)));
		class_add_func("str",		TypeString,			any_p(mf((tmf)&Any::str)));


	add_class(TypeCrypto);
		class_add_element("n",	TypeVli, 0);
		class_add_element("k",	TypeVli, sizeof(vli));
		class_add_func("__init__",	TypeVoid, algebra_p(mf((tmf)&Crypto::__init__)));
		class_add_func("str",		TypeString, algebra_p(mf((tmf)&Crypto::str)));
		class_add_func("from_str",	TypeVoid, algebra_p(mf((tmf)&Crypto::from_str)));
			func_add_param("str",		TypeString);
		class_add_func("Encrypt",	TypeString, algebra_p(mf((tmf)&Crypto::Encrypt)));
			func_add_param("str",		TypeString);
		class_add_func("Decrypt",	TypeString, algebra_p(mf((tmf)&Crypto::Decrypt)));
			func_add_param("str",		TypeString);
			func_add_param("cut",		TypeBool);
	
	add_func_special("complex",		TypeComplex,	CommandComplexSet);
		func_add_param("x",		TypeFloat);
		func_add_param("y",		TypeFloat);
	add_func_special("rect",		TypeRect,	CommandRectSet);
		func_add_param("x1",	TypeFloat);
		func_add_param("x2",	TypeFloat);
		func_add_param("y1",	TypeFloat);
		func_add_param("y2",	TypeFloat);

	
	add_class(TypeFloatInterpolator);
		class_add_element("type",	TypeInt, 0);
		class_add_func("__init__",	TypeVoid, mf((tmf)&Interpolator<float>::__init__));
		class_add_func("clear",	TypeVoid, mf((tmf)&Interpolator<float>::clear));
		class_add_func("set_type",	TypeVoid, mf((tmf)&Interpolator<float>::set_type));
			func_add_param("type",	TypeString);
		class_add_func("add",	TypeVoid, mf((tmf)&Interpolator<float>::add));
			func_add_param("p",	TypeFloatPs);
			func_add_param("dt",	TypeFloat);
		class_add_func("add2",	TypeVoid, mf((tmf)&Interpolator<float>::add2));
			func_add_param("p",	TypeFloatPs);
			func_add_param("v",	TypeFloatPs);
			func_add_param("dt",	TypeFloat);
		class_add_func("add3",	TypeVoid, mf((tmf)&Interpolator<float>::add3));
			func_add_param("p",	TypeFloatPs);
			func_add_param("v",	TypeFloatPs);
			func_add_param("w",	TypeFloat);
			func_add_param("dt",	TypeFloat);
		class_add_func("jump",	TypeVoid, mf((tmf)&Interpolator<float>::jump));
			func_add_param("p",	TypeFloatPs);
			func_add_param("v",	TypeFloatPs);
		class_add_func("get",	TypeFloat, mf((tmf)&Interpolator<float>::get));
			func_add_param("t",	TypeFloat);
		class_add_func("get_tang",	TypeFloat, mf((tmf)&Interpolator<float>::get_tang));
			func_add_param("t",	TypeFloat);
		class_add_func("get_list",	TypeFloatList, mf((tmf)&Interpolator<float>::get_list));
			func_add_param("t",	TypeFloatList);

	
	add_class(TypeVectorInterpolator);
		class_add_element("type",	TypeInt, 0);
		class_add_func("__init__",	TypeVoid, mf((tmf)&Interpolator<vector>::__init__));
		class_add_func("clear",	TypeVoid, mf((tmf)&Interpolator<vector>::clear));
		class_add_func("set_type",	TypeVoid, mf((tmf)&Interpolator<vector>::set_type));
			func_add_param("type",	TypeString);
		class_add_func("add",	TypeVoid, mf((tmf)&Interpolator<vector>::add));
			func_add_param("p",	TypeVector);
			func_add_param("dt",	TypeFloat);
		class_add_func("add2",	TypeVoid, mf((tmf)&Interpolator<vector>::add2));
			func_add_param("p",	TypeVector);
			func_add_param("v",	TypeVector);
			func_add_param("dt",	TypeFloat);
		class_add_func("add3",	TypeVoid, mf((tmf)&Interpolator<vector>::add3));
			func_add_param("p",	TypeVector);
			func_add_param("v",	TypeVector);
			func_add_param("w",	TypeFloat);
			func_add_param("dt",	TypeFloat);
		class_add_func("jump",	TypeVoid, mf((tmf)&Interpolator<vector>::jump));
			func_add_param("p",	TypeVector);
			func_add_param("v",	TypeVector);
		class_add_func("get",	TypeVector, mf((tmf)&Interpolator<vector>::get));
			func_add_param("t",	TypeFloat);
		class_add_func("get_tang",	TypeVector, mf((tmf)&Interpolator<vector>::get_tang));
			func_add_param("t",	TypeFloat);
		class_add_func("get_list",	TypeVectorList, mf((tmf)&Interpolator<vector>::get_list));
			func_add_param("t",	TypeFloatList);

	// mathematical
	add_func("sin",			TypeFloat,	(void*)&sinf);
		func_add_param("x",		TypeFloat);
	add_func("cos",			TypeFloat,	(void*)&cosf);
		func_add_param("x",		TypeFloat);
	add_func("tan",			TypeFloat,	(void*)&tanf);
		func_add_param("x",		TypeFloat);
	add_func("asin",		TypeFloat,	(void*)&asinf);
		func_add_param("x",		TypeFloat);
	add_func("acos",		TypeFloat,	(void*)&acosf);
		func_add_param("x",		TypeFloat);
	add_func("atan",		TypeFloat,	(void*)&atanf);
		func_add_param("x",		TypeFloat);
	add_func("atan2",		TypeFloat,	(void*)&atan2f);
		func_add_param("x",		TypeFloat);
		func_add_param("y",		TypeFloat);
	add_func("sqrt",		TypeFloat,	(void*)&sqrtf);
		func_add_param("x",		TypeFloat);
	add_func("sqr",			TypeFloat,		(void*)&f_sqr);
		func_add_param("x",		TypeFloat);
	add_func("exp",			TypeFloat,		(void*)&expf);
		func_add_param("x",		TypeFloat);
	add_func("log",			TypeFloat,		(void*)&logf);
		func_add_param("x",		TypeFloat);
	add_func("pow",			TypeFloat,		(void*)&powf);
		func_add_param("x",		TypeFloat);
		func_add_param("exp",	TypeFloat);
	add_func("clamp",		TypeFloat,		(void*)&clampf);
		func_add_param("f",		TypeFloat);
		func_add_param("min",	TypeFloat);
		func_add_param("max",	TypeFloat);
	add_func("loop",		TypeFloat,		(void*)&loopf);
		func_add_param("f",		TypeFloat);
		func_add_param("min",	TypeFloat);
		func_add_param("max",	TypeFloat);
	add_func("abs",			TypeFloat,		(void*)&fabsf);
		func_add_param("f",		TypeFloat);
	add_func("min",			TypeFloat,		(void*)&minf);
		func_add_param("a",		TypeFloat);
		func_add_param("b",		TypeFloat);
	add_func("max",			TypeFloat,		(void*)&maxf);
		func_add_param("a",		TypeFloat);
		func_add_param("b",		TypeFloat);
	// int
	add_func("clampi",		TypeInt,		(void*)&clampi);
		func_add_param("i",		TypeInt);
		func_add_param("min",	TypeInt);
		func_add_param("max",	TypeInt);
	add_func("loopi",		TypeInt,		(void*)&loopi);
		func_add_param("i",		TypeInt);
		func_add_param("min",	TypeInt);
		func_add_param("max",	TypeInt);
	// lists
	add_func("range",		TypeIntList,	(void*)&int_range);
		func_add_param("start",		TypeInt);
		func_add_param("end",		TypeInt);
	add_func("rangef",		TypeFloatList,	(void*)&float_range);
		func_add_param("start",		TypeFloat);
		func_add_param("end",		TypeFloat);
		func_add_param("step",		TypeFloat);
	// vectors
	add_func_special("vector",		TypeVector,	CommandVectorSet);
		func_add_param("x",		TypeFloat);
		func_add_param("y",		TypeFloat);
		func_add_param("z",		TypeFloat);
	add_func("VecAngAdd",			TypeVector,	(void*)&VecAngAdd);
		func_add_param("ang1",		TypeVector);
		func_add_param("ang2",		TypeVector);
	add_func("VecAngInterpolate",	TypeVector,	(void*)&VecAngInterpolate);
		func_add_param("ang1",		TypeVector);
		func_add_param("ang2",		TypeVector);
		func_add_param("t",			TypeFloat);
	add_func("VecDotProduct",		TypeFloat,	(void*)&VecDotProduct);
		func_add_param("v1",		TypeVector);
		func_add_param("v2",		TypeVector);
	add_func("VecCrossProduct",		TypeVector,	(void*)&VecCrossProduct);
		func_add_param("v1",		TypeVector);
		func_add_param("v2",		TypeVector);
	// matrices
	add_func("MatrixIdentity",		TypeVoid,	(void*)&MatrixIdentity);
		func_add_param("m_out",		TypeMatrix);
	add_func("MatrixTranslation",	TypeVoid,	(void*)&MatrixTranslation);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("trans",		TypeVector);
	add_func("MatrixRotation",		TypeVoid,	(void*)&MatrixRotation);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeVector);
	add_func("MatrixRotationX",		TypeVoid,	(void*)&MatrixRotationX);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeFloat);
	add_func("MatrixRotationY",		TypeVoid,	(void*)&MatrixRotationY);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeFloat);
	add_func("MatrixRotationZ",		TypeVoid,	(void*)&MatrixRotationZ);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeFloat);
	add_func("MatrixRotationQ",		TypeVoid,	(void*)&MatrixRotationQ);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeQuaternion);
	add_func("MatrixRotationView",	TypeVoid,	(void*)&MatrixRotationView);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("ang",		TypeVector);
	add_func("MatrixScale",			TypeVoid,	(void*)&MatrixScale);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("s_x",		TypeFloat);
		func_add_param("s_y",		TypeFloat);
		func_add_param("s_z",		TypeFloat);
	add_func("MatrixMultiply",		TypeVoid,	(void*)&MatrixMultiply);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("m2",		TypeMatrix);
		func_add_param("m1",		TypeMatrix);
	add_func("MatrixInverse",		TypeVoid,	(void*)&MatrixInverse);
		func_add_param("m_out",		TypeMatrix);
		func_add_param("m_in",		TypeMatrix);
	// quaternions
	add_func("QuaternionRotationV",	TypeVoid,	(void*)&QuaternionRotationV);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("ang",		TypeVector);
	add_func("QuaternionRotationA",	TypeVoid,	(void*)&QuaternionRotationA);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("axis",		TypeVector);
		func_add_param("angle",		TypeFloat);
	add_func("QuaternionScale",		TypeVoid,	(void*)&QuaternionScale);
		func_add_param("q",		TypeQuaternion);
		func_add_param("f",		TypeFloat);
	// plane
	add_func("PlaneFromPoints",	TypeVoid,	(void*)&PlaneFromPoints);
		func_add_param("pl",		TypePlane);
		func_add_param("a",		TypeVector);
		func_add_param("b",		TypeVector);
		func_add_param("c",		TypeVector);
	add_func("PlaneFromPointNormal",	TypeVoid,	(void*)&PlaneFromPointNormal);
		func_add_param("pl",		TypePlane);
		func_add_param("p",		TypeVector);
		func_add_param("n",		TypeVector);
	add_func("PlaneTransform",	TypeVoid,	(void*)&PlaneTransform);
		func_add_param("pl_out",		TypePlane);
		func_add_param("m",		TypeMatrix);
		func_add_param("pl_in",		TypePlane);
	// other types
	add_func("GetBaryCentric",	TypeVoid,	(void*)&GetBaryCentric);
		func_add_param("p",		TypeVector);
		func_add_param("a",		TypeVector);
		func_add_param("b",		TypeVector);
		func_add_param("c",		TypeVector);
		func_add_param("f",		TypeFloatPs);
		func_add_param("g",		TypeFloatPs);
	add_func_special("color",		TypeColor,	CommandColorSet);
		func_add_param("a",		TypeFloat);
		func_add_param("r",		TypeFloat);
		func_add_param("g",		TypeFloat);
		func_add_param("b",		TypeFloat);
	add_func("ColorSetHSB",			TypeColor,		(void*)&SetColorHSB);
		func_add_param("a",		TypeFloat);
		func_add_param("h",		TypeFloat);
		func_add_param("s",		TypeFloat);
		func_add_param("b",		TypeFloat);
	add_func("ColorInterpolate",	TypeColor,		(void*)&ColorInterpolate);
		func_add_param("c1",		TypeColor);
		func_add_param("c2",		TypeColor);
		func_add_param("t",		TypeFloat);
	add_func("CryptoCreateKeys",	TypeVoid,		algebra_p(&CryptoCreateKeys));
		func_add_param("c1",		TypeCrypto);
		func_add_param("c2",		TypeCrypto);
		func_add_param("type",		TypeString);
		func_add_param("bits",		TypeInt);
	// random numbers
	add_func("randi",			TypeInt,		(void*)&randi);
		func_add_param("max",	TypeInt);
	add_func("rand",			TypeFloat,		(void*)&randf);
		func_add_param("max",	TypeFloat);
	add_func("rand_seed",		TypeInt,		(void*)&srand);
		func_add_param("seed",	TypeInt);

	
	// float
	add_const("pi",  TypeFloat, *(void**)&pi);
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
	
	msg_db_l(3);
}

};
