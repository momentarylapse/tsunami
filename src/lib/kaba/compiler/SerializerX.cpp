/*
 * SerializerX.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#include "serializer.h"
#include "../kaba.h"
#include "../../os/msg.h"


// hmmm, do we want to insert "pop local" to read function parameters?
//  but what about the return reference?
//  (we would have calling convention dependency already here)
//  -> only params... insert return ref. in backend?

namespace kaba {

Serializer::~Serializer() {
	/*msg_write("aa");
	list->show();
	msg_write("aa2");
	for (int i=0;i<cmd.num;i++)
		msg_write(format("%3d: ", i) + cmd[i].str(this));
	list->clear();
	msg_write("aa3");*/
}

void Serializer::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	[[maybe_unused]] int push_size = function_call_push_params(f, params, ret);

	SerialNodeParam fp = {NodeKind::FUNCTION, (int_p)f, -1, TypeFunctionP, 0};
	cmd.add_cmd(Asm::InstID::CALL, ret, fp); // the actual call
}

void Serializer::add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	[[maybe_unused]] int push_size = function_call_push_params(f, params, ret);

	auto t1 = add_temp(TypePointer);
	auto t2 = add_temp(TypePointer);
	auto t3 = add_temp(TypeFunctionCodeP);
	cmd.add_cmd(Asm::InstID::MOV, t1, params[0]); // self
	cmd.add_cmd(Asm::InstID::ADD, t2, deref_temp(t1, TypePointer), param_imm(TypeInt, config.pointer_size * f->virtual_index)); // vtable + n
	cmd.add_cmd(Asm::InstID::MOV, t3, deref_temp(t2, TypeFunctionCodeP)); // vtable[n]
	cmd.add_cmd(Asm::InstID::CALL_MEMBER, ret, t3); // the actual call
}

int Serializer::function_call_push_params(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	for (SerialNodeParam &p: params)
		cmd.add_cmd(Asm::InstID::PUSH, p);
	return 0;
}

void Serializer::add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	[[maybe_unused]] int push_size = function_call_push_params(nullptr, params, ret);

	cmd.add_cmd(Asm::InstID::CALL, ret, pointer); // the actual call
}

void Serializer::add_function_outro(Function *f) {
	if (f->literal_return_type == TypeVoid)
		cmd.add_cmd(Asm::InstID::RET);
}

