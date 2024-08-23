//
// Created by Michael Ankele on 2024-06-23.
//

#include "BackendArm64.h"
#include "Serializer.h"
#include "../asm/asm.h"
#include "../../os/msg.h"

namespace Asm{
	extern Asm::RegID r_reg(int i);
	extern Asm::RegID w_reg(int i);
	extern Asm::RegID s_reg(int i);
};

#define VREG_ROOT(r) cmd.virtual_reg[r].reg_root

namespace kaba {

BackendArm64::BackendArm64(Serializer* serializer) : BackendARM(serializer) {
	map_reg_root = {Asm::RegRoot::R0, Asm::RegRoot::R1, Asm::RegRoot::R2, Asm::RegRoot::R3, Asm::RegRoot::R4, Asm::RegRoot::R5, Asm::RegRoot::R6, Asm::RegRoot::R7};
}

void BackendArm64::process(Function *f, int index) {
	cur_func = f;
	cur_func_index = index;
	//call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;

	do_mapping();
	correct();
}

void BackendArm64::add_function_intro_params(Function *f) {
	// return, instance, params
	Array<Variable*> param;

	for (int i=0;i<f->num_params;i++)
		param.add(f->var[i].get());

	// map params...
	Array<Variable*> reg_param;
	Array<Variable*> stack_param;
	Array<Variable*> float_param;
	for (Variable *p: param) {
		if ((p->type == TypeInt32) or (p->type == TypeInt64) or (p->type == TypeInt8) or (p->type == TypeUInt8) or (p->type == TypeBool) or p->type->is_enum() or p->type->is_some_pointer()) {
			if (reg_param.num < 8) {
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
			do_error("parameter type currently not supported 2: " + p->type->name);
		}
	}

	// s0-7
	foreachib(Variable *p, float_param, i) {
		int reg = cmd.add_virtual_reg(Asm::s_reg(i));
		_from_register_float(reg, param_local(p->type, p->_offset));
	}

	// r0-7
	foreachib(Variable *p, reg_param, i) {
		int reg = cmd.add_virtual_reg(Asm::r_reg(i));
		_from_register(reg, param_local(p->type, p->_offset));
		cmd.set_virtual_reg(reg, cmd.cmd.num - 1, cmd.cmd.num - 1);
	}

	// get parameters from stack
	foreachb([[maybe_unused]] Variable *p, stack_param) {
		do_error("func with stack...");
		/*int s = 8;
		cmd.add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}

	if (f->effective_return_type->uses_return_by_memory()) {
		for (Variable *v: weak(f->var))
			if (v->name == Identifier::RETURN_VAR) {
				int reg = cmd.add_virtual_reg(Asm::r_reg(8));
				_from_register(reg, param_local(v->type, v->_offset));
				cmd.set_virtual_reg(reg, cmd.cmd.num - 1, cmd.cmd.num - 1);
				break;
			}
	}
}

void BackendArm64::correct() {
	// instead of in-place editing, let's create a backup and a new list from that
	pre_cmd.ser = cmd.ser;
	cmd.cmd.exchange(pre_cmd.cmd);
	// no vregs yet, but temp vars
	pre_cmd.temp_var = cmd.temp_var;
	for (auto &t: cmd.temp_var)
		t.first = t.last = -1;

	cmd.next_cmd_target(0);

	serializer->cmd_list_out("x:a", "paramtrafo");

	correct_implement_commands();

	cmd.next_cmd_target(0);
	add_function_intro_params(cur_func);
	serializer->cmd_list_out("x:b", "post paramtrafo");
}



	// TODO better use int64?
void BackendArm64::_immediate_to_register_32(int val, int r) {
	// 16bit chunks
	bool first = true;
	unsigned int uval = (unsigned)val;
	unsigned int mask = 0x0000ffff;
	for (int k=0; k<2; k++) {
		if ((val & mask) or (val == 0 and k == 0)) {
			if (first) {
				insert_cmd(Asm::InstID::MOV, param_vreg_auto(TypeInt32, r), param_imm(TypeInt32, (uval & mask)));
			} else {
				insert_cmd(Asm::InstID::MOV, param_preg(TypeInt32, Asm::RegID::W5), param_imm(TypeInt32, (uval & mask)));
				insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt32, r), param_vreg_auto(TypeInt32, r), param_preg(TypeInt32, Asm::RegID::W5));
			}
			first = false;
		}
		mask = mask << 16;
	}
}

void BackendArm64::_immediate_to_register(int64 val, int size, int vreg) {
	if (size == 1) {
		_immediate_to_register_8(val, vreg);
	} else if (size == 4) {
		_immediate_to_register_32(val, vreg);
	} else if (size == 8) {
		_immediate_to_register_64(val, vreg);
	}
}

void BackendArm64::_immediate_to_register_64(int64 val, int r) {
	// 16bit chunks
	bool first = true;
	int64 mask = 0x000000000000ffff;
	for (int k=0; k<4; k++) {
		if ((val & mask) or (val == 0 and k == 0)) {
			if (first) {
				insert_cmd(Asm::InstID::MOV, param_vreg_auto(TypeInt64, r), param_imm(TypeInt64, (val & mask)));
			} else {
				int temp = vreg_alloc(8);
				insert_cmd(Asm::InstID::MOV, param_vreg_auto(TypeInt64, temp), param_imm(TypeInt64, (val & mask)));
				insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, r), param_vreg_auto(TypeInt64, r), param_vreg_auto(TypeInt64, temp));
				vreg_free(temp);
			}
			first = false;
		}
		mask = mask << 16;
	}
}

SerialNodeParam BackendArm64::param_vreg_auto(const Class *type, int vreg) {
	auto preg = reg_resize(cmd.virtual_reg[vreg].reg, max((int)type->size, 4));
	return {NodeKind::REGISTER, (int)preg, vreg, type, 0};
}

void BackendArm64::_local_to_register(int offset, int size, int vreg) {
	if (offset >= 0x1000 * size) {
		int reg2 = vreg_alloc(8);
		_immediate_to_register_64(offset, reg2);
		insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg2), param_vreg_auto(TypeInt64, reg2), param_preg(TypeInt64, Asm::RegID::R31));
		if (size == 1)
			insert_cmd(Asm::InstID::LDRB, param_vreg_auto(TypeInt8, vreg), param_deref_vreg(TypeInt8, reg2));
		else if (size == 4)
			insert_cmd(Asm::InstID::LDR, param_vreg_auto(TypeInt32, vreg), param_deref_vreg(TypeInt32, reg2));
		else //if (size == 8)
			insert_cmd(Asm::InstID::LDR, param_vreg_auto(TypeInt64, vreg), param_deref_vreg(TypeInt64, reg2));
		vreg_free(reg2);
	} else {
		if (size == 1)
			insert_cmd(Asm::InstID::LDRB, param_vreg_auto(TypeInt8, vreg), param_local(TypeInt8, offset));
		else if (size == 4)
			insert_cmd(Asm::InstID::LDR, param_vreg_auto(TypeInt32, vreg), param_local(TypeInt32, offset));
		else //if (size == 8)
			insert_cmd(Asm::InstID::LDR, param_vreg_auto(TypeInt64, vreg), param_local(TypeInt64, offset));
	}
}

int BackendArm64::_to_register(const SerialNodeParam &p, int force_register) {
	//if (p.kind == NodeKind::REGISTER)
	//	return p.p;

	int size = p.type->size;

	int reg = force_register;
	if (reg < 0)
		reg = vreg_alloc(8);

	auto global_mem = [this, size, reg, &p] (int64 address) {
		int reg2 = vreg_alloc(8);
		_immediate_to_register_64(address, reg2);
		if (size == 1)
			insert_cmd(Asm::InstID::LDRB, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		else
			insert_cmd(Asm::InstID::LDR, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		vreg_free(reg2);
	};

	if (p.kind == NodeKind::CONSTANT) {
		auto cc = (Constant*)p.p;
		_immediate_to_register(*((int64*)((char*)cc->p() + p.shift)), size, reg);
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		_immediate_to_register(*((int64*)((char*)p.p + p.shift)), size, reg);
	} else if (p.kind == NodeKind::IMMEDIATE) {
		if (p.shift != 0)
			do_error("immediate + offset");
		_immediate_to_register(p.p, size, reg);
	} else if (p.kind == NodeKind::VAR_LOCAL) {
		auto var2 = (Variable*)p.p;
		_local_to_register(var2->_offset + p.shift, size, reg);
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		int reg2 = vreg_alloc(8);
		_local_to_register(p.p, 8, reg2);
		if (p.shift != 0)
			insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg2), param_vreg_auto(TypeInt64, reg2), param_imm(TypeInt64, p.shift));
		if (size == 1)
			insert_cmd(Asm::InstID::LDRB, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		else
			insert_cmd(Asm::InstID::LDR, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		vreg_free(reg2);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		_local_to_register(p.p + p.shift, size, reg);
	} else if (p.kind == NodeKind::GLOBAL_LOOKUP) {
		// TODO really use global ref
		if (p.shift != 0)
			do_error("global lookup + offset");
		_immediate_to_register((int64)(int_p)global_refs[p.p].p, size, reg);
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		global_mem((int_p)var->memory + p.shift);
	} else if (p.kind == NodeKind::MEMORY) {
		global_mem(p.p + p.shift);
	} else {
		do_error("evil read source..." + kind2str(p.kind));
	}
	return reg;
}

int BackendArm64::_reference_to_register_64(const SerialNodeParam &p, int force_register, const Class *type) {
	if (!type)
		type = module->tree->type_ref(p.type, -1);

	int reg = force_register;
	if (reg < 0)
		reg = vreg_alloc(8);

	auto local_mem = [this, reg](int64 offset) {
	// nope, this will interpret r31 as 0!
		//		_immediate_to_register_64(var->_offset, reg);
		//		insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg), param_vreg_auto(TypeInt64, reg), param_preg(TypeInt64, Asm::RegID::R31));
	//	if (offset >= 4096)
	//		do_error("local memory only referencable until 4k");
		insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg), param_preg(TypeInt64, Asm::RegID::R31), param_imm(TypeInt64, offset & 0xfff));
		if (offset >= 0x1000)
			insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg), param_vreg_auto(TypeInt64, reg), param_imm(TypeInt64, offset & 0xfff000));
	};

	if (p.kind == NodeKind::VAR_LOCAL) {
		// TODO: simplify for offset=0
		auto var = (Variable*)p.p;
		local_mem(var->_offset);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		local_mem(p.p);
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		_immediate_to_register_64((int_p)((char*)p.p + p.shift), reg);
	} else {
		do_error("lea source not handled: " + kind2str(p.kind));
	}
	return reg;
}

void BackendArm64::_from_register(int reg, const SerialNodeParam &p) {
	int size = p.type->size;

	auto local_mem = [this, reg, size, &p] (int offset) {
		if (offset >= 0x1000 * size) {
			int reg2 = vreg_alloc(8);
			_immediate_to_register_64(offset, reg2);
			insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg2), param_vreg_auto(TypeInt64, reg2), param_preg(TypeInt64, Asm::RegID::R31));
			if (size == 1)
				insert_cmd(Asm::InstID::STRB, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
			else
				insert_cmd(Asm::InstID::STR, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
			vreg_free(reg2);
		} else {
			if (size == 1)
				insert_cmd(Asm::InstID::STRB, param_vreg_auto(p.type, reg), param_local(p.type, offset));
			else
				insert_cmd(Asm::InstID::STR, param_vreg_auto(p.type, reg), param_local(p.type, offset));
		}
	};

	auto global_mem = [this, reg, size, &p](int64 address) {
		int reg2 = vreg_alloc(8);
		_immediate_to_register_64(address, reg2);
		if (size == 1)
			insert_cmd(Asm::InstID::STR, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		else
			insert_cmd(Asm::InstID::STR, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		vreg_free(reg2);
	};

	if (p.kind == NodeKind::VAR_LOCAL) {
		auto var = (Variable*)p.p;
		local_mem(var->_offset + p.shift);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		local_mem(p.p + p.shift);
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		global_mem((int_p)var->memory + p.shift);
	} else if (p.kind == NodeKind::MEMORY) {
		global_mem(p.p + p.shift);
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		int reg2 = vreg_alloc(8);
		_local_to_register(p.p, 8, reg2);
		if (p.shift != 0)
			insert_cmd(Asm::InstID::ADD, param_vreg_auto(TypeInt64, reg2), param_vreg_auto(TypeInt64, reg2), param_imm(TypeInt64, p.shift));
		if (size == 1)
			insert_cmd(Asm::InstID::STRB, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		else
			insert_cmd(Asm::InstID::STR, param_vreg_auto(p.type, reg), param_deref_vreg(p.type, reg2));
		vreg_free(reg2);
	} else {
		do_error("evil write target..." + kind2str(p.kind));
	}
}

void BackendArm64::_immediate_to_register_8(int val, int r) {
	insert_cmd(Asm::InstID::MOV, param_vreg_auto(TypeInt32, r), param_imm(TypeInt32, val & 0xff));
}


int BackendArm64::_to_register_float(const SerialNodeParam &p, int force_register) {
	if (force_register < 0)
		do_error("explicit register needed for float");
	int sreg = force_register;//cmd.add_virtual_reg(Asm::RegID::S1);
	int reg = _to_register(p);
	insert_cmd(Asm::InstID::FMOV, param_vreg_auto(TypeFloat32, sreg), param_vreg_auto(TypeFloat32, reg));
	vreg_free(reg);
	return sreg;
}

void BackendArm64::_from_register_float(int sreg, const SerialNodeParam &p) {
	if (p.kind == NodeKind::VAR_LOCAL) {
		auto var = (Variable*)p.p;
		insert_cmd(Asm::InstID::STR, param_vreg_auto(TypeFloat32, sreg), param_local(TypeFloat32, var->_offset + p.shift));
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		insert_cmd(Asm::InstID::STR, param_vreg_auto(TypeFloat32, sreg), param_local(TypeFloat32, p.p + p.shift));
	} else {
		int reg = vreg_alloc(4);
		insert_cmd(Asm::InstID::FMOV, param_vreg_auto(TypeFloat32, reg), param_vreg_auto(TypeFloat32, sreg));
		_from_register(reg, p);
		vreg_free(reg);
	}
}

bool is_8b_aligned(const SerialNodeParam& p) {
	if (p.kind == NodeKind::REGISTER)
		return true;
	if (p.kind == NodeKind::CONSTANT) {
		auto cc = (Constant*)p.p;
		return (((int_p)cc->p() + p.shift) & 0x07) == 0;
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		return (((int_p)p.p + p.shift) & 0x07) == 0;
	} else if (p.kind == NodeKind::VAR_LOCAL) {
		auto var = (Variable*)p.p;
		return ((var->_offset + p.shift) & 0x07) == 0;
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		return ((p.p + p.shift) & 0x07) == 0;
	} else if (p.kind == NodeKind::VAR_GLOBAL) {
		auto var = (Variable*)p.p;
		return (((int_p)var->memory + p.shift) & 0x07) == 0;
	} else if (p.kind == NodeKind::DEREF_LOCAL_MEMORY) {
		return true; // I guess, we could enforce this with alignment rules... somehow
	}
	return false;
}

void BackendArm64::correct_implement_commands() {

	Array<SerialNodeParam> func_params;

	for (int _i=0; _i<pre_cmd.cmd.num; _i++) {
		auto &c = pre_cmd.cmd[_i];
		cmd.next_cmd_index = cmd.cmd.num;

		//msg_write("CORRECT  " + c.str(serializer));
		if (c.inst == Asm::InstID::LABEL or c.inst == Asm::InstID::ASM)
			insert_cmd(c.inst, c.p[0], c.p[1], c.p[2]); // cmd.cmd.add(c);
		else if (c.inst == Asm::InstID::MOV) {
			int size = c.p[0].type->size;
			auto p0 = c.p[0];
			auto p1 = c.p[1];

			int offset = 0;
			if (is_8b_aligned(p0) and is_8b_aligned(p1))
			while (offset + 8 <= size) {
				int reg = _to_register(param_shift(p1, offset, TypeInt64));
				_from_register(reg, param_shift(p0, offset, TypeInt64));
				vreg_free(reg);
				offset += 8;
			}
			while (offset + 4 <= size) {
				int reg = _to_register(param_shift(p1, offset, TypeInt32));
				_from_register(reg, param_shift(p0, offset, TypeInt32));
				vreg_free(reg);
				offset += 4;
			}
			while (offset < size) {
				int reg = _to_register(param_shift(p1, offset, TypeInt8));
				_from_register(reg, param_shift(p0, offset, TypeInt8));
				vreg_free(reg);
				offset += 1;
			}
		} else if (c.inst == Asm::InstID::MOVSX) {
			int vreg = _to_register(c.p[1]);
			if (c.p[0].type == TypeInt32 and c.p[1].type == TypeInt64) {
				// nothing to do
			} else if (c.p[0].type == TypeInt64 and c.p[1].type == TypeInt32) {
				insert_cmd(Asm::InstID::SXTW, param_vreg_auto(TypeInt64, vreg), param_vreg_auto(TypeInt32, vreg));
			} else if (c.p[0].type == TypeInt32 and c.p[1].type == TypeInt8) {
				insert_cmd(Asm::InstID::SXTB, param_vreg_auto(TypeInt32, vreg), param_vreg_auto(TypeInt8, vreg));
			} else {
				do_error(format("movsx for %s <- %s", c.p[0].type->long_name(), c.p[1].type->long_name()));
			}
			_from_register(vreg, c.p[0]);
			vreg_free(vreg);
		} else if (c.inst == Asm::InstID::MOVZX) {
			// hmmm, might not be enough... i8?!? meh, ok for now
			int vreg1 = vreg_alloc(8);
			int vreg2 = _to_register(c.p[1]);
			insert_cmd(Asm::InstID::MOV, param_vreg_auto(TypeInt64, vreg1), param_vreg_auto(TypeInt64, vreg2));
			_from_register(vreg1, c.p[0]);
			vreg_free(vreg1);
			vreg_free(vreg2);
		} else if ((c.inst == Asm::InstID::ADD) or (c.inst == Asm::InstID::SUB) or (c.inst == Asm::InstID::IMUL) or (c.inst == Asm::InstID::IDIV) or (c.inst == Asm::InstID::AND) or (c.inst == Asm::InstID::OR) or (c.inst == Asm::InstID::XOR) or (c.inst == Asm::InstID::SHL) or (c.inst == Asm::InstID::SHR)) {
			auto inst = c.inst;
			if (inst ==  Asm::InstID::IMUL)
				inst = Asm::InstID::MUL;
			else if (inst ==  Asm::InstID::IDIV)
				inst = Asm::InstID::DIV;
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];

			int size = max((int)p0.type->size, 4); // maximum size

			int vreg1 = vreg_alloc(size);
			int vreg2 = vreg_alloc(size);

			if (p2.kind == NodeKind::NONE) {
				// a += b
				_to_register(p0, vreg1);
				_to_register(p1, vreg2);
			} else {
				// a = b + c
				_to_register(p1, vreg1);
				_to_register(p2, vreg2);
			}
			insert_cmd(inst, param_vreg_auto(p0.type, vreg1), param_vreg_auto(p0.type, vreg1), param_vreg_auto(p0.type, vreg2));
			_from_register(vreg1, p0);
			vreg_free(vreg1);
			vreg_free(vreg2);
		} else if (c.inst == Asm::InstID::MODULO) {
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];

			int size = max((int)p0.type->size, 4); // maximum size

			int vreg1 = vreg_alloc(size);
			int vreg2 = vreg_alloc(size);
			int vreg3 = vreg_alloc(size);
			int vreg4 = vreg_alloc(size);

			if (p2.kind == NodeKind::NONE) {
				// a %= b
				_to_register(p0, vreg1);
				_to_register(p1, vreg2);
			} else {
				// a = b % c
				_to_register(p1, vreg1);
				_to_register(p2, vreg2);
			}
			insert_cmd(Asm::InstID::DIV, param_vreg_auto(p0.type, vreg3), param_vreg_auto(p0.type, vreg1), param_vreg_auto(p0.type, vreg2));
			insert_cmd(Asm::InstID::MUL, param_vreg_auto(p0.type, vreg4), param_vreg_auto(p0.type, vreg3), param_vreg_auto(p0.type, vreg2));
			insert_cmd(Asm::InstID::SUB, param_vreg_auto(p0.type, vreg1), param_vreg_auto(p0.type, vreg1), param_vreg_auto(p0.type, vreg4));
			_from_register(vreg1, p0);
			vreg_free(vreg1);
			vreg_free(vreg2);
			vreg_free(vreg3);
			vreg_free(vreg4);
		} else if ((c.inst == Asm::InstID::FADD) or (c.inst == Asm::InstID::FSUB) or (c.inst == Asm::InstID::FMUL) or (c.inst == Asm::InstID::FDIV)) {//or (c.inst == Asm::InstID::SUB) or (c.inst == Asm::InstID::IMUL) /*or (c.inst == Asm::InstID::IDIV)*/ or (c.inst == Asm::InstID::AND) or (c.inst == Asm::InstID::OR)) {
			auto inst = c.inst;
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];

			int sreg1 = vreg_alloc(4, Asm::RegID::S0);
			int sreg2 = vreg_alloc(4, Asm::RegID::S1);

			if (p2.kind == NodeKind::NONE) {
				// a += b
				_to_register_float(p0, sreg1);
				_to_register_float(p1, sreg2);
			} else {
				// a = b + c
				_to_register_float(p1, sreg1);
				_to_register_float(p2, sreg2);
			}

			insert_cmd(inst, param_vreg_auto(TypeFloat32, sreg1), param_vreg_auto(TypeFloat32, sreg1), param_vreg_auto(TypeFloat32, sreg2));

			_from_register_float(sreg1, p0);

			vreg_free(sreg1);
			vreg_free(sreg2);

		} else if (c.inst == Asm::InstID::UCOMISS) { // fcmp
			auto p0 = c.p[0];
			auto p1 = c.p[1];

			int sreg1 = vreg_alloc(4, Asm::RegID::S0);
			int sreg2 = vreg_alloc(4, Asm::RegID::S1);
			_to_register_float(p0, sreg1);
			_to_register_float(p1, sreg2);

			insert_cmd(Asm::InstID::FCMP, param_vreg_auto(TypeFloat32, sreg1), param_vreg_auto(TypeFloat32, sreg2));

			vreg_free(sreg1);
			vreg_free(sreg2);



		} else if (c.inst == Asm::InstID::CVTSI2SS) {
			// i32 -> f32
			auto vregi = _to_register(c.p[1]);
			int vregf = vreg_alloc(4, Asm::s_reg(0));
			insert_cmd(Asm::InstID::SCVTF, param_vreg(c.p[0].type, vregf), param_vreg_auto(c.p[1].type, vregi));
			_from_register_float(vregf, c.p[0]);
			vreg_free(vregi);
			vreg_free(vregf);
		} else if (c.inst == Asm::InstID::CVTTSS2SI) {
			// f32 -> i32
			auto vregf = vreg_alloc(4, Asm::s_reg(0));
			int vregi = vreg_alloc(4);
			_to_register_float(c.p[1], vregf);
			insert_cmd(Asm::InstID::FCVTZS, param_vreg_auto(c.p[0].type, vregi), param_vreg(c.p[1].type, vregf));
			_from_register(vregi, c.p[0]);
			vreg_free(vregi);
			vreg_free(vregf);

		/*} else if (c.inst == Asm::InstID::CVTSS2SD) {
			// f32 -> f64
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			[[maybe_unused]] int veax = cmd.add_virtual_reg(Asm::RegID::XMM0);
			insert_cmd(Asm::InstID::CVTSS2SD, p_xmm0, p2);
			insert_cmd(Asm::InstID::MOVSD, p1, p_xmm0);
		} else if (c.inst == Asm::InstID::CVTSD2SS) {
			// f64 -> f32
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			[[maybe_unused]] int veax = cmd.add_virtual_reg(Asm::RegID::XMM0);
			insert_cmd(Asm::InstID::CVTSD2SS, p_xmm0, p2);
			insert_cmd(Asm::InstID::MOVSS, p1, p_xmm0);*/

		} else if (c.inst == Asm::InstID::CMP) {
			auto p0 = c.p[0];
			auto p1 = c.p[1];

			int size = max((int)p0.type->size, 4);

			int vreg1 = vreg_alloc(size);
			int vreg2 = vreg_alloc(size);

			_to_register(p0, vreg1);
			_to_register(p1, vreg2);

			insert_cmd(Asm::InstID::SUBS, param_vreg_auto(p0.type, vreg1), param_vreg_auto(p0.type, vreg1), param_vreg_auto(p1.type, vreg2));
			vreg_free(vreg1);
			vreg_free(vreg2);
		} else if ((c.inst == Asm::InstID::SETZ) or (c.inst == Asm::InstID::SETNZ) or (c.inst == Asm::InstID::SETNLE) or (c.inst == Asm::InstID::SETNL) or (c.inst == Asm::InstID::SETLE) or (c.inst == Asm::InstID::SETL) or (c.inst == Asm::InstID::SETNBE) or (c.inst == Asm::InstID::SETNB) or (c.inst == Asm::InstID::SETBE) or (c.inst == Asm::InstID::SETB)) {
			auto p0 = c.p[0];
			auto inst = c.inst;
			int reg = vreg_alloc(4, Asm::RegID::W0);
			Asm::ArmCond cond = Asm::ArmCond::Equal;
			if (inst == Asm::InstID::SETZ) { // ==
				cond = Asm::ArmCond::NotEqual;
			} else if (inst == Asm::InstID::SETNZ) { // !=
				cond = Asm::ArmCond::Equal;
			} else if (inst == Asm::InstID::SETNLE) { // > signed
				cond = Asm::ArmCond::LessEqual;
			} else if (inst == Asm::InstID::SETNL) { // >= signed
				cond = Asm::ArmCond::LessThan;
			} else if (inst == Asm::InstID::SETL) { // < signed
				cond = Asm::ArmCond::GreaterEqual;
			} else if (inst == Asm::InstID::SETLE) { // <= signed
				cond = Asm::ArmCond::GreaterThan;
			} else if (inst == Asm::InstID::SETNBE) { // > unsigned
				cond = Asm::ArmCond::UnsignedLowerSame; // ok
			} else if (inst == Asm::InstID::SETNB) { // >= unsigned
				cond = Asm::ArmCond::CarryClear;
			} else if (inst == Asm::InstID::SETB) { // < unsigned
				cond = Asm::ArmCond::CarrySet;
			} else if (inst == Asm::InstID::SETBE) { // <= unsigned
				cond = Asm::ArmCond::UnsignedHigher;
			}
			insert_cmd(Asm::InstID::CSET, param_vreg_auto(TypeInt32, reg), param_imm(TypeInt32, (int)cond));
			insert_cmd(Asm::InstID::AND, param_vreg_auto(TypeInt32, reg), param_vreg_auto(TypeInt32, reg), param_imm(TypeInt32, 1));
			_from_register(reg, p0);
			vreg_free(reg);
		} else if (c.inst == Asm::InstID::JMP) {
			auto p0 = c.p[0];
			insert_cmd(Asm::InstID::B, p0);
		} else if ((c.inst == Asm::InstID::JZ) or (c.inst == Asm::InstID::JNZ) or (c.inst == Asm::InstID::JNLE) or (c.inst == Asm::InstID::JNL) or (c.inst == Asm::InstID::JLE) or (c.inst == Asm::InstID::JL)) {
			auto p0 = c.p[0];
			auto inst = c.inst;
			Asm::ArmCond cond = Asm::ArmCond::Equal;
			if (inst == Asm::InstID::JZ) { // ==
				cond = Asm::ArmCond::Equal;
			} else if (inst == Asm::InstID::JNZ) { // !=
				cond = Asm::ArmCond::NotEqual;
			} else if (inst == Asm::InstID::JNLE) { // >
				cond = Asm::ArmCond::GreaterThan;
			} else if (inst == Asm::InstID::JNL) { // >=
				cond = Asm::ArmCond::GreaterEqual;
			} else if (inst == Asm::InstID::JL) { // <
				cond = Asm::ArmCond::LessThan;
			} else if (inst == Asm::InstID::JLE) { // <=
				cond = Asm::ArmCond::LessEqual;
			}
			insert_cmd(Asm::InstID::B, p0, param_imm(TypeInt8, (int)cond));
		} else if (c.inst == Asm::InstID::LEA) {
			auto p0 = c.p[0];
			auto p1 = c.p[1];

			int vreg = _reference_to_register_64(p1);
			_from_register(vreg, p0);
			vreg_free(vreg);
		} else if (c.inst == Asm::InstID::PUSH) {
			func_params.add(c.p[0]);
		} else if ((c.inst == Asm::InstID::CALL) or (c.inst == Asm::InstID::CALL_MEMBER)) {
			if (c.p[1].type == TypeFunctionCodeRef) {
				auto fp = c.p[1];
				auto ret = c.p[0];
				add_pointer_call(fp, func_params, ret, (c.inst == Asm::InstID::CALL));
//			} else if (is_typed_function_pointer(c.p[1].type)) {
//				do_error("BACKEND: POINTER CALL");
			} else {
				//func_params.add(c.p[0]);
				auto *f = ((Function*)c.p[1].p);
				auto ret = c.p[0];
				add_function_call(f, func_params, ret);
			}
			func_params.clear();
		} else if (c.inst == Asm::InstID::RET) {
			implement_return(c.p[0]);
		} else {
			do_error("unhandled:  " + c.str(serializer));
		}
	}
}

