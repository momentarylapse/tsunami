#if !defined(ASM_H__INCLUDED_)
#define ASM_H__INCLUDED_


namespace Asm
{

// instruction sets
enum{
	InstructionSetX86,
	InstructionSetAMD64
};

struct InstructionSetData
{
	int set;
	int pointer_size;
};

extern InstructionSetData InstructionSet;

// single registers
enum{
	RegEax, RegEcx, RegEdx, RegEbx, RegEsp, RegEsi, RegEdi, RegEbp, // 4 byte
	RegAx, RegCx, RegDx, RegBx, RegBp, RegSp, RegSi, RegDi, // 2 byte
	RegAl, RegCl, RegDl, RegBl, RegAh, RegCh, RegDh, RegBh, // 1 byte
	RegCs, RegDs, RegSs, RegEs, RegFs, RegGs, // segment
	RegCr0, RegCr1, RegCr2, RegCr3,
	RegSt0, RegSt1, RegSt2, RegSt3, RegSt4, RegSt5, RegSt6, RegSt7,
	RegRax, RegRcx, RegRdx, RegRbx, RegRsp, RegRsi, RegRdi, RegRbp, // 8 byte
	RegR8, RegR9, RegR10, RegR11, RegR12, RegR13, RegR14, RegR15,
	RegR8d, RegR9d, RegR10d, RegR11d, RegR12d, RegR13d, RegR14d, RegR15d,
	RegXmm0, RegXmm1, RegXmm2, RegXmm3, RegXmm4, RegXmm5, RegXmm6, RegXmm7, // 16 byte
	NUM_REGISTERS
};

const int NUM_REG_ROOTS = 32;
const int MAX_REG_SIZE = 16;

extern int RegRoot[];
extern int RegResize[NUM_REG_ROOTS][MAX_REG_SIZE + 1];
string GetRegName(int reg);

enum{
	PKInvalid,
	PKNone,
	PKRegister,			// eAX
	PKDerefRegister,	// [eAX]
	PKLocal,			// [ebp + 0x0000]
	PKEdxRel,			// [edx + 0x0000]
	PKConstant,			// 0x00000000
	PKDerefConstant,	// [0x00000000]
	PKLabel				// _label
};


enum{
	inst_add,
	inst_adc,	   // add with carry
	inst_sub,
	inst_sbb,	   // subtract with borrow
	inst_inc,
	inst_dec,
	inst_mul,
	inst_imul,
	inst_div,
	inst_idiv,
	inst_mov,
	inst_movzx,
	inst_movsx,
	inst_and,
	inst_or,
	inst_xor,
	inst_not,
	inst_neg,
	inst_pop,
	inst_popa,
	inst_push,
	inst_pusha,
	
	inst_jo,
	inst_jno,
	inst_jb,
	inst_jnb,
	inst_jz,
	inst_jnz,
	inst_jbe,
	inst_jnbe,
	inst_js,
	inst_jns,
	inst_jp,
	inst_jnp,
	inst_jl,
	inst_jnl,
	inst_jle,
	inst_jnle,
	
	inst_cmp,
	
	inst_seto,
	inst_setno,
	inst_setb,
	inst_setnb,
	inst_setz,
	inst_setnz,
	inst_setbe,
	inst_setnbe,
	inst_sets,
	inst_setns,
	inst_setp,
	inst_setnp,
	inst_setl,
	inst_setnl,
	inst_setle,
	inst_setnle,
	
	inst_sldt,
	inst_str,
	inst_lldt,
	inst_ltr,
	inst_verr,
	inst_verw,
	inst_sgdt,
	inst_sidt,
	inst_lgdt,
	inst_lidt,
	inst_smsw,
	inst_lmsw,
	
	inst_test,
	inst_xchg,
	inst_lea,
	inst_nop,
	inst_cbw_cwde,
	inst_cgq_cwd,
	inst_movs_ds_esi_es_edi,	// mov string
	inst_movs_b_ds_esi_es_edi,
	inst_cmps_ds_esi_es_edi,	// cmp string
	inst_cmps_b_ds_esi_es_edi,
	inst_rol,
	inst_ror,
	inst_rcl,
	inst_rcr,
	inst_shl,
	inst_shr,
	inst_sar,
	inst_ret,
	inst_leave,
	inst_ret_far,
	inst_int,
	inst_iret,
	
