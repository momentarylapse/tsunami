/*
 * list.h
 *
 *  Created on: 30 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_LIB_LIST_H_
#define SRC_LIB_KABA_LIB_LIST_H_

#include "../../base/base.h"
#include "../../os/msg.h"
#include "lib.h"
#include "operators.h"

namespace kaba {

extern const Class *TypeAny;
extern const Class *TypeDynamicArray;
extern const Class *TypePath;

void kaba_make_super_array(Class *t, SyntaxTree *ps);



template<class T>
class XList : public Array<T> {
public:
	void _cdecl __init__() {
		new(this) Array<T>();
	}

	static T sum(const Array<T> &list) {
		T r = 0;
		for (int i=0; i<list.num; i++)
			r += list[i];
		return r;
	}
	static T sum_sqr(const Array<T> &list) {
		T r = 0;
		for (int i=0;i<list.num;i++)
			r += list[i] * list[i];
		return r;
	}

	static Array<T> unique(const Array<T> &list) {
		int ndiff = 0;
		int i0 = 1;
		Array<T> r = list;
		while (r[i0] != r[i0-1])
			i0 ++;
		for (int i=i0; i<r.num;i++) {
			if (r[i] == r[i-1])
				ndiff ++;
			else
				r[i - ndiff] = r[i];
		}
		r.resize(r.num - ndiff);
		return r;
	}
	bool __contains__(T v) const {
		for (int i=0;i<this->num;i++)
			if ((*this)[i] == v)
				return true;
		return false;
	}

	static T min(const XList<T> &a) {
		if (a.num == 0)
			return 0;
		return a[argmin(a)];
	}
	static T max(const XList<T> &a) {
		if (a.num == 0)
			return 0;
		return a[argmax(a)];
	}

	static int argmin(const XList<T> &a) {
		if (a.num == 0)
			return -1;
		int best = 0;
		for (int i=1; i<a.num; i++)
			if (a[i] < a[best])
				best = i;
		return best;
	}

	static int argmax(const XList<T> &a) {
		if (a.num == 0)
			return -1;
		int best = 0;
		for (int i=1; i<a.num; i++)
			if (a[i] > a[best])
				best = i;
		return best;
	}

	string str() const {
		if constexpr (std::is_same<T, int>::value)
			return ia2s(*this);
		else if constexpr (std::is_same<T, float>::value)
			return fa2s(*this);
		else if constexpr (std::is_same<T, double>::value)
			return f64a2s(*this);
		else if constexpr (std::is_same<T, bool>::value)
				return ba2s(*this);
		else if constexpr (std::is_same<T, string>::value)
				return sa2s(*this);
		else
			return "[???]";
	}

// component-wise operations:

	// a += b
	void _cdecl iadd_values(XList<T> &b)	IMPLEMENT_IOP_LIST(+=, T)
	void _cdecl isub_values(XList<T> &b)	IMPLEMENT_IOP_LIST(-=, T)
	void _cdecl imul_values(XList<T> &b)	IMPLEMENT_IOP_LIST(*=, T)
	void _cdecl idiv_values(XList<T> &b)	IMPLEMENT_IOP_LIST(/=, T)

	// a = b + c
	Array<T> _cdecl add_values(XList<T> &b)	IMPLEMENT_OP_LIST(+, T, T)
	Array<T> _cdecl sub_values(XList<T> &b)	IMPLEMENT_OP_LIST(-, T, T)
	Array<T> _cdecl mul_values(XList<T> &b)	IMPLEMENT_OP_LIST(*, T, T)
	Array<T> _cdecl div_values(XList<T> &b)	IMPLEMENT_OP_LIST(/, T, T)
	Array<T> _cdecl exp_values(XList<T> &b)	IMPLEMENT_OP_LIST_FUNC(xop_exp<T>, T, T)

	// a += x
	void _cdecl iadd_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(+=, T)
	void _cdecl isub_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(-=, T)
	void _cdecl imul_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(*=, T)
	void _cdecl idiv_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(/=, T)
	void _cdecl assign_values_scalar(T x)	IMPLEMENT_IOP_LIST_SCALAR(=, T)

	// a = b + x
	Array<T> _cdecl add_values_scalar(T x)	IMPLEMENT_OP_LIST_SCALAR(+, T, T)
	Array<T> _cdecl sub_values_scalar(T x)	IMPLEMENT_OP_LIST_SCALAR(-, T, T)
	Array<T> _cdecl mul_values_scalar(T x)	IMPLEMENT_OP_LIST_SCALAR(*, T, T)
	Array<T> _cdecl div_values_scalar(T x)	IMPLEMENT_OP_LIST_SCALAR(/, T, T)
	Array<T> _cdecl exp_values_scalar(T x)	IMPLEMENT_OP_LIST_SCALAR_FUNC(xop_exp<T>, T, T)

	// a <= b
	Array<bool> _cdecl lt_values(XList<T> &b) IMPLEMENT_OP_LIST(<, T, bool)
	Array<bool> _cdecl le_values(XList<T> &b) IMPLEMENT_OP_LIST(<=, T, bool)
	Array<bool> _cdecl gt_values(XList<T> &b) IMPLEMENT_OP_LIST(>, T, bool)
	Array<bool> _cdecl ge_values(XList<T> &b) IMPLEMENT_OP_LIST(>=, T, bool)
	Array<bool> _cdecl eq_values(XList<T> &b) IMPLEMENT_OP_LIST(==, T, bool)
	Array<bool> _cdecl ne_values(XList<T> &b) IMPLEMENT_OP_LIST(!=, T, bool)

	// a <= x
	Array<bool> _cdecl lt_values_scalar(T x) IMPLEMENT_OP_LIST_SCALAR(<, T, bool)
	Array<bool> _cdecl le_values_scalar(T x) IMPLEMENT_OP_LIST_SCALAR(<=, T, bool)
	Array<bool> _cdecl gt_values_scalar(T x) IMPLEMENT_OP_LIST_SCALAR(>, T, bool)
	Array<bool> _cdecl ge_values_scalar(T x) IMPLEMENT_OP_LIST_SCALAR(>=, T, bool)
	Array<bool> _cdecl eq_values_scalar(T x) IMPLEMENT_OP_LIST_SCALAR(==, T, bool)
	Array<bool> _cdecl ne_values_scalar(T x) IMPLEMENT_OP_LIST_SCALAR(!=, T, bool)
};

class BoolList : public XList<bool> {
public:
	// a = b + c
	Array<bool> _cdecl and_values(BoolList &b)	IMPLEMENT_OP_LIST(and, bool, bool)
	Array<bool> _cdecl or_values(BoolList &b)	IMPLEMENT_OP_LIST(or, bool, bool)
	Array<bool> _cdecl eq_values(BoolList &b)	IMPLEMENT_OP_LIST(==, bool, bool)
	Array<bool> _cdecl ne_values(BoolList &b)	IMPLEMENT_OP_LIST(!=, bool, bool)
	// a = b + x
	Array<bool> _cdecl and_values_scalar(bool x)	IMPLEMENT_OP_LIST_SCALAR(and, bool, bool)
	Array<bool> _cdecl or_values_scalar(bool x)	IMPLEMENT_OP_LIST_SCALAR(or, bool, bool)
	Array<bool> _cdecl eq_values_scalar(bool x)	IMPLEMENT_OP_LIST_SCALAR(==, bool, bool)
	Array<bool> _cdecl ne_values_scalar(bool x)	IMPLEMENT_OP_LIST_SCALAR(!=, bool, bool)

	//Array<bool> _cdecl _not(BoolList &b)	IMPLEMENT_OP_LIST(!, bool, bool)

	bool all() const {
		for (auto &b: *this)
			if (!b)
				return false;
		return true;
	}
	bool any() const {
		for (auto &b: *this)
			if (b)
				return true;
		return false;
	}
};




//using FloatList = XList<float>;
//using Float64List = XList<double>;

}


#endif /* SRC_LIB_KABA_LIB_LIST_H_ */
