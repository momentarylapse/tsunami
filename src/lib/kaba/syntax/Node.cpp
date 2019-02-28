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

extern bool next_extern;

string kind2str(int kind)
{
	if (kind == KIND_VAR_LOCAL)			return "local";
	if (kind == KIND_VAR_GLOBAL)			return "global";
	if (kind == KIND_FUNCTION_NAME)		return "function name";
	if (kind == KIND_FUNCTION_POINTER)		return "function pointer";
	if (kind == KIND_CONSTANT)			return "constant";
	if (kind == KIND_REF_TO_CONST)			return "reference to const";
	if (kind == KIND_FUNCTION_CALL)			return "call";
	if (kind == KIND_POINTER_CALL)			return "pointer call";
	if (kind == KIND_INLINE_CALL)			return "inline";
	if (kind == KIND_VIRTUAL_CALL)	return "virtual call";
	if (kind == KIND_STATEMENT)			return "statement";
	if (kind == KIND_OPERATOR)			return "operator";
	if (kind == KIND_PRIMITIVE_OPERATOR)	return "PRIMITIVE operator";
	if (kind == KIND_BLOCK)				return "block";
	if (kind == KIND_ADDRESS_SHIFT)		return "address shift";
	if (kind == KIND_ARRAY)				return "array element";
	if (kind == KIND_POINTER_AS_ARRAY)		return "pointer as array element";
	if (kind == KIND_REFERENCE)			return "address operator";
	if (kind == KIND_DEREFERENCE)		return "dereferencing";
	if (kind == KIND_DEREF_ADDRESS_SHIFT)	return "deref address shift";
	if (kind == KIND_CLASS)				return "class";
	if (kind == KIND_ARRAY_BUILDER)		return "array builder";
	if (kind == KIND_CONSTRUCTOR_AS_FUNCTION)		return "constructor function";
	if (kind == KIND_VAR_TEMP)			return "temp";
	if (kind == KIND_DEREF_VAR_TEMP)		return "deref temp";
	if (kind == KIND_REGISTER)			return "register";
	if (kind == KIND_ADDRESS)			return "address";
	if (kind == KIND_MEMORY)				return "memory";
	if (kind == KIND_LOCAL_ADDRESS)		return "local address";
	if (kind == KIND_LOCAL_MEMORY)		return "local memory";
	if (kind == KIND_DEREF_REGISTER)		return "deref register";
	if (kind == KIND_MARKER)				return "marker";
	if (kind == KIND_DEREF_MARKER)		return "deref marker";
	if (kind == KIND_GLOBAL_LOOKUP)		return "global lookup";
	if (kind == KIND_DEREF_GLOBAL_LOOKUP)	return "deref global lookup";
	if (kind == KIND_IMMEDIATE)			return "immediate";
	if (kind == KIND_REF_TO_CONST)			return "ref to const";
	if (kind == KIND_DEREF_VAR_LOCAL)		return "deref local";
	return format("UNKNOWN KIND: %d", kind);
}


string Node::sig() const
{
	string t = type->name + " ";
	if (kind == KIND_VAR_LOCAL)			return t + as_local()->name;
	if (kind == KIND_VAR_GLOBAL)			return t + as_global()->name;
	if (kind == KIND_FUNCTION_POINTER)		return t + as_func()->long_name;
	if (kind == KIND_FUNCTION_NAME)		return t + as_func()->long_name;
	if (kind == KIND_CONSTANT)			return t + as_const()->str();
	if (kind == KIND_FUNCTION_CALL)			return as_func()->signature(true);
	if (kind == KIND_POINTER_CALL)			return "";
	if (kind == KIND_INLINE_CALL)	return as_func()->signature(true);
	if (kind == KIND_VIRTUAL_CALL)	return t + i2s(link_no);//s->Functions[nr]->name;
	if (kind == KIND_STATEMENT)			return t + as_statement()->name;
	if (kind == KIND_OPERATOR)			return as_op()->sig();
	if (kind == KIND_PRIMITIVE_OPERATOR)	return as_prim_op()->name;
	if (kind == KIND_BLOCK)				return "";//p2s(as_block());
	if (kind == KIND_ADDRESS_SHIFT)		return t + i2s(link_no);
	if (kind == KIND_ARRAY)				return t;
	if (kind == KIND_POINTER_AS_ARRAY)		return t;
	if (kind == KIND_REFERENCE)			return t;
	if (kind == KIND_DEREFERENCE)		return t;
	if (kind == KIND_DEREF_ADDRESS_SHIFT)	return t + i2s(link_no);
	if (kind == KIND_CLASS)				return as_class()->name;
	if (kind == KIND_REGISTER)			return t + Asm::GetRegName(link_no);
	if (kind == KIND_ADDRESS)			return t + d2h(&link_no, config.pointer_size);
	if (kind == KIND_MEMORY)				return t + d2h(&link_no, config.pointer_size);
	if (kind == KIND_LOCAL_ADDRESS)		return t + d2h(&link_no, config.pointer_size);
	if (kind == KIND_LOCAL_MEMORY)		return t + d2h(&link_no, config.pointer_size);
	return t + i2s(link_no);
}

