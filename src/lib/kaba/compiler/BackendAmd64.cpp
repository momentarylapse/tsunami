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
#include "../../os/msg.h"

namespace kaba {


BackendAmd64::BackendAmd64(Serializer *s) : BackendX86(s) {

	// rax, rcx, rdx
	map_reg_root = {Asm::RegRoot::A, Asm::RegRoot::C, Asm::RegRoot::D};

	if (config.abi == Abi::AMD64_WINDOWS) {
		// rcx, rdx, r8, r9
		param_regs_root = {Asm::RegRoot::C, Asm::RegRoot::D, Asm::RegRoot::R8, Asm::RegRoot::R9};
		max_xmm_params = 4;
		// volatile: rax, rcx, rdx, r8-11, xmm0-5 (can override)
		// non-volatile: rbx, rbp, rdi, rsi, rsp, r12-r15, xmm6-15 (keep saved!)
		// https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-160
	} else {
		// rdi, rsi, rdx, rcx, r8, r9
		param_regs_root = {Asm::RegRoot::DI, Asm::RegRoot::SI, Asm::RegRoot::D, Asm::RegRoot::C, Asm::RegRoot::R8, Asm::RegRoot::R9};
		max_xmm_params = 8;
		// non-volatile: rbx, rsp, rbp, r12-r15
		// volatile: rest
	}
}

BackendAmd64::~BackendAmd64() {
}

[[maybe_unused]] static Asm::InstID trafo_inst_float(Asm::InstID inst, const Class *t) {
	if (t == TypeFloat64) {
		if (inst == Asm::InstID::FADD)
			return Asm::InstID::ADDSD;
		if (inst == Asm::InstID::FSUB)
			return Asm::InstID::SUBSD;
		if (inst == Asm::InstID::FMUL)
			return Asm::InstID::MULSD;
		if (inst == Asm::InstID::FDIV)
			return Asm::InstID::DIVSD;
	} else {
		if (inst == Asm::InstID::FADD)
			return Asm::InstID::ADDSS;
		if (inst == Asm::InstID::FSUB)
			return Asm::InstID::SUBSS;
		if (inst == Asm::InstID::FMUL)
			return Asm::InstID::MULSS;
		if (inst == Asm::InstID::FDIV)
			return Asm::InstID::DIVSS;
	}
	return Asm::InstID::INVALID;
}

[[maybe_unused]] static bool inst_is_arithmetic(Asm::InstID i) {
	if ((i == Asm::InstID::IMUL) /*or (i == Asm::InstID::IDIV)*/ or (i == Asm::InstID::ADD) or (i == Asm::InstID::SUB))
		return true;
	if ((i == Asm::InstID::AND) or (i == Asm::InstID::OR) or (i == Asm::InstID::XOR))
		return true;
	return false;
}


void BackendAmd64::implement_return(const SerialNodeParam &p) {
	if (p.kind != NodeKind::NONE) {
		if (cur_func->effective_return_type->_amd64_allow_pass_in_xmm()) {
			// if ((config.instruction_set == Asm::INSTRUCTION_SET_AMD64) or (config.compile_os)) ???
			//		cmd.add_cmd(Asm::InstID::FLD, t);
			if (cur_func->effective_return_type == TypeFloat32) {
				insert_cmd(Asm::InstID::MOVSS, p_xmm0, p);
			} else if (cur_func->effective_return_type == TypeFloat64) {
				insert_cmd(Asm::InstID::MOVSD, p_xmm0, p);
			} else if (cur_func->effective_return_type->size == 8) {
				// float[2]
				insert_cmd(Asm::InstID::MOVLPS, p_xmm0, p);
			} else if (cur_func->effective_return_type->size == 12) {
				// float[3]
				insert_cmd(Asm::InstID::MOVLPS, p_xmm0,
						param_shift(p, 0, TypeReg64));
				insert_cmd(Asm::InstID::MOVSS, p_xmm1,
						param_shift(p, 8, TypeFloat32));
			} else if (cur_func->effective_return_type->size == 16) {
				// float[4]
				insert_cmd(Asm::InstID::MOVLPS, p_xmm0,
						param_shift(p, 0, TypeReg64));
				insert_cmd(Asm::InstID::MOVLPS, p_xmm1,
						param_shift(p, 8, TypeReg64));
			} else {
				do_error(
						"...ret xmm "
								+ cur_func->effective_return_type->long_name());
			}
		} else {
			// store return directly in eax / fpu stack (4 byte)
			if (cur_func->effective_return_type->size == 1) {
				int v = cmd.add_virtual_reg(Asm::RegID::AL);
				insert_cmd(Asm::InstID::MOV,
						param_vreg(cur_func->effective_return_type, v), p);
			} else if (cur_func->effective_return_type->size == 8) {
				int v = cmd.add_virtual_reg(Asm::RegID::RAX);
				insert_cmd(Asm::InstID::MOV,
						param_vreg(cur_func->effective_return_type, v), p);
			} else {
				int v = cmd.add_virtual_reg(Asm::RegID::EAX);
				insert_cmd(Asm::InstID::MOV,
						param_vreg(cur_func->effective_return_type, v), p);
			}
		}
	}
	insert_cmd(Asm::InstID::LEAVE);
	//if (cur_func->effective_return_type->uses_return_by_memory())
	//	insert_cmd(Asm::InstID::RET, param_imm(TypeReg16, 4));
	//else
	insert_cmd(Asm::InstID::RET);
}


void BackendAmd64::implement_mov_chunk(const SerialNodeParam &p1, const SerialNodeParam &p2, int size) {
	//auto p1 = c.p[0];
	//auto p2 = c.p[1];
	//cmd.remove_cmd(i);
	//msg_error("CORRECT MOV " + p1.type->name);

	for (int offset=0; offset<size-7; offset+=8)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, offset, TypeInt64), param_shift(p2, offset, TypeInt64));
	for (int offset=8*(size/8); offset<size-3; offset+=4)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, offset, TypeInt), param_shift(p2, offset, TypeInt));
	for (int offset=4*(size/4); offset<size; offset++)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, offset, TypeChar), param_shift(p2, offset, TypeChar));
}


