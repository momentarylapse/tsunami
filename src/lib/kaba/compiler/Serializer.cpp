#include "../kaba.h"
#include "Serializer.h"
#include "Compiler.h"
#include "../dynamic/exception.h"
#include "../../os/msg.h"
#include "../../base/algo.h"
#include "../../base/iter.h"
#include "../template/implicit.h"


namespace kaba {


//#define debug_evil_corrections	1

//#ifdef ScriptDebug

// hmmm, do we want to insert "pop local" to read function parameters?
//  but what about the return reference?
//  (we would have calling convention dependency already here)
//  -> only params... insert return ref. in backend?


Serializer::Serializer(Module *m, Asm::InstructionWithParamsList *_list) {
	module = m;
	syntax_tree = m->tree.get();
	list = _list;
	max_push_size = 0;
	stack_max_size = 0;
	stack_offset = 0;
	call_used = false;
	cur_func_index = -1;
	cur_func = nullptr;
	num_labels = 0;

	cmd.ser = this;
}

Serializer::~Serializer() {
	/*msg_write("aa");
	list->show();
	msg_write("aa2");
	for (int i=0;i<cmd.num;i++)
		msg_write(format("%3d: ", i) + cmd[i].str(this));
	list->clear();
	msg_write("aa3");*/
}


// return of a function might need temp vars without destructor FIXME ?!?
SerialNodeParam Serializer::add_temp(const Class *t, bool add_constructor) {
	auto r = cmd._add_temp(t);

	if (t != TypeVoid) {
		if (r.type->get_destructor())
			inserted_temp.add({r, cur_block_level});

		if (add_constructor)
			add_cmd_constructor(r, NodeKind::VarTemp);
	}
	return r;
}

// unpointer...?
const Class *get_subtype(const Class *t) {
	if (t->param.num > 0)
		return t->param[0];
	msg_error("subtype wanted of... " + t->name);
	//msg_write(cur_func->Name);
	return TypeUnknown;
}


SerialNodeParam Serializer::param_vreg(const Class *type, int vreg, Asm::RegID preg) {
	if (preg == Asm::RegID::INVALID)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::Register, (int)preg, vreg, type, 0};
}

SerialNodeParam Serializer::param_deref_vreg(const Class *type, int vreg, Asm::RegID preg) {
	if (preg == Asm::RegID::INVALID)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::DereferenceRegister, (int)preg, vreg, type, 0};
}


void Serializer::cmd_list_out(const string &stage, const string &comment, bool force) {
	if (!config.allow_output_stage(stage) and !force)
		return;
	msg_write(format("-------------------------------- %s  (%s)", stage, comment));
	for (int i=0;i<cmd.cmd.num;i++)
		msg_write(format("%3d: ", i) + cmd.cmd[i].str(this));
	if (false)
		vr_list_out();
	if (true) {
		msg_write("-----------");
		for (auto &&[i,v]: enumerate(cmd.temp_var))
			msg_write(format("  %d   %d -> %d    %s   %s", i, v.first, v.last, v.type->name, v.referenced ? "-referenced-" : ""));
		msg_write("--------------------------------");
	}
	vr_list_out();
}

void Serializer::vr_list_out() {
	msg_write("---------- vr");
	for (auto &r: cmd.virtual_reg)
		msg_write(Asm::get_reg_name(r.reg) + format("  (%d)   %d -> %d", (int)r.reg_root, r.first, r.last));
}

void Serializer::add_member_function_call(Function *cf, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	if (cf->virtual_index < 0) {
		add_function_call(cf, params, ret);
	} else {
		add_virtual_function_call(cf, params, ret);
	}
}


// creates res...
SerialNodeParam Serializer::add_reference(const SerialNodeParam &param, const Class *type) {
	SerialNodeParam ret;
	if (!type)
		type = syntax_tree->type_ref(param.type, -1);
	ret.type = type;
	ret.shift = 0;
	if (param.kind == NodeKind::ConstantByAddress) {
		ret.kind = NodeKind::Immediate;
		ret.p = (int_p)((char*)param.p + param.shift);
	} else if (param.kind == NodeKind::Constant) {
		ret.kind = NodeKind::Immediate;
		ret.p = (int_p)reinterpret_cast<Constant*>((void*)(int_p)param.p)->address_runtime + param.shift;
	} else if ((param.kind == NodeKind::Immediate) or (param.kind == NodeKind::Memory)) {
		ret.kind = NodeKind::Immediate;
		if (param.shift > 0)
			msg_error("Serializer: immediade/mem + shift?!?!?");
		ret.p = param.p;
	} else if (param.kind == NodeKind::DereferenceVarTemp) {
		ret = param;
		ret.kind = NodeKind::VarTemp; // FIXME why was it param.kind ?!?!?
		ret.type = type;
	/*} else if (param.kind == NodeKind::VAR_TEMP) {
		cmd.temp_var[param.p].referenced = true;
		sdkjfhkjh
		ret = param;
		ret.kind = NodeKind::VAR_TEMP; // FIXME why was it param.kind ?!?!?*/
	} else {
		ret = add_temp(type);
		cmd.add_cmd(Asm::InstID::LEA, ret, param);
		/*if (config.instruction_set == Asm::InstructionSet::ARM) {
			if (param.kind == NodeKind::VAR_LOCAL) {
				int r = find_unused_reg(-1, -1, 4);
				add_temp(type, ret);
				add_cmd(Asm::InstID::ADD, param_vreg(TypePointer, r), param_preg(TypePointer, Asm::RegID::R13), param_const(TypeInt32, param.p));
				add_cmd(Asm::InstID::MOV, ret, param_vreg(TypePointer, r));
			} else {
				DoError("reference in ARM: " + param.str());
			}
		} else {
			add_temp(type, ret);
			if (config.instruction_set == Asm::InstructionSet::AMD64) {
				int r = add_virtual_reg(Asm::RegID::RAX);
				add_cmd(Asm::InstID::LEA, param_vreg(TypeReg64, r), param);
				add_cmd(Asm::InstID::MOV, ret, param_vreg(TypeReg64, r));
			} else {
				int r = add_virtual_reg(Asm::RegID::EAX);
				add_cmd(Asm::InstID::LEA, param_vreg(TypeReg32, r), param);
				add_cmd(Asm::InstID::MOV, ret, param_vreg(TypeReg32, r));
			}
		}*/
	}
	return ret;
}

SerialNodeParam Serializer::add_dereference(const SerialNodeParam &param, const Class *type) {
	SerialNodeParam ret;
	/*add_temp(TypePointer, ret);
	SerialCommandParam temp;
	add_temp(TypePointer, temp);
	add_cmd(Asm::inst_mov, temp, param);
	temp.kind = KindDerefVarTemp;
	add_cmd(Asm::inst_mov, ret, temp);*/

	if (param.kind == NodeKind::VarTemp) {
		return deref_temp(param, type);
	} else if (param.kind == NodeKind::Register) {
		msg_write("???  register???");
		ret = param;
		ret.kind = NodeKind::DereferenceRegister;
		ret.type = type;// ? type : get_subtype(param.type);
		ret.shift = 0;
	} else {
		if (config.target.is_arm()) {
			if (param.kind == NodeKind::LocalAddress) {
				ret = param;
				ret.kind = NodeKind::DerefereceLocalMemory;
			} else {
//				do_error("arm deref...");
				SerialNodeParam temp = add_temp(param.type);
				cmd.add_cmd(Asm::InstID::MOV, temp, param);
				return deref_temp(temp, type);
			}
		} else {
			//msg_error(string("unhandled deref ", Kind2Str(param.kind)));
			SerialNodeParam temp = add_temp(param.type);
			cmd.add_cmd(Asm::InstID::MOV, temp, param);
			return deref_temp(temp, type);
		}
	}
	return ret;
}


