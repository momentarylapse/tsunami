#include "../../base/base.h"
#include "../kaba.h"
#include "../../file/file.h"
#include "class.h"

namespace Kaba{

ClassElement::ClassElement()
{
	hidden = false;
	offset = 0;
	type = nullptr;
}

string ClassElement::signature(bool include_class) const
{
	if (include_class)
		return type->name + " " + name;
	return type->name + " " + name;
}


ClassFunction::ClassFunction()
{
	nr = -1;
	virtual_index = -1;
	needs_overriding = false;
	return_type = nullptr;
	script = nullptr;
}

ClassFunction::ClassFunction(const string &_name, Class *_return_type, Script *s, int no)
{
	name = _name;
	return_type = _return_type;
	script = s;
	nr = no;
	virtual_index = -1;
	needs_overriding = false;
}

Function* ClassFunction::func() const
{
	return script->syntax->functions[nr];
}

string ClassFunction::signature(bool include_class) const
{
	Function* f = func();
	if (needs_overriding)
		return f->signature(include_class) + " [NEEDS OVERRIDING]";
	return f->signature(include_class);
}

bool type_match(Class *given, Class *wanted)
{
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
		return given->parent->is_derived_from(wanted->parent);

	return given->is_derived_from(wanted);
}


// allow same classes... TODO deprecate...
bool _type_match(Class *given, bool same_chunk, Class *wanted)
{
	if ((same_chunk) and (wanted == TypeChunk))
		return true;

	return type_match(given, wanted);
}

Class::Class(const string &_name, int _size, SyntaxTree *_owner, Class *_parent)
{
	name = _name;
	owner = _owner;
	size = _size;
	type = Type::OTHER;
	array_length = 0;
	parent = _parent;
	force_call_by_value = false;
	fully_parsed = true;
	_vtable_location_target_ = nullptr;
	_vtable_location_compiler_ = nullptr;
	_vtable_location_external_ = nullptr;
};

Class::~Class()
{
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

bool Class::uses_call_by_reference() const
{
	return (!force_call_by_value and !is_pointer()) or is_array();
}

bool Class::uses_return_by_memory() const
{
	return (!force_call_by_value and !is_pointer()) or is_array();
}



bool Class::is_simple_class() const
{
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
	if (get_default_constructor())
		return false;
	if (get_complex_constructors().num > 0)
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

bool Class::usable_as_super_array() const
{
	if (is_super_array())
		return true;
	if (is_array() or is_dict() or is_pointer())
		return false;
	if (parent)
		return parent->usable_as_super_array();
	return false;
}

Class *Class::get_array_element() const
{
	if (is_array() or is_super_array() or is_dict())
		return parent;
	if (is_pointer())
		return nullptr;
	if (parent)
		return parent->get_array_element();
	return nullptr;
}

bool Class::needs_constructor() const
{
	if (!uses_call_by_reference())
		return false;
	if (is_super_array() or is_dict())
		return true;
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

bool Class::is_size_known() const
{
	if (!fully_parsed)
		return false;
	if (is_super_array() or is_dict() or is_pointer())
		return true;
	for (ClassElement &e: elements)
		if (!e.type->is_size_known())
			return false;
	return true;
}

bool Class::needs_destructor() const
{
	if (!uses_call_by_reference())
		return false;
	if (is_super_array() or is_dict())
		return true;
	if (parent){
		if (parent->get_destructor())
			return true;
		if (parent->needs_destructor())
			return true;
	}
	for (ClassElement &e: elements){
		if (e.type->get_destructor())
			return true;
		if (e.type->needs_destructor())
			return true;
	}
	return false;
}

bool Class::is_derived_from(const Class *root) const
{
	if (this == root)
		return true;
	if (is_super_array() or is_array() or is_dict() or is_pointer())
		return false;
	if (!parent)
		return false;
	return parent->is_derived_from(root);
}

bool Class::is_derived_from_s(const string &root) const
{
	if (name == root)
		return true;
	if (is_super_array() or is_array() or is_dict() or is_pointer())
		return false;
	if (!parent)
		return false;
	return parent->is_derived_from_s(root);
}

ClassFunction *Class::get_func(const string &_name, const Class *return_type, int num_params, const Class *param0) const
{
	for(ClassFunction &f: functions)
		if ((f.name == _name) and (f.return_type == return_type) and (f.param_types.num == num_params)){
			if ((param0) and (num_params > 0)){
				if (f.param_types[0] == param0)
					return &f;
			}else
				return &f;
		}
	return nullptr;
}

ClassFunction *Class::get_same_func(const string &_name, Function *ff) const
{
	for (ClassFunction &f: functions)
		if ((f.name == _name) and (f.return_type == ff->literal_return_type) and (f.param_types.num == ff->num_params)){
			bool match = true;
			for (int i=0; i<ff->num_params; i++)
				if (f.param_types[i] != ff->literal_param_type[i])
					match = false;
			if (match)
				return &f;
		}
	return nullptr;
}

ClassFunction *Class::get_default_constructor() const
{
	return get_func(IDENTIFIER_FUNC_INIT, TypeVoid, 0);
}

Array<ClassFunction*> Class::get_complex_constructors() const
{
	Array<ClassFunction*> c;
	for (ClassFunction &f: functions)
		if ((f.name == IDENTIFIER_FUNC_INIT) and (f.return_type == TypeVoid) and (f.param_types.num > 0))
			c.add(&f);
	return c;
}

ClassFunction *Class::get_destructor() const
{
	return get_func(IDENTIFIER_FUNC_DELETE, TypeVoid, 0);
}

ClassFunction *Class::get_assign() const
{
	return get_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, 1, this);
}

ClassFunction *Class::get_get(const Class *index) const
{
	for (ClassFunction &cf: functions){
		if (cf.name != "__get__")
			continue;
		if (cf.param_types.num != 1)
			continue;
		if (cf.param_types[0] != index)
			continue;
		return &cf;
	}
	return nullptr;
}

ClassFunction *Class::get_virtual_function(int virtual_index) const
{
	for (ClassFunction &f: functions)
		if (f.virtual_index == virtual_index)
			return &f;
	return nullptr;
}

void Class::link_virtual_table()
{
	if (vtable.num == 0)
		return;

	//msg_write("link vtable " + name);
	// derive from parent
	if (parent)
		for (int i=0; i<parent->vtable.num; i++)
			vtable[i] = parent->vtable[i];
	if (config.abi == ABI_WINDOWS_32)
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);

	// link virtual functions into vtable
	for (ClassFunction &cf: functions){
		if (cf.virtual_index >= 0){
			if (cf.nr >= 0){
				//msg_write(i2s(cf.virtual_index) + ": " + cf.GetFunc()->name);
				if (cf.virtual_index >= vtable.num)
					owner->DoError("LinkVirtualTable");
					//vtable.resize(cf.virtual_index + 1);
				vtable[cf.virtual_index] = (void*)cf.script->func[cf.nr];
			}
		}
		if (cf.needs_overriding){
			msg_error("needs overriding: " + cf.signature(true));
		}
	}
}

void Class::link_external_virtual_table(void *p)
{
	// link script functions according to external vtable
	VirtualTable *t = (VirtualTable*)p;
	vtable.clear();
	int max_vindex = 1;
	for (ClassFunction &cf: functions)
		if (cf.virtual_index >= 0){
			if (cf.nr >= 0)
				cf.script->func[cf.nr] = (t_func*)t[cf.virtual_index];
			if (cf.virtual_index >= vtable.num)
				max_vindex = max(max_vindex, cf.virtual_index);
		}
	vtable.resize(max_vindex + 1);
	_vtable_location_compiler_ = vtable.data;
	_vtable_location_target_ = vtable.data;
	_vtable_location_external_ = (void*)t;

	for (int i=0; i<vtable.num; i++)
		vtable[i] = t[i];
	// this should also link the "real" c++ destructor
	if ((config.abi == ABI_WINDOWS_32) or (config.abi == ABI_WINDOWS_64))
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);
}


bool class_func_match(ClassFunction &a, ClassFunction &b)
{
	if (a.name != b.name)
		return false;
	if (a.return_type != b.return_type)
		return false;
	if (a.param_types.num != b.param_types.num)
		return false;
	for (int i=0; i<a.param_types.num; i++)
		if (!type_match(b.param_types[i], a.param_types[i]))
			return false;
	return true;
}


Class *Class::get_pointer() const
{
	return owner->CreateNewClass(name + "*", Class::Type::POINTER, config.pointer_size, 0, const_cast<Class*>(this));
}

Class *Class::get_root() const
{
	Class *r = const_cast<Class*>(this);
	while (r->parent)
		r = r->parent;
	return r;
}

void class_func_out(Class *c, ClassFunction *f)
{
	string ps;
	for (Class *p: f->param_types)
		ps += "  " + p->name;
	msg_write(c->name + "." + f->name + ps);
}

void Class::add_function(SyntaxTree *s, int func_no, bool as_virtual, bool override)
{
	Function *f = s->functions[func_no];
	ClassFunction cf;
	cf.name = f->name.substr(name.num + 1, -1);
	cf.nr = func_no;
	cf.script = s->script;
	cf.return_type = f->return_type;
	for (int i=0; i<f->num_params; i++)
		cf.param_types.add(f->var[i].type);
	if (as_virtual){
		cf.virtual_index = ProcessClassOffset(name, cf.name, max(vtable.num, 2));
		if (vtable.num <= cf.virtual_index)
			vtable.resize(cf.virtual_index + 1);
		_vtable_location_compiler_ = vtable.data;
		_vtable_location_target_ = vtable.data;
	}

	// override?
	ClassFunction *orig = nullptr;
	for (ClassFunction &ocf: functions)
		if (class_func_match(cf, ocf))
			orig = &ocf;
	if (override and !orig)
		s->DoError(format("can not override function %s, no previous definition", f->signature(true).c_str()), f->_exp_no, f->_logical_line_no);
	if (!override and orig){
		msg_write(f->signature(true));
		msg_write(orig->signature(true));
		s->DoError(format("function %s is already defined, use '%s'", f->signature(true).c_str(), IDENTIFIER_OVERRIDE.c_str()), f->_exp_no, f->_logical_line_no);
	}
	if (override){
		orig->script = cf.script;
		orig->nr = cf.nr;
		orig->needs_overriding = false;
		orig->param_types = cf.param_types;
	}else
		functions.add(cf);
}

bool Class::derive_from(const Class* root, bool increase_size)
{
	parent = const_cast<Class*>(root);
	bool found = false;
	if (parent->elements.num > 0){
		// inheritance of elements
		elements = parent->elements;
		found = true;
	}
	if (parent->functions.num > 0){
		// inheritance of functions
		for (ClassFunction &f: parent->functions){
			if (f.name == IDENTIFIER_FUNC_ASSIGN)
				continue;
			ClassFunction ff = f;
			ff.needs_overriding = (f.name == IDENTIFIER_FUNC_INIT) or (f.name == IDENTIFIER_FUNC_DELETE) or (f.name == IDENTIFIER_FUNC_ASSIGN);
			functions.add(ff);
		}
		found = true;
	}
	if (increase_size)
		size += parent->size;
	vtable = parent->vtable;
	_vtable_location_compiler_ = vtable.data;
	_vtable_location_target_ = vtable.data;
	return found;
}

void *Class::create_instance() const
{
	void *p = malloc(size);
	ClassFunction *c = get_default_constructor();
	if (c){
		typedef void con_func(void *);
		con_func * f = (con_func*)c->script->func[c->nr];
		if (f)
			f(p);
	}
	return p;
}

string Class::var2str(void *p) const
{
	if (this == TypeInt){
		return i2s(*(int*)p);
	}else if (this == TypeFloat32){
		return f2s(*(float*)p, 3);
	}else if (this == TypeBool){
		return b2s(*(bool*)p);
	}else if (is_pointer()){
		return p2s(*(void**)p);
	}else if (this == TypeString){
		return "\"" + *(string*)p + "\"";
	}else if (this == TypeCString){
		return "\"" + string((char*)p) + "\"";
	}else if (is_super_array()){
		string s;
		DynamicArray *da = (DynamicArray*)p;
		for (int i=0; i<da->num; i++){
			if (i > 0)
				s += ", ";
			s += parent->var2str(((char*)da->data) + i * da->element_size);
		}
		return "[" + s + "]";
	}else if (is_dict()){
		return "{...}";
	}else if (elements.num > 0){
		string s;
		foreachi(ClassElement &e, elements, i){
			if (i > 0)
				s += ", ";
			s += e.type->var2str(((char*)p) + e.offset);
		}
		return "(" + s + ")";

	}else if (is_array()){
			string s;
			for (int i=0; i<array_length; i++){
				if (i > 0)
					s += ", ";
				s += parent->var2str(((char*)p) + i * parent->size);
			}
			return "[" + s + "]";
		}
	return string((char*)p, size).hex();
}

}

