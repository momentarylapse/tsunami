/*
 * Node.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../asm/asm.h"
#include "../../base/iter.h"
#include "../../os/msg.h"
#include <stdio.h>

namespace kaba {

string kind2str(NodeKind kind) {
	if (kind == NodeKind::PLACEHOLDER)
		return "placeholder";
	if (kind == NodeKind::VAR_LOCAL)
		return "local";
	if (kind == NodeKind::VAR_GLOBAL)
		return "global";
	if (kind == NodeKind::FUNCTION)
		return "function name";
	if (kind == NodeKind::CONSTANT)
		return "constant";
	if (kind == NodeKind::CONSTANT_BY_ADDRESS)
		return "constant by addr";
	if (kind == NodeKind::CALL_FUNCTION)
		return "call";
	if (kind == NodeKind::CALL_RAW_POINTER)
		return "raw pointer call";
	if (kind == NodeKind::CALL_INLINE)
		return "inline";
	if (kind == NodeKind::CALL_VIRTUAL)
		return "virtual call";
	if (kind == NodeKind::STATEMENT)
		return "statement";
	if (kind == NodeKind::CALL_SPECIAL_FUNCTION)
		return "special function call";
	if (kind == NodeKind::SPECIAL_FUNCTION_NAME)
		return "special function name";
	if (kind == NodeKind::OPERATOR)
		return "operator";
	if (kind == NodeKind::ABSTRACT_TOKEN)
		return "token";
	if (kind == NodeKind::ABSTRACT_OPERATOR)
		return "abstract operator";
	if (kind == NodeKind::ABSTRACT_ELEMENT)
		return "abstract element";
	if (kind == NodeKind::ABSTRACT_CALL)
		return "abstract call";
	if (kind == NodeKind::ABSTRACT_TYPE_SHARED)
		return "shared";
	if (kind == NodeKind::ABSTRACT_TYPE_OWNED)
		return "owned";
	if (kind == NodeKind::ABSTRACT_TYPE_POINTER)
		return "pointer";
	if (kind == NodeKind::ABSTRACT_TYPE_LIST)
		return "list";
	if (kind == NodeKind::ABSTRACT_TYPE_DICT)
		return "dict";
	if (kind == NodeKind::ABSTRACT_TYPE_OPTIONAL)
		return "optional";
	if (kind == NodeKind::ABSTRACT_TYPE_CALLABLE)
		return "callable type";
	if (kind == NodeKind::ABSTRACT_VAR)
		return "var";
	if (kind == NodeKind::BLOCK)
		return "block";
	if (kind == NodeKind::ADDRESS_SHIFT)
		return "address shift";
	if (kind == NodeKind::ARRAY)
		return "array element";
	if (kind == NodeKind::DYNAMIC_ARRAY)
		return "dynamic array element";
	if (kind == NodeKind::POINTER_AS_ARRAY)
		return "pointer as array element";
	if (kind == NodeKind::REFERENCE)
		return "address operator";
	if (kind == NodeKind::DEREFERENCE)
		return "dereferencing";
	if (kind == NodeKind::DEREF_ADDRESS_SHIFT)
		return "deref address shift";
	if (kind == NodeKind::CLASS)
		return "class";
	if (kind == NodeKind::ARRAY_BUILDER)
		return "array builder";
	if (kind == NodeKind::ARRAY_BUILDER_FOR)
		return "array builder for";
	if (kind == NodeKind::ARRAY_BUILDER_FOR_IF)
		return "array builder for if";
	if (kind == NodeKind::DICT_BUILDER)
		return "dict builder";
	if (kind == NodeKind::TUPLE)
		return "tuple";
	if (kind == NodeKind::TUPLE_EXTRACTION)
		return "tuple extract";
	if (kind == NodeKind::CONSTRUCTOR_AS_FUNCTION)
		return "constructor function";
	if (kind == NodeKind::VAR_TEMP)
		return "temp";
	if (kind == NodeKind::DEREF_VAR_TEMP)
		return "deref temp";
	if (kind == NodeKind::REGISTER)
		return "register";
	if (kind == NodeKind::ADDRESS)
		return "address";
	if (kind == NodeKind::MEMORY)
		return "memory";
	if (kind == NodeKind::LOCAL_ADDRESS)
		return "local address";
	if (kind == NodeKind::LOCAL_MEMORY)
		return "local memory";
	if (kind == NodeKind::DEREF_REGISTER)
		return "deref register";
	if (kind == NodeKind::LABEL)
		return "label";
	if (kind == NodeKind::DEREF_LABEL)
		return "deref label";
	if (kind == NodeKind::GLOBAL_LOOKUP)
		return "global lookup";
	if (kind == NodeKind::DEREF_GLOBAL_LOOKUP)
		return "deref global lookup";
	if (kind == NodeKind::IMMEDIATE)
		return "immediate";
	if (kind == NodeKind::DEREF_LOCAL_MEMORY)
		return "deref local";
	return format("UNKNOWN KIND: %d", (int)kind);
}


string Node::signature(const Class *ns) const {
	//string t = (kind == NodeKind::ABSTRACT_TOKEN) ? " " : type->cname(ns) + " ";
	string t = ": " + type->cname(ns);
	if (kind == NodeKind::PLACEHOLDER)
		return "";
	if (kind == NodeKind::VAR_LOCAL)
		return as_local()->name + t;
	if (kind == NodeKind::VAR_GLOBAL)
		return as_global()->name + t;
	if (kind == NodeKind::FUNCTION)
		return as_func()->cname(ns) + t;
	if (kind == NodeKind::CONSTANT)
		return as_const()->str() + t;
	if (kind == NodeKind::CALL_FUNCTION)
		return as_func()->signature(ns);
	if (kind == NodeKind::CALL_RAW_POINTER)
		return "(...)" + t;
	if (kind == NodeKind::CALL_INLINE)
		return as_func()->signature(ns);
	if (kind == NodeKind::CALL_VIRTUAL)
		return as_func()->signature(ns);
	if (kind == NodeKind::CONSTRUCTOR_AS_FUNCTION)
		return as_func()->signature(ns);
	if (kind == NodeKind::STATEMENT)
		return as_statement()->name + t;
	if (kind == NodeKind::CALL_SPECIAL_FUNCTION)
		return as_special_function()->name + t;
	if (kind == NodeKind::SPECIAL_FUNCTION_NAME)
		return as_special_function()->name + t;
	if (kind == NodeKind::OPERATOR)
		return as_op()->sig(ns);
	if (kind == NodeKind::ABSTRACT_TOKEN)
		return as_token();
	if (kind == NodeKind::ABSTRACT_OPERATOR)
		return as_abstract_op()->name;
	if (kind == NodeKind::BLOCK)
		return "";//p2s(as_block());
	if (kind == NodeKind::ADDRESS_SHIFT)
		return i2s(link_no) + t;
	if (kind == NodeKind::ARRAY)
		return t;
	if (kind == NodeKind::DYNAMIC_ARRAY)
		return t;
	if (kind == NodeKind::POINTER_AS_ARRAY)
		return t;
	if (kind == NodeKind::REFERENCE)
		return t;
	if (kind == NodeKind::DEREFERENCE)
		return t;
	if (kind == NodeKind::DEREF_ADDRESS_SHIFT)
		return i2s(link_no) + t;
	if (kind == NodeKind::CLASS)
		return as_class()->cname(ns);
	if (kind == NodeKind::REGISTER)
		return Asm::get_reg_name((Asm::RegID)link_no) + t;
	if (kind == NodeKind::ADDRESS)
		return i2h(link_no, config.pointer_size) + t;
	if (kind == NodeKind::MEMORY)
		return i2h(link_no, config.pointer_size) + t;
	if (kind == NodeKind::LOCAL_ADDRESS)
		return i2h(link_no, config.pointer_size) + t;
	if (kind == NodeKind::LOCAL_MEMORY)
		return i2h(link_no, config.pointer_size) + t;
	return i2s(link_no) + t;
}

string Node::str(const Class *ns) const {
	return "<" + kind2str(kind) + ">  " + signature(ns);
}


void Node::show(const Class *ns) const {
	string orig;
	msg_write(str(ns) + orig);
	msg_right();
	for (auto p: params)
		if (p)
			p->show(ns);
		else
			msg_write("<NULL>");
	msg_left();
}





// policy:
//  don't change after creation...
//  edit the tree by shallow copy, relink to old parameters
//  relinked params count as "new" Node!
// ...(although, Block are allowed to be edited)
Node::Node(NodeKind _kind, int64 _link_no, const Class *_type, bool _const, int _token_id) {
	type = _type;
	kind = _kind;
	link_no = _link_no;
	is_const = _const;
	token_id = _token_id;
}

Node::~Node() {
}

Node *Node::modifiable() {
	is_const = false;
	return this;
}

Node *Node::make_const() {
	is_const = true;
	return this;
}

bool Node::is_call() const {
	return (kind == NodeKind::CALL_FUNCTION) or (kind == NodeKind::CALL_VIRTUAL) or (kind == NodeKind::CALL_RAW_POINTER);
}

bool Node::is_function() const {
	return (kind == NodeKind::CALL_FUNCTION) or (kind == NodeKind::CALL_VIRTUAL) or (kind == NodeKind::CALL_INLINE) or (kind == NodeKind::CONSTRUCTOR_AS_FUNCTION);
}

Block *Node::as_block() const {
	return (Block*)this;
}

Function *Node::as_func() const {
	return (Function*)link_no;
}

const Class *Node::as_class() const {
	return (const Class*)link_no;
}

Constant *Node::as_const() const {
	return (Constant*)link_no;
}

Operator *Node::as_op() const {
	return (Operator*)link_no;
}
void *Node::as_func_p() const {
	return (void*)as_func()->address;
}

// will be the address at runtime...(not the current location...)
void *Node::as_const_p() const {
	return as_const()->address;
}

void *Node::as_global_p() const {
	return as_global()->memory;
}

Variable *Node::as_global() const {
	return (Variable*)link_no;
}

Variable *Node::as_local() const {
	return (Variable*)link_no;
}

Statement *Node::as_statement() const {
	return (Statement*)link_no;
}

SpecialFunction *Node::as_special_function() const {
	return (SpecialFunction*)link_no;
}

AbstractOperator *Node::as_abstract_op() const {
	return (AbstractOperator*)link_no;
}

string Node::as_token() const {
	return reinterpret_cast<SyntaxTree*>((int_p)link_no)->expressions.get_token(token_id);
}

void Node::set_instance(shared<Node> p) {
#ifndef NDEBUG
	if (params.num == 0)
		msg_write("no inst...dfljgkldfjg");
#endif
	params[0] = p;
	if (this->_pointer_ref_counter > 1) {
		msg_write("iii");
		msg_write(msg_get_trace());
	}
}

void Node::set_type(const Class *t) {
	type = t;
	if (this->_pointer_ref_counter > 1) {
		msg_write("ttt");
		msg_write(msg_get_trace());
	}
}

void Node::set_num_params(int n) {
	params.resize(n);
	if (this->_pointer_ref_counter > 1) {
		msg_write("nnn");
		msg_write(msg_get_trace());
	}
}

void Node::set_param(int index, shared<Node> p) {
#ifndef NDEBUG
	/*if ((index < 0) or (index >= uparams.num)){
		show();
		throw Exception(format("internal: Node.set_param...  %d %d", index, params.num), "", 0);
	}*/
