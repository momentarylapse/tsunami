/*
 * implicit_pointer.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

#include "../../os/msg.h"

namespace kaba {

extern const Class* TypeNone;


/*static shared<Node> shared_p(shared<Node> n) {
	return n->change_type(tree->get_pointer(t->param[0]));
}*/
#define SHARED_P(N)       (N->change_type(tree->get_pointer(t->param[0])))
//#define SHARED_COUNTER(N) (SHARED_P(self)->deref()->shift(e.offset, e.type))



struct XX {
	void __init__() {
		msg_write("XX.init");
		msg_write(p2s(this));
		p = nullptr;
		msg_write(p2s(p));
	}
	void __del__() {
		msg_write("XX.del");
		msg_write(p2s(this));
		msg_write(p2s(p));
	}
	int *p;
};


void AutoImplementer::implement_shared_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.p = nil
	f->block->add(add_node_operator_by_inline(InlineID::POINTER_ASSIGN,
			SHARED_P(self),
			node_nil()));
}

void AutoImplementer::implement_shared_destructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.clear()
	if (auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {}))
		f->block->add(add_node_member_call(f_clear, self));
	else
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
}

//
void AutoImplementer::implement_shared_assign(Function *f, const Class *t) {
	auto other = add_node_local(f->__get_var("other"));
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.clear()
	auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	auto call_clear = add_node_member_call(f_clear, self);
	f->block->add(call_clear);


	auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(self), other);
	f->block->add(op);


	// if other
	//     other.count ++

	// if other
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, other);

	auto b = new Block(f, f->block.get());

	f->block->add(node_if(cmd_cmp, b));

	auto tt = self->type->param[0];
	bool found = false;
	for (auto &e: tt->elements)
		if (e.name == Identifier::SHARED_COUNT and e.type == TypeInt32) {
			// count ++
			auto count = SHARED_P(self)->deref()->shift(e.offset, e.type);
			auto inc = add_node_operator_by_inline(InlineID::INT32_INCREASE, count, nullptr);
			b->add(inc);
			found = true;
		}
	if (!found)
		do_error_implicit(f, format("class '%s' is not a shared class (declare with '%s class' or add an element 'int %s')", tt->long_name(), Identifier::SHARED, Identifier::SHARED_COUNT));
}

void AutoImplementer::implement_shared_clear(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto tt = t->param[0];

	// if self.p
	//     self.p.count --
	//     if self.p.count == 0
	//         del self.p
	//     self.p = nil

	//auto cmd_if = add_node_statement(StatementID::IF);

	// if self.p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, SHARED_P(self));

	auto b = new Block(f, f->block.get());


	shared<Node> count;
	for (auto &e: tt->elements)
		if (e.name == Identifier::SHARED_COUNT and e.type == TypeInt32)
			count = SHARED_P(self)->deref_shift(e.offset, e.type, -1);
	if (!count)
		do_error_implicit(f, format("class '%s' is not a shared class (declare with '%s class' or add an element 'int %s')", tt->long_name(), Identifier::SHARED, Identifier::SHARED_COUNT));

	// count --
	auto dec = add_node_operator_by_inline(InlineID::INT32_DECREASE, count, nullptr);
	b->add(dec);


	auto cmd_if_del = add_node_statement(StatementID::IF);

	// if count == 0
	auto cmp = add_node_operator_by_inline(InlineID::INT32_EQUAL, count, const_int(0));
	cmd_if_del->set_param(0, cmp);

	auto b2 = new Block(f, b);


	// del self.p
	auto cmd_del = add_node_statement(StatementID::DELETE);
	cmd_del->set_param(0, SHARED_P(self));
	b2->add(cmd_del);
	cmd_if_del->set_param(1, b2);
	b->add(cmd_if_del);


	// self.p = nil
	auto n_op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(self), node_nil());
	b->add(n_op);

	f->block->add(node_if(cmd_cmp, b));
}


void AutoImplementer::implement_shared_create(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto r = add_node_local(f->block->add_var("r", t));


	// r = p
	if (auto f_assign = t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {p->type})) {
		auto call_assign = add_node_member_call(f_assign, r);
		call_assign->set_param(1, p);
		f->block->add(call_assign);
	} else {
		do_error_implicit(f, "= missing...");
	}

	// return r
	f->block->add(node_return(r));
}

