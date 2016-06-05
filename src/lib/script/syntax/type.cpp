#include "../../base/base.h"
#include "type.h"
#include "../script.h"
#include "../../file/file.h"

namespace Script{

ClassElement::ClassElement()
{
	hidden = false;
}


ClassFunction::ClassFunction()
{
	nr = -1;
	virtual_index = -1;
	needs_overriding = false;
}

ClassFunction::ClassFunction(const string &_name, Type *_return_type, Script *s, int no)
{
	name = _name;
	return_type = _return_type;
	script = s;
	nr = no;
	virtual_index = -1;
	needs_overriding = false;
}

Function* ClassFunction::GetFunc()
{
	return script->syntax->functions[nr];
}

bool direct_type_match(Type *a, Type *b)
{
	return ( (a==b) or ( (a->is_pointer) and (b->is_pointer) ) or (a->IsDerivedFrom(b)) );
}

// both operand types have to match the operator's types
//   (operator wants a pointer -> all pointers are allowed!!!)
//   (same for classes of same type...)
bool type_match(Type *type, bool is_class, Type *wanted)
{
	if (type == wanted)
		return true;
	if ((type->is_pointer) and (wanted == TypePointer))
		return true;
	if ((is_class) and (wanted == TypeClass))
		return true;
	if (type->IsDerivedFrom(wanted))
		return true;
	return false;
}

Type::Type()//const string &_name, int _size, SyntaxTree *_owner)
{
	//name = _name;
	owner = NULL;//_owner;
	size = 0;//_size;
	is_array = false;
	is_super_array = false;
	array_length = 0;
	is_pointer = false;
	is_silent = false;
	parent = NULL;
	force_call_by_value = false;
	fully_parsed = true;
};

Type::~Type()
{
}

bool Type::UsesCallByReference() const
{	return ((!force_call_by_value) and (!is_pointer)) or (is_array);	}

bool Type::UsesReturnByMemory() const
{	return ((!force_call_by_value) and (!is_pointer)) or (is_array);	}



bool Type::is_simple_class() const
{
	if (!UsesCallByReference())
		return true;
	/*if (is_array)
		return false;*/
	if (is_super_array)
		return false;
	if (vtable.num > 0)
		return false;
	if (parent)
		if (!parent->is_simple_class())
			return false;
	if (GetDefaultConstructor())
		return false;
	if (GetComplexConstructor())
		return false;
	if (GetDestructor())
		return false;
	if (GetAssign())
		return false;
	for (ClassElement &e : element)
		if (!e.type->is_simple_class())
			return false;
	return true;
}

bool Type::usable_as_super_array() const
{
	if (is_super_array)
		return true;
	if (is_array)
		return false;
	if (is_pointer)
		return false;
	if (parent)
		return parent->usable_as_super_array();
	return false;
}

Type *Type::GetArrayElement() const
{
	if ((is_array) or (is_super_array))
		return parent;
	if (is_pointer)
		return NULL;
	if (parent)
		return parent->GetArrayElement();
	return NULL;
}

bool Type::needs_constructor() const
{
	if (!UsesCallByReference())
		return false;
	if (is_super_array)
		return true;
	if (vtable.num > 0)
		return true;
	if (parent)
		if (parent->needs_constructor())
			return true;
	for (ClassElement &e : element)
		if (e.type->needs_constructor())
			return true;
	return false;
}

bool Type::is_size_known() const
{
	if (!fully_parsed)
		return false;
	if ((is_super_array) or (is_pointer))
		return true;
	for (ClassElement &e : element)
		if (!e.type->is_size_known())
			return false;
	return true;
}

bool Type::needs_destructor() const
{
	if (!UsesCallByReference())
		return false;
	if (is_super_array)
		return true;
	if (parent){
		if (parent->GetDestructor())
			return true;
		if (parent->needs_destructor())
			return true;
	}
	for (ClassElement &e : element){
		if (e.type->GetDestructor())
			return true;
		if (e.type->needs_destructor())
			return true;
	}
	return false;
}

bool Type::IsDerivedFrom(const Type *root) const
{
	if (this == root)
		return true;
	if ((is_super_array) or (is_array) or (is_pointer))
		return false;
	if (!parent)
		return false;
	return parent->IsDerivedFrom(root);
}

bool Type::IsDerivedFrom(const string &root) const
{
	if (name == root)
		return true;
	if ((is_super_array) or (is_array) or (is_pointer))
		return false;
	if (!parent)
		return false;
	return parent->IsDerivedFrom(root);
}

ClassFunction *Type::GetFunc(const string &_name, const Type *return_type, int num_params, const Type *param0) const
{
	foreachi(ClassFunction &f, function, i)
		if ((f.name == _name) and (f.return_type == return_type) and (f.param_type.num == num_params)){
			if ((param0) and (num_params > 0)){
				if (f.param_type[0] == param0)
					return &f;
			}else
				return &f;
		}
	return NULL;
}

ClassFunction *Type::GetDefaultConstructor() const
{
	return GetFunc(IDENTIFIER_FUNC_INIT, TypeVoid, 0);
}

ClassFunction *Type::GetComplexConstructor() const
{
	for (ClassFunction &f : function)
		if ((f.name == IDENTIFIER_FUNC_INIT) and (f.return_type == TypeVoid) and (f.param_type.num > 0))
			return &f;
	return NULL;
}

ClassFunction *Type::GetDestructor() const
{
	return GetFunc(IDENTIFIER_FUNC_DELETE, TypeVoid, 0);
}

ClassFunction *Type::GetAssign() const
{
	return GetFunc(IDENTIFIER_FUNC_ASSIGN, TypeVoid, 1, this);
}

ClassFunction *Type::GetGet(const Type *index) const
{
	for (ClassFunction &cf : function){
		if (cf.name != "__get__")
			continue;
		if (cf.param_type.num != 1)
			continue;
		if (cf.param_type[0] != index)
			continue;
		return &cf;
	}
	return NULL;
}

ClassFunction *Type::GetVirtualFunction(int virtual_index) const
{
	for (ClassFunction &f : function)
		if (f.virtual_index == virtual_index)
			return &f;
	return NULL;
}

void Type::LinkVirtualTable()
{
	if (vtable.num == 0)
		return;

	//msg_write("link vtable " + name);
	// derive from parent
	if (parent)
		for (int i=0;i<parent->vtable.num;i++)
			vtable[i] = parent->vtable[i];
	if (config.abi == ABI_WINDOWS_32)
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);

