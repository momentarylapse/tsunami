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
#include "../script.h"
#include "script_data_common.h"
#include "../../config.h"



#ifdef _X_USE_HUI_
#include "../../hui/hui.h"
#endif



namespace Script{

string DataVersion = "0.12.5.0";

CompilerConfiguration config;

struct ExternalLinkData
{
	string name;
	void *pointer;
};
Array<ExternalLinkData> ExternalLinks;

struct ClassOffsetData
{
	string class_name, element;
	int offset;
	bool is_virtual;
};
Array<ClassOffsetData> ClassOffsets;

struct ClassSizeData
{
	string class_name;
	int size;
};
Array<ClassSizeData> ClassSizes;


//------------------------------------------------------------------------------------------------//
//                                             types                                              //
//------------------------------------------------------------------------------------------------//

Type *TypeUnknown;
Type *TypeReg128;
Type *TypeReg64;
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
Script *cur_package_script = NULL;
int cur_package_index;


void add_package(const string &name, bool used_by_default)
{
	Package p;
	p.name = name;
	p.used_by_default = true;//used_by_default;
	p.script = new Script;
	p.script->Filename = name;
	Packages.add(p);
	cur_package_script = p.script;
	cur_package_index = Packages.num - 1;
}

Type *add_type(const string &name, int size, TypeFlag flag)
{
	msg_db_f("add_type", 4);
	Type *t = new Type;
	t->owner = cur_package_script->syntax;
	t->name = name;
	t->size = size;
	if ((flag & FLAG_CALL_BY_VALUE) > 0)
		t->force_call_by_value = true;
	cur_package_script->syntax->Types.add(t);
	return t;
}
Type *add_type_p(const string &name, Type *sub_type, TypeFlag flag)
{
	msg_db_f("add_type_p", 4);
	Type *t = new Type;
	t->owner = cur_package_script->syntax;
	t->name = name;
	t->size = config.PointerSize;
	t->is_pointer = true;
	if ((flag & FLAG_SILENT) > 0)
		t->is_silent = true;
	t->parent = sub_type;
	cur_package_script->syntax->Types.add(t);
	return t;
}
Type *add_type_a(const string &name, Type *sub_type, int array_length)
{
	msg_db_f("add_type_a", 4);
	Type *t = new Type;
	t->owner = cur_package_script->syntax;
	t->name = name;
	t->parent = sub_type;
	if (array_length < 0){
		// super array
		t->size = config.SuperArraySize;
		t->is_super_array = true;
		script_make_super_array(t);
	}else{
		// standard array
		t->size = sub_type->size * array_length;
		t->is_array = true;
		t->array_length = array_length;
	}
	cur_package_script->syntax->Types.add(t);
	return t;
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
	msg_db_f("add_op", 4);
	PreOperator o;
	o.primitive_id = primitive_op;
	o.return_type = return_type;
	o.param_type_1 = param_type1;
	o.param_type_2 = param_type2;
	o.func = func;
	PreOperators.add(o);
	return PreOperators.num - 1;
}

string PreOperator::str() const
{
	return "(" + param_type_1->name + ") " + PrimitiveOperators[primitive_id].name + " (" + param_type_2->name + ")";
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//



static PreCommand *cur_cmd = NULL;
static Function *cur_func = NULL;
static Type *cur_class;
static ClassFunction *cur_class_func = NULL;

void add_class(Type *root_type)//, PreScript *ps = NULL)
{
	msg_db_f("add_class", 4);
	cur_class = root_type;
}

void class_add_element(const string &name, Type *type, int offset)
{
	msg_db_f("add_class_el", 4);
	ClassElement e;
	e.name = name;
	e.type = type;
	e.offset = offset;
	cur_class->element.add(e);
}

int add_func(const string &name, Type *return_type, void *func, bool is_class);

void _class_add_func_virtual(const string &tname, const string &name, Type *return_type, int index)
{
	//msg_write("virtual: " + tname + "." + name);
	//msg_write(index);
	int cmd = add_func(tname + "." + name + "[virtual]", return_type, NULL, true);
	cur_func->_class = cur_class;
	cur_class->function.add(ClassFunction(name, return_type, cur_package_script, cmd));
	cur_class_func = &cur_class->function.back();
	cur_class_func->virtual_index = index;
	cur_class->num_virtual = max(cur_class->num_virtual, index + 1);
}

void class_add_func(const string &name, Type *return_type, void *func)
{
	msg_db_f("add_class_func", 4);
	string tname = cur_class->name;
	if (tname[0] == '-'){
		foreach(Type *t, cur_package_script->syntax->Types)
			if ((t->is_pointer) && (t->parent == cur_class))
				tname = t->name;
	}
	int cmd = add_func(tname + "." + name, return_type, func, true);
	cur_func->_class = cur_class;
	cur_class->function.add(ClassFunction(name, return_type, cur_package_script, cmd));
	cur_class_func = &cur_class->function.back();
}

void class_add_func_virtual(const string &name, Type *return_type, void *func)
{
	msg_db_f("add_class_func_virtual", 4);
	string tname = cur_class->name;
	if (tname[0] == '-'){
		foreach(Type *t, cur_package_script->syntax->Types)
			if ((t->is_pointer) && (t->parent == cur_class))
				tname = t->name;
	}
	if (config.abi == AbiWindows32){
		if (!func){
			_class_add_func_virtual(tname, name, return_type, 0);
			return;
		}
		unsigned char *pp = (unsigned char*)func;
		try{
			//if ((cur_class->vtable) && (pp[0] == 0x8b) && (pp[1] == 0x01) && (pp[2] == 0xff) && (pp[3] == 0x60)){
			if ((pp[0] == 0x8b) && (pp[1] == 0x44) && (pp[2] == 0x24) && (pp[4] == 0x8b) && (pp[5] == 0x00) && (pp[6] == 0xff) && (pp[7] == 0x60)){
				// 8b.44.24.**    8b.00     ff.60.10
				// virtual function
				int index = (int)pp[8] / 4;
				_class_add_func_virtual(tname, name, return_type, index);
			}else if (pp[0] == 0xe9){
				// jmp
				//msg_write(Asm::Disassemble(func, 16));
				pp = &pp[5] + *(int*)&pp[1];
				//msg_write(Asm::Disassemble(pp, 16));
				if ((pp[0] == 0x8b) && (pp[1] == 0x44) && (pp[2] == 0x24) && (pp[4] == 0x8b) && (pp[5] == 0x00) && (pp[6] == 0xff) && (pp[7] == 0x60)){
					// 8b.44.24.**    8b.00     ff.60.10
					// virtual function
					int index = (int)pp[8] / 4;
					_class_add_func_virtual(tname, name, return_type, index);
				}else
					throw(1);
			}else
				throw(1);
		}catch(...){
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
			msg_write(string((char*)pp, 4).hex());
			msg_write(Asm::Disassemble(func, 16));
		}
	}else{
	
		long p = (long)func;
		if ((p & 1) > 0){
			// virtual function
			int index = p / sizeof(void*);
			_class_add_func_virtual(tname, name, return_type, index);
		}else if (!func){
			_class_add_func_virtual(tname, name, return_type, 0);
		}else{
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
		}

	}
}

void class_link_vtable(void *p)
{
	cur_class->LinkExternalVirtualTable(p);
}


//------------------------------------------------------------------------------------------------//
//                                           constants                                            //
//------------------------------------------------------------------------------------------------//

void add_const(const string &name, Type *type, void *value)
{
	msg_db_f("add_const", 4);
	Constant c;
	c.name = name;
	c.type = type;
	c.data = new char[max(type->size, 8)];//config.PointerSize)];
	// config.PointerSize might be smaller than needed for the following assignment
	if ((type == TypeInt) || (type == TypeFloat) || (type == TypeChar)  || (type == TypeBool) || (type->is_pointer))
		*(void**)c.data = value;
	else
		memcpy(c.data, value, type->size);
	cur_package_script->syntax->Constants.add(c);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//


void add_ext_var(const string &name, Type *type, void *var)
{
	cur_package_script->syntax->AddVar(name, type, &cur_package_script->syntax->RootOfAllEvil);
	cur_package_script->g_var.add(config.allow_std_lib ? (char*)var : NULL);
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


Array<PreCommand> PreCommands;

int add_func(const string &name, Type *return_type, void *func, bool is_class)
{
	Function *f = new Function(name, return_type);
	f->literal_return_type = return_type;
	f->num_params = 0;
	f->_class = NULL;
	cur_package_script->syntax->Functions.add(f);
	cur_package_script->func.add(config.allow_std_lib ? (void (*)())func : NULL);
	cur_cmd = NULL;
	cur_func = f;
	cur_class_func = NULL;
	return cur_package_script->syntax->Functions.num - 1;
}

int add_compiler_func(const string &name, Type *return_type, int index)
{
	PreCommand c;
	c.name = name;
	c.return_type = return_type;
	c.package = cur_package_index;
	if (PreCommands.num < NUM_INTERN_PRE_COMMANDS)
		PreCommands.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommands[index] = c;
	cur_func = NULL;
	cur_cmd = &PreCommands[index];
	cur_class_func = NULL;
	return index;
}

void func_add_param(const string &name, Type *type)
{
	if (cur_cmd){
		PreCommandParam p;
		p.name = name;
		p.type = type;
		cur_cmd->param.add(p);
	}else if (cur_func){
		Variable v;
		v.name = name;
		v.type = type;
		if (cur_func){
			cur_func->var.add(v);
			cur_func->literal_param_type[cur_func->num_params] = type;
			cur_func->num_params ++;
		}
		if (cur_class_func)
			cur_class_func->param_type.add(type);
	}
}

void script_make_super_array(Type *t, SyntaxTree *ps)
{
	msg_db_f("make_super_array", 4);

	add_class(t);
		class_add_element("num", TypeInt, config.PointerSize);

		// always usable operations
		class_add_func("swap", TypeVoid, mf(&DynamicArray::swap));
			func_add_param("i1",		TypeInt);
			func_add_param("i2",		TypeInt);
		class_add_func("iterate", TypeBool, mf(&DynamicArray::iterate));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("iterate_back", TypeBool, mf(&DynamicArray::iterate_back));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("index", TypeInt, mf(&DynamicArray::index));
			func_add_param("pointer",		TypePointer);
		class_add_func("subarray", t, mf(&DynamicArray::ref_subarray));
			func_add_param("start",		TypeInt);
			func_add_param("num",		TypeInt);

		// FIXME  wrong for complicated classes
		if (t->parent->is_simple_class()){
			if (!t->parent->UsesCallByReference()){
				if (t->parent->is_pointer){
					class_add_func("__init__",	TypeVoid, mf(&Array<void*>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_p_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_p_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent == TypeFloat){
					class_add_func("__init__",	TypeVoid, mf(&Array<float>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_f_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_f_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent->size == 4){
					class_add_func("__init__",	TypeVoid, mf(&Array<int>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_4_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_4_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent->size == 1){
					class_add_func("__init__",	TypeVoid, mf(&Array<char>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_1_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_1_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else
					msg_error("evil class type..." + t->name);
			}else{
				class_add_func("add", TypeVoid, mf(&DynamicArray::append_single));
					func_add_param("x",		t->parent);
				class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_single));
					func_add_param("x",		t->parent);
					func_add_param("index",		TypeInt);
			}
			class_add_func("__delete__",	TypeVoid, mf(&DynamicArray::clear));
			class_add_func("clear", TypeVoid, mf(&DynamicArray::clear));
			class_add_func("__assign__", TypeVoid, mf(&DynamicArray::assign));
				func_add_param("other",		t);
			class_add_func("remove", TypeVoid, mf(&DynamicArray::delete_single));
				func_add_param("index",		TypeInt);
			class_add_func("removep", TypeVoid, mf(&DynamicArray::delete_single_by_pointer));
				func_add_param("pointer",		TypePointer);
			class_add_func("resize", TypeVoid, mf(&DynamicArray::resize));
				func_add_param("num",		TypeInt);
			class_add_func("ensure_size", TypeVoid, mf(&DynamicArray::ensure_size));
				func_add_param("num",		TypeInt);
		}

		// low level operations
		class_add_func("__mem_init__", TypeVoid, mf(&DynamicArray::init));
			func_add_param("element_size",		TypeInt);
		class_add_func("__mem_clear__", TypeVoid, mf(&DynamicArray::clear));
		class_add_func("__mem_resize__", TypeVoid, mf(&DynamicArray::resize));
			func_add_param("size",		TypeInt);
		class_add_func("__mem_remove__", TypeVoid, mf(&DynamicArray::delete_single));
			func_add_param("index",		TypeInt);
}


// automatic type casting

#define MAX_TYPE_CAST_BUFFER	32768
char type_cast_buffer[MAX_TYPE_CAST_BUFFER];
int type_cast_buffer_size = 0;

char *get_type_cast_buf(int size)
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

Array<TypeCast> TypeCasts;
void add_type_cast(int penalty, Type *source, Type *dest, const string &cmd, void *func)
{
	TypeCast c;
	c.penalty = penalty;
	c.func_no = -1;
	for (int i=0;i<PreCommands.num;i++)
		if (PreCommands[i].name == cmd){
			c.kind = KindCompilerFunction;
			c.func_no = i;
			c.script = cur_package_script;
			break;
		}
	if (c.func_no < 0)
	for (int i=0;i<cur_package_script->syntax->Functions.num;i++)
		if (cur_package_script->syntax->Functions[i]->name == cmd){
			c.kind = KindFunction;
			c.func_no = i;
			c.script = cur_package_script;
			break;
		}
	if (c.func_no < 0){
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
	void _cdecl assign(StringList &s){	*this = s;	}
	string _cdecl join(const string &glue)
	{	return implode((Array<string>)*this, glue);	}
};

class IntClass
{
	int i;
public:
	string _cdecl str(){	return i2s(i);	}
};

class FloatClass
{
	float f;
public:
	string _cdecl str(){	return f2s(f, 6);	}
	string _cdecl str2(int decimals){	return f2s(f, decimals);	}
};

class BoolClass
{
	bool b;
public:
	string _cdecl str(){	return b2s(b);	}
};

class CharClass
{
	char c;
public:
	string _cdecl str(){	string r;	r.add(c);	return r;	}
};

class PointerClass
{
	void *p;
public:
	string _cdecl str(){	return p2s(p);	}
};

class VirtualTest : public VirtualBase
{
public:
	int i;
	static bool enable_logging;
	VirtualTest(){ if (enable_logging) msg_write("VirtualTest.init()"); i = 13; }
	virtual ~VirtualTest(){ if (enable_logging) msg_write("VirtualTest.~"); }
	void _cdecl __init__(){ new(this) VirtualTest; }
	virtual void _cdecl __delete__(){ if (enable_logging) msg_write("VirtualTest.delete()"); }
	virtual void _cdecl f_virtual(){ msg_write(i); msg_write("VirtualTest.f_virtual()"); }
	void _cdecl f_normal(){ msg_write(i); msg_write("VirtualTest.f_normal()"); }
	void _cdecl test(){ msg_write("VirtualTest.test()"); f_virtual(); }
};
bool VirtualTest::enable_logging;

void SIAddPackageBase()
{
	msg_db_f("SIAddPackageBase", 3);

	add_package("base", true);

	// internal
	TypeUnknown			= add_type  ("-unknown-",	0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type  ("-reg128-",		16, FLAG_CALL_BY_VALUE);
	TypeReg64			= add_type  ("-reg64-",		8, FLAG_CALL_BY_VALUE);
	TypeReg32			= add_type  ("-reg32-",		4, FLAG_CALL_BY_VALUE);
	TypeReg16			= add_type  ("-reg16-",		2, FLAG_CALL_BY_VALUE);
	TypeReg8			= add_type  ("-reg8-",		1, FLAG_CALL_BY_VALUE);
	TypeClass			= add_type  ("-class-",	0); // substitute for all class types

	// "real"
	TypeVoid			= add_type  ("void",		0, FLAG_CALL_BY_VALUE);
	TypeBool			= add_type  ("bool",		sizeof(bool), FLAG_CALL_BY_VALUE);
	TypeInt				= add_type  ("int",			sizeof(int), FLAG_CALL_BY_VALUE);
	TypeFloat			= add_type  ("float",		sizeof(float), FLAG_CALL_BY_VALUE);
	TypeChar			= add_type  ("char",		sizeof(char), FLAG_CALL_BY_VALUE);
	// derived   (must be defined after the primitive types!)
	TypePointer			= add_type_p("void*",		TypeVoid, FLAG_CALL_BY_VALUE); // substitute for all pointer types
	TypePointerPs		= add_type_p("void*&",		TypePointer, FLAG_SILENT);
	TypePointerList		= add_type_a("void*[]",		TypePointer, -1);
	TypeBoolPs			= add_type_p("bool&",		TypeBool, FLAG_SILENT);
	TypeBoolList		= add_type_a("bool[]",		TypeBool, -11);
	TypeIntPs			= add_type_p("int&",		TypeInt, FLAG_SILENT);
	TypeIntList			= add_type_a("int[]",		TypeInt, -1);
	TypeIntArray		= add_type_a("int[?]",		TypeInt, 1);
	TypeFloatPs			= add_type_p("float&",		TypeFloat, FLAG_SILENT);
	TypeFloatArray		= add_type_a("float[?]",	TypeFloat, 1);
	TypeFloatArrayP		= add_type_p("float[?]*",	TypeFloatArray);
	TypeFloatList		= add_type_a("float[]",		TypeFloat, -1);
	TypeCharPs			= add_type_p("char&",		TypeChar, FLAG_SILENT);
	TypeCString			= add_type_a("cstring",		TypeChar, 256);	// cstring := char[256]
	TypeString			= add_type_a("string",		TypeChar, -1);	// string := char[]
	TypeStringList		= add_type_a("string[]",	TypeString, -1);

	
	Type *TypeVirtualTest=add_type  ("VirtualTest",	sizeof(VirtualTest));

	add_class(TypeInt);
		class_add_func("str", TypeString, mf(&IntClass::str));
	add_class(TypeFloat);
		class_add_func("str", TypeString, mf(&FloatClass::str));
		class_add_func("str2", TypeString, mf(&FloatClass::str2));
			func_add_param("decimals",		TypeInt);
	add_class(TypeBool);
		class_add_func("str", TypeString, mf(&BoolClass::str));
	add_class(TypeChar);
		class_add_func("str", TypeString, mf(&CharClass::str));
	add_class(TypePointer);
		class_add_func("str", TypeString, mf(&PointerClass::str));
	
	add_class(TypeString);
		class_add_func("__iadd__", TypeVoid, mf(&string::operator+=));
			func_add_param("x",		TypeString);
		class_add_func("__add__", TypeString, mf(&string::operator+));
			func_add_param("x",		TypeString);
		class_add_func("__eq__", TypeBool, mf(&string::operator==));
			func_add_param("x",		TypeString);
		class_add_func("__ne__", TypeBool, mf(&string::operator!=));
			func_add_param("x",		TypeString);
		class_add_func("__lt__", TypeBool, mf(&string::operator<));
			func_add_param("x",		TypeString);
		class_add_func("__gt__", TypeBool, mf(&string::operator>));
			func_add_param("x",		TypeString);
		class_add_func("__le__", TypeBool, mf(&string::operator<=));
			func_add_param("x",		TypeString);
		class_add_func("__ge__", TypeBool, mf(&string::operator>=));
			func_add_param("x",		TypeString);
		class_add_func("substr", TypeString, mf(&string::substr));
			func_add_param("start",		TypeInt);
			func_add_param("length",	TypeInt);
		class_add_func("head", TypeString, mf(&string::head));
			func_add_param("size",		TypeInt);
		class_add_func("tail", TypeString, mf(&string::tail));
			func_add_param("size",		TypeInt);
		class_add_func("find", TypeInt, mf(&string::find));
			func_add_param("str",		TypeString);
			func_add_param("start",		TypeInt);
		class_add_func("compare", TypeInt, mf(&string::compare));
			func_add_param("str",		TypeString);
		class_add_func("icompare", TypeInt, mf(&string::icompare));
			func_add_param("str",		TypeString);
		class_add_func("replace", TypeString, mf(&string::replace));
			func_add_param("sub",		TypeString);
			func_add_param("by",		TypeString);
		class_add_func("explode", TypeStringList, mf(&string::explode));
			func_add_param("str",		TypeString);
		class_add_func("lower", TypeString, mf(&string::lower));
		class_add_func("upper", TypeString, mf(&string::upper));
		class_add_func("reverse", TypeString, mf(&string::reverse));
		class_add_func("hash", TypeInt, mf(&string::hash));
		class_add_func("hex", TypeString, mf(&string::hex));
			func_add_param("inverted",		TypeBool);
		class_add_func("unhex", TypeString, mf(&string::unhex));
		class_add_func("match", TypeBool, mf(&string::match));
			func_add_param("glob",		TypeString);
		class_add_func("int", TypeInt, mf(&string::_int));
		class_add_func("float", TypeFloat, mf(&string::_float));
		class_add_func("trim", TypeString, mf(&string::trim));
		class_add_func("dirname", TypeString, mf(&string::dirname));
		class_add_func("basename", TypeString, mf(&string::basename));
		class_add_func("extension", TypeString, mf(&string::extension));
	add_class(TypeStringList);
		class_add_func("__init__",	TypeVoid, mf(&StringList::__init__));
		class_add_func("__delete__",	TypeVoid, mf(&StringList::clear));
		class_add_func("add", TypeVoid, mf(&StringList::add));
			func_add_param("x",		TypeString);
		class_add_func("clear", TypeVoid, mf(&StringList::clear));
		class_add_func("remove", TypeVoid, mf(&StringList::erase));
			func_add_param("index",		TypeInt);
		class_add_func("resize", TypeVoid, mf(&StringList::resize));
			func_add_param("num",		TypeInt);
		class_add_func("__assign__",	TypeVoid, mf(&StringList::assign));
			func_add_param("other",		TypeStringList);
		class_add_func("join", TypeString, mf(&StringList::join));
			func_add_param("glue",		TypeString);

	VirtualTest::enable_logging = false;
	add_class(TypeVirtualTest);
		class_add_element("i", TypeInt, offsetof(VirtualTest, i));
		class_add_func("__init__", TypeVoid, mf(&VirtualTest::__init__));
		class_add_func_virtual("__delete__", TypeVoid, mf(&VirtualTest::__delete__));
		class_add_func_virtual("f_virtual", TypeVoid, mf(&VirtualTest::f_virtual));
		class_add_func("f_normal", TypeVoid, mf(&VirtualTest::f_normal));
		class_add_func("test", TypeVoid, mf(&VirtualTest::test));
		class_set_vtable(VirtualTest);
	VirtualTest::enable_logging = true;


	add_const("nil", TypePointer, NULL);
	// bool
	add_const("false", TypeBool, (void*)false);
	add_const("true",  TypeBool, (void*)true);
}


void SIAddBasicCommands()
{
	msg_db_f("SIAddBasicCommands", 3);

/*
	CommandReturn,
	CommandIf,
	CommandIfElse,
	CommandWhile,
	CommandFor,
	CommandBreak,
	CommandContinue,
	CommandNew,
	CommandDelete,
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
	add_compiler_func("-return-",		TypeVoid,	CommandReturn);
		func_add_param("return_value",	TypeVoid); // return: ParamType will be defined by the parser!
	add_compiler_func("-if-",		TypeVoid,	CommandIf);
		func_add_param("b",	TypeBool);
	add_compiler_func("-if/else-",	TypeVoid,	CommandIfElse);
		func_add_param("b",	TypeBool);
	add_compiler_func("-while-",		TypeVoid,	CommandWhile);
		func_add_param("b",	TypeBool);
	add_compiler_func("-for-",		TypeVoid,	CommandFor);
		func_add_param("b",	TypeBool); // internally like a while-loop... but a bit different...
	add_compiler_func("-break-",		TypeVoid,	CommandBreak);
	add_compiler_func("-continue-",	TypeVoid,	CommandContinue);
	add_compiler_func("-new-",	TypePointer,	CommandNew);
	add_compiler_func("-delete-",	TypeVoid,	CommandDelete);
		func_add_param("p",	TypePointer);
	add_compiler_func("sizeof",		TypeInt,	CommandSizeof);
		func_add_param("type",	TypeVoid);
	
	add_compiler_func("wait",		TypeVoid,	CommandWait);
		func_add_param("time",	TypeFloat);
	add_compiler_func("wait_rt",		TypeVoid,	CommandWaitRT);
		func_add_param("time",	TypeFloat);
	add_compiler_func("wait_of",		TypeVoid,	CommandWaitOneFrame);
//	add_func_special("f2i",			TypeInt,	(void*)&_Float2Int);
	add_compiler_func("f2i",			TypeInt,	CommandFloatToInt);    // sometimes causes floating point exceptions...
		func_add_param("f",		TypeFloat);
	add_compiler_func("i2f",			TypeFloat,	CommandIntToFloat);
		func_add_param("i",		TypeInt);
	add_compiler_func("i2c",			TypeChar,	CommandIntToChar);
		func_add_param("i",		TypeInt);
	add_compiler_func("c2i",			TypeInt,	CommandCharToInt);
		func_add_param("c",		TypeChar);
	add_compiler_func("p2b",			TypeBool,	CommandPointerToBool);
		func_add_param("p",		TypePointer);
	add_compiler_func("-asm-",		TypeVoid,	CommandAsm);
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
	msg_db_f("SIAddOperators", 3);
	

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
}

void SIAddCommands()
{
	msg_db_f("SIAddCommands", 3);
	
	// type casting
	add_func("@s2i",				TypeInt,		(void*)&s2i);
		func_add_param("s",		TypeString);
	add_func("@s2f",				TypeFloat,		(void*)&s2f);
		func_add_param("s",		TypeString);
	add_func("@i2s",				TypeString,	(void*)&i2s);
		func_add_param("i",		TypeInt);
	add_func("@f2s",				TypeString,		(void*)&f2s);
		func_add_param("f",			TypeFloat);
		func_add_param("decimals",	TypeInt);
	add_func("@f2sf",			TypeString,		(void*)&f2sf);
		func_add_param("f",			TypeFloat);
	add_func("@b2s",				TypeString,	(void*)&b2s);
		func_add_param("b",		TypeBool);
	add_func("p2s",				TypeString,	(void*)&p2s);
		func_add_param("p",		TypePointer);
	add_func("@ia2s",			TypeString,	(void*)&ia2s);
		func_add_param("a",		TypeIntList);
	add_func("@fa2s",			TypeString,	(void*)&fa2s);
		func_add_param("a",		TypeFloatList);
	add_func("@ba2s",			TypeString,	(void*)&ba2s);
		func_add_param("a",		TypeBoolList);
	add_func("@sa2s",			TypeString,	(void*)&sa2s);
		func_add_param("a",		TypeStringList);
	// debug output
	/*add_func("cprint",			TypeVoid,		(void*)&_cstringout);
		func_add_param("str",	TypeCString);*/
	add_func("print",			TypeVoid,		(void*)&_stringout);
		func_add_param("str",	TypeString);
	// memory
	add_func("@malloc",			TypePointer,		(void*)&malloc);
		func_add_param("size",	TypeInt);
	add_func("@free",			TypeVoid,		(void*)&free);
		func_add_param("p",	TypePointer);
	// system
	add_func("_exec_",			TypeString,		(void*)&shell_execute);
		func_add_param("cmd",	TypeString);


// add_func("ExecuteScript",	TypeVoid);
//		func_add_param("filename",		TypeString);
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

void Init(int instruction_set, int abi, bool allow_std_lib)
{
	msg_db_f("ScriptInit", 1);

	Asm::Init(instruction_set);
	config.instruction_set = Asm::InstructionSet.set;
	config.abi = abi;
	if (abi < 0){
		if (config.instruction_set == Asm::InstructionSetAMD64){
			config.abi = AbiGnu64;
#ifdef WIN64
			config.abi = AbiWindows64;
#endif
		}else if (config.instruction_set == Asm::InstructionSetX86){
			config.abi = AbiGnu32;
#ifdef WIN32
			config.abi = AbiWindows32;
#endif
		}
	}
	config.allow_std_lib = allow_std_lib;
	config.PointerSize = Asm::InstructionSet.pointer_size;
	if ((abi >= 0) || (instruction_set >= 0))
		config.SuperArraySize = config.PointerSize + 3 * sizeof(int);
	else
		config.SuperArraySize = sizeof(DynamicArray);
	config.StackSize = SCRIPT_DEFAULT_STACK_SIZE;

	config.allow_simplification = true;
	config.allow_registers = true;
	config.UseConstAsGlobalVar = false;
	config.StackMemAlign = 8;
	config.FunctionAlign = 2 * config.PointerSize;
	config.StackFrameAlign = 2 * config.PointerSize;

	config.CompileSilently = false;
	config.ShowCompilerStats = true;

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
	cur_package_script = Packages[0].script;
	SIAddCommands();
	
	SIAddOperators();



	add_type_cast(10,	TypeInt,		TypeFloat,	"i2f",	(void*)&CastInt2Float);
	add_type_cast(20,	TypeFloat,		TypeInt,	"f2i",	(void*)&CastFloat2Int);
	add_type_cast(10,	TypeInt,		TypeChar,	"i2c",	(void*)&CastInt2Char);
	add_type_cast(20,	TypeChar,		TypeInt,	"c2i",	(void*)&CastChar2Int);
	add_type_cast(50,	TypePointer,	TypeBool,	"p2b",	(void*)&CastPointer2Bool);
	add_type_cast(50,	TypeInt,		TypeString,	"@i2s",	(void*)&CastInt2StringP);
	add_type_cast(50,	TypeFloat,		TypeString,	"@f2sf",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeBool,		TypeString,	"@b2s",	(void*)&CastBool2StringP);
	add_type_cast(50,	TypePointer,	TypeString,	"p2s",	(void*)&CastPointer2StringP);
	//add_type_cast(50,	TypeClass,		TypeString,	"@f2s",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeIntList,	TypeString,	"@ia2s",	NULL);
	add_type_cast(50,	TypeFloatList,	TypeString,	"@fa2s",	NULL);
	add_type_cast(50,	TypeBoolList,	TypeString,	"@ba2s",	NULL);
	add_type_cast(50,	TypeStringList,	TypeString,	"@sa2s",	NULL);

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
}

void ResetExternalLinkData()
{
	ExternalLinks.clear();
	ClassOffsets.clear();
	ClassSizes.clear();
}

// program variables - specific to the surrounding program, can't always be there...
void LinkExternal(const string &name, void *pointer)
{
	ExternalLinkData l;
	l.name = name;
	l.pointer = pointer;
	ExternalLinks.add(l);
	if (name.head(5) == "lib__"){
		string sname = name.substr(5, -1).replace("@list", "[]").replace("@@", ".");
		foreach(Package &p, Packages)
			foreachi(Function *f, p.script->syntax->Functions, i)
				if (f->name == sname)
					p.script->func[i] = (void(*)())pointer;
	}
}

void *GetExternalLink(const string &name)
{
	foreach(ExternalLinkData &l, ExternalLinks)
		if (l.name == name)
			return l.pointer;
	return NULL;
}

void DeclareClassSize(const string &class_name, int size)
{
	ClassSizeData d;
	d.class_name = class_name;
	d.size = size;
	ClassSizes.add(d);
}

void DeclareClassOffset(const string &class_name, const string &element, int offset)
{
	ClassOffsetData d;
	d.class_name = class_name;
	d.element = element;
	d.offset = offset;
	d.is_virtual = false;
	ClassOffsets.add(d);
}

void DeclareClassVirtualIndex(const string &class_name, const string &func, void *p, void *instance)
{
	ClassOffsetData d;
	d.class_name = class_name;
	d.element = func;
	d.offset = (int)(long)p / sizeof(void*);
	d.is_virtual = true;
	ClassOffsets.add(d);

	VirtualTable *v = *(VirtualTable**)instance;
	LinkExternal(class_name + "." + func, v[d.offset]);
}

int ProcessClassOffset(const string &class_name, const string &element, int offset)
{
	foreach(ClassOffsetData &d, ClassOffsets)
		if ((d.class_name == class_name) && (d.element == element))
			return d.offset;
	return offset;
}
int ProcessClassSize(const string &class_name, int size)
{
	foreach(ClassSizeData &d, ClassSizes)
		if (d.class_name == class_name)
			return d.size;
	return size;
}

int ProcessClassNumVirtuals(const string &class_name, int num_virtual)
{
	foreach(ClassOffsetData &d, ClassOffsets)
		if ((d.class_name == class_name) && (d.is_virtual))
			num_virtual = max(num_virtual, d.offset + 1);
	return num_virtual;
}

void End()
{
	msg_db_f("ScriptEnd", 1);
	DeleteAllScripts(true, true);

	//ResetSemiExternalData();

	PreOperators.clear();

	/*for (int i=0;i<PreTypes.num;i++)
		delete(PreTypes[i]);
	PreTypes.clear();

	PreConstants.clear();
	PreExternalVars.clear();*/
}

};
