/*
 * BackendAmd64.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#include "BackendAmd64.h"
#include "serializer.h"
#include "CommandList.h"
#include "SerialNode.h"
#include "../../file/msg.h"

namespace kaba {


BackendAmd64::BackendAmd64(Serializer *s) : BackendX86(s) {

	// eax, ecx, edx
	map_reg_root = {0,1,2};
}

BackendAmd64::~BackendAmd64() {
}

static int trafo_inst_float(int inst, const Class *t) {
	if (t == TypeFloat64) {
		if (inst == Asm::INST_FADD)
			return Asm::INST_ADDSD;
		if (inst == Asm::INST_FSUB)
			return Asm::INST_SUBSD;
		if (inst == Asm::INST_FMUL)
			return Asm::INST_MULSD;
		if (inst == Asm::INST_FDIV)
			return Asm::INST_DIVSD;
	} else {
		if (inst == Asm::INST_FADD)
			return Asm::INST_ADDSS;
		if (inst == Asm::INST_FSUB)
			return Asm::INST_SUBSS;
		if (inst == Asm::INST_FMUL)
			return Asm::INST_MULSS;
		if (inst == Asm::INST_FDIV)
			return Asm::INST_DIVSS;
	}
	return -1;
}

static bool inst_is_arithmetic(int i) {
	if ((i == Asm::INST_IMUL) /*or (i == Asm::INST_IDIV)*/ or (i == Asm::INST_ADD) or (i == Asm::INST_SUB))
		return true;
	if ((i == Asm::INST_AND) or (i == Asm::INST_OR) or (i == Asm::INST_XOR))
		return true;
	return false;
}


void BackendAmd64::implement_return(kaba::SerialNode &c, int i) {
	auto p = c.p[0];
	cmd.remove_cmd(i);
	cmd.next_cmd_target(i);
	if (p.kind != NodeKind::NONE) {
		if (cur_func->effective_return_type->_amd64_allow_pass_in_xmm()) {
			// if ((config.instruction_set == Asm::INSTRUCTION_SET_AMD64) or (config.compile_os)) ???
			//		cmd.add_cmd(Asm::INST_FLD, t);
			if (cur_func->effective_return_type == TypeFloat32) {
				insert_cmd(Asm::INST_MOVSS, p_xmm0, p);
			} else if (cur_func->effective_return_type == TypeFloat64) {
				insert_cmd(Asm::INST_MOVSD, p_xmm0, p);
			} else if (cur_func->effective_return_type->size == 8) {
				// float[2]
				insert_cmd(Asm::INST_MOVLPS, p_xmm0, p);
			} else if (cur_func->effective_return_type->size == 12) {
				// float[3]
				insert_cmd(Asm::INST_MOVLPS, p_xmm0,
						param_shift(p, 0, TypeReg64));
				insert_cmd(Asm::INST_MOVSS, p_xmm1,
						param_shift(p, 8, TypeFloat32));
			} else if (cur_func->effective_return_type->size == 16) {
				// float[4]
				insert_cmd(Asm::INST_MOVLPS, p_xmm0,
						param_shift(p, 0, TypeReg64));
				insert_cmd(Asm::INST_MOVLPS, p_xmm1,
						param_shift(p, 8, TypeReg64));
			} else {
				do_error(
						"...ret xmm "
								+ cur_func->effective_return_type->long_name());
			}
		} else {
			// store return directly in eax / fpu stack (4 byte)
			if (cur_func->effective_return_type->size == 1) {
				int v = cmd.add_virtual_reg(Asm::REG_AL);
				insert_cmd(Asm::INST_MOV,
						param_vreg(cur_func->effective_return_type, v), p);
			} else if (cur_func->effective_return_type->size == 8) {
				int v = cmd.add_virtual_reg(Asm::REG_RAX);
				insert_cmd(Asm::INST_MOV,
						param_vreg(cur_func->effective_return_type, v), p);
			} else {
				int v = cmd.add_virtual_reg(Asm::REG_EAX);
				insert_cmd(Asm::INST_MOV,
						param_vreg(cur_func->effective_return_type, v), p);
			}
		}
	}
	insert_cmd(Asm::INST_LEAVE);
	//if (cur_func->effective_return_type->uses_return_by_memory())
	//	insert_cmd(Asm::INST_RET, param_imm(TypeReg16, 4));
	//else
	insert_cmd(Asm::INST_RET);
}


