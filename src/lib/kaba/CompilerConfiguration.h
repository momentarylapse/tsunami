/*
 * CompilerConfiguration.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "../base/base.h"
#include "../os/path.h"

namespace Asm {
	enum class InstructionSet;
}

namespace kaba {

class Function;

enum class Abi {
	NATIVE = -1,
	UNKNOWN = -2,

	X86_GNU = 0,
	AMD64_GNU,
	X86_WINDOWS,
	AMD64_WINDOWS,
	ARM32_GNU,
	ARM64_GNU,
};

Asm::InstructionSet extract_instruction_set(Abi abi);

class CompilerConfiguration {
public:
	CompilerConfiguration();
	Asm::InstructionSet instruction_set;
	Abi abi;
	Abi native_abi;
	bool interpreted;
	bool allow_std_lib;

	int64 pointer_size;
	int super_array_size;

	bool allow_simplification;
	bool allow_registers;
	bool allow_simplify_consts;

	Path directory;
	bool verbose;
	string verbose_func_filter;
	string verbose_stage_filter;
	bool allow_output(const Function *f, const string &stage);
	bool allow_output_func(const Function *f);
	bool allow_output_stage(const string &stage);
	bool compile_silently;
	bool show_compiler_stats;

	bool compile_os;
	bool remove_unused;
	bool no_function_frame;
	bool add_entry_point;
	bool override_variables_offset;
	int64 variables_offset;
	bool override_code_origin;
	int64 code_origin;

	int stack_mem_align;
	int function_align;
	int stack_frame_align;

	int function_address_offset;

};

extern CompilerConfiguration config;

}
