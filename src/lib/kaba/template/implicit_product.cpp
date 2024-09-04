/*
 * implicit_product.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"
#include "../../base/iter.h"

namespace kaba {

// using constructor/destructor/assign from regular!

void AutoImplementer::implement_product_equal(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	for (auto& e: t->elements) {
		// if self.e != other.e
		//     return false

		auto cmd_if = add_node_statement(StatementID::If);
		cmd_if->set_param(0, add_not_equal(f, "", self->shift(e.offset, e.type), other->shift(e.offset, e.type)));

		auto b = new Block(f, f->block.get());

		auto cmd_ret = add_node_statement(StatementID::Return);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_false());
		b->add(cmd_ret);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// return true
		auto cmd_ret = add_node_statement(StatementID::Return);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_true());
		f->block->add(cmd_ret);
	}
}

void AutoImplementer::implement_product_not_equal(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::Self));
	auto other = add_node_local(f->__get_var("other"));

	for (auto& e: t->elements) {
		// if self.e == other.e
		//     return false

		auto cmd_if = add_node_statement(StatementID::If);
		cmd_if->set_param(0, add_equal(f, "", self->shift(e.offset, e.type), other->shift(e.offset, e.type)));

		auto b = new Block(f, f->block.get());

		auto cmd_ret = add_node_statement(StatementID::Return);
		cmd_ret->set_num_params(1);
		cmd_ret->set_param(0, node_false());
		b->add(cmd_ret);

		cmd_if->set_param(1, b);
		f->block->add(cmd_if);
	}

	{
		// return true
		auto cmd_ret = add_node_statement(StatementID::Return);
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
	implement_product_equal(prepare_auto_impl(t, t->get_member_func(Identifier::func::Equal, TypeBool, {t})), t); // if exists...
	implement_product_not_equal(prepare_auto_impl(t, t->get_member_func(Identifier::func::NotEqual, TypeBool, {t})), t); // if exists...
}



static string product_class_name(const Array<const Class*> &classes) {
	string name;
	for (auto &c: classes) {
		if (name != "")
			name += ",";
		name += c->name;
	}
	return "("+name+")";
}

static int product_class_size(const Array<const Class*> &classes) {
	int size = 0;
	int total_align = 1;
	for (auto &c: classes) {
		total_align = max(total_align, c->alignment);
		size = mem_align(size, c->alignment);
		size += c->size;
	}
	size = mem_align(size, total_align);
	return size;
}

static int product_class_alignment(const Array<const Class*> &classes) {
	int align = 1;
	for (auto &c: classes)
		align = max(align, c->alignment);
	return align;
}

Class* TemplateClassInstantiatorProduct::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, product_class_name(params), TypeProductT, product_class_size(params), product_class_alignment(params), 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorProduct::add_function_headers(Class* t) {
	int offset = 0;
	for (auto&& [i,cc]: enumerate(t->param)) {
		offset = mem_align(offset, cc->alignment);
		t->elements.add(ClassElement(format("e%d", i), cc, offset));
		offset += cc->size;
	}

	AutoImplementer ai(nullptr, t->owner);
	if (t->needs_constructor())
		add_func_header(t, Identifier::func::Init, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	if (class_can_fully_construct(t))
		ai.add_full_constructor(t);
	if (t->needs_destructor())
		add_func_header(t, Identifier::func::Delete, TypeVoid, {}, {}, nullptr, Flags::Mutable);

	bool allow_assign = true;
	bool allow_equal = true;
	for (const auto p: t->param) {
		if (!class_can_assign(p))
			allow_assign = false;
		if (!class_can_equal(p))
			allow_equal = false;
	}

	if (allow_assign) {
		add_func_header(t, Identifier::func::Assign, TypeVoid, {t}, {"other"}, t->get_assign(), Flags::Mutable);
		if (t->can_memcpy())
			t->get_assign()->inline_no = InlineID::ChunkAssign;
	}
	if (allow_equal) {
		add_func_header(t, Identifier::func::Equal, TypeBool, {t}, {"other"}, nullptr, Flags::Pure);
		add_func_header(t, Identifier::func::NotEqual, TypeBool, {t}, {"other"}, nullptr, Flags::Pure);
	}
}

}