void BackendArm64::implement_return(const SerialNodeParam &p) {
	Array<int> allocated_vgres;
	if (p.kind != NodeKind::NONE) {
		if (cur_func->effective_return_type->uses_return_by_memory()) {
			do_error("return by memory not implemented yet!");
		} else if (cur_func->effective_return_type->_return_in_float_registers()) {
			for (int i=0; i<p.type->size/4; i++) {
				int vreg = vreg_alloc(4, Asm::s_reg(i));
				_to_register_float(param_shift(p, i*4, TypeFloat32), vreg);
				allocated_vgres.add(vreg);
			}
			// TODO: float64
		} else {
			// store return directly in eax / fpu stack (4 byte)
			int vreg = vreg_alloc(8, Asm::RegID::R0);
			_to_register(p, vreg);
			vreg_free(vreg); // better to free at "ret"
		}
	}
	//if (cur_func->effective_return_type->uses_return_by_memory())
	//	insert_cmd(Asm::InstID::RET, param_imm(TypeReg16, 4));
	//else

	/*if (stack_max_size > 0) {
		insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::R31), param_preg(TypePointer, Asm::RegID::R31), param_imm(TypeInt32, stack_max_size + 8));
	}*/
	if (stack_max_size >= 0x200) {
		cmd.add_cmd(Asm::InstID::LDR, param_preg(TypePointer, Asm::RegID::R29), param_local(TypePointer, stack_max_size));
		cmd.add_cmd(Asm::InstID::LDR, param_preg(TypePointer, Asm::RegID::R30), param_local(TypePointer, stack_max_size + 8));
	} else {
		cmd.add_cmd(Asm::InstID::LDP, param_preg(TypePointer, Asm::RegID::R29), param_preg(TypePointer, Asm::RegID::R30), param_local(TypePointer, stack_max_size));
	}
	if (((stack_max_size + 16) & 0xfff) >= 0)
		insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::R31), param_preg(TypePointer, Asm::RegID::R31), param_imm(TypeInt32, (stack_max_size + 16) & 0xfff));
	if (stack_max_size + 16 >= 0x1000)
		insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::R31), param_preg(TypePointer, Asm::RegID::R31), param_imm(TypeInt32, (stack_max_size + 16) & 0xfff000));

	insert_cmd(Asm::InstID::RET);
}

