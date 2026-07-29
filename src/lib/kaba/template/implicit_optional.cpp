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

shared<Node> AutoImplementer::optional_has_value(shared<Node> node) {
	return node->shift(node->type->param[0]->size, common_types._bool);
}

shared<Node> AutoImplementer::optional_data(shared<Node> node) {
	return node->change_type(node->type->param[0]);
}

shared<Node> AutoImplementer::result_type(shared<Node> node) {
	return node->shift(max((int)node->type->param[0]->size, config.target.dynamic_array_size), common_types.u8);
}


void AutoImplementer::implement_optional_constructor(Function *f, const Class *t) {
	implement_from_code(f, "self._has_value = false");
}

void AutoImplementer::implement_result_constructor(Function *f, const Class *t) {
	implement_from_code(f, "self._state = 0x00");
}

void AutoImplementer::implement_result_constructor_error(Function *f, const Class *t) {
	implement_from_code(f, "self._state = 0x02\n(&self as string&).__init__()\n*(&self as string&) = *(&err as string&)");
}

void AutoImplementer::implement_optional_constructor_value(Function *f, const Class *t) {
	if (t->param[0]->is_reference())
		implement_from_code(f, "@noderef(*(&self as X&)) := value\nself._has_value = true");
	else if (t->param[0]->get_default_constructor())
		implement_from_code(f, "@noderef(*(&self as X&)).__init__()\n@noderef(*(&self as X&)) = value\nself._has_value = true");
	else if (t->param[0]->needs_constructor())
		do_error_implicit(f, "missing default constructor");
	else
		implement_from_code(f, "@noderef(*(&self as X&)) = value\nself._has_value = true");
}

void AutoImplementer::implement_result_constructor_value(Function *f, const Class *t) {
	if (t->param[0]->is_reference())
		implement_from_code(f, "@noderef(*(&self as X&)) := value\nself._state = 0x01");
	else if (t->param[0]->get_default_constructor())
		implement_from_code(f, "@noderef(*(&self as X&)).__init__()\n@noderef(*(&self as X&)) = @noderef(value)\nself._state = 0x01");
	else if (t->param[0]->needs_constructor())
		do_error_implicit(f, "missing default constructor");
	else
		implement_from_code(f, "@noderef(*(&self as X&)) = @noderef(value)\nself._state = 0x01");
}

void AutoImplementer::implement_optional_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	if (t->param[0]->get_destructor())
		implement_from_code(f, "if self._has_value\n\t@noderef(*(&self as X&)).__delete__()");
}

void AutoImplementer::implement_result_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	if (t->param[0]->get_destructor())
		implement_from_code(f, "if self._state == 0x01\n\t@noderef(*(&self as X&)).__delete__()\nelse if self._state == 0x02\n\t(&self as string&).__delete__()");
	else
		implement_from_code(f, "if self._state == 0x02\n\t(&self as string&).__delete__()");
}

void AutoImplementer::implement_optional_assign(Function *f, const Class *t) {
#if 1
	string code;
	if (t->param[0]->get_destructor())
		code += "if self._has_value\n\t@noderef(*(&self as X&)).__delete__()";
	if (t->param[0]->is_reference())
		code += "\nif other._has_value\n\t*(&self as i64&) = *(&other as i64&)";
	else if (t->param[0]->get_default_constructor())
		code += "\nif other._has_value\n\t@noderef(*(&self as X&)).__init__()\n\t@noderef(*(&self as X&)) = @noderef(*(&other as X&))";
	else
		code += "\nif other._has_value\n\t@noderef(*(&self as X&)) = @noderef(*(&other as X&))";
	code += "\nself._has_value = other._has_value";
	implement_from_code(f, code);
#else
	auto self = add_node_local(f->__get_var(Identifier::Self), -1);
	auto other = add_node_local(f->__get_var("other"), -1);

	if (auto f_des = t->param[0]->get_destructor()) {
		// if self.has_value
		//     self.value.__delete()

		auto n_del = add_node_member_call(f_des, optional_data(self), -1);
		f->block_node->add(node_if(optional_has_value(self), n_del));
	}


	{
		// if other.has_value
		//     self.data.__init__()
		//     self.data = other.data

		auto b = add_node_block(new Block(f, f->block), common_types._void, -1);

		if (auto f_con = t->param[0]->get_default_constructor()) {
			// self.data.__init__()
			b->add(add_node_member_call(f_con,
										optional_data(self), -1));
		}

		auto op = OperatorID::Assign;
		if (self->type->param[0]->is_reference())
			op = OperatorID::RefAssign;
		if (auto assign = parser->con.link_operator_id(op,
													   optional_data(self), optional_data(other)))
			b->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));

		f->block_node->add(node_if(optional_has_value(other), b));
	}

	{
		// self.has_value = other.has_value
		auto assign = add_node_operator_by_inline(InlineID::BoolAssign,
												  optional_has_value(self),
												  optional_has_value(other), -1);
		f->block_node->add(assign);
	}
#endif
}

