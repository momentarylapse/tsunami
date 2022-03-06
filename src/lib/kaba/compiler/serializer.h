
#pragma once

#include "CommandList.h"
#include "SerialNode.h"

namespace Asm {
	class InstructionWithParamsList;
	class InstructionParam;
	enum class RegID;
	enum class InstID;
}

namespace kaba
{

class Serializer;
class Module;
class SyntaxTree;
class Function;
class Node;
class Block;


struct LoopData {
	int label_continue, label_break;
	int level, index;
};


class Serializer {
public:
	Serializer(Module *m, Asm::InstructionWithParamsList *list);
	~Serializer();

	CommandList cmd;
	int num_labels;
	Module *module;
	SyntaxTree *syntax_tree;
	Function *cur_func;
	int cur_func_index;
	bool call_used;

	Array<LoopData> loop;

	int stack_offset, stack_max_size, max_push_size;

	Asm::InstructionWithParamsList *list;

	SerialNodeParam add_temp(const Class *t, bool add_constructor = true);

	void do_error(const string &msg);
	void do_error_link(const string &msg);

	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	Asm::InstructionParam get_param(Asm::InstID inst, SerialNodeParam &p);

	void serialize_function(Function *f);
	void serialize_block(Block *block);
	SerialNodeParam serialize_node(Node *com, Block *block, int index);

	void simplify_if_statements();
	void simplify_float_store();
	void try_merge_temp_vars();

	void cmd_list_out(const string &stage, const string &comment, bool force=false);
	void vr_list_out();



	Array<SerialNodeParam> inserted_temp;
	void add_cmd_constructor(const SerialNodeParam &param, NodeKind modus);
	void add_cmd_destructor(const SerialNodeParam &param, bool needs_ref = true);

	int temp_in_cmd(int c, int v);
	void scan_temp_var_usage();

	bool param_untouched_in_interval(SerialNodeParam &p, int first, int last);
	void simplify_fpu_stack();
	void simplify_movs();
	void remove_unused_temp_vars();

	void add_member_function_call(Function *cf, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	SerialNodeParam add_reference(const SerialNodeParam &param, const Class *force_type = nullptr);
	SerialNodeParam add_dereference(const SerialNodeParam &param, const Class *type);


	void add_stack_var(TempVar &v, SerialNodeParam &p);


	void insert_destructors_block(Block *b, bool recursive = false);
	void insert_destructors_temp();
	void insert_constructors_block(Block *b);



	SerialNodeParam p_eax, p_eax_int, p_deref_eax;
	SerialNodeParam p_rax;
	SerialNodeParam p_ax, p_al, p_al_bool, p_al_char;
	SerialNodeParam p_xmm0, p_xmm1;


	// SerializerX

	void add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_virtual_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	int function_call_push_params(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	void add_function_outro(Function *f);
	SerialNodeParam serialize_parameter(Node *link, Block *block, int index);
	void serialize_statement(Node *com, const SerialNodeParam &ret, Block *block, int index);
	void serialize_inline_function(Node *com, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);


	void fix_return_by_ref();


	SerialNodeParam param_vreg(const Class *type, int vreg, Asm::RegID preg = (Asm::RegID)-1);
	SerialNodeParam param_deref_vreg(const Class *type, int vreg, Asm::RegID preg = (Asm::RegID)-1);
};


};

