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

string DataVersion = "0.13.12.0";

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
Type *TypeInt64;
Type *TypeFloat;
Type *TypeFloat32;
Type *TypeFloat64;
Type *TypeChar;
Type *TypeString;
Type *TypeCString;

Type *TypeVector;
Type *TypeRect;
Type *TypeColor;
Type *TypeQuaternion;
 // internal:
Type *TypeDynamicArray;
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
	p.script->filename = name;
	Packages.add(p);
	cur_package_script = p.script;
	cur_package_index = Packages.num - 1;
}

Type *add_type(const string &name, int size, ScriptFlag flag)
{
	msg_db_f("add_type", 4);
	Type *t = new Type;
	t->owner = cur_package_script->syntax;
	t->name = name;
	t->size = size;
	if ((flag & FLAG_CALL_BY_VALUE) > 0)
		t->force_call_by_value = true;
	cur_package_script->syntax->types.add(t);
	return t;
}
Type *add_type_p(const string &name, Type *sub_type, ScriptFlag flag)
{
	msg_db_f("add_type_p", 4);
	Type *t = new Type;
	t->owner = cur_package_script->syntax;
	t->name = name;
	t->size = config.pointer_size;
	t->is_pointer = true;
	if ((flag & FLAG_SILENT) > 0)
		t->is_silent = true;
	t->parent = sub_type;
	cur_package_script->syntax->types.add(t);
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
		t->size = config.super_array_size;
		t->is_super_array = true;
		script_make_super_array(t);
	}else{
		// standard array
		t->size = sub_type->size * array_length;
		t->is_array = true;
		t->array_length = array_length;
	}
	cur_package_script->syntax->types.add(t);
	return t;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")
int NumPrimitiveOperators = NUM_PRIMITIVE_OPERATORS;