SerialNodeParam Serializer::serialize_parameter(Node *link, Block *block, int index) {
	SerialNodeParam p;
	p.kind = link->kind;
	p.type = link->type;
	p.p = 0;
	p.shift = 0;

	if (link->kind == NodeKind::MEMORY) {
		p.p = link->link_no;
	} else if (link->kind == NodeKind::ADDRESS) {
		//p.p = link->link_no;
		p.p = (int_p)&link->link_no;
		p.kind = NodeKind::CONSTANT_BY_ADDRESS;
	} else if (link->kind == NodeKind::VAR_GLOBAL) {
		p.p = link->link_no;
		/*p.p = (int_p)link->as_global_p();
		if (!p.p)
			script->do_error_link("variable is not linkable: " + link->as_global()->name);
		p.kind = NodeKind::MEMORY;*/
	} else if (link->kind == NodeKind::VAR_LOCAL) {
		p.p = link->link_no;
		//p.p = link->as_local()->_offset;
		//p.kind = NodeKind::LOCAL_MEMORY;
	} else if (link->kind == NodeKind::LOCAL_MEMORY) {
		p.p = link->link_no;
	} else if (link->kind == NodeKind::LOCAL_ADDRESS) {
		SerialNodeParam param = param_local(TypePointer, link->link_no);
		return add_reference(param, link->type);
	} else if (link->kind == NodeKind::CONSTANT) {
		p.p = link->link_no;
		/*p.p = (int_p)link->as_const()->address; // FIXME ....need a cleaner approach for compiling os...
		if (config.compile_os)
			p.kind = NodeKind::MEMORY;
		else
			p.kind = NodeKind::CONSTANT_BY_ADDRESS;
		if (syntax_tree->flag_function_pointer_as_code and (link->type == TypeFunctionP)) {
			auto *fp = (Function*)(int_p)link->as_const()->as_int64();
			p.kind = NodeKind::LABEL;
			p.p = fp->_label;
		}*/
	} else if ((link->kind == NodeKind::OPERATOR) or (link->kind == NodeKind::CALL_FUNCTION) or (link->kind == NodeKind::CALL_INLINE) or (link->kind == NodeKind::CALL_VIRTUAL) or (link->kind == NodeKind::CALL_RAW_POINTER) or (link->kind == NodeKind::STATEMENT)) {
		p = serialize_node(link, block, index);
	} else if (link->kind == NodeKind::REFERENCE) {
		auto param = serialize_parameter(link->params[0].get(), block, index);
		//printf("%d  -  %s\n",pk,Kind2Str(pk));
		return add_reference(param, link->type);
	} else if (link->kind == NodeKind::DEREFERENCE) {
		auto param = serialize_parameter(link->params[0].get(), block, index);
		/*if ((param.kind == KindVarLocal) or (param.kind == KindVarGlobal)){
			p.type = param.type->sub_type;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		return add_dereference(param, link->type);
	} else if (link->kind == NodeKind::VAR_TEMP) {
		// only used by <new> operator
		p.p = link->link_no;
	} else {
		do_error("unexpected type of parameter: " + kind2str(link->kind));
	}
	return p;
}

void Serializer::serialize_statement(Node *com, const SerialNodeParam &ret, Block *block, int index) {
	auto statement = com->as_statement();
	switch(statement->id){
		case StatementID::IF:{
			int label_after_true = list->create_label("_IF_AFTER_" + i2s(num_labels ++));
			auto cond = serialize_parameter(com->params[0].get(), block, index);
			// cmp;  jz m;  -block-  m;
			cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_true));
			serialize_block(com->params[1]->as_block());
			cmd.add_label(label_after_true);
			}break;
		case StatementID::IF_ELSE:{
			int label_after_true = list->create_label("_IF_AFTER_TRUE_" + i2s(num_labels ++));
			int label_after_false = list->create_label("_IF_AFTER_FALSE_" + i2s(num_labels ++));
			auto cond = serialize_parameter(com->params[0].get(), block, index);
			// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
			cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_true)); // jz ...
			serialize_block(com->params[1]->as_block());
			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_after_false));
			cmd.add_label(label_after_true);
			serialize_block(com->params[2]->as_block());
			cmd.add_label(label_after_false);
			}break;
		case StatementID::WHILE:{
			int label_before_while = list->create_label("_WHILE_BEFORE_" + i2s(num_labels ++));
			int label_after_while = list->create_label("_WHILE_AFTER_" + i2s(num_labels ++));
			cmd.add_label(label_before_while);
			auto cond = serialize_parameter(com->params[0].get(), block, index);
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_while));

			// body of loop
			LoopData l = {label_before_while, label_after_while, block->level, index};
			loop.add(l);
			serialize_block(com->params[1]->as_block());
			loop.pop();

			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_before_while));
			cmd.add_label(label_after_while);
			}break;
		case StatementID::FOR_DIGEST:{
			int label_before_for = list->create_label("_FOR_BEFORE_" + i2s(num_labels ++));
			int label_after_for = list->create_label("_FOR_AFTER_" + i2s(num_labels ++));
			int label_continue = list->create_label("_FOR_CONTINUE_" + i2s(num_labels ++));
			serialize_node(com->params[0].get(), block, index); // i=0
			cmd.add_label(label_before_for);
			auto cond = serialize_parameter(com->params[1].get(), block, index);
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_for));

			// body of loop
			LoopData l = {label_continue, label_after_for, block->level, index};
			loop.add(l);
			serialize_block(com->params[2]->as_block());
			loop.pop();

			// "i++"
			cmd.add_label(label_continue);
			serialize_node(com->params[3].get(), block, index);

			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_before_for));
			cmd.add_label(label_after_for);
			}break;
		case StatementID::BREAK:
			cmd.add_cmd(Asm::InstID::JMP, param_label32(loop.back().label_break));
			break;
		case StatementID::CONTINUE:
			cmd.add_cmd(Asm::InstID::JMP, param_label32(loop.back().label_continue));
			break;
		case StatementID::RETURN:
			if (com->params.num > 0) {
				auto p = serialize_parameter(com->params[0].get(), block, index);
				if (p.kind == NodeKind::DEREF_VAR_TEMP and !cur_func->literal_return_type->uses_return_by_memory()) {
					auto t = add_temp(p.type);
					cmd.add_cmd(Asm::InstID::MOV, t, p);
					p = t;
				}
				insert_destructors_block(block, true);
				cmd.add_cmd(Asm::InstID::RET, p);
			} else {
				insert_destructors_block(block, true);
				cmd.add_cmd(Asm::InstID::RET);
			}

			break;
		case StatementID::NEW:{
			// malloc()
			auto f = syntax_tree->required_func_global("@malloc");
			auto type = com->params[0]->as_func()->name_space; // ret.type->param[0]
			add_function_call(f, {param_imm(TypeInt, type->size)}, ret);

			// __init__()
			auto sub = com->params[0]->shallow_copy();
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
		/*case StatementID::RAISE:
			AddFunctionCall();
			break;*/
		case StatementID::TRY:{
			int label_finish = list->create_label("_TRY_AFTER_" + i2s(num_labels ++));

			// try
			serialize_block(com->params[0]->as_block());
			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_finish));

			// except
			for (int i=2; i<com->params.num; i+=2) {
				serialize_block(com->params[i]->as_block());
				if (i < com->params.num-1)
					cmd.add_cmd(Asm::InstID::JMP, param_label32(label_finish));
			}

			cmd.add_label(label_finish);
			}break;
		case StatementID::ASM:
			//AddAsmBlock(list, script);
			cmd.add_cmd(Asm::InstID::ASM);
			break;
		case StatementID::PASS:
			break;
		case StatementID::RAW_FUNCTION_POINTER: {
			// only from callable can reach here!
			if (config.compile_os)
				do_error("implicit raw_function_pointer() for os not implemented yet (i.e. don't use callables/function pointers)");
			auto func = serialize_parameter(com->params[0].get(), block, index);
			auto t1 = add_temp(TypePointer);
			cmd.add_cmd(Asm::InstID::ADD, t1, func, param_imm(TypeInt, config.function_address_offset)); // Function* pointer
			cmd.add_cmd(Asm::InstID::MOV, ret, deref_temp(t1, TypeFunctionCodeP)); // the actual call
			}break;
		default:
			do_error("statement unimplemented: " + com->as_statement()->name);
	}
}

