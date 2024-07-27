/*
 * implicit_callable.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"
#include "../../base/iter.h"

namespace kaba {

extern const Class *TypeCallableBase;

Array<const Class*> get_callable_param_types(const Class *fp);
const Class *get_callable_return_type(const Class *fp);
Array<const Class*> get_callable_capture_types(const Class *fp);



shared<Node> get_callable_fp(const Class *t, shared<Node> self) {
	for (auto &e: t->elements)
		if (e.name == "_fp")
			return self->shift(e.offset, e.type);
	return nullptr;
}

static const Array<string> DUMMY_PARAMS = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};


Array<const Class*> suggest_callable_bind_param_types(const Class *fp) {
	Array<const Class*> r;
	for (int i=0; i<fp->param.num-1; i++)
		if ((fp->array_length & (1 << i)) == 0)
			r.add(fp->param[i]);
	return r;
}


void AutoImplementer::implement_callable_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	implement_add_virtual_table(self, f, t);

	// self.fp = p
	{
		auto n_p = add_node_local(f->__get_var("p"));
		auto fp = get_callable_fp(t, self);
		f->block->add(add_assign(f, "", format("no operator %s = %s for element \"%s\"", fp->type->long_name(), fp->type->long_name(), "_fp"), fp, n_p));
	}

	int i_capture = 0;
	for (auto &e: t->elements)
		if (e.name.head(7) == "capture") {
			auto n_p = add_node_local(f->__get_var(DUMMY_PARAMS[i_capture ++]));
			auto fp = self->shift(e.offset, e.type);
			f->block->add(add_assign(f, "", format("no operator %s = %s for element \"%s\"", fp->type->long_name(), fp->type->long_name(), e.name), fp, n_p));
		}
}


void AutoImplementer::implement_callable_fp_call(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	//db_add_print_label(this, f->block, "== callable.call ==");

	// contains a Function* pointer, extract its raw pointer
	auto raw = add_node_statement(StatementID::RAW_FUNCTION_POINTER);
	raw->type = TypeFunctionCodeRef;
	raw->set_param(0, get_callable_fp(t, self));

	// call its raw pointer
	auto call = new Node(NodeKind::CALL_RAW_POINTER, 0, f->literal_return_type);
	call->set_num_params(1 + get_callable_param_types(t).num);
	call->set_param(0, raw);
	for (int i=1; i<f->num_params; i++) // skip "self"
		call->set_param(i, add_node_local(f->var[i].get()));

	if (f->literal_return_type == TypeVoid) {
		f->block->add(call);
	} else {
		auto ret = add_node_statement(StatementID::RETURN);
		ret->set_num_params(1);
		ret->set_param(0, call);
		f->block->add(ret);
	}
}



void AutoImplementer::implement_callable_bind_call(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	//db_add_print_label(this, f->block, "== bind.call ==");

	auto fp = get_callable_fp(t, self);
	auto call = add_node_member_call(fp->type->param[0]->get_call(), fp);
	//for (int i=0; i<f->num_params; i++)
	//	call->set_param(i+1, add_node_local(f->var[i].get()));

	/*int
	for (int index=0; index<nn; index++) {

	}*/

	shared_array<Node> params;

	for (auto *v: weak(f->var)) {
		//msg_write("V " + v->name + ": " + v->type->name);
		if (v->name.num == 1) {
			//db_add_print_label_node(this, f->block, "  param " + v->name + ": ", add_node_local(v));
			params.add(add_node_local(v));
			//call->set_param(index ++, add_node_local(v));
		}
	}
	for (auto &e: t->elements) {
		//msg_write("E " + e.name + ": " + e.type->name);
		if (e.name.head(7) == "capture") {
			int n = e.name.replace("capture", "").replace("_ref", "")._int();
			params.insert(self->shift(e.offset, e.type), n);
			//db_add_print_label_node(this, f->block, "  capture " + e.name + ": ", self->shift(e.offset, e.type));
			//call->set_param(index ++, self->shift(e.offset, e.type));
		}
	}

	for (auto&& [i,p]: enumerate(params))
		call->set_param(i+1, p);

	if (f->literal_return_type == TypeVoid) {
		f->block->add(call);
	} else {
		auto ret = add_node_statement(StatementID::RETURN);
		ret->set_num_params(1);
		ret->set_param(0, call);
		f->block->add(ret);
	}
}