void BackendAmd64::implement_mov_chunk(kaba::SerialNode &c, int i, int size) {
	auto p1 = c.p[0];
	auto p2 = c.p[1];
	cmd.remove_cmd(i);
	cmd.next_cmd_target(i);
	//msg_error("CORRECT MOV " + p1.type->name);

	for (int j=0; j<size/8; j++)
		insert_cmd(Asm::INST_MOV, param_shift(p1, j * 8, TypeInt64), param_shift(p2, j * 8, TypeInt64));
	for (int j=8*(size/8); j<size; j++)
		insert_cmd(Asm::INST_MOV, param_shift(p1, j, TypeChar), param_shift(p2, j, TypeChar));
}


void BackendAmd64::fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	const Class *type = ret.get_type_save();

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and (!type->uses_return_by_memory())) {
		if (type->_amd64_allow_pass_in_xmm()) {
			if (type == TypeFloat32) {
				insert_cmd(Asm::INST_MOVSS, ret, p_xmm0);
			} else if (type == TypeFloat64) {
				insert_cmd(Asm::INST_MOVSD, ret, p_xmm0);
			} else if (type->size == 8) { // float[2]
				insert_cmd(Asm::INST_MOVLPS, ret, p_xmm0);
			} else if (type->size == 12) { // float[3]
				insert_cmd(Asm::INST_MOVLPS, param_shift(ret, 0, TypeReg64), p_xmm0);
				insert_cmd(Asm::INST_MOVSS, param_shift(ret, 8, TypeFloat32), p_xmm1);
			} else if (type->size == 16) { // float[4]
				// hmm, weird
				insert_cmd(Asm::INST_MOVLPS, param_shift(ret, 0, TypeReg64), p_xmm0);
				//add_cmd(Asm::INST_MOVHPS, param_shift(ret, 8, TypeReg64), p_xmm0);
				insert_cmd(Asm::INST_MOVLPS, param_shift(ret, 8, TypeReg64), p_xmm1);
				//add_cmd(Asm::INST_MOVUPS, ret, p_xmm0);
			} else {
				do_error("xmm return ..." + type->long_name());
			}
		} else if (type->size == 1) {
			int r = cmd.add_virtual_reg(Asm::REG_AL);
			insert_cmd(Asm::INST_MOV, ret, param_vreg(type, r));
			cmd.set_virtual_reg(r, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		} else if (type->size == 4) {
			int r = cmd.add_virtual_reg(Asm::REG_EAX);
			insert_cmd(Asm::INST_MOV, ret, param_vreg(type, r));
			cmd.set_virtual_reg(r, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		} else {
			int r = cmd.add_virtual_reg(Asm::REG_RAX);
			insert_cmd(Asm::INST_MOV, ret, param_vreg(type, r));
			cmd.set_virtual_reg(r, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		}
	}
}

static bool dist_fits_32bit(void *a, void *b) {
	int_p d = (int_p)a - (int_p)b;
	if (d < 0)
		d = -d;
	return (d < 0x70000000);
}

void BackendAmd64::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;
	int push_size = fc_begin(params, ret, f->is_static());

	if (f->address) {
		if (dist_fits_32bit(f->address, script->opcode)) {
			// 32bit call distance
			insert_cmd(Asm::INST_CALL, param_imm(TypeReg32, (int_p)f->address)); // the actual call
			// function pointer will be shifted later...(asm translates to RIP-relative)
		} else {
			// 64bit call distance
			insert_cmd(Asm::INST_MOV, p_rax, param_imm(TypeReg64, (int_p)f->address));
			insert_cmd(Asm::INST_CALL, p_rax);
		}
	} else if (f->_label >= 0) {
		if (f->owner() == script->syntax) {
			// 32bit call distance
			insert_cmd(Asm::INST_CALL, param_marker(TypeInt, f->_label));
		} else {
			// 64bit call distance
			insert_cmd(Asm::INST_MOV, p_rax, param_marker(TypePointer, f->_label));
			insert_cmd(Asm::INST_CALL, p_rax);
		}
	} else {
		serializer->do_error_link("could not link function " + f->signature());
	}
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	fc_end(push_size, params, ret);
}

void BackendAmd64::add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;
	int push_size = fc_begin(params, ret, true);

	insert_cmd(Asm::INST_MOV, p_rax, fp);
	insert_cmd(Asm::INST_CALL, p_rax);
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	fc_end(push_size, params, ret);
}

