#pragma once

#include "../../base/base.h"
#include "../../base/map.h"
#include "../../base/future.h"
#include "../../base/optional.h"
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
		base::optional<T*> get_item(const string &k) {
			if (this->contains(k))
				return &(*this)[k];
			return base::None;
		}
	};


	template<class T>
	void lib_create_dict(const Class* tt, const Class* get_return) {
		auto t = const_cast<Class*>(tt);
		auto t_element = tt->param[0];

		t->derive_from(TypeDictBase, DeriveFlags::SET_SIZE);

		add_class(t);
			class_add_func(Identifier::func::Init, TypeVoid, &XDict<T>::__init__, Flags::Mutable);
			class_add_func(Identifier::func::Delete, TypeVoid, &XDict<T>::clear, Flags::Mutable);
			class_add_func(Identifier::func::Set, TypeVoid, &XDict<T>::__set, Flags::Mutable);
				func_add_param("key", TypeString);
				func_add_param("x", t_element);
			class_add_func(Identifier::func::Get, get_return, &XDict<T>::get_item, Flags::Ref);
				func_add_param("key", TypeString);
			class_add_func("clear", TypeVoid, &XDict<T>::clear, Flags::Mutable);
			class_add_func(Identifier::func::Contains, TypeBool, &XDict<T>::contains);
				func_add_param("key", TypeString);
			class_add_func(Identifier::func::Assign, TypeVoid, &XDict<T>::assign, Flags::Mutable);
				func_add_param("other", t);
			class_add_func("keys", TypeStringList, &dict_get_keys, Flags::Pure);
			class_add_func(Identifier::func::Str, TypeString, &XDict<T>::str);
	}


}
