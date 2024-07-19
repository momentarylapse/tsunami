/*
 * BackendARM.cpp
 *
 *  Created on: Dec 1, 2020
 *      Author: michi
 */


#include "BackendARM.h"
#include "Serializer.h"
#include "CommandList.h"
#include "SerialNode.h"
#include "../asm/asm.h"
#include "../../os/msg.h"


namespace Asm{
	extern Asm::RegID r_reg(int i);
	extern Asm::RegID s_reg(int i);
};

namespace kaba {

#define reg_s0 param_preg(TypeFloat32, Asm::RegID::S0)
#define reg_s1 param_preg(TypeFloat32, Asm::RegID::S1)
#define VREG_ROOT(r) cmd.virtual_reg[r].reg_root

//bool is_typed_function_pointer(const Class *c);

BackendARM::BackendARM(Serializer *s) : Backend(s) {
	map_reg_root = {Asm::RegRoot::R0, Asm::RegRoot::R1};
}

BackendARM::~BackendARM() {
}

void BackendARM::process(Function *f, int index) {
	cur_func = f;
	cur_func_index = index;
	//call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;

	do_mapping();
	correct();
}

void BackendARM::correct() {
	cmd.next_cmd_target(0);

	serializer->cmd_list_out("x:a", "paramtrafo");

	correct_implement_commands();

	cmd.next_cmd_target(0);
	add_function_intro_params(cur_func);
	serializer->cmd_list_out("x:b", "post paramtrafo");
}

void BackendARM::implement_mov_chunk(kaba::SerialNode &c, int i, int size) {
	auto p1 = c.p[0];
	auto p2 = c.p[1];
	cmd.remove_cmd(i);
	cmd.next_cmd_target(i);
	//msg_error("CORRECT MOV " + p1.type->name);

	for (int j=0; j<size/4; j++)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, j * 4, TypeInt32), param_shift(p2, j * 4, TypeInt32));
	for (int j=4*(size/4); j<size; j++)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, j, TypeInt8), param_shift(p2, j, TypeInt8));
}

static int first_bit(int i) {
	for (int b=0; b<32; b++)
		if ((i & (1 << b)) != 0)
			return b;
	return 0;
}

// TODO better use int64?
void BackendARM::_immediate_to_register_32(int val, int r) {
	bool first = true;
	while (true) {
		int b0 = first_bit(val) & 0xfe; // only even bit positions allowed!
		int mask = 0xff << b0;
		if (first)
			insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt32, r), param_imm(TypeInt32, val&mask));
		else
			insert_cmd(Asm::InstID::ADD, param_vreg(TypeInt32, r), param_vreg(TypeInt32, r), param_imm(TypeInt32, val&mask));
		val -= (val & mask);
		if (val == 0)
			break;
		first = false;
	}
}

void BackendARM::_immediate_to_register_8(int val, int r) {
	insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt32, r), param_imm(TypeInt32, val & 0xff));
}

void BackendARM::_register_to_local_32(int r, int offset) {
	insert_cmd(Asm::InstID::STR, param_vreg(TypeInt32, r), param_local(TypeInt32, offset));
}

void BackendARM::_register_to_local_8(int r, int offset) {
	insert_cmd(Asm::InstID::STRB, param_vreg(TypeInt8, r), param_local(TypeInt8, offset));
}

void BackendARM::_register_to_global_32(int r, int64 addr) {
	int reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(r));
	_immediate_to_register_32(addr, reg);
	insert_cmd(Asm::InstID::STR, param_vreg(TypeInt32, r), param_deref_vreg(TypeInt32, reg));
}

void BackendARM::_register_to_global_8(int r, int64 addr) {
	int reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(r));
	_immediate_to_register_32(addr, reg);
	insert_cmd(Asm::InstID::STRB, param_vreg(TypeInt8, r), param_deref_vreg(TypeInt8, reg));
}

void BackendARM::_local_to_register_32(int offset, int r) {
	insert_cmd(Asm::InstID::LDR, param_vreg(TypeInt32, r), param_local(TypeInt32, offset));
}

void BackendARM::_local_to_register_8(int offset, int r) {
	insert_cmd(Asm::InstID::LDRB, param_vreg(TypeInt8, r), param_local(TypeInt8, offset));
}

void BackendARM::_global_to_register_32(int64 addr, int r) {
	int reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(r));
	_immediate_to_register_32(addr, reg);
	insert_cmd(Asm::InstID::LDR, param_vreg(TypeInt32, r), param_deref_vreg(TypeInt32, reg));
}

