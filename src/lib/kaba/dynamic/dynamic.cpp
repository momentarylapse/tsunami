#include "dynamic.h"
#include "../kaba.h"
#include "exception.h"
#include "call.h"
#include "../../any/any.h"
#include "../../base/callable.h"
#include "../../os/msg.h"

namespace kaba {
	
extern const Class *TypeIntList;
extern const Class *TypeFloatList;
extern const Class *TypeBoolList;
extern const Class *TypeAny;
extern const Class *TypePath;
extern const Class *TypeSpecialFunction;

	
	

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


void var_assign(void *pa, const void *pb, const Class *type) {
	if ((type == TypeInt) or (type == TypeFloat32)) {
		*(int*)pa = *(int*)pb;
	} else if ((type == TypeBool) or (type == TypeInt8)) {
		*(char*)pa = *(char*)pb;
	} else if (type->is_pointer_raw()) {
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

void var_init(void *p, const Class *type) {
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

void array_clear(void *p, const Class *type) {
	auto *f = type->get_member_func("clear", TypeVoid, {});
	if (!f)
		kaba_raise_exception(new KabaException("can not clear an array of type " + type->long_name()));
	typedef void func_t(void*);
	auto *ff = (func_t*)f->address;
	ff(p);
}

void array_resize(void *p, const Class *type, int num) {
	auto *f = type->get_member_func("resize", TypeVoid, {TypeInt});
	if (!f)
		kaba_raise_exception(new KabaException("can not resize an array of type " + type->long_name()));
	typedef void func_t(void*, int);
	auto *ff = (func_t*)f->address;
	ff(p, num);
}

void array_add(DynamicArray &array, void *p, const Class *type) {
	//msg_write("array add " + type->long_name());
	if ((type == TypeIntList) or (type == TypeFloatList)) {
		array.append_4_single(*(int*)p);
	} else if (type == TypeBoolList) {
		array.append_1_single(*(char*)p);
	} else {
		auto *f = type->get_member_func("add", TypeVoid, {type->param[0]});
		if (!f)
			kaba_raise_exception(new KabaException("can not add to array type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(&array, p);
	}
}


struct EnumLabel {
	const Class *type;
	int value;
	string label;
};
Array<EnumLabel> all_enum_labels;
void add_enum_label(const Class *type, int value, const string &label) {
	all_enum_labels.add({type, value, label});
}
void remove_enum_labels(const Class *type) {
	for (int i=all_enum_labels.num-1; i>=0; i--)
		if (all_enum_labels[i].type == type)
			all_enum_labels.erase(i);
}
string find_enum_label(const Class *type, int value) {
	// explicit labels
	for (auto &l: all_enum_labels)
		if (l.type == type and l.value == value)
			return l.label;

	// const names
	for (auto c: type->constants)
		if (c->type == type and c->as_int() == value)
			return c->name;

	// not found
	return i2s(value);
}

int enum_parse(const string &label, const Class *type) {
	// explicit labels
	for (auto &l: all_enum_labels)
		if (l.type == type and l.label == label)
			return l.value;

	// const names
	for (auto c: type->constants)
		if (c->type == type and c->name == label)
			return c->as_int();

	if (str_is_integer(label))
		return s2i(label);

	// not found
	return -1;
}



string class_repr(const Class *c) {
	if (c)
		return c->long_name();
	return "nil";
}

// probably deprecated...?
string func_repr(const Function *f) {
	if (f)
		return "<func " + f->long_name() + ">";
	return "<func -nil->";
}


Array<const Class*> get_callable_param_types(const Class *fp);
const Class *get_callable_return_type(const Class *fp);
string make_callable_signature(const Array<const Class*> &param, const Class *ret);
string callable_signature(const Class *type) {
	auto pp = get_callable_param_types(type);
	auto r = get_callable_return_type(type);
	return make_callable_signature(pp, r);
}

string callable_repr(const void *p, const Class *type) {
	return "<callable " + callable_signature(type) + ">";
}

string _cdecl var_repr_str(const void *p, const Class *type, bool as_repr) {
//	msg_write(type->name);
	// fixed
	if (type == TypeInt32) {
		return str(*reinterpret_cast<const int *>(p));
	} if (type == TypeInt8) {
		return format("0x%02x", (int)*reinterpret_cast<const char *>(p));
	} if (type == TypeInt64) {
		return str(*reinterpret_cast<const int64*>(p));
	} else if (type == TypeFloat32) {
		return f2s(*reinterpret_cast<const float*>(p), 6);
	} else if (type == TypeFloat64) {
		return f642s(*reinterpret_cast<const double*>(p), 6);
	} else if (type == TypeBool) {
		return b2s(*reinterpret_cast<const bool*>(p));
	//} else if (type == TypeClass) {
	//	return class_repr(reinterpret_cast<const Class*>(p));
	} else if (type->is_callable_fp() or type->is_callable_bind()) {
		return callable_repr(p, type);
	} else if (type == TypeFunction or type->type == Class::Type::FUNCTION) {
		// probably not...
		return func_repr(reinterpret_cast<const Function*>(p));
	} else if (type == TypeSpecialFunction) {
		return format("<special function %s>", reinterpret_cast<const SpecialFunction*>(p)->name);
	} else if (type == TypeAny) {
		if (as_repr)
			return reinterpret_cast<const Any*>(p)->repr();
		else
			return reinterpret_cast<const Any*>(p)->str();
	} else if (type->is_reference()) {
		auto *pp = *(void**)p;
		// auto deref?
		if (type->param[0] == TypeClass)
			return class_repr(*reinterpret_cast<const Class* const *>(p));
		if (type->param[0]->is_callable())
			return var_repr_str(pp, type->param[0], as_repr);
		if (type->param[0] == TypeFunction)
			return var_repr_str(pp, type->param[0], as_repr);
		if (type->param[0]->is_callable())
			return var_repr_str(pp, type->param[0], as_repr);
		if (type->param[0] != TypeVoid)
			return /*"&" +*/ var_repr_str(pp, type->param[0], as_repr);
		return p2s(pp);
	} else if (type->is_some_pointer()) {
		auto *pp = *(void**)p;
		// auto deref?
		if (pp and (type->param[0] != TypeVoid))
			return /*"&" +*/ var_repr_str(pp, type->param[0], as_repr);
		if (pp and type->param[0]->is_callable())
			return var_repr_str(pp, type->param[0], as_repr);
		if (pp and type->param[0] == TypeFunction)
			return var_repr_str(pp, type->param[0], as_repr);
		return p2s(pp);
	} else if (type == TypeString) { // covered by user code...
		if (as_repr)
			return reinterpret_cast<const string*>(p)->repr();
		else
			return *reinterpret_cast<const string*>(p);
	} else if (type == TypeCString) {
		if (as_repr)
			return string((char*)p).repr();
		else
			return string((char*)p);
	} else if (type == TypePath) { // covered by user code...
		if (as_repr)
			return reinterpret_cast<const Path*>(p)->str().repr();
		else
			return reinterpret_cast<const Path*>(p)->str();
	} else if (type->is_enum()) {
		return find_enum_label(type, *reinterpret_cast<const int*>(p));
	} else if (type->is_optional()) {
		if (*reinterpret_cast<const bool*>((int_p)p + type->size - 1))
			return var_repr_str(p, type->param[0], as_repr);
		return "nil";
	} else if (type->is_list()) {
		string s;
		auto *da = reinterpret_cast<const DynamicArray*>(p);
		for (int i=0; i<da->num; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)da->data) + i * da->element_size, type->param[0]);
		}
		return "[" + s + "]";
	} else if (type->is_dict()) {
		string s;
		auto *da = reinterpret_cast<const DynamicArray*>(p);
		for (int i=0; i<da->num; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)da->data) + i * da->element_size, TypeString);
			s += ": ";
			s += var_repr(((char*)da->data) + i * da->element_size + sizeof(string), type->param[0]);
		}
		return "{" + s + "}";
	} else if (type->is_array()) {
		string s;
		for (int i=0; i<type->array_length; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)p) + i * type->param[0]->size, type->param[0]);
		}
		return "[" + s + "]";
	} else if (type->is_enum()) {
		return find_enum_label(type, *(int*)p);
	}


	// try user code
	auto f_str = type->get_member_func(Identifier::Func::STR, TypeString, {});
	auto f_repr = type->get_member_func(Identifier::Func::REPR, TypeString, {});
	auto f = f_str;
	if ((as_repr and f_repr) or !f_str)
		f = f_repr;
	if (f) {
		string r;
		if (call_member_function(f, const_cast<void*>(p), &r, {}, true))
			return r;
	}

	// basic "universal" representations
	if (type->elements.num > 0) {
		string s;
		for (auto &e: type->elements) {
			if (e.hidden())
				continue;
			if (s.num > 0)
				s += ", ";
			s += var_repr(((char*)p) + e.offset, e.type);
		}
		return "(" + s + ")";
	}
	return d2h(p, type->size);
}


