#include "../file/file.h"
#include "script.h"
#include "../00_config.h"
#include "script_data_common.h"

#ifdef _X_USE_ALGEBRA_
	#include "../algebra/algebra.h"
#else
		typedef int vli;
#endif

#ifdef _X_USE_ALGEBRA_
	#define algebra_p(p)		(void*)p
#else
	#define algebra_p(p)		NULL
#endif

extern sType *TypeComplexList;
extern sType *TypeFloatList;
extern sType *TypeMatrix;
extern sType *TypePlane;
extern sType *TypeMatrix3;
extern sType *TypeIntList;
extern sType *TypeFloatPs;


float _cdecl f_sqr(float f){	return f*f;	}



float sum_float_list(Array<float> &a)
{
	float r = 0;
	for (int i=0;i<a.num;i++)
		r += a[i];
	return r;
}

int sum_int_list(Array<int> &a)
{
	int r = 0;
	for (int i=0;i<a.num;i++)
		r += a[i];
	return r;
}

float sum2_float_list(Array<float> &a)
{
	float r = 0;
	for (int i=0;i<a.num;i++)
		r += a[i] * a[i];
	return r;
}

complex sum_complex_list(Array<complex> &a)
{
	complex r = complex(0, 0);
	for (int i=0;i<a.num;i++)
		r += a[i];
	return r;
}

float sum2_complex_list(Array<complex> &a)
{
	float r = 0;
	for (int i=0;i<a.num;i++)
		r += a[i].x * a[i].x + a[i].y * a[i].y;
	return r;
}

CSuperArray int_range(int start, int end)
{
	CSuperArray a;
	//a.init_by_type(TypeInt); // done by kaba-constructors for temp variables
	for (int i=start;i<end;i++)
		a.append_4_single(i);
	return a;
}

CSuperArray float_range(float start, float end, float step)
{
	CSuperArray a;
	//a.init_by_type(TypeFloat); // done by kaba-constructors for temp variables
	//msg_write(a.element_size);
	for (float f=start;f<end;f+=step)
		a.append_4_single(*(int*)&f);
	return a;
}

