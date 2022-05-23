/*
 * Function.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"
#include "../../base/pointer.h"

namespace kaba {

class Class;
class Block;
class SyntaxTree;
class Variable;
class Node;
enum class InlineID;
enum class Flags;


class Function : public Sharable<Empty> {
public:
	Function(const string &name, const Class *return_type, const Class *name_space, Flags flags = Flags(0));
	~Function();

	SyntaxTree *owner() const;
	
	string name;
	string long_name() const; // "Class.Function"
	string cname(const Class *ns) const;
	// parameters (linked to intern variables)
	int num_params;
	int mandatory_params;
	shared_array<Node> default_parameters;
	shared_array<Node> abstract_param_types;
	shared<Node> abstract_return_type;
	// block of code
	shared<Block> block;
	// local variables
	shared_array<Variable> var;
	Array<const Class*> literal_param_type;
	const Class *name_space;
	const Class *effective_return_type;
	const Class *literal_return_type;
	Flags flags;
	bool auto_declared;
	bool is_extern() const;
	bool is_pure() const;
	bool is_static() const;
	bool is_member() const;
	bool is_const() const;
	bool is_selfref() const;
	bool throws_exceptions() const; // for external
	bool is_template() const;
	bool needs_overriding() const;
	InlineID inline_no;
	int virtual_index;
	int num_slightly_hidden_vars;
	// for compilation...
	int64 _var_size;
	int token_id;
	int64 address;
	void *address_preprocess;
	int _label;
	Variable *add_param(const string &name, const Class *type, Flags flags);
	void set_return_type(const Class *type);
	Variable *__get_var(const string &name) const;
	string create_slightly_hidden_name();
	void update_parameters_after_parsing();
	void add_self_parameter();
	string signature(const Class *ns = nullptr) const;
	Array<Block*> all_blocks();
	void show(const string &stage = "") const;

	Function *create_dummy_clone(const Class *name_space) const;
};

class BindingTemplate {
public:
	Function *outer;
	Function *bind_temp;
	Function *inner;
	Array<Variable*> captures_local;
	Array<Variable*> captures_global;
	bytes capture_data;
	int capture_data_used = 0;
	int counter = 0;
};
extern Array<BindingTemplate*> binding_templates;



}