bool node_is_assign_mem(Node *n) {
	if (n->kind == NodeKind::CallInline) {
		return (n->as_func()->inline_no == InlineID::ChunkAssign);
	}
	// nope...
	/*if (n->kind == NodeKind::FUNCTION_CALL) {
		msg_error("test 2");
		msg_write(n->as_func()->name);
		msg_write(n->as_func()->long_name());
		return n->as_func()->name == "__assign__";
	}*/
	return false;
}


SerialNodeParam Serializer::serialize_node(Node *com, Block *block, int index) {

	/*----- direct nodes --------*/
#if __cplusplus >= 202000
	SerialNodeParam p{
			.kind = com->kind,
			.p = 0,
			.type = com->type,
			.shift = 0
	};
#else
	SerialNodeParam p{com->kind, 0, 0, com->type, 0};
#endif

	if (com->kind == NodeKind::Memory) {
		p.p = com->link_no;
		return p;
	} else if (com->kind == NodeKind::Address) {
		//p.p = com->link_no;
		if (config.fully_linear_output)
			p.p = com->link_no; // by map_address_constants_to_opcode()
		else
			p.p = (int_p)&com->link_no;
		p.kind = NodeKind::ConstantByAddress;
		return p;
	} else if (com->kind == NodeKind::VarGlobal) {
		p.p = com->link_no;
		/*p.p = (int_p)com->as_global_p();
		if (!p.p)
			script->do_error_link("variable is not linkable: " + com->as_global()->name);
		p.kind = NodeKind::MEMORY;*/
		return p;
	} else if (com->kind == NodeKind::VarLocal) {
		p.p = com->link_no;
		//p.p = com->as_local()->_offset;
		//p.kind = NodeKind::LOCAL_MEMORY;
		return p;
	} else if (com->kind == NodeKind::LocalMemory) {
		p.p = com->link_no;
		return p;
	} else if (com->kind == NodeKind::LocalAddress) {
		SerialNodeParam param = param_local(TypePointer, com->link_no);
		return add_reference(param, com->type);
	} else if (com->kind == NodeKind::Constant) {
		p.p = com->link_no;
		/*p.p = (int_p)com->as_const()->address; // FIXME ....need a cleaner approach for compiling os...
		if (config.compile_os)
			p.kind = NodeKind::MEMORY;
		else
			p.kind = NodeKind::CONSTANT_BY_ADDRESS;
		if (syntax_tree->flag_function_pointer_as_code and (com->type == TypeFunctionP)) {
			auto *fp = (Function*)(int_p)com->as_const()->as_int64();
			p.kind = NodeKind::LABEL;
			p.p = fp->_label;
		}*/
		return p;
	} else if (com->kind == NodeKind::Reference) {
		auto param = serialize_node(com->params[0].get(), block, index);
		return add_reference(param, com->type);
	} else if (com->kind == NodeKind::Dereference) {
		auto param = serialize_node(com->params[0].get(), block, index);
		/*if ((param.kind == KindVarLocal) or (param.kind == KindVarGlobal)){
			p.type = param.type->sub_type;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		return add_dereference(param, com->type);
	} else if (com->kind == NodeKind::VarTemp) {
		// only used by <new> operator
		p.p = com->link_no;
		return p;
	}


	/*----- complex nodes --------*/

	if (com->kind == NodeKind::Statement) {
		// for/while can't pre-serialize params, because they need a label to this point
		return serialize_statement(com, block, index);
	} else if (com->kind == NodeKind::Block) {
		return serialize_block(com->as_block());
	}


	// EXPERIMENTAL DIRTY HACK !!!!!!!!!!!
	//syntax_tree->ShowNode(com, cur_func);
	Node *override_ret = nullptr;
#if 1
	if (node_is_assign_mem(com) /*and (config.abi == Abi::AMD64_GNU)*/) {
		auto dst = com->params[0].get();
		auto src = com->params[1].get();
		if (src->kind == NodeKind::CallFunction or src->kind == NodeKind::CallInline) {
			if (dst->kind == NodeKind::VarLocal or dst->kind == NodeKind::VarGlobal or dst->kind == NodeKind::LocalAddress) {
				override_ret = dst;
				com = src;

			}
		}
	}
#endif


	// return value
	SerialNodeParam ret;
	if (override_ret) {
		ret = serialize_node(override_ret, block, index);
	} else {
		bool create_constructor_for_return = ((com->kind != NodeKind::Statement) and (com->kind != NodeKind::CallFunction) and (com->kind != NodeKind::CallVirtual));
		ret = add_temp(com->type, create_constructor_for_return);
	}


	auto serialize_params = [this, block, index] (Node *com) {
		Array<SerialNodeParam> params;
		params.resize(com->params.num);
		for (int p=0; p<com->params.num; p++)
			params[p] = serialize_node(com->params[p].get(), block, index);
		return params;
	};


	if (com->kind == NodeKind::CallFunction) {
		const auto params = serialize_params(com);
		add_function_call(com->as_func(), params, ret);
	} else if (com->kind == NodeKind::CallVirtual) {
		const auto params = serialize_params(com);
		add_member_function_call(com->as_func(), params, ret);
	} else if (com->kind == NodeKind::CallInline) {
		const auto params = serialize_params(com);
		serialize_inline_function(com, params, ret);
	} else if (com->kind == NodeKind::CallRawPointer) {
		const auto params = serialize_params(com);
		add_pointer_call(params[0], params.sub_ref(1), ret);
	} else {
		do_error("type of command is unimplemented: " + kind2str(com->kind));
	}

	return ret;
}

SerialNodeParam Serializer::serialize_block(Block *block) {
	block->_label_start = list->create_label("_BLOCK_START_" + p2s(block));
	block->_label_end = list->create_label("_BLOCK_END_" + p2s(block));
	cmd.add_label(block->_label_start);

	SerialNodeParam ret{};

	cur_block_level ++;

	insert_constructors_block(block);

	for (int i=0; i<block->params.num; i++) {
		stack_offset = cur_func->_var_size;

		// serialize
		ret = serialize_node(block->params[i].get(), block, i);

		// keep return temp vars alive longer...
		if (i == block->params.num - 1 and ret.kind == NodeKind::VarTemp) {
			for (int k=0; k<inserted_temp.num; k++)
				if (inserted_temp[k].param.p == ret.p)
					inserted_temp[k].block_level = cur_block_level - 1;
		}
		
		// destruct new temp vars
		insert_destructors_temp();

		// end of loop?
		// DEPRECATING...
		if (loop_stack.num > 0)
			if ((loop_stack.back().level == block->level) and (loop_stack.back().index == i - 1))
				loop_stack.pop();
	}

	insert_destructors_block(block);

	cur_block_level --;

	cmd.add_label(block->_label_end);
	return ret;
}

void Serializer::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	[[maybe_unused]] int push_size = function_call_push_params(f, params, ret);

	SerialNodeParam fp = {NodeKind::Function, (int_p)f, -1, TypeFunctionRef, 0};
	cmd.add_cmd(Asm::InstID::CALL, ret, fp); // the actual call
}