void AutoImplementer::implement_owned_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.p = nil
	f->block->add(add_node_operator_by_inline(InlineID::POINTER_ASSIGN,
			SHARED_P(self),
			node_nil()));
}

void AutoImplementer::implement_owned_destructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	//db_add_print_label(f->block.get(), "owned del a");

	// self.clear()
	if (auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {}))
		f->block->add(add_node_member_call(f_clear, self));
	else
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	//db_add_print_label(f->block.get(), "owned del b");
}

// TODO prevent self-assignment...
void AutoImplementer::implement_owned_assign_raw(Function *f, const Class *t) {
	auto other = add_node_local(f->__get_var("other"));
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// self.clear()
	if (auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {})) {
		auto call_clear = add_node_member_call(f_clear, self);
		f->block->add(call_clear);
	} else {
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	}

	{
		// self.p = other
		auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(self), other);
		f->block->add(op);
	}
}

// TODO prevent self-assignment...
void AutoImplementer::implement_owned_assign(Function *f, const Class *t) {
	auto other = add_node_local(f->__get_var("other"));
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	//db_add_print_label(f->block.get(), "owned = a");

	// self.clear()
	if (auto f_clear = t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {})) {
		auto call_clear = add_node_member_call(f_clear, self);
		f->block->add(call_clear);
	} else {
		do_error_implicit(f, Identifier::Func::SHARED_CLEAR + "() missing");
	}
	//db_add_print_label(f->block.get(), "owned = b");

	{
		// self.p = other.p
		auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(self), SHARED_P(other));
		f->block->add(op);
	}

	{
		// "forget"
		// other.p = nil
		auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(other), node_nil());
		f->block->add(op);
	}
	//db_add_print_label(f->block.get(), "owned = c");
}

void AutoImplementer::implement_owned_clear(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// if self.p
	//     del self.p
	//     self.p = nil

	//auto cmd_if = add_node_statement(StatementID::IF);

	// if self.p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, SHARED_P(self));

	auto b = new Block(f, f->block.get());


	// del self.p
	auto cmd_del = add_node_statement(StatementID::DELETE);
	cmd_del->set_param(0, SHARED_P(self));
	b->add(cmd_del);


	// self.p = nil
	auto n_op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(self), node_nil());
	b->add(n_op);

	f->block->add(node_if(cmd_cmp, b));
}

void AutoImplementer::implement_owned_give(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));
	auto r = add_node_local(f->block->add_var("r", TypePointer));

	// let r = self.p
	{
		auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, r, SHARED_P(self));
		f->block->add(op);
	}

	// "forget"
	// self.p = nil
	{
		auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, SHARED_P(self), node_nil());
		f->block->add(op);
	}

	// return r
	f->block->add(node_return(r));
}

void AutoImplementer::_implement_functions_for_shared(const Class *t) {
	auto t_xfer = tree->request_implicit_class_xfer(t->param[0], -1);
	implement_shared_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_shared_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_shared_clear(prepare_auto_impl(t, t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {})), t);
	implement_shared_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t_xfer})), t);
	implement_shared_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {TypeNone})), t);
	implement_shared_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t})), t);
	implement_shared_create(prepare_auto_impl(t, t->get_func(Identifier::Func::SHARED_CREATE, t, {t_xfer})), t);
}

void AutoImplementer::_implement_functions_for_owned(const Class *t) {
	[[maybe_unused]] auto t_p = tree->get_pointer(t->param[0]);
	auto t_xfer = tree->request_implicit_class_xfer(t->param[0], -1);
	implement_owned_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_owned_destructor(prepare_auto_impl(t, t->get_destructor()), t);
	implement_owned_clear(prepare_auto_impl(t, t->get_member_func(Identifier::Func::SHARED_CLEAR, TypeVoid, {})), t);
	implement_owned_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t_xfer})), t);
	implement_owned_assign_raw(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {TypeNone})), t);
	//implement_owned_assign(prepare_auto_impl(t, t->get_member_func(Identifier::Func::ASSIGN, TypeVoid, {t})), t);
	//implement_shared_create(prepare_auto_impl(t, t->get_func(Identifier::Func::SHARED_CREATE, t, {nullptr, tree->get_pointer(t->param[0])})), t);
	implement_owned_give(prepare_auto_impl(t, t->get_member_func(Identifier::Func::OWNED_GIVE, t_xfer, {})), t);
}

