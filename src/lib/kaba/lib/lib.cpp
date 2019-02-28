/*----------------------------------------------------------------------------*\
| Kaba Lib                                                                     |
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
#include "../kaba.h"
#include "common.h"
#include "exception.h"
#include "../../config.h"
#include "../../math/complex.h"



#ifdef _X_USE_HUI_
#include "../../hui/hui.h"
#endif



namespace Kaba{

string LibVersion = "0.17.-1.2";

const string IDENTIFIER_CLASS = "class";
const string IDENTIFIER_FUNC_INIT = "__init__";
const string IDENTIFIER_FUNC_DELETE = "__delete__";
const string IDENTIFIER_FUNC_ASSIGN = "__assign__";
const string IDENTIFIER_FUNC_GET = "__get__";
const string IDENTIFIER_FUNC_SUBARRAY = "__subarray__";
const string IDENTIFIER_SUPER = "super";
const string IDENTIFIER_SELF = "self";
const string IDENTIFIER_EXTENDS = "extends";
const string IDENTIFIER_STATIC = "static";
const string IDENTIFIER_NEW = "new";
const string IDENTIFIER_DELETE = "delete";
const string IDENTIFIER_SIZEOF = "sizeof";
const string IDENTIFIER_TYPE = "type";
const string IDENTIFIER_STR = "str";
const string IDENTIFIER_LEN = "len";
const string IDENTIFIER_NAMESPACE = "namespace";
const string IDENTIFIER_RETURN_VAR = "-return-";
const string IDENTIFIER_VTABLE_VAR = "-vtable-";
const string IDENTIFIER_ENUM = "enum";
const string IDENTIFIER_CONST = "const";
const string IDENTIFIER_OVERRIDE = "override";
const string IDENTIFIER_VIRTUAL = "virtual";
const string IDENTIFIER_EXTERN = "extern";
const string IDENTIFIER_USE = "use";
const string IDENTIFIER_RETURN = "return";
const string IDENTIFIER_RAISE = "raise";
const string IDENTIFIER_TRY = "try";
const string IDENTIFIER_EXCEPT = "except";
const string IDENTIFIER_IF = "if";
const string IDENTIFIER_ELSE = "else";
const string IDENTIFIER_WHILE = "while";
const string IDENTIFIER_FOR = "for";
const string IDENTIFIER_IN = "in";
const string IDENTIFIER_BREAK = "break";
const string IDENTIFIER_CONTINUE = "continue";
const string IDENTIFIER_PASS = "pass";
const string IDENTIFIER_AND = "and";
const string IDENTIFIER_OR = "or";
const string IDENTIFIER_XOR = "xor";
const string IDENTIFIER_NOT = "not";
const string IDENTIFIER_IS = "is";
const string IDENTIFIER_ASM = "asm";

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

const Class *TypeUnknown;
const Class *TypeReg128;
const Class *TypeReg64;
const Class *TypeReg32;
const Class *TypeReg16;
const Class *TypeReg8;
const Class *TypeVoid;
const Class *TypePointer;
const Class *TypeChunk;
const Class *TypeBool;
const Class *TypeInt;
const Class *TypeInt64;
const Class *TypeFloat;
const Class *TypeFloat32;
const Class *TypeFloat64;
const Class *TypeChar;
const Class *TypeString;
const Class *TypeCString;

const Class *TypeVector;
const Class *TypeRect;
const Class *TypeColor;
const Class *TypeQuaternion;
 // internal:
const Class *TypeDynamicArray;
const Class *TypeDictBase;
const Class *TypePointerList;
const Class *TypeCharPs;
const Class *TypeBoolPs;
const Class *TypeBoolList;
const Class *TypeIntPs;
const Class *TypeIntList;
const Class *TypeIntArray;
const Class *TypeIntDict;
const Class *TypeFloatP;
const Class *TypeFloatPs;
const Class *TypeFloatList;
const Class *TypeFloatArray;
const Class *TypeFloatArrayP;
const Class *TypeFloatDict;
const Class *TypeComplex;
const Class *TypeComplexList;
const Class *TypeStringList;
const Class *TypeStringDict;
const Class *TypeVectorArray;
const Class *TypeVectorArrayP;
const Class *TypeVectorList;
const Class *TypeMatrix;
const Class *TypePlane;
const Class *TypePlaneList;
const Class *TypeMatrix3;
const Class *TypeDate;
const Class *TypeImage;

const Class *TypeException;
const Class *TypeExceptionP;

const Class *TypeClass;
const Class *TypeClassP;
const Class *TypeFunction;
const Class *TypeFunctionP;


Array<Package> Packages;
Script *cur_package_script = nullptr;
int cur_package_index;


static Function *cur_func = nullptr;
static Class *cur_class;
static ClassFunction *cur_class_func = nullptr;


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

const Class *add_type(const string &name, int size, ScriptFlag flag)
{
	Class *t = new Class(name, size, cur_package_script->syntax);
	if ((flag & FLAG_CALL_BY_VALUE) > 0)
		t->force_call_by_value = true;
	cur_package_script->syntax->classes.add(t);
	return t;
}
const Class *add_type_p(const string &name, const Class *sub_type, ScriptFlag flag)
{
	Class *t = new Class(name, config.pointer_size, cur_package_script->syntax);
	t->type = t->Type::POINTER;
	if ((flag & FLAG_SILENT) > 0)
		t->type = t->Type::POINTER_SILENT;
	t->parent = sub_type;
	cur_package_script->syntax->classes.add(t);
	return t;
}
const Class *add_type_a(const string &name, const Class *sub_type, int array_length)
{
	Class *t = new Class(name, 0, cur_package_script->syntax, sub_type);
	if (array_length < 0){
		// super array
		t->size = config.super_array_size;
		t->type = t->Type::SUPER_ARRAY;
		script_make_super_array(t);
	}else{
		// standard array
		t->size = sub_type->size * array_length;
		t->type = t->Type::ARRAY;
		t->array_length = array_length;
	}
	cur_package_script->syntax->classes.add(t);
	return t;
}

const Class *add_type_d(const string &name, const Class *sub_type)
{
	Class *t = new Class(name, config.super_array_size, cur_package_script->syntax, sub_type);
	t->type = t->Type::DICT;
	script_make_dict(t);
	cur_package_script->syntax->classes.add(t);
	return t;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")

PrimitiveOperator PrimitiveOperators[NUM_PRIMITIVE_OPERATORS]={
	{"=",  OPERATOR_ASSIGN,        true,  1, IDENTIFIER_FUNC_ASSIGN},
	{"+",  OPERATOR_ADD,           false, 11, "__add__"},
	{"-",  OPERATOR_SUBTRACT,      false, 11, "__sub__"},
	{"*",  OPERATOR_MULTIPLY,      false, 12, "__mul__"},
	{"/",  OPERATOR_DIVIDE,        false, 12, "__div__"},
	{"+=", OPERATOR_ADDS,          true,  1,  "__iadd__"},
	{"-=", OPERATOR_SUBTRACTS,     true,  1,  "__isub__"},
	{"*=", OPERATOR_MULTIPLYS,     true,  1,  "__imul__"},
	{"/=", OPERATOR_DIVIDES,       true,  1,  "__idiv__"},
	{"==", OPERATOR_EQUAL,         false, 8,  "__eq__"},
	{"!=", OPERATOR_NOTEQUAL,      false, 8,  "__ne__"},
	{"!",  OPERATOR_NEGATE,        false, 2,  "__not__"},
	{"<",  OPERATOR_SMALLER,       false, 9,  "__lt__"},
	{">",  OPERATOR_GREATER,       false, 9,  "__gt__"},
	{"<=", OPERATOR_SMALLER_EQUAL, false, 9,  "__le__"},
	{">=", OPERATOR_GREATER_EQUAL, false, 9,  "__ge__"},
	{IDENTIFIER_AND, OPERATOR_AND, false, 4,  "__and__"},
	{IDENTIFIER_OR,  OPERATOR_OR,  false, 3,  "__or__"},
	{"%",  OPERATOR_MODULO,        false, 12, "__mod__"},
	{"&",  OPERATOR_BIT_AND,       false, 7, "__bitand__"},
	{"|",  OPERATOR_BIT_OR,        false, 5, "__bitor__"},
	{"<<", OPERATOR_SHIFT_LEFT,    false, 10, "__lshift__"},
	{">>", OPERATOR_SHIFT_RIGHT,   false, 10, "__rshift__"},
	{"++", OPERATOR_INCREASE,      true,  2, "__inc__"},
	{"--", OPERATOR_DECREASE,      true,  2, "__dec__"},
	{IDENTIFIER_IS, OPERATOR_IS,   false, 2,  "-none-"},
	{IDENTIFIER_EXTENDS, OPERATOR_EXTENDS, false, 2,  "-none-"}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

//   with type information

void add_operator(int primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, int inline_index, void *func = nullptr)
{
	Operator *o = new Operator;
	o->primitive_id = primitive_op;
	o->return_type = return_type;
	o->param_type_1 = param_type1;
	o->param_type_2 = param_type2;
	add_func(PrimitiveOperators[primitive_op].function_name, return_type, func);
	func_set_inline(inline_index);
	func_add_param("a", param_type1);
	func_add_param("b", param_type2);
	o->f = cur_func;
	o->owner = cur_package_script->syntax;
	cur_package_script->syntax->operators.add(o);
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//


void add_class(const Class *root_type)//, PreScript *ps = NULL)
{
	cur_class = const_cast<Class*>(root_type);
}

void class_add_element(const string &name, const Class *type, int offset, ScriptFlag flag)
{
	ClassElement e;
	e.name = name;
	e.type = type;
	e.offset = offset;
	e.hidden = ((flag & FLAG_HIDDEN) > 0);
	cur_class->elements.add(e);
}
void class_derive_from(const Class *parent, bool increase_size, bool copy_vtable)
{
	cur_class->derive_from(parent, increase_size);
	if (copy_vtable)
		cur_class->vtable = parent->vtable;
}

int _class_override_num_params = -1;

ClassFunction *_class_add_func(const Class *ccc, const ClassFunction &f, ScriptFlag flag)
{
	Class *c = const_cast<Class*>(ccc);
	if ((flag & FLAG_OVERRIDE) > 0){
		foreachi(ClassFunction &ff, c->functions, i)
			if (ff.name == f.name){
				if (_class_override_num_params < 0 or _class_override_num_params == ff.param_types.num){
					ff = f;
					return &ff;
				}
			}
		msg_error("could not override " + c->name + "." + f.name);
	}else{
		// name alone is not enough for matching...
		/*foreachi(ClassFunction &ff, c->functions, i)
			if (ff.name == f.name){
				if (_class_override_num_params < 0 or _class_override_num_params == ff.param_types.num){
					msg_error("missing override " + c->name + "." + f.name);
					break;
				}
			}*/
	}
	c->functions.add(f);
	return &c->functions.back();
}