void Serializer::add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	call_used = true;
	[[maybe_unused]] int push_size = function_call_push_params(f, params, ret);

	auto t1 = add_temp(TypePointer);
	auto t2 = add_temp(TypePointer);
	auto t3 = add_temp(TypeFunctionCodeRef);
	cmd.add_cmd(Asm::InstID::MOV, t1, params[0]); // self
	cmd.add_cmd(Asm::InstID::ADD, t2, deref_temp(t1, TypePointer), param_imm(TypeInt32, config.target.pointer_size * f->virtual_index)); // vtable + n
	cmd.add_cmd(Asm::InstID::MOV, t3, deref_temp(t2, TypeFunctionCodeRef)); // vtable[n]
	cmd.add_cmd(Asm::InstID::CALL_MEMBER, ret, t3); // the actual call
}

inline SerialNodeParam auto_weakify(Serializer *ser, Function *f, const SerialNodeParam& p) {
	if (p.type->is_pointer_owned() or p.type->is_pointer_shared()) {
		//msg_write("AUTO WEAK  " + f->signature());
		auto pp = p;
		pp.type = ser->module->tree->get_pointer(p.type->param[0]);
		return pp;
	}
	return p;
}

int Serializer::function_call_push_params(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	for (SerialNodeParam &p: params)
		cmd.add_cmd(Asm::InstID::PUSH, auto_weakify(this, f, p));
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


void Serializer::serialize_assign(const SerialNodeParam& p1, const SerialNodeParam& p2, Block *block, int token_id) {
	if (p1.type == TypeVoid)
		return;
	if (p1.type->can_memcpy()) {
		cmd.add_cmd(Asm::InstID::MOV, p1, p2);
	} else {
		if (auto f = p1.type->get_assign()) {
			auto type_ref = syntax_tree->request_implicit_class_reference(p1.type, token_id);
			auto r1 = add_reference(p1, type_ref);
			auto r2 = add_reference(p2, type_ref);
			auto call = add_node_call(f, token_id);
			add_function_call(f, {r1, r2}, p_none);
		} else {
			do_error(format("can not copy type '%s' out of if", p1.type->long_name()));
		}
	}
}

SerialNodeParam Serializer::serialize_statement(Node *com, Block *block, int index) {
	auto statement = com->as_statement();
	switch (statement->id) {
		case StatementID::If:
			if (com->params.num == 2) { // if
				int label_after_true = list->create_label("_IF_AFTER_" + i2s(num_labels ++));
				auto cond = serialize_node(com->params[0].get(), block, index);
				// cmp;  jz m;  -block-  m;
				cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
				cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_true));
				serialize_node(com->params[1].get(), block, index);
				cmd.add_label(label_after_true);
			} else { // if/else
				auto ret = add_temp(com->type, true);

				int label_after_true = list->create_label("_IF_AFTER_TRUE_" + i2s(num_labels ++));
				int label_after_false = list->create_label("_IF_AFTER_FALSE_" + i2s(num_labels ++));
				auto cond = serialize_node(com->params[0].get(), block, index);
				// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
				cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
				cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_true)); // jz ...
				const auto ret_true = serialize_node(com->params[1].get(), block, index);
				if (com->type != TypeVoid)
					serialize_assign(ret, ret_true, block, com->token_id);
				cmd.add_cmd(Asm::InstID::JMP, param_label32(label_after_false));
				cmd.add_label(label_after_true);
				auto ret_false = serialize_node(com->params[2].get(), block, index);
				if (com->type != TypeVoid)
					serialize_assign(ret, ret_false, block, com->token_id);
				cmd.add_label(label_after_false);

				return ret;
			}
			break;
		case StatementID::While:{
			int label_before_while = list->create_label("_WHILE_BEFORE_" + i2s(num_labels ++));
			int label_after_while = list->create_label("_WHILE_AFTER_" + i2s(num_labels ++));
			cmd.add_label(label_before_while);
			auto cond = serialize_node(com->params[0].get(), block, index);
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_while));

			// body of loop
			LoopData l = {label_before_while, label_after_while, block->level, index};
			loop_stack.add(l);
			serialize_node(com->params[1].get(), block, index);
			loop_stack.pop();

			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_before_while));
			cmd.add_label(label_after_while);
			}break;
		case StatementID::ForDigest:{
			int label_before_for = list->create_label("_FOR_BEFORE_" + i2s(num_labels ++));
			int label_after_for = list->create_label("_FOR_AFTER_" + i2s(num_labels ++));
			int label_continue = list->create_label("_FOR_CONTINUE_" + i2s(num_labels ++));
			serialize_node(com->params[0].get(), block, index); // i=0
			cmd.add_label(label_before_for);
			auto cond = serialize_node(com->params[1].get(), block, index);
			// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
			// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
			cmd.add_cmd(Asm::InstID::CMP, cond, param_imm(TypeBool, 0x0));
			cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_for));

			// body of loop
			LoopData l = {label_continue, label_after_for, block->level, index};
			loop_stack.add(l);
			serialize_node(com->params[2].get(), block, index);
			loop_stack.pop();

			// "i++"
			cmd.add_label(label_continue);
			serialize_node(com->params[3].get(), block, index);

			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_before_for));
			cmd.add_label(label_after_for);
			}break;
		case StatementID::Break:
			// FIXME: destructors
			cmd.add_cmd(Asm::InstID::JMP, param_label32(loop_stack.back().label_break));
			break;
		case StatementID::Continue:
			// FIXME: destructors
			cmd.add_cmd(Asm::InstID::JMP, param_label32(loop_stack.back().label_continue));
			break;
		case StatementID::Return:
			if (com->params.num > 0) {
				auto p = serialize_node(com->params[0].get(), block, index);
				if (p.kind == NodeKind::DereferenceVarTemp and !cur_func->literal_return_type->uses_return_by_memory()) {
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
		case StatementID::New:{
			auto ret = add_temp(com->type, false);

			// malloc()
			auto f = syntax_tree->required_func_global("@malloc");
			auto type = com->params[0]->as_func()->name_space; // ret.type->param[0]
			add_function_call(f, {param_imm(TypeInt32, type->size)}, ret);

			// __init__()
			auto sub = com->params[0]->shallow_copy();
			Node *c_ret = new Node(NodeKind::VarTemp, ret.p, ret.type);
			sub->set_instance(c_ret);
			serialize_node(sub.get(), block, index);
			//delete sub;
			return ret;}
		case StatementID::Delete:{
			// __delete__()
			auto operand = serialize_node(com->params[0].get(), block, index);
			add_cmd_destructor(operand, false);

			// free()
			auto f = syntax_tree->required_func_global("@free");
			add_function_call(f, {operand}, p_none);
			break;}
		case StatementID::Raise:{
			auto e = com->params[0]->as_const_p();
			auto f = syntax_tree->required_func_global(block->is_in_try() ? Identifier::Raise : "@die");
			add_function_call(f, {param_global(TypePointer, e)}, p_none);
			break;}
		case StatementID::RaiseLocal:{
			// FIXME destructors!
			cmd.add_cmd(Asm::InstID::JMP, param_label32(try_stack.back().label_except));
			break;}
		case StatementID::Try:{
			int label_except = list->create_label("_TRY_EXCEPT_" + i2s(num_labels ++));
			int label_after = list->create_label("_TRY_AFTER_" + i2s(num_labels ++));

			// try
			try_stack.add({label_except, label_after});
			serialize_node(com->params[0].get(), block, index);
			try_stack.pop();
			cmd.add_cmd(Asm::InstID::JMP, param_label32(label_after));

			// except
			for (int i=2; i<com->params.num; i+=2) {
				if (i == com->params.num-1)
					cmd.add_label(label_except);
				serialize_node(com->params[i].get(), block, index);
				if (i < com->params.num-1)
					cmd.add_cmd(Asm::InstID::JMP, param_label32(label_after));
			}

			if (com->params.num == 1)
				cmd.add_label(label_except);
			cmd.add_label(label_after);
			}break;
		case StatementID::Asm:
			cmd.add_cmd(Asm::InstID::ASM, param_imm(TypeInt32, com->params[0]->as_const()->as_int()));
			break;
		case StatementID::Pass:
			break;
		case StatementID::RawFunctionPointer: {
			auto ret = add_temp(com->type, false);
			// only from callable can reach here!
			if (config.fully_linear_output)
				do_error("implicit raw_function_pointer() for os not implemented yet (i.e. don't use callables/function pointers)");
			auto func = serialize_node(com->params[0].get(), block, index);
			auto t1 = add_temp(TypePointer);
			cmd.add_cmd(Asm::InstID::ADD, t1, func, param_imm(TypeInt32, config.function_address_offset)); // Function* pointer
			cmd.add_cmd(Asm::InstID::MOV, ret, deref_temp(t1, TypeFunctionCodeRef)); // the actual code pointer
			return ret;}
		default:
			do_error("statement unimplemented: " + com->as_statement()->name);
	}
	return p_none;
}

