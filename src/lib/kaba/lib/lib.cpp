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

string LibVersion = "0.17.6.2";


bool call_function(Function *f, void *ff, void *ret, void *inst, const Array<void*> &param);

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
const string IDENTIFIER_LET = "let";
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
const string IDENTIFIER_MAP = "map";
const string IDENTIFIER_LAMBDA = "lambda";
const string IDENTIFIER_SORTED = "sorted";
const string IDENTIFIER_FILTER = "filter";

CompilerConfiguration config;

struct ExternalLinkData {
	string name;
	void *pointer;
};
Array<ExternalLinkData> ExternalLinks;

struct ClassOffsetData {
	string class_name, element;
	int offset;
	bool is_virtual;
};
Array<ClassOffsetData> ClassOffsets;

struct ClassSizeData {
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
const Class *TypeFunctionCode;
const Class *TypeFunctionCodeP;


Array<Script*> Packages;
Script *cur_package = nullptr;


static Function *cur_func = nullptr;
static Class *cur_class;


void add_package(const string &name, bool used_by_default)
{
	Script* s = new Script;
	s->used_by_default = used_by_default;
	s->filename = name;
	Packages.add(s);
	cur_package = s;
}

void __add_class__(Class *t) {
	cur_package->syntax->base_class->classes.add(t);
	t->name_space = cur_package->syntax->base_class;
}

const Class *add_type(const string &name, int size, ScriptFlag flag) {
	Class *t = new Class(name, size, cur_package->syntax);
	if ((flag & FLAG_CALL_BY_VALUE) > 0)
		t->force_call_by_value = true;
	__add_class__(t);
	return t;
}

const Class *add_type_p(const string &name, const Class *sub_type, ScriptFlag flag) {
	Class *t = new Class(name, config.pointer_size, cur_package->syntax);
	t->type = Class::Type::POINTER;
	if ((flag & FLAG_SILENT) > 0)
		t->type = Class::Type::POINTER_SILENT;
	t->parent = sub_type;
	__add_class__(t);
	return t;
}
const Class *add_type_a(const string &name, const Class *sub_type, int array_length) {
	Class *t = new Class(name, 0, cur_package->syntax, sub_type);
	if (array_length < 0) {
		// super array
		t->size = config.super_array_size;
		t->type = Class::Type::SUPER_ARRAY;
		script_make_super_array(t);
	} else {
		// standard array
		t->size = sub_type->size * array_length;
		t->type = Class::Type::ARRAY;
		t->array_length = array_length;
	}
	__add_class__(t);
	return t;
}

const Class *add_type_d(const string &name, const Class *sub_type) {
	Class *t = new Class(name, config.super_array_size, cur_package->syntax, sub_type);
	t->type = Class::Type::DICT;
	script_make_dict(t);
	__add_class__(t);
	return t;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")

PrimitiveOperator PrimitiveOperators[(int)OperatorID::_COUNT_] = {
	{"=",  OperatorID::ASSIGN,        true,  1, IDENTIFIER_FUNC_ASSIGN},
	{"+",  OperatorID::ADD,           false, 11, "__add__"},
	{"-",  OperatorID::SUBTRACT,      false, 11, "__sub__"},
	{"*",  OperatorID::MULTIPLY,      false, 12, "__mul__"},
	{"/",  OperatorID::DIVIDE,        false, 12, "__div__"},
	{"+=", OperatorID::ADDS,          true,  1,  "__iadd__"},
	{"-=", OperatorID::SUBTRACTS,     true,  1,  "__isub__"},
	{"*=", OperatorID::MULTIPLYS,     true,  1,  "__imul__"},
	{"/=", OperatorID::DIVIDES,       true,  1,  "__idiv__"},
	{"==", OperatorID::EQUAL,         false, 8,  "__eq__"},
	{"!=", OperatorID::NOTEQUAL,      false, 8,  "__ne__"},
	{"!",  OperatorID::NEGATE,        false, 2,  "__not__"},
	{"<",  OperatorID::SMALLER,       false, 9,  "__lt__"},
	{">",  OperatorID::GREATER,       false, 9,  "__gt__"},
	{"<=", OperatorID::SMALLER_EQUAL, false, 9,  "__le__"},
	{">=", OperatorID::GREATER_EQUAL, false, 9,  "__ge__"},
	{IDENTIFIER_AND, OperatorID::AND, false, 4,  "__and__"},
	{IDENTIFIER_OR,  OperatorID::OR,  false, 3,  "__or__"},
	{"%",  OperatorID::MODULO,        false, 12, "__mod__"},
	{"&",  OperatorID::BIT_AND,       false, 7, "__bitand__"},
	{"|",  OperatorID::BIT_OR,        false, 5, "__bitor__"},
	{"<<", OperatorID::SHIFT_LEFT,    false, 10, "__lshift__"},
	{">>", OperatorID::SHIFT_RIGHT,   false, 10, "__rshift__"},
	{"++", OperatorID::INCREASE,      true,  2, "__inc__"},
	{"--", OperatorID::DECREASE,      true,  2, "__dec__"},
	{IDENTIFIER_IS, OperatorID::IS,   false, 2,  "-none-"},
	{IDENTIFIER_EXTENDS, OperatorID::EXTENDS, false, 2,  "-none-"}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

//   with type information

void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func = nullptr) {
	Operator *o = new Operator;
	o->owner = cur_package->syntax;
	o->primitive = &PrimitiveOperators[(int)primitive_op];
	o->return_type = return_type;
	o->param_type_1 = param_type1;
	o->param_type_2 = param_type2;
	o->f = add_func(o->primitive->function_name, return_type, func);
	func_set_inline(inline_index);
	func_add_param("a", param_type1);
	func_add_param("b", param_type2);
	cur_package->syntax->operators.add(o);
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//


Class *add_class(const Class *root_type) {
	cur_class = const_cast<Class*>(root_type);
	return cur_class;
}

void class_add_element(const string &name, const Class *type, int offset, ScriptFlag flag) {
	auto e = ClassElement(name, type, offset);
	e.hidden = ((flag & FLAG_HIDDEN) > 0);
	cur_class->elements.add(e);
}

void class_derive_from(const Class *parent, bool increase_size, bool copy_vtable) {
	cur_class->derive_from(parent, increase_size);
	if (copy_vtable)
		cur_class->vtable = parent->vtable;
}

int _class_override_num_params = -1;

void _class_add_member_func(const Class *ccc, Function *f, ScriptFlag flag) {
	Class *c = const_cast<Class*>(ccc);
	if ((flag & FLAG_OVERRIDE) > 0) {
		foreachi(Function *ff, c->member_functions, i)
			if (ff->name == f->name) {
				if (_class_override_num_params < 0 or _class_override_num_params == ff->num_params) {
					//msg_write("OVERRIDE");
					c->member_functions[i] = f;
					return;
				}
			}
		msg_error("could not override " + c->name + "." + f->name);
	} else {
		// name alone is not enough for matching...
		/*foreachi(ClassFunction &ff, c->functions, i)
			if (ff.name == f.name) {
				if (_class_override_num_params < 0 or _class_override_num_params == ff.param_types.num) {
					msg_error("missing override " + c->name + "." + f.name);
					break;
				}
			}*/
		c->member_functions.add(f);
	}
}


Function* class_add_func(const string &name, const Class *return_type, void *func, ScriptFlag flag) {
	Function *f = new Function(name, return_type, cur_class);
	f->is_pure = ((flag & FLAG_PURE) > 0);
	f->throws_exceptions = ((flag & FLAG_RAISES_EXCEPTIONS) > 0);
	f->is_static = ((flag & FLAG_STATIC) > 0);
	cur_package->syntax->functions.add(f);
	f->address_preprocess = func;
	if (config.allow_std_lib)
		f->address = func;
	cur_func = f;


	if (f->is_static)
		cur_class->static_functions.add(f);
	else
		_class_add_member_func(cur_class, f, flag);
	return f;
}

int get_virtual_index(void *func, const string &tname, const string &name) {
	if (config.abi == Abi::WINDOWS_32) {
		if (!func)
			return 0;
		unsigned char *pp = (unsigned char*)func;
		try {
			//if ((cur_class->vtable) and (pp[0] == 0x8b) and (pp[1] == 0x01) and (pp[2] == 0xff) and (pp[3] == 0x60)){
			if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
				// 8b.44.24.**    8b.00     ff.60.10
				// virtual function
				return (int)pp[8] / 4;
			} else if (pp[0] == 0xe9) {
				// jmp
				//msg_write(Asm::Disassemble(func, 16));
				pp = &pp[5] + *(int*)&pp[1];
				//msg_write(Asm::Disassemble(pp, 16));
				if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
					// 8b.44.24.**    8b.00     ff.60.10
					// virtual function
					return (int)pp[8] / 4;
				} else {
					throw(1);
				}
			} else {
				throw(1);
			}
		} catch (...) {
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
			msg_write(string((char*)pp, 4).hex());
			msg_write(Asm::Disassemble(func, 16));
		}
	} else {

		int_p p = (int_p)func;
		if ((p & 1) > 0) {
			// virtual function
			return p / sizeof(void*);
		} else if (!func) {
			return 0;
		} else {
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
		}
	}
	return -1;
}

Function* class_add_func_virtual(const string &name, const Class *return_type, void *func, ScriptFlag flag) {
	string tname = cur_class->name;
	int index = get_virtual_index(func, tname, name);
	//msg_write("virtual: " + tname + "." + name);
		//msg_write(index);
	Function *f = class_add_func(name, return_type, func, flag);
	cur_func->virtual_index = index;
	if (index >= cur_class->vtable.num)
		cur_class->vtable.resize(index + 1);
	cur_class->_vtable_location_compiler_ = cur_class->vtable.data;
	cur_class->_vtable_location_target_ = cur_class->vtable.data;
	return f;
}

void class_link_vtable(void *p) {
	cur_class->link_external_virtual_table(p);
}


//------------------------------------------------------------------------------------------------//
//                                           constants                                            //
//------------------------------------------------------------------------------------------------//

void class_add_const(const string &name, const Class *type, const void *value) {
	Constant *c = cur_package->syntax->add_constant(type, cur_class);
	c->name = name;
	c->address = c->p();

	// config.PointerSize might be smaller than needed for the following assignment
	if ((type == TypeInt) or (type == TypeFloat32) or (type == TypeChar)  or (type == TypeBool) or (type->is_pointer()))
		*(const void**)c->p() = value;
	else
		memcpy(c->p(), value, type->size);
}

void add_const(const string &name, const Class *type, const void *value) {
	cur_class = cur_package->syntax->base_class;
	class_add_const(name, type, value);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//


void add_ext_var(const string &name, const Class *type, void *var) {
	auto *v = new Variable(name, type);
	cur_package->syntax->base_class->static_variables.add(v);
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
void _kaba_array_sort(DynamicArray &array, int offset_by) {
	T *p = (T*)((char*)array.data + offset_by);
	for (int i=0; i<array.num; i++) {
		T *q = (T*)((char*)p + array.element_size);
		for (int j=i+1; j<array.num; j++) {
			if (*p > *q)
				array.simple_swap(i, j);
			q = (T*)((char*)q + array.element_size);
		}
		p = (T*)((char*)p + array.element_size);
	}
}

template<class T>
void _kaba_array_sort_p(DynamicArray &array, int offset_by) {
	char **p = (char**)array.data;
	for (int i=0; i<array.num; i++) {
		T *pp = (T*)(*p + offset_by);
		char **q = p + 1;
		for (int j=i+1; j<array.num; j++) {
			T *qq = (T*)(*q + offset_by);
			if (*pp > *qq){
				array.simple_swap(i, j);
				pp = (T*)(*p + offset_by);
			}
			q ++;
		}
		p ++;
	}
}

void kaba_var_assign(void *pa, const void *pb, const Class *type) {
	//msg_write("assign " + type->long_name());
	if ((type == TypeInt) or (type == TypeFloat32)) {
		*(int*)pa = *(int*)pb;
	} else if ((type == TypeBool) or (type == TypeChar)) {
		*(char*)pa = *(char*)pb;
	} else if (type->is_pointer()) {
		*(void**)pa = *(void**)pb;
	} else {
		auto *f = type->get_assign();
		if (!f)
			kaba_raise_exception(new KabaException("can not assign variables of type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(pa, pb);
	}
}

void kaba_var_init(void *p, const Class *type) {
	//msg_write("init " + type->long_name());
	if (!type->needs_constructor())
		return;
	auto *f = type->get_default_constructor();
	if (!f)
		kaba_raise_exception(new KabaException("can not init a variable of type " + type->long_name()));
	typedef void func_t(void*);
	auto *ff = (func_t*)f->address;
	ff(p);
}

void kaba_array_add(DynamicArray &array, void *p, const Class *type) {
	//msg_write("array add " + type->long_name());
	if ((type == TypeIntList) or (type == TypeFloatList)) {
		array.append_4_single(*(int*)p);
	} else if (type == TypeBoolList) {
		array.append_1_single(*(char*)p);
	} else {
		auto *f = type->get_func("add", TypeVoid, {type->parent});
		if (!f)
			kaba_raise_exception(new KabaException("can not add to array type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(&array, p);
	}
}

DynamicArray _cdecl kaba_array_sort(DynamicArray &array, const Class *type, const string &by) {
	if (!type->is_super_array())
		kaba_raise_exception(new KabaException("type '" + type->name + "' is not an array"));
	const Class *el = type->parent;
	if (array.element_size != el->size)
		kaba_raise_exception(new KabaException("element type size mismatch..."));

	DynamicArray rr;
	kaba_var_init(&rr, type);
	kaba_var_assign(&rr, &array, type);

	const Class *rel = el;

	if (el->is_pointer())
		rel = el->parent;

	int offset = -1;
	const Class *by_type = nullptr;
	if (by == "") {
		offset = 0;
		by_type = rel;
	} else {
		for (auto &e: rel->elements)
			if (e.name == by) {
				by_type = e.type;
				offset = e.offset;
			}
		if (!by_type)
			kaba_raise_exception(new KabaException("type '" + rel->name + "' does not have an element '" + by + "'"));
	}


	if (el->is_pointer()) {
		if (by_type == TypeString)
			_kaba_array_sort_p<string>(rr, offset);
		else if (by_type == TypeInt)
			_kaba_array_sort_p<int>(rr, offset);
		else if (by_type == TypeFloat)
			_kaba_array_sort_p<float>(rr, offset);
		else if (by_type == TypeBool)
			_kaba_array_sort_p<bool>(rr, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	} else {
		if (by_type == TypeString)
			_kaba_array_sort<string>(rr, offset);
		else if (by_type == TypeInt)
			_kaba_array_sort<int>(rr, offset);
		else if (by_type == TypeFloat)
			_kaba_array_sort<float>(rr, offset);
		else if (by_type == TypeBool)
			_kaba_array_sort<bool>(rr, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	}
	return rr;
}

DynamicArray _cdecl kaba_array_filter(Function *f, DynamicArray &array, const Class *type) {
	DynamicArray rr;
	kaba_var_init(&rr, type);

	for (int i=0; i<array.num; i++) {
		bool filter_pass = false;
		void *pa = (char*)array.data + i * array.element_size;
		bool ok = call_function(f, f->address, &filter_pass, nullptr, {pa});
		if (!ok)
			kaba_raise_exception(new KabaException("can not call filter function " + f->signature()));
		if (filter_pass)
			kaba_array_add(rr, pa, type);
	}
	return rr;
}

string _cdecl var2str(const void *var, const Class *type) {
	return type->var2str(var);
}

DynamicArray kaba_map(Function *func, DynamicArray *a) {
	DynamicArray r;
	auto *ti = func->literal_param_type[0];
	auto *to = func->literal_return_type;
	r.init(to->size);
	if (to->needs_constructor()) {
		if (to == TypeString) {
			((string*)&r)->resize(a->num);
		} else  {
			kaba_raise_exception(new KabaException("map(): output type not allowed: " + to->long_name()));
		}
	} else {
		r.simple_resize(a->num);
	}
	for (int i=0; i<a->num; i++) {
		void *po = (char*)r.data + to->size * i;
		void *pi = (char*)a->data + ti->size * i;
		bool ok = call_function(func, func->address, po, nullptr, {pi});
		if (!ok)
			kaba_raise_exception(new KabaException("map(): failed to dynamically call " + func->signature()));
	}
	return r;
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


Array<Statement*> Statements;

Function *add_func(const string &name, const Class *return_type, void *func, ScriptFlag flag) {
	add_class(cur_package->base_class());
	return class_add_func(name, return_type, func, flag);
}

Statement *statement_from_id(StatementID id) {
	for (auto *s: Statements)
		if (s->id == id)
			return s;
	return nullptr;
}

int add_statement(const string &name, StatementID id, int num_params = 0) {
	Statement *s = new Statement;
	s->name = name;
	s->id = id;
	s->num_params = num_params;
	Statements.add(s);
	return 0;
}

void func_set_inline(InlineID index) {
	if (cur_func)
		cur_func->inline_no = index;
}

void func_add_param(const string &name, const Class *type) {
	if (cur_func) {
		Variable *v = new Variable(name, type);
		cur_func->var.add(v);
		cur_func->literal_param_type.add(type);
		cur_func->num_params ++;
	}
}

void script_make_super_array(Class *t, SyntaxTree *ps)
{
	const Class *parent = t->parent;
	t->derive_from(TypeDynamicArray, false);
	t->parent = parent;
	add_class(t);

	Function *sub = t->get_func(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, {nullptr,nullptr});
	sub->literal_return_type = t;
	sub->return_type = t;

	// FIXME  wrong for complicated classes
	if (t->parent->is_simple_class()){
		if (!t->parent->uses_call_by_reference()){
			if (t->parent->is_pointer()){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<void*>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_p_single);
					func_add_param("x", t->parent);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_p_single);
					func_add_param("x", t->parent);
					func_add_param("index", TypeInt);
			}else if (t->parent == TypeFloat32){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<float>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_f_single);
					func_add_param("x", t->parent);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_f_single);
					func_add_param("x", t->parent);
					func_add_param("index", TypeInt);
			}else if (t->parent == TypeFloat64){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<double>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_d_single);
					func_add_param("x", t->parent);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_d_single);
					func_add_param("x", t->parent);
					func_add_param("index", TypeInt);
			}else if (t->parent->size == 4){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<int>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_4_single);
					func_add_param("x", t->parent);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_4_single);
					func_add_param("x", t->parent);
					func_add_param("index", TypeInt);
			}else if (t->parent->size == 1){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<char>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_1_single);
					func_add_param("x", t->parent);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_1_single);
					func_add_param("x", t->parent);
					func_add_param("index", TypeInt);
			}else{
				msg_error("evil class:  " + t->name);
			}
		}else{
			// __init__ must be defined manually...!
			class_add_funcx("add", TypeVoid, &DynamicArray::append_single);
				func_add_param("x", t->parent);
			class_add_funcx("insert", TypeVoid, &DynamicArray::insert_single);
				func_add_param("x", t->parent);
				func_add_param("index", TypeInt);
		}
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &DynamicArray::simple_clear);
		class_add_funcx("clear", TypeVoid, &DynamicArray::simple_clear);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &DynamicArray::simple_assign);
			func_add_param("other", t);
		class_add_funcx("remove", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);
		class_add_funcx("resize", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("num", TypeInt);
	}else if (t->parent == TypeString){
		// handled manually later...
	}else{
		msg_error("evil class:  " + t->name);
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
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Map<string,int>::clear);
		class_add_funcx("clear", TypeVoid, &Map<string,int>::clear);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &IntDict::assign);
			func_add_param("other", t);
	}

	if (t->parent == TypeInt){
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,int>::__init__);
		class_add_funcx("set", TypeVoid, &IntDict::set_int);
			func_add_param("key", TypeString);
			func_add_param("x", t->parent);
		class_add_funcx("__get__", t->parent, &IntDict::get_int, FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_funcx("str", TypeString, &IntDict::str);
	}else if (t->parent == TypeFloat){
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,float>::__init__);
		class_add_funcx("set", TypeVoid, &FloatDict::set_float);
			func_add_param("key", TypeString);
			func_add_param("x", t->parent);
		class_add_funcx("__get__", t->parent, &FloatDict::get_float, FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_funcx("str", TypeString, &FloatDict::str);
	}else if (t->parent == TypeString){
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Map<string,string>::__init__);
		class_add_funcx("set", TypeVoid, &Map<string,string>::set);
			func_add_param("key", TypeString);
			func_add_param("x", t->parent);
		class_add_funcx("__get__", t->parent, &StringDict::get_string, FLAG_RAISES_EXCEPTIONS);
			func_add_param("key", TypeString);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &Map<string,string>::clear);
		class_add_funcx("clear", TypeVoid, &Map<string,string>::clear);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &StringDict::assign);
			func_add_param("other", t);
		class_add_funcx("str", TypeString, &StringDict::str);
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
void CastInt642Int(Value &r, Value &s)
{
	r.init(TypeInt);
	r.as_int() = s.as_int();
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
	for (auto *f: cur_package->syntax->base_class->static_functions)
		if (f->long_name() == cmd){
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

class PointerClass {
	void *p;
public:
	string _cdecl str(){	return p2s(p);	}
};



int xop_int_add(int a, int b) {
	return a + b;
}



void SIAddPackageBase() {
	add_package("base", true);

	// internal
	TypeUnknown			= add_type  ("-unknown-", 0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type  ("-reg128-", 16, FLAG_CALL_BY_VALUE);
	TypeReg64			= add_type  ("-reg64-", 8, FLAG_CALL_BY_VALUE);
	TypeReg32			= add_type  ("-reg32-", 4, FLAG_CALL_BY_VALUE);
	TypeReg16			= add_type  ("-reg16-", 2, FLAG_CALL_BY_VALUE);
	TypeReg8			= add_type  ("-reg8-", 1, FLAG_CALL_BY_VALUE);
	TypeChunk			= add_type  ("-chunk-", 0); // substitute for all plane-old-data types

	// "real"
	TypeVoid			= add_type  ("void", 0, FLAG_CALL_BY_VALUE);
	TypeBool			= add_type  ("bool", sizeof(bool), FLAG_CALL_BY_VALUE);
	TypeInt				= add_type  ("int", sizeof(int), FLAG_CALL_BY_VALUE);
	TypeInt64			= add_type  ("int64", sizeof(int64), FLAG_CALL_BY_VALUE);
	TypeFloat32			= add_type  ("float32", sizeof(float), FLAG_CALL_BY_VALUE);
	TypeFloat64			= add_type  ("float64", sizeof(double), FLAG_CALL_BY_VALUE);
	TypeChar			= add_type  ("char", sizeof(char), FLAG_CALL_BY_VALUE);
	TypeDynamicArray	= add_type  ("@DynamicArray", config.super_array_size);
	TypeDictBase		= add_type  ("@DictBase",   config.super_array_size);

	TypeException		= add_type  ("Exception", sizeof(KabaException));
	TypeExceptionP		= add_type_p("Exception*", TypeException);


	// select default float type
	TypeFloat = TypeFloat32;
	(const_cast<Class*>(TypeFloat))->name = "float";



	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt, config.pointer_size);
		class_add_funcx("swap", TypeVoid, &DynamicArray::simple_swap);
			func_add_param("i1", TypeInt);
			func_add_param("i2", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, &DynamicArray::ref_subarray);
			func_add_param("start", TypeInt);
			func_add_param("end", TypeInt);
		// low level operations
		class_add_funcx("__mem_init__", TypeVoid, &DynamicArray::init);
			func_add_param("element_size", TypeInt);
		class_add_funcx("__mem_clear__", TypeVoid, &DynamicArray::simple_clear);
		class_add_funcx("__mem_resize__", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("size", TypeInt);
		class_add_funcx("__mem_remove__", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt, config.pointer_size);
		// low level operations
		class_add_funcx("__mem_init__", TypeVoid, &DynamicArray::init);
			func_add_param("element_size", TypeInt);
		class_add_funcx("__mem_clear__", TypeVoid, &DynamicArray::simple_clear);
		class_add_funcx("__mem_resize__", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("size", TypeInt);
		class_add_funcx("__mem_remove__", TypeVoid, &DynamicArray::delete_single);
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
	


	//	add_func_special("f2i", TypeInt, (void*)&_Float2Int);
	add_funcx("f2i", TypeInt, &_Float2Int, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::FLOAT_TO_INT);    // sometimes causes floating point exceptions...
		func_add_param("f", TypeFloat32);
	add_funcx("i2f", TypeFloat32, &_Int2Float, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::INT_TO_FLOAT);
		func_add_param("i", TypeInt);
	add_funcx("f2f64", TypeFloat64, &_Float2Float64, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::FLOAT_TO_FLOAT64);
		func_add_param("f", TypeFloat32);
	add_funcx("f642f", TypeFloat32, &_Float642Float, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::FLOAT64_TO_FLOAT);
		func_add_param("f", TypeFloat64);
	add_funcx("i2i64", TypeInt64, &_Int2Int64, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::INT_TO_INT64);
		func_add_param("i", TypeInt);
	add_funcx("i642i", TypeInt, &_Int642Int, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::INT64_TO_INT);
		func_add_param("i", TypeInt64);
	add_funcx("i2c", TypeChar, &_Int2Char, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::INT_TO_CHAR);
		func_add_param("i", TypeInt);
	add_funcx("c2i", TypeInt, &_Char2Int, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::CHAR_TO_INT);
		func_add_param("c", TypeChar);
	add_funcx("p2b", TypeBool, &_Pointer2Bool, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::POINTER_TO_BOOL);
		func_add_param("p", TypePointer);


	add_class(TypePointer);
		class_add_funcx("str", TypeString, &PointerClass::str, FLAG_PURE);


	add_class(TypeInt);
		class_add_funcx("str", TypeString, &IntClass::str, FLAG_PURE);
		class_add_funcx("add", TypeInt, &xop_int_add, FLAG_PURE);
			func_set_inline(InlineID::INT_ADD);
			func_add_param("b", TypeInt);


	add_class(TypeInt64);
		class_add_funcx("str", TypeString, &Int64Class::str, FLAG_PURE);


	add_class(TypeFloat32);
		class_add_funcx("str", TypeString, &FloatClass::str, FLAG_PURE);
		class_add_funcx("str2", TypeString, &FloatClass::str2, FLAG_PURE);
			func_add_param("decimals", TypeInt);


	add_class(TypeFloat64);
		class_add_funcx("str", TypeString, &Float64Class::str, FLAG_PURE);
		class_add_funcx("str2", TypeString, &Float64Class::str2, FLAG_PURE);
			func_add_param("decimals", TypeInt);


	add_class(TypeBool);
		class_add_funcx("str", TypeString, &BoolClass::str, FLAG_PURE);


	add_class(TypeChar);
		class_add_funcx("str", TypeString, &CharClass::str, FLAG_PURE);


	add_class(TypeString);
		class_add_funcx("__iadd__", TypeVoid, &string::operator+=);
			func_add_param("x", TypeString);
		class_add_funcx("__add__", TypeString, &string::operator+, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__eq__", TypeBool, &string::operator==, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__ne__", TypeBool, &string::operator!=, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__lt__", TypeBool, &string::operator<, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__gt__", TypeBool, &string::operator>, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__le__", TypeBool, &string::operator<=, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("__ge__", TypeBool, &string::operator>=, FLAG_PURE);
			func_add_param("x", TypeString);
		class_add_funcx("substr", TypeString, &string::substr, FLAG_PURE);
			func_add_param("start", TypeInt);
			func_add_param("length", TypeInt);
		class_add_funcx("head", TypeString, &string::head, FLAG_PURE);
			func_add_param("size", TypeInt);
		class_add_funcx("tail", TypeString, &string::tail, FLAG_PURE);
			func_add_param("size", TypeInt);
		class_add_funcx("find", TypeInt, &string::find, FLAG_PURE);
			func_add_param("str", TypeString);
			func_add_param("start", TypeInt);
		class_add_funcx("compare", TypeInt, &string::compare, FLAG_PURE);
			func_add_param("str", TypeString);
		class_add_funcx("icompare", TypeInt, &string::icompare, FLAG_PURE);
			func_add_param("str", TypeString);
		class_add_funcx("replace", TypeString, &string::replace, FLAG_PURE);
			func_add_param("sub", TypeString);
			func_add_param("by", TypeString);
		class_add_funcx("explode", TypeStringList, &string::explode, FLAG_PURE);
			func_add_param("str", TypeString);
		class_add_funcx("lower", TypeString, &string::lower, FLAG_PURE);
		class_add_funcx("upper", TypeString, &string::upper, FLAG_PURE);
		class_add_funcx("reverse", TypeString, &string::reverse, FLAG_PURE);
		class_add_funcx("hash", TypeInt, &string::hash, FLAG_PURE);
		class_add_funcx("md5", TypeString, &string::md5, FLAG_PURE);
		class_add_funcx("hex", TypeString, &string::hex, FLAG_PURE);
			func_add_param("inverted", TypeBool);
		class_add_funcx("unhex", TypeString, &string::unhex, FLAG_PURE);
		class_add_funcx("match", TypeBool, &string::match, FLAG_PURE);
			func_add_param("glob", TypeString);
		class_add_funcx("int", TypeInt, &string::_int, FLAG_PURE);
		class_add_funcx("int64", TypeInt64, &string::i64, FLAG_PURE);
		class_add_funcx("float", TypeFloat32, &string::_float, FLAG_PURE);
		class_add_funcx("float64", TypeFloat64, &string::f64, FLAG_PURE);
		class_add_funcx("trim", TypeString, &string::trim, FLAG_PURE);
		class_add_funcx("dirname", TypeString, &string::dirname, FLAG_PURE);
		class_add_funcx("basename", TypeString, &string::basename, FLAG_PURE);
		class_add_funcx("extension", TypeString, &string::extension, FLAG_PURE);

		class_add_funcx("escape", TypeString, &str_escape, FLAG_PURE);
		class_add_funcx("unescape", TypeString, &str_unescape, FLAG_PURE);


	add_class(TypeStringList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &StringList::__init__);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &StringList::clear);
		class_add_funcx("add", TypeVoid, &StringList::add);
			func_add_param("x", TypeString);
		class_add_funcx("clear", TypeVoid, &StringList::clear);
		class_add_funcx("remove", TypeVoid, &StringList::erase);
			func_add_param("index", TypeInt);
		class_add_funcx("resize", TypeVoid, &StringList::resize);
			func_add_param("num", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &StringList::assign);
			func_add_param("other", TypeStringList);
		class_add_funcx("join", TypeString, &StringList::join, FLAG_PURE);
			func_add_param("glue", TypeString);


	// constants
	add_const("nil", TypePointer, nullptr);
	add_const("false", TypeBool, (void*)false);
	add_const("true",  TypeBool, (void*)true);


	add_funcx("int_add", TypeInt, &xop_int_add, ScriptFlag(FLAG_PURE | FLAG_STATIC));
		func_set_inline(InlineID::INT_ADD);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);

	add_class(TypeException);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaException::__init__);
			func_add_param("message", TypeString);
		class_add_func_virtualx(IDENTIFIER_FUNC_DELETE, TypeVoid, &KabaException::__delete__);
		class_add_func_virtualx("message", TypeString, &KabaException::message);
		class_add_element("text", TypeString, config.pointer_size);
		class_set_vtable(KabaException);

	add_funcx(IDENTIFIER_RAISE, TypeVoid, &kaba_raise_exception, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
		func_add_param("e", TypeExceptionP);
}



#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

Script *__load_script__(const string &filename, bool just_analyse) {
	KABA_EXCEPTION_WRAPPER( return Load(filename, just_analyse); );
	return nullptr;
}

Script *__create_from_source__(const string &source, bool just_analyse) {
	KABA_EXCEPTION_WRAPPER( return CreateForSource(source, just_analyse); );
	return nullptr;
}

void __execute_single_command__(const string &cmd) {
	KABA_EXCEPTION_WRAPPER( ExecuteSingleScriptCommand(cmd); );
}

#pragma GCC pop_options

void SIAddPackageKaba() {
	add_package("kaba", false);


	TypeClass 			= add_type  ("Class", sizeof(Class));
	TypeClassP			= add_type_p("Class*", TypeClass);
	auto *TypeClassPList = add_type_a("Class*[]", TypeClassP, -1);

	TypeFunction		= add_type  ("Function", sizeof(Function));
	TypeFunctionP		= add_type_p("Function*", TypeFunction);
	auto *TypeFunctionPList = add_type_a("Function*[]", TypeFunctionP, -1);
	TypeFunctionCode	= add_type  ("func", 32); // whatever
	TypeFunctionCodeP	= add_type_p("func*", TypeFunctionCode);
	auto *TypeStatement = add_type  ("Statement", sizeof(Statement));
	auto *TypeStatementP= add_type_p("Statement*", TypeStatement);
	auto *TypeStatementPList = add_type_a("Statement*[]", TypeStatementP, -1);
		

	auto *TypeScript = add_type  ("Script", sizeof(Script));
	auto *TypeScriptP = add_type_p("Script*", TypeScript);
	auto *TypeScriptPList = add_type_a("Script*[]", TypeScriptP, -1);

	
	auto *TypeClassElement = add_type("ClassElement", sizeof(ClassElement));
	auto *TypeClassElementList = add_type_a("ClassElement[]", TypeClassElement, -1);
	auto *TypeVariable = add_type("Variable", sizeof(Variable));
	auto *TypeVariableP = add_type_p("Variable*", TypeVariable);
	auto *TypeVariablePList = add_type_a("Variable*[]", TypeVariableP, -1);
	auto *TypeConstant = add_type("Constant", sizeof(Constant));
	auto *TypeConstantP = add_type_p("Constant*", TypeConstant);
	auto *TypeConstantPList = add_type_a("Constant*[]", TypeConstantP, -1);
	
	
	add_class(TypeClassElement);
		class_add_elementx("name", TypeString, &ClassElement::name);
		class_add_elementx("type", TypeClassP, &ClassElement::type);
		class_add_elementx("hidden", TypeBool, &ClassElement::hidden);
		class_add_elementx("offset", TypeInt, &ClassElement::offset);


	add_class(TypeClass);
		class_add_elementx("name", TypeString, &Class::name);
		class_add_elementx("size", TypeInt, &Class::size);
		class_add_elementx("parent", TypeClassP, &Class::parent);
		class_add_elementx("namespace", TypeClassP, &Class::name_space);
		class_add_elementx("elements", TypeClassElementList, &Class::elements);
		class_add_elementx("functions", TypeFunctionPList, &Class::member_functions);
		class_add_elementx("static_functions", TypeFunctionPList, &Class::static_functions);
		class_add_elementx("classes", TypeClassPList, &Class::classes);
		class_add_elementx("constants", TypeConstantPList, &Class::constants);
		class_add_funcx("is_derived_from", TypeBool, &Class::is_derived_from);
			func_add_param("c", TypeClassP);
		class_add_funcx("long_name", TypeString, &Class::long_name);

	add_class(TypeFunction);
		class_add_elementx("name", TypeString, &Function::name);
		class_add_funcx("long_name", TypeString, &Function::long_name);
		class_add_funcx("signature", TypeString, &Function::signature);
		class_add_elementx("namespace", TypeClassP, &Function::name_space);
		class_add_elementx("num_params", TypeInt, &Function::num_params);
		class_add_elementx("var", TypeVariablePList, &Function::var);
		class_add_elementx("param_type", TypeClassPList, &Function::literal_param_type);
		class_add_elementx("return_type", TypeClassP, &Function::literal_return_type);
		class_add_elementx("is_static", TypeBool, &Function::is_static);
		class_add_elementx("is_pure", TypeBool, &Function::is_pure);
		class_add_elementx("is_extern", TypeBool, &Function::is_extern);
		class_add_elementx("needs_overriding", TypeBool, &Function::needs_overriding);
		class_add_elementx("virtual_index", TypeInt, &Function::virtual_index);
		class_add_elementx("inline_index", TypeInt, &Function::inline_no);
		class_add_elementx("code", TypeFunctionCodeP, &Function::address);


	add_class(TypeVariable);
		class_add_elementx("name", TypeString, &Variable::name);
		class_add_elementx("type", TypeClassP, &Variable::type);
		
	add_class(TypeConstant);
		class_add_elementx("name", TypeString, &Constant::name);
		class_add_elementx("type", TypeClassP, &Constant::type);

	add_class(TypeScript);
		class_add_elementx("name", TypeString, &Script::filename);
		class_add_elementx("used_by_default", TypeBool, &Script::used_by_default);
		class_add_funcx("classes", TypeClassPList, &Script::classes);
		class_add_funcx("functions", TypeFunctionPList, &Script::functions);
		class_add_funcx("variables", TypeVariablePList, &Script::variables);
		class_add_funcx("constants", TypeConstantPList, &Script::constants);
		class_add_funcx("base_class", TypeClassP, &Script::base_class);
		class_add_funcx("load", TypeScriptP, &__load_script__, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("filename", TypeString);
			func_add_param("just_analize", TypeBool);
		class_add_funcx("create", TypeScriptP, &__create_from_source__, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("source", TypeString);
			func_add_param("just_analize", TypeBool);
		class_add_funcx("delete", TypeVoid, &Remove, FLAG_STATIC);
			func_add_param("script", TypeScriptP);
		class_add_funcx("execute_single_command", TypeVoid, &__execute_single_command__, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
			func_add_param("cmd", TypeString);
	
	add_class(TypeStatement);
		class_add_elementx("name", TypeString, &Statement::name);
		class_add_elementx("id", TypeInt, &Statement::id);
		class_add_elementx("num_params", TypeInt, &Statement::num_params);
		
	add_class(TypeClassElementList);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<ClassElement>::__init__);

	add_funcx("get_dynamic_type", TypeClassP, &GetDynamicType, FLAG_STATIC);
		func_add_param("p", TypePointer);

	add_ext_var("packages", TypeScriptPList, (void*)&Packages);
	add_ext_var("statements", TypeStatementPList, (void*)&Statements);
}


void SIAddBasicCommands() {
	// statements
	add_statement(IDENTIFIER_RETURN, StatementID::RETURN); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_IF, StatementID::IF, 2); // [CMP, BLOCK]
	add_statement("-if/else-", StatementID::IF_ELSE, 3); // [CMP, BLOCK, ELSE-BLOCK]
	add_statement(IDENTIFIER_WHILE, StatementID::WHILE, 2); // [CMP, BLOCK]
	add_statement("-for-array-", StatementID::FOR_ARRAY, 4); // [VAR, INDEX, ARRAY, BLOCK]
	add_statement("-for-range-", StatementID::FOR_RANGE, 5); // [VAR, START, STOP, STEP, BLOCK]
	add_statement(IDENTIFIER_FOR, StatementID::FOR_DIGEST, 4); // [INIT, CMP, BLOCK, INC] internally like a while-loop... but a bit different...
	add_statement(IDENTIFIER_BREAK, StatementID::BREAK);
	add_statement(IDENTIFIER_CONTINUE, StatementID::CONTINUE);
	add_statement(IDENTIFIER_NEW, StatementID::NEW);
	add_statement(IDENTIFIER_DELETE, StatementID::DELETE, 1);
	add_statement(IDENTIFIER_SIZEOF, StatementID::SIZEOF, 1);
	add_statement(IDENTIFIER_TYPE, StatementID::TYPE, 1);
	add_statement(IDENTIFIER_STR, StatementID::STR, 1);
	add_statement(IDENTIFIER_LEN, StatementID::LEN, 1);
	add_statement(IDENTIFIER_LET, StatementID::LET);
	add_statement(IDENTIFIER_ASM, StatementID::ASM);
	//add_statement(IDENTIFIER_RAISE, StatementID::RAISE); NOPE, now it's a function!
	add_statement(IDENTIFIER_TRY, StatementID::TRY); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_EXCEPT, StatementID::EXCEPT); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_PASS, StatementID::PASS);
	add_statement(IDENTIFIER_MAP, StatementID::MAP);
	add_statement(IDENTIFIER_LAMBDA, StatementID::LAMBDA);
	add_statement(IDENTIFIER_SORTED, StatementID::SORTED);
	add_statement(IDENTIFIER_FILTER, StatementID::FILTER);
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

void SIAddOperators() {
	add_operator(OperatorID::ASSIGN, TypeVoid, TypePointer, TypePointer, InlineID::POINTER_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypePointer, TypePointer, InlineID::POINTER_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_NOT_EQUAL);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeChar, TypeChar, InlineID::CHAR_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeChar, TypeChar, InlineID::CHAR_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_NOT_EQUAL);
	add_operator(OperatorID::GREATER, TypeBool, TypeChar, TypeChar, InlineID::CHAR_GREATER);
	add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_GREATER_EQUAL);
	add_operator(OperatorID::SMALLER, TypeBool, TypeChar, TypeChar, InlineID::CHAR_SMALLER);
	add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeChar, TypeChar, InlineID::CHAR_SMALLER_EQUAL);
	add_operator(OperatorID::ADD, 	TypeChar, TypeChar, TypeChar, InlineID::CHAR_ADD);
	add_operator(OperatorID::SUBTRACTS, TypeChar, TypeChar, TypeChar, InlineID::CHAR_SUBTRACT_ASSIGN);
	add_operator(OperatorID::ADDS, 	TypeChar, TypeChar, TypeChar, InlineID::CHAR_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACT, TypeChar, TypeChar, TypeChar, InlineID::CHAR_SUBTRACT);
	add_operator(OperatorID::BIT_AND, TypeChar, TypeChar, TypeChar, InlineID::CHAR_AND);
	add_operator(OperatorID::BIT_OR, 	TypeChar, TypeChar, TypeChar, InlineID::CHAR_OR);
	add_operator(OperatorID::SUBTRACT, TypeChar, TypeVoid, TypeChar, InlineID::CHAR_NEGATE);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeBool, TypeBool, InlineID::BOOL_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeBool, TypeBool, InlineID::BOOL_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_NOT_EQUAL);
	add_operator(OperatorID::AND, 	TypeBool, TypeBool, TypeBool, InlineID::BOOL_AND);
	add_operator(OperatorID::OR, 	TypeBool, TypeBool, TypeBool, InlineID::BOOL_OR);
	add_operator(OperatorID::NEGATE, TypeBool, TypeVoid, TypeBool, InlineID::BOOL_NEGATE);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt, TypeInt, InlineID::INT_ASSIGN);
	add_operator(OperatorID::ADD, 	TypeInt, TypeInt, TypeInt, InlineID::INT_ADD, (void*)op_int_add);
	add_operator(OperatorID::SUBTRACT, TypeInt, TypeInt, TypeInt, InlineID::INT_SUBTRACT, (void*)op_int_sub);
	add_operator(OperatorID::MULTIPLY, TypeInt, TypeInt, TypeInt, InlineID::INT_MULTIPLY, (void*)op_int_mul);
	add_operator(OperatorID::DIVIDE, TypeInt, TypeInt, TypeInt, InlineID::INT_DIVIDE, (void*)op_int_div);
	add_operator(OperatorID::ADDS, 	TypeVoid, TypeInt, TypeInt, InlineID::INT_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt, TypeInt, InlineID::INT_SUBTRACT_ASSIGN);
	add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt, TypeInt, InlineID::INT_MULTIPLY_ASSIGN);
	add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt, TypeInt, InlineID::INT_DIVIDE_ASSIGN);
	add_operator(OperatorID::MODULO, TypeInt, TypeInt, TypeInt, InlineID::INT_MODULO, (void*)op_int_mod);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeInt, TypeInt, InlineID::INT_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_NOT_EQUAL);
	add_operator(OperatorID::GREATER, TypeBool, TypeInt, TypeInt, InlineID::INT_GREATER);
	add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_GREATER_EQUAL);
	add_operator(OperatorID::SMALLER, TypeBool, TypeInt, TypeInt, InlineID::INT_SMALLER);
	add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT_SMALLER_EQUAL);
	add_operator(OperatorID::BIT_AND, TypeInt, TypeInt, TypeInt, InlineID::INT_AND);
	add_operator(OperatorID::BIT_OR, 	TypeInt, TypeInt, TypeInt, InlineID::INT_OR);
	add_operator(OperatorID::SHIFT_RIGHT, TypeInt, TypeInt, TypeInt, InlineID::INT_SHIFT_RIGHT, (void*)op_int_shr);
	add_operator(OperatorID::SHIFT_LEFT, TypeInt, TypeInt, TypeInt, InlineID::INT_SHIFT_LEFT, (void*)op_int_shl);
	add_operator(OperatorID::SUBTRACT, TypeInt, TypeVoid, TypeInt, InlineID::INT_NEGATE);
	add_operator(OperatorID::INCREASE, TypeVoid, TypeInt, TypeVoid, InlineID::INT_INCREASE);
	add_operator(OperatorID::DECREASE, TypeVoid, TypeInt, TypeVoid, InlineID::INT_DECREASE);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ASSIGN);
	add_operator(OperatorID::ADD, 	TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_ADD, (void*)op_int64_add);
	add_operator(OperatorID::ADD, 	TypeInt64, TypeInt64, TypeInt, InlineID::INT64_ADD_INT, (void*)op_int64_add_int);
	add_operator(OperatorID::SUBTRACT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT, (void*)op_int64_sub);
	add_operator(OperatorID::MULTIPLY, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY, (void*)op_int64_mul);
	add_operator(OperatorID::DIVIDE, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE, (void*)op_int64_div);
	add_operator(OperatorID::ADDS, 	TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT_ASSIGN);
	add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY_ASSIGN);
	add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE_ASSIGN);
	add_operator(OperatorID::MODULO, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MODULO, (void*)op_int64_mod);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeInt64, TypeInt64, InlineID::INT64_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_NOT_EQUAL);
	add_operator(OperatorID::GREATER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER);
	add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER_EQUAL);
	add_operator(OperatorID::SMALLER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER);
	add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER_EQUAL);
	add_operator(OperatorID::BIT_AND, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_AND);
	add_operator(OperatorID::BIT_OR, 	TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_OR);
	add_operator(OperatorID::SHIFT_RIGHT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_RIGHT, (void*)op_int64_shr);
	add_operator(OperatorID::SHIFT_LEFT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_LEFT, (void*)op_int64_shl);
	add_operator(OperatorID::SUBTRACT, TypeInt64, TypeVoid, TypeInt64, InlineID::INT64_NEGATE);
	add_operator(OperatorID::INCREASE, TypeVoid, TypeInt64, TypeVoid, InlineID::INT64_INCREASE);
	add_operator(OperatorID::DECREASE, TypeVoid, TypeInt64, TypeVoid, InlineID::INT64_DECREASE);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_ASSIGN);
	add_operator(OperatorID::ADD, 	TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_ADD, (void*)op_float_add);
	add_operator(OperatorID::SUBTRACT, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_SUBTARCT, (void*)op_float_sub);
	add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_MULTIPLY, (void*)op_float_mul);
	add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeFloat32, TypeInt, InlineID::FLOAT_MULTIPLY_FI);
	add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeInt, TypeFloat32, InlineID::FLOAT_MULTIPLY_IF);
	add_operator(OperatorID::DIVIDE, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT_DIVIDE, (void*)op_float_div);
	add_operator(OperatorID::ADDS, 	TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_SUBTRACT_ASSIGN);
	add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_MULTIPLY_ASSIGN);
	add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT_DIVIDE_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_NOT_EQUAL);
	add_operator(OperatorID::GREATER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_GREATER);
	add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_GREATER_EQUAL);
	add_operator(OperatorID::SMALLER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_SMALLER);
	add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT_SMALLER_EQUAL);
	add_operator(OperatorID::SUBTRACT, TypeFloat32, TypeVoid, TypeFloat32, InlineID::FLOAT_NEGATE);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ASSIGN);
	add_operator(OperatorID::ADD, 	TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD, (void*)op_float64_add);
	add_operator(OperatorID::SUBTRACT, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT, (void*)op_float64_sub);
	add_operator(OperatorID::MULTIPLY, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY, (void*)op_float64_mul);
	add_operator(OperatorID::MULTIPLY, TypeFloat64, TypeFloat64, TypeInt, InlineID::FLOAT64_MULTIPLY_FI);
	add_operator(OperatorID::MULTIPLY, TypeFloat64, TypeInt, TypeFloat64, InlineID::FLOAT64_MULTIPLY_IF);
	add_operator(OperatorID::DIVIDE, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE, (void*)op_float64_div);
	add_operator(OperatorID::ADDS, 	TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT_ASSIGN);
	add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY_ASSIGN);
	add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_NOT_EQUAL);
	add_operator(OperatorID::GREATER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER);
	add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER_EQUAL);
	add_operator(OperatorID::SMALLER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER);
	add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER_EQUAL);
	add_operator(OperatorID::SUBTRACT, TypeFloat32, TypeVoid, TypeFloat64, InlineID::FLOAT64_NEGATE);