void _class_add_func_virtual(const string &tname, const string &name, const Class *return_type, int index, ScriptFlag flag)
{
	//msg_write("virtual: " + tname + "." + name);
	//msg_write(index);
	add_func(name, return_type, nullptr, ScriptFlag((flag | FLAG_CLASS) & ~FLAG_OVERRIDE));
	cur_func->long_name = tname + "." + name + "[virtual]";
	cur_func->_class = cur_class;
	cur_class_func = _class_add_func(cur_class, ClassFunction(name, return_type, cur_package_script, cur_func), flag);
	cur_class_func->virtual_index = index;
	if (index >= cur_class->vtable.num)
		cur_class->vtable.resize(index + 1);
	cur_class->_vtable_location_compiler_ = cur_class->vtable.data;
	cur_class->_vtable_location_target_ = cur_class->vtable.data;
}

void class_add_func(const string &name, const Class *return_type, void *func, ScriptFlag flag)
{
	string tname = cur_class->name;
	if (tname[0] == '-'){
		for (const Class *t: cur_package_script->syntax->classes)
			if (t->is_pointer() and (t->parent == cur_class))
				tname = t->name;
	}
	add_func(name, return_type, func, ScriptFlag(flag | FLAG_CLASS));
	cur_func->long_name = tname + "." + name;
	cur_func->_class = cur_class;
	cur_class_func = _class_add_func(cur_class, ClassFunction(name, return_type, cur_package_script, cur_func), flag);
}

int get_virtual_index(void *func, const string &tname, const string &name)
{
	if (config.abi == ABI_WINDOWS_32){
		if (!func)
			return 0;
		unsigned char *pp = (unsigned char*)func;
		try {
			//if ((cur_class->vtable) and (pp[0] == 0x8b) and (pp[1] == 0x01) and (pp[2] == 0xff) and (pp[3] == 0x60)){
			if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
				// 8b.44.24.**    8b.00     ff.60.10
				// virtual function
				return (int)pp[8] / 4;
			}else if (pp[0] == 0xe9){
				// jmp
				//msg_write(Asm::Disassemble(func, 16));
				pp = &pp[5] + *(int*)&pp[1];
				//msg_write(Asm::Disassemble(pp, 16));
				if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
					// 8b.44.24.**    8b.00     ff.60.10
					// virtual function
					return (int)pp[8] / 4;
				}else
					throw(1);
			}else
				throw(1);
		}catch (...){
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
			msg_write(string((char*)pp, 4).hex());
			msg_write(Asm::Disassemble(func, 16));
		}
	}else{

		int_p p = (int_p)func;
		if ((p & 1) > 0){
			// virtual function
			return p / sizeof(void*);
		}else if (!func){
			return 0;
		}else{
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
		}
	}
	return -1;
}

void class_add_func_virtual(const string &name, const Class *return_type, void *func, ScriptFlag flag)
{
	string tname = cur_class->name;
	if (tname[0] == '-'){
		for (auto *t: cur_package_script->syntax->classes)
			if ((t->is_pointer()) and (t->parent == cur_class))
				tname = t->name;
	}
	int index = get_virtual_index(func, tname, name);
	_class_add_func_virtual(tname, name, return_type, index, flag);
}

void class_link_vtable(void *p)
{
	cur_class->link_external_virtual_table(p);
}


//------------------------------------------------------------------------------------------------//
//                                           constants                                            //
//------------------------------------------------------------------------------------------------//

