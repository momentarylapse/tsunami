/*----------------------------------------------------------------------------*\
| Script Data                                                                  |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include <algorithm>
#include <string.h>

#ifdef WIN32
	#include "windows.h"
#endif
#include "script.h"
#include "../00_config.h"

string ScriptDataVersion = "0.9.4.0";


#ifdef _X_USE_HUI_
#include "../hui/hui.h"
#endif




int ScriptStackSize = SCRIPT_DEFAULT_STACK_SIZE;



//------------------------------------------------------------------------------------------------//
//                                             types                                              //
//------------------------------------------------------------------------------------------------//

sType *TypeUnknown;
sType *TypeReg32;
sType *TypeReg16;
sType *TypeReg8;
sType *TypeVoid;
sType *TypePointer;
sType *TypeClass;
sType *TypeBool;
sType *TypeInt;
sType *TypeFloat;
sType *TypeChar;
sType *TypeString;
sType *TypeCString;
sType *TypeSuperArray;

sType *TypeVector;
sType *TypeRect;
sType *TypeColor;
sType *TypeQuaternion;
 // internal:
sType *TypePointerPs;
sType *TypePointerList;
sType *TypeBoolList;
sType *TypeIntPs;
sType *TypeIntList;
sType *TypeIntArray;
sType *TypeFloatPs;
sType *TypeFloatList;
sType *TypeFloatArray;
sType *TypeFloatArrayP;
sType *TypeComplex;
sType *TypeComplexList;
sType *TypeStringList;
sType *TypeVectorArray;
sType *TypeVectorArrayP;
sType *TypeVectorList;
sType *TypeMatrix;
sType *TypePlane;
sType *TypePlaneList;
sType *TypeMatrix3;
sType *TypeDate;
sType *TypeImage;


Array<sPackage> Package;
sPackage *cur_package = NULL;
int cur_package_index = -1;


void set_cur_package(const char *name)
{
	cur_package_index = Package.num;
	sPackage p;
	p.name = name;
	Package.add(p);
	cur_package = &Package.back();
}

Array<sType*> PreType;
sType *add_type(const char *name, int size)
{
	msg_db_r("add_type", 4);
	sType *t = new sType;
	strcpy(t->Name, name);
	t->Owner = NULL;
	t->Size = size;
	t->IsArray = false;
	t->IsSuperArray = false;
	t->ArrayLength = 0;
	t->IsPointer = false;
	t->IsSilent = false;
	t->SubType = NULL;
	PreType.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}
sType *add_type_p(const char *name, sType *sub_type, bool is_silent = false)
{
	msg_db_r("add_type_p", 4);
	sType *t = new sType;
	strcpy(t->Name, name);
	t->Owner = NULL;
	t->Size = PointerSize;
	t->IsArray = false;
	t->IsSuperArray = false;
	t->ArrayLength = 0;
	t->IsPointer = true;
	t->IsSilent = is_silent;
	t->SubType = sub_type;
	PreType.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}
sType *add_type_a(const char *name, sType *sub_type, int array_length)
{
	msg_db_r("add_type_a", 4);
	sType *t = new sType;
	strcpy(t->Name, name);
	t->Owner = NULL;
	t->IsPointer = false;
	t->IsSilent = false;
	t->SubType = sub_type;
	if (array_length < 0){
		// super array
		t->Size = SuperArraySize;
		t->IsArray = false;
		t->IsSuperArray = true;
		t->ArrayLength = 0;
		//script_make_super_array(t); // do it later !!!
	}else{
		// standard array
		t->Size = sub_type->Size * array_length;
		t->IsArray = true;
		t->IsSuperArray = false;
		t->ArrayLength = array_length;
	}
	PreType.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}

sType *ScriptGetPreType(const char *name)
{
	for (int i=0;i<PreType.num;i++)
		if (strcmp(name, PreType[i]->Name) == 0)
			return PreType[i];
	return TypeUnknown;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")
enum{
	OperatorAssign,			//  =
	OperatorAdd,			//  +
	OperatorSubtract,		//  -
	OperatorMultiply,		//  *
	OperatorDivide,			//  /
	OperatorAddS,			// +=
	OperatorSubtractS,		// -=
	OperatorMultiplyS,		// *=
	OperatorDivideS,		// /=
	OperatorEqual,			// ==
	OperatorNotEqual,		// !=
	OperatorNegate,			//  !
	OperatorSmaller,		//  <
	OperatorGreater,		//  >
	OperatorSmallerEqual,	// <=
	OperatorGreaterEqual,	// >=
/*	OperatorAnd,			// &&
	OperatorOr,				// ||  */
	OperatorAndLiteral,		// and
	OperatorOrLiteral,		// or
	OperatorModulo,			//  %
	OperatorBitAnd,			//  &
	OperatorBitOr,			//  |
	OperatorShiftLeft,		// <<
	OperatorShiftRight,		// >>
	OperatorIncrease,		// ++
	OperatorDecrease,		// --
	NUM_PRIMITIVE_OPERATORS
};
int NumPrimitiveOperators = NUM_PRIMITIVE_OPERATORS;

