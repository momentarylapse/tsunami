/*
 * SerialNode.cpp
 *
 *  Created on: Nov 26, 2020
 *      Author: michi
 */

#include "SerialNode.h"
#include "../kaba.h"
#include "Serializer.h"

namespace kaba {


const SerialNodeParam p_none = {NodeKind::None, -1, 0, nullptr, 0};


bool SerialNodeParam::operator == (const SerialNodeParam &param) const {
	return (kind == param.kind) and (p == param.p) and (type == param.type) and (shift == param.shift);
}

const Class* SerialNodeParam::get_type_save() const {
	return type ? type : TypeVoid;
}

Asm::RegID SerialNodeParam::as_reg() const {
	return (Asm::RegID)p;
}



string signed_hex(int64 i) {
	string s;
	if (i < 0) {
		s = "-";
		i = -i;
	}
	if (i < 256)
		return s + "0x" + i2h(i, 1);
	if (i < 65536)
		return s + "0x" + i2h(i, 2);
	return s + "0x" + i2h(i, 4);
}

string guess_constant(int64 c, Serializer *ser) {
	if (c != 0) {
		for (auto s: ser->syntax_tree->includes)
			for (auto *f: s->tree->functions)
				if (c == f->address)
					return "FUNC:" + f->long_name();
	}

	return "C:"+p2s((void*)c);
}

string type_name_safe(const Class *t) {
	if (!t)
		return "(NO TYPE)";
	return t->long_name();
}

string _cdecl var_repr(const void *p, const Class *type);

string guess_local_mem(int offset, Serializer *ser) {
	for (auto &v: ser->cur_func->var) {
		if (offset == v->_offset)
			return format("%s @%s", v->name, signed_hex(offset));
		if (offset >= v->_offset and offset < v->_offset + v->type->size)
			return format("%s+%d @%s", v->name, offset - v->_offset, signed_hex(offset));
	}
	// hmmm...not working...
	for (auto &v: ser->cmd.temp_var) {
		if (offset == v.stack_offset)
			return format("(tmp) @%s", signed_hex(offset));
	}
	return "? @" + signed_hex(offset);
}

string SerialNodeParam::str(Serializer *ser) const {
	string str;
	if (kind != NodeKind::None) {
		string n = p2s((void*)p);
		if ((kind == NodeKind::Register) or (kind == NodeKind::DereferenceRegister))
			n = Asm::get_reg_name(as_reg());
		else if ((kind == NodeKind::VarTemp) or (kind == NodeKind::DereferenceVarTemp))
			n = "#" + i2s(p);
		else if (kind == NodeKind::Label)
			return ser->list->label[p].name;
		else if (kind == NodeKind::LocalMemory)
			n = guess_local_mem(p + shift, ser);
		else if (kind == NodeKind::Memory)
			n = "0x" + i2h(p + shift, config.target.pointer_size);
		else if (kind == NodeKind::Immediate)
			n = guess_constant(p + shift, ser);
		else if (kind == NodeKind::VarLocal)
			n = ((Variable*)p)->name;
		else if (kind == NodeKind::VarGlobal)
			n = ((Variable*)p)->name;
		else if (kind == NodeKind::Constant)
			n = ((Constant*)p)->str();
		else if (kind == NodeKind::ConstantByAddress) {
			if (config.fully_linear_output)
				n = "@" + p2s((void*)(int_p)p);
			else
				n = var_repr((void*)(int_p)(p + shift), type) + " @" + p2s((void*)(int_p)p);
		} else if (kind == NodeKind::Function)
			n = ((Function*)p)->signature(TypeVoid);
		str = "(" + type_name_safe(type) + ") <" + kind2str(kind) + "> " + n;
		if (shift > 0)
			str += format(" + shift %d", shift);
	}
	return str;
}

string _cond_str(Asm::ArmCond cond) {
	if (cond == Asm::ArmCond::Equal)
		return "eq";
	if (cond == Asm::ArmCond::NotEqual)
		return "ne";
	if (cond == Asm::ArmCond::LessThan)
		return "lt";
	if (cond == Asm::ArmCond::LessEqual)
		return "le";
	if (cond == Asm::ArmCond::GreaterThan)
		return "gt";
	if (cond == Asm::ArmCond::GreaterEqual)
		return "ge";
	if (cond == Asm::ArmCond::CarrySet)
		return "carry";
	if (cond == Asm::ArmCond::CarryClear)
		return "nocarry";
	if (cond == Asm::ArmCond::Negative)
		return "neg";
	if (cond == Asm::ArmCond::Positive)
		return "pos";
	if (cond == Asm::ArmCond::Overflow)
		return "over";
	if (cond == Asm::ArmCond::NoOverflow)
		return "nover";
	return "?";
}

string SerialNode::str(Serializer *ser) const {
	if (inst == Asm::InstID::LABEL)
		return "-- " + ser->list->label[p[0].p].name + " --";
	if (inst == Asm::InstID::ASM)
		return format("-- Asm %d --", p[0].p);
	string t;
	if (cond != Asm::ArmCond::Always)
		t += _cond_str(cond) + ":";
	t += format("%-6s %s", Asm::get_instruction_name(inst), p[0].str(ser));
	if (p[1].kind != NodeKind::None)
		t += ",  " + p[1].str(ser);
	if (p[2].kind != NodeKind::None)
		t += ",  " + p[2].str(ser);
	return t;
}


SerialNodeParam deref_temp(const SerialNodeParam &param, const Class *type) {
	return {NodeKind::DereferenceVarTemp, param.p, -1, type, 0};
}

SerialNodeParam param_shift(const SerialNodeParam &param, int shift, const Class *t) {
	SerialNodeParam p = param;
	p.shift += shift;
	p.type = t;
	return p;
}

SerialNodeParam param_global(const Class *type, void *v) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::Memory;
	p.p = (int_p)v;
	p.shift = 0;
	return p;
}

SerialNodeParam param_local(const Class *type, int offset) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::LocalMemory;
	p.p = offset;
	p.shift = 0;
	return p;
}

SerialNodeParam param_imm(const Class *type, int64 c) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::Immediate;
	p.p = c;
	p.shift = 0;
	return p;
}

SerialNodeParam param_label(const Class *type, int m) {
	return {NodeKind::Label, m, -1, type, 0};
}

SerialNodeParam param_label32(int m) {
	return param_label(TypeInt32, m);
}

SerialNodeParam param_deref_label(const Class *type, int m) {
	return {NodeKind::DereferenceLabel, m, -1, type, 0};
}

SerialNodeParam param_preg(const Class *type, Asm::RegID reg) {
	return {NodeKind::Register, (int)reg, -1, type, 0};
}

SerialNodeParam param_deref_preg(const Class *type, Asm::RegID reg) {
	return {NodeKind::DereferenceRegister, (int)reg, -1, type, 0};
}

SerialNodeParam param_lookup(const Class *type, int ref) {
	return {NodeKind::GlobalLookup, ref, -1, /*type*/TypePointer, 0};
}

SerialNodeParam param_deref_lookup(const Class *type, int ref) {
	return {NodeKind::DereferenceGlobalLookup, ref, -1, /*type*/TypePointer, 0};
}


}
