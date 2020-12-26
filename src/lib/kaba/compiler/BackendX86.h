/*
 * BackendAmd64.h
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#pragma once

#include "../kaba.h"
#include "Backend.h"

namespace kaba {


class BackendX86 : public Backend {
public:
	BackendX86(Serializer *serializer);
	~BackendX86() override;

	void process(Function *f, int index) override;

	void correct() override;
	void correct_parameters();

	virtual void correct_implement_commands();
	virtual void implement_return(kaba::SerialNode &c, int i);
	virtual void implement_mov_chunk(kaba::SerialNode &c, int i, int size);

	virtual int fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static);
	virtual void fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);

	/*void map();
	void assemble();

	void map_referenced_temp_vars_to_stack();*/

	SerialNodeParam p_eax, p_eax_int, p_deref_eax;
	SerialNodeParam p_rax;
	SerialNodeParam p_ax, p_al, p_al_bool, p_al_char;
	SerialNodeParam p_xmm0, p_xmm1;


	//static int reg_resize(int reg, int size);
	void _resolve_deref_reg_shift_(SerialNodeParam &p, int i);

	//static int get_reg(int root, int size);

	SerialNodeParam insert_reference(const SerialNodeParam &param, const Class *type = nullptr);


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
	void scan_temp_var_usage();
	void solve_deref_temp_local(int c, int np, bool is_local);

	void assemble() override;

	Asm::InstructionParam prepare_param(int inst, SerialNodeParam &p);
	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	void add_function_intro_frame(int stack_alloc_size);

	void mark_regs_busy_at_call(int index);
	void extend_reg_usage_to_call(int index);
};

}

