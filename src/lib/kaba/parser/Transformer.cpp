/*
 * Transformer.cpp
 *
 *  Created on: 27 Feb 2023
 *      Author: michi
 */

#include "Transformer.h"
#include "Parser.h"
#include "../syntax/Node.h"
#include "../syntax/SyntaxTree.h"
#include "../CompilerConfiguration.h"
#include "../asm/asm.h"
#include <lib/os/msg.h>
#include <lib/base/iter.h>

namespace kaba {


bool is_func(shared<Node> n);
static shared_array<Node> _transform_insert_before_;
int dict_row_size(const Class *t_val);


InlineID __get_pointer_add_int() {
	if (config.target.pointer_size == 8)
		return InlineID::Int64AddInt32;
	return InlineID::Int32Add;
}


bool node_is_executable(shared<Node> n) {
	if ((n->kind == NodeKind::Constant) or (n->kind == NodeKind::VarLocal) or (n->kind == NodeKind::VarGlobal))
		return false;
	if ((n->kind == NodeKind::AddressShift) or (n->kind == NodeKind::Array) or (n->kind == NodeKind::DynamicArray)
			or (n->kind == NodeKind::Reference) or (n->kind == NodeKind::Dereference)
			or (n->kind == NodeKind::DereferenceAddressShift))
		return node_is_executable(n->params[0]);
	return true;
}

Transformer::Transformer(SyntaxTree* t) {
	tree = t;
}

Transformer::~Transformer() = default;


shared<Node> Transformer::conv_break_down_med_level(shared<Node> c) {
	if (c->kind == NodeKind::DynamicArray) {
		return conv_break_down_low_level(
				add_node_parray(
						c->params[0]->change_type(tree->type_ref(c->type, c->token_id)),
						c->params[1], c->type));
	}
	return c;
}

shared<Node> Transformer::conv_break_down_high_level(shared<Node> n, Block *b) {
	if (n->kind == NodeKind::ConstructorAsFunction) {
		if (config.verbose) {
			msg_error("constr func....");
			n->show(tree->base_class);
		}

		// TODO later in serializer!

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type, n->token_id);
		vv->explicitly_constructed = true;
		auto dummy = add_node_local(vv);

		auto ib = add_node_call(n->as_func(), n->token_id);
		ib->params = n->params;
		ib->set_instance(dummy);
		if (config.verbose)
			ib->show(tree->base_class);

		_transform_insert_before_.add(ib);

		return dummy;
	} else if (n->kind == NodeKind::ArrayBuilder) {
		auto *t_el = n->type->get_array_element();
		Function *cf = n->type->get_member_func("add", common_types._void, {t_el});
		if (!cf) {
			msg_error("AAA");
			n->show();
			msg_write(p2s(n->type));
			msg_write(n->type->name);
			tree->do_error(format("[..]: can not find '%s.add(%s)' function???", n->type->long_name(), t_el->long_name()));
		}

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type, n->token_id);
		auto array = add_node_local(vv);