sPrimitiveOperator PrimitiveOperator[NUM_PRIMITIVE_OPERATORS]={
	{"=",	OperatorAssign,			true,	1},
	{"+",	OperatorAdd,			false,	11},
	{"-",	OperatorSubtract,		false,	11},
	{"*",	OperatorMultiply,		false,	12},
	{"/",	OperatorDivide,			false,	12},
	{"+=",	OperatorAddS,			true,	1},
	{"-=",	OperatorSubtractS,		true,	1},
	{"*=",	OperatorMultiplyS,		true,	1},
	{"/=",	OperatorDivideS,		true,	1},
	{"==",	OperatorEqual,			false,	8},
	{"!=",	OperatorNotEqual,		false,	8},
	{"!",	OperatorNegate,			false,	2},
	{"<",	OperatorSmaller,		false,	9},
	{">",	OperatorGreater,		false,	9},
	{"<=",	OperatorSmallerEqual,	false,	9},
	{">=",	OperatorGreaterEqual,	false,	9},
/*	{"-&&",	OperatorAnd,			false,	4},
	{"-||",	OperatorOr,				false,	3},*/
	{"and",	OperatorAndLiteral,		false,	4},
	{"or",	OperatorOrLiteral,		false,	3},
	{"%",	OperatorModulo,			false,	12},
	{"&",	OperatorBitAnd,			false,	7},
	{"|",	OperatorBitOr,			false,	5},
	{"<<",	OperatorShiftLeft,		false,	10},
	{">>",	OperatorShiftRight,		false,	10},
	{"++",	OperatorIncrease,		true,	2},
	{"--",	OperatorDecrease,		true,	2}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

//   with type information

Array<sPreOperator> PreOperator;
int add_operator(int primitive_op, sType *return_type, sType *param_type1, sType *param_type2, void *func = NULL)
{
	msg_db_r("add_op", 4);
	sPreOperator o;
	o.PrimitiveID = primitive_op;
	o.ReturnType = return_type;
	o.ParamType1 = param_type1;
	o.ParamType2 = param_type2;
	o.Func = func;
	PreOperator.add(o);
	msg_db_l(4);
	return PreOperator.num - 1;
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//



sType *cur_class;

void add_class(sType *root_type)//, CPreScript *ps = NULL)
{
	msg_db_r("add_class", 4);
	cur_class = root_type;
	msg_db_l(4);
}

void class_add_element(const char *name, sType *type, int offset)
{
	msg_db_r("add_class_el", 4);
	sClassElement e;
	strcpy(e.Name, name);
	e.Type = type;
	e.Offset = offset;
	cur_class->Element.add(e);
	msg_db_l(4);
}

int add_func(const char *name, sType *return_type, void *func, bool is_class = false);

void class_add_func(const char *name, sType *return_type, void *func)
{
	msg_db_r("add_class_func", 4);
	char *tname = cur_class->Name;
	if (tname[0] == '-')
		for (int i=0;i<PreType.num;i++)
			if ((PreType[i]->IsPointer) && (PreType[i]->SubType == cur_class))
				tname = PreType[i]->Name;
	char *mname = new char[strlen(tname) + strlen(name) + 2];
	strcpy(mname, tname);
	strcat(mname, ".");
	strcat(mname, name);
	int cmd = add_func(mname, return_type, func, true);
	sClassFunction f;
	strcpy(f.Name, name);
	f.Kind = KindCompilerFunction;
	f.Nr = cmd;
	cur_class->Function.add(f);
	msg_db_l(4);
}


//------------------------------------------------------------------------------------------------//
//                                           constants                                            //
//------------------------------------------------------------------------------------------------//

Array<sPreConstant> PreConstant;
void add_const(const char *name, sType *type, void *value)
{
	msg_db_r("add_const", 4);
	sPreConstant c;
	c.Name = name;
	c.Type = type;
	c.Value = value;
	c.package = cur_package_index;
	PreConstant.add(c);
	msg_db_l(4);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//

Array<sPreExternalVar> PreExternalVar;

void add_ext_var(const char *name, sType *type, void *var)
{
	sPreExternalVar v;
	v.Name = name;
	v.Type = type;
	v.Pointer = var;
	v.IsSemiExternal = false;
	v.package = cur_package_index;
	PreExternalVar.add(v);
};

//------------------------------------------------------------------------------------------------//
//                                      compiler functions                                        //
//------------------------------------------------------------------------------------------------//



#ifndef FILE_OS_WINDOWS
	//#define _cdecl
	#include <stdlib.h>
#endif

//void _cdecl _stringout(char *str){	msg_write(string("StringOut: ",str));	}
void _cdecl _cstringout(char *str){	msg_write(str);	}
void _cdecl _stringout(string &str){	msg_write(str);	}
int _cdecl _Float2Int(float f){	return (int)f;	}


typedef void (CFile::*tmf)();
typedef char *tcpa[4];
void *mf(tmf vmf)
{
	tcpa *cpa=(tcpa*)&vmf;
	return (*cpa)[0];
}

void *f_cp = (void*)1; // for fake (compiler-) functions


Array<sPreCommand> PreCommand;

int cur_func;

int add_func(const char *name, sType *return_type, void *func, bool is_class)
{
	sPreCommand c;
	c.Name = name;
	c.ReturnType = return_type;
	c.Func = func;
	c.IsSpecial = false;
	c.IsClassFunction = is_class;
	c.IsSemiExternal = false;
	c.package = cur_package_index;
	if (PreCommand.num < NUM_INTERN_PRE_COMMANDS)
		PreCommand.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommand.add(c);
	cur_func = PreCommand.num - 1;
	return cur_func;
}

int add_func_special(const char *name, sType *return_type, int index)
{
	sPreCommand c;
	c.Name = name;
	c.ReturnType = return_type;
	c.Func = NULL;
	c.IsSpecial = true;
	c.IsClassFunction = false;
	c.IsSemiExternal = false;
	c.package = cur_package_index;
	if (PreCommand.num < NUM_INTERN_PRE_COMMANDS)
		PreCommand.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommand[index] = c;
	cur_func = index;
	return cur_func;
}

void func_add_param(const char *name, sType *type)
{
	sPreCommandParam p;
	p.Name = name;
	p.Type = type;
	PreCommand[cur_func].Param.add(p);
}

void CSuperArray::init_by_type(sType *t)
{	init(t->Size);	}

void CSuperArray::int_sort()
{	std::sort((int*)data, (int*)data + num);	}

void CSuperArray::int_unique()
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

void CSuperArray::float_sort()
{	std::sort((float*)data, (float*)data + num);	}


void super_array_assign(CSuperArray *a, CSuperArray *b)
{
	a->element_size = b->element_size; // ...
	a->reserve(b->num);
	memcpy(a->data, b->data, b->element_size * b->num);
	a->num = b->num;
}

// a += b
void super_array_add_s_int(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	for (int i=0;i<n;i++)	*(pa ++) += *(pb ++);	}
void super_array_sub_s_int(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	for (int i=0;i<n;i++)	*(pa ++) -= *(pb ++);	}
void super_array_mul_s_int(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	for (int i=0;i<n;i++)	*(pa ++) *= *(pb ++);	}
void super_array_div_s_int(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	for (int i=0;i<n;i++)	*(pa ++) /= *(pb ++);	}

// a = b + c
void super_array_add_int(CSuperArray *a, CSuperArray *b, CSuperArray *c)
{	int n = min(b->num, c->num);	a->resize(n);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	int *pc = (int*)c->data;	for (int i=0;i<n;i++)	*(pa ++) = *(pb ++) + *(pc ++);	}
void super_array_sub_int(CSuperArray *a, CSuperArray *b, CSuperArray *c)
{	int n = min(b->num, c->num);	a->resize(n);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	int *pc = (int*)c->data;	for (int i=0;i<n;i++)	*(pa ++) = *(pb ++) - *(pc ++);	}
void super_array_mul_int(CSuperArray *a, CSuperArray *b, CSuperArray *c)
{	int n = min(b->num, c->num);	a->resize(n);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	int *pc = (int*)c->data;	for (int i=0;i<n;i++)	*(pa ++) = *(pb ++) * *(pc ++);	}
void super_array_div_int(CSuperArray *a, CSuperArray *b, CSuperArray *c)
{	int n = min(b->num, c->num);	a->resize(n);	int *pa = (int*)a->data;	int *pb = (int*)b->data;	int *pc = (int*)c->data;	for (int i=0;i<n;i++)	*(pa ++) = *(pb ++) / *(pc ++);	}

// a += x
void super_array_add_s_int_int(CSuperArray *a, int x)
{	int *pa = (int*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) += x;	}
void super_array_sub_s_int_int(CSuperArray *a, int x)
{	int *pa = (int*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) -= x;	}
void super_array_mul_s_int_int(CSuperArray *a, int x)
{	int *pa = (int*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}
void super_array_div_s_int_int(CSuperArray *a, int x)
{	int *pa = (int*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) /= x;	}

void super_array_add_s_float(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	float *pa = (float*)a->data;	float *pb = (float*)b->data;	for (int i=0;i<n;i++)	*(pa ++) += *(pb ++);	}
void super_array_sub_s_float(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	float *pa = (float*)a->data;	float *pb = (float*)b->data;	for (int i=0;i<n;i++)	*(pa ++) -= *(pb ++);	}
void super_array_mul_s_float(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	float *pa = (float*)a->data;	float *pb = (float*)b->data;	for (int i=0;i<n;i++)	*(pa ++) *= *(pb ++);	}
void super_array_div_s_float(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	float *pa = (float*)a->data;	float *pb = (float*)b->data;	for (int i=0;i<n;i++)	*(pa ++) /= *(pb ++);	}

void super_array_add_s_float_float(CSuperArray *a, float x)
{	float *pa = (float*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) += x;	}
void super_array_sub_s_float_float(CSuperArray *a, float x)
{	float *pa = (float*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) -= x;	}
void super_array_mul_s_float_float(CSuperArray *a, float x)
{	float *pa = (float*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}
void super_array_div_s_float_float(CSuperArray *a, float x)
{	float *pa = (float*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) /= x;	}

void super_array_add_s_com(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) += *(pb ++);	}
void super_array_sub_s_com(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) -= *(pb ++);	}
void super_array_mul_s_com(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) *= *(pb ++);	}
void super_array_div_s_com(CSuperArray *a, CSuperArray *b)
{	int n = min(a->num, b->num);	complex *pa = (complex*)a->data;	complex *pb = (complex*)b->data;	for (int i=0;i<n;i++)	*(pa ++) /= *(pb ++);	}

void super_array_add_s_com_com(CSuperArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) += x;	}
void super_array_sub_s_com_com(CSuperArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) -= x;	}
void super_array_mul_s_com_com(CSuperArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}
void super_array_div_s_com_com(CSuperArray *a, complex x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) /= x;	}
void super_array_mul_s_com_float(CSuperArray *a, float x)
{	complex *pa = (complex*)a->data;	for (int i=0;i<a->num;i++)	*(pa ++) *= x;	}

