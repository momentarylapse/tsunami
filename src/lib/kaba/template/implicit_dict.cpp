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

extern const Class* TypeNoValueError;
extern const Class* TypeStringList;
extern const Class* TypeDictBase;
extern const Class* TypeDynamicArray;

Array<string> dict_get_keys(const DynamicArray& a);

static shared<Node> sa_num(shared<Node> node) {
	return node->shift(config.target.pointer_size, TypeInt32);
}
int dict_row_size(const Class *t_val) {
	return mem_align(t_val->size,config.target.pointer_size) + TypeString->size;
}

void AutoImplementer::implement_dict_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));

	auto te = t->get_array_element();
	auto ff = t->get_member_func("__mem_init__", TypeVoid, {TypeInt32});
	f->block->add(add_node_member_call(ff,
			self, -1,
			{const_int(dict_row_size(te))}));
}

void AutoImplementer::implement_dict_clear(Function *f, const Class *t) {
	auto te = t->get_array_element();

	auto self = add_node_local(f->__get_var(Identifier::Self));

// delete...

	auto *var_key = f->block->add_var("k", tree->request_implicit_class_reference(TypeString, -1));
	auto *var_val = f->block->add_var("v", tree->request_implicit_class_reference(te, -1));

	Block *b = new Block(f, f->block.get());

	// key.__delete__()
	if (auto f_del = TypeString->get_destructor()) {
		auto key = add_node_local(var_key)->deref(TypeString);
		auto cmd_delete = add_node_member_call(f_del, key);
		b->add(cmd_delete);
	}

	// value.__delete__()
	if (auto f_del = te->get_destructor()) {
		auto value = add_node_local(var_val)->deref_shift(TypeString->size, te, -1);
		auto cmd_delete = add_node_member_call(f_del, value);
		b->add(cmd_delete);
	} else if (te->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	auto cmd_for = add_node_statement(StatementID::ForContainer);
	cmd_for->set_param(0, add_node_local(var_val));
	cmd_for->set_param(1, add_node_local(var_key));
	cmd_for->set_param(2, self);
	cmd_for->set_param(3, b);

	f->block->add(cmd_for);

	{
		// clear
		auto cmd_clear = add_node_member_call(t->get_member_func("__mem_clear__", TypeVoid, {}), self);
		f->block->add(cmd_clear);
	}
}

void AutoImplementer::implement_dict_assign(Function *f, const Class *t) {

	auto te = t->get_array_element();

	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	auto *var_key = f->block->add_var("k", tree->request_implicit_class_reference(TypeString, -1));
	auto *var_val = f->block->add_var("v", tree->request_implicit_class_reference(te, -1));


	{
		// self.clear()
		auto cmd_clear = add_node_member_call(t->get_member_func("clear", TypeVoid, {}), self);
		f->block->add(cmd_clear);
	}

	Block *b_loop = new Block(f, f->block.get());

	{
		// other.set(key, value)
		auto cmd_set = add_node_member_call(t->get_member_func(Identifier::func::Set, TypeVoid, {TypeString, te}), self);
		cmd_set->set_param(1, add_node_local(var_key)->deref());
		cmd_set->set_param(2, add_node_local(var_val)->deref());
		b_loop->add(cmd_set);
	}

	auto cmd_for = add_node_statement(StatementID::ForContainer);
	cmd_for->set_param(0, add_node_local(var_val));
	cmd_for->set_param(1, add_node_local(var_key));
	cmd_for->set_param(2, other);
	cmd_for->set_param(3, b_loop);

	f->block->add(cmd_for);
}

void AutoImplementer::implement_dict_get(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto te_ref_opt = f->literal_return_type;
	auto te_ref = te_ref_opt->param[0];

	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto in_key = add_node_local(f->__get_var("key"));

	auto *var_key = f->block->add_var("k", tree->request_implicit_class_reference(TypeString, -1));
	auto *var_val = f->block->add_var("v", tree->request_implicit_class_reference(te, -1));

	Block *b_loop = new Block(f, f->block.get());

	{
		// if key == in_key
		//     return T&?(&value)
		Block *b_if = new Block(f, b_loop);
		auto ret = add_node_statement(StatementID::Return);
		ret->set_num_params(1);
		if (auto ff = te_ref_opt->get_func(Identifier::func::Init, TypeVoid, {nullptr, te_ref})) {
			auto c = add_node_constructor(ff, t->token_id);
			c->set_num_params(2);
			c->set_param(1, add_node_local(var_val));
			ret->set_param(0, c);
		} else {
			do_error_implicit(f, "aaaaa1");
		}
		b_if->add(ret);

		auto eq = add_equal(f, "...", add_node_local(var_key)->deref(), in_key);

		auto cmd_if = add_node_statement(StatementID::If);
		cmd_if->set_param(0, eq);
		cmd_if->set_param(1, b_if);
		b_loop->add(cmd_if);
	}

	auto cmd_for = add_node_statement(StatementID::ForContainer);
	cmd_for->set_param(0, add_node_local(var_val));
	cmd_for->set_param(1, add_node_local(var_key));
	cmd_for->set_param(2, self);
	cmd_for->set_param(3, b_loop);

	f->block->add(cmd_for);

	{

		// return T&?()
		auto ret = add_node_statement(StatementID::Return);
		ret->set_num_params(1);
		if (auto ff = te_ref_opt->get_default_constructor()) {
			ret->set_param(0, add_node_constructor(ff, t->token_id));
		} else {
			do_error_implicit(f, "aaaaa");
		}
		f->block->add(ret);
	}
}

void AutoImplementer::implement_dict_set(Function *f, const Class *t) {
	auto te = t->get_array_element();

	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto in_key = add_node_local(f->__get_var("key"));
	auto in_value = add_node_local(f->__get_var("value"));

// delete...

	auto *var_key = f->block->add_var("k", tree->request_implicit_class_reference(TypeString, -1));
	auto *var_val = f->block->add_var("v", tree->request_implicit_class_reference(te, -1));

	Block *b_loop = new Block(f, f->block.get());

	Block *b_if = new Block(f, b_loop);
	b_if->add(add_assign(f, "...", add_node_local(var_val)->deref(), in_value));
	b_if->add(add_node_statement(StatementID::Return));

	// if key == in_key
	//     value = in_value
	//     return
	auto key = add_node_local(var_key)->deref();
	auto eq = add_equal(f, "...", key, in_key);

	auto cmd_if = add_node_statement(StatementID::If);
	cmd_if->set_param(0, eq);
	cmd_if->set_param(1, b_if);
	b_loop->add(cmd_if);

	auto cmd_for = add_node_statement(StatementID::ForContainer);
	cmd_for->set_param(0, add_node_local(var_val));
	cmd_for->set_param(1, add_node_local(var_key));
	cmd_for->set_param(2, self);
	cmd_for->set_param(3, b_loop);

	f->block->add(cmd_for);

	{
		// __mem_resize__(self.num + 1)
		auto cmd_add = add_node_operator_by_inline(InlineID::Int32Add, sa_num(self), const_int(1));
		auto cmd_resize = add_node_member_call(t->get_member_func("__mem_resize__", TypeVoid, {TypeInt32}), self);
		cmd_resize->set_param(1, cmd_add);
		f->block->add(cmd_resize);
	}
	auto t_row = t->classes[0].get();
	auto cmd_sub = add_node_operator_by_inline(InlineID::Int32Subtract, sa_num(self), const_int(1));
	auto back_row = add_node_parray(self->change_type(tree->type_ref(t_row, -1)), cmd_sub, t_row);
	auto back_key = back_row->change_type(TypeString);
	auto back_value = back_row->shift(TypeString->size, te, -1);
	if (auto f_init = TypeString->get_default_constructor()) {
		// back.key.__init__()
		f->block->add(add_node_member_call(f_init, back_key));
	}
	if (auto f_init = te->get_default_constructor()) {
		// back.value.__init__()
		f->block->add(add_node_member_call(f_init, back_value));
	}
	// back.key = in_key
	f->block->add(add_assign(f, "...", back_key, in_key));
	// back.value = in_value
	f->block->add(add_assign(f, "...", back_value, in_value));
}

void AutoImplementer::implement_dict_contains(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto in_key = add_node_local(f->__get_var("key"));

	auto *var_key = f->block->add_var("k", tree->request_implicit_class_reference(TypeString, -1));
	auto *var_val = f->block->add_var("v", TypeReference);

	Block *b_loop = new Block(f, f->block.get());

	{
		// if key == in_key
		//     return true
		Block *b_if = new Block(f, b_loop);
		auto ret = add_node_statement(StatementID::Return);
		ret->set_num_params(1);
		ret->set_param(0, node_true());
		b_if->add(ret);

		auto eq = add_equal(f, "...", add_node_local(var_key)->deref(), in_key);

		auto cmd_if = add_node_statement(StatementID::If);
		cmd_if->set_param(0, eq);
		cmd_if->set_param(1, b_if);
		b_loop->add(cmd_if);
	}

	auto cmd_for = add_node_statement(StatementID::ForContainer);
	cmd_for->set_param(0, add_node_local(var_val));
	cmd_for->set_param(1, add_node_local(var_key));
	cmd_for->set_param(2, self);
	cmd_for->set_param(3, b_loop);

	f->block->add(cmd_for);

	{
		// return false
		auto ret = add_node_statement(StatementID::Return);
		ret->set_num_params(1);
		ret->set_param(0, node_false());
		f->block->add(ret);
	}
//	f->block->show();
}

void AutoImplementer::_implement_functions_for_dict(const Class *t) {
	auto t_el = t->param[0];

	implement_dict_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_dict_clear(prepare_auto_impl(t, t->get_destructor()), t);
	implement_dict_clear(prepare_auto_impl(t, t->get_member_func("clear", TypeVoid, {})), t);
	implement_dict_set(prepare_auto_impl(t, t->get_member_func(Identifier::func::Set, TypeVoid, {TypeString, nullptr})), t);
	implement_dict_get(prepare_auto_impl(t, t->get_get(TypeString)), t);
	implement_dict_contains(prepare_auto_impl(t, t->get_member_func(Identifier::func::Contains, TypeBool, {TypeString})), t);
	implement_dict_assign(prepare_auto_impl(t, t->get_assign()), t);
}



Class* TemplateClassInstantiatorDict::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, class_name_might_need_parantheses(params[0]) + "{}", TypeDictT, config.target.dynamic_array_size, config.target.pointer_size, 0, TypeDictBase, params, token_id);
}
void TemplateClassInstantiatorDict::add_function_headers(Class* c) {
	auto params = c->param; // ->derive_from() will overwrite params!!!
	c->derive_from(TypeDynamicArray); // we already set its size!
	if (!class_can_default_construct(params[0]))
		c->owner->do_error(format("can not create a dynamic array from type '%s', missing default constructor", params[0]->long_name()), c->token_id);
	c->param = params;

	auto t_value = c->param[0];
	auto t_value_ref = c->owner->request_implicit_class_reference(t_value, c->token_id);
	auto t_value_ref_opt = c->owner->request_implicit_class_optional(t_value_ref, c->token_id);
	add_func_header(c, Identifier::func::Init, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(c, Identifier::func::Delete, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(c, "clear", TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(c, Identifier::func::Set, TypeVoid, {TypeString, t_value}, {"key", "value"}, nullptr, Flags::Mutable);
	add_func_header(c, Identifier::func::Get, t_value_ref_opt, {TypeString}, {"key"});
	add_func_header(c, Identifier::func::Assign, TypeVoid, {c}, {"other"}, nullptr, Flags::Mutable);
	add_func_header(c, Identifier::func::Contains, TypeBool, {TypeString}, {"key"}, nullptr, Flags::Pure);

	add_class(c);
		class_add_func("keys", TypeStringList, &dict_get_keys, Flags::Pure);

	[[maybe_unused]] auto t_row = c->owner->create_new_class_no_check("Row", nullptr, dict_row_size(t_value), 0, nullptr, {}, c, -1);
}

}