void AutoImplementer::_implement_functions_for_xfer(const Class *t) {
}

void AutoImplementer::_implement_functions_for_alias(const Class *t) {
}



Class* TemplateClassInstantiatorPointerRaw::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, class_name_might_need_parantheses(params[0]) + "*", TypeRawT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
	//return create_class(format("%s[%s]", Identifier::RAW_POINTER, params[0]->name), Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, params, token_id);
}

void TemplateClassInstantiatorPointerRaw::add_function_headers(Class* c) {
	flags_set(c->flags, Flags::FORCE_CALL_BY_VALUE);
};



Class* TemplateClassInstantiatorReference::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, class_name_might_need_parantheses(params[0]) + "&", TypeReferenceT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorReference::add_function_headers(Class* c) {
	flags_set(c->flags, Flags::FORCE_CALL_BY_VALUE);
}


Class* TemplateClassInstantiatorPointerShared::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s[%s]", Identifier::SHARED, params[0]->name), TypeSharedT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorPointerShared::add_function_headers(Class* t) {
	//flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE); // FIXME why not?!?
	//t->derive_from(TypeSharedPointer);

	auto t_xfer = t->owner->request_implicit_class_xfer(t->param[0], -1);
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::SHARED_CLEAR, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	// do we really need this, or can we use auto cast xfer[X] -> shared[X]?!?
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t_xfer}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {TypeNone}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::SHARED_CREATE, t, {t_xfer}, {"p"}, nullptr, Flags::STATIC);
}


Class* TemplateClassInstantiatorPointerSharedNotNull::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s![%s]", Identifier::SHARED, params[0]->name), TypeSharedNotNullT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}


Class* TemplateClassInstantiatorPointerOwned::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s[%s]", Identifier::OWNED, params[0]->name), TypeOwnedT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorPointerOwned::add_function_headers(Class* t) {
	flags_set(t->flags, Flags::FORCE_CALL_BY_VALUE);


	[[maybe_unused]] auto t_p = t->owner->get_pointer(t->param[0]);
	auto t_xfer = t->owner->request_implicit_class_xfer(t->param[0], -1);
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	//	f->address_preprocess = mf(&XX::__del__);
	//	f->address = (int_p)f->address_preprocess;
	add_func_header(t, Identifier::Func::SHARED_CLEAR, TypeVoid, {}, {}, nullptr, Flags::MUTABLE);
	//f->address_preprocess = mf(&XX::__del__);
	//f->address = (int_p)f->address_preprocess;
	add_func_header(t, Identifier::Func::OWNED_GIVE, t_xfer, {}, {}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t_xfer}, {"other"}, nullptr, Flags::MUTABLE);
	add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {TypeNone}, {"other"}, nullptr, Flags::MUTABLE);
	//auto assign = add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"});
	//flags_set(assign->var.back()->flags, Flags::OUT);
	//add_func_header(t, Identifier::Func::SHARED_CREATE, t, {t->param[0]->get_pointer()}, {"p"}, nullptr, Flags::STATIC);
}


Class* TemplateClassInstantiatorPointerOwnedNotNull::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s![%s]", Identifier::OWNED, params[0]->name), TypeOwnedNotNullT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}


Class* TemplateClassInstantiatorPointerXfer::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s[%s]", Identifier::XFER, params[0]->name), TypeXferT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorPointerXfer::add_function_headers(Class* c) {
	flags_set(c->flags, Flags::FORCE_CALL_BY_VALUE);

	auto assign = add_func_header(c, Identifier::Func::ASSIGN, TypeVoid, {c}, {"other"});
	assign->inline_no = InlineID::POINTER_ASSIGN;
}


Class* TemplateClassInstantiatorPointerAlias::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s[%s]", Identifier::ALIAS, params[0]->name), TypeAliasT, config.target.pointer_size, config.target.pointer_size, 0, nullptr, params, token_id);
}

void TemplateClassInstantiatorPointerAlias::add_function_headers(Class* c) {
	flags_set(c->flags, Flags::FORCE_CALL_BY_VALUE);

	auto assign = add_func_header(c, Identifier::Func::ASSIGN, TypeVoid, {c}, {"other"});
	assign->inline_no = InlineID::POINTER_ASSIGN;
}

}



