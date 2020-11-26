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


bool is_typed_function_pointer(const Class *c);

static int get_reg(int root, int size) {
#if 1
	if ((size != 1) and (size != 4) and (size != 8)) {
		msg_write(msg_get_trace());
		throw Asm::Exception("get_reg: bad reg size: " + i2s(size), "...", 0, 0);
	}
#endif
	return Asm::RegResize[root][size];
}

BackendAmd64::BackendAmd64(Serializer *s) : cmd(s->cmd) {
	serializer = s;
	script = s->script;
	list = s->list;
	cur_func = nullptr;
	cur_func_index = -1;

	stack_max_size = 0;
	max_push_size = 0;
	stack_offset = 0;

	map_reg_root.add(0); // eax
	map_reg_root.add(1); // ecx
	map_reg_root.add(2); // edx

	p_eax = param_preg(TypeReg32, Asm::REG_EAX);
	p_eax_int = param_preg(TypeInt, Asm::REG_EAX);
	p_rax = param_preg(TypeReg64, Asm::REG_RAX);

	p_deref_eax = param_deref_preg(TypePointer, Asm::REG_EAX);

	p_ax = param_preg(TypeReg16, Asm::REG_AX);
	p_al = param_preg(TypeReg8, Asm::REG_AL);
	p_al_bool = param_preg(TypeBool, Asm::REG_AL);
	p_al_char = param_preg(TypeChar, Asm::REG_AL);
	p_st0 = param_preg(TypeFloat32, Asm::REG_ST0);
	p_st1 = param_preg(TypeFloat32, Asm::REG_ST1);
	p_xmm0 = param_preg(TypeReg128, Asm::REG_XMM0);
	p_xmm1 = param_preg(TypeReg128, Asm::REG_XMM1);
}

BackendAmd64::~BackendAmd64() {
}

void BackendAmd64::process(Function *f, int index) {
	cur_func = f;
	cur_func_index = index;
	//call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;

	correct();
}