void BackendARM::_global_to_register_8(int64 addr, int r) {
	int reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(r));
	_immediate_to_register_32(addr, reg);
	insert_cmd(Asm::InstID::LDRB, param_vreg(TypeInt8, r), param_deref_vreg(TypeInt8, reg));
}

int BackendARM::_to_register_32(const SerialNodeParam &p, int offset, int force_register) {
	//if (p.kind == NodeKind::REGISTER)
	//	return p.p;

	int reg = force_register;
	if (reg < 0)
		reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4);//Asm::RegID::R0;
	if (p.kind == NodeKind::CONSTANT) {
		auto cc = (Constant*)p.p;
		_immediate_to_register_32(*((int*)((char*)cc->p() + offset)), reg);
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		_immediate_to_register_32(*((int*)((char*)p.p + offset)), reg);
	} else if (p.kind == NodeKind::IMMEDIATE) {
		if (offset != 0)
			do_error("immediate + offset");
		_immediate_to_register_32(p.p, reg);
	} else if (p.kind == NodeKind::VAR_LOCAL) {
		auto var2 = (Variable*)p.p;
		_local_to_register_32(var2->_offset + offset, reg);
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		int reg2 = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(reg));
		_local_to_register_32(p.p + offset, reg2);
		insert_cmd(Asm::InstID::LDR, param_vreg(TypeInt32, reg), param_deref_vreg(TypeInt32, reg2));
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		_local_to_register_32(p.p + offset, reg);
	} else if (p.kind == NodeKind::GLOBAL_LOOKUP) {
		// TODO really use global ref
		if (offset != 0)
			do_error("global lookup + offset");
		_immediate_to_register_32((int)(int_p)global_refs[p.p].p, reg);
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		_global_to_register_32((int_p)var->memory + offset, reg);
	} else {
		do_error("evil read source..." + kind2str(p.kind));
	}
	return reg;
}

void BackendARM::_from_register_32(int reg, const SerialNodeParam &p, int offset) {
	if (p.kind == NodeKind::VAR_LOCAL) {
		auto var = (Variable*)p.p;
		_register_to_local_32(reg, var->_offset + offset);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		_register_to_local_32(reg, p.p + offset);
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		_register_to_global_32(reg, (int_p)var->memory + offset);
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		int reg2 = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(reg));
		// *mem = reg
		if (offset != 0)
			do_error("deref local + shift...");

		// reg2 = mem
		_local_to_register_32(p.p, reg2);
		// [reg2] = reg
		insert_cmd(Asm::InstID::STR, param_vreg(TypeInt32, reg), param_deref_vreg(TypeInt32, reg2));
	} else {
		do_error("evil write target..." + kind2str(p.kind));
	}
}



int BackendARM::_to_register_8(const SerialNodeParam &p, int offset, int force_register) {
	//if (p.kind == NodeKind::REGISTER)
	//	return p.p;

	int reg = force_register;
	if (reg < 0)
		reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4);//Asm::RegID::R0;
	if (p.kind == NodeKind::CONSTANT) {
		auto cc = (Constant*)p.p;
		_immediate_to_register_8(*((char*)cc->p() + offset), reg);
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		_immediate_to_register_8(*((char*)p.p + offset), reg);
	} else if (p.kind == NodeKind::IMMEDIATE) {
		if (offset != 0)
			do_error("immediate + offset");
		_immediate_to_register_8(p.p, reg);
	} else if (p.kind == NodeKind::VAR_LOCAL) {
		auto var2 = (Variable*)p.p;
		_local_to_register_8(var2->_offset + offset, reg);
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		int reg2 = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(reg));
		_local_to_register_32(p.p + offset, reg2);
		insert_cmd(Asm::InstID::LDR, param_vreg(TypeInt8, reg), param_deref_vreg(TypeInt8, reg2));
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		_local_to_register_8(p.p + offset, reg);
	/*} else if (p.kind == NodeKind::GLOBAL_LOOKUP) {
		do_error("global loo")
		// TODO really use global ref
		if (offset != 0)
			do_error("global lookup + offset");
		_immediate_to_register_32((int)(int_p)global_refs[p.p].p, reg);*/
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		_global_to_register_8((int_p)var->memory + offset, reg);
	} else {
		do_error("evil read source..." + kind2str(p.kind));
	}
	return reg;
}

void BackendARM::_from_register_8(int reg, const SerialNodeParam &p, int offset) {
	if (p.kind == NodeKind::VAR_LOCAL) {
		auto var = (Variable*)p.p;
		_register_to_local_8(reg, var->_offset + offset);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		_register_to_local_8(reg, p.p + offset);
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		_register_to_global_8(reg, (int_p)var->memory + offset);
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		int reg2 = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4, VREG_ROOT(reg));
		// *mem = reg
		if (offset != 0)
			do_error("deref local + shift...");

		// reg2 = mem
		_local_to_register_32(p.p, reg2);
		// [reg2] = reg
		insert_cmd(Asm::InstID::STRB, param_vreg(TypeInt8, reg), param_deref_vreg(TypeInt8, reg2));
	} else {
		do_error("evil write target..." + kind2str(p.kind));
	}
}