void BackendAmd64::function_call_post(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	const Class *type = ret.get_type_save();

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and (!type->uses_return_by_memory())) {
		if (type->_amd64_allow_pass_in_xmm()) {
			if (type == TypeFloat32) {
				insert_cmd(Asm::InstID::MOVSS, ret, p_xmm0);
			} else if (type == TypeFloat64) {
				insert_cmd(Asm::InstID::MOVSD, ret, p_xmm0);
			} else if (type->size == 8) { // float[2]
				insert_cmd(Asm::InstID::MOVLPS, ret, p_xmm0);
			} else if (type->size == 12) { // float[3]
				insert_cmd(Asm::InstID::MOVLPS, param_shift(ret, 0, TypeReg64), p_xmm0);
				insert_cmd(Asm::InstID::MOVSS, param_shift(ret, 8, TypeFloat32), p_xmm1);
			} else if (type->size == 16) { // float[4]
				// hmm, weird
				insert_cmd(Asm::InstID::MOVLPS, param_shift(ret, 0, TypeReg64), p_xmm0);
				//add_cmd(Asm::InstID::MOVHPS, param_shift(ret, 8, TypeReg64), p_xmm0);
				insert_cmd(Asm::InstID::MOVLPS, param_shift(ret, 8, TypeReg64), p_xmm1);
				//add_cmd(Asm::InstID::MOVUPS, ret, p_xmm0);
			} else {
				do_error("xmm return ..." + type->long_name());
			}
		} else if (type->size == 1) {
			int r = cmd.add_virtual_reg(Asm::RegID::AL);
			insert_cmd(Asm::InstID::MOV, ret, param_vreg(type, r));
			cmd.set_virtual_reg(r, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		} else if (type->size == 4) {
			int r = cmd.add_virtual_reg(Asm::RegID::EAX);
			insert_cmd(Asm::InstID::MOV, ret, param_vreg(type, r));
			cmd.set_virtual_reg(r, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		} else {
			int r = cmd.add_virtual_reg(Asm::RegID::RAX);
			insert_cmd(Asm::InstID::MOV, ret, param_vreg(type, r));
			cmd.set_virtual_reg(r, cmd.next_cmd_index - 2, cmd.next_cmd_index - 1);
		}
	}
}

bool dist_fits_32bit(int64 a, void *b) {
	int_p d = (int_p)a - (int_p)b;
	if (d < 0)
		d = -d;
	return (d < 0x70000000);
}

void BackendAmd64::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;
	int push_size = function_call_pre(params, ret, f->is_static());

	if (f->address != 0) {
		if (dist_fits_32bit(f->address, module->opcode)) {
			// 32bit call distance
			insert_cmd(Asm::InstID::CALL, param_imm(TypeReg32, f->address)); // the actual call
			// function pointer will be shifted later...(asm translates to RIP-relative)
		} else {
			// 64bit call distance
			insert_cmd(Asm::InstID::MOV, p_rax, param_imm(TypeReg64, f->address));
			insert_cmd(Asm::InstID::CALL, p_rax);
		}
	} else if (f->_label >= 0) {
		if (f->owner() == module->syntax) {
			// 32bit call distance
			insert_cmd(Asm::InstID::CALL, param_label(TypeInt, f->_label));
		} else {
			// 64bit call distance
			insert_cmd(Asm::InstID::MOV, p_rax, param_label(TypePointer, f->_label));
			insert_cmd(Asm::InstID::CALL, p_rax);
		}
	} else {
		serializer->do_error_link("could not link function " + f->signature());
	}
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	function_call_post(push_size, params, ret);
}