//	add_operator(OperatorID::ASSIGN, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_ASSIGN);
	add_operator(OperatorID::ADD, 	TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_ADD);
	add_operator(OperatorID::SUBTRACT, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_SUBTRACT);
	add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY, (void*)op_complex_mul);
	add_operator(OperatorID::MULTIPLY, TypeComplex, TypeFloat32, TypeComplex, InlineID::COMPLEX_MULTIPLY_FC);
	add_operator(OperatorID::MULTIPLY, TypeComplex, TypeComplex, TypeFloat32, InlineID::COMPLEX_MULTIPLY_CF);
	add_operator(OperatorID::DIVIDE, TypeComplex, TypeComplex, TypeComplex, InlineID::NONE /*InlineID::COMPLEX_DIVIDE*/, (void*)op_complex_div);
	add_operator(OperatorID::ADDS, 	TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_SUBTARCT_ASSIGN);
	add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_MULTIPLY_ASSIGN);
	add_operator(OperatorID::DIVIDES, TypeVoid, TypeComplex, TypeComplex, InlineID::COMPLEX_DIVIDE_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeComplex, TypeComplex, InlineID::COMPLEX_EQUAL);
	add_operator(OperatorID::SUBTRACT, TypeComplex, TypeVoid, TypeComplex, InlineID::COMPLEX_NEGATE);
	add_operator(OperatorID::ASSIGN, TypeVoid, TypeChunk, TypeChunk, InlineID::CHUNK_ASSIGN);
	add_operator(OperatorID::EQUAL, 	TypeBool, TypeChunk, TypeChunk, InlineID::CHUNK_EQUAL);
	add_operator(OperatorID::NOTEQUAL, TypeBool, TypeChunk, TypeChunk, InlineID::CHUNK_NOT_EQUAL);
	add_operator(OperatorID::ADD, 	TypeVector, TypeVector, TypeVector, InlineID::VECTOR_ADD);
	add_operator(OperatorID::SUBTRACT, TypeVector, TypeVector, TypeVector, InlineID::VECTOR_SUBTRACT);
	add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeVector, TypeVector, InlineID::VECTOR_MULTIPLY_VV);
	add_operator(OperatorID::MULTIPLY, TypeVector, TypeVector, TypeFloat32, InlineID::VECTOR_MULTIPLY_VF);
	add_operator(OperatorID::MULTIPLY, TypeVector, TypeFloat32, TypeVector, InlineID::VECTOR_MULTIPLY_FV);
	add_operator(OperatorID::DIVIDE, TypeVector, TypeVector, TypeFloat32, InlineID::VECTOR_DIVIDE_VF);
	add_operator(OperatorID::ADDS, 	TypeVoid, TypeVector, TypeVector, InlineID::VECTOR_ADD_ASSIGN);
	add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeVector, TypeVector, InlineID::VECTOR_SUBTARCT_ASSIGN);
	add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeVector, TypeFloat32, InlineID::VECTOR_MULTIPLY_ASSIGN);
	add_operator(OperatorID::DIVIDES, TypeVoid, TypeVector, TypeFloat32, InlineID::VECTOR_DIVIDE_ASSIGN);
	add_operator(OperatorID::SUBTRACT, TypeVector, TypeVoid, TypeVector, InlineID::VECTOR_NEGATE);
}