void super_array_assign_8_single(CSuperArray *a, complex x)
{
	complex *p = (complex*)a->data;
	for (int i=0;i<a->num;i++)
		*(p ++) = x;
}

void super_array_assign_4_single(CSuperArray *a, int x)
{
	int *p = (int*)a->data;
	for (int i=0;i<a->num;i++)
		*(p ++) = x;
}

void super_array_assign_1_single(CSuperArray *a, char x)
{
	char *p = (char*)a->data;
	for (int i=0;i<a->num;i++)
		*(p ++) = x;
}

void super_array_add_s_str(string *a, string *b)
{
	a->append(b);
}

string super_array_add_str(string *a, string *b)
{
	string r;
	//r.init(1); // done by kaba-constructors for temp variables
	r.assign(a);
	r.append(b);
	return r;
}

void super_array_assign_str_cstr(string *a, char *b)
{
	int l = strlen(b);
	a->resize(l);
	memcpy((char*)a->data, b, l);
}

void super_array_add_str_cstr(string *a, char *b)
{
	int n_old = a->num;
	int l = strlen(b);
	a->resize(a->num + l);
	memcpy(&((char*)a->data)[n_old], b, l);
}

bool super_array_equal_str(string *a, string *b)
{	return *a == *b;	}

bool super_array_notequal_str(string *a, string *b)
{	return *a != *b;	}

class string_list : public Array<string>
{
};

