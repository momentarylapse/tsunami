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
Abi guess_native_abi();
string abi_name(Abi abi);


inline int mem_align(int x, int n) {
	return ((x + n - 1) / n) * n;
}

class CompilerConfiguration {
public:
	CompilerConfiguration();

	struct Target {
		static Target get_native();
		static Target get_for_abi(Abi abi);

		Asm::InstructionSet instruction_set;
		Abi abi;
		bool interpreted;
		bool is_native;

		int pointer_size;
		int dynamic_array_size;

		int stack_mem_align;
		int function_align;
		int stack_frame_align;

		bool is_arm() const;
		bool is_x86() const; // 32 or 64 bit
	} target, native_target;

	bool allow_std_lib = true;
	bool allow_simplification = true;
	bool allow_registers = true;
	bool allow_simplify_consts = true;

	Path directory;
	bool verbose = false;
	string verbose_func_filter;
	string verbose_stage_filter;
	bool allow_output(const Function *f, const string &stage) const;
	bool allow_output_func(const Function *f) const;
	bool allow_output_stage(const string &stage) const;
	bool compile_silently = false;
	bool show_compiler_stats = true;

	bool fully_linear_output = false;
	bool remove_unused = false;
	bool add_entry_point = false;
	bool override_variables_offset = false;
	int64 variables_offset = 0;
	bool override_code_origin = false;
	int64 code_origin = 0;

	int function_address_offset = 0;

	Path default_filename; // for create_for_source()
};

extern CompilerConfiguration config;

}
