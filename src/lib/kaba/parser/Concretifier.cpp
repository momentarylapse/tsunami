/*
 * Concretifier.cpp
 *
 *  Created on: 23 May 2022
 *      Author: michi
 */

#include "Concretifier.h"
#include "Parser.h"
#include "template.h"
#include "../lib/lib.h"
#include "../../base/set.h"
#include "../../os/msg.h"

namespace kaba {

extern Array<Operator*> global_operators;


bool type_match(const Class *given, const Class *wanted);
bool type_match_with_cast(shared<Node> node, bool is_modifiable, const Class *wanted, CastingData &cd);

const Class *merge_type_tuple_into_product(SyntaxTree *tree, const Array<const Class*> &classes, int token_id);

shared<Node> digest_type(SyntaxTree *tree, shared<Node> n) {
	if (!is_type_tuple(n))
		return n;
	auto classes = class_tuple_extract_classes(n);
	return add_node_class(merge_type_tuple_into_product(tree, classes, n->token_id), n->token_id);
}


const Class *get_user_friendly_type(shared<Node> operand) {
	const Class *type = operand->type;
	bool deref = false;
	bool only_static = false;

	if (operand->kind == NodeKind::CLASS) {
		// referencing class functions
		return operand->as_class();
	} else if (type->is_some_pointer()) {
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
	if (fp->is_pointer())
		return fp->param[0]->param.sub_ref(0, -1); // skip return value
	return fp->param.sub_ref(0, -1); // skip return value
}

const Class *get_callable_return_type(const Class *fp) {
	if (fp->is_pointer())
		return fp->param[0]->param.back();
	return fp->param.back();
}

Array<const Class*> get_callable_capture_types(const Class *fp) {
	if (fp->is_pointer())
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
	if (node->kind == NodeKind::FUNCTION)
		return func_effective_params(node->as_func());
	return get_callable_param_types(node->type);
}

const Class *node_call_return_type(shared<Node> node) {
	if (node->type->is_callable())
		return get_callable_return_type(node->type);
	if (node->kind == NodeKind::FUNCTION)
		return node->as_func()->literal_return_type;
	return get_callable_return_type(node->type);
}


Concretifier::Concretifier(Parser *_parser, SyntaxTree *_tree) {
	parser = _parser;
	tree = _tree;
	auto_implementer = &parser->auto_implementer;
}


extern const Class *TypeIntList;
extern const Class *TypeAnyList;
extern const Class *TypeAnyDict;
extern const Class *TypeDynamicArray;
extern const Class *TypeIntDict;
extern const Class *TypeStringAutoCast;
extern const Class *TypePath;

const int TYPE_CAST_NONE = -1;
const int TYPE_CAST_DEREFERENCE = -2;
const int TYPE_CAST_REFERENCE = -3;
const int TYPE_CAST_OWN_STRING = -10;
const int TYPE_CAST_ABSTRACT_LIST = -20;
const int TYPE_CAST_ABSTRACT_TUPLE = -30;
const int TYPE_CAST_TUPLE_AS_CONSTRUCTOR = -31;
const int TYPE_CAST_FUNCTION_AS_CALLABLE = -32;
const int TYPE_CAST_MAKE_SHARED = -40;
const int TYPE_CAST_MAKE_OWNED = -41;


shared<Node> Concretifier::explicit_cast(shared<Node> node, const Class *wanted) {
	auto type = node->type;
	if (type == wanted)
		return node;

	CastingData cast;
	if (type_match_with_cast(node, false, wanted, cast)) {
		if (cast.cast == TYPE_CAST_NONE) {
			auto c = node->shallow_copy();
			c->type = wanted;
			return c;
		}
		return apply_type_cast(cast, node, wanted);
	}

	// explicit pointer cast
	if (wanted->is_some_pointer() and type->is_some_pointer()) {
		node->type = wanted;
		return node;
	}

	if (wanted == TypeString)
		return add_converter_str(node, false);

	if (type->get_member_func("__" + wanted->name + "__", wanted, {})) {
		auto rrr = turn_class_into_constructor(wanted, {node}, node->token_id);
		if (rrr.num > 0) {
			rrr[0]->set_param(0, node);
			return rrr[0];
		}
	}

	do_error(format("can not cast expression of type '%s' to type '%s'", node->type->long_name(), wanted->long_name()), node);
	return nullptr;
}


bool type_match_tuple_as_contructor(shared<Node> node, Function *f_constructor, int &penalty) {
	if (f_constructor->literal_param_type.num != node->params.num + 1)
		return false;

	penalty = 20;
	foreachi (auto *e, weak(node->params).sub_ref(1), i) {
		CastingData cast;
		if (!type_match_with_cast(e, false, f_constructor->literal_param_type[i], cast))
			return false;
		penalty += cast.penalty;
	}
	return true;
}

const Class *make_effective_class_callable(shared<Node> node) {
	auto f = node->as_func();
	if (f->is_member() and node->params.num > 0 and node->params[0])
		return f->owner()->make_class_callable_fp(f->literal_param_type.sub_ref(1), f->literal_return_type, node->token_id);
	return f->owner()->make_class_callable_fp(f, node->token_id);
}

bool type_match_with_cast(shared<Node> node, bool is_modifiable, const Class *wanted, CastingData &cd) {
	cd.penalty = 0;
	auto given = node->type;
	cd.cast = TYPE_CAST_NONE;
	if (type_match(given, wanted))
		return true;
	if (wanted == TypeStringAutoCast and given == TypeString)
		return true;
	if (wanted == TypeString and given == TypePath)
		return true;
	if (is_modifiable) // is a variable getting assigned.... better not cast
		return false;
	if (given->is_pointer()) {
		if (type_match(given->param[0], wanted)) {
			cd.penalty = 10;
			cd.cast = TYPE_CAST_DEREFERENCE;
			return true;
		}
	}
	if (wanted->is_pointer_shared() and given->is_pointer()) {
		if (type_match(given->param[0], wanted->param[0])) {
			cd.penalty = 10;
			cd.cast = TYPE_CAST_MAKE_SHARED;
			cd.f = wanted->get_func(IDENTIFIER_FUNC_SHARED_CREATE, wanted, {wanted->param[0]->get_pointer()});
			return true;
		}
	}
	/*if (wanted->is_pointer_owned() and given->is_pointer()) {
		if (type_match(given->param[0], wanted->param[0])) {
			penalty = 10;
			cast = TYPE_CAST_MAKE_OWNED;
			return true;
		}
	}*/
	if (node->kind == NodeKind::ARRAY_BUILDER and given == TypeUnknown) {
		if (wanted->is_super_array()) {
			auto t = wanted->get_array_element();
			CastingData cast;
			for (auto *e: weak(node->params)) {
				if (!type_match_with_cast(e, false, t, cast))
					return false;
				cd.penalty += cast.penalty;
			}
			cd.cast = TYPE_CAST_ABSTRACT_LIST;
			return true;
		}
		if (wanted == TypeDynamicArray) {
			cd.cast = TYPE_CAST_ABSTRACT_LIST;
			return true;
		}
		// TODO: only for tuples
		for (auto *f: wanted->get_constructors()) {
			if (type_match_tuple_as_contructor(node, f, cd.penalty)) {
				cd.cast = TYPE_CAST_TUPLE_AS_CONSTRUCTOR;
				cd.f = f;
				return true;
			}
		}
	}
	if ((node->kind == NodeKind::TUPLE) and (given == TypeUnknown)) {

		for (auto *f: wanted->get_constructors()) {
			if (type_match_tuple_as_contructor(node, f, cd.penalty)) {
				cd.cast = TYPE_CAST_TUPLE_AS_CONSTRUCTOR;
				cd.f = f;
				return true;
			}
		}

		// FIXME this probably doesn't make sense... we should already know the wanted type!
		if (!wanted->is_product())
			return false;
		if (wanted->param.num != node->params.num)
			return false;
		for (int i=0; i<node->params.num; i++)
			if (!type_match(node->params[i]->type, wanted->param[i]))
				return false;
		msg_error("product");
		cd.cast = TYPE_CAST_ABSTRACT_TUPLE;
		return true;
	}
	if (wanted->is_callable() and (given == TypeUnknown)) {
		if (node->kind == NodeKind::FUNCTION) {
			auto ft = make_effective_class_callable(node);
			if (type_match(ft, wanted)) {
				cd.cast = TYPE_CAST_FUNCTION_AS_CALLABLE;
				return true;
			}
		}
	}
	if (wanted == TypeStringAutoCast) {
		//Function *cf = given->get_func(IDENTIFIER_FUNC_STR, TypeString, {});
		//if (cf) {
			cd.penalty = 50;
			cd.cast = TYPE_CAST_OWN_STRING;
			return true;
		//}
	}
	foreachi(auto &c, TypeCasts, i)
		if (type_match(given, c.source) and type_match(c.dest, wanted)) {
			cd.penalty = c.penalty;
			cd.cast = i;
			return true;
		}
	return false;
}

shared<Node> Concretifier::apply_type_cast(const CastingData &cast, shared<Node> node, const Class *wanted) {
	if (cast.cast == TYPE_CAST_NONE)
		return node;
	if (cast.cast == TYPE_CAST_DEREFERENCE)
		return node->deref();
	if (cast.cast == TYPE_CAST_REFERENCE)
		return node->ref();
	if (cast.cast == TYPE_CAST_OWN_STRING)
		return add_converter_str(node, false);
	if (cast.cast == TYPE_CAST_ABSTRACT_LIST) {
		if (wanted == TypeDynamicArray)
			return force_concrete_type(node);
		CastingData cd2;
		foreachi (auto e, node->params, i) {
			if (!type_match_with_cast(e, false, wanted->get_array_element(), cd2)) {
				do_error("nope????", node);
			}
			node->params[i] = apply_type_cast(cd2, e, wanted->get_array_element());
		}
		node->type = wanted;
		return node;
	}
	if (cast.cast == TYPE_CAST_ABSTRACT_TUPLE) {
		msg_error("AUTO TUPLE");
		node->type = wanted;
		return node;
	}
	if (cast.cast == TYPE_CAST_FUNCTION_AS_CALLABLE) {
		return force_concrete_type(node);
	}

	if (cast.cast == TYPE_CAST_TUPLE_AS_CONSTRUCTOR) {
		Array<CastingData> c;
		c.resize(node->params.num);
		auto f = cast.f;
		foreachi (auto e, node->params, i)
			if (!type_match_with_cast(e, false, f->literal_param_type[i+1], c[i])) { do_error("aaaaa", e); }
		auto cmd = add_node_constructor(f);
		return apply_params_with_cast(cmd, node->params, c, f->literal_param_type, 1);
	}
	if ((cast.cast == TYPE_CAST_MAKE_SHARED) or (cast.cast == TYPE_CAST_MAKE_OWNED)) {
		if (!cast.f)
			do_error(format("internal: make shared... %s._create() missing...", wanted->name), node);
		auto nn = add_node_call(cast.f, node->token_id);
		nn->set_param(0, node);
		return nn;
	}

	auto c = add_node_call(TypeCasts[cast.cast].f, node->token_id);
	c->type = TypeCasts[cast.cast].dest;
	c->set_param(0, node);
	return c;
}

shared<Node> Concretifier::link_special_operator_is(shared<Node> param1, shared<Node> param2, int token_id) {
	if (param2->kind != NodeKind::CLASS)
		do_error("class name expected after 'is'", param2);
	const Class *t2 = param2->as_class();
	if (t2->vtable.num == 0)
		do_error(format("class after 'is' needs to have virtual functions: '%s'", t2->long_name()), param2);

	const Class *t1 = param1->type;
	if (t1->is_pointer()) {
		param1 = param1->deref();
		t1 = t1->param[0];
	}
	if (!t2->is_derived_from(t1))
		do_error(format("'is': class '%s' is not derived from '%s'", t2->long_name(), t1->long_name()), token_id);

	// vtable2
	auto vtable2 = add_node_const(tree->add_constant_pointer(TypePointer, t2->_vtable_location_compiler_), token_id);

	// vtable1
	param1->type = TypePointer;

	return add_node_operator_by_inline(InlineID::POINTER_EQUAL, param1, vtable2, token_id);
}

shared<Node> Concretifier::link_special_operator_in(shared<Node> param1, shared<Node> param2, int token_id) {
	param2 = force_concrete_type(param2);
	auto *f = param2->type->get_member_func("__contains__", TypeBool, {param1->type});
	if (!f)
		do_error(format("no 'bool %s.__contains__(%s)' found", param2->type->long_name(), param1->type->long_name()), token_id);

	auto n = add_node_member_call(f, param2, token_id);
	n->set_param(1, param1);
	return n;
}

shared<Node> Concretifier::link_special_operator_as(shared<Node> param1, shared<Node> param2, int token_id) {
	if (param2->kind != NodeKind::CLASS)
		do_error("class name expected after 'as'", param2);
	auto wanted = param2->as_class();
	return explicit_cast(param1, wanted);
}

shared<Node> Concretifier::link_operator_id(OperatorID op_no, shared<Node> param1, shared<Node> param2, int token_id) {
	return link_operator(&abstract_operators[(int)op_no], param1, param2, token_id);
}

bool tuple_all_editable(shared<Node> node) {
	for (auto p: weak(node->params))
		if ((p->kind != NodeKind::VAR_LOCAL) and (p->kind != NodeKind::VAR_GLOBAL))
			return false;
	return true;
}

Array<const Class*> tuple_get_element_types(const Class *type) {
	Array<const Class*> r;
	if (type->is_super_array())
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

	auto node = new Node(NodeKind::TUPLE_EXTRACTION, -1, TypeVoid, false, token_id);
	node->set_num_params(etypes.num + 1);
	node->set_param(0, param2);
	for (int i=0; i<etypes.num; i++)
		node->set_param(i+1, param1->params[i]);
	//node->show();
	return node;
}

shared<Node> Concretifier::link_operator(AbstractOperator *primop, shared<Node> param1, shared<Node> param2, int token_id) {
	bool left_modifiable = primop->left_modifiable;
	bool order_inverted = primop->order_inverted;
	string op_func_name = primop->function_name;
	shared<Node> op;

	// tuple extractor?
	if ((primop->id == OperatorID::ASSIGN) and (param1->kind == NodeKind::TUPLE))
		return link_special_operator_tuple_extract(param1, param2, token_id);

	if (primop->left_modifiable and param1->is_const)
		do_error("trying to modify a constant expression", token_id);

	if (primop->id == OperatorID::IS)
		return link_special_operator_is(param1, param2, token_id);
	if (primop->id == OperatorID::IN)
		return link_special_operator_in(param1, param2, token_id);
	if (primop->id == OperatorID::AS)
		return link_special_operator_as(param1, param2, token_id);


	auto *p1 = param1->type;
	auto *p2 = param2->type;

	const Class *pp1 = p1;
	if (pp1->is_pointer())
		pp1 = p1->param[0];

	if (primop->id == OperatorID::ASSIGN) {
		//param1->show();
		if (param1->kind == NodeKind::CALL_FUNCTION) {
			auto f = param1->as_func();
			if (f->name == IDENTIFIER_FUNC_GET) {
				auto inst = param1->params[0];
				auto index = param1->params[1];
				//msg_write(format("[]=...    void %s.__set__(%s, %s)?", inst->type->long_name(), index->type->long_name(), p2->long_name()));
				for (auto *ff: weak(inst->type->functions))
					if (ff->name == IDENTIFIER_FUNC_SET and ff->literal_return_type == TypeVoid and ff->num_params == 3) {
						if (ff->literal_param_type[1] != index->type)
							continue;
						CastingData cast;
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
	for (auto *f: weak(pp1->functions))
		if ((f->name == op_func_name) and f->is_member()) {
			// exact match as class function but missing a "&"?
			if (f->literal_param_type.num != 2)
				continue;

			auto type2 = f->literal_param_type[1];
			if (type_match(p2, type2)) {
				auto inst = param1;
				if (p1 == pp1)
					op = add_node_member_call(f, inst, token_id);
				else
					op = add_node_member_call(f, inst->deref(), token_id);
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
	CastingData c1 = {TYPE_CAST_NONE, 0};
	CastingData c2 = {TYPE_CAST_NONE, 0};
	CastingData c1_best = {TYPE_CAST_NONE, 1000};
	CastingData c2_best = {TYPE_CAST_NONE, 1000};
	const Class *t1_best = nullptr, *t2_best = nullptr;
	Operator *op_found = nullptr;
	Function *op_cf_found = nullptr;
	for (auto *op: global_operators)
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
		if (cf->name == op_func_name)
			if (type_match_with_cast(param2, false, cf->literal_param_type[1], c2))
				if (c2.penalty < c2_best.penalty) {
					op_cf_found = cf;
					c1_best.cast = TYPE_CAST_NONE;
					c2_best = c2;
					t2_best = cf->literal_param_type[1];
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

	return nullptr;
}

void Concretifier::concretify_all_params(shared<Node> &node, Block *block, const Class *ns) {
	for (int p=0; p<node->params.num; p++)
		if (node->params[p]->type == TypeUnknown) {
			node->params[p] = concretify_node(node->params[p], block, ns);
		}
};


shared<Node> Concretifier::concretify_call(shared<Node> node, Block *block, const Class *ns) {
	//concretify_all_params(node, block, ns, this);
	auto links = concretify_node_multi(node->params[0], block, ns);
	for (int p=1; p<node->params.num; p++)
		if (node->params[p]->type == TypeUnknown)
			node->params[p] = concretify_node(node->params[p], block, ns);

	auto params = node->params.sub_ref(1);


	// make links callable
	foreachi (auto l, weak(links), i) {
		if (l->kind == NodeKind::FUNCTION) {
			links[i] = make_func_node_callable(l);
		} else if (l->kind == NodeKind::CLASS) {
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
			//return add_node_member_call(l->type->param[0]->get_call(), l->deref(), params);
		} else {
			do_error("can't call " + kind2str(l->kind), l);
		}
	}
	return try_to_match_apply_params(links, params);
}

shared_array<Node> Concretifier::concretify_element(shared<Node> node, Block *block, const Class *ns) {
	auto base = concretify_node(node->params[0], block, ns);
	int token_id = node->params[1]->token_id;
	auto el = parser->Exp.get_token(token_id);

	base = force_concrete_type(base);
	auto links = tree->get_element_of(base, el, token_id);
	if (links.num > 0)
		return links;

	if (base->kind == NodeKind::CLASS) {
		auto c = add_node_const(tree->add_constant(TypeClassP), node->token_id);
		c->as_const()->as_int64() = (int_p)base->as_class();

		auto links = tree->get_element_of(c, el, token_id);
		if (links.num > 0)
			return links;
	}

	if (base->kind == NodeKind::FUNCTION) {
		msg_write("FFF");
	}

	do_error(format("unknown element of '%s'", get_user_friendly_type(base)->long_name()), node->params[1]);
	return {};
}

shared<Node> Concretifier::concretify_array(shared<Node> node, Block *block, const Class *ns) {
	auto operand = concretify_node(node->params[0], block, ns);
	auto index = concretify_node(node->params[1], block, ns);
	shared<Node> index2;
	if (node->params.num >= 3)
		index2 = concretify_node(node->params[2], block, ns);

	// int[3]
	if (operand->kind == NodeKind::CLASS) {
		// find array index
		index = tree->transform_node(index, [&] (shared<Node> n) {
			return tree->conv_eval_const_func(n);
		});

		if ((index->kind != NodeKind::CONSTANT) or (index->type != TypeInt))
			do_error("only constants of type 'int' allowed for size of arrays", index);
		int array_size = index->as_const()->as_int();
		auto t = tree->make_class_array(operand->as_class(), array_size, operand->token_id);
		return add_node_class(t);

	}

	// min[float]()
	if (operand->kind == NodeKind::FUNCTION) {
		auto links = concretify_node_multi(node->params[0], block, ns);
		if (index->kind != NodeKind::CLASS)
			do_error("functions can only be indexed by a type", index);
		auto t = index->as_class();
		for (auto l: weak(links)) {
			auto f = l->as_func();
			auto ff = TemplateManager::get_instantiated(parser, f, {t}, block, ns, node->token_id);
			if (ff) {
				auto tf = add_node_func_name(ff);
				tf->params = l->params; // in case we have a member instance
				return tf;
			}
		}
		do_error(format("function has no version [%s]", t->name), index);
	}


	// auto deref?
	if (operand->type->is_pointer()) {
		if (!operand->type->param[0]->is_array() and !operand->type->param[0]->usable_as_super_array())
			do_error(format("using pointer type '%s' as an array (like in C) is deprecated", operand->type->long_name()), index);
		operand = operand->deref();
	}


	// subarray() ?
	if (index2) {
		auto *cf = operand->type->get_member_func(IDENTIFIER_FUNC_SUBARRAY, operand->type, {index->type, index->type});
		if (cf) {
			auto f = add_node_member_call(cf, operand, operand->token_id);
			f->is_const = operand->is_const;
			f->set_param(1, index);
			f->set_param(2, index2);
			return f;
		} else {
			do_error(format("function '%s.%s(int,int) -> %s' required by '[a:b]' missing", operand->type->name, IDENTIFIER_FUNC_SUBARRAY, operand->type->name), index);
		}
	}

	// __get__() ?
	auto *cf = operand->type->get_get(index->type);
	if (cf) {
		auto f = add_node_member_call(cf, operand, operand->token_id);
		f->is_const = operand->is_const;
		f->set_param(1, index);
		return f;
	}

	// allowed?
	if (!operand->type->is_array() and !operand->type->usable_as_super_array())
		do_error(format("type '%s' is neither an array nor does it have a function __get__(%s)", operand->type->long_name(), index->type->long_name()), index);


	if (index->type != TypeInt)
		do_error(format("array index needs to be of type 'int', not '%s'", index->type->long_name()), index);

	shared<Node> array_element;

	// pointer?
	if (operand->type->usable_as_super_array()) {
		array_element = add_node_dyn_array(operand, index);
	} else if (operand->type->is_pointer()) {
		array_element = add_node_parray(operand, index, operand->type->param[0]->param[0]);
	} else {
		array_element = add_node_array(operand, index);
	}
	array_element->is_const = operand->is_const;
	return array_element;

}

shared_array<Node> Concretifier::concretify_node_multi(shared<Node> node, Block *block, const Class *ns) {
	if (node->type != TypeUnknown)
		return {node};

	if (node->kind == NodeKind::ABSTRACT_TOKEN) {
		string token = node->as_token();

		// direct operand
		auto operands = tree->get_existence(token, block, ns, node->token_id);
		if (operands.num > 0) {
			// direct operand
			return operands;
		} else {
			auto t = parser->get_constant_type(token);
			if (t == TypeUnknown) {

				msg_write("--------");
				msg_write(block->function->signature());
				msg_write("local vars:");
				for (auto vv: weak(block->function->var))
					msg_write(format("    %s: %s", vv->name, vv->type->name));
				msg_write("params:");
				for (auto p: block->function->literal_param_type)
					msg_write("    " + p->name);
				//crash();
				do_error(format("unknown operand \"%s\"", token), node);
			}

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
	} else if (node->kind == NodeKind::ABSTRACT_ELEMENT) {
		return concretify_element(node, block, ns);
	} else {
		return {concretify_node(node, block, ns)};
	}
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
		node->params[0] = check_param_link(node->params[0], block->function->literal_return_type, IDENTIFIER_RETURN);
	}
	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_if(shared<Node> node, Block *block, const Class *ns) {
	// [COND, TRUE-BLOCK, FALSE-BLOCK]
	concretify_all_params(node, block, ns);
	node->type = TypeVoid;
	//node->set_param(0, check_param_link(node->params[0], TypeBool, IDENTIFIER_IF));
	node->params[0] = check_param_link(node->params[0], TypeBool, IDENTIFIER_IF);
	return node;
}

shared<Node> Concretifier::concretify_statement_while(shared<Node> node, Block *block, const Class *ns) {
	// [COND, BLOCK]
	concretify_all_params(node, block, ns);
	node->type = TypeVoid;
	node->params[0] = check_param_link(node->params[0], TypeBool, IDENTIFIER_WHILE);
	return node;
}

shared<Node> Concretifier::concretify_statement_for_range(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, VALUE0, VALUE1, STEP, BLOCK]

	auto var_name = parser->Exp.get_token(node->params[0]->token_id);
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

shared<Node> Concretifier::concretify_statement_for_array(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, INDEX, ARRAY, BLOCK]

	auto array = force_concrete_type(concretify_node(node->params[2], block, ns));
	array = deref_if_pointer(array);
	if (!array->type->usable_as_super_array() and !array->type->is_array())
		do_error("array or list expected as second parameter in 'for . in .'", array);
	node->params[2] = array;


	// variable...
	auto var_name = parser->Exp.get_token(node->params[0]->token_id);
	auto var_type = array->type->get_array_element();
	auto var = block->add_var(var_name, var_type);
	if (array->is_const)
		flags_set(var->flags, Flags::CONST);
	node->set_param(0, add_node_local(var));

	string index_name = format("-for_index_%d-", for_index_count ++);
	if (node->params[1])
		index_name = parser->Exp.get_token(node->params[1]->token_id);
	auto index = block->add_var(index_name, TypeInt);
	node->set_param(1, add_node_local(index));

	// block
	node->params[3] = concretify_node(node->params[3], block, ns);
	parser->post_process_for(node);

	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_str(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	return add_converter_str(node->params[0], false);
}

shared<Node> Concretifier::concretify_statement_repr(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	return add_converter_str(node->params[0], true);
}

shared<Node> Concretifier::concretify_statement_sizeof(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);

	if (sub->kind == NodeKind::CLASS) {
		return add_node_const(tree->add_constant_int(sub->as_class()->size), node->token_id);
	} else {
		return add_node_const(tree->add_constant_int(sub->type->size), node->token_id);
	}
}

shared<Node> Concretifier::concretify_statement_typeof(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);

	auto c = add_node_const(tree->add_constant(TypeClassP), node->token_id);
	if (sub->kind == NodeKind::CLASS) {
		return add_node_class(sub->as_class(), node->token_id);
	} else {
		return add_node_class(sub->type, node->token_id);
	}
}

shared<Node> Concretifier::concretify_statement_len(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);
	sub = deref_if_pointer(sub);

	// array?
	if (sub->type->is_array())
		return add_node_const(tree->add_constant_int(sub->type->array_length), node->token_id);

	// __length__() function?
	if (auto *f = sub->type->get_member_func(IDENTIFIER_FUNC_LENGTH, TypeInt, {}))
		return add_node_member_call(f, sub, node->token_id);

	// element "int num/length"?
	for (auto &e: sub->type->elements)
		if (e.type == TypeInt and (e.name == "length" or e.name == "num")) {
			return sub->shift(e.offset, e.type, node->token_id);
		}

	// length() function?
	for (auto f: sub->type->functions)
		if ((f->name == "length") and (f->num_params == 1))
			return add_node_member_call(f.get(), sub, node->token_id);


	do_error(format("don't know how to get the length of an object of class '%s'", sub->type->long_name()), node);
	return nullptr;
}

shared<Node> Concretifier::concretify_statement_new(shared<Node> node, Block *block, const Class *ns) {
	auto constr = concretify_node(node->params[0], block, block->name_space());
	if (constr->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION)
		do_error("constructor call expected after 'new'", node->params[0]);
	constr->kind = NodeKind::CALL_FUNCTION;
	constr->type = TypeVoid;
	node->params[0] = constr;

	auto ff = constr->as_func();
	auto tt = ff->name_space;
	//do_error("NEW " + tt->long_name());

	node->type = tt->get_pointer();
	return node;
}

shared<Node> Concretifier::concretify_statement_delete(shared<Node> node, Block *block, const Class *ns) {
	auto p = concretify_node(node->params[0], block, block->name_space());
	if (!p->type->is_pointer())
		do_error("pointer expected after 'del'", node->params[0]);

	// override del operator?
	if (auto f = p->type->param[0]->get_member_func(IDENTIFIER_FUNC_DELETE_OVERRIDE, TypeVoid, {})) {
		auto cmd = add_node_call(f, node->token_id);
		cmd->set_instance(p->deref());
		return cmd;
	}

	// default delete
	node->params[0] = p;
	node->type = TypeVoid;
	return node;
}

shared<Node> Concretifier::concretify_statement_dyn(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	//sub = force_concrete_type(sub); // TODO
	return make_dynamical(sub);
}

shared<Node> Concretifier::concretify_statement_sorted(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns);
	auto array = force_concrete_type(node->params[0]);
	auto crit = force_concrete_type(node->params[1]);

	if (!array->type->is_super_array())
		do_error("sorted(): first parameter must be a list[]", array);
	if (crit->type != TypeString)
		do_error("sorted(): second parameter must be a string", crit);

	Function *f = tree->required_func_global("@sorted", node->token_id);

	auto cmd = add_node_call(f, node->token_id);
	cmd->set_param(0, array);
	cmd->set_param(1, add_node_class(array->type));
	cmd->set_param(2, crit);
	cmd->type = array->type;
	return cmd;
}

shared<Node> Concretifier::concretify_statement_weak(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());

	auto t = sub->type;
	while (true) {
		if (t->is_pointer_shared() or t->is_pointer_owned()) {
			auto tt = t->param[0]->get_pointer();
			return sub->shift(0, tt, node->token_id);
		} else if (t->is_super_array() and t->get_array_element()->is_pointer_shared()) {
			auto tt = tree->make_class_super_array(t->param[0]->param[0]->get_pointer(), node->token_id);
			return sub->shift(0, tt, node->token_id);
		}
		if (t->parent)
			t = t->parent;
		else
			break;
	}
	do_error("weak() expects either a shared pointer, an owned pointer, or a shared pointer array", sub);
	return nullptr;
}

shared<Node> Concretifier::concretify_statement_map(shared<Node> node, Block *block, const Class *ns) {
	auto func = concretify_node(node->params[0], block, block->name_space());
	auto array = concretify_node(node->params[1], block, block->name_space());
	func = force_concrete_type(func);
	array = force_concrete_type(array);


	if (!func->type->is_callable())
		do_error("map(): first parameter must be callable", func);
	if (!array->type->is_super_array())
		do_error("map(): second parameter must be a list[]", array);

	auto p = node_call_effective_params(func);
	auto rt = node_call_return_type(func);
	if (p.num != 1)
		do_error("map(): function must have exactly one parameter", func);
	if (p[0] != array->type->param[0])
		do_error("map(): function parameter does not match list type", array);

	auto *f = tree->required_func_global("@xmap", node->token_id);

	auto cmd = add_node_call(f, node->token_id);
	cmd->set_param(0, func);
	cmd->set_param(1, array);
	cmd->set_param(2, add_node_class(p[0]));
	cmd->set_param(3, add_node_class(p[1]));
	cmd->type = tree->make_class_super_array(rt, node->token_id);
	return cmd;
}

shared<Node> Concretifier::concretify_statement_raw_function_pointer(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	if (sub->kind != NodeKind::FUNCTION)
		do_error("raw_function_pointer() expects a function name", sub);
	auto func = add_node_const(tree->add_constant(TypeFunctionP), node->token_id);
	func->as_const()->as_int64() = (int_p)sub->as_func();

	node->type = TypeFunctionCodeP;
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
			ex_type = digest_type(tree, ex_type);
			auto var_name = parser->Exp.get_token(ex->params[1]->token_id);

			ex->params.resize(1);


			if (ex_type->kind != NodeKind::CLASS)
				do_error("Exception class expected", ex_type);
			auto type = ex_type->as_class();
			if (!type->is_derived_from(TypeException))
				do_error("Exception class expected", ex_type);
			ex->type = type;

			auto *v = ex_block->as_block()->add_var(var_name, type->get_pointer());
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

// inner_callable: (A,B,C,D,E)->R
// captures:       [-,x0,-,-,x1]
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

	auto bind_wrapper_type = tree->make_class_callable_bind(param_types, return_type, capture_types, capture_via_ref, token_id);

	// "new bind(f, x0, x1, ...)"
	for (auto *cf: bind_wrapper_type->get_constructors()) {
		auto cmd_new = add_node_statement(StatementID::NEW);
		auto con = add_node_constructor(cf);
		shared_array<Node> params = {inner_callable.get()};
		for (auto c: weak(captures))
			if (c)
				params.add(c);
		con = concretifier->apply_params_direct(con, params, 1);
		con->kind = NodeKind::CALL_FUNCTION;
		con->type = TypeVoid;

		cmd_new->type = tree->make_class_callable_fp(outer_call_types, return_type, token_id);
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
		flags_clear(f->flags, Flags::STATIC); // allow finding "self.x" via "x"

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
			auto ret = add_node_statement(StatementID::RETURN);
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
	flags_set(f->flags, Flags::STATIC);

	tree->base_class->add_function(tree, f, false, false);

	// find captures
	Set<Variable*> captures;
	auto find_captures = [block, &captures](shared<Node> n) {
		if (n->kind == NodeKind::VAR_LOCAL) {
			auto v = n->as_local();
			for (auto vv: block->function->var)
				if (v == vv)
					captures.add(v);
		}
		return n;
	};
	tree->transform_block(f->block.get(), find_captures);


	// no captures?
	if (captures.num == 0) {
		f->update_parameters_after_parsing();
		return add_node_func_name(f);
	}

	auto explicit_param_types = f->literal_param_type;


	if (config.verbose)
		msg_write("CAPTURES:");

	Array<bool> capture_via_ref;

	auto should_capture_via_ref = [this, node] (Variable *v) {
		if (v->name == IDENTIFIER_SELF)
			return true;
		if (v->type->can_memcpy() or v->type == TypeString)
			return false;
		do_error(format("currently not supported to capture variable '%s' of type '%s'", v->name, v->type->long_name()), node);
		return true;
	};

	// replace captured variables by adding more parameters to f
	for (auto v: captures) {
		if (config.verbose)
			msg_write("  * " + v->name);

		bool via_ref = should_capture_via_ref(v);
		capture_via_ref.add(via_ref);
		auto cap_type = via_ref ? v->type->get_pointer() : v->type;


		auto vvv = f->add_param(v->name, cap_type, Flags::NONE);
		//if (!flags_has(flags, Flags::OUT))
		//flags_set(v->flags, Flags::CONST);



		auto replace_local = [v,vvv,cap_type,via_ref](shared<Node> n) {
			if (n->kind == NodeKind::VAR_LOCAL)
				if (n->as_local() == v) {
					if (via_ref) {
						n->link_no = (int_p)vvv;
						n->type = cap_type;
						return n->deref();
					} else {
						n->link_no = (int_p)vvv;
					}
				}
			return n;
		};
		tree->transform_block(f->block.get(), replace_local);
	}

	f->update_parameters_after_parsing();

	auto create_inner_lambda = wrap_function_into_callable(f, node->token_id);

	shared_array<Node> capture_nodes;
	foreachi (auto &c, captures, i) {
		if (capture_via_ref[i])
			capture_nodes.add(add_node_local(c)->ref());
		else
			capture_nodes.add(add_node_local(c));
	}
	for (auto e: explicit_param_types) {
		capture_nodes.insert(nullptr, 0);
		capture_via_ref.insert(false, 0);
	}

	return create_bind(this, create_inner_lambda, capture_nodes, capture_via_ref);
}

shared<Node> Concretifier::concretify_statement(shared<Node> node, Block *block, const Class *ns) {
	auto s = node->as_statement();
	if (s->id == StatementID::RETURN) {
		return concretify_statement_return(node, block, ns);
	} else if ((s->id == StatementID::IF) or (s->id == StatementID::IF_ELSE)) {
		return concretify_statement_if(node, block, ns);
	} else if (s->id == StatementID::WHILE) {
		return concretify_statement_while(node, block, ns);
	} else if (s->id == StatementID::FOR_RANGE) {
		return concretify_statement_for_range(node, block, ns);
	} else if (s->id == StatementID::FOR_ARRAY) {
		return concretify_statement_for_array(node, block, ns);
	} else if (s->id == StatementID::STR) {
		return concretify_statement_str(node, block, ns);
	} else if (s->id == StatementID::REPR) {
		return concretify_statement_repr(node, block, ns);
	} else if (s->id == StatementID::SIZEOF) {
		return concretify_statement_sizeof(node, block, ns);
	} else if (s->id == StatementID::TYPEOF) {
		return concretify_statement_typeof(node, block, ns);
	} else if (s->id == StatementID::LEN) {
		return concretify_statement_len(node, block, ns);
	} else if (s->id == StatementID::NEW) {
		return concretify_statement_new(node, block, ns);
	} else if (s->id == StatementID::DELETE) {
		return concretify_statement_delete(node, block, ns);
	} else if (s->id == StatementID::DYN) {
		return concretify_statement_dyn(node, block, ns);
	} else if (s->id == StatementID::RAW_FUNCTION_POINTER) {
		return concretify_statement_raw_function_pointer(node, block, ns);
	} else if (s->id == StatementID::WEAK) {
		return concretify_statement_weak(node, block, ns);
	} else if (s->id == StatementID::SORTED) {
		return concretify_statement_sorted(node, block, ns);
	} else if (s->id == StatementID::MAP) {
		return concretify_statement_map(node, block, ns);
	} else if (s->id == StatementID::TRY) {
		return concretify_statement_try(node, block, ns);
	} else if (s->id == StatementID::LAMBDA) {
		return concretify_statement_lambda(node, block, ns);
	} else {
		node->show();
		do_error("INTERNAL: unexpected statement", node);
	}
	return nullptr;
}

shared<Node> Concretifier::concretify_operator(shared<Node> node, Block *block, const Class *ns) {
	auto op_no = node->as_abstract_op();

	if (op_no->id == OperatorID::FUNCTION_PIPE) {
		concretify_all_params(node, block, ns);
		// well... we're abusing that we will always get the FIRST 2 pipe elements!!!
		return build_function_pipe(node->params[0], node->params[1]);
	} else if (op_no->id == OperatorID::MAPS_TO) {
		return build_lambda_new(node->params[0], node->params[1]);
	}
	concretify_all_params(node, block, ns);

	if (node->params.num == 2) {
		// binary operator A+B
		auto param1 = node->params[0];
		auto param2 = force_concrete_type_if_function(node->params[1]);
		auto op = link_operator(op_no, param1, param2, node->token_id);
		if (!op)
			do_error(format("no operator found: '%s %s %s'", param1->type->long_name(), op_no->name, give_useful_type(this, param2)->long_name()), node);
		return op;
	} else {
		return link_unary_operator(op_no, node->params[0], block, node->token_id);
	}
}

shared<Node> Concretifier::concretify_var_declaration(shared<Node> node, Block *block, const Class *ns) {
	const Class *type = nullptr;
	if (node->params[0]) {
		// explicit type
		auto t = digest_type(tree, force_concrete_type(concretify_node(node->params[0], block, ns)));
		if (t->kind != NodeKind::CLASS)
			do_error("variable declaration requires a type", t);
		type = t->as_class();
	} else {
		//assert(node->params[2]);
		auto rhs = force_concrete_type(concretify_node(node->params[2]->params[1], block, ns));
		node->params[2]->params[1] = rhs;
		type = rhs->type;
	}

	if (node->params[1]->kind == NodeKind::TUPLE) {
		auto etypes = tuple_get_element_types(type);
		foreachi (auto t, etypes, i) {
			if (t->needs_constructor() and !t->get_default_constructor())
				do_error(format("declaring a variable of type '%s' requires a constructor but no default constructor exists", t->long_name()), node);
			block->add_var(node->params[1]->params[i]->as_token(), t);
		}
	} else {
		if (type->needs_constructor() and !type->get_default_constructor())
			do_error(format("declaring a variable of type '%s' requires a constructor but no default constructor exists", type->long_name()), node);
		block->add_var(node->params[1]->as_token(), type);
	}

	if (node->params.num == 3)
		return concretify_node(node->params[2], block, ns);
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

	// create an array
	auto type_el = fake_for->params.back()->type;
	auto type_array = tree->make_class_super_array(type_el, node->token_id);
	auto *var = block->add_var(block->function->create_slightly_hidden_name(), type_array);



	// array.add(exp)
	auto *f_add = type_array->get_member_func("add", TypeVoid, {type_el});
	if (!f_add)
		do_error("...add() ???", node);
	auto n_add = add_node_member_call(f_add, add_node_local(var));
	n_add->set_param(1, n_exp);
	n_add->type = TypeUnknown; // mark abstract so n_exp will be concretified

	// add new code to the loop
	Block *b;
	if (n_cmp) {
		auto b_if = new Block(block->function, block, TypeUnknown);
		auto b_add = new Block(block->function, b_if, TypeUnknown);
		b_add->add(n_add);

		auto n_if = add_node_statement(StatementID::IF, node->token_id, TypeUnknown);
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

	auto n = new Node(NodeKind::ARRAY_BUILDER_FOR, 0, type_array);
	n->set_num_params(2);
	n->set_param(0, n_for);
	n->set_param(1, add_node_local(var));
	return n;
}

const Class *make_pointer_shared(SyntaxTree *tree, const Class *parent, int token_id) {
	if (!parent->name_space)
		tree->do_error("shared not allowed for: " + parent->long_name(), token_id); // TODO
	return tree->make_class(parent->name + " " + IDENTIFIER_SHARED, Class::Type::POINTER_SHARED, config.pointer_size, 0, nullptr, {parent}, parent->name_space, token_id);
}

const Class *make_pointer_owned(SyntaxTree *tree, const Class *parent, int token_id) {
	if (!parent->name_space)
		tree->do_error("owned not allowed for: " + parent->long_name(), token_id);
	return tree->make_class(parent->name + " " + IDENTIFIER_OWNED, Class::Type::POINTER_OWNED, config.pointer_size, 0, nullptr, {parent}, parent->name_space, token_id);
}

shared<Node> Concretifier::concretify_node(shared<Node> node, Block *block, const Class *ns) {
	if (node->type != TypeUnknown)
		return node;

	if (node->kind == NodeKind::ABSTRACT_OPERATOR) {
		return concretify_operator(node, block, ns);
	} else if (node->kind == NodeKind::DEREFERENCE) {
		concretify_all_params(node, block, ns);
		auto sub = node->params[0];
		if (!sub->type->is_pointer())
			do_error("only pointers can be dereferenced using '*'", node);
		node->type = sub->type->param[0];
	} else if (node->kind == NodeKind::REFERENCE) {
		concretify_all_params(node, block, ns);
		auto sub = node->params[0];
		node->type = sub->type->get_pointer();
	} else if (node->kind == NodeKind::ABSTRACT_CALL) {
		return concretify_call(node, block, ns);
	} else if (node->kind == NodeKind::ARRAY) {
		return concretify_array(node, block, ns);
	} else if (node->kind == NodeKind::TUPLE) {
		concretify_all_params(node, block, ns);
		return node;
	} else if (node->kind == NodeKind::ARRAY_BUILDER) {
		concretify_all_params(node, block, ns);
		return node;
	} else if (node->kind == NodeKind::DICT_BUILDER) {
		concretify_all_params(node, block, ns);
		for (int p=0; p<node->params.num; p+=2)
			if (node->params[p]->type != TypeString or node->params[p]->kind != NodeKind::CONSTANT)
				do_error("key needs to be a constant string", node->params[p]);
		return node;
	} else if (node->kind == NodeKind::FUNCTION) {
		return node;
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_POINTER) {
		concretify_all_params(node, block, ns);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected before '*'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		return add_node_class(t->get_pointer());
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_LIST) {
		concretify_all_params(node, block, ns);
		auto n = digest_type(tree, node->params[0]);
		if (n->kind != NodeKind::CLASS)
			do_error("type expected before '[]'", n);
		const Class *t = n->as_class();
		t = tree->make_class_super_array(t, node->token_id);
		return add_node_class(t);
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_DICT) {
		concretify_all_params(node, block, ns);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected before '{}'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		t = tree->make_class_dict(t, node->token_id);
		return add_node_class(t);
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_CALLABLE) {
		concretify_all_params(node, block, ns);
		node->params[0] = digest_type(tree, node->params[0]);
		node->params[1] = digest_type(tree, node->params[1]);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected before '->'", node->params[0]);
		if (node->params[1]->kind != NodeKind::CLASS)
			do_error("type expected before '->'", node->params[1]);
		const Class *t0 = node->params[0]->as_class();
		const Class *t1 = node->params[1]->as_class();
		return add_node_class(tree->make_class_callable_fp({t0}, t1, node->token_id));
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_SHARED) {
		concretify_all_params(node, block, ns);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected after 'shared'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		t = make_pointer_shared(tree, t, node->token_id);
		return add_node_class(t);
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_OWNED) {
		concretify_all_params(node, block, ns);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected after 'owned'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		t = make_pointer_owned(tree, t, node->token_id);
		return add_node_class(t);
	} else if ((node->kind == NodeKind::ABSTRACT_TOKEN) or (node->kind == NodeKind::ABSTRACT_ELEMENT)) {
		auto operands = concretify_node_multi(node, block, ns);
		if (operands.num > 1) {
			for (auto o: weak(operands))
				o->show();
			msg_write(format("WARNING: node not unique:  %s  -  line %d", node->as_token(), reinterpret_cast<SyntaxTree*>(node->link_no)->expressions.token_physical_line_no(node->token_id) + 1));
		}
		if (operands.num > 0)
			return operands[0];
	} else if (node->kind == NodeKind::STATEMENT) {
		return concretify_statement(node, block, ns);
	} else if (node->kind == NodeKind::BLOCK) {
		for (int i=0; i<node->params.num; i++)
			node->params[i] = concretify_node(node->params[i], node->as_block(), ns);
		//concretify_all_params(node, node->as_block(), ns, this);
		node->type = TypeVoid;
		for (int i=node->params.num-1; i>=0; i--)
			if (node->params[i]->kind == NodeKind::ABSTRACT_VAR)
				node->params.erase(i);
	} else if (node->kind == NodeKind::ABSTRACT_VAR) {
		return concretify_var_declaration(node, block, ns);
	} else if (node->kind == NodeKind::ARRAY_BUILDER_FOR) {
		return concretify_array_builder_for(node, block, ns);
	} else if (node->kind == NodeKind::NONE) {
	} else if (node->kind == NodeKind::CALL_FUNCTION) {
		concretify_all_params(node, block, ns);
		node->type = node->as_func()->literal_return_type;
	} else {
		node->show();
		do_error("INTERNAL ERROR: unexpected node", node);
	}

	return node;
}


const Class *Concretifier::concretify_as_type(shared<Node> node, Block *block, const Class *ns) {
	auto cc = concretify_node(node, block, ns);
	cc = digest_type(tree, cc);
	if (cc->kind != NodeKind::CLASS) {
		cc->show(TypeVoid);
		do_error("type expected", cc);
	}
	return cc->as_class();
}


const Class *type_more_abstract(const Class *a, const Class *b) {
	if (a == b)
		return a;
	if (a == TypeInt and b == TypeFloat32)
		return TypeFloat32;
	if (a == TypeFloat32 and b == TypeInt)
		return TypeFloat32;
	return nullptr;
}

shared<Node> Concretifier::wrap_node_into_callable(shared<Node> node) {
	if (node->kind != NodeKind::FUNCTION)
		return node;
	auto f = node->as_func();
	auto callable = wrap_function_into_callable(f, node->token_id);
	if (f->is_member() and node->params.num > 0 and node->params[0]) {
		//if (f->literal_param_type.num > 1)
		//	do_error("wrapping member functions with parameters into callables currently not implemented...", node);
		shared_array<Node> captures = {node->params[0]};
		Array<bool> capture_via_ref = {true};
		for (int i=1; i<f->literal_param_type.num; i++) {
			captures.add(nullptr);
			capture_via_ref.add(false);
		}
		auto b = create_bind(this, callable, captures, capture_via_ref);
		return b;
	}
	return callable;
}

// f : (A,B,...)->R  =>  new Callable[](f) : (A,B,...)->R
shared<Node> Concretifier::wrap_function_into_callable(Function *f, int token_id) {
	auto t = tree->make_class_callable_fp(f, token_id);

	for (auto *cf: t->param[0]->get_constructors()) {
		if (cf->num_params == 2) {
			auto cmd = add_node_statement(StatementID::NEW, token_id);
			auto con = add_node_constructor(cf);
			auto fp = tree->add_constant(TypeFunctionP);
			fp->as_int64() = (int_p)f;
			con = apply_params_direct(con, {add_node_const(fp, token_id)}, 1);
			con->kind = NodeKind::CALL_FUNCTION;
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
	if (node->kind == NodeKind::FUNCTION)
		return wrap_node_into_callable(node);
	return node;
}

shared<Node> Concretifier::force_concrete_type(shared<Node> node) {
	if (node->type != TypeUnknown)
		return node;

	if (node->kind == NodeKind::ARRAY_BUILDER) {
		if (node->params.num == 0) {
			node->type = TypeIntList;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[0]->type;
		for (int i=1; i<node->params.num; i++)
			t = type_more_abstract(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract array", node);

		for (int i=0; i<node->params.num; i++) {
			CastingData cast;
			type_match_with_cast(node->params[i].get(), false, t, cast);
			node->params[i] = apply_type_cast(cast, node->params[i].get(), t);
		}

		node->type = tree->make_class_super_array(t, node->token_id);
		return node;
	} else if (node->kind == NodeKind::DICT_BUILDER) {
		if (node->params.num == 0) {
			node->type = TypeIntDict;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[1]->type;
		for (int i=3; i<node->params.num; i+=2)
			t = type_more_abstract(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract dict", node);

		for (int i=1; i<node->params.num; i+=2) {
			CastingData cast;
			type_match_with_cast(node->params[i].get(), false, t, cast);
			node->params[i] = apply_type_cast(cast, node->params[i].get(), t);
		}

		node->type = tree->make_class_dict(t, node->token_id);
		return node;
	} else if (node->kind == NodeKind::TUPLE) {
		auto type = merge_type_tuple_into_product(tree, node_extract_param_types(node), node->token_id);
		auto xx = turn_class_into_constructor(type, node->params, node->token_id);
		return try_to_match_apply_params(xx, node->params);
	} else if (node->kind == NodeKind::FUNCTION) {
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
	r->kind = NodeKind::CALL_FUNCTION;
	r->type = f->literal_return_type;
	r->set_num_params(f->num_params);

	// virtual?
	if (f->virtual_index >= 0)
		r->kind = NodeKind::CALL_VIRTUAL;
	return r;
}

shared<Node> Concretifier::make_func_pointer_node_callable(const shared<Node> l) {
	auto f = l->type->param[0]->get_call();

	shared<Node> c;
	if (f->virtual_index >= 0) {
		c = new Node(NodeKind::CALL_VIRTUAL, (int_p)f, f->literal_return_type, true);
	} else {
		do_error("function pointer call should be virtual???", l);
		c = new Node(NodeKind::CALL_FUNCTION, (int_p)f, f->literal_return_type, true);
	}
	c->set_num_params(f->num_params);
	c->set_instance(l->deref());
	return c;
}

shared<Node> SyntaxTree::make_fake_constructor(const Class *t, const Class *param_type, int token_id) {
	//if ((t == TypeInt) and (param_type == TypeFloat32))
	//	return add_node_call(get_existence("f2i", nullptr, nullptr, false)[0]->as_func());
	if (param_type->is_pointer())
		param_type = param_type->param[0];

	string fname = "__" + t->name + "__";
	auto *cf = param_type->get_member_func(fname, t, {});
	if (!cf)
		do_error(format("illegal fake constructor... requires '%s.%s()'", param_type->long_name(), fname), token_id);
	return add_node_member_call(cf, nullptr, token_id); // temp var added later...

	auto *dummy = new Node(NodeKind::PLACEHOLDER, 0, TypeVoid);
	return add_node_member_call(cf, dummy, token_id); // temp var added later...
}

shared_array<Node> Concretifier::turn_class_into_constructor(const Class *t, const shared_array<Node> &params, int token_id) {
	if (((t == TypeInt) or (t == TypeFloat32) or (t == TypeInt64) or (t == TypeFloat64) or (t == TypeBool) or (t == TypeChar)) and (params.num == 1))
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
	bool ok=false;
	for (auto *_op: global_operators)
		if (po == _op->abstract)
			if ((!_op->param_type_2) and (type_match(p1, _op->param_type_1))) {
				op = _op;
				ok = true;
				break;
			}


	// needs type casting?
	if (!ok) {
		CastingData current;
		CastingData best = {-1, 10000};
		const Class *t_best = nullptr;
		for (auto *_op: global_operators)
			if (po == _op->abstract)
				if ((!_op->param_type_2) and (type_match_with_cast(operand, false, _op->param_type_1, current))) {
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

void Concretifier::concretify_function_header(Function *f) {
	auto block = tree->root_of_all_evil->block.get();

	f->set_return_type(TypeVoid);
	if (f->abstract_return_type) {
		f->set_return_type(concretify_as_type(f->abstract_return_type, block, f->name_space));
	}
	f->literal_param_type.resize(f->abstract_param_types.num);
	foreachi (auto at, weak(f->abstract_param_types), i) {
		auto t = concretify_as_type(at, block, f->name_space);
		auto v = f->var[i];
		v->type = t;
		f->literal_param_type[i] = t;

		// mandatory_params not yet
		if ((i < f->default_parameters.num) and f->default_parameters[i]) {
			f->default_parameters[i] = concretify_node(f->default_parameters[i], block, f->name_space);
			if (f->default_parameters[i]->type != t)
				do_error(format("trying to set a default value of type '%s' for a parameter of type '%s'", f->default_parameters[i]->type->name, t->name), f->default_parameters[i]);
		}
	}
	flags_clear(f->flags, Flags::TEMPLATE);
}

void Concretifier::concretify_function_body(Function *f) {
	concretify_node(f->block.get(), f->block.get(), f->name_space);

	// auto implement destructor?
	if (f->name == IDENTIFIER_FUNC_DELETE)
		auto_implementer->auto_implement_regular_destructor(f, f->name_space);
}

Array<const Class*> Concretifier::type_list_from_nodes(const shared_array<Node> &nn) {
	Array<const Class*> t;
	for (auto &n: nn)
		t.add(force_concrete_type(n)->type);
	return t;
}

shared<Node> check_const_params(SyntaxTree *tree, shared<Node> n) {
	if ((n->kind == NodeKind::CALL_FUNCTION) or (n->kind == NodeKind::CALL_VIRTUAL)) {
		auto f = n->as_func();
		int offset = 0;
		if (f->is_member()) {
			offset = 1;
			if (f->is_selfref()) {
				// const(return) = const(instance)
				n->is_const = n->params[0]->is_const;
			} else if (n->params[0]->is_const and !f->is_const()) {
				//n->show();
				tree->do_error(f->long_name() + ": member function expects a mutable instance, because it is declared without 'const'", n->token_id);
			}
		}
		for (int i=offset; i<f->num_params; i++)
			if (n->params[i]->is_const and !f->var[i]->is_const())
				tree->do_error(format("%s: function parameter %d ('%s') is 'out' and does not accept a constant value", f->long_name(), i+1-offset, f->var[i]->name), n->token_id);
	}
	return n;
}

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
	return "(" + s + ")";
}

shared<Node> Concretifier::try_to_match_apply_params(const shared_array<Node> &links, shared_array<Node> &_params) {

	//force_concrete_types(params);
	auto params = _params;
	if (node_is_member_function_with_instance(links[0])) {
		params.insert(links[0]->params[0], 0);

	}

	// direct match...
	for (shared<Node> operand: links) {
		if (!direct_param_match(operand, params))
			continue;

		return check_const_params(tree, apply_params_direct(operand, params));
	}


	// advanced match...
	Array<CastingData> casts;
	Array<const Class*> wanted;
	int min_penalty = 1000000;
	shared<Node> chosen;
	for (auto operand: links) {
		Array<CastingData> cur_casts;
		Array<const Class*> cur_wanted;
		int cur_penalty;
		if (!param_match_with_cast(operand, params, cur_casts, cur_wanted, &cur_penalty))
			continue;
		if (cur_penalty < min_penalty){
			casts = cur_casts;
			wanted = cur_wanted;
			chosen = operand;
			min_penalty = cur_penalty;
		}
	}

	if (chosen)
		return check_const_params(tree, apply_params_with_cast(chosen, params, casts, wanted));


	// error message

	if (links.num == 0)
		do_error("can not call ...WTF??", -1); //, links[0]);

	if (links.num == 1) {
		param_match_with_cast(links[0], params, casts, wanted, &min_penalty);
		do_error("invalid function parameters: " + param_match_with_cast_error(params, wanted), links[0]);
	}

	string found = type_list_to_str(type_list_from_nodes(params));
	string available;
	for (auto link: links) {
		//auto p = get_wanted_param_types(link);
		//available += format("\n * %s for %s", type_list_to_str(p), link->sig(tree->base_class));
		available += format("\n * %s", link->signature(tree->base_class));
	}
	do_error(format("invalid function parameters: %s given, possible options:%s", found, available), links[0]);
	return shared<Node>();
}

shared<Node> Concretifier::build_function_pipe(const shared<Node> &input, const shared<Node> &func) {

	if (func->kind != NodeKind::FUNCTION)
		do_error("function expected after '|>", func);
	auto f = func->as_func();
	if (f->num_params != 1)
		do_error("function after '|>' needs exactly 1 parameter (including self)", func);
	//if (f->literal_param_type[0] != input->type)
	//	do_error("pipe type mismatch...");

	auto out = add_node_call(f, func->token_id);

	shared_array<Node> inputs;
	inputs.add(input);

	Array<CastingData> casts;
	Array<const Class*> wanted;
	int penalty;
	if (!param_match_with_cast(out, {input}, casts, wanted, &penalty))
		do_error("pipe: " + param_match_with_cast_error({input}, wanted), func);
	return check_const_params(tree, apply_params_with_cast(out, {input}, casts, wanted));
	//auto out = p->add_node_call(f);
	//out->set_param(0, input);
	//return out;
}


shared<Node> Concretifier::build_lambda_new(const shared<Node> &param, const shared<Node> &expression) {
	do_error("abstract lambda not implemented yet", param);
	return nullptr;
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
		if (link->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION) {
			//msg_write("FIXME CONSTR AS FUNC");
			// TODO: what is better, giving here an "effective" parameter set, or offseting from the outside?
		}
		return p;
	} else if (link->kind == NodeKind::CLASS) {
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

// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
shared<Node> Concretifier::check_param_link(shared<Node> link, const Class *wanted, const string &f_name, int param_no, int num_params) {
	// type cast needed and possible?
	const Class *given = link->type;

	if (type_match(given, wanted))
		return link;

	CastingData cast;
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
		if (!type_match(params[p]->type, wanted_types[p]))
			return false;
	}
	return true;
}

bool Concretifier::param_match_with_cast(const shared<Node> operand, const shared_array<Node> &params, Array<CastingData> &casts, Array<const Class*> &wanted, int *max_penalty) {
	int mandatory_params;
	wanted = get_wanted_param_types(operand, mandatory_params);
	if ((params.num < mandatory_params) or (params.num > wanted.num))
		return false;
	casts.resize(params.num);
	*max_penalty = 0;
	for (int p=0; p<params.num; p++) {
		int penalty;
		if (!type_match_with_cast(params[p], false, wanted[p], casts[p]))
			return false;
		*max_penalty = max(*max_penalty, casts[p].penalty);
	}
	return true;
}

string Concretifier::param_match_with_cast_error(const shared_array<Node> &params, const Array<const Class*> &wanted) {
	if (wanted.num != params.num)
		return format("%d parameters given, %d expected", params.num, wanted.num);
	for (int p=0; p<params.num; p++) {
		CastingData cast;
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

shared<Node> Concretifier::apply_params_with_cast(shared<Node> operand, const shared_array<Node> &params, const Array<CastingData> &casts, const Array<const Class*> &wanted, int offset) {
	auto r = operand->shallow_copy();
	for (int p=0; p<params.num; p++) {
		auto pp = apply_type_cast(casts[p], params[p], wanted[p]);
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

shared<Node> Concretifier::deref_if_pointer(shared<Node> node) {
	if (node->type->is_some_pointer())
		return node->deref();
	return node;
}


shared<Node> Concretifier::add_converter_str(shared<Node> sub, bool repr) {
	sub = force_concrete_type(sub);
	// evil shortcut for pointers (carefull with nil!!)
	if (!repr)
		sub = deref_if_pointer(sub);

	auto *t = sub->type;

	Function *cf = nullptr;
	if (repr)
		cf = t->get_member_func(IDENTIFIER_FUNC_REPR, TypeString, {});
	if (!cf)
		cf = t->get_member_func(IDENTIFIER_FUNC_STR, TypeString, {});
	if (cf)
		return add_node_member_call(cf, sub, sub->token_id);

	// "universal" var2str() or var_repr()
	auto *c = tree->add_constant_pointer(TypeClassP, t);

	Function *f = tree->required_func_global(repr ? "@var_repr" : "@var2str", sub->token_id);

	auto cmd = add_node_call(f, sub->token_id);
	cmd->set_param(0, sub->ref());
	cmd->set_param(1, add_node_const(c));
	return cmd;
}

shared<Node> Concretifier::make_dynamical(shared<Node> node) {
	if (node->kind == NodeKind::ARRAY_BUILDER and node->type == TypeUnknown) {
		for (int i=0; i<node->params.num; i++)
			node->params[i] = make_dynamical(node->params[i].get());
		// TODO create...
		node->type = TypeAnyList;
		//return node;
	} else  if (node->kind == NodeKind::DICT_BUILDER and node->type == TypeUnknown) {
		for (int i=1; i<node->params.num; i+=2)
			node->params[i] = make_dynamical(node->params[i].get());
		// TODO create...
		node->type = TypeAnyDict;
		//return node;
	}
	//node = force_concrete_type(tree, node);

	auto *c = tree->add_constant_pointer(TypeClassP, node->type);

	Function *f = tree->required_func_global("@dyn", node->token_id);

	auto cmd = add_node_call(f, node->token_id);
	cmd->set_param(0, node->ref());
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