PrimitiveOperator PrimitiveOperators[NUM_PRIMITIVE_OPERATORS]={
	{"=",	OPERATOR_ASSIGN,			true,	1,	"__assign__"},
	{"+",	OPERATOR_ADD,			false,	11,	"__add__"},
	{"-",	OPERATOR_SUBTRACT,		false,	11,	"__sub__"},
	{"*",	OPERATOR_MULTIPLY,		false,	12,	"__mul__"},
	{"/",	OPERATOR_DIVIDE,			false,	12,	"__div__"},
	{"+=",	OPERATOR_ADDS,			true,	1,	"__iadd__"},
	{"-=",	OPERATOR_SUBTRACTS,		true,	1,	"__isub__"},
	{"*=",	OPERATOR_MULTIPLYS,		true,	1,	"__imul__"},
	{"/=",	OPERATOR_DIVIDES,		true,	1,	"__idiv__"},
	{"==",	OPERATOR_EQUAL,			false,	8,	"__eq__"},
	{"!=",	OPERATOR_NOTEQUAL,		false,	8,	"__ne__"},
	{"!",	OPERATOR_NEGATE,			false,	2,	"__not__"},
	{"<",	OPERATOR_SMALLER,		false,	9,	"__lt__"},
	{">",	OPERATOR_GREATER,		false,	9,	"__gt__"},
	{"<=",	OPERATOR_SMALLER_EQUAL,	false,	9,	"__le__"},
	{">=",	OPERATOR_GREATER_EQUAL,	false,	9,	"__ge__"},
	{"and",	OPERATOR_AND,			false,	4,	"__and__"},
	{"or",	OPERATOR_OR,				false,	3,	"__or__"},
	{"%",	OPERATOR_MODULO,			false,	12,	"__mod__"},
	{"&",	OPERATOR_BIT_AND,			false,	7,	"__bitand__"},
	{"|",	OPERATOR_BIT_OR,			false,	5,	"__bitor__"},
	{"<<",	OPERATOR_SHIFT_LEFT,		false,	10,	"__lshift__"},
	{">>",	OPERATOR_SHIFT_RIGHT,		false,	10,	"__rshift__"},
	{"++",	OPERATOR_INCREASE,		true,	2,	"__inc__"},
	{"--",	OPERATOR_DECREASE,		true,	2,	"__dec__"}
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

void class_add_element(const string &name, Type *type, int offset, ScriptFlag flag)
{
	msg_db_f("add_class_el", 4);
	ClassElement e;
	e.name = name;
	e.type = type;
	e.offset = offset;
	e.hidden = ((flag & FLAG_HIDDEN) > 0);
	cur_class->element.add(e);
}

ClassFunction *_class_add_func(Type *c, const ClassFunction &f, ScriptFlag flag)
{
	if ((flag & FLAG_OVERWRITE) > 0){
		foreachi(ClassFunction &ff, c->function, i)
			if (ff.name == f.name){
				ff = f;
				return &ff;
			}
		msg_error("could not overwrite " + c->name + "." + f.name);
	}
	c->function.add(f);
	return &c->function.back();
}

void _class_add_func_virtual(const string &tname, const string &name, Type *return_type, int index, ScriptFlag flag)
{
	//msg_write("virtual: " + tname + "." + name);
	//msg_write(index);
	int cmd = add_func(tname + "." + name + "[virtual]", return_type, NULL, ScriptFlag(flag | FLAG_CLASS));
	cur_func->_class = cur_class;
	cur_class_func = _class_add_func(cur_class, ClassFunction(name, return_type, cur_package_script, cmd), flag);
	cur_class_func->virtual_index = index;
	if (index >= cur_class->vtable.num)
		cur_class->vtable.resize(index + 1);
	cur_class->_vtable_location_compiler_ = cur_class->vtable.data;
	cur_class->_vtable_location_target_ = cur_class->vtable.data;
}

void class_add_func(const string &name, Type *return_type, void *func, ScriptFlag flag)
{
	msg_db_f("add_class_func", 4);
	string tname = cur_class->name;
	if (tname[0] == '-'){
		foreach(Type *t, cur_package_script->syntax->types)
			if ((t->is_pointer) && (t->parent == cur_class))
				tname = t->name;
	}
	int cmd = add_func(tname + "." + name, return_type, func, ScriptFlag(flag | FLAG_CLASS));
	cur_func->_class = cur_class;
	cur_class_func = _class_add_func(cur_class, ClassFunction(name, return_type, cur_package_script, cmd), flag);
}

void class_add_func_virtual(const string &name, Type *return_type, void *func, ScriptFlag flag)
{
	msg_db_f("add_class_func_virtual", 4);
	string tname = cur_class->name;
	if (tname[0] == '-'){
		foreach(Type *t, cur_package_script->syntax->types)
			if ((t->is_pointer) && (t->parent == cur_class))
				tname = t->name;
	}
	if (config.abi == ABI_WINDOWS_32){
		if (!func){
			_class_add_func_virtual(tname, name, return_type, 0, flag);
			return;
		}
		unsigned char *pp = (unsigned char*)func;
		try{
			//if ((cur_class->vtable) && (pp[0] == 0x8b) && (pp[1] == 0x01) && (pp[2] == 0xff) && (pp[3] == 0x60)){
			if ((pp[0] == 0x8b) && (pp[1] == 0x44) && (pp[2] == 0x24) && (pp[4] == 0x8b) && (pp[5] == 0x00) && (pp[6] == 0xff) && (pp[7] == 0x60)){
				// 8b.44.24.**    8b.00     ff.60.10
				// virtual function
				int index = (int)pp[8] / 4;
				_class_add_func_virtual(tname, name, return_type, index, flag);
			}else if (pp[0] == 0xe9){
				// jmp
				//msg_write(Asm::Disassemble(func, 16));
				pp = &pp[5] + *(int*)&pp[1];
				//msg_write(Asm::Disassemble(pp, 16));
				if ((pp[0] == 0x8b) && (pp[1] == 0x44) && (pp[2] == 0x24) && (pp[4] == 0x8b) && (pp[5] == 0x00) && (pp[6] == 0xff) && (pp[7] == 0x60)){
					// 8b.44.24.**    8b.00     ff.60.10
					// virtual function
					int index = (int)pp[8] / 4;
					_class_add_func_virtual(tname, name, return_type, index, flag);
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
			_class_add_func_virtual(tname, name, return_type, index, flag);
		}else if (!func){
			_class_add_func_virtual(tname, name, return_type, 0, flag);
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
	c.value.resize(max(type->size, 8));//config.PointerSize));
	// config.PointerSize might be smaller than needed for the following assignment
	if ((type == TypeInt) || (type == TypeFloat32) || (type == TypeChar)  || (type == TypeBool) || (type->is_pointer))
		*(void**)c.value.data = value;
	else
		memcpy(c.value.data, value, type->size);
	cur_package_script->syntax->constants.add(c);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//


void add_ext_var(const string &name, Type *type, void *var)
{
	cur_package_script->syntax->root_of_all_evil.AddVar(name, type);
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
double _cdecl _Float2Float64(float f){	return (double)f;	}
float _cdecl _Float642Float(double f){	return (float)f;	}
float _cdecl _Int2Float(int i){	return (float)i;	}
int _cdecl _Int642Int(long long i){	return (int)i;	}
long long _cdecl _Int2Int64(int i){	return (long long)i;	}
char _cdecl _Int2Char(int i){	return (char)i;	}
int _cdecl _Char2Int(char c){	return (int)c;	}
bool _cdecl _Pointer2Bool(void *p){	return (p != NULL);	}


Array<PreCommand> PreCommands;

int add_func(const string &name, Type *return_type, void *func, ScriptFlag flag)
{
	Function *f = new Function(cur_package_script->syntax, name, return_type);
	f->literal_return_type = return_type;
	f->num_params = 0;
	f->_class = NULL;
	f->is_pure = ((flag & FLAG_PURE) > 0);
	cur_package_script->syntax->functions.add(f);
	cur_package_script->func.add(config.allow_std_lib ? (void (*)())func : NULL);
	cur_cmd = NULL;
	cur_func = f;
	cur_class_func = NULL;
	return cur_package_script->syntax->functions.num - 1;
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

void func_set_inline(int index)
{
	if (cur_func)
		cur_func->inline_no = index;
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

/*void script_make_super_array_func_headers(Type *t, SyntaxTree *ps, bool pre_define_funcs)
{
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

		// define later...
		if (pre_define_funcs){
		class_add_func("__init__",	TypeVoid, NULL);
		class_add_func("add", TypeVoid, NULL);
			func_add_param("x",		t->parent);
		class_add_func("insert", TypeVoid, NULL);
			func_add_param("x",		t->parent);
			func_add_param("index",		TypeInt);
		class_add_func("__delete__",	TypeVoid, NULL);
		class_add_func("clear", TypeVoid, NULL);
		class_add_func("__assign__", TypeVoid, NULL);
			func_add_param("other",		t);
		class_add_func("remove", TypeVoid, NULL);
			func_add_param("index",		TypeInt);
		class_add_func("resize", TypeVoid, NULL);
			func_add_param("num",		TypeInt);
		class_add_func("ensure_size", TypeVoid, NULL);
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
}*/

void script_make_super_array(Type *t, SyntaxTree *ps)
{
	msg_db_f("make_super_array", 4);

	Type *parent = t->parent;
	t->DeriveFrom(TypeDynamicArray, false);
	t->parent = parent;
	add_class(t);

	ClassFunction *sub = t->GetFunc("subarray", TypeDynamicArray, 2);
	sub->return_type = t;

		// FIXME  wrong for complicated classes
		if (t->parent->is_simple_class()){
			if (!t->parent->UsesCallByReference()){
				if (t->parent->is_pointer){
					class_add_func("__init__",	TypeVoid, mf(&Array<void*>::__init__));
					class_add_func("add", TypeVoid, mf(&Array<void*>::add));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&Array<void*>::insert));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent == TypeFloat32){
					class_add_func("__init__",	TypeVoid, mf(&Array<float>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_f_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_f_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent == TypeFloat64){
					class_add_func("__init__",	TypeVoid, mf(&Array<double>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_d_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_d_single));
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
			class_add_func("resize", TypeVoid, mf(&DynamicArray::resize));
				func_add_param("num",		TypeInt);
		}
}


// automatic type casting

string CastFloat2Int(string &s)
{
	string r;
	r.resize(8);
	*(int*)r.data = (int)*(float*)s.data;
	return r;
}
string CastFloat2Float64(string &s)
{
	string r;
	r.resize(8);
	*(double*)r.data = *(float*)s.data;
	return r;
}
string CastInt2Float(string &s)
{
	string r;
	r.resize(8);
	*(float*)r.data = (float)*(int*)s.data;
	return r;
}
string CastInt2Int64(string &s)
{
	string r;
	r.resize(8);
	*(long long*)r.data = *(int*)s.data;
	return r;
}
string CastInt2Char(string &s)
{
	return s;
}
string CastChar2Int(string &s)
{
	string r;
	r.resize(8);
	*(int*)r.data = *(char*)s.data;
	return r;
}
string CastPointer2Bool(string &s)
{
	string r;
	r.resize(8);
	*(bool*)r.data = (*(void**)s.data != NULL);
	return r;
}
string CastInt2StringP(string &s)
{
	return i2s(*(int*)s.data);
}
string CastInt642StringP(string &s)
{
	return i642s(*(long long*)s.data);
}
string CastFloat2StringP(string &s)
{
	return f2s(*(float*)s.data, 6);
}
string CastFloat642StringP(string &s)
{
	return f642s(*(double*)s.data, 6);
}
string CastBool2StringP(string &s)
{
	return b2s(*(bool*)s.data);
}
string CastPointer2StringP(string &s)
{
	return p2s(*(void**)s.data);
}

Array<TypeCast> TypeCasts;
void add_type_cast(int penalty, Type *source, Type *dest, const string &cmd, void *func)
{
	TypeCast c;
	c.penalty = penalty;
	c.func_no = -1;
	for (int i=0;i<PreCommands.num;i++)
		if (PreCommands[i].name == cmd){
			c.kind = KIND_COMPILER_FUNCTION;
			c.func_no = i;
			c.script = cur_package_script;
			break;
		}
	if (c.func_no < 0)
	for (int i=0;i<cur_package_script->syntax->functions.num;i++)
		if (cur_package_script->syntax->functions[i]->name == cmd){
			c.kind = KIND_FUNCTION;
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

class Int64Class
{
	long long i;
public:
	string _cdecl str(){	return i642s(i);	}
};

class FloatClass
{
	float f;
public:
	string _cdecl str(){	return f2s(f, 6);	}
	string _cdecl str2(int decimals){	return f2s(f, decimals);	}
};

class Float64Class
{
	double f;
public:
	string _cdecl str(){	return f642s(f, 6);	}
	string _cdecl str2(int decimals){	return f642s(f, decimals);	}
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
	TypeInt64			= add_type  ("int64",		sizeof(long long), FLAG_CALL_BY_VALUE);
	TypeFloat32			= add_type  ("float32",		sizeof(float), FLAG_CALL_BY_VALUE);
	TypeFloat64			= add_type  ("float64",		sizeof(double), FLAG_CALL_BY_VALUE);
	TypeChar			= add_type  ("char",		sizeof(char), FLAG_CALL_BY_VALUE);
	TypeDynamicArray	= add_type  ("@DynamicArray", config.super_array_size);



	// select default float type
	TypeFloat = TypeFloat32;
	TypeFloat->name = "float";



	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt, config.pointer_size);
		class_add_func("swap", TypeVoid, mf(&DynamicArray::swap));
			func_add_param("i1", TypeInt);
			func_add_param("i2", TypeInt);
		/*class_add_func("iterate", TypeBool, mf(&DynamicArray::iterate));
			func_add_param("pointer", TypePointerPs);
		class_add_func("iterate_back", TypeBool, mf(&DynamicArray::iterate_back));
			func_add_param("pointer", TypePointerPs);
		class_add_func("index", TypeInt, mf(&DynamicArray::index));
			func_add_param("pointer", TypePointer);*/
		class_add_func("subarray", TypeDynamicArray, mf(&DynamicArray::ref_subarray));
			func_add_param("start", TypeInt);
			func_add_param("num", TypeInt);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, mf(&DynamicArray::init));
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, mf(&DynamicArray::clear));
		class_add_func("__mem_resize__", TypeVoid, mf(&DynamicArray::resize));
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, mf(&DynamicArray::delete_single));
			func_add_param("index", TypeInt);

	// derived   (must be defined after the primitive types!)
	TypePointer			= add_type_p("void*",		TypeVoid, FLAG_CALL_BY_VALUE); // substitute for all pointer types
	TypePointerPs		= add_type_p("void*&",		TypePointer, FLAG_SILENT);
	TypePointerList		= add_type_a("void*[]",		TypePointer, -1);
	TypeBoolPs			= add_type_p("bool&",		TypeBool, FLAG_SILENT);
	TypeBoolList		= add_type_a("bool[]",		TypeBool, -1);
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


	//	add_func_special("f2i",			TypeInt,	(void*)&_Float2Int);
	add_func("f2i",			TypeInt,	(void*)&_Float2Int, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_FLOAT_TO_INT);    // sometimes causes floating point exceptions...
		func_add_param("f",		TypeFloat32);
	add_func("i2f",			TypeFloat32,	(void*)&_Int2Float, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_INT_TO_FLOAT);
		func_add_param("i",		TypeInt);
	add_func("f2f64",			TypeFloat64,	(void*)&_Float2Float64, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_FLOAT_TO_FLOAT64);
		func_add_param("f",		TypeFloat32);
	add_func("f642f",			TypeFloat32,	(void*)&_Float642Float, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_FLOAT64_TO_FLOAT);
		func_add_param("f",		TypeFloat32);
	add_func("i2i64",			TypeInt64,	(void*)&_Int2Int64, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_INT_TO_INT64);
		func_add_param("i",		TypeInt);
	add_func("i642i",			TypeInt,	(void*)&_Int642Int, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_INT64_TO_INT);
		func_add_param("i",		TypeInt64);
	add_func("i2c",			TypeChar,	(void*)&_Int2Char, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_INT_TO_CHAR);
		func_add_param("i",		TypeInt);
	add_func("c2i",			TypeInt,	(void*)&_Char2Int, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_CHAR_TO_INT);
		func_add_param("c",		TypeChar);
	add_func("p2b",			TypeBool,	(void*)&_Pointer2Bool, FLAG_PURE);
		func_set_inline(COMMAND_INLINE_POINTER_TO_BOOL);
		func_add_param("p",		TypePointer);

	
	Type *TypeVirtualTest=add_type  ("VirtualTest",	sizeof(VirtualTest));

	add_class(TypeInt);
		class_add_func("str", TypeString, mf(&IntClass::str), FLAG_PURE);
	add_class(TypeInt64);
		class_add_func("str", TypeString, mf(&Int64Class::str), FLAG_PURE);
		add_class(TypeFloat32);
			class_add_func("str", TypeString, mf(&FloatClass::str), FLAG_PURE);
			class_add_func("str2", TypeString, mf(&FloatClass::str2), FLAG_PURE);
				func_add_param("decimals",		TypeInt);
	add_class(TypeFloat64);
		class_add_func("str", TypeString, mf(&Float64Class::str), FLAG_PURE);
		class_add_func("str2", TypeString, mf(&Float64Class::str2), FLAG_PURE);
			func_add_param("decimals",		TypeInt);
	add_class(TypeBool);
		class_add_func("str", TypeString, mf(&BoolClass::str), FLAG_PURE);
	add_class(TypeChar);
		class_add_func("str", TypeString, mf(&CharClass::str), FLAG_PURE);
	add_class(TypePointer);
		class_add_func("str", TypeString, mf(&PointerClass::str), FLAG_PURE);

	add_class(TypeString);
		class_add_func("__iadd__", TypeVoid, mf(&string::operator+=));
			func_add_param("x",		TypeString);
		class_add_func("__add__", TypeString, mf(&string::operator+), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("__eq__", TypeBool, mf(&string::operator==), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("__ne__", TypeBool, mf(&string::operator!=), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("__lt__", TypeBool, mf(&string::operator<), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("__gt__", TypeBool, mf(&string::operator>), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("__le__", TypeBool, mf(&string::operator<=), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("__ge__", TypeBool, mf(&string::operator>=), FLAG_PURE);
			func_add_param("x",		TypeString);
		class_add_func("substr", TypeString, mf(&string::substr), FLAG_PURE);
			func_add_param("start",		TypeInt);
			func_add_param("length",	TypeInt);
		class_add_func("head", TypeString, mf(&string::head), FLAG_PURE);
			func_add_param("size",		TypeInt);
		class_add_func("tail", TypeString, mf(&string::tail), FLAG_PURE);
			func_add_param("size",		TypeInt);
		class_add_func("find", TypeInt, mf(&string::find), FLAG_PURE);
			func_add_param("str",		TypeString);
			func_add_param("start",		TypeInt);
		class_add_func("compare", TypeInt, mf(&string::compare), FLAG_PURE);
			func_add_param("str",		TypeString);
		class_add_func("icompare", TypeInt, mf(&string::icompare), FLAG_PURE);
			func_add_param("str",		TypeString);
		class_add_func("replace", TypeString, mf(&string::replace), FLAG_PURE);
			func_add_param("sub",		TypeString);
			func_add_param("by",		TypeString);
		class_add_func("explode", TypeStringList, mf(&string::explode), FLAG_PURE);
			func_add_param("str",		TypeString);
		class_add_func("lower", TypeString, mf(&string::lower), FLAG_PURE);
		class_add_func("upper", TypeString, mf(&string::upper), FLAG_PURE);
		class_add_func("reverse", TypeString, mf(&string::reverse), FLAG_PURE);
		class_add_func("hash", TypeInt, mf(&string::hash), FLAG_PURE);
		class_add_func("hex", TypeString, mf(&string::hex), FLAG_PURE);
			func_add_param("inverted",		TypeBool);
		class_add_func("unhex", TypeString, mf(&string::unhex), FLAG_PURE);
		class_add_func("match", TypeBool, mf(&string::match), FLAG_PURE);
			func_add_param("glob",		TypeString);
		class_add_func("int", TypeInt, mf(&string::_int), FLAG_PURE);
		class_add_func("int64", TypeInt64, mf(&string::i64), FLAG_PURE);
		class_add_func("float", TypeFloat32, mf(&string::_float), FLAG_PURE);
		class_add_func("float64", TypeFloat64, mf(&string::f64), FLAG_PURE);
		class_add_func("trim", TypeString, mf(&string::trim), FLAG_PURE);
		class_add_func("dirname", TypeString, mf(&string::dirname), FLAG_PURE);
		class_add_func("basename", TypeString, mf(&string::basename), FLAG_PURE);
		class_add_func("extension", TypeString, mf(&string::extension), FLAG_PURE);

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
		class_add_func("join", TypeString, mf(&StringList::join), FLAG_PURE);
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
	add_compiler_func("-return-",		TypeVoid,	COMMAND_RETURN);
		func_add_param("return_value",	TypeVoid); // return: ParamType will be defined by the parser!
	add_compiler_func("-if-",		TypeVoid,	COMMAND_IF);
		func_add_param("b",	TypeBool);
	add_compiler_func("-if/else-",	TypeVoid,	COMMAND_IF_ELSE);
		func_add_param("b",	TypeBool);
	add_compiler_func("-while-",		TypeVoid,	COMMAND_WHILE);
		func_add_param("b",	TypeBool);
	add_compiler_func("-for-",		TypeVoid,	COMMAND_FOR);
		func_add_param("b",	TypeBool); // internally like a while-loop... but a bit different...
	add_compiler_func("-break-",		TypeVoid,	COMMAND_BREAK);
	add_compiler_func("-continue-",	TypeVoid,	COMMAND_CONTINUE);
	add_compiler_func("-new-",	TypePointer,	COMMAND_NEW);
	add_compiler_func("-delete-",	TypeVoid,	COMMAND_DELETE);
		func_add_param("p",	TypePointer);
	add_compiler_func("sizeof",		TypeInt,	COMMAND_SIZEOF);
		func_add_param("type",	TypeVoid);
	
	add_compiler_func("wait",		TypeVoid,	COMMAND_WAIT);
		func_add_param("time",	TypeFloat32);
	add_compiler_func("wait_rt",		TypeVoid,	COMMAND_WAIT_RT);
		func_add_param("time",	TypeFloat32);
	add_compiler_func("wait_of",		TypeVoid,	COMMAND_WAIT_ONE_FRAME);
	add_compiler_func("-asm-",		TypeVoid,	COMMAND_ASM);
}



void op_int_add(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data + *(int*)b.data;	}
void op_int_sub(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data - *(int*)b.data;	}
void op_int_mul(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data * *(int*)b.data;	}
void op_int_div(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data / *(int*)b.data;	}
void op_int_mod(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data % *(int*)b.data;	}
void op_int_shr(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data >> *(int*)b.data;	}
void op_int_shl(string &r, string &a, string &b)
{	*(int*)r.data = *(int*)a.data << *(int*)b.data;	}
void op_float_add(string &r, string &a, string &b)
{	*(float*)r.data = *(float*)a.data + *(float*)b.data;	}
void op_float_sub(string &r, string &a, string &b)
{	*(float*)r.data = *(float*)a.data - *(float*)b.data;	}
void op_float_mul(string &r, string &a, string &b)
{	*(float*)r.data = *(float*)a.data * *(float*)b.data;	}
void op_float_div(string &r, string &a, string &b)
{	*(float*)r.data = *(float*)a.data / *(float*)b.data;	}

void op_int64_add(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data + *(long long*)b.data;	}
void op_int64_sub(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data - *(long long*)b.data;	}
void op_int64_mul(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data * *(long long*)b.data;	}
void op_int64_div(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data / *(long long*)b.data;	}
void op_int64_mod(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data % *(long long*)b.data;	}
void op_int64_shr(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data >> *(long long*)b.data;	}
void op_int64_shl(string &r, string &a, string &b)
{	*(long long*)r.data = *(long long*)a.data << *(long long*)b.data;	}
void op_float64_add(string &r, string &a, string &b)
{	*(double*)r.data = *(double*)a.data + *(double*)b.data;	}
void op_float64_sub(string &r, string &a, string &b)
{	*(double*)r.data = *(double*)a.data - *(double*)b.data;	}
void op_float64_mul(string &r, string &a, string &b)
{	*(double*)r.data = *(double*)a.data * *(double*)b.data;	}
void op_float64_div(string &r, string &a, string &b)
{	*(double*)r.data = *(double*)a.data / *(double*)b.data;	}

void SIAddOperators()
{
	msg_db_f("SIAddOperators", 3);
	

	// same order as in .h file...
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypePointer,	TypePointer);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypePointer,	TypePointer);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypePointer,	TypePointer);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeChar,		TypeChar);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeChar,		TypeChar);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeChar,		TypeChar);
	add_operator(OPERATOR_ADD,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OPERATOR_SUBTRACTS,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OPERATOR_ADDS,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OPERATOR_SUBTRACT,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OPERATOR_BIT_AND,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OPERATOR_BIT_OR,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OPERATOR_SUBTRACT,		TypeChar,		TypeVoid,		TypeChar);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeBool,		TypeBool);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeBool,		TypeBool); // ???????? char? FIXME
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_AND,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_OR,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OPERATOR_NEGATE,		TypeBool,		TypeVoid,		TypeBool);	
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OPERATOR_ADD,			TypeInt,		TypeInt,		TypeInt,	(void*)op_int_add);
	add_operator(OPERATOR_SUBTRACT,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_mul);
	add_operator(OPERATOR_DIVIDE,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeInt,		TypeInt);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OPERATOR_MODULO,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_mod);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeInt,		TypeInt);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeInt,		TypeInt);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeInt,		TypeInt);
	add_operator(OPERATOR_BIT_AND,		TypeInt,		TypeInt,		TypeInt);
	add_operator(OPERATOR_BIT_OR,			TypeInt,		TypeInt,		TypeInt);
	add_operator(OPERATOR_SHIFT_RIGHT,	TypeInt,		TypeInt,		TypeInt,	(void*)op_int_shr);
	add_operator(OPERATOR_SHIFT_LEFT,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_shl);
	add_operator(OPERATOR_SUBTRACT,		TypeInt,		TypeVoid,		TypeInt);
	add_operator(OPERATOR_INCREASE,		TypeVoid,		TypeInt,		TypeVoid);
	add_operator(OPERATOR_DECREASE,		TypeVoid,		TypeInt,		TypeVoid);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_ADD,			TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_add);
	add_operator(OPERATOR_SUBTRACT,		TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_mul);
	add_operator(OPERATOR_DIVIDE,		TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_MODULO,		TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_mod);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_BIT_AND,		TypeInt64,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_BIT_OR,			TypeInt64,		TypeInt64,		TypeInt64);
	add_operator(OPERATOR_SHIFT_RIGHT,	TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_shr);
	add_operator(OPERATOR_SHIFT_LEFT,		TypeInt64,		TypeInt64,		TypeInt64,	(void*)op_int64_shl);
	add_operator(OPERATOR_SUBTRACT,		TypeInt64,		TypeVoid,		TypeInt64);
	add_operator(OPERATOR_INCREASE,		TypeVoid,		TypeInt64,		TypeVoid);
	add_operator(OPERATOR_DECREASE,		TypeVoid,		TypeInt64,		TypeVoid);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_ADD,			TypeFloat32,		TypeFloat32,		TypeFloat32,	(void*)op_float_add);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat32,		TypeFloat32,		TypeFloat32,	(void*)op_float_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,		TypeFloat32,		TypeFloat32,	(void*)op_float_mul);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,		TypeFloat32,		TypeInt);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,		TypeInt,		TypeFloat32);
	add_operator(OPERATOR_DIVIDE,		TypeFloat32,		TypeFloat32,		TypeFloat32,	(void*)op_float_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeFloat32,		TypeFloat32);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat32,	TypeVoid,		TypeFloat32);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_ADD,			TypeFloat64,	TypeFloat64,	TypeFloat64,	(void*)op_float64_add);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat64,	TypeFloat64,	TypeFloat64,	(void*)op_float64_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat64,	TypeFloat64,	TypeFloat64,	(void*)op_float64_mul);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat64,	TypeFloat64,	TypeInt);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat64,	TypeInt,		TypeFloat64);
	add_operator(OPERATOR_DIVIDE,		TypeFloat64,	TypeFloat64,	TypeFloat64,	(void*)op_float64_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeFloat64,	TypeFloat64);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat32,		TypeVoid,		TypeFloat64);