string _cdecl var_repr(const void *p, const Class *type) {
	return var_repr_str(p, type, true);
}

string _cdecl var2str(const void *p, const Class *type) {
	return var_repr_str(p, type, false);
}

Any _cdecl dynify(const void *var, const Class *type) {
	if (type == TypeInt or type->is_enum())
		return Any(*(int*)var);
	if (type == TypeFloat32)
		return Any(*(float*)var);
	if (type == TypeBool)
		return Any(*(bool*)var);
	if (type == TypeString)
		return Any(*(string*)var);
	if (type->is_some_pointer())
		return Any(*(void**)var);
	if (type == TypeAny)
		return *(Any*)var;
	if (type->is_array()) {
		Any a = Any::EmptyArray;
		auto *t_el = type->get_array_element();
		for (int i=0; i<type->array_length; i++)
			a.add(dynify((char*)var + t_el->size * i, t_el));
		return a;
	}
	if (type->is_list()) {
		Any a = Any::EmptyArray;
		auto *ar = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<ar->num; i++)
			a.add(dynify((char*)ar->data + ar->element_size * i, t_el));
		return a;
	}
	if (type->is_dict()) {
		Any a = Any::EmptyMap;
		auto *da = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<da->num; i++) {
			string key = *(string*)(((char*)da->data) + i * da->element_size);
			a.map_set(key, dynify(((char*)da->data) + i * da->element_size + sizeof(string), t_el));
		}
		return a;
	}
	
	// class
	Any a;
	for (auto &e: type->elements) {
		if (!e.hidden())
			a.map_set(e.name, dynify((char*)var + e.offset, e.type));
	}
	return a;
}

Array<const Class*> func_effective_params(const Function *f);

// deprecated, but who knows...
DynamicArray array_map(void *fff, DynamicArray *a, const Class *ti, const Class *to) {
	//msg_write("map " + ti->long_name() + " -> " + to->long_name());

	DynamicArray r;
	r.init(to->size);
	if (to->needs_constructor()) {
		if (to == TypeString) {
			((Array<string>*)&r)->resize(a->num);
		} else  {
			kaba_raise_exception(new KabaException("map(): output type not allowed: " + to->long_name()));
		}
	} else {
		r.simple_resize(a->num);
	}
	for (int i=0; i<a->num; i++) {
		void *po = r.simple_element(i);
		void *pi = a->simple_element(i);
		bool ok = call_callable(fff, po, {pi}, to, {ti});
		if (!ok)
			kaba_raise_exception(new KabaException(format("map(): failed to dynamically call %s -> %s", ti->long_name(), to->long_name())));
	}
	return r;
}

#pragma GCC pop_options

	
	
}
