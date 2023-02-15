/*
 * implicit_shared.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

void AutoImplementer::_add_missing_function_headers_for_shared(Class *t) {
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::SHARED_CLEAR, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {tree->get_pointer(t->param[0])}, {"p"});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"p"});
	add_func_header(t, Identifier::Func::SHARED_CREATE, t, {tree->get_pointer(t->param[0])}, {"p"}, nullptr, Flags::STATIC);
}

void AutoImplementer::_add_missing_function_headers_for_owned(Class *t) {
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::SHARED_CLEAR, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {tree->get_pointer(t->param[0])}, {"p"});
	//add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"p"});
	//add_func_header(t, Identifier::Func::SHARED_CREATE, t, {t->param[0]->get_pointer()}, {"p"}, nullptr, Flags::STATIC);
}

void AutoImplementer::implement_shared_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto te = t->param[0];
	// self.p = nil
	f->block->add(add_node_operator_by_inline(InlineID::POINTER_ASSIGN,
			self->shift(0, TypePointer),
			add_node_const(tree->add_constant_pointer(tree->get_pointer(te), nullptr))));
}

void AutoImplementer::implement_shared_destructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// call clear()
	auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	f->block->add(add_node_member_call(f_clear, self));
}


void AutoImplementer::implement_shared_assign(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	// call clear()
	auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	auto call_clear = add_node_member_call(f_clear, self);
	f->block->add(call_clear);


	auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, p);
	f->block->add(op);


	// if p
	//     p.count ++
	auto cmd_if = add_node_statement(StatementID::IF);

	// if p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, p);
	cmd_if->set_param(0, cmd_cmp);

	auto b = new Block(f, f->block.get());
	cmd_if->set_param(1, b);


	f->block->add(cmd_if);

	auto tt = self->type->param[0];
	bool found = false;
	for (auto &e: tt->elements)
		if (e.name == Identifier::SHARED_COUNT and e.type == TypeInt) {
			// count ++
			auto count = self_p->deref()->shift(e.offset, e.type);
			auto inc = add_node_operator_by_inline(InlineID::INT_INCREASE, count, nullptr);
			b->add(inc);
			found = true;
		}
	if (!found)
		do_error_implicit(f, format("class '%s' is not a shared class (declare with '%s class' or add an element 'int %s')", tt->long_name(), Identifier::SHARED, Identifier::SHARED_COUNT));
}

void AutoImplementer::implement_shared_clear(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	auto tt = t->param[0];

	// if self.p
	//     self.p.count --
	//     if self.p.count == 0
	//         del self.p
	//     self.p = nil

	auto cmd_if = add_node_statement(StatementID::IF);

	// if self.p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, self_p);
	cmd_if->set_param(0, cmd_cmp);

	auto b = new Block(f, f->block.get());


	shared<Node> count;
	for (auto &e: tt->elements)
		if (e.name == Identifier::SHARED_COUNT and e.type == TypeInt)
			count = self_p->deref_shift(e.offset, e.type, -1);
	if (!count)
		do_error_implicit(f, format("class '%s' is not a shared class (declare with '%s class' or add an element 'int %s')", tt->long_name(), Identifier::SHARED, Identifier::SHARED_COUNT));

	// count --
	auto dec = add_node_operator_by_inline(InlineID::INT_DECREASE, count, nullptr);
	b->add(dec);


	auto cmd_if_del = add_node_statement(StatementID::IF);

	// if count == 0
	auto zero = add_node_const(tree->add_constant_int(0));
	auto cmp = add_node_operator_by_inline(InlineID::INT_EQUAL, count, zero);
	cmd_if_del->set_param(0, cmp);

	auto b2 = new Block(f, b);


	// del self
	auto cmd_del = add_node_statement(StatementID::DELETE);
	cmd_del->set_param(0, self_p);
	b2->add(cmd_del);
	cmd_if_del->set_param(1, b2);
	b->add(cmd_if_del);


	// self = nil
	auto n_null = add_node_const(tree->add_constant_pointer(tree->get_pointer(t->param[0]), nullptr));
	auto n_op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, n_null);
	b->add(n_op);

	cmd_if->set_param(1, b);
	f->block->add(cmd_if);
}


void AutoImplementer::implement_shared_create(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto r = add_node_local(f->block->add_var("r", t));


	// r = p
	auto f_assign = t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {p->type});
	if (!f_assign)
		do_error_implicit(f, "= missing...");
	auto call_assign = add_node_member_call(f_assign, r);
	call_assign->set_param(1, p);
	f->block->add(call_assign);

	// return r
	auto ret = add_node_statement(StatementID::RETURN);
	ret->set_num_params(1);
	ret->set_param(0, r);
	f->block->add(ret);
}



void AutoImplementer::implement_owned_assign(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	// call clear()
	auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	auto call_clear = add_node_member_call(f_clear, self);
	f->block->add(call_clear);


	auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, p);
	f->block->add(op);
}

void AutoImplementer::implement_owned_clear(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	// if self.p
	//     del self.p
	//     self.p = nil

	auto cmd_if = add_node_statement(StatementID::IF);

	// if self.p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, self_p);
	cmd_if->set_param(0, cmd_cmp);

	auto b = new Block(f, f->block.get());


	// del self
	auto cmd_del = add_node_statement(StatementID::DELETE);
	cmd_del->set_param(0, self_p);
	b->add(cmd_del);


	// self = nil
	auto n_null = add_node_const(tree->add_constant_pointer(tree->get_pointer(t->param[0]), nullptr));
	auto n_op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, n_null);
	b->add(n_op);

	cmd_if->set_param(1, b);
	f->block->add(cmd_if);
}

void AutoImplementer::_implement_functions_for_shared(const Class *t) {
	implement_shared_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_shared_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_shared_clear(prepare_auto_impl(t, t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {})), t);
	implement_shared_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {tree->get_pointer(t->param[0])})), t);
	implement_shared_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t})), t);
	implement_shared_create(prepare_auto_impl(t, t->get_func(Identifier::Func::SHARED_CREATE, t, {tree->get_pointer(t->param[0])})), t);
}

void AutoImplementer::_implement_functions_for_owned(const Class *t) {
	implement_shared_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_shared_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_owned_clear(prepare_auto_impl(t, t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {})), t);
	implement_owned_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {tree->get_pointer(t->param[0])})), t);
	//implement_shared_assign(prepare_auto_impl(t, t->get_func(Identifier::Func::ASSIGN, TypeVoid, {nullptr, t})), t);
	//implement_shared_create(prepare_auto_impl(t, t->get_func(Identifier::Func::SHARED_CREATE, t, {nullptr, tree->get_pointer(t->param[0])})), t);
}



}



