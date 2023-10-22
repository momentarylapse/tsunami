/*
 * future.h
 *
 *  Created on: 18 Oct 2023
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_LIB_FUTURE_H_
#define SRC_LIB_KABA_LIB_FUTURE_H_

#include "../../base/future.h"
#include "../../base/callable.h"
#include "../kaba.h"
#include "lib.h"

namespace kaba {

extern const Class* TypeCallback;


template<class T>
struct KabaFuture : public base::future<T> {
	void __init__() {
//		new(this) KabaFuture<T>();
	}
	void __delete__() {
		this->~KabaFuture<T>();
	}
	void kaba_then(Callable<void(typename base::xparam<T>::t)> &c) {
		this->then([&c] (typename base::xparam<T>::t p) { c(p); });
	}
	void kaba_then_or_fail(Callable<void(typename base::xparam<T>::t)> &c, Callable<void()> &c_fail) {
		this->then([&c] (typename base::xparam<T>::t p) { c(p); }).on_fail([&c_fail] { c_fail(); });
	}
};

struct KabaVoidFuture : public base::future<void> {
	void __init__() {
//		new(this) KabaVoidFuture();
	}
	void __delete__() {
		this->~KabaVoidFuture();
	}
	void kaba_then(Callable<void()> &c) {
		this->then([&c] { c(); });
	}
	void kaba_then_or_fail(Callable<void()> &c, Callable<void()> &c_fail) {
		this->then([&c] { c(); }).on_fail([&c_fail] { c_fail(); });
	}
};

template<class T>
void lib_create_future(const Class *tt, const Class *t_cb) {
	auto t = const_cast<Class*>(tt);

	add_class(t);
		//class_add_func(Identifier::Func::INIT, TypeVoid, &KabaFuture<T>::__init__);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &KabaFuture<T>::__delete__);
		class_add_func("then", TypeVoid, &KabaFuture<T>::kaba_then, Flags::CONST);
			func_add_param("cb", t_cb);
		class_add_func("then_or_fail", TypeVoid, &KabaFuture<T>::kaba_then_or_fail, Flags::CONST);
			func_add_param("cb", t_cb);
			func_add_param("cb_fail", TypeCallback);
}

template<>
void lib_create_future<void>(const Class *tt, const Class *t_cb) {
	auto t = const_cast<Class*>(tt);

	add_class(t);
		//class_add_func(Identifier::Func::INIT, TypeVoid, &KabaVoidFuture::__init__);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &KabaVoidFuture::__delete__);
		class_add_func("then", TypeVoid, &KabaVoidFuture::kaba_then, Flags::CONST);
			func_add_param("cb", t_cb);
		class_add_func("then_or_fail", TypeVoid, &KabaVoidFuture::kaba_then_or_fail, Flags::CONST);
			func_add_param("cb", t_cb);
			func_add_param("cb_fail", TypeCallback);
}

}

#endif /* SRC_LIB_KABA_LIB_FUTURE_H_ */