	// link virtual functions into vtable
	for (ClassFunction &cf : function){
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
			msg_error("needs overwriting: " + name + " : " + cf.name);
		}
	}
}

void Type::LinkExternalVirtualTable(void *p)
{
	// link script functions according to external vtable
	VirtualTable *t = (VirtualTable*)p;
	vtable.clear();
	int max_vindex = 1;
	for (ClassFunction &cf : function)
		if (cf.virtual_index >= 0){
			if (cf.nr >= 0)
				cf.script->func[cf.nr] = (t_func*)t[cf.virtual_index];
			if (cf.virtual_index >= vtable.num)
				max_vindex = max(max_vindex, cf.virtual_index);
		}
	vtable.resize(max_vindex + 1);
	_vtable_location_compiler_ = vtable.data;
	_vtable_location_target_ = vtable.data;

	for (int i=0;i<vtable.num;i++)
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
	if (a.param_type.num != b.param_type.num)
		return false;
	for (int i=0; i<a.param_type.num; i++)
		if (!direct_type_match(b.param_type[i], a.param_type[i]))
			return false;
	return true;
}

string func_signature(Function *f)
{
	string r = f->literal_return_type->name + " " + f->name + "(";
	for (int i=0;i<f->num_params;i++){
		if (i > 0)
			r += ", ";
		r += f->literal_param_type[i]->name;
	}
	return r + ")";
}


