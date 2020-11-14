/*
 * Function.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../lib/common.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace kaba {

string namespacify_rel(const string &name, const Class *name_space, const Class *observer_ns);

Variable::Variable(const string &_name, const Class *_type) {
	name = _name;
	type = _type;
	_offset = 0;
	is_extern = false;
	is_const = false;
	explicitly_constructed = false;
	memory = nullptr;
	memory_owner = false;
	_label = -1;
}

Variable::~Variable() {
	if (memory_owner)
		free(memory);
}

string Variable::long_name(const Class *ns) const {
	return namespacify_rel(name, ns, nullptr);
}

string Variable::cname(const Class *ns, const Class *ns_obs) const {
	return namespacify_rel(name, ns, ns_obs);
}


Function::Function(const string &_name, const Class *_return_type, const Class *_name_space, Flags _flags) {
	name = _name;
	block = new Block(this, nullptr);
	num_params = 0;
	effective_return_type = _return_type;
	literal_return_type = _return_type;
	name_space = _name_space;
	flags = _flags;
	auto_declared = false;;
	_param_size = 0;
	_var_size = 0;
	_logical_line_no = -1;
	_exp_no = -1;
	inline_no = InlineID::NONE;
	virtual_index = -1;
	num_slightly_hidden_vars = 0;
	address = address_preprocess = nullptr;
	_label = -1;
	needs_overriding = false;
}

#include "../../base/set.h"
#include "SyntaxTree.h"

void test_node_recursion(shared<Node> root, const Class *ns, const string &message) {
	/*Set<Node*> nodes;
	SyntaxTree::transform_node(root, [&](shared<Node> n) {
		if (nodes.contains(n.get())) {
			msg_error("node double..." + message);
			//msg_write(f->long_name);
			msg_write(n->str(ns));
		} else {
			nodes.add(n.get());
		}
		return n; });*/
}

Function::~Function() {
	//test_node_recursion(block, long_name());
}

SyntaxTree *Function::owner() const {
	return name_space->owner;
}


string Function::long_name() const {
	string p = (needs_overriding ? " [NEEDS OVERRIDE]" : "");
	return namespacify_rel(name, name_space, nullptr) + p;
}

string Function::cname(const Class *ns) const {
	string p = (needs_overriding ? " [NEEDS OVERRIDE]" : "");
	return namespacify_rel(name, name_space, ns) + p;
}

void Function::show(const string &stage) const {
	if (!config.allow_output(this, stage))
		return;
	auto ns = owner()->base_class;
	msg_write("[function] " + literal_return_type->cname(ns) + " " + cname(ns));
	block->show(ns);
}

string Function::create_slightly_hidden_name() {
	return format("-temp-%d-", ++ num_slightly_hidden_vars);
}

Variable *Function::__get_var(const string &name) const {
	return block->get_var(name);
}

string Function::signature(const Class *ns) const {
	if (!ns)
		ns = owner()->base_class;
	string r = literal_return_type->cname(ns) + " ";
	r += cname(ns) + "(";
	for (int i=0; i<num_params; i++) {
		if (i > 0)
			r += ", ";
		r += literal_param_type[i]->cname(ns);
	}
	return r + ")";
}

void blocks_add_recursive(Array<Block*> &blocks, Block *block) {
	blocks.add(block);
	for (auto n: weak(block->params)) {
		if (n->kind == NodeKind::BLOCK)
			blocks_add_recursive(blocks, n->as_block());
		if (n->kind == NodeKind::STATEMENT) {
			for (auto p: weak(n->params))
				if (p->kind == NodeKind::BLOCK)
					blocks_add_recursive(blocks, p->as_block());
		}
	}
}

Array<Block*> Function::all_blocks() {
	Array<Block*> blocks;
	if (block)
		blocks_add_recursive(blocks, block.get());
	return blocks;
}


void Function::update_parameters_after_parsing() {
	// save "original" param types (Var[].Type gets altered for call by reference)
	for (int i=literal_param_type.num;i<num_params;i++)
		literal_param_type.add(var[i]->type);
	// but only, if not existing yet...

	// return by memory
	if (literal_return_type->uses_return_by_memory())
		block->add_var(IDENTIFIER_RETURN_VAR, literal_return_type->get_pointer());

	// class function
	if (!is_static()) {
		if (!__get_var(IDENTIFIER_SELF))
			block->add_var(IDENTIFIER_SELF, name_space, is_const());
	}
}


Function *Function::create_dummy_clone(const Class *_name_space) const {
	Function *f = new Function(name, literal_return_type, _name_space, flags);
	f->needs_overriding = true;

	f->num_params = num_params;
	f->literal_param_type = literal_param_type;
	for (int i=0; i<num_params; i++) {
		f->block->add_var(var[i]->name, var[i]->type);
		f->var[i]->is_const = var[i]->is_const;
	}

	f->virtual_index = virtual_index;

	f->update_parameters_after_parsing();
	if (config.verbose)
		msg_write("DUMMY CLONE   " + f->signature(_name_space));
	return f;
}

bool Function::is_extern() const {
	return flags_has(flags, Flags::EXTERN);
}

bool Function::is_pure() const {
	return flags_has(flags, Flags::PURE);
}

bool Function::is_static() const {
	return flags_has(flags, Flags::STATIC);
}

bool Function::is_const() const {
	return flags_has(flags, Flags::CONST);
}

bool Function::is_selfref() const {
	return flags_has(flags, Flags::SELFREF);
}

bool Function::throws_exceptions() const {
	return flags_has(flags, Flags::RAISES_EXCEPTIONS);
}

}

