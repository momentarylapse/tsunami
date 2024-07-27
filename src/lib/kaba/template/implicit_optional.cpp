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
extern const Class *TypeNone;

shared<Node> AutoImplementer::optional_has_value(shared<Node> node) {
	return node->shift(node->type->param[0]->size, TypeBool);
}

shared<Node> AutoImplementer::optional_data(shared<Node> node) {
	return node->change_type(node->type->param[0]);
}


void AutoImplementer::implement_optional_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.has_value = false
	f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
											  optional_has_value(self),
											  node_false()));
}

void AutoImplementer::implement_optional_constructor_wrap(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto value = add_node_local(f->__get_var("value"));

	if (auto f_con = t->param[0]->get_default_constructor()) {
		// self.data.__init__()
		f->block->add(add_node_member_call(f_con,
										   optional_data(self)));
	}

	{
		// self.data = value
		if (auto assign = parser->con.link_operator_id(OperatorID::ASSIGN,
													   optional_data(self),
													   value))
			f->block->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
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

	auto self = add_node_local(f->__get_var(Identifier::SELF));

	{
		// if self.has_value
		//     self.value.__delete()
		auto n_del = add_node_member_call(f_des, optional_data(self));
		f->block->add(node_if(optional_has_value(self), n_del));
	}
}

void AutoImplementer::implement_optional_assign(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
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

		if (auto assign = parser->con.link_operator_id(OperatorID::ASSIGN,
													   optional_data(self), optional_data(other)))
			b->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));

		f->block->add(node_if(optional_has_value(other), b));
	}

	{
		// self.has_value = other.has_value
		auto assign = add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
												  optional_has_value(self),
												  optional_has_value(other));
		f->block->add(assign);
	}
}

void AutoImplementer::implement_optional_assign_raw(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
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
		if (auto assign = parser->con.link_operator_id(OperatorID::ASSIGN,
													   optional_data(self),
													   other))
			f->block->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
												  optional_has_value(self),
												  node_true()));
	}
}

void AutoImplementer::implement_optional_assign_null(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	if (auto f_des = t->param[0]->get_destructor()) {
		// if self.has_value
		//     self.data.__delete()

		auto n_del = add_node_member_call(f_des, optional_data(self));
		f->block->add(node_if(optional_has_value(self), n_del));
	}

	{
		// self.has_value = false
		f->block->add(add_node_operator_by_inline(InlineID::BOOL_ASSIGN,
												  optional_has_value(self),
												  node_false()));
	}
}

void AutoImplementer::implement_optional_has_value(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// return self.has_value
	f->block->add(node_return(optional_has_value(self)));
}

void AutoImplementer::implement_optional_equal_raw(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto other = add_node_local(f->__get_var("other"));

	{
		// if not self.has_value
		//     return false
		auto cmd_not = node_not(optional_has_value(self));
		f->block->add(node_if(cmd_not, node_return(node_false())));
	}

	{
		// return self.data == other
		if (auto n_eq = parser->con.link_operator_id(OperatorID::EQUAL, optional_data(self), other))
			f->block->add(node_return(n_eq));
		else
			do_error_implicit(f, format("no operator %s == %s found", t->param[0]->long_name(), t->param[0]->long_name()));
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
		auto cmd_and = add_node_operator_by_inline(InlineID::BOOL_AND, optional_has_value(self), optional_has_value(other));

		if (auto n_eq = parser->con.link_operator_id(OperatorID::EQUAL, optional_data(self), optional_data(other)))
			f->block->add(node_if(cmd_and, node_return(n_eq)));
		else
			do_error_implicit(f, format("no operator %s == %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// return self.has_value == other.has_value
		auto n_eq = add_node_operator_by_inline(InlineID::BOOL_EQUAL, optional_has_value(self), optional_has_value(other));
		f->block->add(node_return(n_eq));
	}
}

void AutoImplementer::_implement_functions_for_optional(const Class *t) {
	implement_optional_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_optional_constructor(prepare_auto_impl(t, t->get_member_func(Identifier::Func::INIT, TypeVoid, {TypeNone})), t);
	implement_optional_constructor_wrap(prepare_auto_impl(t, t->get_member_func(Identifier::Func::INIT, TypeVoid, {t->param[0]})), t);
	implement_optional_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_optional_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t})), t);
	implement_optional_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t->param[0]})), t);
	implement_optional_assign_null(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {TypeNone})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func(Identifier::Func::OPTIONAL_HAS_VALUE, TypeBool, {})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func("__bool__", TypeBool, {})), t);
	implement_optional_equal(prepare_auto_impl(t, t->get_member_func(Identifier::Func::EQUAL, TypeBool, {t})), t);
	implement_optional_equal_raw(prepare_auto_impl(t, t->get_member_func(Identifier::Func::EQUAL, TypeBool, {t->param[0]})), t);
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

	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {t->param[0]}, {"value"}, nullptr, Flags::AUTO_CAST | Flags::MUTABLE);
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {TypeNone}, {"value"}, nullptr, Flags::AUTO_CAST | Flags::MUTABLE);
	//if (t->param[0]->get_destructor())
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t->param[0]}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {TypeNone}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::OPTIONAL_HAS_VALUE, TypeBool, {}, {}, nullptr, Flags::PURE);
	add_func_header(t, "__bool__", TypeBool, {}, {}, nullptr, Flags::PURE);
	//add_func_header(t, "_get_p", t->param[0], {}, {}, nullptr, Flags::REF);
	if (t->param[0]->get_member_func(Identifier::Func::EQUAL, TypeBool, {t->param[0]})) {
		add_func_header(t, Identifier::Func::EQUAL, TypeBool, {t}, {"other"}, nullptr, Flags::PURE);
		add_func_header(t, Identifier::Func::EQUAL, TypeBool, {t->param[0]}, {"other"}, nullptr, Flags::PURE);
	}
}

}



