/*
 * Function.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../asm/asm.h"
#include "../../os/msg.h"
#include <stdio.h>

namespace kaba {

string namespacify_rel(const string &name, const Class *name_space, const Class *observer_ns);

Array<BindingTemplate*> binding_templates;



Function::Function(const string &_name, const Class *_return_type, const Class *_name_space, Flags _flags) {
	name = _name;
	block = new Block(this, nullptr);
	num_params = 0;
	mandatory_params = 0;
	effective_return_type = _return_type;
	literal_return_type = _return_type;
	name_space = _name_space;
	flags = _flags;
	auto_declared = false;
	_var_size = 0;
	token_id = -1;
	inline_no = InlineID::None;
	virtual_index = -1;
	num_slightly_hidden_vars = 0;
	address = 0;
	address_preprocess = nullptr;
	_label = -1;
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
	return cname(nullptr);
}

string Function::cname(const Class *ns) const {
	string p;
	p = (is_unimplemented() and !is_extern()) ? " [NEEDS OVERRIDE]" : "";
	p = is_template() ? " [TEMPLATE]" : "";
	return namespacify_rel(name, name_space, ns) + p;
}

void Function::show(const string &stage) const {
	if (!config.allow_output(this, stage))
		return;
	auto ns = owner()->base_class;
	msg_write("[function] " + signature(ns));
	block->show(ns);
}

string Function::create_slightly_hidden_name() {
	return format(":temp-%d:", ++ num_slightly_hidden_vars);
}

Variable *Function::__get_var(const string &name) const {
	return block->get_var(name);
}

Variable *Function::add_param(const string &name, const Class *type, Flags flags) {
	auto v = block->insert_var(num_params, name, type);
	if (flags_has(flags, Flags::Out))
		flags_set(v->flags, Flags::Out);
	else
		flags_clear(v->flags, Flags::Mutable);
	literal_param_type.add(type);
	num_params ++;
	return v;
}

void Function::set_return_type(const Class *type) {
	literal_return_type = type;
	effective_return_type = type;
}

string Function::signature(const Class *ns) const {
	if (!ns)
		ns = owner()->base_class;
	string r = cname(ns) + "(";
	int first = is_member() ? 1 : 0;
	for (int i=first; i<num_params; i++) {
		if (i > first)
			r += ", ";
		if (flags_has(var[i]->flags, Flags::Out))
			r += "out ";
		r += literal_param_type[i]->cname(ns);
	}
	r += ")";
	if (literal_return_type != TypeVoid)
		r += " -> " + literal_return_type->cname(ns);
	return r;
}

void blocks_add_recursive(Array<Block*> &blocks, Block *block) {
	blocks.add(block);
	for (auto n: weak(block->params)) {
		if (n->kind == NodeKind::Block)
			blocks_add_recursive(blocks, n->as_block());
		if (n->kind == NodeKind::Statement) {
			for (auto p: weak(n->params))
				if (p->kind == NodeKind::Block)
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
	mandatory_params = num_params;
	for (int i=default_parameters.num-1; i>=0; i--)
		if (default_parameters[i])
			mandatory_params = i;


	// save "original" param types (var[].type gets altered for call by reference)
	for (int i=literal_param_type.num;i<num_params;i++)
		literal_param_type.add(var[i]->type);
	// but only, if not existing yet...

	// return by memory
	if (literal_return_type->uses_return_by_memory())
		//if (!__get_var(Identifier::RETURN_VAR))
			block->add_var(Identifier::ReturnVar, owner()->type_ref(literal_return_type));

	// class function
	if (is_member()) {
		if (!__get_var(Identifier::Self))
			add_self_parameter();
		/*if (flags_has(flags, Flags::CONST))
			flags_set(__get_var(Identifier::SELF)->flags, Flags::CONST);
		if (flags_has(flags, Flags::REF))
			flags_set(__get_var(Identifier::SELF)->flags, Flags::REF);*/
	}
}

void Function::add_self_parameter() {
	auto _flags = Flags::None;
	if (flags_has(flags, Flags::Mutable))
		flags_set(_flags, Flags::Out | Flags::Mutable);
	if (flags_has(flags, Flags::Ref))
		flags_set(_flags, Flags::Ref);
	block->insert_var(0, Identifier::Self, name_space, _flags);
	literal_param_type.insert(name_space, 0);
	abstract_param_types.insert(nullptr, 0);
	num_params ++;
	mandatory_params ++;
	default_parameters.insert(nullptr, 0);
}

// * NOT added to namespace
// * update_parameters_after_parsing() called
Function *Function::create_dummy_clone(const Class *_name_space) const {
	Function *f = new Function(name, literal_return_type, _name_space, flags);
	flags_set(f->flags, Flags::Unimplemented);
	flags_clear(f->flags, Flags::Extern);

	f->num_params = num_params;
	f->default_parameters = default_parameters;
	f->literal_param_type = literal_param_type;
	f->abstract_param_types = abstract_param_types;
	f->abstract_return_type = abstract_return_type;
	for (int i=0; i<num_params; i++) {
		auto type = var[i]->type;
		if (is_member() and (i == 0)) { // adapt the "self" parameter
			type = _name_space;
			f->literal_param_type[0] = type;
		}
		f->block->add_var(var[i]->name, var[i]->type);
		f->var[i]->flags = var[i]->flags;
	}

	f->virtual_index = virtual_index;

	if (!is_template())
		f->update_parameters_after_parsing();
	if (config.verbose)
		msg_write("DUMMY CLONE   " + f->signature(_name_space));
	return f;
}

bool Function::is_extern() const {
	return flags_has(flags, Flags::Extern);
}

bool Function::is_pure() const {
	return flags_has(flags, Flags::Pure);
}

bool Function::is_static() const {
	return flags_has(flags, Flags::Static);
}

bool Function::is_member() const {
	return !flags_has(flags, Flags::Static);
}

bool Function::is_mutable() const {
	return flags_has(flags, Flags::Mutable);

	// hmmm, might be better, to use self:
	if (is_static())
		return true;
	return __get_var(Identifier::Self)->is_mutable();
}

bool Function::is_selfref() const {
	if (is_static())
		return false;
	return flags_has(flags, Flags::Ref);
	return flags_has(__get_var(Identifier::Self)->flags, Flags::Ref);
}

bool Function::throws_exceptions() const {
	return flags_has(flags, Flags::RaisesExceptions);
}

bool Function::is_template() const {
	return flags_has(flags, Flags::Template);
}

bool Function::is_macro() const {
	return flags_has(flags, Flags::Macro);
}

bool Function::is_unimplemented() const {
	return flags_has(flags, Flags::Unimplemented);
}

}

