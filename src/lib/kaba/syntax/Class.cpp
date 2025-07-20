#include "../../base/base.h"
#include "../../base/iter.h"
#include "../../base/set.h"
#include "../kaba.h"
#include "../../os/msg.h"
#include "Class.h"

namespace kaba {

void remove_enum_labels(const Class *type);

base::set<Class*> _all_classes_;

ClassElement::ClassElement() {
	offset = 0;
	type = nullptr;
	allow_indirect_use = false;
}

ClassElement::ClassElement(const string &_name, const Class *_type, int64 _offset) {
	name = _name;
	offset = _offset;
	type = _type;
	allow_indirect_use = false;
}

string ClassElement::signature(bool include_class) const {
	if (include_class)
		return type->name + " " + name;
	return type->name + " " + name;
}

string ClassElement::str() const {
	return format("(%d, %s, %s)", offset, name.repr(), type->long_name());
}

bool ClassElement::hidden() const {
	return (name[0] == '_') or (name[0] == '-');
}

bool type_match_up(const Class *given, const Class *wanted);



Class::Class(const Class* _from_template, const string &_name, int64 _size, int _alignment, SyntaxTree *_owner, const Class *_parent, const Array<const Class*> &_param) {
	flags = Flags::FullyParsed;
	name = _name;
	owner = _owner;
	size = _size;
	alignment = _alignment;
	from_template = _from_template;
	array_length = 0;
	parent = _parent;
	param = _param;
	name_space = nullptr;
	// force_call_by_value = false;
	// fully_parsed = true;
	// _amd64_allow_pass_in_xmm = false;
	token_id = -1;
	_vtable_location_target_ = nullptr;
	_vtable_location_compiler_ = nullptr;
	_vtable_location_external_ = nullptr;
	_all_classes_.add(this);
};

Class::~Class() {
	remove_enum_labels(this);

	_all_classes_.erase(this);
}

bool Class::force_call_by_value() const {
	return flags_has(flags, Flags::ForceCallByValue);
}

bool Class::fully_parsed() const {
	return flags_has(flags, Flags::FullyParsed);
}

bool Class::_return_in_float_registers() const {
	return flags_has(flags, Flags::ReturnInFloatRegisters);
}

bool Class::is_template() const {
	return flags_has(flags, Flags::Template);
}

bool reachable_from(const Class *ns, const Class *observer_ns) {
	if (ns == observer_ns)
		return true;
	if (observer_ns->name_space)
		return reachable_from(ns, observer_ns->name_space);
	return false;
}

bool ns_needed(const Class *ns, const Class *observer_ns) {
	if (!ns)
		return false;
	if (ns->name[0] == '-')
		return false;
	if (observer_ns and reachable_from(ns, observer_ns))
		return false;
	if (ns == ns->owner->module->context->internal_packages[0]->base_class()) // always ignore "base"
		if (ns == ns->owner->base_class)
			return false;
	return true;
}

string namespacify_rel(const string &name, const Class *name_space, const Class *observer_ns) {
	if (ns_needed(name_space, observer_ns))
		return namespacify_rel(name_space->name + "." + name, name_space->name_space, observer_ns);
	return name;
}

string Class::long_name() const {
	return namespacify_rel(name, name_space, nullptr);
}

string Class::cname(const Class *ns) const {
	return namespacify_rel(name, name_space, ns);
}

bool Class::is_regular() const {
	return from_template == nullptr;
}

bool Class::is_struct() const {
	return from_template == TypeStructT;
}

bool Class::is_array() const {
	return from_template == TypeArrayT;
}

bool Class::is_list() const {
	return from_template == TypeListT;
}

bool Class::is_some_pointer() const {
	return is_pointer_raw()
			or is_reference()
			or is_pointer_shared()
			or is_pointer_shared_not_null()
			or is_pointer_owned()
			or is_pointer_owned_not_null()
			or is_pointer_xfer_not_null()
			or is_pointer_alias();
}

bool Class::is_some_pointer_not_null() const {
	return is_pointer_shared_not_null()
			or is_pointer_owned_not_null()
			or is_reference()
			or is_pointer_alias();
}

bool Class::is_pointer_raw() const {
	return from_template == TypeRawT;
}

bool Class::is_pointer_shared() const {
	return from_template == TypeSharedT;
}

bool Class::is_pointer_shared_not_null() const {
	return from_template == TypeSharedNotNullT;
}

bool Class::is_pointer_owned() const {
	return from_template == TypeOwnedT;
}

bool Class::is_pointer_owned_not_null() const {
	return from_template == TypeOwnedNotNullT;
}

bool Class::is_pointer_xfer_not_null() const {
	return from_template == TypeXferT;
}

bool Class::is_pointer_alias() const {
	return from_template == TypeAliasT;
}

bool Class::is_reference() const {
	return from_template == TypeReferenceT;
}

bool Class::is_enum() const {
	return from_template == TypeEnumT;
}

bool Class::is_namespace() const {
	return from_template == TypeNamespaceT;
}

bool Class::is_interface() const {
	return from_template == TypeInterfaceT;
}

bool Class::is_dict() const {
	return from_template == TypeDictT;
}

bool Class::is_product() const {
	return from_template == TypeProductT;
}

bool Class::is_optional() const {
	return from_template == TypeOptionalT;
}

bool Class::is_callable() const {
	if (is_pointer_raw())
		return param[0]->is_callable_fp() or param[0]->is_callable_bind();
	return false;
}

bool Class::is_callable_fp() const {
	return from_template == TypeCallableFPT;
}

bool Class::is_callable_bind() const {
	return from_template == TypeCallableBindT;
}

bool Class::uses_call_by_reference() const {
	return (!force_call_by_value() and !is_pointer_raw() and !is_reference() and !is_pointer_alias()) or is_array() or is_optional();
}

bool Class::uses_return_by_memory() const {
	if (_return_in_float_registers())
		return false;
	return (!force_call_by_value() and !is_pointer_raw() and !is_reference() and !is_pointer_alias()) or is_array() or is_optional();
}


// is just a bag of plain-old-data?
//   -> can be assigned as a chunk
bool Class::can_memcpy() const {
	if (!uses_call_by_reference())
		return true;
	if (is_array() or is_optional())
		return param[0]->can_memcpy();
	if (is_list())
		return false;
	if (is_dict())
		return false;
	if (vtable.num > 0)
		return false;
	if (parent)
		if (!parent->can_memcpy())
			return false;
	if (get_constructors().num > 0)
		return false;
	if (get_destructor())
		return false;
	//if (get_assign())
	//	return false;
	for (ClassElement &e: elements)
		if (!e.type->can_memcpy())
			return false;
	return true;
}

bool Class::usable_as_list() const {
	if (is_list())
		return true;
	if (is_array() or is_dict() or is_pointer_raw())
		return false;
	if (parent)
		return parent->usable_as_list();
	return false;
}

const Class *Class::get_array_element() const {
	if (is_array() or is_list() or is_dict())
		return param[0];
	if (is_pointer_raw())
		return nullptr;
	if (parent)
		return parent->get_array_element();
	return nullptr;
}

// hmmm, very vague concept...
bool Class::needs_constructor() const {
	if (!uses_call_by_reference()) // int/float/pointer etc
		return false;
	if (is_list() or is_dict() or is_optional())
		return true;
	if (initializers.num > 0)
		return true;
	if (is_array())
		return param[0]->needs_constructor();
	if (vtable.num > 0)
		return true;
	if (is_product())
		for (auto p: param)
			if (p->get_default_constructor())
				return true;
	if (parent)
		if (parent->needs_constructor())
			return true;
	for (ClassElement &e: elements)
		if (e.type->needs_constructor() or e.type->get_default_constructor())
			return true;
	return false;
}

bool Class::is_size_known() const {
	if (is_list() or is_dict() or is_some_pointer() or is_enum())
		return true;
	if (!fully_parsed())
		return false;
	if (is_optional())
		return param[0]->is_size_known();
	for (ClassElement &e: elements)
		if (!e.type->is_size_known())
			return false;
	return true;
}

bool Class::needs_destructor() const {
	if (!uses_call_by_reference())
		return false;
	if (is_list() or is_dict() or is_optional())
		return true;
	if (is_array())
		return param[0]->needs_destructor();
	if (parent) {
		if (parent->get_destructor())
			return true;
		if (parent->needs_destructor())
			return true;
	}
	for (ClassElement &e: elements) {
		if (e.type->get_destructor())
			return true;
		if (e.type->needs_destructor())
			return true;
	}
	return false;
}

bool Class::is_derived_from(const Class *root) const {
	if (this == root)
		return true;
	/*if (is_super_array() or is_array() or is_dict() or is_pointer())
		return false;*/  // since parent/param split
	if (!parent)
		return false;
	return parent->is_derived_from(root);
}

bool Class::is_derived_from_s(const string &root) const {
	if (long_name().match(root))
		return true;
	/*if (is_super_array() or is_array() or is_dict() or is_pointer())
		return false;*/
	if (!parent)
		return false;
	return parent->is_derived_from_s(root);
}

// don't care if static
Function *Class::get_func(const string &_name, const Class *return_type, const Array<const Class*> &params) const {
	for (auto *f: weak(functions))
		if ((f->name == _name) and (f->literal_return_type == return_type) and (f->num_params == params.num)) {
			bool match = true;
			for (int i=0; i<params.num; i++) {
				if (params[i] and (f->literal_param_type[i] != params[i])) {
					match = false;
					break;
				}
			}
			if (match)
				return f;
		}
	return nullptr;
}

Function *Class::get_member_func(const string &_name, const Class *return_type, const Array<const Class*> &params) const {
	for (auto *f: weak(functions))
		if (f->is_member() and (f->name == _name) and (f->literal_return_type == return_type) and (f->num_params == params.num+1)) {
			bool match = true;
			for (int i=0; i<params.num; i++) {
				if (params[i] and (f->literal_param_type[i+1] != params[i])) {
					match = false;
					break;
				}
			}
			if (match)
				return f;
		}
	return nullptr;
}

Function *Class::get_same_func(const string &_name, Function *ff) const {
	auto param = ff->literal_param_type;
	param[0] = nullptr; // ignore self
	return get_func(_name, ff->literal_return_type, param);
}

Function *Class::get_default_constructor() const {
	return get_func(Identifier::func::Init, TypeVoid, {nullptr});
}

Array<Function*> Class::get_constructors() const {
	Array<Function*> c;
	for (auto *f: weak(functions))
		if ((f->name == Identifier::func::Init) and (f->literal_return_type == TypeVoid))
			c.add(f);
	return c;
}

Function *Class::get_destructor() const {
	return get_func(Identifier::func::Delete, TypeVoid, {nullptr});
}

Function *Class::get_assign() const {
	return get_func(Identifier::func::Assign, TypeVoid, {nullptr, this});
}

Function *Class::get_get(const Class *index) const {
	for (auto *cf: weak(functions)) {
		if (cf->name != Identifier::func::Get)
			continue;
		if (cf->is_static())
			continue;
		if (cf->num_params != 2)
			continue;
		if (cf->literal_param_type[1] != index)
			continue;
		return cf;
	}
	return nullptr;
}

Function *Class::get_call() const {
	for (auto *cf: weak(functions)) {
		if (cf->name == Identifier::func::Call)
			return cf;
	}
	return nullptr;
}

Function *Class::get_virtual_function(int virtual_index) const {
	for (auto *f: weak(functions))
		if (f->virtual_index == virtual_index)
			return f;
	return nullptr;
}

void Class::link_virtual_table() {
	if (vtable.num == 0)
		return;

	//msg_write("link vtable " + long_name());
	// derive from parent
	if (parent)
		for (int i=0; i<parent->vtable.num; i++)
			vtable[i] = parent->vtable[i];
	if ((config.target.abi == Abi::X86_WINDOWS) or (config.target.abi == Abi::AMD64_WINDOWS))
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);

