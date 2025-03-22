/*
 * implicit_optional.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../../os/msg.h"
#include "../parser/Parser.h"

namespace kaba {

extern const Class *TypeNoValueError;
extern const Class *TypeNone;

shared<Node> AutoImplementer::optional_has_value(shared<Node> node) {
	return node->shift(node->type->param[0]->size, TypeBool);
}

shared<Node> AutoImplementer::optional_data(shared<Node> node) {
	return node->change_type(node->type->param[0]);
}


void AutoImplementer::implement_optional_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));

	// self.has_value = false
	f->block->add(add_node_operator_by_inline(InlineID::BoolAssign,
											  optional_has_value(self),
											  node_false()));
}

void AutoImplementer::implement_optional_constructor_wrap(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto value = add_node_local(f->__get_var("value"));

	if (auto f_con = t->param[0]->get_default_constructor()) {
		// self.data.__init__()
		f->block->add(add_node_member_call(f_con,
										   optional_data(self)));
	}

	{
		// self.data = value
		auto op = OperatorID::Assign;
		if (value->type->is_reference())
			op = OperatorID::RefAssign;
		if (auto assign = parser->con.link_operator_id(op,
													   optional_data(self),
													   value))
			f->block->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block->add(add_node_operator_by_inline(InlineID::BoolAssign,
												  optional_has_value(self),
												  node_true()));
	}
}

void AutoImplementer::implement_optional_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto f_des = t->param[0]->get_destructor();
	if (!f_des)
		return;

	auto self = add_node_local(f->__get_var(Identifier::Self));

	{
		// if self.has_value
		//     self.value.__delete()
		auto n_del = add_node_member_call(f_des, optional_data(self));
		f->block->add(node_if(optional_has_value(self), n_del));
	}
}

void AutoImplementer::implement_optional_assign(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	if (auto f_des = t->param[0]->get_destructor()) {
		// if self.has_value
		//     self.value.__delete()

		auto n_del = add_node_member_call(f_des, optional_data(self));
		f->block->add(node_if(optional_has_value(self), n_del));
	}


	{
		// if other.has_value
		//     self.data.__init__()
		//     self.data = other.data

		auto b = new Block(f, f->block.get());

		if (auto f_con = t->param[0]->get_default_constructor()) {
			// self.data.__init__()
			b->add(add_node_member_call(f_con,
										optional_data(self)));
		}

		auto op = OperatorID::Assign;
		if (self->type->param[0]->is_reference())
			op = OperatorID::RefAssign;
		if (auto assign = parser->con.link_operator_id(op,
													   optional_data(self), optional_data(other)))
			b->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));

		f->block->add(node_if(optional_has_value(other), b));
	}

	{
		// self.has_value = other.has_value
		auto assign = add_node_operator_by_inline(InlineID::BoolAssign,
												  optional_has_value(self),
												  optional_has_value(other));
		f->block->add(assign);
	}
}

void AutoImplementer::implement_optional_assign_raw(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	if (auto f_con = t->param[0]->get_default_constructor()) {
		// if not self.has_value
		//     self.value.__init__()
		auto cmd_not = node_not(optional_has_value(self));
		auto n_init = add_node_member_call(f_con, optional_data(self));
		f->block->add(node_if(cmd_not, n_init));
	}

	{
		// self.data = other
		auto op = OperatorID::Assign;
		if (other->type->is_reference())
			op = OperatorID::RefAssign;
		if (auto assign = parser->con.link_operator_id(op,
													   optional_data(self),
													   other))
			f->block->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block->add(add_node_operator_by_inline(InlineID::BoolAssign,
												  optional_has_value(self),
												  node_true()));
	}
}

void AutoImplementer::implement_optional_assign_null(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));

	if (auto f_des = t->param[0]->get_destructor()) {
		// if self.has_value
		//     self.data.__delete()

		auto n_del = add_node_member_call(f_des, optional_data(self));
		f->block->add(node_if(optional_has_value(self), n_del));
	}

	{
		// self.has_value = false
		f->block->add(add_node_operator_by_inline(InlineID::BoolAssign,
												  optional_has_value(self),
												  node_false()));
	}
}

void AutoImplementer::implement_optional_has_value(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));

	// return self.has_value
	f->block->add(node_return(optional_has_value(self)));
}

void AutoImplementer::implement_optional_equal_raw(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	{
		// if not self.has_value
		//     return false
		auto cmd_not = node_not(optional_has_value(self));
		f->block->add(node_if(cmd_not, node_return(node_false())));
	}

	{
		// return self.data == other
		if (auto n_eq = parser->con.link_operator_id(OperatorID::Equal, optional_data(self), other))
			f->block->add(node_return(n_eq));
		else
			do_error_implicit(f, format("no operator %s == %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}
}

void AutoImplementer::implement_optional_equal(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	{
		// if self.has_value and other.has_value
		//     return self.data == other.data
		auto cmd_and = add_node_operator_by_inline(InlineID::BoolAnd, optional_has_value(self), optional_has_value(other));

		if (auto n_eq = parser->con.link_operator_id(OperatorID::Equal, optional_data(self), optional_data(other)))
			f->block->add(node_if(cmd_and, node_return(n_eq)));
		else
			do_error_implicit(f, format("no operator %s == %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// return self.has_value == other.has_value
		auto n_eq = add_node_operator_by_inline(InlineID::BoolEqual, optional_has_value(self), optional_has_value(other));
		f->block->add(node_return(n_eq));
	}
}

void AutoImplementer::_implement_functions_for_optional(const Class *t) {
	implement_optional_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_optional_constructor(prepare_auto_impl(t, t->get_member_func(Identifier::func::Init, TypeVoid, {TypeNone})), t);
	implement_optional_constructor_wrap(prepare_auto_impl(t, t->get_member_func(Identifier::func::Init, TypeVoid, {t->param[0]})), t);
	implement_optional_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_optional_assign(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, TypeVoid, {t})), t);
	implement_optional_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, TypeVoid, {t->param[0]})), t);
	implement_optional_assign_null(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, TypeVoid, {TypeNone})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func(Identifier::func::OptionalHasValue, TypeBool, {})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func("__bool__", TypeBool, {})), t);
	implement_optional_equal(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, TypeBool, {t})), t);
	implement_optional_equal_raw(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, TypeBool, {t->param[0]})), t);
}



int _make_optional_size(const Class *t) {
	return mem_align(t->size + 1, t->alignment);
}


Class* TemplateClassInstantiatorOptional::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, class_name_might_need_parantheses(params[0]) + "?", TypeOptionalT, _make_optional_size(params[0]), params[0]->alignment, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorOptional::add_function_headers(Class* t) {
	if (!class_can_default_construct(t->param[0]))
		t->owner->do_error(format("can not create an optional from type '%s', missing default constructor", t->param[0]->long_name()), t->token_id);

	add_func_header(t, Identifier::func::Init, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Init, TypeVoid, {t->param[0]}, {"value"}, nullptr, Flags::AutoCast | Flags::Mutable);
	add_func_header(t, Identifier::func::Init, TypeVoid, {TypeNone}, {"value"}, nullptr, Flags::AutoCast | Flags::Mutable);
	//if (t->param[0]->get_destructor())
	add_func_header(t, Identifier::func::Delete, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, TypeVoid, {t}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, TypeVoid, {t->param[0]}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, TypeVoid, {TypeNone}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::OptionalHasValue, TypeBool, {}, {}, nullptr, Flags::Pure);
	add_func_header(t, "__bool__", TypeBool, {}, {}, nullptr, Flags::Pure);
	//add_func_header(t, "_get_p", t->param[0], {}, {}, nullptr, Flags::REF);
	if (t->param[0]->get_member_func(Identifier::func::Equal, TypeBool, {t->param[0]})) {
		add_func_header(t, Identifier::func::Equal, TypeBool, {t}, {"other"}, nullptr, Flags::Pure);
		add_func_header(t, Identifier::func::Equal, TypeBool, {t->param[0]}, {"other"}, nullptr, Flags::Pure);
	}
}

}



