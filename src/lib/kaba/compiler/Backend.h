/*
 * Backend.h
 *
 *  Created on: Nov 30, 2020
 *      Author: michi
 */

#pragma once

#include "../kaba.h"
#include "serializer.h"

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

	Script *script;
	CommandList &cmd;
	Function *cur_func;
	int cur_func_index;
	Asm::InstructionWithParamsList *list;
	Serializer *serializer;


	Array<int> map_reg_root;

	virtual void do_mapping() = 0;


	bool is_reg_root_used_in_interval(int reg_root, int first, int last);
	int find_unused_reg(int first, int last, int size, int exclude = -1);
	int reg_resize(int reg, int size);

	SerialNodeParam param_vreg(const Class *type, int vreg, int preg = -1);
	SerialNodeParam param_deref_vreg(const Class *type, int vreg, int preg = -1);


	void insert_cmd(int inst, const SerialNodeParam &p1 = p_none, const SerialNodeParam &p2 = p_none, const SerialNodeParam &p3 = p_none);
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



	static int get_reg(int root, int size);

	void do_error(const string &e);


	void add_asm_block();
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