void add_const(const string &name, const Class *type, void *value)
{
	Constant *c = new Constant(type, cur_package_script->syntax);
	c->name = name;
	c->address = c->p();

	// config.PointerSize might be smaller than needed for the following assignment
	if ((type == TypeInt) or (type == TypeFloat32) or (type == TypeChar)  or (type == TypeBool) or (type->is_pointer()))
		*(void**)c->p() = value;
	else
		memcpy(c->p(), value, type->size);
	cur_package_script->syntax->constants.add(c);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//


void add_ext_var(const string &name, const Class *type, void *var)
{
	auto *v = cur_package_script->syntax->root_of_all_evil.block->add_var(name, type);
	if (config.allow_std_lib)
		v->memory = var;
};

//------------------------------------------------------------------------------------------------//
//                                      compiler functions                                        //
//------------------------------------------------------------------------------------------------//



#ifndef OS_WINDOWS
	//#define _cdecl
	#include <stdlib.h>
#endif

void _cdecl _cstringout(char *str)
{	msg_write(str);	}
void _cdecl _print(string &str)
{	msg_write(str);	}
void _cdecl _printi(int i)
{	msg_write(i);	}
int _cdecl _Float2Int(float f)
{	return (int)f;	}
double _cdecl _Float2Float64(float f)
{	return (double)f;	}
float _cdecl _Float642Float(double f)
{	return (float)f;	}
float _cdecl _Int2Float(int i)
{	return (float)i;	}
int _cdecl _Int642Int(int64 i)
{	return (int)i;	}
int64 _cdecl _Int2Int64(int i)
{	return (int64)i;	}
char _cdecl _Int2Char(int i)
{	return (char)i;	}
int _cdecl _Char2Int(char c)
{	return (int)c;	}
bool _cdecl _Pointer2Bool(void *p)
{	return (p != nullptr);	}


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

template<class T>
void _ultra_sort(DynamicArray &array, int offset_by)
{
	T *p = (T*)((char*)array.data + offset_by);
	for (int i=0; i<array.num; i++){
		T *q = (T*)((char*)p + array.element_size);
		for (int j=i+1; j<array.num; j++){
			if (*p > *q)
				array.swap(i, j);
			q = (T*)((char*)q + array.element_size);
		}
		p = (T*)((char*)p + array.element_size);
	}
}

template<class T>
void _ultra_sort_p(DynamicArray &array, int offset_by)
{
	char **p = (char**)array.data;
	for (int i=0; i<array.num; i++){
		T *pp = (T*)(*p + offset_by);
		char **q = p + 1;
		for (int j=i+1; j<array.num; j++){
			T *qq = (T*)(*q + offset_by);
			if (*pp > *qq){
				array.swap(i, j);
				pp = (T*)(*p + offset_by);
			}
			q ++;
		}
		p ++;
	}
}

void _cdecl ultra_sort(DynamicArray &array, const Class *type, const string &by)
{
	if (!type->is_super_array())
		kaba_raise_exception(new KabaException("type '" + type->name + "' is not an array"));
	const Class *el = type->parent;
	if (array.element_size != el->size)
		kaba_raise_exception(new KabaException("element type size mismatch..."));

	const Class *rel = el;

	if (el->is_pointer()){
		rel = el->parent;
	}

	ClassElement *ell = nullptr;
	for (auto &e: rel->elements)
		if (e.name == by)
			ell = &e;
	if (!ell)
		kaba_raise_exception(new KabaException("type '" + rel->name + "' does not have an element '" + by + "'"));
	int offset = ell->offset;


	if (el->is_pointer()){
		if (ell->type == TypeString)
			_ultra_sort_p<string>(array, offset);
		else if (ell->type == TypeInt)
			_ultra_sort_p<int>(array, offset);
		else if (ell->type == TypeFloat)
			_ultra_sort_p<float>(array, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by '" + ell->name + " " + by + "'"));
	}else{
		if (ell->type == TypeString)
			_ultra_sort<string>(array, offset);
		else if (ell->type == TypeInt)
			_ultra_sort<int>(array, offset);
		else if (ell->type == TypeFloat)
			_ultra_sort<float>(array, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by '" + ell->name + " " + by + "'"));
	}
}

string _cdecl var2str(const void *var, const Class *type)
{
	return type->var2str(var);
}

string _cdecl kaba_shell_execute(const string &cmd)
{
	try{
		return shell_execute(cmd);
	}catch(::Exception &e){
		kaba_raise_exception(new KabaException(e.message()));
	}
	return "";
}

#pragma GCC pop_options


Array<Statement> Statements;

int add_func(const string &name, const Class *return_type, void *func, ScriptFlag flag)
{
	Function *f = new Function(name, return_type, cur_package_script->syntax);
	f->is_pure = ((flag & FLAG_PURE) > 0);
	f->throws_exceptions = ((flag & FLAG_RAISES_EXCEPTIONS) > 0);
	cur_package_script->syntax->functions.add(f);
	if (config.allow_std_lib)
		f->address = func;
	cur_func = f;
	cur_class_func = nullptr;
	return cur_package_script->syntax->functions.num - 1;
}

int add_statement(const string &name, int index, int num_params = 0)
{
	Statement s;
	s.name = name;
	s.num_params = num_params;
	if (Statements.num < NUM_STATEMENTS)
		Statements.resize(NUM_STATEMENTS);
	Statements[index] = s;
	return index;
}

void func_set_inline(int index)
{
	if (cur_func)
		cur_func->inline_no = index;
}

void func_add_param(const string &name, const Class *type)
{
	if (cur_func){
		Variable *v = new Variable(name, type);
		cur_func->var.add(v);
		cur_func->literal_param_type.add(type);
		cur_func->num_params ++;
	}
	if (cur_class_func)
		cur_class_func->param_types.add(type);
}

void script_make_super_array(Class *t, SyntaxTree *ps)
{
	const Class *parent = t->parent;
	t->derive_from(TypeDynamicArray, false);
	t->parent = parent;
	add_class(t);

	ClassFunction *sub = t->get_func(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, {nullptr,nullptr});
	sub->return_type = t;

		// FIXME  wrong for complicated classes
		if (t->parent->is_simple_class()){
			if (!t->parent->uses_call_by_reference()){
				if (t->parent->is_pointer()){
					class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Array<void*>::__init__));
					class_add_func("add", TypeVoid, mf(&Array<void*>::add));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&Array<void*>::insert));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent == TypeFloat32){
					class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Array<float>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_f_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_f_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent == TypeFloat64){
					class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Array<double>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_d_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_d_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent->size == 4){
					class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Array<int>::__init__));
					class_add_func("add", TypeVoid, mf(&DynamicArray::append_4_single));
						func_add_param("x",		t->parent);
					class_add_func("insert", TypeVoid, mf(&DynamicArray::insert_4_single));
						func_add_param("x",		t->parent);
						func_add_param("index",		TypeInt);
				}else if (t->parent->size == 1){
					class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Array<char>::__init__));
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
			class_add_func(IDENTIFIER_FUNC_DELETE,	TypeVoid, mf(&DynamicArray::clear));
			class_add_func("clear", TypeVoid, mf(&DynamicArray::clear));
			class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&DynamicArray::assign));
				func_add_param("other",		t);
			class_add_func("remove", TypeVoid, mf(&DynamicArray::delete_single));
				func_add_param("index",		TypeInt);
			class_add_func("resize", TypeVoid, mf(&DynamicArray::resize));
				func_add_param("num",		TypeInt);
		}
}


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

class IntDict : public Map<string,int>
{
public:
	void set_int(const string &k, int v)
	{ set(k, v); }
	int get_int(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return 0; }
	void assign(const IntDict &o)
	{ *this = o; }
	string str()
	{
		string s;
		for (string &k: keys()){
			if (s != "")
				s += ", ";
			s += "\"" + k + "\": " + i2s((*this)[k]);
		}
		return "{" + s + "}";
	}
};

class FloatDict : public Map<string,float>
{
public:
	void set_float(const string &k, float v)
	{ set(k, v); }
	float get_float(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return 0.0f; }
	string str()
	{
		string s;
		for (string &k: keys()){
			if (s != "")
				s += ", ";
			s += "\"" + k + "\": " + f2s((*this)[k], 6);
		}
		return "{" + s + "}";
	}
};

