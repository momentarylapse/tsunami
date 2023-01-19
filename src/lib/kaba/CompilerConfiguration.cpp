/*
 * CompilerConfiguration.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */


#include "CompilerConfiguration.h"
#include "asm/asm.h"
#include "syntax/Function.h"


template<class T>
int element_offset(T p) {
	return *(int*)(void*)&p;
}

namespace kaba {

CompilerConfiguration config;

Asm::InstructionSet extract_instruction_set(Abi abi) {
	if ((abi == Abi::X86_GNU) or (abi == Abi::X86_WINDOWS))
		return Asm::InstructionSet::X86;
	if ((abi == Abi::AMD64_GNU) or (abi == Abi::AMD64_WINDOWS))
		return Asm::InstructionSet::AMD64;
	if ((abi == Abi::ARM32_GNU) or (abi == Abi::ARM64_GNU))
		return Asm::InstructionSet::ARM;
	//if (abi == Abi::NATIVE)
	return Asm::InstructionSet::NATIVE;
}

Abi guess_native_abi() {
	auto instruction_set = Asm::guess_native_instruction_set();
	if (instruction_set == Asm::InstructionSet::AMD64) {
#ifdef OS_WINDOWS
		return Abi::AMD64_WINDOWS;
#endif
		return Abi::AMD64_GNU;
	} else if (config.instruction_set == Asm::InstructionSet::X86) {
#ifdef OS_WINDOWS
		return Abi::X86_WINDOWS;
#endif
		return Abi::X86_GNU;
	} else if (config.instruction_set == Asm::InstructionSet::ARM) {
		return Abi::ARM32_GNU;
	}
	return Abi::UNKNOWN;
}

CompilerConfiguration::CompilerConfiguration() {
	native_abi = guess_native_abi();
	abi = native_abi;
	instruction_set = extract_instruction_set(abi);
	interpreted = false;
	allow_std_lib = true;
	pointer_size = sizeof(void*);
	super_array_size = sizeof(DynamicArray);

	allow_simplification = true;
	allow_registers = true;
	allow_simplify_consts = true;
	stack_mem_align = 8;
	function_align = 2 * pointer_size;
	stack_frame_align = 2 * pointer_size;

	compile_silently = false;
	verbose = false;
	verbose_func_filter = "*";
	verbose_stage_filter = "*";
	show_compiler_stats = true;

	compile_os = false;
	remove_unused = false;
	override_variables_offset = false;
	variables_offset = 0;
	override_code_origin = false;
	code_origin = 0;
	add_entry_point = false;

	function_address_offset = element_offset(&Function::address); // offsetof(Function, address);
}



bool CompilerConfiguration::allow_output_func(const Function *f) {
	if (!verbose)
		return false;
	if (!f)
		return true;
	Array<string> filters = verbose_func_filter.explode(",");
	for (auto &fil: filters)
		if (f->long_name().match(fil))
			return true;
	return false;
}

bool CompilerConfiguration::allow_output_stage(const string &stage) {
	if (!verbose)
		return false;
	auto filters = verbose_stage_filter.explode(",");
	for (auto &fil: filters)
		if (stage.match(fil))
			return true;
	return false;
}

bool CompilerConfiguration::allow_output(const Function *f, const string &stage) {
	if (!verbose)
		return false;
	if (!allow_output_func(f))
		return false;
	if (!allow_output_stage(stage))
		return false;
	return true;
}

}


