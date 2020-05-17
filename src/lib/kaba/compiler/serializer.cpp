#include "../kaba.h"
#include "serializer.h"
#include "serializer_x86.h"
#include "serializer_amd64.h"
#include "serializer_arm.h"
#include "../../file/file.h"


namespace Kaba{


//#define debug_evil_corrections	1

//#ifdef ScriptDebug

void TempVar::use(int _first, int _last) {
	if ((first < 0) or ((first >= 0) and (first > _first)))
		first = _first;
	if ((last < 0) or ((last >= 0) and (last > _last)))
		last = _last;
}

const SerialNodeParam Serializer::p_none = {NodeKind::NONE, -1, 0, nullptr, 0};


int Serializer::add_virtual_reg(int preg) {
	VirtualRegister c = {preg, Asm::RegRoot[preg], -1, -1};
	virtual_reg.add(c);
	return virtual_reg.num - 1;
}

void Serializer::set_virtual_reg(int v, int first, int last) {
	//msg_write(format("set %d", v));
	virtual_reg[v].first = first;
	virtual_reg[v].last = last;
	//vr_list_out();
}

void Serializer::use_virtual_reg(int v, int first, int last) {
	//msg_write(format("use %d", v));
	if ((first < virtual_reg[v].first) or (virtual_reg[v].first < 0))
		virtual_reg[v].first = first;
	if ((last > virtual_reg[v].last) or (virtual_reg[v].last < 0))
		virtual_reg[v].last = last;
	//vr_list_out();
}



// return of a function might need temp vars without destructor FIXME ?!?
SerialNodeParam Serializer::add_temp(const Class *t, bool add_constructor) {
	if (t == TypeVoid)
		return p_none;

	//msg_write("add temp " + t->name + " " + b2s(add_constructor));
	TempVar v;
	v.mapped = false;
	v.stack_offset = -1;
	v.first = -1;
	v.last = -1;
	v.type = t;
	v.referenced = false;
	v.force_stack = (t->size > config.pointer_size) or t->is_super_array() or t->is_array() or (t->elements.num > 0);
	v.entangled = 0;
	temp_var.add(v);
	SerialNodeParam param;
	param.kind = NodeKind::VAR_TEMP;
	param.p = temp_var.num - 1;
	param.type = t;
	param.shift = 0;

	if (param.type->get_destructor())
		inserted_temp.add(param);

	if (add_constructor)
		add_cmd_constructor(param, NodeKind::VAR_TEMP);
	return param;
}

// unpointer...?
inline const Class *get_subtype(const Class *t) {
	if (t->param)
		return t->param;
	msg_error("subtype wanted of... " + t->name);
	//msg_write(cur_func->Name);
	return TypeUnknown;
}

inline SerialNodeParam deref_temp(const SerialNodeParam &param, const Class *type) {
	SerialNodeParam deref;
	//deref = param;
	deref.kind = NodeKind::DEREF_VAR_TEMP;
	deref.p = param.p;
	deref.type = type;
	deref.shift = 0;
	return deref;
}

SerialNodeParam Serializer::param_shift(const SerialNodeParam &param, int shift, const Class *t) {
	SerialNodeParam p = param;
	p.shift += shift;
	p.type = t;
	return p;
}

SerialNodeParam Serializer::param_global(const Class *type, void *v) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::MEMORY;
	p.p = (int_p)v;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_local(const Class *type, int offset) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::LOCAL_MEMORY;
	p.p = offset;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_imm(const Class *type, int64 c) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::IMMEDIATE;
	p.p = c;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_marker(const Class *type, int m) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::MARKER;
	p.p = m;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_marker32(int m) {
	return param_marker(TypeInt, m);
}

SerialNodeParam Serializer::param_deref_marker(const Class *type, int m) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::DEREF_MARKER;
	p.p = m;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_vreg(const Class *type, int vreg, int preg) {
	SerialNodeParam p;
	p.kind = NodeKind::REGISTER;
	if (preg >= 0)
		p.p = preg;
	else
		p.p = virtual_reg[vreg].reg;
	p.virt = vreg;
	p.type = type;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_preg(const Class *type, int reg) {
	SerialNodeParam p;
	p.kind = NodeKind::REGISTER;
	p.p = reg;
	p.virt = -1;
	p.type = type;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_deref_vreg(const Class *type, int vreg, int preg) {
	SerialNodeParam p;
	p.kind = NodeKind::DEREF_REGISTER;
	if (preg >= 0)
		p.p = preg;
	else
		p.p = virtual_reg[vreg].reg;
	p.virt = vreg;
	p.type = type;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_deref_preg(const Class *type, int reg) {
	SerialNodeParam p;
	p.kind = NodeKind::DEREF_REGISTER;
	p.p = reg;
	p.virt = -1;
	p.type = type;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_lookup(const Class *type, int ref) {
	SerialNodeParam p;
	p.type = TypePointer;
	p.p = ref;
	p.kind = NodeKind::GLOBAL_LOOKUP;
	p.shift = 0;
	return p;
}

SerialNodeParam Serializer::param_deref_lookup(const Class *type, int ref) {
	SerialNodeParam p;
	p.type = TypePointer;
	p.p = ref;
	p.kind = NodeKind::DEREF_GLOBAL_LOOKUP;
	p.shift = 0;

	return p;
}

string signed_hex(int64 i) {
	string s;
	if (i < 0) {
		s = "-";
		i = -i;
	}
	if (i < 256)
		return s + d2h(&i, 1);
	if (i < 65536)
		return s + d2h(&i, 2);
	return s + d2h(&i, 4);
}


string guess_constant(int64 c, Serializer *ser) {
	if (c != 0) {
		for (auto *s: ser->syntax_tree->includes)
			for (auto *f: s->syntax->functions)
				if (c == (int_p)f->address)
					return "FUNC:" + f->long_name();
	}

	return "C:"+p2s((void*)c);
}

string type_name_safe(const Class *t) {
	if (!t)
		return "(NO TYPE)";
	return t->long_name();
}

string SerialNodeParam::str(Serializer *ser) const
{
	string str;
	if (kind != NodeKind::NONE) {
		string n = p2s((void*)p);
		if ((kind == NodeKind::REGISTER) or (kind == NodeKind::DEREF_REGISTER))
			n = Asm::get_reg_name(p);
		else if ((kind == NodeKind::VAR_TEMP) or (kind == NodeKind::DEREF_VAR_TEMP))
			n = "#" + i2s(p);
		else if (kind == NodeKind::MARKER)
			return ser->list->label[p].name;
		else if (kind == NodeKind::LOCAL_MEMORY)
			n = signed_hex(p);
		else if (kind == NodeKind::MEMORY)
			n = d2h(&p, config.pointer_size);
		else if (kind == NodeKind::IMMEDIATE)
			n = guess_constant(p, ser);
		str = "  (" + type_name_safe(type) + ") " + kind2str(kind) + " " + n;
		if (shift > 0)
			str += format(" + shift %d", shift);
	}
	return str;
}

string SerialNode::str(Serializer *ser) const
{
	if (inst == INST_MARKER)
		return "-- " + ser->list->label[p[0].p].name + " --";
	if (inst == INST_ASM)
		return format("-- Asm %d --", p[0].p);
	string t;
	if (cond != Asm::ARM_COND_ALWAYS)
		t += "[cond]";
	t += Asm::GetInstructionName(inst);
	t += p[0].str(ser);
	if (p[1].kind != NodeKind::NONE)
		t += "," + p[1].str(ser);
	if (p[2].kind != NodeKind::NONE)
		t += "," + p[2].str(ser);
	return t;
}

void Serializer::cmd_list_out(const string &stage) {
	if (!config.allow_output_stage(stage))
		return;
	msg_write("-------------------------------- " + stage);
	for (int i=0;i<cmd.num;i++)
		msg_write(format("%3d: ", i) + cmd[i].str(this));
	if (false)
		vr_list_out();
	if (true) {
		msg_write("-----------");
		foreachi(TempVar &v, temp_var, i)
			msg_write(format("  %d   %d -> %d    %s   %s", i, v.first, v.last, v.type->name.c_str(), v.referenced ? "-referenced-" : ""));
		msg_write("--------------------------------");
	}
}

void Serializer::vr_list_out() {
	msg_write("---------- vr");
	for (auto &r: virtual_reg)
		msg_write(Asm::get_reg_name(r.reg) + format("  (%d)   %d -> %d", r.reg_root, r.first, r.last));
}

int Serializer::get_reg(int root, int size) {
#if 1
	if ((size != 1) and (size != 4) and (size != 8)) {
		msg_write(msg_get_trace());
		throw Asm::Exception("get_reg: bad reg size: " + i2s(size), "...", 0, 0);
	}
#endif
	return Asm::RegResize[root][size];
}

void Serializer::set_cmd_param(SerialNode &c, int param_index, const SerialNodeParam &p) {
	c.p[param_index] = p;
	if ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::DEREF_REGISTER))
		if (p.virt >= 0)
			use_virtual_reg(p.virt, c.index, c.index);
	if ((p.kind == NodeKind::VAR_TEMP) or (p.kind == NodeKind::DEREF_VAR_TEMP)) {
		int v = (int_p)p.p;
		temp_var[v].use(c.index, c.index);
		if ((c.inst == Asm::INST_LEA) and (param_index == 1)) {
//			msg_error("ref a " + i2s(v));
			temp_var[v].referenced = true;
		}
	}
}

void Serializer::add_cmd(int cond, int inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3) {
	SerialNode c;
	c.inst = inst;
	c.cond = cond;
	c.index = next_cmd_index;

	if (next_cmd_index == cmd.num) {
		cmd.add(c);
	} else {
		for (int i=next_cmd_index; i<cmd.num; i++)
			cmd[i].index ++;
		cmd.insert(c, next_cmd_index);

		// adjust temp vars
		for (TempVar &v: temp_var) {
			if (v.first >= next_cmd_index)
				v.first ++;
			if (v.last >= next_cmd_index)
				v.last ++;
		}

		// adjust reg channels
		for (VirtualRegister &r: virtual_reg) {
			if (r.first >= next_cmd_index)
				r.first ++;
			if (r.last >= next_cmd_index)
				r.last ++;
		}
	}

	set_cmd_param(cmd[next_cmd_index], 0, p1);
	set_cmd_param(cmd[next_cmd_index], 1, p2);
	set_cmd_param(cmd[next_cmd_index], 2, p3);

	// call violates all used registers...
	if (inst == Asm::INST_CALL)
		for (int i=0;i<map_reg_root.num;i++) {
			int v = add_virtual_reg(get_reg(i, 4));
			use_virtual_reg(v, next_cmd_index, next_cmd_index);
		}

	next_cmd_index = cmd.num;
}

void Serializer::add_cmd(int inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3) {
	add_cmd(Asm::ARM_COND_ALWAYS, inst, p1, p2, p3);
}

void Serializer::add_cmd(int inst, const SerialNodeParam &p1, const SerialNodeParam &p2) {
	add_cmd(Asm::ARM_COND_ALWAYS, inst, p1, p2, p_none);
}

void Serializer::add_cmd(int inst, const SerialNodeParam &p) {
	add_cmd(Asm::ARM_COND_ALWAYS, inst, p, p_none, p_none);
}

void Serializer::add_cmd(int inst) {
	add_cmd(Asm::ARM_COND_ALWAYS, inst, p_none, p_none, p_none);
}

void Serializer::next_cmd_target(int index) {
	next_cmd_index = index;
}

void Serializer::remove_cmd(int index) {
	cmd.erase(index);

	// adjust temp vars
	for (TempVar &v: temp_var) {
		if (v.first >= index)
			v.first --;
		if (v.last >= index)
			v.last --;
	}

	// adjust reg channels
	for (VirtualRegister &r: virtual_reg) {
		if (r.first >= index)
			r.first --;
		if (r.last >= index)
			r.last --;
	}
}

void Serializer::remove_temp_var(int v) {
	for (SerialNode &c: cmd) {
		for (int i=0; i<SERIAL_NODE_NUM_PARAMS; i++)
			if ((c.p[i].kind == NodeKind::VAR_TEMP) or (c.p[i].kind == NodeKind::DEREF_VAR_TEMP))
				if (c.p[i].p > v)
					c.p[i].p --;
	}
	temp_var.erase(v);
}

void Serializer::move_param(SerialNodeParam &p, int from, int to) {
	if ((p.kind == NodeKind::VAR_TEMP) or (p.kind == NodeKind::DEREF_VAR_TEMP)) {
		// move_param temp
		int64 v = p.p;
		if (temp_var[v].last < max(from, to))
			temp_var[v].last = max(from, to);
		if (temp_var[v].first > min(from, to))
			temp_var[v].first = min(from, to);
	} else if ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::DEREF_REGISTER)) {
		// move_param reg
		int r = Asm::RegRoot[p.p];
		bool found = false;
		for (VirtualRegister &rc: virtual_reg)
			if ((r == rc.reg_root) and (from >= rc.first) and (from >= rc.first)) {
				if (rc.last < max(from, to))
					rc.last = max(from, to);
				if (rc.first > min(from, to))
					rc.first = min(from, to);
				found = true;
			}
		if (!found) {
			msg_error(format("move_param: no RegChannel...  reg_root=%d  from=%d", r, from));
			msg_write(script->filename + " : " + cur_func->long_name());
		}
	}
}

// l is an asm label index
int Serializer::add_marker(int l) {
	SerialNodeParam p = p_none;
	if (l < 0)
		do_error("trying to add non existing label");
	p.kind = NodeKind::MARKER;
	p.p = l;
	add_cmd(INST_MARKER, p);
	return l;
}

int Serializer::reg_resize(int reg, int size) {
	if (size == 2) {
		msg_error("size = 2");
		msg_write(msg_get_trace());
		throw Asm::Exception("size=2", "kjlkjl", 0, 0);
		//Asm::DoError("size=2");
	}
	return get_reg(Asm::RegRoot[reg], size);
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
		add_cmd(Asm::INST_LEA, ret, param);
		/*if (config.instruction_set == Asm::InstructionSet::ARM) {
			if (param.kind == NodeKind::VAR_LOCAL) {
				int r = find_unused_reg(-1, -1, 4);
				add_temp(type, ret);
				add_cmd(Asm::INST_ADD, param_vreg(TypePointer, r), param_preg(TypePointer, Asm::REG_R13), param_const(TypeInt, param.p));
				add_cmd(Asm::INST_MOV, ret, param_vreg(TypePointer, r));
			} else {
				DoError("reference in ARM: " + param.str());
			}
		} else {
			add_temp(type, ret);
			if (config.instruction_set == Asm::InstructionSet::AMD64) {
				int r = add_virtual_reg(Asm::REG_RAX);
				add_cmd(Asm::INST_LEA, param_vreg(TypeReg64, r), param);
				add_cmd(Asm::INST_MOV, ret, param_vreg(TypeReg64, r));
			} else {
				int r = add_virtual_reg(Asm::REG_EAX);
				add_cmd(Asm::INST_LEA, param_vreg(TypeReg32, r), param);
				add_cmd(Asm::INST_MOV, ret, param_vreg(TypeReg32, r));
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
				add_cmd(Asm::INST_MOV, temp, param);
				return deref_temp(temp, type);
			}
		} else {
			//msg_error(string("unhandled deref ", Kind2Str(param.kind)));
			SerialNodeParam temp = add_temp(param.type);
			add_cmd(Asm::INST_MOV, temp, param);
			return deref_temp(temp, type);
		}
	}
	return ret;
}


int Serializer::add_global_ref(void *p) {
	foreachi(GlobalRef &g, global_refs, i)
		if (g.p == p)
			return i;
	GlobalRef g;
	g.p = p;
	g.label = -1;
	global_refs.add(g);
	return global_refs.num - 1;
}

bool node_is_assign_mem(Node *n) {
	if (n->kind == NodeKind::INLINE_CALL) {
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
	// for/while need a marker to this point
	bool ignore_params = ((com->kind == NodeKind::BLOCK) or (com->kind == NodeKind::STATEMENT));// and ((com->link_no == STATEMENT_WHILE) or (com->link_no == STATEMENT_FOR) or (com->link_no == STATEMENT_IF) or (com->link_no == STATEMENT_IF_ELSE)));


	// EXPERIMENTAL DIRTY HACK !!!!!!!!!!!
	//syntax_tree->ShowNode(com, cur_func);
	Node *override_ret = nullptr;
#if 1
	if (node_is_assign_mem(com)) {
		Node *dst, *src;
			dst = com->params[0];
			src = com->params[1];
		if (src->kind == NodeKind::FUNCTION_CALL or src->kind == NodeKind::INLINE_CALL) {
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
		bool create_constructor_for_return = ((com->kind != NodeKind::STATEMENT) and (com->kind != NodeKind::FUNCTION_CALL) and (com->kind != NodeKind::VIRTUAL_CALL));
		ret = add_temp(com->type, create_constructor_for_return);
	}


	Array<SerialNodeParam> params;
	params.resize(com->params.num);

	if (!ignore_params) {

		// compile parameters
		for (int p=0;p<com->params.num;p++)
			params[p] = serialize_parameter(com->params[p], block, index);

		// class function -> compile instance
		//if (com->instance)
		//	params.insert(serialize_parameter(com->instance, block, index), 0);
	}


	if (com->kind == NodeKind::FUNCTION_CALL) {

		add_function_call(com->as_func(), params, ret);

	} else if (com->kind == NodeKind::VIRTUAL_CALL) {

		add_member_function_call(com->as_func(), params, ret);

	} else if (com->kind == NodeKind::INLINE_CALL) {

		serialize_inline_function(com, params, ret);

	} else if (com->kind == NodeKind::POINTER_CALL) {

		add_pointer_call(params[0], params.sub(1, -1), ret);

	} else if (com->kind == NodeKind::STATEMENT) {
		serialize_statement(com, ret, block, index);
	} else if (com->kind == NodeKind::BLOCK) {
		serialize_block(com->as_block());
	} else if (com->kind == NodeKind::CONSTANT) {
		// sometimes "nil" is used as pass etc...
	} else {
		//DoError("type of command is unimplemented: " + Kind2Str(com->kind));
	}

	return ret;
}

void Serializer::serialize_block(Block *block) {
	block->_label_start = list->create_label("_BLOCK_START_" + p2s(block));
	block->_label_end = list->create_label("_BLOCK_END_" + p2s(block));
	add_marker(block->_label_start);

	insert_constructors_block(block);

	for (int i=0;i<block->params.num;i++) {
		stack_offset = cur_func->_var_size;

		// serialize
		serialize_node(block->params[i], block, i);
		
		// destruct new temp vars
		insert_destructors_temp();

		// end of loop?
		// DEPRECATING...
		if (loop.num > 0)
			if ((loop.back().level == block->level) and (loop.back().index == i - 1))
				loop.pop();
	}

	insert_destructors_block(block);

	add_marker(block->_label_end);
}

// modus: NodeKind::VAR_LOCAL / NodeKind::VAR_TEMP
//    -1: -return-/new   -> don't destruct
void Serializer::add_cmd_constructor(const SerialNodeParam &param, NodeKind modus) {
	const Class *class_type = param.type;
	if (modus == NodeKind::NONE)
		class_type = class_type->param;
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
		Function *f = param.type->param->get_destructor();
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
	if (cmd[c].inst >= INST_MARKER)
		return 0;
	int r = 0;
	for (int i=0; i<SERIAL_NODE_NUM_PARAMS; i++)
		if ((cmd[c].p[i].kind == NodeKind::VAR_TEMP) or (cmd[c].p[i].kind == NodeKind::DEREF_VAR_TEMP))
			if (cmd[c].p[i].p == v)
				r += (1<<i) + ((cmd[c].p[i].kind == NodeKind::DEREF_VAR_TEMP) ? (8<<i) : 0);
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

inline bool param_combi_allowed(int inst, SerialNodeParam &p1, SerialNodeParam &p2) {
//	if (inst >= Asm::inst_marker)
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


int Serializer::find_unused_reg(int first, int last, int size, int exclude) {
	//vr_list_out();
	for (int r: map_reg_root)
		if (r != exclude)
			if (!is_reg_root_used_in_interval(r, first, last)) {
				return add_virtual_reg(get_reg(r, size));
			}
	cmd_list_out("fur");
	do_error(format("no free register of size %d   in %d:%d", size, first, last));
	return -1;
}

// inst ... [local] ...
// ->      mov reg, local     inst ...[reg]...
void Serializer::solve_deref_temp_local(int c, int np, bool is_local) {
	SerialNodeParam p = cmd[c].p[np];
	int shift = p.shift;

	const Class *type_pointer = is_local ? TypePointer : temp_var[p.p].type;
	const Class *type_data = p.type;
	
	p.kind = is_local ? NodeKind::LOCAL_MEMORY : NodeKind::VAR_TEMP;
	p.shift = 0;
	p.type = type_pointer;

	int reg = find_unused_reg(c, c, config.pointer_size);
	if (reg < 0)
		script->do_error_internal("solve_deref_temp_local... no registers available");
	SerialNodeParam p_reg = param_vreg(type_pointer, reg);
	SerialNodeParam p_deref_reg = param_deref_vreg(type_data, reg);
	
	set_cmd_param(cmd[c], np, p_deref_reg);

	next_cmd_target(c);
	add_cmd(Asm::INST_MOV, p_reg, p);
	if (shift > 0) {
		// solve_deref_temp_local
		next_cmd_target(c + 1);
		add_cmd(Asm::INST_ADD, p_reg, param_imm(TypeInt, shift));
		set_virtual_reg(reg, c, c+2);
	} else {
		set_virtual_reg(reg, c, c+1);
	}
}

#if 0
void ResolveDerefLocal() {
	for (int i=cmd.num-1;i>=0;i--) {
		if (cmd[i].inst >= Asm::inst_marker)
			continue;
		bool dl1 = (cmd[i].p[0].kind == NodeKind::DEREF_LOCAL_MEMORY);
		bool dl2 = (cmd[i].p[1].kind == NodeKind::DEREF_LOCAL_MEMORY);
		if (!(dl1 or dl2))
			continue;
		
		so(string2("deref local... cmd=%d", i));
		if (!dl2) {
			solve_deref_temp(i, 0);
			i ++;
		} else if (!dl1) {
			solve_deref_temp(i, 1);
			i ++;
		} else {
			// hopefully... p2 is read-only

			int reg = find_unused_reg(i, i, true);
			if (reg < 0)
				script->do_error_internal("deref local... both sides... .no registers available");
			SerialNodeParam p_reg = param_reg(TypeReg32, reg);
			add_reg_channel(reg, i, i); // temp
			
			int reg2 = find_unused_reg(i, i, true);
			if (reg2 < 0)
				script->do_error_internal("deref local... both sides... .no registers available");
			SerialNodeParam p_reg2 = param_reg(TypeReg32, reg2);
			SerialNodeParam p_deref_reg2;
			p_deref_reg2.kind = NodeKind::DEREF_REGISTER;
			p_deref_reg2.p = (char*)reg2;
			p_deref_reg2.type = TypePointer;
			p_deref_reg2.shift = 0;
			reg_channel.resize(reg_channel.num - 1); // remove temp reg channel...

			// inst [l1] [l2]
			// ->
			// mov reg2, l1
			// mov reg, [reg2]
			// mov reg2, l2
			// inst [reg2], reg
			SerialNodeParam p1 = cmd[i].p[0];
			SerialNodeParam p2 = cmd[i].p[1];
			if (p1.shift + p2.shift > 0)
				script->do_error_internal("deref local... both sides... shift");
			p1.kind = p2.kind = NodeKind::VAR_LOCAL;
			cmd[i].p[0] = p_deref_reg2;
			cmd[i].p[1] = p_reg;
	
			add_cmd(Asm::INST_MOV, p_reg2, p2);
			move_last_cmd(i);
	
			add_cmd(Asm::INST_MOV, p_reg, p_deref_reg2);
			move_last_cmd(i + 1);
	
			add_cmd(Asm::INST_MOV, p_reg2, p1);
			move_last_cmd(i + 2);

			add_reg_channel(reg, i + 1, i + 3);
			add_reg_channel(reg2, i, i + 3);
				
			i += 3;
		}
	}
}
#endif


void Serializer::resolve_deref_temp_and_local() {
	for (int i=cmd.num-1;i>=0;i--) {
		if (cmd[i].inst >= INST_MARKER)
			continue;
		bool dl1 = ((cmd[i].p[0].kind == NodeKind::DEREF_LOCAL_MEMORY) or (cmd[i].p[0].kind == NodeKind::DEREF_VAR_TEMP));
		bool dl2 = ((cmd[i].p[1].kind == NodeKind::DEREF_LOCAL_MEMORY) or (cmd[i].p[1].kind == NodeKind::DEREF_VAR_TEMP));
		if (!(dl1 or dl2))
			continue;

		bool is_local1 = (cmd[i].p[0].kind == NodeKind::DEREF_LOCAL_MEMORY);
		bool is_local2 = (cmd[i].p[1].kind == NodeKind::DEREF_LOCAL_MEMORY);
		
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
			const Class *type_data = cmd[i].p[0].type;

			int reg = find_unused_reg(i, i, type_data->size);
			if (reg < 0)
				do_error("deref local... both sides... .no registers available");
			
			SerialNodeParam p_reg = param_vreg(type_data, reg);
			
			int reg2 = find_unused_reg(i, i, config.pointer_size, virtual_reg[reg].reg_root);
			if (reg2 < 0)
				do_error("deref temp/local... both sides... .no registers available");
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
			SerialNodeParam p1 = cmd[i].p[0];
			SerialNodeParam p2 = cmd[i].p[1];
			int shift1 = p1.shift;
			int shift2 = p2.shift;
			p1.shift = p2.shift = 0;
			
			p1.kind = is_local1 ? NodeKind::LOCAL_MEMORY : NodeKind::VAR_TEMP;
			p2.kind = is_local2 ? NodeKind::LOCAL_MEMORY : NodeKind::VAR_TEMP;
			p1.type = type_pointer;
			p2.type = type_pointer;
			set_cmd_param(cmd[i], 0, p_deref_reg2);
			set_cmd_param(cmd[i], 1, p_reg);
			int cmd_pos = i;

			int r2_first = cmd_pos;
			next_cmd_target(cmd_pos ++);
			add_cmd(Asm::INST_MOV, p_reg2, p2);

			if (shift2 > 0) {
				// resolve deref temp&loc 2
				next_cmd_target(cmd_pos ++);
				add_cmd(Asm::INST_ADD, p_reg2, param_imm(TypeInt, shift2));
			}

			int r1_first = cmd_pos;
			next_cmd_target(cmd_pos ++);
			add_cmd(Asm::INST_MOV, p_reg, p_deref_reg2);

			next_cmd_target(cmd_pos ++);
			add_cmd(Asm::INST_MOV, p_reg2, p1);

			if (shift1 > 0) {
				// resolve deref temp&loc 1
				next_cmd_target(cmd_pos ++);
				add_cmd(Asm::INST_ADD, p_reg2, param_imm(TypeInt, shift1));
			}

			set_virtual_reg(reg, r1_first, cmd_pos);
			set_virtual_reg(reg2, r2_first, cmd_pos);
				
			i = cmd_pos;
		}
	}
}

bool Serializer::param_untouched_in_interval(SerialNodeParam &p, int first, int last) {
	// direct usage?
	for (int i=first;i<=last;i++)
		if ((cmd[i].p[0] == p) or (cmd[i].p[1] == p))
			return false;
	
	// registers may be more subtle..
	if ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::DEREF_REGISTER)) {
		for (int i=first;i<=last;i++) {
			
			// call violates all!
			if (cmd[i].inst == Asm::INST_CALL)
				return false;

			// div violates eax and edx
			if (cmd[i].inst == Asm::INST_DIV)
				if ((p.p == Asm::REG_EDX) or (p.p == Asm::REG_EAX))
					return false;

			// registers used? (may be part of the same meta-register)
			if ((cmd[i].p[0].kind == NodeKind::REGISTER) or (cmd[i].p[0].kind == NodeKind::DEREF_REGISTER))
				if (Asm::RegRoot[cmd[i].p[0].p] == Asm::RegRoot[p.p])
					return false;
			if ((cmd[i].p[1].kind == NodeKind::REGISTER) or (cmd[i].p[1].kind == NodeKind::DEREF_REGISTER))
				if (Asm::RegRoot[cmd[i].p[1].p] == Asm::RegRoot[p.p])
					return false;
		}
	}
	return true;
}

void Serializer::simplify_fpu_stack() {
// fstp temp
// fld temp
	for (int vi=temp_var.num-1;vi>=0;vi--) {
		TempVar &v = temp_var[vi];
		if (v.first < 0)
			continue;

		// may only appear two times
		if (v.usage_count > 2)
			continue;

		// stored then loaded...?
		if ((cmd[v.first].inst != Asm::INST_FSTP) or (cmd[v.last].inst != Asm::INST_FLD))
			continue;

		// value still on the stack?
		int d_stack = 0, min_d_stack = 0, max_d_stack = 0;
		for (int i=v.first + 1;i<v.last;i++) {
			if (cmd[i].inst == Asm::INST_FLD)
				d_stack ++;
			else if (cmd[i].inst == Asm::INST_FSTP)
				d_stack --;
			min_d_stack = min(min_d_stack, d_stack);
			max_d_stack = max(max_d_stack, d_stack);
		}
		if ((d_stack != 0) or (min_d_stack < 0) or (max_d_stack > 5))
			continue;

		// reuse value on the stack
//		msg_write(format("fpu (a)  var=%d first=%d last=%d stack: d=%d min=%d max=%d", vi, v.first, v.last, d_stack, min_d_stack, max_d_stack));
		remove_cmd(v.last);
		remove_cmd(v.first);
		remove_temp_var(vi);
	}

// fstp temp
// mov xxx, temp
	for (int vi=temp_var.num-1;vi>=0;vi--) {
		TempVar &v = temp_var[vi];
		if (v.first < 0)
			continue;

		// may only appear two times
		if (v.usage_count > 2)
			continue;

		// stored then moved...?
		if ((cmd[v.first].inst != Asm::INST_FSTP) or (cmd[v.last].inst != Asm::INST_MOV))
			continue;
		if (temp_in_cmd(v.last, vi) != 2)
			continue;
		// moved into fstore'able?
		auto kind = cmd[v.last].p[0].kind;
		if ((kind != NodeKind::LOCAL_MEMORY) and (kind != NodeKind::MEMORY) and (kind != NodeKind::VAR_TEMP) and (kind != NodeKind::DEREF_VAR_TEMP) and (kind != NodeKind::DEREF_REGISTER))
		    continue;

		// check, if mov target is used in between
		SerialNodeParam target = cmd[v.last].p[0];
		if (!param_untouched_in_interval(target, v.first + 1 ,v.last - 1))
			continue;
		// ...we are lazy...
		//if (v.last - v.first != 1)
		//	continue;

		// store directly into target
//		msg_write(format("fpu (b)  var=%d first=%d last=%d", v, v.first, v.last));
		set_cmd_param(cmd[v.first], 0, target);
		move_param(target, v.last, v.first);
		remove_cmd(v.last);
		remove_temp_var(vi);
	}
}

// remove  mov x,...    mov ...,x   (and similar)
//    (does not affect reg channels)
void Serializer::simplify_movs() {
	// TODO: count > 2 .... first == input and all_other == output?  (only if first == mov (!=eax?)... else count == 2)
	// should take care of fpu simplification (b)...

	for (int vi=temp_var.num-1;vi>=0;vi--) {
		TempVar &v = temp_var[vi];
		if (v.first < 0)
			continue;
		
		// may only appear two times
		if (v.usage_count > 2)
			continue;

		// both times in a mov command (or fld as second)
		if (cmd[v.first].inst != Asm::INST_MOV)
			continue;
		int n = cmd[v.last].inst;
		bool fld = (n == Asm::INST_FLD) or (n == Asm::INST_FADD) or (n == Asm::INST_FADD) or (n == Asm::INST_FSUB) or (n == Asm::INST_FMUL) or (n == Asm::INST_FDIV);
		if ((cmd[v.last].inst != Asm::INST_MOV) and (!fld))
			continue;
		
		// used as source/target?   no deref?
		if ((temp_in_cmd(v.first, vi) != 1) or (temp_in_cmd(v.last, vi) != (fld ? 1 : 2)))
			continue;

		// new construction allowed?
		SerialNodeParam target = cmd[v.last].p[0];
		SerialNodeParam source = cmd[v.first].p[1];
		if (fld) {
			if (!param_combi_allowed(cmd[v.last].inst, source, cmd[v.last].p[1]))
				continue;
		} else {
			if (!param_combi_allowed(cmd[v.last].inst, cmd[v.last].p[0], source))
				continue;
		}

		// check, if mov source or target are used in between
		if (!param_untouched_in_interval(target, v.first + 1 ,v.last - 1))
			continue;
		if (!param_untouched_in_interval(source, v.first + 1 ,v.last - 1))
			continue;
		
//		msg_write(format("mov simplification allowed  v=%d first=%d last=%d", vi, v.first, v.last));
		if (fld)
			set_cmd_param(cmd[v.last], 0, source);
		else
			set_cmd_param(cmd[v.last], 1, source);
		move_param(source, v.first, v.last);
		remove_cmd(v.first);
		remove_temp_var(vi);
	}

	// TODO: should happen automatically...
	//ScanTempVarUsage();
	//cmd_list_out();
}

void Serializer::remove_unused_temp_vars() {
	// unused temp vars...
	for (int v=temp_var.num-1;v>=0;v--)
		if (temp_var[v].first < 0) {
			remove_temp_var(v);
		}
}

/*inline void test_reg_usage(int c) {
	// call -> violates all...
	if (cmd[c].inst == Asm::inst_call) {
		for (int i=0;i<max_reg;i++)
			RegUsed[i] = true;
		return;
	}
	if ((cmd[c].p[0].kind == KindRegister) or (cmd[c].p[0].kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p[0].p);
	if ((cmd[c].p[1].kind == KindRegister) or (cmd[c].p[1].kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p[1].p);
}*/

void Serializer::map_temp_var_to_reg(int vi, int reg) {
	TempVar &v = temp_var[vi];
//	msg_write(format("temp=reg:  %d - %d:   tv %d := reg %d", v.first, v.last, vi, reg));
	
	SerialNodeParam p = param_vreg(v.type, reg);
	
	// map register
	for (int i=v.first;i<=v.last;i++) {
		int r = temp_in_cmd(i, vi);
		if (r & 1) {
			p.shift = cmd[i].p[0].shift;
			cmd[i].p[0] = p;
			if (r & 4)
				cmd[i].p[0].kind = NodeKind::DEREF_REGISTER;
		}
		if (r & 2) {
			p.shift = cmd[i].p[1].shift;
			cmd[i].p[1] = p;
			if (r & 8)
				cmd[i].p[1].kind = NodeKind::DEREF_REGISTER;
		}
	}
	set_virtual_reg(reg, v.first, v.last);
}

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
		for (TempVar &v: s->temp_var) {
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

void Serializer::map_temp_var_to_stack(int vi) {
	TempVar &v = temp_var[vi];
//	msg_write(format("temp=stack: %d   (%d - %d)", vi, v.first, v.last));

	SerialNodeParam p;
	add_stack_var(v, p);
	
	// map
	for (int i=v.first;i<=v.last;i++) {
		int r = temp_in_cmd(i, vi);
		if (r == 0)
			continue;

		if ((r & 3) == 3)
			script->do_error_internal("asm error: (MapTempVar) temp var on both sides of command");
		
		SerialNodeParam *p_own;
		if ((r & 1) > 0) {
			p_own = &cmd[i].p[0];
		} else {
			p_own = &cmd[i].p[1];
		}
		bool deref = (r > 3);

		p_own->kind = deref ? NodeKind::DEREF_LOCAL_MEMORY : NodeKind::LOCAL_MEMORY;
		p_own->p = p.p;
		
#if 0
		SerialNodeParam *p_own, *p_other;
		if ((r & 1) > 0) {
			p_own = &cmd[i].p[0];
			p_other = &cmd[i].p[1];
		} else {
			p_own = &cmd[i].p[1];
			p_other = &cmd[i].p[0];
		}
		bool deref = (r > 3);

		// allowed directly?
		if (!deref) {
			if (param_is_simple(*p_other)) {
				*p_own = p;
				continue;
			}
		}

		// TODO: map literally... solve unallowed combis later...?

		// is our variable used for writing... or reading?
		bool var_read = false;
		bool var_write = false;
		int c = cmd[i].inst;
		bool dummy1, dummy2;
		if ((r & 1) > 0)
			GetInstructionParamFlags(cmd[i].inst, var_read, var_write, dummy1, dummy2);
		else if ((r & 2) > 0)
			GetInstructionParamFlags(cmd[i].inst, dummy1, dummy2, var_read, var_write);

		if ((var_write) and (var_read)) { // rw
			if (deref)
				script->do_error_internal("map_stack_var (read, write, deref)");
			*p_own = p_eax;
			add_cmd(Asm::INST_MOV, p_eax, p);
			move_last_cmd(i);
			add_cmd(Asm::INST_MOV, p, p_eax);
			move_last_cmd(i+2);
			add_reg_channel(REG_EAX, i, i + 2);
			i += 2;
			
		} else if (var_write) { // write only
			if (deref) {
				//script->DoErrorInternal("map_stack_var (write, deref)");
				int shift = p_own->shift;
				*p_own = p_deref_eax;
				add_cmd(Asm::INST_MOV, p_eax, p);
				move_last_cmd(i);
				if (shift > 0) {
					add_cmd(Asm::INST_ADD, p_eax, param_imm(TypeInt, (void*)shift));
					move_last_cmd(i + 1);
					add_reg_channel(REG_EAX, i, i + 2);
				} else
					add_reg_channel(REG_EAX, i, i + 1);
			} else {
				*p_own = p_eax;
				add_cmd(Asm::INST_MOV, p, p_eax);
				move_last_cmd(i+1);
				add_reg_channel(REG_EAX, i, i + 1);
			}
			i ++;
		} else { // read only
			int shift = p_own->shift;
			*p_own = deref ? p_deref_eax : p_eax;
			add_cmd(Asm::INST_MOV, p_eax, p);
			move_last_cmd(i);
			if ((deref) and (shift > 0)) {
				add_cmd(Asm::INST_ADD, p_eax, param_imm(TypeInt, (void*)shift));
				move_last_cmd(i + 1);
				add_reg_channel(REG_EAX, i, i + 2);
			} else
				add_reg_channel(REG_EAX, i, i + 1);
			i ++;
		}
#endif
	}
}

bool Serializer::is_reg_root_used_in_interval(int reg_root, int first, int last) {
	for (int i=0;i<virtual_reg.num;i++)
		if (virtual_reg[i].reg_root == reg_root) {
			if ((virtual_reg[i].first <= last) and (virtual_reg[i].last >= first)) {
				return true;
			}
		}
	return false;
}

void Serializer::map_temp_var(int vi) {
	TempVar &v = temp_var[vi];
	int first = v.first;
	int last = v.last;
	if (first < 0)
		return;

	bool reg_allowed = true;
	for (int i=first;i<=last;i++)
		if (temp_in_cmd(i, vi))
			if (!Asm::get_instruction_allow_gen_reg(cmd[i].inst)) {
				reg_allowed = false;
				break;
			}

	int reg = -1;
	if (reg_allowed) {

		// any register not used in this interval?
		for (int i=0;i<max_reg;i++)
			reg_root_used[i] = false;
		for (int i=0;i<virtual_reg.num;i++)
			if ((virtual_reg[i].first <= last) and (virtual_reg[i].last >= first))
				reg_root_used[virtual_reg[i].reg_root] = true;
		for (int i=0;i<map_reg_root.num;i++)
			if (map_reg_root[i] != 0)
				if (!reg_root_used[map_reg_root[i]]) {
					reg = get_reg(map_reg_root[i], v.type->size);
					break;
				}
	}

	if (reg >= 0)
		map_temp_var_to_reg(vi, reg);
	else
		map_temp_var_to_stack(vi);
}

void Serializer::map_temp_vars() {
	for (int i=0;i<temp_var.num;i++)
		map_temp_var(i);
	
	//cmd_list_out();
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

// break large (unreferenced) temp vars into small (register sized) temp vars
void Serializer::disentangle_shifted_temp_vars() {
	for (int i=0;i<cmd.num;i++) {
		if ((cmd[i].p[0].kind == NodeKind::VAR_TEMP) and (cmd[i].p[0].shift > 0)) {
			temp_var[cmd[i].p[0].p].entangled = max(temp_var[cmd[i].p[0].p].entangled, cmd[i].p[0].shift);
		}
		if ((cmd[i].p[1].kind == NodeKind::VAR_TEMP) and (cmd[i].p[1].shift > 0)) {
			temp_var[cmd[i].p[1].p].entangled = max(temp_var[cmd[i].p[1].p].entangled, cmd[i].p[1].shift);
		}
	}

	for (int i=temp_var.num-1;i>=0;i--)
		if (temp_var[i].entangled > 0) {
			if (temp_var[i].referenced)
				continue;
			int n = temp_var[i].entangled / 4 + 1;
			const Class *t = temp_var[i].type;
			// entangled
			SerialNodeParam *p = new SerialNodeParam[n];

			// create small temp vars
			for (int j=0;j<n;j++) {
				const Class *tt = TypeReg32;
				// corresponding to element in a class?
				for (int k=0;k<t->elements.num;k++)
					if (t->elements[k].offset == j * 4)
						if (t->elements[k].type->size == 4)
							tt = t->elements[k].type;
				p[j] = add_temp(tt);
			}
			
			for (int j=0;j<cmd.num;j++) {
				if ((cmd[j].p[0].kind == NodeKind::VAR_TEMP) and (cmd[j].p[0].p == i))
					set_cmd_param(cmd[j], 0, p[cmd[j].p[0].shift / 4]);
				if ((cmd[j].p[1].kind == NodeKind::VAR_TEMP) and (cmd[j].p[1].p == i))
					set_cmd_param(cmd[j], 1, p[cmd[j].p[1].shift / 4]);
			}
			delete[]p;
			remove_temp_var(i);
		}

	scan_temp_var_usage();
}

void Serializer::_resolve_deref_reg_shift_(SerialNodeParam &p, int i) {
	int s = p.shift;
	p.shift = 0;
	msg_write("_resolve_deref_reg_shift_");
	int v = p.virt;
	int preg = reg_resize(p.p, 4);
	next_cmd_target(i);
	add_cmd(Asm::INST_ADD, param_vreg(TypeReg32, v, preg), param_imm(TypeInt, s));
	next_cmd_target(i + 2);
	add_cmd(Asm::INST_SUB, param_vreg(TypeReg32, v, preg), param_imm(TypeInt, s));
	use_virtual_reg(v, i, i+2);
}

// TODO....
void Serializer::resolve_deref_reg_shift() {
	for (int i=cmd.num-1;i>=0;i--) {
		if ((cmd[i].p[0].kind == NodeKind::DEREF_REGISTER) and (cmd[i].p[0].shift > 0)) {
			_resolve_deref_reg_shift_(cmd[i].p[0], i);
			continue;
		}
		if ((cmd[i].p[1].kind == NodeKind::DEREF_REGISTER) and (cmd[i].p[1].shift > 0)) {
			_resolve_deref_reg_shift_(cmd[i].p[1], i);
			continue;
		}
	}
}

void Serializer::serialize_function(Function *f) {
	syntax_tree->create_asm_meta_info();
	syntax_tree->asm_meta_info->line_offset = 0;
	Asm::CurrentMetaInfo = syntax_tree->asm_meta_info;

	cur_func = f;
	num_markers = 0;
	call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;
	next_cmd_index = 0;
	map_reg_root.clear();
	

	if (config.instruction_set == Asm::InstructionSet::ARM) {
		for (int i=0; i<10; i++)
			map_reg_root.add(i);//Asm::REG_R0+i);

	} else {
	if (config.allow_registers) {
		map_reg_root.add(0); // eax
		map_reg_root.add(1); // ecx
		map_reg_root.add(2); // edx
	//	MapRegRoot.add(3); // ebx
	//	MapRegRoot.add(6); // esi
	//	MapRegRoot.add(7); // edi
	}
	}

// serialize

	add_function_intro_params(f);

	// function
	serialize_block(f->block);
	scan_temp_var_usage();

	if (config.verbose)
		cmd_list_out("ser:a");



	// outro (if last command != return)
	bool need_outro = true;
	if (f->block->params.num > 0)
		if ((f->block->params.back()->kind == NodeKind::STATEMENT) and (f->block->params.back()->as_statement()->id == StatementID::RETURN))
			need_outro = false;
	if (need_outro)
		add_function_outro(f);

	if (config.verbose)
		cmd_list_out("ser:b");

	// map global ref labels
	if (config.instruction_set == Asm::InstructionSet::ARM) {
		// ???? might be nonsense
		foreachi(GlobalRef &g, global_refs, i)
			g.label = list->get_label(format("_kaba_ref_%d_%d", cur_func_index, i));
	}

	simplify_if_statements();
	try_merge_temp_vars();
	simplify_float_store();

	if (config.verbose)
		cmd_list_out("ser:c");
	


	//cmd_list_out();
}


void Serializer::simplify_if_statements() {
	for (int i=0;i<cmd.num - 4;i++) {
		if ((cmd[i].inst == Asm::INST_CMP) and (cmd[i+2].inst == Asm::INST_CMP) and (cmd[i+3].inst == Asm::INST_JZ)) {
			if (cmd[i+1].inst == Asm::INST_SETL)
				cmd[i+3].inst = Asm::INST_JNL;
			else if (cmd[i+1].inst == Asm::INST_SETLE)
				cmd[i+3].inst = Asm::INST_JNLE;
			else if (cmd[i+1].inst == Asm::INST_SETNL)
				cmd[i+3].inst = Asm::INST_JL;
			else if (cmd[i+1].inst == Asm::INST_SETNLE)
				cmd[i+3].inst = Asm::INST_JLE;
			else if (cmd[i+1].inst == Asm::INST_SETZ)
				cmd[i+3].inst = Asm::INST_JNZ;
			else if (cmd[i+1].inst == Asm::INST_SETNZ)
				cmd[i+3].inst = Asm::INST_JZ;
			else
				continue;

			remove_cmd(i + 2);
			remove_cmd(i + 1);
		}
	}
}

void Serializer::try_merge_temp_vars() {
	return;
	for (int i=0;i<cmd.num;i++)
		if (cmd[i].inst == Asm::INST_MOV)
			if ((cmd[i].p[0].kind == NodeKind::VAR_TEMP) and (cmd[i].p[1].kind == NodeKind::VAR_TEMP)) {
				int v1 = cmd[i].p[0].p;
				int v2 = cmd[i].p[1].p;
				if ((temp_var[v1].first == i) and (temp_var[v2].last == i)) {
					// swap v1 -> v2
					for (int j=i+1;j<=temp_var[v1].last;j++) {
						if (((cmd[j].p[0].kind == NodeKind::VAR_TEMP) or (cmd[j].p[0].kind == NodeKind::DEREF_VAR_TEMP)) and (cmd[j].p[0].p == v1))
							cmd[j].p[0].p = v2;
						if (((cmd[j].p[1].kind == NodeKind::VAR_TEMP) or (cmd[j].p[1].kind == NodeKind::DEREF_VAR_TEMP)) and (cmd[j].p[1].p == v1))
							cmd[j].p[1].p = v2;
					}
					temp_var[v2].last = temp_var[v1].last;
				}
				remove_cmd(i);
				remove_temp_var(v1);
			}
}

void Serializer::simplify_float_store() {
	for (int i=0;i<cmd.num - 1;i++) {
		if ((cmd[i].inst == Asm::INST_FSTP) and (cmd[i+1].inst == Asm::INST_MOV)) {
			if (cmd[i].p[0].kind == NodeKind::VAR_TEMP) {
				int v = cmd[i].p[0].p;
				if ((temp_var[v].first == i) and (temp_var[v].last == i+1)) {
					cmd[i].p[0] = cmd[i+1].p[0];
					remove_cmd(i + 1);
					remove_temp_var(v);
				}
			}
		}
	}
}


void Serializer::map_referenced_temp_vars_to_stack() {
	for (SerialNode &c: cmd)
		if (c.inst == Asm::INST_LEA)
			if (c.p[1].kind == NodeKind::VAR_TEMP) {
				int v = c.p[1].p;
//				msg_error("ref b " + i2s(v));
				temp_var[v].referenced = true;
				temp_var[v].force_stack = true;
			}

	for (int i=temp_var.num-1;i>=0;i--) {
		if (!temp_var[i].force_stack)
			continue;
		SerialNodeParam stackvar;
		add_stack_var(temp_var[i], stackvar);
		for (int j=0;j<cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				try_map_param_to_stack(cmd[j].p[k], i, stackvar);
		}
		remove_temp_var(i);
	}
}

void Serializer::try_map_temp_vars_to_registers() {
	for (int i=temp_var.num-1;i>=0;i--) {
		if (temp_var[i].force_stack)
			continue;
	}
}

void Serializer::map_remaining_temp_vars_to_stack() {
	for (int i=temp_var.num-1;i>=0;i--) {
		SerialNodeParam stackvar;
		add_stack_var(temp_var[i], stackvar);
		for (int j=0;j<cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				try_map_param_to_stack(cmd[j].p[k], i, stackvar);
		}
		remove_temp_var(i);
	}
}


Asm::InstructionParam Serializer::get_param(int inst, SerialNodeParam &p) {
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

void AddAsmBlock(Asm::InstructionWithParamsList *list, Script *s) {
	//msg_write(".------------------------------- asm");
	SyntaxTree *ps = s->syntax;
	if (ps->asm_blocks.num == 0)
		s->do_error("asm block mismatch");
	ps->asm_meta_info->line_offset = ps->asm_blocks[0].line;
	list->append_from_source(ps->asm_blocks[0].block);
	ps->asm_blocks.erase(0);
}

// convert    SerialNode[] cmd   into    Asm::Instruction..List list
void Serializer::assemble() {
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

	for (int i=0;i<cmd.num;i++) {

		if (cmd[i].inst == INST_MARKER) {
			list->insert_label(cmd[i].p[0].p);
		} else if (cmd[i].inst == INST_ASM) {
			AddAsmBlock(list, script);
		} else {

			if (config.instruction_set == Asm::InstructionSet::ARM)
				assemble_cmd_arm(cmd[i]);
			else
				assemble_cmd(cmd[i]);
		}
	}
	list->add2(Asm::INST_ALIGN_OPCODE);
}

void Serializer::do_error(const string &msg) {
	script->do_error_internal(msg);
}

void Serializer::do_error_link(const string &msg) {
	script->do_error_link(msg);
}

Serializer::Serializer(Script *s, Asm::InstructionWithParamsList *_list) {
	script = s;
	syntax_tree = s->syntax;
	list = _list;
	max_push_size = 0;

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

Serializer::~Serializer() {
}

Serializer *CreateSerializer(Script *s, Asm::InstructionWithParamsList *list) {
	if (config.instruction_set == Asm::InstructionSet::AMD64)
		return new SerializerAMD64(s, list);
	if (config.instruction_set == Asm::InstructionSet::X86)
		return new SerializerX86(s, list);
	if (config.instruction_set == Asm::InstructionSet::ARM)
		return new SerializerARM(s, list);
	return nullptr;
}

void Script::assemble_function(int index, Function *f, Asm::InstructionWithParamsList *list) {
	if (config.verbose and config.allow_output(cur_func, "asm"))
		msg_write("serializing " + f->long_name() + " -------------------");
	f->show("asm");

	cur_func = f;
	Serializer *d = CreateSerializer(this, list);

	try{
		d->cur_func_index = index;
		d->serialize_function(f);
		d->do_mapping();
		d->assemble();
	}catch(Exception &e) {
		throw e;
	}catch(Asm::Exception &e) {
		throw Exception(e, this, f);
	}
	functions_to_link.append(d->list->wanted_label);
	delete(d);
}

void Script::compile_functions(char *oc, int &ocs) {
	auto *list = new Asm::InstructionWithParamsList(0);
	Array<int> func_offset;

	// link external functions
	int func_no = 0;
	for (Function *f: syntax->functions) {
		if (f->is_extern()) {
			f->address = get_external_link(f->long_name() + ":" + i2s(f->num_params));
			if (!f->address)
				f->address = get_external_link(f->long_name());
			if (!f->address)
				do_error_link("external function " + f->long_name() + " not linkable");
		} else {
			f->_label = list->create_label("_FUNC_" + i2s(func_no ++));
		}
	}

	// create assembler
	foreachi(Function *f, syntax->functions, i) {
		func_offset.add(list->num);
		if (!f->is_extern()) {
			assemble_function(i, f, list);
		}
	}
	func_offset.add(list->num);


	if (config.verbose and config.allow_output(cur_func, "comp:x"))
		list->show();

	// assemble into opcode
	try{
		list->optimize(oc, ocs);
		list->compile(oc, ocs);
	}catch(Asm::Exception &e) {
		Function *f = nullptr;
		for (int i=0; i<func_offset.num; i++)
			if (e.line >= func_offset[i] and e.line < func_offset[i+1]) {
				f = syntax->functions[i];
				break;
			}
		throw Exception(e, this, f);
	}


	// get function addresses
	for (auto *f: syntax->functions)
		if (!f->is_extern()) {
			f->address = (void*)list->_label_value(f->_label);
			for (Block *b: f->all_blocks()) {
				b->_start = (void*)list->_label_value(b->_label_start);
				b->_end = (void*)list->_label_value(b->_label_end);
			}
		}

	delete(list);
}

};
