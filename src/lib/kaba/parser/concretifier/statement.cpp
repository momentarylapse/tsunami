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

#include "lib/kaba/parser/Transformer.h"

namespace kaba {

	const Class *try_digest_type(SyntaxTree *tree, shared<Node> n);
	Array<const Class*> get_callable_param_types(const Class *fp);
	const Class *get_callable_return_type(const Class *fp);

shared<Node> Concretifier::concretify_statement_return(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	if (block->function->literal_return_type == common_types._void) {
		if (node->params.num > 0)
			do_error("current function has type 'void', can not return a value", node);
	} else {
		if (node->params.num == 0)
			do_error("return value expected", node);
		if (block->function->literal_return_type != common_types.unknown)
			node->params[0] = check_param_link(node->params[0], block->function->literal_return_type, Identifier::Return);
	}
	node->type = common_types._void;
	return node;
}

shared<Node> Concretifier::concretify_statement_if(shared<Node> node, Block *block, const Class *ns) {
	// [COND, TRUE-BLOCK, [FALSE-BLOCK]]
	concretify_all_params(node, block, ns);

	node->type = common_types._void;
	if (node->params.num >= 3) { // if/else
		if (node->params[1]->type != common_types._void and node->params[2]->type != common_types._void) {
			// return type from true block
			node->type = node->params[1]->type;
			if (node->params[1]->type != node->params[2]->type)
				do_error(format("type returned by `if` branch (`%s`) and type returned by `else` branch (`%s`) have to be the same", node->params[1]->type->long_name(), node->params[2]->type->long_name()), node);
		}
	}

	node->params[0] = check_param_link(node->params[0], common_types._bool, Identifier::If);
	return node;
}

shared<Node> Concretifier::concretify_statement_if_compiletime(shared<Node> node, Block *block, const Class *ns) {
	// [COND, TRUE-BLOCK, [FALSE-BLOCK]]
	auto cond = concretify_node(node->params[0], block, ns);

	// try to eval condition
	Transformer t(tree);
	cond = Transformer::transform_node(cond, [&t] (shared<Node> n) {
		return t.conv_eval_const_func(n);
	});

	if (cond->kind != NodeKind::Constant)
		do_error("'if @compiletime' expects a compile-time constant expression", cond);
	if (cond->type != common_types._bool)
		do_error(format("if condition must be of type 'bool', not '%s'", cond->type->name), cond);

	if (cond->as_const()->as_int() == 1)
		return concretify_node(node->params[1], block, ns);
	if (node->params.num >= 3)
		return concretify_node(node->params[2], block, ns);

	return add_node_statement(StatementID::Pass, node->token_id);
}

shared<Node> Concretifier::concretify_statement_while(shared<Node> node, Block *block, const Class *ns) {
	// [COND, BLOCK]
	concretify_all_params(node, block, ns);
	node->type = common_types._void;
	node->params[0] = check_param_link(node->params[0], common_types._bool, Identifier::While);
	return node;
}

shared<Node> Concretifier::concretify_statement_new(shared<Node> node, Block *block, const Class *ns) {
	auto constr = concretify_node(node->params[0], block, block->name_space());
	if (constr->kind != NodeKind::ConstructorAsFunction)
		do_error("constructor call expected after 'new'", node->params[0]);
	constr->kind = NodeKind::CallFunction;
	constr->type = common_types._void;
	node->params[0] = constr;

	auto ff = constr->as_func();
	auto tt = ff->name_space;
	//do_error("NEW " + tt->long_name());

	// return xfer[T]
	node->type = tree->request_implicit_class_xfer(tt, node->token_id);

	// new shared T() ?
	if (flags_has(node->flags, Flags::Shared)) {
		// return shared![T]
		auto t = tree->request_implicit_class_shared_not_null(tt, node->token_id);
		CastingDataSingle cd{};
		if (!type_match_with_cast(node, false, t, cd))
			do_error("can not convert to shared![T]...", node);
		return apply_type_cast(cd, node, t);
	}

	return node;
}

shared<Node> Concretifier::concretify_statement_delete(shared<Node> node, Block *block, const Class *ns) {
	auto p = force_concrete_type(concretify_node(node->params[0], block, block->name_space()));

	/*if (p->type->is_pointer_raw()) {
		// classic default delete  -  OBSOLETE
		node->params[0] = p;
		node->type = common_types._void;
		return node;
	}*/
	if (p->type->is_pointer_shared() or p->type->is_pointer_owned()) {
		if (auto f = p->type->get_member_func(Identifier::func::SharedClear, common_types._void, {}))
			return add_node_member_call(f, p, p->token_id);
		do_error("clear missing...", p);
	} else if (p->type->is_list()) {
		if (auto f = p->type->get_member_func("clear", common_types._void, {}))
			return add_node_member_call(f, p, p->token_id);
		do_error("clear missing...", p);
	}

	// override del operator?
	if (auto f = p->type->get_member_func(Identifier::func::DeleteOverride, common_types._void, {}))
		return add_node_member_call(f, p, node->token_id);

	do_error("shared/owned pointer expected after 'del'", node->params[0]);
	return nullptr;
}

shared<Node> Concretifier::concretify_statement_raw_function_pointer(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	if (sub->kind != NodeKind::Function)
		do_error("raw_function_pointer() expects a function name", sub);
	auto func = add_node_const(tree->add_constant(common_types.function_code_ref, node->token_id), node->token_id);
	func->as_const()->as_int64() = (int_p)sub->as_func(); // will be replaced during linking

	node = node->shallow_copy();
	node->type = common_types.function_code_ref;
	node->set_param(0, func);
	return node;
}

shared<Node> Concretifier::concretify_statement_try(shared<Node> node, Block *block, const Class *ns) {
	// [TRY-BLOCK, EX:[TYPE, NAME], EX-BLOCK, ...]

	_try_level ++;

	auto try_block = concretify_node(node->params[0], block, block->name_space());
	node->params[0] = try_block;

	int num_exceptions = (node->params.num - 1) / 2;

	for (int i=0; i<num_exceptions; i++) {

		auto ex = node->params[1 + 2*i];

		auto ex_block = node->params[2 + 2*i];
		ex_block->link_no = (int_p)new Block(block->function, block); // we need block/variables BEFORE actually concretifying the block...

		if (ex->params.num > 0) {
			auto ex_type = ex->params[0];
			ex_type = concretify_node(ex_type, block, block->name_space());
			auto type = try_digest_type(tree, ex_type);
			auto var_name = ex->params[1]->as_token();

			ex->params.resize(1);


			if (!type)
				do_error("Exception class expected", ex_type);
			if (!type->is_derived_from(common_types.exception))
				do_error("Exception class expected", ex_type);
			ex->type = type;

			auto *v = ex_block->as_block()->add_var(var_name, tree->get_pointer(type, -1), node->token_id);
			ex->set_param(0, add_node_local(v));
		} else {
			ex->type = common_types._void;
		}

		// find types AFTER creating the variable
		ex_block = concretify_node(ex_block, block, block->name_space());
		node->params[2 + 2*i] = ex_block;
	}
	node->type = common_types._void;
	_try_level --;
	return node;
}

bool Concretifier::is_in_trust_me() const {
	return _trust_me_level > 0;
}

bool Concretifier::is_in_try() const {
	return _try_level > 0;
}

shared<Node> Concretifier::concretify_statement_raise(shared<Node> node, Block *block, const Class *ns) {
	return node;
}

shared<Node> Concretifier::concretify_statement_trust_me(shared<Node> node, Block *block, const Class *ns) {
	_trust_me_level ++;
	auto sub = node->params[0];
	sub = concretify_node(sub, block, ns);
	_trust_me_level --;
	return sub;
}

// inner_callable: (A,B,C,D,E)->R
// captures:       [-,x,-,-,y]
// => outer:       (A,  C,D  )->R
shared<Node> create_bind(Concretifier *concretifier, shared<Node> inner_callable, const shared_array<Node> &captures, const Array<bool> &capture_via_ref) {
	SyntaxTree *tree = concretifier->tree;
	int token_id = inner_callable->token_id;

	Array<const Class*> capture_types;
	for (auto c: weak(captures))
		if (c)
			capture_types.add(c->type);
		else
			capture_types.add(nullptr);

	auto param_types = get_callable_param_types(inner_callable->type);
	auto return_type = get_callable_return_type(inner_callable->type);

	Array<const Class*> outer_call_types;
	for (int i=0; i<param_types.num; i++)
		if (!captures[i])
			outer_call_types.add(param_types[i]);

	auto bind_wrapper_type = tree->request_implicit_class_callable_bind(param_types, return_type, capture_types, capture_via_ref, token_id);
	auto bind_return_type = tree->request_implicit_class_callable_fp(outer_call_types, return_type, token_id);

	// "new bind(f, x, y, ...)"
	for (auto *cf: bind_wrapper_type->get_constructors()) {
		auto cmd_new = add_node_statement(StatementID::New);
		auto con = add_node_constructor(cf);
		shared_array<Node> params = {inner_callable.get()};
		for (auto c: weak(captures))
			if (c)
				params.add(c);
		con = concretifier->apply_params_direct(con, params, 1);
		con->kind = NodeKind::CallFunction;
		con->type = common_types._void;

		cmd_new->type = bind_return_type;
		cmd_new->set_param(0, con);
		cmd_new->token_id = inner_callable->token_id;
		return cmd_new;
	}

	concretifier->do_error("bind failed...", inner_callable);
	return nullptr;
}

shared<Node> Concretifier::concretify_statement_lambda(shared<Node> node, Block *block, const Class *ns) {
	auto f = node->params[0]->as_func();


	auto *prev_func = parser->cur_func;

	f->block->parent = block; // to allow captured variable lookup
	if (block->function->is_member())
		flags_clear(f->flags, Flags::Static); // allow finding "self.x" via "x"

	parser->cur_func = f;

	if (f->block_node->params.num == 1) {
		// func(i)              (multi line)
		//     bla..
		//     return i*i       (explicit return)

		auto cmd = f->block_node->params[0];
		cmd = concretify_node(cmd, f->block, block->name_space());

		f->literal_return_type = cmd->type;
		f->effective_return_type = cmd->type;

		if (cmd->type == common_types._void) {
			f->block_node->params[0] = cmd;
		} else {
			auto ret = add_node_statement(StatementID::Return);
			ret->set_num_params(1);
			ret->params[0] = cmd;
			f->block_node->params[0] = ret;
		}

	} else {
		// func(i) i*i      (single line, direct return)
		f->block_node->type = common_types.unknown;
		f->literal_return_type = common_types._void;
		f->effective_return_type = common_types._void;
		concretify_node(f->block_node.get(), f->block, f->name_space);
	}

	parser->cur_func = prev_func;


	f->block->parent = nullptr;
	flags_set(f->flags, Flags::Static);

	tree->base_class->add_function(tree, f, false, false);

// --- find captures
	base::set<Variable*> captured_variables;
	auto find_captures = [block, &captured_variables](shared<Node> n) {
		if (n->kind == NodeKind::VarLocal) {
			auto v = n->as_local();
			for (auto vv: block->function->var)
				if (v == vv)
					captured_variables.add(v);
		}
		return n;
	};
	Transformer::transform_block(f->block_node.get(), find_captures);


// --- no captures?
	if (captured_variables.num == 0) {
		f->update_parameters_after_realizing();
		return add_node_func_name(f);
	}

	auto explicit_param_types = f->literal_param_type;


	if (config.verbose)
		msg_write("CAPTURES:");

	Array<bool> capture_via_ref;

	auto should_capture_via_ref = [this, node] (Variable *v) {
		if (v->name == Identifier::Self)
			return true;
		if (v->type->can_memcpy() or v->type == common_types.string /*or v->type->is_pointer_shared() or v->type->is_pointer_shared_not_null()*/)
			return false;
		do_error(format("currently not supported to capture variable '%s' of type '%s'", v->name, v->type->long_name()), node);
		return true;
	};

// --- replace captured variables by adding more parameters to f
	for (auto v: captured_variables) {
		if (config.verbose)
			msg_write("  * " + v->name);

		bool via_ref = should_capture_via_ref(v);
		capture_via_ref.add(via_ref);
		auto cap_type = via_ref ? tree->request_implicit_class_reference(v->type, node->token_id) : v->type;


		auto new_param = f->add_param(v->name, cap_type, v->token_id, Flags::None);
		//if (!flags_has(flags, Flags::OUT))
		//flags_set(v->flags, Flags::CONST);



		auto replace_local = [v,new_param,cap_type,via_ref](shared<Node> n) {
			if (n->kind == NodeKind::VarLocal)
				if (n->as_local() == v) {
					if (via_ref) {
						//return add_node_local(new_param)->deref();
						n->link_no = (int_p)new_param;
						n->type = cap_type;
						return n->deref();
					} else {
						n->link_no = (int_p)new_param;
					}
				}
			return n;
		};
		Transformer::transform_block(f->block_node.get(), replace_local);
	}

	f->update_parameters_after_realizing();

	auto inner_lambda = wrap_function_into_callable(f, node->token_id);

	shared_array<Node> capture_nodes;
	for (auto&& [i,c]: enumerate(captured_variables)) {
		if (capture_via_ref[i])
			capture_nodes.add(add_node_local(c)->ref(tree));
		else
			capture_nodes.add(add_node_local(c));
	}
	for ([[maybe_unused]] auto e: explicit_param_types) {
		capture_nodes.insert(nullptr, 0);
		capture_via_ref.insert(false, 0);
	}

	return create_bind(this, inner_lambda, capture_nodes, capture_via_ref);
}

shared<Node> Concretifier::concretify_statement_match(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);

