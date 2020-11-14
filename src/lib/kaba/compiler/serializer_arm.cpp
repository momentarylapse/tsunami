#include "../kaba.h"
#include "serializer_arm.h"

#include "../../file/file.h"


namespace Asm{
	extern int ARM_DATA_INSTRUCTIONS[16]; // -> asm.cpp
};

namespace kaba {

#define reg_s0 param_preg(TypeFloat32, Asm::REG_S0)
#define reg_s1 param_preg(TypeFloat32, Asm::REG_S1)

int func_index(Function *f);


int SerializerARM::fc_begin(Function *f, const Array<SerialNodeParam> &_params, const SerialNodeParam &ret)
{
	const Class *type = ret.type;
	if (!type)
		type = TypeVoid;

	// grow stack (down) for local variables of the calling function
//	cmd.add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	int64 push_size = 0;

	Array<SerialNodeParam> params = _params;

	// instance as first parameter
	//if (instance.type)
	//	params.insert(instance, 0);

	// return as _very_ first parameter
	if (type->uses_return_by_memory()){
		SerialNodeParam ret_ref = add_reference(ret);
		params.insert(ret_ref, 0);
	}

	// map params...
	Array<SerialNodeParam> reg_param;
	Array<SerialNodeParam> stack_param;
	Array<SerialNodeParam> float_param;
	for (SerialNodeParam &p: params){
		if ((p.type == TypeInt) /*or (p.type == TypeInt64)*/ or (p.type == TypeChar) or (p.type == TypeBool) or p.type->is_some_pointer()){
			if (reg_param.num < 4){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if ((p.type == TypeFloat32) /*or (p.type == TypeFloat64)*/){
			if (float_param.num < 8){
				float_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			do_error("parameter type currently not supported: " + p.type->name);
	}

	// push parameters onto stack
/*	push_size = 4 * stack_param.num;
	if (push_size > 127)
		cmd.add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::REG_RSP), param_const(TypeInt, (void*)push_size));
	else if (push_size > 0)
		cmd.add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::REG_RSP), param_const(TypeChar, (void*)push_size));
	foreachb(SerialCommandParam &p, stack_param)
		cmd.add_cmd(Asm::inst_push, p);
	max_push_size = max(max_push_size, push_size);*/

	// s0-7
	foreachib(auto &p, float_param, i){
		int reg = Asm::REG_S0 + i;
		/*if (p.type == TypeFloat64)
			cmd.add_cmd(Asm::inst_movsd, param_reg(TypeReg128, reg), p);
		else*/
			cmd.add_cmd(Asm::INST_FLDS, param_preg(TypeFloat32, reg), p);
	}

	// r0, r1, r2, r3
	foreachib(auto &p, reg_param, i){
		int v = cmd.add_virtual_reg(Asm::REG_R0 + i);
		cmd.add_cmd(Asm::INST_MOV, param_vreg(p.type, v), p);
		cmd.set_virtual_reg(v, cmd.cmd.num - 1, -100); // -> call
	}

	// extend reg channels to call
	for (VirtualRegister &r: cmd.virtual_reg)
		if (r.last == -100)
			r.last = cmd.cmd.num;

	return push_size;
}

void SerializerARM::fc_end(int push_size, const SerialNodeParam &ret) {
	const Class *type = ret.type;
	if (!type)
		return;

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and (!type->uses_return_by_memory())) {
		if (type == TypeFloat32) {
			cmd.add_cmd(Asm::INST_FSTS, ret, reg_s0);
		//else if (type == TypeFloat64)
			//cmd.add_cmd(Asm::INST_MOVSD, ret, param_preg(TypeReg128, Asm::REG_XMM0));
		} else if ((type->size == 1) or (type->size == 4)) {
			int v = cmd.add_virtual_reg(Asm::REG_R0);
			cmd.add_cmd(Asm::INST_MOV, ret, param_vreg(TypeReg32, v));
			cmd.set_virtual_reg(v, cmd.cmd.num - 2, cmd.cmd.num - 1);
		} else {
			do_error("unhandled function value receiving... " + type->long_name());
			int v = cmd.add_virtual_reg(Asm::REG_R0);
			cmd.add_cmd(Asm::INST_MOV, ret, param_vreg(TypeReg32, v));
			cmd.set_virtual_reg(v, cmd.cmd.num - 2, cmd.cmd.num - 1);
		}
	}
}

void SerializerARM::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	int push_size = fc_begin(f, params, ret);

	if ((f->owner() == syntax_tree) and (!f->is_extern())) {
		cmd.add_cmd(Asm::INST_CALL, param_marker(TypePointer, f->_label));
	} else {
		if (!f->address)
			do_error_link("could not link function " + f->long_name());
		if (abs((int_p)f->address - (int_p)this->script->opcode) < 30000000) {
			cmd.add_cmd(Asm::INST_CALL, param_imm(TypePointer, (int_p)f->address)); // the actual call
			// function pointer will be shifted later...
		} else {

			// TODO FIXME
			// really find a usable register...

			int v = cmd.add_virtual_reg(Asm::REG_R4);//find_unused_reg(cmd.cmd.num-1, cmd.cmd.num-1, 4);
			cmd.add_cmd(Asm::INST_MOV, param_vreg(TypePointer, v), param_lookup(TypePointer, add_global_ref(f->address)));
			cmd.add_cmd(Asm::INST_CALL, param_vreg(TypePointer, v));
			cmd.set_virtual_reg(v, cmd.cmd.num-2, cmd.cmd.num-1);
		}
	}

	fc_end(push_size, ret);
}

void SerializerARM::add_virtual_function_call(Function *f, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) {
	call_used = true;
	int push_size = fc_begin(f, param, ret);

	int v1 = cmd.add_virtual_reg(Asm::REG_R4);//find_unused_reg(cmd.cmd.num-1, cmd.cmd.num-1, 4);
	int v2 = cmd.add_virtual_reg(Asm::REG_R5);//find_unused_reg(cmd.cmd.num-1, cmd.cmd.num-1, 4);
	cmd.add_cmd(Asm::INST_MOV, param_vreg(TypePointer, v1), param[0]);
	cmd.add_cmd(Asm::INST_MOV, param_vreg(TypePointer, v2), param_deref_vreg(TypePointer, v1));
	cmd.add_cmd(Asm::INST_MOV, param_vreg(TypePointer, v1), param_imm(TypeInt, 4*f->virtual_index));
	cmd.add_cmd(Asm::INST_ADD, param_vreg(TypePointer, v1), param_vreg(TypePointer, v2), param_vreg(TypePointer, v1));
	cmd.add_cmd(Asm::INST_MOV, param_vreg(TypePointer, v2), param_deref_vreg(TypePointer, v1));
	cmd.add_cmd(Asm::INST_CALL, param_vreg(TypePointer, v2));
	cmd.set_virtual_reg(v1, cmd.cmd.num-5, cmd.cmd.num-1);
	cmd.set_virtual_reg(v2, cmd.cmd.num-5, cmd.cmd.num-1);

	fc_end(push_size, ret);
}


void SerializerARM::add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) {
	call_used = true;
	do_error("pointer call");
}

void SerializerARM::serialize_statement(Node *com, const SerialNodeParam &ret, Block *block, int index)
{
	auto statement = com->as_statement();
	switch(statement->id){
		case StatementID::IF:{
			int m_after_true = list->create_label("_IF_AFTER_" + i2s(num_markers ++));
			auto cond = serialize_parameter(com->params[0].get(), block, index);
			// cmp;  jz m;  -block-  m;
			cmd.add_cmd(Asm::INST_CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker32(m_after_true), p_none, p_none);
			cmd.add_marker(m_after_true);
			}break;
		case StatementID::IF_ELSE:{
			int m_after_true = list->create_label("_IF_AFTER_TRUE_" + i2s(num_markers ++));
			int m_after_false = list->create_label("_IF_AFTER_FALSE_" + i2s(num_markers ++));
			auto cond = serialize_parameter(com->params[0].get(), block, index);
			// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
			cmd.add_cmd(Asm::INST_CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker32(m_after_true), p_none, p_none); // jz ...
			cmd.add_marker(m_after_true);
			serialize_block(com->params[2]->as_block());
			cmd.add_marker(m_after_false);
			}break;
		case StatementID::WHILE:{
			int marker_before_while = list->create_label("_WHILE_BEFORE_" + i2s(num_markers ++));
			int marker_after_while = list->create_label("_WHILE_AFTER_" + i2s(num_markers ++));
			cmd.add_marker(marker_before_while);
			auto cond = serialize_parameter(com->params[0].get(), block, index); // while
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			cmd.add_cmd(Asm::INST_CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker32(marker_after_while), p_none, p_none);

			// body of loop
			LoopData l = {marker_before_while, marker_after_while, block->level, index};
			loop.add(l);
			serialize_block(com->params[1]->as_block());
			loop.pop();

			cmd.add_cmd(Asm::INST_JMP, param_marker32(marker_before_while));
			cmd.add_marker(marker_after_while);
			}break;
		case StatementID::FOR_DIGEST:{
			int marker_before_for = list->create_label("_FOR_BEFORE_" + i2s(num_markers ++));
			int marker_after_for = list->create_label("_FOR_AFTER_" + i2s(num_markers ++));
			int marker_continue = list->create_label("_FOR_CONTINUE_" + i2s(num_markers ++));
			serialize_node(com->params[0].get(), block, index); // i=0
			cmd.add_marker(marker_before_for);
			auto cond = serialize_parameter(com->params[1].get(), block, index); // for
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			cmd.add_cmd(Asm::INST_CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker32(marker_after_for), p_none, p_none);

			// body of loop
			LoopData l = {marker_continue, marker_after_for, block->level, index};
			loop.add(l);
			serialize_block(com->params[2]->as_block());
			loop.pop();

			// "i++"
			cmd.add_marker(marker_continue);
			serialize_node(com->params[3].get(), block, index);

			cmd.add_cmd(Asm::INST_JMP, param_marker32(marker_before_for));
			cmd.add_marker(marker_after_for);
			}break;
		case StatementID::BREAK:
			cmd.add_cmd(Asm::INST_B, param_marker32(loop.back().marker_break));
			break;
		case StatementID::CONTINUE:
			cmd.add_cmd(Asm::INST_B, param_marker32(loop.back().marker_continue));
			break;
		case StatementID::RETURN:
			if (com->params.num > 0){
				auto operand = serialize_parameter(com->params[0].get(), block, index);

				if (cur_func->effective_return_type->uses_return_by_memory()){ // we already got a return address in [ebp+0x08] (> 4 byte)
					insert_destructors_block(block, true);
					// internally handled...

					add_function_outro(cur_func);
				}else{ // store return directly in eax / fpu stack (4 byte)
//					SerialNodeParam t = add_temp(cur_func->return_type);
					insert_destructors_block(block, true);
					if ((cur_func->effective_return_type == TypeInt) or (cur_func->effective_return_type->size == 1)) {
						int v = cmd.add_virtual_reg(Asm::REG_R0);
						cmd.add_cmd(Asm::INST_MOV, param_vreg(cur_func->effective_return_type, v), operand);
						cmd.set_virtual_reg(v, cmd.cmd.num-1, cmd.cmd.num);
					} else if (cur_func->effective_return_type == TypeFloat32) {
						//int v = cmd.add_virtual_reg(Asm::REG_S0);
						//cmd.add_cmd(Asm::INST_MOV, param_vreg(cur_func->effective_return_type, v), operand);
						cmd.add_cmd(Asm::INST_FLDS, reg_s0, operand);
						//cmd.set_virtual_reg(v, cmd.cmd.num-1, cmd.cmd.num);
					} else {
						do_error("return != int");
					}
					add_function_outro(cur_func);
				}
			}else{
				insert_destructors_block(block, true);
				add_function_outro(cur_func);
			}
			break;
		case StatementID::NEW:{
			// malloc()
			auto f = syntax_tree->required_func_global("@malloc");
			add_function_call(f, {param_imm(TypeInt, ret.type->parent->size)}, ret);

			// __init__()
			auto sub = com->params[0];
			Node *c_ret = new Node(NodeKind::VAR_TEMP, ret.p, ret.type);
			sub->set_instance(c_ret);
			serialize_node(sub.get(), block, index);
			//delete sub;
			break;}
		case StatementID::DELETE:{
			// __delete__()
			auto operand = serialize_parameter(com->params[0].get(), block, index);
			add_cmd_destructor(operand, false);

			// free()
			auto f = syntax_tree->required_func_global("@free");
			add_function_call(f, {operand}, p_none);
			break;}
		case StatementID::ASM:
			cmd.add_cmd(INST_ASM);
			//AddAsmBlock(list, script);
			break;
		case StatementID::PASS:
			break;
		default:
			do_error("statement unimplemented: " + statement->name);
	}
}

void SerializerARM::serialize_inline_function(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) {
	auto index = com->as_func()->inline_no;
	switch(index){
		case InlineID::INT_ASSIGN:
		case InlineID::FLOAT_ASSIGN:
		case InlineID::POINTER_ASSIGN:
			cmd.add_cmd(Asm::INST_MOV, param[0], param[1]);
			break;
		case InlineID::CHAR_ASSIGN:
		case InlineID::BOOL_ASSIGN:
			cmd.add_cmd(Asm::INST_MOV, param[0], param[1]);
			break;
		case InlineID::CHUNK_ASSIGN:
		case InlineID::INT64_ASSIGN:
		case InlineID::FLOAT64_ASSIGN:
			for (int i=0; i<(com->params[0]->type->size/4); i++)
				cmd.add_cmd(Asm::INST_MOV, param_shift(param[0], i * 4, TypeInt), param_shift(param[1], i * 4, TypeInt));
			for (int i=4*(com->params[0]->type->size/4); i<com->params[0]->type->size;i++)
				cmd.add_cmd(Asm::INST_MOV, param_shift(param[0], i, TypeChar), param_shift(param[1], i, TypeChar));
			break;
// int
		case InlineID::INT_ADD_ASSIGN:
			cmd.add_cmd(Asm::INST_ADD, param[0], param[0], param[1]);
			break;
		case InlineID::INT_SUBTRACT_ASSIGN:
			cmd.add_cmd(Asm::INST_SUB, param[0], param[0], param[1]);
			break;
		case InlineID::INT_MULTIPLY_ASSIGN:
			cmd.add_cmd(Asm::INST_IMUL, param[0], param[0], param[1]);
			break;
		case InlineID::INT_ADD:
			cmd.add_cmd(Asm::INST_ADD, ret, param[0], param[1]);
			break;
		case InlineID::INT_SUBTRACT:
			cmd.add_cmd(Asm::INST_SUB, ret, param[0], param[1]);
			break;
		case InlineID::INT_MULTIPLY:
			cmd.add_cmd(Asm::INST_MUL, ret, param[0], param[1]);
			break;
		case InlineID::INT_EQUAL:
		case InlineID::POINTER_EQUAL:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			cmd.add_cmd(Asm::ARM_COND_EQUAL,     Asm::INST_MOV, ret, param_imm(TypeBool, 1), p_none);
			cmd.add_cmd(Asm::ARM_COND_NOT_EQUAL, Asm::INST_MOV, ret, param_imm(TypeBool, 0), p_none);
			break;
		case InlineID::INT_NOT_EQUAL:
		case InlineID::POINTER_NOT_EQUAL:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			cmd.add_cmd(Asm::ARM_COND_NOT_EQUAL, Asm::INST_MOV, ret, param_imm(TypeBool, 1), p_none);
			cmd.add_cmd(Asm::ARM_COND_EQUAL,     Asm::INST_MOV, ret, param_imm(TypeBool, 0), p_none);
			break;
		case InlineID::INT_GREATER:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			cmd.add_cmd(Asm::ARM_COND_GREATER_THAN, Asm::INST_MOV, ret, param_imm(TypeBool, 1), p_none);
			cmd.add_cmd(Asm::ARM_COND_LESS_EQUAL,   Asm::INST_MOV, ret, param_imm(TypeBool, 0), p_none);
			break;
		case InlineID::INT_GREATER_EQUAL:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			cmd.add_cmd(Asm::ARM_COND_GREATER_EQUAL, Asm::INST_MOV, ret, param_imm(TypeBool, 1), p_none);
			cmd.add_cmd(Asm::ARM_COND_LESS_THAN,     Asm::INST_MOV, ret, param_imm(TypeBool, 0), p_none);
			break;
		case InlineID::INT_SMALLER:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			cmd.add_cmd(Asm::ARM_COND_LESS_THAN,     Asm::INST_MOV, ret, param_imm(TypeBool, 1), p_none);
			cmd.add_cmd(Asm::ARM_COND_GREATER_EQUAL, Asm::INST_MOV, ret, param_imm(TypeBool, 0), p_none);
			break;
		case InlineID::INT_SMALLER_EQUAL:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			cmd.add_cmd(Asm::ARM_COND_LESS_EQUAL,   Asm::INST_MOV, ret, param_imm(TypeBool, 1), p_none);
			cmd.add_cmd(Asm::ARM_COND_GREATER_THAN, Asm::INST_MOV, ret, param_imm(TypeBool, 0), p_none);
			break;
		case InlineID::INT_AND:
			cmd.add_cmd(Asm::INST_AND, ret, param[0], param[1]);
			break;
		case InlineID::INT_OR:
			cmd.add_cmd(Asm::INST_OR, ret, param[0], param[1]);
			break;
		case InlineID::INT_NEGATE:
			cmd.add_cmd(Asm::INST_MOV, ret, param_imm(TypeInt, 0x0));
			cmd.add_cmd(Asm::INST_SUB, ret, ret, param[0]);
			break;
		case InlineID::INT_INCREASE:
			cmd.add_cmd(Asm::INST_ADD, param[0], param[0], param_imm(TypeInt, 0x1));
			break;
		case InlineID::INT_DECREASE:
			cmd.add_cmd(Asm::INST_SUB, param[0], param[0], param_imm(TypeInt, 0x1));
			break;
	// bool/char
		case InlineID::CHAR_EQUAL:
		case InlineID::CHAR_NOT_EQUAL:
		case InlineID::BOOL_EQUAL:
		case InlineID::BOOL_NOT_EQUAL:
		case InlineID::CHAR_GREATER:
		case InlineID::CHAR_GREATER_EQUAL:
		case InlineID::CHAR_SMALLER:
		case InlineID::CHAR_SMALLER_EQUAL:
			cmd.add_cmd(Asm::INST_CMP, param[0], param[1]);
			if ((index == InlineID::CHAR_EQUAL) or (index == InlineID::BOOL_EQUAL))
				cmd.add_cmd(Asm::INST_SETZ, ret);
			else if ((index ==InlineID::CHAR_NOT_EQUAL) or (index == InlineID::BOOL_NOT_EQUAL))
				cmd.add_cmd(Asm::INST_SETNZ, ret);
			else if (index == InlineID::CHAR_GREATER)
				cmd.add_cmd(Asm::INST_SETNLE, ret);
			else if (index == InlineID::CHAR_GREATER_EQUAL)
				cmd.add_cmd(Asm::INST_SETNL, ret);
			else if (index == InlineID::CHAR_SMALLER)
				cmd.add_cmd(Asm::INST_SETL, ret);
			else if (index == InlineID::CHAR_SMALLER_EQUAL)
				cmd.add_cmd(Asm::INST_SETLE, ret);
			break;
		case InlineID::BOOL_AND:
			cmd.add_cmd(Asm::INST_AND, ret, param[0], param[1]);
			break;
		case InlineID::BOOL_OR:
			cmd.add_cmd(Asm::INST_OR, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_ADD_ASSIGN:
			cmd.add_cmd(Asm::INST_ADD, param[0], param[0], param[1]);
			break;
		case InlineID::CHAR_SUBTRACT_ASSIGN:
			cmd.add_cmd(Asm::INST_SUB, param[0], param[0], param[1]);
			break;
		case InlineID::CHAR_ADD:
			cmd.add_cmd(Asm::INST_ADD, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_SUBTRACT:
			cmd.add_cmd(Asm::INST_SUB, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_AND:
			cmd.add_cmd(Asm::INST_AND, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_OR:
			cmd.add_cmd(Asm::INST_OR, ret, param[0], param[1]);
			break;
		case InlineID::BOOL_NEGATE:
			cmd.add_cmd(Asm::INST_XOR, ret, param[0], param_imm(TypeBool, 0x1));
			break;
		case InlineID::CHAR_NEGATE:
			cmd.add_cmd(Asm::INST_MOV, ret, param_imm(TypeChar, 0x0));
			cmd.add_cmd(Asm::INST_SUB, ret, ret, param[0]);
			break;
		case InlineID::FLOAT_ADD_ASSIGN:
		case InlineID::FLOAT_SUBTRACT_ASSIGN:
		case InlineID::FLOAT_MULTIPLY_ASSIGN:
		case InlineID::FLOAT_DIVIDE_ASSIGN:
			cmd.add_cmd(Asm::INST_FLDS, reg_s0, param[0]);
			cmd.add_cmd(Asm::INST_FLDS, reg_s1, param[1]);
			if (index == InlineID::FLOAT_ADD_ASSIGN)
				cmd.add_cmd(Asm::INST_FADDS, reg_s0, reg_s0, reg_s1);
			if (index == InlineID::FLOAT_SUBTRACT_ASSIGN)
				cmd.add_cmd(Asm::INST_FSUBS, reg_s0, reg_s0, reg_s1);
			if (index == InlineID::FLOAT_MULTIPLY_ASSIGN)
				cmd.add_cmd(Asm::INST_FMULS, reg_s0, reg_s0, reg_s1);
			if (index == InlineID::FLOAT_DIVIDE_ASSIGN)
				cmd.add_cmd(Asm::INST_FDIVS, reg_s0, reg_s0, reg_s1);
			cmd.add_cmd(Asm::INST_FSTS, param[0], reg_s0);
			break;
		case InlineID::FLOAT_ADD:
		case InlineID::FLOAT_SUBTARCT:
		case InlineID::FLOAT_MULTIPLY:
		case InlineID::FLOAT_DIVIDE:
			cmd.add_cmd(Asm::INST_FLDS, reg_s0, param[0]);
			cmd.add_cmd(Asm::INST_FLDS, reg_s1, param[1]);
			if (index == InlineID::FLOAT_ADD)
				cmd.add_cmd(Asm::INST_FADDS, reg_s0, reg_s0, reg_s1);
			if (index == InlineID::FLOAT_SUBTARCT)
				cmd.add_cmd(Asm::INST_FSUBS, reg_s0, reg_s0, reg_s1);
			if (index == InlineID::FLOAT_MULTIPLY)
				cmd.add_cmd(Asm::INST_FMULS, reg_s0, reg_s0, reg_s1);
			if (index == InlineID::FLOAT_DIVIDE)
				cmd.add_cmd(Asm::INST_FDIVS, reg_s0, reg_s0, reg_s1);
			cmd.add_cmd(Asm::INST_FSTS, ret, reg_s0);
			break;
		case InlineID::RECT_SET:
		case InlineID::COLOR_SET:
			cmd.add_cmd(Asm::INST_MOV, param_shift(ret, 12, TypeFloat32), param[3]);
			/* fall through */
		case InlineID::VECTOR_SET:
			cmd.add_cmd(Asm::INST_MOV, param_shift(ret, 8, TypeFloat32), param[2]);
		case InlineID::COMPLEX_SET:
			cmd.add_cmd(Asm::INST_MOV, param_shift(ret, 4, TypeFloat32), param[1]);
			cmd.add_cmd(Asm::INST_MOV, param_shift(ret, 0, TypeFloat32), param[0]);
			break;
		default:
			do_error("unimplemented inline function: #" + i2s((int)index));
	}
}

bool const_is_arm_representable(int value)
{
	for (int ex=0; ex<=30; ex+=2){
		unsigned int mask = (0xffffff00 >> ex) | (0xffffff00 << (32-ex));
		if ((value & mask) == 0)
			return true;
	}
	return false;
}

int func_index(Function *f) {
	f->owner()->do_error("ARM func_index...");
	return -1;
}

// create data for a (function) parameter
//   and compile its command if the parameter is executable itself
SerialNodeParam SerializerARM::serialize_parameter(Node *link, Block *block, int index) {
	SerialNodeParam p;
	p.kind = link->kind;
	p.type = link->type;
	p.p = 0;
	p.shift = 0;

	if (link->kind == NodeKind::MEMORY){
		return param_deref_lookup(p.type, add_global_ref((void*)p.p));
	}else if (link->kind == NodeKind::ADDRESS){
		return param_lookup(p.type, add_global_ref((void*)link->link_no));
	}else if (link->kind == NodeKind::VAR_GLOBAL){
		if (!link->as_global_p())
			script->do_error_link("variable is not linkable: " + link->as_global()->name);
		return param_deref_lookup(p.type, add_global_ref(link->as_global_p()));
	}else if (link->kind == NodeKind::VAR_LOCAL){
		p.p = link->as_local()->_offset;
		p.kind = NodeKind::LOCAL_MEMORY;
	}else if (link->kind == NodeKind::LOCAL_MEMORY){
		p.p = link->link_no;
	}else if (link->kind == NodeKind::LOCAL_ADDRESS){
		SerialNodeParam param = param_local(TypePointer, link->link_no);
		return add_reference(param, link->type);
	}else if (link->kind == NodeKind::CONSTANT){
		p.p = link->link_no;
		p.kind = NodeKind::CONSTANT;
		return p;
		void *pp = link->as_const_p();
		int c = *(int*)pp;
		//if (const_is_arm_representable(c)){
			p.p = c;
			p.kind = NodeKind::IMMEDIATE;
		/*}else{
			return param_lookup(p.type, add_global_ref(*(int**)pp));
		}*/
	}else if ((link->kind==NodeKind::OPERATOR) or (link->kind==NodeKind::FUNCTION_CALL) or (link->kind==NodeKind::VIRTUAL_CALL) or (link->kind==NodeKind::INLINE_CALL) or (link->kind == NodeKind::STATEMENT)){
		return serialize_node(link, block, index);
	}else if (link->kind == NodeKind::REFERENCE){
		auto param = serialize_parameter(link->params[0].get(), block, index);
		//printf("%d  -  %s\n",pk,Kind2Str(pk));
		return add_reference(param, link->type);
	}else if (link->kind == NodeKind::DEREFERENCE){
		auto param = serialize_parameter(link->params[0].get(), block, index);
		/*if ((param.kind == KindVarLocal) or (param.kind == KindVarGlobal)){
			p.type = param.type->sub_type;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		return add_dereference(param, link->type);
	}else if (link->kind == NodeKind::VAR_TEMP){
		// only used by <new> operator
		p.p = link->link_no;
	}else{
		do_error("unexpected type of parameter: " + kind2str(link->kind));
	}
	return p;
}


void SerializerARM::process_references()
{
	for (int i=0;i<cmd.cmd.num;i++)
		if (cmd.cmd[i].inst == Asm::INST_LEA){
			if (cmd.cmd[i].p[1].kind == NodeKind::LOCAL_MEMORY){
				//do_error("var local/local mem");
				auto p0 = cmd.cmd[i].p[0];
				auto p1 = cmd.cmd[i].p[1];
				int r = find_unused_reg(i, i, 4);
				cmd.remove_cmd(i);
				cmd.next_cmd_target(i);
				cmd.add_cmd(Asm::INST_ADD, param_vreg(TypePointer, r), param_preg(TypePointer, Asm::REG_R13), param_imm(TypeInt, p1.p));
				cmd.next_cmd_target(i + 1);
				cmd.add_cmd(Asm::INST_MOV, p0, param_vreg(TypePointer, r));
				cmd.set_virtual_reg(r, i, i+1);
			}else{
				do_error("reference in ARM: " + cmd.cmd[i].p[1].str(this));
			}
		}
}

void SerializerARM::process_dereferences()
{
	for (int i=0;i<cmd.cmd.num;i++)
		for (int j=0;j<SERIAL_NODE_NUM_PARAMS;j++)
			if ((cmd.cmd[i].p[j].kind == NodeKind::DEREF_LOCAL_MEMORY) or (cmd.cmd[i].p[j].kind == NodeKind::DEREF_VAR_TEMP)){
				SerialNodeParam p = cmd.cmd[i].p[j];
				SerialNodeParam rp = cmd.cmd[i].p[j];
				if (cmd.cmd[i].p[j].kind == NodeKind::DEREF_LOCAL_MEMORY)
					rp.kind = NodeKind::LOCAL_MEMORY;
				else
					rp.kind = NodeKind::VAR_TEMP;
				rp.type = p.type->get_pointer();
				int r = find_unused_reg(i, i, 4);
				cmd.next_cmd_target(i);
				cmd.add_cmd(Asm::INST_MOV, param_vreg(TypePointer, r), rp);
				cmd.set_cmd_param(cmd.cmd[i+1], j, param_deref_vreg(p.type, r));
				cmd.set_virtual_reg(r, i, i+1);
			}
}


inline bool _____arm_param_combi_allowed(int inst, SerialNodeParam &p1, SerialNodeParam &p2, SerialNodeParam &p3)
{
//	if (inst >= Asm::inst_marker)
//		return true;
	if (inst == Asm::INST_MOV)
		return (p1.kind == NodeKind::REGISTER) and (p2.kind == NodeKind::REGISTER);
	if (inst == Asm::INST_ADD)
		return (p1.kind == NodeKind::REGISTER) and (p2.kind == NodeKind::REGISTER) and (p3.kind == NodeKind::REGISTER);
	return true;
}

void SerializerARM::transfer_by_reg_in(SerialNode &c, int &i, int pno)
{
	SerialNodeParam p = c.p[pno];
	int r = find_unused_reg(i, i, /*p.type->size*/ 4);
	SerialNodeParam pr = param_vreg(p.type, r);
	cmd.set_virtual_reg(r, i, cmd.cmd.num);
	cmd.set_cmd_param(cmd.cmd[i], pno, pr);
	cmd.next_cmd_target(i);
	cmd.add_cmd(c.cond, Asm::INST_MOV, pr, p, p_none);
	i ++;
}

void SerializerARM::transfer_by_reg_out(SerialNode &c, int &i, int pno)
{
	SerialNodeParam p = c.p[pno];
	int r = find_unused_reg(i, i, 4);//p.type->size);
	SerialNodeParam pr = param_vreg(p.type, r);
	cmd.set_virtual_reg(r, i, cmd.cmd.num);
	cmd.set_cmd_param(c, pno, pr);
	cmd.next_cmd_target(i+1);
	cmd.add_cmd(c.cond, Asm::INST_MOV, p, pr, p_none);
}

void SerializerARM::gr_transfer_by_reg_in(SerialNode &c, int &i, int pno)
{
	SerialNodeParam p = c.p[pno];
	if (config.verbose)
		msg_write("in " + c.str(this));
	if (p.kind == NodeKind::DEREF_GLOBAL_LOOKUP){
		// cmd ..., [global ref]

		// mov r2, [ref]
		// ldr r1, [r2]
		// cmd ..., r1


		int r2 = find_unused_reg(i, i, 4);
		cmd.next_cmd_target(i);
		cmd.add_cmd(c.cond, Asm::INST_MOV, param_vreg(TypePointer, r2), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);

		int r1 = find_unused_reg(i+1, i+1, 4/*p.type->size*/, cmd.virtual_reg[r2].reg_root);
		cmd.next_cmd_target(i+1);
		cmd.add_cmd(c.cond, Asm::INST_LDR, param_vreg(p.type, r1), param_deref_vreg(TypePointer, r2), p_none);

		cmd.set_cmd_param(cmd.cmd[i+2], pno, param_vreg(p.type, r1));

		cmd.set_virtual_reg(r2, i, i + 1);
		cmd.set_virtual_reg(r1, i + 1, i + 2);

		i += 2;
	}else{
		// cmd ..., global

		// mov r1, [ref]
		// cmd ..., r1


		int r1 = find_unused_reg(i, i, 4);
		cmd.next_cmd_target(i);
		cmd.add_cmd(c.cond, Asm::INST_MOV, param_vreg(TypePointer, r1), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);
		cmd.set_virtual_reg(r1, i, i + 1);

		cmd.set_cmd_param(cmd.cmd[i+1], pno, param_vreg(p.type, r1));

		i += 1;
	}
}

void SerializerARM::gr_transfer_by_reg_out(SerialNode &c, int &i, int pno)
{
	SerialNodeParam p = c.p[pno];
	if (config.verbose)
		msg_write("out " + c.str(this));
	if (p.kind == NodeKind::DEREF_GLOBAL_LOOKUP){
		// cmd [global ref], ...

		// cmd r1, ...
		// mov r2, [ref]
		// str r1, [r2]


		int r2 = find_unused_reg(i, i, 4);
		cmd.next_cmd_target(i+1);
		cmd.add_cmd(c.cond, Asm::INST_MOV, param_vreg(TypePointer, r2), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);

		int r1 = find_unused_reg(i, i+1, 4 /*p.type->size*/, cmd.virtual_reg[r2].reg_root);
		cmd.next_cmd_target(i+2);
		cmd.add_cmd(c.cond, Asm::INST_STR, param_vreg(p.type, r1), param_deref_vreg(TypePointer, r2), p_none);

		cmd.set_cmd_param(cmd.cmd[i], pno, param_vreg(p.type, r1));

		cmd.set_virtual_reg(r1, i, i + 2);
		cmd.set_virtual_reg(r2, i+1, i+2);
	}else{
		// cmd global, ...

		// cmd r1, ...
		// str r1, [ref]


		int r1 = find_unused_reg(i, i, 4);
		cmd.next_cmd_target(i+2);
		cmd.add_cmd(c.cond, Asm::INST_STR, param_vreg(TypePointer, r1), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);
		cmd.set_virtual_reg(r1, i, i+1);

		cmd.set_cmd_param(cmd.cmd[i], pno, param_vreg(p.type, r1));
	}
}

inline bool is_data_op3(int inst)
{
	if (inst == Asm::INST_MOV)
		return false;
	if (inst == Asm::INST_MUL)
		return true;
	for (int i=0; i<16; i++)
		if (inst == Asm::ARM_DATA_INSTRUCTIONS[i])
			return true;
	return false;
}

inline bool is_data_op2(int inst)
{
	if (inst == Asm::INST_MOV)
		return true;
	if (inst == Asm::INST_CMP)
		return true;
	if (inst == Asm::INST_CMN)
		return true;
	if (inst == Asm::INST_TST)
		return true;
	if (inst == Asm::INST_TEQ)
		return true;
	return false;
}

inline bool is_global_lookup(SerialNodeParam &p)
{
	return (p.kind == NodeKind::DEREF_GLOBAL_LOOKUP) or (p.kind == NodeKind::GLOBAL_LOOKUP);
}

// create global lookup accesses
void SerializerARM::convert_global_lookups()
{
	for (int i=cmd.cmd.num-1;i>=0;i--){
		if (cmd.cmd[i].inst == Asm::INST_MOV){
			if (is_global_lookup(cmd.cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd.cmd[i], i, 1);
			if (is_global_lookup(cmd.cmd[i].p[0]))
				gr_transfer_by_reg_out(cmd.cmd[i], i, 0);
		}else if (is_data_op2(cmd.cmd[i].inst)){
			if (is_global_lookup(cmd.cmd[i].p[0]))
				gr_transfer_by_reg_in(cmd.cmd[i], i, 0);
			if (is_global_lookup(cmd.cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd.cmd[i], i, 1);
		}else if (is_data_op3(cmd.cmd[i].inst)){
			if (is_global_lookup(cmd.cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd.cmd[i], i, 1);
			if (is_global_lookup(cmd.cmd[i].p[2]))
				gr_transfer_by_reg_in(cmd.cmd[i], i, 2);
			if (is_global_lookup(cmd.cmd[i].p[0]))
				gr_transfer_by_reg_out(cmd.cmd[i], i, 0);
		}else if (cmd.cmd[i].inst == Asm::INST_FLDS){
			if (is_global_lookup(cmd.cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd.cmd[i], i, 1);
		}
	}
	scan_temp_var_usage();
}

void SerializerARM::split_mov_reg_immediate(SerialNode &c, int &i) {
	int64 v = c.p[1].p;
	auto reg = cmd.cmd[i].p[0];

	//cmd.cmd[i].inst = Asm::INST_MOV;
	cmd.set_cmd_param(cmd.cmd[i], 1, param_imm(TypeInt, v & 0x000000ff));
	cmd.next_cmd_target(i + 1);
	cmd.add_cmd(Asm::INST_ADD, reg, reg, param_imm(TypeInt, v & 0x0000ff00));
	cmd.next_cmd_target(i + 2);
	cmd.add_cmd(Asm::INST_ADD, reg, reg, param_imm(TypeInt, v & 0x00ff0000));
	cmd.next_cmd_target(i + 3);
	cmd.add_cmd(Asm::INST_ADD, reg, reg, param_imm(TypeInt, v & 0xff000000));
	i += 3;
}

void unshift_immediate(SerialNodeParam &p) {
	if (p.kind == NodeKind::CONSTANT) {
		auto *c = (Constant*)(int_p)p.p;
		p.kind = NodeKind::IMMEDIATE;
		p.p = *(int*)((char*)c->p() + p.shift);
		p.shift = 0;
		//msg_write(p.p);
	}
}

// create local variable accesses
void SerializerARM::correct_unallowed_param_combis()
{
	for (int i=cmd.cmd.num-1;i>=0;i--){
		unshift_immediate(cmd.cmd[i].p[0]);
		unshift_immediate(cmd.cmd[i].p[1]);
		unshift_immediate(cmd.cmd[i].p[2]);

		if (cmd.cmd[i].inst == Asm::INST_MOV){
			if ((cmd.cmd[i].p[0].kind != NodeKind::REGISTER) and (cmd.cmd[i].p[1].kind != NodeKind::REGISTER))
				transfer_by_reg_in(cmd.cmd[i], i, 1);
		}else if (is_data_op2(cmd.cmd[i].inst)){
			if (cmd.cmd[i].p[1].kind != NodeKind::REGISTER)
				transfer_by_reg_in(cmd.cmd[i], i, 1);
			if (cmd.cmd[i].p[0].kind != NodeKind::REGISTER)
				transfer_by_reg_in(cmd.cmd[i], i, 0);
		}else if (is_data_op3(cmd.cmd[i].inst)){
			if (cmd.cmd[i].p[1].kind != NodeKind::REGISTER)
				transfer_by_reg_in(cmd.cmd[i], i, 1);
			if (cmd.cmd[i].p[2].kind != NodeKind::REGISTER)
				transfer_by_reg_in(cmd.cmd[i], i, 2);
			if (cmd.cmd[i].p[0].kind != NodeKind::REGISTER)
				transfer_by_reg_out(cmd.cmd[i], i, 0);
		}else if (cmd.cmd[i].inst == Asm::INST_FLDS){
			if (cmd.cmd[i].p[1].kind == NodeKind::IMMEDIATE)
				transfer_by_reg_in(cmd.cmd[i], i, 1);
		}
	}

	cmd_list_out("map:d", "mid unallowed");


	for (int i=cmd.cmd.num-1;i>=0;i--) {
		if (cmd.cmd[i].inst == Asm::INST_MOV) {
			if ((cmd.cmd[i].p[0].kind == NodeKind::REGISTER) and (cmd.cmd[i].p[1].kind == NodeKind::IMMEDIATE))
				if (!const_is_arm_representable(cmd.cmd[i].p[1].p))
					split_mov_reg_immediate(cmd.cmd[i], i);
		}
	}

	for (int i=cmd.cmd.num-1;i>=0;i--) {
		if (cmd.cmd[i].inst == Asm::INST_FLDS) {
			if (cmd.cmd[i].p[1].kind == NodeKind::REGISTER) {
				cmd.cmd[i].inst = Asm::INST_FMSR;
				//std::swap(cmd.cmd[i].p[0], cmd.cmd[i].p[1]);
			}
		} else if (cmd.cmd[i].inst == Asm::INST_FSTS) {
			if (cmd.cmd[i].p[0].kind == NodeKind::REGISTER) {
				cmd.cmd[i].inst = Asm::INST_FMRS;
			}
		}
	}
	scan_temp_var_usage();
}

void SerializerARM::add_function_intro_params(Function *f)
{
	// return, instance, params
	Array<Variable*> param;
	if (f->effective_return_type->uses_return_by_memory()){
		for (Variable *v: weak(f->var))
			if (v->name == IDENTIFIER_RETURN_VAR){
				param.add(v);
				break;
			}
	}
	if (!f->is_static()){
		for (Variable *v: weak(f->var))
			if (v->name == IDENTIFIER_SELF){
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
	for (Variable *p: param){
		if ((p->type == TypeInt) or (p->type == TypeChar) or (p->type == TypeBool) or p->type->is_some_pointer()){
			if (reg_param.num < 4){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if (p->type == TypeFloat32){
			if (float_param.num < 8){
				float_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			do_error("parameter type currently not supported: " + p->type->name);
	}

	// s0-7
	foreachib(Variable *p, float_param, i){
		int reg = Asm::REG_S0+ i;
		cmd.add_cmd(Asm::INST_FSTS, param_local(p->type, p->_offset), param_preg(p->type, reg));
	}

	// rdi, rsi,rdx, rcx, r8, r9
	int param_regs[4] = {Asm::REG_R0, Asm::REG_R1, Asm::REG_R2, Asm::REG_R3};
	foreachib(Variable *p, reg_param, i){
		int reg = cmd.add_virtual_reg(param_regs[i]);
		cmd.add_cmd(Asm::INST_MOV, param_local(p->type, p->_offset), param_vreg(p->type, reg));
		cmd.set_virtual_reg(reg, cmd.cmd.num - 1, cmd.cmd.num - 1);
	}

	// get parameters from stack
	foreachb(Variable *p, stack_param){
		do_error("func with stack...");
		/*int s = 8;
		cmd.add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}

void SerializerARM::add_function_intro_frame(int stack_alloc_size) {
	cmd.next_cmd_target(0);
	cmd.add_cmd(Asm::INST_STMDB, param_preg(TypePointer, Asm::REG_R13), param_imm(TypeInt, 0x6ff0)); // {r4,r5,r6,r7,r8,r9,r10,r11,r13,r14}
	if (stack_max_size > 0){
		cmd.next_cmd_target(1);
		cmd.add_cmd(Asm::INST_MOV, param_preg(TypePointer, Asm::REG_R11), param_preg(TypePointer, Asm::REG_R13));
		cmd.next_cmd_target(2);
		cmd.add_cmd(Asm::INST_SUB, param_preg(TypePointer, Asm::REG_R13), param_preg(TypePointer, Asm::REG_R13), param_imm(TypeInt, stack_max_size));
	}
}

void SerializerARM::add_function_outro(Function *f)
{
	// will be translated into a "real" outro later...
	cmd.add_cmd(Asm::INST_RET);
}


void SerializerARM::do_mapping() {
	cmd_list_out("map:a", "aaa");

	map_referenced_temp_vars_to_stack();

	cmd_list_out("map:b", "post ref map");

	process_dereferences();

	cmd_list_out("map:c", "post deref");
	process_references();

	cmd_list_out("map:d", "post ref");

	// --- remove unnecessary temp vars

	try_map_temp_vars_to_registers();

	cmd_list_out("map:e", "post var reg");

	map_remaining_temp_vars_to_stack();

	//ResolveDerefTempAndLocal();

	cmd_list_out("map:f", "pre global");

	convert_global_lookups();

	cmd_list_out("map:g", "post global");

	correct_unallowed_param_combis();

	cmd_list_out("map:h", "post unallowed");

	for (int i=0; i<cmd.cmd.num; i++)
		convert_mem_movs_to_ldr_str(cmd.cmd[i]);

	convert_global_refs();

	cmd_list_out("map:z", "end");
}

void SerializerARM::convert_global_refs()
{
	for (int i=0; i<cmd.cmd.num; i++){
		if ((cmd.cmd[i].inst == Asm::INST_LDR) and (cmd.cmd[i].p[0].kind == NodeKind::REGISTER) and (cmd.cmd[i].p[1].kind == NodeKind::DEREF_MARKER)){
			bool found = false;
			int64 data;
			for (GlobalRef &r: global_refs){
				if (r.label == cmd.cmd[i].p[1].p){
					data = (int_p)r.p;
					found = true;
					break;
				}
			}
			if (!found)
				continue;
			cmd.cmd[i].inst = Asm::INST_MOV;
			cmd.set_cmd_param(cmd.cmd[i], 1, param_imm(TypeInt, data & 0x000000ff));
			cmd.next_cmd_target(i + 1);
			cmd.add_cmd(Asm::INST_ADD, cmd.cmd[i].p[0], cmd.cmd[i].p[0], param_imm(TypeInt, data & 0x0000ff00));
			cmd.next_cmd_target(i + 2);
			cmd.add_cmd(Asm::INST_ADD, cmd.cmd[i].p[0], cmd.cmd[i].p[0], param_imm(TypeInt, data & 0x00ff0000));
			cmd.next_cmd_target(i + 3);
			cmd.add_cmd(Asm::INST_ADD, cmd.cmd[i].p[0], cmd.cmd[i].p[0], param_imm(TypeInt, data & 0xff000000));
			i += 3;
		}
	}
}

void SerializerARM::convert_mem_movs_to_ldr_str(SerialNode &c)
{
	if (c.inst == Asm::INST_MOV){
		if ((c.p[0].kind == NodeKind::LOCAL_MEMORY) or (c.p[0].kind == NodeKind::DEREF_REGISTER)){
			if (c.p[0].type->size == 1)
				c.inst = Asm::INST_STRB;
			else
				c.inst = Asm::INST_STR;
			SerialNodeParam p = c.p[0];
			cmd.set_cmd_param(c, 0, c.p[1]);
			cmd.set_cmd_param(c, 1, p);
		}else if ((c.p[1].kind == NodeKind::LOCAL_MEMORY) or (c.p[1].kind == NodeKind::DEREF_REGISTER)){
			if (c.p[1].type->size == 1)
				c.inst = Asm::INST_LDRB;
			else
				c.inst = Asm::INST_LDR;
		}else if (c.p[1].kind == NodeKind::DEREF_MARKER){
			c.inst = Asm::INST_LDR;
		}
	}
}

void SerializerARM::correct_return()
{
	for (int i=0;i<cmd.cmd.num;i++)
		if (cmd.cmd[i].inst == Asm::INST_RET){
			cmd.remove_cmd(i);
			if (stack_max_size > 0){
				cmd.next_cmd_target(i ++);
				cmd.add_cmd(Asm::INST_ADD, param_preg(TypePointer, Asm::REG_R13), param_preg(TypePointer, Asm::REG_R13), param_imm(TypeInt, stack_max_size));
			}
			cmd.next_cmd_target(i);
			cmd.add_cmd(Asm::INST_LDMIA, param_preg(TypePointer, Asm::REG_R13), param_imm(TypeInt, 0xaff0)); // {r4,r5,r6,r7,r8,r9,r10,r11,r13,r15}
		}
}

};
