#include "Concretifier.h"
#include "../Parser.h"
#include "../../template/template.h"
#include "../../Context.h"
#include "../../lib/lib.h"

namespace kaba {




shared<Node> Concretifier::concretify_statement_for_unwrap_pointer(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// [OUT-VAR, ---, EXPRESSION, TRUE-BLOCK, [FALSE-BLOCK]]
	auto expr = container;//concretify_node(node->params[2], block, ns);
	auto t0 = expr->type;
	auto var_name = node->params[0]->as_token();

	auto block_x = add_node_block(new Block(block->function, block), common_types._void);

	auto t_out = tree->request_implicit_class_alias(t0->param[0], node->token_id);

	auto *var = block_x->as_block()->add_var(var_name, t_out, node->token_id);
	if (!node->params[0]->is_mutable())
		flags_clear(var->flags, Flags::Mutable);
	block_x->add(add_node_operator_by_inline(InlineID::PointerAssign, add_node_local(var), expr->change_type(t_out)));

	auto n_if = add_node_statement(StatementID::If, node->token_id);
	n_if->set_num_params(node->params.num - 2);
	Function *f_p2b = tree->required_func_global("p2b", node->token_id);
	auto n_p2b = add_node_call(f_p2b);
	n_p2b->set_num_params(1);
	n_p2b->set_param(0, add_node_local(var));
	n_if->set_param(0, n_p2b);
	n_if->set_param(1, concretify_node(cp_node(node->params[3], block_x->as_block()), block_x->as_block(), ns));
	if (n_if->params[1]->type != common_types._void)
		do_error("typed block not allowed in for statement", node);
	if (node->params.num >= 5)
		n_if->set_param(2, concretify_node(cp_node(node->params[4], block_x->as_block()), block_x->as_block(), ns));
	block_x->add(n_if);

	return block_x;
}

shared<Node> Concretifier::concretify_statement_for_unwrap_pointer_shared(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// [OUT-VAR, ---, EXPRESSION, TRUE-BLOCK, [FALSE-BLOCK]]
	auto expr = container;//concretify_node(node->params[2], block, ns);
	auto t0 = expr->type;
	auto var_name = node->params[0]->as_token();

	auto block_x = add_node_block(new Block(block->function, block), common_types._void);
	auto t_out = tree->request_implicit_class_shared_not_null(t0->param[0], node->token_id);

	auto var = block_x->as_block()->add_var(var_name, t_out, node->token_id);
	if (!node->params[0]->is_mutable())
		flags_clear(var->flags, Flags::Mutable);
	block_x->add(parser->con.link_operator_id(OperatorID::Assign, add_node_local(var), expr->change_type(t_out)));

	auto n_if = add_node_statement(StatementID::If, node->token_id);
	n_if->set_num_params(node->params.num - 2);
	Function *f_p2b = tree->required_func_global("p2b", node->token_id);
	auto n_p2b = add_node_call(f_p2b);
	n_p2b->set_num_params(1);
	n_p2b->set_param(0, add_node_local(var));
	n_if->set_param(0, n_p2b);
	n_if->set_param(1, concretify_node(cp_node(node->params[3], block_x->as_block()), block_x->as_block(), ns));
	if (n_if->params[1]->type != common_types._void)
		do_error("typed block not allowed in for statement", node);
	if (node->params.num >= 5)
		n_if->set_param(2, concretify_node(cp_node(node->params[4], block_x->as_block()), block_x->as_block(), ns));
	block_x->add(n_if);

	return block_x;
}

bool expression_is_temporary(const shared<Node>& node) {
	if (node->kind == NodeKind::VarLocal or node->kind == NodeKind::VarGlobal)
		return false;
	if (node->kind == NodeKind::AddressShift)
		return expression_is_temporary(node->params[0]);
	return true;
}

shared<Node> Concretifier::concretify_statement_for_unwrap_optional(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// [OUT-VAR, ---, EXPRESSION, TRUE-BLOCK, [FALSE-BLOCK]]
	auto expr = concretify_node(container, block, ns);
	auto expr_static = expr;
	auto t0 = expr->type;
	auto var_name = node->params[0]->as_token();
	bool is_temporary = expression_is_temporary(expr);

	auto block_x = add_node_block(new Block(block->function, block), common_types._void);

	auto t_out = tree->request_implicit_class_alias(t0->param[0], node->token_id);

	// alias pointer
	auto var_p = block_x->as_block()->add_var(var_name, t_out, node->token_id);
	if (!node->params[0]->is_mutable())
		flags_clear(var_p->flags, Flags::Mutable);

	if (is_temporary) {
		// mutable temporaries are allowed... for example return of selfref/globalref functions

		// store in temp variable
		static int nnn = 0;
		auto var1 = block_x->as_block()->add_var(":tempop" + i2s(nnn++), t0, node->token_id);
		auto assign1 = auto_implementer->add_assign(block->function, "", add_node_local(var1), expr);
		block_x->add(assign1);

		expr_static = add_node_local(var1);
	}

	auto assign_p = add_node_operator_by_inline(InlineID::PointerAssign, add_node_local(var_p), expr_static->ref(t_out));

	auto n_if = add_node_statement(StatementID::If, node->token_id);
	n_if->set_num_params(node->params.num - 2);
	auto f_has_val = t0->get_member_func(Identifier::func::OptionalHasValue, common_types._bool, {});
//	if (!f_has_val)
//		do_error("")
	n_if->set_param(0, add_node_member_call(f_has_val, expr_static));
	n_if->set_param(1, concretify_node(cp_node(node->params[3], block_x->as_block()), block_x->as_block(), ns));
	if (node->params.num >= 5)
		n_if->set_param(2, concretify_node(cp_node(node->params[4], block_x->as_block()), block_x->as_block(), ns));
	block_x->add(n_if);

	n_if->params[1]->params.insert(assign_p, 0);

	return block_x;
}

shared<Node> Concretifier::concretify_statement_for(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, INDEX, CONTAINER, BLOCK]

