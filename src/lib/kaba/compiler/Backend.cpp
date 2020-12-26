/*
 * Backend.cpp
 *
 *  Created on: Nov 30, 2020
 *      Author: michi
 */

#include "Backend.h"
#include "serializer.h"
#include "CommandList.h"
#include "SerialNode.h"
#include "../../file/msg.h"

namespace kaba {

Backend::Backend(Serializer *s) : cmd(s->cmd) {
	serializer = s;
	script = s->script;
	list = s->list;
	cur_func = nullptr;
	cur_func_index = -1;

	stack_max_size = 0;
	max_push_size = 0;
	stack_offset = 0;
}
Backend::~Backend() {
}


SerialNodeParam Backend::param_vreg(const Class *type, int vreg, int preg) {
	if (preg < 0)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::REGISTER, preg, vreg, type, 0};
}

SerialNodeParam Backend::param_deref_vreg(const Class *type, int vreg, int preg) {
	if (preg < 0)
		preg = cmd.virtual_reg[vreg].reg;
	return {NodeKind::DEREF_REGISTER, preg, vreg, type, 0};
}


void Backend::insert_cmd(int inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3) {
	int i = cmd.next_cmd_index;
	cmd.add_cmd(inst, p1, p2, p3);
	cmd.next_cmd_target(i + 1);
}


bool Backend::is_reg_root_used_in_interval(int reg_root, int first, int last) {
	return serializer->is_reg_root_used_in_interval(reg_root, first, last);
}

int Backend::find_unused_reg(int first, int last, int size, int exclude) {
	//vr_list_out();
	for (int r: map_reg_root)
		if (r != exclude)
			if (!is_reg_root_used_in_interval(r, first, last)) {
				return cmd.add_virtual_reg(get_reg(r, size));
			}
	serializer->cmd_list_out("fur", "find unused reg", true);
	msg_write(serializer->cur_func->long_name());
	do_error(format("no free register of size %d   in %d:%d", size, first, last));
	return -1;
}

int Backend::reg_resize(int reg, int size) {
	if (size == 2) {
		msg_error("size = 2");
		msg_write(msg_get_trace());
		throw Asm::Exception("size=2", "kjlkjl", 0, 0);
		//Asm::DoError("size=2");
	}
	return get_reg(Asm::RegRoot[reg], size);
}



int Backend::get_reg(int root, int size) {
#if 1
	if ((size != 1) and (size != 4) and (size != 8)) {
		msg_write(msg_get_trace());
		throw Asm::Exception("get_reg: bad reg size: " + i2s(size), "...", 0, 0);
	}
#endif
	return Asm::RegResize[root][size];
}

void Backend::do_error(const string &e) {
	serializer->do_error(e);
}


void Backend::add_asm_block() {
	//msg_write(".------------------------------- asm");
	SyntaxTree *ps = script->syntax;
	if (ps->asm_blocks.num == 0)
		do_error("asm block mismatch");
	ps->asm_meta_info->line_offset = ps->asm_blocks[0].line;
	list->append_from_source(ps->asm_blocks[0].block);
	ps->asm_blocks.erase(0);
}




void StackOccupationX::create(Serializer *s, bool down, int reserved, int first, int last) {
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
void StackOccupationX::set(int start, int size) {
	if (down)
		start = - start - size;

	msg_error("StackOccupationX... /32 %23... probably a typo???");

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

bool StackOccupationX::is_free(int start, int size) {
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

int StackOccupationX::find_free(int size) {
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


}
