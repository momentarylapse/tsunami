/*----------------------------------------------------------------------------*\
| Kaba Lib                                                                     |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "../kaba.h"
#include "../template/template.h"
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

Package* cur_package = nullptr;
Module* cur_package_module = nullptr;


static Function *cur_func = nullptr;
static Class *cur_class;

Flags operator|(Flags a, Flags b) {
	return (Flags)((int)a | (int)b);
}

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
	Flags r = Flags::None;
	for (Flags ff: f)
		r = Flags(int(r) | int(ff));
	return r;
}


void add_internal_package(Context* c, const string& name, const string& version, Flags flags) {
	for (auto p: weak(c->internal_packages))
		if (p->name == name) {
			cur_package = p;
			cur_package_module = p->main_module.get();
			return;
		}

	// new
	shared p = new Package(name, version, Path::EMPTY);
	p->auto_import = flags_has(flags, Flags::AutoImport);
	shared s = c->create_empty_module(name);
	s->tree->base_class->name = name;
	p->main_module = s;
	c->internal_packages.add(p);
	cur_package = p.get();
	cur_package_module = s.get();
}

void __add_class__(Class *t, const Class *name_space) {
	if (name_space) {
		const_cast<Class*>(name_space)->classes.add(t);
		t->name_space = name_space;
	} else {
		cur_package_module->tree->base_class->classes.add(t);
		t->name_space = cur_package_module->tree->base_class;
	}
}

// class: alignment later determined by members
const Class *add_type(const string &name, int size, Flags flags, const Class *name_space) {
	Class *t = new Class(nullptr, name, size, 1, cur_package_module->tree.get());
	flags_set(t->flags, flags);
	__add_class__(t, name_space);
	return t;
}

const Class *add_type_simple(const string &name, int size, int alignment, Flags flags, const Class *name_space) {
	Class *t = new Class(nullptr, name, size, alignment, cur_package_module->tree.get());
	flags_set(t->flags, flags);
	__add_class__(t, name_space);
	return t;
}


const Class *add_class_template(const string &name, const Array<string>& params, TemplateClassInstantiator* instantiator) {
	auto t = cur_package_module->context->template_manager->create_class_template(cur_package_module->tree.get(), name, params, instantiator);
	__add_class__(t, nullptr);
	return t;
}

const Class *add_type_p_raw(const Class *sub_type) {
	//string name = format("%s[%s]", Identifier::RAW_POINTER, sub_type->name);
	string name = sub_type->name + "*";
	Class *t = new Class(common_types.raw_t, name, config.target.pointer_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	flags_set(t->flags, Flags::ForceCallByValue);
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(cur_package_module->tree.get(), t, common_types.raw_t, {sub_type});
	return t;
}

const Class *add_type_ref(const Class *sub_type) {
	string name = sub_type->name + "&";
	Class *t = new Class(common_types.reference_t, name, config.target.pointer_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	flags_set(t->flags, Flags::ForceCallByValue);
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(cur_package_module->tree.get(), t, common_types.reference_t, {sub_type});
	return t;
}

const Class *add_type_p_owned(const Class *sub_type) {
	string name = format("%s[%s]", Identifier::Owned, sub_type->name);
	Class *t = new Class(common_types.owned_t, name, config.target.pointer_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(cur_package_module->tree.get(), t, common_types.owned_t, {sub_type});
	return t;
}

const Class *add_type_p_shared(const Class *sub_type) {
	string name = format("%s[%s]", Identifier::Shared, sub_type->name);
	Class *t = new Class(common_types.shared_t, name, config.target.pointer_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.shared_t, {sub_type});
	return t;
}

const Class *add_type_p_shared_not_null(const Class *sub_type) {
	string name = format("%s![%s]", Identifier::Shared, sub_type->name);
	Class *t = new Class(common_types.shared_not_null_t, name, config.target.pointer_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.shared_not_null_t, {sub_type});
	return t;
}

const Class *add_type_p_xfer(const Class *sub_type) {
	string name = format("%s[%s]", Identifier::Xfer, sub_type->name);
	Class *t = new Class(common_types.xfer_t, name, config.target.pointer_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	flags_set(t->flags, Flags::ForceCallByValue);
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.xfer_t, {sub_type});
	return t;
}

// fixed array
const Class *add_type_array(const Class *sub_type, int array_length) {
	string name = sub_type->name + "[" + i2s(array_length) + "]";
	Class *t = new Class(common_types.array_t, name, sub_type->size * array_length, sub_type->alignment, cur_package_module->tree.get(), nullptr, {sub_type});
	t->array_length = array_length;
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.array_t, {sub_type}, array_length);
	return t;
}

// dynamic array
const Class *add_type_list(const Class *sub_type) {
	auto t = cur_package_module->context->template_manager->declare_new_class(cur_package_module->tree.get(), common_types.list_t, {sub_type}, 0, -1);
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.list_t, {sub_type});
	return t;
}

// dict
const Class *add_type_dict(const Class *sub_type) {
	string name = sub_type->name + "{}";
	Class *t = new Class(common_types.dict_t, name, config.target.dynamic_array_size, config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.dict_t, {sub_type});
	return t;
}

void capture_implicit_type(const Class *_t, const string &name) {
	auto t = const_cast<Class*>(_t);
	t->name = name;
	//__add_class__(t, t->param[0]->name_space);
}

// enum
const Class *add_type_enum(const string &name, const Class *_namespace) {
	Class *t = new Class(common_types.enum_t, name, sizeof(int), sizeof(int), cur_package_module->tree.get());
	flags_set(t->flags, Flags::ForceCallByValue);
	__add_class__(t, _namespace);
	return t;
}

	int _make_optional_size(const Class *t);

const Class *add_type_optional(const Class *sub_type) {
	string name = sub_type->name + "?";
	Class *t = new Class(common_types.optional_t, name, _make_optional_size(sub_type), sub_type->alignment, cur_package_module->tree.get(), nullptr, {sub_type});
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.optional_t, {sub_type});
	return t;
}

const Class *add_type_future(const Class *sub_type) {
	string name = "future[" + sub_type->name + "]";
	Class *t = new Class(common_types.future_t, name, sizeof(void*), config.target.pointer_size, cur_package_module->tree.get(), nullptr, {sub_type});
	__add_class__(t, sub_type->name_space);
	cur_package_module->context->template_manager->add_explicit_class_instance(
			cur_package_module->tree.get(),
			t, common_types.future_t, {sub_type});
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
		this->f = _f;
		this->fp = reinterpret_cast<t_func*>(_f->address);
	}
	void __init__(Function *_f) {
		new(this) KabaCallable<R(A...)>(_f);
	}

	R operator()(A... args) const override {
		if (this->fp) {
			return this->fp(args...);
		} else {
			throw EmptyCallableError();
		}
	}
};

string make_callable_signature(const Array<const Class*> &param, const Class *ret);

const Class *add_type_func(const Class *ret_type, const Array<const Class*> &params) {

	auto params_ret = params;
	if ((params.num == 1) and (params[0] == common_types._void))
		params_ret = {};
	params_ret.add(ret_type);


	string name = make_callable_signature(params, ret_type);

	auto ff = cur_package_module->context->template_manager->declare_new_class(cur_package_module->tree.get(), common_types.callable_fp_t, params_ret, 0, -1);
	__add_class__(ff, cur_package_module->tree->base_class);

	// simple register parameter?
	auto ptr_param = [] (const Class *p) {
		// ...kind of everything except float...
		return p->is_pointer_raw() or p->uses_call_by_reference() or (p == common_types._bool) or (p == common_types.i32);
	};

	add_class(ff);
	if (ret_type == common_types._void) {
		if (params.num == 0) {
			class_add_func(Identifier::func::Init, common_types._void, &KabaCallable<void()>::__init__);
				func_add_param("fp", common_types.pointer);
			class_add_func_virtual("call", common_types._void, &KabaCallable<void()>::operator());
		} else if (params.num == 1 and ptr_param(params[0])) {
			class_add_func(Identifier::func::Init, common_types._void, &KabaCallable<void(void*)>::__init__);
				func_add_param("fp", common_types.pointer);
			class_add_func_virtual("call", common_types._void, &KabaCallable<void(void*)>::operator());
				func_add_param("a", params[0]);
		} else if (params.num == 2 and ptr_param(params[0]) and ptr_param(params[1])) {
			class_add_func(Identifier::func::Init, common_types._void, &KabaCallable<void(void*,void*)>::__init__);
				func_add_param("fp", common_types.pointer);
			class_add_func_virtual("call", common_types._void, &KabaCallable<void(void*,void*)>::operator());
				func_add_param("a", params[0]);
				func_add_param("b", params[1]);
		} else {
			msg_error("NOT SURE HOW TO CREATE ..." + ff->long_name());
		}
	}
	auto pp = const_cast<Class*>(cur_package_module->tree->request_implicit_class_pointer(ff, -1));
	pp->name = name;
	return pp;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//


//   with type information

void add_operator_x(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func) {
	Operator *o = new Operator;
	o->owner = cur_package_module->tree.get();
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

	Flags flags = Flags::Mutable;
	if (!(o->abstract->flags & OperatorFlags::LeftIsModifiable))
		flags = Flags::Pure;

	//if (!c->uses_call_by_reference())
	if ((o->abstract->flags & OperatorFlags::LeftIsModifiable) and !c->uses_call_by_reference())
		flags_set(flags, Flags::Static);

	if (!flags_has(flags, Flags::Static)) {
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
	if (inline_index != InlineID::None and cur_package_module->filename.extension() == "")
		cur_package_module->context->global_operators.add(o);
	else if (primitive_op == OperatorID::Negative and param_type1 == common_types.f64)
		// FIXME quick hack...
		cur_package_module->context->global_operators.add(o);
	else
		delete o;
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
	cur_class->alignment = max(cur_class->alignment, type->alignment);
}

void class_derive_from(const Class *parent, DeriveFlags flags) {
	cur_class->derive_from(parent, flags);
}

void _class_add_member_func(const Class *ccc, Function *f, Flags flag) {
	Class *c = const_cast<Class*>(ccc);
	if (flags_has(flag, Flags::Override)) {
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
	cur_package_module->tree->functions.add(f);
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
	if (!func)
		return 0;
	if ((config.native_target.abi == Abi::X86_WINDOWS) or (config.native_target.abi == Abi::AMD64_WINDOWS)) {
		unsigned char* pp = (unsigned char*)func;

		if ((pp[0] == 0x48) and (pp[1] == 0x8b) and (pp[2] == 0x01) and (pp[3] == 0xff) and (pp[4] == 0x60)) {
			// amd64 small
			// 48.8b.01    ff.60.NN
			// virtual function
			return (unsigned int)pp[5] / 8;
		}
		if ((pp[0] == 0x48) and (pp[1] == 0x8b) and (pp[2] == 0x01) and (pp[3] == 0xff) and (pp[4] == 0xa0)) {
			// amd64 big
			// 48.8b.01    ff.a0.NN.NN.NN.NN
			// virtual function
			return *(int*)&pp[5] / 8;
		}
		if ((pp[0] == 0x8b) and (pp[1] == 0x44) and (pp[2] == 0x24) and (pp[4] == 0x8b) and (pp[5] == 0x00) and (pp[6] == 0xff) and (pp[7] == 0x60)) {
			// 8b.44.24.**    8b.00     ff.60.10
			// virtual function
			return (unsigned int)pp[8] / 4;
		}
		if (pp[0] == 0xe9) {
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
				return *(int*)&pp[5] / 8;
			}
		}
		msg_error("class_add_func_virtual(" + tname + "." + name + "):  can't read virtual index");
		msg_write(p2s(pp));
		msg_write(Asm::disassemble(func, 16));
		exit(1);
	} else if (config.native_target.abi == Abi::AMD64_WINDOWS) {
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
	Constant *c = cur_package_module->tree->add_constant(type, cur_class);
	c->name = name;

	// enums can't be referenced...
	if (type == common_types.i32 or type->is_enum())
		c->as_int64() = (int_p)value;
		//*(const void**)c->p() = value;
	else if (type == common_types.string)
		c->as_string() = *(const string*)value;
	else if (value)
		memcpy(c->p(), value, type->size);
}

void add_const(const string &name, const Class *type, const void *value) {
	cur_class = cur_package_module->tree->base_class;
	class_add_const(name, type, value);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//


void add_ext_var(const string &name, const Class *type, void *var) {
	auto *v = new Variable(name, type);
	flags_set(v->flags, Flags::Extern); // prevent initialization when importing
	cur_package_module->tree->base_class->static_variables.add(v);
	if (config.allow_std_lib)
		v->memory = var;
};

//------------------------------------------------------------------------------------------------//
//                                      compiler functions                                        //
//------------------------------------------------------------------------------------------------//



Function *add_func_x(const string &name, const Class *return_type, void *func, Flags flag) {
	add_class(cur_package_module->base_class());
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
		cur_func->abstract_node->params[2]->params.resize(cur_func->num_params*3);
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
		cur_func->abstract_node->params[2]->params.resize(cur_func->num_params*3);

		Constant *c = cur_package_module->tree->add_constant(type, cur_class);
		if (type == common_types.i32)
			c->as_int() = *(int*)p;
		if (type == common_types.f32)
			c->as_float() = *(float*)p;
		cur_func->abstract_node->params[2]->set_param(cur_func->num_params*3-1, add_node_const(c));
	}
}


void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd) {
	TypeCast c;
	c.penalty = penalty;
	c.f = nullptr;
	for (auto *f: cur_package_module->tree->functions)
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
	cur_package_module->context->type_casts.add(c);
}


void SIAddStatements();

void SIAddXCommands(Context *c);
void SIAddPackageBase(Context *c);
void SIAddPackageKaba(Context *c);
void SIAddPackageAsync(Context *c);
void SIAddPackageTime(Context *c);
void SIAddPackageOS(Context *c);
void SIAddPackageOSPath(Context *c);
void SIAddPackageMath(Context *c);
void SIAddPackageThread(Context *c);
void SIAddPackageImage(Context *c);


void init_lib(Context *c) {


	SIAddPackageBase(c);
	SIAddPackageOSPath(c);
	SIAddPackageAsync(c); // depends on os.Path
	SIAddPackageKaba(c);
	SIAddPackageMath(c);
	SIAddPackageTime(c);
	SIAddPackageOS(c);
	SIAddPackageImage(c);
	SIAddPackageThread(c);

	add_internal_package(c, "base", "(ignored...)");
	SIAddXCommands(c);


	// consistency checks
	// -> now done by regression tests!
}


Context *_secret_lib_context_ = nullptr;

void init(Abi abi, bool allow_std_lib) {
	config.native_target = CompilerConfiguration::Target::get_native();
	if (abi == Abi::NATIVE) {
		config.target = config.native_target;
	} else {
		config.target = CompilerConfiguration::Target::get_for_abi(abi);
	}
	Asm::init(config.target.instruction_set);
	config.allow_std_lib = allow_std_lib;

	SIAddStatements();


	_secret_lib_context_ = new Context;
	init_lib(_secret_lib_context_);

	default_context = Context::create();
}

extern base::set<Class*> _all_classes_;
extern base::set<Module*> _all_modules_;

void clean_up() {
	default_context->clean_up();
	_secret_lib_context_->clean_up();

	for (auto s: Statements)
		delete s;
	for (auto s: special_functions)
		delete s;

	if (_all_modules_.num > 0) {
		msg_error(format("remaining modules: %d", _all_modules_.num));
		for (auto m: _all_modules_)
			msg_write("  " + str(m->filename) + "  " + str(m->_pointer_ref_counter));
	} else if (_all_classes_.num > 0) {
		msg_error(format("remaining classes: %d", _all_classes_.num));
		for (auto c: _all_classes_)
			msg_write("  " + c->name + "  " + str(c->_pointer_ref_counter));
	}
}

};
