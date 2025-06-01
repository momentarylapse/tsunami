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

}

