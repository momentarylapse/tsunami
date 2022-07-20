/*
 * algo.h
 *
 *  Created on: 21 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_ALGO_H_
#define SRC_LIB_BASE_ALGO_H_

#include "array.h"
#include <functional>

template<class T, class F>
T *find_if(const Array<T> &array, F f) {
	for (auto &e: array)
		if (f(e))
			return &e;
	return nullptr;
}

template<class T>
T *find(const Array<T> &array, const T &t) {
	return find_if(array, [&t] (const T &v) { return v == t; });
}

template<class T, class E, class V>
T *find_by_element(const Array<T> &array, E e, const V &t) {
	return find_if(array, [&t, e] (const T &v) { return v.*e == t; });
}

template<class T, class F>
int find_index_if(const Array<T> &array, F f) {
	for (int i=0; i<array.num; i++)
		if (f(array[i]))
			return i;
	return -1;
}

template<class T>
int find_index(const Array<T> &array, const T &t) {
	return find_index_if(array, [&t] (const T &v) { return v == t; });
}

template<class T, class F>
int count_if(const Array<T> &array, F f) {
	int n = 0;
	for (auto &e: array)
		if (f(e))
			n ++;
	return n;
}

template<class T>
int count(const Array<T> &array, const T &t) {
	return count_if(array, [&t] (const T &v) { return v == t; });
}

template<class T, class F>
void replace_if(Array<T> &array, F f, const T &by) {
	for (T &e: array)
		if (f(e))
			e = by;
}

template<class T>
void replace(Array<T> &array, const T &t, const T &by) {
	replace_if(array, [&t] (const T &v) { return v == t; }, by);
}

template<class T>
void fill(Array<T> &array, const T &t) {
	for (T &e: array)
		e = t;
}

template<class T, class F>
Array<T> filter(const Array<T> &array, F f) {
	Array<T> r;
	for (T &e: array)
		if (f(e))
			r.add(e);
	return r;
}

template<class T, class F, class U>
Array<U> transform(const Array<T> &array, F f) {
	Array<U> r;
	for (T &e: array)
		r.add(f(e));
	return r;
}

template<class T>
void inplace_reverse(Array<T> &array) {
	for (int i=0; i<array.num/2; i++)
		array.swap(i, array.num - i - 1);
}

template<class T>
Array<T> reverse(const Array<T> &array) {
	Array<T> r = array;
	inplace_reverse(r);
	return r;
}

template<class T, class F>
void inplace_bubble_sort(Array<T> &array, F f) {
	for (int i=0; i<array.num; i++)
		for (int j=i+1; j<array.num; j++)
			if (!f(array[i], array[j]))
				array.swap(i, j);
}

#include "../os/msg.h"
#define DEBUG_SORT 0

template<class T, class F>
void _inplace_quick_sort_step(Array<T> &array, int first, int last, F f) {
	if (first < 0 or last < 0 or first >= last)
		return;

	auto partition = [first, last] (Array<T> &array, F f) {
		int ipivot = (first + last) / 2;
		T pivot = array[ipivot];
#if DEBUG_SORT>=2
		if constexpr (std::is_same<T, int>::value)
			msg_write("  piv  " + i2s(pivot));
#endif
		int left = first -1;
		int right = last + 1;
		while (true) {
			if (left != ipivot)
				left ++;
			if (right != ipivot)
				right --;
			while (!f(pivot, array[left])) // A < P  <=>  !(P <= A)
				left ++;
			while (!f(array[right], pivot)) // A > P  <=>  !(A <= P)
				right --;
#if DEBUG_SORT>=2
			msg_write(format("   %d:%d", left-first, right-first));
#endif
			if (left >= right)
				return right;
			array.swap(left, right);
#if DEBUG_SORT>=2
			if constexpr (std::is_same<T, int>::value)
				msg_write("  swap  " + ia2s(array.sub_ref(first, last+1)));
#endif
			if (left == ipivot)
				ipivot = right;
			else if (right == ipivot)
				ipivot = left;
		}
		return 0;
	};
#if DEBUG_SORT>=2
	if constexpr (std::is_same<T, int>::value) {
		msg_write("q  " + ia2s(array.sub_ref(first, last+1)));
	}
	msg_right();
#endif
	int p = partition(array, f);
#if DEBUG_SORT>=2
	msg_write("  -> " + i2s(p-first));
#endif
#if DEBUG_SORT>=1
	if constexpr (std::is_same<T, int>::value) {
		for (int i=first; i<p; i++)
			if (!f(array[i], array[p]))
				throw(Exception("p-fail  " + ia2s(array) + "   " + i2s(p)));
		for (int i=p+1; i<=last; i++)
			if (!f(array[p], array[i]))
				throw(Exception("p-fail  " + ia2s(array) + "   " + i2s(p)));
	}
#endif
	//if (p > first + 1)
		_inplace_quick_sort_step(array, first, p-1, f);
	//if (p >= first)
		_inplace_quick_sort_step(array, p + 1, last, f);
#if DEBUG_SORT>=1
	if constexpr (std::is_same<T, int>::value) {
		auto A = array.sub_ref(first, last+1);
#if DEBUG_SORT>=2
		msg_write(" => " + ia2s(A));
#endif
		auto B = A;
		inplace_bubble_sort(B, f);
		if (A != B)
			throw(Exception("failed"));
	}
#endif
#if DEBUG_SORT>=2
	msg_left();
#endif
}


template<class T, class F>
void inplace_quick_sort(Array<T> &array, F f) {
	_inplace_quick_sort_step(array, 0, array.num - 1, f);
}

template<class T, class F>
void inplace_sort(Array<T> &array, F f /*= [](const T &a, const T &b) { return a <= b; }*/) {
	if (array.num <= 8) {
		// small
		inplace_bubble_sort(array, f);
	} else {
		// large
		inplace_quick_sort(array, f);
	}
}

template<class T, class F>
Array<T> sorted(const Array<T> &array, F f /*= [](const T &a, const T &b) { return a <= b; }*/) {
	auto r = array;
	inplace_sort(r, f);
	return r;
}

#endif /* SRC_LIB_BASE_ALGO_H_ */
