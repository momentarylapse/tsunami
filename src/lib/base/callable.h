/*
 * callable.h
 *
 *  Created on: Sep 30, 2021
 *      Author: michi
 */

#pragma once

#include "base.h"

class EmptyCallableError : public Exception {
public:
	EmptyCallableError() : Exception("empty callable") {}
};

template<typename Sig>
class Callable;

template<typename R, typename ...A>
class Callable<R(A...)> {
public:
	typedef R t_func(A...);
	t_func *fp;
	void *pp;

	Callable() {
		fp = nullptr;
		pp = nullptr;
	}

	Callable(t_func *_p) {
		fp = _p;
		pp = nullptr;
	}

	virtual ~Callable() {}

	virtual R operator()(A... args) const {
		if (fp) {
			return fp(args...);
		} else {
			throw EmptyCallableError();
		}
	}
};

/*template<typename R, typename ...A>
class Callable<R(A...)> {
public:
	enum class Type {
		EMPTY,
		FUNCTION_POINTER,
		COMPLEX
	};
	Type type;
	typedef R t_func(A...);
	t_func *p;

	Callable() {
		type = Type::EMPTY;
		p = nullptr;
	}

	Callable(t_func *_p) {
		type = Type::FUNCTION_POINTER;
		p = _p;
	}

	virtual ~Callable() {}

	virtual R call_complex(A... args) {}

	R operator()(A... args) {
		if (type == Type::FUNCTION_POINTER) {
			return p(args...);
		} else if (type == Type::COMPLEX) {
			return call_complex(args...);
		} else {
			throw EmptyCallableError();
		}
	}
};*/