void script_make_super_array(sType *t, CPreScript *ps)
{
	msg_db_r("make_super_array", 4);
	add_class(t);
		class_add_element("num", TypeInt, PointerSize);
		class_add_func("clear", TypeVoid, mf((tmf)&CSuperArray::clear));
		class_add_func("remove", TypeVoid, mf((tmf)&CSuperArray::delete_single));
			func_add_param("index",		TypeInt);
		class_add_func("removep", TypeVoid, mf((tmf)&CSuperArray::delete_single_by_pointer));
			func_add_param("pointer",		TypePointer);
		class_add_func("swap", TypeVoid, mf((tmf)&CSuperArray::swap));
			func_add_param("i1",		TypeInt);
			func_add_param("i2",		TypeInt);
		class_add_func("iterate", TypeBool, mf((tmf)&CSuperArray::iterate));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("iterate_back", TypeBool, mf((tmf)&CSuperArray::iterate_back));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("index", TypeInt, mf((tmf)&CSuperArray::index));
			func_add_param("pointer",		TypePointer);
		class_add_func("resize", TypeVoid, mf((tmf)&CSuperArray::resize));
			func_add_param("num",		TypeInt);
		class_add_func("ensure_size", TypeVoid, mf((tmf)&CSuperArray::ensure_size));
			func_add_param("num",		TypeInt);
		if (t->SubType->Size == 4){
			class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_4_single));
				func_add_param("x",		t->SubType);
		}else if (t->SubType->Size == 1){
			class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_1_single));
				func_add_param("x",		t->SubType);
		}else if (t->SubType == TypeComplex){
			class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_single));
				func_add_param("x",		t->SubType);
		}else if (t->SubType == TypeString){
			class_add_func("add", TypeVoid, mf((tmf)&string_list::add));
				func_add_param("x",		t->SubType);
		}else if ((t->SubType->IsArray) || (t->SubType->Element.num > 0)){
			class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_single));
				func_add_param("x",		t->SubType);
		}else{
			class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_single));
				func_add_param("x",		TypePointer);
		}
		if (t->SubType == TypeInt){
			class_add_func("sort", TypeVoid, mf((tmf)&CSuperArray::int_sort));
			class_add_func("unique", TypeVoid, mf((tmf)&CSuperArray::int_unique));
		}else if (t->SubType == TypeFloat){
			class_add_func("sort", TypeVoid, mf((tmf)&CSuperArray::float_sort));
		}else if (t->SubType == TypeChar){
			class_add_func("substr", TypeString, mf((tmf)&string::substr));
				func_add_param("start",		TypeInt);
				func_add_param("length",	TypeInt);
			class_add_func("find", TypeInt, mf((tmf)&string::find));
				func_add_param("str",		TypeString);
				func_add_param("start",		TypeInt);
			class_add_func("compare", TypeInt, mf((tmf)&string::compare));
				func_add_param("str",		TypeString);
			class_add_func("icompare", TypeInt, mf((tmf)&string::icompare));
				func_add_param("str",		TypeString);
			/*class_add_func("replace", TypeInt, mf((tmf)&string::replace));
				func_add_param("start",		TypeInt);
				func_add_param("length",	TypeInt);
				func_add_param("str",		TypeString);*/
			class_add_func("replace", TypeVoid, mf((tmf)&string::replace));
				func_add_param("sub",		TypeString);
				func_add_param("by",		TypeString);
			class_add_func("explode", TypeStringList, mf((tmf)&string::explode));
				func_add_param("str",		TypeString);
		}
		class_add_func("subarray", t, mf((tmf)&CSuperArray::ref_subarray));
			func_add_param("start",		TypeInt);
			func_add_param("num",		TypeInt);

	// until we have automatic initialization... (T_T)
		class_add_func("_manual_init_", TypeVoid, mf((tmf)&CSuperArray::init));
			func_add_param("element_size",		TypeInt);
	msg_db_l(4);
}


// automatic type casting

#define MAX_TYPE_CAST_BUFFER	32768
char type_cast_buffer[MAX_TYPE_CAST_BUFFER];
int type_cast_buffer_size = 0;

inline char *get_type_cast_buf(int size)
{
	char *str = &type_cast_buffer[type_cast_buffer_size];
	type_cast_buffer_size += size;
	if (type_cast_buffer_size >= MAX_TYPE_CAST_BUFFER){
		msg_error("Script: type_cast_buffer overflow");
		type_cast_buffer_size = 0;
		str = type_cast_buffer;
	}
	return str;
}


