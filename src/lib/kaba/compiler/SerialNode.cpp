/*
 * SerialNode.cpp
 *
 *  Created on: Nov 26, 2020
 *      Author: michi
 */

#include "SerialNode.h"
#include "../kaba.h"
#include "serializer.h"

namespace kaba {


const SerialNodeParam p_none = {NodeKind::NONE, -1, 0, nullptr, 0};


bool SerialNodeParam::operator == (const SerialNodeParam &param) const {
	return (kind == param.kind) and (p == param.p) and (type == param.type) and (shift == param.shift);
}

const Class* SerialNodeParam::get_type_save() const {
	return type ? type : TypeVoid;
}



SerialNodeParam deref_temp(const SerialNodeParam &param, const Class *type) {
	return {NodeKind::DEREF_VAR_TEMP, param.p, -1, type, 0};
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
	p.kind = NodeKind::MEMORY;
	p.p = (int_p)v;
	p.shift = 0;
	return p;
}

SerialNodeParam param_local(const Class *type, int offset) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::LOCAL_MEMORY;
	p.p = offset;
	p.shift = 0;
	return p;
}

SerialNodeParam param_imm(const Class *type, int64 c) {
	SerialNodeParam p;
	p.type = type;
	p.kind = NodeKind::IMMEDIATE;
	p.p = c;
	p.shift = 0;
	return p;
}

SerialNodeParam param_marker(const Class *type, int m) {
	return {NodeKind::MARKER, m, -1, type, 0};
}

SerialNodeParam param_marker32(int m) {
	return param_marker(TypeInt, m);
}

SerialNodeParam param_deref_marker(const Class *type, int m) {
	return {NodeKind::DEREF_MARKER, m, -1, type, 0};
}

SerialNodeParam param_preg(const Class *type, int reg) {
	return {NodeKind::REGISTER, reg, -1, type, 0};
}

SerialNodeParam param_deref_preg(const Class *type, int reg) {
	return {NodeKind::DEREF_REGISTER, reg, -1, type, 0};
}

SerialNodeParam param_lookup(const Class *type, int ref) {
	return {NodeKind::GLOBAL_LOOKUP, ref, -1, /*type*/TypePointer, 0};
}

SerialNodeParam param_deref_lookup(const Class *type, int ref) {
	return {NodeKind::DEREF_GLOBAL_LOOKUP, ref, -1, /*type*/TypePointer, 0};
}


}