class StringDict : public Map<string,string>
{
public:
	string get_string(const string &k)
	{ KABA_EXCEPTION_WRAPPER(return (*this)[k]); return ""; }
	void assign(const StringDict &o)
	{ msg_write("assign");*this = o; }
	string str()
	{
		string s;
		for (string &k: keys()){
			if (s != "")
				s += ", ";
			s += "\"" + k + "\": \"" + (*this)[k] + "\"";
		}
		return "{" + s + "}";
	}
};
#pragma GCC pop_options

void script_make_dict(Class *t, SyntaxTree *ps)
{
	const Class *parent = t->parent;
	t->derive_from(TypeDictBase, false);
	t->parent = parent;
	add_class(t);

	if (t->parent->is_simple_class()){
		// elements don't need a destructor
		class_add_func(IDENTIFIER_FUNC_DELETE,	TypeVoid, mf(&Map<string,int>::clear));
		class_add_func("clear", TypeVoid, mf(&Map<string,int>::clear));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&IntDict::assign));
			func_add_param("other",		t);
	}

	if (t->parent == TypeInt){
		class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Map<string,int>::__init__));
		class_add_func("set", TypeVoid, mf(&IntDict::set_int));
			func_add_param("key",		TypeString);
			func_add_param("x",		t->parent);
		class_add_func("__get__", t->parent, mf(&IntDict::get_int), FLAG_RAISES_EXCEPTIONS);
			func_add_param("key",		TypeString);
		class_add_func("str", TypeString, mf(&IntDict::str));
	}else if (t->parent == TypeFloat){
		class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Map<string,float>::__init__));
		class_add_func("set", TypeVoid, mf(&FloatDict::set_float));
			func_add_param("key",		TypeString);
			func_add_param("x",		t->parent);
		class_add_func("__get__", t->parent, mf(&FloatDict::get_float), FLAG_RAISES_EXCEPTIONS);
			func_add_param("key",		TypeString);
		class_add_func("str", TypeString, mf(&FloatDict::str));
	}else if (t->parent == TypeString){
		class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&Map<string,string>::__init__));
		class_add_func("set", TypeVoid, mf(&Map<string,string>::set));
			func_add_param("key",		TypeString);
			func_add_param("x",		t->parent);
		class_add_func("__get__", t->parent, mf(&StringDict::get_string), FLAG_RAISES_EXCEPTIONS);
			func_add_param("key",		TypeString);
		class_add_func(IDENTIFIER_FUNC_DELETE,	TypeVoid, mf(&Map<string,string>::clear));
		class_add_func("clear", TypeVoid, mf(&Map<string,string>::clear));
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, mf(&StringDict::assign));
			func_add_param("other",		t);
		class_add_func("str", TypeString, mf(&StringDict::str));
	}
}


// automatic type casting

void CastFloat2Int(Value &r, Value &s)
{
	r.init(TypeInt);
	r.as_int() = (int)s.as_float();
}
void CastFloat2Float64(Value &r, Value &s)
{
	r.init(TypeFloat64);
	r.as_float64() = (double)s.as_float();
}
void CastInt2Float(Value &r, Value &s)
{
	r.init(TypeFloat);
	r.as_float() = (float)s.as_int();
}
void CastInt2Int64(Value &r, Value &s)
{
	r.init(TypeInt64);
	r.as_int64() = (int64)s.as_int();
}
void CastInt2Char(Value &r, Value &s)
{
	r.init(TypeChar);
	r.as_int() = s.as_int();
}
void CastChar2Int(Value &r, Value &s)
{
	r.init(TypeInt);
	r.as_int() = s.as_int();
}
void CastPointer2Bool(Value &r, Value &s)
{
	r.init(TypeBool);
	r.as_int() = ((*(void**)s.p()) != nullptr);
}
void CastInt2String(Value &r, Value &s)
{
	r.init(TypeString);
	r.as_string() = i2s(s.as_int());
}
void CastInt642String(Value &r, Value &s)
{
	r.init(TypeString);
	r.as_string() = i642s(s.as_int64());
}
void CastFloat2String(Value &r, Value &s)
{
	r.init(TypeString);
	r.as_string() = f2s(s.as_float(), 6);
}
void CastFloat642String(Value &r, Value &s)
{
	r.init(TypeString);
	r.as_string() = f642s(s.as_float64(), 6);
}
void CastBool2String(Value &r, Value &s)
{
	r.init(TypeString);
	r.as_string() = b2s((bool)s.as_int());
}
void CastPointer2String(Value &r, Value &s)
{
	r.init(TypeString);
	r.as_string() = p2s(*(void**)s.p());
}

