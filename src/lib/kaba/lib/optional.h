/*
 * optional.h
 *
 *  Created on: 26 Feb 2023
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_LIB_OPTIONAL_H_
#define SRC_LIB_KABA_LIB_OPTIONAL_H_


#include "../../base/optional.h"
#include "../../os/msg.h"
#include "../kaba.h"
#include "lib.h"
#include "../dynamic/exception.h"

namespace kaba {

	extern const Class *TypeNone;

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


template<class T>
class XOptional : public base::optional<T> {
public:
	void __init__() {
		new(this) XOptional();
	}
	static void __init_raw__(base::optional<T> &self, T x) {
		new(&self) base::optional(x);
	}
	void __init_nil__(void* p) {
		new(this) XOptional();
	}
	void __delete__() {
		this->XOptional<T>::~XOptional();
	}
	T _value() const {
		try {
			return this->value();
		} catch(::Exception &e) {
			kaba_raise_exception(new KabaException(e.message()));
			return T();
		}
	}
	void __assign__(const base::optional<T>& o) {
		*((base::optional<T>*)this) = o;
	}
	void __assign_raw__(const T o) {
		*((base::optional<T>*)this) = o;
	}
	void __assign_nil__(void *p) {
		this->clear();
	}
	bool __equal__(const base::optional<T>& o) const {
		return *((base::optional<T>*)this) == o;
	}
	/*bool __equal__(const base::optional<T>& o) const {
		return *((base::optional<T>*)this) == o;
	}*/
};

#pragma GCC pop_options

template<class T>
void lib_create_optional(const Class *tt) {
	auto t = const_cast<Class*>(tt);

	add_class(t);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XOptional<T>::__init__);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XOptional<T>::__init_raw__, Flags::AUTO_CAST);
			func_add_param("x", tt->param[0]);
		class_add_func(Identifier::Func::INIT, TypeVoid, &XOptional<T>::__init_nil__, Flags::AUTO_CAST);
			func_add_param("x", TypeNone);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &XOptional<T>::__delete__);
		class_add_func(Identifier::Func::OPTIONAL_HAS_VALUE, TypeBool, &XOptional<T>::has_value, Flags::PURE);
		class_add_func("__bool__", TypeBool, &XOptional<T>::has_value, Flags::PURE);
		class_add_func("_value", tt->param[0], &XOptional<T>::_value, Flags::REF | Flags::RAISES_EXCEPTIONS);

		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XOptional<T>::__assign__);
			func_add_param("x", tt);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XOptional<T>::__assign_raw__);
			func_add_param("x", tt->param[0]);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XOptional<T>::__assign_nil__);
			func_add_param("x", TypeNone);

		class_add_func(Identifier::Func::EQUAL, TypeBool, &XOptional<T>::__equal__);
			func_add_param("other", tt);
}

}



#endif /* SRC_LIB_KABA_LIB_OPTIONAL_H_ */
