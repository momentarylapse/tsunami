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


Variable::Variable(const string &_name, const Class *_type)
{
	name = _name;
	type = _type;
	_offset = 0;
	is_extern = false;
	dont_add_constructor = false;
	memory = nullptr;
	memory_owner = false;
}

Variable::~Variable()
{
	if (memory_owner)
		free(memory);
}


Function::Function(SyntaxTree *_tree, const string &_name, const Class *_return_type)
{
	tree = _tree;
	name = _name;
	long_name = name;
	block = nullptr;
	num_params = 0;
	return_type = _return_type;
	literal_return_type = _return_type;
	_class = nullptr;
	is_extern = false;
	auto_declared = false;
	is_pure = false;
	_param_size = 0;
	_var_size = 0;
	_logical_line_no = -1;
	_exp_no = -1;
	inline_no = -1;
	throws_exceptions = false;
	num_slightly_hidden_vars = 0;
	address = nullptr;
	_label = -1;
}

Function::~Function()
{
	for (Variable* v: var)
		delete v;
	if (block)
		delete block;
}

void Function::show(const string &stage) const
{
	if (!config.allow_output(this, stage))
		return;
	msg_write("[function] " + return_type->name + " " + long_name);
	block->show();
}

string Function::create_slightly_hidden_name()
{
	return format("-temp-%d-", ++ num_slightly_hidden_vars);
}

Variable *Function::__get_var(const string &name) const
{
	return block->get_var(name);
}

string Function::signature(bool include_class) const
{
	string r = literal_return_type->name + " ";
	if (include_class)
		r += long_name + "(";
	else
		r += name + "(";
	for (int i=0; i<num_params; i++){
		if (i > 0)
			r += ", ";
		r += literal_param_type[i]->name;
	}
	return r + ")";
}

void blocks_add_recursive(Array<Block*> &blocks, Block *block)
{
	blocks.add(block);
	for (Node* n: block->params){
		if (n->kind == KIND_BLOCK)
			blocks_add_recursive(blocks, n->as_block());
		if (n->kind == KIND_STATEMENT){
			if (n->link_no == STATEMENT_FOR){
				blocks_add_recursive(blocks, n->params[2]->as_block());
			}else if (n->link_no == STATEMENT_TRY){
				blocks_add_recursive(blocks, n->params[0]->as_block());
				blocks_add_recursive(blocks, n->params[2]->as_block());
			}else if (n->link_no == STATEMENT_IF){
				blocks_add_recursive(blocks, n->params[1]->as_block());
			}else if (n->link_no == STATEMENT_IF_ELSE){
				blocks_add_recursive(blocks, n->params[1]->as_block());
				blocks_add_recursive(blocks, n->params[2]->as_block());
			}
		}
	}
}

Array<Block*> Function::all_blocks()
{
	Array<Block*> blocks;
	if (block)
		blocks_add_recursive(blocks, block);
	return blocks;
}


}

