
#ifndef SERIALIZER_H_
#define SERIALIZER_H_

namespace Kaba
{

class Serializer;


#define max_reg			8 // >= all RegXXX used...

// represents a register
// (or rather the data inside, since many VirtualRegisters might be mapped to the same physical register)
struct VirtualRegister {
	int reg;
	int reg_root;
	int first, last;
};

// high level instructions
enum {
	INST_MARKER = 10000,
	INST_ASM,
};

struct LoopData {
	int marker_continue, marker_break;
	int level, index;
};


struct SerialNodeParam {
	NodeKind kind;
	int64 p;
	int virt; // virtual register (if p represents a physical register)
	const Class *type;
	int shift;
	//int c_id, v_id;
	bool operator == (const SerialNodeParam &param) const
	{	return (kind == param.kind) and (p == param.p) and (type == param.type) and (shift == param.shift);	}
	string str(Serializer *ser) const;
	const Class* get_type_save() const
	{	return type ? type : TypeVoid;	}
};

#define SERIAL_NODE_NUM_PARAMS	3

struct SerialNode {
	int inst;
	int cond;
	SerialNodeParam p[SERIAL_NODE_NUM_PARAMS];
	int index;
	string str(Serializer *ser) const;
};

struct TempVar {
	const Class *type;
	int first, last, usage_count;
	bool mapped;
	bool referenced;
	bool force_stack;
	int stack_offset;
	int entangled;
	void use(int first, int last);
};


class Serializer {
public:
	Serializer(Script *script, Asm::InstructionWithParamsList *list);
	virtual ~Serializer();

	Array<SerialNode> cmd;
	int num_markers;
	Script *script;
	SyntaxTree *syntax_tree;
	Function *cur_func;
	int cur_func_index;
	bool call_used;
	int next_cmd_index;

	Array<int> map_reg_root;
	Array<VirtualRegister> virtual_reg;

	bool reg_root_used[max_reg];
	Array<LoopData> loop;

	int stack_offset, stack_max_size, max_push_size;
	Array<TempVar> temp_var;

	struct GlobalRef {
		int label;
		void *p;
	};
	Array<GlobalRef> global_refs;
	int add_global_ref(void *p);

	Asm::InstructionWithParamsList *list;

	void do_error(const string &msg);
	void do_error_link(const string &msg);

	void assemble();
	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	Asm::InstructionParam get_param(int inst, SerialNodeParam &p);

	void serialize_function(Function *f);
	void serialize_block(Block *block);
	virtual SerialNodeParam SerializeParameter(Node *link, Block *block, int index) = 0;
	SerialNodeParam serialize_node(Node *com, Block *block, int index);
	virtual void SerializeStatement(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret, Block *block, int index) = 0;
	virtual void SerializeInlineFunction(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) = 0;
	virtual void AddFunctionIntro(Function *f) = 0;
	virtual void AddFunctionOutro(Function *f) = 0;
	virtual void CorrectReturn(){};

	void SimplifyIfStatements();
	void SimplifyFloatStore();
	void TryMergeTempVars();

	void cmd_list_out(const string &stage);
	void vr_list_out();

	//void add_reg_channel(int reg, int first, int last);
	int add_virtual_reg(int reg);
	void set_virtual_reg(int v, int first, int last);
	void use_virtual_reg(int v, int first, int last);
	SerialNodeParam add_temp(const Class *t, bool add_constructor = true);
	void add_cmd(int cond, int inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3);
	void add_cmd(int inst, const SerialNodeParam &p1, const SerialNodeParam &p2, const SerialNodeParam &p3);
	void add_cmd(int inst, const SerialNodeParam &p1, const SerialNodeParam &p2);
	void add_cmd(int inst, const SerialNodeParam &p);
	void add_cmd(int inst);
	void set_cmd_param(SerialNode &c, int param_index, const SerialNodeParam &p);
	void next_cmd_target(int index);
	void remove_cmd(int index);
	void remove_temp_var(int v);
	void move_param(SerialNodeParam &p, int from, int to);
	int add_marker(int m = -1);


	Array<SerialNodeParam> inserted_temp;
	void add_cmd_constructor(const SerialNodeParam &param, NodeKind modus);
	void add_cmd_destructor(const SerialNodeParam &param, bool needs_ref = true);

	virtual void DoMapping() = 0;
	void MapReferencedTempVarsToStack();
	void TryMapTempVarsRegisters();
	void MapRemainingTempVarsToStack();

	bool is_reg_root_used_in_interval(int reg_root, int first, int last);
	void MapTempVar(int vi);
	void MapTempVars();
	void DisentangleShiftedTempVars();
	void resolve_deref_reg_shift();

	int temp_in_cmd(int c, int v);
	void ScanTempVarUsage();

	int find_unused_reg(int first, int last, int size, int exclude = -1);
	void solve_deref_temp_local(int c, int np, bool is_local);
	void resolve_deref_temp_and_local();
	bool ParamUntouchedInInterval(SerialNodeParam &p, int first, int last);
	void SimplifyFPUStack();
	void SimplifyMovs();
	void RemoveUnusedTempVars();

	void AddFunctionCall(Function *f, const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret);
	void AddClassFunctionCall(Function *cf, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret);
	virtual void add_function_call(Function *f, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual void add_virtual_function_call(int virtual_index, const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual void add_pointer_call(const SerialNodeParam &pointer, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual int fc_begin(const SerialNodeParam &instance, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) = 0;
	virtual void fc_end(int push_size, const SerialNodeParam &ret) = 0;
	SerialNodeParam AddReference(const SerialNodeParam &param, const Class *force_type = nullptr);
	SerialNodeParam AddDereference(const SerialNodeParam &param, const Class *type);

	void MapTempVarToReg(int vi, int reg);
	void add_stack_var(TempVar &v, SerialNodeParam &p);
	void MapTempVarToStack(int vi);


	void FillInDestructorsBlock(Block *b, bool recursive = false);
	void FillInDestructorsTemp();
	void FillInConstructorsBlock(Block *b);



	SerialNodeParam p_eax, p_eax_int, p_deref_eax;
	SerialNodeParam p_rax;
	SerialNodeParam p_ax, p_al, p_al_bool, p_al_char;
	SerialNodeParam p_st0, p_st1, p_xmm0, p_xmm1;
	static const SerialNodeParam p_none;


	static SerialNodeParam param_shift(const SerialNodeParam &param, int shift, const Class *t);
	static SerialNodeParam param_global(const Class *type, void *v);
	static SerialNodeParam param_local(const Class *type, int offset);
	static SerialNodeParam param_imm(const Class *type, int64 c);
	static SerialNodeParam param_marker(const Class *type, int m);
	static SerialNodeParam param_marker32(int m);
	static SerialNodeParam param_deref_marker(const Class *type, int m);
	SerialNodeParam param_vreg(const Class *type, int vreg, int preg = -1);
	static SerialNodeParam param_preg(const Class *type, int reg);
	SerialNodeParam param_deref_vreg(const Class *type, int vreg, int preg = -1);
	static SerialNodeParam param_deref_preg(const Class *type, int reg);
	static SerialNodeParam param_lookup(const Class *type, int ref);
	static SerialNodeParam param_deref_lookup(const Class *type, int ref);

	static int reg_resize(int reg, int size);
	void _resolve_deref_reg_shift_(SerialNodeParam &p, int i);

	static int get_reg(int root, int size);
};


};

#endif /* SERIALIZER_H_ */
