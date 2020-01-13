#include "../../base/base.h"
#include "../kaba.h"
#include "../../file/file.h"
#include "Class.h"

namespace Kaba{

extern const Class* TypeObject;
extern const Class* TypeEmptyList;

ClassElement::ClassElement() {
	offset = 0;
	type = nullptr;
}

ClassElement::ClassElement(const string &_name, const Class *_type, int _offset) {
	name = _name;
	offset = _offset;
	type = _type;
}

string ClassElement::signature(bool include_class) const {
	if (include_class)
		return type->name + " " + name;
	return type->name + " " + name;
}

bool ClassElement::hidden() const {
	return (name[0] == '_') or (name[0] == '-');
}


bool type_match(const Class *given, const Class *wanted) {
	// exact match?
	if (given == wanted)
		return true;

	// allow any pointer?
	if ((given->is_pointer()) and (wanted == TypePointer))
		return true;

	// FIXME... quick'n'dirty hack to allow nil as parameter
	if ((given == TypePointer) and wanted->is_pointer() and !wanted->is_pointer_silent())
		return true;

	// compatible pointers (of same or derived class)
	if (given->is_pointer() and wanted->is_pointer())
		return given->param->is_derived_from(wanted->param);

	if (wanted->is_super_array()) {
		if (given == TypeEmptyList)
			return true;
		if (given->is_super_array()) {
			if (type_match(given->param, wanted->param) and (given->param->size == wanted->param->size))
				return true;
		}
	}

	return given->is_derived_from(wanted);
}


// allow same classes... TODO deprecate...
bool _type_match(const Class *given, bool same_chunk, const Class *wanted) {
	if ((same_chunk) and (wanted == TypeChunk))
		return true;

	return type_match(given, wanted);
}

Class::Class(const string &_name, int _size, SyntaxTree *_owner, const Class *_parent, const Class *_param) {
	name = _name;
	owner = _owner;
	size = _size;
	type = Type::OTHER;
	array_length = 0;
	parent = _parent;
	param = _param;
	name_space = nullptr;
	force_call_by_value = false;
	fully_parsed = true;
	_amd64_allow_pass_in_xmm = false;
	_logical_line_no = _exp_no = -1;
	_vtable_location_target_ = nullptr;
	_vtable_location_compiler_ = nullptr;
	_vtable_location_external_ = nullptr;
};

Class::~Class() {
	for (auto *c: constants)
		delete c;
	for (auto *v: static_variables)
		delete v;
	for (auto *f: functions)
		if (f->name_space == this)
			delete f;
	for (auto *c: classes)
		if (c->owner == owner)
			delete c;
}



string namespacify(const string &name, const Class *name_space) {
	if (name_space)
		if (name_space->name[0] != '-')
			return namespacify(name_space->name + "." + name, name_space->name_space);
	return name;
}

string Class::long_name() const {
	return namespacify(name, name_space);
}

bool Class::is_array() const
{ return type == Type::ARRAY; }

bool Class::is_super_array() const
{ return type == Type::SUPER_ARRAY; }

bool Class::is_pointer() const
{ return type == Type::POINTER or type == Type::POINTER_SILENT; }

bool Class::is_pointer_silent() const
{ return type == Type::POINTER_SILENT; }

bool Class::is_dict() const
{ return type == Type::DICT; }

bool Class::uses_call_by_reference() const {
	return (!force_call_by_value and !is_pointer()) or is_array();
}

bool Class::uses_return_by_memory() const {
	if (_amd64_allow_pass_in_xmm)
		return false;
	return (!force_call_by_value and !is_pointer()) or is_array();
}



bool Class::is_simple_class() const {
	if (!uses_call_by_reference())
		return true;
	/*if (is_array)
		return false;*/
	if (is_super_array())
		return false;
	if (is_dict())
		return false;
	if (vtable.num > 0)
		return false;
	if (parent)
		if (!parent->is_simple_class())
			return false;
	if (get_constructors().num > 0)
		return false;
	if (get_destructor())
		return false;
	if (get_assign())
		return false;
	for (ClassElement &e: elements)
		if (!e.type->is_simple_class())
			return false;
	return true;
}

bool Class::usable_as_super_array() const {
	if (is_super_array())
		return true;
	if (is_array() or is_dict() or is_pointer())
		return false;
	if (parent)
		return parent->usable_as_super_array();
	return false;
}

const Class *Class::get_array_element() const {
	if (is_array() or is_super_array() or is_dict())
		return param;
	if (is_pointer())
		return nullptr;
	if (parent)
		return parent->get_array_element();
	return nullptr;
}

bool Class::needs_constructor() const {
	if (!uses_call_by_reference()) // int/float/pointer etc
		return false;
	if (is_super_array() or is_dict())
		return true;
	if (is_array())
		return param->needs_constructor();
	if (vtable.num > 0)
		return true;
	if (parent)
		if (parent->needs_constructor())
			return true;
	for (ClassElement &e: elements)
		if (e.type->needs_constructor())
			return true;
	return false;
}

bool Class::is_size_known() const {
	if (!fully_parsed)
		return false;
	if (is_super_array() or is_dict() or is_pointer())
		return true;
	for (ClassElement &e: elements)
		if (!e.type->is_size_known())
			return false;
	return true;
}

bool Class::needs_destructor() const {
	if (!uses_call_by_reference())
		return false;
	if (is_super_array() or is_dict())
		return true;
	if (is_array())
		return param->get_destructor();
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
	if (name == root)
		return true;
	/*if (is_super_array() or is_array() or is_dict() or is_pointer())
		return false;*/
	if (!parent)
		return false;
	return parent->is_derived_from_s(root);
}

// don't care if static
Function *Class::get_func(const string &_name, const Class *return_type, const Array<const Class*> &params) const {
	for (auto *f: functions)
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

Function *Class::get_same_func(const string &_name, Function *ff) const {
	return get_func(_name, ff->literal_return_type, ff->literal_param_type);
}

Function *Class::get_default_constructor() const {
	return get_func(IDENTIFIER_FUNC_INIT, TypeVoid, {});
}

Array<Function*> Class::get_constructors() const {
	Array<Function*> c;
	for (auto *f: functions)
		if ((f->name == IDENTIFIER_FUNC_INIT) and (f->literal_return_type == TypeVoid))
			c.add(f);
	return c;
}

Function *Class::get_destructor() const {
	return get_func(IDENTIFIER_FUNC_DELETE, TypeVoid, {});
}

Function *Class::get_assign() const {
	return get_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, {this});
}

Function *Class::get_get(const Class *index) const {
	for (Function *cf: functions) {
		if (cf->name != "__get__")
			continue;
		if (cf->num_params != 1)
			continue;
		if (cf->literal_param_type[0] != index)
			continue;
		return cf;
	}
	return nullptr;
}

Function *Class::get_virtual_function(int virtual_index) const {
	for (Function *f: functions)
		if (f->virtual_index == virtual_index)
			return f;
	return nullptr;
}

void Class::link_virtual_table() {
	if (vtable.num == 0)
		return;

	//msg_write("link vtable " + name);
	// derive from parent
	if (parent)
		for (int i=0; i<parent->vtable.num; i++)
			vtable[i] = parent->vtable[i];
	if (config.abi == Abi::WINDOWS_32)
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);