void Serializer::serialize_inline_function(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) {
	auto index = com->as_func()->inline_no;
	switch (index) {
		case InlineID::INT_TO_FLOAT:
			cmd.add_cmd(Asm::InstID::CVTSI2SS, ret, param[0]);
			break;
		case InlineID::FLOAT_TO_INT:
			cmd.add_cmd(Asm::InstID::CVTTSS2SI, ret, param[0]);
			break;
		case InlineID::FLOAT_TO_FLOAT64:
			cmd.add_cmd(Asm::InstID::CVTSS2SD, ret, param[0]);
			break;
		case InlineID::FLOAT64_TO_FLOAT:
			cmd.add_cmd(Asm::InstID::CVTSD2SS, ret, param[0]);
			break;
/*
		case InlineID::FLOAT_TO_FLOAT64:
			cmd.add_cmd(Asm::InstID::CVTSS2SD, p_xmm0, param[0]);
			cmd.add_cmd(Asm::InstID::MOVSD, ret, p_xmm0);
			break;
		case InlineID::FLOAT64_TO_FLOAT:
			cmd.add_cmd(Asm::InstID::CVTSD2SS, p_xmm0, param[0]);
			cmd.add_cmd(Asm::InstID::MOVSS, ret, p_xmm0);
			break;*/
		case InlineID::POINTER_TO_BOOL:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param_imm(TypePointer, 0));
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::INT_TO_CHAR:
		case InlineID::CHAR_TO_INT:
			cmd.add_cmd(Asm::InstID::MOVZX, ret, param[0]);
			break;
		case InlineID::RECT_SET:
		case InlineID::VECTOR_SET:
		case InlineID::COMPLEX_SET:
		case InlineID::COLOR_SET:
			for (int i=0; i<ret.type->size/4; i++)
				cmd.add_cmd(Asm::InstID::MOV, param_shift(ret, i*4, TypeFloat32), param[i]);
			break;
		case InlineID::INT_ASSIGN:
		case InlineID::INT64_ASSIGN:
		case InlineID::FLOAT_ASSIGN:
		case InlineID::FLOAT64_ASSIGN:
		case InlineID::POINTER_ASSIGN:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param[1]);
			break;
		case InlineID::SHARED_POINTER_INIT:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param_imm(TypeInt, 0));
			break;
		case InlineID::CHAR_ASSIGN:
		case InlineID::BOOL_ASSIGN:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param[1]);
			break;