char CastTemp[256];
char *CastFloat2Int(float *f)
{
	*(int*)&CastTemp[0]=int(*f);
	return &CastTemp[0];
}
char *CastInt2Float(int *i)
{
	*(float*)&CastTemp[0]=float(*i);
	return &CastTemp[0];
}
char *CastInt2Char(int *i)
{
	*(char*)&CastTemp[0]=char(*i);
	return &CastTemp[0];
}
char *CastChar2Int(char *c)
{
	*(int*)&CastTemp[0]=int(*c);
	return &CastTemp[0];
}
char *CastPointer2Bool(void **p)
{
	*(bool*)&CastTemp[0]=( (*p) != NULL );
	return &CastTemp[0];
}
char *CastInt2StringP(int *i)
{
	string s = i2s(*i);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastFloat2StringP(float *f)
{
	string s = f2sf(*f);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastBool2StringP(bool *b)
{
	string s = b2s(*b);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	type_cast_buffer_size += strlen(str) + 1;
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastPointer2StringP(void *p)
{
	string s = p2s(p);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastVector2StringP(vector *v)
{
	string s = fff2s((float*)v);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastFFFF2StringP(quaternion *v)
{
	string s = ffff2s((float*)v);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastComplex2StringP(complex *z)
{
	string s = ff2s((float*)z);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}

Array<sTypeCast> TypeCast;
void add_type_cast(int penalty, sType *source, sType *dest, const char *cmd, void *func)
{
	sTypeCast c;
	c.Penalty = penalty;
	c.Command = -1;
	for (int i=0;i<PreCommand.num;i++)
		if (strcmp(PreCommand[i].Name, cmd) == 0){
			c.Command = i;
			break;
		}
	if (c.Command < 0){
#ifdef _X_USE_HUI_
		HuiErrorBox(NULL, "", "add_type_cast (ScriptInit): " + string(cmd) + " not found");
		HuiRaiseError("add_type_cast (ScriptInit): " + string(cmd) + " not found");
#else
		msg_error("add_type_cast (ScriptInit): " + string(cmd) + " not found"));
		exit(1);
#endif
	}
	c.Source = source;
	c.Dest = dest;
	c.Func = (t_cast_func*) func;
	TypeCast.add(c);
}



void SIAddPackageBase()
{
	msg_db_r("SIAddPackageBase", 3);

	set_cur_package("base");

	// internal
	TypeUnknown			= add_type  ("-\?\?\?-",	0); // should not appear anywhere....or else we're screwed up!
	TypeReg32			= add_type  ("-reg32-",		PointerSize);
	TypeReg16			= add_type  ("-reg16-",		PointerSize);
	TypeReg8			= add_type  ("-reg8-",		PointerSize);
	TypeClass			= add_type  ("-class-",	0); // substitute for all class types

	// "real"
	TypeVoid			= add_type  ("void",		0);
	TypeSuperArray		= add_type_a("void[]",		TypeVoid, -1); // substitute for all super arrays
	TypePointer			= add_type_p("void*",		TypeVoid); // substitute for all pointer types
	TypePointerPs		= add_type_p("void*&",		TypePointer, true);
	TypePointerList		= add_type_a("void*[]",		TypePointer, -1);
	TypeBool			= add_type  ("bool",		sizeof(bool));
	TypeBoolList		= add_type_a("bool[]",		TypeBool, -11);
	TypeInt				= add_type  ("int",			sizeof(int));
	TypeIntPs			= add_type_p("int&",		TypeInt, true);
	TypeIntList			= add_type_a("int[]",		TypeInt, -1);
	TypeIntArray		= add_type_a("int[?]",		TypeInt, 1);
	TypeFloat			= add_type  ("float",		sizeof(float));
	TypeFloatPs			= add_type_p("float&",		TypeFloat, true);
	TypeFloatArray		= add_type_a("float[?]",	TypeFloat, 1);
	TypeFloatArrayP		= add_type_p("float[?]*",	TypeFloatArray);
	TypeFloatList		= add_type_a("float[]",		TypeFloat, -1);
	TypeChar			= add_type  ("char",		sizeof(char));
	TypeCString			= add_type_a("cstring",		TypeChar, 256);	// cstring := char[256]
	TypeString			= add_type_a("string",		TypeChar, -1);	// string := char[]
	TypeStringList		= add_type_a("string[]",	TypeString, -1);



	add_const("nil", TypePointer, NULL);
	// bool
	add_const("false", TypeBool, (void*)false);
	add_const("true",  TypeBool, (void*)true);
	
	msg_db_l(3);
}


void SIAddBasicCommands()
{
	msg_db_r("SIAddBasicCommands", 3);

/*
	CommandReturn,
	CommandIf,
	CommandIfElse,
	CommandWhile,
	CommandFor,
	CommandBreak,
	CommandContinue,
	CommandSizeof,
	CommandWait,
	CommandWaitRT,
	CommandWaitOneFrame,
	CommandFloatToInt,
	CommandIntToFloat,
	CommandIntToChar,
	CommandCharToInt,
	CommandPointerToBool,
	CommandComplexSet,
	CommandVectorSet,
	CommandRectSet,
	CommandColorSet,
	CommandAsm,
*/


// "intern" functions
	add_func_special("return",		TypeVoid,	CommandReturn);
		func_add_param("return_value",	TypeVoid); // return: ParamType will be defined by the parser!
	add_func_special("-if-",		TypeVoid,	CommandIf);
		func_add_param("b",	TypeBool);
	add_func_special("-if/else-",	TypeVoid,	CommandIfElse);
		func_add_param("b",	TypeBool);
	add_func_special("-while-",		TypeVoid,	CommandWhile);
		func_add_param("b",	TypeBool);
	add_func_special("-for-",		TypeVoid,	CommandFor);
		func_add_param("b",	TypeBool); // internally like a while-loop... but a bit different...
	add_func_special("-break-",		TypeVoid,	CommandBreak);
	add_func_special("-continue-",	TypeVoid,	CommandContinue);
	add_func_special("sizeof",		TypeInt,	CommandSizeof);
		func_add_param("type",	TypeVoid);
	
	add_func_special("wait",		TypeVoid,	CommandWait);
		func_add_param("time",	TypeFloat);
	add_func_special("wait_rt",		TypeVoid,	CommandWaitRT);
		func_add_param("time",	TypeFloat);
	add_func_special("wait_of",		TypeVoid,	CommandWaitOneFrame);
//	add_func_special("f2i",			TypeInt,	(void*)&_Float2Int);
	add_func_special("f2i",			TypeInt,	CommandFloatToInt);    // sometimes causes floating point exceptions...
		func_add_param("f",		TypeFloat);
	add_func_special("i2f",			TypeFloat,	CommandIntToFloat);
		func_add_param("i",		TypeInt);
	add_func_special("i2c",			TypeChar,	CommandIntToChar);
		func_add_param("i",		TypeInt);
	add_func_special("c2i",			TypeInt,	CommandCharToInt);
		func_add_param("c",		TypeChar);
	add_func_special("p2b",			TypeBool,	CommandPointerToBool);
		func_add_param("p",		TypePointer);
	add_func_special("-asm-",		TypeVoid,	CommandAsm);
	
	msg_db_l(3);
}



void op_int_add(int &r, int &a, int &b)
{	r = a + b;	}
void op_int_sub(int &r, int &a, int &b)
{	r = a - b;	}
void op_int_mul(int &r, int &a, int &b)
{	r = a * b;	}
void op_int_div(int &r, int &a, int &b)
{	r = a / b;	}
void op_int_mod(int &r, int &a, int &b)
{	r = a % b;	}
void op_int_shr(int &r, int &a, int &b)
{	r = a >> b;	}
void op_int_shl(int &r, int &a, int &b)
{	r = a << b;	}
void op_float_add(float &r, float &a, float &b)
{	r = a + b;	}
void op_float_sub(float &r, float &a, float &b)
{	r = a - b;	}
void op_float_mul(float &r, float &a, float &b)
{	r = a * b;	}
void op_float_div(float &r, float &a, float &b)
{	r = a / b;	}

void SIAddOperators()
{
	msg_db_r("SIAddOperators", 3);
	

	// same order as in .h file...
	add_operator(OperatorAssign,		TypeVoid,		TypePointer,	TypePointer);
	add_operator(OperatorEqual,			TypeBool,		TypePointer,	TypePointer);
	add_operator(OperatorNotEqual,		TypeBool,		TypePointer,	TypePointer);
	add_operator(OperatorAssign,		TypeVoid,		TypeChar,		TypeChar);
	add_operator(OperatorEqual,			TypeBool,		TypeChar,		TypeChar);
	add_operator(OperatorNotEqual,		TypeBool,		TypeChar,		TypeChar);
	add_operator(OperatorAdd,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorSubtractS,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorAddS,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorSubtract,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorBitAnd,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorBitOr,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorSubtract,		TypeChar,		TypeVoid,		TypeChar);
	add_operator(OperatorAssign,		TypeVoid,		TypeBool,		TypeBool);
	add_operator(OperatorEqual,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorNotEqual,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorGreater,		TypeBool,		TypeBool,		TypeBool); // ???????? char? FIXME
	add_operator(OperatorGreaterEqual,	TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorSmaller,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorSmallerEqual,	TypeBool,		TypeBool,		TypeBool);
/*	add_operator(OperatorAnd,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorOr,			TypeBool,		TypeBool,		TypeBool);*/
	add_operator(OperatorAndLiteral,	TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorOrLiteral,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorNegate,		TypeBool,		TypeVoid,		TypeBool);	
	add_operator(OperatorAssign,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorAdd,			TypeInt,		TypeInt,		TypeInt,	(void*)op_int_add);
	add_operator(OperatorSubtract,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_sub);
	add_operator(OperatorMultiply,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_mul);
	add_operator(OperatorDivide,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_div);
	add_operator(OperatorAddS,			TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorDivideS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorModulo,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_mod);
	add_operator(OperatorEqual,			TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorNotEqual,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorGreater,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorGreaterEqual,	TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorSmaller,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorSmallerEqual,	TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorBitAnd,		TypeInt,		TypeInt,		TypeInt);
	add_operator(OperatorBitOr,			TypeInt,		TypeInt,		TypeInt);
	add_operator(OperatorShiftRight,	TypeInt,		TypeInt,		TypeInt,	(void*)op_int_shr);
	add_operator(OperatorShiftLeft,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_shl);
	add_operator(OperatorSubtract,		TypeInt,		TypeVoid,		TypeInt);
	add_operator(OperatorIncrease,		TypeVoid,		TypeInt,		TypeVoid);
	add_operator(OperatorDecrease,		TypeVoid,		TypeInt,		TypeVoid);
	add_operator(OperatorAssign,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorAdd,			TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_add);
	add_operator(OperatorSubtract,		TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_sub);
	add_operator(OperatorMultiply,		TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_mul);
	add_operator(OperatorMultiply,		TypeFloat,		TypeFloat,		TypeInt);
	add_operator(OperatorMultiply,		TypeFloat,		TypeInt,		TypeFloat);
	add_operator(OperatorDivide,		TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_div);
	add_operator(OperatorAddS,			TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorDivideS,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorEqual,			TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorNotEqual,		TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorGreater,		TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorGreaterEqual,	TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorSmaller,		TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorSmallerEqual,	TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorSubtract,		TypeFloat,		TypeVoid,		TypeFloat);
//	add_operator(OperatorAssign,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorAdd,			TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorSubtract,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorMultiply,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorMultiply,		TypeComplex,	TypeFloat,		TypeComplex);
	add_operator(OperatorMultiply,		TypeComplex,	TypeComplex,	TypeFloat);
	add_operator(OperatorDivide,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorAddS,			TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorDivideS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorEqual,			TypeBool,		TypeComplex,	TypeComplex);
	add_operator(OperatorSubtract,		TypeComplex,	TypeVoid,		TypeComplex);
	add_operator(OperatorAssign,		TypeVoid,		TypeCString,	TypeCString);
	add_operator(OperatorAdd,			TypeCString,	TypeCString,	TypeCString);
	add_operator(OperatorAddS,			TypeVoid,		TypeCString,	TypeCString);
	add_operator(OperatorEqual,			TypeBool,		TypeCString,	TypeCString);
	add_operator(OperatorNotEqual,		TypeBool,		TypeCString,	TypeCString);
	add_operator(OperatorAssign,		TypeVoid,		TypeClass,		TypeClass);
	add_operator(OperatorEqual,			TypeBool,		TypeClass,		TypeClass);
	add_operator(OperatorNotEqual,		TypeBool,		TypeClass,		TypeClass);
	add_operator(OperatorAdd,			TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorSubtract,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorMultiply,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorMultiply,		TypeVector,		TypeVector,		TypeFloat);
	add_operator(OperatorMultiply,		TypeVector,		TypeFloat,		TypeVector);
	add_operator(OperatorDivide,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorDivide,		TypeVector,		TypeVector,		TypeFloat);
	add_operator(OperatorAddS,			TypeVoid,		TypeVector,		TypeVector);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeVector,		TypeVector);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeVector,		TypeFloat);
	add_operator(OperatorDivideS,		TypeVoid,		TypeVector,		TypeFloat);
	add_operator(OperatorSubtract,		TypeVector,		TypeVoid,		TypeVector);

	add_operator(OperatorAddS,			TypeVoid,		TypeString,		TypeString);
	add_operator(OperatorAdd,			TypeString,		TypeString,		TypeString);
	add_operator(OperatorAssign,		TypeVoid,		TypeString,		TypeCString);
	add_operator(OperatorAddS,			TypeVoid,		TypeString,		TypeCString);
	add_operator(OperatorEqual,			TypeBool,		TypeString,		TypeString);
	add_operator(OperatorNotEqual,		TypeBool,		TypeString,		TypeString);
	
	msg_db_l(3);
}

void SIAddSuperArrays()
{
	msg_db_r("SIAddSuperArrays", 3);

	for (int i=0;i<PreType.num;i++)
		if (PreType[i]->IsSuperArray){
			//msg_error(string("super array:  ", PreType[i]->Name));
			script_make_super_array(PreType[i]);
		}
	
	add_operator(OperatorAssign,		TypeVoid,		TypeSuperArray,	TypeSuperArray);
	add_operator(OperatorAddS,			TypeVoid,		TypeIntList,	TypeIntList);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeIntList,	TypeIntList);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeIntList,	TypeIntList);
	add_operator(OperatorDivideS,		TypeVoid,		TypeIntList,	TypeIntList);
	/*add_operator(OperatorAdd,			TypeIntList,	TypeIntList,	TypeIntList);
	add_operator(OperatorSubtract,		TypeIntList,	TypeIntList,	TypeIntList);
	add_operator(OperatorMultiply,		TypeIntList,	TypeIntList,	TypeIntList);
	add_operator(OperatorDivide,		TypeIntList,	TypeIntList,	TypeIntList);*/
	add_operator(OperatorAssign,		TypeVoid,		TypeIntList,	TypeInt);
	add_operator(OperatorAddS,			TypeVoid,		TypeIntList,	TypeInt);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeIntList,	TypeInt);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeIntList,	TypeInt);
	add_operator(OperatorDivideS,		TypeVoid,		TypeIntList,	TypeInt);
	/*add_operator(OperatorAdd,			TypeIntList,	TypeIntList,	TypeInt);
	add_operator(OperatorSubtract,		TypeIntList,	TypeIntList,	TypeInt);
	add_operator(OperatorMultiply,		TypeIntList,	TypeIntList,	TypeInt);
	add_operator(OperatorDivide,		TypeIntList,	TypeIntList,	TypeInt);*/
	add_operator(OperatorAddS,			TypeVoid,		TypeFloatList,	TypeFloatList);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeFloatList,	TypeFloatList);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeFloatList,	TypeFloatList);
	add_operator(OperatorDivideS,		TypeVoid,		TypeFloatList,	TypeFloatList);
	add_operator(OperatorAssign,		TypeVoid,		TypeFloatList,	TypeFloat);
	add_operator(OperatorAddS,			TypeVoid,		TypeFloatList,	TypeFloat);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeFloatList,	TypeFloat);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeFloatList,	TypeFloat);
	add_operator(OperatorDivideS,		TypeVoid,		TypeFloatList,	TypeFloat);
	add_operator(OperatorAddS,			TypeVoid,		TypeComplexList,TypeComplexList);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeComplexList,TypeComplexList);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeComplexList,TypeComplexList);
	add_operator(OperatorDivideS,		TypeVoid,		TypeComplexList,TypeComplexList);
	add_operator(OperatorAssign,		TypeVoid,		TypeComplexList,TypeComplex);
	add_operator(OperatorAddS,			TypeVoid,		TypeComplexList,TypeComplex);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeComplexList,TypeComplex);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeComplexList,TypeComplex);
	add_operator(OperatorDivideS,		TypeVoid,		TypeComplexList,TypeComplex);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeComplexList,TypeFloat);
	
	msg_db_l(3);
}

