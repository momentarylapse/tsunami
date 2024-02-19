/*
 * shared.h
 *
 *  Created on: 22 Feb 2023
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_LIB_SHARED_H_
#define SRC_LIB_KABA_LIB_SHARED_H_

#include "../../base/pointer.h"
#include "../kaba.h"
#include "lib.h"

namespace kaba {

extern const Class* TypeNone;

template<class T>
class XShared : public shared<T> {
public:
	void __init__() {
		new(this) XShared();
	}
	void __assign_raw__(T *p) {
		this->set(p);
	}
	void __assign__(shared<T> p) {
		this->set(p.get());
	}
	static shared<T> create(T *p) {
		return p;
	}
};

inline void lib_create_pointer_xfer(const Class *tt) {
	auto t = const_cast<Class*>(tt);

	add_class(t);
		add_operator(OperatorID::ASSIGN, TypeVoid, t, t, InlineID::POINTER_ASSIGN);
}

template<class T>
void lib_create_pointer_shared(const Class *tt, const Class *t_xfer) {
	auto t = const_cast<Class*>(tt);

	add_class(t);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XShared<T>::__init__, Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &XShared<T>::clear, Flags::MUTABLE);
		class_add_func(Identifier::Func::SHARED_CLEAR, TypeVoid, &XShared<T>::clear, Flags::MUTABLE);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XShared<T>::__assign_raw__, Flags::MUTABLE);
			func_add_param("other", t_xfer);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XShared<T>::__assign_raw__, Flags::MUTABLE);
			func_add_param("other", TypeNone);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XShared<T>::__assign__, Flags::MUTABLE);
			func_add_param("other", t);
		class_add_func(Identifier::Func::SHARED_CREATE, t, &XShared<T>::create, Flags::STATIC);
			func_add_param("p", t_xfer);
}

}

#endif /* SRC_LIB_KABA_LIB_SHARED_H_ */