	// link virtual functions into vtable
	for (auto *cf: weak(functions)) {
		if (cf->virtual_index >= 0) {
			//msg_write(i2s(cf->virtual_index) + ": " + cf->signature());
			if (cf->virtual_index >= vtable.num)
				owner->do_error("LinkVirtualTable");
				//vtable.resize(cf.virtual_index + 1);
			if (config.verbose)
				msg_write("VIRTUAL   " + i2s(cf->virtual_index) + "   " + cf->signature());
			vtable[cf->virtual_index] = (void*)cf->address;
		}
		if (cf->needs_overriding()) {
			msg_error("needs overriding: " + cf->signature());
		}
	}
}

void Class::link_external_virtual_table(void *p) {
	// link module functions according to external vtable
	VirtualTable *t = (VirtualTable*)p;
	vtable.clear();
	int max_vindex = 1;
	for (auto *cf: weak(functions))
		if (cf->virtual_index >= 0) {
			cf->address = (int_p)t[cf->virtual_index];
			if (cf->virtual_index >= vtable.num)
				max_vindex = max(max_vindex, cf->virtual_index);
		}
	vtable.resize(max_vindex + 1);
	_vtable_location_compiler_ = vtable.data;
	_vtable_location_target_ = vtable.data;
	_vtable_location_external_ = (void*)t;

	for (int i=0; i<vtable.num; i++)
		vtable[i] = t[i];
	// this should also link the "real" c++ destructor
	if ((config.target.abi == Abi::X86_WINDOWS) or (config.target.abi == Abi::AMD64_WINDOWS))
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);
}


