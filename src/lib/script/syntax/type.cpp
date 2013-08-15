#include "../../base/base.h"
#include "type.h"
#include "../script.h"
#include "../../file/file.h"

namespace Script{


ClassFunction::ClassFunction(const string &_name, Type *_return_type, Script *s, int no)
{
	name = _name;
	return_type = _return_type;
	script = s;
	nr = no;
	virtual_index = -1;
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
	vtable = NULL;
	num_virtual = 0;
};

Type::~Type()
{
	if (vtable)
		delete[](vtable);
}

bool Type::UsesCallByReference()
{	return ((!force_call_by_value) && (!is_pointer)) || (is_array);	}

bool Type::UsesReturnByMemory()
{	return ((!force_call_by_value) && (!is_pointer)) || (is_array);	}



bool Type::is_simple_class()
{
	if (!UsesCallByReference())
		return true;
	/*if (is_array)
		return false;*/
	if (is_super_array)
		return false;
	if (vtable)
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
	if (GetFunc("__assign__") >= 0)
		return false;
	foreach(ClassElement &e, element)
		if (!e.type->is_simple_class())
			return false;
	return true;
}

bool Type::IsDerivedFrom(Type *root) const
{
	if (this == root)
		return true;
	if ((is_super_array) || (is_array) || (is_pointer))
		return false;
	if (!parent)
		return false;
	return parent->IsDerivedFrom(root);
}

int Type::GetFunc(const string &name)
{
	foreachi(ClassFunction &f, function, i)
		if (f.name == name)
			return i;
	return -1;
}

ClassFunction *Type::GetDefaultConstructor()
{
	foreach(ClassFunction &f, function)
		if ((f.name == "__init__") && (f.return_type == TypeVoid) && (f.param_type.num == 0))
			return &f;
	return NULL;
}

ClassFunction *Type::GetComplexConstructor()
{
	foreach(ClassFunction &f, function)
		if ((f.name == "__init__") && (f.return_type == TypeVoid) && (f.param_type.num > 0))
			return &f;
	return NULL;
}

ClassFunction *Type::GetDestructor()
{
	foreach(ClassFunction &f, function)
		if ((f.name == "__delete__") && (f.return_type == TypeVoid) && (f.param_type.num == 0))
			return &f;
	return NULL;
}

ClassFunction *Type::GetVirtualFunction(int virtual_index)
{
	foreach(ClassFunction &f, function)
		if (f.virtual_index == virtual_index)
			return &f;
	return NULL;
}

void Type::LinkVirtualTable()
{
	if (!vtable)
		return;

	//msg_write("link vtable " + name);
	// derive from parent
	if (parent)
		for (int i=0;i<parent->num_virtual;i++)
			vtable[i] = parent->vtable[i];
	if (config.abi == AbiWindows32)
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);

	// link virtual functions into vtable
	foreach(ClassFunction &cf, function)
		if (cf.virtual_index >= 0){
			if (cf.nr >= 0){
				//msg_write(i2s(cf.virtual_index) + ": " + cf.script->syntax->Functions[cf.nr]->name);
				vtable[cf.virtual_index] = (void*)cf.script->func[cf.nr];
			}
			num_virtual = max(cf.virtual_index + 1, num_virtual);
		}
}

void Type::LinkExternalVirtualTable(void *p)
{
	// link script functions according to external vtable
	VirtualTable *t = (VirtualTable*)p;
	num_virtual = 0;
	foreach(ClassFunction &cf, function)
		if (cf.virtual_index >= 0){
			if (cf.nr >= 0)
				cf.script->func[cf.nr] = (t_func*)t[cf.virtual_index];
			num_virtual = max(cf.virtual_index + 1, num_virtual);
		}

	vtable = new VirtualTable[num_virtual];
	for (int i=0;i<num_virtual;i++)
		vtable[i] = t[i];
	// this should also link the "real" c++ destructor
	if (config.abi == AbiWindows32)
		vtable[0] = mf(&VirtualBase::__delete_external__);
	else
		vtable[1] = mf(&VirtualBase::__delete_external__);
}

bool Type::DeriveFrom(Type* root)
{
	parent = root;
	bool found = false;
	if (parent->element.num > 0){
		// inheritance of elements
		element = parent->element;
		found = true;
	}
	if (parent->function.num > 0){
		// inheritance of functions
		foreach(ClassFunction &f, parent->function){
			if ((f.name != "__init__") && (f.name != "__assign__"))
				function.add(f);
		}
		found = true;
	}
	size += parent->size;
	num_virtual += parent->num_virtual;
	return found;
}

void *Type::CreateInstance()
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

string Type::var2str(void *p)
{
	if (this == TypeInt)
		return i2s(*(int*)p);
	else if (this == TypeFloat)
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

