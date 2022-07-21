/*
 * sort.h
 *
 *  Created on: 21 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_SORT_H_
#define SRC_LIB_BASE_SORT_H_


#include "array.h"

#define DEBUG_SORT 0
#if DEBUG_SORT > 0
	#include "../os/msg.h"
#endif


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


#endif /* SRC_LIB_BASE_SORT_H_ */
