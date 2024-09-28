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


extern Module *cur_package;
extern const Class *TypeFutureT;
extern const Class *TypePromiseT;
extern const Class* TypeCallback;

template<class T>
class ParamCallback : public Callable<void(typename base::xparam<T>::t)> {};
template<>
class ParamCallback<void> : public Callable<void()> {};

template<class T>
struct KabaFuture : public base::future<T> {
	using CoreType = typename base::future<T>::P::CoreType;
	KabaFuture() : base::future<T>(new CoreType) {
	}
	void __init__() {
		new(this) KabaFuture();
	}
	void __delete__() {
		this->~KabaFuture<T>();
	}
	void assign(KabaFuture& other) {
		*this = other;
	}
	void kaba_then(ParamCallback<T> &c) {
		if constexpr (std::is_same_v<T, void>)
			this->then([&c] { c(); });
		else
			this->then([&c] (typename base::xparam<T>::t p) { c(p); });
	}
	void kaba_then_or_fail(ParamCallback<T> &c, Callable<void()> &c_fail) {
		if constexpr (std::is_same_v<T, void>)
			this->then([&c] { c(); }).on_fail([&c_fail] { c_fail(); });
		else
			this->then([&c] (typename base::xparam<T>::t p) { c(p); }).on_fail([&c_fail] { c_fail(); });
	}
};

template<class T>
struct KabaPromise : base::promise<T> {
	void __init__() {
		new(this) KabaPromise<T>();
	}
	void __delete__() {
		this->~KabaPromise<T>();
	}
	void assign(KabaPromise& other) {
		*this = other;
	}
};

template<class T>
struct KabaPromiseX : KabaPromise<T> {
	void call(typename base::xparam<T>::t value) {
		(*this)(value);
	}
};

template<>
struct KabaPromiseX<void> : KabaPromise<void> {
	void call() {
		(*this)();
	}
};


template<class T>
void lib_create_future(const Class *tt, const Class *pp, const Class *t_cb) {
	auto t = const_cast<Class*>(tt);
	t->param = {pp};

	add_class(t);
		class_add_func(Identifier::func::Init, TypeVoid, &KabaFuture<T>::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &KabaFuture<T>::__delete__, Flags::Mutable);
		class_add_func(Identifier::func::Assign, TypeVoid, &KabaFuture<T>::assign, Flags::Mutable);
			func_add_param("other", tt);
		class_add_func("then", TypeVoid, &KabaFuture<T>::kaba_then);
			func_add_param("cb", t_cb);
		class_add_func("then_or_fail", TypeVoid, &KabaFuture<T>::kaba_then_or_fail);
			func_add_param("cb", t_cb);
			func_add_param("cb_fail", TypeCallback);

	cur_package->context->template_manager->add_explicit_class_instance(
			cur_package->tree.get(), tt, TypeFutureT, {pp}, 0);
}

template<class T>
void lib_create_promise(const Class *tt, const Class *pp, const Class *tfut) {
	auto t = const_cast<Class*>(tt);
	t->param = {pp};

	add_class(t);
	class_add_func(Identifier::func::Init, TypeVoid, &KabaPromise<T>::__init__, Flags::Mutable);
	class_add_func(Identifier::func::Delete, TypeVoid, &KabaPromise<T>::__delete__, Flags::Mutable);
	class_add_func(Identifier::func::Assign, TypeVoid, &KabaPromise<T>::assign, Flags::Mutable);
	func_add_param("other", tt);
	class_add_func("get_future", tfut, &KabaPromise<T>::get_future);
	if (pp == TypeVoid) {
		class_add_func(Identifier::func::Call, TypeVoid, &KabaPromiseX<T>::call);
	} else {
		class_add_func(Identifier::func::Call, TypeVoid, &KabaPromiseX<T>::call);
		func_add_param("value", pp);
	}
	class_add_func("fail", TypeVoid, &KabaPromiseX<T>::fail);

	cur_package->context->template_manager->add_explicit_class_instance(
			cur_package->tree.get(), tt, TypePromiseT, {pp}, 0);
}

}

#endif /* SRC_LIB_KABA_LIB_FUTURE_H_ */
