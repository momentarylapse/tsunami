/*
 * implicit_product.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

// using constructor/destructor/assign from regular!

void AutoImplementer::_add_missing_function_headers_for_product(Class *t) {
	if (t->needs_constructor())
		add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {});
	if (class_can_fully_construct(t))
		add_full_constructor(t);
	if (t->needs_destructor())
		add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {});

	bool allow_assign = true;
	bool allow_equal = true;
	for (const auto p: t->param) {
		if (!class_can_assign(p))
			allow_assign = false;
		if (!class_can_equal(p))
			allow_equal = false;
	}

	if (allow_assign) {
		add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
		if (t->can_memcpy())
			t->get_assign()->inline_no = InlineID::CHUNK_ASSIGN;
	}
	if (allow_equal) {
		add_func_header(t, Identifier::Func::EQUAL, TypeBool, {t}, {"other"}, nullptr, Flags::PURE);
		add_func_header(t, Identifier::Func::NOT_EQUAL, TypeBool, {t}, {"other"}, nullptr, Flags::PURE);
	}
}

void AutoImplementer::implement_product_equal(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	for (auto& e: t->elements) {
		// if self.e != other.e
		//     return false

		auto cmd_if = add_node_statement(StatementID::IF);
		cmd_if->set_param(0, add_not_equal(f, "", self->shift(e.offset, e.type), other->shift(e.offset, e.type)));

		auto b = new Block(f, f->block.get());

		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_false());
		b->add(cmd_ret);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// return true
		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_true());
		f->block->add(cmd_ret);
	}
}

void AutoImplementer::implement_product_not_equal(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	for (auto& e: t->elements) {
		// if self.e == other.e
		//     return false

		auto cmd_if = add_node_statement(StatementID::IF);
		cmd_if->set_param(0, add_equal(f, "", self->shift(e.offset, e.type), other->shift(e.offset, e.type)));

		auto b = new Block(f, f->block.get());

		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_false());
		b->add(cmd_ret);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// return true
		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_true());
		f->block->add(cmd_ret);
	}
}

void AutoImplementer::_implement_functions_for_product(const Class *t) {
	for (auto *cf: t->get_constructors())
		implement_regular_constructor(prepare_auto_impl(t, cf), t, true);
	implement_regular_destructor(prepare_auto_impl(t, t->get_destructor()), t); // if exists...
	implement_regular_assign(prepare_auto_impl(t, t->get_assign()), t); // if exists...
	implement_product_equal(prepare_auto_impl(t, t->get_member_func(Identifier::Func::EQUAL, TypeBool, {t})), t); // if exists...
	implement_product_not_equal(prepare_auto_impl(t, t->get_member_func(Identifier::Func::NOT_EQUAL, TypeBool, {t})), t); // if exists...
}


}



