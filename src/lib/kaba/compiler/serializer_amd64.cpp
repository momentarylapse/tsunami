#include "../kaba.h"
#include "serializer_amd64.h"
#include "../../file/file.h"



namespace Kaba{


int SerializerAMD64::fc_begin(Function *__f, const Array<SerialNodeParam> &_params, const SerialNodeParam &ret) {
	const Class *type = ret.get_type_save();

	// return data too big... push address
	SerialNodeParam ret_ref;
	if (type->uses_return_by_memory()){
		//add_temp(type, ret_temp);
		ret_ref = add_reference(/*ret_temp*/ ret);
		//add_ref();
		//add_cmd(Asm::inst_lea, KindRegister, (char*)RegEaxCompilerFunctionReturn.kind, CompilerFunctionReturn.param);
	}

	// grow stack (down) for local variables of the calling function
//	add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	int64 push_size = 0;

	Array<SerialNodeParam> params = _params;
		
	// instance as first parameter
/*	if (instance.type)
		params.insert(instance, 0);*/

	// return as _very_ first parameter
	if (type->uses_return_by_memory()){
		//add_temp(type, ret_temp);
//		ret_ref = AddReference(/*ret_temp*/ ret);
		params.insert(ret_ref, 0);
	}

	// map params...
	Array<SerialNodeParam> reg_param;
	Array<SerialNodeParam> stack_param;
	Array<SerialNodeParam> xmm_param;
	for (SerialNodeParam &p: params){
		if ((p.type == TypeInt) or (p.type == TypeInt64) or (p.type == TypeChar) or (p.type == TypeBool) or p.type->is_pointer()){
			if (reg_param.num < 6){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if ((p.type == TypeFloat32) or (p.type == TypeFloat64)){
			if (xmm_param.num < 8){
				xmm_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			do_error("parameter type currently not supported: " + p.type->name);
	}
	
	// push parameters onto stack
	push_size = 8 * stack_param.num;
	if (push_size > 127)
		add_cmd(Asm::INST_ADD, param_preg(TypePointer, Asm::REG_RSP), param_imm(TypeInt, push_size));
	else if (push_size > 0)
		add_cmd(Asm::INST_ADD, param_preg(TypePointer, Asm::REG_RSP), param_imm(TypeChar, push_size));
	foreachb(SerialNodeParam &p, stack_param){
		add_cmd(Asm::INST_MOV, param_preg(p.type, get_reg(0, p.type->size)), p);
		add_cmd(Asm::INST_PUSH, p_rax);
	}
	max_push_size = max(max_push_size, (int)push_size);

	// xmm0-7
	foreachib(SerialNodeParam &p, xmm_param, i){
		int reg = Asm::REG_XMM0 + i;
		if (p.type == TypeFloat64)
			add_cmd(Asm::INST_MOVSD, param_preg(TypeReg128, reg), p);
		else
			add_cmd(Asm::INST_MOVSS, param_preg(TypeReg128, reg), p);
	}
	
	Array<int> virts;

	// rdi, rsi,rdx, rcx, r8, r9 
	int param_regs_root[6] = {7, 6, 2, 1, 8, 9};
	foreachib(SerialNodeParam &p, reg_param, i){
		int root = param_regs_root[i];
		int preg = get_reg(root, p.type->size);
		if (preg >= 0){
			int v = add_virtual_reg(preg);
			virts.add(v);
			add_cmd(Asm::INST_MOV, param_vreg(p.type, v), p);
		}else{
			// some registers are not 8bit'able
			int v = add_virtual_reg(get_reg(root, 4));
			virts.add(v);
			int va = add_virtual_reg(Asm::REG_EAX);
			add_cmd(Asm::INST_MOV, param_vreg(p.type, va, Asm::REG_AL), p);
			add_cmd(Asm::INST_MOV, param_vreg(TypeReg32, v), param_vreg(p.type, va));
		}
	}

	// extend reg channels to call
	for (int v: virts)
		use_virtual_reg(v, cmd.num, cmd.num);

	return push_size;
}

void SerializerAMD64::fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	const Class *type = ret.get_type_save();

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and (!type->uses_return_by_memory())) {
		if (type->_amd64_allow_pass_in_xmm) {
			if (type == TypeFloat32) {
				add_cmd(Asm::INST_MOVSS, ret, p_xmm0);
			} else if (type == TypeFloat64) {
				add_cmd(Asm::INST_MOVSD, ret, p_xmm0);
			} else if (type->size == 8) { // float[2]
				add_cmd(Asm::INST_MOVLPS, ret, p_xmm0);
			} else if (type->size == 12) { // float[3]
				add_cmd(Asm::INST_MOVLPS, param_shift(ret, 0, TypeReg64), p_xmm0);
				add_cmd(Asm::INST_MOVSS, param_shift(ret, 8, TypeFloat32), p_xmm1);
			} else if (type->size == 16) { // float[4]
				// hmm, weird
				add_cmd(Asm::INST_MOVLPS, param_shift(ret, 0, TypeReg64), p_xmm0);
				//add_cmd(Asm::INST_MOVHPS, param_shift(ret, 8, TypeReg64), p_xmm0);
				add_cmd(Asm::INST_MOVLPS, param_shift(ret, 8, TypeReg64), p_xmm1);
				//add_cmd(Asm::INST_MOVUPS, ret, p_xmm0);
			} else {
				do_error("xmm return ..." + type->long_name());
			}
		} else if (type->size == 1) {
			int r = add_virtual_reg(Asm::REG_AL);
			add_cmd(Asm::INST_MOV, ret, param_vreg(type, r));
			set_virtual_reg(r, cmd.num - 2, cmd.num - 1);
		} else if (type->size == 4) {
			int r = add_virtual_reg(Asm::REG_EAX);
			add_cmd(Asm::INST_MOV, ret, param_vreg(type, r));
			set_virtual_reg(r, cmd.num - 2, cmd.num - 1);
		} else {
			int r = add_virtual_reg(Asm::REG_RAX);
			add_cmd(Asm::INST_MOV, ret, param_vreg(type, r));
			set_virtual_reg(r, cmd.num - 2, cmd.num - 1);
		}
	}
}

bool dist_fits_32bit(void *a, void *b) {
	int_p d = (int_p)a - (int_p)b;
	if (d < 0)
		d = -d;
	return (d < 0x70000000);
}

void SerializerAMD64::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	int push_size = fc_begin(f, params, ret);

	if (f->address) {
		if (dist_fits_32bit(f->address, script->opcode)) {
			// 32bit call distance
			add_cmd(Asm::INST_CALL, param_imm(TypeReg32, (int_p)f->address)); // the actual call
			// function pointer will be shifted later...(asm translates to RIP-relative)
		} else {
			// 64bit call distance
			add_cmd(Asm::INST_MOV, p_rax, param_imm(TypeReg64, (int_p)f->address));
			add_cmd(Asm::INST_CALL, p_rax);
		}
	} else if (f->_label >= 0) {
		if (f->owner() == syntax_tree) {
			// 32bit call distance
			add_cmd(Asm::INST_CALL, param_marker(TypeInt, f->_label));
		} else {
			// 64bit call distance
			add_cmd(Asm::INST_MOV, p_rax, param_marker(TypePointer, f->_label));
			add_cmd(Asm::INST_CALL, p_rax);
		}
	} else {
		do_error_link("could not link function " + f->signature());
	}

	fc_end(push_size, params, ret);
}

void SerializerAMD64::add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	int push_size = fc_begin(f, params, ret);

	add_cmd(Asm::INST_MOV, p_rax, params[0]); // self
	add_cmd(Asm::INST_MOV, p_rax, p_deref_eax); // vtable
	add_cmd(Asm::INST_ADD, p_rax, param_imm(TypeInt, 8 * f->virtual_index)); // vtable + n
	add_cmd(Asm::INST_MOV, p_rax, p_deref_eax); // vtable[n]
	add_cmd(Asm::INST_CALL, p_rax); // the actual call

	fc_end(push_size, params, ret);
}

void SerializerAMD64::add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	//do_error("pointer call not working...");
	call_used = true;
	int push_size = fc_begin(nullptr, params, ret);

