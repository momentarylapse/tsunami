#if !defined(ASM_H__INCLUDED_)
#define ASM_H__INCLUDED_


namespace Asm
{

// instruction sets
enum{
	InstructionSetDefault,
	InstructionSetX86,
	InstructionSetAMD64
};

// single registers
enum{
	RegEax, RegEcx, RegEdx, RegEbx, RegEsp, RegEsi, RegEdi, RegEbp, // 4 byte
	RegAx, RegCx, RegDx, RegBx, RegBp, RegSp, RegSi, RegDi, // 2 byte
	RegAl, RegCl, RegDl, RegBl, RegAh, RegCh, RegDh, RegBh, // 1 byte
	RegCs, RegDs, RegSs, RegEs, RegFs, RegGs, // segment
	RegCr0, RegCr1, RegCr2, RegCr3,
	RegSt0, RegSt1, RegSt2, RegSt3, RegSt4, RegSt5, RegSt6, RegSt7,
	RegRax, RegRcx, RegRdx, RegRbx, RegRsp, RegRsi, RegRdi, RegRbp, // 8 byte
	NUM_REGISTERS
};

extern int RegRoot[];

enum{
	PKInvalid,
	PKNone,
	PKRegister,			// eAX
	PKDerefRegister,	// [eAX]
	PKLocal,			// [ebp + 0x0000]
	PKEdxRel,			// [edx + 0x0000]
	PKConstant32,		// 0x00000000
	PKConstant16,		// 0x0000
	PKConstant8,		// 0x00
	PKConstantDouble,   // 0x00:0x0000   ...
	PKDerefConstant,	// [0x0000]
	PKLabel				// _label
};


enum{
	inst_add,
	inst_add_b,
	inst_adc,	   // add with carry
	inst_adc_b,
	inst_sub,
	inst_sub_b,
	inst_sbb,	   // subtract with borrow
	inst_sbb_b,
	inst_inc,
	inst_inc_b,
	inst_dec,
	inst_dec_b,
	inst_mul,
	inst_mul_b,
	inst_imul,
	inst_imul_b,
	inst_div,
	inst_div_b,
	inst_idiv,
	inst_idiv_b,
	inst_mov,
	inst_mov_b,
	inst_movzx,
	inst_movsx,
	inst_and,
	inst_and_b,
	inst_or,
	inst_or_b,
	inst_xor,
	inst_xor_b,
	inst_not,
	inst_not_b,
	inst_neg,
	inst_neg_b,
	inst_pop,
	inst_popa,
	inst_push,
	inst_pusha,
	
	inst_jo,
	inst_jo_b,
	inst_jno,
	inst_jno_b,
	inst_jb,
	inst_jb_b,
	inst_jnb,
	inst_jnb_b,
	inst_jz,
	inst_jz_b,
	inst_jnz,
	inst_jnz_b,
	inst_jbe,
	inst_jbe_b,
	inst_jnbe,
	inst_jnbe_b,
	inst_js,
	inst_js_b,
	inst_jns,
	inst_jns_b,
	inst_jp,
	inst_jp_b,
	inst_jnp,
	inst_jnp_b,
	inst_jl,
	inst_jl_b,
	inst_jnl,
	inst_jnl_b,
	inst_jle,
	inst_jle_b,
	inst_jnle,
	inst_jnle_b,
	
	inst_cmp,
	inst_cmp_b,
	
	inst_seto_b,
	inst_setno_b,
	inst_setb_b,
	inst_setnb_b,
	inst_setz_b,
	inst_setnz_b,
	inst_setbe_b,
	inst_setnbe_b,
	inst_sets_b,
	inst_setns_b,
	inst_setp_b,
	inst_setnp_b,
	inst_setl_b,
	inst_setnl_b,
	inst_setle_b,
	inst_setnle_b,
	
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
	inst_test_b,
	inst_xchg,
	inst_xchg_b,
	inst_lea,
	inst_nop,
	inst_cbw_cwde,
	inst_cgq_cwd,
	inst_movs_ds_esi_es_edi,	// mov string
	inst_movs_b_ds_esi_es_edi,
	inst_cmps_ds_esi_es_edi,	// cmp string
	inst_cmps_b_ds_esi_es_edi,
	inst_rol,
	inst_rol_b,
	inst_ror,
	inst_ror_b,
	inst_rcl,
	inst_rcl_b,
	inst_rcr,
	inst_rcr_b,
	inst_shl,
	inst_shl_b,
	inst_shr,
	inst_shr_b,
	inst_sar,
	inst_sar_b,
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
	inst_in_b,
	inst_out,
	inst_out_b,
	
	inst_call,
	inst_call_far,
	inst_jmp,
	inst_jmp_b,
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
	
	NUM_INSTRUCTION_NAMES
};

string GetInstructionName(int inst);

struct GlobalVar
{
	string Name;
	void *Pos; // points into the memory of a script
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

	void add_easy(int inst, int param1_type = PKNone, void *param1 = NULL, int param2_type = PKNone, void *param2 = NULL);
	int add_label(const string &name, bool declaring);

	void AppendFromSource(const char *code);
	void ShrinkJumps(void *oc, int ocs);
	void Optimize(void *oc, int ocs);
	void Compile(void *oc, int &ocs);
	void LinkWantedLabels(void *oc);
	bool AddInstruction(char *oc, int &ocs, int n);

	Array<Label> label;
	Array<WantedLabel> wanted_label;
	int current_line;
	int current_inst;
};

void Init();
bool Assemble(const char *code, char *oc, int &ocs);
string Disassemble(void *code, int length = -1, bool allow_comments = true);
extern bool Error;
extern int ErrorLine;
extern string ErrorMessage;

bool AddInstruction(char *oc, int &ocs, int inst, int param1_type = PKNone, void *param1 = NULL, int param2_type = PKNone, void *param2 = NULL);
void SetInstructionSet(int set);
bool ImmediateAllowed(int inst);
extern int OCParam;
extern MetaInfo *CurrentMetaInfo;

void GetInstructionParamFlags(int inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write);
bool GetInstructionAllowConst(int inst);
bool GetInstructionAllowGenReg(int inst);

};

#endif