int trafo_inst_float(int inst, const Class *t) {
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

bool inst_is_arithmetic(int i) {
	if ((i == Asm::INST_IMUL) or (i == Asm::INST_IDIV) or (i == Asm::INST_ADD) or (i == Asm::INST_SUB))
		return true;
	if ((i == Asm::INST_AND) or (i == Asm::INST_OR) or (i == Asm::INST_XOR))
		return true;
	return false;
}

void BackendAmd64::correct() {
	cmd.next_cmd_target(0);
	add_function_intro_params(cur_func);

	correct_parameters();

	serializer->cmd_list_out("x:a", "paramtrafo");

	correct_implement_commands();
	serializer->cmd_list_out("x:b", "post paramtrafo");
}

void BackendAmd64::correct_parameters() {
	for (auto &c: cmd.cmd) {
		for (auto &p: c.p) {
			if (p.kind == NodeKind::VAR_LOCAL) {
				p.p = ((Variable*)p.p)->_offset;
				p.kind = NodeKind::LOCAL_MEMORY;
			} else if (p.kind == NodeKind::VAR_GLOBAL) {
				p.p = (int_p)((Variable*)p.p)->memory;
				if (!p.p)
					script->do_error_link("variable is not linkable: " + ((Variable*)p.p)->name);
				p.kind = NodeKind::MEMORY;
			} else if (p.kind == NodeKind::CONSTANT) {
				p.p = (int_p)((Constant*)p.p)->address; // FIXME ....need a cleaner approach for compiling os...
				if (config.compile_os)
					p.kind = NodeKind::MEMORY;
				else
					p.kind = NodeKind::CONSTANT_BY_ADDRESS;
				if (script->syntax->flag_function_pointer_as_code and (p.type == TypeFunctionP)) {
					auto *fp = (Function*)(int_p)((Constant*)p.p)->as_int64();
					p.kind = NodeKind::MARKER;
					p.p = fp->_label;
				}
			}
		}
	}
}

void BackendAmd64::correct_implement_commands() {

	Array<SerialNodeParam> func_params;

	for (int i=0; i<cmd.cmd.num; i++) {
		auto &c = cmd.cmd[i];
		if (c.inst == Asm::INST_MOV) {
			int size = c.p[0].type->size;
			if (size > 8) {
				auto p1 = c.p[0];
				auto p2 = c.p[1];
				cmd.remove_cmd(i);
				cmd.next_cmd_target(i);
				//msg_error("CORRECT MOV " + p1.type->name);

				for (int j=0; j<size/8; j++)
					insert_cmd(Asm::INST_MOV, param_shift(p1, j * 8, TypeInt64), param_shift(p2, j * 8, TypeInt64));
				for (int j=8*(size/8); j<size; j++)
					insert_cmd(Asm::INST_MOV, param_shift(p1, j, TypeChar), param_shift(p2, j, TypeChar));
				i = cmd.next_cmd_index - 1;
			}
		} else if (c.inst == Asm::INST_MOVSX) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
//			msg_write("MOVSX " + p1.type->name + " << "+ p2.type->name);
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			int reg = find_unused_reg(i, i, p2.type->size);
			insert_cmd(Asm::INST_MOV, param_vreg(p2.type, reg), p2);
			int preg_x = reg_resize(cmd.virtual_reg[reg].reg, p1.type->size);
			insert_cmd(Asm::INST_MOV, p1, param_vreg(p1.type, reg, preg_x));
			i ++;
		} else if (c.inst == Asm::INST_MODULO) {
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			if (p1.type == TypeInt) {
				int veax = cmd.add_virtual_reg(Asm::REG_EAX);
				int vedx = cmd.add_virtual_reg(Asm::REG_EDX);
				insert_cmd(Asm::INST_MOV, param_vreg(TypeInt, veax), p1);
				insert_cmd(Asm::INST_MOV, param_vreg(TypeInt, vedx), param_vreg(TypeInt, veax));
				insert_cmd(Asm::INST_SAR, param_vreg(TypeInt, vedx), param_imm(TypeChar, 0x1f));
				insert_cmd(Asm::INST_IDIV, param_vreg(TypeInt, veax), p2);
				insert_cmd(Asm::INST_MOV, r, param_vreg(TypeInt, vedx));
			} else { // int64
				int vrax = cmd.add_virtual_reg(Asm::REG_RAX);
				int vrdx = cmd.add_virtual_reg(Asm::REG_RDX);
				insert_cmd(Asm::INST_MOV, param_vreg(TypeInt64, vrax), p1);
				insert_cmd(Asm::INST_MOV, param_vreg(TypeInt64, vrdx), param_vreg(TypeInt64, vrax));
				insert_cmd(Asm::INST_SAR, param_vreg(TypeInt64, vrdx), param_imm(TypeChar, 0x1f));
				insert_cmd(Asm::INST_IDIV, param_vreg(TypeInt64, vrax), p2);
				insert_cmd(Asm::INST_MOV, r, param_vreg(TypeInt64, vrdx));
			}
			i += 4;
		} else if ((c.inst == Asm::INST_SHL) or (c.inst == Asm::INST_SHR)) {
			auto inst = c.inst;
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			auto type = p1.type;
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			int vecx;
			if (type == TypeInt64) {
				msg_error("shl int64");
				vecx = cmd.add_virtual_reg(Asm::REG_RCX);
			} else
				vecx = cmd.add_virtual_reg(Asm::REG_ECX);
			insert_cmd(Asm::INST_MOV, param_vreg(type, vecx), p2);
			insert_cmd(Asm::INST_MOV, r, p1);
			insert_cmd(inst, r, param_vreg(TypeChar, vecx, Asm::REG_CL));
			i += 2;
		} else if (inst_is_arithmetic(c.inst)) {
			if (c.p[2].kind == NodeKind::NONE)
				continue;
			auto inst = c.inst;
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			auto type = p1.type;
			int reg = find_unused_reg(i, i, type->size);

			auto t = param_vreg(type, reg);
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			insert_cmd(Asm::INST_MOV, t, p1);
			insert_cmd(inst, t, p2);
			insert_cmd(Asm::INST_MOV, r, t);
			cmd.set_virtual_reg(reg, i, i + 2);

			i += 2;
		} else if ((c.inst == Asm::INST_FMUL) or (c.inst == Asm::INST_FDIV) or (c.inst == Asm::INST_FADD) or (c.inst == Asm::INST_FSUB)) {
			auto inst = c.inst;
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			auto p3 = c.p[2];
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);

			inst = trafo_inst_float(inst, p1.type);
			int inst_mov = (p1.type == TypeFloat64) ? Asm::INST_MOVSD : Asm::INST_MOVSS;

			if (p3.kind == NodeKind::NONE) {
				// a += b
				insert_cmd(inst_mov, p_xmm0, p1);
				insert_cmd(inst, p_xmm0, p2);
				insert_cmd(inst_mov, p1, p_xmm0);
			} else {
				// a = b + c
				insert_cmd(inst_mov, p_xmm0, p2);
				insert_cmd(inst, p_xmm0, p3);
				insert_cmd(inst_mov, p1, p_xmm0);
			}
			i += 2;
		} else if (c.inst == Asm::INST_UCOMISS) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			insert_cmd(Asm::INST_MOVSS, p_xmm0, p1);
			insert_cmd(Asm::INST_UCOMISS, p_xmm0, p2);
			i ++;
		} else if (c.inst == Asm::INST_CVTSI2SS) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			insert_cmd(Asm::INST_CVTSI2SS, p_xmm0, p2);
			insert_cmd(Asm::INST_MOVSS, p1, p_xmm0);
			i ++;
		} else if (c.inst == Asm::INST_CVTTSS2SI) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);
			int veax = cmd.add_virtual_reg(Asm::REG_EAX);
			insert_cmd(Asm::INST_MOVSS, p_xmm0, p2);
			insert_cmd(Asm::INST_CVTTSS2SI, param_vreg(TypeInt, veax), p_xmm0);
			insert_cmd(Asm::INST_MOV, p1, param_vreg(TypeInt, veax));
			i += 2;
		} else if (c.inst == Asm::INST_PUSH) {
			func_params.add(c.p[0]);
			cmd.remove_cmd(i);
			i --;
		} else if (c.inst == Asm::INST_CALL) {

			if (c.p[1].type == TypeFunctionCodeP) {
				//serializer->do_error("indirect call...");
				auto fp = c.p[1];
				auto ret = c.p[0];
				cmd.remove_cmd(i);
				cmd.next_cmd_target(i);
				add_pointer_call(fp, func_params, ret);
			} else if (is_typed_function_pointer(c.p[1].type)) {
				serializer->do_error("BACKEND: POINTER CALL");
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
		} else if (c.inst == Asm::INST_RET) {
			auto p = c.p[0];
			cmd.remove_cmd(i);
			cmd.next_cmd_target(i);




			if (p.kind != NodeKind::NONE){

				if (cur_func->effective_return_type->_amd64_allow_pass_in_xmm()) {
					// if ((config.instruction_set == Asm::INSTRUCTION_SET_AMD64) or (config.compile_os)) ???
					//		cmd.add_cmd(Asm::INST_FLD, t);
					if (cur_func->effective_return_type == TypeFloat32){
						insert_cmd(Asm::INST_MOVSS, p_xmm0, p);
					}else if (cur_func->effective_return_type == TypeFloat64){
						insert_cmd(Asm::INST_MOVSD, p_xmm0, p);
					}else if (cur_func->effective_return_type->size == 8){ // float[2]
						insert_cmd(Asm::INST_MOVLPS, p_xmm0, p);
					}else if (cur_func->effective_return_type->size == 12){ // float[3]
						insert_cmd(Asm::INST_MOVLPS, p_xmm0, param_shift(p, 0, TypeReg64));
						insert_cmd(Asm::INST_MOVSS, p_xmm1, param_shift(p, 8, TypeFloat32));
					}else if (cur_func->effective_return_type->size == 16){ // float[4]
						insert_cmd(Asm::INST_MOVLPS, p_xmm0, param_shift(p, 0, TypeReg64));
						insert_cmd(Asm::INST_MOVLPS, p_xmm1, param_shift(p, 8, TypeReg64));
					} else {
						serializer->do_error("...ret xmm " + cur_func->effective_return_type->long_name());
					}
				}else{ // store return directly in eax / fpu stack (4 byte)

					if (cur_func->effective_return_type->size == 1){
						int v = cmd.add_virtual_reg(Asm::REG_AL);
						insert_cmd(Asm::INST_MOV, param_vreg(cur_func->effective_return_type, v), p);
					}else if (cur_func->effective_return_type->size == 8){
						int v = cmd.add_virtual_reg(Asm::REG_RAX);
						insert_cmd(Asm::INST_MOV, param_vreg(cur_func->effective_return_type, v), p);
					}else{
						int v = cmd.add_virtual_reg(Asm::REG_EAX);
						insert_cmd(Asm::INST_MOV, param_vreg(cur_func->effective_return_type, v), p);
					}
				}
			}


			insert_cmd(Asm::INST_LEAVE);
			//if (cur_func->effective_return_type->uses_return_by_memory())
			//	insert_cmd(Asm::INST_RET, param_imm(TypeReg16, 4));
			//else
				insert_cmd(Asm::INST_RET);
			i = cmd.next_cmd_index - 1;
		}
	}
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
				serializer->do_error("xmm return ..." + type->long_name());
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
	int push_size = fc_begin(params, ret);

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
	int push_size = fc_begin(params, ret);

	insert_cmd(Asm::INST_MOV, p_rax, fp);
	insert_cmd(Asm::INST_CALL, p_rax);
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	fc_end(push_size, params, ret);
}

