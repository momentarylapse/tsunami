#pragma once

#include "../../base/base.h"
#include "../../base/map.h"
#include "../../base/future.h"
#include "../kaba.h"
#include "../dynamic/exception.h"


namespace kaba {
	class Class;
	class SyntaxTree;
	extern const Class *TypeStringList;
	extern const Class *TypeDictBase;
	
	Array<string> dict_get_keys(const DynamicArray& a);


	template<class T>
	class XDict : public base::map<string, T> {
	public:
		void __init__() {
			new(this) base::map<string, T>();
		}
		void assign(XDict<T> &o) {
			*this = o;
		}
		void __set(const string &k, const typename base::xparam<T>::t v) {
			this->set(k, v);
		}
		T get_item(const string &k) {
			KABA_EXCEPTION_WRAPPER(return (*this)[k]);
			return T();
		}
	};


	template<class T>
	void lib_create_dict(const Class *tt) {
		auto t = const_cast<Class*>(tt);
		auto t_element = tt->param[0];

		t->derive_from(TypeDictBase, DeriveFlags::SET_SIZE);

		add_class(t);
			class_add_func(Identifier::Func::INIT, TypeVoid, &XDict<T>::__init__, Flags::MUTABLE);
			class_add_func(Identifier::Func::DELETE, TypeVoid, &XDict<T>::clear, Flags::MUTABLE);
			class_add_func(Identifier::Func::SET, TypeVoid, &XDict<T>::__set, Flags::MUTABLE);
				func_add_param("key", TypeString);
				func_add_param("x", t_element);
			class_add_func(Identifier::Func::GET, t_element, &XDict<T>::get_item, Flags::RAISES_EXCEPTIONS);
				func_add_param("key", TypeString);
			class_add_func("clear", TypeVoid, &XDict<T>::clear, Flags::MUTABLE);
			class_add_func(Identifier::Func::CONTAINS, TypeBool, &XDict<T>::contains);
				func_add_param("key", TypeString);
			class_add_func(Identifier::Func::ASSIGN, TypeVoid, &XDict<T>::assign, Flags::MUTABLE);
				func_add_param("other", t);
			class_add_func("keys", TypeStringList, &dict_get_keys, Flags::PURE);
			class_add_func(Identifier::Func::STR, TypeString, &XDict<T>::str);
	}


}
