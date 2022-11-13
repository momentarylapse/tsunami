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
#include "../parser/template.h"
#include "lib.h"
#include "dict.h"
#include "../dynamic/exception.h"
#include "../../config.h"
#include "../../math/complex.h"
#include "../../any/any.h"
#include "../../base/callable.h"
#include "../../base/iter.h"
#include "../../os/msg.h"


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

const Class *TypeVec3;
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
const Class *TypeCallableBase;
const Class *TypeSharedPointer;
const Class *TypePointerList;
const Class *TypeBoolList;
const Class *TypeIntP;
const Class *TypeIntList;
const Class *TypeIntDict;
const Class *TypeFloatP;
const Class *TypeFloatList;
const Class *TypeFloatDict;
const Class *TypeFloat64List;
const Class *TypeComplex;
const Class *TypeComplexList;
const Class *TypeStringList;
const Class *TypeStringDict;
const Class *TypeVec2;
const Class *TypeVec2List;
const Class *TypeVec3List;
const Class *TypeMat4;
const Class *TypePlane;
const Class *TypePlaneList;
const Class *TypeColorList;
const Class *TypeMat3;
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


shared_array<Module> packages;
Module *cur_package = nullptr;


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
	shared<Module> s = new Module;
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
	Class *t = new Class(Class::Type::REGULAR, name, size, cur_package->syntax);
	if (flags_has(flag, Flags::CALL_BY_VALUE))
		flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
	__add_class__(t, name_space);
	return t;
}

const Class *add_type_p(const Class *sub_type, Flags flag, const string &_name) {
	string name = _name;
	if (name == "") {
		if (flags_has(flag, Flags::SHARED))
			name = sub_type->name + " shared";
		else
			name = sub_type->name + "*";
	}
	Class *t = new Class(Class::Type::POINTER, name, config.pointer_size, cur_package->syntax, nullptr, {sub_type});
	if (flags_has(flag, Flags::SHARED))
		t->type = Class::Type::POINTER_SHARED;
	__add_class__(t, sub_type->name_space);
	implicit_class_registry::add(t);
	return t;
}

// fixed array
const Class *add_type_a(const Class *sub_type, int array_length, const string &_name) {
	string name = _name;
	if (name == "")
		name = sub_type->name + "[" + i2s(array_length) + "]";
	Class *t = new Class(Class::Type::ARRAY, name, sub_type->size * array_length, cur_package->syntax, nullptr, {sub_type});
	t->array_length = array_length;
	__add_class__(t, sub_type->name_space);
	implicit_class_registry::add(t);
	return t;
}

// super array
const Class *add_type_l(const Class *sub_type, const string &_name) {
	string name = _name;
	if (name == "")
		name = sub_type->name + "[]";
	Class *t = new Class(Class::Type::SUPER_ARRAY, name, config.super_array_size, cur_package->syntax, nullptr, {sub_type});
	kaba_make_super_array(t);
	__add_class__(t, sub_type->name_space);
	implicit_class_registry::add(t);
	return t;
}

// dict
const Class *add_type_d(const Class *sub_type, const string &_name) {
	string name = _name;
	if (name == "")
		name = sub_type->name + "{}";
	Class *t = new Class(Class::Type::DICT, name, config.super_array_size, cur_package->syntax, nullptr, {sub_type});
	kaba_make_dict(t);
	__add_class__(t, sub_type->name_space);
	implicit_class_registry::add(t);
	return t;
}

// enum
const Class *add_type_e(const string &name, const Class *_namespace) {
	Class *t = new Class(Class::Type::ENUM, name, sizeof(int), cur_package->syntax);
	flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);
	__add_class__(t, _namespace);
	return t;
}

template<typename Sig>
class KabaCallable;

template<typename R, typename ...A>
class KabaCallable<R(A...)> : public Callable<R(A...)> {
public:
	typedef R t_func(A...);
	Function *f;

	KabaCallable(Function *_f) {
		f = _f;
	}
	void __init__(Function *_f) {
		new(this) KabaCallable<R(A...)>(_f);
	}

	R operator()(A... args) const override {
		if (f) {
			auto fp = (t_func*)f->address;
			return fp(args...);
		} else {
			throw EmptyCallableError();
		}
	}
};

string make_callable_signature(const Array<const Class*> &param, const Class *ret);

const Class *add_type_f(const Class *ret_type, const Array<const Class*> &params) {
	string name = make_callable_signature(params, ret_type);

	auto params_ret = params;
	if ((params.num == 1) and (params[0] == TypeVoid))
		params_ret = {};
	params_ret.add(ret_type);

	//auto ff = cur_package->syntax->make_class("Callable[" + name + "]", Class::Type::CALLABLE_FUNCTION_POINTER, TypeCallableBase->size, 0, nullptr, params_ret, cur_package->syntax->base_class);
	Class *ff = new Class(Class::Type::CALLABLE_FUNCTION_POINTER, "XCallable[" + name + "]", /*TypeCallableBase->size*/ sizeof(KabaCallable<void()>), cur_package->syntax, nullptr, params_ret);
	__add_class__(ff, cur_package->syntax->base_class);
	implicit_class_registry::add(ff);

	auto ptr_param = [] (const Class *p) {
		return p->is_pointer() or p->uses_call_by_reference();
	};

	add_class(ff);
	if (ret_type == TypeVoid) {
		if (params.num == 0) {
			class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaCallable<void()>::__init__);
				func_add_param("fp", TypePointer);
			class_add_func_virtual("call", TypeVoid, &KabaCallable<void()>::operator());
		} else if (params.num == 1 and ptr_param(params[0])) {
			class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaCallable<void(void*)>::__init__);
				func_add_param("fp", TypePointer);
			class_add_func_virtual("call", TypeVoid, &KabaCallable<void(void*)>::operator());
				func_add_param("a", params[0]);
		} else if (params.num == 2 and ptr_param(params[0]) and ptr_param(params[1])) {
			class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &KabaCallable<void(void*,void*)>::__init__);
				func_add_param("fp", TypePointer);
			class_add_func_virtual("call", TypeVoid, &KabaCallable<void(void*,void*)>::operator());
				func_add_param("a", params[0]);
				func_add_param("b", params[1]);
		}
	}
	return cur_package->syntax->request_implicit_class(name, Class::Type::POINTER, config.pointer_size, 0, nullptr, {ff}, cur_package->syntax->base_class, -1);

	/*auto c = cur_package->syntax->make_class_callable_fp(params, ret_type);
	add_class(c);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &kaba_callable_fp_init);
			func_add_param("fp", TypePointer);
	return c;*/
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//