int BackendAmd64::fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret) {
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
			serializer->do_error("parameter type currently not supported: " + p.type->name);
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

void BackendAmd64::mark_regs_busy_at_call(int index) {
	// call violates all used registers...
	for (int i=0;i<map_reg_root.num;i++) {
		int v = cmd.add_virtual_reg(get_reg(i, 4));
		cmd.use_virtual_reg(v, index, index);
	}
}

void BackendAmd64::extend_reg_usage_to_call(int index) {
	for (int v: func_param_virts)
		cmd.use_virtual_reg(v, index, index);
}

SerialNodeParam BackendAmd64::param_vreg(const Class *type, int vreg, int preg) {
	if (preg < 0)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::REGISTER, preg, vreg, type, 0};
}

SerialNodeParam BackendAmd64::param_deref_vreg(const Class *type, int vreg, int preg) {
	if (preg < 0)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::DEREF_REGISTER, preg, vreg, type, 0};
}


void BackendAmd64::insert_cmd(int inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3) {
	int i = cmd.next_cmd_index;
	cmd.add_cmd(inst, p1, p2, p3);
	cmd.next_cmd_target(i + 1);
}

SerialNodeParam BackendAmd64::insert_reference(const SerialNodeParam &param, const Class *type) {
	SerialNodeParam ret;
	if (!type)
		type = param.type->get_pointer();
	ret.type = type;
	ret.shift = 0;
	if (param.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		ret.kind = NodeKind::IMMEDIATE;
		ret.p = (int_p)((char*)param.p + param.shift);
	} else if ((param.kind == NodeKind::IMMEDIATE) or (param.kind == NodeKind::MEMORY)) {
		ret.kind = NodeKind::IMMEDIATE;
		if (param.shift > 0)
			msg_error("Serializer: immediade/mem + shift?!?!?");
		ret.p = param.p;
	} else if (param.kind == NodeKind::DEREF_VAR_TEMP) {
		ret = param;
		ret.kind = NodeKind::VAR_TEMP; // FIXME why was it param.kind ?!?!?
	} else {
		ret = cmd._add_temp(type);
		insert_cmd(Asm::INST_LEA, ret, param);
	}
	return ret;
}


