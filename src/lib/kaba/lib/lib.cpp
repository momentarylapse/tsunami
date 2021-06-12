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

#include "../kaba.h"
#include "lib.h"
#include "dict.h"
#include "../dynamic/exception.h"
#include "../../config.h"
#include "../../math/complex.h"
#include "../../any/any.h"



#ifdef _X_USE_HUI_
#include "../../hui/hui.h"
#endif



namespace kaba {


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
const Class *TypeStringAutoCast;
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
const Class *TypeFloat64List;
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


//   with type information

void add_operator_x(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func) {
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
		o->f = class_add_func_x(o->primitive->function_name, return_type, func, flags);
		if (p)
			func_add_param("b", p);
	} else {
		add_class(c);
		o->f = class_add_func_x(o->primitive->function_name, return_type, func, flags);
		func_add_param("a", c);
		if (p)
			func_add_param("b", p);
	}
	func_set_inline(inline_index);
	cur_package->syntax->operators.add(o);
}


void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, nullptr);
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//


Class *add_class(const Class *root_type) {
	cur_class = const_cast<Class*>(root_type);
	return cur_class;
}

void class_add_element_x(const string &name, const Class *type, int offset, Flags flag) {
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
		msg_error(format("could not override %s.%s", c->name, f->name));
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


Function* class_add_func_x(const string &name, const Class *return_type, void *func, Flags flags) {
	Function *f = new Function(name, return_type, cur_class, flags);
	cur_package->syntax->functions.add(f);
	f->address_preprocess = func;
	if (config.allow_std_lib)
		f->address = (int_p)func;
	cur_func = f;


	if (f->is_static())
		cur_class->functions.add(f);
	else
		_class_add_member_func(cur_class, f, flags);
	return f;
}

Function* class_add_func(const string &name, const Class *return_type, nullptr_t func, Flags flag) {
	return class_add_func_x(name, return_type, nullptr, flag);
}

int get_virtual_index(void *func, const string &tname, const string &name) {
	if ((config.native_abi == Abi::X86_WINDOWS) or (config.native_abi == Abi::AMD64_WINDOWS)) {
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
	} else if (config.native_abi == Abi::AMD64_WINDOWS) {
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

Function* class_add_func_virtual_x(const string &name, const Class *return_type, void *func, Flags flag) {
	string tname = cur_class->name;
	int index = get_virtual_index(func, tname, name);
	//msg_write("virtual: " + tname + "." + name);
	//msg_write(index);
	Function *f = class_add_func_x(name, return_type, func, flag);
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



Function *add_func_x(const string &name, const Class *return_type, void *func, Flags flag) {
	add_class(cur_package->base_class());
	return class_add_func_x(name, return_type, func, flag);
}

void func_set_inline(InlineID index) {
	if (cur_func)
		cur_func->inline_no = index;
}

void func_add_param(const string &name, const Class *type, Flags flags) {
	if (cur_func) {
		Variable *v = new Variable(name, type);
		v->flags = flags;
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

void script_make_super_array(Class *t, SyntaxTree *ps) {
	const Class *p = t->param[0];
	t->derive_from(TypeDynamicArray, false);
	t->param[0] = p;
	add_class(t);

	// already done by derive_from()
	//Function *sub = t->get_func(IDENTIFIER_FUNC_SUBARRAY, TypeDynamicArray, {nullptr,nullptr});
	//sub->literal_return_type = t;
	//sub->effective_return_type = t;

	// FIXME  wrong for complicated classes
	if (p->can_memcpy()) {
		if (!p->uses_call_by_reference()){
			if (p->is_pointer()){
				class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<void*>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_p_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_p_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
				class_add_func("__contains__", TypeBool, &PointerList::__contains__);
					func_add_param("x", p);
			}else if (p == TypeFloat32){
				class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<float>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_f_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_f_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p == TypeFloat64){
				class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<double>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_d_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_d_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p->size == 4){
				class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<int>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_4_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_4_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p->size == 1){
				class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &Array<char>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_1_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_1_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else{
				msg_error("evil class:  " + t->name);
			}
		}else{
			// __init__ must be defined manually...!
			class_add_func("add", TypeVoid, &DynamicArray::append_single);
				func_add_param("x", p);
			class_add_func("insert", TypeVoid, &DynamicArray::insert_single);
				func_add_param("x", p);
				func_add_param("index", TypeInt);
		}
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &DynamicArray::simple_clear);
		class_add_func("clear", TypeVoid, &DynamicArray::simple_clear);
		class_add_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, &DynamicArray::simple_assign);
			func_add_param("other", t);
		class_add_func("remove", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);
		class_add_func("resize", TypeVoid, &DynamicArray::simple_resize);
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
		hui::RaiseError("add_type_cast (ScriptInit): " + cmd + " not found");
#else
		msg_error("add_type_cast (ScriptInit): " + string(cmd) + " not found");
		exit(1);
#endif
	}
	c.source = source;
	c.dest = dest;
	TypeCasts.add(c);
}


void SIAddStatements();

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
void SIAddPackageDoc();
void SIAddPackageVulkan();



void init(Abi abi, bool allow_std_lib) {
	if (abi == Abi::NATIVE) {
		config.abi = config.native_abi;
	} else {
		config.abi = abi;
	}
	config.instruction_set = extract_instruction_set(config.abi);
	Asm::init(config.instruction_set);
	config.allow_std_lib = allow_std_lib;
	config.pointer_size = Asm::instruction_set.pointer_size;
	if (abi == Abi::NATIVE)
		config.super_array_size = sizeof(DynamicArray);
	else
		config.super_array_size = mem_align(config.pointer_size + 3 * sizeof(int), config.pointer_size);

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
	SIAddPackageDoc();
	SIAddPackageVulkan();

	add_package("base");
	SIAddXCommands();




	add_type_cast(10, TypeInt, TypeFloat32, "int.__float__");
	add_type_cast(10, TypeInt, TypeFloat64, "int.__float64__");
	add_type_cast(10, TypeInt, TypeInt64, "int.__int64__");
	add_type_cast(15, TypeInt64, TypeInt, "int64.__int__");
	add_type_cast(10, TypeFloat32, TypeFloat64,"float.__float64__");
	add_type_cast(20, TypeFloat32, TypeInt, "float.__int__");
	add_type_cast(10, TypeInt, TypeChar, "int.__char__");
	add_type_cast(20, TypeChar, TypeInt, "char.__int__");
	add_type_cast(30, TypeBoolList, TypeBool, "bool[].__bool__");
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

void clean_up() {
	delete_all_scripts(true, true);

	packages.clear();

	reset_external_data();
}
};
