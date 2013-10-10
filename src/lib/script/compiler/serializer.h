
#ifndef SERIALIZER_H_
#define SERIALIZER_H_

namespace Script
{


#define max_reg			8 // >= all RegXXX used...

struct RegChannel
{
	int reg_root;
	int first, last;
};

// high level instructions
enum{
	inst_marker = 10000,
	inst_asm,
	inst_func_intro,
	inst_func_outro,
	inst_call_label,
};

struct LoopData
{
	int marker_continue, marker_break;
	int level, index;
};


struct SerialCommandParam
{
	int kind;
	char *p;
	Type *type;
	int shift;
	//int c_id, v_id;
	bool operator == (const SerialCommandParam &param) const
	{	return (kind == param.kind) && (p == param.p) && (type == param.type) && (shift == param.shift);	}
};

struct SerialCommand
{
	int inst;
	SerialCommandParam p1, p2;
	int pos;
};

struct TempVar
{
	Type *type;
	int first, last, count;
	bool force_stack;
	int entangled;
};

struct AddLaterData
{
	int kind, marker, level, index;
};

enum{
	StuffKindMarker,
	StuffKindJump,
};


struct Serializer
{
	Serializer(Script *script);
	~Serializer();

	Array<SerialCommand> cmd;
	int NumMarkers;
	Script *script;
	SyntaxTree *syntax_tree;
	Function *cur_func;
	bool call_used;
	Command *NextCommand;
	bool TempVarRangesDefined;

	Array<int> MapRegRoot;
	Array<RegChannel> reg_channel;

	bool RegRootUsed[max_reg];
	Array<LoopData> loop;

	int StackOffset, StackMaxSize, MaxPushSize;
	Array<TempVar> temp_var;

	Array<AddLaterData> add_later;

	Asm::InstructionWithParamsList *list;

	void DoError(const string &msg);
	void DoErrorLink(const string &msg);

	void Assemble(char *Opcode, int &OpcodeSize);

	void SerializeFunction(Function *f);
	void SerializeBlock(Block *block, int level);
	void SerializeParameter(Command *link, int level, int index, SerialCommandParam &param);
	SerialCommandParam SerializeCommand(Command *com, int level, int index);
	void SerializeCompilerFunction(Command *com, Array<SerialCommandParam> &param, SerialCommandParam &ret, int level, int index, int marker_before_params);
	void SerializeOperator(Command *com, Array<SerialCommandParam> &param, SerialCommandParam &ret);
	void AddFunctionIntro(Function *f);

	void SimplifyIfStatements();
	void SimplifyFloatStore();
	void TryMergeTempVars();

	void cmd_list_out();

	void add_reg_channel(int reg, int first, int last);
	void add_temp(Type *t, SerialCommandParam &param, bool add_constructor = true);
	void add_cmd(int inst, SerialCommandParam p1, SerialCommandParam p2);
	void add_cmd(int inst, SerialCommandParam p);
	void add_cmd(int inst);
	void move_last_cmd(int index);
	void remove_cmd(int index);
	void remove_temp_var(int v);
	void move_param(SerialCommandParam &p, int from, int to);
	int add_marker(int m = -1);
	int add_marker_after_command(int level, int index);
	void add_jump_after_command(int level, int index, int marker);


	Array<SerialCommandParam> InsertedConstructorFunc;
	Array<SerialCommandParam> InsertedConstructorTemp;
	void add_cmd_constructor(SerialCommandParam &param, int modus);
	void add_cmd_destructor(SerialCommandParam &param, bool ref = true);

	void DoMapping();
	void FindReferencedTempVars();
	void TryMapTempVarsRegisters();
	void MapRemainingTempVarsToStack();

	bool is_reg_root_used_in_interval(int reg_root, int first, int last);
	void MapTempVar(int vi);
	void MapTempVars();
	void MapReferencedTempVars();
	void DisentangleShiftedTempVars();
	void ResolveDerefRegShift();

	int temp_in_cmd(int c, int v);
	void ScanTempVarUsage();
	void CorrectUnallowedParamCombis();

	int find_unused_reg(int first, int last, int size, bool allow_eax);
	void solve_deref_temp_local(int c, int np, bool is_local);
	void ResolveDerefTempAndLocal();
	bool ParamUntouchedInInterval(SerialCommandParam &p, int first, int last);
	void SimplifyFPUStack();
	void SimplifyMovs();
	void RemoveUnusedTempVars();

	void AddFunctionCall(Script *script, int func_no);
	void AddClassFunctionCall(ClassFunction *cf);
	void add_function_call_x86(Script *script, int func_no);
	void add_function_call_amd64(Script *script, int func_no);
	void add_virtual_function_call_x86(int virtual_index);
	void add_virtual_function_call_amd64(int virtual_index);
	int fc_x86_begin();
	void fc_x86_end(int push_size);
	int fc_amd64_begin();
	void fc_amd64_end(int push_size);
	void AddReference(SerialCommandParam &param, Type *type, SerialCommandParam &ret);
	void AddDereference(SerialCommandParam &param, SerialCommandParam &ret, Type *force_type = NULL);

	void MapTempVarToReg(int vi, int reg);
	void add_stack_var(Type *type, int first, int last, SerialCommandParam &p);
	void MapTempVarToStack(int vi);


	void FillInDestructors(bool from_temp);
	void FillInConstructorsFunc();
};

};

#endif /* SERIALIZER_H_ */