		auto bb = add_node_block(new Block(f, b), common_types._void);
		for (int i=0; i<n->params.num; i++){
			auto cc = add_node_member_call(cf, array, n->token_id);
			cc->set_param(1, n->params[i]);
			bb->add(cc);
		}
		_transform_insert_before_.add(bb);
		return array;
	} else if (n->kind == NodeKind::DictBuilder) {
		auto *t_el = n->type->get_array_element();
		Function *cf = n->type->get_member_func("__set__", common_types._void, {common_types.string, t_el});
		if (!cf)
			tree->do_error(format("[..]: can not find '%s.__set__(string,%s)' function???", n->type->long_name(), t_el->long_name()));

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type, n->token_id);
		auto array = add_node_local(vv);

		auto bb = add_node_block(new Block(f, b), common_types._void);
		for (int i=0; i<n->params.num/2; i++){
			auto cc = add_node_member_call(cf, array, n->token_id);
			cc->set_param(1, n->params[i*2]);
			cc->set_param(2, n->params[i*2+1]);
			bb->add(cc);
		}
		_transform_insert_before_.add(bb);
		return array;
	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::ForRange)) {

		// [VAR, START, STOP, STEP, BLOCK]
		auto var = n->params[0];
		auto val0 = n->params[1];
		auto val1 = n->params[2];
		auto step = n->params[3];
		auto block = n->params[4];


		auto nn = add_node_statement(StatementID::ForDigest);
		nn->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]

		// assign
		nn->set_param(0, add_node_operator_by_inline(InlineID::Int32Assign, var, val0));

		// while(for_var < val1)
		nn->set_param(1, add_node_operator_by_inline(InlineID::Int32Smaller, var, val1));

		nn->set_param(2, block);


		// ...for_var += 1
		shared<Node> cmd_inc;
		if (var->type == common_types.i32) {
			if (step->kind == NodeKind::Constant and step->as_const()->as_int() == 1)
				cmd_inc = add_node_operator_by_inline(InlineID::Int32Increase, var, nullptr);
			else
				cmd_inc = add_node_operator_by_inline(InlineID::Int32AddAssign, var, step);
		} else {
			cmd_inc = add_node_operator_by_inline(InlineID::Float32AddAssign, var, step);
		}
		nn->set_param(3, cmd_inc); // add to loop-block

		return nn;
	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::For)) {

		// [VAR, INDEX, ARRAY, BLOCK]
		auto var = n->params[0];
		auto key = n->params[1];
		auto array = n->params[2];
		auto block = n->params[3];
		auto index = key;


		// array needs execution?
		if (node_is_executable(array)) {
			// -> assign into variable before the loop
			auto *v = b->add_var(b->function->create_slightly_hidden_name(), array->type, n->token_id);

			auto assign = tree->parser->con.link_operator_id(OperatorID::Assign, add_node_local(v), array);
			_transform_insert_before_.add(assign);

			array = add_node_local(v);
		}

		auto nn = add_node_statement(StatementID::ForDigest);
		nn->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]

		if (array->type->is_dict()) {
			static int for_index_count = 0;
			string index_name = format("-for_dict_index_%d-", for_index_count++);
			index = add_node_local(b->add_var(index_name, common_types.i32, n->token_id));
		}


		// 0
		auto val0 = add_node_const(tree->add_constant_int(0));

		// implement
		// for_index = 0
		nn->set_param(0, add_node_operator_by_inline(InlineID::Int32Assign, index, val0));

		shared<Node> val1;
		if (array->type->usable_as_list() or array->type->is_dict()) {
			// array.num
			val1 = array->shift(config.target.pointer_size, common_types.i32, array->token_id);
		} else {
			// array.size
			val1 = add_node_const(tree->add_constant_int(array->type->array_length));
		}

		// while(for_index < val1)
		nn->set_param(1, add_node_operator_by_inline(InlineID::Int32Smaller, index, val1));

		// ...block
		nn->set_param(2, block);

		// ...for_index += 1
		nn->set_param(3, add_node_operator_by_inline(InlineID::Int32Increase, index, nullptr));

		if (array->type->is_dict()) {

			auto row = add_node_operator_by_inline(__get_pointer_add_int(),
					array->change_type(common_types.reference),
					add_node_operator_by_inline(InlineID::Int32Multiply,
							index,
							add_node_const(tree->add_constant_int(dict_row_size(array->type->param[0])))),
					n->token_id,
					common_types.reference)->deref();


			// &for_var = &row.value
			auto cmd_var_assign = add_node_operator_by_inline(InlineID::PointerAssign, var, row->shift(common_types.string->size, array->type->param[0])->ref(tree));
			block->params.insert(cmd_var_assign, 0);

			// &for_var = &row.value
			auto cmd_key_assign = add_node_operator_by_inline(InlineID::PointerAssign, key, row->change_type(common_types.string)->ref(tree));
			block->params.insert(cmd_key_assign, 0);
		} else {

			// array[index]
			shared<Node> el;
			if (array->type->usable_as_list()) {
				el = add_node_dyn_array(array, index);
			} else {
				el = add_node_array(array, index);
			}

			// &for_var = &array[index]
			auto cmd_var_assign = add_node_operator_by_inline(InlineID::PointerAssign, var, el->ref(tree));
			block->params.insert(cmd_var_assign, 0);
		}

		return nn;
	} else if (n->kind == NodeKind::ArrayBuilderFor) {

		_transform_insert_before_.add(n->params[0]);
		return n->params[1];
	} else if (n->kind == NodeKind::TupleExtraction) {

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->params[0]->type, n->token_id);
		auto temp = add_node_local(vv);

		auto bb = add_node_block(new Block(f, b), common_types._void);

		// tuple assign -> temp
		Function *cf = n->params[0]->type->get_assign();
		if (!cf)
			tree->do_error(format("tuple-extract: don't know how to assign tuple of type '%s'", n->params[0]->type->long_name()), n->token_id);
		auto assign = add_node_member_call(cf, temp, n->token_id, {n->params[0]});
		bb->add(assign);

		for (int i=1; i<n->params.num; i++) {
			// element assign

			Function *ecf = n->params[i]->type->get_assign();
			if (!ecf)
				tree->do_error(format("tuple-extract: don't know how to assign tuple element %d of type '%s'", i, n->params[i]->type->long_name()), n->token_id);
			auto &e = n->params[0]->type->elements[i-1];
			auto assign = add_node_member_call(ecf, n->params[i], n->token_id, {temp->shift(e.offset, e.type, n->token_id)});
			bb->add(assign);
		}

		//bb->show();
		return bb;

	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::Match)) {

		auto *vv = b->add_var(b->function->create_slightly_hidden_name(), n->params[0]->type, n->token_id);
		auto temp = add_node_local(vv);

		auto bb = add_node_block(new Block(b->function, b), common_types._void);
		bb->type = n->type;
		auto& ai = tree->parser->auto_implementer;
		bb->add(ai.add_assign(b->function, "", temp, n->params[0]));

		const int ncases = (n->params.num - 1) / 2;
		shared<Node> cmd;
		for (int i=ncases-1; i>=0; i--) {
			const bool is_last = (i == ncases - 1);
			if (is_last and n->params[1 + 2*i]->kind == NodeKind::Statement) {
				// default?
				cmd = n->params[2 + 2*i];
				continue;
			}
			auto cmd_if = add_node_statement(StatementID::If, n->token_id, n->type);
			cmd_if->set_num_params(is_last ? 2 : 3);
			cmd_if->set_param(0, ai.add_equal(b->function, "", temp, n->params[1 + 2*i]));
			cmd_if->set_param(1, n->params[2 + 2*i]);
			// BLOCKS!!!!!
			if (!is_last)
				cmd_if->set_param(2, cmd);
			cmd = cmd_if;
		}
		bb->add(cmd);

		return bb;

	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::RawFunctionPointer)) {
		// only extract explicit raw_function_pointer()
		// skip implicit from callable...
		if (n->params[0]->kind == NodeKind::Constant) {
			n->params[0]->as_const()->type = common_types.function_code_ref;
			return n->params[0];
		}
	}

	// TODO experimental dynamic type insertion
	if (false and is_func(n)) {
		auto f = n->as_func();
		for (int i=0; i<f->num_params; i++)
			if (f->literal_param_type[i] == common_types.dynamic) {
				msg_error("conv dyn!");
				auto c = tree->add_constant(common_types.class_ref, n->token_id);
				c->as_int64() = (int64)(int_p)n->params[i]->type;
				n->params.insert(add_node_const(c), i+1);
				n->show(tree->base_class);
			}
	}
	return n;
}