void AutoImplementer::implement_result_assign(Function *f, const Class *t) {
	string code;
	code += "if self._state != other._state";
	if (t->param[0]->get_destructor())
		code += "\n\tif self._state == 0x01\n\t\t@noderef(*(&self as X&)).__delete__()";
	code += "\n\tif self._state == 0x02\n\t\t(&self as string&).__delete__()";
	code += "\n\tself._state = other._state";
	if (t->param[0]->get_default_constructor())
		code += "\n\tif self._state == 0x01\n\t\t@noderef(*(&self as X&)).__init__()";
	code += "\n\tif self._state == 0x02\n\t\t(&self as string&).__init__()";

	if (t->param[0]->is_reference())
		code += "\nif self._state == 0x01\n\t@noderef(*(&self as X&)) := @noderef(*(&other as X&))";
	else
		code += "\nif self._state == 0x01\n\t@noderef(*(&self as X&)) = @noderef(*(&other as X&))";
	code += "\nif self._state == 0x02\n\t(&self as string&) = (&other as string&)";
	implement_from_code(f, code);
}

void AutoImplementer::implement_optional_assign_raw(Function *f, const Class *t) {
#if 1
	string code;
	if (t->param[0]->get_default_constructor())
		code += "if not self._has_value\n\t@noderef(*(&self as X&)).__init__()";

	if (t->param[0]->is_reference())
		code += "\n*(&self as i64&) = *(&other as i64&)";
	else
		code += "\n@noderef(*(&self as X&)) = @noderef(*(&other as X&))";
	code += "\nself._has_value = true";
	implement_from_code(f, code);
#else
	auto self = add_node_local(f->__get_var(Identifier::Self), -1);
	auto other = add_node_local(f->__get_var("other"), -1);

	if (auto f_con = t->param[0]->get_default_constructor()) {
		// if not self.has_value
		//     self.value.__init__()
		auto cmd_not = node_not(optional_has_value(self));
		auto n_init = add_node_member_call(f_con, optional_data(self), -1);
		f->block_node->add(node_if(cmd_not, n_init));
	}

	{
		// self.data = other
		auto op = OperatorID::Assign;
		if (other->type->is_reference())
			op = OperatorID::RefAssign;
		if (auto assign = parser->con.link_operator_id(op,
													   optional_data(self),
													   other))
			f->block_node->add(assign);
		else
			do_error_implicit(f, format("no operator %s = %s found", t->param[0]->long_name(), t->param[0]->long_name()));
	}

	{
		// self.has_value = true
		f->block_node->add(add_node_operator_by_inline(InlineID::BoolAssign,
												  optional_has_value(self),
												  node_true(), -1));
	}
#endif
}

void AutoImplementer::implement_optional_assign_null(Function *f, const Class *t) {
	string code;
	if (t->param[0]->get_destructor())
		code += "if self._has_value\n\t@noderef(*(&self as X&)).__delete__()";
	code += "\nself._has_value = false";
	implement_from_code(f, code);
}

void AutoImplementer::implement_optional_has_value(Function *f, const Class *t) {
	implement_from_code(f, "return self._has_value");
}

void AutoImplementer::implement_result_has_value(Function *f, const Class *t) {
	implement_from_code(f, "return self._state == 0x01");
}

void AutoImplementer::implement_result_error(Function *f, const Class *t) {
	implement_from_code(f, "return *(&self as string&)");
}

void AutoImplementer::implement_optional_equal_raw(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, "if not self._has_value\n\treturn false\nreturn @noderef(*(&self as X&)) == @noderef(other)");
}

void AutoImplementer::implement_optional_equal(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, "if self._has_value and other._has_value\n\treturn @noderef(*(&self as X&)) == @noderef(*(&other as X&))\nreturn self._has_value == other._has_value");
}

void AutoImplementer::implement_result_equal_raw(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, "if self._state != 0x01\n\treturn false\nreturn @noderef(*(&self as X&)) == @noderef(other)");
}

void AutoImplementer::implement_result_equal(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, "if self._state == 0x01 and other._state == 0x01\n\treturn @noderef(*(&self as X&)) == @noderef(*(&other as X&))\nreturn self._state == other._state");
}

void AutoImplementer::_implement_functions_for_optional(const Class *t) {
	implement_optional_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_optional_constructor(prepare_auto_impl(t, t->get_member_func(Identifier::func::Init, common_types._void, {common_types.none})), t);
	implement_optional_constructor_value(prepare_auto_impl(t, t->get_member_func(Identifier::func::Init, common_types._void, {t->param[0]})), t);
	implement_optional_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_optional_assign(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {t})), t);
	implement_optional_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {t->param[0]})), t);
	implement_optional_assign_null(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {common_types.none})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func(Identifier::func::OptionalHasValue, common_types._bool, {})), t);
	implement_optional_has_value(prepare_auto_impl(t, t->get_member_func("__bool__", common_types._bool, {})), t);
	implement_optional_equal(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, common_types._bool, {t})), t);
	implement_optional_equal_raw(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, common_types._bool, {t->param[0]})), t);
}