Array<TypeCast> TypeCasts;
void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd, void *func)
{
	TypeCast c;
	c.penalty = penalty;
	c.f = nullptr;
	for (auto *f: cur_package_script->syntax->functions)
		if (f->long_name == cmd){
			c.f = f;
			break;
		}
	if (!c.f){
#ifdef _X_USE_HUI_
		hui::ErrorBox(nullptr, "", "add_type_cast (ScriptInit): " + string(cmd) + " not found");
		hui::RaiseError("add_type_cast (ScriptInit): " + string(cmd) + " not found");
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
	{ return implode(*this, glue); }
};

class IntClass
{
	int i;
public:
	string _cdecl str(){	return i2s(i);	}
};

class Int64Class
{
	int64 i;
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



int xop_int_add(int a, int b)
{
	return a + b;
}



void SIAddPackageBase()
{
	add_package("base", true);

	// internal
	TypeUnknown			= add_type  ("-unknown-",	0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type  ("-reg128-",		16, FLAG_CALL_BY_VALUE);
	TypeReg64			= add_type  ("-reg64-",		8, FLAG_CALL_BY_VALUE);
	TypeReg32			= add_type  ("-reg32-",		4, FLAG_CALL_BY_VALUE);
	TypeReg16			= add_type  ("-reg16-",		2, FLAG_CALL_BY_VALUE);
	TypeReg8			= add_type  ("-reg8-",		1, FLAG_CALL_BY_VALUE);
	TypeChunk			= add_type  ("-chunk-",	0); // substitute for all plane-old-data types

	// "real"
	TypeVoid			= add_type  ("void",		0, FLAG_CALL_BY_VALUE);
	TypeBool			= add_type  ("bool",		sizeof(bool), FLAG_CALL_BY_VALUE);
	TypeInt				= add_type  ("int",			sizeof(int), FLAG_CALL_BY_VALUE);
	TypeInt64			= add_type  ("int64",		sizeof(int64), FLAG_CALL_BY_VALUE);
	TypeFloat32			= add_type  ("float32",		sizeof(float), FLAG_CALL_BY_VALUE);
	TypeFloat64			= add_type  ("float64",		sizeof(double), FLAG_CALL_BY_VALUE);
	TypeChar			= add_type  ("char",		sizeof(char), FLAG_CALL_BY_VALUE);
	TypeDynamicArray	= add_type  ("@DynamicArray", config.super_array_size);
	TypeDictBase		= add_type  ("@DictBase",   config.super_array_size);

	TypeException		= add_type  ("Exception",	sizeof(KabaException));
	TypeExceptionP		= add_type_p("Exception*", TypeException);

	TypeClass 			= add_type  ("Class",	sizeof(Class));
	TypeClassP			= add_type_p("Class*", TypeClass);

	TypeFunction		= add_type  ("func", sizeof(Function));
	TypeFunctionP		= add_type_p("func*", TypeFunction);


	// select default float type
	TypeFloat = TypeFloat32;
	(const_cast<Class*>(TypeFloat))->name = "float";



	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt, config.pointer_size);
		class_add_func("swap", TypeVoid, mf(&DynamicArray::swap));
			func_add_param("i1", TypeInt);
			func_add_param("i2", TypeInt);
		class_add_func(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, mf(&DynamicArray::ref_subarray));
			func_add_param("start", TypeInt);
			func_add_param("end", TypeInt);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, mf(&DynamicArray::init));
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, mf(&DynamicArray::clear));
		class_add_func("__mem_resize__", TypeVoid, mf(&DynamicArray::resize));
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, mf(&DynamicArray::delete_single));
			func_add_param("index", TypeInt);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt, config.pointer_size);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, mf(&DynamicArray::init));
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, mf(&DynamicArray::clear));
		class_add_func("__mem_resize__", TypeVoid, mf(&DynamicArray::resize));
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, mf(&DynamicArray::delete_single));
			func_add_param("index", TypeInt);

	// derived   (must be defined after the primitive types!)
	TypePointer     = add_type_p("void*",     TypeVoid, FLAG_CALL_BY_VALUE); // substitute for all pointer types
	TypePointerList = add_type_a("void*[]",   TypePointer, -1);
	TypeBoolPs      = add_type_p("bool&",     TypeBool, FLAG_SILENT);
	TypeBoolList    = add_type_a("bool[]",    TypeBool, -1);
	TypeIntPs       = add_type_p("int&",      TypeInt, FLAG_SILENT);
	TypeIntList     = add_type_a("int[]",     TypeInt, -1);
	TypeIntArray    = add_type_a("int[?]",    TypeInt, 1);
	TypeFloatP      = add_type_p("float*",    TypeFloat);
	TypeFloatPs     = add_type_p("float&",    TypeFloat, FLAG_SILENT);
	TypeFloatArray  = add_type_a("float[?]",  TypeFloat, 1);
	TypeFloatArrayP = add_type_p("float[?]*", TypeFloatArray);
	TypeFloatList   = add_type_a("float[]",   TypeFloat, -1);
	TypeCharPs      = add_type_p("char&",     TypeChar, FLAG_SILENT);
	TypeCString     = add_type_a("cstring",   TypeChar, 256);	// cstring := char[256]
	TypeString      = add_type_a("string",    TypeChar, -1);	// string := char[]
	TypeStringList  = add_type_a("string[]",  TypeString, -1);

	TypeIntDict     = add_type_d("int{}",     TypeInt);
	TypeFloatDict   = add_type_d("float{}",   TypeFloat);
	TypeStringDict  = add_type_d("string{}",  TypeString);


	add_class(TypeClass);
		class_add_element("name", TypeString, offsetof(Class, name));
		class_add_element("size", TypeInt, offsetof(Class, size));
		class_add_element("parent", TypeClassP, offsetof(Class, parent));
		class_add_func("is_derived_from", TypeBool, mf(&Class::is_derived_from));
			func_add_param("c", TypeClassP);
		class_add_func("var2str", TypeString, mf(&Class::var2str));
			func_add_param("var", TypePointer);

	add_class(TypeFunction);
		class_add_element("name", TypeString, offsetof(Function, name));
		class_add_element("long_name", TypeString, offsetof(Function, long_name));
		class_add_element("class", TypeClassP, offsetof(Function, _class));


	//	add_func_special("f2i",			TypeInt,	(void*)&_Float2Int);
	add_func("f2i",			TypeInt,	(void*)&_Float2Int, FLAG_PURE);
		func_set_inline(INLINE_FLOAT_TO_INT);    // sometimes causes floating point exceptions...
		func_add_param("f",		TypeFloat32);
	add_func("i2f",			TypeFloat32,	(void*)&_Int2Float, FLAG_PURE);
		func_set_inline(INLINE_INT_TO_FLOAT);
		func_add_param("i",		TypeInt);
	add_func("f2f64",			TypeFloat64,	(void*)&_Float2Float64, FLAG_PURE);
		func_set_inline(INLINE_FLOAT_TO_FLOAT64);
		func_add_param("f",		TypeFloat32);
	add_func("f642f",			TypeFloat32,	(void*)&_Float642Float, FLAG_PURE);
		func_set_inline(INLINE_FLOAT64_TO_FLOAT);
		func_add_param("f",		TypeFloat32);
	add_func("i2i64",			TypeInt64,	(void*)&_Int2Int64, FLAG_PURE);
		func_set_inline(INLINE_INT_TO_INT64);
		func_add_param("i",		TypeInt);
	add_func("i642i",			TypeInt,	(void*)&_Int642Int, FLAG_PURE);
		func_set_inline(INLINE_INT64_TO_INT);
		func_add_param("i",		TypeInt64);
	add_func("i2c",			TypeChar,	(void*)&_Int2Char, FLAG_PURE);
		func_set_inline(INLINE_INT_TO_CHAR);
		func_add_param("i",		TypeInt);
	add_func("c2i",			TypeInt,	(void*)&_Char2Int, FLAG_PURE);
		func_set_inline(INLINE_CHAR_TO_INT);
		func_add_param("c",		TypeChar);
	add_func("p2b",			TypeBool,	(void*)&_Pointer2Bool, FLAG_PURE);
		func_set_inline(INLINE_POINTER_TO_BOOL);
		func_add_param("p",		TypePointer);


	add_class(TypePointer);
		class_add_func("str", TypeString, mf(&PointerClass::str), FLAG_PURE);


	add_class(TypeInt);
		class_add_func("str", TypeString, mf(&IntClass::str), FLAG_PURE);
		class_add_func("add", TypeInt, (void*)&xop_int_add, FLAG_PURE);
			func_set_inline(INLINE_INT_ADD);
			func_add_param("b", TypeInt);


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
		class_add_func("md5", TypeString, mf(&string::md5), FLAG_PURE);
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

		class_add_func("escape", TypeString, mf(&str_escape), FLAG_PURE);
		class_add_func("unescape", TypeString, mf(&str_unescape), FLAG_PURE);


	add_class(TypeStringList);
		class_add_func(IDENTIFIER_FUNC_INIT,	TypeVoid, mf(&StringList::__init__));
		class_add_func(IDENTIFIER_FUNC_DELETE,	TypeVoid, mf(&StringList::clear));
		class_add_func("add", TypeVoid, mf(&StringList::add));
			func_add_param("x",		TypeString);
		class_add_func("clear", TypeVoid, mf(&StringList::clear));
		class_add_func("remove", TypeVoid, mf(&StringList::erase));
			func_add_param("index",		TypeInt);
		class_add_func("resize", TypeVoid, mf(&StringList::resize));
			func_add_param("num",		TypeInt);
		class_add_func(IDENTIFIER_FUNC_ASSIGN,	TypeVoid, mf(&StringList::assign));
			func_add_param("other",		TypeStringList);
		class_add_func("join", TypeString, mf(&StringList::join), FLAG_PURE);
			func_add_param("glue",		TypeString);


	// constants
	add_const("nil", TypePointer, nullptr);
	add_const("false", TypeBool, (void*)false);
	add_const("true",  TypeBool, (void*)true);


	add_func("int_add", TypeInt, (void*)&xop_int_add, FLAG_PURE);
		func_set_inline(INLINE_INT_ADD);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);

	add_class(TypeException);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&KabaException::__init__));
			func_add_param("message", TypeString);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, mf(&KabaException::__delete__));
		class_add_func_virtual("message", TypeString, mf(&KabaException::message));
		class_add_element("text", TypeString, config.pointer_size);
		class_set_vtable(KabaException);

	add_func(IDENTIFIER_RAISE, TypeVoid, (void*)&kaba_raise_exception, FLAG_RAISES_EXCEPTIONS);
		func_add_param("e", TypeExceptionP);
}


