/*
 * tuple.h
 *
 *  Created on: Oct 16, 2020
 *      Author: michi
 */

#pragma once

namespace base {

template<class A, class B>
struct tuple {
	tuple() = default;
	tuple(const A &_a, const B &_b) {
		a = _a;
		b = _b;
	}
	tuple(const tuple<A,B>& t) {
		a = t.a;
		b = t.b;
	}
	void operator=(const tuple<A,B>& t) {
		a = t.a;
		b = t.b;
	}
	A a;
	B b;
};


template<class A, class B, class C>
struct tuple3 {
	tuple3() = default;
	tuple3(const A& _a, const B& _b, const C& _c) {
		a = _a;
		b = _b;
		c = _c;
	}
	tuple3(const tuple3<A,B,C>& t) {
		a = t.a;
		b = t.b;
		c = t.c;
	}
	void operator=(const tuple3<A,B,C>& t) {
		a = t.a;
		b = t.b;
		c = t.c;
	}
	A a;
	B b;
	C c;
};

/*template<class T, class... More>
struct tuple {
	TODO
};*/

}