	add_cmd(Asm::INST_MOV, p_rax, pointer);
	add_cmd(Asm::INST_CALL, p_rax); // the actual call

	fc_end(push_size, params, ret);
}


void SerializerAMD64::add_function_intro_params(Function *f)
{
	// return, instance, params
	Array<Variable*> param;
	if (f->return_type->uses_return_by_memory()){
		for (Variable *v: f->var)
			if (v->name == IDENTIFIER_RETURN_VAR){
				param.add(v);
				break;
			}
	}
	if (!f->is_static()){
		for (Variable *v: f->var)
			if (v->name == IDENTIFIER_SELF){
				param.add(v);
				break;
			}
	}
	for (int i=0;i<f->num_params;i++)
		param.add(f->var[i]);

	// map params...
	Array<Variable*> reg_param;
	Array<Variable*> stack_param;
	Array<Variable*> xmm_param;
	for (Variable *p: param){
		if ((p->type == TypeInt) or (p->type == TypeChar) or (p->type == TypeBool) or p->type->is_pointer()){
			if (reg_param.num < 6){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if ((p->type == TypeFloat32) or (p->type == TypeFloat64)){
			if (xmm_param.num < 8){
				xmm_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			do_error("parameter type currently not supported: " + p->type->name);
	}

	// xmm0-7
	foreachib(Variable *p, xmm_param, i){
		int reg = Asm::REG_XMM0 + i;
		if (p->type == TypeFloat64)
			add_cmd(Asm::INST_MOVSD, param_local(p->type, p->_offset), param_preg(TypeReg128, reg));
		else
			add_cmd(Asm::INST_MOVSS, param_local(p->type, p->_offset), param_preg(TypeReg128, reg));


	}

	// rdi, rsi,rdx, rcx, r8, r9
	int param_regs_root[6] = {7, 6, 2, 1, 8, 9};
	foreachib(Variable *p, reg_param, i){
		int root = param_regs_root[i];
		int preg = get_reg(root, p->type->size);
		if (preg >= 0){
			int v = add_virtual_reg(preg);
			add_cmd(Asm::INST_MOV, param_local(p->type, p->_offset), param_vreg(p->type, v));
			set_virtual_reg(v, cmd.num - 1, cmd.num - 1);
		}else{
			// some registers are not 8bit'able
			int v = add_virtual_reg(get_reg(root, 4));
			int va = add_virtual_reg(Asm::REG_EAX);
			add_cmd(Asm::INST_MOV, param_vreg(TypeReg32, va), param_vreg(TypeReg32, v));
			add_cmd(Asm::INST_MOV, param_local(p->type, p->_offset), param_vreg(p->type, va, get_reg(0, p->type->size)));
			set_virtual_reg(v, cmd.num - 2, cmd.num - 2);
			set_virtual_reg(va, cmd.num - 2, cmd.num - 1);
		}
	}
		
	// get parameters from stack
	foreachb(Variable *p, stack_param){
		do_error("func with stack...");
		/*int s = 8;
		add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}

void SerializerAMD64::add_function_outro(Function *f)
{
	add_cmd(Asm::INST_LEAVE);
	add_cmd(Asm::INST_RET);
}

//#define debug_evil_corrections

void _test_param_mem(SerialNodeParam &p) {
	//if (p.kind == NodeKind::ADDRESS)
}

void SerializerAMD64::correct_unallowed_param_combis2(SerialNode &c)
{
	// push 8 bit -> push 32 bit
	if (c.inst == Asm::INST_PUSH)
		if (c.p[0].kind == NodeKind::REGISTER)
			c.p[0].p = reg_resize(c.p[0].p, config.pointer_size);

	_test_param_mem(c.p[0]);
	_test_param_mem(c.p[1]);



	// FIXME
	// evil hack to allow inconsistent param types (in address shifts)
	if (config.instruction_set == Asm::InstructionSet::AMD64){
		if ((c.inst == Asm::INST_ADD) or (c.inst == Asm::INST_MOV)){
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
			if ((c.p[0].type->size == 8) and (c.p[1].type->size == 4)){
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
				}else*/ if (c.p[1].kind == NodeKind::REGISTER){
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
			if ((c.p[0].type->size < 8) and (c.p[1].type->size == 8)){
				if ((c.p[0].kind == NodeKind::REGISTER) and ((c.p[1].kind == NodeKind::REGISTER) or (c.p[1].kind == NodeKind::DEREF_REGISTER))){
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

};