void SIAddPackageMath()
{
	msg_db_r("SIAddPackageMath", 3);

	set_cur_package("math");

	// types
	TypeComplex		= add_type  ("complex",		sizeof(float) * 2);
	TypeComplexList		= add_type_a("complex[]",	TypeComplex, -1);
	TypeVector		= add_type  ("vector",		sizeof(vector));
	TypeRect		= add_type  ("rect",		sizeof(rect));
	TypeMatrix		= add_type  ("matrix",		sizeof(matrix));
	TypeQuaternion		= add_type  ("quaternion",	sizeof(quaternion));
	TypePlane		= add_type  ("plane",		sizeof(plane));
	TypeColor		= add_type  ("color",		sizeof(color));
	TypeMatrix3		= add_type  ("matrix3",		sizeof(matrix3));
	sType*
	TypeFloatArray3		= add_type_a("float[3]",	TypeFloat, 3);
	sType*
	TypeFloatArray4		= add_type_a("float[4]",	TypeFloat, 4);
	sType*
	TypeFloatArray4x4	= add_type_a("float[4][4]",	TypeFloatArray4, 4);
	sType*
	TypeFloatArray16	= add_type_a("float[16]",	TypeFloat, 16);
	sType*
	TypeFloatArray3x3	= add_type_a("float[3][3]",	TypeFloatArray3, 3);
	sType*
	TypeFloatArray9		= add_type_a("float[9]",	TypeFloat, 9);
	sType*
	TypeVli			= add_type  ("vli",		sizeof(vli));
	
	
	
	add_class(TypeComplex);
		class_add_element("x",		TypeFloat,	0);
		class_add_element("y",		TypeFloat,	4);
	
	add_class(TypeVector);
		class_add_element("x",		TypeFloat,	0);
		class_add_element("y",		TypeFloat,	4);
		class_add_element("z",		TypeFloat,	8);
	
	add_class(TypeQuaternion);
		class_add_element("x",		TypeFloat,	0);
		class_add_element("y",		TypeFloat,	4);
		class_add_element("z",		TypeFloat,	8);
		class_add_element("w",		TypeFloat,	12);
	
	add_class(TypeRect);
		class_add_element("x1",	TypeFloat,	0);
		class_add_element("x2",	TypeFloat,	4);
		class_add_element("y1",	TypeFloat,	8);
		class_add_element("y2",	TypeFloat,	12);
	
	add_class(TypeColor);
		class_add_element("a",		TypeFloat,	12);
		class_add_element("r",		TypeFloat,	0);
		class_add_element("g",		TypeFloat,	4);
		class_add_element("b",		TypeFloat,	8);
	
	add_class(TypePlane);
		class_add_element("a",		TypeFloat,	0);
		class_add_element("b",		TypeFloat,	4);
		class_add_element("c",		TypeFloat,	8);
		class_add_element("d",		TypeFloat,	12);
	
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
	
	add_class(TypeVli);
		class_add_element("dummy1",	TypeInt,	0);
		class_add_element("data",	TypeIntList,	0);
	
	add_func_special("complex",		TypeComplex,	CommandComplexSet);
		func_add_param("x",		TypeFloat);
		func_add_param("y",		TypeFloat);
	add_func_special("rect",		TypeRect,	CommandRectSet);
		func_add_param("x1",	TypeFloat);
		func_add_param("x2",	TypeFloat);
		func_add_param("y1",	TypeFloat);
		func_add_param("y2",	TypeFloat);

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
	add_func("sum",			TypeInt,	(void*)&sum_int_list);
		func_add_param("x",		TypeIntList);
	add_func("sumf",		TypeFloat,	(void*)&sum_float_list);
		func_add_param("x",		TypeFloatList);
	add_func("sum2f",		TypeFloat,	(void*)&sum2_float_list);
		func_add_param("x",		TypeFloatList);
	add_func("sumc",		TypeComplex,	(void*)&sum_complex_list);
		func_add_param("x",		TypeComplexList);
	add_func("sum2c",		TypeFloat,	(void*)&sum2_complex_list);
		func_add_param("x",		TypeComplexList);
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
	add_func("VecNormalize",		TypeVoid,	(void*)&VecNormalize);
		func_add_param("v",			TypeVector);
	add_func("VecDir2Ang",			TypeVector,	(void*)&VecDir2Ang);
		func_add_param("dir",		TypeVector);
	add_func("VecDir2Ang2",			TypeVector,	(void*)&VecDir2Ang2);
		func_add_param("dir",		TypeVector);
		func_add_param("up",		TypeVector);
	add_func("VecAng2Dir",			TypeVector,	(void*)&VecAng2Dir);
		func_add_param("ang",		TypeVector);
	add_func("VecAngAdd",			TypeVector,	(void*)&VecAngAdd);
		func_add_param("ang1",		TypeVector);
		func_add_param("ang2",		TypeVector);
	add_func("VecAngInterpolate",	TypeVector,	(void*)&VecAngInterpolate);
		func_add_param("ang1",		TypeVector);
		func_add_param("ang2",		TypeVector);
		func_add_param("t",			TypeFloat);
	add_func("VecRotate",			TypeVector,	(void*)&VecRotate);
		func_add_param("v",			TypeVector);
		func_add_param("ang",		TypeVector);
	add_func("VecLength",			TypeFloat,	(void*)&VecLength);
		func_add_param("v",			TypeVector);
	add_func("VecLengthSqr",		TypeFloat,	(void*)&VecLengthSqr);
		func_add_param("v",			TypeVector);
	add_func("VecLengthFuzzy",		TypeFloat,	(void*)&VecLengthFuzzy);
		func_add_param("v",			TypeVector);
	add_func("VecTransform",		TypeVoid,	(void*)&VecTransform);
		func_add_param("v_out",		TypeVector);
		func_add_param("m",			TypeMatrix);
		func_add_param("v_in",		TypeVector);
	add_func("VecNormalTransform",	TypeVoid,	(void*)&VecNormalTransform);
		func_add_param("v_out",		TypeVector);
		func_add_param("m",			TypeMatrix);
		func_add_param("v_in",		TypeVector);
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
	add_func("QuaternionMultiply",	TypeVoid,	(void*)&QuaternionMultiply);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("q2",		TypeQuaternion);
		func_add_param("q1",		TypeQuaternion);
	add_func("QuaternionInverse",	TypeVoid,	(void*)&QuaternionInverse);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("q_in",		TypeQuaternion);
	add_func("QuaternionScale",		TypeVoid,	(void*)&QuaternionScale);
		func_add_param("q",		TypeQuaternion);
		func_add_param("f",		TypeFloat);
	add_func("QuaternionNormalize",	TypeVoid,	(void*)&QuaternionNormalize);
		func_add_param("q_out",		TypeQuaternion);
		func_add_param("q_in",		TypeQuaternion);
	add_func("QuaternionToAngle",	TypeVector,	(void*)&QuaternionToAngle);
		func_add_param("q",		TypeQuaternion);
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
	add_func("PlaneGetNormal",	TypeVector,	(void*)&GetNormal);
		func_add_param("pl",		TypePlane);
	add_func("PlaneIntersectLine",	TypeBool,	(void*)&PlaneIntersectLine);
		func_add_param("inter",		TypeVector);
		func_add_param("pl",		TypePlane);
		func_add_param("l1",		TypeVector);
		func_add_param("l2",		TypeVector);
	add_func("PlaneInverse",	TypeVoid,	(void*)&PlaneInverse);
		func_add_param("pl",		TypePlane);
	add_func("PlaneDistance",	TypeFloat,	(void*)&PlaneDistance);
		func_add_param("pl",		TypePlane);
		func_add_param("p",		TypeVector);
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
	// random numbers
	add_func("randi",			TypeInt,		(void*)&randi);
		func_add_param("max",	TypeInt);
	add_func("rand",			TypeFloat,		(void*)&randf);
		func_add_param("max",	TypeFloat);
	add_func("rand_seed",		TypeInt,		(void*)&srand);
		func_add_param("seed",	TypeInt);

	
	
	add_func("vli_init",		TypeVoid,			algebra_p(&vli_init));
		func_add_param("v",			TypeVli);
	add_func("vli_set",			TypeVoid,			algebra_p(&vli_set));
		func_add_param("v",			TypeVli);
		func_add_param("w",			TypeVli);
	add_func("vli_set_str",		TypeVoid,			algebra_p(&vli_set_str));
		func_add_param("v",			TypeVli);
		func_add_param("str",		TypeString);
	add_func("vli_set_int",		TypeVoid,			algebra_p(&vli_set_int));
		func_add_param("v",			TypeVli);
		func_add_param("i",			TypeInt);
	add_func("vli_to_str",		TypeString,			algebra_p(&vli_to_str));
		func_add_param("v",			TypeVli);
	add_func("vli_cmp",			TypeInt,			algebra_p(&vli_cmp));
		func_add_param("v",			TypeVli);
		func_add_param("w",			TypeVli);
	add_func("vli_add",			TypeVoid,			algebra_p(&vli_add));
		func_add_param("a",			TypeVli);
		func_add_param("b",			TypeVli);
	add_func("vli_sub",			TypeVoid,			algebra_p(&vli_sub));
		func_add_param("a",			TypeVli);
		func_add_param("b",			TypeVli);
	add_func("vli_mul",			TypeVoid,			algebra_p(&vli_mul));
		func_add_param("a",			TypeVli);
		func_add_param("b",			TypeVli);
	add_func("vli_div",			TypeVoid,			algebra_p(&vli_div));
		func_add_param("a",			TypeVli);
		func_add_param("b",			TypeVli);
		func_add_param("r",			TypeVli);
	add_func("vli_exp",			TypeVoid,			algebra_p(&vli_exp));
		func_add_param("base",		TypeVli);
		func_add_param("e",			TypeVli);
		func_add_param("r",			TypeVli);
	add_func("vli_exp_mod",		TypeVoid,			algebra_p(&vli_exp_mod));
		func_add_param("base",		TypeVli);
		func_add_param("e",			TypeVli);
		func_add_param("mod",		TypeVli);
		func_add_param("r",			TypeVli);

	
	// float
	add_const("pi", TypeFloat, *(void**)&pi);
	// complex
	add_const("ci", TypeComplex, (void**)&ci);
	// vector
	add_const("v0",  TypeVector, (void*)&v0);
	add_const("e_x", TypeVector, (void*)&e_x);
	add_const("e_y", TypeVector, (void*)&e_y);
	add_const("e_z", TypeVector, (void*)&e_z);
	// matrix
	//add_const("MatrixID",TypeMatrix,(void*)&m_id);
	add_const("m_id", TypeMatrix, (void*)&m_id);
	// quaternion
	//add_const("QuaternionID",TypeVector,(void*)&q_id);
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
	//add_const("Rect01",TypeRect,(void*)&r01);
	add_const("r01", TypeRect, (void*)&r01);
	
	msg_db_l(3);
}
