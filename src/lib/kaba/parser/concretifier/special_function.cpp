#include "Concretifier.h"
#include "../Parser.h"
#include "../../template/template.h"
#include "../../Context.h"
#include "../../lib/lib.h"
#include "../../dynamic/exception.h"
#include "../../dynamic/dynamic.h"
#include <lib/base/set.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>

namespace kaba {


shared<Node> Concretifier::concretify_special_function_str(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	return add_converter_str(node->params[0], false);
}

shared<Node> Concretifier::concretify_special_function_repr(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	return add_converter_str(node->params[0], true);
}

shared<Node> Concretifier::concretify_special_function_sizeof(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);

	if (sub->kind == NodeKind::Class) {
		return add_node_const(tree->add_constant_int(sub->as_class()->size), node->token_id);
	} else {
		return add_node_const(tree->add_constant_int(sub->type->size), node->token_id);
	}
}

shared<Node> Concretifier::concretify_special_function_typeof(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);
	return add_node_class(sub->type, node->token_id);
}

shared<Node> implement_len(shared<Node> node, Concretifier *con, Block *block, const Class *ns, int token_id) {
	node = con->concretify_node(node, block, ns);
	node = con->force_concrete_type(node);
	node = con->deref_if_reference(node);

	// array?
	if (node->type->is_array())
		return add_node_const(con->tree->add_constant_int(node->type->array_length), token_id);

	// __length__() function?
	if (auto *f = node->type->get_member_func(Identifier::func::Length, common_types.i32, {}))
		return add_node_member_call(f, node, node->token_id);

	// element "int num/length"?
	for (auto &e: node->type->elements)
		if (e.type == common_types.i32 and (e.name == "length" or e.name == "num")) {
			return node->shift(e.offset, e.type, node->token_id);
		}

	// length() function?
	for (auto f: node->type->functions)
		if ((f->name == "length") and (f->num_params == 1))
			return add_node_member_call(f.get(), node, node->token_id);


	con->do_error(format("don't know how to get the length of an object of class '%s'", node->type->long_name()), node);
	return nullptr;
}

shared<Node> Concretifier::concretify_special_function_len(shared<Node> node, Block *block, const Class *ns) {
	return implement_len(node->params[0], this, block, block->name_space(), node->token_id);
}

shared<Node> Concretifier::concretify_special_function_dyn(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	//sub = force_concrete_type(sub); // TODO
	return make_dynamical(sub);
}

shared<Node> Concretifier::concretify_special_function_sort(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	if (node->params.num < 2) {
		// default criterion ""
		node = cp_node(node);
		node->set_num_params(2);
		auto crit = tree->add_constant(common_types.string, -1);
		node->set_param(1, add_node_const(crit));
	}

	auto array = force_concrete_type(node->params[0]);
	auto crit = force_concrete_type(node->params[1]);

	if (!array->type->is_list())
		do_error(format("%s(): first parameter must be a list[]", Identifier::Sort), array);
	if (crit->type != common_types.string or crit->is_mutable())
		do_error(format("%s(): second parameter must be a string literal", Identifier::Sort), crit);

	Function *f = tree->required_func_global("@sorted", node->token_id);

	auto cmd = add_node_call(f, node->token_id);
	cmd->set_param(0, array);
	cmd->set_param(1, add_node_class(array->type));
	cmd->set_param(2, crit);
	cmd->type = array->type;
	return cmd;
}

shared<Node> Concretifier::concretify_special_function_weak(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	int token_id = node->token_id;

	auto t = sub->type;
	if (t->is_pointer_owned() or t->is_pointer_shared()) {
		auto tt = tree->get_pointer(t->param[0], token_id);
		return sub->change_type(tt, token_id);
	} else if (t->is_pointer_owned_not_null() or t->is_pointer_shared_not_null()) {
		auto tt = tree->request_implicit_class_reference(t->param[0], token_id);
		return sub->change_type(tt, token_id);
	} else if (t->is_list() and (t->param[0]->is_pointer_shared() or t->param[0]->is_pointer_owned())) {
		auto tt = tree->request_implicit_class_list(tree->get_pointer(t->param[0]->param[0], token_id), token_id);
		return sub->change_type(tt, token_id);
	} else if (t->is_list() and (t->param[0]->is_pointer_shared_not_null() or t->param[0]->is_pointer_owned_not_null())) {
		auto tt = tree->request_implicit_class_list(tree->request_implicit_class_reference(t->param[0]->param[0], token_id), token_id);
		return sub->change_type(tt, token_id);
	}
	do_error(format("weak() expects either a shared/owned pointer, or a shared/owned pointer array. Given: '%s'", t->long_name()), sub);
	return nullptr;
}

shared<Node> Concretifier::concretify_special_function_give(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());

	auto t = sub->type;
	if (/*t->is_pointer_shared() or*/ t->is_pointer_owned() or t->is_pointer_owned_not_null()) {
		auto t_xfer = tree->request_implicit_class_xfer(t->param[0], -1);
		if (auto f = t->get_member_func(Identifier::func::OwnedGive, t_xfer, {}))
			return add_node_member_call(f, sub);
		do_error("give...aaaa", sub);
	} else if (t->is_list() and (t->get_array_element()->is_pointer_owned() or t->get_array_element()->is_pointer_owned_not_null())) {
		auto t_xfer = tree->request_implicit_class_xfer(t->param[0]->param[0], -1);
		auto t_xfer_list = tree->request_implicit_class_list(t_xfer, -1);
		if (auto f = t->get_member_func(Identifier::func::OwnedGive, t_xfer_list, {}))
			return add_node_member_call(f, sub);
		do_error("give...aaaa", sub);
	}
	do_error("give() expects an owned pointer", sub);
	return nullptr;
}

shared<Node> Concretifier::concretify_special_function_call(shared<Node> node, SpecialFunction *s, Block *block, const Class *ns) {
	node = node->shallow_copy();
	node->params.erase(0);
	if (node->params.num < s->min_params)
		do_error(format("special function %s() expects at least %d parameter(s), but %d were given", s->name, s->min_params, node->params.num), node);
	if (node->params.num > s->max_params)
		do_error(format("special function %s() expects at most %d parameter(s), but %d were given", s->name, s->max_params, node->params.num), node);

	if (s->id == SpecialFunctionID::Str) {
		return concretify_special_function_str(node, block, ns);
	} else if (s->id == SpecialFunctionID::Repr) {
		return concretify_special_function_repr(node, block, ns);
	} else if (s->id == SpecialFunctionID::Sizeof) {
		return concretify_special_function_sizeof(node, block, ns);
	} else if (s->id == SpecialFunctionID::Typeof) {
		return concretify_special_function_typeof(node, block, ns);
	} else if (s->id == SpecialFunctionID::Len) {
		return concretify_special_function_len(node, block, ns);
	} else if (s->id == SpecialFunctionID::Dyn) {
		return concretify_special_function_dyn(node, block, ns);
	} else if (s->id == SpecialFunctionID::Weak) {
		return concretify_special_function_weak(node, block, ns);
	} else if (s->id == SpecialFunctionID::Give) {
		return concretify_special_function_give(node, block, ns);
	} else if (s->id == SpecialFunctionID::Sort) {
		return concretify_special_function_sort(node, block, ns);
	} else if (s->id == SpecialFunctionID::Filter) {
		//return concretify_special_function_filter(node, block, ns);
		do_error("filter() not allowed outside |> pipes", node);
	} else {
		node->show();
		//tree->module->do_error("");
		do_error("INTERNAL: unexpected special function", node);
	}
	return nullptr;
}


}