void SIAddCommands() {
	// type casting
	add_func("@s2i", TypeInt, (void*)&s2i, FLAG_STATIC);
		func_add_param("s", TypeString);
	add_func("@s2f", TypeFloat32, (void*)&s2f, FLAG_STATIC);
		func_add_param("s", TypeString);
	add_func("@i2s", TypeString, (void*)&i2s, FLAG_STATIC);
		func_add_param("i", TypeInt);
	add_func("@i642s", TypeString, (void*)&i642s, FLAG_STATIC);
		func_add_param("i", TypeInt64);
	add_func("@f2s", TypeString, (void*)&f2s, FLAG_STATIC);
		func_add_param("f", TypeFloat32);
		func_add_param("decimals", TypeInt);
	add_func("@f2sf", TypeString, (void*)&f2sf, FLAG_STATIC);
		func_add_param("f", TypeFloat32);
	add_func("@f642sf", TypeString, (void*)&f642sf, FLAG_STATIC);
		func_add_param("f", TypeFloat64);
	add_func("@b2s", TypeString, (void*)&b2s, FLAG_STATIC);
		func_add_param("b", TypeBool);
	add_func("p2s", TypeString, (void*)&p2s, FLAG_STATIC);
		func_add_param("p", TypePointer);
	add_func("@ia2s", TypeString, (void*)&ia2s, FLAG_STATIC);
		func_add_param("a", TypeIntList);
	add_func("@fa2s", TypeString, (void*)&fa2s, FLAG_STATIC); // TODO...
		func_add_param("a", TypeFloatList);
	add_func("@ba2s", TypeString, (void*)&ba2s, FLAG_STATIC);
		func_add_param("a", TypeBoolList);
	add_func("@sa2s", TypeString, (void*)&sa2s, FLAG_STATIC);
		func_add_param("a", TypeStringList);
	// debug output
	/*add_func("cprint", TypeVoid, (void*)&_cstringout, FLAG_STATIC);
		func_add_param("str", TypeCString);*/
	add_func("print", TypeVoid, (void*)&_print, FLAG_STATIC);
		func_add_param("str", TypeString);
	// memory
	add_func("@malloc", TypePointer, (void*)&malloc, FLAG_STATIC);
		func_add_param("size", TypeInt);
	add_func("@free", TypeVoid, (void*)&free, FLAG_STATIC);
		func_add_param("p", TypePointer);
	// system
	add_func("shell_execute", TypeString, (void*)&kaba_shell_execute, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
		func_add_param("cmd", TypeString);


	add_func("-sorted-", TypeDynamicArray, (void*)&kaba_array_sort, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
		func_add_param("list", TypePointer);
		func_add_param("class", TypeClassP);
		func_add_param("by", TypeString);
	add_func("-filter-", TypeDynamicArray, (void*)&kaba_array_filter, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
		func_add_param("func", TypeFunctionP);
		func_add_param("list", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("-var2str-", TypeString, (void*)var2str, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassP);
	add_func("-map-", TypeDynamicArray, (void*)kaba_map, ScriptFlag(FLAG_RAISES_EXCEPTIONS | FLAG_STATIC));
		func_add_param("func", TypeFunctionP);
		func_add_param("array", TypePointer);


// add_func("ExecuteScript", TypeVoid);
//		func_add_param("filename", TypeString);
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

void Init(Asm::InstructionSet instruction_set, Abi abi, bool allow_std_lib) {
	Asm::Init(instruction_set);
	config.instruction_set = Asm::instruction_set.set;
	if (abi == Abi::NATIVE){
		if (config.instruction_set == Asm::InstructionSet::AMD64){
			abi = Abi::GNU_64;
#ifdef OS_WINDOWS
			abi = Abi::WINDOWS_64;
#endif
		}else if (config.instruction_set == Asm::InstructionSet::X86){
			abi = Abi::GNU_32;
#ifdef OS_WINDOWS
			abi = ABI_WINDOWS_32;
#endif
		}else if (config.instruction_set == Asm::InstructionSet::ARM){
			abi = Abi::GNU_ARM_32;
		}
	}
	config.abi = abi;
	config.allow_std_lib = allow_std_lib;
	config.pointer_size = Asm::instruction_set.pointer_size;
	if ((abi != Abi::NATIVE) or (instruction_set != Asm::InstructionSet::NATIVE))
		config.super_array_size = mem_align(config.pointer_size + 3 * sizeof(int), config.pointer_size);
	else
		config.super_array_size = sizeof(DynamicArray);
	config.stack_size = DEFAULT_STACK_SIZE;

	config.allow_simplification = true;
	config.allow_registers = true;
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
	
	SIAddPackageKaba();




	SIAddPackageMath();
	SIAddPackageFile();
	SIAddPackageImage();
	SIAddPackageHui();
	SIAddPackageNix();
	SIAddPackageNet();
	SIAddPackageSound();
	SIAddPackageThread();
	SIAddPackageX();

	cur_package = Packages[0];
	SIAddCommands();
	SIAddOperators();




	add_type_cast(10, TypeInt, TypeFloat32, "i2f", (void*)&CastInt2Float);
	add_type_cast(10, TypeInt, TypeInt64, "i2i64", (void*)&CastInt2Int64);
	add_type_cast(15, TypeInt64, TypeInt, "i642i", (void*)&CastInt642Int);
	add_type_cast(10, TypeFloat32, TypeFloat64,"f2f64", (void*)&CastFloat2Float64);
	add_type_cast(20, TypeFloat32, TypeInt, "f2i", (void*)&CastFloat2Int);
	add_type_cast(10, TypeInt, TypeChar, "i2c", (void*)&CastInt2Char);
	add_type_cast(20, TypeChar, TypeInt, "c2i", (void*)&CastChar2Int);
	add_type_cast(50, TypePointer, TypeBool, "p2b", (void*)&CastPointer2Bool);
	add_type_cast(50, TypeInt, TypeString, "@i2s", (void*)&CastInt2String);
	add_type_cast(50, TypeInt64, TypeString, "@i642s", (void*)&CastInt642String);
	add_type_cast(50, TypeFloat32, TypeString, "@f2sf", (void*)&CastFloat2String);
	add_type_cast(50, TypeFloat64, TypeString, "@f642sf", (void*)&CastFloat642String);
	add_type_cast(50, TypeBool, TypeString, "@b2s", (void*)&CastBool2String);
	add_type_cast(50, TypePointer, TypeString, "p2s", (void*)&CastPointer2String);
	add_type_cast(50, TypeIntList, TypeString, "@ia2s", nullptr);
	add_type_cast(50, TypeFloatList, TypeString, "@fa2s", nullptr);
	add_type_cast(50, TypeBoolList, TypeString, "@ba2s", nullptr);
	add_type_cast(50, TypeStringList, TypeString, "@sa2s", nullptr);


	// consistency checks
#ifndef NDEBUG
	for (auto *p: Packages)
		for (auto *c: p->classes()) {
			if (c->is_super_array()) {
				if (!c->get_default_constructor() or !c->get_assign() or !c->get_destructor())
					msg_error("SUPER ARRAY INCONSISTENT: " + c->name);
			}
			// x package failing
			/*for (auto *f: c->member_functions)
				if (f->needs_overriding and (f->name != IDENTIFIER_FUNC_SUBARRAY))
					msg_error(f->signature());*/
		}
#endif
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

	Array<string> names = name.explode(":");
	string sname = names[0].replace("@list", "[]").replace("@@", ".");
	for (auto *p: Packages)
		foreachi(Function *f, p->syntax->functions, i)
			if (f->long_name() == sname){
				if (names.num > 0)
					if (f->num_params != names[1]._int())
						continue;
				f->address = pointer;
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
#ifdef OS_WINDOWS
	return;
#endif
	VirtualTable *v = *(VirtualTable**)instance;

	ClassOffsetData d;
	d.class_name = class_name;
	d.element = func;
	d.offset = get_virtual_index(p, class_name, func);
	d.is_virtual = true;
	ClassOffsets.add(d);

	LinkExternal(class_name + "." + func, v[d.offset]);
}

int process_class_offset(const string &class_name, const string &element, int offset)
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
		if (f->long_name().match(fil))
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
