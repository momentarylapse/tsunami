#include "dynamic.h"
#include "../kaba.h"
#include "exception.h"
#include "call.h"
#include "../../any/any.h"
#include "../../base/callable.h"

namespace kaba {
	
extern const Class *TypeIntList;
extern const Class *TypeFloatList;
extern const Class *TypeBoolList;
extern const Class *TypeAny;
extern const Class *TypePath;


	
	

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

template<class T>
void _kaba_array_sort(DynamicArray &array, int offset_by, bool reverse) {
	T *p = (T*)((char*)array.data + offset_by);
	for (int i=0; i<array.num; i++) {
		T *q = (T*)((char*)p + array.element_size);
		for (int j=i+1; j<array.num; j++) {
			if ((*p > *q) xor reverse)
				array.simple_swap(i, j);
			q = (T*)((char*)q + array.element_size);
		}
		p = (T*)((char*)p + array.element_size);
	}
}

template<class T>
void _kaba_array_sort_p(DynamicArray &array, int offset_by, bool reverse) {
	char **p = (char**)array.data;
	for (int i=0; i<array.num; i++) {
		T *pp = (T*)(*p + offset_by);
		char **q = p + 1;
		for (int j=i+1; j<array.num; j++) {
			T *qq = (T*)(*q + offset_by);
			if ((*pp > *qq) xor reverse) {
				array.simple_swap(i, j);
				pp = (T*)(*p + offset_by);
			}
			q ++;
		}
		p ++;
	}
}

template<class T>
void _kaba_array_sort_pf(DynamicArray &array, Function *f, bool reverse) {
	char **p = (char**)array.data;
	T r1, r2;
	for (int i=0; i<array.num; i++) {
		if (!call_function(f, &r1, {*p}))
			kaba_raise_exception(new KabaException("call failed " + f->long_name()));

		//T *pp = (T*)(*p + offset_by);
		char **q = p + 1;
		for (int j=i+1; j<array.num; j++) {
			if (!call_function(f, &r2, {*q}))
				kaba_raise_exception(new KabaException("call failed"));
			if ((r1 > r2) xor reverse) {
				array.simple_swap(i, j);
				std::swap(r1, r2);
			}
			q ++;
		}
		p ++;
	}
}

void kaba_var_assign(void *pa, const void *pb, const Class *type) {
	if ((type == TypeInt) or (type == TypeFloat32)) {
		*(int*)pa = *(int*)pb;
	} else if ((type == TypeBool) or (type == TypeChar)) {
		*(char*)pa = *(char*)pb;
	} else if (type->is_pointer()) {
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

void kaba_var_init(void *p, const Class *type) {
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

void kaba_array_clear(void *p, const Class *type) {
	auto *f = type->get_func("clear", TypeVoid, {});
	if (!f)
		kaba_raise_exception(new KabaException("can not clear an array of type " + type->long_name()));
	typedef void func_t(void*);
	auto *ff = (func_t*)f->address;
	ff(p);
}

void kaba_array_resize(void *p, const Class *type, int num) {
	auto *f = type->get_func("resize", TypeVoid, {TypeInt});
	if (!f)
		kaba_raise_exception(new KabaException("can not resize an array of type " + type->long_name()));
	typedef void func_t(void*, int);
	auto *ff = (func_t*)f->address;
	ff(p, num);
}

void kaba_array_add(DynamicArray &array, void *p, const Class *type) {
	//msg_write("array add " + type->long_name());
	if ((type == TypeIntList) or (type == TypeFloatList)) {
		array.append_4_single(*(int*)p);
	} else if (type == TypeBoolList) {
		array.append_1_single(*(char*)p);
	} else {
		auto *f = type->get_func("add", TypeVoid, {type->param[0]});
		if (!f)
			kaba_raise_exception(new KabaException("can not add to array type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(&array, p);
	}
}

DynamicArray _cdecl kaba_array_sort(DynamicArray &array, const Class *type, const string &_by) {
	if (!type->is_super_array())
		kaba_raise_exception(new KabaException("type '" + type->name + "' is not an array"));
	const Class *el = type->param[0];
	if (array.element_size != el->size)
		kaba_raise_exception(new KabaException("element type size mismatch..."));

	DynamicArray rr;
	kaba_var_init(&rr, type);
	kaba_var_assign(&rr, &array, type);

	const Class *rel = el;

	if (el->is_pointer())
		rel = el->param[0];

	string by = _by;
	bool reverse = false;
	if (_by.head(1) == "-") {
		by = by.sub(1);
		reverse = true;
	}

	int offset = -1;
	const Class *by_type = nullptr;
	Function *sfunc = nullptr;
	if (by == "") {
		offset = 0;
		by_type = rel;
	} else {
		for (auto &e: rel->elements)
			if (e.name == by) {
				by_type = e.type;
				offset = e.offset;
			}
		if (!by_type) {
			for (auto *f: weak(rel->functions))
				if (f->name == by) {
					if (f->num_params > 0)
						kaba_raise_exception(new KabaException("can only sort by a function without parameters"));
					by_type = f->literal_return_type;
					sfunc = f;
				}
			if (!sfunc)
				kaba_raise_exception(new KabaException("type '" + rel->name + "' does not have an element '" + by + "'"));
		}
	}

	if (sfunc) {
		if (!el->is_pointer())
			kaba_raise_exception(new KabaException("function sorting only for pointers"));
		if (by_type == TypeString)
			_kaba_array_sort_pf<string>(rr, sfunc, reverse);
		else if (by_type == TypePath)
			_kaba_array_sort_pf<Path>(rr, sfunc, reverse);
		else if (by_type == TypeInt)
			_kaba_array_sort_pf<int>(rr, sfunc, reverse);
		else if (by_type == TypeFloat32)
			_kaba_array_sort_pf<float>(rr, sfunc, reverse);
		else if (by_type == TypeBool)
			_kaba_array_sort_pf<bool>(rr, sfunc, reverse);
		else
			kaba_raise_exception(new KabaException("can't sort by function '" + by_type->long_name() + "' yet"));

	} else if (el->is_pointer()) {
		if (by_type == TypeString)
			_kaba_array_sort_p<string>(rr, offset, reverse);
		else if (by_type == TypePath)
			_kaba_array_sort_p<Path>(rr, offset, reverse);
		else if (by_type == TypeInt)
			_kaba_array_sort_p<int>(rr, offset, reverse);
		else if (by_type == TypeFloat32)
			_kaba_array_sort_p<float>(rr, offset, reverse);
		else if (by_type == TypeBool)
			_kaba_array_sort_p<bool>(rr, offset, reverse);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	} else {
		if (by_type == TypeString)
			_kaba_array_sort<string>(rr, offset, reverse);
		else if (by_type == TypePath)
			_kaba_array_sort<Path>(rr, offset, reverse);
		else if (by_type == TypeInt)
			_kaba_array_sort<int>(rr, offset, reverse);
		else if (by_type == TypeFloat32)
			_kaba_array_sort<float>(rr, offset, reverse);
		else if (by_type == TypeBool)
			_kaba_array_sort<bool>(rr, offset, reverse);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	}
	return rr;
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

string _cdecl var_repr(const void *p, const Class *type) {
	if (type == TypeInt) {
		return i2s(*(int*)p);
	} else if (type == TypeFloat32) {
		return f2s(*(float*)p, 6);
	} else if (type == TypeFloat64) {
		return f2s((float)*(double*)p, 6);
	} else if (type == TypeBool) {
		return b2s(*(bool*)p);
	} else if (type == TypeClass) {
		return class_repr((Class*)p);
	} else if (type->is_callable_fp() or type->is_callable_bind()) {
		return callable_repr(p, type);
	} else if (type == TypeFunction or type->type == Class::Type::FUNCTION) {
		// probably not...
		return func_repr((Function*)p);
	} else if (type == TypeAny) {
		return ((Any*)p)->repr();
	} else if (type->is_some_pointer()) {
		auto *pp = *(void**)p;
		// auto deref?
		if (pp and (type->param[0] != TypeVoid))
			return var_repr(pp, type->param[0]);
		return p2s(pp);
	} else if (type == TypeString) {
		return ((string*)p)->repr();
	} else if (type == TypeCString) {
		return string((char*)p).repr();
	} else if (type == TypePath) {
		return ((Path*)p)->str().repr();
	} else if (type->is_super_array()) {
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
	} else if (type->elements.num > 0) {
		string s;
		for (auto &e: type->elements) {
			if (e.hidden())
				continue;
			if (s.num > 0)
				s += ", ";
			s += var_repr(((char*)p) + e.offset, e.type);
		}
		return "(" + s + ")";

	} else if (type->is_array()) {
		string s;
		for (int i=0; i<type->array_length; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)p) + i * type->param[0]->size, type->param[0]);
		}
		return "[" + s + "]";
	}
	return d2h(p, type->size);
}

string _cdecl var2str(const void *p, const Class *type) {
	if (type == TypeString)
		return *(string*)p;
	if (type == TypeCString)
		return string((char*)p);
	if (type == TypePath)
		return ((Path*)p)->str();
	if (type == TypeAny)
		return reinterpret_cast<const Any*>(p)->str();
	return var_repr(p, type);
}

Any _cdecl kaba_dyn(const void *var, const Class *type) {
	if (type == TypeInt)
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
			a.add(kaba_dyn((char*)var + t_el->size * i, t_el));
		return a;
	}
	if (type->is_super_array()) {
		Any a = Any::EmptyArray;
		auto *ar = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<ar->num; i++)
			a.add(kaba_dyn((char*)ar->data + ar->element_size * i, t_el));
		return a;
	}
	if (type->is_dict()) {
		Any a = Any::EmptyMap;
		auto *da = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<da->num; i++) {
			string key = *(string*)(((char*)da->data) + i * da->element_size);
			a.map_set(key, kaba_dyn(((char*)da->data) + i * da->element_size + sizeof(string), type->param[0]));
		}
		return a;
	}
	
	// class
	Any a;
	for (auto &e: type->elements) {
		if (!e.hidden())
			a.map_set(e.name, kaba_dyn((char*)var + e.offset, e.type));
	}
	return a;
}

Array<const Class*> func_effective_params(const Function *f);

DynamicArray kaba_xmap(void *fff, DynamicArray *a, const Class *ti, const Class *to) {
	//msg_write("xmap " + ti->long_name() + " -> " + to->long_name());

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

void assert_num_params(Function *f, int n) {
	auto p = func_effective_params(f);
	if (p.num != n)
		kaba_raise_exception(new KabaException("call(): " + i2s(p.num) + " parameters expected, " + i2s(n) + " given"));
}

void assert_return_type(Function *f, const Class *ret) {
	msg_write("TODO check type");
	if (f->literal_return_type != ret)
		kaba_raise_exception(new KabaException("call(): function returns " + f->literal_return_type->long_name() + ", " + ret->long_name() + " required"));
}

void kaba_call0(Function *func) {
	assert_num_params(func, 0);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

void kaba_call1(Function *func, void *p1) {
	assert_num_params(func, 1);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {p1}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

void kaba_call2(Function *func, void *p1, void *p2) {
	assert_num_params(func, 2);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {p1, p2}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

void kaba_call3(Function *func, void *p1, void *p2, void *p3) {
	assert_num_params(func, 3);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {p1, p2, p3}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

#pragma GCC pop_options

	
	
}
