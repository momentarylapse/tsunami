
#pragma once

#include "CommandList.h"
#include "SerialNode.h"

namespace kaba
{

class Serializer;


// high level instructions
enum {
	INST_MARKER = 10000,
	INST_ASM,
};

struct LoopData {
	int marker_continue, marker_break;
	int level, index;
};


class Serializer {
public:
	Serializer(Script *script, Asm::InstructionWithParamsList *list);
	virtual ~Serializer();

	CommandList cmd;
	int num_markers;
	Script *script;
	SyntaxTree *syntax_tree;
	Function *cur_func;
	int cur_func_index;
	bool call_used;

	Array<int> map_reg_root;

	bool reg_root_used[max_reg];
	Array<LoopData> loop;

	int stack_offset, stack_max_size, max_push_size;

	struct GlobalRef {
		int label;
		void *p;
	};
	Array<GlobalRef> global_refs;
	int add_global_ref(void *p);

	Asm::InstructionWithParamsList *list;

	SerialNodeParam add_temp(const Class *t, bool add_constructor = true);

	void do_error(const string &msg);
	void do_error_link(const string &msg);

	void assemble();
	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	Asm::InstructionParam get_param(int inst, SerialNodeParam &p);

	void serialize_function(Function *f);
	void serialize_block(Block *block);
	virtual SerialNodeParam serialize_parameter(Node *link, Block *block, int index) = 0;
	SerialNodeParam serialize_node(Node *com, Block *block, int index);
	virtual void serialize_statement(Node *com, const SerialNodeParam &ret, Block *block, int index) = 0;
	virtual void serialize_inline_function(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) = 0;
	virtual void add_function_intro_params(Function *f) = 0;
	virtual void add_function_intro_frame(int stack_alloc_size) = 0;
	virtual void add_function_outro(Function *f) = 0;
	virtual void correct_return(){};

	void simplify_if_statements();
	void simplify_float_store();
	void try_merge_temp_vars();

	void cmd_list_out(const string &stage, const string &comment, bool force=false);
	void vr_list_out();



	Array<SerialNodeParam> inserted_temp;
	void add_cmd_constructor(const SerialNodeParam &param, NodeKind modus);
	void add_cmd_destructor(const SerialNodeParam &param, bool needs_ref = true);

	virtual void do_mapping() = 0;
	void map_referenced_temp_vars_to_stack();
	void try_map_temp_vars_to_registers();
	void map_remaining_temp_vars_to_stack();

	bool is_reg_root_used_in_interval(int reg_root, int first, int last);
	void map_temp_var(int vi);
	void map_temp_vars();
	void disentangle_shifted_temp_vars();
	void resolve_deref_reg_shift();

	int temp_in_cmd(int c, int v);
	void scan_temp_var_usage();

	int find_unused_reg(int first, int last, int size, int exclude = -1);
	void solve_deref_temp_local(int c, int np, bool is_local);
	void resolve_deref_temp_and_local();
	bool param_untouched_in_interval(SerialNodeParam &p, int first, int last);
	void simplify_fpu_stack();
	void simplify_movs();
	void remove_unused_temp_vars();

	void add_member_function_call(Function *cf, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual void add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual int fc_begin(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual void fc_end(int push_size, const SerialNodeParam &ret) = 0;
	SerialNodeParam add_reference(const SerialNodeParam &param, const Class *force_type = nullptr);
	SerialNodeParam add_dereference(const SerialNodeParam &param, const Class *type);


	void map_temp_var_to_reg(int vi, int reg);
	void add_stack_var(TempVar &v, SerialNodeParam &p);
	void map_temp_var_to_stack(int vi);


	void insert_destructors_block(Block *b, bool recursive = false);
	void insert_destructors_temp();
	void insert_constructors_block(Block *b);



	SerialNodeParam p_eax, p_eax_int, p_deref_eax;
	SerialNodeParam p_rax;
	SerialNodeParam p_ax, p_al, p_al_bool, p_al_char;
	SerialNodeParam p_xmm0, p_xmm1;


	SerialNodeParam param_vreg(const Class *type, int vreg, int preg = -1);
	SerialNodeParam param_deref_vreg(const Class *type, int vreg, int preg = -1);

	static int reg_resize(int reg, int size);
	void _resolve_deref_reg_shift_(SerialNodeParam &p, int i);

	static int get_reg(int root, int size);
};


};

