/*
 * Function.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_FUNCTION_H_
#define SRC_LIB_KABA_SYNTAX_FUNCTION_H_

#include "../../base/base.h"
#include "../../base/pointer.h"

namespace kaba {

class Class;
class Block;
class SyntaxTree;
enum class InlineID;
enum class Flags;


class Variable : public Sharable<Empty> {
public:
	Variable(const string &name, const Class *type);
	~Variable();
	const Class *type; // for creating instances
	string name;
	string long_name(const Class *ns) const;
	string cname(const Class *ns, const Class *ns_obs) const;
	int64 _offset; // for compilation
	void *memory;
	bool memory_owner;
	bool is_extern;
	bool is_const;
	bool explicitly_constructed;
	int _label;
};

// user defined functions
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
	// block of code
	shared<Block> block;
	// local variables
	shared_array<Variable> var;
	Array<const Class*> literal_param_type;
	const Class *name_space;
	const Class *return_type;
	const Class *literal_return_type;
	Flags flags;
	bool auto_declared;
	bool is_extern() const;
	bool is_pure() const;
	bool is_static() const;
	bool is_const() const;
	bool is_selfref() const;
	bool throws_exceptions() const; // for external
	InlineID inline_no;
	int virtual_index;
	bool needs_overriding;
	int num_slightly_hidden_vars;
	// for compilation...
	int64 _var_size, _param_size;
	int _logical_line_no;
	int _exp_no;
	void *address;
	void *address_preprocess;
	int _label;
	Variable *__get_var(const string &name) const;
	string create_slightly_hidden_name();
	void update_parameters_after_parsing();
	string signature(const Class *ns = nullptr) const;
	Array<Block*> all_blocks();
	void show(const string &stage = "") const;

	Function *create_dummy_clone(const Class *name_space) const;
};



}

#endif /* SRC_LIB_KABA_SYNTAX_FUNCTION_H_ */