	const int ncases = (node->params.num - 1) / 2;
	const auto input_type = node->params[0]->type;
	const auto output_type = node->params[2]->type;
	bool has_default = false;
	for (int i=0; i<ncases; i++) {

		if (node->params[1+i*2]->kind == NodeKind::Statement) {
			has_default = true;
		} else {
			// TODO find a better place for this conversion
			node->params[1+i*2] = type_to_value(node->params[1+i*2]);

			const auto case_type = node->params[1+i*2]->type;
			if (node->params[1+i*2]->kind != NodeKind::Constant)
				do_error("constant expected for 'match' pattern", node->params[1+i*2]);
			if (case_type != input_type)
				do_error(format("'match' pattern type (%s) does not match input type (%s)", case_type->long_name(), input_type->long_name()), node->params[1+i*2]);
		}
		if (node->params[2+i*2]->type != output_type)
			do_error("'match' must have consistent result types", node->params[2+i*2]);
	}

	node->type = output_type;

	[[maybe_unused]] bool is_exhaustive = false;

	if (!has_default) {
		if (input_type->is_enum()) {
			base::set<int> coverage;
			for (int i=0; i<ncases; i++)
				coverage.add(node->params[1+i*2]->as_const()->as_int());
			// TODO check coverage
			for (auto c: weak(input_type->constants))
				if (c->type == input_type)
					if (!coverage.contains(c->as_int()))
						do_error(format("enum value not covered by 'match': %s (%d)", find_enum_label(input_type, c->as_int()), c->as_int()), node);
		} else {
			do_error("'match' requires a default pattern ('else =>'), except for fully covered enums", node);
		}
		// turn into "else" case, to make block-returns easier
		node->params[ncases*2-1] = add_node_statement(StatementID::Pass, node->params[ncases*2-1]->token_id, common_types._void);
	}
	return node;
}

