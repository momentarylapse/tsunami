/*
 * Block.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "Node.h"
#include "Flags.h"

namespace kaba {

// {...}-block
struct Block {
	Block(Function *f, Block *parent, int x);
	Array<Variable*> vars;
	Function *function;
	Block *parent;
	owned_array<Block> children;
	void *_start, *_end; // opcode range
	int _label_start, _label_end;
	int level;

	Block* create_child();

	const Class *name_space() const;

	Variable *get_var(const string &name) const;
	Variable *add_var(const string &name, const Class *type, int token_id, Flags flags = Flags::Mutable);
	Variable *insert_var(int index, const string &name, const Class *type, int token_id, Flags flags = Flags::Mutable);
};


}
