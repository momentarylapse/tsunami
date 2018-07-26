
#ifndef SERIALIZER_H_
#define SERIALIZER_H_

namespace Kaba
{


#define max_reg			8 // >= all RegXXX used...

// represents a register
// (or rather the data inside, since many VirtualRegisters might be mapped to the same physical register)
struct VirtualRegister
{
	int reg;
	int reg_root;
	int first, last;
};

// high level instructions
enum{
	INST_MARKER = 10000,
	INST_ASM,
};

struct LoopData
{
	int marker_continue, marker_break;
	int level, index;
};


struct SerialNodeParam
{
	int kind;
	int64 p;
	int virt; // virtual register (if p represents a physical register)
	Class *type;
	int shift;
	//int c_id, v_id;
	bool operator == (const SerialNodeParam &param) const
	{	return (kind == param.kind) and (p == param.p) and (type == param.type) and (shift == param.shift);	}
	string str() const;
	Class* get_type_save() const
	{	return type ? type : TypeVoid;	}
};

#define SERIAL_NODE_NUM_PARAMS	3

struct SerialNode
{
	int inst;
	int cond;
	SerialNodeParam p[SERIAL_NODE_NUM_PARAMS];
	int index;
	string str() const;
};

struct TempVar
{
	Class *type;
	int first, last, usage_count;
	bool mapped;
	bool referenced;
	bool force_stack;
	int stack_offset;
	int entangled;
	void use(int first, int last);
};

struct AddLaterData
{
	int kind, label, level, index;
};

enum{
	STUFF_KIND_MARKER,
	STUFF_KIND_JUMP,
};


class Serializer
{
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
	Node *next_node;
	int next_cmd_index;

	Array<int> map_reg_root;
	Array<VirtualRegister> virtual_reg;

	bool reg_root_used[max_reg];
	Array<LoopData> loop;

	int stack_offset, stack_max_size, max_push_size;
	Array<TempVar> temp_var;

	Array<AddLaterData> add_later;

	struct GlobalRef
	{
		int label;
		void *p;
	};
	Array<GlobalRef> global_refs;
	int add_global_ref(void *p);

	Asm::InstructionWithParamsList *list;

	void DoError(const string &msg);
	void DoErrorLink(const string &msg);

	void Assemble();
	void assemble_cmd(SerialNode &c);
	void assemble_cmd_arm(SerialNode &c);
	Asm::InstructionParam get_param(int inst, SerialNodeParam &p);

	void SerializeFunction(Function *f);
	void SerializeBlock(Block *block);
	virtual SerialNodeParam SerializeParameter(Node *link, Block *block, int index) = 0;
	SerialNodeParam SerializeNode(Node *com, Block *block, int index);
	virtual void SerializeStatement(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret, Block *block, int index, int marker_before_params) = 0;
	virtual void SerializeInlineFunction(Node *com, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) = 0;
	virtual void AddFunctionIntro(Function *f) = 0;
	virtual void AddFunctionOutro(Function *f) = 0;
	virtual void CorrectReturn(){};

	void SimplifyIfStatements();
	void SimplifyFloatStore();
	void TryMergeTempVars();

	void cmd_list_out(const string &message);
	void vr_list_out();

	//void add_reg_channel(int reg, int first, int last);
	int add_virtual_reg(int reg);
	void set_virtual_reg(int v, int first, int last);
	void use_virtual_reg(int v, int first, int last);
	SerialNodeParam add_temp(Class *t, bool add_constructor = true);
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
	int add_marker_after_command(int level, int index);
	void add_jump_after_command(int level, int index, int marker);


	Array<SerialNodeParam> inserted_temp;
	void add_cmd_constructor(const SerialNodeParam &param, int modus);
	void add_cmd_destructor(const SerialNodeParam &param, bool needs_ref = true);

	virtual void DoMapping() = 0;
	void MapReferencedTempVarsToStack();
	void TryMapTempVarsRegisters();
	void MapRemainingTempVarsToStack();

	bool is_reg_root_used_in_interval(int reg_root, int first, int last);
	void MapTempVar(int vi);
	void MapTempVars();
	void DisentangleShiftedTempVars();
	void ResolveDerefRegShift();

	int temp_in_cmd(int c, int v);
	void ScanTempVarUsage();

	int find_unused_reg(int first, int last, int size, int exclude = -1);
	void solve_deref_temp_local(int c, int np, bool is_local);
	void ResolveDerefTempAndLocal();
	bool ParamUntouchedInInterval(SerialNodeParam &p, int first, int last);
	void SimplifyFPUStack();
	void SimplifyMovs();
	void RemoveUnusedTempVars();

	void AddFunctionCall(Script *script, int func_no, const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret);
	void AddClassFunctionCall(ClassFunction *cf, const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret);
	virtual void add_function_call(Script *script, int func_no, const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) = 0;
	virtual void add_virtual_function_call(int virtual_index, const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) = 0;
	virtual int fc_begin(const SerialNodeParam &instance, const Array<SerialNodeParam> &param, const SerialNodeParam &ret) = 0;
	virtual void fc_end(int push_size, const SerialNodeParam &ret) = 0;
	SerialNodeParam AddReference(const SerialNodeParam &param, Class *force_type = nullptr);
	SerialNodeParam AddDereference(const SerialNodeParam &param, Class *force_type = nullptr);

	void MapTempVarToReg(int vi, int reg);
	void add_stack_var(TempVar &v, SerialNodeParam &p);
	void MapTempVarToStack(int vi);


	void FillInDestructorsBlock(Block *b, bool recursive = false);
	void FillInDestructorsTemp();
	void FillInConstructorsBlock(Block *b);

	void InsertAddedStuffIfNeeded(Block *b, int index);



	SerialNodeParam p_eax, p_eax_int, p_deref_eax;
	SerialNodeParam p_rax;
	SerialNodeParam p_ax, p_al, p_al_bool, p_al_char;
	SerialNodeParam p_st0, p_st1, p_xmm0, p_xmm1;
	static const SerialNodeParam p_none;


	static SerialNodeParam param_shift(const SerialNodeParam &param, int shift, Class *t);
	static SerialNodeParam param_global(Class *type, void *v);
	static SerialNodeParam param_local(Class *type, int offset);
	static SerialNodeParam param_const(Class *type, int64 c);
	static SerialNodeParam param_marker(int m);
	static SerialNodeParam param_deref_marker(Class *type, int m);
	SerialNodeParam param_vreg(Class *type, int vreg, int preg = -1);
	static SerialNodeParam param_preg(Class *type, int reg);
	SerialNodeParam param_deref_vreg(Class *type, int vreg, int preg = -1);
	static SerialNodeParam param_deref_preg(Class *type, int reg);
	static SerialNodeParam param_lookup(Class *type, int ref);
	static SerialNodeParam param_deref_lookup(Class *type, int ref);

	static int reg_resize(int reg, int size);
	void _resolve_deref_reg_shift_(SerialNodeParam &p, int i);

	static int get_reg(int root, int size);
};


};

#endif /* SERIALIZER_H_ */