bool member_func_override_match(Function *a, Function *b) {
	if (a->name != b->name)
		return false;
	if (a->literal_return_type != b->literal_return_type)
		return false;
	if (a->num_params != b->num_params)
		return false;
	for (int i=1; i<a->num_params; i++)
		if (!type_match_up(b->literal_param_type[i], a->literal_param_type[i]))
			return false;
	return true;
}

// only used to create "implicit" types
// TODO: find a better system
string class_name_might_need_parantheses(const Class *t) {
	if (t->is_callable() /*or t->is_product()*/)
		return "(" + t->long_name() + ")";
	return t->name;
	//return t->long_name();
}

const Class *Class::get_root() const {
	const Class *r = this;
	while (r->parent)
		r = r->parent;
	return r;
}

void Class::add_template_function(SyntaxTree *s, Function *f, bool as_virtual, bool override) {
	if (config.verbose)
		msg_write("CLASS ADD TEMPLATE   " + long_name() + "    " + f->signature());
	if (f->is_static()) {
		if (config.verbose)
			msg_write("   STATIC");
		functions.add(f);
	} else {
		// too lazy for checking...
		functions.add(f);
	}
}

// TODO: split!
void Class::add_function(SyntaxTree *s, Function *f, bool as_virtual, bool override) {
	if (config.verbose)
		msg_write("CLASS ADD   " + long_name() + "    " + f->signature());
	if (f->is_static()) {
		if (config.verbose)
			msg_write("   STATIC");
		functions.add(f);
	} else {
		if (config.verbose)
			msg_write("   MEMBER");
		if (as_virtual and (f->virtual_index < 0)) {
			if (config.verbose)
				msg_write("VVVVV +");
			f->virtual_index = s->module->context->external->process_class_offset(cname(owner->base_class), f->name, max(vtable.num, 2));
			if ((f->name == Identifier::func::Delete) and (config.target.abi == Abi::AMD64_WINDOWS or config.target.abi == Abi::X86_WINDOWS))
				f->virtual_index = 1;
		}

		// override?
		Function *orig = nullptr;
		int orig_index = -1;
		for (auto&& [i,ocf]: enumerate(weak(functions)))
			if (member_func_override_match(f, ocf)) {
				orig = ocf;
				orig_index = i;
			}
		if (override and !orig)
			s->do_error(format("can not override function %s, no previous definition", f->signature()), f->token_id);
		if (!override and orig) {
			msg_write(f->signature());
			msg_write(orig->signature());
			s->do_error(format("function %s is already defined, use '%s'", f->signature(), Identifier::Override), f->token_id);
		}
		if (override) {
			if (config.verbose)
				msg_write("OVERRIDE    " + orig->signature());
			f->virtual_index = orig->virtual_index;
			//f->flags = orig->flags;
			// don't copy __INIT_FILL_ALL_PARAMS etc...
			// better copy one-by-one for now
			if (flags_has(orig->flags, Flags::Mutable))
				flags_set(f->flags, Flags::Mutable);
			else
				flags_clear(f->flags, Flags::Mutable);
			if (flags_has(orig->flags, Flags::Ref))
				flags_set(f->flags, Flags::Ref);

			if (auto self = f->__get_var(Identifier::Self)) {
				if (flags_has(f->flags, Flags::Mutable))
					flags_set(self->flags, Flags::Mutable);
				else
					flags_clear(self->flags, Flags::Mutable);
			}
			functions[orig_index] = f;
		} else {
			functions.add(f);
		}

		if (f->virtual_index >= 0) {
			if (config.verbose) {
				msg_write("VVVVV");
				msg_write(f->virtual_index);
			}
			if (vtable.num <= f->virtual_index)
				vtable.resize(f->virtual_index + 1);
			_vtable_location_compiler_ = vtable.data;
			_vtable_location_target_ = vtable.data;
		}
	}
}