void BackendAmd64::add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, bool is_static) {
	serializer->call_used = true;
	int push_size = function_call_pre(params, ret, is_static);

	insert_cmd(Asm::InstID::MOV, p_rax, fp);
	insert_cmd(Asm::InstID::CALL, p_rax);
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	function_call_post(push_size, params, ret);
}

bool amd64_type_uses_int_register(const Class *t) {
	return (t == TypeInt) or (t == TypeInt64) or (t == TypeChar) or (t == TypeBool) or t->is_enum() or t->is_some_pointer();
}

int BackendAmd64::function_call_pre(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) {
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
		if ((config.abi == Abi::AMD64_WINDOWS) and !is_static and params[0].type->is_some_pointer())
			params.insert(insert_reference(ret), 1);
		else
			params.insert(insert_reference(ret), 0);
	}

	// map params...
	Array<SerialNodeParam> reg_param;
	Array<Asm::RegRoot> reg_param_root;
	Array<SerialNodeParam> xmm_param;
	Array<Asm::RegID> xmm_param_reg;
	Array<SerialNodeParam> stack_param;
	int reg_param_counter = 0;
	int xmm_param_counter = 0;
	for (auto &p: params) {
		if (amd64_type_uses_int_register(p.type)) {
			if (reg_param_counter < param_regs_root.num) {
				reg_param.add(p);
				reg_param_root.add(param_regs_root[reg_param_counter ++]);
				if (config.abi == Abi::AMD64_WINDOWS)
					xmm_param_counter ++;
			} else {
				stack_param.add(p);
			}
		} else if ((p.type == TypeFloat32) or (p.type == TypeFloat64)) {
			if (xmm_param_counter < max_xmm_params) {
				xmm_param.add(p);
				xmm_param_reg.add((Asm::RegID)((int)Asm::RegID::XMM0 + (xmm_param_counter ++)));
				if (config.abi == Abi::AMD64_WINDOWS)
					reg_param_counter++;
			} else {
				stack_param.add(p);
			}
		} else {
			do_error("parameter type currently not supported: " + p.type->name);
		}
	}

	// push parameters onto stack
	push_size = 8 * stack_param.num;
	if (stack_param.num > 0) {
		if (config.abi == Abi::AMD64_WINDOWS) {
			// TODO optimize... don't push, just write into stack
			push_size += 32;
		}
		// stack pointer is already low enough to include stack parameters
		// to compensate the following push's pre-increase rsp
		if (push_size > 127)
			insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::RSP), param_imm(TypeInt, push_size));
		else if (push_size > 0)
			insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::RSP), param_imm(TypeChar, push_size));
		//}
		foreachb (SerialNodeParam &p, stack_param) {
			insert_cmd(Asm::InstID::MOV, param_preg(p.type, get_reg(Asm::RegRoot::A, p.type->size)), p);
			insert_cmd(Asm::InstID::PUSH, p_rax);
		}
		if (config.abi == Abi::AMD64_WINDOWS) {
			push_size -= 32;
			insert_cmd(Asm::InstID::SUB, param_preg(TypePointer, Asm::RegID::RSP), param_imm(TypeChar, 32));
		}
	}
	max_push_size = max(max_push_size, (int)push_size);

	// xmm0-7
	foreachib(auto &p, xmm_param, i) {
		auto reg = xmm_param_reg[i];
		if (p.type == TypeFloat64)
			insert_cmd(Asm::InstID::MOVSD, param_preg(TypeReg128, reg), p);
		else
			insert_cmd(Asm::InstID::MOVSS, param_preg(TypeReg128, reg), p);
	}

	func_param_virts = {};

	foreachib(auto &p, reg_param, i) {
		auto root = reg_param_root[i];
		auto preg = get_reg(root, p.type->size);
		if (preg != Asm::RegID::INVALID) {
			int v = cmd.add_virtual_reg(preg);
			func_param_virts.add(v);
			insert_cmd(Asm::InstID::MOV, param_vreg(p.type, v), p);
		} else {
			// some registers are not 8bit'able
			int v = cmd.add_virtual_reg(get_reg(root, 4));
			func_param_virts.add(v);
			int va = cmd.add_virtual_reg(Asm::RegID::EAX);
			insert_cmd(Asm::InstID::MOV, param_vreg(p.type, va, Asm::RegID::AL), p);
			insert_cmd(Asm::InstID::MOV, param_vreg(TypeReg32, v), param_vreg(TypeReg32, va));
		}
	}

	return push_size;
}