// chunk...
		case InlineID::CHUNK_ASSIGN:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param[1]);
			break;
		case InlineID::CHUNK_EQUAL:
			if (param[0].type->size > config.pointer_size) {
				// chunk cmp
				int label_after_cmp = list->create_label("_CMP_AFTER_" + i2s(num_labels ++));
				for (int k=0; k<param[0].type->size/4; k++) {
					cmd.add_cmd(Asm::InstID::CMP, param_shift(param[0], k*4, TypeInt), param_shift(param[1], k*4, TypeInt));
					cmd.add_cmd(Asm::InstID::SETZ, ret);
					if (k < param[0].type->size/4 - 1)
						cmd.add_cmd(Asm::InstID::JNZ, param_label32(label_after_cmp));
				}
				cmd.add_label(label_after_cmp);
			} else {
				cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
				cmd.add_cmd(Asm::InstID::SETZ, ret);
			}
			break;
		case InlineID::CHUNK_NOT_EQUAL:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::PASSTHROUGH:
			cmd.add_cmd(Asm::InstID::MOV, ret, param[0]);
			break;
// int
		case InlineID::INT_ADD_ASSIGN:
		case InlineID::INT64_ADD_ASSIGN:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param[1]);
			break;
		case InlineID::INT_SUBTRACT_ASSIGN:
		case InlineID::INT64_SUBTRACT_ASSIGN:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param[1]);
			break;
		case InlineID::INT_MULTIPLY_ASSIGN:
		case InlineID::INT64_MULTIPLY_ASSIGN:
			cmd.add_cmd(Asm::InstID::IMUL, param[0], param[1]);
			break;
		case InlineID::INT_DIVIDE_ASSIGN:
		case InlineID::INT64_DIVIDE_ASSIGN:
			cmd.add_cmd(Asm::InstID::IDIV, param[0], param[1]);
			break;
		case InlineID::INT_ADD:
		case InlineID::INT64_ADD:
			cmd.add_cmd(Asm::InstID::ADD, ret, param[0], param[1]);
			break;
		case InlineID::INT64_ADD_INT:{
			auto t = add_temp(TypeInt64, false);
			cmd.add_cmd(Asm::InstID::MOVSX, t, param[1]);
			cmd.add_cmd(Asm::InstID::ADD, ret, param[0], t);
			}break;
		case InlineID::INT_SUBTRACT:
		case InlineID::INT64_SUBTRACT:
			cmd.add_cmd(Asm::InstID::SUB, ret, param[0], param[1]);
			break;
		case InlineID::INT_MULTIPLY:
		case InlineID::INT64_MULTIPLY:
			cmd.add_cmd(Asm::InstID::IMUL, ret, param[0], param[1]);
			break;
		case InlineID::INT_DIVIDE:
		case InlineID::INT64_DIVIDE:
			cmd.add_cmd(Asm::InstID::IDIV, ret, param[0], param[1]);
			break;
		case InlineID::INT_MODULO:
		case InlineID::INT64_MODULO:
			cmd.add_cmd(Asm::InstID::MODULO, ret, param[0], param[1]);
			break;
		case InlineID::INT_EQUAL:
		case InlineID::INT64_EQUAL:
		case InlineID::POINTER_EQUAL:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETZ, ret);
			break;
		case InlineID::INT_NOT_EQUAL:
		case InlineID::INT64_NOT_EQUAL:
		case InlineID::POINTER_NOT_EQUAL:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::INT_GREATER:
		case InlineID::INT64_GREATER:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNLE, ret);
			break;
		case InlineID::INT_GREATER_EQUAL:
		case InlineID::INT64_GREATER_EQUAL:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNL, ret);
			break;
		case InlineID::INT_SMALLER:
		case InlineID::INT64_SMALLER:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETL, ret);
			break;
		case InlineID::INT_SMALLER_EQUAL:
		case InlineID::INT64_SMALLER_EQUAL:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETLE, ret);
			break;
		case InlineID::INT_AND:
		case InlineID::INT64_AND:
			cmd.add_cmd(Asm::InstID::AND, ret, param[0], param[1]);
			break;
		case InlineID::INT_OR:
		case InlineID::INT64_OR:
			cmd.add_cmd(Asm::InstID::OR, ret, param[0], param[1]);
			break;
		case InlineID::INT_SHIFT_RIGHT:
		case InlineID::INT64_SHIFT_RIGHT:
			cmd.add_cmd(Asm::InstID::SHR, ret, param[0], param[1]);
			break;
		case InlineID::INT_SHIFT_LEFT:
		case InlineID::INT64_SHIFT_LEFT:
			cmd.add_cmd(Asm::InstID::SHL, ret, param[0], param[1]);
			break;
		case InlineID::INT_NEGATIVE:
		case InlineID::INT64_NEGATIVE:
			cmd.add_cmd(Asm::InstID::SUB, ret, param_imm(TypeInt, 0x0), param[0]);
			break;
		case InlineID::INT_INCREASE:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param_imm(TypeInt, 0x1));
			break;
		case InlineID::INT64_INCREASE:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param_imm(TypeInt64, 0x1));
			break;
		case InlineID::INT_DECREASE:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param_imm(TypeInt, 0x1));
			break;
		case InlineID::INT64_DECREASE:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param_imm(TypeInt64, 0x1));
			break;
		case InlineID::INT64_TO_INT:
		case InlineID::INT_TO_INT64:
			cmd.add_cmd(Asm::InstID::MOVSX, ret, param[0]);
			break;
