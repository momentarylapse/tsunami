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
#include "dict.h"
#include "../../config.h"
#include "../../math/complex.h"
#include "../../any/any.h"



#ifdef _X_USE_HUI_
#include "../../hui/hui.h"
#endif



namespace kaba {


const string IDENTIFIER_CLASS = "class";
const string IDENTIFIER_FUNC = "func";
const string IDENTIFIER_FUNC_INIT = "__init__";
const string IDENTIFIER_FUNC_DELETE = "__delete__";
const string IDENTIFIER_FUNC_ASSIGN = "__assign__";
const string IDENTIFIER_FUNC_GET = "__get__";
const string IDENTIFIER_FUNC_SET = "__set__";
const string IDENTIFIER_FUNC_LENGTH = "__length__";
const string IDENTIFIER_FUNC_STR = "__str__";
const string IDENTIFIER_FUNC_REPR = "__repr__";
const string IDENTIFIER_FUNC_SUBARRAY = "__subarray__";
const string IDENTIFIER_FUNC_SHARED_CLEAR = "_clear";
const string IDENTIFIER_FUNC_SHARED_CREATE = "_create";
const string IDENTIFIER_SUPER = "super";
const string IDENTIFIER_SELF = "self";
const string IDENTIFIER_EXTENDS = "extends";
const string IDENTIFIER_STATIC = "static";
const string IDENTIFIER_NEW = "new";
const string IDENTIFIER_DELETE = "del";
const string IDENTIFIER_SIZEOF = "sizeof";
const string IDENTIFIER_TYPE = "type";
const string IDENTIFIER_STR = "str";
const string IDENTIFIER_REPR = "repr";
const string IDENTIFIER_LEN = "len";
const string IDENTIFIER_LET = "let";
const string IDENTIFIER_NAMESPACE = "namespace";
const string IDENTIFIER_RETURN_VAR = "-return-";
const string IDENTIFIER_VTABLE_VAR = "-vtable-";
const string IDENTIFIER_SHARED_COUNT = "_shared_ref_count";
const string IDENTIFIER_ENUM = "enum";
const string IDENTIFIER_CONST = "const";
const string IDENTIFIER_OUT = "out";
const string IDENTIFIER_OVERRIDE = "override";
const string IDENTIFIER_VIRTUAL = "virtual";
const string IDENTIFIER_EXTERN = "extern";
//const string IDENTIFIER_ACCESSOR = "accessor";
const string IDENTIFIER_SELFREF = "selfref";
const string IDENTIFIER_WEAK = "weak";
const string IDENTIFIER_SHARED = "shared";
const string IDENTIFIER_OWNED = "owned";
const string IDENTIFIER_PURE = "pure";
const string IDENTIFIER_THROWS = "throws";
const string IDENTIFIER_USE = "use";
const string IDENTIFIER_IMPORT = "import";
const string IDENTIFIER_RETURN = "return";
const string IDENTIFIER_RAISE = "raise";
const string IDENTIFIER_TRY = "try";
const string IDENTIFIER_EXCEPT = "except";
const string IDENTIFIER_IF = "if";
const string IDENTIFIER_ELSE = "else";
const string IDENTIFIER_WHILE = "while";
const string IDENTIFIER_FOR = "for";
const string IDENTIFIER_IN = "in";
const string IDENTIFIER_AS = "as";
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
const string IDENTIFIER_DYN = "dyn";
const string IDENTIFIER_CALL = "call";

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
const Class *TypeObject;
const Class *TypeObjectP;
const Class *TypeBool;
const Class *TypeInt;
const Class *TypeInt64;
const Class *TypeFloat;
const Class *TypeFloat32;
const Class *TypeFloat64;
const Class *TypeChar;
const Class *TypeString = nullptr;
const Class *TypeCString;

const Class *TypeVector;
const Class *TypeRect;
const Class *TypeColor;
const Class *TypeQuaternion;
const Class *TypeAny = nullptr;
const Class *TypeAnyList;
const Class *TypeAnyDict;
 // internal:
const Class *TypeDynamic;
const Class *TypeDynamicArray;
const Class *TypeDictBase;
const Class *TypeSharedPointer;
const Class *TypePointerList;
const Class *TypeCharPs;
const Class *TypeBoolPs;
const Class *TypeBoolList;
const Class *TypeIntPs;
const Class *TypeIntP;
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
const Class *TypeColorList;
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

extern const Class *TypePath;


shared_array<Script> packages;
Script *cur_package = nullptr;


static Function *cur_func = nullptr;
static Class *cur_class;

bool flags_has(Flags flags, Flags t) {
	return ((int(flags) & int(t)) == int(t));
}

void flags_set(Flags &flags, Flags t) {
	flags = Flags(int(flags) | int(t));
}

void flags_clear(Flags &flags, Flags t) {
	flags = Flags(int(flags) & (~int(t)));
}

Flags flags_mix(const Array<Flags> &f) {
	Flags r = Flags::NONE;
	for (Flags ff: f)
		r = Flags(int(r) | int(ff));
	return r;
}

void add_package(const string &name, Flags flags) {
	for (auto &p: packages)
		if (p->filename.str() == name) {
			cur_package = p.get();
			return;
		}
	shared<Script> s = new Script;
	s->used_by_default = flags_has(flags, Flags::AUTO_IMPORT);
	s->syntax->base_class->name = name;
	s->filename = name;
	packages.add(s);
	cur_package = s.get();
}

void __add_class__(Class *t, const Class *name_space) {
	if (name_space) {
		const_cast<Class*>(name_space)->classes.add(t);
		t->name_space = name_space;
	} else {
		cur_package->syntax->base_class->classes.add(t);
		t->name_space = cur_package->syntax->base_class;
	}
}

const Class *add_type(const string &name, int size, Flags flag, const Class *name_space) {
	Class *t = new Class(name, size, cur_package->syntax);
	if (flags_has(flag, Flags::CALL_BY_VALUE))
		flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
	__add_class__(t, name_space);
	return t;
}

const Class *add_type_p(const Class *sub_type, Flags flag, const string &_name) {
	string name = _name;
	if (name == "") {
		if (flags_has(flag, Flags::SHARED))
			name = "shared " + sub_type->name;
		else if (flags_has(flag, Flags::SILENT))
			name = sub_type->name + "&";
		else
			name = sub_type->name + "*";
	}
	Class *t = new Class(name, config.pointer_size, cur_package->syntax, nullptr, {sub_type});
	t->type = Class::Type::POINTER;
	if (flags_has(flag, Flags::SILENT))
		t->type = Class::Type::POINTER_SILENT;
	else if (flags_has(flag, Flags::SHARED))
		t->type = Class::Type::POINTER_SHARED;
	__add_class__(t, sub_type->name_space);
	return t;
}

// fixed array
const Class *add_type_a(const Class *sub_type, int array_length, const string &_name) {
	string name = _name;
	if (name == "")
		name = sub_type->name + "[" + i2s(array_length) + "]";
	Class *t = new Class(name, 0, cur_package->syntax, nullptr, {sub_type});
	t->size = sub_type->size * array_length;
	t->type = Class::Type::ARRAY;
	t->array_length = array_length;
	__add_class__(t, sub_type->name_space);
	return t;
}

// super array
const Class *add_type_l(const Class *sub_type, const string &_name) {
	string name = _name;
	if (name == "")
		name = sub_type->name + "[]";
	Class *t = new Class(name, 0, cur_package->syntax, nullptr, {sub_type});
	t->size = config.super_array_size;
	t->type = Class::Type::SUPER_ARRAY;
	script_make_super_array(t);
	__add_class__(t, sub_type->name_space);
	return t;
}

const Class *add_type_d(const Class *sub_type, const string &_name) {
	string name = _name;
	if (name == "")
		name = sub_type->name + "{}";
	Class *t = new Class(name, config.super_array_size, cur_package->syntax, nullptr, {sub_type});
	t->type = Class::Type::DICT;
	script_make_dict(t);
	__add_class__(t, sub_type->name_space);
	return t;
}

const Class *add_type_f(const Class *ret_type, const Array<const Class*> &params) {
	return cur_package->syntax->make_class_func(params, ret_type);
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")

PrimitiveOperator PrimitiveOperators[(int)OperatorID::_COUNT_] = {
	{"=",  OperatorID::ASSIGN,        true,  0, IDENTIFIER_FUNC_ASSIGN, 3, false},
	{"+",  OperatorID::ADD,           false, 11, "__add__", 3, false},
	{"-",  OperatorID::SUBTRACT,      false, 11, "__sub__", 3, false},
	{"*",  OperatorID::MULTIPLY,      false, 12, "__mul__", 3, false},
	{"/",  OperatorID::DIVIDE,        false, 12, "__div__", 3, false},
	{"-",  OperatorID::NEGATIVE,      false, 13, "__neg__", 2, false}, // -1 etc
	{"+=", OperatorID::ADDS,          true,  0,  "__iadd__", 3, false},
	{"-=", OperatorID::SUBTRACTS,     true,  0,  "__isub__", 3, false},
	{"*=", OperatorID::MULTIPLYS,     true,  0,  "__imul__", 3, false},
	{"/=", OperatorID::DIVIDES,       true,  0,  "__idiv__", 3, false},
	{"==", OperatorID::EQUAL,         false, 8,  "__eq__", 3, false},
	{"!=", OperatorID::NOTEQUAL,      false, 8,  "__ne__", 3, false},
	{IDENTIFIER_NOT,OperatorID::NEGATE,        false, 2,  "__not__", 2, false},
	{"<",  OperatorID::SMALLER,       false, 9,  "__lt__", 3, false},
	{">",  OperatorID::GREATER,       false, 9,  "__gt__", 3, false},
	{"<=", OperatorID::SMALLER_EQUAL, false, 9,  "__le__", 3, false},
	{">=", OperatorID::GREATER_EQUAL, false, 9,  "__ge__", 3, false},
	{IDENTIFIER_AND, OperatorID::AND, false, 4,  "__and__", 3, false},
	{IDENTIFIER_OR,  OperatorID::OR,  false, 3,  "__or__", 3, false},
	{"%",  OperatorID::MODULO,        false, 12, "__mod__", 3, false},
	{"&",  OperatorID::BIT_AND,       false, 7, "__bitand__", 3, false},
	{"|",  OperatorID::BIT_OR,        false, 5, "__bitor__", 3, false},
	{"<<", OperatorID::SHIFT_LEFT,    false, 10, "__lshift__", 3, false},
	{">>", OperatorID::SHIFT_RIGHT,   false, 10, "__rshift__", 3, false},
	{"++", OperatorID::INCREASE,      true,  2, "__inc__", 1, false},
	{"--", OperatorID::DECREASE,      true,  2, "__dec__", 1, false},
	{IDENTIFIER_IS, OperatorID::IS,   false, 2,  "-none-", 3, false},
	{IDENTIFIER_IN, OperatorID::IN,   false, 12, "__contains__", 3, true}, // INVERTED
	{IDENTIFIER_EXTENDS, OperatorID::EXTENDS, false, 2,  "-none-", 3, false},
	{"^",  OperatorID::EXPONENT,      false, 14,  "__exp__", 3, false},
	{",",  OperatorID::COMMA,      false, 1,  "-none-", 3, false},
	{"*",  OperatorID::DEREFERENCE,      false, 15,  "__get__", 2, false},
	{"&",  OperatorID::REFERENCE,      false, 15,  "-none-", 2, false},
	{"[...]",  OperatorID::ARRAY,      false, 16,  "-none-", 3, false}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

//   with type information

void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func) {
	Operator *o = new Operator;
	o->owner = cur_package->syntax;
	o->primitive = &PrimitiveOperators[(int)primitive_op];
	o->return_type = return_type;
	o->param_type_1 = param_type1;
	o->param_type_2 = param_type2;
	auto c = param_type1;
	auto p = param_type2;
	if (!c) {
		c = p;
		p = nullptr;
	}

	Flags flags = Flags::NONE;
	if (!o->primitive->left_modifiable)
		flags = Flags::PURE;

	//if (!c->uses_call_by_reference())
	if (o->primitive->left_modifiable and !c->uses_call_by_reference())
		flags = flags_mix({flags, Flags::STATIC});

	if (!flags_has(flags, Flags::STATIC)) {
		add_class(c);
		o->f = class_add_func(o->primitive->function_name, return_type, func, flags);
		if (p)
			func_add_param("b", p);
	} else {
		add_class(c);
		o->f = class_add_func(o->primitive->function_name, return_type, func, flags);
		func_add_param("a", c);
		if (p)
			func_add_param("b", p);
	}
	func_set_inline(inline_index);
	cur_package->syntax->operators.add(o);
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//


Class *add_class(const Class *root_type) {
	cur_class = const_cast<Class*>(root_type);
	return cur_class;
}

void class_add_element(const string &name, const Class *type, int offset, Flags flag) {
	cur_class->elements.add(ClassElement(name, type, offset));
}

void class_derive_from(const Class *parent, bool increase_size, bool copy_vtable) {
	cur_class->derive_from(parent, increase_size);
	if (copy_vtable)
		cur_class->vtable = parent->vtable;
}

int _class_override_num_params = -1;

void _class_add_member_func(const Class *ccc, Function *f, Flags flag) {
	Class *c = const_cast<Class*>(ccc);
	if (flags_has(flag, Flags::OVERRIDE)) {
		foreachi(Function *ff, weak(c->functions), i)
			if (ff->name == f->name) {
				if (_class_override_num_params < 0 or _class_override_num_params == ff->num_params) {
					//msg_write("OVERRIDE");
					c->functions[i] = f;
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
		c->functions.add(f);
	}
}


Function* class_add_func(const string &name, const Class *return_type, void *func, Flags flags) {
	Function *f = new Function(name, return_type, cur_class, flags);
	cur_package->syntax->functions.add(f);
	f->address_preprocess = func;
	if (config.allow_std_lib)
		f->address = func;
	cur_func = f;


	if (f->is_static())
		cur_class->functions.add(f);
	else
		_class_add_member_func(cur_class, f, flags);
	return f;
}

int get_virtual_index(void *func, const string &tname, const string &name) {
	if ((config.abi == Abi::WINDOWS_32) or (config.abi == Abi::WINDOWS_64)) {
		if (!func)
			return 0;
		unsigned char* pp = (unsigned char*)func;
		try {
			//if ((cur_class->vtable) and (pp[0] == 0x8b) and (pp[1] == 0x01) and (pp[2] == 0xff) and (pp[3] == 0x60)){
			if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
				// 8b.44.24.**    8b.00     ff.60.10
				// virtual function
				return (int)pp[8] / 4;
			} else if (pp[0] == 0xe9) {
				// jmp
				//msg_write(Asm::disassemble(func, 16));
				pp = &pp[5] + *(int*)&pp[1];
				//msg_write(Asm::disassemble(pp, 16));
				if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
					// x86
					// 8b.44.24.**    8b.00     ff.60.10
					// virtual function
					return (unsigned int)pp[8] / 4;
				} else if ((pp[0] == 0x48) and (pp[1] == 0x8b) and (pp[2] == 0x01) and (pp[3] == 0xff) and (pp[4] == 0x60)) {
					// amd64
					// 48.8b.01   mov rax, [ecx]
					// ff.60.NN   jmp [eax+N*8]
					// virtual function
					return (unsigned int)pp[5] / 8;
				} else if ((pp[0] == 0x48) and (pp[1] == 0x8b) and (pp[2] == 0x01) and (pp[3] == 0xff) and (pp[4] == 0xa0)) {
					// amd64
					// 48.8b.01            mov rax, [ecx]
					// ff.a0.NN.NN.NN.NN   jmp [eax+N*8]
					// virtual function
					int* ppi = (int*)&pp[5];
					return *ppi / 8;
				} else {
					throw(1);
				}
			} else {
				throw(1);
			}
		} catch (...) {
			msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
			msg_write(p2s(pp));
			msg_write(Asm::disassemble(func, 16));
		}
	} else if (config.abi == Abi::WINDOWS_64) {
		msg_error("Script class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
		msg_write(Asm::disassemble(func, 16));
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

Function* class_add_func_virtual(const string &name, const Class *return_type, void *func, Flags flag) {
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

	// enums can't be referenced...
	if (type == TypeInt)
		*(const void**)c->p() = value;
	else if (value)
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



Array<Statement*> Statements;

Function *add_func(const string &name, const Class *return_type, void *func, Flags flag) {
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
		v->is_const = true;
		cur_func->var.add(v);
		cur_func->literal_param_type.add(type);
		cur_func->num_params ++;
	}
}

class PointerList : public Array<void*> {
public:
	bool __contains__(void *x) {
		return find(x) >= 0;
	}
};

void script_make_super_array(Class *t, SyntaxTree *ps)
{
	const Class *p = t->param[0];
	t->derive_from(TypeDynamicArray, false);
	t->param[0] = p;
	add_class(t);

	Function *sub = t->get_func(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, {nullptr,nullptr});
	sub->literal_return_type = t;
	sub->effective_return_type= t;

	// FIXME  wrong for complicated classes
	if (p->can_memcpy()) {
		if (!p->uses_call_by_reference()){
			if (p->is_pointer()){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<void*>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_p_single);
					func_add_param("x", p);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_p_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
				class_add_funcx("__contains__", TypeBool, &PointerList::__contains__);
					func_add_param("x", p);
			}else if (p == TypeFloat32){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<float>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_f_single);
					func_add_param("x", p);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_f_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p == TypeFloat64){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<double>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_d_single);
					func_add_param("x", p);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_d_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p->size == 4){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<int>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_4_single);
					func_add_param("x", p);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_4_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p->size == 1){
				class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<char>::__init__);
				class_add_funcx("add", TypeVoid, &DynamicArray::append_1_single);
					func_add_param("x", p);
				class_add_funcx("insert", TypeVoid, &DynamicArray::insert_1_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else{
				msg_error("evil class:  " + t->name);
			}
		}else{
			// __init__ must be defined manually...!
			class_add_funcx("add", TypeVoid, &DynamicArray::append_single);
				func_add_param("x", p);
			class_add_funcx("insert", TypeVoid, &DynamicArray::insert_single);
				func_add_param("x", p);
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
	}else if (p == TypeString or p == TypeAny or p == TypePath){
		// handled manually later...
	}else{
		msg_error("evil class:  " + t->name);
	}
}




Array<TypeCast> TypeCasts;
void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd) {
	TypeCast c;
	c.penalty = penalty;
	c.f = nullptr;
	for (auto *f: cur_package->syntax->functions)
		if (f->long_name() == cmd){
			c.f = f;
			break;
		}
	if (!c.f){
#ifdef _X_USE_HUI_
		hui::ErrorBox(nullptr, "", "add_type_cast (ScriptInit): " + cmd + " not found");
		hui::RaiseError("add_type_cast (ScriptInit): " + cmd + " not found");
#else
		msg_error("add_type_cast (ScriptInit): " + string(cmd) + " not found"));
		exit(1);
#endif
	}
	c.source = source;
	c.dest = dest;
	TypeCasts.add(c);
}


void SIAddStatements() {
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
	add_statement(IDENTIFIER_NEW, StatementID::NEW, 1);
	add_statement(IDENTIFIER_DELETE, StatementID::DELETE, 1);
	add_statement(IDENTIFIER_SIZEOF, StatementID::SIZEOF, 1);
	add_statement(IDENTIFIER_TYPE, StatementID::TYPE, 1);
	add_statement(IDENTIFIER_STR, StatementID::STR, 1);
	add_statement(IDENTIFIER_REPR, StatementID::REPR, 1);
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
	add_statement(IDENTIFIER_DYN, StatementID::DYN);
	add_statement(IDENTIFIER_CALL, StatementID::CALL);
	add_statement(IDENTIFIER_WEAK, StatementID::WEAK, 1);
}




void SIAddXCommands();
void SIAddPackageBase();
void SIAddPackageKaba();
void SIAddPackageTime();
void SIAddPackageOS();
void SIAddPackageOSPath();
void SIAddPackageMath();
void SIAddPackageThread();
void SIAddPackageHui();
void SIAddPackageNix();
void SIAddPackageNet();
void SIAddPackageImage();
void SIAddPackageVulkan();

CompilerConfiguration::CompilerConfiguration() {
	abi = Abi::NATIVE;
	instruction_set = Asm::InstructionSet::NATIVE;
	interpreted = false;
	allow_std_lib = true;
	pointer_size = sizeof(void*);
	super_array_size = sizeof(DynamicArray);

	allow_simplification = true;
	allow_registers = true;
	allow_simplify_consts = true;
	stack_mem_align = 8;
	function_align = 2 * pointer_size;
	stack_frame_align = 2 * pointer_size;

	compile_silently = false;
	verbose = false;
	verbose_func_filter = "*";
	verbose_stage_filter = "*";
	show_compiler_stats = true;

	compile_os = false;
	remove_unused = false;
	use_new_serializer = true;
	override_variables_offset = false;
	variables_offset = 0;
	override_code_origin = false;
	code_origin = 0;
	add_entry_point = false;
	no_function_frame = false;

	function_address_offset = element_offset(&Function::address); // offsetof(Function, address);
}

void init(Asm::InstructionSet instruction_set, Abi abi, bool allow_std_lib) {
	Asm::init(instruction_set);
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
			abi = Abi::WINDOWS_32;
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

	config.function_align = 2 * config.pointer_size;
	config.stack_frame_align = 2 * config.pointer_size;


	SIAddStatements();

	SIAddPackageBase();
	SIAddPackageOSPath();
	SIAddPackageKaba();
	SIAddPackageMath();
	SIAddPackageTime();
	SIAddPackageOS();
	SIAddPackageImage();
	SIAddPackageHui();
	SIAddPackageNix();
	SIAddPackageNet();
	SIAddPackageThread();
	SIAddPackageVulkan();

	add_package("base");
	SIAddXCommands();




	add_type_cast(10, TypeInt, TypeFloat32, "int.__float__");
	add_type_cast(10, TypeInt, TypeInt64, "int.__int64__");
	add_type_cast(15, TypeInt64, TypeInt, "int64.__int__");
	add_type_cast(10, TypeFloat32, TypeFloat64,"float.__float64__");
	add_type_cast(20, TypeFloat32, TypeInt, "float.__int__");
	add_type_cast(10, TypeInt, TypeChar, "int.__char__");
	add_type_cast(20, TypeChar, TypeInt, "char.__int__");
	add_type_cast(50, TypePointer, TypeBool, "p2b");
	add_type_cast(50, TypePointer, TypeString, "p2s");
	add_package("math");
	add_type_cast(50, TypeInt, TypeAny, "math.@int2any");
	add_type_cast(50, TypeFloat32, TypeAny, "math.@float2any");
	add_type_cast(50, TypeBool, TypeAny, "math.@bool2any");
	add_type_cast(50, TypeString, TypeAny, "math.@str2any");
	add_type_cast(50, TypePointer, TypeAny, "math.@pointer2any");
	add_package("os");
	add_type_cast(50, TypeString, TypePath, "os.Path.@from_str");


	// consistency checks
	// -> now done by regression tests!
}

void reset_external_data() {
	ExternalLinks.clear();
	ClassOffsets.clear();
	ClassSizes.clear();
}

// program variables - specific to the surrounding program, can't always be there...
void link_external(const string &name, void *pointer) {
	ExternalLinkData l;
	l.name = name;
	l.pointer = pointer;
	if (abs((int_p)pointer) < 1000)
		msg_error("probably a virtual function: " + name);
	ExternalLinks.add(l);

	Array<string> names = name.explode(":");
	string sname = names[0].replace("@list", "[]").replace("@@", ".");
	for (auto p: packages)
		foreachi(Function *f, p->syntax->functions, i)
			if (f->cname(p->base_class()) == sname) {
				int n = f->num_params;
				if (!f->is_static())
					n ++;
				if (names.num > 0)
					if (n != names[1]._int())
						continue;
				f->address = pointer;
			}
}

void *get_external_link(const string &name) {
	for (ExternalLinkData &l: ExternalLinks)
		if (l.name == name)
			return l.pointer;
	return nullptr;
}

void declare_class_size(const string &class_name, int size) {
	ClassSizeData d;
	d.class_name = class_name;
	d.size = size;
	ClassSizes.add(d);
}

void split_namespace(const string &name, string &class_name, string &element) {
	int p = name.rfind(".");
	class_name = name.substr(0, p);
	element = name.tail(name.num - p - 1);
}

void _declare_class_element(const string &name, int offset) {
	ClassOffsetData d;
	split_namespace(name, d.class_name, d.element);
	d.offset = offset;
	d.is_virtual = false;
	ClassOffsets.add(d);
}

void _link_external_virtual(const string &name, void *p, void *instance) {
#ifdef OS_WINDOWS
	return;
#endif
	VirtualTable *v = *(VirtualTable**)instance;


	ClassOffsetData d;
	split_namespace(name, d.class_name, d.element);
	d.offset = get_virtual_index(p, d.class_name, d.element);
	d.is_virtual = true;
	ClassOffsets.add(d);

	link_external(name, v[d.offset]);
}

int process_class_offset(const string &class_name, const string &element, int offset) {
	for (auto &d: ClassOffsets)
		if ((d.class_name == class_name) and (d.element == element))
			return d.offset;
	return offset;
}

int process_class_size(const string &class_name, int size) {
	for (auto &d: ClassSizes)
		if (d.class_name == class_name)
			return d.size;
	return size;
}

int process_class_num_virtuals(const string &class_name, int num_virtual) {
	for (auto &d: ClassOffsets)
		if ((d.class_name == class_name) and (d.is_virtual))
			num_virtual = max(num_virtual, d.offset + 1);
	return num_virtual;
}

void clean_up() {
	delete_all_scripts(true, true);

	packages.clear();

	reset_external_data();
}


bool CompilerConfiguration::allow_output_func(const Function *f) {
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

bool CompilerConfiguration::allow_output_stage(const string &stage) {
	if (!verbose)
		return false;
	auto filters = verbose_stage_filter.explode(",");
	for (auto &fil: filters)
		if (stage.match(fil))
			return true;
	return false;
}

bool CompilerConfiguration::allow_output(const Function *f, const string &stage) {
	if (!verbose)
		return false;
	if (!allow_output_func(f))
		return false;
	if (!allow_output_stage(stage))
		return false;
	return true;
}

};