BackendArm64::CallData BackendArm64::fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) {
	const Class *type = ret.type;
	if (!type)
		type = TypeVoid;

	CallData r;

	// grow stack (down) for local variables of the calling function
//	insert_cmd(- cur_func->_VarSize - LocalOffset - 8);
	r.push_size = 0;

	Array<SerialNodeParam> params = _params;

	// instance as first parameter
	//if (instance.type)
	//	params.insert(instance, 0);

	int max_reg_params = 8;
	int reg_param_offset = 0;

	// map params...
	Array<SerialNodeParam> reg_param;
	Array<SerialNodeParam> stack_param;
	Array<SerialNodeParam> float_param;
	for (SerialNodeParam &p: params) {
		if ((p.type == TypeInt32) or (p.type == TypeInt64) or (p.type == TypeInt8) or (p.type == TypeUInt8) or (p.type == TypeBool) or p.type->is_enum() or p.type->is_some_pointer()) {
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
			do_error("parameter type currently not supported 1: " + p.type->name);
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
		auto preg = Asm::s_reg(i);
		int vreg = vreg_alloc(4, preg);
		/*if (p.type == TypeFloat64)
			insert_cmd(Asm::inst_movsd, param_reg(TypeReg128, vreg), p);
		else*/
		_to_register_float(p, vreg);
		r.allocated_vregs.add(vreg);
	}

	// big return -> r8 (address)
	if (type->uses_return_by_memory()) {
		int vreg = vreg_alloc(8, Asm::RegID::R8);
		_reference_to_register_64(ret, vreg);
		r.allocated_vregs.add(vreg);
	}

	// r0, .., r7
	foreachib(auto &p, reg_param, i) {
		int vreg = vreg_alloc(8, Asm::r_reg(i + reg_param_offset));
		_to_register(p, vreg);
		r.allocated_vregs.add(vreg);
	}

	return r;
}

void BackendArm64::fc_end(const CallData& d, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	const Class *type = ret.type;

	// extend reg channels to call
	for (auto vreg: d.allocated_vregs)
		vreg_free(vreg);

	if (!type)
		return;

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and !type->uses_return_by_memory()) {
		if (type->_return_in_float_registers()) {
			Array<int> vregs;
			for (int i=0; i<type->size/4; i++)
				vregs.add(vreg_alloc(4, Asm::s_reg(i)));
			for (int i=0; i<type->size/4; i++) {
				_from_register_float(vregs[i], param_shift(ret, i*4, TypeFloat32));
				vreg_free(vregs[i]);
			}
		} else {
			int vreg = vreg_alloc(8, Asm::RegID::R0);
			_from_register(vreg, ret);
			cmd.set_virtual_reg(vreg, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
			vreg_free(vreg);
		}
	}
}

static bool reachable_arm64(int64 a, void *b) {
	return (abs((int_p)a - (int_p)b) < 0x4000000);
}

void BackendArm64::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;
	auto call_data = fc_begin(params, ret, f->is_static());


	if (f->address != 0) {
		if (reachable_arm64(f->address, this->module->opcode)) {
			insert_cmd(Asm::InstID::BL, param_imm(TypePointer, f->address)); // the actual call
			// function pointer will be shifted later...
		} else {
			int vreg = _to_register(param_imm(TypeInt64, (int_p)f->address));
			insert_cmd(Asm::InstID::BLR, param_vreg_auto(TypePointer, vreg));
			vreg_free(vreg);
		}
	} else if (f->_label >= 0) {
		insert_cmd(Asm::InstID::BL, param_label(TypePointer, f->_label));
		// TODO far lables?
	} else {
		serializer->do_error_link("could not link function " + f->signature());
	}

	fc_end(call_data, params, ret);
}

