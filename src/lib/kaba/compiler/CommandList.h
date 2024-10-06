/*
 * CommandList.h
 *
 *  Created on: Nov 26, 2020
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"

namespace Asm {
	enum class RegID;
	enum class RegRoot;
	enum class ArmCond;
	enum class InstID;
}


namespace kaba {

class Serializer;
struct SerialNode;
struct SerialNodeParam;
class Class;



#define max_reg			8 // >= all RegXXX used...

// represents a register
// (or rather the data inside, since many VirtualRegisters might be mapped to the same physical register)
struct VirtualRegister {
	Asm::RegID reg;
	Asm::RegRoot reg_root;
	int first, last;
};


struct TempVar {
	const Class *type;
	int first, last, usage_count;
	bool mapped;
	bool referenced;
	bool force_stack;
	bool force_register;
	int stack_offset;
	int entangled;
	TempVar();
	void use(int first, int last);
};

struct CommandList {
	Array<SerialNode> cmd;
	Array<VirtualRegister> virtual_reg;
	Array<TempVar> temp_var;
	int next_cmd_index = 0;
	Serializer *ser = nullptr;

	//void add_reg_channel(int reg, int first, int last);
	int add_virtual_reg(Asm::RegID reg);
	void set_virtual_reg(int v, int first, int last);
	void use_virtual_reg(int v, int first, int last);
	SerialNodeParam _add_temp(const Class *t);
	void add_cmd(Asm::ArmCond cond, Asm::InstID inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3);
	void add_cmd(Asm::InstID inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3);
	void add_cmd(Asm::InstID inst, const SerialNodeParam &p1, const SerialNodeParam &p2);
	void add_cmd(Asm::InstID inst, const SerialNodeParam &p);
	void add_cmd(Asm::InstID inst);
	void set_cmd_param(int index, int param_index, const SerialNodeParam &p);
	void next_cmd_target(int index);
	void remove_cmd(int index);
	void remove_temp_var(int v);
	void move_param(SerialNodeParam &p, int from, int to);
	int add_label(int m = -1);
};


};


