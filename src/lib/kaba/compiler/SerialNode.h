/*
 * SerialNode.h
 *
 *  Created on: Nov 26, 2020
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"

namespace Asm {
	enum class RegID;
	enum class ArmCond;
	enum class InstID;
}


namespace kaba {

class Serializer;
enum class NodeKind;
class Class;

struct SerialNodeParam {
	NodeKind kind;
	int64 p;
	int vreg; // virtual register (if p represents a physical register)
	const Class *type;
	int shift;
	//int c_id, v_id;
	bool operator == (const SerialNodeParam &param) const;
	string str(Serializer *ser) const;
	const Class* get_type_save() const;
	Asm::RegID as_reg() const;
};
extern const SerialNodeParam p_none;

#define SERIAL_NODE_NUM_PARAMS	3

struct SerialNode {
	Asm::InstID inst;
	Asm::ArmCond cond;
	SerialNodeParam p[SERIAL_NODE_NUM_PARAMS];
	int index;
	string str(Serializer *ser) const;
};

SerialNodeParam param_shift(const SerialNodeParam &param, int shift, const Class *t);
SerialNodeParam param_global(const Class *type, void *v);
SerialNodeParam param_local(const Class *type, int offset);
SerialNodeParam param_imm(const Class *type, int64 c);
SerialNodeParam param_label(const Class *type, int m);
SerialNodeParam param_label32(int m);
SerialNodeParam param_deref_label(const Class *type, int m);
SerialNodeParam param_preg(const Class *type, Asm::RegID reg);
SerialNodeParam param_deref_preg(const Class *type, Asm::RegID reg);
SerialNodeParam param_lookup(const Class *type, int ref);
SerialNodeParam param_deref_lookup(const Class *type, int ref);
SerialNodeParam deref_temp(const SerialNodeParam &param, const Class *type);

}