void BackendArm64::add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, bool is_static) {
	serializer->call_used = true;
	auto call_data = fc_begin(params, ret, is_static);

	int vreg = _to_register(fp);
	insert_cmd(Asm::InstID::BLR, param_vreg_auto(TypePointer, vreg));
	//mark_regs_busy_at_call(cmd.next_cmd_index - 1);
	vreg_free(vreg);

	fc_end(call_data, params, ret);
}

void BackendArm64::add_function_intro_frame(int stack_alloc_size) {
	cmd.next_cmd_target(0);
	if (((stack_max_size + 16) & 0xfff) > 0)
		insert_cmd(Asm::InstID::SUB, param_preg(TypePointer, Asm::RegID::R31), param_preg(TypePointer, Asm::RegID::R31), param_imm(TypeInt32, (stack_max_size + 16) & 0xfff));
	if ((stack_max_size + 16) >= 0x1000)
		insert_cmd(Asm::InstID::SUB, param_preg(TypePointer, Asm::RegID::R31), param_preg(TypePointer, Asm::RegID::R31), param_imm(TypeInt32, (stack_max_size + 16) & 0xfff000));

	if (stack_max_size >= 0x200) {
		insert_cmd(Asm::InstID::STR, param_preg(TypePointer, Asm::RegID::R29), param_local(TypePointer, stack_max_size));
		insert_cmd(Asm::InstID::STR, param_preg(TypePointer, Asm::RegID::R30), param_local(TypePointer, stack_max_size + 8));
	} else {
		insert_cmd(Asm::InstID::STP, param_preg(TypePointer, Asm::RegID::R29), param_preg(TypePointer, Asm::RegID::R30), param_local(TypePointer, stack_max_size));
	}

	insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::R29), param_preg(TypePointer, Asm::RegID::R31), param_imm(TypeInt32, stack_max_size & 0xfff));
	if (stack_max_size >= 0x1000)
		insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::R29), param_preg(TypePointer, Asm::RegID::R29), param_imm(TypeInt32, stack_max_size & 0xfff000));
}

void BackendArm64::assemble() {
	// intro + allocate stack memory

	/*foreachi(GlobalRef &g, global_refs, i) {
		g.label = list->create_label(format("_kaba_ref_%d_%d", cur_func_index, i));
		list->insert_location_label(g.label);
		list->add2(Asm::InstID::DD, Asm::param_imm((int_p)g.p, 4));
	}*/

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

	//list->add2(Asm::InstID::ALIGN_OPCODE);
}

} // kaba