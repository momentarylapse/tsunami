#if !defined(ASM_H__INCLUDED_)
#define ASM_H__INCLUDED_


namespace Asm
{

// instruction sets
enum class InstructionSet {
	UNKNOWN = -2,
	NATIVE = -1,
	X86,
	AMD64,
	ARM32,
	ARM64,
};

struct InstructionSetData {
	InstructionSet set;
	int pointer_size;
};

extern InstructionSetData instruction_set;

// single registers
enum class RegID {
	INVALID = -1,
	EAX, ECX, EDX, EBX, ESP, ESI, EDI, EBP, // 4 byte
	AX, CX, DX, BX, BP, SP, SI, DI, // 2 byte
	AL, CL, DL, BL, AH, CH, DH, BH, // 1 byte
	CS, DS, SS, ES, FS, GS, // segment
	CR0, CR1, RC2, CR3, CR4,
	ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7,
	RAX, RCX, RDX, RBX, RSP, RSI, RDI, RBP, // 8 byte
	R0, R1, R2, R3, R4, R5, R6, R7, // ARM
	R8, R9, R10, R11, R12, R13, R14, R15, // ARM 4 byte / AMD64 8 byte
	R8D, R9D, R10D, R11D, R12D, R13D, R14D, R15D,
	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, // 16 byte
	S0,  S1,  S2,  S3,  S4,  S5,  S6,  S7, // ARM float
	S8,  S9,  S10, S11, S12, S13, S14, S15,
	S16, S17, S18, S19, S20, S21, S22, S23,
	S24, S25, S26, S27, S28, S29, S30, S31,
	COUNT
};

enum class RegRoot {
	A, C, D, B, SP, SI, DI, BP,
	R0=A,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15,
	S0=32,S1,S2,S3,S4,S5,S6,S7,S8,S9,S10,S11,S12,S13,S14,S15,
	X0,X1,X2,X3,X4,X5,X6,X7,
	NONE,
	COUNT,
};

const int MAX_REG_SIZE = 16;


extern RegRoot reg_root[];
extern RegID reg_from_root[(int)RegRoot::COUNT][MAX_REG_SIZE + 1];
string get_reg_name(RegID reg);



enum class InstID {
	// data instructions
	DB,
	DW,
	DD,
	DQ,
	DS,
	DZ,
	ALIGN_OPCODE,

	ADD,
	ADC,	   // add with carry
	SUB,
	SBB,	   // subtract with borrow
	INC,
	DEC,
	MUL,
	IMUL,
	DIV,
	IDIV,
	MOV,
	MOVZX,
	MOVSX,
	MOVSXD,
	AND,
	OR,
	XOR,
	NOT,
	NEG,
	POP,
	POPA,
	PUSH,
	PUSHA,
	
	JO,
	JNO,
	JB,
	JNB,
	JZ,
	JNZ,
	JBE,
	JNBE,
	JS,
	JNS,
	JP,
	JNP,
	JL,
	JNL,
	JLE,
	JNLE,
	
	CMP,
	
	SETO,
	SETNO,
	SETB,
	SETNB,
	SETZ,
	SETNZ,
	SETBE,
	SETNBE,
	SETS,
	SETNS,
	SETP,
	SETNP,
	SETL,
	SETNL,
	SETLE,
	SETNLE,
	
	SLDT,
	STR,
	LLDT,
	LTR,
	VERR,
	VERW,
	SGDT,
	SIDT,
	LGDT,
	LIDT,
	SMSW,
	LMSW,
	
	TEST,
	XCHG,
	LEA,
	NOP,
	CBW_CWDE,
	CGQ_CWD,
	MOVS_DS_ESI_ES_EDI,	// mov string
	MOVS_B_DS_ESI_ES_EDI,
	CMPS_DS_ESI_ES_EDI,	// cmp string
	CMPS_B_DS_ESI_ES_EDI,
	ROL,
	ROR,
	RCL,
	RCR,
	SHL,
	SHR,
	SAR,
	RET,
	LEAVE,
	RET_FAR,
	INT,
	IRET,
	
	// x87
	FADD,
	FMUL,
	FSUB,
	FDIV,
	FLD,
	FLD1,
	FLDZ,
	FLDPI,
	FXCH,
	FST,
	FSTP,
	FILD,
	FADDP,
	FMULP,
	FSUBP,
	FDIVP,
	FLDCW,
	FNSTCW,
	FNSTSW,
	FISTP,
	FSQRT,
	FSIN,
	FCOS,
	FPTAN,
	FPATAN,
	FYL2X,
	FCHS,
	FABS,
	FUCOMPP,
	