void SIAddBasicCommands()
{
	// statements
	add_statement(IDENTIFIER_RETURN, STATEMENT_RETURN); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_IF, STATEMENT_IF, 2);
	add_statement("-if/else-",	STATEMENT_IF_ELSE, 3);
	add_statement(IDENTIFIER_WHILE, STATEMENT_WHILE, 2);
	add_statement(IDENTIFIER_FOR, STATEMENT_FOR, 4); // internally like a while-loop... but a bit different...
	add_statement(IDENTIFIER_BREAK, STATEMENT_BREAK);
	add_statement(IDENTIFIER_CONTINUE, STATEMENT_CONTINUE);
	add_statement(IDENTIFIER_NEW, STATEMENT_NEW);
	add_statement(IDENTIFIER_DELETE, STATEMENT_DELETE, 1);
	add_statement(IDENTIFIER_SIZEOF, STATEMENT_SIZEOF, 1);
	add_statement(IDENTIFIER_TYPE, STATEMENT_TYPE, 1);
	add_statement(IDENTIFIER_STR, STATEMENT_STR, 1);
	add_statement(IDENTIFIER_LEN, STATEMENT_LEN, 1);
	add_statement(IDENTIFIER_ASM, STATEMENT_ASM);
	add_statement(IDENTIFIER_TRY, STATEMENT_TRY); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_EXCEPT, STATEMENT_EXCEPT); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_PASS, STATEMENT_PASS);
}



void op_int_add(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() + b.as_int();	}
void op_int_sub(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() - b.as_int();	}
void op_int_mul(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() * b.as_int();	}
void op_int_div(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() / b.as_int();	}
void op_int_mod(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() % b.as_int();	}
void op_int_shr(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() >> b.as_int();	}
void op_int_shl(Value &r, Value &a, Value &b)
{	r.as_int() = a.as_int() << b.as_int();	}
void op_float_add(Value &r, Value &a, Value &b)
{	r.as_float() = a.as_float() + b.as_float();	}
void op_float_sub(Value &r, Value &a, Value &b)
{	r.as_float() = a.as_float() - b.as_float();	}
void op_float_mul(Value &r, Value &a, Value &b)
{	r.as_float() = a.as_float() * b.as_float();	}
void op_float_div(Value &r, Value &a, Value &b)
{	r.as_float() = a.as_float() / b.as_float();	}

void op_int64_add(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() + b.as_int64();	}
void op_int64_add_int(Value &r, Value &a, Value &b)
{
	r.as_int64() = a.as_int64() + b.as_int();
}
void op_int64_sub(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() - b.as_int64();	}
void op_int64_mul(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() * b.as_int64();	}
void op_int64_div(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() / b.as_int64();	}
void op_int64_mod(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() % b.as_int64();	}
void op_int64_shr(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() >> b.as_int64();	}
void op_int64_shl(Value &r, Value &a, Value &b)
{	r.as_int64() = a.as_int64() << b.as_int64();	}
void op_float64_add(Value &r, Value &a, Value &b)
{	r.as_float64() = a.as_float64() + b.as_float64();	}
void op_float64_sub(Value &r, Value &a, Value &b)
{	r.as_float64() = a.as_float64() - b.as_float64();	}
void op_float64_mul(Value &r, Value &a, Value &b)
{	r.as_float64() = a.as_float64() * b.as_float64();	}
void op_float64_div(Value &r, Value &a, Value &b)
{	r.as_float64() = a.as_float64() / b.as_float64();	}

void op_complex_mul(Value &r, Value &a, Value &b)
{	r.as_complex() = a.as_complex() * b.as_complex(); }
void op_complex_div(Value &r, Value &a, Value &b)
{	r.as_complex() = a.as_complex() / b.as_complex(); }