shared<Node> Transformer::conv_cbr(shared<Node> c, Variable *var) {
	// convert
	if ((c->kind == NodeKind::VarLocal) and (c->as_local() == var)) {
		auto r = add_node_local(var);
		r->set_type(tree->type_ref(c->type, c->token_id));
		return r->deref();
	}
	return c;
}

#if 0
void conv_return(SyntaxTree *ps, nodes *c) {
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->params[i]);

	if ((c->kind == NodeKind::STATEMENT) and (c->link_no == COMMAND_RETURN)) {
		msg_write("conv ret");
		ref_command_old(ps, c);
	}
}
#endif


shared<Node> Transformer::conv_calls(shared<Node> c) {
	if ((c->kind == NodeKind::Statement) and (c->as_statement()->id == StatementID::Return))
		if (c->params.num > 0) {
			if ((c->params[0]->type->is_array()) /*or (c->Param[j]->Type->IsSuperArray)*/) {
				c->set_param(0, c->params[0]->ref(tree));
			}
			return c;
		}

	if (c->is_call() or (c->kind == NodeKind::ConstructorAsFunction)) {
		auto r = c->shallow_copy();
		bool changed = false;

		auto needs_conversion = [&c] (int j) {
			if (c->params[j]->type->uses_call_by_reference())
				return true;
			if (c->is_function()) {
				auto f = c->as_func();
				int param_offset = 0;//f->is_static() ? 0 : 1;
				// TODO does g++ keep const refs to instance?!
				if ((j >= param_offset) and (flags_has(f->var[j]->flags, Flags::Out)))
					return true;
			}
			return false;
		};

		// parameters, instance: class as reference
		for (int j=0;j<c->params.num;j++)
			if (c->params[j] and needs_conversion(j)) {
				r->set_param(j, c->params[j]->ref(tree));
				changed = true;
			}

		// return: array reference (-> dereference)
		if (c->type->is_array() /*or c->type->is_super_array()*/) {
			r->set_type(tree->type_ref(c->type, c->token_id));
			return r->deref();
		}
		if (changed)
			return r;
	}
	return c;
}