int BackendARM::_to_register_float(const SerialNodeParam &p, int offset, int force_register) {
	if (force_register < 0)
		do_error("explicit register needed for float");
	int sreg = force_register;//cmd.add_virtual_reg(Asm::RegID::S1);
	int reg = _to_register_32(p, offset);
	insert_cmd(Asm::InstID::FMSR, param_vreg(TypeFloat32, sreg), param_vreg(TypeFloat32, reg));
	return sreg;
}

void BackendARM::_from_register_float(int sreg, const SerialNodeParam &p, int offset) {
	if (p.kind == NodeKind::VAR_LOCAL) {
		auto var = (Variable*)p.p;
		insert_cmd(Asm::InstID::FSTS, param_local(TypeFloat32, var->_offset + offset), param_vreg(TypeFloat32, sreg));
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		insert_cmd(Asm::InstID::FSTS, param_local(TypeFloat32, p.p + offset), param_vreg(TypeFloat32, sreg));
	} else {
		int reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4);
		insert_cmd(Asm::InstID::FMRS, param_vreg(TypeFloat32, reg), param_vreg(TypeFloat32, sreg));
		_from_register_32(reg, p, offset);
	}
}

void BackendARM::correct_implement_commands() {

	Array<SerialNodeParam> func_params;

	for (int i=0; i<cmd.cmd.num; i++) {
		auto &c = cmd.cmd[i];
		//msg_write("CORRECT  " + c.str(serializer));
		if (c.inst == Asm::InstID::LABEL)
			continue;
		if (c.inst == Asm::InstID::ASM)
			continue;
		if (c.inst == Asm::InstID::MOV) {
			int size = c.p[0].type->size;
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			cmd.remove_cmd(i);

			for (int k=0; k<size/4; k++) {
				int reg = _to_register_32(p1, k*4);
				_from_register_32(reg, p0, k*4);
			}
			int offset = (size / 4) * 4;
			for (int k=0; k<size%4; k++) {
				int reg = _to_register_8(p1, offset + k);
				_from_register_8(reg, p0, offset + k);
			}
			i = cmd.next_cmd_index - 1;
		} else if (c.inst == Asm::InstID::MOVSX or c.inst == Asm::InstID::MOVZX) {
			do_error("no movsx yet");

		} else if ((c.inst == Asm::InstID::ADD) or (c.inst == Asm::InstID::SUB) or (c.inst == Asm::InstID::IMUL) /*or (c.inst == Asm::InstID::IDIV)*/ or (c.inst == Asm::InstID::AND) or (c.inst == Asm::InstID::OR)) {
			auto inst = c.inst;
			if (inst ==  Asm::InstID::ADD)
				inst = Asm::InstID::ADDS;
			else if (inst ==  Asm::InstID::SUB)
				inst = Asm::InstID::SUBS;
			else if (inst ==  Asm::InstID::IMUL)
				inst = Asm::InstID::MULS;
//			if (inst ==  Asm::InstID::IDIV)
//				inst = Asm::InstID::DIV;
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			cmd.remove_cmd(i);

			int reg1 = find_unused_reg(i, i, 4);
			int reg2 = find_unused_reg(i, i, 4, VREG_ROOT(reg1));

			if (p2.kind == NodeKind::NONE) {
				// a += b
				_to_register_32(p0, 0, reg1);
				_to_register_32(p1, 0, reg2);
			} else {
				// a = b + c
				_to_register_32(p1, 0, reg1);
				//cmd.set_virtual_reg(reg1, i, cmd.next_cmd_index);
				_to_register_32(p2, 0, reg2);
			}
			insert_cmd(inst, param_vreg(TypeInt32, reg1), param_vreg(TypeInt32, reg1), param_vreg(TypeInt32, reg2));
			_from_register_32(reg1, p0, 0);

			i = cmd.next_cmd_index - 1;
		} else if ((c.inst == Asm::InstID::FADD) or (c.inst == Asm::InstID::FSUB) or (c.inst == Asm::InstID::FMUL) or (c.inst == Asm::InstID::FDIV)) {//or (c.inst == Asm::InstID::SUB) or (c.inst == Asm::InstID::IMUL) /*or (c.inst == Asm::InstID::IDIV)*/ or (c.inst == Asm::InstID::AND) or (c.inst == Asm::InstID::OR)) {
			auto inst = c.inst;
			if (inst ==  Asm::InstID::FADD)
				inst = Asm::InstID::FADDS;
			else if (inst ==  Asm::InstID::FSUB)
				inst = Asm::InstID::FSUBS;
			else if (inst ==  Asm::InstID::FMUL)
				inst = Asm::InstID::FMULS;
			else if (inst ==  Asm::InstID::FDIV)
				inst = Asm::InstID::FDIVS;
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			cmd.remove_cmd(i);

			int sreg1 = cmd.add_virtual_reg(Asm::RegID::S0);
			int sreg2 = cmd.add_virtual_reg(Asm::RegID::S1);

			if (p2.kind == NodeKind::NONE) {
				// a += b
				_to_register_float(p0, 0, sreg1);
				_to_register_float(p1, 0, sreg2);
			} else {
				// a = b + c
				_to_register_float(p1, 0, sreg1);
				_to_register_float(p2, 0, sreg2);
			}

			insert_cmd(inst, param_vreg(TypeInt32, sreg1), param_vreg(TypeInt32, sreg1), param_vreg(TypeInt32, sreg2));

			_from_register_float(sreg1, p0, 0);

			i = cmd.next_cmd_index - 1;


		} else if (c.inst == Asm::InstID::CMP) {
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			cmd.remove_cmd(i);

			int reg1 = find_unused_reg(i, i, 4);
			int reg2 = find_unused_reg(i, i, 4, VREG_ROOT(reg1));

			if (p0.type->size == 1) {
				_to_register_8(p0, 0, reg1);
				_to_register_8(p1, 0, reg2);
			} else {
				_to_register_32(p0, 0, reg1);
				_to_register_32(p1, 0, reg2);
			}

			insert_cmd(Asm::InstID::CMP, param_vreg(p0.type, reg1), param_vreg(p1.type, reg2));
			i = cmd.next_cmd_index - 1;
		} else if ((c.inst == Asm::InstID::SETZ) or (c.inst == Asm::InstID::SETNZ) or (c.inst == Asm::InstID::SETNLE) or (c.inst == Asm::InstID::SETNL) or (c.inst == Asm::InstID::SETLE) or (c.inst == Asm::InstID::SETL)) {
			auto p0 = c.p[0];
			auto inst = c.inst;
			cmd.remove_cmd(i);
			int reg = cmd.add_virtual_reg(Asm::RegID::R0);
			insert_cmd(Asm::InstID::MOV, param_vreg(p0.type, reg), param_imm(TypeBool, 1), p_none);
			insert_cmd(Asm::InstID::MOV, param_vreg(p0.type, reg), param_imm(TypeBool, 0), p_none);
			if (inst == Asm::InstID::SETZ) { // ==
				cmd.cmd[cmd.next_cmd_index - 2].cond = Asm::ArmCond::Equal;
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::NotEqual;
			} else if (inst == Asm::InstID::SETNZ) { // !=
				cmd.cmd[cmd.next_cmd_index - 2].cond = Asm::ArmCond::NotEqual;
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::Equal;
			} else if (inst == Asm::InstID::SETNLE) { // >
				cmd.cmd[cmd.next_cmd_index - 2].cond = Asm::ArmCond::GreaterThan;
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::LessEqual;
			} else if (inst == Asm::InstID::SETNL) { // >=
				cmd.cmd[cmd.next_cmd_index - 2].cond = Asm::ArmCond::GreaterEqual;
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::LessThan;
			} else if (inst == Asm::InstID::SETL) { // <
				cmd.cmd[cmd.next_cmd_index - 2].cond = Asm::ArmCond::LessThan;
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::GreaterEqual;
			} else if (inst == Asm::InstID::SETLE) { // <=
				cmd.cmd[cmd.next_cmd_index - 2].cond = Asm::ArmCond::LessEqual;
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::GreaterThan;
			}
			_from_register_8(reg, p0, 0);
			i = cmd.next_cmd_index - 1;
		} else if ((c.inst == Asm::InstID::JMP) or (c.inst == Asm::InstID::JZ) or (c.inst == Asm::InstID::JNZ) or (c.inst == Asm::InstID::JNLE) or (c.inst == Asm::InstID::JNL) or (c.inst == Asm::InstID::JLE) or (c.inst == Asm::InstID::JL)) {
			auto p0 = c.p[0];
			auto inst = c.inst;
			cmd.remove_cmd(i);
			insert_cmd(Asm::InstID::B, p0);
			if (inst == Asm::InstID::JZ) { // ==
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::Equal;
			} else if (inst == Asm::InstID::JNZ) { // !=
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::NotEqual;
			} else if (inst == Asm::InstID::JNLE) { // >
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::GreaterThan;
			} else if (inst == Asm::InstID::JNL) { // >=
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::GreaterEqual;
			} else if (inst == Asm::InstID::JL) { // <
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::LessThan;
			} else if (inst == Asm::InstID::JLE) { // <=
				cmd.cmd[cmd.next_cmd_index - 1].cond = Asm::ArmCond::LessEqual;
			}
			i = cmd.next_cmd_index - 1;
		} else if (c.inst == Asm::InstID::LEA) {
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			cmd.remove_cmd(i);

			int reg = _reference_to_register_32(p1);
			_from_register_32(reg, p0, 0);

			i = cmd.next_cmd_index - 1;
		} else if (c.inst == Asm::InstID::PUSH) {
			func_params.add(c.p[0]);
			cmd.remove_cmd(i);
			i --;
		} else if (c.inst == Asm::InstID::CALL) {

			if (c.p[1].type == TypeFunctionCodeRef) {
				//do_error("indirect call...");
				auto fp = c.p[1];
				auto ret = c.p[0];
				cmd.remove_cmd(i);
				cmd.next_cmd_target(i);
				add_pointer_call(fp, func_params, ret);
//			} else if (is_typed_function_pointer(c.p[1].type)) {
//				do_error("BACKEND: POINTER CALL");
			} else {
				//func_params.add(c.p[0]);
				auto *f = ((Function*)c.p[1].p);
				auto ret = c.p[0];
				cmd.remove_cmd(i);
				cmd.next_cmd_target(i);
				add_function_call(f, func_params, ret);
			}
			func_params.clear();
			i = cmd.next_cmd_index - 1;
		} else if (c.inst == Asm::InstID::RET) {
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			if (stack_max_size > 0) {
				cmd.next_cmd_target(i ++);
				insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::R13), param_preg(TypePointer, Asm::RegID::R13), param_imm(TypeInt32, stack_max_size));
			}
			cmd.next_cmd_target(i);
			insert_cmd(Asm::InstID::LDMIA, param_preg(TypePointer, Asm::RegID::R13), param_imm(TypeInt32, 0xaff0)); // {r4,r5,r6,r7,r8,r9,r10,r11,r13,r15}
		} else {
			do_error("unhandled:  " + c.str(serializer));
		}
	}
}