void SIAddOperators()
{
	// same order as in .h file...
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypePointer,	TypePointer, INLINE_POINTER_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypePointer,	TypePointer, INLINE_POINTER_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypePointer,	TypePointer, INLINE_POINTER_NOT_EQUAL);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeChar,		TypeChar, INLINE_CHAR_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeChar,		TypeChar, INLINE_CHAR_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeChar,		TypeChar, INLINE_CHAR_NOT_EQUAL);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeChar,		TypeChar, INLINE_CHAR_GREATER);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeChar,		TypeChar, INLINE_CHAR_GREATER_EQUAL);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeChar,		TypeChar, INLINE_CHAR_SMALLER);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeChar,		TypeChar, INLINE_CHAR_SMALLER_EQUAL);
	add_operator(OPERATOR_ADD,			TypeChar,		TypeChar,		TypeChar, INLINE_CHAR_ADD);
	add_operator(OPERATOR_SUBTRACTS,		TypeChar,		TypeChar,		TypeChar, INLINE_CHAR_SUBTRACT_ASSIGN);
	add_operator(OPERATOR_ADDS,			TypeChar,		TypeChar,		TypeChar, INLINE_CHAR_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACT,		TypeChar,		TypeChar,		TypeChar, INLINE_CHAR_SUBTRACT);
	add_operator(OPERATOR_BIT_AND,		TypeChar,		TypeChar,		TypeChar, INLINE_CHAR_AND);
	add_operator(OPERATOR_BIT_OR,			TypeChar,		TypeChar,		TypeChar, INLINE_CHAR_OR);
	add_operator(OPERATOR_SUBTRACT,		TypeChar,		TypeVoid,		TypeChar, INLINE_CHAR_NEGATE);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeBool,		TypeBool, INLINE_BOOL_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeBool,		TypeBool, INLINE_BOOL_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeBool,		TypeBool, INLINE_BOOL_NOT_EQUAL);
	add_operator(OPERATOR_AND,			TypeBool,		TypeBool,		TypeBool, INLINE_BOOL_AND);
	add_operator(OPERATOR_OR,			TypeBool,		TypeBool,		TypeBool, INLINE_BOOL_OR);
	add_operator(OPERATOR_NEGATE,		TypeBool,		TypeVoid,		TypeBool, INLINE_BOOL_NEGATE);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeInt,		TypeInt, INLINE_INT_ASSIGN);
	add_operator(OPERATOR_ADD,			TypeInt,		TypeInt,		TypeInt, INLINE_INT_ADD,	(void*)op_int_add);
	add_operator(OPERATOR_SUBTRACT,		TypeInt,		TypeInt,		TypeInt, INLINE_INT_SUBTRACT,	(void*)op_int_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeInt,		TypeInt,		TypeInt, INLINE_INT_MULTIPLY,	(void*)op_int_mul);
	add_operator(OPERATOR_DIVIDE,		TypeInt,		TypeInt,		TypeInt, INLINE_INT_DIVIDE,	(void*)op_int_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeInt,		TypeInt, INLINE_INT_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeInt,		TypeInt, INLINE_INT_SUBTRACT_ASSIGN);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeInt,		TypeInt, INLINE_INT_MULTIPLY_ASSIGN);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeInt,		TypeInt, INLINE_INT_DIVIDE_ASSIGN);
	add_operator(OPERATOR_MODULO,		TypeInt,		TypeInt,		TypeInt, INLINE_INT_MODULO,	(void*)op_int_mod);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeInt,		TypeInt, INLINE_INT_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeInt,		TypeInt, INLINE_INT_NOT_EQUAL);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeInt,		TypeInt, INLINE_INT_GREATER);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeInt,		TypeInt, INLINE_INT_GREATER_EQUAL);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeInt,		TypeInt, INLINE_INT_SMALLER);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeInt,		TypeInt, INLINE_INT_SMALLER_EQUAL);
	add_operator(OPERATOR_BIT_AND,		TypeInt,		TypeInt,		TypeInt, INLINE_INT_AND);
	add_operator(OPERATOR_BIT_OR,			TypeInt,		TypeInt,		TypeInt, INLINE_INT_OR);
	add_operator(OPERATOR_SHIFT_RIGHT,	TypeInt,		TypeInt,		TypeInt, INLINE_INT_SHIFT_RIGHT,	(void*)op_int_shr);
	add_operator(OPERATOR_SHIFT_LEFT,		TypeInt,		TypeInt,		TypeInt, INLINE_INT_SHIFT_LEFT,	(void*)op_int_shl);
	add_operator(OPERATOR_SUBTRACT,		TypeInt,		TypeVoid,		TypeInt, INLINE_INT_NEGATE);
	add_operator(OPERATOR_INCREASE,		TypeVoid,		TypeInt,		TypeVoid, INLINE_INT_INCREASE);
	add_operator(OPERATOR_DECREASE,		TypeVoid,		TypeInt,		TypeVoid, INLINE_INT_DECREASE);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeInt64,		TypeInt64, INLINE_INT64_ASSIGN);
	add_operator(OPERATOR_ADD,			TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_ADD,	(void*)op_int64_add);
	add_operator(OPERATOR_ADD,			TypeInt64,		TypeInt64,		TypeInt, INLINE_INT64_ADD_INT,	(void*)op_int64_add_int);
	add_operator(OPERATOR_SUBTRACT,		TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_SUBTRACT,	(void*)op_int64_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_MULTIPLY,	(void*)op_int64_mul);
	add_operator(OPERATOR_DIVIDE,		TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_DIVIDE,	(void*)op_int64_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeInt64,		TypeInt64, INLINE_INT64_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeInt64,		TypeInt64, INLINE_INT64_SUBTRACT_ASSIGN);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeInt64,		TypeInt64, INLINE_INT64_MULTIPLY_ASSIGN);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeInt64,		TypeInt64, INLINE_INT64_DIVIDE_ASSIGN);
	add_operator(OPERATOR_MODULO,		TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_MODULO,	(void*)op_int64_mod);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeInt64,		TypeInt64, INLINE_INT64_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeInt64,		TypeInt64, INLINE_INT64_NOT_EQUAL);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeInt64,		TypeInt64, INLINE_INT64_GREATER);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeInt64,		TypeInt64, INLINE_INT64_GREATER_EQUAL);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeInt64,		TypeInt64, INLINE_INT64_SMALLER);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeInt64,		TypeInt64, INLINE_INT64_SMALLER_EQUAL);
	add_operator(OPERATOR_BIT_AND,		TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_AND);
	add_operator(OPERATOR_BIT_OR,			TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_OR);
	add_operator(OPERATOR_SHIFT_RIGHT,	TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_SHIFT_RIGHT,	(void*)op_int64_shr);
	add_operator(OPERATOR_SHIFT_LEFT,		TypeInt64,		TypeInt64,		TypeInt64, INLINE_INT64_SHIFT_LEFT,	(void*)op_int64_shl);
	add_operator(OPERATOR_SUBTRACT,		TypeInt64,		TypeVoid,		TypeInt64, INLINE_INT64_NEGATE);
	add_operator(OPERATOR_INCREASE,		TypeVoid,		TypeInt64,		TypeVoid, INLINE_INT64_INCREASE);
	add_operator(OPERATOR_DECREASE,		TypeVoid,		TypeInt64,		TypeVoid, INLINE_INT64_DECREASE);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_ASSIGN);
	add_operator(OPERATOR_ADD,			TypeFloat32,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_ADD,	(void*)op_float_add);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat32,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_SUBTARCT,	(void*)op_float_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_MULTIPLY,	(void*)op_float_mul);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,		TypeFloat32,		TypeInt, INLINE_FLOAT_MULTIPLY_FI);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,		TypeInt,		TypeFloat32, INLINE_FLOAT_MULTIPLY_IF);
	add_operator(OPERATOR_DIVIDE,		TypeFloat32,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_DIVIDE,	(void*)op_float_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_SUBTRACT_ASSIGN);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_MULTIPLY_ASSIGN);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_DIVIDE_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_NOT_EQUAL);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_GREATER);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_GREATER_EQUAL);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_SMALLER);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeFloat32,		TypeFloat32, INLINE_FLOAT_SMALLER_EQUAL);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat32,	TypeVoid,		TypeFloat32, INLINE_FLOAT_NEGATE);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_ASSIGN);
	add_operator(OPERATOR_ADD,			TypeFloat64,	TypeFloat64,	TypeFloat64, INLINE_FLOAT64_ADD,	(void*)op_float64_add);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat64,	TypeFloat64,	TypeFloat64, INLINE_FLOAT64_SUBTRACT,	(void*)op_float64_sub);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat64,	TypeFloat64,	TypeFloat64, INLINE_FLOAT64_MULTIPLY,	(void*)op_float64_mul);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat64,	TypeFloat64,	TypeInt, INLINE_FLOAT64_MULTIPLY_FI);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat64,	TypeInt,		TypeFloat64, INLINE_FLOAT64_MULTIPLY_IF);
	add_operator(OPERATOR_DIVIDE,		TypeFloat64,	TypeFloat64,	TypeFloat64, INLINE_FLOAT64_DIVIDE,	(void*)op_float64_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_SUBTRACT_ASSIGN);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_MULTIPLY_ASSIGN);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_DIVIDE_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_NOT_EQUAL);
	add_operator(OPERATOR_GREATER,		TypeBool,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_GREATER);
	add_operator(OPERATOR_GREATER_EQUAL,	TypeBool,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_GREATER_EQUAL);
	add_operator(OPERATOR_SMALLER,		TypeBool,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_SMALLER);
	add_operator(OPERATOR_SMALLER_EQUAL,	TypeBool,		TypeFloat64,	TypeFloat64, INLINE_FLOAT64_SMALLER_EQUAL);
	add_operator(OPERATOR_SUBTRACT,		TypeFloat32,		TypeVoid,		TypeFloat64, INLINE_FLOAT64_NEGATE);