// remove &*x
shared<Node> Transformer::conv_easyfy_ref_deref(shared<Node> c, int l) {
	if (c->kind == NodeKind::Reference) {
		if (c->params[0]->kind == NodeKind::Dereference) {
			// remove 2 knots...
			return c->params[0]->params[0];
		}
	}
	return c;
}

// remove (*x)[] and (*x).y
shared<Node> Transformer::conv_easyfy_shift_deref(shared<Node> c, int l) {
	if ((c->kind == NodeKind::AddressShift) or (c->kind == NodeKind::Array)) {
		if (c->params[0]->kind == NodeKind::Dereference) {
			// unify 2 knots (remove 1)
			c->show(common_types._void);
			auto kind = (c->kind == NodeKind::AddressShift) ? NodeKind::DereferenceAddressShift : NodeKind::PointerAsArray;
			auto r = new Node(kind, 0, c->type, c->flags);
			r->set_param(0, c->params[0]->params[0]);
			r->set_param(1, c->params[1]);
			r->show(common_types._void);
			return r;

			/*auto t = c->params[0]->params[0];
			c->kind = (c->kind == NodeKind::ADDRESS_SHIFT) ? NodeKind::DEREF_ADDRESS_SHIFT : NodeKind::POINTER_AS_ARRAY;
			c->set_param(0, t);
			return c;*/
		}
	}

	return c;
}


shared<Node> Transformer::conv_return_by_memory(shared<Node> n, Function *f) {
	tree->parser->cur_func = f;

	if ((n->kind != NodeKind::Statement) or (n->as_statement()->id != StatementID::Return))
		return n;

	// convert into   *-return- = param
	shared<Node> p_ret;
	for (Variable *v: weak(f->var))
		if (v->name == Identifier::ReturnVar) {
			p_ret = add_node_local(v);
		}
	if (!p_ret)
		tree->do_error("-return- not found...");
	auto ret = p_ret->deref();
	auto cmd_assign = tree->parser->con.link_operator_id(OperatorID::Assign, ret, n->params[0]);
	if (!cmd_assign)
		tree->do_error(format("no operator '%s = %s' for return from function found: '%s'", ret->type->long_name(), n->params[0]->type->long_name(), f->long_name()));
	_transform_insert_before_.add(cmd_assign);

	return add_node_statement(StatementID::Return);
}


// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void Transformer::convert_call_by_reference() {
	if (config.verbose)
		msg_write("ConvertCallByReference");
	// convert functions
	for (Function *f: tree->functions) {
		if (f->literal_return_type == common_types.unknown)
			continue;

		// parameter: array/class as reference
		for (auto v: weak(f->var).sub_ref(0, f->num_params))
			if (v->type->uses_call_by_reference() or flags_has(v->flags, Flags::Out)) {
				v->type = tree->type_ref(v->type, -1);

				// usage inside the function
				transform_block(f->block_node.get(), [this, v](shared<Node> n) {
					return conv_cbr(n, v);
				});
			}
	}

	// convert return...
	for (Function *f: tree->functions)
		if (f->literal_return_type->uses_return_by_memory() and (f->literal_return_type != common_types.unknown))
			//convert_return_by_memory(this, f->block, f);
			transform_block(f->block_node.get(), [this, f](shared<Node> n) {
				return conv_return_by_memory(n, f);
			});

	// convert function calls
	transform(tree, [this](shared<Node> n) {
		return conv_calls(n);
	});
}


