#include "dynamic.h"
#include "../kaba.h"
#include "exception.h"
#include "call.h"
#include "../../any/any.h"
#include "../../base/callable.h"
#include "../../base/sort.h"
#include "../../os/msg.h"

namespace kaba {
	
extern const Class *TypeInt32List;
extern const Class *TypeFloat32List;
extern const Class *TypeBoolList;
extern const Class *TypeAny;
extern const Class *TypePath;



KABA_LINK_GROUP_BEGIN


void array_inplace_reverse(DynamicArray &array) {
	if (array.element_size == 4)
		base::inplace_reverse(*reinterpret_cast<Array<int>*>(&array));
	else if (array.element_size == 1)
		base::inplace_reverse(*reinterpret_cast<Array<char>*>(&array));
	else if (array.element_size == 8)
		base::inplace_reverse(*reinterpret_cast<Array<int64>*>(&array));
	else
		array.reverse();
}

template<class F>
void array_inplace_quick_sort(DynamicArray &array, int first, int last, F f) {
	if (first < 0 or last < 0 or first >= last)
		return;
	auto partition = [first, last] (DynamicArray &array, F f) {
		int ipivot = (first + last) / 2;
		int left = first-1;
		int right = last+1;
		while (true) {
			void *pivot = array.simple_element(ipivot);
			if (left != ipivot)
				left ++;
			if (right != ipivot)
				right --;
			while (!f(pivot, array.simple_element(left))) // A < P  <=>  !(P <= A)
				left ++;
			while (!f(array.simple_element(right), pivot)) // A > P  <=>  !(A <= P)
				right --;
			if (left >= right)
				return right;
			array.simple_swap(left, right);
			if (left == ipivot)
				ipivot = right;
			else if (right == ipivot)
				ipivot = left;
		}
		return 0;
	};
	int p = partition(array, f);
	array_inplace_quick_sort(array, first, p-1, f);
	array_inplace_quick_sort(array, p+1, last, f);
}

template<class F>
void array_inplace_bubble_sort(DynamicArray &array, F f) {
	bool more = true;
	while (more) {
		more = false;
		for (int i=1; i<array.num; i++) {
			const auto pa = array.simple_element(i-1);
			const auto pb = array.simple_element(i);
			if (!f(pa, pb)) {
				array.simple_swap(i-1, i);
				more = true;
			}
		}
	}
}

template<class T>
void _array_sort(DynamicArray &array, int offset_by, bool stable) {
	auto f = [offset_by] (const void *a, const void *b) {
		return *(T*)((const char*)a + offset_by) <= *(T*)((const char*)b + offset_by);
	};
	if (stable)
		array_inplace_bubble_sort(array, f);
	else
		array_inplace_quick_sort(array, 0, array.num-1, f);
}

template<class T>
void _array_sort_p(DynamicArray &array, int offset_by, bool stable) {
	auto f = [offset_by] (const void *a, const void *b) {
		auto *aa = (const T*) ((const char*)a + offset_by);
		auto *bb = (const T*) ((const char*)b + offset_by);
		return (*aa <= *bb);
	};
	if (stable)
		base::inplace_bubble_sort(*(Array<void*>*)&array, f);
	else
		base::inplace_sort(*(Array<void*>*)&array, f);
}

template<class T>
void _array_sort_pf(DynamicArray &array, Function *func, bool stable) {
	auto f = [func] (void *a, void *b) {
		T r1{}, r2{};
		if (!call_function(func, &r1, {a}) or !call_function(func, &r2, {b}))
			kaba_raise_exception(new KabaException("call failed " + func->long_name()));
		return (r1 <= r2);
	};
	if (stable)
		base::inplace_bubble_sort(*(Array<void*>*)&array, f);
	else
		base::inplace_sort(*(Array<void*>*)&array, f);
}

DynamicArray _cdecl array_sort(DynamicArray &array, const Class *type, const string &_by) {
	if (!type->is_list())
		kaba_raise_exception(new KabaException(format("type '%s' is not an array", type->name)));
	const Class *el = type->param[0];
	if (array.element_size != el->size)
		kaba_raise_exception(new KabaException(format("element type size mismatch... type=%s: %d  vs  array: %d", el->name, el->size, array.element_size)));

	DynamicArray rr;
	var_init(&rr, type);
	var_assign(&rr, &array, type);

	const Class *rel = el;

	if (el->is_some_pointer())
		rel = el->param[0];

	string by = _by;
	bool stable = (by.find("!") >= 0);
	by = by.replace("!", "");
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
					if (f->num_params != 1)
						kaba_raise_exception(new KabaException("can only sort by a member function without parameters"));
					by_type = f->literal_return_type;
					sfunc = const_cast<Function*>(f);
				}
			if (!sfunc)
				kaba_raise_exception(new KabaException("type '" + rel->name + "' does not have an element '" + by + "'"));
		}
	}

	if (sfunc) {
		if (!el->is_some_pointer())
			kaba_raise_exception(new KabaException("function sorting only for pointers"));
		if (by_type == TypeString)
			_array_sort_pf<string>(rr, sfunc, stable);
		else if (by_type == TypePath)
			_array_sort_pf<Path>(rr, sfunc, stable);
		else if (by_type == TypeInt32)
			_array_sort_pf<int>(rr, sfunc, stable);
		else if (by_type == TypeFloat32)
			_array_sort_pf<float>(rr, sfunc, stable);
		else if (by_type == TypeBool)
			_array_sort_pf<bool>(rr, sfunc, stable);
		else
			kaba_raise_exception(new KabaException("can't sort by function '" + by_type->long_name() + "' yet"));
	} else if (el->is_some_pointer()) {
		if (by_type == TypeString)
			_array_sort_p<string>(rr, offset, stable);
		else if (by_type == TypePath)
			_array_sort_p<Path>(rr, offset, stable);
		else if (by_type == TypeInt32)
			_array_sort_p<int>(rr, offset, stable);
		else if (by_type == TypeFloat32)
			_array_sort_p<float>(rr, offset, stable);
		else if (by_type == TypeBool)
			_array_sort_p<bool>(rr, offset, stable);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	} else {
		if (by_type == TypeString)
			_array_sort<string>(rr, offset, stable);
		else if (by_type == TypePath)
			_array_sort<Path>(rr, offset, stable);
		else if (by_type == TypeInt32)
			_array_sort<int>(rr, offset, stable);
		else if (by_type == TypeFloat32)
			_array_sort<float>(rr, offset, stable);
		else if (by_type == TypeBool)
			_array_sort<bool>(rr, offset, stable);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	}
	if (reverse)
		array_inplace_reverse(rr);
	return rr;
}


KABA_LINK_GROUP_END

	
	
}
