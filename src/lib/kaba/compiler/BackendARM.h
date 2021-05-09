/*
 * BackendARM.h
 *
 *  Created on: Dec 1, 2020
 *      Author: michi
 */

#pragma once

#include "../kaba.h"
#include "Backend.h"

namespace kaba {


class BackendARM : public Backend {
public:
	BackendARM(Serializer *serializer);
	~BackendARM() override;


	void process(Function *f, int index) override;

	void correct() override;
	void do_mapping() override;
	void assemble() override;
	void correct_implement_commands();
	void implement_mov_chunk(kaba::SerialNode &c, int i, int size);


	int fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static);
	void fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_function_intro_params(Function *f);
	void assemble_cmd_arm(SerialNode &c);
	Asm::InstructionParam prepare_param(Asm::InstID inst, SerialNodeParam &p);


	int _reference_to_register_32(const SerialNodeParam &p, const Class *type = nullptr);
	int add_global_ref(void *p);


	int _to_register_32(const SerialNodeParam &p, int offset, int force_register = -1);
	void _from_register_32(int reg, const SerialNodeParam &p, int offset);

	int _to_register_8(const SerialNodeParam &p, int offset, int force_register = -1);
	void _from_register_8(int reg, const SerialNodeParam &p, int offset);

	void _immediate_to_register_32(int val, int r);
	void _register_to_local_32(int r, int offset);
	void _register_to_global_32(int r, int64 addr);
	void _local_to_register_32(int offset, int r);
	void _global_to_register_32(int64 addr, int r);

	void _immediate_to_register_8(int val, int r);
	void _register_to_local_8(int r, int offset);
	void _register_to_global_8(int r, int64 addr);
	void _local_to_register_8(int offset, int r);
	void _global_to_register_8(int64 addr, int r);

	int _to_register_float(const SerialNodeParam &p, int offset, int force_register = -1);
	void _from_register_float(int reg, const SerialNodeParam &p, int offset);



	void map_remaining_temp_vars_to_stack();

	void add_function_intro_frame(int stack_alloc_size);


#if 0
	virtual void correct_implement_commands();
	virtual void implement_return(kaba::SerialNode &c, int i);
	virtual void implement_mov_chunk(kaba::SerialNode &c, int i, int size);


	/*void map();
	void assemble();

	void map_referenced_temp_vars_to_stack();*/


	//static int reg_resize(int reg, int size);
	void _resolve_deref_reg_shift_(SerialNodeParam &p, int i);

	//static int get_reg(int root, int size);


	virtual void add_function_outro(Function *f);
	virtual void add_function_intro_params(Function *f);

	void do_mapping() override;


	void map_referenced_temp_vars_to_stack();
	virtual void process_references();
	void try_map_temp_vars_to_registers();
	void map_remaining_temp_vars_to_stack();
	void resolve_deref_temp_and_local();
	void correct_unallowed_param_combis();
	void correct_unallowed_param_combis2(SerialNode &node);

	void add_stack_var(TempVar &v, SerialNodeParam &p);
	void solve_deref_temp_local(int c, int np, bool is_local);

	void assemble() override;

	Asm::InstructionParam get_param(int inst, SerialNodeParam &p);
	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	void add_function_intro_frame(int stack_alloc_size);

	void mark_regs_busy_at_call(int index);
	void extend_reg_usage_to_call(int index);
#endif
};

}

