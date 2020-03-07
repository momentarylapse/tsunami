#include "dynamic.h"
#include "../kaba.h"
#include "common.h"
#include "exception.h"
#include "../../any/any.h"

namespace Kaba {
	
extern const Class *TypeIntList;
extern const Class *TypeFloatList;
extern const Class *TypeBoolList;
extern const Class *TypeAny;
	
bool call_function(Function *f, void *ff, void *ret, const Array<void*> &param);

	
	

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

template<class T>
void _kaba_array_sort(DynamicArray &array, int offset_by) {
	T *p = (T*)((char*)array.data + offset_by);
	for (int i=0; i<array.num; i++) {
		T *q = (T*)((char*)p + array.element_size);
		for (int j=i+1; j<array.num; j++) {
			if (*p > *q)
				array.simple_swap(i, j);
			q = (T*)((char*)q + array.element_size);
		}
		p = (T*)((char*)p + array.element_size);
	}
}

template<class T>
void _kaba_array_sort_p(DynamicArray &array, int offset_by) {
	char **p = (char**)array.data;
	for (int i=0; i<array.num; i++) {
		T *pp = (T*)(*p + offset_by);
		char **q = p + 1;
		for (int j=i+1; j<array.num; j++) {
			T *qq = (T*)(*q + offset_by);
			if (*pp > *qq){
				array.simple_swap(i, j);
				pp = (T*)(*p + offset_by);
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
		auto *f = type->get_func("add", TypeVoid, {type->param});
		if (!f)
			kaba_raise_exception(new KabaException("can not add to array type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(&array, p);
	}
}

DynamicArray _cdecl kaba_array_sort(DynamicArray &array, const Class *type, const string &by) {
	if (!type->is_super_array())
		kaba_raise_exception(new KabaException("type '" + type->name + "' is not an array"));
	const Class *el = type->param;
	if (array.element_size != el->size)
		kaba_raise_exception(new KabaException("element type size mismatch..."));

	DynamicArray rr;
	kaba_var_init(&rr, type);
	kaba_var_assign(&rr, &array, type);

	const Class *rel = el;

	if (el->is_pointer())
		rel = el->param;

	int offset = -1;
	const Class *by_type = nullptr;
	if (by == "") {
		offset = 0;
		by_type = rel;
	} else {
		for (auto &e: rel->elements)
			if (e.name == by) {
				by_type = e.type;
				offset = e.offset;
			}
		if (!by_type)
			kaba_raise_exception(new KabaException("type '" + rel->name + "' does not have an element '" + by + "'"));
	}


	if (el->is_pointer()) {
		if (by_type == TypeString)
			_kaba_array_sort_p<string>(rr, offset);
		else if (by_type == TypeInt)
			_kaba_array_sort_p<int>(rr, offset);
		else if (by_type == TypeFloat32)
			_kaba_array_sort_p<float>(rr, offset);
		else if (by_type == TypeBool)
			_kaba_array_sort_p<bool>(rr, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	} else {
		if (by_type == TypeString)
			_kaba_array_sort<string>(rr, offset);
		else if (by_type == TypeInt)
			_kaba_array_sort<int>(rr, offset);
		else if (by_type == TypeFloat32)
			_kaba_array_sort<float>(rr, offset);
		else if (by_type == TypeBool)
			_kaba_array_sort<bool>(rr, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	}
	return rr;
}

string class_repr(const Class *c) {
	if (c)
		return "<class " + c->long_name() + ">";
	return "<class -nil->";
}

string func_repr(const Function *f) {
	if (f)
		return "<func " + f->long_name() + ">";
	return "<func -nil->";
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
	} else if (type == TypeFunction) {
		return func_repr((Function*)p);
	} else if (type == TypeAny) {
		return ((Any*)p)->repr();
	} else if (type->is_pointer()) {
		auto *pp = *(void**)p;
		// auto deref?
		if (pp and (type->param != TypeVoid))
			return var_repr(pp, type->param);
		return p2s(pp);
	} else if (type == TypeString) {
		return ((string*)p)->repr();
	} else if (type == TypeCString) {
		return string((char*)p).repr();
	} else if (type->is_super_array()) {
		string s;
		auto *da = reinterpret_cast<const DynamicArray*>(p);
		for (int i=0; i<da->num; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)da->data) + i * da->element_size, type->param);
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
			s += var_repr(((char*)da->data) + i * da->element_size + sizeof(string), type->param);
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
			s += var_repr(((char*)p) + i * type->param->size, type->param);
		}
		return "[" + s + "]";
	}
	return d2h(p, type->size, false);
}

string _cdecl var2str(const void *p, const Class *type) {
	if (type == TypeString)
		return *(string*)p;
	if (type == TypeCString)
		return string((char*)p);
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
	if (type->is_pointer())
		return Any(*(void**)var);
	if (type == TypeAny)
		return *(Any*)var;
	if (type->is_array()) {
		Any a = EmptyArray;
		auto *t_el = type->get_array_element();
		for (int i=0; i<type->array_length; i++)
			a.add(kaba_dyn((char*)var + t_el->size * i, t_el));
		return a;
	}
	if (type->is_super_array()) {
		Any a = EmptyArray;
		auto *ar = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<ar->num; i++)
			a.add(kaba_dyn((char*)ar->data + ar->element_size * i, t_el));
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

DynamicArray kaba_map(Function *func, DynamicArray *a) {
	DynamicArray r;
	auto p = func_effective_params(func);
	if (p.num != 1)
		kaba_raise_exception(new KabaException("map(): only functions with exactly 1 parameter allowed"));
	auto *ti = p[0];
	auto *to = func->literal_return_type;
	r.init(to->size);
	if (to->needs_constructor()) {
		if (to == TypeString) {
			((string*)&r)->resize(a->num);
		} else  {
			kaba_raise_exception(new KabaException("map(): output type not allowed: " + to->long_name()));
		}
	} else {
		r.simple_resize(a->num);
	}
	for (int i=0; i<a->num; i++) {
		void *po = (char*)r.data + to->size * i;
		void *pi = (char*)a->data + ti->size * i;
		bool ok = call_function(func, func->address, po, {pi});
		if (!ok)
			kaba_raise_exception(new KabaException("map(): failed to dynamically call " + func->signature()));
	}
	return r;
}

#pragma GCC pop_options

	
	
}