	// link virtual functions into vtable
	for (Function *cf: functions) {
		if (cf->virtual_index >= 0) {
			//msg_write(i2s(cf->virtual_index) + ": " + cf->signature());
			if (cf->virtual_index >= vtable.num)
				owner->do_error("LinkVirtualTable");
				//vtable.resize(cf.virtual_index + 1);
			if (config.verbose)
				msg_write("VIRTUAL   " + i2s(cf->virtual_index) + "   " + cf->signature());
			vtable[cf->virtual_index] = cf->address;
		}
		if (cf->needs_overriding) {
			msg_error("needs overriding: " + cf->signature());
		}
	}
}

void Class::link_external_virtual_table(void *p) {
	// link script functions according to external vtable
	VirtualTable *t = (VirtualTable*)p;
	vtable.clear();
	int max_vindex = 1;
	for (Function *cf: functions)
		if (cf->virtual_index >= 0) {
			cf->address = t[cf->virtual_index];
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
	if ((config.abi == Abi::WINDOWS_32) or (config.abi == Abi::WINDOWS_64))
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);
}


bool class_func_match(Function *a, Function *b) {
	if (a->name != b->name)
		return false;
	if (a->literal_return_type != b->literal_return_type)
		return false;
	if (a->num_params != b->num_params)
		return false;
	for (int i=0; i<a->num_params; i++)
		if (!type_match(b->literal_param_type[i], a->literal_param_type[i]))
			return false;
	return true;
}


