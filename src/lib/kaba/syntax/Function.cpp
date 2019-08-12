/*
 * Function.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Kaba{


Variable::Variable(const string &_name, const Class *_type) {
	name = _name;
	type = _type;
	_offset = 0;
	is_extern = false;
	dont_add_constructor = false;
	memory = nullptr;
	memory_owner = false;
	_label = -1;
}

Variable::~Variable() {
	if (memory_owner)
		free(memory);
}


Function::Function(const string &_name, const Class *_return_type, const Class *_name_space) {
	owner = _name_space->owner;
	name = _name;
	block = nullptr;
	num_params = 0;
	return_type = _return_type;
	literal_return_type = _return_type;
	_class = _name_space;
	is_extern = false;
	is_static = true;
	auto_declared = false;
	is_pure = false;
	_param_size = 0;
	_var_size = 0;
	_logical_line_no = -1;
	_exp_no = -1;
	inline_no = -1;
	throws_exceptions = false;
	num_slightly_hidden_vars = 0;
	address = address_preprocess = nullptr;
	_label = -1;
}

#include "../../base/set.h"
#include "SyntaxTree.h"

void test_node_recursion(Node *root, const string &message) {
	Set<Node*> nodes;
	SyntaxTree::transform_node(root, [&](Node *n){
		if (nodes.contains(n)){
			msg_error("node double..." + message);
			//msg_write(f->long_name);
			msg_write(n->str());
		}else
			nodes.add(n);
		return n; });
}

Function::~Function() {
	//test_node_recursion(block, long_name());
	if (block)
		delete block;
	for (Variable* v: var)
		delete v;
}

string Function::long_name() const {
	if (_class)
		if (_class->name_space)
			return _class->long_name() + "." + name;
	return name;
}

void Function::show(const string &stage) const {
	if (!config.allow_output(this, stage))
		return;
	msg_write("[function] " + return_type->long_name() + " " + long_name());
	block->show();
}

string Function::create_slightly_hidden_name() {
	return format("-temp-%d-", ++ num_slightly_hidden_vars);
}

Variable *Function::__get_var(const string &name) const {
	return block->get_var(name);
}

string Function::signature(bool include_class) const {
	string r = literal_return_type->long_name() + " ";
	if (include_class)
		r += long_name() + "(";
	else
		r += name + "(";
	for (int i=0; i<num_params; i++) {
		if (i > 0)
			r += ", ";
		r += literal_param_type[i]->long_name();
	}
	return r + ")";
}

void blocks_add_recursive(Array<Block*> &blocks, Block *block) {
	blocks.add(block);
	for (Node* n: block->params) {
		if (n->kind == KIND_BLOCK)
			blocks_add_recursive(blocks, n->as_block());
		if (n->kind == KIND_STATEMENT) {
			if (n->link_no == STATEMENT_FOR) {
				blocks_add_recursive(blocks, n->params[2]->as_block());
			} else if (n->link_no == STATEMENT_TRY) {
				blocks_add_recursive(blocks, n->params[0]->as_block());
				blocks_add_recursive(blocks, n->params[2]->as_block());
			} else if (n->link_no == STATEMENT_IF) {
				blocks_add_recursive(blocks, n->params[1]->as_block());
			} else if (n->link_no == STATEMENT_IF_ELSE) {
				blocks_add_recursive(blocks, n->params[1]->as_block());
				blocks_add_recursive(blocks, n->params[2]->as_block());
			}
		}
	}
}

Array<Block*> Function::all_blocks() {
	Array<Block*> blocks;
	if (block)
		blocks_add_recursive(blocks, block);
	return blocks;
}


}

