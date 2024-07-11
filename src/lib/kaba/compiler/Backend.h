/*
 * Backend.h
 *
 *  Created on: Nov 30, 2020
 *      Author: michi
 */

#pragma once

#include "../kaba.h"
#include "CommandList.h"
#include "SerialNode.h"

namespace Asm {
	enum class RegID;
	enum class RegRoot;
	enum class InstID;
}

namespace kaba {

class Serializer;
class SerialNode;



struct TempVar;
struct SerialNodeParam;
struct VirtualRegister;

class Backend {
public:
	Backend(Serializer *serializer);
	virtual ~Backend();

	virtual void process(Function *f, int index) = 0;
	virtual void correct() = 0;

	Module *module;
	CommandList &cmd;
	Function *cur_func;
	int cur_func_index;
	Asm::InstructionWithParamsList *list;
	Serializer *serializer;


	Array<Asm::RegRoot> map_reg_root;

	virtual void do_mapping() = 0;


	bool is_reg_root_used_in_interval(Asm::RegRoot reg_root, int first, int last);
	int find_unused_reg(int first, int last, int size, Asm::RegRoot exclude = (Asm::RegRoot)-1);
	int vreg_alloc(int size, Asm::RegID preg = Asm::RegID::INVALID);
	void vreg_free(int vreg);
	Asm::RegID reg_resize(Asm::RegID reg, int size);

	SerialNodeParam param_vreg(const Class *type, int vreg, Asm::RegID preg = (Asm::RegID)-1);
	SerialNodeParam param_deref_vreg(const Class *type, int vreg, Asm::RegID preg = (Asm::RegID)-1);


	void insert_cmd(Asm::InstID inst, const SerialNodeParam &p1 = p_none, const SerialNodeParam &p2 = p_none, const SerialNodeParam &p3 = p_none);
	//SerialNodeParam insert_reference(const SerialNodeParam &param, const Class *type = nullptr);


	int stack_max_size;
	int max_push_size;
	int stack_offset;

	virtual void assemble() = 0;

	Array<int> func_param_virts;

	struct GlobalRef {
		int label;
		void *p;
	};
	Array<GlobalRef> global_refs;
	int add_global_ref(void *p);



	static Asm::RegID get_reg(Asm::RegRoot root, int size);

	void do_error(const string &e);


	void add_asm_block(int uuid);
};



struct StackOccupationX {
	Array<int> x;
	int reserved;
	bool down;

	void create(Serializer *s, bool down, int reserved, int first, int last);
	void set(int start, int size);
	bool is_free(int start, int size);
	int find_free(int size);
};


}