const Class *Class::get_pointer() const {
	return owner->make_class(name + "*", Class::Type::POINTER, config.pointer_size, 0, nullptr, this, name_space);
}

const Class *Class::get_root() const {
	const Class *r = this;
	while (r->parent)
		r = r->parent;
	return r;
}

void Class::add_function(SyntaxTree *s, Function *f, bool as_virtual, bool override) {
	if (config.verbose)
		msg_write("CLASS ADD   " + long_name() + "    " + f->signature());
	if (f->is_static) {
		if (config.verbose)
			msg_write("   STATIC");
		functions.add(f);
	} else {
		if (config.verbose)
			msg_write("   MEMBER");
		if (as_virtual and (f->virtual_index < 0)) {
			if (config.verbose)
				msg_write("VVVVV +");
			f->virtual_index = process_class_offset(name, f->name, max(vtable.num, 2));
		}

		// override?
		Function *orig = nullptr;
		int orig_index = -1;
		foreachi (Function *ocf, functions, i)
			if (class_func_match(f, ocf)) {
				orig = ocf;
				orig_index = i;
			}
		if (override and !orig)
			s->do_error(format("can not override function %s, no previous definition", f->signature().c_str()), f->_exp_no, f->_logical_line_no);
		if (!override and orig) {
			msg_write(f->signature());
			msg_write(orig->signature());
			s->do_error(format("function %s is already defined, use '%s'", f->signature().c_str(), IDENTIFIER_OVERRIDE.c_str()), f->_exp_no, f->_logical_line_no);
		}
		if (override) {
			if (config.verbose)
				msg_write("OVERRIDE    " + orig->signature());
			f->virtual_index = orig->virtual_index;
			if (orig->name_space == this)
				delete orig;
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

void Class::derive_from(const Class* root, bool increase_size) {
	if (config.verbose)
		msg_write("DERIVE  " + long_name() + " from " + root->long_name());
	parent = const_cast<Class*>(root);
	param = root->param;

	// inheritance of elements
	elements = parent->elements;

	// inheritance of functions
	for (Function *f: parent->functions) {
		if (f->name == IDENTIFIER_FUNC_ASSIGN)
			continue;
		Function *ff = f;
		if ((f->name == IDENTIFIER_FUNC_INIT) or (f->name == IDENTIFIER_FUNC_DELETE)) {
			ff = f->create_dummy_clone(this);
		} else if (f->name == IDENTIFIER_FUNC_SUBARRAY) {
			ff = f->create_dummy_clone(this);
			ff->_label = f->_label;
			ff->address = f->address;
			//ff->literal_return_type = this;
			//ff->return_type = this;
		}
		if (config.verbose)
			msg_write("INHERIT   " + ff->signature());
		functions.add(ff);
	}

	if (increase_size)
		size += parent->size;
	vtable = parent->vtable;
	_vtable_location_compiler_ = vtable.data;
	_vtable_location_target_ = vtable.data;
}

void *Class::create_instance() const {
	void *p = malloc(size);
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

