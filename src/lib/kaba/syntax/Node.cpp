/*
 * Node.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Kaba{

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
	if (kind == NodeKind::FUNCTION_CALL)
		return "call";
	if (kind == NodeKind::POINTER_CALL)
		return "pointer call";
	if (kind == NodeKind::INLINE_CALL)
		return "inline";
	if (kind == NodeKind::VIRTUAL_CALL)
		return "virtual call";
	if (kind == NodeKind::STATEMENT)
		return "statement";
	if (kind == NodeKind::OPERATOR)
		return "operator";
	if (kind == NodeKind::PRIMITIVE_OPERATOR)
		return "PRIMITIVE operator";
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
	if (kind == NodeKind::MARKER)
		return "marker";
	if (kind == NodeKind::DEREF_MARKER)
		return "deref marker";
	if (kind == NodeKind::GLOBAL_LOOKUP)
		return "global lookup";
	if (kind == NodeKind::DEREF_GLOBAL_LOOKUP)
		return "deref global lookup";
	if (kind == NodeKind::IMMEDIATE)
		return "immediate";
	if (kind == NodeKind::DEREF_LOCAL_MEMORY)
		return "deref local";
	return format("UNKNOWN KIND: %d", kind);
}


string Node::sig() const {
	string t = type->name + " ";
	if (kind == NodeKind::PLACEHOLDER)
		return "";
	if (kind == NodeKind::VAR_LOCAL)
		return t + as_local()->name;
	if (kind == NodeKind::VAR_GLOBAL)
		return t + as_global()->name;
	if (kind == NodeKind::FUNCTION)
		return t + as_func()->long_name();
	if (kind == NodeKind::CONSTANT)
		return t + as_const()->str();
	if (kind == NodeKind::FUNCTION_CALL)
		return as_func()->signature();
	if (kind == NodeKind::POINTER_CALL)
		return t + "(...)";
	if (kind == NodeKind::INLINE_CALL)
		return as_func()->signature();
	if (kind == NodeKind::VIRTUAL_CALL)
		return as_func()->signature();
	if (kind == NodeKind::CONSTRUCTOR_AS_FUNCTION)
		return as_func()->signature();
	if (kind == NodeKind::STATEMENT)
		return t + as_statement()->name;
	if (kind == NodeKind::OPERATOR)
		return as_op()->sig();
	if (kind == NodeKind::PRIMITIVE_OPERATOR)
		return as_prim_op()->name;
	if (kind == NodeKind::BLOCK)
		return "";//p2s(as_block());
	if (kind == NodeKind::ADDRESS_SHIFT)
		return t + i2s(link_no);
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
		return t + i2s(link_no);
	if (kind == NodeKind::CLASS)
		return as_class()->name;
	if (kind == NodeKind::REGISTER)
		return t + Asm::get_reg_name(link_no);
	if (kind == NodeKind::ADDRESS)
		return t + d2h(&link_no, config.pointer_size);
	if (kind == NodeKind::MEMORY)
		return t + d2h(&link_no, config.pointer_size);
	if (kind == NodeKind::LOCAL_ADDRESS)
		return t + d2h(&link_no, config.pointer_size);
	if (kind == NodeKind::LOCAL_MEMORY)
		return t + d2h(&link_no, config.pointer_size);
	return t + i2s(link_no);
}

string Node::str() const {
	return "<" + kind2str(kind) + ">  " + sig();
}


void Node::show() const {
	string orig;
	msg_write(str() + orig);
	msg_right();
	for (Node *p: params)
		if (p)
			p->show();
		else
			msg_write("<-NULL->");
	msg_left();
}




Block::Block(Function *f, Block *_parent) :
	Node(NodeKind::BLOCK, (int_p)this, TypeVoid)
{
	level = 0;
	function = f;
	parent = _parent;
	if (parent)
		level = parent->level + 1;
	_start = _end = nullptr;
	_label_start = _label_end = -1;
}

Block::~Block() {
}


inline void set_command(Node *&a, Node *b) {
	a = b;
}

void Block::add(Node *c) {
	if (c)
		params.add(c);
}

void Block::set(int index, Node *c) {
	params[index] = c;
}

Variable *Block::add_var(const string &name, const Class *type) {
	if (get_var(name))
		function->owner()->do_error(format("variable '%s' already declared in this context", name.c_str()));
	Variable *v = new Variable(name, type);
	function->var.add(v);
	vars.add(v);
	return v;
}

Variable *Block::get_var(const string &name) {
	for (auto *v: vars)
		if (v->name == name)
			return v;
	if (parent)
		return parent->get_var(name);
	return nullptr;
}

const Class *Block::name_space() const {
	return function->name_space;
}


Node::Node(NodeKind _kind, int64 _link_no, const Class *_type) {
	type = _type;
	kind = _kind;
	link_no = _link_no;
}

Node::~Node() {
	for (auto &p: params)
		if (p)
			delete p;
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
	return as_func()->address;
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

PrimitiveOperator *Node::as_prim_op() const {
	return (PrimitiveOperator*)link_no;
}

void Node::set_instance(Node *p) {
#ifndef NDEBUG
	if (params.num == 0)
		msg_write("no inst...dfljgkldfjg");
#endif
	set_command(params[0], p);
}

void Node::set_num_params(int n) {
	params.resize(n);
}

void Node::set_param(int index, Node *p) {
#ifndef NDEBUG
	/*if ((index < 0) or (index >= uparams.num)){
		show();
		throw Exception(format("internal: Node.set_param...  %d %d", index, params.num), "", 0);
	}*/
#endif
	set_command(params[index], p);
}


}