void Transformer::simplify_ref_deref() {
	// remove &*
	transform(tree, [this] (shared<Node> n) {
		return conv_easyfy_ref_deref(n, 0);
	});
}

void Transformer::simplify_shift_deref() {
	// remove &*
	transform(tree, [this] (shared<Node> n) {
		return conv_easyfy_shift_deref(n, 0);
	});
}

shared<Node> Transformer::conv_break_down_low_level(shared<Node> c) {

	if (c->kind == NodeKind::Array) {

		auto *el_type = c->type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0]->ref(tree), // array
				add_node_operator_by_inline(InlineID::Int32Multiply,
						c->params[1], // ref
						add_node_const(tree->add_constant_int(el_type->size))),
				c->token_id,
				tree->type_ref(el_type, c->token_id))->deref();

	} else if (c->kind == NodeKind::PointerAsArray) {

		auto *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0], // ref array
				add_node_operator_by_inline(InlineID::Int32Multiply,
						c->params[1], // index
						add_node_const(tree->add_constant_int(el_type->size))),
				c->token_id,
				tree->type_ref(el_type, c->token_id))->deref();
	} else if (c->kind == NodeKind::AddressShift) {

		auto *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

		//if (c->link_no == 0)
		//	return c->params[0]->ref(this)->deref(el_type);
		// FIXME this causes a bug, probably by omtimizing away *(&x) and changing types

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0]->ref(tree), // struct
				add_node_const(tree->add_constant_int(c->link_no)),
				c->token_id,
				tree->type_ref(el_type, c->token_id))->deref();

	} else if (c->kind == NodeKind::DereferenceAddressShift) {

		auto *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		//if (c->link_no == 0)
		//	return c->params[0]->deref(el_type);

		// create command for shift constant
		// address = &struct + shift
		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0], // ref struct
				add_node_const(tree->add_constant_int(c->link_no)),
				c->token_id,
				tree->type_ref(el_type, c->token_id))->deref();
	}
	return c;
}

shared<Node> Transformer::transform_node(shared<Node> n, std::function<shared<Node>(shared<Node>)> F) {
	if (n->kind == NodeKind::Block) {
		transform_block(n.get(), F);
		return F(n);
	} else {
		shared<Node> r = n;
		for (int i=0; i<n->params.num; i++)
			if (n->params[i]) {
				auto rr = transform_node(n->params[i], F);
				if (rr != n->params[i].get()) {
					if (r.get() == n.get())
						r = n->shallow_copy();
					r->set_param(i, rr);
				}
			}
		return F(r);
	}
}

shared<Node> Transformer::transformb_node(shared<Node> n, Block *b, std::function<shared<Node>(shared<Node>, Block*)> F) {
	if (n->kind == NodeKind::Block) {
		transformb_block(n.get(), F);
		return F(n, b);
	} else {
		shared<Node> r = n;
		for (int i=0; i<n->params.num; i++)
			if (n->params[i]) {
				auto rr = transformb_node(n->params[i], b, F);
				if (rr != n->params[i].get()) {
					if (r.get() == n.get())
						r = n->shallow_copy();
					r->set_param(i, rr);
				}
			}
		return F(r, b);
	}
}

// preventing a sub-block to handle insertions of an outer block
#define PUSH_BLOCK_INSERT \
	auto XXX = _transform_insert_before_; \
	_transform_insert_before_.clear();
#define POP_BLOCK_INSERT \
	_transform_insert_before_ = XXX;

void handle_insert_before(Node *block, int &i) {
	if (_transform_insert_before_.num > 0) {
		for (auto *ib: weak(_transform_insert_before_)) {
			if (config.verbose)
				msg_error("INSERT BEFORE...2");
			block->params.insert(ib, i);
			i ++;
		}
		_transform_insert_before_.clear();
	}
}


