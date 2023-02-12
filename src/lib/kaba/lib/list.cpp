/*
 * list.cpp
 *
 *  Created on: 30 Jul 2022
 *      Author: michi
 */

#include "list.h"
#include "lib.h"
#include "../kaba.h"

namespace kaba {

class PointerList : public Array<void*> {
public:
	bool __contains__(void *x) {
		return find(x) >= 0;
	}
};

void kaba_make_super_array(Class *t, SyntaxTree *ps) {
	const Class *p = t->param[0];
	t->derive_from(TypeDynamicArray, false);
	t->param = {p};
	add_class(t);

	// already done by derive_from()
	//Function *sub = t->get_func(Identifier::Func::SUBARRAY, TypeDynamicArray, {nullptr,nullptr});
	//sub->literal_return_type = t;
	//sub->effective_return_type = t;

	// FIXME  wrong for complicated classes
	if (p->can_memcpy()) {
		if (!p->uses_call_by_reference()){
			if (p->is_some_pointer()){
				class_add_func(Identifier::Func::INIT, TypeVoid, &XList<void*>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_p_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_p_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
				class_add_func("__contains__", TypeBool, &PointerList::__contains__);
					func_add_param("x", p);
			}else if (p == TypeFloat32){
				class_add_func(Identifier::Func::INIT, TypeVoid, &XList<float>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_f_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_f_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p == TypeFloat64){
				class_add_func(Identifier::Func::INIT, TypeVoid, &XList<double>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_d_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_d_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p->size == 4){
				class_add_func(Identifier::Func::INIT, TypeVoid, &XList<int>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_4_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_4_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else if (p->size == 1){
				class_add_func(Identifier::Func::INIT, TypeVoid, &XList<char>::__init__);
				class_add_func("add", TypeVoid, &DynamicArray::append_1_single);
					func_add_param("x", p);
				class_add_func("insert", TypeVoid, &DynamicArray::insert_1_single);
					func_add_param("x", p);
					func_add_param("index", TypeInt);
			}else{
				msg_error("evil class:  " + t->name);
			}
		}else{
			// __init__ must be defined manually...!
			class_add_func("add", TypeVoid, &DynamicArray::append_single);
				func_add_param("x", p);
			class_add_func("insert", TypeVoid, &DynamicArray::insert_single);
				func_add_param("x", p);
				func_add_param("index", TypeInt);
		}
		class_add_func(Identifier::Func::DELETE, TypeVoid, &DynamicArray::simple_clear);
		class_add_func("clear", TypeVoid, &DynamicArray::simple_clear);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, &DynamicArray::simple_assign);
			func_add_param("other", t);
		class_add_func("remove", TypeVoid, &DynamicArray::delete_single);
			func_add_param("index", TypeInt);
		class_add_func("resize", TypeVoid, &DynamicArray::simple_resize);
			func_add_param("num", TypeInt);
	}else if (p == TypeString or p == TypeAny or p == TypePath){
		// handled manually later...
	}else{
		msg_error("evil class:  " + t->name);
	}
}



}