// float
		case InlineID::FLOAT_ADD_ASSIGN:
		case InlineID::FLOAT64_ADD_ASSIGN:
			cmd.add_cmd(Asm::InstID::FADD, param[0], param[1]);
			break;
		case InlineID::FLOAT_SUBTRACT_ASSIGN:
		case InlineID::FLOAT64_SUBTRACT_ASSIGN:
			cmd.add_cmd(Asm::InstID::FSUB, param[0], param[1]);
			break;
		case InlineID::FLOAT_MULTIPLY_ASSIGN:
		case InlineID::FLOAT64_MULTIPLY_ASSIGN:
			cmd.add_cmd(Asm::InstID::FMUL, param[0], param[1]);
			break;
		case InlineID::FLOAT_DIVIDE_ASSIGN:
		case InlineID::FLOAT64_DIVIDE_ASSIGN:
			cmd.add_cmd(Asm::InstID::FDIV, param[0], param[1]);
			break;
		case InlineID::FLOAT_ADD:
		case InlineID::FLOAT64_ADD:
			cmd.add_cmd(Asm::InstID::FADD, ret, param[0], param[1]);
			break;
		case InlineID::FLOAT_SUBTARCT:
		case InlineID::FLOAT64_SUBTRACT:
			cmd.add_cmd(Asm::InstID::FSUB, ret, param[0], param[1]);
			break;
		case InlineID::FLOAT_MULTIPLY:
		case InlineID::FLOAT64_MULTIPLY:
			cmd.add_cmd(Asm::InstID::FMUL, ret, param[0], param[1]);
			break;
		case InlineID::FLOAT_DIVIDE:
		case InlineID::FLOAT64_DIVIDE:
			cmd.add_cmd(Asm::InstID::FDIV, ret, param[0], param[1]);
			break;
		case InlineID::FLOAT_EQUAL:
		case InlineID::FLOAT64_EQUAL:
			cmd.add_cmd(Asm::InstID::UCOMISS, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETZ, ret);
			break;
		case InlineID::FLOAT_NOT_EQUAL:
		case InlineID::FLOAT64_NOT_EQUAL:
			cmd.add_cmd(Asm::InstID::UCOMISS, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::FLOAT_SMALLER:
		case InlineID::FLOAT64_SMALLER:
			cmd.add_cmd(Asm::InstID::UCOMISS, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETB, ret);
			break;
		case InlineID::FLOAT_SMALLER_EQUAL:
		case InlineID::FLOAT64_SMALLER_EQUAL:
			cmd.add_cmd(Asm::InstID::UCOMISS, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETBE, ret);
			break;
		case InlineID::FLOAT_GREATER:
		case InlineID::FLOAT64_GREATER:
			cmd.add_cmd(Asm::InstID::UCOMISS, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNBE, ret);
			break;
		case InlineID::FLOAT_GREATER_EQUAL:
		case InlineID::FLOAT64_GREATER_EQUAL:
			cmd.add_cmd(Asm::InstID::UCOMISS, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNB, ret);
			break;

		case InlineID::FLOAT_NEGATIVE:
			cmd.add_cmd(Asm::InstID::XOR, ret, param[0], param_imm(TypeInt, 0x80000000));
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
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			if ((index == InlineID::CHAR_EQUAL) or (index == InlineID::BOOL_EQUAL))
				cmd.add_cmd(Asm::InstID::SETZ, ret);
			else if ((index == InlineID::CHAR_NOT_EQUAL) or (index == InlineID::BOOL_NOT_EQUAL))
				cmd.add_cmd(Asm::InstID::SETNZ, ret);
			else if (index == InlineID::CHAR_GREATER)
				cmd.add_cmd(Asm::InstID::SETNLE, ret);
			else if (index == InlineID::CHAR_GREATER_EQUAL)
				cmd.add_cmd(Asm::InstID::SETNL, ret);
			else if (index == InlineID::CHAR_SMALLER)
				cmd.add_cmd(Asm::InstID::SETL, ret);
			else if (index == InlineID::CHAR_SMALLER_EQUAL)
				cmd.add_cmd(Asm::InstID::SETLE, ret);
			break;
		case InlineID::BOOL_AND:
			cmd.add_cmd(Asm::InstID::AND, ret, param[0], param[1]);
			break;
		case InlineID::BOOL_OR:
			cmd.add_cmd(Asm::InstID::OR, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_ADD_ASSIGN:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param[1]);
			break;
		case InlineID::CHAR_SUBTRACT_ASSIGN:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param[1]);
			break;
		case InlineID::CHAR_ADD:
			cmd.add_cmd(Asm::InstID::ADD, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_SUBTRACT:
			cmd.add_cmd(Asm::InstID::SUB, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_AND:
			cmd.add_cmd(Asm::InstID::AND, ret, param[0], param[1]);
			break;
		case InlineID::CHAR_OR:
			cmd.add_cmd(Asm::InstID::OR, ret, param[0], param[1]);
			break;
		case InlineID::BOOL_NOT:
			cmd.add_cmd(Asm::InstID::XOR, ret, param[0], param_imm(TypeBool, 0x1));
			break;
		case InlineID::CHAR_NEGATIVE:
			cmd.add_cmd(Asm::InstID::SUB, ret, param[0], param_imm(TypeChar, 0x0));
			break;
// vec2
		case InlineID::VEC2_ADD_ASSIGN:
			cmd.add_cmd(Asm::InstID::FADD, param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FADD, param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::VEC2_SUBTARCT_ASSIGN:
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::VEC2_ADD:
			cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, 0, TypeFloat32), param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, 4, TypeFloat32), param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::VEC2_SUBTRACT:
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, 0, TypeFloat32), param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, 4, TypeFloat32), param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::VEC2_MULTIPLY_SV:
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 0, TypeFloat32), param[0], param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 4, TypeFloat32), param[0], param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::VEC2_MULTIPLY_VS:
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 0, TypeFloat32), param_shift(param[0], 0, TypeFloat32), param[1]);
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 4, TypeFloat32), param_shift(param[0], 4, TypeFloat32), param[1]);
			break;
		case InlineID::VEC2_DIVIDE_VS:
			cmd.add_cmd(Asm::InstID::FDIV, param_shift(ret, 0 * 4, TypeFloat32), param_shift(param[0], 0 * 4, TypeFloat32), param[1]);
			cmd.add_cmd(Asm::InstID::FDIV, param_shift(ret, 1 * 4, TypeFloat32), param_shift(param[0], 1 * 4, TypeFloat32), param[1]);
			break;