	LOOP,
	LOOPE,
	LOOPNE,
	IN,
	OUT,
	
	CALL,
	CALL_FAR,
	JMP,
	JMP_FAR,
	LOCK,
	REP,
	REPNE,
	HLT,
	CMC,
	CLC,
	STC,
	CLI,
	STI,
	CLD,
	STD,

	// sse
	MOVSS,
	MOVSD,
	MOVUPS,
	MOVAPS,
	MOVLPS,
	MOVHPS,
	ADDSS,
	ADDSD,
	ADDPS,
	SUBSS,
	SUBSD,
	MULSS,
	MULSD,
	DIVSS,
	DIVSD,
	SQRTSS,
	SQRTSD,
	MINSS,
	MINSD,
	MAXSS,
	MAXSD,
	CVTSS2SD,
	CVTSD2SS,
	CVTTSS2SI,
	CVTTSD2SI,
	CVTSI2SS,
	CVTSI2SD,
	COMISS,
	COMISD,
	UCOMISS,
	UCOMISD,

	WRMSR,
	RDTSC,
	RDMSR,
	RDPMC,
	CPUID,
	LFENCE,
	MFENCE,
	SFENCE,
	CLFLUSH,

	SYSCALL,
	SYSRET,
	SYSENTER,
	SYSEXIT,

	
	// ARM
	B,
	BL,
	BLX,

	MULS,
	ADDS,
	SUBS,
	RSBS,
	ADCS,
	SBCS,
	RSCS,
	ANDS,
	BICS,
	XORS,
	ORS,
	MOVS,
	MVNS,

	LDR,
	LDRB,
//	STR,
	STRB,

	LDMIA,
	LDMIB,
	LDMDA,
	LDMDB,
	STMIA,
	STMIB,
	STMDA,
	STMDB,

	RSB,
	SBC,
	RSC,
	TST,
	TEQ,
	CMN,
	BIC,
	MVN,

	// ARM float
	FMACS,
	FNMACS,
	FMSCS,
	FNMSCS,
	FMULS,
	FNMULS,
	FADDS,
	FSUBS,
	FDIVS,
	FCPYS,
	FABSS,
	FNEGS,
	FSQRTS,
	FCMPS,
	FCMPES,
	FCMPZS,
	FCMPEZS,
	CVTDS,
	FTOUIS,
	FTOUIZS,
	FTOSIS,
	FTOSIZS,
	FUITOS,
	FSITOS,
	FMRS,
	FMSR,
	FLDS,
	FSTS,

	// fake
	MODULO,
	LABEL,
	ASM,
	CALL_MEMBER,

	NUM_INSTRUCTION_NAMES,
	INVALID = -1
};

#ifdef OVERFLOW
#undef OVERFLOW
#endif

enum class ArmCond {
	EQUAL,
	NOT_EQUAL,
	CARRY_SET,
	CARRY_CLEAR,
	NEGATIVE,
	POSITIVE,
	OVERFLOW,
	NO_OVERFLOW,
	UNSIGNED_HIGHER,
	UNSIGNED_LOWER_SAME,
	GREATER_EQUAL,
	LESS_THAN,
	GREATER_THAN,
	LESS_EQUAL,
	ALWAYS,
	UNKNOWN = -1,
};

const string get_instruction_name(InstID inst);

struct GlobalVar {
	string name;
	void *pos; // points into the memory of a script
	int size;
};

struct Label {
	string name;
	int inst_no;
	int64 value;
};

struct WantedLabel {
	string name;
	int pos; // position to fill into     relative to CodeOrigin (Opcode[0])
	int size; // number of bytes to fill
	int add; // to add to the value...
	int label_no;
	int inst_no;
	bool relative;
	bool abs;
};

struct AsmData {
	int size; // number of bytes
	int cmd_pos;
	int offset; // relative to code_origin (Opcode[0])
	//void *data;
};

struct BitChange {
	int cmd_pos;
	int offset; // relative to code_origin (Opcode[0])
	int bits_size;
};

struct MetaInfo {
	int64 code_origin; // how to interpret opcode buffer[0]
	int bits_size;
	int line_offset; // number of script lines preceding asm block (to give correct error messages)