void Transformer::transform_block(Node *block, std::function<shared<Node>(shared<Node>)> F) {
	PUSH_BLOCK_INSERT;
	for (int i=0; i<block->params.num; i++) {
		block->params[i] = transform_node(block->params[i], F);
		handle_insert_before(block, i);
	}
	POP_BLOCK_INSERT;
}

void Transformer::transformb_block(Node *block, std::function<shared<Node>(shared<Node>, Block*)> F) {
	PUSH_BLOCK_INSERT;
	for (int i=0; i<block->params.num; i++) {
		block->params[i] = transformb_node(block->params[i], block->as_block(), F);
		handle_insert_before(block, i);
	}
	POP_BLOCK_INSERT;
}

// split arrays and address shifts into simpler commands...
void Transformer::transform(SyntaxTree* tree, std::function<shared<Node>(shared<Node>)> F) {
	for (Function *f: tree->functions)
		if (!f->is_template() and !f->is_macro()) {
			tree->parser->cur_func = f;
			transform_block(f->block_node.get(), F);
		}
}
void Transformer::transformb(SyntaxTree* tree, std::function<shared<Node>(shared<Node>, Block*)> F) {
	for (Function *f: tree->functions)
		if (!f->is_template() and !f->is_macro()) {
			tree->parser->cur_func = f;
			transformb_block(f->block_node.get(), F);
		}
}

shared<Node> Transformer::conv_fake_constructors(shared<Node> n) {
	if (n->kind != NodeKind::ConstructorAsFunction)
		return n;
	if ((n->type == common_types.vec3) or (n->type == common_types.vec2) or (n->type == common_types.color) or (n->type == common_types.rect) or (n->type == common_types.complex)) {
		return make_constructor_static(n, "_create");
	}
	if (n->type == common_types.quaternion) {
		if (n->params.num == 2 and n->params[1]->type == common_types.vec3)
			return make_constructor_static(n, "_rotation_v");
		if (n->params.num == 3 and n->params[1]->type == common_types.vec3)
			return make_constructor_static(n, "_rotation_a");
		if (n->params.num == 2 and n->params[1]->type == common_types.mat4)
			return make_constructor_static(n, "_rotation_m");
	}
	return n;
}

shared<Node> Transformer::conv_class_and_func_to_const(shared<Node> n) {
	if (n->kind == NodeKind::Class) {
		return add_node_const(tree->add_constant_pointer(common_types.class_ref, n->as_class()));
	}
	if (n->kind == NodeKind::SpecialFunctionName) {
		return add_node_const(tree->add_constant_pointer(common_types.special_function_ref, n->as_special_function()));
	}
	return n;
}


shared<Node> Transformer::conv_func_inline(shared<Node> n) {
	if (n->kind == NodeKind::CallFunction) {
		if (n->as_func()->inline_no != InlineID::None) {
			auto r = new Node(NodeKind::CallInline, n->link_no, n->type, n->flags);
			r->params = n->params;
			return r;
		}
	}
	if (n->kind == NodeKind::Operator) {
		Operator *op = n->as_op();
		if (op->f->inline_no != InlineID::None) {
			auto r = new Node(NodeKind::CallInline, (int_p)op->f, n->type, n->flags);
			r->params = n->params;
			return r;
		} else {
			auto r = new Node(NodeKind::CallFunction, (int_p)op->f, n->type, n->flags);
			r->params = n->params;
			return r;
		}
	}
	return n;
}