//	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeComplex,	TypeComplex, INLINE_COMPLEX_ASSIGN);
	add_operator(OPERATOR_ADD,			TypeComplex,	TypeComplex,	TypeComplex, INLINE_COMPLEX_ADD);
	add_operator(OPERATOR_SUBTRACT,		TypeComplex,	TypeComplex,	TypeComplex, INLINE_COMPLEX_SUBTRACT);
	add_operator(OPERATOR_MULTIPLY,		TypeComplex,	TypeComplex,	TypeComplex, INLINE_COMPLEX_MULTIPLY, (void*)op_complex_mul);
	add_operator(OPERATOR_MULTIPLY,		TypeComplex,	TypeFloat32,		TypeComplex, INLINE_COMPLEX_MULTIPLY_FC);
	add_operator(OPERATOR_MULTIPLY,		TypeComplex,	TypeComplex,	TypeFloat32, INLINE_COMPLEX_MULTIPLY_CF);
	add_operator(OPERATOR_DIVIDE,		TypeComplex,	TypeComplex,	TypeComplex, -1/*INLINE_COMPLEX_DIVIDE*/, (void*)op_complex_div);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeComplex,	TypeComplex, INLINE_COMPLEX_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeComplex,	TypeComplex, INLINE_COMPLEX_SUBTARCT_ASSIGN);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeComplex,	TypeComplex, INLINE_COMPLEX_MULTIPLY_ASSIGN);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeComplex,	TypeComplex, INLINE_COMPLEX_DIVIDE_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeComplex,	TypeComplex, INLINE_COMPLEX_EQUAL);
	add_operator(OPERATOR_SUBTRACT,		TypeComplex,	TypeVoid,		TypeComplex, INLINE_COMPLEX_NEGATE);
	add_operator(OPERATOR_ASSIGN,		TypeVoid,		TypeChunk,		TypeChunk, INLINE_CHUNK_ASSIGN);
	add_operator(OPERATOR_EQUAL,			TypeBool,		TypeChunk,		TypeChunk, INLINE_CHUNK_EQUAL);
	add_operator(OPERATOR_NOTEQUAL,		TypeBool,		TypeChunk,		TypeChunk, INLINE_CHUNK_NOT_EQUAL);
	add_operator(OPERATOR_ADD,			TypeVector,		TypeVector,		TypeVector, INLINE_VECTOR_ADD);
	add_operator(OPERATOR_SUBTRACT,		TypeVector,		TypeVector,		TypeVector, INLINE_VECTOR_SUBTRACT);
	add_operator(OPERATOR_MULTIPLY,		TypeFloat32,	TypeVector,		TypeVector, INLINE_VECTOR_MULTIPLY_VV);
	add_operator(OPERATOR_MULTIPLY,		TypeVector,		TypeVector,		TypeFloat32, INLINE_VECTOR_MULTIPLY_VF);
	add_operator(OPERATOR_MULTIPLY,		TypeVector,		TypeFloat32,		TypeVector, INLINE_VECTOR_MULTIPLY_FV);
	add_operator(OPERATOR_DIVIDE,		TypeVector,		TypeVector,		TypeFloat32, INLINE_VECTOR_DIVIDE_VF);
	add_operator(OPERATOR_ADDS,			TypeVoid,		TypeVector,		TypeVector, INLINE_VECTOR_ADD_ASSIGN);
	add_operator(OPERATOR_SUBTRACTS,		TypeVoid,		TypeVector,		TypeVector, INLINE_VECTOR_SUBTARCT_ASSIGN);
	add_operator(OPERATOR_MULTIPLYS,		TypeVoid,		TypeVector,		TypeFloat32, INLINE_VECTOR_MULTIPLY_ASSIGN);
	add_operator(OPERATOR_DIVIDES,		TypeVoid,		TypeVector,		TypeFloat32, INLINE_VECTOR_DIVIDE_ASSIGN);
	add_operator(OPERATOR_SUBTRACT,		TypeVector,		TypeVoid,		TypeVector, INLINE_VECTOR_NEGATE);
}

void SIAddCommands()
{
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
	add_func("print",			TypeVoid,		(void*)&_print);
		func_add_param("str",	TypeString);
	// memory
	add_func("@malloc",			TypePointer,		(void*)&malloc);
		func_add_param("size",	TypeInt);
	add_func("@free",			TypeVoid,		(void*)&free);
		func_add_param("p",	TypePointer);
	// system
	add_func("shell_execute",	TypeString,	(void*)&kaba_shell_execute, FLAG_RAISES_EXCEPTIONS);
		func_add_param("cmd",	TypeString);


	add_func("sort_list",	TypeVoid,	(void*)&ultra_sort, FLAG_RAISES_EXCEPTIONS);
		func_add_param("list",	TypePointer);
		func_add_param("class",	TypeClassP);
		func_add_param("by",	TypeString);
	add_func("var2str", TypeString, (void*)var2str, FLAG_RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);


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
	config.stack_size = DEFAULT_STACK_SIZE;

	config.allow_simplification = true;
	config.allow_registers = true;
	config.use_const_as_global_var = false;
	config.stack_mem_align = 8;
	config.function_align = 2 * config.pointer_size;
	config.stack_frame_align = 2 * config.pointer_size;

	config.compile_silently = false;
	config.verbose = false;
	config.verbose_func_filter = "*";
	config.verbose_stage_filter = "*";
	config.show_compiler_stats = true;

	config.compile_os = false;
	config.override_variables_offset = false;
	config.variables_offset = 0;
	config.override_code_origin = false;
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
	add_type_cast(50,	TypeInt,		TypeString,	"@i2s",	(void*)&CastInt2String);
	add_type_cast(50,	TypeInt64,		TypeString,	"@i642s",	(void*)&CastInt642String);
	add_type_cast(50,	TypeFloat32,		TypeString,	"@f2sf",	(void*)&CastFloat2String);
	add_type_cast(50,	TypeFloat64,	TypeString,	"@f642sf",	(void*)&CastFloat642String);
	add_type_cast(50,	TypeBool,		TypeString,	"@b2s",	(void*)&CastBool2String);
	add_type_cast(50,	TypePointer,	TypeString,	"p2s",	(void*)&CastPointer2String);
	add_type_cast(50,	TypeIntList,	TypeString,	"@ia2s",	nullptr);
	add_type_cast(50,	TypeFloatList,	TypeString,	"@fa2s",	nullptr);
	add_type_cast(50,	TypeBoolList,	TypeString,	"@ba2s",	nullptr);
	add_type_cast(50,	TypeStringList,	TypeString,	"@sa2s",	nullptr);

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
		Array<string> names = name.explode(":");
		string sname = names[0].substr(5, -1).replace("@list", "[]").replace("@@", ".");
		for (Package &p: Packages)
			foreachi(Function *f, p.script->syntax->functions, i)
				if (f->name == sname){
					if (names.num > 0)
						if (f->num_params != names[1]._int())
							continue;
					f->address = pointer;
				}
	}
}

void *GetExternalLink(const string &name)
{
	for (ExternalLinkData &l: ExternalLinks)
		if (l.name == name)
			return l.pointer;
	return nullptr;
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
	VirtualTable *v = *(VirtualTable**)instance;

	ClassOffsetData d;
	d.class_name = class_name;
	d.element = func;
	d.offset = get_virtual_index(p, class_name, func);
	d.is_virtual = true;
	ClassOffsets.add(d);

	LinkExternal(class_name + "." + func, v[d.offset]);
}

int ProcessClassOffset(const string &class_name, const string &element, int offset)
{
	for (ClassOffsetData &d: ClassOffsets)
		if ((d.class_name == class_name) and (d.element == element))
			return d.offset;
	return offset;
}
int ProcessClassSize(const string &class_name, int size)
{
	for (ClassSizeData &d: ClassSizes)
		if (d.class_name == class_name)
			return d.size;
	return size;
}

int ProcessClassNumVirtuals(const string &class_name, int num_virtual)
{
	for (ClassOffsetData &d: ClassOffsets)
		if ((d.class_name == class_name) and (d.is_virtual))
			num_virtual = max(num_virtual, d.offset + 1);
	return num_virtual;
}

void End()
{
	DeleteAllScripts(true, true);

	Packages.clear();

	ResetExternalLinkData();
}


bool CompilerConfiguration::allow_output_func(const Function *f)
{
	if (!verbose)
		return false;
	if (!f)
		return true;
	Array<string> filters = verbose_func_filter.explode(",");
	for (auto &fil: filters)
		if (f->long_name.match(fil))
			return true;
	return false;
}

bool CompilerConfiguration::allow_output_stage(const string &stage)
{
	if (!verbose)
		return false;
	Array<string> filters = verbose_stage_filter.explode(",");
	for (auto &fil: filters)
		if (stage.match(fil))
			return true;
	return false;
}

bool CompilerConfiguration::allow_output(const Function *f, const string &stage)
{
	if (!verbose)
		return false;
	if (!allow_output_func(f))
		return false;
	if (!allow_output_stage(stage))
		return false;
	return true;
}

};
