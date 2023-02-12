/*
 * implicit_dict.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

void AutoImplementer::_add_missing_function_headers_for_dict(Class *t) {
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {});
	add_func_header(t, "clear", TypeVoid, {}, {});
	add_func_header(t, "add", TypeVoid, {TypeString, t->param[0]}, {"key", "x"});
	add_func_header(t, Identifier::Func::GET, t->param[0], {TypeString}, {"key"});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"});
}

void AutoImplementer::implement_dict_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto te = t->get_array_element();
	auto ff = t->get_member_func("__mem_init__", TypeVoid, {TypeInt});
	f->block->add(add_node_member_call(ff,
			self, -1,
			{add_node_const(tree->add_constant_int(te->size + TypeString->size))}));
}

void AutoImplementer::_implement_functions_for_dict(const Class *t) {
	implement_super_array_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_super_array_destructor(prepare_auto_impl(t, t->get_destructor()), t);
}

}