bool BackendAmd64::is_reg_root_used_in_interval(int reg_root, int first, int last) {
	return serializer->is_reg_root_used_in_interval(reg_root, first, last);
}

int BackendAmd64::find_unused_reg(int first, int last, int size, int exclude) {
	//vr_list_out();
	for (int r: map_reg_root)
		if (r != exclude)
			if (!is_reg_root_used_in_interval(r, first, last)) {
				return cmd.add_virtual_reg(get_reg(r, size));
			}
	serializer->cmd_list_out("fur", "find unused reg", true);
	msg_write(serializer->cur_func->long_name());
	serializer->do_error(format("no free register of size %d   in %d:%d", size, first, last));
	return -1;
}

int BackendAmd64::reg_resize(int reg, int size) {
	if (size == 2) {
		msg_error("size = 2");
		msg_write(msg_get_trace());
		throw Asm::Exception("size=2", "kjlkjl", 0, 0);
		//Asm::DoError("size=2");
	}
	return get_reg(Asm::RegRoot[reg], size);
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
			serializer->do_error("parameter type currently not supported: " + p->type->name);
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
		serializer->do_error("func with stack...");
		/*int s = 8;
		add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}


void BackendAmd64::do_mapping() {
	map_referenced_temp_vars_to_stack();


	process_references();

	try_map_temp_vars_to_registers();

	serializer->cmd_list_out("map:a", "post temp -> reg");

	map_remaining_temp_vars_to_stack();

	serializer->cmd_list_out("map:b", "post temp -> stack");

	resolve_deref_temp_and_local();

	serializer->cmd_list_out("map:c", "post deref t&l");

	correct_unallowed_param_combis();

	serializer->cmd_list_out("map:d", "unallowed");

	/*MapReferencedTempVars();

	//HandleDerefTemp();

	DisentangleShiftedTempVars();

	ResolveDerefTempAndLocal();

	RemoveUnusedTempVars();

	if (config.allow_simplification){
	SimplifyMovs();

	SimplifyFPUStack();
	}

	MapTempVars();

	ResolveDerefRegShift();

	//ResolveDerefLocal();

	CorrectUnallowedParamCombis();*/


	for (int i=0; i<cmd.cmd.num; i++)
		correct_unallowed_param_combis2(cmd.cmd[i]);

	serializer->cmd_list_out("map:z", "end");
}

//#define debug_evil_corrections

static void _test_param_mem(SerialNodeParam &p) {
	//if (p.kind == NodeKind::ADDRESS)
}

void BackendAmd64::correct_unallowed_param_combis2(SerialNode &c) {
	// push 8 bit -> push 32 bit
	if (c.inst == Asm::INST_PUSH)
		if (c.p[0].kind == NodeKind::REGISTER)
			c.p[0].p = reg_resize(c.p[0].p, config.pointer_size);

	_test_param_mem(c.p[0]);
	_test_param_mem(c.p[1]);

	if (c.inst == Asm::INST_CMP)
		if ((c.p[1].kind == NodeKind::IMMEDIATE) and (c.p[1].type->size == 8)) {
			if (c.p[1].p & 0xffffffff00000000 != 0)
				serializer->do_error("cmp immediate > 32bit");
			c.p[1].type = TypeInt;
		}


	// FIXME
	// evil hack to allow inconsistent param types (in address shifts)
	if (config.instruction_set == Asm::InstructionSet::AMD64) {
		if ((c.inst == Asm::INST_ADD) or (c.inst == Asm::INST_MOV)) {
			if ((c.p[0].kind == NodeKind::REGISTER) and (c.p[1].kind == NodeKind::CONSTANT_BY_ADDRESS)){
				// TODO: should become an optimization if value fits into 32 bit...
				/*if (c.p[0].type->is_pointer){
#ifdef debug_evil_corrections
					msg_write("----evil resize a");
					msg_write(c.str());
#endif
					c.p[0].type = TypeReg32;
					c.p[0].p = reg_resize(c.p[0].p, 4);
#ifdef debug_evil_corrections
					msg_write(c.str());
#endif
				}*/
			}
			if ((c.p[0].type->size == 8) and (c.p[1].type->size == 4)) {
				/*if ((c.p[0].kind == KindRegister) and ((c.p[1].kind == KindRegister) or (c.p[1].kind == KindConstant) or (c.p[1].kind == KindRefToConst))){
#ifdef debug_evil_corrections
					msg_write("----evil resize b");
					msg_write(c.str());
#endif
					c.p[0].type = c.p[1].type;
					c.p[0].p = (char*)(long)reg_resize((long)c.p[0].p, c.p[1].type->size);
#ifdef debug_evil_corrections
					msg_write(c.str());
#endif
				}else*/ if (c.p[1].kind == NodeKind::REGISTER) {
#ifdef debug_evil_corrections
					msg_write("----evil resize c");
					msg_write(c.str());
#endif
					c.p[1].type = c.p[0].type;
					c.p[1].p = reg_resize(c.p[1].p, c.p[0].type->size);
#ifdef debug_evil_corrections
					msg_write(c.str());
#endif
				}
			}
			if ((c.p[0].type->size < 8) and (c.p[1].type->size == 8)) {
				if ((c.p[0].kind == NodeKind::REGISTER) and ((c.p[1].kind == NodeKind::REGISTER) or (c.p[1].kind == NodeKind::DEREF_REGISTER))) {
#ifdef debug_evil_corrections
					msg_write("----evil resize d");
					msg_write(c.str());
#endif
					c.p[0].type = c.p[1].type;
					c.p[0].p = reg_resize(c.p[0].p, c.p[1].type->size);
#ifdef debug_evil_corrections
					msg_write(c.str());
#endif
				}
			}
			/*if (c.p[0].type->size > c.p[1].type->size){
				msg_write("size ok");
				if ((c.p[0].kind == KindRegister) and ((c.p[1].kind == KindRegister) or (c.p[1].kind == KindConstant) or (c.p[1].kind == KindRefToConst))){
					msg_error("----evil resize");
					c.p[0].type = c.p[1].type;
					c.p[0].p = (char*)(long)Asm::RegResize[Asm::RegRoot[(long)c.p[0].p]][c.p[1].type->size];
				}
			}*/
		}
	}
}

