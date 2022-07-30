/*
 * operators.h
 *
 *  Created on: 30 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_LIB_OPERATORS_H_
#define SRC_LIB_KABA_LIB_OPERATORS_H_

#include "../../base/base.h"
#include <cmath>


namespace kaba {


template<class T>
T xop_exp(T a, T b) {
	if constexpr (std::is_same<T, int>::value)
		return (int)pow((double)a, (double)b);
	else
		return pow(a, b);
}



#define MAKE_OP_FOR(T) \
	T op_##T##_add(T a, T b) { return a + b; } \
	T op_##T##_sub(T a, T b) { return a - b; } \
	T op_##T##_mul(T a, T b) { return a * b; } \
	T op_##T##_div(T a, T b) { return a / b; } \
	T op_##T##_neg(T a) { return - a; } \
	bool op_##T##_eq(T a, T b) { return a == b; } \
	bool op_##T##_neq(T a, T b) { return a != b; } \
	bool op_##T##_l(T a, T b) { return a < b; } \
	bool op_##T##_le(T a, T b) { return a <= b; } \
	bool op_##T##_g(T a, T b) { return a > b; } \
	bool op_##T##_ge(T a, T b) { return a >= b; }


// T[] += T[]
#define IMPLEMENT_IOP_LIST(OP, TYPE) \
{ \
	int n = ::min(this->num, b.num); \
	TYPE *pa = (TYPE*)this->data; \
	TYPE *pb = (TYPE*)b.data; \
	for (int i=0;i<n;i++) \
		*(pa ++) OP *(pb ++); \
}

// T[] += x
#define IMPLEMENT_IOP_LIST_SCALAR(OP, TYPE) \
{ \
	TYPE *pa = (TYPE*)this->data; \
	for (int i=0;i<this->num;i++) \
		*(pa ++) OP x; \
}


// R[] = T[] + T[]
#define IMPLEMENT_OP_LIST(OP, TYPE, RETURN) \
{ \
	int n = ::min(this->num, b.num); \
	Array<RETURN> r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)this->data; \
	TYPE *pb = (TYPE*)b.data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<n;i++) \
		*(pr ++) = *(pa ++) OP *(pb ++); \
	return r; \
}
// R[] = T[] + x
#define IMPLEMENT_OP_LIST_SCALAR(OP, TYPE, RETURN) \
{ \
	Array<RETURN> r; \
	r.resize(this->num); \
	TYPE *pa = (TYPE*)this->data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<this->num;i++) \
		*(pr ++) = *(pa ++) OP x; \
	return r; \
}
// R[] = F(T[], T[])
#define IMPLEMENT_OP_LIST_FUNC(F, TYPE, RETURN) \
{ \
	int n = ::min(this->num, b.num); \
	Array<RETURN> r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)this->data; \
	TYPE *pb = (TYPE*)b.data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<n;i++) \
		*(pr ++) = F(*(pa ++), *(pb ++)); \
	return r; \
}

// R[] = F(T[], x)
#define IMPLEMENT_OP_LIST_SCALAR_FUNC(F, TYPE, RETURN) \
{ \
	Array<RETURN> r; \
	r.resize(this->num); \
	TYPE *pa = (TYPE*)this->data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<this->num;i++) \
		*(pr ++) = F(*(pa ++), x); \
	return r; \
}


}


#endif /* SRC_LIB_KABA_LIB_OPERATORS_H_ */
