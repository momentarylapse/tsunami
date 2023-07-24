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


void AutoImplementer::_add_missing_function_headers_for_callable_fp(Class *t) {
	add_func_header(t, Identifier::Func::INIT, TypeVoid, {TypePointer}, {"p"});
	add_func_header(t, Identifier::Func::CALL,
			get_callable_return_type(t),
			get_callable_param_types(t),
			{"a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::CONST)->virtual_index = TypeCallableBase->get_call()->virtual_index;
}

void AutoImplementer::_add_missing_function_headers_for_callable_bind(Class *t) {
	auto types = get_callable_capture_types(t);
	types.insert(TypePointer, 0);
	add_func_header(t, Identifier::Func::INIT, TypeVoid, types,
			{"p", "a", "b", "c", "d", "e", "f", "g", "h"});
	add_func_header(t, Identifier::Func::CALL,
			get_callable_return_type(t),
			get_callable_param_types(t),
			{"a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::CONST)->virtual_index = TypeCallableBase->get_call()->virtual_index;
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
		//msg_write("V " + v->name);
		if (v->name.num == 1) {
			//db_add_print_label_node(this, f->block, "  param " + v->name + ": ", add_node_local(v));
			params.add(add_node_local(v));
			//call->set_param(index ++, add_node_local(v));
		}
	}
	for (auto &e: t->elements) {
		//msg_write("E " + e.name);
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


}