int BackendAmd64::fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) {
	const Class *type = ret.get_type_save();


	// grow stack (down) for local variables of the calling function
//	add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	int64 push_size = 0;

	auto params = _params;

	// instance as first parameter
/*	if (instance.type)
		params.insert(instance, 0);*/

	// return as _very_ first parameter
	if (type->uses_return_by_memory()) {
		params.insert(insert_reference(ret), 0);
	}

	// map params...
	Array<SerialNodeParam> reg_param;
	Array<SerialNodeParam> stack_param;
	Array<SerialNodeParam> xmm_param;
	for (auto &p: params) {
		if ((p.type == TypeInt) or (p.type == TypeInt64) or (p.type == TypeChar) or (p.type == TypeBool) or p.type->is_some_pointer()) {
			if (reg_param.num < 6) {
				reg_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else if ((p.type == TypeFloat32) or (p.type == TypeFloat64)) {
			if (xmm_param.num < 8) {
				xmm_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else {
			do_error("parameter type currently not supported: " + p.type->name);
		}
	}

	// push parameters onto stack
	push_size = 8 * stack_param.num;
	if (push_size > 127)
		insert_cmd(Asm::INST_ADD, param_preg(TypePointer, Asm::REG_RSP), param_imm(TypeInt, push_size));
	else if (push_size > 0)
		insert_cmd(Asm::INST_ADD, param_preg(TypePointer, Asm::REG_RSP), param_imm(TypeChar, push_size));
	foreachb(SerialNodeParam &p, stack_param) {
		insert_cmd(Asm::INST_MOV, param_preg(p.type, get_reg(0, p.type->size)), p);
		insert_cmd(Asm::INST_PUSH, p_rax);
	}
	max_push_size = max(max_push_size, (int)push_size);

	// xmm0-7
	foreachib(auto &p, xmm_param, i) {
		int reg = Asm::REG_XMM0 + i;
		if (p.type == TypeFloat64)
			insert_cmd(Asm::INST_MOVSD, param_preg(TypeReg128, reg), p);
		else
			insert_cmd(Asm::INST_MOVSS, param_preg(TypeReg128, reg), p);
	}

	func_param_virts = {};

	// rdi, rsi, rdx, rcx, r8, r9
	int param_regs_root[6] = {7, 6, 2, 1, 8, 9};
	foreachib(auto &p, reg_param, i) {
		int root = param_regs_root[i];
		int preg = get_reg(root, p.type->size);
		if (preg >= 0) {
			int v = cmd.add_virtual_reg(preg);
			func_param_virts.add(v);
			insert_cmd(Asm::INST_MOV, param_vreg(p.type, v), p);
		} else {
			// some registers are not 8bit'able
			int v = cmd.add_virtual_reg(get_reg(root, 4));
			func_param_virts.add(v);
			int va = cmd.add_virtual_reg(Asm::REG_EAX);
			insert_cmd(Asm::INST_MOV, param_vreg(p.type, va, Asm::REG_AL), p);
			insert_cmd(Asm::INST_MOV, param_vreg(TypeReg32, v), param_vreg(TypeReg32, va));
		}
	}

	return push_size;
}

void BackendAmd64::add_function_outro(Function *f) {
	insert_cmd(Asm::INST_LEAVE, p_none);
	if (f->effective_return_type->uses_return_by_memory())
		insert_cmd(Asm::INST_RET, param_imm(TypeReg16, 4));
	else
		insert_cmd(Asm::INST_RET, p_none);
}

void BackendAmd64::add_function_intro_params(Function *f) {
	// return, instance, params
	Array<Variable*> param;
	if (f->effective_return_type->uses_return_by_memory()) {
		for (Variable *v: weak(f->var))
			if (v->name == IDENTIFIER_RETURN_VAR) {
				param.add(v);
				break;
			}
	}
	if (!f->is_static()) {
		for (Variable *v: weak(f->var))
			if (v->name == IDENTIFIER_SELF) {
				param.add(v);
				break;
			}
	}
	for (int i=0;i<f->num_params;i++)
		param.add(f->var[i].get());

	// map params...
	Array<Variable*> reg_param;
	Array<Variable*> stack_param;
	Array<Variable*> xmm_param;
	for (Variable *p: param) {
		if ((p->type == TypeInt) or (p->type == TypeChar) or (p->type == TypeBool) or p->type->is_some_pointer()) {
			if (reg_param.num < 6) {
				reg_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else if ((p->type == TypeFloat32) or (p->type == TypeFloat64)) {
			if (xmm_param.num < 8) {
				xmm_param.add(p);
			} else {
				stack_param.add(p);
			}
		} else {
			do_error("parameter type currently not supported: " + p->type->name);
		}
	}

	// xmm0-7
	foreachib(Variable *p, xmm_param, i){
		int reg = Asm::REG_XMM0 + i;
		if (p->type == TypeFloat64)
			insert_cmd(Asm::INST_MOVSD, param_local(p->type, p->_offset), param_preg(TypeReg128, reg));
		else
			insert_cmd(Asm::INST_MOVSS, param_local(p->type, p->_offset), param_preg(TypeReg128, reg));


	}

	// rdi, rsi,rdx, rcx, r8, r9
	int param_regs_root[6] = {7, 6, 2, 1, 8, 9};
	foreachib(Variable *p, reg_param, i) {
		int root = param_regs_root[i];
		int preg = get_reg(root, p->type->size);
		if (preg >= 0) {
			int v = cmd.add_virtual_reg(preg);
			insert_cmd(Asm::INST_MOV, param_local(p->type, p->_offset), param_vreg(p->type, v));
			cmd.set_virtual_reg(v, 0, cmd.next_cmd_index-1);
		} else {
			// some registers are not 8bit'able
			int v = cmd.add_virtual_reg(get_reg(root, 4));
			int va = cmd.add_virtual_reg(Asm::REG_EAX);
			insert_cmd(Asm::INST_MOV, param_vreg(TypeReg32, va), param_vreg(TypeReg32, v));
			insert_cmd(Asm::INST_MOV, param_local(p->type, p->_offset), param_vreg(p->type, va, get_reg(0, p->type->size)));
			cmd.set_virtual_reg(v, 0, cmd.next_cmd_index-2);
			cmd.set_virtual_reg(va, cmd.next_cmd_index-2, cmd.next_cmd_index-1);
		}
	}

	// get parameters from stack
	foreachb(Variable *p, stack_param) {
		do_error("func with stack...");
		/*int s = 8;
		add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}


//#define debug_evil_corrections

static void _test_param_mem(SerialNodeParam &p) {
	//if (p.kind == NodeKind::ADDRESS)
}


void BackendAmd64::process_references() {
	for (int i=0;i<cmd.cmd.num;i++)
		if (cmd.cmd[i].inst == Asm::INST_LEA) {
			if (cmd.cmd[i].p[1].kind == NodeKind::LOCAL_MEMORY) {
				auto p0 = cmd.cmd[i].p[0];
				auto p1 = cmd.cmd[i].p[1];
				cmd.remove_cmd(i);

				int r = cmd.add_virtual_reg(Asm::REG_RAX);
				insert_cmd(Asm::INST_LEA, param_vreg(TypeReg64, r), p1);
				insert_cmd(Asm::INST_MOV, p0, param_vreg(TypeReg64, r));
				cmd.set_virtual_reg(r, i, i+1);
			} else {
				do_error("reference in x86: " + cmd.cmd[i].p[1].str(serializer));
			}
		}
}


void BackendAmd64::add_function_intro_frame(int stack_alloc_size) {
	int_p reg_bp = (config.instruction_set == Asm::InstructionSet::AMD64) ? Asm::REG_RBP : Asm::REG_EBP;
	int_p reg_sp = (config.instruction_set == Asm::InstructionSet::AMD64) ? Asm::REG_RSP : Asm::REG_ESP;
	//int s = config.pointer_size;
	list->add2(Asm::INST_PUSH, Asm::param_reg(reg_bp));
	list->add2(Asm::INST_MOV, Asm::param_reg(reg_bp), Asm::param_reg(reg_sp));
	if (stack_alloc_size > 127){
		list->add2(Asm::INST_SUB, Asm::param_reg(reg_sp), Asm::param_imm(stack_alloc_size, Asm::SIZE_32));
	}else if (stack_alloc_size > 0){
		list->add2(Asm::INST_SUB, Asm::param_reg(reg_sp), Asm::param_imm(stack_alloc_size, Asm::SIZE_8));
	}
}

}