	auto container = force_concrete_type(concretify_node(node->params[2], block, ns));
	container = deref_if_reference(container);

	if (node->params[0]->is_mutable() and !container->is_mutable())
		do_error("can not iterate mutating over a constant container", node);

	auto t_c = container->type;
	if (container->kind == NodeKind::Slice)
		return concretify_statement_for_slice(node, container, block, ns);
	if (t_c->is_pointer_shared() and flags_has(node->params[0]->flags, Flags::Shared))
		return concretify_statement_for_unwrap_pointer_shared(node, container, block, ns);
	if (t_c->is_pointer_shared() or t_c->is_pointer_owned() or t_c->is_pointer_raw())
		return concretify_statement_for_unwrap_pointer(node, container, block, ns);
	if (t_c->is_optional())
		return concretify_statement_for_unwrap_optional(node, container, block, ns);
	if (t_c->usable_as_list() or t_c->is_array())
		return concretify_statement_for_array(node, container, block, ns);
	if (t_c->is_dict())
		return concretify_statement_for_dict(node, container, block, ns);

	do_error(format("unable to iterate over type '%s' - array/list/dict/shared/owned/optional expected as second parameter in 'for . in .'", t_c->long_name()), container);
	return nullptr;
}

shared<Node> Concretifier::concretify_statement_for_slice(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {

	auto var_name = node->params[0]->as_token();
	auto val0 = container->params[0];
	auto val1 = container->params[1];
	auto step = container->params[2];

	// type?
	const Class *t = val0->type;

	val0 = check_param_link(val0, t, "for", 1, 2);
	val1 = check_param_link(val1, t, "for", 1, 2);
	step = check_param_link(step, t, "for", 1, 2);

	auto cmd_for = add_node_statement(StatementID::ForRange, node->token_id, common_types.unknown);
	cmd_for->flags = container->flags;
	cmd_for->set_param(1, val0);
	cmd_for->set_param(2, val1);
	cmd_for->set_param(3, step);

	// variable...
	auto var = block->add_var(var_name, t, node->token_id);
	cmd_for->set_param(0, add_node_local(var));

	// block
	cmd_for->params[4] = concretify_node(node->params[3], block, ns);
	parser->post_process_for(cmd_for);
	if (cmd_for->params[4]->type != common_types._void and !flags_has(node->flags, Flags::Extern))
		do_error("typed block not allowed in for loop A", cmd_for);

	cmd_for->type = common_types._void;
	return cmd_for;
}

shared<Node> Concretifier::concretify_statement_for_array(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// variable...
	node->params[2] = container;

	auto var_name = node->params[0]->as_token();
	auto var_type = tree->request_implicit_class_alias(container->type->get_array_element(), node->params[0]->token_id);
	auto var = block->add_var(var_name, var_type, node->token_id);
	if (!node->params[0]->is_mutable())
		flags_clear(var->flags, Flags::Mutable);
	node->set_param(0, add_node_local(var));

	string index_name = format("-for_index_%d-", for_index_count ++);
	if (node->params[1])
		index_name = node->params[1]->as_token();
	auto index = block->add_var(index_name, common_types.i32, node->token_id);
	node->set_param(1, add_node_local(index));

	// block
	node->params[3] = concretify_node(node->params[3], block, ns);
	parser->post_process_for(node);
	if (node->params[3]->type != common_types._void and !flags_has(node->flags, Flags::Extern))
		do_error("typed block not allowed in for loop XXX", node);

	node->type = common_types._void;
	return node;
}

shared<Node> Concretifier::concretify_statement_for_dict(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// variable...
	node->params[2] = container;

	auto var_name = node->params[0]->as_token();
	auto var_type = tree->request_implicit_class_alias(container->type->get_array_element(), node->params[0]->token_id);
	auto key_type = tree->request_implicit_class_alias(common_types.string, node->params[0]->token_id);
	auto var = block->add_var(var_name, var_type, node->token_id);
	if (!node->params[0]->is_mutable())
		flags_clear(var->flags, Flags::Mutable);
	node->set_param(0, add_node_local(var));

	string key_name = format("-for_key_%d-", for_index_count ++);
	if (node->params[1])
		key_name = node->params[1]->as_token();
	auto index = block->add_var(key_name, key_type, node->token_id);
	node->set_param(1, add_node_local(index));

	// block
	node->params[3] = concretify_node(node->params[3], block, ns);
	parser->post_process_for(node);
	if (node->params[3]->type != common_types._void)
		do_error("typed block not allowed in for loop", node);

	node->type = common_types._void;
	return node;
}

}
