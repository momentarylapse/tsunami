/*
 * CommandList.cpp
 *
 *  Created on: Nov 26, 2020
 *      Author: michi
 */

#include "CommandList.h"
#include "SerialNode.h"
#include "serializer.h"
#include "../kaba.h"
#include "../../file/msg.h"

namespace kaba {

TempVar::TempVar() {
	type = TypeUnknown;
	first = last = -1;
	usage_count = 0;
	mapped = false;
	referenced = false;
	force_stack = false;
	force_register = false;
	stack_offset = -1;
	entangled = 0;
}

void TempVar::use(int _first, int _last) {
	if ((first < 0) or ((first >= 0) and (first > _first)))
		first = _first;
	if ((last < 0) or ((last >= 0) and (last > _last)))
		last = _last;
}


int CommandList::add_virtual_reg(Asm::RegID preg) {
	VirtualRegister c = {preg, Asm::reg_root[(int)preg], -1, -1};
	virtual_reg.add(c);
	return virtual_reg.num - 1;
}

void CommandList::set_virtual_reg(int v, int first, int last) {
	//msg_write(format("set %d", v));
	virtual_reg[v].first = first;
	virtual_reg[v].last = last;
	//vr_list_out();
}

void CommandList::use_virtual_reg(int v, int first, int last) {
	//msg_write(format("use %d", v));
	if ((first < virtual_reg[v].first) or (virtual_reg[v].first < 0))
		virtual_reg[v].first = first;
	if ((last > virtual_reg[v].last) or (virtual_reg[v].last < 0))
		virtual_reg[v].last = last;
	//vr_list_out();
}



SerialNodeParam CommandList::_add_temp(const Class *t) {
	if (t == TypeVoid)
		return p_none;

	//msg_write("add temp " + t->name + " " + b2s(add_constructor));
	TempVar v;
	v.type = t;
	v.force_stack = (t->size > config.pointer_size) or t->is_super_array() or t->is_array() or (t->elements.num > 0);
	temp_var.add(v);
	SerialNodeParam param;
	param.kind = NodeKind::VAR_TEMP;
	param.p = temp_var.num - 1;
	param.type = t;
	param.shift = 0;

	return param;
}



void CommandList::set_cmd_param(int index, int param_index, const SerialNodeParam &p) {
	SerialNode &c = cmd[index];
	c.p[param_index] = p;
	if ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::DEREF_REGISTER))
		if (p.vreg >= 0)
			use_virtual_reg(p.vreg, index, index);
	if ((p.kind == NodeKind::VAR_TEMP) or (p.kind == NodeKind::DEREF_VAR_TEMP)) {
		int v = (int_p)p.p;
		temp_var[v].use(index, index);
		if ((c.inst == Asm::InstID::LEA) and (param_index == 1)) {
//			msg_error("ref a " + i2s(v));
			temp_var[v].referenced = true;
		}
	}
}

void CommandList::add_cmd(Asm::ArmCond cond, Asm::InstID inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3) {
	SerialNode c;
	c.inst = inst;
	c.cond = cond;
	c.index = next_cmd_index;

	if (next_cmd_index == cmd.num) {
		cmd.add(c);
	} else {
		cmd.insert(c, next_cmd_index);
		for (int i=next_cmd_index; i<cmd.num; i++)
			cmd[i].index = i;

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

	set_cmd_param(next_cmd_index, 0, p1);
	set_cmd_param(next_cmd_index, 1, p2);
	set_cmd_param(next_cmd_index, 2, p3);

	next_cmd_index = cmd.num;
}

void CommandList::add_cmd(Asm::InstID inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3) {
	add_cmd(Asm::ArmCond::ALWAYS, inst, p1, p2, p3);
}

void CommandList::add_cmd(Asm::InstID inst, const SerialNodeParam &p1, const SerialNodeParam &p2) {
	add_cmd(Asm::ArmCond::ALWAYS, inst, p1, p2, p_none);
}

void CommandList::add_cmd(Asm::InstID inst, const SerialNodeParam &p) {
	add_cmd(Asm::ArmCond::ALWAYS, inst, p, p_none, p_none);
}

void CommandList::add_cmd(Asm::InstID inst) {
	add_cmd(Asm::ArmCond::ALWAYS, inst, p_none, p_none, p_none);
}

void CommandList::next_cmd_target(int index) {
	next_cmd_index = index;
}

void CommandList::remove_cmd(int index) {
	next_cmd_index = index;
	if (cmd[index].inst == Asm::InstID::CALL) {
		for (auto &r: virtual_reg)
			if (r.first == index and r.last == index)
				r.first = r.last = -1;
	}

	cmd.erase(index);
	for (int i=index; i<cmd.num; i++)
		cmd[i].index = i;

	// adjust temp vars
	for (TempVar &v: temp_var) {
		if (v.first > index)
			v.first --;
		if (v.last >= index)
			v.last --;
	}

	// adjust reg channels
	for (VirtualRegister &r: virtual_reg) {
		if (r.first > index)
			r.first --;
		if (r.last >= index)
			r.last --;
	}
}

void CommandList::remove_temp_var(int v) {
	for (SerialNode &c: cmd) {
		for (int i=0; i<SERIAL_NODE_NUM_PARAMS; i++)
			if ((c.p[i].kind == NodeKind::VAR_TEMP) or (c.p[i].kind == NodeKind::DEREF_VAR_TEMP))
				if (c.p[i].p > v)
					c.p[i].p --;
	}
	temp_var.erase(v);
}

void CommandList::move_param(SerialNodeParam &p, int from, int to) {
	if ((p.kind == NodeKind::VAR_TEMP) or (p.kind == NodeKind::DEREF_VAR_TEMP)) {
		// move_param temp
		int64 v = p.p;
		if (temp_var[v].last < max(from, to))
			temp_var[v].last = max(from, to);
		if (temp_var[v].first > min(from, to))
			temp_var[v].first = min(from, to);
	} else if ((p.kind == NodeKind::REGISTER) or (p.kind == NodeKind::DEREF_REGISTER)) {
		// move_param reg
		auto r = Asm::reg_root[p.p];
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
			msg_error(format("move_param: no RegChannel...  reg_root=%d  from=%d", (int)r, from));
			msg_write(ser->module->filename.str() + " : " + ser->cur_func->long_name());
		}
	}
}

// l is an asm label index
int CommandList::add_label(int l) {
	SerialNodeParam p = p_none;
	if (l < 0)
		ser->do_error("trying to add non existing label");
	p.kind = NodeKind::LABEL;
	p.p = l;
	add_cmd(Asm::InstID::LABEL, p);
	return l;
}

}