void Serializer::serialize_inline_function(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) {
	auto index = com->as_func()->inline_no;
	switch (index) {
		case InlineID::Int32ToFloat32:
			cmd.add_cmd(Asm::InstID::CVTSI2SS, ret, param[0]);
			break;
		case InlineID::FloatToInt32:
			cmd.add_cmd(Asm::InstID::CVTTSS2SI, ret, param[0]);
			break;
		case InlineID::Float32ToFloat64:
			cmd.add_cmd(Asm::InstID::CVTSS2SD, ret, param[0]);
			break;
		case InlineID::Float64ToFloat32:
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
		case InlineID::PointerToBool:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param_imm(TypePointer, 0));
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::Int32ToUint8:
		case InlineID::Uint8ToInt32:
			cmd.add_cmd(Asm::InstID::MOVZX, ret, param[0]);
			break;
		case InlineID::Int32ToInt8:
		case InlineID::Int8ToInt32:
			cmd.add_cmd(Asm::InstID::MOVSX, ret, param[0]);
			break;
		case InlineID::RectSet:
		case InlineID::VectorSet:
		case InlineID::ComplexSet:
		case InlineID::ColorSet:
			for (int i=0; i<ret.type->size/4; i++)
				cmd.add_cmd(Asm::InstID::MOV, param_shift(ret, i*4, TypeFloat32), param[i]);
			break;
		case InlineID::Int32Assign:
		case InlineID::Int64Assign:
		case InlineID::Float32Assign:
		case InlineID::Float64Assign:
		case InlineID::PointerAssign:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param[1]);
			break;
		case InlineID::SharedPointerInit:
			// FIXME does this work for 64bit?!?
			cmd.add_cmd(Asm::InstID::MOV, param[0], param_imm(TypeInt32, 0));
			break;
		case InlineID::Int8Assign:
		case InlineID::BoolAssign:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param[1]);
			break;
// chunk...
		case InlineID::ChunkAssign:
			cmd.add_cmd(Asm::InstID::MOV, param[0], param[1]);
			break;
		case InlineID::ChunkEqual:
			if (param[0].type->size > config.target.pointer_size) {
				// chunk cmp
				int label_after_cmp = list->create_label("_CMP_AFTER_" + i2s(num_labels ++));
				for (int k=0; k<param[0].type->size/4; k++) {
					cmd.add_cmd(Asm::InstID::CMP, param_shift(param[0], k*4, TypeInt32), param_shift(param[1], k*4, TypeInt32));
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
		case InlineID::ChunkNotEqual:
			if (param[0].type->size > config.target.pointer_size) {
				// chunk cmp
				int label_after_cmp = list->create_label("_CMP_AFTER_" + i2s(num_labels ++));
				for (int k=0; k<param[0].type->size/4; k++) {
					cmd.add_cmd(Asm::InstID::CMP, param_shift(param[0], k*4, TypeInt32), param_shift(param[1], k*4, TypeInt32));
					cmd.add_cmd(Asm::InstID::SETNZ, ret);
					if (k < param[0].type->size/4 - 1)
						cmd.add_cmd(Asm::InstID::JZ, param_label32(label_after_cmp));
				}
				cmd.add_label(label_after_cmp);
			} else {
				cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
				cmd.add_cmd(Asm::InstID::SETNZ, ret);
			}
			break;
		case InlineID::Passthrough:
			cmd.add_cmd(Asm::InstID::MOV, ret, param[0]);
			break;
// int
		case InlineID::Int32AddAssign:
		case InlineID::Int64AddAssign:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param[1]);
			break;
		case InlineID::Int32SubtractAssign:
		case InlineID::Int64SubtractAssign:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param[1]);
			break;
		case InlineID::Int32MultiplyAssign:
		case InlineID::Int64MultiplyAssign:
			cmd.add_cmd(Asm::InstID::IMUL, param[0], param[1]);
			break;
		case InlineID::Int32DivideAssign:
		case InlineID::Int64DivideAssign:
			cmd.add_cmd(Asm::InstID::IDIV, param[0], param[1]);
			break;
		case InlineID::Int32Add:
		case InlineID::Int64Add:
			cmd.add_cmd(Asm::InstID::ADD, ret, param[0], param[1]);
			break;
		case InlineID::Int64AddInt32:{
			auto t = add_temp(TypeInt64, false);
			cmd.add_cmd(Asm::InstID::MOVSX, t, param[1]);
			cmd.add_cmd(Asm::InstID::ADD, ret, param[0], t);
			}break;
		case InlineID::Int32Subtract:
		case InlineID::Int64Subtract:
			cmd.add_cmd(Asm::InstID::SUB, ret, param[0], param[1]);
			break;
		case InlineID::Int32Multiply:
		case InlineID::Int64Multiply:
			cmd.add_cmd(Asm::InstID::IMUL, ret, param[0], param[1]);
			break;
		case InlineID::Int32Divide:
		case InlineID::Int64Divide:
			cmd.add_cmd(Asm::InstID::IDIV, ret, param[0], param[1]);
			break;
		case InlineID::Int32Modulo:
		case InlineID::Int64Modulo:
			cmd.add_cmd(Asm::InstID::MODULO, ret, param[0], param[1]);
			break;
		case InlineID::Int32Equal:
		case InlineID::Int64Equal:
		case InlineID::PointerEqual:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETZ, ret);
			break;
		case InlineID::Int32NotEqual:
		case InlineID::Int64NotEqual:
		case InlineID::PointerNotEqual:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::Int32Greater:
		case InlineID::Int64Greater:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNLE, ret);
			break;
		case InlineID::Int32GreaterEqual:
		case InlineID::Int64GreaterEqual:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNL, ret);
			break;
		case InlineID::Int32Smaller:
		case InlineID::Int64Smaller:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETL, ret);
			break;
		case InlineID::Int32SmallerEqual:
		case InlineID::Int64SmallerEqual:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETLE, ret);
			break;
		case InlineID::Int32BitAnd:
		case InlineID::Int64BitAnd:
			cmd.add_cmd(Asm::InstID::AND, ret, param[0], param[1]);
			break;
		case InlineID::Int32BitOr:
		case InlineID::Int64BitOr:
			cmd.add_cmd(Asm::InstID::OR, ret, param[0], param[1]);
			break;
		case InlineID::Int32ShiftRight:
		case InlineID::Int64ShiftRight:
			cmd.add_cmd(Asm::InstID::SHR, ret, param[0], param[1]);
			break;
		case InlineID::Int32ShiftLeft:
		case InlineID::Int64ShiftLeft:
			cmd.add_cmd(Asm::InstID::SHL, ret, param[0], param[1]);
			break;
		case InlineID::Int32Negative:
		case InlineID::Int64Negative:
			cmd.add_cmd(Asm::InstID::SUB, ret, param_imm(TypeInt32, 0x0), param[0]);
			break;
		case InlineID::Int32Increase:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param_imm(TypeInt32, 0x1));
			break;
		case InlineID::Int64Increase:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param_imm(TypeInt64, 0x1));
			break;
		case InlineID::Int32Decrease:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param_imm(TypeInt32, 0x1));
			break;
		case InlineID::Int64Decrease:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param_imm(TypeInt64, 0x1));
			break;
		case InlineID::Int64ToInt32:
		case InlineID::Int32ToInt64:
			cmd.add_cmd(Asm::InstID::MOVSX, ret, param[0]);
			break;
