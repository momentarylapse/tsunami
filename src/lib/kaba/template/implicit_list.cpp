/*
 * implicit_list.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

string class_name_might_need_parantheses(const Class *t);

void AutoImplementer::implement_list_constructor(Function *f, const Class *t) {
	auto te = t->get_array_element();
	implement_from_code(f, format("__mem_init__(%d)", te->size));
}

void AutoImplementer::implement_list_destructor(Function *f, const Class *t) {
	implement_from_code(f, "clear()");
}

void AutoImplementer::implement_list_assign(Function *f, const Class *t) {
	if (!f)
		return;
	auto t_el = t->get_array_element();
	if (t_el->is_reference())
		implement_from_code(f, "resize(other.num)\nfor mut i=>el in self\n\tel := other[i]");
	else
		implement_from_code(f, "resize(other.num)\nfor mut i=>el in self\n\t@noderef(el) = @noderef(other[i])");
}

void AutoImplementer::implement_list_clear(Function *f, const Class *t) {
	auto te = t->get_array_element();
	if (te->get_destructor())
		implement_from_code(f, "for mut el in self\n\t@noderef(el).__delete__()\n__mem_clear__()");
	else if (te->needs_destructor())
		do_error_implicit(f, "element destructor missing");
	else
		implement_from_code(f, "__mem_clear__()");
}

void AutoImplementer::implement_list_resize(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	string code = "let num_old = self.num";
	if (te->get_destructor())
		code += "\nfor i in num:self.num\n\t@noderef(self[i]).__delete__()";
	else if (te->needs_destructor())
		do_error_implicit(f, "element destructor missing");
	code += "\n__mem_resize__(num)";
	if (te->get_default_constructor())
		code += "\nfor i in num_old:num\n\t@noderef(self[i]).__init__()";
	else if (te->needs_constructor())
		do_error_implicit(f, "element default constructor missing");
	implement_from_code(f, code);
}


void AutoImplementer::implement_list_remove(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	if (te->get_destructor())
		implement_from_code(f, "@noderef(self[index]).__delete__()\n__mem_remove__(index)");
	else if (te->needs_destructor())
		do_error_implicit(f, "element destructor missing");
	else
		implement_from_code(f, "__mem_remove__(index)");
}

void AutoImplementer::implement_list_add(Function *f, const Class *t) {
	if (!f)
		return;

	if (t->param[0]->is_reference())
		implement_from_code(f, "resize(self.num + 1)\nself[self.num - 1] := x");
	else
		implement_from_code(f, "resize(self.num + 1)\nself[self.num - 1] = x");
}

void AutoImplementer::implement_list_equal(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, R"foo(if self.num != other.num
	return false
for i=>e in self
	if @noderef(e) != @noderef(other[i])
		return false
return true)foo");
}

void AutoImplementer::implement_list_contains(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, R"foo(for i=>e in self
	if @noderef(e) == @noderef(x)
		return true
return false)foo");
}

void AutoImplementer::implement_list_join_into(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	if (te->is_reference())
		implement_from_code(f, R"foo(let i0 = self.num
self.resize(self.num + other.num)
for i=>e in other
	@noderef(self[i + i0]) := @noderef(e))foo");
	else
		implement_from_code(f, R"foo(let i0 = self.num
self.resize(self.num + other.num)
for i=>e in other
	@noderef(self[i + i0]) = @noderef(e))foo");
}

void AutoImplementer::implement_list_join(Function *f, const Class *t) {
	if (!f)
		return;
	implement_from_code(f, "var r = @noderef(self)\nr |= @noderef(other)\nreturn r");
}

void AutoImplementer::implement_list_give(Function *f, const Class *t) {
	auto t_el = t->get_array_element();
#if 0
	// TODO
	//implement_from_code(f, "var temp: xfer[" + t_el->param[0]->name + "][]\n...\n__mem_forget__()\nreturn temp");
	implement_from_code(f, "var temp: xfer[" + t_el->param[0]->name + "][]\n__mem_forget__()\nreturn temp");
#else
	auto t_xfer = tree->request_implicit_class_xfer(t_el->param[0], -1);
	auto t_xfer_list = tree->request_implicit_class_list(t_xfer, -1);
	auto self = add_node_local(f->__get_var(Identifier::Self), -1);
	auto temp = add_node_local(f->block->add_var("temp", t_xfer_list, -1), -1);

	{
		// memcpy(temp, self)
		f->block_node->add(add_node_operator_by_inline(InlineID::ChunkAssign, temp, self, -1));
	}

	{
		// self.forget()
		f->block_node->add(add_node_member_call(t->get_member_func("__mem_forget__", common_types._void, {}), self, -1));
	}

	{
		// return temp
		f->block_node->add(node_return(temp));
	}
#endif
}

void AutoImplementer::_implement_functions_for_list(const Class *t) {
	implement_list_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_list_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_list_clear(prepare_auto_impl(t, t->get_member_func("clear", common_types._void, {})), t);
	implement_list_resize(prepare_auto_impl(t, t->get_member_func("resize", common_types._void, {common_types.i32})), t);
	implement_list_remove(prepare_auto_impl(t, t->get_member_func("remove", common_types._void, {common_types.i32})), t);
	implement_list_add(prepare_auto_impl(t, t->get_member_func("add", common_types._void, {nullptr})), t);
	if (t->param[0]->is_pointer_owned() or t->param[0]->is_pointer_owned_not_null()) {
		auto t_xfer = tree->request_implicit_class_xfer(t->param[0]->param[0], -1);
		auto t_xfer_list = tree->request_implicit_class_list(t_xfer, -1);
		implement_list_give(prepare_auto_impl(t, t->get_member_func(Identifier::func::OwnedGive, t_xfer_list, {})), t);
		implement_list_assign(prepare_auto_impl(t, t->get_member_func(Identifier::func::Assign, common_types._void, {t_xfer_list})), t);
	}
	implement_list_assign(prepare_auto_impl(t, t->get_assign()), t);
	implement_list_join_into(prepare_auto_impl(t, t->get_member_func(Identifier::func::BitOrAssign, common_types._void, {t})), t);
	implement_list_join(prepare_auto_impl(t, t->get_member_func(Identifier::func::BitOr, t, {t})), t);
	implement_list_equal(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, common_types._bool, {t})), t);
	implement_list_contains(prepare_auto_impl(t, t->get_member_func(Identifier::func::Contains, common_types._bool, {t->param[0]})), t);
}



Class* TemplateClassInstantiatorList::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, class_name_might_need_parantheses(params[0]) + "[]", common_types.list_t, config.target.dynamic_array_size, config.target.pointer_size, -1, common_types.dynamic_array, params, token_id);
}

void TemplateClassInstantiatorList::add_function_headers(Class* c) {
	c->derive_from(common_types.dynamic_array); // we already set its size!
	auto el = c->param[0];
	if (!class_can_default_construct(el))
		c->owner->do_error(format("can not create a list from type '%s', missing default constructor", el->long_name()), c->token_id);
	bool single_ownership = false;

	add_func_header(c, Identifier::func::Init, common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(c, Identifier::func::Delete, common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(c, "clear", common_types._void, {}, {}, nullptr, Flags::Mutable);
	add_func_header(c, "resize", common_types._void, {common_types.i32}, {"num"}, nullptr, Flags::Mutable);
	if (el->is_pointer_owned() or el->is_pointer_owned_not_null()) {
		single_ownership = true;
		auto t_xfer = c->owner->request_implicit_class_xfer(el->param[0], -1);
		auto t_xfer_list = c->owner->request_implicit_class_list(t_xfer, -1);
		add_func_header(c, "add", common_types._void, {t_xfer}, {"x"}, nullptr, Flags::Mutable);
		add_func_header(c, Identifier::func::OwnedGive, t_xfer_list, {}, {}, nullptr, Flags::Mutable);
		//add_func_header(c, Identifier::Func::ASSIGN, common_types._void, {t_xfer_list}, {"other"});
		add_func_header(c, Identifier::func::Assign, common_types._void, {t_xfer_list}, {"other"}, nullptr, Flags::Mutable);
	} else if (el->is_pointer_xfer_not_null()) {
		//	add_func_header(c, "add", common_types._void, {el}, {"x"});
		add_func_header(c, Identifier::func::Assign, common_types._void, {c}, {"other"}, nullptr, Flags::Mutable);
	} else if (el->is_reference()) {
		add_func_header(c, "add", common_types._void, {el}, {"x"}, nullptr, Flags::Mutable);
		add_func_header(c, Identifier::func::Assign, common_types._void, {c}, {"other"}, nullptr, Flags::Mutable);
	} else {
		add_func_header(c, "add", common_types._void, {el}, {"x"}, nullptr, Flags::Mutable);
		if (class_can_assign(el))
			add_func_header(c, Identifier::func::Assign, common_types._void, {c}, {"other"}, nullptr, Flags::Mutable);
	}
	add_func_header(c, "remove", common_types._void, {common_types.i32}, {"index"}, nullptr, Flags::Mutable);
	if (!single_ownership and class_can_assign(el)) {
		add_func_header(c, Identifier::func::BitOrAssign, common_types._void, {c}, {"other"}, nullptr, Flags::Mutable);
		add_func_header(c, Identifier::func::BitOr, c, {c}, {"other"}, nullptr, Flags::Pure);
	}
	if (class_can_equal(el)) {
		add_func_header(c, Identifier::func::Equal, common_types._bool, {c}, {"other"}, nullptr, Flags::Pure);
		add_func_header(c, Identifier::func::Contains, common_types._bool, {el}, {"x"}, nullptr, Flags::Pure);
	}
}

}