DeriveFlags operator|(DeriveFlags a, DeriveFlags b) {
	return (DeriveFlags)((int)a | (int)b);
}

int operator&(DeriveFlags a, DeriveFlags b) {
	return ((int)a & (int)b);
}

void Class::derive_from(const Class* root, DeriveFlags derive_flags) {
	if (config.verbose)
		msg_write("DERIVE  " + long_name() + " from " + root->long_name());
	parent = const_cast<Class*>(root);
	param = root->param;

	// inheritance of elements
	elements = parent->elements;

	// inheritance of functions
	for (auto *f: weak(parent->functions)) {
		if (f->name == Identifier::func::Assign)
			continue;
		Function *ff = f;
		if (f->name == Identifier::func::Init) {
			if (!(derive_flags & DeriveFlags::KEEP_CONSTRUCTORS))
				continue;
			ff = f->create_dummy_clone(this);
		} else if (f->name == Identifier::func::Delete) {
			ff = f->create_dummy_clone(this);
		} else if (f->name == Identifier::func::Subarray) {
			ff = f->create_dummy_clone(this);
			ff->_label = f->_label;
			ff->address = f->address;

			// leave it for now
			//   kaba_make_super_array() is looking for DynamicArray as a return type
			// nahhh, let's do it here
			ff->literal_return_type = this;
			ff->effective_return_type = this;
		}
		if (config.verbose)
			msg_write("INHERIT   " + ff->signature());
		functions.add(ff);
	}

	if (derive_flags & DeriveFlags::SET_SIZE)
		size = parent->size;

	//if (derive_flags & DeriveFlags::COPY_VTABLE)
	vtable = parent->vtable;
	_vtable_location_compiler_ = vtable.data;
	_vtable_location_target_ = vtable.data;
}

void *Class::create_instance() const {
	void *p = malloc(size);
	memset(p, 0, size);
	Function *c = get_default_constructor();
	if (c) {
		typedef void con_func(void *);
		con_func * f = (con_func*)c->address;
		if (f)
			f(p);
	}
	return p;
}

}

