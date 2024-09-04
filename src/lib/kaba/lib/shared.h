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
		add_operator(OperatorID::Assign, TypeVoid, t, t, InlineID::PointerAssign);
}

template<class T>
void lib_create_pointer_shared(const Class *tt, const Class *t_xfer) {
	auto t = const_cast<Class*>(tt);

	add_class(t);
		class_add_func(Identifier::func::Init, TypeVoid, &XShared<T>::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &XShared<T>::clear, Flags::Mutable);
		class_add_func(Identifier::func::SharedClear, TypeVoid, &XShared<T>::clear, Flags::Mutable);
		class_add_func(Identifier::func::Assign, TypeVoid, &XShared<T>::__assign_raw__, Flags::Mutable);
			func_add_param("other", t_xfer);
		class_add_func(Identifier::func::Assign, TypeVoid, &XShared<T>::__assign_raw__, Flags::Mutable);
			func_add_param("other", TypeNone);
		class_add_func(Identifier::func::Assign, TypeVoid, &XShared<T>::__assign__, Flags::Mutable);
			func_add_param("other", t);
		class_add_func(Identifier::func::SharedCreate, t, &XShared<T>::create, Flags::Static);
			func_add_param("p", t_xfer);
}

}

#endif /* SRC_LIB_KABA_LIB_SHARED_H_ */