// float
		case InlineID::Float32AddAssign:
		case InlineID::Float64AddAssign:
			cmd.add_cmd(Asm::InstID::FADD, param[0], param[1]);
			break;
		case InlineID::Float32SubtractAssign:
		case InlineID::Float64SubtractAssign:
			cmd.add_cmd(Asm::InstID::FSUB, param[0], param[1]);
			break;
		case InlineID::Float32MultiplyAssign:
		case InlineID::Float64MultiplyAssign:
			cmd.add_cmd(Asm::InstID::FMUL, param[0], param[1]);
			break;
		case InlineID::Float32DivideAssign:
		case InlineID::Float64DivideAssign:
			cmd.add_cmd(Asm::InstID::FDIV, param[0], param[1]);
			break;
		case InlineID::Float32Add:
		case InlineID::Float64Add:
			cmd.add_cmd(Asm::InstID::FADD, ret, param[0], param[1]);
			break;
		case InlineID::Float32Subtarct:
		case InlineID::Float64Subtract:
			cmd.add_cmd(Asm::InstID::FSUB, ret, param[0], param[1]);
			break;
		case InlineID::Float32Multiply:
		case InlineID::Float64Multiply:
			cmd.add_cmd(Asm::InstID::FMUL, ret, param[0], param[1]);
			break;
		case InlineID::Float32Divide:
		case InlineID::Float64Divide:
			cmd.add_cmd(Asm::InstID::FDIV, ret, param[0], param[1]);
			break;
		case InlineID::Float32Equal:
		case InlineID::Float64Equal:
			cmd.add_cmd(Asm::InstID::FCMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETZ, ret);
			break;
		case InlineID::Float32NotEqual:
		case InlineID::Float64NotEqual:
			cmd.add_cmd(Asm::InstID::FCMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNZ, ret);
			break;
		case InlineID::Float32Smaller:
		case InlineID::Float64Smaller:
			cmd.add_cmd(Asm::InstID::FCMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETB, ret);
			break;
		case InlineID::Float32SmallerEqual:
		case InlineID::Float64SmallerEqual:
			cmd.add_cmd(Asm::InstID::FCMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETBE, ret);
			break;
		case InlineID::Float32Greater:
		case InlineID::Float64Greater:
			cmd.add_cmd(Asm::InstID::FCMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNBE, ret);
			break;
		case InlineID::Float32GreaterEqual:
		case InlineID::Float64GreaterEqual:
			cmd.add_cmd(Asm::InstID::FCMP, param[0], param[1]);
			cmd.add_cmd(Asm::InstID::SETNB, ret);
			break;

		case InlineID::Float32Negative:
			cmd.add_cmd(Asm::InstID::XOR, ret, param[0], param_imm(TypeInt32, 0x80000000));
			break;
// bool/char
		case InlineID::Int8Equal:
		case InlineID::Int8NotEqual:
		case InlineID::BoolEqual:
		case InlineID::BoolNotEqual:
		case InlineID::Int8Greater:
		case InlineID::Int8GreaterEqual:
		case InlineID::Int8Smaller:
		case InlineID::Int8SmallerEqual:
		case InlineID::Uint8Greater:
		case InlineID::Uint8GreaterEqual:
		case InlineID::Uint8Smaller:
		case InlineID::Uint8SmallerEqual:
			cmd.add_cmd(Asm::InstID::CMP, param[0], param[1]);
			if ((index == InlineID::Int8Equal) or (index == InlineID::BoolEqual))
				cmd.add_cmd(Asm::InstID::SETZ, ret);
			else if ((index == InlineID::Int8NotEqual) or (index == InlineID::BoolNotEqual))
				cmd.add_cmd(Asm::InstID::SETNZ, ret);
			else if (index == InlineID::Int8Greater)
				cmd.add_cmd(Asm::InstID::SETNLE, ret);
			else if (index == InlineID::Int8GreaterEqual)
				cmd.add_cmd(Asm::InstID::SETNL, ret);
			else if (index == InlineID::Int8Smaller)
				cmd.add_cmd(Asm::InstID::SETL, ret);
			else if (index == InlineID::Int8SmallerEqual)
				cmd.add_cmd(Asm::InstID::SETLE, ret);
			else if (index == InlineID::Uint8Greater)
				cmd.add_cmd(Asm::InstID::SETNBE, ret);
			else if (index == InlineID::Uint8GreaterEqual)
				cmd.add_cmd(Asm::InstID::SETNB, ret);
			else if (index == InlineID::Uint8Smaller)
				cmd.add_cmd(Asm::InstID::SETB, ret);
			else if (index == InlineID::Uint8SmallerEqual)
				cmd.add_cmd(Asm::InstID::SETBE, ret);
			// "above/below" -> unsigned, "greater/less" -> signed
			break;
		case InlineID::BoolAnd:
			cmd.add_cmd(Asm::InstID::AND, ret, param[0], param[1]);
			break;
		case InlineID::BoolOr:
			cmd.add_cmd(Asm::InstID::OR, ret, param[0], param[1]);
			break;
		case InlineID::Int8AddAssign:
			cmd.add_cmd(Asm::InstID::ADD, param[0], param[1]);
			break;
		case InlineID::Int8SubtractAssign:
			cmd.add_cmd(Asm::InstID::SUB, param[0], param[1]);
			break;
		case InlineID::Int8Add:
			cmd.add_cmd(Asm::InstID::ADD, ret, param[0], param[1]);
			break;
		case InlineID::Int8Subtract:
			cmd.add_cmd(Asm::InstID::SUB, ret, param[0], param[1]);
			break;
		case InlineID::Int8BitAnd:
			cmd.add_cmd(Asm::InstID::AND, ret, param[0], param[1]);
			break;
		case InlineID::Int8BitOr:
			cmd.add_cmd(Asm::InstID::OR, ret, param[0], param[1]);
			break;
		case InlineID::BoolNot:
			cmd.add_cmd(Asm::InstID::XOR, ret, param[0], param_imm(TypeBool, 0x1));
			break;
		case InlineID::Int8Negative:
			cmd.add_cmd(Asm::InstID::SUB, ret, param[0], param_imm(TypeInt8, 0x0));
			break;