Type *Type::GetPointer() const
{
	return owner->CreateNewType(name + "*", config.pointer_size, true, false, false, 0, const_cast<Type*>(this));
}

Type *Type::GetRoot() const
{
	Type *r = const_cast<Type*>(this);
	while (r->parent)
		r = r->parent;
	return r;
}

void class_func_out(Type *c, ClassFunction *f)
{
	string ps;
	for (Type *p : f->param_type)
		ps += "  " + p->name;
	msg_write(c->name + "." + f->name + ps);
}

void Type::AddFunction(SyntaxTree *s, int func_no, bool as_virtual, bool override)
{
	Function *f = s->functions[func_no];
	ClassFunction cf;
	cf.name = f->name.substr(name.num + 1, -1);
	cf.nr = func_no;
	cf.script = s->script;
	cf.return_type = f->return_type;
	for (int i=0;i<f->num_params;i++)
		cf.param_type.add(f->var[i].type);
	if (as_virtual){
		cf.virtual_index = ProcessClassOffset(name, cf.name, max(vtable.num, 2));
		if (vtable.num <= cf.virtual_index)
			vtable.resize(cf.virtual_index + 1);
		_vtable_location_compiler_ = vtable.data;
		_vtable_location_target_ = vtable.data;
	}

	// override?
	ClassFunction *orig = NULL;
	for (ClassFunction &ocf : function)
		if (class_func_match(ocf, cf))
			orig = &ocf;
	if (override and !orig)
		s->DoError(format("can not override function '%s', no previous definition", func_signature(f).c_str()), f->_exp_no, f->_logical_line_no);
	if (!override and orig)
		s->DoError(format("function '%s' is already defined, use '%s' to override", func_signature(f).c_str(), IDENTIFIER_OVERRIDE.c_str()), f->_exp_no, f->_logical_line_no);
	if (override){
		orig->script = cf.script;
		orig->nr = cf.nr;
		orig->needs_overriding = false;
		orig->param_type = cf.param_type;
	}else
		function.add(cf);
}

bool Type::DeriveFrom(const Type* root, bool increase_size)
{
	parent = const_cast<Type*>(root);
	bool found = false;
	if (parent->element.num > 0){
		// inheritance of elements
		element = parent->element;
		found = true;
	}
	if (parent->function.num > 0){
		// inheritance of functions
		for (ClassFunction &f : parent->function){
			if (f.name == IDENTIFIER_FUNC_ASSIGN)
				continue;
			ClassFunction ff = f;
			ff.needs_overriding = (f.name == IDENTIFIER_FUNC_INIT) or (f.name == IDENTIFIER_FUNC_DELETE) or (f.name == IDENTIFIER_FUNC_ASSIGN);
			function.add(ff);
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

void *Type::CreateInstance() const
{
	void *p = malloc(size);
	ClassFunction *c = GetDefaultConstructor();
	if (c){
		typedef void con_func(void *);
		con_func * f = (con_func*)c->script->func[c->nr];
		if (f)
			f(p);
	}
	return p;
}

string Type::var2str(void *p) const
{
	if (this == TypeInt)
		return i2s(*(int*)p);
	else if (this == TypeFloat32)
		return f2s(*(float*)p, 3);
	else if (this == TypeBool)
		return b2s(*(bool*)p);
	else if (is_pointer)
		return p2s(*(void**)p);
	else if (this == TypeString)
		return "\"" + *(string*)p + "\"";
	else if (is_super_array){
		string s;
		DynamicArray *da = (DynamicArray*)p;
		for (int i=0; i<da->num; i++){
			if (i > 0)
				s += ", ";
			s += parent->var2str(((char*)da->data) + i * da->element_size);
		}
		return "[" + s + "]";
	}else if (element.num > 0){
		string s;
		foreachi(ClassElement &e, element, i){
			if (i > 0)
				s += ", ";
			s += e.type->var2str(((char*)p) + e.offset);
		}
		return "(" + s + ")";

	}else if (is_array){
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

