/*
 * Module.h
 *
 *  Created on: Nov 15, 2022
 *      Author: michi
 */

#pragma once

#include "../base/base.h"
#include "../base/pointer.h"
#include "../os/path.h"
#include "asm/asm.h"

namespace kaba {

class Class;
class Function;
class Variable;
class Constant;
class SyntaxTree;
class Interpreter;
class Context;


// executable (compiled) data
class Module : public Sharable<base::Empty> {
public:
	// don't call yourself.... better use LoadScript(...)
	Module(Context *c, const Path &filename);
	~Module();

	void load(const Path &filename, bool just_analyse = false);

	// error messages
	void do_error(const string &msg, int override_token = -1);
	void do_error_link(const string &msg);
	void do_error_internal(const string &msg);

	// execution
	void *match_function(const string &name, const string &return_type, const Array<string> &param_types);
	void set_variable(const string &name, void *data);

	//debug displaying
	void show_vars(bool include_consts=false);

	bool is_system_module() const;

// data

	Path filename;
	owned<SyntaxTree> tree;
    Context *context;

	char *opcode; // executable code
	int opcode_size;

	char *memory;
	int memory_size;

	Array<Asm::WantedLabel> functions_to_link;
	Array<int> function_vars_to_link;

	bool just_analyse, show_compiler_stats;

	owned<Interpreter> interpreter;


	// package
	bool used_by_default;
	Array<const Class*> classes();
	Array<Variable*> variables();
	Array<Constant*> constants();
	Array<Function*> functions();
	const Class* base_class();
};

}