// complex
		case InlineID::COMPLEX_MULTIPLY:{
			auto t1 = add_temp(TypeFloat32);
			auto t2 = add_temp(TypeFloat32);
			// t1 = a.x * b.x
			cmd.add_cmd(Asm::InstID::FMUL, t1, param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			// t2 = a.y * b.y
			cmd.add_cmd(Asm::InstID::FMUL, t2, param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			// r.x = t1 - t2
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, 0, TypeFloat32), t1, t2);
			// t1 = a.x * b.y
			cmd.add_cmd(Asm::InstID::FMUL, t1, param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			// t2 = a.y * b.x
			cmd.add_cmd(Asm::InstID::FMUL, t2, param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			// r.y = t1 + t2
			cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, 4, TypeFloat32), t1, t2);
			}break;
// vector
		case InlineID::VECTOR_ADD_ASSIGN:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FADD, param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::VECTOR_MULTIPLY_ASSIGN:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::VECTOR_DIVIDE_ASSIGN:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FDIV, param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::VECTOR_SUBTARCT_ASSIGN:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FSUB, param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::VECTOR_ADD:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::VECTOR_SUBTRACT:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::VECTOR_MULTIPLY_VF:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::VECTOR_MULTIPLY_FV:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, i * 4, TypeFloat32), param[0], param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::VECTOR_MULTIPLY_VV:{
			cmd.add_cmd(Asm::InstID::FMUL, ret, param_shift(param[0], 0 * 4, TypeFloat32), param_shift(param[1], 0 * 4, TypeFloat32));
			auto t = add_temp(TypeFloat32);
			for (int i=1;i<3;i++) {
				cmd.add_cmd(Asm::InstID::FMUL, t, param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
				cmd.add_cmd(Asm::InstID::FADD, ret, t);
			}
			}break;
		case InlineID::VECTOR_DIVIDE_VF:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FDIV, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::VECTOR_NEGATIVE:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::XOR, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_imm(TypeInt, 0x80000000));
			break;
		default:
			do_error("inline function unimplemented: " + com->as_func()->signature(TypeVoid));
	}
}