void BackendAmd64::add_function_outro(Function *f) {
	insert_cmd(Asm::InstID::LEAVE, p_none);
	if (f->effective_return_type->uses_return_by_memory())
		insert_cmd(Asm::InstID::RET, param_imm(TypeReg16, 4));
	else
		insert_cmd(Asm::InstID::RET, p_none);
}

void BackendAmd64::add_function_intro_params(Function *f) {
	// return, instance, params
	Array<Variable*> param;
	if (f->effective_return_type->uses_return_by_memory()) {
		for (Variable *v: weak(f->var))
			if (v->name == Identifier::RETURN_VAR) {
				param.add(v);
				break;
			}
	}

	// self: already in params!
	/*if (!f->is_static()) {
		for (Variable *v: weak(f->var))
			if (v->name == Identifier::SELF) {
				param.add(v);
				break;
			}
	}*/

	for (int i=0;i<f->num_params;i++)
		param.add(f->var[i].get());

	// windows: self before return
	if ((param.num == 2) and (config.abi == Abi::AMD64_WINDOWS) and param[1]->type->is_some_pointer()) {
		param.swap(0, 1);
	}

	// map params...
	Array<Variable*> reg_param;
	Array<Asm::RegRoot> reg_param_root;
	Array<Variable*> xmm_param;
	Array<Asm::RegID> xmm_param_reg;
	Array<Variable*> stack_param;
	int reg_param_counter = 0;
	int xmm_param_counter = 0;
	for (Variable *p: param) {
		if (amd64_type_uses_int_register(p->type)) {
			if (reg_param_counter < param_regs_root.num) {
				reg_param.add(p);
				reg_param_root.add(param_regs_root[reg_param_counter ++]);
				if (config.abi == Abi::AMD64_WINDOWS)
					xmm_param_counter ++;
			} else {
				stack_param.add(p);
			}
		} else if ((p->type == TypeFloat32) or (p->type == TypeFloat64)) {
			if (xmm_param_counter < max_xmm_params) {
				xmm_param.add(p);
				xmm_param_reg.add((Asm::RegID)((int)Asm::RegID::XMM0 + (xmm_param_counter ++)));
				if (config.abi == Abi::AMD64_WINDOWS)
					reg_param_counter ++;
			} else {
				stack_param.add(p);
			}
		} else {
			do_error("parameter type currently not supported: " + p->type->name);
		}
	}

	// xmm0-7
	foreachib (Variable *p, xmm_param, i){
		auto reg = xmm_param_reg[i];
		if (p->type == TypeFloat64)
			insert_cmd(Asm::InstID::MOVSD, param_local(p->type, p->_offset), param_preg(TypeReg128, reg));
		else
			insert_cmd(Asm::InstID::MOVSS, param_local(p->type, p->_offset), param_preg(TypeReg128, reg));


	}

	foreachib (Variable *p, reg_param, i) {
		auto root = reg_param_root[i];
		auto preg = get_reg(root, p->type->size);
		if (preg != Asm::RegID::INVALID) {
			int v = cmd.add_virtual_reg(preg);
			insert_cmd(Asm::InstID::MOV, param_local(p->type, p->_offset), param_vreg(p->type, v));
			cmd.set_virtual_reg(v, 0, cmd.next_cmd_index-1);
		} else {
			// some registers are not 8bit'able
			int v = cmd.add_virtual_reg(get_reg(root, 4));
			int va = cmd.add_virtual_reg(Asm::RegID::EAX);
			insert_cmd(Asm::InstID::MOV, param_vreg(TypeReg32, va), param_vreg(TypeReg32, v));
			insert_cmd(Asm::InstID::MOV, param_local(p->type, p->_offset), param_vreg(p->type, va, get_reg(Asm::RegRoot::A, p->type->size)));
			cmd.set_virtual_reg(v, 0, cmd.next_cmd_index-2);
			cmd.set_virtual_reg(va, cmd.next_cmd_index-2, cmd.next_cmd_index-1);
		}
	}

	// get parameters from stack
	foreachb([[maybe_unused]] Variable *p, stack_param) {
		// variables are already where expect them ([rbp+...])
		if (config.abi != Abi::AMD64_WINDOWS)
			do_error("func with stack...");
		/*int s = 8;
		add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}



// so far not used... x86 also implements both...
void BackendAmd64::add_function_intro_frame(int stack_alloc_size) {
	if (config.abi == Abi::AMD64_WINDOWS)
		stack_alloc_size += 32; // shadow space

	auto reg_bp = Asm::RegID::RBP;
	auto reg_sp = Asm::RegID::RSP;
	//int s = config.pointer_size;
	list->add2(Asm::InstID::PUSH, Asm::param_reg(reg_bp));
	list->add2(Asm::InstID::MOV, Asm::param_reg(reg_bp), Asm::param_reg(reg_sp));
	if (stack_alloc_size > 127){
		list->add2(Asm::InstID::SUB, Asm::param_reg(reg_sp), Asm::param_imm(stack_alloc_size, Asm::SIZE_32));
	}else if (stack_alloc_size > 0){
		list->add2(Asm::InstID::SUB, Asm::param_reg(reg_sp), Asm::param_imm(stack_alloc_size, Asm::SIZE_8));
	}
}

}
