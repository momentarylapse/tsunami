
#ifndef SERIALIZER_H_
#define SERIALIZER_H_

namespace Script
{


#define max_reg			8 // >= all RegXXX used...

struct sRegChannel
{
	int reg;
	int first, last;
};

// high level instructions
enum{
	inst_marker = 10000,
	inst_asm,
	inst_func_init,
};

struct sLoopData
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

struct sTempVar
{
	Type *type;
	int first, last, count;
	bool referenced;
	int entangled;
};

struct sStuffToAdd
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
	PreScript *pre_script;
	Function *cur_func;
	bool call_used;
	int LastCommandSize;
	Command *NextCommand;
	bool TempVarRangesDefined;

	Array<int> MapReg;
	Array<sRegChannel> RegChannel;

	bool RegUsed[max_reg];
	Array<sLoopData> LoopData;

	int StackOffset, StackMaxSize;
	Array<sTempVar> TempVar;

	Array<sStuffToAdd> StuffToAdd;

	Asm::InstructionWithParamsList *list;

	void DoError(const string &msg);
	void DoErrorLink(const string &msg);

	void Assemble(char *Opcode, int &OpcodeSize);

	void SerializeFunction(Function *f);
	void SerializeBlock(Block *block, int level);
	void SerializeParameter(Command *link, int level, int index, SerialCommandParam &param);
	SerialCommandParam SerializeCommand(Command *com, int level, int index);
	void SerializeOperator(Command *com, SerialCommandParam *param, SerialCommandParam &ret);


	void add_reg_channel(int reg, int first, int last);
	void add_temp(Type *t, SerialCommandParam &param);
	void add_cmd(int inst, SerialCommandParam p1, SerialCommandParam p2);
	void add_cmd(int inst, SerialCommandParam p);
	void add_cmd(int inst);
	void move_last_cmd(int index);
	void remove_cmd(int index);
	void remove_temp_var(int v);
	void move_param(SerialCommandParam &p, int from, int to);
	void add_marker(int m = -1);
	int add_marker_after_command(int level, int index);
	void add_jump_after_command(int level, int index, int marker);

	void add_cmd_constructor(SerialCommandParam &param, bool is_temp);
	void add_cmd_destructor(SerialCommandParam &param);

	bool is_reg_used_in_interval(int reg, int first, int last);
	void MapTempVar(int vi);
	void MapTempVars();
	void MapReferencedTempVars();
	void DisentangleShiftedTempVars();
	void ResolveDerefRegShift();

	int temp_in_cmd(int c, int v);
	void ScanTempVarUsage();
	void CorrectUnallowedParamCombis();

	int find_unused_reg(int first, int last, bool allow_eax);
	void solve_deref_temp_local(int c, int np, bool is_local);
	void ResolveDerefTempAndLocal();
	bool ParamUntouchedInInterval(SerialCommandParam &p, int first, int last);
	void SimplifyFPUStack();
	void SimplifyMovs();

	void AddFunctionCall(void *func, int func_no = -1);
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