void SIAddCommands()
{
	msg_db_r("SIAddCommands", 3);
	
	// type casting
	add_func("s2i",				TypeInt,		(void*)&s2i);
		func_add_param("s",		TypeString);
	add_func("s2f",				TypeFloat,		(void*)&s2f);
		func_add_param("s",		TypeString);
	add_func("i2s",				TypeString,	(void*)&i2s);
		func_add_param("i",		TypeInt);
	add_func("f2s",				TypeString,		(void*)&f2s);
		func_add_param("f",			TypeFloat);
		func_add_param("decimals",	TypeInt);
	add_func("f2sf",			TypeString,		(void*)&f2sf);
		func_add_param("f",			TypeFloat);
	add_func("b2s",				TypeString,	(void*)&b2s);
		func_add_param("b",		TypeBool);
	add_func("p2s",				TypeString,	(void*)&p2s);
		func_add_param("p",		TypePointer);
	add_func("v2s",				TypeString,	(void*)&fff2s);
		func_add_param("v",		TypeVector);
	add_func("complex2s",		TypeString,	(void*)&ff2s);
		func_add_param("z",		TypeComplex);
	add_func("quaternion2s",	TypeString,	(void*)&ffff2s);
		func_add_param("q",		TypeQuaternion);
	add_func("plane2s",			TypeString,	(void*)&ffff2s);
		func_add_param("p",		TypePlane);
	add_func("color2s",			TypeString,	(void*)&ffff2s);
		func_add_param("c",		TypeColor);
	add_func("rect2s",			TypeString,	(void*)&ffff2s);
		func_add_param("r",		TypeRect);
	add_func("ia2s",			TypeString,	(void*)&ia2s);
		func_add_param("a",		TypeIntList);
	add_func("fa2s",			TypeString,	(void*)&fa2s);
		func_add_param("a",		TypeFloatList);
	add_func("ba2s",			TypeString,	(void*)&ba2s);
		func_add_param("a",		TypeBoolList);
	add_func("sa2s",			TypeString,	(void*)&sa2s);
		func_add_param("a",		TypeStringList);
	add_func("hash_func",		TypeInt, (void*)&hash_func);
		func_add_param("s",		TypeString);
	// debug output
	/*add_func("cprint",			TypeVoid,		(void*)&_cstringout);
		func_add_param("str",	TypeCString);*/
	add_func("print",			TypeVoid,		(void*)&_stringout);
		func_add_param("str",	TypeString);
	// memory
	add_func("_malloc_",			TypePointer,		(void*)&malloc);
		func_add_param("size",	TypeInt);
	add_func("_free_",			TypeVoid,		(void*)&free);
		func_add_param("p",	TypePointer);
	// system
	add_func("_exec_",			TypeString,		(void*)&shell_execute);
		func_add_param("cmd",	TypeString);


// add_func("ExecuteScript",	TypeVoid);
//		func_add_param("filename",		TypeString);
	
	msg_db_l(3);
}