int BackendARM::_reference_to_register_32(const SerialNodeParam &p, const Class *type) {
	if (!type)
		type = module->tree->type_ref(p.type, -1);

	int reg = find_unused_reg(cmd.next_cmd_index, cmd.next_cmd_index, 4);

	if (p.kind == NodeKind::VAR_LOCAL) {
		// TODO: simplify for offset=0
		auto var = (Variable*)p.p;
		_immediate_to_register_32(var->_offset, reg);
		insert_cmd(Asm::InstID::ADD, param_vreg(TypeInt32, reg), param_vreg(TypeInt32, reg), param_preg(TypeInt32, Asm::RegID::R13));
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		_immediate_to_register_32(p.p, reg);
		insert_cmd(Asm::InstID::ADD, param_vreg(TypeInt32, reg), param_vreg(TypeInt32, reg), param_preg(TypeInt32, Asm::RegID::R13));
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		_immediate_to_register_32((int_p)((char*)p.p + p.shift), reg);
	} else {
		do_error("lea source not handled: " + kind2str(p.kind));
	}



	/*if ((param.kind == NodeKind::IMMEDIATE) or (param.kind == NodeKind::MEMORY)) {
		ret.kind = NodeKind::IMMEDIATE;
		if (param.shift > 0)
			msg_error("Serializer: immediade/mem + shift?!?!?");
		ret.p = param.p;
	} else if (param.kind == NodeKind::DEREF_VAR_TEMP) {
		ret = param;
		ret.kind = NodeKind::VAR_TEMP; // FIXME why was it param.kind ?!?!?
	} else {
		ret = cmd._add_temp(type);
		insert_cmd(Asm::InstID::LEA, ret, param);
	}*/
	return reg;
}

