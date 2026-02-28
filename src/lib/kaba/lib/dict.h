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
		T* get_item(const string &k) {
			if (this->contains(k))
				return &(*this)[k];
			return nullptr;
		}
	};


	template<class T>
	void lib_create_dict(const Class* tt, const Class* get_return) {
		auto t = const_cast<Class*>(tt);
		auto t_element = tt->param[0];

		t->derive_from(common_types.dict_base, DeriveFlags::SET_SIZE);

		add_class(t);
			class_add_func(Identifier::func::Init, common_types._void, &XDict<T>::__init__, Flags::Mutable);
			class_add_func(Identifier::func::Delete, common_types._void, &XDict<T>::clear, Flags::Mutable);
			class_add_func(Identifier::func::Set, common_types._void, &XDict<T>::__set, Flags::Mutable);
				func_add_param("key", common_types.string);
				func_add_param("x", t_element);
			class_add_func(Identifier::func::Get, get_return, &XDict<T>::get_item, Flags::Ref);
				func_add_param("key", common_types.string);
			class_add_func("clear", common_types._void, &XDict<T>::clear, Flags::Mutable);
			class_add_func(Identifier::func::Contains, common_types._bool, &XDict<T>::contains);
				func_add_param("key", common_types.string);
			class_add_func(Identifier::func::Assign, common_types._void, &XDict<T>::assign, Flags::Mutable);
				func_add_param("other", t);
			class_add_func("keys", common_types.string_list, &dict_get_keys, Flags::Pure);
			class_add_func(Identifier::func::Str, common_types.string, &XDict<T>::str);
	}


}
