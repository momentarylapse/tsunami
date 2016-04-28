#include "../script.h"
#include "serializer_arm.h"
#include "../../file/file.h"


namespace Asm{
	extern int ARM_DATA_INSTRUCTIONS[16]; // -> asm.cpp
};

namespace Script{



int SerializerARM::fc_begin()
{
	Type *type = CompilerFunctionReturn.type;

	// grow stack (down) for local variables of the calling function
//	add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	long push_size = 0;


	// instance as first parameter
	if (CompilerFunctionInstance.type)
		CompilerFunctionParam.insert(CompilerFunctionInstance, 0);

	// return as _very_ first parameter
	if (type->UsesReturnByMemory()){
		SerialCommandParam ret_ref = AddReference(CompilerFunctionReturn);
		CompilerFunctionParam.insert(ret_ref, 0);
	}

	// map params...
	Array<SerialCommandParam> reg_param;
	Array<SerialCommandParam> stack_param;
	Array<SerialCommandParam> xmm_param;
	foreach(SerialCommandParam &p, CompilerFunctionParam){
		if ((p.type == TypeInt) || (p.type == TypeInt64) || (p.type == TypeChar) || (p.type == TypeBool) || (p.type->is_pointer)){
			if (reg_param.num < 4){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if ((p.type == TypeFloat32) || (p.type == TypeFloat64)){
			if (xmm_param.num < 8){
				xmm_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			DoError("parameter type currently not supported: " + p.type->name);
	}

	// push parameters onto stack
/*	push_size = 4 * stack_param.num;
	if (push_size > 127)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::REG_RSP), param_const(TypeInt, (void*)push_size));
	else if (push_size > 0)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::REG_RSP), param_const(TypeChar, (void*)push_size));
	foreachb(SerialCommandParam &p, stack_param)
		add_cmd(Asm::inst_push, p);
	max_push_size = max(max_push_size, push_size);

	// xmm0-7
	foreachib(SerialCommandParam &p, xmm_param, i){
		int reg = Asm::REG_XMM0 + i;
		if (p.type == TypeFloat64)
			add_cmd(Asm::inst_movsd, param_reg(TypeReg128, reg), p);
		else
			add_cmd(Asm::inst_movss, param_reg(TypeReg128, reg), p);
	}*/

	// r0, r1, r2, r3
	foreachib(SerialCommandParam &p, reg_param, i){
		int v = add_virtual_reg(Asm::REG_R0 + i);
		add_cmd(Asm::INST_MOV, param_vreg(p.type, v), p);
		set_virtual_reg(v, cmd.num - 1, -100); // -> call
	}

	// extend reg channels to call
	foreach(VirtualRegister &r, virtual_reg)
		if (r.last == -100)
			r.last = cmd.num;

	return push_size;
}

void SerializerARM::fc_end(int push_size)
{
	Type *type = CompilerFunctionReturn.type;

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) && (!type->UsesReturnByMemory())){
		if (type == TypeFloat32)
			add_cmd(Asm::INST_MOVSS, CompilerFunctionReturn, param_preg(TypeReg128, Asm::REG_XMM0));
		else if (type == TypeFloat64)
			add_cmd(Asm::INST_MOVSD, CompilerFunctionReturn, param_preg(TypeReg128, Asm::REG_XMM0));
		else if ((type->size == 1) or (type->size == 4)){
			int v = add_virtual_reg(Asm::REG_R0);
			add_cmd(Asm::INST_MOV, CompilerFunctionReturn, param_vreg(TypeReg32, v));
			set_virtual_reg(v, cmd.num - 2, cmd.num - 1);
		}else{
			int v = add_virtual_reg(Asm::REG_R0);
			add_cmd(Asm::INST_MOV, CompilerFunctionReturn, param_vreg(TypeReg32, v));
			set_virtual_reg(v, cmd.num - 2, cmd.num - 1);
		}
	}
}

void SerializerARM::add_function_call(Script *script, int func_no)
{
	int push_size = fc_begin();

	if ((script == this->script) and (!script->syntax->functions[func_no]->is_extern)){
		add_cmd(Asm::INST_CALL, param_marker(list->get_label("_kaba_func_" + i2s(func_no))));
	}else{
		void *func = (void*)script->func[func_no];

		if (!func)
			DoErrorLink("could not link function " + script->syntax->functions[func_no]->name);
		if (abs((long)func - (long)this->script->opcode) < 30000000){
			add_cmd(Asm::INST_CALL, param_const(TypePointer, (long)func)); // the actual call
			// function pointer will be shifted later...
		}else{

			// TODO FIXME
			// really find a usable register...

			int v = add_virtual_reg(Asm::REG_R4);//find_unused_reg(cmd.num-1, cmd.num-1, 4);
			add_cmd(Asm::INST_MOV, param_vreg(TypePointer, v), param_lookup(TypePointer, add_global_ref(func)));
			add_cmd(Asm::INST_CALL, param_vreg(TypePointer, v));
			set_virtual_reg(v, cmd.num-2, cmd.num-1);
		}
	}

	fc_end(push_size);
}

void SerializerARM::add_virtual_function_call(int virtual_index){}

void SerializerARM::SerializeCompilerFunction(Command *com, Array<SerialCommandParam> &param, SerialCommandParam &ret, Block *block, int index, int marker_before_params)
{

	switch(com->link_no){
		case COMMAND_IF:{
			// cmp;  jz m;  -block-  m;
			add_cmd(Asm::INST_CMP, param[0], param_const(TypeBool, 0x0));
			int m_after_true = add_marker_after_command(block->level, index + 1);
			add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker(m_after_true), p_none, p_none);
			}break;
		case COMMAND_IF_ELSE:{
			// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
			add_cmd(Asm::INST_CMP, param[0], param_const(TypeBool, 0x0));
			int m_after_true = add_marker_after_command(block->level, index + 1);
			int m_after_false = add_marker_after_command(block->level, index + 2);
			add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker(m_after_true), p_none, p_none); // jz ...
			add_jump_after_command(block->level, index + 1, m_after_false); // insert before <m_after_true> is inserted!
			}break;
		case COMMAND_WHILE:
		case COMMAND_FOR:{
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			add_cmd(Asm::INST_CMP, param[0], param_const(TypeBool, 0x0));
			int marker_after_while = add_marker_after_command(block->level, index + 1);
			add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_B, param_marker(marker_after_while), p_none, p_none);
			add_jump_after_command(block->level, index + 1, marker_before_params); // insert before <marker_after_while> is inserted!

			int marker_continue = marker_before_params;
			if (com->link_no == COMMAND_FOR){
				// NextCommand is a block!
				if (next_command->kind != KIND_BLOCK)
					DoError("command block in \"for\" loop missing");
				marker_continue = add_marker_after_command(block->level + 1, next_command->as_block()->commands.num - 2);
			}
			LoopData l = {marker_continue, marker_after_while, block->level, index};
			loop.add(l);
			}break;
		case COMMAND_BREAK:
			add_cmd(Asm::INST_B, param_marker(loop.back().marker_break));
			break;
		case COMMAND_CONTINUE:
			add_cmd(Asm::INST_B, param_marker(loop.back().marker_continue));
			break;
		case COMMAND_RETURN:
			if (com->param.num > 0){
				if (cur_func->return_type->UsesReturnByMemory()){ // we already got a return address in [ebp+0x08] (> 4 byte)
					FillInDestructorsBlock(block, true);
					// internally handled...

					AddFunctionOutro(cur_func);
				}else{ // store return directly in eax / fpu stack (4 byte)
					SerialCommandParam t = add_temp(cur_func->return_type);
					FillInDestructorsBlock(block, true);
					if ((cur_func->return_type == TypeInt) or (cur_func->return_type->size == 1)){
						int v = add_virtual_reg(Asm::REG_R0);
						add_cmd(Asm::INST_MOV, param_vreg(cur_func->return_type, v), param[0]);
						set_virtual_reg(v, cmd.num-1, cmd.num);
					}else{
						DoError("return != int");
					}
					AddFunctionOutro(cur_func);
				}
			}else{
				FillInDestructorsBlock(block, true);
				AddFunctionOutro(cur_func);
			}
			break;
		case COMMAND_NEW:
			AddFuncParam(param_const(TypeInt, ret.type->parent->size));
			AddFuncReturn(ret);
			if (!syntax_tree->GetExistence("@malloc", NULL))
				DoError("@malloc not found????");
			AddFunctionCall(syntax_tree->GetExistenceLink.script, syntax_tree->GetExistenceLink.link_no);
			if (com->param.num > 0){
				// copy + edit command
				Command sub = *com->param[0];
				Command c_ret(KIND_VAR_TEMP, (long)ret.p, script, ret.type);
				sub.instance = &c_ret;
				SerializeCommand(&sub, block, index);
			}else
				add_cmd_constructor(ret, -1);
			break;
		case COMMAND_DELETE:
			add_cmd_destructor(param[0], false);
			AddFuncParam(param[0]);
			if (!syntax_tree->GetExistence("@free", NULL))
				DoError("@free not found????");
			AddFunctionCall(syntax_tree->GetExistenceLink.script, syntax_tree->GetExistenceLink.link_no);
			break;
		case COMMAND_ASM:
			add_cmd(INST_ASM);
			break;
		default:
			DoError("compiler function unimplemented: " + PreCommands[com->link_no].name);
	}
}

