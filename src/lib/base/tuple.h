/*
 * tuple.h
 *
 *  Created on: Oct 16, 2020
 *      Author: michi
 */

#once


template<class A, class B>
class tuple {
public:
	tuple(const A &_a, const B &_b) {
		a = _a;
		b = _b;
	}
	A a;
	B b;
};



