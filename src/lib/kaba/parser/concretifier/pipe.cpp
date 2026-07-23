#include "Concretifier.h"
#include "../Parser.h"
#include "../../template/template.h"
#include "../../Context.h"
#include "../../lib/lib.h"

namespace kaba {

	shared<Node> check_const_params(SyntaxTree *tree, shared<Node> n);
	Array<const Class*> node_call_effective_params(shared<Node> node);
	const Class *node_call_return_type(shared<Node> node);
	shared<Node> implement_len(shared<Node> node, Concretifier *con, Block *block, const Class *ns, int token_id);

shared<Node> Concretifier::build_pipe_sort(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id) {
	if (!input->type->is_list())
		do_error(format("'|> %s' only allowed for lists", Identifier::Sort), input);
	Function *f = tree->required_func_global("@sorted", token_id);

	shared_array<Node> params;
	if (rhs->kind == NodeKind::AbstractCall)
		params = rhs->params.sub_ref(1);

	auto cmd = add_node_call(f, token_id);
	cmd->set_param(0, input);
	cmd->set_param(1, add_node_class(input->type));
	if (params.num >= 1) {
		auto crit = concretify_node(params[0], block, ns);
		if (crit->type != common_types.string or crit->kind != NodeKind::Constant)
			do_error(format("%s() expects a string literal when used in a pipe", Identifier::Sort), token_id);
		cmd->set_param(2, crit);
	} else {
		auto crit = tree->add_constant(common_types.string, token_id);
		cmd->set_param(2, add_node_const(crit));
	}
	cmd->type = input->type;
	return cmd;
}

// rhs is already the "lambda"  x=>y
shared<Node> Concretifier::build_pipe_filter(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id) {
	if (!input->type->is_list())
		do_error(format("'|> filter(...)' expects a list on the left, but '%s' given", input->type->long_name()), token_id);

	shared<Node> l;
	if (rhs->kind == NodeKind::AbstractCall and rhs->params.num == 2) {
		auto ll = rhs->params[1];
		if (ll->kind == NodeKind::AbstractOperator and ll->as_abstract_op()->name == "=>")
			l = ll;
		else
			do_error("lambda expression 'var => expression' expected inside 'filter(...)'", ll);
	}
	if (!l)
		do_error("lambda expression 'var => expression' expected inside 'filter(...)'", rhs);

//  p = [REF_VAR, KEY, ARRAY]
	auto n_for = add_node_statement(StatementID::For, token_id, common_types.unknown);
	n_for->set_param(0, l->params[0]); // token variable
	//n_for->set_param(1, key);
	n_for->set_param(2, input);

	auto n = new Node(NodeKind::ArrayBuilderFor, token_id, common_types.unknown);
	n->set_num_params(3);
	n->set_param(0, n_for);
	n->set_param(1, l->params[0]); // expression -> variable
	n->set_param(2, l->params[1]); // comparison

	return concretify_node(n, block, ns);
}

shared<Node> Concretifier::try_build_pipe_map_array_unwrap(const shared<Node> &input, Node *f, const Class *rt, const Class *pt, Block *block, const Class *ns, int token_id) {
	if (input->type->param[0] != pt)
		return nullptr;
	// -> map(func, array)


	// [VAR, INDEX, ARRAY, BLOCK]
	auto n_for = add_node_statement(StatementID::For, token_id, common_types._void);
	n_for->set_param(2, input);

	auto el_type = input->type->get_array_element();
	static int map_counter = 0;
	string viname = format("<map-index-%d>", map_counter);
	string vname = format("<map-var-%d>", map_counter++);
	auto var = block->add_var(vname, tree->request_implicit_class_reference(el_type, token_id), token_id);
	flags_clear(var->flags, Flags::Mutable);
	n_for->set_param(0, add_node_local(var));
	auto index = block->add_var(viname, common_types.i32, token_id
		);
	n_for->set_param(1, add_node_local(index));

	auto out = add_node_call(f->as_func(), f->token_id);

	CastingDataCall casts;
	auto nvar = add_node_local(var);

	if (!param_match_with_cast(out, {nvar}, casts))
		return nullptr; //do_error("pipe: " + param_match_with_cast_error({input}, wanted), f);

	auto n_exp = check_const_params(tree, apply_params_with_cast(out, {nvar}, casts));
	n_exp = concretify_node(n_exp, block, ns);

//	n_for->type = common_types.unknown;
	auto rrr = concretify_array_builder_for_inner(n_for, n_exp, nullptr, rt, block, ns, token_id);
	rrr->params[0]->params[3] = concretify_node(rrr->params[0]->params[3], block, ns);

	parser->post_process_for(rrr->params[0]);

	return rrr;
}

shared<Node> Concretifier::try_build_pipe_map_optional_unwrap(const shared<Node> &input, Node *f, const Class *rt, const Class *pt, Block *block, const Class *ns, int token_id) {
	if (input->type->param[0] != pt)
		return nullptr;

	auto ff = f->as_func();
	bool needs_wrapping = !ff->literal_return_type->is_optional();
	auto t_out = ff->literal_return_type;
	if (needs_wrapping)
		t_out = tree->request_implicit_class_optional(ff->literal_return_type, token_id);

	auto b = add_node_block(new Block(block->function, block), t_out);
	// variable into OUTER block for returnable life-time
	auto v = block->add_var(block->function->create_slightly_hidden_name(), input->type, token_id);

	b->add(auto_implementer->add_assign(block->function, "...", add_node_local(v), input));

	auto cif = add_node_statement(StatementID::If, token_id, t_out);
	cif->set_num_params(3);

	auto f_has_val = input->type->get_member_func(Identifier::func::OptionalHasValue, common_types._bool, {});
	cif->set_param(0, add_node_member_call(f_has_val, add_node_local(v), token_id));
	auto call = add_node_call(ff, token_id);
	call->set_param(0, add_node_local(v)->change_type(input->type->param[0]));
	if (needs_wrapping) {
		for (auto c: t_out->get_constructors())
			if (c->num_params == 2 and c->literal_param_type[1] == pt) {
				cif->set_param(1, add_node_constructor(c, token_id));
				cif->params[1]->params[1] = call;
			}
	} else {
		cif->set_param(1, call);
	}
	cif->set_param(2, add_node_constructor(t_out->get_default_constructor(), token_id));

	b->add(cif);

	return b;
}

shared<Node> Concretifier::try_build_pipe_map_direct(const shared<Node> &input, Node *f, const Class *rt, const Class *pt, Block *block, const Class *ns, int token_id) {

	shared<Node> out = add_node_call(f->as_func(), f->token_id);

	CastingDataCall casts;
	if (!param_match_with_cast(out, {input}, casts))
		return nullptr; //do_error("pipe: " + param_match_with_cast_error({input}, wanted), f);
	return check_const_params(tree, apply_params_with_cast(out, {input}, casts));
}

shared<Node> Concretifier::build_pipe_map(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id) {

	auto funcs = concretify_node_multi(rhs, block, ns);
	for (auto f: weak(funcs)) {
		//f->show();


		/*if (f->kind != NodeKind::SPECIAL_FUNCTION_NAME) {
			auto s = f->as_special_function();
			if (s->id == SpecialFunctionID::STR)
		}*/

		//if (!func->type->is_callable())
		//	do_error("operand after '|>' must be callable", func);
		if (f->kind != NodeKind::Function)
			do_error("operand after '|>' must be a function", f);

		auto p = node_call_effective_params(f);
		auto rt = node_call_return_type(f);

		if (p.num != 1)
			continue;//do_error("function after '|>' needs exactly 1 parameter (including self)", f);
		//if (f->literal_param_type[0] != input->type)
		//	do_error("pipe type mismatch...");

		// array |> func
		if (input->type->is_list())
			if (auto x = try_build_pipe_map_array_unwrap(input, f, rt, p[0], block, ns, token_id))
				return x;

		// optional |> func
		if (input->type->is_optional())
			if (auto x = try_build_pipe_map_optional_unwrap(input, f, rt, p[0], block, ns, token_id))
				return x;

		if (auto x = try_build_pipe_map_direct(input, f, rt, p[0], block, ns, token_id))
			return x;
	}
	if (input->type->is_list())
		do_error(format("'|>' type mismatch: can not call right side with type '%s' or '%s'", input->type->long_name(), input->type->param[0]->long_name()), rhs);
	else
		do_error(format("'|>' type mismatch: can not call right side with type '%s'", input->type->long_name()), rhs);
	return nullptr;
}

shared<Node> Concretifier::build_pipe_len(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id) {
	if (!input->type->is_list())
		do_error(format("'|> %s' only allowed for lists", Identifier::Len), input);

	return implement_len(input, this, block, block->name_space(), token_id);
}

shared<Node> Concretifier::build_function_pipe(const shared<Node> &abs_input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id) {
//	auto func = force_concrete_type(_func);
	auto input = abs_input;
	if (input->type == common_types.unknown)
		input = concretify_node(input, block, ns);
	input = force_concrete_type(input);
	input = deref_if_reference(input);

	if ((rhs->kind == NodeKind::AbstractToken)) {
		if (auto s = parser->which_special_function(rhs->as_token())) {
			if (s->id == SpecialFunctionID::Filter)
				return build_pipe_filter(input, rhs, block, ns, token_id);
			if (s->id == SpecialFunctionID::Sort)
				return build_pipe_sort(input, rhs, block, ns, token_id);
			if (s->id == SpecialFunctionID::Len)
				return build_pipe_len(input, rhs, block, ns, token_id);
		}
	} else if ((rhs->kind == NodeKind::AbstractCall)) {
		if (auto s = parser->which_special_function(rhs->params[0]->as_token())) {
			if (s->id == SpecialFunctionID::Filter)
				return build_pipe_filter(input, rhs, block, ns, token_id);
			if (s->id == SpecialFunctionID::Sort)
				return build_pipe_sort(input, rhs, block, ns, token_id);
		}
	}

	return build_pipe_map(input, rhs, block, ns, token_id);
}



}