	//Array<Label> label;
	//Array<WantedLabel> wanted_label;

	Array<AsmData> data;
	Array<BitChange> bit_change;
	Array<GlobalVar> global_var;

	MetaInfo(int bits_size);
};

struct Register;
enum class ParamType;
enum class DispMode;

// a real parameter (usable)
struct InstructionParam {
	InstructionParam();
	ParamType type;
	DispMode disp;
	Register *reg, *reg2;
	bool deref;
	int size;
	int64 value; // disp or immediate
	bool is_label;
	bool write_back;

	bool has_explicit_size() const;
	string str(bool hide_size = false);
};

struct InstructionWithParams {
	InstID inst;
	ArmCond condition;
	InstructionParam p[3];
	int line, col;
	int size;
	int addr_size;
	int param_size;
	string str();
};


enum {
	SIZE_8 = 1,
	SIZE_16 = 2,
	SIZE_24 = 3,
	SIZE_32 = 4,
	SIZE_48 = 6,
	SIZE_64 = 8,
	SIZE_128 = 16,
	SIZE_8L4 = -13,
	SIZE_12 = -14,
	SIZE_8S2 = -15,
	/*SIZE_VARIABLE = -5,
	SIZE_32OR48 = -6,*/
	SIZE_UNKNOWN = -7,
};

extern InstructionParam param_none;
InstructionParam param_reg(RegID reg);
InstructionParam param_reg_set(int set);
InstructionParam param_deref_reg(RegID reg, int size);
InstructionParam param_deref_reg_shift(RegID reg, int shift, int size);
InstructionParam param_deref_reg_shift_reg(RegID reg, RegID reg2, int size);
InstructionParam param_imm(int64 value, int size);
InstructionParam param_deref_imm(int64 value, int size);
InstructionParam param_label(int64 value, int size);
InstructionParam param_deref_label(int64 value, int size);

struct InstructionWithParamsList : public Array<InstructionWithParams> {
	InstructionWithParamsList(int line_offset);
	~InstructionWithParamsList();

//	void add_easy(int inst, int param1_type = PK_NONE, int param1_size = -1, void *param1 = NULL, int param2_type = PK_NONE, int param2_size = -1, void *param2 = NULL);
	void add2(InstID inst, const InstructionParam &p1 = param_none, const InstructionParam &p2 = param_none);
	void add_arm(ArmCond cond, InstID inst, const InstructionParam &p1, const InstructionParam &p2 = param_none, const InstructionParam &p3 = param_none);


	// new label system
	int create_label(const string &name);
	int _find_label(const string &name);
	void insert_location_label(int index);
	int64 _label_value(int index);


	int find_or_create_label(const string &name);
	void *get_label_value(const string &name);

	void add_wanted_label(int pos, int label_no, int inst_no, bool rel, bool abs, int size);

	void append_from_source(const string &code);
	void shrink_jumps(void *oc, int ocs);
	void optimize(void *oc, int ocs);
	void compile(void *oc, int &ocs);
	void link_wanted_labels(void *oc);
	void add_instruction(char *oc, int &ocs, int n);
	void add_instruction_arm(char *oc, int &ocs, int n);

	void show();

	Array<Label> label;
	Array<WantedLabel> wanted_label;
	int current_line;
	int current_col;
	int current_inst;
};

void init(InstructionSet instruction_set = InstructionSet::NATIVE);
InstructionSet guess_native_instruction_set();
bool assemble(const char *code, char *oc, int &ocs);
string disassemble(void *code, int length = -1, bool allow_comments = true);

class Exception : public ::Exception {
public:
	Exception(const string &message, const string &expression, int line, int column);
	~Exception() override;
	string message() const override;
	void print() const;
	int line, column;
	string expression;
};

void add_instruction(char *oc, int &ocs, Asm::InstID inst, const InstructionParam &p1, const InstructionParam &p2 = param_none, const InstructionParam &p3 = param_none);
void set_instructionSet(int set);
bool immediate_allowed(InstID inst);
extern int OCParam;
extern MetaInfo *CurrentMetaInfo;

void get_instruction_param_flags(InstID inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write);
bool get_instruction_allow_const(InstID inst);
bool get_instruction_allow_gen_reg(InstID inst);

};

#endif
