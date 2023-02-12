/*
 * implicit_optional.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

extern const Class *TypeNoValueError;

static shared<Node> op_has_value(shared<Node> node) {
	return node->shift(node->type->size - 1, TypeBool);
}

static shared<Node> op_data(shared<Node> node) {
	return node->shift(0, node->type->param[0]);
}

void AutoImplementer::_add_missing_function_headers_for_optional(Class *t) {
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {t->param[0]}, {"value"}, nullptr, Flags::AUTO_CAST);
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {TypePointer}, {"value"}, nullptr, Flags::AUTO_CAST);
	//if (t->param[0]->get_destructor())
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t->param[0]}, {"other"});
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {TypePointer}, {"other"});
	add_func_header(t, Identifier::Func::OPTIONAL_HAS_VALUE, TypeBool, {}, {}, nullptr, Flags::PURE);
	add_func_header(t, "__bool__", TypeBool, {}, {}, nullptr, Flags::PURE);
	add_func_header(t, Identifier::Func::CALL, t->param[0], {}, {}, nullptr, Flags::REF);
	if (t->param[0]->get_member_func(Identifier::Func::EQUAL, TypeBool, {t->param[0]})) {
		add_func_header(t, Identifier::Func::EQUAL, TypeBool, {t}, {"other"}, nullptr, Flags::PURE);
		add_func_header(t, Identifier::Func::EQUAL, TypeBool, {t->param[0]}, {"other"}, nullptr, Flags::PURE);
	}
}

void AutoImplementer::implement_optional_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.has_value = false
	f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
			op_has_value(self),
			node_false()));
}

void AutoImplementer::implement_optional_constructor_wrap(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto value = add_node_local(f->__get_var("value"));

	if (auto f_con = t->param[0]->get_default_constructor()) {
		// self.data.__init__()
		f->block->add(add_node_member_call(f_con,
				op_data(self)));
	}

	{
		// self.data = value
		if (auto assign = parser->con.link_operator_id(OperatorID::ASSIGN,
				op_data(self),
				value))
			f->block->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
				op_has_value(self),
				node_true()));
	}
}

void AutoImplementer::implement_optional_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto f_des = t->param[0]->get_destructor();
	if (!f_des)
		return;

	auto self = add_node_local(f->__get_var(Identifier::SELF));

	{
		// if self.has_value
		//     self.value.__delete()
		auto cmd_if = add_node_statement(StatementID::IF);
		cmd_if->set_param(0, op_has_value(self));

		auto b = new Block(f, f->block.get());

		// self.data.__delete__()
		b->add(add_node_member_call(f_des,
				op_data(self)));

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}
}

void AutoImplementer::implement_optional_assign(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	if (auto f_des = t->param[0]->get_destructor()) {
		// if self.has_value
		//     self.value.__delete()
		auto cmd_if = add_node_statement(StatementID::IF);
		cmd_if->set_param(0, op_has_value(self));

		auto b = new Block(f, f->block.get());

		// self.data.__delete__()
		b->add(add_node_member_call(f_des,
				op_data(self)));

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}


	{
		// if other.has_value
		//     self.data.__init__()
		//     self.data = other.data
		auto cmd_if = add_node_statement(StatementID::IF);
		cmd_if->set_param(0, op_has_value(other));

		auto b = new Block(f, f->block.get());

		if (auto f_con = t->param[0]->get_default_constructor()) {
			// self.data.__init__()
			b->add(add_node_member_call(f_con,
					op_data(self)));
		}

		if (auto assign = parser->con.link_operator_id(OperatorID::ASSIGN,
				op_data(self), op_data(other)))
			b->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// self.has_value = other.has_value
		auto assign = add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
				op_has_value(self),
				op_has_value(other));
		f->block->add(assign);
	}
}

void AutoImplementer::implement_optional_assign_raw(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	if (auto f_con = t->param[0]->get_default_constructor()) {
		// if not self.has_value
		//     self.value.__init__()
		auto cmd_if = add_node_statement(StatementID::IF);
		auto cmd_not = add_node_operator_by_inline(InlineID::BOOL_NOT, op_has_value(self), nullptr);
		cmd_if->set_param(0, cmd_not);

		auto b = new Block(f, f->block.get());

		// self.data.__init__()
		b->add(add_node_member_call(f_con,
				op_data(self)));

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// self.data = other
		if (auto assign = parser->con.link_operator_id(OperatorID::ASSIGN,
				op_data(self),
				other))
			f->block->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
				op_has_value(self),
				node_true()));
	}
}

void AutoImplementer::implement_optional_assign_null(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	if (auto f_des = t->param[0]->get_destructor()) {
		// if self.has_value
		//     self.data.__delete()
		auto cmd_if = add_node_statement(StatementID::IF);
		cmd_if->set_param(0, op_has_value(self));

		auto b = new Block(f, f->block.get());

		// self.data.__delete__()
		b->add(add_node_member_call(f_des,
				op_data(self)));

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// self.has_value = false
		f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
				op_has_value(self),
				node_false()));
	}
}

void AutoImplementer::implement_optional_has_value(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// return self.has_value
	auto ret = add_node_statement(StatementID::RETURN, -1);
	ret->set_num_params(1);
	ret->set_param(0, op_has_value(self));
	f->block->add(ret);
}

void AutoImplementer::implement_optional_value(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	{
		// if not self.has_value
		//     raise(new NoValueError())
		auto cmd_if = add_node_statement(StatementID::IF);
		auto cmd_not = add_node_operator_by_inline(InlineID::BOOL_NOT, op_has_value(self), nullptr);
		cmd_if->set_param(0, cmd_not);

		auto b = new Block(f, f->block.get());

		auto f_ex = TypeNoValueError->get_default_constructor();
		auto cmd_call_ex = add_node_call(f_ex, -1);
		cmd_call_ex->set_num_params(1);
		cmd_call_ex->set_param(0, new Node(NodeKind::PLACEHOLDER, 0, TypeVoid));

		auto cmd_new = add_node_statement(StatementID::NEW);
		cmd_new->set_num_params(1);
		cmd_new->set_param(0, cmd_call_ex);
		cmd_new->type = TypeExceptionP;

		auto cmd_raise = add_node_call(tree->required_func_global("raise"));
		cmd_raise->set_param(0, cmd_new);
		b->add(cmd_raise);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}


	{
		// return self.data
		auto ret = add_node_statement(StatementID::RETURN, -1);
		ret->set_num_params(1);
		ret->set_param(0, op_data(self));
		f->block->add(ret);
	}
}

void AutoImplementer::implement_optional_equal_raw(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	{
		// if not self.has_value
		//     return false
		auto cmd_if = add_node_statement(StatementID::IF);
		auto cmd_not = add_node_operator_by_inline(InlineID::BOOL_NOT, op_has_value(self), nullptr);
		cmd_if->set_param(0, cmd_not);

		auto b = new Block(f, f->block.get());

		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_false());
		b->add(cmd_ret);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// return self.data == other

		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		if (auto n_eq = parser->con.link_operator_id(OperatorID::EQUAL, op_data(self), other))
			cmd_ret->set_param(0, n_eq);
		else
			do_error_implicit(f, format("no operator %s == %s found", t->param[0]->long_name(), t->param[0]->long_name()));
		f->block->add(cmd_ret);
	}
}

void AutoImplementer::implement_optional_equal(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	{
		// if self.has_value and other.has_value
		//     return self.data == other.data
		auto cmd_if = add_node_statement(StatementID::IF);
		auto cmd_and = add_node_operator_by_inline(InlineID::BOOL_AND, op_has_value(self), op_has_value(other));
		cmd_if->set_param(0, cmd_and);

		auto b = new Block(f, f->block.get());

		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		if (auto n_eq = parser->con.link_operator_id(OperatorID::EQUAL, op_data(self), op_data(other)))
			cmd_ret->set_param(0, n_eq);
		else
			do_error_implicit(f, format("no operator %s == %s found", t->param[0]->long_name(), t->param[0]->long_name()));
		b->add(cmd_ret);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// return self.has_value == other.has_value
		auto n_eq = add_node_operator_by_inline(InlineID::BOOL_EQUAL, op_has_value(self), op_has_value(other));
		auto cmd_ret = add_node_statement(StatementID::RETURN);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, n_eq);
		f->block->add(cmd_ret);
	}
}

void AutoImplementer::_implement_functions_for_optional(const Class *t) {
	implement_optional_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_optional_constructor(prepare_auto_impl(t, t->get_member_func(Identifier::Func::INIT, TypeVoid, {TypePointer})), t);
	implement_optional_constructor_wrap(prepare_auto_impl(t, t->get_member_func(Identifier::Func::INIT, TypeVoid, {t->param[0]})), t);
	implement_optional_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_optional_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t})), t);
	implement_optional_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t->param[0]})), t);
	implement_optional_assign_null(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {TypePointer})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func(Identifier::Func::OPTIONAL_HAS_VALUE, TypeBool, {})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func("__bool__", TypeBool, {})), t);
	implement_optional_value(prepare_auto_impl(t, t->get_member_func(Identifier::Func::CALL, t->param[0], {})), t);
	implement_optional_equal(prepare_auto_impl(t, t->get_member_func(Identifier::Func::EQUAL, TypeBool, {t})), t);
	implement_optional_equal_raw(prepare_auto_impl(t, t->get_member_func(Identifier::Func::EQUAL, TypeBool, {t->param[0]})), t);
}

}