void BackendAmd64::process_references() {
	for (int i=0;i<cmd.cmd.num;i++)
		if (cmd.cmd[i].inst == Asm::INST_LEA) {
			if (cmd.cmd[i].p[1].kind == NodeKind::LOCAL_MEMORY) {
				SerialNodeParam p0 = cmd.cmd[i].p[0];
				SerialNodeParam p1 = cmd.cmd[i].p[1];
				cmd.remove_cmd(i);

				if (config.instruction_set == Asm::InstructionSet::AMD64) {
					int r = cmd.add_virtual_reg(Asm::REG_RAX);
					cmd.next_cmd_target(i);
					insert_cmd(Asm::INST_LEA, param_vreg(TypeReg64, r), p1);
					cmd.next_cmd_target(i+1);
					insert_cmd(Asm::INST_MOV, p0, param_vreg(TypeReg64, r));
					cmd.set_virtual_reg(r, i, i+1);
				} else {
					int r = cmd.add_virtual_reg(Asm::REG_EAX);
					cmd.next_cmd_target(i);
					insert_cmd(Asm::INST_LEA, param_vreg(TypeReg32, r), p1);
					cmd.next_cmd_target(i+1);
					insert_cmd(Asm::INST_MOV, p0, param_vreg(TypeReg32, r));
					cmd.set_virtual_reg(r, i, i+1);
				}
			} else {
				serializer->do_error("reference in x86: " + cmd.cmd[i].p[1].str(serializer));
			}
		}
}

inline void try_map_param_to_stack(SerialNodeParam &p, int v, SerialNodeParam &stackvar) {
	if ((p.kind == NodeKind::VAR_TEMP) and (p.p == v)) {
		p.kind = NodeKind::LOCAL_MEMORY;//stackvar.kind;
		p.p = stackvar.p;
	} else if ((p.kind == NodeKind::DEREF_VAR_TEMP) and (p.p == v)) {
		p.kind = NodeKind::DEREF_LOCAL_MEMORY;
		p.p = stackvar.p;
	}
}


void BackendAmd64::map_referenced_temp_vars_to_stack() {
	for (SerialNode &c: cmd.cmd)
		if (c.inst == Asm::INST_LEA)
			if (c.p[1].kind == NodeKind::VAR_TEMP) {
				int v = c.p[1].p;
//				msg_error("ref b " + i2s(v));
				cmd.temp_var[v].referenced = true;
				cmd.temp_var[v].force_stack = true;
			}

	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		if (!cmd.temp_var[i].force_stack)
			continue;
		SerialNodeParam stackvar;
		add_stack_var(cmd.temp_var[i], stackvar);
		for (int j=0;j<cmd.cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				try_map_param_to_stack(cmd.cmd[j].p[k], i, stackvar);
		}
		cmd.remove_temp_var(i);
	}
}

void BackendAmd64::try_map_temp_vars_to_registers() {
	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		if (cmd.temp_var[i].force_stack)
			continue;
	}
}

void BackendAmd64::map_remaining_temp_vars_to_stack() {
	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		SerialNodeParam stackvar;
		add_stack_var(cmd.temp_var[i], stackvar);
		for (int j=0;j<cmd.cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				try_map_param_to_stack(cmd.cmd[j].p[k], i, stackvar);
		}
		cmd.remove_temp_var(i);
	}
}



struct StackOccupationX {
	Array<int> x;
	int reserved;
	bool down;

	void create(Serializer *s, bool down, int reserved, int first, int last) {
		x.clear();
		this->down = down;
		this->reserved = reserved;
		for (TempVar &v: s->cmd.temp_var) {
			if (!v.mapped)
				continue;
			if ((v.first > first and v.last > last) or (v.first < first and v.last < last))
				continue;
			set(v.stack_offset, v.type->size);
		}
	}
	void set(int start, int size) {
		if (down)
			start = - start - size;

		int a = start / 32;
		int mask = 1 << (start % 23);
		for (int i=0; i<size; i++) {
			if (mask == 1) {
				if (a >= x.num)
					x.resize(a + 1);
			}
			x[a] |= mask;
			mask *= 2;
			if (mask == 0) {
				mask = 1;
				a ++;
			}
		}
	}

