#include "../kaba.h"
#include "serializer.h"
#include "../../file/file.h"

#include "BackendAmd64.h"
#include "BackendX86.h"
#include "BackendARM.h"
#include "../Interpreter.h"


namespace kaba {


//#define debug_evil_corrections	1

//#ifdef ScriptDebug



// return of a function might need temp vars without destructor FIXME ?!?
SerialNodeParam Serializer::add_temp(const Class *t, bool add_constructor) {
	auto r = cmd._add_temp(t);

	if (t != TypeVoid) {
		if (r.type->get_destructor())
			inserted_temp.add(r);

		if (add_constructor)
			add_cmd_constructor(r, NodeKind::VAR_TEMP);
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
	return {NodeKind::REGISTER, (int)preg, vreg, type, 0};
}

SerialNodeParam Serializer::param_deref_vreg(const Class *type, int vreg, Asm::RegID preg) {
	if (preg == Asm::RegID::INVALID)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::DEREF_REGISTER, (int)preg, vreg, type, 0};
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
		foreachi(TempVar &v, cmd.temp_var, i)
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
		ret = add_temp(type);
		cmd.add_cmd(Asm::InstID::LEA, ret, param);
		/*if (config.instruction_set == Asm::InstructionSet::ARM) {
			if (param.kind == NodeKind::VAR_LOCAL) {
				int r = find_unused_reg(-1, -1, 4);
				add_temp(type, ret);
				add_cmd(Asm::InstID::ADD, param_vreg(TypePointer, r), param_preg(TypePointer, Asm::RegID::R13), param_const(TypeInt, param.p));
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

	if (param.kind == NodeKind::VAR_TEMP) {
		return deref_temp(param, type);
	} else if (param.kind == NodeKind::REGISTER) {
		msg_write("???  register???");
		ret = param;
		ret.kind = NodeKind::DEREF_REGISTER;
		ret.type = type;// ? type : get_subtype(param.type);
		ret.shift = 0;
	} else {
		if (config.instruction_set == Asm::InstructionSet::ARM) {
			if (param.kind == NodeKind::LOCAL_ADDRESS) {
				ret = param;
				ret.kind = NodeKind::DEREF_LOCAL_MEMORY;
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
	if (n->kind == NodeKind::CALL_INLINE) {
		return (n->as_func()->inline_no == InlineID::CHUNK_ASSIGN);
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
	// for/while need a label to this point
	bool ignore_params = ((com->kind == NodeKind::BLOCK) or (com->kind == NodeKind::STATEMENT));// and ((com->link_no == STATEMENT_WHILE) or (com->link_no == STATEMENT_FOR) or (com->link_no == STATEMENT_IF) or (com->link_no == STATEMENT_IF_ELSE)));


	// EXPERIMENTAL DIRTY HACK !!!!!!!!!!!
	//syntax_tree->ShowNode(com, cur_func);
	Node *override_ret = nullptr;
#if 1
	if (node_is_assign_mem(com) /*and (config.abi == Abi::AMD64_GNU)*/) {
		auto dst = com->params[0].get();
		auto src = com->params[1].get();
		if (src->kind == NodeKind::CALL_FUNCTION or src->kind == NodeKind::CALL_INLINE) {
			if (dst->kind == NodeKind::VAR_LOCAL or dst->kind == NodeKind::VAR_GLOBAL or dst->kind == NodeKind::LOCAL_ADDRESS) {
				override_ret = dst;
				com = src;

			}
		}
	}
#endif


	// return value
	SerialNodeParam ret;
	if (override_ret) {
		ret = serialize_parameter(override_ret, block, index);
	} else {
		bool create_constructor_for_return = ((com->kind != NodeKind::STATEMENT) and (com->kind != NodeKind::CALL_FUNCTION) and (com->kind != NodeKind::CALL_VIRTUAL));
		ret = add_temp(com->type, create_constructor_for_return);
	}


	Array<SerialNodeParam> params;
	params.resize(com->params.num);