void SerializerARM::SerializeOperator(Command *com, Array<SerialCommandParam> &param, SerialCommandParam &ret)
{
	msg_db_f("SerializeOperatorARM", 4);
	switch(com->link_no){
		case OperatorIntAssign:
		case OperatorInt64Assign:
		case OperatorFloatAssign:
		case OperatorFloat64Assign:
		case OperatorPointerAssign:
			add_cmd(Asm::INST_MOV, param[0], param[1]);
			break;
		case OperatorCharAssign:
		case OperatorBoolAssign:
			add_cmd(Asm::INST_MOV, param[0], param[1]);
			break;
		case OperatorClassAssign:
			for (int i=0;i<signed(com->param[0]->type->size)/4;i++)
				add_cmd(Asm::INST_MOV, param_shift(param[0], i * 4, TypeInt), param_shift(param[1], i * 4, TypeInt));
			for (int i=4*signed(com->param[0]->type->size/4);i<signed(com->param[0]->type->size);i++)
				add_cmd(Asm::INST_MOV, param_shift(param[0], i, TypeChar), param_shift(param[1], i, TypeChar));
			break;
// int
		case OperatorIntAddS:
			add_cmd(Asm::INST_ADD, param[0], param[0], param[1]);
			break;
		case OperatorIntSubtractS:
			add_cmd(Asm::INST_SUB, param[0], param[0], param[1]);
			break;
		case OperatorIntMultiplyS:
			add_cmd(Asm::INST_IMUL, param[0], param[0], param[1]);
			break;
		case OperatorIntAdd:
			add_cmd(Asm::INST_ADD, ret, param[0], param[1]);
			break;
		case OperatorIntSubtract:
			add_cmd(Asm::INST_SUB, ret, param[0], param[1]);
			break;
		case OperatorIntMultiply:
			add_cmd(Asm::INST_MUL, ret, param[0], param[1]);
			break;
		case OperatorIntEqual:
		case OperatorPointerEqual:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			add_cmd(Asm::ARM_COND_EQUAL,     Asm::INST_MOV, ret, param_const(TypeBool, 1), p_none);
			add_cmd(Asm::ARM_COND_NOT_EQUAL, Asm::INST_MOV, ret, param_const(TypeBool, 0), p_none);
			break;
		case OperatorIntNotEqual:
		case OperatorPointerNotEqual:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			add_cmd(Asm::ARM_COND_NOT_EQUAL,     Asm::INST_MOV, ret, param_const(TypeBool, 1), p_none);
			add_cmd(Asm::ARM_COND_EQUAL, Asm::INST_MOV, ret, param_const(TypeBool, 0), p_none);
			break;
		case OperatorIntGreater:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			add_cmd(Asm::ARM_COND_GREATER_THAN,     Asm::INST_MOV, ret, param_const(TypeBool, 1), p_none);
			add_cmd(Asm::ARM_COND_LESS_EQUAL, Asm::INST_MOV, ret, param_const(TypeBool, 0), p_none);
			break;
		case OperatorIntGreaterEqual:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			add_cmd(Asm::ARM_COND_GREATER_EQUAL,     Asm::INST_MOV, ret, param_const(TypeBool, 1), p_none);
			add_cmd(Asm::ARM_COND_LESS_THAN, Asm::INST_MOV, ret, param_const(TypeBool, 0), p_none);
			break;
		case OperatorIntSmaller:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			add_cmd(Asm::ARM_COND_LESS_THAN,     Asm::INST_MOV, ret, param_const(TypeBool, 1), p_none);
			add_cmd(Asm::ARM_COND_GREATER_EQUAL, Asm::INST_MOV, ret, param_const(TypeBool, 0), p_none);
			break;
		case OperatorIntSmallerEqual:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			add_cmd(Asm::ARM_COND_LESS_EQUAL,     Asm::INST_MOV, ret, param_const(TypeBool, 1), p_none);
			add_cmd(Asm::ARM_COND_GREATER_THAN, Asm::INST_MOV, ret, param_const(TypeBool, 0), p_none);
			break;
		case OperatorIntBitAnd:
			add_cmd(Asm::INST_AND, ret, param[0], param[1]);
			break;
		case OperatorIntBitOr:
			add_cmd(Asm::INST_OR, ret, param[0], param[1]);
			break;
		case OperatorIntNegate:
			add_cmd(Asm::INST_MOV, ret, param_const(TypeInt, 0x0));
			add_cmd(Asm::INST_SUB, ret, ret, param[0]);
			break;
		case OperatorIntIncrease:
			add_cmd(Asm::INST_ADD, param[0], param[0], param_const(TypeInt, 0x1));
			break;
		case OperatorIntDecrease:
			add_cmd(Asm::INST_SUB, param[0], param[0], param_const(TypeInt, 0x1));
			break;
	// bool/char
		case OperatorCharEqual:
		case OperatorCharNotEqual:
		case OperatorBoolEqual:
		case OperatorBoolNotEqual:
		case OperatorBoolGreater:
		case OperatorBoolGreaterEqual:
		case OperatorBoolSmaller:
		case OperatorBoolSmallerEqual:
			add_cmd(Asm::INST_CMP, param[0], param[1]);
			if ((com->link_no == OperatorCharEqual) || (com->link_no == OperatorBoolEqual))
				add_cmd(Asm::INST_SETZ, ret);
			else if ((com->link_no ==OperatorCharNotEqual) || (com->link_no == OperatorBoolNotEqual))
				add_cmd(Asm::INST_SETNZ, ret);
			else if (com->link_no == OperatorBoolGreater)		add_cmd(Asm::INST_SETNLE, ret);
			else if (com->link_no == OperatorBoolGreaterEqual)	add_cmd(Asm::INST_SETNL, ret);
			else if (com->link_no == OperatorBoolSmaller)		add_cmd(Asm::INST_SETL, ret);
			else if (com->link_no == OperatorBoolSmallerEqual)	add_cmd(Asm::INST_SETLE, ret);
			break;
		case OperatorBoolAnd:
			add_cmd(Asm::INST_AND, ret, param[0], param[1]);
			break;
		case OperatorBoolOr:
			add_cmd(Asm::INST_OR, ret, param[0], param[1]);
			break;
		case OperatorCharAddS:
			add_cmd(Asm::INST_ADD, param[0], param[0], param[1]);
			break;
		case OperatorCharSubtractS:
			add_cmd(Asm::INST_SUB, param[0], param[0], param[1]);
			break;
		case OperatorCharAdd:
			add_cmd(Asm::INST_ADD, ret, param[0], param[1]);
			break;
		case OperatorCharSubtract:
			add_cmd(Asm::INST_SUB, ret, param[0], param[1]);
			break;
		case OperatorCharBitAnd:
			add_cmd(Asm::INST_AND, ret, param[0], param[1]);
			break;
		case OperatorCharBitOr:
			add_cmd(Asm::INST_OR, ret, param[0], param[1]);
			break;
		case OperatorBoolNegate:
			add_cmd(Asm::INST_XOR, ret, param[0], param_const(TypeBool, 0x1));
			break;
		case OperatorCharNegate:
			add_cmd(Asm::INST_MOV, ret, param_const(TypeChar, 0x0));
			add_cmd(Asm::INST_SUB, ret, ret, param[0]);
			break;
		default:
			DoError("unimplemented operator: " + PreOperators[com->link_no].str());
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

// create data for a (function) parameter
//   and compile its command if the parameter is executable itself
SerialCommandParam SerializerARM::SerializeParameter(Command *link, Block *block, int index)
{
	msg_db_f("SerializeParameter", 4);
	SerialCommandParam p;
	p.kind = link->kind;
	p.type = link->type;
	p.p = 0;
	p.shift = 0;
	//Type *rt=link->;
	if (link->kind == KIND_VAR_FUNCTION){
		p.p = (long)link->script->func[link->link_no];
		p.kind = KIND_VAR_GLOBAL;
		if (!p.p){
			if (link->script == script){
				p.p = link->link_no + 0xefef0000;
				script->function_vars_to_link.add(link->link_no);
			}else
				DoErrorLink("could not link function as variable: " + link->script->syntax->functions[link->link_no]->name);
			//p.kind = Asm::PKLabel;
			//p.p = (char*)(long)list->add_label("_kaba_func_" + link->script->syntax->Functions[link->link_no]->name, false);
		}
		return param_deref_lookup(p.type, add_global_ref((void*)p.p));
	}else if (link->kind == KIND_MEMORY){
		return param_deref_lookup(p.type, add_global_ref((void*)p.p));
	}else if (link->kind == KIND_ADDRESS){
		return param_lookup(p.type, add_global_ref((void*)(long)link->link_no));
	}else if (link->kind == KIND_VAR_GLOBAL){
		if (!link->script->g_var[link->link_no])
			script->DoErrorLink("variable is not linkable: " + link->script->syntax->root_of_all_evil.var[link->link_no].name);
		return param_deref_lookup(p.type, add_global_ref(link->script->g_var[link->link_no]));
	}else if (link->kind == KIND_VAR_LOCAL){
		p.p = cur_func->var[link->link_no]._offset;
	}else if (link->kind == KIND_LOCAL_MEMORY){
		p.p = link->link_no;
		p.kind = KIND_VAR_LOCAL;
	}else if (link->kind == KIND_LOCAL_ADDRESS){
		SerialCommandParam param = param_local(TypePointer, link->link_no);
		return AddReference(param, link->type);
	}else if (link->kind == KIND_CONSTANT){
		void *pp = link->script->cnst[link->link_no];
		int c = *(int*)pp;
		if (const_is_arm_representable(c)){
			p.p = c;
			p.kind = KIND_IMMEDIATE;
		}else{
			return param_lookup(p.type, add_global_ref(*(int**)pp));
		}
	}else if ((link->kind==KIND_OPERATOR) || (link->kind==KIND_FUNCTION) || (link->kind==KIND_VIRTUAL_FUNCTION) || (link->kind==KIND_COMPILER_FUNCTION) || (link->kind==KIND_ARRAY_BUILDER)){
		return SerializeCommand(link, block, index);
	}else if (link->kind == KIND_REFERENCE){
		SerialCommandParam param = SerializeParameter(link->param[0], block, index);
		//printf("%d  -  %s\n",pk,Kind2Str(pk));
		return AddReference(param, link->type);
	}else if (link->kind == KIND_DEREFERENCE){
		SerialCommandParam param = SerializeParameter(link->param[0], block, index);
		/*if ((param.kind == KindVarLocal) || (param.kind == KindVarGlobal)){
			p.type = param.type->sub_type;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		return AddDereference(param);
	}else if (link->kind == KIND_VAR_TEMP){
		// only used by <new> operator
		p.p = link->link_no;
	}else{
		DoError("unexpected type of parameter: " + Kind2Str(link->kind));
	}
	return p;
}


void SerializerARM::ProcessReferences()
{
	msg_db_f("ProcessReferences", 3);
	for (int i=0;i<cmd.num;i++)
		if (cmd[i].inst == Asm::INST_LEA){
			if (cmd[i].p[1].kind == KIND_VAR_LOCAL){
				SerialCommandParam p0 = cmd[i].p[0];
				SerialCommandParam p1 = cmd[i].p[1];
				int r = find_unused_reg(i, i, 4);
				remove_cmd(i);
				next_cmd_target(i);
				add_cmd(Asm::INST_ADD, param_vreg(TypePointer, r), param_preg(TypePointer, Asm::REG_R13), param_const(TypeInt, p1.p));
				next_cmd_target(i + 1);
				add_cmd(Asm::INST_MOV, p0, param_vreg(TypePointer, r));
				set_virtual_reg(r, i, i+1);
			}else{
				DoError("reference in ARM: " + cmd[i].p[1].str());
			}
		}
}

void SerializerARM::ProcessDereferences()
{
	msg_db_f("ProcessDereeferences", 3);
	for (int i=0;i<cmd.num;i++)
		for (int j=0;j<SERIAL_COMMAND_NUM_PARAMS;j++)
			if ((cmd[i].p[j].kind == KIND_DEREF_VAR_LOCAL) or (cmd[i].p[j].kind == KIND_DEREF_VAR_TEMP)){
				SerialCommandParam p = cmd[i].p[j];
				SerialCommandParam rp = cmd[i].p[j];
				if (cmd[i].p[j].kind == KIND_DEREF_VAR_LOCAL)
					rp.kind = KIND_VAR_LOCAL;
				else
					rp.kind = KIND_VAR_TEMP;
				rp.type = p.type->GetPointer();
				int r = find_unused_reg(i, i, 4);
				next_cmd_target(i);
				add_cmd(Asm::INST_MOV, param_vreg(TypePointer, r), rp);
				set_cmd_param(cmd[i+1], j, param_deref_vreg(p.type, r));
				set_virtual_reg(r, i, i+1);
			}
}


inline bool _____arm_param_combi_allowed(int inst, SerialCommandParam &p1, SerialCommandParam &p2, SerialCommandParam &p3)
{
//	if (inst >= Asm::inst_marker)
//		return true;
	if (inst == Asm::INST_MOV)
		return (p1.kind == KIND_REGISTER) and (p2.kind == KIND_REGISTER);
	if (inst == Asm::INST_ADD)
		return (p1.kind == KIND_REGISTER) and (p2.kind == KIND_REGISTER) and (p3.kind == KIND_REGISTER);
	return true;
}

void SerializerARM::transfer_by_reg_in(SerialCommand &c, int &i, int pno)
{
	SerialCommandParam p = c.p[pno];
	int r = find_unused_reg(i, i, /*p.type->size*/ 4);
	SerialCommandParam pr = param_vreg(p.type, r);
	set_virtual_reg(r, i, cmd.num);
	set_cmd_param(cmd[i], pno, pr);
	next_cmd_target(i);
	add_cmd(c.cond, Asm::INST_MOV, pr, p, p_none);
	i ++;
}

void SerializerARM::transfer_by_reg_out(SerialCommand &c, int &i, int pno)
{
	SerialCommandParam p = c.p[pno];
	int r = find_unused_reg(i, i, 4);//p.type->size);
	SerialCommandParam pr = param_vreg(p.type, r);
	set_virtual_reg(r, i, cmd.num);
	set_cmd_param(c, pno, pr);
	next_cmd_target(i+1);
	add_cmd(c.cond, Asm::INST_MOV, p, pr, p_none);
}

void SerializerARM::gr_transfer_by_reg_in(SerialCommand &c, int &i, int pno)
{
	SerialCommandParam p = c.p[pno];
	if (config.verbose)
		msg_write("in " + c.str());
	if (p.kind == KIND_DEREF_GLOBAL_LOOKUP){
		// cmd ..., [global ref]

		// mov r2, [ref]
		// ldr r1, [r2]
		// cmd ..., r1


		int r2 = find_unused_reg(i, i, 4);
		next_cmd_target(i);
		add_cmd(c.cond, Asm::INST_MOV, param_vreg(TypePointer, r2), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);

		int r1 = find_unused_reg(i+1, i+1, 4/*p.type->size*/, virtual_reg[r2].reg_root);
		next_cmd_target(i+1);
		add_cmd(c.cond, Asm::INST_LDR, param_vreg(p.type, r1), param_deref_vreg(TypePointer, r2), p_none);

		set_cmd_param(cmd[i+2], pno, param_vreg(p.type, r1));

		set_virtual_reg(r2, i, i + 1);
		set_virtual_reg(r1, i + 1, i + 2);

		i += 2;
	}else{
		// cmd ..., global

		// mov r1, [ref]
		// cmd ..., r1


		int r1 = find_unused_reg(i, i, 4);
		next_cmd_target(i);
		add_cmd(c.cond, Asm::INST_MOV, param_vreg(TypePointer, r1), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);
		set_virtual_reg(r1, i, i + 1);

		set_cmd_param(cmd[i+1], pno, param_vreg(p.type, r1));

		i += 1;
	}
}

void SerializerARM::gr_transfer_by_reg_out(SerialCommand &c, int &i, int pno)
{
	SerialCommandParam p = c.p[pno];
	if (config.verbose)
		msg_write("out " + c.str());
	if (p.kind == KIND_DEREF_GLOBAL_LOOKUP){
		// cmd [global ref], ...

		// cmd r1, ...
		// mov r2, [ref]
		// str r1, [r2]


		int r2 = find_unused_reg(i, i, 4);
		next_cmd_target(i+1);
		add_cmd(c.cond, Asm::INST_MOV, param_vreg(TypePointer, r2), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);

		int r1 = find_unused_reg(i, i+1, 4 /*p.type->size*/, virtual_reg[r2].reg_root);
		next_cmd_target(i+2);
		add_cmd(c.cond, Asm::INST_STR, param_vreg(p.type, r1), param_deref_vreg(TypePointer, r2), p_none);

		set_cmd_param(cmd[i], pno, param_vreg(p.type, r1));

		set_virtual_reg(r1, i, i + 2);
		set_virtual_reg(r2, i+1, i+2);
	}else{
		// cmd global, ...

		// cmd r1, ...
		// str r1, [ref]


		int r1 = find_unused_reg(i, i, 4);
		next_cmd_target(i+2);
		add_cmd(c.cond, Asm::INST_STR, param_vreg(TypePointer, r1), param_deref_marker(TypePointer, global_refs[p.p].label), p_none);
		set_virtual_reg(r1, i, i+1);

		set_cmd_param(cmd[i], pno, param_vreg(p.type, r1));
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

inline bool is_global_lookup(SerialCommandParam &p)
{
	return (p.kind == KIND_DEREF_GLOBAL_LOOKUP) or (p.kind == KIND_GLOBAL_LOOKUP);
}

// create global lookup accesses
void SerializerARM::ConvertGlobalLookups()
{
	msg_db_f("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst == Asm::INST_MOV){
			if (is_global_lookup(cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd[i], i, 1);
			if (is_global_lookup(cmd[i].p[0]))
				gr_transfer_by_reg_out(cmd[i], i, 0);
		}else if (is_data_op2(cmd[i].inst)){
			if (is_global_lookup(cmd[i].p[0]))
				gr_transfer_by_reg_in(cmd[i], i, 0);
			if (is_global_lookup(cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd[i], i, 1);
		}else if (is_data_op3(cmd[i].inst)){
			if (is_global_lookup(cmd[i].p[1]))
				gr_transfer_by_reg_in(cmd[i], i, 1);
			if (is_global_lookup(cmd[i].p[2]))
				gr_transfer_by_reg_in(cmd[i], i, 2);
			if (is_global_lookup(cmd[i].p[0]))
				gr_transfer_by_reg_out(cmd[i], i, 0);
		}
	}
	ScanTempVarUsage();
}

// create local variable accesses
void SerializerARM::CorrectUnallowedParamCombis()
{
	msg_db_f("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst == Asm::INST_MOV){
			if ((cmd[i].p[0].kind != KIND_REGISTER) and (cmd[i].p[1].kind != KIND_REGISTER))
				transfer_by_reg_in(cmd[i], i, 1);
		}else if (is_data_op2(cmd[i].inst)){
			if (cmd[i].p[1].kind != KIND_REGISTER)
				transfer_by_reg_in(cmd[i], i, 1);
			if (cmd[i].p[0].kind != KIND_REGISTER)
				transfer_by_reg_in(cmd[i], i, 0);
		}else if (is_data_op3(cmd[i].inst)){
			if (cmd[i].p[1].kind != KIND_REGISTER)
				transfer_by_reg_in(cmd[i], i, 1);
			if (cmd[i].p[2].kind != KIND_REGISTER)
				transfer_by_reg_in(cmd[i], i, 2);
			if (cmd[i].p[0].kind != KIND_REGISTER)
				transfer_by_reg_out(cmd[i], i, 0);
		}
	}
	ScanTempVarUsage();
}

void SerializerARM::AddFunctionIntro(Function *f)
{

	// return, instance, params
	Array<Variable> param;
	if (f->return_type->UsesReturnByMemory()){
		foreach(Variable &v, f->var)
			if (v.name == "-return-"){
				param.add(v);
				break;
			}
	}
	if (f->_class){
		foreach(Variable &v, f->var)
			if (v.name == "self"){
				param.add(v);
				break;
			}
	}
	for (int i=0;i<f->num_params;i++)
		param.add(f->var[i]);

	// map params...
	Array<Variable> reg_param;
	Array<Variable> stack_param;
	Array<Variable> xmm_param;
	foreach(Variable &p, param){
		if ((p.type == TypeInt) || (p.type == TypeChar) || (p.type == TypeBool) || (p.type->is_pointer)){
			if (reg_param.num < 4){
				reg_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else if (p.type == TypeFloat32){
			DoError("arm float param");
			if (xmm_param.num < 8){
				xmm_param.add(p);
			}else{
				stack_param.add(p);
			}
		}else
			DoError("parameter type currently not supported: " + p.type->name);
	}

	// xmm0-7
	/*foreachib(Variable &p, xmm_param, i){
		int reg = Asm::REG_XMM0 + i;
		add_cmd(Asm::inst_movss, param_local(p.type, p._offset), param_reg(p.type, reg));
	}*/

	// rdi, rsi,rdx, rcx, r8, r9
	int param_regs[4] = {Asm::REG_R0, Asm::REG_R1, Asm::REG_R2, Asm::REG_R3};
	foreachib(Variable &p, reg_param, i){
		int reg = add_virtual_reg(param_regs[i]);
		add_cmd(Asm::INST_MOV, param_local(p.type, p._offset), param_vreg(p.type, reg));
		set_virtual_reg(reg, cmd.num - 1, cmd.num - 1);
	}

	// get parameters from stack
	foreachb(Variable &p, stack_param){
		DoError("func with stack...");
		/*int s = 8;
		add_cmd(Asm::inst_push, p);
		push_size += s;*/
	}
}

void SerializerARM::AddFunctionOutro(Function *f)
{
	// will be translated into a "real" outro later...
	add_cmd(Asm::INST_RET);
}


void SerializerARM::DoMapping()
{
	if (config.verbose)
		cmd_list_out("aaa");

	MapReferencedTempVarsToStack();

	if (config.verbose)
		cmd_list_out("post ref map");

	ProcessDereferences();
	ProcessReferences();

	// --- remove unnecessary temp vars

	TryMapTempVarsRegisters();

	MapRemainingTempVarsToStack();

	//ResolveDerefTempAndLocal();

	if (config.verbose)
		cmd_list_out("pre global");

	ConvertGlobalLookups();

	if (config.verbose)
		cmd_list_out("post global");

	CorrectUnallowedParamCombis();

	if (config.verbose)
		cmd_list_out("post unallowed");

	for (int i=0; i<cmd.num; i++)
		ConvertMemMovsToLdrStr(cmd[i]);

	ConvertGlobalRefs();

	if (config.verbose)
		cmd_list_out("end");
}

void SerializerARM::ConvertGlobalRefs()
{
	for (int i=0; i<cmd.num; i++){
		if ((cmd[i].inst == Asm::INST_LDR) and (cmd[i].p[0].kind == KIND_REGISTER) and (cmd[i].p[1].kind == KIND_DEREF_MARKER)){
			bool found = false;
			long data;
			foreach(GlobalRef &r, global_refs){
				if (r.label == cmd[i].p[1].p){
					data = (long)r.p;
					found = true;
					break;
				}
			}
			if (!found)
				continue;
			cmd[i].inst = Asm::INST_MOV;
			set_cmd_param(cmd[i], 1, param_const(TypeInt, data & 0x000000ff));
			next_cmd_target(i + 1);
			add_cmd(Asm::INST_ADD, cmd[i].p[0], cmd[i].p[0], param_const(TypeInt, data & 0x0000ff00));
			next_cmd_target(i + 2);
			add_cmd(Asm::INST_ADD, cmd[i].p[0], cmd[i].p[0], param_const(TypeInt, data & 0x00ff0000));
			next_cmd_target(i + 3);
			add_cmd(Asm::INST_ADD, cmd[i].p[0], cmd[i].p[0], param_const(TypeInt, data & 0xff000000));
			i += 3;
		}
	}
}

void SerializerARM::ConvertMemMovsToLdrStr(SerialCommand &c)
{
	if (c.inst == Asm::INST_MOV){
		if ((c.p[0].kind == KIND_VAR_LOCAL) or (c.p[0].kind == KIND_DEREF_REGISTER)){
			if (c.p[0].type->size == 1)
				c.inst = Asm::INST_STRB;
			else
				c.inst = Asm::INST_STR;
			SerialCommandParam p = c.p[0];
			set_cmd_param(c, 0, c.p[1]);
			set_cmd_param(c, 1, p);
		}else if ((c.p[1].kind == KIND_VAR_LOCAL) or (c.p[1].kind == KIND_DEREF_REGISTER)){
			if (c.p[1].type->size == 1)
				c.inst = Asm::INST_LDRB;
			else
				c.inst = Asm::INST_LDR;
		}else if (c.p[1].kind == KIND_DEREF_MARKER){
			c.inst = Asm::INST_LDR;
		}
	}
}

void SerializerARM::CorrectReturn()
{
	for (int i=0;i<cmd.num;i++)
		if (cmd[i].inst == Asm::INST_RET){
			remove_cmd(i);
			if (stack_max_size > 0){
				next_cmd_target(i ++);
				add_cmd(Asm::INST_ADD, param_preg(TypePointer, Asm::REG_R13), param_preg(TypePointer, Asm::REG_R13), param_const(TypeInt, stack_max_size));
			}
			next_cmd_target(i);
			add_cmd(Asm::INST_LDMIA, param_preg(TypePointer, Asm::REG_R13), param_const(TypeInt, 0x8ff0));
		}
}

};
