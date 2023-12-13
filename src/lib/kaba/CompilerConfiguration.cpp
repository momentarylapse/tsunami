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
	if (abi == Abi::ARM32_GNU)
		return Asm::InstructionSet::ARM32;
	if (abi == Abi::ARM64_GNU)
		return Asm::InstructionSet::ARM64;
	if (abi == Abi::NATIVE)
		return Asm::InstructionSet::NATIVE;
	return Asm::InstructionSet::UNKNOWN;
}

Abi guess_native_abi() {
	auto instruction_set = Asm::guess_native_instruction_set();
	if (instruction_set == Asm::InstructionSet::AMD64) {
#ifdef OS_WINDOWS
		return Abi::AMD64_WINDOWS;
#endif
		return Abi::AMD64_GNU;
	} else if (instruction_set == Asm::InstructionSet::X86) {
#ifdef OS_WINDOWS
		return Abi::X86_WINDOWS;
#endif
		return Abi::X86_GNU;
	} else if (instruction_set == Asm::InstructionSet::ARM32) {
		return Abi::ARM32_GNU;
	} else if (instruction_set == Asm::InstructionSet::ARM64) {
		return Abi::ARM64_GNU;
	}
	return Abi::UNKNOWN;
}

string abi_name(Abi abi) {
	if (abi == Abi::AMD64_GNU)
		return "amd64:gnu";
	if (abi == Abi::AMD64_WINDOWS)
		return "amd64:win";
	if (abi == Abi::X86_GNU)
		return "x86:gnu";
	if (abi == Abi::X86_WINDOWS)
		return "x86:win";
	if (abi == Abi::ARM32_GNU)
		return "arm32:gnu";
	if (abi == Abi::ARM64_GNU)
		return "arm64:gnu";
	return "<unknown>";
}

CompilerConfiguration::Target CompilerConfiguration::Target::get_native() {
	Target t;
	t.is_native = true;
	t.abi = guess_native_abi();
	t.instruction_set = extract_instruction_set(t.abi);
	t.interpreted = false;
	t.pointer_size = sizeof(void*);
	t.dynamic_array_size = sizeof(DynamicArray);
	t.stack_mem_align = 8;
	t.function_align = 2 * t.pointer_size;
	t.stack_frame_align = 2 * t.pointer_size;
	return t;
}


CompilerConfiguration::Target CompilerConfiguration::Target::get_for_abi(Abi abi) {
	Target t;
	t.is_native = (abi == guess_native_abi());
	t.abi = abi;
	t.instruction_set = extract_instruction_set(abi);
	t.interpreted = false;
	if ((t.instruction_set == Asm::InstructionSet::X86) or (t.instruction_set == Asm::InstructionSet::ARM32))
		t.pointer_size = 4;
	else
		t.pointer_size = 8;
	t.dynamic_array_size = mem_align(t.pointer_size + 3 * sizeof(int), t.pointer_size);
	t.stack_mem_align = 8;
	t.function_align = 2 * t.pointer_size;
	t.stack_frame_align = 2 * t.pointer_size;
	return t;
}

bool CompilerConfiguration::Target::is_arm() const {
	return instruction_set == Asm::InstructionSet::ARM32 or instruction_set == Asm::InstructionSet::ARM64;
}

bool CompilerConfiguration::Target::is_x86() const {
	return instruction_set == Asm::InstructionSet::X86 or instruction_set == Asm::InstructionSet::AMD64;
}

CompilerConfiguration::CompilerConfiguration() {
	allow_std_lib = true;

	allow_simplification = true;
	allow_registers = true;
	allow_simplify_consts = true;

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