	if (!ignore_params) {

		// compile parameters
		for (int p=0;p<com->params.num;p++)
			params[p] = serialize_parameter(com->params[p].get(), block, index);

		// class function -> compile instance
		//if (com->instance)
		//	params.insert(serialize_parameter(com->instance, block, index), 0);
	}


	if (com->kind == NodeKind::CALL_FUNCTION) {
		add_function_call(com->as_func(), params, ret);
	} else if (com->kind == NodeKind::CALL_VIRTUAL) {
		add_member_function_call(com->as_func(), params, ret);
	} else if (com->kind == NodeKind::CALL_INLINE) {
		serialize_inline_function(com, params, ret);
	} else if (com->kind == NodeKind::CALL_RAW_POINTER) {
		add_pointer_call(params[0], params.sub_ref(1), ret);
	} else if (com->kind == NodeKind::STATEMENT) {
		serialize_statement(com, ret, block, index);
	} else if (com->kind == NodeKind::BLOCK) {
		serialize_block(com->as_block());
	} else if (com->kind == NodeKind::CONSTANT) {
		// sometimes "nil" is used as pass etc...
	} else {
		do_error("type of command is unimplemented: " + kind2str(com->kind));
	}

	return ret;
}

void Serializer::serialize_block(Block *block) {
	block->_label_start = list->create_label("_BLOCK_START_" + p2s(block));
	block->_label_end = list->create_label("_BLOCK_END_" + p2s(block));
	cmd.add_label(block->_label_start);

	insert_constructors_block(block);

	for (int i=0;i<block->params.num;i++) {
		stack_offset = cur_func->_var_size;

		// serialize
		serialize_node(block->params[i].get(), block, i);
		
		// destruct new temp vars
		insert_destructors_temp();

		// end of loop?
		// DEPRECATING...
		if (loop.num > 0)
			if ((loop.back().level == block->level) and (loop.back().index == i - 1))
				loop.pop();
	}

	insert_destructors_block(block);

	cmd.add_label(block->_label_end);
}

// modus: NodeKind::VAR_LOCAL / NodeKind::VAR_TEMP
//    -1: -return-/new   -> don't destruct
void Serializer::add_cmd_constructor(const SerialNodeParam &param, NodeKind modus) {
	const Class *class_type = param.type;
	if (modus == NodeKind::NONE)
		class_type = class_type->param[0];
	Function *f = class_type->get_default_constructor();
	if (!f) {
		if (class_type->needs_constructor())
			do_error("missing default constructor for " + class_type->name);
		return;
	}

	SerialNodeParam instance = param;
	if (modus == NodeKind::NONE) {
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
			add_cmd_constructor(param, (v->name == IDENTIFIER_RETURN_VAR) ? NodeKind::NONE : NodeKind::VAR_LOCAL);
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
	for (SerialNodeParam &p: inserted_temp)
		add_cmd_destructor(p);
	inserted_temp.clear();
}

int Serializer::temp_in_cmd(int c, int v) {
	if ((int)cmd.cmd[c].inst >= (int)Asm::InstID::LABEL)
		return 0;
	int r = 0;
	for (int i=0; i<SERIAL_NODE_NUM_PARAMS; i++)
		if ((cmd.cmd[c].p[i].kind == NodeKind::VAR_TEMP) or (cmd.cmd[c].p[i].kind == NodeKind::DEREF_VAR_TEMP))
			if (cmd.cmd[c].p[i].p == v)
				r += (1<<i) + ((cmd.cmd[c].p[i].kind == NodeKind::DEREF_VAR_TEMP) ? (8<<i) : 0);
	return r;
}

void Serializer::scan_temp_var_usage() {
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

inline bool param_is_simple(SerialNodeParam &p) {
	return ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::VAR_TEMP) or (p.kind == NodeKind::NONE));
}