int _make_optional_size(const Class *t) {
	return mem_align(t->size + 1, t->alignment);
}

int _make_result_size(const Class *t) {
	return mem_align(max((int)t->size, config.target.dynamic_array_size) + 1, 8);
}


Class* TemplateClassInstantiatorOptional::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	auto c = create_raw_class(tree, class_name_might_need_parantheses(params[0]) + "?", common_types.optional_t, _make_optional_size(params[0]), params[0]->alignment, 0, nullptr, params, token_id);
	c->type_aliases.set("X", params[0]);
	c->elements.add({"_has_value", common_types._bool, params[0]->size, -1});
	return c;
}
void TemplateClassInstantiatorOptional::add_function_headers(Class* t) {
	if (!class_can_default_construct(t->param[0]))
		t->owner->do_error(format("can not create an optional from type '%s', missing default constructor", t->param[0]->long_name()), t->token_id);

	add_func_header(t, Identifier::func::Init, common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Init, common_types._void, {t->param[0]}, {"value"}, nullptr, Flags::AutoCast | Flags::Mutable);
	add_func_header(t, Identifier::func::Init, common_types._void, {common_types.none}, {"value"}, nullptr, Flags::AutoCast | Flags::Mutable);
	//if (t->param[0]->get_destructor())
	add_func_header(t, Identifier::func::Delete, common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, common_types._void, {t}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, common_types._void, {t->param[0]}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, common_types._void, {common_types.none}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::OptionalHasValue, common_types._bool, {}, {}, nullptr, Flags::Pure);
	add_func_header(t, "__bool__", common_types._bool, {}, {}, nullptr, Flags::Pure);
	//add_func_header(t, "_get_p", t->param[0], {}, {}, nullptr, Flags::REF);
	if (t->param[0]->get_member_func(Identifier::func::Equal, common_types._bool, {t->param[0]})) {
		add_func_header(t, Identifier::func::Equal, common_types._bool, {t}, {"other"}, nullptr, Flags::Pure);
		add_func_header(t, Identifier::func::Equal, common_types._bool, {t->param[0]}, {"other"}, nullptr, Flags::Pure);
	}
}


void AutoImplementer::_implement_functions_for_result(const Class *t) {
	implement_result_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_result_constructor_value(prepare_auto_impl(t, t->get_member_func(Identifier::func::Init, common_types._void, {t->param[0]})), t);
	implement_result_constructor_error(prepare_auto_impl(t, t->get_member_func(Identifier::func::Init, common_types._void, {common_types.error})), t);
	implement_result_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_result_assign(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {t})), t);
	//implement_result_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {t->param[0]})), t);
//	implement_optional_assign_null(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {common_types.none})), t);
	implement_result_has_value(prepare_auto_impl(t, t->get_member_func(Identifier::func::OptionalHasValue, common_types._bool, {})), t);
	implement_result_has_value(prepare_auto_impl(t, t->get_member_func("__bool__", common_types._bool, {})), t);
	implement_result_equal(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, common_types._bool, {t})), t);
	implement_result_equal_raw(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, common_types._bool, {t->param[0]})), t);
}

Class* TemplateClassInstantiatorResult::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	auto v = (params[0] == common_types._void) ? common_types.i32 : params[0];
	auto c = create_raw_class(tree, "result[" + params[0]->name + "]", common_types.result_t, _make_result_size(v), 8, 0, nullptr, {v}, token_id);
	c->type_aliases.set("X", v);
	c->elements.add({"_state", common_types.u8, max((int)v->size, config.target.dynamic_array_size), -1});
	return c;
}
void TemplateClassInstantiatorResult::add_function_headers(Class* t) {
	if (!class_can_default_construct(t->param[0]))
		t->owner->do_error(format("can not create result[...] from type '%s', missing default constructor", t->param[0]->long_name()), t->token_id);

	add_func_header(t, Identifier::func::Init, common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Init, common_types._void, {t->param[0]}, {"value"}, nullptr, Flags::AutoCast | Flags::Mutable);
	add_func_header(t, Identifier::func::Init, common_types._void, {common_types.error}, {"err"}, nullptr, Flags::AutoCast | Flags::Mutable);
	//if (t->param[0]->get_destructor())
	add_func_header(t, Identifier::func::Delete, common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, common_types._void, {t}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, common_types._void, {t->param[0]}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Assign, common_types._void, {common_types.none}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::OptionalHasValue, common_types._bool, {}, {}, nullptr, Flags::Pure);
	add_func_header(t, "__bool__", common_types._bool, {}, {}, nullptr, Flags::Pure);
	//add_func_header(t, "_get_p", t->param[0], {}, {}, nullptr, Flags::REF);
	if (t->param[0]->get_member_func(Identifier::func::Equal, common_types._bool, {t->param[0]})) {
		add_func_header(t, Identifier::func::Equal, common_types._bool, {t}, {"other"}, nullptr, Flags::Pure);
		add_func_header(t, Identifier::func::Equal, common_types._bool, {t->param[0]}, {"other"}, nullptr, Flags::Pure);
	}
}

}



