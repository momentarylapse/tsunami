/*
 * implicit_array.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

void AutoImplementer::_add_missing_function_headers_for_array(Class *t) {
	if (t->param[0]->needs_constructor() and class_can_default_construct(t->param[0]))
		add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	if (t->param[0]->needs_destructor() and class_can_destruct(t->param[0]))
		add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	if (class_can_assign(t->param[0]))
		add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, nullptr, Flags::MUTABLE);
	if (class_can_equal(t->param[0]) and false) // TODO
		add_func_header(t, Identifier::Func::EQUAL, TypeBool, {t}, {"other"}, nullptr, Flags::PURE);
}

void AutoImplementer::implement_array_constructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto te = t->get_array_element();
	if (auto *f_el_init = te->get_default_constructor()) {
		for (int i=0; i<t->array_length; i++) {
			// self[i].__init__()
			f->block->add(add_node_member_call(f_el_init,
					self->shift(te->size * i, te)));
		}
	} else if (te->needs_constructor()) {
		do_error_implicit(f, format("missing default constructor for %s", te->long_name()));
	}
}

void AutoImplementer::implement_array_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	if (auto *f_el_del = te->get_destructor()) {
		for (int i=0; i<t->array_length; i++) {
			// self[i].__delete__()
			f->block->add(add_node_member_call(f_el_del,
					self->shift(te->size * i, te)));
		}
	} else if (te->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}
}

void AutoImplementer::implement_array_assign(Function *f, const Class *t) {
	if (!f)
		return;

	auto n_other = add_node_local(f->__get_var("other"));
	auto n_self = add_node_local(f->__get_var(Identifier::SELF));

	// for i=>el in self
	//    el = other[i]

	auto *v_el = f->block->add_var("el", tree->request_implicit_class_reference(t->get_array_element(), -1));
	auto *v_i = f->block->add_var("i", TypeInt);

	Block *b = new Block(f, f->block.get());

	// other[i]
	shared<Node> n_other_el = add_node_array(n_other, add_node_local(v_i));

	b->add(add_assign(f, "", add_node_local(v_el)->deref(), n_other_el));

	auto n_for = add_node_statement(StatementID::FOR_CONTAINER);
	// [VAR, INDEX, ARRAY, BLOCK]
	n_for->set_param(0, add_node_local(v_el));
	n_for->set_param(1, add_node_local(v_i));
	n_for->set_param(2, n_self);
	n_for->set_param(3, b);
	f->block->add(n_for);
}
void AutoImplementer::_implement_functions_for_array(const Class *t) {
	implement_array_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_array_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_array_assign(prepare_auto_impl(t, t->get_assign()), t);
}



}