inline bool param_combi_allowed(Asm::InstID inst, SerialNodeParam &p1, SerialNodeParam &p2) {
//	if (inst >= Asm::inst_label)
//		return true;
	if ((!param_is_simple(p1)) and (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	Asm::get_instruction_param_flags(inst, r1, w1, r2, w2);
	if ((w1) and (p1.kind == NodeKind::IMMEDIATE))
		return false;
	if ((w2) and (p2.kind == NodeKind::IMMEDIATE))
		return false;
	if ((p1.kind == NodeKind::IMMEDIATE) or (p2.kind == NodeKind::IMMEDIATE))
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
	if ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::DEREF_REGISTER)) {
		for (int i=first;i<=last;i++) {
			
			// call violates all!
			if (cmd.cmd[i].inst == Asm::InstID::CALL)
				return false;

			// div violates eax and edx
			if (cmd.cmd[i].inst == Asm::InstID::DIV)
				if ((p.as_reg() == Asm::RegID::EDX) or (p.as_reg() == Asm::RegID::EAX))
					return false;

			// registers used? (may be part of the same meta-register)
			if ((cmd.cmd[i].p[0].kind == NodeKind::REGISTER) or (cmd.cmd[i].p[0].kind == NodeKind::DEREF_REGISTER))
				if (Asm::reg_root[cmd.cmd[i].p[0].p] == Asm::reg_root[p.p])
					return false;
			if ((cmd.cmd[i].p[1].kind == NodeKind::REGISTER) or (cmd.cmd[i].p[1].kind == NodeKind::DEREF_REGISTER))
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
		if ((kind != NodeKind::LOCAL_MEMORY) and (kind != NodeKind::MEMORY) and (kind != NodeKind::VAR_TEMP) and (kind != NodeKind::DEREF_VAR_TEMP) and (kind != NodeKind::DEREF_REGISTER))
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
//	foreachi(TempVar &t, temp_var, i)
//		if (&t == &v)
//			msg_write("#" + i2s(i));
	so.create(this, (config.instruction_set != Asm::InstructionSet::ARM), cur_func->_var_size, v.first, v.last);

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


void Serializer::serialize_function(Function *f) {
	syntax_tree->create_asm_meta_info();
	syntax_tree->asm_meta_info->line_offset = 0;
	Asm::CurrentMetaInfo = syntax_tree->asm_meta_info.get();

	cur_func = f;
	num_labels = 0;
	call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;


// serialize

	// function
	serialize_block(f->block.get());
	scan_temp_var_usage();

	cmd_list_out("ser:a", "start");



	// outro (if last command != return)
	bool need_outro = true;
	if (f->block->params.num > 0)
		if ((f->block->params.back()->kind == NodeKind::STATEMENT) and (f->block->params.back()->as_statement()->id == StatementID::RETURN))
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
			if ((cmd.cmd[i].p[0].kind == NodeKind::VAR_TEMP) and (cmd.cmd[i].p[1].kind == NodeKind::VAR_TEMP)) {
				int v1 = cmd.cmd[i].p[0].p;
				int v2 = cmd.cmd[i].p[1].p;
				if ((cmd.temp_var[v1].first == i) and (cmd.temp_var[v2].last == i)) {
					// swap v1 -> v2
					for (int j=i+1;j<=cmd.temp_var[v1].last;j++) {
						if (((cmd.cmd[j].p[0].kind == NodeKind::VAR_TEMP) or (cmd.cmd[j].p[0].kind == NodeKind::DEREF_VAR_TEMP)) and (cmd.cmd[j].p[0].p == v1))
							cmd.cmd[j].p[0].p = v2;
						if (((cmd.cmd[j].p[1].kind == NodeKind::VAR_TEMP) or (cmd.cmd[j].p[1].kind == NodeKind::DEREF_VAR_TEMP)) and (cmd.cmd[j].p[1].p == v1))
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
			if (cmd.cmd[i].p[0].kind == NodeKind::VAR_TEMP) {
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


Asm::InstructionParam Serializer::get_param(Asm::InstID inst, SerialNodeParam &p) {
	if (p.kind == NodeKind::NONE) {
		return Asm::param_none;
	} else if (p.kind == NodeKind::LABEL) {
		return Asm::param_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::DEREF_LABEL) {
		return Asm::param_deref_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::REGISTER) {
		if (p.shift > 0)
			module->do_error_internal("get_param: reg + shift");
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
			module->do_error_internal("get_param: evil global of type " + p.type->name);
		return Asm::param_deref_imm(p.p + p.shift, size);
	} else if (p.kind == NodeKind::LOCAL_MEMORY) {
		if (config.instruction_set == Asm::InstructionSet::ARM) {
			return Asm::param_deref_reg_shift(Asm::RegID::R13, p.p + p.shift, p.type->size);
		} else {
			return Asm::param_deref_reg_shift(Asm::RegID::EBP, p.p + p.shift, p.type->size);
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
			module->do_error_internal("get_param: immediate + shift");
		return Asm::param_imm(p.p, p.type->size);
	} else
		module->do_error_internal("get_param: unexpected param..." + kind2str(p.kind));
	return Asm::param_none;
}


void Serializer::assemble_cmd(SerialNode &c) {
	// translate parameters
	Asm::InstructionParam p1 = get_param(c.inst, c.p[0]);
	Asm::InstructionParam p2 = get_param(c.inst, c.p[1]);

	// assemble instruction
	//list->current_line = c.
	list->add2(c.inst, p1, p2);
}

void Serializer::assemble_cmd_arm(SerialNode &c) {
	// translate parameters
	Asm::InstructionParam p1 = get_param(c.inst, c.p[0]);
	Asm::InstructionParam p2 = get_param(c.inst, c.p[1]);
	Asm::InstructionParam p3 = get_param(c.inst, c.p[2]);

	// assemble instruction
	//list->current_line = c.
	list->add_arm(c.cond, c.inst, p1, p2, p3);
}

void AddAsmBlock(Asm::InstructionWithParamsList *list, Module *s) {
	//msg_write(".------------------------------- asm");
	SyntaxTree *ps = s->syntax;
	if (ps->asm_blocks.num == 0)
		s->do_error("asm block mismatch");
	ps->asm_meta_info->line_offset = ps->asm_blocks[0].line;
	list->append_from_source(ps->asm_blocks[0].block);
	ps->asm_blocks.erase(0);
}


void Serializer::do_error(const string &msg) {
	module->do_error_internal(msg);
}

void Serializer::do_error_link(const string &msg) {
	module->do_error_link(msg);
}

Serializer::Serializer(Module *m, Asm::InstructionWithParamsList *_list) {
	module = m;
	syntax_tree = m->syntax;
	list = _list;
	max_push_size = 0;
	stack_max_size = 0;
	stack_offset = 0;
	call_used = false;
	cur_func_index = -1;
	cur_func = nullptr;
	num_labels = 0;

	cmd.ser = this;

	p_eax = param_preg(TypeReg32, Asm::RegID::EAX);
	p_eax_int = param_preg(TypeInt, Asm::RegID::EAX);
	p_rax = param_preg(TypeReg64, Asm::RegID::RAX);

	p_deref_eax = param_deref_preg(TypePointer, Asm::RegID::EAX);

	p_ax = param_preg(TypeReg16, Asm::RegID::AX);
	p_al = param_preg(TypeReg8, Asm::RegID::AL);
	p_al_bool = param_preg(TypeBool, Asm::RegID::AL);
	p_al_char = param_preg(TypeChar, Asm::RegID::AL);
	p_xmm0 = param_preg(TypeReg128, Asm::RegID::XMM0);
	p_xmm1 = param_preg(TypeReg128, Asm::RegID::XMM1);
}

bool is_func(shared<Node> n) {
	return (n->kind == NodeKind::CALL_FUNCTION or n->kind == NodeKind::CALL_VIRTUAL or n->kind == NodeKind::FUNCTION);
}

int check_needed(SyntaxTree *tree, Function *f) {
	int ref_count = 0;
	tree->transform([&](shared<Node> n){ if (is_func(n) and n->as_func() == f) ref_count ++; return n; });

	if (f->is_static() and f->name == "main")
		ref_count ++;
	if (f->virtual_index >= 0)
		ref_count ++;
	// well, for now, only allow these functions:
	if (f->name != IDENTIFIER_FUNC_ASSIGN and f->name != IDENTIFIER_FUNC_DELETE and f->name != IDENTIFIER_FUNC_INIT)
		ref_count ++;

	return ref_count;
}

Backend *create_backend(Serializer *s) {
	if (config.instruction_set == Asm::InstructionSet::AMD64)
		return new BackendAmd64(s);
	if (config.instruction_set == Asm::InstructionSet::X86)
		return new BackendX86(s);
	if (config.instruction_set == Asm::InstructionSet::ARM)
		return new BackendARM(s);
	s->module->do_error("unable to create a backend for the architecture");
	return nullptr;
}

void Module::assemble_function(int index, Function *f, Asm::InstructionWithParamsList *list) {
	if (config.verbose and config.allow_output(f, "asm"))
		msg_write("serializing " + f->long_name() + " -------------------");
	f->show("asm");


	// skip unused functions?
	if (config.remove_unused)
		if (check_needed(syntax, f) == 0)
			return;

	if (config.verbose and config.allow_output(f, "ser:0"))
		f->block->show(TypeVoid);

	if (config.interpreted) {
		auto x = new Serializer(this, list);
		x->cur_func_index = index;
		x->serialize_function(f);
		x->fix_return_by_ref();
		if (!syntax->module->interpreter)
			syntax->module->interpreter = new Interpreter(syntax->module);
		syntax->module->interpreter->add_function(f, x);
		return;
	}


	auto x = new Serializer(this, list);
	x->cur_func_index = index;
	x->serialize_function(f);
	x->fix_return_by_ref();
	auto be = create_backend(x);

	try {
		be->process(f, index);
		be->assemble();
	} catch (Exception &e) {
		throw e;
	} catch (Asm::Exception &e) {
		throw Exception(e, this, f);
	}
	functions_to_link.append(be->list->wanted_label);
	delete be;
	delete x;

}


string function_link_name(Function *f);

void function_update_address(Function *f, Asm::InstructionWithParamsList *list) {
	f->address = list->_label_value(f->_label);
	for (Block *b: f->all_blocks()) {
		b->_start = (void*)list->_label_value(b->_label_start);
		b->_end = (void*)list->_label_value(b->_label_end);
	}
}

void Module::compile_functions(char *oc, int &ocs) {
	auto *list = new Asm::InstructionWithParamsList(0);
	Array<int> func_offset;

	// link external functions
	int func_no = 0;
	for (Function *f: syntax->functions) {
		if (f->is_abstract) {
			//msg_write("SKIP COMPILE " + f->signature());
		} else if (f->is_extern()) {
			string name = function_link_name(f);
			f->address = (int_p)get_external_link(name);
			if (f->address == 0)
				f->address = (int_p)get_external_link(f->cname(f->owner()->base_class));
			if (f->address == 0)
				do_error_link(format("external function '%s' not linkable", name));
		} else {
			f->_label = list->create_label("_FUNC_" + i2s(func_no ++));
		}
	}

	// create assembler
	foreachi(Function *f, syntax->functions, i) {
		func_offset.add(list->num);
		if (!f->is_extern() and !f->is_abstract) {
			assemble_function(i, f, list);
		}
	}
	func_offset.add(list->num);


	//if (config.verbose and config.allow_output(cur_func, "comp:x"))
	//	list->show();

	// assemble into opcode
	try {
		list->optimize(oc, ocs);
		list->compile(oc, ocs);
	} catch(Asm::Exception &e) {
		list->show();
		Function *f = nullptr;
		for (int i=0; i<func_offset.num; i++)
			if (e.line >= func_offset[i] and e.line < func_offset[i+1]) {
				f = syntax->functions[i];
				break;
			}
		msg_write(f->long_name());
		throw Exception(e, this, f);
	}


	// get function addresses
	for (auto *f: syntax->functions)
		if (!f->is_extern() and !f->is_abstract)
			function_update_address(f, list);

	if (!config.interpreted)
		delete list;
}

};