#endif
	params[index] = p;
#if 0
	if (this->_pointer_ref_counter > 1) {
		msg_write("ppp");
		msg_write(msg_get_trace());
	}
#endif
}

shared<Node> Node::shallow_copy() const {
	auto r = new Node(kind, link_no, type, is_const, token_id);
	r->params = params;
	return r;
}

shared<Node> Node::ref(const Class *t) const {
	shared<Node> c = new Node(NodeKind::REFERENCE, 0, t, false, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::ref(SyntaxTree *tree) const {
	return ref(tree->get_pointer(type));
}

shared<Node> Node::deref(const Class *override_type) const {
	if (!override_type)
		override_type = type->param[0];
	shared<Node> c = new Node(NodeKind::DEREFERENCE, 0, override_type, is_const, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::shift(int64 shift, const Class *type, int token_id) const {
	shared<Node> c = new Node(NodeKind::ADDRESS_SHIFT, shift, type, is_const, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::deref_shift(int64 shift, const Class *type, int token_id) const {
	shared<Node> c = new Node(NodeKind::DEREF_ADDRESS_SHIFT, shift, type, is_const, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}


// recursive
shared<Node> cp_node(shared<Node> c, Block *parent_block) {
	shared<Node> cmd;
	if (c->kind == NodeKind::BLOCK) {
		if (!parent_block)
			parent_block = c->as_block()->parent;
		cmd = new Block(c->as_block()->function, parent_block, c->type);
		cmd->as_block()->vars = c->as_block()->vars;
		parent_block = cmd->as_block();
	} else {
		cmd = new Node(c->kind, c->link_no, c->type, c->is_const);
	}
	cmd->token_id = c->token_id;
	cmd->set_num_params(c->params.num);
	for (int i=0;i<c->params.num;i++)
		if (c->params[i])
			cmd->set_param(i, cp_node(c->params[i], parent_block));
	return cmd;
}



shared<Node> add_node_constructor(Function *f, int token_id) {
	auto *dummy = new Node(NodeKind::PLACEHOLDER, 0, f->name_space, false);
	auto n = add_node_member_call(f, dummy, token_id); // temp var added later...
	n->kind = NodeKind::CONSTRUCTOR_AS_FUNCTION;
	n->type = f->name_space;
	return n;
}

shared<Node> add_node_const(Constant *c, int token_id) {
	return new Node(NodeKind::CONSTANT, (int_p)c, c->type.get(), true, token_id);
}

/*shared<Node> add_node_block(Block *b) {
	return new Node(NodeKind::BLOCK, (int_p)b, TypeVoid);
}*/

shared<Node> add_node_statement(StatementID id, int token_id, const Class *type) {
	auto *s = statement_from_id(id);
	auto c = new Node(NodeKind::STATEMENT, (int_p)s, type, false, token_id);
	c->set_num_params(s->num_params);
	return c;
}

shared<Node> add_node_special_function_call(SpecialFunctionID id, int token_id, const Class *type) {
	auto *s = special_function_from_id(id);
	auto c = new Node(NodeKind::CALL_SPECIAL_FUNCTION, (int_p)s, type, false, token_id);
	c->set_num_params(s->max_params);
	return c;
}

shared<Node> add_node_special_function_name(SpecialFunctionID id, int token_id, const Class *type) {
	auto *s = special_function_from_id(id);
	auto c = new Node(NodeKind::SPECIAL_FUNCTION_NAME, (int_p)s, type, false, token_id);
	return c;
}

// virtual call, if func is virtual
shared<Node> add_node_member_call(Function *f, const shared<Node> inst, int token_id, const shared_array<Node> &params, bool force_non_virtual) {
	shared<Node> c;
	if ((f->virtual_index >= 0) and !force_non_virtual) {
		c = new Node(NodeKind::CALL_VIRTUAL, (int_p)f, f->literal_return_type, true, token_id);
	} else {
		c = new Node(NodeKind::CALL_FUNCTION, (int_p)f, f->literal_return_type, true, token_id);
	}
	c->set_num_params(f->num_params);
	c->set_instance(inst);
	for (auto&& [i,p]: enumerate(params))
		c->set_param(i + 1, p);
	return c;
}

// non-member!
shared<Node> add_node_call(Function *f, int token_id) {
	// FIXME: literal_return_type???
	shared<Node> c = new Node(NodeKind::CALL_FUNCTION, (int_p)f, f->literal_return_type, true, token_id);
		c->set_num_params(f->num_params);
	return c;
}

shared<Node> add_node_func_name(Function *f, int token_id) {
	return new Node(NodeKind::FUNCTION, (int_p)f, TypeUnknown, true, token_id);
}

shared<Node> add_node_class(const Class *c, int token_id) {
	return new Node(NodeKind::CLASS, (int_p)c, TypeClassP, true, token_id);
}


shared<Node> add_node_operator(Operator *op, const shared<Node> p1, const shared<Node> p2, int token_id, const Class *override_type) {
	if (!override_type)
		override_type = op->return_type;
	shared<Node> cmd = new Node(NodeKind::OPERATOR, (int_p)op, override_type, true, token_id);
	if (op->abstract->param_flags == 3) {
		cmd->set_num_params(2); // binary
		cmd->set_param(0, p1);
		cmd->set_param(1, p2);
	} else {
		cmd->set_num_params(1); // unary
		cmd->set_param(0, p1);
	}
	return cmd;
}


shared<Node> add_node_local(Variable *v, const Class *type, int token_id) {
	return new Node(NodeKind::VAR_LOCAL, (int_p)v, type, v->is_const(), token_id);
}

shared<Node> add_node_local(Variable *v, int token_id) {
	return new Node(NodeKind::VAR_LOCAL, (int_p)v, v->type, v->is_const(), token_id);
}

shared<Node> add_node_global(Variable *v, int token_id) {
	return new Node(NodeKind::VAR_GLOBAL, (int_p)v, v->type, v->is_const(), token_id);
}

shared<Node> add_node_parray(shared<Node> p, shared<Node> index, const Class *type) {
	shared<Node> cmd_el = new Node(NodeKind::POINTER_AS_ARRAY, 0, type, false, index->token_id);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

shared<Node> add_node_dyn_array(shared<Node> array, shared<Node> index) {
	shared<Node> cmd_el = new Node(NodeKind::DYNAMIC_ARRAY, 0, array->type->get_array_element(), false, index->token_id);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, array);
	cmd_el->set_param(1, index);
	return cmd_el;
}

shared<Node> add_node_array(shared<Node> array, shared<Node> index, const Class *type) {
	if (!type)
		type = array->type->param[0];
	auto *el = new Node(NodeKind::ARRAY, 0, type, false, index->token_id);
	el->set_num_params(2);
	el->set_param(0, array);
	el->set_param(1, index);
	return el;
}

shared<Node> make_constructor_static(shared<Node> n, const string &name) {
	for (auto *f: weak(n->type->functions))
		if (f->name == name) {
			auto nn = add_node_call(f, n->token_id);
			for (int i=0; i<n->params.num-1; i++)
				nn->set_param(i, n->params[i+1]);
			//nn->params = n->params.sub(1,-1);
			return nn;
		}
	return n;
}

extern Array<Operator*> global_operators;

shared<Node> add_node_operator_by_inline(InlineID inline_index, const shared<Node> p1, const shared<Node> p2, int token_id, const Class *override_type) {
	for (auto *op: global_operators)
		if (op->f->inline_no == inline_index)
			return add_node_operator(op, p1, p2, token_id, override_type);

	throw Exception(format("INTERNAL ERROR: operator inline index not found: %d", (int)inline_index), "", -1, -1, nullptr);
	return nullptr;
}



Array<const Class*> node_extract_param_types(const shared<Node> n) {
	Array<const Class*> classes;
	for (auto p: weak(n->params))
		classes.add(p->type);
	return classes;
}

bool node_is_member_function_with_instance(shared<Node> n) {
	if (!n->is_function())
		return false;
	auto *f = n->as_func();
	if (f->is_static())
		return false;
	return n->params.num == 0 or n->params[0];
}

bool is_type_tuple(const shared<Node> n) {
	if (n->kind != NodeKind::TUPLE)
		return false;
	for (auto p: weak(n->params))
		if (p->kind != NodeKind::CLASS)
			return false;
	return true;
}

Array<const Class*> class_tuple_extract_classes(const shared<Node> n) {
	Array<const Class*> classes;
	for (auto p: weak(n->params))
		classes.add(p->as_class());
	return classes;
}

}