void AutoImplementer::_implement_functions_for_callable_fp(const Class *t) {
	for (auto *cf: t->get_constructors())
		implement_callable_constructor(prepare_auto_impl(t, cf), t);
	implement_callable_fp_call(prepare_auto_impl(t, t->get_call()), t);
}

void AutoImplementer::_implement_functions_for_callable_bind(const Class *t) {
	for (auto *cf: t->get_constructors())
		implement_callable_constructor(prepare_auto_impl(t, cf), t);
	implement_callable_bind_call(prepare_auto_impl(t, t->get_call()), t);
}



string make_callable_signature_t(const Array<const Class*> &_params) {
	auto ret = _params.back();
	auto params = _params.sub_ref(0, _params.num - 1);
	// maybe some day...
	string signature;// = param->name;
	for (int i=0; i<params.num; i++) {
		if (i > 0)
			signature += ",";
		signature += class_name_might_need_parantheses(params[i]);
	}
	if (params.num == 0)
		signature = "void";
	if (params.num > 1)
		signature = "(" + signature + ")";
	if (params.num == 0 or (params.num == 1 and params[0] == TypeVoid)) {
		signature = "void";
	}
	return signature + "->" + class_name_might_need_parantheses(ret);
}

Class* TemplateClassInstantiatorCallableFP::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	string name = make_callable_signature_t(params);
	//int size = sizeof(KabaCallable<void()>);
	int size = TypeCallableBase->size + 8; // for additional Function*
	return create_raw_class(tree, "@Callable[" + name + "]", TypeCallableFPT, size, config.target.pointer_size, 0, nullptr, params, token_id);
}
void TemplateClassInstantiatorCallableFP::add_function_headers(Class* c) {
	auto params = c->param; // ->derive_from() will overwrite params!!!
	c->derive_from(TypeCallableBase);
	c->functions.clear(); // don't inherit call() with specific types!
	c->param = params;

	add_func_header(c, Identifier::Func::INIT, TypeVoid, {TypePointer}, {"p"}, nullptr, Flags::MUTABLE);
	add_func_header(c, Identifier::Func::CALL,
			get_callable_return_type(c),
			get_callable_param_types(c),
			{"a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::NONE)->virtual_index = TypeCallableBase->get_call()->virtual_index;
}

Class* TemplateClassInstantiatorCallableBind::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {

	static int unique_bind_counter = 0;
	//	auto t = TemplateClassInstantiator::create_raw_class(tree, format(":bind-%d:", unique_bind_counter++), Class::Type::CALLABLE_BIND, TypeCallableBase->size, config.target.pointer_size, magic, nullptr, outer_params_ret, token_id);
	auto t = create_raw_class(tree, format(":bind-%d:", unique_bind_counter++), TypeCallableBindT, TypeCallableBase->size, config.target.pointer_size, array_size, nullptr, params, token_id);

	auto pp = t->param;
	t->derive_from(TypeCallableBase);
	t->functions.clear(); // don't inherit call() with specific types!
	t->param = pp;

	int offset = t->size;
	for (int i=0; i<16; i++)
		if ((array_size & (1 << i))) {
			auto b = params[i];
			if ((array_size & ((1 << i) << 16)))
				b = tree->request_implicit_class_reference(b, token_id);
			offset = mem_align(offset, b->alignment);
			auto el = ClassElement(format("capture%d", i), b, offset);
			offset += b->size;
			t->elements.add(el);
		}
	t->size = offset;

	for (auto &e: t->elements)
		if (e.name == "_fp")
			e.type = tree->module->context->template_manager->request_callable_fp(tree, params.sub_ref(0, params.num-1), params.back(), token_id);

	return t;
}

void TemplateClassInstantiatorCallableBind::add_function_headers(Class* c) {
#if 0
	auto params = c->param; // ->derive_from() will overwrite params!!!

	c->array_length = max(c->array_length, 0);
	c->derive_from(TypeCallableBase);
	c->functions.clear(); // don't inherit call() with specific types!
	c->param = params;
	//AutoImplementerInternal ai(nullptr, c->owner);
	//ai.add_missing_function_headers_for_class(c); // later... depending on the bind variables
#endif


	auto types = get_callable_capture_types(c);
	types.insert(TypePointer, 0);
	add_func_header(c, Identifier::Func::INIT, TypeVoid, types,
			{"p", "a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::MUTABLE);
	add_func_header(c, Identifier::Func::CALL,
			get_callable_return_type(c),
			suggest_callable_bind_param_types(c),
			{"a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::NONE)->virtual_index = TypeCallableBase->get_call()->virtual_index;
}

}



