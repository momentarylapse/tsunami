/*
 * implicit_future.h
 *
 *  Created on: 21 Oct 2023
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_TEMPLATE_IMPLICIT_FUTURE_H_
#define SRC_LIB_KABA_TEMPLATE_IMPLICIT_FUTURE_H_

#include "implicit.h"

namespace kaba {

class AutoImplementerFutureCore : public AutoImplementer {
public:
	AutoImplementerFutureCore(Parser *p, SyntaxTree *tree) : AutoImplementer(p, tree) {}

	void add_missing_function_headers(Class *t);
	void implement_functions(const Class *t);

	void implement_future_core_constructor(Function *f, const Class *t);

	void complete_type(Class *t);
};

class AutoImplementerFuture : public AutoImplementer {
public:
	AutoImplementerFuture(Parser *p, SyntaxTree *tree) : AutoImplementer(p, tree) {}

	void add_missing_function_headers(Class *t);
	void implement_functions(const Class *t);

	void implement_future_constructor(Function *f, const Class *t);

	void complete_type(Class *t);
};

class AutoImplementerPromise : public AutoImplementer {
public:
	AutoImplementerPromise(Parser *p, SyntaxTree *tree) : AutoImplementer(p, tree) {}

	void add_missing_function_headers(Class *t);
	void implement_functions(const Class *t);

	void implement_promise_constructor(Function *f, const Class *t);

	void complete_type(Class *t);
};

}


#endif /* SRC_LIB_KABA_TEMPLATE_IMPLICIT_FUTURE_H_ */
