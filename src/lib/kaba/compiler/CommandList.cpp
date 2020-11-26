/*
 * CommandList.cpp
 *
 *  Created on: Nov 26, 2020
 *      Author: michi
 */

#include "CommandList.h"
#include "SerialNode.h"
#include "../kaba.h"

namespace kaba {


int CommandList::add_virtual_reg(int preg) {
	VirtualRegister c = {preg, Asm::RegRoot[preg], -1, -1};
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

	return param;
}

}
