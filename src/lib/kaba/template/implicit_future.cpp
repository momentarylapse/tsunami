/*
 * implicit_future.cpp
 *
 *  Created on: 24 Sep 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit_future.h"
#include "template.h"
#include "../parser/Parser.h"
#include "../../os/msg.h"
#include "../../base/future.h"

namespace kaba {

void AutoImplementerFutureCore::add_missing_function_headers(Class *t) {
	//auto t_shared_core = tree->request_implicit_class_shared_not_null(BLA, -1);
	msg_error("NEW FUTURE CORE: ");
	msg_write(t->param[0]->long_name());
	auto t_callback = tree->request_implicit_class_callable_fp({t->param[0]}, TypeVoid, -1);
	auto t_callback_fail = tree->request_implicit_class_callable_fp({}, TypeVoid, -1);
	add_func_header(t, Identifier::func::Init, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, Identifier::func::Delete, TypeVoid, {}, {}, nullptr, Flags::Mutable);
	add_func_header(t, "then", TypeVoid, {t_callback}, {"f"});
	add_func_header(t, "then_or_fail", TypeVoid, {t_callback, t_callback_fail}, {"f"});
	//add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"});
}

void AutoImplementerFutureCore::complete_type(Class *t) {
	add_missing_function_headers(t);
	auto t_callback = tree->request_implicit_class_callable_fp({t->param[0]}, TypeVoid, -1);
	auto t_callback_fail = tree->request_implicit_class_callable_fp({}, TypeVoid, -1);
	t->elements.add(ClassElement(Identifier::SharedCount, TypeInt32, 0));
	t->elements.add(ClassElement("cb_success", t_callback, 4));
	t->elements.add(ClassElement("cb_fail", t_callback_fail, 4 + config.target.pointer_size));
	t->elements.add(ClassElement("state", TypeInt32, 4 + config.target.pointer_size * 2));
	t->elements.add(ClassElement("result", t->param[0], 8 + config.target.pointer_size * 2));
}





void AutoImplementerFuture::add_missing_function_headers(Class *t) {
	auto t_callback = tree->request_implicit_class_callable_fp({t->param[0]}, TypeVoid, -1);
	auto t_callback_fail = tree->request_implicit_class_callable_fp({}, TypeVoid, -1);
	//add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {});
	add_func_header(t, Identifier::func::Delete, TypeVoid, {}, {}, nullptr, Flags::Mutable | Flags::Extern);
	add_func_header(t, "then", TypeVoid, {t_callback}, {"f"}, nullptr, Flags::Extern);
	add_func_header(t, "then_or_fail", TypeVoid, {t_callback, t_callback_fail}, {"f", "f_fail"}, nullptr, Flags::Extern);
	//add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"});
}

void AutoImplementerFuture::implement_future_constructor(Function *f, const Class *t) {
	/*auto self = add_node_local(f->__get_var(Identifier::SELF));

	auto te = t->get_array_element();
	auto ff = t->get_member_func("__mem_init__", TypeVoid, {TypeInt32});
	f->block->add(add_node_member_call(ff,
			self, -1,
			{add_node_const(tree->add_constant_int(te->size + TypeString->size))}));*/
}

void AutoImplementerFuture::implement_functions(const Class *t) {
	/*implement_list_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	implement_list_destructor(prepare_auto_impl(t, t->get_destructor()), t);*/
}

void AutoImplementerFuture::complete_type(Class *t) {
	add_missing_function_headers(t);
/*	auto t_core = context->template_manager->request_futurecore(tree, t->param[0], t->token_id);
	auto t_core_p = context->template_manager->request_shared_not_null(tree, t_core, t->token_id);
	t->elements.add(ClassElement("core", t_core_p, 0));*/
}




Class* TemplateClassInstantiatorFuture::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("%s[%s]", Identifier::Future, params[0]->name), TypeFutureT, sizeof(base::future<void>), config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorFuture::add_function_headers(Class* c) {
	AutoImplementerFuture ai(nullptr, c->owner);
	ai.complete_type(c);
}

Class* TemplateClassInstantiatorFutureCore::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	return create_raw_class(tree, format("@futurecore[%s]", params[0]->name), TypeFutureCoreT, sizeof(base::_promise_core_<void>) + params[0]->size, config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorFutureCore::add_function_headers(Class* c) {
	AutoImplementerFutureCore ai(nullptr, c->owner);
	ai.complete_type(c);
}

}