//	add_operator(OperatorAssign,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OPERATOR_ADD,			TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OPERATOR_SUBTRACT,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OPERATOR_MULTIPLY,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OPERATOR_MULTIPLY,		TypeComplex,	TypeFloat32,		TypeComplex);
	add_operator(OPERATOR_MULTIPLY,		TypeComplex,	TypeComplex,	TypeFloat32);
	add_operator(OPERATOR_DIVIDE,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeComplex,	TypeComplex);
	add_operator(OPERATOR_SUBTRACT,		TypeComplex,	TypeVoid,		TypeComplex);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeClass,		TypeClass);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeClass,		TypeClass);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeClass,		TypeClass);
	add_operator(OPERATOR_ADD,			TypeVector,		TypeVector,		TypeVector);
	add_operator(OPERATOR_SUBTRACT,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,	TypeVector,		TypeVector);
	add_operator(OPERATOR_MULTIPLY,		TypeVector,		TypeVector,		TypeFloat32);
	add_operator(OPERATOR_MULTIPLY,		TypeVector,		TypeFloat32,		TypeVector);
	add_operator(OPERATOR_DIVIDE,		TypeVector,		TypeVector,		TypeFloat32);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeVector,		TypeVector);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeVector,		TypeVector);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeVector,		TypeFloat32);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeVector,		TypeFloat32);
	add_operator(OPERATOR_SUBTRACT,		TypeVector,		TypeVoid,		TypeVector);
}

