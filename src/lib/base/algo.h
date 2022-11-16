/*
 * algo.h
 *
 *  Created on: 21 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_ALGO_H_
#define SRC_LIB_BASE_ALGO_H_

#include "array.h"

namespace base {

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

template<class T>
void fill(Array<T> &array, const T &t) {
	for (T &e: array)
		e = t;
}

template<class F>
struct filter {
	const F &f;
	filter(const F& _f) : f(_f) {}
	template<class T>
	friend Array<T> operator>>(const Array<T> &array, const filter<F> &t) {
		Array<T> r;
		for (T &e: array)
			if (t.f(e))
				r.add(e);
		return r;
	}
};

template<class F>
struct transform {
	const F &f;
	transform(const F& _f) : f(_f) {}
	template<class T>
	friend auto operator>>(const Array<T> &array, const transform<F> &t) -> Array<decltype(t.f(T()))> {
		Array<decltype(t.f(T()))> r;
		for (T &e: array)
			r.add(t.f(e));
		return r;
	}
};

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

template<class T, class F>
void remove_if(Array<T> &array, F f) {
	for (int i=0; i<array.num; i++)
		if (f(array[i])) {
			array.erase(i);
			i --;
		}
}

template<class T>
void remove(Array<T> &array, const T &x) {
	for (int i=0; i<array.num; i++)
		if (array[i] == x) {
			array.erase(i);
			i --;
		}
}

}

#endif /* SRC_LIB_BASE_ALGO_H_ */