void SIAddPackageFile();
void SIAddPackageMath();
void SIAddPackageThread();
void SIAddPackageHui();
void SIAddPackageNix();
void SIAddPackageNet();
void SIAddPackageImage();
void SIAddPackageSound();
void SIAddPackageX();

void ScriptInit()
{
	msg_db_r("ScriptInit", 1);

	AsmInit();

	SIAddPackageBase();
	SIAddBasicCommands();




	SIAddPackageFile();
	SIAddPackageMath();
	SIAddPackageImage();
	SIAddPackageHui();
	SIAddPackageNix();
	SIAddPackageNet();
	SIAddPackageSound();
	SIAddPackageThread();
	SIAddPackageX();

	cur_package_index = 0;
	cur_package = &Package[0];
	SIAddCommands();
	
	SIAddOperators();
	SIAddSuperArrays();





	add_type_cast(10,	TypeInt,		TypeFloat,	"i2f",	(void*)&CastInt2Float);
	add_type_cast(20,	TypeFloat,		TypeInt,	"f2i",	(void*)&CastFloat2Int);
	add_type_cast(10,	TypeInt,		TypeChar,	"i2c",	(void*)&CastInt2Char);
	add_type_cast(20,	TypeChar,		TypeInt,	"c2i",	(void*)&CastChar2Int);
	add_type_cast(50,	TypePointer,	TypeBool,	"p2b",	(void*)&CastPointer2Bool);
	add_type_cast(50,	TypeInt,		TypeString,	"i2s",	(void*)&CastInt2StringP);
	add_type_cast(50,	TypeFloat,		TypeString,	"f2sf",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeBool,		TypeString,	"b2s",	(void*)&CastBool2StringP);
	add_type_cast(50,	TypePointer,	TypeString,	"p2s",	(void*)&CastPointer2StringP);
	add_type_cast(50,	TypeVector,		TypeString,	"v2s",	(void*)&CastVector2StringP);
	add_type_cast(50,	TypeComplex,	TypeString,	"complex2s",	(void*)&CastComplex2StringP);
	add_type_cast(50,	TypeColor,		TypeString,	"color2s",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeQuaternion,	TypeString,	"quaternion2s",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypePlane,		TypeString,	"plane2s",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeRect,		TypeString,	"rect2s",	(void*)&CastFFFF2StringP);
	//add_type_cast(50,	TypeClass,		TypeString,	"f2s",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeIntList,	TypeString,	"ia2s",	NULL);
	add_type_cast(50,	TypeFloatList,	TypeString,	"fa2s",	NULL);
	add_type_cast(50,	TypeBoolList,	TypeString,	"ba2s",	NULL);
	add_type_cast(50,	TypeStringList,	TypeString,	"sa2s",	NULL);

	/*msg_write("------------------test");
	foreach(PreType, t){
		if (t->SubType)
			msg_write(t->SubType->Name);
	}
	foreach(PreCommand, c){
		msg_write("-----");
		msg_write(c.Name);
		msg_write(c.ReturnType->Name);
		foreach(c.Param, p)
			msg_write(p.Type->Name);
	}
	foreach(PreExternalVar, v){
		msg_write(v.Name);
		msg_write(v.Type->Name);
	}*/


	msg_db_l(1);
}