	bool is_free(int start, int size) {
		if (down)
			start = - start - size;

		int a = start / 32;
		int mask = 1 << (start % 23);
		msg_write("------");
		msg_write(start);
		msg_write(a);
		msg_write(mask);
		if (a >= x.num)
			return true;
		for (int i=0; i<size; i++) {
			if (mask == 1) {
				if (a >= x.num)
					return true;
			}
			if (x[a] & mask)
				return false;
			mask *= 2;
			if (mask == 0) {
				mask = 1;
				a ++;
			}
		}

		return true;
	}

	int find_free(int size) {
		int pos = down ? -(reserved + size) : reserved;
		while (true) {
			if (is_free(pos, size))
				return pos;
			if (down)
				pos -= 4;
			else
				pos += 4;
		}
		return pos;
	}
};

void BackendAmd64::add_stack_var(TempVar &v, SerialNodeParam &p) {
	// find free stack space for the life span of the variable....
	// don't mind... use old algorithm: ALWAYS grow stack... remove ALL on each command in a block

	int s = mem_align(v.type->size, 4);
	StackOccupationX so;
//	msg_write(format("add stack var  %s %d   %d-%d       vs=%d", v.type->name.c_str(), v.type->size, v.first, v.last, cur_func->_var_size));
//	foreachi(TempVar &t, temp_var, i)
//		if (&t == &v)
//			msg_write("#" + i2s(i));
	so.create(serializer, (config.instruction_set != Asm::InstructionSet::ARM), cur_func->_var_size, v.first, v.last);

	if (true) {
		// TODO super important!!!!!!
		if (config.instruction_set == Asm::InstructionSet::ARM) {
			v.stack_offset = stack_offset;
			stack_offset += s;

		} else {
			stack_offset += s;
			v.stack_offset = - stack_offset;
		}
	} else {
		v.stack_offset = so.find_free(v.type->size);
		if (config.instruction_set == Asm::InstructionSet::ARM) {
			stack_offset = v.stack_offset + s;
		} else {
			stack_offset = - v.stack_offset;
		}
	}
//	msg_write("=>");
//	msg_write(v.stack_offset);

	if (stack_offset > stack_max_size)
		stack_max_size = stack_offset;

	v.mapped = true;

	p.kind = NodeKind::LOCAL_MEMORY;
	p.p = v.stack_offset;
	p.type = v.type;
	p.shift = 0;
}

inline bool param_is_simple(SerialNodeParam &p) {
	return ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::VAR_TEMP) or (p.kind == NodeKind::NONE));
}

