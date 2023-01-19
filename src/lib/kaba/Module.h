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

	// building operational code
	void compile();
	void map_constants_to_opcode();
	void _map_global_variables_to_memory(char *mem, int &offset, char *address, const Class *name_space);
	void map_global_variables_to_memory();
	void map_constants_to_memory(char *mem, int &offset, char *address);
	void allocate_opcode();
	void align_opcode();
	void allocate_memory();
	void assemble_function(int index, Function *f, Asm::InstructionWithParamsList *list);
	void compile_functions(char *oc, int &ocs);
	void CompileOsEntryPoint();
	void LinkOsEntryPoint();
	void link_functions();
	void link_virtual_functions_into_vtable(const Class *c);

	// error messages
	void do_error(const string &msg, int override_token = -1);
	void do_error_link(const string &msg);
	void do_error_internal(const string &msg);

	// execution
	void *match_function(const string &name, const string &return_type, const Array<string> &param_types);
	void set_variable(const string &name, void *data);

	//debug displaying
	void show_vars(bool include_consts=false);

// data

	Path filename;
	SyntaxTree *syntax;
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