void SIAddCommands()
{
	msg_db_f("SIAddCommands", 3);
	
	// type casting
	add_func("@s2i",				TypeInt,		(void*)&s2i);
		func_add_param("s",		TypeString);
	add_func("@s2f",				TypeFloat32,		(void*)&s2f);
		func_add_param("s",		TypeString);
	add_func("@i2s",				TypeString,	(void*)&i2s);
		func_add_param("i",		TypeInt);
	add_func("@i642s",				TypeString,	(void*)&i642s);
		func_add_param("i",		TypeInt64);
	add_func("@f2s",				TypeString,		(void*)&f2s);
		func_add_param("f",			TypeFloat32);
		func_add_param("decimals",	TypeInt);
	add_func("@f2sf",			TypeString,		(void*)&f2sf);
		func_add_param("f",			TypeFloat32);
	add_func("@f642sf",			TypeString,		(void*)&f642sf);
		func_add_param("f",			TypeFloat64);
	add_func("@b2s",				TypeString,	(void*)&b2s);
		func_add_param("b",		TypeBool);
	add_func("p2s",				TypeString,	(void*)&p2s);
		func_add_param("p",		TypePointer);
	add_func("@ia2s",			TypeString,	(void*)&ia2s);
		func_add_param("a",		TypeIntList);
	add_func("@fa2s",			TypeString,	(void*)&fa2s); // TODO...
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
	if (abi < 0){
		if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64){
			abi = ABI_GNU_64;
#ifdef OS_WINDOWS
			abi = ABI_WINDOWS_64;
#endif
		}else if (config.instruction_set == Asm::INSTRUCTION_SET_X86){
			abi = ABI_GNU_32;
#ifdef OS_WINDOWS
			abi = ABI_WINDOWS_32;
#endif
		}else if (config.instruction_set == Asm::INSTRUCTION_SET_ARM){
			abi = ABI_GNU_ARM_32;
		}
	}
	config.abi = abi;
	config.allow_std_lib = allow_std_lib;
	config.pointer_size = Asm::InstructionSet.pointer_size;
	if ((abi >= 0) or (instruction_set >= 0))
		config.super_array_size = mem_align(config.pointer_size + 3 * sizeof(int), config.pointer_size);
	else
		config.super_array_size = sizeof(DynamicArray);
	config.stack_size = SCRIPT_DEFAULT_STACK_SIZE;

	config.allow_simplification = true;
	config.allow_registers = true;
	config.use_const_as_global_var = false;
	config.stack_mem_align = 8;
	config.function_align = 2 * config.pointer_size;
	config.stack_frame_align = 2 * config.pointer_size;

	config.compile_silently = false;
	config.verbose = false;
	config.show_compiler_stats = true;

	config.compile_os = false;
	config.overwrite_variables_offset = false;
	config.variables_offset = 0;
	config.overwrite_code_origin = false;
	config.code_origin = 0;
	config.add_entry_point = false;
	config.no_function_frame = false;

	SIAddPackageBase();
	SIAddBasicCommands();




	SIAddPackageMath();
	SIAddPackageFile();
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



	add_type_cast(10,	TypeInt,		TypeFloat32,	"i2f",	(void*)&CastInt2Float);
	add_type_cast(10,	TypeInt,		TypeInt64,	"i2i64",	(void*)&CastInt2Int64);
	add_type_cast(10,	TypeFloat32,		TypeFloat64,"f2f64",	(void*)&CastFloat2Float64);
	add_type_cast(20,	TypeFloat32,		TypeInt,	"f2i",	(void*)&CastFloat2Int);
	add_type_cast(10,	TypeInt,		TypeChar,	"i2c",	(void*)&CastInt2Char);
	add_type_cast(20,	TypeChar,		TypeInt,	"c2i",	(void*)&CastChar2Int);
	add_type_cast(50,	TypePointer,	TypeBool,	"p2b",	(void*)&CastPointer2Bool);
	add_type_cast(50,	TypeInt,		TypeString,	"@i2s",	(void*)&CastInt2StringP);
	add_type_cast(50,	TypeInt64,		TypeString,	"@i642s",	(void*)&CastInt642StringP);
	add_type_cast(50,	TypeFloat32,		TypeString,	"@f2sf",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeFloat64,	TypeString,	"@f642sf",	(void*)&CastFloat642StringP);
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
			foreachi(Function *f, p.script->syntax->functions, i)
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