void Serializer::fix_return_by_ref() {



#if 0
	for (int i=0; i<cmd.cmd.num; i++) {
		if (cmd.cmd[i].inst == Asm::InstID::RET) {

	if (com->params.num > 0){
		auto operand = serialize_parameter(com->params[0].get(), block, index);

		if (cur_func->effective_return_type->_amd64_allow_pass_in_xmm()) {
			insert_destructors_block(block, true);
			// if ((config.instruction_set == Asm::INSTRUCTION_SET_AMD64) or (config.compile_os)) ???
			//		cmd.add_cmd(Asm::InstID::FLD, t);
			if (cur_func->effective_return_type == TypeFloat32){
				cmd.add_cmd(Asm::InstID::MOVSS, p_xmm0, operand);
			}else if (cur_func->effective_return_type == TypeFloat64){
				cmd.add_cmd(Asm::InstID::MOVSD, p_xmm0, operand);
			}else if (cur_func->effective_return_type->size == 8){ // float[2]
				cmd.add_cmd(Asm::InstID::MOVLPS, p_xmm0, operand);
			}else if (cur_func->effective_return_type->size == 12){ // float[3]
				cmd.add_cmd(Asm::InstID::MOVLPS, p_xmm0, param_shift(operand, 0, TypeReg64));
				cmd.add_cmd(Asm::InstID::MOVSS, p_xmm1, param_shift(operand, 8, TypeFloat32));
			}else if (cur_func->effective_return_type->size == 16){ // float[4]
				cmd.add_cmd(Asm::InstID::MOVLPS, p_xmm0, param_shift(operand, 0, TypeReg64));
				cmd.add_cmd(Asm::InstID::MOVLPS, p_xmm1, param_shift(operand, 8, TypeReg64));
			} else {
				do_error("...ret xmm " + cur_func->effective_return_type->long_name());
			}
			add_function_outro(cur_func);
		} else if (cur_func->effective_return_type->uses_return_by_memory()){ // we already got a return address in [ebp+0x08] (> 4 byte)
			insert_destructors_block(block, true);
			// internally handled...
#if 0
			int s = mem_align(cur_func->effective_return_type->size);

			// slow
			/*SerialCommandParam p, p_deref;
			p.kind = KindVarLocal;
			p.type = TypeReg32;
			p.p = (char*) 0x8;
			p.shift = 0;
			for (int j=0;j<s/4;j++){
				AddDereference(p, p_deref);
				cmd.add_cmd(Asm::inst_mov, p_deref, param_shift(param[0], j * 4, TypeInt));
				cmd.add_cmd(Asm::inst_add, p, param_const(TypeInt, (void*)0x4));
			}*/

			// test
			SerialNodeParam p_edx = param_reg(TypeReg32, Asm::REG_EDX), p_deref_edx;
			SerialNodeParam p_ret_addr;
			p_ret_addr.kind = NodeKind::VAR_LOCAL;
			p_ret_addr.type = TypeReg32;
			p_ret_addr.p = (char*)0x8;
			p_ret_addr.shift = 0;
			int c_0 = cmd.cmd.num;
			cmd.add_cmd(Asm::InstID::MOV, p_edx, p_ret_addr);
			add_dereference(p_edx, p_deref_edx, TypeReg32);
			for (int j=0;j<s/4;j++)
				cmd.add_cmd(Asm::InstID::MOV, param_shift(p_deref_edx, j * 4, TypeInt), param_shift(params[0], j * 4, TypeInt));
			add_reg_channel(Asm::REG_EDX, c_0, cmd.cmd.num - 1);
#endif

			add_function_outro(cur_func);
		}else{ // store return directly in eax / fpu stack (4 byte)
			SerialNodeParam t = add_temp(cur_func->effective_return_type);
			cmd.add_cmd(Asm::InstID::MOV, t, operand); //?????
			insert_destructors_block(block, true);

			if (cur_func->effective_return_type->size == 1){
				int v = cmd.add_virtual_reg(Asm::REG_AL);
				cmd.add_cmd(Asm::InstID::MOV, param_vreg(cur_func->effective_return_type, v), t);
			}else if (cur_func->effective_return_type->size == 8){
				int v = cmd.add_virtual_reg(Asm::REG_RAX);
				cmd.add_cmd(Asm::InstID::MOV, param_vreg(cur_func->effective_return_type, v), t);
			}else{
				int v = cmd.add_virtual_reg(Asm::REG_EAX);
				cmd.add_cmd(Asm::InstID::MOV, param_vreg(cur_func->effective_return_type, v), t);
			}
			add_function_outro(cur_func);
		}
	}else{
		insert_destructors_block(block, true);
		add_function_outro(cur_func);
	}
#endif
}

}
