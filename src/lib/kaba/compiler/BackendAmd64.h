/*
 * BackendAmd64.h
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_COMPILER_BACKENDAMD64_H_
#define SRC_LIB_KABA_COMPILER_BACKENDAMD64_H_

#include "../kaba.h"
#include "serializer.h"

namespace kaba {

class Serializer;
class SerialNode;



struct TempVar;
struct SerialNodeParam;
struct VirtualRegister;

class BackendAmd64 {
public:
	BackendAmd64(Serializer *serializer);
	virtual ~BackendAmd64();

	void process(Function *f, int index);

	void correct();
	void correct_parameters();
	void correct_implement_commands();

	int fc_begin(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret);
	void fc_end(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);

	/*void map();
	void assemble();

	void map_referenced_temp_vars_to_stack();*/

	Script *script;
	CommandList &cmd;
	Function *cur_func;
	int cur_func_index;
	Asm::InstructionWithParamsList *list;
	Serializer *serializer;


	Array<int> map_reg_root;


	bool is_reg_root_used_in_interval(int reg_root, int first, int last);
	int find_unused_reg(int first, int last, int size, int exclude = -1);
	int reg_resize(int reg, int size);

	SerialNodeParam p_eax, p_eax_int, p_deref_eax;
	SerialNodeParam p_rax;
	SerialNodeParam p_ax, p_al, p_al_bool, p_al_char;
	SerialNodeParam p_st0, p_st1, p_xmm0, p_xmm1;


	SerialNodeParam param_vreg(const Class *type, int vreg, int preg = -1);
	SerialNodeParam param_deref_vreg(const Class *type, int vreg, int preg = -1);

	//static int reg_resize(int reg, int size);
	void _resolve_deref_reg_shift_(SerialNodeParam &p, int i);

	//static int get_reg(int root, int size);

	void insert_cmd(int inst, const SerialNodeParam &p1 = p_none, const SerialNodeParam &p2 = p_none, const SerialNodeParam &p3 = p_none);
	SerialNodeParam insert_reference(const SerialNodeParam &param, const Class *type = nullptr);


	void add_function_outro(Function *f);
	void add_function_intro_params(Function *f);

	void do_mapping();


	void map_referenced_temp_vars_to_stack();
	void process_references();
	void try_map_temp_vars_to_registers();
	void map_remaining_temp_vars_to_stack();
	void resolve_deref_temp_and_local();
	void correct_unallowed_param_combis();
	void correct_unallowed_param_combis2(SerialNode &node);

	void add_stack_var(TempVar &v, SerialNodeParam &p);
	void scan_temp_var_usage();
	void solve_deref_temp_local(int c, int np, bool is_local);

	void assemble();

	int stack_max_size;
	int max_push_size;
	int stack_offset;

	void correct_return() {}
	Asm::InstructionParam get_param(int inst, SerialNodeParam &p);
	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	void add_function_intro_frame(int stack_alloc_size);

	Array<int> func_param_virts;
	void mark_regs_busy_at_call(int index);
	void extend_reg_usage_to_call(int index);

	struct GlobalRef {
		int label;
		void *p;
	};
	Array<GlobalRef> global_refs;
	int add_global_ref(void *p);
};

}

#endif /* SRC_LIB_KABA_COMPILER_BACKENDAMD64_H_ */