	inst_fadd,
	inst_fmul,
	inst_fsub,
	inst_fdiv,
	inst_fld,
	inst_fxch,
	inst_fst,
	inst_fstp,
	inst_fild,
	inst_faddp,
	inst_fmulp,
	inst_fsubp,
	inst_fdivp,
	inst_fldcw,
	inst_fnstcw,
	inst_fnstsw,
	inst_fistp,
	inst_fsqrt,
	inst_fsin,
	inst_fcos,
	inst_fchs,
	inst_fabs,
	inst_fucompp,
	
	inst_loop,
	inst_loope,
	inst_loopne,
	inst_in,
	inst_out,
	
	inst_call,
	inst_call_far,
	inst_jmp,
	inst_lock,
	inst_rep,
	inst_repne,
	inst_hlt,
	inst_cmc,
	inst_clc,
	inst_stc,
	inst_cli,
	inst_sti,
	inst_cld,
	inst_std,

	inst_movss,
	inst_movsd,
	
	NUM_INSTRUCTION_NAMES
};

string GetInstructionName(int inst);

struct GlobalVar
{
	string Name;
	void *Pos; // points into the memory of a script
	int Size;
};

struct Label
{
	string Name;
	int InstNo;
	int Value;
};

struct WantedLabel
{
	string Name;
	int Pos; // position to fill into     relative to CodeOrigin (Opcode[0])
	int Size; // number of bytes to fill
	int Add; // to add to the value...
	int LabelNo;
	int InstNo;
	bool Relative;
};

struct AsmData
{
	int Size; // number of bytes
	int Pos; // relative to CodeOrigin (Opcode[0])
};

struct BitChange
{
	int Pos; // relative to CodeOrigin (Opcode[0])
	int Bits;
};

struct MetaInfo
{
	long CurrentOpcodePos; // current position in the opcode buffer (including script)
	int PreInsertionLength; // size of script opcode preceding the asm block
	long CodeOrigin; // how to interpret opcode buffer[0]
	char *Opcode; // entire opcode of the script
	bool Mode16;
	int LineOffset; // number of script lines preceding asm block (to give correct error messages)

	//Array<Label> label;
	//Array<WantedLabel> wanted_label;

	Array<AsmData> data;
	Array<BitChange> bit_change;
	Array<GlobalVar> global_var;
};


struct InstructionWithParams;
struct InstructionWithParamsList : public Array<InstructionWithParams>
{
	InstructionWithParamsList(int line_offset);
	~InstructionWithParamsList();

	void add_easy(int inst, int param1_type = PKNone, int param1_size = -1, void *param1 = NULL, int param2_type = PKNone, int param2_size = -1, void *param2 = NULL);
	int add_label(const string &name, bool declaring);

	void add_func_intro(int stack_alloc_size);
	void add_func_return(int return_size);

	void AppendFromSource(const string &code);
	void ShrinkJumps(void *oc, int ocs);
	void Optimize(void *oc, int ocs);
	void Compile(void *oc, int &ocs);
	void LinkWantedLabels(void *oc);
	void AddInstruction(char *oc, int &ocs, int n);

	Array<Label> label;
	Array<WantedLabel> wanted_label;
	int current_line;
	int current_col;
	int current_inst;
};

void Init(int instruction_set = -1);
bool Assemble(const char *code, char *oc, int &ocs);
string Disassemble(void *code, int length = -1, bool allow_comments = true);

class Exception
{
public:
	Exception(const string &message, const string &expression, int line, int column);
	virtual ~Exception();
	void print() const;
	string message;
	int line, column;
};

void AddInstruction(char *oc, int &ocs, int inst, int param1_type = PKNone, int param1_size = -1, void *param1 = NULL, int param2_type = PKNone, int param2_size = -1, void *param2 = NULL);
void SetInstructionSet(int set);
bool ImmediateAllowed(int inst);
extern int OCParam;
extern MetaInfo *CurrentMetaInfo;

void GetInstructionParamFlags(int inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write);
bool GetInstructionAllowConst(int inst);
bool GetInstructionAllowGenReg(int inst);

};

#endif