// vec2
		case InlineID::Vec2AddAssign:
			cmd.add_cmd(Asm::InstID::FADD, param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FADD, param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::Vec2SubtarctAssign:
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::Vec2Add:
			cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, 0, TypeFloat32), param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, 4, TypeFloat32), param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::Vec2Subtract:
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, 0, TypeFloat32), param_shift(param[0], 0, TypeFloat32), param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, 4, TypeFloat32), param_shift(param[0], 4, TypeFloat32), param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::Vec2MultiplySV:
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 0, TypeFloat32), param[0], param_shift(param[1], 0, TypeFloat32));
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 4, TypeFloat32), param[0], param_shift(param[1], 4, TypeFloat32));
			break;
		case InlineID::Vec2MultiplyVS:
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 0, TypeFloat32), param_shift(param[0], 0, TypeFloat32), param[1]);
			cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, 4, TypeFloat32), param_shift(param[0], 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec2DivideVs:
			cmd.add_cmd(Asm::InstID::FDIV, param_shift(ret, 0 * 4, TypeFloat32), param_shift(param[0], 0 * 4, TypeFloat32), param[1]);
			cmd.add_cmd(Asm::InstID::FDIV, param_shift(ret, 1 * 4, TypeFloat32), param_shift(param[0], 1 * 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec2MultiplyAssign:
			for (int i=0;i<2;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec2DivideAssign:
			for (int i=0;i<2;i++)
				cmd.add_cmd(Asm::InstID::FDIV, param_shift(param[0], i * 4, TypeFloat32), param[1]);
		break;
	case InlineID::Vec2Negative:
		for (int i=0;i<2;i++)
			cmd.add_cmd(Asm::InstID::XOR, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_imm(TypeInt32, 0x80000000));
		break;
// complex
		case InlineID::ComplexMultiply:{
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
		case InlineID::Vec3AddAssign:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FADD, param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::Vec3MultiplyAssign:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec3DivideAssign:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FDIV, param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec3SubtarctAssign:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FSUB, param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::Vec3Add:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FADD, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::Vec3Subtract:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FSUB, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::Vec3MultiplyVF:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec3MultiplyFV:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FMUL, param_shift(ret, i * 4, TypeFloat32), param[0], param_shift(param[1], i * 4, TypeFloat32));
			break;
		case InlineID::Vec3MultiplyVV:{
			cmd.add_cmd(Asm::InstID::FMUL, ret, param_shift(param[0], 0 * 4, TypeFloat32), param_shift(param[1], 0 * 4, TypeFloat32));
			auto t = add_temp(TypeFloat32);
			for (int i=1;i<3;i++) {
				cmd.add_cmd(Asm::InstID::FMUL, t, param_shift(param[0], i * 4, TypeFloat32), param_shift(param[1], i * 4, TypeFloat32));
				cmd.add_cmd(Asm::InstID::FADD, ret, t);
			}
			}break;
		case InlineID::Vec3DivideVF:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::FDIV, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param[1]);
			break;
		case InlineID::Vec3Negative:
			for (int i=0;i<3;i++)
				cmd.add_cmd(Asm::InstID::XOR, param_shift(ret, i * 4, TypeFloat32), param_shift(param[0], i * 4, TypeFloat32), param_imm(TypeInt32, 0x80000000));
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
		auto operand = serialize_node(com->params[0].get(), block, index);

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
				cmd.add_cmd(Asm::inst_mov, p_deref, param_shift(param[0], j * 4, TypeInt32));
				cmd.add_cmd(Asm::inst_add, p, param_const(TypeInt32, (void*)0x4));
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
				cmd.add_cmd(Asm::InstID::MOV, param_shift(p_deref_edx, j * 4, TypeInt32), param_shift(params[0], j * 4, TypeInt32));
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


// modus: NodeKind::VAR_LOCAL / NodeKind::VAR_TEMP
//    -1: -return-/new   -> don't destruct
void Serializer::add_cmd_constructor(const SerialNodeParam &param, NodeKind modus) {
	const Class *class_type = param.type;
	if (modus == NodeKind::None)
		class_type = class_type->param[0];
	Function *f = class_type->get_default_constructor();
	if (!f) {
		if (class_type->needs_constructor())
			do_error("missing default constructor for " + class_type->name);
		return;
	}

	SerialNodeParam instance = param;
	if (modus == NodeKind::None) {
	} else {
		instance = add_reference(param);
	}

	add_member_function_call(f, {instance}, p_none);
}

void Serializer::add_cmd_destructor(const SerialNodeParam &param, bool needs_ref) {
	if (needs_ref) {
		Function *f = param.type->get_destructor();
		if (!f)
			return;
		SerialNodeParam inst = add_reference(param);
		add_member_function_call(f, {inst}, p_none);
	} else {
		// for explicit "del p" TODO separete function!
		Function *f = param.type->param[0]->get_destructor();
		if (!f)
			return;
		add_member_function_call(f, {param}, p_none);
	}
}

void Serializer::insert_constructors_block(Block *b) {
	for (auto *v: b->vars) {
		if (!v->explicitly_constructed) {
			SerialNodeParam param = param_local(v->type, v->_offset);
			add_cmd_constructor(param, (v->name == Identifier::ReturnVar) ? NodeKind::None : NodeKind::VarLocal);
		}
	}
}

void Serializer::insert_destructors_block(Block *b, bool recursive) {
	for (auto *v: b->vars) {
		SerialNodeParam p = param_local(v->type, v->_offset);
		add_cmd_destructor(p);
	}
}

void Serializer::insert_destructors_temp() {
	for (auto &[p, l]: inserted_temp)
		if (l >= cur_block_level) {
			add_cmd_destructor(p);
		}
	base::remove_if(inserted_temp, [this] (auto &t) {
		return t.block_level >= cur_block_level;
	});
}

int Serializer::temp_in_cmd(int c, int v) {
	if ((int)cmd.cmd[c].inst >= (int)Asm::InstID::LABEL)
		return 0;
	int r = 0;
	for (int i=0; i<SERIAL_NODE_NUM_PARAMS; i++)
		if ((cmd.cmd[c].p[i].kind == NodeKind::VarTemp) or (cmd.cmd[c].p[i].kind == NodeKind::DereferenceVarTemp))
			if (cmd.cmd[c].p[i].p == v)
				r += (1<<i) + ((cmd.cmd[c].p[i].kind == NodeKind::DereferenceVarTemp) ? (8<<i) : 0);
	return r;
}

void Serializer::scan_temp_var_usage() {
	/*msg_write("ScanTempVarUsage");
	for (auto &&[i,v]: enumerate(temp_var)) {
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

inline bool param_is_simple(SerialNodeParam &p) {
	return ((p.kind == NodeKind::Register) or (p.kind == NodeKind::VarTemp) or (p.kind == NodeKind::None));
}

inline bool param_combi_allowed(Asm::InstID inst, SerialNodeParam &p1, SerialNodeParam &p2) {
//	if (inst >= Asm::inst_label)
//		return true;
	if ((!param_is_simple(p1)) and (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	Asm::get_instruction_param_flags(inst, r1, w1, r2, w2);
	if ((w1) and (p1.kind == NodeKind::Immediate))
		return false;
	if ((w2) and (p2.kind == NodeKind::Immediate))
		return false;
	if ((p1.kind == NodeKind::Immediate) or (p2.kind == NodeKind::Immediate))
		if (!Asm::get_instruction_allow_const(inst))
			return false;
	return true;
}

// mov [0x..] [0x...]  ->  mov temp, [0x..]   mov [0x..] temp
/*void CorrectUnallowedParamCombis() {
	for (int i=cmd.num-1;i>=0;i--)
		if (!param_combi_allowed(cmd[i].inst, cmd[i].p[0], cmd[i].p[1])) {
			msg_write(string2("correcting param combi  cmd=%d", i));
			bool mov_first_param = (cmd[i].p[1].kind < 0) or (cmd[i].p[0].kind == KindRefToConst) or (cmd[i].p[0].kind == KindConstant);
			SerialCommandParam *pp = mov_first_param ? &cmd[i].p[0] : &cmd[i].p[1];
			SerialCommandParam temp, p = *pp;
			add_temp(p.type, temp);

			*pp = temp;
			if (p.type->Size == 1)
				add_cmd(Asm::inst_mov, temp, p);
			else
				add_cmd(Asm::inst_mov, temp, p);
			move_last_cmd(i);
		}
	ScanTempVarUsage();
}*/


bool Serializer::param_untouched_in_interval(SerialNodeParam &p, int first, int last) {
	// direct usage?
	for (int i=first;i<=last;i++)
		if ((cmd.cmd[i].p[0] == p) or (cmd.cmd[i].p[1] == p))
			return false;
	
	// registers may be more subtle..
	if ((p.kind == NodeKind::Register) or (p.kind == NodeKind::DereferenceRegister)) {
		for (int i=first;i<=last;i++) {
			
			// call violates all!
			if (cmd.cmd[i].inst == Asm::InstID::CALL)
				return false;

			// div violates eax and edx
			if (cmd.cmd[i].inst == Asm::InstID::DIV)
				if ((p.as_reg() == Asm::RegID::EDX) or (p.as_reg() == Asm::RegID::EAX))
					return false;

			// registers used? (may be part of the same meta-register)
			if ((cmd.cmd[i].p[0].kind == NodeKind::Register) or (cmd.cmd[i].p[0].kind == NodeKind::DereferenceRegister))
				if (Asm::reg_root[cmd.cmd[i].p[0].p] == Asm::reg_root[p.p])
					return false;
			if ((cmd.cmd[i].p[1].kind == NodeKind::Register) or (cmd.cmd[i].p[1].kind == NodeKind::DereferenceRegister))
				if (Asm::reg_root[cmd.cmd[i].p[1].p] == Asm::reg_root[p.p])
					return false;
		}
	}
	return true;
}

void Serializer::simplify_fpu_stack() {
// fstp temp
// fld temp
	for (int vi=cmd.temp_var.num-1;vi>=0;vi--) {
		TempVar &v = cmd.temp_var[vi];
		if (v.first < 0)
			continue;

		// may only appear two times
		if (v.usage_count > 2)
			continue;

		// stored then loaded...?
		if ((cmd.cmd[v.first].inst != Asm::InstID::FSTP) or (cmd.cmd[v.last].inst != Asm::InstID::FLD))
			continue;

		// value still on the stack?
		int d_stack = 0, min_d_stack = 0, max_d_stack = 0;
		for (int i=v.first + 1;i<v.last;i++) {
			if (cmd.cmd[i].inst == Asm::InstID::FLD)
				d_stack ++;
			else if (cmd.cmd[i].inst == Asm::InstID::FSTP)
				d_stack --;
			min_d_stack = min(min_d_stack, d_stack);
			max_d_stack = max(max_d_stack, d_stack);
		}
		if ((d_stack != 0) or (min_d_stack < 0) or (max_d_stack > 5))
			continue;

		// reuse value on the stack
//		msg_write(format("fpu (a)  var=%d first=%d last=%d stack: d=%d min=%d max=%d", vi, v.first, v.last, d_stack, min_d_stack, max_d_stack));
		cmd.remove_cmd(v.last);
		cmd.remove_cmd(v.first);
		cmd.remove_temp_var(vi);
	}

// fstp temp
// mov xxx, temp
	for (int vi=cmd.temp_var.num-1;vi>=0;vi--) {
		TempVar &v = cmd.temp_var[vi];
		if (v.first < 0)
			continue;

		// may only appear two times
		if (v.usage_count > 2)
			continue;

		// stored then moved...?
		if ((cmd.cmd[v.first].inst != Asm::InstID::FSTP) or (cmd.cmd[v.last].inst != Asm::InstID::MOV))
			continue;
		if (temp_in_cmd(v.last, vi) != 2)
			continue;
		// moved into fstore'able?
		auto kind = cmd.cmd[v.last].p[0].kind;
		if ((kind != NodeKind::LocalMemory) and (kind != NodeKind::Memory) and (kind != NodeKind::VarTemp) and (kind != NodeKind::DereferenceVarTemp) and (kind != NodeKind::DereferenceRegister))
		    continue;

		// check, if mov target is used in between
		SerialNodeParam target = cmd.cmd[v.last].p[0];
		if (!param_untouched_in_interval(target, v.first + 1 ,v.last - 1))
			continue;
		// ...we are lazy...
		//if (v.last - v.first != 1)
		//	continue;

		// store directly into target
//		msg_write(format("fpu (b)  var=%d first=%d last=%d", v, v.first, v.last));
		cmd.set_cmd_param(v.first, 0, target);
		cmd.move_param(target, v.last, v.first);
		cmd.remove_cmd(v.last);
		cmd.remove_temp_var(vi);
	}
}

// remove  mov x,...    mov ...,x   (and similar)
//    (does not affect reg channels)
void Serializer::simplify_movs() {
	// TODO: count > 2 .... first == input and all_other == output?  (only if first == mov (!=eax?)... else count == 2)
	// should take care of fpu simplification (b)...

	for (int vi=cmd.temp_var.num-1;vi>=0;vi--) {
		TempVar &v = cmd.temp_var[vi];
		if (v.first < 0)
			continue;
		
		// may only appear two times
		if (v.usage_count > 2)
			continue;

		// both times in a mov command (or fld as second)
		if (cmd.cmd[v.first].inst != Asm::InstID::MOV)
			continue;
		auto n = cmd.cmd[v.last].inst;
		bool fld = (n == Asm::InstID::FLD) or (n == Asm::InstID::FADD) or (n == Asm::InstID::FADD) or (n == Asm::InstID::FSUB) or (n == Asm::InstID::FMUL) or (n == Asm::InstID::FDIV);
		if ((cmd.cmd[v.last].inst != Asm::InstID::MOV) and (!fld))
			continue;
		
		// used as source/target?   no deref?
		if ((temp_in_cmd(v.first, vi) != 1) or (temp_in_cmd(v.last, vi) != (fld ? 1 : 2)))
			continue;

		// new construction allowed?
		SerialNodeParam target = cmd.cmd[v.last].p[0];
		SerialNodeParam source = cmd.cmd[v.first].p[1];
		if (fld) {
			if (!param_combi_allowed(cmd.cmd[v.last].inst, source, cmd.cmd[v.last].p[1]))
				continue;
		} else {
			if (!param_combi_allowed(cmd.cmd[v.last].inst, cmd.cmd[v.last].p[0], source))
				continue;
		}

		// check, if mov source or target are used in between
		if (!param_untouched_in_interval(target, v.first + 1 ,v.last - 1))
			continue;
		if (!param_untouched_in_interval(source, v.first + 1 ,v.last - 1))
			continue;
		
//		msg_write(format("mov simplification allowed  v=%d first=%d last=%d", vi, v.first, v.last));
		if (fld)
			cmd.set_cmd_param(v.last, 0, source);
		else
			cmd.set_cmd_param(v.last, 1, source);
		cmd.move_param(source, v.first, v.last);
		cmd.remove_cmd(v.first);
		cmd.remove_temp_var(vi);
	}

	// TODO: should happen automatically...
	//ScanTempVarUsage();
	//cmd_list_out();
}

void Serializer::remove_unused_temp_vars() {
	// unused temp vars...
	for (int v=cmd.temp_var.num-1;v>=0;v--)
		if (cmd.temp_var[v].first < 0) {
			cmd.remove_temp_var(v);
		}
}

/*inline void test_reg_usage(int c) {
	// call -> violates all...
	if (cmd.cmd[c].inst == Asm::inst_call) {
		for (int i=0;i<max_reg;i++)
			RegUsed[i] = true;
		return;
	}
	if ((cmd.cmd[c].p[0].kind == KindRegister) or (cmd.cmd[c].p[0].kind == KindDerefRegister))
		set_reg_used((long)cmd.cmd[c].p[0].p);
	if ((cmd.cmd[c].p[1].kind == KindRegister) or (cmd.cmd[c].p[1].kind == KindDerefRegister))
		set_reg_used((long)cmd.cmd[c].p[1].p);
}*/

struct StackOccupation
{
	Array<int> x;
	int reserved;
	bool down;

	void create(Serializer *s, bool down, int reserved, int first, int last)
	{
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
	void set(int start, int size)
	{
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

	bool is_free(int start, int size)
	{
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

	int find_free(int size)
	{
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

void Serializer::add_stack_var(TempVar &v, SerialNodeParam &p) {
	// find free stack space for the life span of the variable....
	// don't mind... use old algorithm: ALWAYS grow stack... remove ALL on each command in a block

	int s = mem_align(v.type->size, 4);
	StackOccupation so;
//	msg_write(format("add stack var  %s %d   %d-%d       vs=%d", v.type->name.c_str(), v.type->size, v.first, v.last, cur_func->_var_size));
//	for (auto&& [i,t]: temp_var)
//		if (&t == &v)
//			msg_write("#" + i2s(i));
	so.create(this, !config.target.is_arm(), cur_func->_var_size, v.first, v.last);

	if (true) {
	// TODO super important!!!!!!
	if (config.target.is_arm()) {
		int align = (s > 4) ? 8 : 4;
		stack_offset = mem_align(stack_offset, align);
		v.stack_offset = stack_offset;
		stack_offset += mem_align(s, align);

	} else {
		stack_offset += s;
		v.stack_offset = - stack_offset;
	}
	} else {
		v.stack_offset = so.find_free(v.type->size);
		if (config.target.is_arm()) {
			stack_offset = v.stack_offset + s;
		} else {
			stack_offset = - v.stack_offset;
		}
	}
	//msg_write(format("STACK VAR  %s =>  %d", v.type->name, v.stack_offset));

	if (stack_offset > stack_max_size)
		stack_max_size = stack_offset;

	v.mapped = true;

	p.kind = NodeKind::LocalMemory;
	p.p = v.stack_offset;
	p.type = v.type;
	p.shift = 0;
}


void Serializer::serialize_function(Function *f) {
	syntax_tree->create_asm_meta_info();
	syntax_tree->asm_meta_info->line_offset = 0;
	Asm::CurrentMetaInfo = syntax_tree->asm_meta_info.get();

	cur_func = f;
	cur_block_level = 0;
	num_labels = 0;
	call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;


// serialize

	// function

	serialize_block(f->block.get());
	insert_destructors_temp();
	scan_temp_var_usage();

	cmd_list_out("ser:a", "start");



	// outro (if last command != return)
	bool need_outro = true;
	if (f->block->params.num > 0)
		if ((f->block->params.back()->kind == NodeKind::Statement) and (f->block->params.back()->as_statement()->id == StatementID::Return))
			need_outro = false;
	if (need_outro)
		add_function_outro(f);

	cmd_list_out("ser:b", "outro");

	simplify_if_statements();
	cmd_list_out("ser:c", "simple if");
	try_merge_temp_vars();
	cmd_list_out("ser:d", "merge");
	simplify_float_store();

	cmd_list_out("ser:z", "float/end");
	


	//cmd_list_out();
}


void Serializer::simplify_if_statements() {
	// TODO fcmp (float cmp)
	for (int i=0;i<cmd.cmd.num - 4;i++) {
		if ((cmd.cmd[i].inst == Asm::InstID::CMP) and (cmd.cmd[i+2].inst == Asm::InstID::CMP) and (cmd.cmd[i+3].inst == Asm::InstID::JZ)) {
			if (cmd.cmd[i+1].inst == Asm::InstID::SETL)
				cmd.cmd[i+3].inst = Asm::InstID::JNL;
			else if (cmd.cmd[i+1].inst == Asm::InstID::SETLE)
				cmd.cmd[i+3].inst = Asm::InstID::JNLE;
			else if (cmd.cmd[i+1].inst == Asm::InstID::SETNL)
				cmd.cmd[i+3].inst = Asm::InstID::JL;
			else if (cmd.cmd[i+1].inst == Asm::InstID::SETNLE)
				cmd.cmd[i+3].inst = Asm::InstID::JLE;
			else if (cmd.cmd[i+1].inst == Asm::InstID::SETZ)
				cmd.cmd[i+3].inst = Asm::InstID::JNZ;
			else if (cmd.cmd[i+1].inst == Asm::InstID::SETNZ)
				cmd.cmd[i+3].inst = Asm::InstID::JZ;
			else
				continue;

			cmd.remove_cmd(i + 2);
			cmd.remove_cmd(i + 1);
		}
	}
}

void Serializer::try_merge_temp_vars() {
	return;
	for (int i=0;i<cmd.cmd.num;i++)
		if (cmd.cmd[i].inst == Asm::InstID::MOV)
			if ((cmd.cmd[i].p[0].kind == NodeKind::VarTemp) and (cmd.cmd[i].p[1].kind == NodeKind::VarTemp)) {
				int v1 = cmd.cmd[i].p[0].p;
				int v2 = cmd.cmd[i].p[1].p;
				if ((cmd.temp_var[v1].first == i) and (cmd.temp_var[v2].last == i)) {
					// swap v1 -> v2
					for (int j=i+1;j<=cmd.temp_var[v1].last;j++) {
						if (((cmd.cmd[j].p[0].kind == NodeKind::VarTemp) or (cmd.cmd[j].p[0].kind == NodeKind::DereferenceVarTemp)) and (cmd.cmd[j].p[0].p == v1))
							cmd.cmd[j].p[0].p = v2;
						if (((cmd.cmd[j].p[1].kind == NodeKind::VarTemp) or (cmd.cmd[j].p[1].kind == NodeKind::DereferenceVarTemp)) and (cmd.cmd[j].p[1].p == v1))
							cmd.cmd[j].p[1].p = v2;
					}
					cmd.temp_var[v2].last = cmd.temp_var[v1].last;
				}
				cmd.remove_cmd(i);
				cmd.remove_temp_var(v1);
			}
}

void Serializer::simplify_float_store() {
	for (int i=0;i<cmd.cmd.num - 1;i++) {
		if ((cmd.cmd[i].inst == Asm::InstID::FSTP) and (cmd.cmd[i+1].inst == Asm::InstID::MOV)) {
			if (cmd.cmd[i].p[0].kind == NodeKind::VarTemp) {
				int v = cmd.cmd[i].p[0].p;
				if ((cmd.temp_var[v].first == i) and (cmd.temp_var[v].last == i+1)) {
					cmd.cmd[i].p[0] = cmd.cmd[i+1].p[0];
					cmd.remove_cmd(i + 1);
					cmd.remove_temp_var(v);
				}
			}
		}
	}
}

void Serializer::do_error(const string &msg) {
	module->do_error_internal(msg);
}

void Serializer::do_error_link(const string &msg) {
	module->do_error_link(msg);
}

};