//   with type information

Array<Operator*> global_operators;

void add_operator_x(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func) {
	Operator *o = new Operator;
	o->owner = cur_package->syntax;
	o->abstract = &abstract_operators[(int)primitive_op];
	o->return_type = return_type;
	if (!param_type1) {
		param_type1 = param_type2;
		param_type2 = nullptr;
	}
	o->param_type_1 = param_type1;
	o->param_type_2 = param_type2;
	auto c = param_type1;
	auto p = param_type2;
	if (!c) {
		c = p;
		p = nullptr;
	}

	Flags flags = Flags::NONE;
	if (!o->abstract->left_modifiable)
		flags = Flags::PURE;

	//if (!c->uses_call_by_reference())
	if (o->abstract->left_modifiable and !c->uses_call_by_reference())
		flags_set(flags, Flags::STATIC);

	if (!flags_has(flags, Flags::STATIC)) {
		add_class(c);
		o->f = class_add_func_x(o->abstract->function_name, return_type, func, flags);
		if (p)
			func_add_param("b", p);
	} else {
		add_class(c);
		o->f = class_add_func_x(o->abstract->function_name, return_type, func, flags);
		func_add_param("a", c);
		if (p)
			func_add_param("b", p);
	}
	func_set_inline(inline_index);
	if (inline_index != InlineID::NONE)
		global_operators.add(o);
}


void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, nullptr);
}
void add_operator(OperatorID primitive_op, const Class* return_type, const Class* param_type1, const Class* param_type2, InlineID inline_index, std::nullptr_t func) {
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

void _class_add_member_func(const Class *ccc, Function *f, Flags flag) {
	Class *c = const_cast<Class*>(ccc);
	if (flags_has(flag, Flags::OVERRIDE)) {
		for (auto&& [i, ff]: enumerate(weak(c->functions)))
			if (ff->name == f->name) {
				//msg_write("OVERRIDE");
				c->functions[i] = f;
				return;
			}
		msg_error(format("could not override %s.%s", c->name, f->name));
	} else {
		// name alone is not enough for matching...
		/*for (auto&& [i,ff]: enumerate(c->functions))
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


	if (f->is_static()) {
		cur_class->functions.add(f);
	} else {
		f->add_self_parameter();
		_class_add_member_func(cur_class, f, flags);
	}
	return f;
}

Function* class_add_func(const string &name, const Class *return_type, std::nullptr_t func, Flags flag) {
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
			msg_error("class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
			msg_write(p2s(pp));
			msg_write(Asm::disassemble(func, 16));
		}
	} else if (config.native_abi == Abi::AMD64_WINDOWS) {
		msg_error("class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
		msg_write(Asm::disassemble(func, 16));
	} else {

		int_p p = (int_p)func;
		if ((p & 1) > 0) {
			// virtual function
			return p / sizeof(void*);
		} else if (!func) {
			return 0;
		} else {
			msg_error("class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
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
	if (type == TypeInt or type->is_enum())
		c->as_int64() = (int_p)value;
		//*(const void**)c->p() = value;
	else if (type == TypeString)
		c->as_string() = *(const string*)value;
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
	flags_set(v->flags, Flags::EXTERN); // prevent initialization when importing
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
		// FIXME: use call-by-reference type?
		Variable *v = new Variable(name, type);
		v->flags = flags;
		cur_func->var.add(v);
		cur_func->literal_param_type.add(type);
		cur_func->num_params ++;
		cur_func->mandatory_params = cur_func->num_params;
	}
}

void func_add_param_def_x(const string &name, const Class *type, const void *p, Flags flags) {
	if (cur_func) {
		// FIXME: use call-by-reference type?
		Variable *v = new Variable(name, type);
		v->flags = flags;
		cur_func->var.add(v);
		cur_func->literal_param_type.add(type);
		cur_func->num_params ++;
		//cur_func->mandatory_params = cur_func->num_params;

		Constant *c = cur_package->syntax->add_constant(type, cur_class);
		if (type == TypeInt)
			c->as_int() = *(int*)p;
		if (type == TypeFloat32)
			c->as_float() = *(float*)p;
		cur_func->default_parameters.resize(cur_func->num_params - 1);
		cur_func->default_parameters.add(add_node_const(c));
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
		msg_error("add_type_cast: " + string(cmd) + " not found");
		exit(1);
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
void SIAddPackageGl();
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
	SIAddPackageDoc();
	SIAddPackageHui(); // depends on doc
	SIAddPackageGl();
	SIAddPackageNet();
	SIAddPackageThread();
	SIAddPackageVulkan();

	add_package("base");
	SIAddXCommands();


	// consistency checks
	// -> now done by regression tests!
}

void clean_up() {
	delete_all_modules(true, true);

	packages.clear();

	reset_external_data();
}
};
