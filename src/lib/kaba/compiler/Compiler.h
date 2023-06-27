/*
 * Compiler.h
 *
 *  Created on: 27 Feb 2023
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_COMPILER_COMPILER_H_
#define SRC_LIB_KABA_COMPILER_COMPILER_H_

namespace kaba {

#include "../asm/asm.h"

class SyntaxTree;
class Module;
class Function;
class Class;
class Context;

class Compiler {
public:
	Compiler(Module *m);
	~Compiler();

	static void compile(Module *m);

	void _compile();
	void map_constants_to_opcode();
	void map_address_constants_to_opcode();
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


	Module *module;
	SyntaxTree *tree;
	Context *context;
};

}

#endif /* SRC_LIB_KABA_COMPILER_COMPILER_H_ */