void Transformer::fully_transform() {
	if (config.verbose)
		tree->show("digest:pre");

	// turn vector(x,y,z) into vector._create(x,y,z)
	// TODO make more universal! maybe general __create__() function as fake constructor?
	transform(tree, [this] (shared<Node> n) {
		return conv_fake_constructors(n);
	});

	transform(tree, [this] (shared<Node> n) {
		return conv_class_and_func_to_const(n);
	});

	if (config.allow_simplify_consts)
		eval_const_expressions(true);

	transformb(tree, [this] (shared<Node> n, Block* b) {
		return conv_break_down_high_level(n, b);
	});

	if (config.verbose)
		tree->show("digest:break-high");


	transform(tree, [this] (shared<Node> n) {
		return conv_break_down_med_level(n);
	});
	transform(tree, [this] (shared<Node> n) {
		return conv_break_down_low_level(n);
	});
	if (config.verbose)
		tree->show("digest:break-low");

	simplify_shift_deref();
	simplify_ref_deref();

	if (config.allow_simplify_consts)
		eval_const_expressions(true);

	if (config.verbose)
		tree->show("digest:pre-proc");

	transform(tree, [this] (shared<Node> n) {
		return conv_func_inline(n);
	});
	if (config.verbose)
		tree->show("digest:inline");

	convert_call_by_reference();
	if (config.verbose)
		tree->show("digest:call-by-ref");

	map_local_variables_to_stack();
	if (config.verbose)
		tree->show("digest:map");

	simplify_ref_deref();
	simplify_shift_deref();
	if (config.verbose)
		tree->show("digest:deref");

	if (config.allow_simplify_consts)
		eval_const_expressions(false);
	if (config.verbose)
		tree->show("digest:map");


	//pre_processor_addresses();
}

void MapLVSX86Return(Function *f, int64 &stack_offset) {
	for (auto &v: f->var)
		if (v->name == Identifier::ReturnVar) {
			v->_offset = stack_offset;
			stack_offset += config.target.pointer_size;
		}
}

void MapLVSX86Self(Function *f, int64 &stack_offset) {
	for (auto &v: f->var)
		if (v->name == Identifier::Self) {
			v->_offset = stack_offset;
			stack_offset += config.target.pointer_size;
		}
}

void Transformer::map_local_variables_to_stack() {
	for (Function *f: tree->functions) {

		if (config.target.instruction_set == Asm::InstructionSet::X86) {
			f->_var_size = 0;
			int64 stack_offset = 2 * config.target.pointer_size; // space for eIP and eBP
			// offsets to stack pointer (for push parameters)

			if (config.target.abi == Abi::X86_WINDOWS) {
				// map "self" to the VERY first parameter
				if (f->is_member())
					MapLVSX86Self(f, stack_offset);

				// map "-return-" to the first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);
			} else {
				// map "-return-" to the VERY first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);

				// map "self" to the first parameter
				if (f->is_member())
					MapLVSX86Self(f, stack_offset);
			}

			for (auto&& [i,v]: enumerate(weak(f->var))) {
				if (f->is_member() and (v->name == Identifier::Self))
					continue;
				if (v->name == Identifier::ReturnVar)
					continue;
				int s = mem_align(v->type->size, 4);
				if (i < f->num_params) {
					// parameters
					v->_offset = stack_offset;
					stack_offset += s;
				} else {
					// "real" local variables
					v->_offset = - f->_var_size - s;
					f->_var_size += s;
				}
			}
		} else if (config.target.instruction_set == Asm::InstructionSet::AMD64) {
			f->_var_size = 0;
			int64 stack_offset = 2 * config.target.pointer_size; // space for rIP and rBP
			// offsets to stack pointer (for push parameters)

			if (config.target.abi == Abi::AMD64_WINDOWS) {

				// map "self" to the VERY first parameter
				if (f->is_member())
					MapLVSX86Self(f, stack_offset);

				// map "-return-" to the first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);

				for (auto&& [i,v]: enumerate(weak(f->var))) {
					if (f->is_member() and (v->name == Identifier::Self))
						continue;
					if (v->name == Identifier::ReturnVar)
						continue;
					if (i < f->num_params) {
						// parameters
						int s = 8;
						v->_offset = stack_offset;
						stack_offset += s;
					} else {
						// "real" local variables
						int64 s = mem_align(v->type->size, 4);
						f->_var_size += s;
						v->_offset = -f->_var_size;
					}
				}
			} else {
				// TODO map push parameters...
				for (auto v: weak(f->var)) {
					int64 s = mem_align(v->type->size, 4);
					f->_var_size += s;
					v->_offset = -f->_var_size;
				}
			}
		} else if (config.target.is_arm()) {
			f->_var_size = 0;

			for (auto v: weak(f->var)) {
				int align = (v->type->size > 4) ? 8 : 4;
				int s = mem_align((int)v->type->size, align);
				v->_offset = mem_align(f->_var_size, align);
				f->_var_size = v->_offset + s;
			}
		}
	}
}

}