inline bool param_combi_allowed(int inst, SerialNodeParam &p1, SerialNodeParam &p2) {
//	if (inst >= Asm::inst_marker)
//		return true;
	if ((!param_is_simple(p1)) and (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	Asm::get_instruction_param_flags(inst, r1, w1, r2, w2);
	if (w1 and (p1.kind == NodeKind::IMMEDIATE))
		return false;
	if (w2 and (p2.kind == NodeKind::IMMEDIATE))
		return false;
	if ((p1.kind == NodeKind::IMMEDIATE) or (p2.kind == NodeKind::IMMEDIATE))
		if (!Asm::get_instruction_allow_const(inst))
			return false;
	return true;
}

// mov [0x..] [0x...]  ->  mov eax, [0x..]   mov [0x..] eax    (etc)
void BackendAmd64::correct_unallowed_param_combis() {
	for (int i=cmd.cmd.num-1;i>=0;i--){
		if (cmd.cmd[i].inst >= INST_MARKER)
			continue;

		// bad?
		if (param_combi_allowed(cmd.cmd[i].inst, cmd.cmd[i].p[0], cmd.cmd[i].p[1]))
			continue;

		// correct
//		msg_write(format("correcting param combi  cmd=%d", i));
		bool mov_first_param = (cmd.cmd[i].p[1].kind == NodeKind::NONE) or (cmd.cmd[i].p[0].kind == NodeKind::CONSTANT_BY_ADDRESS) or (cmd.cmd[i].p[0].kind == NodeKind::IMMEDIATE);
		int p_index = mov_first_param ? 0 : 1;
		SerialNodeParam p = cmd.cmd[i].p[p_index];
		SerialNodeParam p2 = p;

		//msg_error("correct");
		//msg_write(p.type->name);
		if (p.type->name == "color")
			serializer->do_error("color in assembler..." + serializer->cur_func->long_name());
		int reg = find_unused_reg(i, i, p.type->size);
		p2 = param_vreg(p.type, reg);
		cmd.next_cmd_target(i);
		insert_cmd(Asm::INST_MOV, p2, p);
		cmd.set_cmd_param(cmd.cmd[i+1], p_index, p2);
		cmd.set_virtual_reg(reg, i, i + 1);
	}
	scan_temp_var_usage();
}

// inst ... [local] ...
// ->      mov reg, local     inst ...[reg]...
void BackendAmd64::solve_deref_temp_local(int c, int np, bool is_local) {
	SerialNodeParam p = cmd.cmd[c].p[np];
	int shift = p.shift;

	const Class *type_pointer = is_local ? TypePointer : cmd.temp_var[p.p].type;
	const Class *type_data = p.type;

	p.kind = is_local ? NodeKind::LOCAL_MEMORY : NodeKind::VAR_TEMP;
	p.shift = 0;
	p.type = type_pointer;

	int reg = find_unused_reg(c, c, config.pointer_size);
	if (reg < 0)
		script->do_error_internal("solve_deref_temp_local... no registers available");
	SerialNodeParam p_reg = param_vreg(type_pointer, reg);
	SerialNodeParam p_deref_reg = param_deref_vreg(type_data, reg);

	cmd.set_cmd_param(cmd.cmd[c], np, p_deref_reg);

	cmd.next_cmd_target(c);
	insert_cmd(Asm::INST_MOV, p_reg, p);
	if (shift > 0) {
		// solve_deref_temp_local
		cmd.next_cmd_target(c + 1);
		insert_cmd(Asm::INST_ADD, p_reg, param_imm(TypeInt, shift));
		cmd.set_virtual_reg(reg, c, c+2);
	} else {
		cmd.set_virtual_reg(reg, c, c+1);
	}
}


void BackendAmd64::resolve_deref_temp_and_local() {
	for (int i=cmd.cmd.num-1;i>=0;i--) {
		if (cmd.cmd[i].inst >= INST_MARKER)
			continue;
		bool dl1 = ((cmd.cmd[i].p[0].kind == NodeKind::DEREF_LOCAL_MEMORY) or (cmd.cmd[i].p[0].kind == NodeKind::DEREF_VAR_TEMP));
		bool dl2 = ((cmd.cmd[i].p[1].kind == NodeKind::DEREF_LOCAL_MEMORY) or (cmd.cmd[i].p[1].kind == NodeKind::DEREF_VAR_TEMP));
		if (!(dl1 or dl2))
			continue;

		bool is_local1 = (cmd.cmd[i].p[0].kind == NodeKind::DEREF_LOCAL_MEMORY);
		bool is_local2 = (cmd.cmd[i].p[1].kind == NodeKind::DEREF_LOCAL_MEMORY);

		//msg_write(format("deref temp/local... cmd=%d", i));
		if (!dl2) {
			solve_deref_temp_local(i, 0, is_local1);
			i ++;
		} else if (!dl1) {
			solve_deref_temp_local(i, 1, is_local2);
			i ++;
		} else {
			// hopefully... p2 is read-only

			const Class *type_pointer = TypePointer;
			const Class *type_data = cmd.cmd[i].p[0].type;

			int reg = find_unused_reg(i, i, type_data->size);
			if (reg < 0)
				serializer->do_error("deref local... both sides... .no registers available");

			SerialNodeParam p_reg = param_vreg(type_data, reg);

			int reg2 = find_unused_reg(i, i, config.pointer_size, cmd.virtual_reg[reg].reg_root);
			if (reg2 < 0)
				serializer->do_error("deref temp/local... both sides... .no registers available");
			SerialNodeParam p_reg2 = param_vreg(type_pointer, reg2);
			SerialNodeParam p_deref_reg2 = param_deref_vreg(type_data, reg2);

			// inst [l1] [l2]
			// ->
			// mov reg2, l2
			//   (add reg2, shift2)
			// mov reg, [reg2]
			// mov reg2, l1
			//   (add reg2, shift1)
			// inst [reg2], reg
			SerialNodeParam p1 = cmd.cmd[i].p[0];
			SerialNodeParam p2 = cmd.cmd[i].p[1];
			int shift1 = p1.shift;
			int shift2 = p2.shift;
			p1.shift = p2.shift = 0;

			p1.kind = is_local1 ? NodeKind::LOCAL_MEMORY : NodeKind::VAR_TEMP;
			p2.kind = is_local2 ? NodeKind::LOCAL_MEMORY : NodeKind::VAR_TEMP;
			p1.type = type_pointer;
			p2.type = type_pointer;
			cmd.set_cmd_param(cmd.cmd[i], 0, p_deref_reg2);
			cmd.set_cmd_param(cmd.cmd[i], 1, p_reg);
			int cmd_pos = i;

			int r2_first = cmd_pos;
			cmd.next_cmd_target(cmd_pos ++);
			insert_cmd(Asm::INST_MOV, p_reg2, p2);

			if (shift2 > 0) {
				// resolve deref temp&loc 2
				cmd.next_cmd_target(cmd_pos ++);
				insert_cmd(Asm::INST_ADD, p_reg2, param_imm(TypeInt, shift2));
			}

			int r1_first = cmd_pos;
			cmd.next_cmd_target(cmd_pos ++);
			insert_cmd(Asm::INST_MOV, p_reg, p_deref_reg2);

			cmd.next_cmd_target(cmd_pos ++);
			insert_cmd(Asm::INST_MOV, p_reg2, p1);

			if (shift1 > 0) {
				// resolve deref temp&loc 1
				cmd.next_cmd_target(cmd_pos ++);
				insert_cmd(Asm::INST_ADD, p_reg2, param_imm(TypeInt, shift1));
			}

			cmd.set_virtual_reg(reg, r1_first, cmd_pos);
			cmd.set_virtual_reg(reg2, r2_first, cmd_pos);

			i = cmd_pos;
		}
	}
}

void BackendAmd64::scan_temp_var_usage() {
	/*msg_write("ScanTempVarUsage");
	foreachi(TempVar &v, temp_var, i) {
		v.first = -1;
		v.last = -1;
		v.usage_count = 0;
		for (int c=0;c<cmd.num;c++) {
			if (temp_in_cmd(c, i) > 0) {
				v.usage_count ++;
				if (v.first < 0)
					v.first = c;
				v.last = c;
			}
		}
	}
	temp_var_ranges_defined = true;*/
}





Asm::InstructionParam BackendAmd64::get_param(int inst, SerialNodeParam &p) {
	if (p.kind == NodeKind::NONE) {
		return Asm::param_none;
	} else if (p.kind == NodeKind::MARKER) {
		return Asm::param_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::DEREF_MARKER) {
		return Asm::param_deref_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::REGISTER) {
		if (p.shift > 0)
			script->do_error_internal("get_param: reg + shift");
		return Asm::param_reg(p.p);
		//param_size = p.type->size;
	} else if (p.kind == NodeKind::DEREF_REGISTER) {
		if (p.shift != 0)
			return Asm::param_deref_reg_shift(p.p, p.shift, p.type->size);
		else
			return Asm::param_deref_reg(p.p, p.type->size);
	} else if (p.kind == NodeKind::MEMORY) {
		int size = p.type->size;
		// compiler self-test
		if ((size != 1) and (size != 2) and (size != 4) and (size != 8))
			script->do_error_internal("get_param: evil global of type " + p.type->name);
		return Asm::param_deref_imm(p.p + p.shift, size);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		if (config.instruction_set == Asm::InstructionSet::ARM) {
			return Asm::param_deref_reg_shift(Asm::REG_R13, p.p + p.shift, p.type->size);
		} else {
			return Asm::param_deref_reg_shift(Asm::REG_EBP, p.p + p.shift, p.type->size);
		}
		//if ((param_size != 1) and (param_size != 2) and (param_size != 4) and (param_size != 8))
		//	param_size = -1; // lea doesn't need size...
			//s->DoErrorInternal("get_param: evil local of type " + p.type->name);
	} else if (p.kind == NodeKind::CONSTANT_BY_ADDRESS) {
		bool imm_allowed = Asm::get_instruction_allow_const(inst);
		if ((imm_allowed) and (p.type->is_pointer())) {
			return Asm::param_imm(*(int_p*)(p.p + p.shift), p.type->size);
		} else if ((p.type->size <= 4) and (imm_allowed)) {
			return Asm::param_imm(*(int*)(p.p + p.shift), p.type->size);
		} else {
			return Asm::param_deref_imm(p.p + p.shift, p.type->size);
		}
	} else if (p.kind == NodeKind::IMMEDIATE) {
		if (p.shift > 0)
			script->do_error_internal("get_param: immediate + shift");
		return Asm::param_imm(p.p, p.type->size);
	} else
		script->do_error_internal("get_param: unexpected param..." + kind2str(p.kind));
	return Asm::param_none;
}


void BackendAmd64::assemble_cmd(SerialNode &c) {
	// translate parameters
	Asm::InstructionParam p1 = get_param(c.inst, c.p[0]);
	Asm::InstructionParam p2 = get_param(c.inst, c.p[1]);

	// assemble instruction
	//list->current_line = c.
	list->add2(c.inst, p1, p2);
}

void BackendAmd64::assemble_cmd_arm(SerialNode &c) {
	// translate parameters
	Asm::InstructionParam p1 = get_param(c.inst, c.p[0]);
	Asm::InstructionParam p2 = get_param(c.inst, c.p[1]);
	Asm::InstructionParam p3 = get_param(c.inst, c.p[2]);

	// assemble instruction
	//list->current_line = c.
	list->add_arm(c.cond, c.inst, p1, p2, p3);
}

static void AddAsmBlock(Asm::InstructionWithParamsList *list, Script *s) {
	//msg_write(".------------------------------- asm");
	SyntaxTree *ps = s->syntax;
	if (ps->asm_blocks.num == 0)
		s->do_error("asm block mismatch");
	ps->asm_meta_info->line_offset = ps->asm_blocks[0].line;
	list->append_from_source(ps->asm_blocks[0].block);
	ps->asm_blocks.erase(0);
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
// convert    SerialNode[] cmd   into    Asm::Instruction..List list
void BackendAmd64::assemble() {
	// intro + allocate stack memory
	if (config.instruction_set != Asm::InstructionSet::ARM)
		stack_max_size += max_push_size;
	stack_max_size = mem_align(stack_max_size, config.stack_frame_align);

	if (config.instruction_set == Asm::InstructionSet::ARM) {
		foreachi(GlobalRef &g, global_refs, i) {
			g.label = list->add_label(format("_kaba_ref_%d_%d", cur_func_index, i));
			list->add2(Asm::INST_DD, Asm::param_imm((int_p)g.p, 4));
		}
	}

	list->insert_label(cur_func->_label);

	if (!config.no_function_frame)
		add_function_intro_frame(stack_max_size); // param intro later...
	correct_return();

	for (int i=0;i<cmd.cmd.num;i++) {

		if (cmd.cmd[i].inst == INST_MARKER) {
			list->insert_label(cmd.cmd[i].p[0].p);
		} else if (cmd.cmd[i].inst == INST_ASM) {
			AddAsmBlock(list, script);
		} else {

			if (config.instruction_set == Asm::InstructionSet::ARM)
				assemble_cmd_arm(cmd.cmd[i]);
			else
				assemble_cmd(cmd.cmd[i]);
		}
	}
	list->add2(Asm::INST_ALIGN_OPCODE);
}


}