void ScriptResetSemiExternalData()
{
	msg_db_r("ScriptResetSemiExternalData", 2);
	for (int i=PreExternalVar.num-1;i>=0;i--)
		if (PreExternalVar[i].IsSemiExternal){
			delete[](PreExternalVar[i].Name);
			PreExternalVar.erase(i);
		}
	for (int i=PreCommand.num-1;i>=0;i--)
		if (PreCommand[i].IsSemiExternal){
			delete[](PreCommand[i].Name);
			PreCommand.erase(i);
		}
	msg_db_l(2);
}

// program variables - specific to the surrounding program, can't always be there...
void ScriptLinkSemiExternalVar(const char *name, void *pointer)
{
	msg_db_r("ScriptLinkSemiExternalVar", 2);
	sPreExternalVar v;
	v.Name = new char[strlen(name) + 1];
	strcpy((char*)v.Name, (char*)name);
	v.Pointer = pointer;
	v.Type = TypeUnknown; // unusable until defined via "extern" in the script!
	v.IsSemiExternal = true; // ???
	PreExternalVar.add(v);
	msg_db_l(2);
}

// program functions - specific to the surrounding program, can't always be there...
void ScriptLinkSemiExternalFunc(const char *name, void *pointer)
{
	sPreCommand c;
	c.Name = new char[strlen(name) + 1];
	strcpy((char*)c.Name, (char*)name);
	c.IsClassFunction = false;
	c.Func = pointer;
	c.ReturnType = TypeUnknown; // unusable until defined via "extern" in the script!
	c.IsSemiExternal = true;
	PreCommand.add(c);
}	

Array<sScriptLocation> ScriptLocation;

void ScriptEnd()
{
	msg_db_r("ScriptEnd", 1);
	DeleteAllScripts(true, true);

	ScriptResetSemiExternalData();
	
	// locations
	ScriptLocation.clear();

	PreOperator.clear();

	for (int i=0;i<PreType.num;i++){
		PreType[i]->Element.clear();
		for (int j=0;j<PreType[i]->Function.num;j++)
			if (PreType[i]->Function[j].Kind == KindCompilerFunction) // always true...
				delete[](PreCommand[PreType[i]->Function[j].Nr].Name);
		PreType[i]->Function.clear();
		delete(PreType[i]);
	}
	PreType.clear();

	PreConstant.clear();
	PreExternalVar.clear();
	msg_db_l(1);
}