[[maybe_unused]]
static bool arm_type_uses_int_register(const Class *t) {
	return (t == TypeInt32) /*or (t == TypeInt64)*/ or (t == TypeInt8) or (t == TypeBool) or t->is_enum() or t->is_some_pointer();
}

int BackendARM::fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) {
	const Class *type = ret.type;
	if (!type)
		type = TypeVoid;

	// grow stack (down) for local variables of the calling function
//	insert_cmd(- cur_func->_VarSize - LocalOffset - 8);
	int64 push_size = 0;

	Array<SerialNodeParam> params = _params;

	// instance as first parameter
	//if (instance.type)
	//	params.insert(instance, 0);

	int max_reg_params = 4;
	int reg_param_offset = 0;
	if (type->uses_return_by_memory()) {
		max_reg_params = 3;
		reg_param_offset = 1;
	}


	// map params...
	Array<SerialNodeParam> reg_param;
	Array<SerialNodeParam> stack_param;
	Array<SerialNodeParam> float_param;
	for (SerialNodeParam &p: params) {
		if ((p.type == TypeInt32) /*or (p.type == TypeInt64)*/ or (p.type == TypeInt8) or (p.type == TypeBool) or p.type->is_some_pointer()) {
			if (reg_param.num < max_reg_params) {
				reg_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else if (p.type == TypeFloat32 /*or (p.type == TypeFloat64)*/) {
			if (float_param.num < 8) {
				float_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else
			do_error("parameter type currently not supported: " + p.type->name);
	}

	// push parameters onto stack
/*	push_size = 4 * stack_param.num;
	if (push_size > 127)
		insert_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegID::RSP), param_const(TypeInt32, (void*)push_size));
	else if (push_size > 0)
		insert_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegID::RSP), param_const(TypeInt8, (void*)push_size));
	foreachb(SerialCommandParam &p, stack_param)
		insert_cmd(Asm::inst_push, p);
	max_push_size = max(max_push_size, push_size);*/

	// s0-7
	foreachib(auto &p, float_param, i) {
		auto reg = Asm::s_reg(i);
		/*if (p.type == TypeFloat64)
			insert_cmd(Asm::inst_movsd, param_reg(TypeReg128, reg), p);
		else*/
		_to_register_float(p, 0, cmd.add_virtual_reg(reg));
			//insert_cmd(Asm::InstID::FLDS, param_preg(TypeFloat32, reg), p);
	}

	// return as _very_ first parameter
	if (type->uses_return_by_memory()) {
		int reg = _reference_to_register_32(ret);
		cmd.set_virtual_reg(reg, cmd.next_cmd_index - 1, -100); // -> call
	}

	// r0, r1, r2, r3
	foreachib(auto &p, reg_param, i) {
		int v = cmd.add_virtual_reg(Asm::r_reg(i + reg_param_offset));
		_to_register_32(p, 0, v);
		//insert_cmd(Asm::InstID::MOV, param_vreg(p.type, v), p);
		cmd.set_virtual_reg(v, cmd.next_cmd_index - 1, -100); // -> call
	}

	// extend reg channels to call
	for (VirtualRegister &r: cmd.virtual_reg)
		if (r.last == -100)
			r.last = cmd.next_cmd_index;

	return push_size;
}

void BackendARM::fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	const Class *type = ret.type;
	if (!type)
		return;

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and (!type->uses_return_by_memory())) {
		if (type == TypeFloat32) {
			int sreg = cmd.add_virtual_reg(Asm::RegID::S0);
			_from_register_float(sreg, ret, 0);
		//else if (type == TypeFloat64)
			//insert_cmd(Asm::InstID::MOVSD, ret, param_preg(TypeReg128, Asm::RegID::XMM0));
		} else if ((type->size == 1) or (type->size == 4)) {
			int v = cmd.add_virtual_reg(Asm::RegID::R0);
			_from_register_32(v, ret, 0);
			cmd.set_virtual_reg(v, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		} else {
			do_error("unhandled function value receiving... " + type->long_name());
			int v = cmd.add_virtual_reg(Asm::RegID::R0);
			insert_cmd(Asm::InstID::MOV, ret, param_vreg(TypeReg32, v));
			cmd.set_virtual_reg(v, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		}
	}
}

static bool reachable_arm(int64 a, void *b) {
	return (abs((int_p)a - (int_p)b) < 30000000);
}

void BackendARM::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;
	int push_size = fc_begin(params, ret, f->is_static());

	if ((f->owner() == module->tree) and (!f->is_extern())) {
		insert_cmd(Asm::InstID::CALL, param_label(TypePointer, f->_label));
	} else {
		if (f->address == 0)
			module->do_error_link("could not link function " + f->long_name());
		if (reachable_arm(f->address, this->module->opcode)) {
			insert_cmd(Asm::InstID::CALL, param_imm(TypePointer, f->address)); // the actual call
			// function pointer will be shifted later...
		} else {

			// TODO FIXME
			// really find a usable register...

			int v = cmd.add_virtual_reg(Asm::RegID::R4);//find_unused_reg(cmd.next_cmd_index-1, cmd.next_cmd_index-1, 4);
			_to_register_32(param_lookup(TypePointer, add_global_ref((void*)(int_p)f->address)), 0, v);
			//insert_cmd(Asm::InstID::MOV, param_vreg(TypePointer, v), param_lookup(TypePointer, add_global_ref(f->address)));
			insert_cmd(Asm::InstID::CALL, param_vreg(TypePointer, v));
			cmd.set_virtual_reg(v, cmd.next_cmd_index-2, cmd.next_cmd_index-1);
		}
	}

	fc_end(push_size, params, ret);
}


void BackendARM::add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;

	int push_size = fc_begin(params, ret, true);

	int reg = _to_register_32(fp, 0);
	insert_cmd(Asm::InstID::CALL, param_vreg(TypePointer, reg));

	fc_end(push_size, params, ret);
}

void BackendARM::add_function_intro_params(Function *f) {
	// return, instance, params
	Array<Variable*> param;
	if (f->effective_return_type->uses_return_by_memory()) {
		for (Variable *v: weak(f->var))
			if (v->name == Identifier::RETURN_VAR) {
				param.add(v);
				break;
			}
	}
	if (!f->is_static()) {
		for (Variable *v: weak(f->var))
			if (v->name == Identifier::SELF) {
				param.add(v);
				break;
			}
	}
	for (int i=0;i<f->num_params;i++)
		param.add(f->var[i].get());

	// map params...
	Array<Variable*> reg_param;
	Array<Variable*> stack_param;
	Array<Variable*> float_param;
	for (Variable *p: param) {
		if ((p->type == TypeInt32) or (p->type == TypeInt8) or (p->type == TypeBool) or p->type->is_some_pointer()) {
			if (reg_param.num < 4) {
				reg_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else if (p->type == TypeFloat32) {
			if (float_param.num < 8) {
				float_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else {
			do_error("parameter type currently not supported: " + p->type->name);
		}
	}

	// s0-7
	foreachib(Variable *p, float_param, i) {
		int reg = cmd.add_virtual_reg(Asm::s_reg(i));
		_from_register_float(reg, param_local(p->type, p->_offset), 0);
	}

	// rdi, rsi,rdx, rcx, r8, r9
	Asm::RegID param_regs[4] = {Asm::RegID::R0, Asm::RegID::R1, Asm::RegID::R2, Asm::RegID::R3};
	foreachib(Variable *p, reg_param, i) {
		int reg = cmd.add_virtual_reg(param_regs[i]);
		_from_register_32(reg, param_local(p->type, p->_offset), 0);
		cmd.set_virtual_reg(reg, cmd.cmd.num - 1, cmd.cmd.num - 1);
	}

	// get parameters from stack
	foreachb([[maybe_unused]] Variable *p, stack_param) {
		do_error("func with stack...");
		/*int s = 8;
		cmd.add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}

int BackendARM::add_global_ref(void *p) {
	foreachi(GlobalRef &g, global_refs, i)
		if (g.p == p)
			return i;
	GlobalRef g;
	g.p = p;
	g.label = -1;
	global_refs.add(g);
	return global_refs.num - 1;
}

void BackendARM::do_mapping() {

	map_remaining_temp_vars_to_stack();
	stack_max_size = mem_align(stack_max_size, config.target.stack_frame_align);

	serializer->cmd_list_out("map:e", "post var reg");
}

namespace armhelper {

inline void try_map_param_to_stack(SerialNodeParam &p, int v, SerialNodeParam &stackvar) {
	if ((p.kind == NodeKind::VAR_TEMP) and (p.p == v)) {
		p.kind = NodeKind::LOCAL_MEMORY;//stackvar.kind;
		p.p = stackvar.p + p.shift;
		p.shift = 0;
	} else if ((p.kind == NodeKind::DEREF_VAR_TEMP) and (p.p == v)) {
		p.kind = NodeKind::DEREF_LOCAL_MEMORY;
		p.p = stackvar.p;
	}
}

}

void BackendARM::map_remaining_temp_vars_to_stack() {
	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		SerialNodeParam stackvar;
		serializer->add_stack_var(cmd.temp_var[i], stackvar);
		stack_max_size = max((int64)stack_max_size, stackvar.p + stackvar.type->size);
		for (int j=0;j<cmd.cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				armhelper::try_map_param_to_stack(cmd.cmd[j].p[k], i, stackvar);
		}
		cmd.remove_temp_var(i);
	}
}

Asm::InstructionParam BackendARM::prepare_param(Asm::InstID inst, SerialNodeParam &p) {
	if (p.kind == NodeKind::NONE) {
		return Asm::param_none;
	} else if (p.kind == NodeKind::LABEL) {
		return Asm::param_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::DEREF_LABEL) {
		return Asm::param_deref_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::REGISTER) {
		if (p.shift > 0)
			do_error("prepare_param: reg + shift");
		return Asm::param_reg(p.as_reg());
		//param_size = p.type->size;
	} else if (p.kind == NodeKind::DEREF_REGISTER) {
		if (p.shift != 0)
			return Asm::param_deref_reg_shift(p.as_reg(), p.shift, p.type->size);
		else
			return Asm::param_deref_reg(p.as_reg(), p.type->size);
	} else if (p.kind == NodeKind::MEMORY) {
		int size = p.type->size;
		// compiler self-test
		if ((size != 1) and (size != 2) and (size != 4) and (size != 8))
			do_error("prepare_param: evil global of type " + p.type->name);
		return Asm::param_deref_imm(p.p + p.shift, size);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		if (config.target.instruction_set == Asm::InstructionSet::ARM64)
			return Asm::param_deref_reg_shift(Asm::RegID::R31, p.p + p.shift, p.type->size);
		else
			return Asm::param_deref_reg_shift(Asm::RegID::R13, p.p + p.shift, p.type->size);
		//if ((param_size != 1) and (param_size != 2) and (param_size != 4) and (param_size != 8))
		//	param_size = -1; // lea doesn't need size...
			//s->DoErrorInternal("prepare_param: evil local of type " + p.type->name);
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		bool imm_allowed = Asm::get_instruction_allow_const(inst);
		if ((imm_allowed) and (p.type->is_pointer_raw())) {
			return Asm::param_imm(*(int_p*)(p.p + p.shift), p.type->size);
		} else if ((p.type->size <= 4) and (imm_allowed)) {
			return Asm::param_imm(*(int*)(p.p + p.shift), p.type->size);
		} else {
			return Asm::param_deref_imm(p.p + p.shift, p.type->size);
		}
	} else if (p.kind == NodeKind::IMMEDIATE) {
		if (p.shift > 0)
			do_error("prepare_param: immediate + shift");
		return Asm::param_imm(p.p, p.type->size);
	} else {
		do_error("prepare_param: unexpected param..." + p.str(serializer));
	}
	return Asm::param_none;
}


void BackendARM::assemble_cmd_arm(SerialNode &c) {
	// translate parameters
	auto p1 = prepare_param(c.inst, c.p[0]);
	auto p2 = prepare_param(c.inst, c.p[1]);
	auto p3 = prepare_param(c.inst, c.p[2]);

	// assemble instruction
	//list->current_line = c.
	list->add_arm(c.cond, c.inst, p1, p2, p3);
}


void BackendARM::add_function_intro_frame(int stack_alloc_size) {
	cmd.next_cmd_target(0);
	cmd.add_cmd(Asm::InstID::STMDB, param_preg(TypePointer, Asm::RegID::R13), param_imm(TypeInt32, 0x6ff0)); // {r4,r5,r6,r7,r8,r9,r10,r11,r13,r14}
	if (stack_max_size > 0) {
		cmd.next_cmd_target(1);
		cmd.add_cmd(Asm::InstID::MOV, param_preg(TypePointer, Asm::RegID::R11), param_preg(TypePointer, Asm::RegID::R13));
		cmd.next_cmd_target(2);
		cmd.add_cmd(Asm::InstID::SUB, param_preg(TypePointer, Asm::RegID::R13), param_preg(TypePointer, Asm::RegID::R13), param_imm(TypeInt32, stack_max_size));
	}
}

void BackendARM::assemble() {
	// intro + allocate stack memory

	foreachi(GlobalRef &g, global_refs, i) {
		g.label = list->create_label(format("_kaba_ref_%d_%d", cur_func_index, i));
		list->insert_location_label(g.label);
		list->add2(Asm::InstID::DD, Asm::param_imm((int_p)g.p, 4));
	}

	list->insert_location_label(cur_func->_label);

	if (!flags_has(cur_func->flags, Flags::NOFRAME))
		add_function_intro_frame(stack_max_size);

//	do_error("new ARM assemble() not yet implemented");
	for (int i=0;i<cmd.cmd.num;i++) {

		if (cmd.cmd[i].inst == Asm::InstID::LABEL) {
			list->insert_location_label(cmd.cmd[i].p[0].p);
		} else if (cmd.cmd[i].inst == Asm::InstID::ASM) {
			add_asm_block(cmd.cmd[i].p[0].p);
		} else {
			assemble_cmd_arm(cmd.cmd[i]);
		}
	}
	list->add2(Asm::InstID::ALIGN_OPCODE);
}



}


#undef reg_s0
#undef reg_s1
#undef VREG_ROOT