string Node::str() const
{
	return "<" + kind2str(kind) + ">  " + sig();
}


void Node::show() const
{
	string orig;
	msg_write(str() + orig);
	msg_right();
	if (instance)
		instance->show();
	for (Node *p: params)
		if (p)
			p->show();
		else
			msg_write("<param nil>");
	msg_left();
}




Block::Block(Function *f, Block *_parent) :
	Node(KIND_BLOCK, (int_p)this, TypeVoid)
{
	level = 0;
	function = f;
	parent = _parent;
	if (parent)
		level = parent->level + 1;
	_start = _end = nullptr;
	_label_start = _label_end = -1;
}

Block::~Block()
{
}


inline void set_command(Node *&a, Node *b)
{
	a = b;
}

void Block::add(Node *c)
{
	if (c){
		params.add(c);
	}
}

void Block::set(int index, Node *c)
{
	params[index] = c;
}

Variable *Block::add_var(const string &name, const Class *type)
{
	if (get_var(name))
		function->owner->do_error(format("variable '%s' already declared in this context", name.c_str()));
	Variable *v = new Variable(name, type);
	v->is_extern = next_extern;
	function->var.add(v);
	vars.add(v);
	return v;
}

Variable *Block::get_var(const string &name)
{
	for (auto *v: vars)
		if (v->name == name)
			return v;
	if (parent)
		return parent->get_var(name);
	return nullptr;
}


Node::Node(int _kind, int64 _link_no, const Class *_type)
{
	type = _type;
	kind = _kind;
	link_no = _link_no;
	instance = nullptr;
}

Node::~Node()
{
	if (instance)
		delete instance;
	for (auto &p: params)
		if (p)
			delete p;
}

Block *Node::as_block() const
{
	return (Block*)this;
}

Function *Node::as_func() const
{
	return (Function*)link_no;
}

const Class *Node::as_class() const
{
	return (const Class*)link_no;
}

Constant *Node::as_const() const
{
	return (Constant*)link_no;
}

Operator *Node::as_op() const
{
	return (Operator*)link_no;
}
void *Node::as_func_p() const
{
	return as_func()->address;
}

// will be the address at runtime...(not the current location...)
void *Node::as_const_p() const
{
	return as_const()->address;
}

void *Node::as_global_p() const
{
	return as_global()->memory;
}

Variable *Node::as_global() const
{
	return (Variable*)link_no;
}

Variable *Node::as_local() const
{
	return (Variable*)link_no;
}

Statement *Node::as_statement() const
{
	return &Statements[link_no];
}

PrimitiveOperator *Node::as_prim_op() const
{
	return &PrimitiveOperators[link_no];
}

void Node::set_instance(Node *p)
{
	set_command(instance, p);
}

void Node::set_num_params(int n)
{
	params.resize(n);
}

void Node::set_param(int index, Node *p)
{
	/*if ((index < 0) or (index >= params.num)){
		show();
		throw Exception(format("internal: Node.set_param...  %d %d", index, params.num), "", 0);
	}*/
	set_command(params[index], p);
}

}