shared<Node> Concretifier::concretify_statement(shared<Node> node, Block *block, const Class *ns) {
	auto s = node->as_statement();
	if (s->id == StatementID::Return)
		return concretify_statement_return(node, block, ns);
	if (s->id == StatementID::If)
		return concretify_statement_if(node, block, ns);
	if (s->id == StatementID::IfCompiletime)
		return concretify_statement_if_compiletime(node, block, ns);
	if (s->id == StatementID::While)
		return concretify_statement_while(node, block, ns);
	if (s->id == StatementID::For)
		return concretify_statement_for(node, block, ns);
	if (s->id == StatementID::New)
		return concretify_statement_new(node, block, ns);
	if (s->id == StatementID::Delete)
		return concretify_statement_delete(node, block, ns);
	if (s->id == StatementID::RawFunctionPointer)
		return concretify_statement_raw_function_pointer(node, block, ns);
	if (s->id == StatementID::Try)
		return concretify_statement_try(node, block, ns);
	if (s->id == StatementID::Raise)
		return concretify_statement_raise(node, block, ns);
	if (s->id == StatementID::Lambda)
		return concretify_statement_lambda(node, block, ns);
	if (s->id == StatementID::Match)
		return concretify_statement_match(node, block, ns);
	if (s->id == StatementID::TrustMe)
		return concretify_statement_trust_me(node, block, ns);

	node->show();
	do_error("INTERNAL: unexpected statement", node);
	return nullptr;
}


}
