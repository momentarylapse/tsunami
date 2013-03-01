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
#include "script_data_common.h"
#include "../00_config.h"



#ifdef _X_USE_HUI_
#include "../hui/hui.h"
#endif



namespace Script{

string DataVersion = "0.10.5.0";



int StackSize = SCRIPT_DEFAULT_STACK_SIZE;



//------------------------------------------------------------------------------------------------//
//                                             types                                              //
//------------------------------------------------------------------------------------------------//

Type *TypeUnknown;
Type *TypeReg32;
Type *TypeReg16;
Type *TypeReg8;
Type *TypeVoid;
Type *TypePointer;
Type *TypeClass;
Type *TypeBool;
Type *TypeInt;
Type *TypeFloat;
Type *TypeChar;
Type *TypeString;
Type *TypeCString;
Type *TypeSuperArray;

Type *TypeVector;
Type *TypeRect;
Type *TypeColor;
Type *TypeQuaternion;
 // internal:
Type *TypePointerPs;
Type *TypePointerList;
Type *TypeCharPs;
Type *TypeBoolPs;
Type *TypeBoolList;
Type *TypeIntPs;
Type *TypeIntList;
Type *TypeIntArray;
Type *TypeFloatPs;
Type *TypeFloatList;
Type *TypeFloatArray;
Type *TypeFloatArrayP;
Type *TypeComplex;
Type *TypeComplexList;
Type *TypeStringList;
Type *TypeVectorArray;
Type *TypeVectorArrayP;
Type *TypeVectorList;
Type *TypeMatrix;
Type *TypePlane;
Type *TypePlaneList;
Type *TypeMatrix3;
Type *TypeDate;
Type *TypeImage;


Array<Package> Packages;
Package *cur_package = NULL;
int cur_package_index = -1;


void set_cur_package(const string &name)
{
	cur_package_index = Packages.num;
	Package p;
	p.name = name;
	Packages.add(p);
	cur_package = &Packages.back();
}

Array<Type*> PreTypes;
Type *add_type(const string &name, int size, TypeFlag flag)
{
	msg_db_r("add_type", 4);
	Type *t = new Type;
	t->name = name;
	t->size = size;
	if ((flag & FLAG_CALL_BY_VALUE) > 0)
		t->force_call_by_value = true;
	PreTypes.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}
Type *add_type_p(const string &name, Type *sub_type, TypeFlag flag)
{
	msg_db_r("add_type_p", 4);
	Type *t = new Type;
	t->name = name;
	t->size = PointerSize;
	t->is_pointer = true;
	if ((flag & FLAG_SILENT) > 0)
		t->is_silent = true;
	t->parent = sub_type;
	PreTypes.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}
Type *add_type_a(const string &name, Type *sub_type, int array_length)
{
	msg_db_r("add_type_a", 4);
	Type *t = new Type;
	t->name = name;
	t->parent = sub_type;
	if (array_length < 0){
		// super array
		t->size = SuperArraySize;
		t->is_super_array = true;
		//script_make_super_array(t); // do it later !!!
	}else{
		// standard array
		t->size = sub_type->size * array_length;
		t->is_array = true;
		t->array_length = array_length;
	}
	PreTypes.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}

Type *ScriptGetPreType(const string &name)
{
	for (int i=0;i<PreTypes.num;i++)
		if (name == PreTypes[i]->name)
			return PreTypes[i];
	return TypeUnknown;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")
int NumPrimitiveOperators = NUM_PRIMITIVE_OPERATORS;

PrimitiveOperator PrimitiveOperators[NUM_PRIMITIVE_OPERATORS]={
	{"=",	OperatorAssign,			true,	1,	"__assign__"},
	{"+",	OperatorAdd,			false,	11,	"__add__"},
	{"-",	OperatorSubtract,		false,	11,	"__sub__"},
	{"*",	OperatorMultiply,		false,	12,	"__mul__"},
	{"/",	OperatorDivide,			false,	12,	"__div__"},
	{"+=",	OperatorAddS,			true,	1,	"__iadd__"},
	{"-=",	OperatorSubtractS,		true,	1,	"__isub__"},
	{"*=",	OperatorMultiplyS,		true,	1,	"__imul__"},
	{"/=",	OperatorDivideS,		true,	1,	"__idiv__"},
	{"==",	OperatorEqual,			false,	8,	"__eq__"},
	{"!=",	OperatorNotEqual,		false,	8,	"__ne__"},
	{"!",	OperatorNegate,			false,	2,	"__not__"},
	{"<",	OperatorSmaller,		false,	9,	"__lt__"},
	{">",	OperatorGreater,		false,	9,	"__gt__"},
	{"<=",	OperatorSmallerEqual,	false,	9,	"__le__"},
	{">=",	OperatorGreaterEqual,	false,	9,	"__ge__"},
	{"and",	OperatorAnd,			false,	4,	"__and__"},
	{"or",	OperatorOr,				false,	3,	"__or__"},
	{"%",	OperatorModulo,			false,	12,	"__mod__"},
	{"&",	OperatorBitAnd,			false,	7,	"__bitand__"},
	{"|",	OperatorBitOr,			false,	5,	"__bitor__"},
	{"<<",	OperatorShiftLeft,		false,	10,	"__lshift__"},
	{">>",	OperatorShiftRight,		false,	10,	"__rshift__"},
	{"++",	OperatorIncrease,		true,	2,	"__inc__"},
	{"--",	OperatorDecrease,		true,	2,	"__dec__"}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

//   with type information

Array<PreOperator> PreOperators;
int add_operator(int primitive_op, Type *return_type, Type *param_type1, Type *param_type2, void *func = NULL)
{
	msg_db_r("add_op", 4);
	PreOperator o;
	o.primitive_id = primitive_op;
	o.return_type = return_type;
	o.param_type_1 = param_type1;
	o.param_type_2 = param_type2;
	o.func = func;
	PreOperators.add(o);
	msg_db_l(4);
	return PreOperators.num - 1;
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//



Type *cur_class;
ClassFunction *cur_class_func = NULL;

void add_class(Type *root_type)//, PreScript *ps = NULL)
{
	msg_db_r("add_class", 4);
	cur_class = root_type;
	msg_db_l(4);
}

void class_add_element(const string &name, Type *type, int offset)
{
	msg_db_r("add_class_el", 4);
	ClassElement e;
	e.name = name;
	e.type = type;
	e.offset = offset;
	cur_class->element.add(e);
	msg_db_l(4);
}

int add_func(const string &name, Type *return_type, void *func, bool is_class);

void class_add_func(const string &name, Type *return_type, void *func)
{
	msg_db_r("add_class_func", 4);
	string tname = cur_class->name;
	if (tname[0] == '-')
		for (int i=0;i<PreTypes.num;i++)
			if ((PreTypes[i]->is_pointer) && (PreTypes[i]->parent == cur_class))
				tname = PreTypes[i]->name;
	int cmd = add_func(tname + "." + name, return_type, func, true);
	ClassFunction f;
	f.name = name;
	f.kind = KindCompilerFunction;
	f.nr = cmd;
	f.return_type = return_type;
	cur_class->function.add(f);
	cur_class_func = &cur_class->function.back();
	msg_db_l(4);
}


//------------------------------------------------------------------------------------------------//
//                                           constants                                            //
//------------------------------------------------------------------------------------------------//

Array<PreConstant> PreConstants;
void add_const(const string &name, Type *type, void *value)
{
	msg_db_r("add_const", 4);
	PreConstant c;
	c.name = name;
	c.type = type;
	c.value = value;
	c.package = cur_package_index;
	PreConstants.add(c);
	msg_db_l(4);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//

Array<PreExternalVar> PreExternalVars;

void add_ext_var(const string &name, Type *type, void *var)
{
	PreExternalVar v;
	v.name = name;
	v.type = type;
	v.pointer = var;
	v.is_semi_external = false;
	v.package = cur_package_index;
	PreExternalVars.add(v);
};

//------------------------------------------------------------------------------------------------//
//                                      compiler functions                                        //
//------------------------------------------------------------------------------------------------//



#ifndef OS_WINDOWS
	//#define _cdecl
	#include <stdlib.h>
#endif

//void _cdecl _stringout(char *str){	msg_write(string("StringOut: ",str));	}
void _cdecl _cstringout(char *str){	msg_write(str);	}
void _cdecl _stringout(string &str){	msg_write(str);	}
int _cdecl _Float2Int(float f){	return (int)f;	}
string _cdecl ff2s(complex &x){	return x.str();	}
string _cdecl fff2s(vector &x){	return x.str();	}
string _cdecl ffff2s(quaternion &x){	return x.str();	}


void *f_cp = (void*)1; // for fake (compiler-) functions


Array<PreCommand> PreCommands;

int cur_func;

int add_func(const string &name, Type *return_type, void *func, bool is_class)
{
	PreCommand c;
	c.name = name;
	c.return_type = return_type;
	c.func = func;
	c.is_special = false;
	c.is_class_function = is_class;
	c.is_semi_external = false;
	c.package = cur_package_index;
	if (PreCommands.num < NUM_INTERN_PRE_COMMANDS)
		PreCommands.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommands.add(c);
	cur_func = PreCommands.num - 1;
	return cur_func;
}

int add_func_special(const string &name, Type *return_type, int index)
{
	PreCommand c;
	c.name = name;
	c.return_type = return_type;
	c.func = NULL;
	c.is_special = true;
	c.is_class_function = false;
	c.is_semi_external = false;
	c.package = cur_package_index;
	if (PreCommands.num < NUM_INTERN_PRE_COMMANDS)
		PreCommands.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommands[index] = c;
	cur_func = index;
	cur_class_func = NULL;
	return cur_func;
}

void func_add_param(const string &name, Type *type)
{
	PreCommandParam p;
	p.name = name;
	p.type = type;
	PreCommands[cur_func].param.add(p);
	if (cur_class_func)
		cur_class_func->param_type.add(type);
}


bool type_is_simple_class(Type *t)
{
	if (!t->UsesCallByReference())
		return true;
	/*if (t->IsArray)
		return false;*/
	if (t->is_super_array)
		return false;
	if (t->GetFunc("__init__") >= 0)
		return false;
	if (t->GetFunc("__delete__") >= 0)
		return false;
	if (t->GetFunc("__assign__") >= 0)
		return false;
	foreach(ClassElement &e, t->element)
		if (!type_is_simple_class(e.type))
			return false;
	return true;
}

void script_make_super_array(Type *t, PreScript *ps)
{
	msg_db_r("make_super_array", 4);
	add_class(t);
		class_add_element("num", TypeInt, PointerSize);

		// always usable operations
		class_add_func("swap", TypeVoid, mf((tmf)&DynamicArray::swap));
			func_add_param("i1",		TypeInt);
			func_add_param("i2",		TypeInt);
		class_add_func("iterate", TypeBool, mf((tmf)&DynamicArray::iterate));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("iterate_back", TypeBool, mf((tmf)&DynamicArray::iterate_back));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("index", TypeInt, mf((tmf)&DynamicArray::index));
			func_add_param("pointer",		TypePointer);
		class_add_func("subarray", t, mf((tmf)&DynamicArray::ref_subarray));
			func_add_param("start",		TypeInt);
			func_add_param("num",		TypeInt);

		if (type_is_simple_class(t->parent)){
			if (!t->parent->UsesCallByReference()){
				if (t->parent->size == 4){
					class_add_func("__init__",	TypeVoid, mf((tmf)&Array<int>::__init__));
					class_add_func("add", TypeVoid, mf((tmf)&DynamicArray::append_4_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf((tmf)&DynamicArray::insert_4_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent->size == 1){
					class_add_func("__init__",	TypeVoid, mf((tmf)&Array<char>::__init__));
					class_add_func("add", TypeVoid, mf((tmf)&DynamicArray::append_1_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf((tmf)&DynamicArray::insert_1_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}
			}else{
				class_add_func("add", TypeVoid, mf((tmf)&DynamicArray::append_single));
					func_add_param("x",		t->parent);
				class_add_func("insert", TypeVoid, mf((tmf)&DynamicArray::insert_single));
					func_add_param("x",		t->parent);
					func_add_param("index",		TypeInt);
			}
			class_add_func("__delete__",	TypeVoid, mf((tmf)&DynamicArray::clear));
			class_add_func("clear", TypeVoid, mf((tmf)&DynamicArray::clear));
			class_add_func("__assign__", TypeVoid, mf((tmf)&DynamicArray::assign));
				func_add_param("other",		t);
			class_add_func("remove", TypeVoid, mf((tmf)&DynamicArray::delete_single));
				func_add_param("index",		TypeInt);
			class_add_func("removep", TypeVoid, mf((tmf)&DynamicArray::delete_single_by_pointer));
				func_add_param("pointer",		TypePointer);
			class_add_func("resize", TypeVoid, mf((tmf)&DynamicArray::resize));
				func_add_param("num",		TypeInt);
			class_add_func("ensure_size", TypeVoid, mf((tmf)&DynamicArray::ensure_size));
				func_add_param("num",		TypeInt);
		}

		// low level operations
		class_add_func("__mem_init__", TypeVoid, mf((tmf)&DynamicArray::init));
			func_add_param("element_size",		TypeInt);
		class_add_func("__mem_clear__", TypeVoid, mf((tmf)&DynamicArray::clear));
		class_add_func("__mem_resize__", TypeVoid, mf((tmf)&DynamicArray::resize));
			func_add_param("size",		TypeInt);
		class_add_func("__mem_remove__", TypeVoid, mf((tmf)&DynamicArray::delete_single));
			func_add_param("index",		TypeInt);
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
	string s = v->str();
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastFFFF2StringP(quaternion *q)
{
	string s = q->str();
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastComplex2StringP(complex *z)
{
	string s = z->str();
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}

Array<TypeCast> TypeCasts;
void add_type_cast(int penalty, Type *source, Type *dest, const string &cmd, void *func)
{
	TypeCast c;
	c.penalty = penalty;
	c.command = -1;
	for (int i=0;i<PreCommands.num;i++)
		if (PreCommands[i].name == cmd){
			c.command = i;
			break;
		}
	if (c.command < 0){
#ifdef _X_USE_HUI_
		HuiErrorBox(NULL, "", "add_type_cast (ScriptInit): " + string(cmd) + " not found");
		HuiRaiseError("add_type_cast (ScriptInit): " + string(cmd) + " not found");
#else
		msg_error("add_type_cast (ScriptInit): " + string(cmd) + " not found"));
		exit(1);
#endif
	}
	c.source = source;
	c.dest = dest;
	c.func = (t_cast_func*) func;
	TypeCasts.add(c);
}


class StringList : public Array<string>
{
public:
	void assign(StringList &s){	*this = s;	}
	string join(const string &glue)
	{	return implode((Array<string>)*this, glue);	}
};

class IntClass
{
	int i;
public:
	string str(){	return i2s(i);	}
};

class FloatClass
{
	float f;
public:
	string str(){	return f2s(f, 6);	}
	string str2(int decimals){	return f2s(f, decimals);	}
};

class BoolClass
{
	bool b;
public:
	string str(){	return b2s(b);	}
};

class CharClass
{
	char c;
public:
	string str(){	string r;	r.add(c);	return r;	}
};

class PointerClass
{
	void *p;
public:
	string str(){	return p2s(p);	}
};

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
	TypePointer			= add_type_p("void*",		TypeVoid, FLAG_CALL_BY_VALUE); // substitute for all pointer types
	TypePointerPs		= add_type_p("void*&",		TypePointer, FLAG_SILENT);
	TypePointerList		= add_type_a("void*[]",		TypePointer, -1);
	TypeBool			= add_type  ("bool",		sizeof(bool), FLAG_CALL_BY_VALUE);
	TypeBoolPs			= add_type_p("bool&",		TypeBool, FLAG_SILENT);
	TypeBoolList		= add_type_a("bool[]",		TypeBool, -11);
	TypeInt				= add_type  ("int",			sizeof(int), FLAG_CALL_BY_VALUE);
	TypeIntPs			= add_type_p("int&",		TypeInt, FLAG_SILENT);
	TypeIntList			= add_type_a("int[]",		TypeInt, -1);
	TypeIntArray		= add_type_a("int[?]",		TypeInt, 1);
	TypeFloat			= add_type  ("float",		sizeof(float), FLAG_CALL_BY_VALUE);
	TypeFloatPs			= add_type_p("float&",		TypeFloat, FLAG_SILENT);
	TypeFloatArray		= add_type_a("float[?]",	TypeFloat, 1);
	TypeFloatArrayP		= add_type_p("float[?]*",	TypeFloatArray);
	TypeFloatList		= add_type_a("float[]",		TypeFloat, -1);
	TypeChar			= add_type  ("char",		sizeof(char), FLAG_CALL_BY_VALUE);
	TypeCharPs			= add_type_p("char&",		TypeChar, FLAG_SILENT);
	TypeCString			= add_type_a("cstring",		TypeChar, 256);	// cstring := char[256]
	TypeString			= add_type_a("string",		TypeChar, -1);	// string := char[]
	TypeStringList		= add_type_a("string[]",	TypeString, -1);

	
	add_class(TypeInt);
		class_add_func("str", TypeString, mf((tmf)&IntClass::str));
	add_class(TypeFloat);
		class_add_func("str", TypeString, mf((tmf)&FloatClass::str));
		class_add_func("str2", TypeString, mf((tmf)&FloatClass::str2));
			func_add_param("decimals",		TypeInt);
	add_class(TypeBool);
		class_add_func("str", TypeString, mf((tmf)&BoolClass::str));
	add_class(TypeChar);
		class_add_func("str", TypeString, mf((tmf)&CharClass::str));
	add_class(TypePointer);
		class_add_func("str", TypeString, mf((tmf)&PointerClass::str));
	
	add_class(TypeString);
		class_add_func("__iadd__", TypeVoid, mf((tmf)&string::operator+=));
			func_add_param("x",		TypeString);
		class_add_func("__add__", TypeString, mf((tmf)&string::operator+));
			func_add_param("x",		TypeString);
		class_add_func("__eq__", TypeBool, mf((tmf)&string::operator==));
			func_add_param("x",		TypeString);
		class_add_func("__ne__", TypeBool, mf((tmf)&string::operator!=));
			func_add_param("x",		TypeString);
		class_add_func("__lt__", TypeBool, mf((tmf)&string::operator<));
			func_add_param("x",		TypeString);
		class_add_func("__gt__", TypeBool, mf((tmf)&string::operator>));
			func_add_param("x",		TypeString);
		class_add_func("__le__", TypeBool, mf((tmf)&string::operator<=));
			func_add_param("x",		TypeString);
		class_add_func("__ge__", TypeBool, mf((tmf)&string::operator>=));
			func_add_param("x",		TypeString);
		class_add_func("substr", TypeString, mf((tmf)&string::substr));
			func_add_param("start",		TypeInt);
			func_add_param("length",	TypeInt);
		class_add_func("head", TypeString, mf((tmf)&string::head));
			func_add_param("size",		TypeInt);
		class_add_func("tail", TypeString, mf((tmf)&string::tail));
			func_add_param("size",		TypeInt);
		class_add_func("find", TypeInt, mf((tmf)&string::find));
			func_add_param("str",		TypeString);
			func_add_param("start",		TypeInt);
		class_add_func("compare", TypeInt, mf((tmf)&string::compare));
			func_add_param("str",		TypeString);
		class_add_func("icompare", TypeInt, mf((tmf)&string::icompare));
			func_add_param("str",		TypeString);
		class_add_func("replace", TypeString, mf((tmf)&string::replace));
			func_add_param("sub",		TypeString);
			func_add_param("by",		TypeString);
		class_add_func("explode", TypeStringList, mf((tmf)&string::explode));
			func_add_param("str",		TypeString);
		class_add_func("lower", TypeString, mf((tmf)&string::lower));
		class_add_func("upper", TypeString, mf((tmf)&string::upper));
		class_add_func("reverse", TypeString, mf((tmf)&string::reverse));
		class_add_func("hash", TypeInt, mf((tmf)&string::hash));
		class_add_func("hex", TypeString, mf((tmf)&string::hex));
			func_add_param("inverted",		TypeBool);
		class_add_func("unhex", TypeString, mf((tmf)&string::unhex));
		class_add_func("match", TypeBool, mf((tmf)&string::match));
			func_add_param("glob",		TypeString);
		class_add_func("int", TypeInt, mf((tmf)&string::_int));
		class_add_func("float", TypeFloat, mf((tmf)&string::_float));
		class_add_func("trim", TypeString, mf((tmf)&string::trim));
		class_add_func("dirname", TypeString, mf((tmf)&string::dirname));
		class_add_func("basename", TypeString, mf((tmf)&string::basename));
		class_add_func("extension", TypeString, mf((tmf)&string::extension));

	add_class(TypeStringList);
		class_add_func("__init__",	TypeVoid, mf((tmf)&StringList::__init__));
		class_add_func("__delete__",	TypeVoid, mf((tmf)&StringList::clear));
		class_add_func("add", TypeVoid, mf((tmf)&StringList::add));
			func_add_param("x",		TypeString);
		class_add_func("clear", TypeVoid, mf((tmf)&StringList::clear));
		class_add_func("remove", TypeVoid, mf((tmf)&StringList::erase));
			func_add_param("index",		TypeInt);
		class_add_func("resize", TypeVoid, mf((tmf)&StringList::resize));
			func_add_param("num",		TypeInt);
		class_add_func("__assign__",	TypeVoid, mf((tmf)&StringList::assign));
			func_add_param("other",		TypeStringList);
		class_add_func("join", TypeString, mf((tmf)&StringList::join));
			func_add_param("glue",		TypeString);


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
	add_operator(OperatorAnd,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorOr,			TypeBool,		TypeBool,		TypeBool);
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
	
	msg_db_l(3);
}

void SIAddSuperArrays()
{
	msg_db_r("SIAddSuperArrays", 3);

	for (int i=0;i<PreTypes.num;i++)
		if (PreTypes[i]->is_super_array){
			//msg_error(string("super array:  ", PreType[i]->Name));
			script_make_super_array(PreTypes[i]);
		}
	
	msg_db_l(3);
}

void SIAddCommands()
{
	msg_db_r("SIAddCommands", 3);
	
	// type casting
	add_func("-s2i-",				TypeInt,		(void*)&s2i);
		func_add_param("s",		TypeString);
	add_func("-s2f-",				TypeFloat,		(void*)&s2f);
		func_add_param("s",		TypeString);
	add_func("-i2s-",				TypeString,	(void*)&i2s);
		func_add_param("i",		TypeInt);
	add_func("-f2s-",				TypeString,		(void*)&f2s);
		func_add_param("f",			TypeFloat);
		func_add_param("decimals",	TypeInt);
	add_func("-f2sf-",			TypeString,		(void*)&f2sf);
		func_add_param("f",			TypeFloat);
	add_func("-b2s-",				TypeString,	(void*)&b2s);
		func_add_param("b",		TypeBool);
	add_func("p2s",				TypeString,	(void*)&p2s);
		func_add_param("p",		TypePointer);
	add_func("-v2s-",				TypeString,	(void*)&fff2s);
		func_add_param("v",		TypeVector);
	add_func("-complex2s-",		TypeString,	(void*)&ff2s);
		func_add_param("z",		TypeComplex);
	add_func("-quaternion2s-",	TypeString,	(void*)&ffff2s);
		func_add_param("q",		TypeQuaternion);
	add_func("-plane2s-",			TypeString,	(void*)&ffff2s);
		func_add_param("p",		TypePlane);
	add_func("-color2s-",			TypeString,	(void*)&ffff2s);
		func_add_param("c",		TypeColor);
	add_func("-rect2s-",			TypeString,	(void*)&ffff2s);
		func_add_param("r",		TypeRect);
	add_func("-ia2s-",			TypeString,	(void*)&ia2s);
		func_add_param("a",		TypeIntList);
	add_func("-fa2s-",			TypeString,	(void*)&fa2s);
		func_add_param("a",		TypeFloatList);
	add_func("-ba2s-",			TypeString,	(void*)&ba2s);
		func_add_param("a",		TypeBoolList);
	add_func("-sa2s-",			TypeString,	(void*)&sa2s);
		func_add_param("a",		TypeStringList);
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

void Init()
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
	cur_package = &Packages[0];
	SIAddCommands();
	
	SIAddOperators();
	SIAddSuperArrays();





	add_type_cast(10,	TypeInt,		TypeFloat,	"i2f",	(void*)&CastInt2Float);
	add_type_cast(20,	TypeFloat,		TypeInt,	"f2i",	(void*)&CastFloat2Int);
	add_type_cast(10,	TypeInt,		TypeChar,	"i2c",	(void*)&CastInt2Char);
	add_type_cast(20,	TypeChar,		TypeInt,	"c2i",	(void*)&CastChar2Int);
	add_type_cast(50,	TypePointer,	TypeBool,	"p2b",	(void*)&CastPointer2Bool);
	add_type_cast(50,	TypeInt,		TypeString,	"-i2s-",	(void*)&CastInt2StringP);
	add_type_cast(50,	TypeFloat,		TypeString,	"-f2sf-",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeBool,		TypeString,	"-b2s-",	(void*)&CastBool2StringP);
	add_type_cast(50,	TypePointer,	TypeString,	"p2s",	(void*)&CastPointer2StringP);
	add_type_cast(50,	TypeVector,		TypeString,	"-v2s-",	(void*)&CastVector2StringP);
	add_type_cast(50,	TypeComplex,	TypeString,	"-complex2s-",	(void*)&CastComplex2StringP);
	add_type_cast(50,	TypeColor,		TypeString,	"-color2s-",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeQuaternion,	TypeString,	"-quaternion2s-",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypePlane,		TypeString,	"-plane2s-",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeRect,		TypeString,	"-rect2s-",	(void*)&CastFFFF2StringP);
	//add_type_cast(50,	TypeClass,		TypeString,	"-f2s-",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeIntList,	TypeString,	"-ia2s-",	NULL);
	add_type_cast(50,	TypeFloatList,	TypeString,	"-fa2s-",	NULL);
	add_type_cast(50,	TypeBoolList,	TypeString,	"-ba2s-",	NULL);
	add_type_cast(50,	TypeStringList,	TypeString,	"-sa2s-",	NULL);

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

void ResetSemiExternalData()
{
	msg_db_r("ScriptResetSemiExternalData", 2);
	for (int i=PreExternalVars.num-1;i>=0;i--)
		if (PreExternalVars[i].is_semi_external)
			PreExternalVars.erase(i);
	for (int i=PreCommands.num-1;i>=0;i--)
		if (PreCommands[i].is_semi_external)
			PreCommands.erase(i);
	msg_db_l(2);
}

// program variables - specific to the surrounding program, can't always be there...
void LinkSemiExternalVar(const string &name, void *pointer)
{
	msg_db_r("ScriptLinkSemiExternalVar", 2);
	PreExternalVar v;
	v.name = name;
	v.pointer = pointer;
	v.type = TypeUnknown; // unusable until defined via "extern" in the script!
	v.is_semi_external = true; // ???
	PreExternalVars.add(v);
	msg_db_l(2);
}

// program functions - specific to the surrounding program, can't always be there...
void LinkSemiExternalFunc(const string &name, void *pointer)
{
	PreCommand c;
	c.name = name;
	c.is_class_function = false;
	c.func = pointer;
	c.return_type = TypeUnknown; // unusable until defined via "extern" in the script!
	c.is_semi_external = true;
	PreCommands.add(c);
}

void _LinkSemiExternalClassFunc(const string &name, void (DummyClass::*function)())
{
	LinkSemiExternalFunc(name, (void*)function);
}

void End()
{
	msg_db_r("ScriptEnd", 1);
	DeleteAllScripts(true, true);

	ResetSemiExternalData();

	PreOperators.clear();

	for (int i=0;i<PreTypes.num;i++)
		delete(PreTypes[i]);
	PreTypes.clear();

	PreConstants.clear();
	PreExternalVars.clear();
	msg_db_l(1);
}

};
