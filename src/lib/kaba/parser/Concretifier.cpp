/*
 * Concretifier.cpp
 *
 *  Created on: 23 May 2022
 *      Author: michi
 */

#include "Concretifier.h"
#include "Parser.h"
#include "../template/template.h"
#include "../Context.h"
#include "../lib/lib.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"
#include "../../base/set.h"
#include "../../base/iter.h"
#include "../../os/msg.h"

namespace kaba {

extern const Class *TypeSpecialFunctionRef;

extern const Class *TypeInt32List;
extern const Class *TypeAnyList;
extern const Class *TypeAnyDict;
extern const Class *TypeInt32Dict;
extern const Class *TypeNone;



bool type_match_up(const Class *given, const Class *wanted);

const Class *merge_type_tuple_into_product(SyntaxTree *tree, const Array<const Class*> &classes, int token_id);

string type_list_to_str(const Array<const Class*> &tt) {
	string s;
	for (auto *t: tt) {
		if (s.num > 0)
			s += ", ";
		if (t)
			s += t->long_name();
		else
			s += "<nil>";
	}
	return s;
}


shared<Node> __digest_type(SyntaxTree *tree, shared<Node> n) {
	if (!is_type_tuple(n))
		return n;
	auto classes = class_tuple_extract_classes(n);
	return add_node_class(merge_type_tuple_into_product(tree, classes, n->token_id), n->token_id);
}


const Class *try_digest_type(SyntaxTree *tree, shared<Node> n) {
	if (n->kind == NodeKind::Class)
		return n->as_class();
	if (is_type_tuple(n)) {
		auto classes = class_tuple_extract_classes(n);
		return merge_type_tuple_into_product(tree, classes, n->token_id);
	}
	return nullptr;
}

const Class *get_user_friendly_type(shared<Node> operand) {
	const Class *type = operand->type;

	if (operand->kind == NodeKind::Class) {
		// referencing class functions
		return operand->as_class();
	} else if (type->is_reference()) {
		return type->param[0];
	}
	return type;
}


const Class *give_useful_type(Concretifier *con, shared<Node> node) {
	if (node->type == TypeUnknown)
		return con->force_concrete_type(node)->type;
	return node->type;
}


// usable for pointer AND Callable class!
Array<const Class*> get_callable_param_types(const Class *fp) {
	if (fp->is_pointer_raw())
		return get_callable_param_types(fp->param[0]);

	// TODO look at call() signature

	if (fp->is_callable_bind()) {
		Array<const Class*> r;
		for (int i=0; i<fp->param.num-1; i++)
			if ((fp->array_length & (1 << i)) == 0)
				r.add(fp->param[i]);
		return r;
	}
	return fp->param.sub_ref(0, -1); // skip return value
}

const Class *get_callable_return_type(const Class *fp) {
	if (fp->is_pointer_raw())
		return get_callable_return_type(fp->param[0]);
	return fp->param.back();
}

Array<const Class*> get_callable_capture_types(const Class *fp) {
	if (fp->is_pointer_raw())
		return get_callable_capture_types(fp->param[0]);
	Array<const Class*> binds;
	for (auto &e: fp->elements)
		if (e.name.head(7) == "capture")
			binds.add(e.type);
	return binds;
}

Array<const Class*> func_effective_params(const Function *f) {
	return f->literal_param_type;
}

Array<const Class*> node_call_effective_params(shared<Node> node) {
	if (node->type->is_callable())
		return get_callable_param_types(node->type);
	if (node->kind == NodeKind::Function)
		return func_effective_params(node->as_func());
	return get_callable_param_types(node->type);
}

const Class *node_call_return_type(shared<Node> node) {
	if (node->type->is_callable())
		return get_callable_return_type(node->type);
	if (node->kind == NodeKind::Function)
		return node->as_func()->literal_return_type;
	return get_callable_return_type(node->type);
}


Concretifier::Concretifier(Context *c, Parser *_parser, SyntaxTree *_tree) {
	parser = _parser;
	tree = _tree;
	context = c;
	auto_implementer = &parser->auto_implementer;
}

const Class *Concretifier::make_effective_class_callable(shared<Node> node) {
	auto f = node->as_func();
	if (f->is_member() and node->params.num > 0 and node->params[0])
		return tree->request_implicit_class_callable_fp(f->literal_param_type.sub_ref(1), f->literal_return_type, node->token_id);
	return tree->request_implicit_class_callable_fp(f, node->token_id);
}

shared<Node> Concretifier::link_special_operator_is(shared<Node> param1, shared<Node> param2, int token_id) {
	const Class *t2 = try_digest_type(tree, param2);
	if (!t2)
		do_error("class name expected after 'is'", param2);
	if (t2->vtable.num == 0)
		do_error(format("class after 'is' needs to have virtual functions: '%s'", t2->long_name()), param2);

	// vtable1
	const Class *t1 = param1->type;
	if (t1->is_some_pointer()) {
		// FIXME is this safe?
		param1->type = tree->type_ref(TypePointer, token_id);
		param1 = param1->deref();
		t1 = t1->param[0];
	}
	if (!t2->is_derived_from(t1))
		do_error(format("'is': class '%s' is not derived from '%s'", t2->long_name(), t1->long_name()), token_id);

	// vtable2
	auto vtable2 = add_node_const(tree->add_constant_pointer(TypePointer, t2->_vtable_location_compiler_), token_id);

	return add_node_operator_by_inline(InlineID::PointerEqual, param1, vtable2, token_id);
}

shared<Node> Concretifier::link_special_operator_in(shared<Node> param1, shared<Node> param2, int token_id) {
	param2 = force_concrete_type(param2);
	auto *f = param2->type->get_member_func(Identifier::func::Contains, TypeBool, {param1->type});
	if (!f)
		do_error(format("no 'bool %s.%s(%s)' found", param2->type->long_name(), Identifier::func::Contains, param1->type->long_name()), token_id);

	auto n = add_node_member_call(f, param2, token_id);
	n->set_param(1, param1);
	return n;
}

shared<Node> Concretifier::link_special_operator_as(shared<Node> param1, shared<Node> param2, int token_id) {
	auto wanted = try_digest_type(tree, param2);
	if (!wanted)
		do_error("class name expected after 'as'", param2);
	return explicit_cast(param1, wanted);
}

shared<Node> Concretifier::link_operator_id(OperatorID op_no, shared<Node> param1, shared<Node> param2, int token_id) {
	return link_operator(&abstract_operators[(int)op_no], param1, param2, token_id);
}

bool tuple_all_editable(shared<Node> node) {
	for (auto p: weak(node->params))
		if ((p->kind != NodeKind::VarLocal) and (p->kind != NodeKind::VarGlobal))
			return false;
	return true;
}

Array<const Class*> tuple_get_element_types(const Class *type) {
	Array<const Class*> r;
	if (type->is_list())
		return r;

	for (auto &e: type->elements)
		if (!e.hidden())
			r.add(e.type);
	return r;
}

shared<Node> Concretifier::link_special_operator_tuple_extract(shared<Node> param1, shared<Node> param2, int token_id) {
	if (!tuple_all_editable(param1))
		do_error("tuple extraction only allowed if all elements are variables", token_id);

	param2 = force_concrete_type(param2);

	auto t = param2->type;
	auto etypes = tuple_get_element_types(t);

	if (etypes.num == 0)
		do_error(format("can not extract type '%s' into a tuple", t->long_name()), token_id);

	if (param1->params.num != etypes.num)
		do_error(format("tuple extraction: number mismatch (%d vs %d)", param1->params.num, t->elements.num), token_id);
	for (int i=0; i<etypes.num; i++)
		if (param1->params[i]->type != etypes[i])
			do_error(format("tuple extraction: type mismatch element #%d (%s vs %s)", i+1, param1->params[i]->type->long_name(), etypes[i]->long_name()), token_id);

	auto node = new Node(NodeKind::TupleExtraction, -1, TypeVoid, Flags::None, token_id);
	node->set_num_params(etypes.num + 1);
	node->set_param(0, param2);
	for (int i=0; i<etypes.num; i++)
		node->set_param(i+1, param1->params[i]);
	//node->show();
	return node;
}

// TODO clean-up
shared<Node> Concretifier::link_special_operator_ref_assign(shared<Node> param1, shared<Node> param2, int token_id) {
	// we already know, param1/2 are refs!
	if (!type_match_up(param2->type->param[0], param1->type->param[0]))
		do_error(format("can not assign references '%s' := '%s'. The left side must be equal or a base class of the right side!", param1->type->long_name(), param2->type->long_name()), token_id);
	return add_node_operator_by_inline(InlineID::PointerAssign, param1, param2, token_id);
}

shared<Node> Concretifier::link_operator(AbstractOperator *primop, shared<Node> param1, shared<Node> param2, int token_id) {
	bool left_modifiable = primop->flags & OperatorFlags::LeftIsModifiable;
	[[maybe_unused]] bool order_inverted = primop->flags & OperatorFlags::OrderInverted;
	string op_func_name = primop->function_name;
	shared<Node> op;

	// tuple extractor?
	if ((primop->id == OperatorID::Assign) and (param1->kind == NodeKind::Tuple))
		return link_special_operator_tuple_extract(param1, param2, token_id);

	if (left_modifiable and !param1->is_mutable())
		do_error("trying to modify a constant expression", token_id);

	// &ref := &ref
	if ((primop->id == OperatorID::RefAssign) and (param1->type->is_reference() and param2->type->is_reference()))
		return link_special_operator_ref_assign(param1, param2, token_id);

	if (primop->id == OperatorID::Is)
		return link_special_operator_is(param1, param2, token_id);
	if (primop->id == OperatorID::In)
		return link_special_operator_in(param1, param2, token_id);
	if (primop->id == OperatorID::As)
		return link_special_operator_as(param1, param2, token_id);

	param1 = force_concrete_type(param1);

	auto *p1 = param1->type;
	auto *p2 = param2->type;

	if (primop->id == OperatorID::Assign) {
		// x.get(..) = y   =>   x.set(.., y)
		if (param1->kind == NodeKind::CallFunction) {
			auto f = param1->as_func();
			if (f->name == Identifier::func::Get) {
				auto inst = param1->params[0];
				auto index = param1->params[1];
				//msg_write(format("[]=...    void %s.__set__(%s, %s)?", inst->type->long_name(), index->type->long_name(), p2->long_name()));
				for (auto *ff: weak(inst->type->functions))
					if (ff->name == Identifier::func::Set and ff->literal_return_type == TypeVoid and ff->num_params == 3) {
						if (ff->literal_param_type[1] != index->type)
							continue;
						CastingDataSingle cast;
						if (!type_match_with_cast(param2, false, ff->literal_param_type[2], cast))
							continue;
						//msg_write(ff->signature());
						auto nn = add_node_member_call(ff, inst, token_id);
						nn->set_param(1, index);
						nn->set_param(2, apply_type_cast(cast, param2, ff->literal_param_type[2]));
						return nn;
					}
			}
		}
	}

	// exact match as class function?
	for (auto *f: weak(p1->functions))
		if ((f->name == op_func_name) and f->is_member()) {
			// exact match as class function?
			if (f->literal_param_type.num != 2)
				continue;

			auto type2 = f->literal_param_type[1];
			if (type_match_up(p2, type2)) {
				auto inst = param1;
				op = add_node_member_call(f, inst, token_id);
				op->set_param(1, param2);
				return op;
			}
		}

	// exact (operator) match?
	// FIXME don't auto cast into arbitrary crap...
	/*for (auto *op: tree->operators)
		if (primop == op->primitive)
			if (type_match(p1, op->param_type_1) and type_match(p2, op->param_type_2)) {
				// UNUSED ANYWAY???
	//			return add_node_operator(op, param1, param2);
			}*/


	// needs type casting?
	CastingDataSingle c1 = {TypeCastId::NONE, 0};
	CastingDataSingle c2 = {TypeCastId::NONE, 0};
	CastingDataSingle c1_best = {TypeCastId::NONE, 1000};
	CastingDataSingle c2_best = {TypeCastId::NONE, 1000};
	const Class *t1_best = nullptr, *t2_best = nullptr;
	Operator *op_found = nullptr;
	Function *op_cf_found = nullptr;
	for (auto op: weak(context->global_operators))
		if (primop == op->abstract)
			if (type_match_with_cast(param1, left_modifiable, op->param_type_1,  c1) and type_match_with_cast(param2, false, op->param_type_2, c2))
				if (c1.penalty + c2.penalty < c1_best.penalty + c2_best.penalty) {
					op_found = op;
					c1_best = c1;
					c2_best = c2;
					t1_best = op->param_type_1;
					t2_best = op->param_type_2;
				}
	for (auto *cf: weak(p1->functions))
		if (cf->name == op_func_name) {
			if (type_match_with_cast(param2, false, cf->literal_param_type[1], c2)) {
				if (c2.penalty < c2_best.penalty) {
					op_cf_found = cf;
					c1_best.cast = TypeCastId::NONE;
					c2_best = c2;
					t2_best = cf->literal_param_type[1];
				}
			}
		}
	// cast
	if (op_found or op_cf_found) {
		param1 = apply_type_cast(c1_best, param1, t1_best);
		param2 = apply_type_cast(c2_best, param2, t2_best);
		if (op_cf_found) {
			op = add_node_member_call(op_cf_found, param1, token_id);
			op->set_param(1, param2);
		} else {
			return add_node_operator(op_found, param1, param2, token_id);
		}
		return op;
	}

	if (p1->is_some_pointer_not_null())
		return link_operator(primop, param1->deref(), param2, token_id);

	return nullptr;
}

void Concretifier::concretify_all_params(shared<Node> &node, Block *block, const Class *ns) {
	for (int p=0; p<node->params.num; p++)
		if (node->params[p]->type == TypeUnknown) {
			node->params[p] = concretify_node(node->params[p], block, ns);
		}
};

shared<Node> apply_macro(Concretifier *con, Function* f, shared<Node> node, shared_array<Node>& params, Block *block, const Class *ns) {
	if (f->num_params != params.num)
		con->do_error(format("can not pass %d parameters to a macro expecting %d", params.num, f->num_params), node);

	auto b = cp_node(f->block.get());
	con->tree->transform_block((Block*)b.get(), [f, params] (shared<Node> n) {
		if (n->kind == NodeKind::AbstractToken) {
			for (int i=0; i<params.num; i++) {
				if (n->as_token() == f->var[i]->name) {
					//con->do_error("FOUND PARRAM", n);
					return params[i];
				}
			}
		}
		return n;
	});

	return con->concretify_block(b, block, ns);
}

shared<Node> Concretifier::concretify_call(shared<Node> node, Block *block, const Class *ns) {

	// special function
	if (node->params[0]->kind == NodeKind::AbstractToken)
		if (auto s = parser->which_special_function(node->params[0]->as_token()))
			return concretify_special_function_call(node, s, block, ns);


	//concretify_all_params(node, block, ns, this);
	auto links = concretify_node_multi(node->params[0], block, ns);
	for (int p=1; p<node->params.num; p++)
		if (node->params[p]->type == TypeUnknown)
			node->params[p] = concretify_node(node->params[p], block, ns);

	auto params = node->params.sub_ref(1);

	// make links callable
	for (auto&& [i,l]: enumerate(weak(links))) {
		if (l->kind == NodeKind::Function) {
			auto f = l->as_func();
			if (f->is_macro())
				return apply_macro(this, f, l, params, block, ns);
			links[i] = make_func_node_callable(l);
		} else if (l->kind == NodeKind::Class) {
			auto *t = l->as_class();
			return try_to_match_apply_params(turn_class_into_constructor(t, params, node->token_id), params);
#if 0
		} else if (is_typed_function_pointer(l->type)) {
			auto c = new Node(NodeKind::POINTER_CALL, 0, get_function_pointer_return_type(l->type));
			c->set_num_params(1 + get_function_pointer_param_types(l->type).num);
			c->set_param(0, l);
			return try_to_match_apply_params({c}, params);
		/*} else if (l->type == TypeFunctionCodeP) {
			auto c = new Node(NodeKind::POINTER_CALL, 0, TypeVoid);
			c->set_num_params(1);
			c->set_param(0, l);
			return try_to_match_params({c});*/
#endif
		} else if (l->type->is_callable()) {
			links[i] = make_func_pointer_node_callable(l);
		} else if (auto c = l->type->get_call()) {
			links[i] = add_node_member_call(c, l, node->token_id, {});
		} else {
			do_error(format("this %s does not seem callable", kind2str(l->kind)), l);
		}
	}

	if (links.num > 0)
		return try_to_match_apply_params(links, params);

	do_error("not callable", node);
	return nullptr;
}

shared_array<Node> Concretifier::concretify_element(shared<Node> node, Block *block, const Class *ns) {
	auto base = concretify_node(node->params[0], block, ns);
	int token_id = node->params[1]->token_id;
	auto el = node->params[1]->as_token();

	base = force_concrete_type(base);

	if (base->kind == NodeKind::Class) {
		auto links = tree->get_element_of(base, el, token_id);
		if (links.num > 0)
			return links;
	} else if (base->kind == NodeKind::Function) {
		msg_write("FFF");
	}

	base = deref_if_reference(base);

	if (base->type->is_some_pointer() and !base->type->is_some_pointer_not_null())
		do_error("can not implicitly dereference a pointer that can be null. Use '!' or 'for . in .'", node);

	auto links = tree->get_element_of(base, el, token_id);
	if (links.num > 0)
		return links;

	do_error(format("unknown element of '%s'", get_user_friendly_type(base)->long_name()), node->params[1]);
	return {};
}

shared<Node> Concretifier::concretify_array(shared<Node> node, Block *block, const Class *ns) {
	auto operand = concretify_node(node->params[0], block, ns);
	auto index = concretify_node(node->params[1], block, ns);

	if (operand->kind == NodeKind::Class) {
		if (index->kind == NodeKind::Class) {
			auto c1 = operand->as_class();
			auto c2 = index->as_class();
			if (auto cc = context->template_manager->request_class_instance(tree, c1, {c2}, node->token_id)) {
				return add_node_class(cc, node->token_id);
			}
		}
	}

	// int[3]
	if (operand->kind == NodeKind::Class) {
		// find array size
		index = tree->transform_node(index, [this] (shared<Node> n) {
			return tree->conv_eval_const_func(n);
		});

		if (index->type != TypeInt32)
			do_error(format("array size must be of type 'int', not '%s'", index->type->name), index);
		if (index->kind != NodeKind::Constant)
			do_error("array size must be compile-time constant", index);
		int array_size = index->as_const()->as_int();
		auto t = tree->request_implicit_class_array(operand->as_class(), array_size, operand->token_id);
		return add_node_class(t);
	}

	// min[float]()
	if (operand->kind == NodeKind::Function) {
		auto links = concretify_node_multi(node->params[0], block, ns);
		Array<const Class*> tt;
		if (index->kind == NodeKind::Tuple and index->params.num == 2) {
			if (index->params[0]->kind != NodeKind::Class or index->params[1]->kind != NodeKind::Class)
				do_error("functions can only be indexed by a type", index);
			tt.add(index->params[0]->as_class());
			tt.add(index->params[1]->as_class());
		} else if (index->kind == NodeKind::Class) {
			tt.add(index->as_class());
		} else {
			do_error("functions can only be indexed by a type", index);
		}
		for (auto l: weak(links)) {
			auto f = l->as_func();
			if (auto ff = context->template_manager->request_function_instance(tree, f, tt, node->token_id)) {
				auto tf = add_node_func_name(ff);
				tf->params = l->params; // in case we have a member instance
				return tf;
			}
		}
		do_error(format("function has no version [%s]", type_list_to_str(tt)), index);
	}

	operand = force_concrete_type(operand);

	// auto deref?
	operand = deref_if_reference(operand);

	if (operand->type->is_pointer_raw())
		do_error(format("using pointer type '%s' as an array (like in C) is deprecated", operand->type->long_name()), index);


	// __subarray__() ?
	if (index->kind == NodeKind::Slice) {
		auto t1 = index->params[0]->type;
		auto t2 = index->params[1]->type;
		if (auto *cf = operand->type->get_member_func(Identifier::func::Subarray, operand->type, {t1, t2})) {
			auto f = add_node_member_call(cf, operand, operand->token_id);
			f->set_mutable(operand->is_mutable());
			f->set_param(1, index->params[0]);
			f->set_param(2, index->params[1]);
			return f;
		} else {
			do_error(format("function '%s.%s(%s,%s) -> %s' required by '[a:b]' not found", operand->type->name, Identifier::func::Subarray, t1->long_name(), t2->long_name(), operand->type->name), index);
		}
	}

	// tuple
	if (operand->type->is_product()) {
		index = tree->transform_node(index, [this] (shared<Node> n) {
			return tree->conv_eval_const_func(n);
		});
		if (index->type != TypeInt32)
			do_error("tuple index must be of type 'i32'", index);
		if (index->kind != NodeKind::Constant)
			do_error("tuple index must be compile-time constant", index);
		int i = index->as_const()->as_int();
		if (i < 0 or i >= operand->type->param.num)
			do_error("tuple index out of range", index);
		auto &e = operand->type->elements[i];
		return operand->shift(e.offset, e.type, operand->token_id);
	}

	// __get__() ?
	if (auto *cf = operand->type->get_get(index->type)) {
		auto f = add_node_member_call(cf, operand, operand->token_id);
		f->set_mutable(operand->is_mutable());
		f->set_param(1, index);
		return f;
	}

	if (index->type != TypeInt32)
		do_error(format("array index needs to be of type 'i32', not '%s'", index->type->long_name()), index);

	index = tree->transform_node(index, [this] (shared<Node> n) {
		return tree->conv_eval_const_func(n);
	});
	auto is_directly_accessible = [] (NodeKind k) {
		return k == NodeKind::VarGlobal or k == NodeKind::VarLocal or k == NodeKind::Constant;
	};
	if (index->kind == NodeKind::Constant) {
		int n = index->as_const()->as_int();
		if (n < 0) {
			if (!is_directly_accessible(operand->kind))
				do_error("negative indices only allowed for simple operands (variables or constants)", index);
			auto l = add_node_special_function_call(SpecialFunctionID::Len, index->token_id, index->type);
			l->set_param(0, operand);
			l = concretify_special_function_len(l, block, ns);
			index = add_node_operator_by_inline(InlineID::Int32Add, l, index, index->token_id);
		}
	}

	shared<Node> array_element;
	if (operand->type->usable_as_list())
		array_element = add_node_dyn_array(operand, index);
	else if (operand->type->is_array())
		array_element = add_node_array(operand, index);
	else
		do_error(format("type '%s' is neither an array nor does it have a function %s(%s)", operand->type->long_name(), Identifier::func::Get, index->type->long_name()), index);
	array_element->set_mutable(operand->is_mutable());
	return array_element;

}

shared_array<Node> Concretifier::concretify_node_multi(shared<Node> node, Block *block, const Class *ns) {
	if (node->type != TypeUnknown)
		return {node};

	if (node->kind == NodeKind::AbstractToken) {
		return concretify_token(node, block, ns);
	} else if (node->kind == NodeKind::AbstractElement) {
		return concretify_element(node, block, ns);
	} else {
		return {concretify_node(node, block, ns)};
	}
	return {};
}

shared_array<Node> Concretifier::concretify_token(shared<Node> node, Block *block, const Class *ns) {

	string token = node->as_token();

	// direct operand
	auto operands = tree->get_existence(token, block, ns, node->token_id);
	if (operands.num > 0) {
		// direct operand
		return operands;
	}

	// constant?
	auto t = parser->get_constant_type(token);
	if (t != TypeUnknown) {
		Value v;
		parser->get_constant_value(token, v);

		if (t == TypeString) {
			return {parser->try_parse_format_string(block, v, node->token_id)};
		} else {
			auto *c = tree->add_constant(t);
			c->set(v);
			return {add_node_const(c, node->token_id)};
		}
	}

	// special function name
	if (auto s = parser->which_special_function(token)) {
		// no call, just the name
		return {add_node_special_function_name(s->id, node->token_id, TypeSpecialFunctionRef)};
	}

#if 0
	msg_write("--------");
	msg_write(block->function->signature());
	msg_write("local vars:");
	for (auto vv: weak(block->function->var))
		msg_write(format("    %s: %s", vv->name, vv->type->name));
	msg_write("params:");
	for (auto p: block->function->literal_param_type)
		msg_write("    " + p->name);
	//crash();
#endif
	do_error(format("unknown operand \"%s\"", token), node);
	return {};
}


shared<Node> Concretifier::concretify_statement_return(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	if (block->function->literal_return_type == TypeVoid) {
		if (node->params.num > 0)
			do_error("current function has type 'void', can not return a value", node);
	} else {
		if (node->params.num == 0)
			do_error("return value expected", node);
		node->params[0] = check_param_link(node->params[0], block->function->literal_return_type, Identifier::Return);
	}
	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_if(shared<Node> node, Block *block, const Class *ns) {
	// [COND, TRUE-BLOCK, [FALSE-BLOCK]]
	concretify_all_params(node, block, ns);

	node->type = TypeVoid;
	if (node->params.num >= 3) { // if/else
		if (node->params[1]->type != TypeVoid and node->params[2]->type != TypeVoid) {
			// return type from true block
			node->type = node->params[1]->type;
			if (node->params[1]->type != node->params[2]->type)
				do_error(format("type returned by `if` branch (`%s`) and type returned by `else` branch (`%s`) have to be the same", node->params[1]->type->long_name(), node->params[2]->type->long_name()), node);
		}
	}

	node->params[0] = check_param_link(node->params[0], TypeBool, Identifier::If);
	return node;
}

shared<Node> Concretifier::concretify_statement_if_compiletime(shared<Node> node, Block *block, const Class *ns) {
	// [COND, TRUE-BLOCK, [FALSE-BLOCK]]
	auto cond = concretify_node(node->params[0], block, ns);

	// try to eval condition
	cond = tree->transform_node(cond, [this] (shared<Node> n) {
		return tree->conv_eval_const_func(n);
	});

	if (cond->kind != NodeKind::Constant)
		do_error("'if @compiletime' expects a compile-time constant expression", cond);
	if (cond->type != TypeBool)
		do_error(format("if condition must be of type 'bool', not '%s'", cond->type->name), cond);

	if (cond->as_const()->as_int() == 1)
		return concretify_node(node->params[1], block, ns);
	if (node->params.num >= 3)
		return concretify_node(node->params[2], block, ns);

	return add_node_statement(StatementID::Pass, node->token_id);
}

shared<Node> Concretifier::concretify_statement_for_unwrap_pointer(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// [OUT-VAR, ---, EXPRESSION, TRUE-BLOCK, [FALSE-BLOCK]]
	auto expr = container;//concretify_node(node->params[2], block, ns);
	auto t0 = expr->type;
	auto var_name = node->params[0]->as_token();

	auto block_x = new Block(block->function, block);

	auto t_out = tree->request_implicit_class_alias(t0->param[0], node->token_id);

	auto *var = block_x->add_var(var_name, t_out);
	block_x->add(add_node_operator_by_inline(InlineID::PointerAssign, add_node_local(var), expr->change_type(t_out)));

	auto n_if = add_node_statement(StatementID::If, node->token_id);
	n_if->set_num_params(node->params.num - 2);
	Function *f_p2b = tree->required_func_global("p2b", node->token_id);
	auto n_p2b = add_node_call(f_p2b);
	n_p2b->set_num_params(1);
	n_p2b->set_param(0, add_node_local(var));
	n_if->set_param(0, n_p2b);
	n_if->set_param(1, concretify_node(cp_node(node->params[3], block_x), block_x, ns));
	if (node->params.num >= 5)
		n_if->set_param(2, concretify_node(cp_node(node->params[4], block_x), block_x, ns));
	block_x->add(n_if);

	return block_x;
}

shared<Node> Concretifier::concretify_statement_for_unwrap_pointer_shared(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// [OUT-VAR, ---, EXPRESSION, TRUE-BLOCK, [FALSE-BLOCK]]
	auto expr = container;//concretify_node(node->params[2], block, ns);
	auto t0 = expr->type;
	auto var_name = node->params[0]->as_token();

	auto block_x = new Block(block->function, block);
	auto t_out = tree->request_implicit_class_shared_not_null(t0->param[0], node->token_id);

	auto *var = block_x->add_var(var_name, t_out);
	block_x->add(parser->con.link_operator_id(OperatorID::Assign, add_node_local(var), expr->change_type(t_out)));

	auto n_if = add_node_statement(StatementID::If, node->token_id);
	n_if->set_num_params(node->params.num - 2);
	Function *f_p2b = tree->required_func_global("p2b", node->token_id);
	auto n_p2b = add_node_call(f_p2b);
	n_p2b->set_num_params(1);
	n_p2b->set_param(0, add_node_local(var));
	n_if->set_param(0, n_p2b);
	n_if->set_param(1, concretify_node(cp_node(node->params[3], block_x), block_x, ns));
	if (node->params.num >= 5)
		n_if->set_param(2, concretify_node(cp_node(node->params[4], block_x), block_x, ns));
	block_x->add(n_if);

	return block_x;
}

shared<Node> Concretifier::concretify_statement_for_unwrap_optional(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// [OUT-VAR, ---, EXPRESSION, TRUE-BLOCK, [FALSE-BLOCK]]
	auto expr = concretify_node(node->params[2], block, ns);
	auto t0 = expr->type;
	auto var_name = node->params[0]->as_token();

	auto block_x = new Block(block->function, block);

	auto t_out = tree->request_implicit_class_alias(t0->param[0], node->token_id);

	auto *var = block_x->add_var(var_name, t_out);
	auto assign = add_node_operator_by_inline(InlineID::PointerAssign, add_node_local(var), expr->ref(t_out));

	auto n_if = add_node_statement(StatementID::If, node->token_id);
	n_if->set_num_params(node->params.num - 2);
	auto f_has_val = t0->get_member_func(Identifier::func::OptionalHasValue, TypeBool, {});
//	if (!f_has_val)
//		do_error("")
	n_if->set_param(0, add_node_member_call(f_has_val, expr));
	n_if->set_param(1, concretify_node(cp_node(node->params[3], block_x), block_x, ns));
	if (node->params.num >= 5)
		n_if->set_param(2, concretify_node(cp_node(node->params[4], block_x), block_x, ns));
	block_x->add(n_if);

	n_if->params[1]->params.insert(assign, 0);

	return block_x;
}

shared<Node> Concretifier::concretify_statement_while(shared<Node> node, Block *block, const Class *ns) {
	// [COND, BLOCK]
	concretify_all_params(node, block, ns);
	node->type = TypeVoid;
	node->params[0] = check_param_link(node->params[0], TypeBool, Identifier::While);
	return node;
}

shared<Node> Concretifier::concretify_statement_for_range(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, VALUE0, VALUE1, STEP, BLOCK]

	auto var_name = node->params[0]->as_token();
	auto val0 = force_concrete_type(concretify_node(node->params[1], block, ns));
	auto val1 = force_concrete_type(concretify_node(node->params[2], block, ns));
	auto step = node->params[3];
	if (step)
		step = force_concrete_type(concretify_node(step, block, ns));

	// type?
	const Class *t = val0->type;
	if (val1->type == TypeFloat32)
		t = val1->type;
	if (step)
		if (step->type == TypeFloat32)
			t = step->type;

	if (!step) {
		if (t) {
			step = add_node_const(tree->add_constant_int(1));
		} else {
			step = add_node_const(tree->add_constant(TypeFloat32));
			step->as_const()->as_float() = 1.0f;
		}
	}

	val0 = check_param_link(val0, t, "for", 1, 2);
	val1 = check_param_link(val1, t, "for", 1, 2);
	if (step)
		step = check_param_link(step, t, "for", 1, 2);

	node->params[1] = val0;
	node->params[2] = val1;
	node->params[3] = step;

	// variable...
	auto *var = block->add_var(var_name, t);
	node->set_param(0, add_node_local(var));

	// block
	node->params[4] = concretify_node(node->params[4], block, ns);
	parser->post_process_for(node);

	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_for_container(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, INDEX, ARRAY, BLOCK]

	auto container = force_concrete_type(concretify_node(node->params[2], block, ns));
	container = deref_if_reference(container);
	auto t_c = container->type;
	if (t_c->is_pointer_shared() and flags_has(node->flags, Flags::Shared))
		return concretify_statement_for_unwrap_pointer_shared(node, container, block, ns);
	else if (t_c->is_pointer_shared() or t_c->is_pointer_owned() or t_c->is_pointer_raw())
		return concretify_statement_for_unwrap_pointer(node, container, block, ns);
	else if (t_c->is_optional())
		return concretify_statement_for_unwrap_optional(node, container, block, ns);
	else if (t_c->usable_as_list() or t_c->is_array())
		return concretify_statement_for_array(node, container, block, ns);
	else if (t_c->is_dict())
		return concretify_statement_for_dict(node, container, block, ns);

	do_error(format("unable to iterate over type '%s' - array/list/dict/shared/owned/optional expected as second parameter in 'for . in .'", t_c->long_name()), container);
	return nullptr;
}

shared<Node> Concretifier::concretify_statement_for_array(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// variable...
	node->params[2] = container;

	auto var_name = node->params[0]->as_token();
	auto var_type = tree->request_implicit_class_alias(container->type->get_array_element(), node->params[0]->token_id);
	auto var = block->add_var(var_name, var_type);
	if (node->is_mutable()) {
		if (!container->is_mutable())
			do_error("can not iterate mutating over a constant container", node);
	} else {
		flags_clear(var->flags, Flags::Mutable);
	}
	node->set_param(0, add_node_local(var));

	string index_name = format("-for_index_%d-", for_index_count ++);
	if (node->params[1])
		index_name = node->params[1]->as_token();
	auto index = block->add_var(index_name, TypeInt32);
	node->set_param(1, add_node_local(index));

	// block
	node->params[3] = concretify_node(node->params[3], block, ns);
	parser->post_process_for(node);

	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_for_dict(shared<Node> node, shared<Node> container, Block *block, const Class *ns) {
	// variable...
	node->params[2] = container;

	auto var_name = node->params[0]->as_token();
	auto var_type = tree->request_implicit_class_alias(container->type->get_array_element(), node->params[0]->token_id);
	auto key_type = tree->request_implicit_class_alias(TypeString, node->params[0]->token_id);
	auto var = block->add_var(var_name, var_type);
	if (!container->is_mutable())
		flags_clear(var->flags, Flags::Mutable);
	node->set_param(0, add_node_local(var));

	string key_name = format("-for_key_%d-", for_index_count ++);
	if (node->params[1])
		key_name = node->params[1]->as_token();
	auto index = block->add_var(key_name, key_type);
	node->set_param(1, add_node_local(index));

	// block
	node->params[3] = concretify_node(node->params[3], block, ns);
	parser->post_process_for(node);

	node->type = TypeVoid;
	return node;
}

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
	if (auto *f = node->type->get_member_func(Identifier::func::Length, TypeInt32, {}))
		return add_node_member_call(f, node, node->token_id);

	// element "int num/length"?
	for (auto &e: node->type->elements)
		if (e.type == TypeInt32 and (e.name == "length" or e.name == "num")) {
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

shared<Node> Concretifier::concretify_statement_new(shared<Node> node, Block *block, const Class *ns) {
	auto constr = concretify_node(node->params[0], block, block->name_space());
	if (constr->kind != NodeKind::ConstructorAsFunction)
		do_error("constructor call expected after 'new'", node->params[0]);
	constr->kind = NodeKind::CallFunction;
	constr->type = TypeVoid;
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
		node->type = TypeVoid;
		return node;
	}*/
	if (p->type->is_pointer_shared() or p->type->is_pointer_owned()) {
		if (auto f = p->type->get_member_func(Identifier::func::SharedClear, TypeVoid, {}))
			return add_node_member_call(f, p, p->token_id);
		do_error("clear missing...", p);
	} else if (p->type->is_list()) {
		if (auto f = p->type->get_member_func("clear", TypeVoid, {}))
			return add_node_member_call(f, p, p->token_id);
		do_error("clear missing...", p);
	}

	// override del operator?
	if (auto f = p->type->get_member_func(Identifier::func::DeleteOverride, TypeVoid, {}))
		return add_node_member_call(f, p, node->token_id);

	do_error("shared/owned pointer expected after 'del'", node->params[0]);
	return nullptr;
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
		auto crit = tree->add_constant(TypeString);
		node->set_param(1, add_node_const(crit));
	}

	auto array = force_concrete_type(node->params[0]);
	auto crit = force_concrete_type(node->params[1]);

	if (!array->type->is_list())
		do_error(format("%s(): first parameter must be a list[]", Identifier::Sort), array);
	if (crit->type != TypeString or crit->is_mutable())
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

shared<Node> Concretifier::concretify_statement_raw_function_pointer(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	if (sub->kind != NodeKind::Function)
		do_error("raw_function_pointer() expects a function name", sub);
	auto func = add_node_const(tree->add_constant(TypeFunctionCodeRef), node->token_id);
	func->as_const()->as_int64() = (int_p)sub->as_func(); // will be replaced during linking

	node = node->shallow_copy();
	node->type = TypeFunctionCodeRef;
	node->set_param(0, func);
	return node;
}

shared<Node> Concretifier::concretify_statement_try(shared<Node> node, Block *block, const Class *ns) {
	// [TRY-BLOCK, EX:[TYPE, NAME], EX-BLOCK, ...]

	auto try_block = concretify_node(node->params[0], block, block->name_space());
	node->params[0] = try_block;

	int num_exceptions = (node->params.num - 1) / 2;

	for (int i=0; i<num_exceptions; i++) {

		auto ex = node->params[1 + 2*i];

		auto ex_block = node->params[2 + 2*i];

		if (ex->params.num > 0) {
			auto ex_type = ex->params[0];
			ex_type = concretify_node(ex_type, block, block->name_space());
			auto type = try_digest_type(tree, ex_type);
			auto var_name = ex->params[1]->as_token();

			ex->params.resize(1);


			if (!type)
				do_error("Exception class expected", ex_type);
			if (!type->is_derived_from(TypeException))
				do_error("Exception class expected", ex_type);
			ex->type = type;

			auto *v = ex_block->as_block()->add_var(var_name, tree->get_pointer(type, -1));
			ex->set_param(0, add_node_local(v));
		} else {
			ex->type = TypeVoid;
		}

		// find types AFTER creating the variable
		ex_block = concretify_node(ex_block, block, block->name_space());
		node->params[2 + 2*i] = ex_block;
	}
	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_raise(shared<Node> node, Block *block, const Class *ns) {
	return node;
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
		con->type = TypeVoid;

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

	if (f->block->params.num == 1) {
		// func(i)              (multi line)
		//     bla..
		//     return i*i       (explicit return)

		auto cmd = f->block->params[0];
		cmd = concretify_node(cmd, f->block.get(), block->name_space());

		f->literal_return_type = cmd->type;
		f->effective_return_type = cmd->type;

		if (cmd->type == TypeVoid) {
			f->block->params[0] = cmd;
		} else {
			auto ret = add_node_statement(StatementID::Return);
			ret->set_num_params(1);
			ret->params[0] = cmd;
			f->block->params[0] = ret;
		}

	} else {
		// func(i) i*i      (single line, direct return)
		f->block->type = TypeUnknown;
		f->literal_return_type = TypeVoid;
		f->effective_return_type = TypeVoid;
		concretify_node(f->block.get(), f->block.get(), f->name_space);
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
	tree->transform_block(f->block.get(), find_captures);


// --- no captures?
	if (captured_variables.num == 0) {
		f->update_parameters_after_parsing();
		return add_node_func_name(f);
	}

	auto explicit_param_types = f->literal_param_type;


	if (config.verbose)
		msg_write("CAPTURES:");

	Array<bool> capture_via_ref;

	auto should_capture_via_ref = [this, node] (Variable *v) {
		if (v->name == Identifier::Self)
			return true;
		if (v->type->can_memcpy() or v->type == TypeString /*or v->type->is_pointer_shared() or v->type->is_pointer_shared_not_null()*/)
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


		auto new_param = f->add_param(v->name, cap_type, Flags::None);
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
		tree->transform_block(f->block.get(), replace_local);
	}

	f->update_parameters_after_parsing();

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
			if (node->params[1+i*2]->kind == NodeKind::Class)
				node->params[1+i*2] = add_node_const(tree->add_constant_pointer(TypeClassRef, node->params[1+i*2]->as_class()));

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
	if (s->id == StatementID::ForRange)
		return concretify_statement_for_range(node, block, ns);
	if (s->id == StatementID::ForContainer)
		return concretify_statement_for_container(node, block, ns);
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

	node->show();
	do_error("INTERNAL: unexpected statement", node);
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

shared<Node> Concretifier::concretify_operator(shared<Node> node, Block *block, const Class *ns) {
	auto op_no = node->as_abstract_op();

	if (op_no->id == OperatorID::PipeRight) {
		// concretify_all_params(..); NO, the parameters' types will influence each other
		// well... we're abusing that we will always get the FIRST 2 pipe elements!!!
		return build_function_pipe(node->params[0], node->params[1], block, ns, node->token_id);
	} else if (op_no->id == OperatorID::MapsTo) {
		return build_lambda_new(node->params[0], node->params[1], block, ns, node->token_id);
	}
	concretify_all_params(node, block, ns);

	if (node->params.num == 2) {
		// binary operator A+B
		auto param1 = node->params[0];
		auto param2 = force_concrete_type_if_function(node->params[1]);
		auto op = link_operator(op_no, param1, param2, node->token_id);
		if (!op)
			do_error(format("no operator found: '%s %s %s'", force_concrete_type(param1)->type->long_name(), op_no->name, give_useful_type(this, param2)->long_name()), node);
		return op;
	} else {
		return link_unary_operator(op_no, node->params[0], block, node->token_id);
	}
}

const Class *type_ownify_xfer(SyntaxTree *tree, const Class *t, int token_id) {
	if (t->is_pointer_xfer_not_null())
		return tree->request_implicit_class_owned_not_null(t->param[0], token_id);
	if (t->is_list()) {
		auto tt = type_ownify_xfer(tree, t->param[0], token_id);
		if (tt != t->param[0])
			return tree->request_implicit_class_list(tt, token_id);
	}
	return t;
}

bool is_non_owning_pointer(const Class *t) {
	return t->is_reference() or t->is_pointer_raw();
}

shared<Node> Concretifier::concretify_block(shared<Node> node, Block *block, const Class *ns) {
	for (int i=0; i<node->params.num; i++) {
		node->params[i] = concretify_node(node->params[i], node->as_block(), ns);
		if (node->params[i]->type->is_pointer_xfer_not_null())
			do_error("xfer[..] values must not be discarded", node->params[i]);
	}
	//concretify_all_params(node, node->as_block(), ns, this);

	// return type from last command:
	node->type = TypeVoid;
	if (node->params.num > 0) {
		auto b = node->params.back();
		if (b->type != TypeVoid)
			node->type = b->type;
	}

	for (int i=node->params.num-1; i>=0; i--)
		if (node->params[i]->kind == NodeKind::AbstractVar)
			node->params.erase(i);
	return node;
}

shared<Node> Concretifier::concretify_var_declaration(shared<Node> node, Block *block, const Class *ns) {
	// [TYPE?, VAR, =[VAR,EXPR]]
	bool as_mutable = node->is_mutable();

	// type?
	const Class *type = nullptr;
	if (node->params[0]) {
		// explicit type
		type = try_digest_type(tree, concretify_node(node->params[0], block, ns));
		//auto t = digest_type(tree, force_concrete_type(concretify_node(node->params[0], block, ns)));
		if (!type)
			do_error("variable declaration requires a type", node->params[0]);
		if (type->is_some_pointer_not_null() and node->params.num < 3)
			do_error("variables of reference type must be initialized", node->params[0]);
		if (type->is_pointer_xfer_not_null())
			do_error("no variables of type xfer[..] allowed", node->params[0]);
	} else {
		//assert(node->params[2]);
		auto rhs = force_concrete_type(concretify_node(node->params[2]->params[1], block, ns));
		node->params[2]->params[1] = rhs;
		// don't create xfer[X] variables!
		type = type_ownify_xfer(tree, rhs->type, node->token_id);
	}

	//as_const
	auto create_var = [block, &node, this] (const Class *type, const string &name) {
		if (type->needs_constructor() and !type->get_default_constructor())
			do_error(format("declaring a variable of type '%s' requires a constructor but no default constructor exists", type->long_name()), node);
		return block->add_var(name, type);
	};
	Array<Variable*> vars;

	// add variable(s)
	if (node->params[1]->kind == NodeKind::Tuple) {
		auto etypes = tuple_get_element_types(type);
		for (auto&& [i,t]: enumerate(etypes))
			vars.add(create_var(t, node->params[1]->params[i]->as_token()));
	} else {
		vars.add(create_var(type, node->params[1]->as_token()));
	}

	// assign?
	if (node->params.num == 3) {
		auto rhs = concretify_node(node->params[2]->params[1], block, ns);
		node->params[2]->params[1] = rhs;
		//if (type->is_some_pointer_not_null() and !rhs->type->is_pointer_xfer_not_null()) {
		if (is_non_owning_pointer(type)) {
			if (rhs->type != vars[0]->type)
				if (rhs->type != TypeNone)
					do_error(format("pointer initialization type mismatch '%s = %s'", vars[0]->type->long_name(), rhs->type->long_name()), rhs);
			node = add_node_operator_by_inline(InlineID::PointerAssign, add_node_local(vars[0]), rhs, node->token_id);
		} else {
			node = concretify_node(node->params[2], block, ns);
		}
	}
	if (!as_mutable)
		for (auto v: vars)
			flags_clear(v->flags, Flags::Mutable);
	return node;
}

shared<Node> Concretifier::concretify_array_builder_for(shared<Node> node, Block *block, const Class *ns) {
	// IN:  [FOR, EXP, IF]
	// OUT: [FOR, VAR]

	auto n_for = node->params[0];
	auto n_exp = node->params[1];
	auto n_cmp = node->params[2];

	// first pass: find types in the for loop
	auto fake_for = cp_node(n_for); //->shallow_copy();
	fake_for->set_param(fake_for->params.num - 1, cp_node(n_exp)); //->shallow_copy());
	fake_for = concretify_node(fake_for, block, ns);
	// TODO: remove new variables!

	return concretify_array_builder_for_inner(n_for, n_exp, n_cmp, fake_for->params.back()->type, block, ns, node->token_id);
}

shared<Node> Concretifier::concretify_array_builder_for_inner(shared<Node> n_for, shared<Node> n_exp, shared<Node> n_cmp, const Class *type_el, Block *block, const Class *ns, int token_id) {
	// OUT: [FOR, VAR]

	// create an array
	auto array_type = tree->request_implicit_class_list(type_el, token_id);
	auto array_var = block->add_var(block->function->create_slightly_hidden_name(), array_type);

	// array.add(exp)
	auto f_add = array_type->get_member_func("add", TypeVoid, {type_el});
	if (!f_add)
		do_error("...add() ???", token_id);
	auto n_add = new Node(NodeKind::AbstractCall, 0, TypeUnknown, Flags::Mutable, token_id);
	n_add->set_num_params(3);
	n_add->set_param(0, add_node_func_name(f_add));
	n_add->set_param(1, add_node_local(array_var));
	n_add->set_param(2, n_exp);

	// add new code to the loop
	Block *b;
	if (n_cmp) {
		auto b_if = new Block(block->function, block, TypeUnknown);
		auto b_add = new Block(block->function, b_if, TypeUnknown);
		b_add->add(n_add);

		auto n_if = add_node_statement(StatementID::If, token_id, TypeUnknown);
		n_if->set_param(0, n_cmp);
		n_if->set_param(1, b_add);

		b_if->add(n_if);
		b = b_if;
	} else {
		b = new Block(block->function, block, TypeUnknown);
		b->add(n_add);
	}

	n_for->set_param(n_for->params.num - 1, b);

	// NOW we can set the types
	n_for = concretify_node(n_for, block, ns);

	auto n = new Node(NodeKind::ArrayBuilderFor, 0, array_type);
	n->set_num_params(2);
	n->set_param(0, n_for);
	n->set_param(1, add_node_local(array_var));
	return n;
}

KabaException* create_exception(ErrorID code) {
	if (code == ErrorID::OPTIONAL_NO_VALUE)
		return new KabaNoValueError;
	if (code == ErrorID::NULL_POINTER)
		return new KabaNullPointerError();
	return new KabaException("???");
}

shared<Node> add_raise(SyntaxTree* tree, int token_id, ErrorID code) {
	auto e = create_exception(code);
	tree->raised_exceptions.add(e);
	auto node = add_node_statement(StatementID::Raise, token_id);
	node->set_param(0, add_node_const(tree->add_constant_pointer(TypePointer, e)));
	return node;
}

// concretify as far as possible
// will leave FLEXIBLE:
//  * list [...]
//  * dict {...}
//  * tuple (...)
//  * function name
shared<Node> Concretifier::concretify_node(shared<Node> node, Block *block, const Class *ns) {
	if (node->type != TypeUnknown)
		return node;

	if (node->kind == NodeKind::AbstractOperator) {
		return concretify_operator(node, block, ns);
	} else if (node->kind == NodeKind::Dereference) {
		concretify_all_params(node, block, ns);
		auto sub = node->params[0];//deref_if_reference(node->params[0]);
		if (block->is_trust_me()) {
			if (!sub->type->is_some_pointer())
				do_error("only pointers can be dereferenced using '*' inside 'trust_me'", node);
		} else {
			if (!sub->type->is_some_pointer_not_null()) //   is_pointer_raw()) // and !sub->type->is_reference())
				do_error("only not-null pointers (references, shared![X], owned![X]) can be dereferenced using '*'", node);
		}
		node->type = sub->type->param[0];
	} else if (node->kind == NodeKind::Reference) {
		concretify_all_params(node, block, ns);
		auto sub = node->params[0];
		node->type = tree->request_implicit_class_reference(sub->type, node->token_id);
		node->set_mutable(sub->is_mutable());
	} else if (node->kind == NodeKind::AbstractCall) {
		return concretify_call(node, block, ns);
	} else if (node->kind == NodeKind::Array) {
		return concretify_array(node, block, ns);
	} else if (node->kind == NodeKind::Tuple) {
		concretify_all_params(node, block, ns);
		// NOT specifying the type
		return node;
	} else if (node->kind == NodeKind::ArrayBuilder) {
		concretify_all_params(node, block, ns);
		// NOT specifying the type
		return node;
	} else if (node->kind == NodeKind::DictBuilder) {
		concretify_all_params(node, block, ns);
		for (int p=0; p<node->params.num; p+=2) {
			if (node->params[p]->type != TypeString)
				do_error(format("key type needs to be 'string', not '%s'", node->params[p]->type->long_name()), node->params[p]);
			if (node->params[p]->kind != NodeKind::Constant)
				do_error("key needs to be a compile-time constant", node->params[p]);
		}
		// NOT specifying the type
		return node;
	} else if (node->kind == NodeKind::Function) {
		return node;
	} else if (node->kind == NodeKind::AbstractTypeStar) {
		concretify_all_params(node, block, ns);
		auto t = try_digest_type(tree, node->params[0]);
		if (!t)
			do_error("type expected before '*'", node->params[0]);
		return add_node_class(tree->get_pointer(t, node->token_id), node->token_id);
	} else if (node->kind == NodeKind::AbstractTypeReference) {
		concretify_all_params(node, block, ns);
		auto t = try_digest_type(tree, node->params[0]);
		if (!t)
			do_error("type expected before '&'", node->params[0]);
		t = tree->request_implicit_class_reference(t, node->token_id);
		return add_node_class(t, node->token_id);
	} else if (node->kind == NodeKind::AbstractTypeList) {
		concretify_all_params(node, block, ns);
		auto t = try_digest_type(tree, node->params[0]);
		if (!t)
			do_error("type expected before '[]'", node->params[0]);
		t = tree->request_implicit_class_list(t, node->token_id);
		return add_node_class(t, node->token_id);
	} else if (node->kind == NodeKind::AbstractTypeDict) {
		concretify_all_params(node, block, ns);
		auto t = try_digest_type(tree, node->params[0]);
		if (!t)
			do_error("type expected before '{}'", node->params[0]);
		t = tree->request_implicit_class_dict(t, node->token_id);
		return add_node_class(t, node->token_id);
	} else if (node->kind == NodeKind::AbstractTypeOptional) {
		concretify_all_params(node, block, ns);
		if (auto t = try_digest_type(tree, node->params[0]))
			return add_node_class(tree->request_implicit_class_optional(t, node->token_id), node->token_id);
		/*auto t = node->params[0]->type;
		if (t->is_optional()) {
			auto n = new Node(NodeKind::MAYBE, 0, t->param[0], node->flags, node->token_id);
			n->set_num_params(1);
			n->set_param(0, node->params[0]);
			return n;
		}*/
		do_error("type expected before '?'", node->params[0]);
	} else if (node->kind == NodeKind::AbstractTypeCallable) {
		concretify_all_params(node, block, ns);
		auto t0 = try_digest_type(tree, node->params[0]);
		auto t1 = try_digest_type(tree, node->params[1]);
		if (!t0)
			do_error("type expected before '->'", node->params[0]);
		if (!t1)
			do_error("type expected before '->'", node->params[1]);
		return add_node_class(tree->request_implicit_class_callable_fp({t0}, t1, node->token_id), node->token_id);
	} else if ((node->kind == NodeKind::AbstractToken) or (node->kind == NodeKind::AbstractElement)) {
		auto operands = concretify_node_multi(node, block, ns);
		if (operands.num > 1) {
			for (auto o: weak(operands))
				o->show();
			msg_write(format("WARNING: node not unique:  %s  -  line %d", node->as_token(), reinterpret_cast<SyntaxTree*>(node->link_no)->expressions.token_physical_line_no(node->token_id) + 1));
		}
		if (operands.num > 0)
			return operands[0];
	} else if (node->kind == NodeKind::Statement) {
		return concretify_statement(node, block, ns);
	/*} else if (node->kind == NodeKind::CALL_SPECIAL_FUNCTION) {
		return concretify_special_function(node, block, ns);
	} else if (node->kind == NodeKind::SPECIAL_FUNCTION_NAME) {*/
	} else if (node->kind == NodeKind::Block) {
		return concretify_block(node, block, ns);
	} else if (node->kind == NodeKind::AbstractVar) {
		return concretify_var_declaration(node, block, ns);
	} else if (node->kind == NodeKind::ArrayBuilderFor) {
		return concretify_array_builder_for(node, block, ns);
	} else if (node->kind == NodeKind::None) {
	} else if (node->kind == NodeKind::CallFunction) {
		concretify_all_params(node, block, ns);
		node->type = node->as_func()->literal_return_type;
	} else if (node->kind == NodeKind::Slice) {
		concretify_all_params(node, block, ns);
		node->type = TypeVoid;
	} else if (node->kind == NodeKind::Definitely) {
		return concretify_definitely(node, block, ns);
	} else if (node->kind == NodeKind::NamedParameter) {
		node->params[1] = concretify_node(node->params[1], block, ns);
		node->type = node->params[1]->type;
	} else {
		node->show();
		do_error("INTERNAL ERROR: unexpected node", node);
	}

	return node;
}

// TODO: wrap in block + save input in variable
// TODO: variable life-time ... maybe in outer block?
shared<Node> Concretifier::concretify_definitely(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	auto sub = node->params[0];
	auto t = sub->type;
	if (t->is_optional()) {
		// optional?
		if (block->is_trust_me()) {
			return sub->change_type(t->param[0]);
		} else {
			// value or raise
			auto bb = new Block(block->function, block, TypeUnknown);
			auto cmd_if = add_node_statement(StatementID::If, node->token_id, TypeVoid);
			if (auto f = t->get_member_func(Identifier::func::OptionalHasValue, TypeBool, {}))
				cmd_if->set_param(0, add_node_operator_by_inline(InlineID::BoolNot, add_node_member_call(f, sub), nullptr, node->token_id));
			if (block->is_in_try())
				cmd_if->set_param(1, add_node_statement(StatementID::RaiseLocal, node->token_id, TypeVoid));
			else
				cmd_if->set_param(1, add_raise(tree, node->token_id, ErrorID::OPTIONAL_NO_VALUE));
			bb->add(cmd_if);
			bb->add(sub->change_type(t->param[0]));
			return concretify_node(bb, block, ns);
		}
	} else if (t->is_pointer_raw() or t->is_pointer_owned() or t->is_pointer_shared()) {
		// null-able pointer?
		const Class* t_def;
		if (t->is_pointer_raw())
			t_def = tree->request_implicit_class_reference(t->param[0], node->token_id);
		if (t->is_pointer_owned())
			t_def = tree->request_implicit_class_owned_not_null(t->param[0], node->token_id);
		if (t->is_pointer_shared())
			t_def = tree->request_implicit_class_shared_not_null(t->param[0], node->token_id);

		if (block->is_trust_me()) {
			return sub->change_type(t_def);
		} else {
			auto bb = new Block(block->function, block, TypeUnknown);
			auto cmd_if = add_node_statement(StatementID::If, node->token_id, TypeVoid);
			auto f = tree->required_func_global("p2b", node->token_id);
			auto cmd_p2b = add_node_call(f, node->token_id);
			cmd_p2b->set_param(0, sub);
			cmd_if->set_param(0, add_node_operator_by_inline(InlineID::BoolNot, cmd_p2b, nullptr, node->token_id));
			if (block->is_in_try())
				cmd_if->set_param(1, add_node_statement(StatementID::RaiseLocal, node->token_id, TypeVoid));
			else
				cmd_if->set_param(1, add_raise(tree, node->token_id, ErrorID::NULL_POINTER));
			bb->add(cmd_if);
			bb->add(sub->change_type(t_def));
			return concretify_node(bb, block, ns);
		}
	} else {
		do_error("'!' only allowed for optional values and null-able pointers", node);
	}
	return node;
}

const Class *Concretifier::concretify_as_type(shared<Node> node, Block *block, const Class *ns) {
	auto cc = concretify_node(node, block, ns);
	auto t = try_digest_type(tree, cc);
	if (!t) {
		cc->show();
		do_error("type expected", cc);
	}
	return t;
}


const Class *type_more_dominant(const Class *a, const Class *b) {
	if (a == b)
		return a;
	auto is_x = [a, b] (const Class *t1, const Class *t2) {
		return ((a == t1 and b == t2) or (a == t2 and b == t1));
	};
	if (is_x(TypeInt32, TypeFloat32))
		return TypeFloat32;
	if (is_x(TypeInt32, TypeFloat64))
		return TypeFloat64;
	if (is_x(TypeInt32, TypeInt64))
		return TypeInt64;
	if (is_x(TypeInt64, TypeFloat64))
		return TypeFloat64;
	return nullptr;
}

shared<Node> Concretifier::wrap_node_into_callable(shared<Node> node) {
	if (node->kind != NodeKind::Function)
		return node;
	auto f = node->as_func();
	auto callable = wrap_function_into_callable(f, node->token_id);
	if (f->is_member() and node->params.num > 0 and node->params[0]) {
		shared_array<Node> captures = {node->params[0]->ref(tree)}; // self
		Array<bool> capture_via_ref = {true};
		for (int i=1; i<f->literal_param_type.num; i++) {
			captures.add(nullptr);
			capture_via_ref.add(false);
		}
		return create_bind(this, callable, captures, capture_via_ref);
	}
	return callable;
}

// f : (A,B,...)->R  =>  new Callable[](f) : (A,B,...)->R
shared<Node> Concretifier::wrap_function_into_callable(Function *f, int token_id) {
	auto t = tree->request_implicit_class_callable_fp(f, token_id);

	for (auto *cf: t->param[0]->get_constructors()) {
		if (cf->num_params == 2) {
			auto cmd = add_node_statement(StatementID::New, token_id);
			auto con = add_node_constructor(cf);
			auto fp = tree->add_constant(TypeFunctionRef);
			fp->as_int64() = (int_p)f;
			con = apply_params_direct(con, {add_node_const(fp, token_id)}, 1);
			con->kind = NodeKind::CallFunction;
			con->type = TypeVoid;

			cmd->type = t;
			cmd->set_param(0, con);
			return cmd;
		}
	}
	do_error("wrap_function_into_callable() failed? " + f->signature(), token_id);
	return nullptr;
}

void Concretifier::force_concrete_types(shared_array<Node> &nodes) {
	for (int i=0; i<nodes.num; i++)
		nodes[i] = force_concrete_type(nodes[i].get());
}

shared<Node> Concretifier::force_concrete_type_if_function(shared<Node> node) {
	if (node->kind == NodeKind::Function)
		return wrap_node_into_callable(node);
	return node;
}

shared<Node> Concretifier::force_concrete_type(shared<Node> node) {
	if (node->type != TypeUnknown)
		return node;

	if (node->kind == NodeKind::ArrayBuilder) {
		if (node->params.num == 0) {
			node->type = TypeInt32List;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[0]->type;
		for (int i=1; i<node->params.num; i++)
			t = type_more_dominant(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract array", node);

		for (int i=0; i<node->params.num; i++) {
			CastingDataSingle cast;
			type_match_with_cast(node->params[i].get(), false, t, cast);
			node->params[i] = apply_type_cast(cast, node->params[i].get(), t);
		}

		node->type = tree->request_implicit_class_list(t, node->token_id);
		return node;
	} else if (node->kind == NodeKind::DictBuilder) {
		if (node->params.num == 0) {
			node->type = TypeInt32Dict;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[1]->type;
		for (int i=3; i<node->params.num; i+=2)
			t = type_more_dominant(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract dict", node);

		for (int i=1; i<node->params.num; i+=2) {
			CastingDataSingle cast;
			type_match_with_cast(node->params[i].get(), false, t, cast);
			node->params[i] = apply_type_cast(cast, node->params[i].get(), t);
		}

		node->type = tree->request_implicit_class_dict(t, node->token_id);
		return node;
	} else if (node->kind == NodeKind::Tuple) {
		auto type = merge_type_tuple_into_product(tree, node_extract_param_types(node), node->token_id);
		auto xx = turn_class_into_constructor(type, node->params, node->token_id);
		return try_to_match_apply_params(xx, node->params);
	} else if (node->kind == NodeKind::Function) {
		return wrap_node_into_callable(node);
	} else {
		do_error("unhandled abstract type: " + kind2str(node->kind), node);
	}
	return node;
}


shared<Node> Concretifier::make_func_node_callable(const shared<Node> l) {
	Function *f = l->as_func();
	//auto r = add_node_call(f);
	auto r = l->shallow_copy();
	r->kind = NodeKind::CallFunction;
	r->type = f->literal_return_type;
	r->set_num_params(f->num_params);

	// virtual?
	if (f->virtual_index >= 0)
		r->kind = NodeKind::CallVirtual;
	return r;
}

shared<Node> Concretifier::match_template_params(const shared<Node> l, const shared_array<Node> &params, bool allow_fail) {
	auto f0 = l->as_func();
	auto _params = params;
	force_concrete_types(_params);
	try {
		auto ff = context->template_manager->request_function_instance_matching(tree, f0, _params, l->token_id);
		auto r = l->shallow_copy();
		r->link_no = (int_p)ff;
		return r;
	} catch (...) {
		if (allow_fail)
			return nullptr;
		throw;
	}
}

shared<Node> Concretifier::make_func_pointer_node_callable(const shared<Node> l) {
	auto f = l->type->param[0]->get_call();

	shared<Node> c;
	if (f->virtual_index >= 0) {
		c = new Node(NodeKind::CallVirtual, (int_p)f, f->literal_return_type);
	} else {
		do_error("function pointer call should be virtual???", l);
		c = new Node(NodeKind::CallFunction, (int_p)f, f->literal_return_type);
	}
	c->set_num_params(f->num_params);
	c->set_instance(l->deref());
	return c;
}

shared<Node> SyntaxTree::make_fake_constructor(const Class *t, const Class *param_type, int token_id) {
	//if ((t == TypeInt32) and (param_type == TypeFloat32))
	//	return add_node_call(get_existence("f2i", nullptr, nullptr, false)[0]->as_func());
	if (param_type->is_some_pointer_not_null())
		param_type = param_type->param[0];

	string fname = "__" + t->name + "__";
	auto *cf = param_type->get_member_func(fname, t, {});
	if (!cf)
		do_error(format("illegal fake constructor... requires '%s.%s()'", param_type->long_name(), fname), token_id);
	return add_node_member_call(cf, nullptr, token_id); // temp var added later...

	auto *dummy = new Node(NodeKind::Placeholder, 0, TypeVoid);
	return add_node_member_call(cf, dummy, token_id); // temp var added later...
}

shared_array<Node> Concretifier::turn_class_into_constructor(const Class *t, const shared_array<Node> &params, int token_id) {
	if (((t == TypeInt32) or (t == TypeFloat32) or (t == TypeInt64) or (t == TypeFloat64) or (t == TypeBool) or (t == TypeInt8) or (t == TypeUInt8)) and (params.num == 1))
		return {tree->make_fake_constructor(t, params[0]->type, token_id)};

	// constructor
	//auto *vv = block->add_var(block->function->create_slightly_hidden_name(), t);
	//vv->explicitly_constructed = true;
	//shared<Node> dummy = add_node_local(vv);
	shared_array<Node> links;
	for (auto *cf: t->get_constructors())
		if ((params.num >= cf->mandatory_params-1) and (params.num <= cf->num_params-1)) // skip "self"
			links.add(add_node_constructor(cf, token_id));
	if (links.num == 0) {
		for (auto *cf: t->get_constructors()) {
			msg_write(cf->signature(TypeVoid));
			msg_write(cf->mandatory_params);
		}
		do_error(format("class %s does not have a constructor with %d parameters", t->long_name(), params.num), token_id);
	}
	return links;
}

shared<Node> Concretifier::link_unary_operator(AbstractOperator *po, shared<Node> operand, Block *block, int token_id) {
	Operator *op = nullptr;
	const Class *p1 = operand->type;

	// exact match?
	bool ok = false;
	for (auto _op: weak(context->global_operators))
		if (po == _op->abstract)
			if (!_op->param_type_2 and type_match_up(p1, _op->param_type_1)) {
				op = _op;
				ok = true;
				break;
			}


	// needs type casting?
	if (!ok) {
		CastingDataSingle current;
		CastingDataSingle best = {-1, 10000};
		const Class *t_best = nullptr;
		for (auto _op: weak(context->global_operators))
			if (po == _op->abstract)
				if (!_op->param_type_2 and type_match_with_cast(operand, false, _op->param_type_1, current)) {
					ok = true;
					if (current.penalty < best.penalty) {
						op = _op;
						best = current;
						t_best = _op->param_type_1;
					}
			}
		// cast
		if (ok) {
			operand = apply_type_cast(best, operand, t_best);
		}
	}


	if (!ok)
		do_error(format("unknown unitary operator '%s %s'", po->name, p1->long_name()), token_id);
	return add_node_operator(op, operand, nullptr, token_id);
}

void check_function_signature_legal(Concretifier *c, Function *f) {
	auto forbidden = [](const Class *t) {
		return t->is_pointer_owned() or t->is_pointer_owned_not_null();
	};

	if (forbidden(f->literal_return_type))
		c->do_error("return type must not be owned. Use xfer[...] instead", f->abstract_return_type);
	for (int i=0; i<f->num_params; i++)
		if (forbidden(f->literal_param_type[i]))
			c->do_error("parameter must not be owned. Use xfer[...] instead", f->abstract_param_types[i]);
}

void Concretifier::concretify_function_header(Function *f) {
	auto block = tree->root_of_all_evil->block.get();

	f->set_return_type(TypeVoid);
	if (f->abstract_return_type) {
		f->set_return_type(concretify_as_type(f->abstract_return_type, block, f->name_space));
	}
	f->literal_param_type.resize(f->abstract_param_types.num);
	for (auto&& [i,at]: enumerate(weak(f->abstract_param_types))) {
		auto t = concretify_as_type(at, block, f->name_space);
		auto v = f->var[i].get();
		v->type = t;
		f->literal_param_type[i] = t;

		// mandatory_params not yet
		if ((i < f->default_parameters.num) and f->default_parameters[i]) {
			f->default_parameters[i] = concretify_node(f->default_parameters[i], block, f->name_space);
			if (f->default_parameters[i]->type != t)
				do_error(format("trying to set a default value of type '%s' for a parameter of type '%s'", f->default_parameters[i]->type->name, t->name), f->default_parameters[i]);
		}
	}
	flags_clear(f->flags, Flags::Template);

	check_function_signature_legal(this, f);
}

void Concretifier::concretify_function_body(Function *f) {
	concretify_node(f->block.get(), f->block.get(), f->name_space);

	// auto implement destructor?
	if (f->name == Identifier::func::Delete)
		auto_implementer->implement_regular_destructor(f, f->name_space);
}

Array<const Class*> Concretifier::type_list_from_nodes(const shared_array<Node> &nn) {
	Array<const Class*> t;
	for (auto &n: nn)
		t.add(force_concrete_type(n)->type);
	return t;
}

shared<Node> check_const_params(SyntaxTree *tree, shared<Node> n) {
	if ((n->kind == NodeKind::CallFunction) or (n->kind == NodeKind::CallVirtual)) {
		auto f = n->as_func();
		int offset = 0;

		// "ref" parameter -> return mut/const depends on param!
		if (f->num_params >= 1) {
			if (flags_has(f->var[0]->flags, Flags::Ref) or flags_has(f->flags, Flags::Ref))
				n->set_mutable(n->params[0]->is_mutable());
		}

		// const check
		if (f->is_member()) {
			offset = 1;
			if (f->is_selfref()) {
			} else if (!n->params[0]->is_mutable() and f->is_mutable()) {
				tree->do_error(f->long_name() + ": member function expects a mutable instance, because it is declared 'mut'", n->token_id);
			}
		}
		for (int i=offset; i<f->num_params; i++)
			if (!n->params[i]->is_mutable() and f->var[i]->is_mutable())
				tree->do_error(format("%s: function parameter %d ('%s') is 'out' and does not accept a constant value", f->long_name(), i+1-offset, f->var[i]->name), n->token_id);
	}
	return n;
}

// links can include calls to templates!
shared<Node> Concretifier::try_to_match_apply_params(const shared_array<Node> &links, shared_array<Node> &_params) {
	//force_concrete_types(params);
	// no, keep params FLEXIBLE

	if (links.num == 0)
		do_error("can not call ...WTF??", -1); //, links[0]);

	CastingDataCall casts;
	casts.penalty = 10000000;
	shared<Node> chosen;
	shared_array<Node> chosen_params;

	struct ParamMapResult {
		enum class Code {
			Ok,
			ErrorTooMany,
			ErrorTooFew,
			ErrorNameNotFound
		};
		Code code;
		int index;
	};


	auto sort_params = [this, &_params] (shared_array<Node>& params, shared<Node> operand) -> ParamMapResult {
		int offset = 0;
		params.resize(get_num_wanted_params(operand));

		// self?
		if (node_is_member_function_with_instance(operand)) {
			params[0] = operand->params[0];
			offset = 1;
		}

		if (offset + _params.num > params.num)
			return {ParamMapResult::Code::ErrorTooMany, params.num};

		if (operand->is_function()) {
			auto f = operand->as_func();

			for (const auto& [i,p]: enumerate(weak(_params))) {
				if (p->kind == NodeKind::NamedParameter) {
					string name = p->params[0]->as_token();
					bool found = false;
					for (int k=0; k<f->num_params; k++)
						if (f->var[k]->name == name) {
							params[k] = p->params[1];
							found = true;
						}
					if (!found)
						return {ParamMapResult::Code::ErrorNameNotFound, i};
				} else {
					params[offset++] = p;
				}
			}
			for (int i=0; i<f->num_params; i++)
				if (!params[i]) {
					if (i >= f->mandatory_params and f->default_parameters[i]) {
						params[i] = f->default_parameters[i];
					} else {
						return {ParamMapResult::Code::ErrorTooFew, f->mandatory_params};
					}
				}
		} else {
			params = _params;
		}
		return {ParamMapResult::Code::Ok, 0};
	};

	for (auto& operand: links) {

		//auto params = _params;
		shared_array<Node> params;
		if (sort_params(params, operand).code != ParamMapResult::Code::Ok)
			continue;

		// direct match...
		if (direct_param_match(operand, params))
			return check_const_params(tree, apply_params_direct(operand, params));

		// advanced match...

		// type casting?
		CastingDataCall cur_casts;
		if (!param_match_with_cast(operand, params, cur_casts))
			continue;
		if (cur_casts.penalty < casts.penalty){
			casts = cur_casts;
			chosen = operand;
			chosen_params = params;
		}
	}

	if (chosen)
		return check_const_params(tree, apply_params_with_cast(chosen, chosen_params, casts));


// error messages


	for (auto operand: links) {
		if (operand->kind == NodeKind::CallFunction) {
			auto f = operand->as_func();
			if (f->is_template()) {
				if (auto ff = match_template_params(operand, _params, true)) {
				} else {
					do_error("template parameter matching or instantiation failed", operand);
				}
			}
		}
	}

	//do_error("invalid parameters to call (TODO improve error)", links[0]);
	if (links.num == 1) {
		shared_array<Node> params;
		const auto res = sort_params(params, links[0]);
		if (res.code == ParamMapResult::Code::Ok) {
			param_match_with_cast(links[0], params, casts);
			do_error("invalid function parameters: " + param_match_with_cast_error(params, casts.wanted), links[0]);
		} else if (res.code == ParamMapResult::Code::ErrorTooMany) {
			do_error(format("too many function parameters: %d given, %d expected", _params.num, res.index), links[0]);
		} else if (res.code == ParamMapResult::Code::ErrorTooFew) {
			do_error(format("too few function parameters: %d given, %d expected", _params.num, res.index), links[0]);
		} else if (res.code == ParamMapResult::Code::ErrorNameNotFound) {
			do_error(format("named function parameter not found: '%s'", _params[res.index]->params[0]->as_token()), _params[res.index]);
		} else {
			do_error("invalid function parameters: ... not mappable", links[0]);
		}
	}

	string found = type_list_to_str(type_list_from_nodes(_params));
	string available;
	for (auto link: links) {
		//auto p = get_wanted_param_types(link);
		//available += format("\n * %s for %s", type_list_to_str(p), link->sig(tree->base_class));
		available += format("\n * %s", link->signature(tree->base_class));
	}
	do_error(format("invalid function parameters: (%s) given, possible options:%s", found, available), links[0]);
	return shared<Node>();
}

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
		if (crit->type != TypeString or crit->kind != NodeKind::Constant)
			do_error(format("%s() expects a string literal when used in a pipe", Identifier::Sort), token_id);
		cmd->set_param(2, crit);
	} else {
		auto crit = tree->add_constant(TypeString);
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
	auto n_for = add_node_statement(StatementID::ForContainer, token_id, TypeUnknown);
	n_for->set_param(0, l->params[0]); // token variable
	//n_for->set_param(1, key);
	n_for->set_param(2, input);

	auto n = new Node(NodeKind::ArrayBuilderFor, token_id, TypeUnknown);
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
	auto n_for = add_node_statement(StatementID::ForContainer, token_id, TypeVoid);
	n_for->set_param(2, input);

	auto el_type = input->type->get_array_element();
	static int map_counter = 0;
	string viname = format("<map-index-%d>", map_counter);
	string vname = format("<map-var-%d>", map_counter++);
	auto var = block->add_var(vname, tree->request_implicit_class_reference(el_type, token_id));
	flags_clear(var->flags, Flags::Mutable);
	n_for->set_param(0, add_node_local(var));
	auto index = block->add_var(viname, TypeInt32);
	n_for->set_param(1, add_node_local(index));

	auto out = add_node_call(f->as_func(), f->token_id);

	CastingDataCall casts;
	auto nvar = add_node_local(var);

	if (!param_match_with_cast(out, {nvar}, casts))
		return nullptr; //do_error("pipe: " + param_match_with_cast_error({input}, wanted), f);

	auto n_exp = check_const_params(tree, apply_params_with_cast(out, {nvar}, casts));
	n_exp = concretify_node(n_exp, block, ns);

//	n_for->type = TypeUnknown;
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

	auto b = new Block(block->function, block, t_out);
	// variable into OUTER block for returnable life-time
	auto v = block->add_var(b->function->create_slightly_hidden_name(), input->type);

	b->add(auto_implementer->add_assign(block->function, "...", add_node_local(v), input));

	auto cif = add_node_statement(StatementID::If, token_id, t_out);
	cif->set_num_params(3);

	auto f_has_val = input->type->get_member_func(Identifier::func::OptionalHasValue, TypeBool, {});
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
	if (input->type == TypeUnknown)
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


shared<Node> Concretifier::build_lambda_new(const shared<Node> &param, const shared<Node> &expression, Block *block, const Class *ns, int token_id) {

	static int lambda_count = 0;
	string name = format(":lambda-evil-%d:", lambda_count ++);
	Function *f = tree->add_function(name, TypeUnknown, tree->base_class, Flags::Static);

	//f->abstract_param_types.add();
	[[maybe_unused]] auto v = f->add_param(param->as_token(), TypeInt32, Flags::None);
	parser->post_process_function_header(f, {}, tree->base_class, Flags::Static);

	// body
	f->block->add(expression);

	// statement wrapper
	auto node = add_node_statement(StatementID::Lambda, f->token_id, TypeUnknown);
	node->set_num_params(1);
	node->set_param(0, add_node_func_name(f));

	return concretify_statement_lambda(node, block, ns);
}

// when calling ...(...)
Array<const Class*> Concretifier::get_wanted_param_types(shared<Node> link, int &mandatory_params) {
	if (link->is_function()) {
		auto f = link->as_func();
		mandatory_params = f->mandatory_params;
		auto p = f->literal_param_type;
		/*if (f->is_member() and (link->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION))
			if (link->params.num == 0 or !link->params[0])
				p.insert(f->name_space, 0);*/
		if (link->kind == NodeKind::ConstructorAsFunction) {
			//msg_write("FIXME CONSTR AS FUNC");
			// TODO: what is better, giving here an "effective" parameter set, or offseting from the outside?
		}
		return p;
	} else if (link->kind == NodeKind::Class) {
		// should be caught earlier and turned to func...
		const Class *t = link->as_class();
		for (auto *c: t->get_constructors()) {
			mandatory_params = c->num_params;
			return c->literal_param_type;
		}
	/*} else if (link->kind == NodeKind::CALL_RAW_POINTER) {
		return get_callable_param_types(link->params[0]->type);*/
	} else {
		do_error("evil function...kind: "+kind2str(link->kind), link);
	}

	return {};
}

int Concretifier::get_num_wanted_params(shared<Node> link) {
	if (link->is_function()) {
		auto f = link->as_func();
		return f->num_params;
	} else if (link->kind == NodeKind::Class) {
		// should be caught earlier and turned to func...
		const Class *t = link->as_class();
		for (auto *c: t->get_constructors()) {
			return c->num_params;
		}
	} else {
		do_error("evil function...kind: "+kind2str(link->kind), link);
	}
	return 0;
}

// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
shared<Node> Concretifier::check_param_link(shared<Node> link, const Class *wanted, const string &f_name, int param_no, int num_params) {
	// type cast needed and possible?
	const Class *given = link->type;

	if (type_match_up(given, wanted))
		return link;

	CastingDataSingle cast;
	if (type_match_with_cast(link, false, wanted, cast))
		return apply_type_cast(cast, link, wanted);

	parser->Exp.rewind();
	if (num_params > 1)
		do_error(format("command '%s': type '%s' given, '%s' expected for parameter #%d", f_name, given->long_name(), wanted->long_name(), param_no + 1), link);
	else
		do_error(format("command '%s': type '%s' given, '%s' expected", f_name, given->long_name(), wanted->long_name()), link);
	return link;
}

bool Concretifier::direct_param_match(const shared<Node> operand, const shared_array<Node> &params) {
	int mandatory_params;
	auto wanted_types = get_wanted_param_types(operand, mandatory_params);
	if (wanted_types.num != params.num)
		return false;
	for (auto c: wanted_types)
		if (c == TypeDynamic)
			parser->found_dynamic_param = true;
	for (int p=0; p<params.num; p++) {
		if (!type_match_up(params[p]->type, wanted_types[p]))
			return false;
	}
	return true;
}

bool Concretifier::param_match_with_cast(const shared<Node> _operand, const shared_array<Node> &params, CastingDataCall &casts) {

	shared<Node> operand = _operand;

	// template deduction
	if (operand->kind == NodeKind::CallFunction) {
		auto f = operand->as_func();
		if (f->is_template()) {
			if (auto ff = match_template_params(operand, params, true)) {
				casts.func_override = ff->as_func();
				// TODO extra penalty...
				operand->link_no = (int_p)ff->as_func();
			} else {
				return false;
			}
		}
	}


	int mandatory_params;
	casts.wanted = get_wanted_param_types(operand, mandatory_params);
	if (params.num != casts.wanted.num)
		return false;
	casts.params.resize(params.num);
	casts.penalty = 0;
	for (int p=0; p<params.num; p++) {
		if (!type_match_with_cast(params[p], false, casts.wanted[p], casts.params[p]))
			return false;
		casts.penalty = max(casts.penalty, casts.params[p].penalty);
	}
	return true;
}

string Concretifier::param_match_with_cast_error(const shared_array<Node> &params, const Array<const Class*> &wanted) {
	if (wanted.num != params.num)
		return format("%d parameters given, %d expected", params.num, wanted.num);
	for (int p=0; p<params.num; p++) {
		CastingDataSingle cast;
		if (!type_match_with_cast(params[p], false, wanted[p], cast)) {
			auto pt = give_useful_type(this, params[p]);
			if (params.num > 1)
				return format("type '%s' given, '%s' expected for parameter #%d", pt->long_name(), wanted[p]->long_name(), p+1);
			else
				return format("type '%s' given, '%s' expected    %s  %s", pt->long_name(), wanted[p]->long_name(), p2s(pt), p2s(wanted[p]));
		}
	}
	return "";
}

shared<Node> Concretifier::apply_params_direct(shared<Node> operand, const shared_array<Node> &params, int offset) {
	auto r = operand->shallow_copy();
	for (int p=0; p<params.num; p++)
		r->set_param(p+offset, params[p]);
	return r;
}

shared<Node> Concretifier::apply_params_with_cast(shared<Node> operand, const shared_array<Node> &params, const CastingDataCall &casts, int offset) {
	auto r = operand->shallow_copy();

	// function template instantiation
	if (casts.func_override) {
		r->link_no = (int_p)casts.func_override;
		r->type = casts.func_override->literal_return_type;
		r->set_num_params(casts.func_override->num_params);
	}

	// parameter casts
	for (int p=0; p<params.num; p++) {
		auto pp = apply_type_cast(casts.params[p], params[p], casts.wanted[p]);
		r->set_param(p+offset, pp);
	}

	// default values
	if (operand->is_function()) {
		auto f = operand->as_func();
		for (int p=params.num+offset; p<f->num_params; p++) {
			r->set_param(p, f->default_parameters[p]);
		}
	}
	return r;
}


shared<Node> Concretifier::add_converter_str(shared<Node> node, bool as_repr) {
	node = force_concrete_type(node);

	auto *t = node->type;

	// member x.__str__/__repr__()
	Function *cf = nullptr;
	if (as_repr)
		cf = t->get_member_func(Identifier::func::Repr, TypeString, {});
	if (!cf)
		cf = t->get_member_func(Identifier::func::Str, TypeString, {});
	if (cf)
		return add_node_member_call(cf, node, node->token_id);

	// "universal" var2str() or var_repr()
	auto *c = tree->add_constant_pointer(TypeClassRef, t);

	Function *f = tree->required_func_global(as_repr ? "@var_repr" : "@var2str", node->token_id);

	auto cmd = add_node_call(f, node->token_id);
	cmd->set_param(0, node->ref(tree));
	cmd->set_param(1, add_node_const(c));
	return cmd;
}

shared<Node> Concretifier::make_dynamical(shared<Node> node) {
	if (node->kind == NodeKind::ArrayBuilder and node->type == TypeUnknown) {
		for (int i=0; i<node->params.num; i++)
			node->params[i] = make_dynamical(node->params[i].get());
		// TODO create...
		node->type = TypeAnyList;
		//return node;
	} else  if (node->kind == NodeKind::DictBuilder and node->type == TypeUnknown) {
		for (int i=1; i<node->params.num; i+=2)
			node->params[i] = make_dynamical(node->params[i].get());
		// TODO create...
		node->type = TypeAnyDict;
		//return node;
	}
	//node = force_concrete_type(tree, node);

	auto *c = tree->add_constant_pointer(TypeClassRef, node->type);

	Function *f = tree->required_func_global("@dyn", node->token_id);

	auto cmd = add_node_call(f, node->token_id);
	cmd->set_param(0, node->ref(tree));
	cmd->set_param(1, add_node_const(c));
	return cmd;
}

void Concretifier::do_error(const string &str, shared<Node> node) {
	parser->do_error_exp(str, node->token_id);
}

void Concretifier::do_error(const string &str, int token_id) {
	parser->do_error_exp(str, token_id);
}


}
