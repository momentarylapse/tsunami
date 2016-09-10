#include "../../base/base.h"
#include "../../file/file.h"
#include "asm.h"
#include <stdio.h>

namespace Asm
{


int OCParam;



InstructionSetData InstructionSet;

struct ParserState
{
	bool EndOfLine;
	bool EndOfCode;
	int LineNo;
	int ColumnNo;
	int DefaultSize;
	int ParamSize, AddrSize;
	bool ExtendModRMBase;
	bool ExtendModRMReg;
	bool ExtendModRMIndex;
	int FullRegisterSize;
	InstructionWithParamsList *list;
	void init()
	{
		DefaultSize = SIZE_32;
		FullRegisterSize = InstructionSet.pointer_size;

		if (CurrentMetaInfo)
			if (CurrentMetaInfo->mode16)
				DefaultSize = SIZE_16;

		list = NULL;
	}
	void reset(InstructionWithParamsList *_list)
	{
		ParamSize = DefaultSize;
		AddrSize = DefaultSize;
		ExtendModRMBase = false;
		ExtendModRMReg = false;
		ExtendModRMIndex = false;
		list = _list;
	}
	string get_label(int i)
	{
		if (list)
			if ((i >= 0) and (i < list->label.num))
				return list->label[i].name;
		return "_label_" + i2s(i);
	}
};
static ParserState state;

const char *code_buffer;
MetaInfo *CurrentMetaInfo = NULL;
MetaInfo DummyMetaInfo;

int arm_encode_8l4(unsigned int value);

Exception::Exception(const string &_message, const string &_expression, int _line, int _column)
{
	if (_expression.num > 0)
		message += "\"" + _expression + "\": ";
	message += _message;
	line = _line;
	column = _column;
	if (line >= 0)
		message += "\nline " + i2s(line);
}

Exception::~Exception(){}

void Exception::print() const
{
	msg_error(message);
}

void SetError(const string &str)
{
	//msg_error(str + format("\nline %d", LineNo + 1));
	throw Exception(str, "", state.LineNo, state.ColumnNo);
}


bool DebugAsm = false;

static void so(const char *str)
{
	if (DebugAsm)
		printf("%s\n",str);
}

static void so(const string &str)
{
	if (DebugAsm)
		printf("%s\n",str.c_str());
}

static void so(int i)
{
	if (DebugAsm)
		printf("%d\n",i);
}



MetaInfo::MetaInfo()
{
	mode16 = false;
	code_origin = 0;
	line_offset = 0;
}



// groups of registers
enum
{
	REG_GROUP_NONE,
	REG_GROUP_GENERAL,
	REG_GROUP_GENERAL2,
	REG_GROUP_SEGMENT,
	REG_GROUP_FLAGS,
	REG_GROUP_CONTROL,
	REG_GROUP_X87,
	REG_GROUP_XMM,
	REG_GROUP_VFP, // ARM-float
};


struct Register
{
	string name;
	int id, group, size;
	bool extend_mod_rm;
};
Array<Register> Registers;
Array<Register*> RegisterByID;
int RegRoot[NUM_REGISTERS];
int RegResize[NUM_REG_ROOTS][MAX_REG_SIZE + 1];

void add_reg(const string &name, int id, int group, int size, int root = -1)
{
	Register r;
	r.extend_mod_rm = false;
	r.name = name;
	r.id = id;
	r.group = group;
	if (group == REG_GROUP_GENERAL2){
		r.group = REG_GROUP_GENERAL;
		r.extend_mod_rm = true;
	}
	r.size = size;
	Registers.add(r);
	if (root < 0)
		root = NUM_REG_ROOTS - 1;
	RegRoot[id] = root;
	RegResize[root][size] = id;
}

string GetRegName(int reg)
{
	if ((reg < 0) or (reg >= NUM_REGISTERS))
		return "INVALID REG: " + i2s(reg);
	return RegisterByID[reg]->name;
}

struct InstructionName
{
	int inst;
	//const string name;
	const char *name;
	int rw1, rw2; // parameter is read(1), modified(2) or both (3)
	// 32 -> don't allow gen reg
	// 64 -> don't allow immediate
};

// rw1/2: 
const InstructionName InstructionNames[NUM_INSTRUCTION_NAMES + 1] = {
	{INST_DB,		"db"},
	{INST_DW,		"dw"},
	{INST_DD,		"dd"},
	{INST_DS,		"ds"},
	{INST_DZ,		"dz"},

	{INST_ADD,		"add",		3, 1},
	{INST_ADC,		"adc",		3, 1},
	{INST_SUB,		"sub",		3, 1},
	{INST_SBB,		"sbb",		3, 1},
	{INST_INC,		"inc",		3},
	{INST_DEC,		"dec",		3},
	{INST_MUL,		"mul",		3, 1},
	{INST_IMUL,		"imul",		3, 1},
	{INST_DIV,		"div",		64+3, 64+1},
	{INST_IDIV,		"idiv",		64+3, 64+1},
	{INST_MOV,		"mov",		2, 1},
	{INST_MOVZX,	"movzx",	2, 1},
	{INST_MOVSX,	"movsx",	2, 1},
	{INST_AND,		"and",		3, 1},
	{INST_OR,		"or",		3, 1},
	{INST_XOR,		"xor",		3, 1},
	{INST_NOT,		"not",		3},
	{INST_NEG,		"neg",		3},
	{INST_POP,		"pop",		2},
	{INST_POPA,		"popa",		2},
	{INST_PUSH,		"push",		1},
	{INST_PUSHA,	"pusha",	1},
	
	{INST_JO,		"jo",		1},
	{INST_JNO,		"jno",		1},
	{INST_JB,		"jb",		1},
	{INST_JNB,		"jnb",		1},
	{INST_JZ,		"jz",		1},
	{INST_JNZ,		"jnz",		1},
	{INST_JBE,		"jbe",		1},
	{INST_JNBE,		"jnbe",		1},
	{INST_JS,		"js",		1},
	{INST_JNS,		"jns",		1},
	{INST_JP,		"jp",		1},
	{INST_JNP,		"jnp",		1},
	{INST_JL,		"jl",		1},
	{INST_JNL,		"jnl",		1},
	{INST_JLE,		"jle",		1},
	{INST_JNLE,		"jnle",		1},
	
	{INST_CMP,		"cmp",		1, 1},
	
	{INST_SETO,		"seto",		2},
	{INST_SETNO,	"setno",	2},
	{INST_SETB,		"setb",		2},
	{INST_SETNB,	"setnb",	2},
	{INST_SETZ,		"setz",		2},
	{INST_SETNZ,	"setnz",	2},
	{INST_SETBE,	"setbe",	2},
	{INST_SETNBE,	"setnbe",	2},
	{INST_SETS,		"sets",		2},
	{INST_SETNS,	"setns",	2},
	{INST_SETP,		"setp",		2},
	{INST_SETNP,	"setnp",	2},
	{INST_SETL,		"setl",		2},
	{INST_SETNL,	"setnl",	2},
	{INST_SETLE,	"setle",	2},
	{INST_SETNLE,	"setnle",	2},
	
	{INST_SLDT,		"sldt"},
	{INST_STR,		"str"},
	{INST_LLDT,		"lldt"},
	{INST_LTR,		"ltr"},
	{INST_VERR,		"verr"},
	{INST_VERW,		"verw"},
	{INST_SGDT,		"sgdt"},
	{INST_SIDT,		"sidt"},
	{INST_LGDT,		"lgdt"},
	{INST_LIDT,		"lidt"},
	{INST_SMSW,		"smsw"},
	{INST_LMSW,		"lmsw"},
	
	{INST_TEST,		"test",		1, 1},
	{INST_XCHG,		"xchg",		3, 3},
	{INST_LEA,		"lea", 		32+2, 32+1},
	{INST_NOP,		"nop"},
	{INST_CBW_CWDE,	"cbw/cwde"},
	{INST_CGQ_CWD,	"cgq/cwd"},
	{INST_MOVS_DS_ESI_ES_EDI,	"movs_ds:esi,es:edi"},
	{INST_MOVS_B_DS_ESI_ES_EDI,	"movs.b_ds:esi,es:edi"},
	{INST_CMPS_DS_ESI_ES_EDI,	"cmps_ds:esi,es:edi"},
	{INST_CMPS_B_DS_ESI_ES_EDI,	"cmps.b_ds:esi,es:edi"},
	{INST_ROL,		"rol",		3, 1},
	{INST_ROR,		"ror",		3, 1},
	{INST_RCL,		"rcl",		3, 1},
	{INST_RCR,		"rcr",		3, 1},
	{INST_SHL,		"shl",		3, 1},
	{INST_SHR,		"shr",		3, 1},
	{INST_SAR,		"sar",		3, 1},
	{INST_RET,		"ret",		1},
	{INST_LEAVE,	"leave",	1},
	{INST_RET_FAR,	"ret_far",	1},
	{INST_INT,		"int",		1},
	{INST_IRET,		"iret",		1},
	
	// x87
	{INST_FADD,		"fadd",		64+32+1},
	{INST_FMUL,		"fmul",		64+32+1},
	{INST_FSUB,		"fsub",		64+32+1},
	{INST_FDIV,		"fdiv",		64+32+1},
	{INST_FLD,		"fld",		64+32+1},
	{INST_FLD1,		"fld1",		64+32+0},
	{INST_FLDZ,		"fldz",		64+32+0},
	{INST_FLDPI,	"fldpi",	64+32+0},
	{INST_FXCH,		"fxch",		64+32+3, 64+32+3},
	{INST_FST,		"fst",		64+32+2},
	{INST_FSTP,		"fstp",		64+32+2},
	{INST_FILD,		"fild",		64+32+1},
	{INST_FADDP,	"faddp",	64+32+1},
	{INST_FMULP,	"fmulp",	64+32+1},
	{INST_FSUBP,	"fsubp",	64+32+1},
	{INST_FDIVP,	"fdivp",	64+32+1},
	{INST_FLDCW,	"fldcw",	64+32+1},
	{INST_FNSTCW,	"fnstcw",	64+32+2},
	{INST_FNSTSW,	"fnstsw",	64+32+2},
	{INST_FISTP,	"fistp",	64+32+2},
	{INST_FSQRT,	"fsqrt",	64+32+3},
	{INST_FSIN,		"fsin",		64+32+3},
	{INST_FCOS,		"fcos",		64+32+3},
	{INST_FPTAN,	"fptan",	64+32+3},
	{INST_FPATAN,	"fpatan",	64+32+3},
	{INST_FYL2X,	"fyl2x",	64+32+3},
	{INST_FCHS,		"fchs",		64+32+3},
	{INST_FABS,		"fabs",		64+32+3},
	{INST_FUCOMPP,	"fucompp",	64+32+1, 64+32+1},
	
	{INST_LOOP,		"loop"},
	{INST_LOOPE,	"loope"},
	{INST_LOOPNE,	"loopne"},
	{INST_IN,		"in",		2, 1},
	{INST_OUT,		"out",		1, 1},
	
	{INST_CALL,		"call",		1},
	{INST_CALL_FAR,	"call_far", 1},
	{INST_JMP,		"jmp",		1},
	{INST_JMP_FAR,	"jmp_far",		1},
	{INST_LOCK,		"lock"},
	{INST_REP,		"rep"},
	{INST_REPNE,	"repne"},
	{INST_HLT,		"hlt"},
	{INST_CMC,		"cmc"},
	{INST_CLC,		"clc"},
	{INST_STC,		"stc"},
	{INST_CLI,		"cli"},
	{INST_STI,		"sti"},
	{INST_CLD,		"cld"},
	{INST_STD,		"std"},

	// sse
	{INST_MOVSS,  "movss",  64+3, 64+1},
	{INST_MOVSD,  "movsd",  64+3, 64+1},
	{INST_MOVUPS, "movups", 64+3, 64+1},
	{INST_MOVAPS, "movaps", 64+3, 64+1},
	{INST_ADDSS,  "addss",  64+3, 64+1},
	{INST_ADDSD,  "addsd",  64+3, 64+1},
	{INST_ADDPS,  "addps",  64+3, 64+1},
	{INST_SUBSS,  "subss",  64+3, 64+1},
	{INST_SUBSD,  "subsd",  64+3, 64+1},
	{INST_MULSS,  "mulss",  64+3, 64+1},
	{INST_MULSD,  "mulsd",  64+3, 64+1},
	{INST_DIVSS,  "divss",  64+3, 64+1},
	{INST_DIVSD,  "divsd",  64+3, 64+1},
	{INST_SQRTSS, "sqrtss", 64+3, 64+1},
	{INST_SQRTSD, "sqrtsd", 64+3, 64+1},
	{INST_MINSS,  "minss",  64+3, 64+1},
	{INST_MINSD,  "minsd",  64+3, 64+1},
	{INST_MAXSS,  "maxss",  64+3, 64+1},
	{INST_MAXSD,  "maxsd",  64+3, 64+1},
	{INST_CVTTSS2SI, "cvttss2si", 64+3, 64+1},
	{INST_CVTTSD2SI, "cvttsd2si", 64+3, 64+1},
	{INST_CVTSI2SS,  "cvtsi2ss",  64+3, 64+1},
	{INST_CVTSI2SD,  "cvtsi2sd",  64+3, 64+1},
	{INST_COMISS,    "comiss",    64+3, 64+1},
	{INST_COMISD,    "comisd",    64+3, 64+1},
	{INST_UCOMISS,   "ucomiss",   64+3, 64+1},
	{INST_UCOMISD,   "ucomisd",   64+3, 64+1},

	{INST_B,		"b"},
	{INST_BL,		"bl"},
	{INST_BLX,		"blx"},

	{INST_LDR,		"ldr"},
	{INST_LDRB,		"ldrb"},
//	{inst_str,		"str"},
	{INST_STRB,		"strb"},

	{INST_LDMIA,		"ldmia"},
	{INST_LDMIB,		"ldmib"},
	{INST_LDMDA,		"ldmda"},
	{INST_LDMDB,		"ldmdb"},
	{INST_STMIA,		"stmia"},
	{INST_STMIB,		"stmib"},
	{INST_STMDA,		"stmda"},
	{INST_STMDB,		"stmdb"},

	{INST_RSB,	"rsb"},
	{INST_SBC,	"sbc"},
	{INST_RSC,	"rsc"},
	{INST_TST,	"tst"},
	{INST_TEQ,	"teq"},
	{INST_CMN,	"cmn"},
	{INST_BIC,	"bic"},
	{INST_MVN,	"mvn"},


	// ARM float
	{INST_FMACS,	"fmacs"},
	{INST_FNMACS,	"fnmacs"},
	{INST_FMSCS,	"fmscs"},
	{INST_FNMSCS,	"fnmscs"},
	{INST_FMULS,	"fmuls"},
	{INST_FNMULS,	"fnmuls"},
	{INST_FADDS,	"fadds"},
	{INST_FSUBS,	"fsubs"},
	{INST_FDIVS,	"fdivs"},
	{INST_FCPYS,	"fcpys"},
	{INST_FABSS,	"fabss"},
	{INST_FNEGS,	"fnegs"},
	{INST_FSQRTS,	"fsqrts"},
	{INST_FCMPS,	"fcmps"},
	{INST_FCMPES,	"fcmpes"},
	{INST_FCMPZS,	"fcmpzs"},
	{INST_FCMPEZS,	"fcmpezs"},
	{INST_CVTDS,	"cvtds"},
	{INST_FTOUIS,	"ftouis"},
	{INST_FTOUIZS,	"ftouizs"},
	{INST_FTOSIS,	"ftosis"},
	{INST_FTOSIZS,	"ftosizs"},
	{INST_FUITOS,	"fuitos"},
	{INST_FSITOS,	"fsitos"},
	{INST_FMRS,	"fmrs"},
	{INST_FMSR,	"fmsr"},
	{INST_FLDS,	"flds"},
	{INST_FSTS,	"fsts"},
	
	{-1,			"???"}
};



// parameter types
enum
{
	PARAMT_IMMEDIATE,
	PARAMT_REGISTER,
	PARAMT_REGISTER_OR_MEM, // ...
	PARAMT_MEMORY,
	PARAMT_REGISTER_SET,
	//PARAMT_SIB,
	PARAMT_NONE,
	PARAMT_INVALID
};


InstructionParam param_none;

// short parameter type
enum
{
	__Dummy__ = 10000,
	Eb,Ew,Ed,Eq,E48,
	Gb,Gw,Gd,Gq,
	Ib,Iw,Id,Iq,I48,
	Ob,Ow,Od,Oq,
	Rb,Rw,Rd,Rq,
	Cb,Cw,Cd,Cq,
	Mb,Mw,Md,Mq,
	Jb,Jw,Jd,Jq,
	Sw,Xx,
	XMd,XMq,XMdq
};

// displacement for registers
enum
{
	DISP_MODE_NONE,     // reg
	DISP_MODE_8,        // reg + 8bit
	DISP_MODE_16,       // reg + 16bit
	DISP_MODE_32,       // reg + 32bit
	DISP_MODE_SIB,      // SIB-byte
	DISP_MODE_8_SIB,    // SIB-byte + 8bit
	DISP_MODE_REG2,     // reg + reg2
	DISP_MODE_8_REG2,   // reg + reg2 + 8bit
	DISP_MODE_16_REG2   // reg + reg2 + 16bit
};


InstructionWithParamsList::InstructionWithParamsList(int line_no)
{
	current_inst = 0;
	current_line = line_no;
	current_col = 0;
}

InstructionWithParamsList::~InstructionWithParamsList()
{}

Register *get_reg(int reg)
{
	if ((reg < 0) or (reg >= RegisterByID.num))
		SetError("invalid register index: " + i2s(reg));
	return RegisterByID[reg];
}

InstructionParam param_reg(int reg)
{
	InstructionParam p;
	p.type = PARAMT_REGISTER;
	p.reg = get_reg(reg);
	p.size = p.reg->size;
	return p;
}

InstructionParam param_deref_reg(int reg, int size)
{
	InstructionParam p;
	p.type = PARAMT_REGISTER;
	p.reg = get_reg(reg);
	p.size = size;
	p.deref = true;
	return p;
}

InstructionParam param_reg_set(int set)
{
	InstructionParam p;
	p.type = PARAMT_REGISTER_SET;
	p.size = SIZE_32;
	p.value = set;
	return p;
}

InstructionParam param_deref_reg_shift(int reg, int shift, int size)
{
	InstructionParam p;
	p.type = PARAMT_REGISTER;
	p.reg = get_reg(reg);
	p.size = size;
	p.deref = true;
	p.value = shift;
	p.disp = ((shift < 120) and (shift > -120)) ? DISP_MODE_8 : DISP_MODE_32;
	return p;
}

InstructionParam param_deref_reg_shift_reg(int reg, int reg2, int size)
{
	InstructionParam p;
	p.type = PARAMT_REGISTER;
	p.reg = get_reg(reg);
	p.size = size;
	p.reg2 = get_reg(reg2);
	p.deref = true;
	p.value = 1;
	p.disp = DISP_MODE_REG2;
	return p;
}

InstructionParam param_imm(long long value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	return p;
}

InstructionParam param_deref_imm(long long value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	p.deref = true;
	return p;
}

InstructionParam param_label(long long value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	p.is_label = true;
	return p;
}

InstructionParam param_deref_label(long long value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	p.is_label = true;
	p.deref = true;
	return p;
}

void InstructionWithParamsList::add_arm(int cond, int inst, const InstructionParam &p1 = param_none, const InstructionParam &p2, const InstructionParam &p3)
{
	InstructionWithParams i;
	i.inst = inst;
	i.condition = cond;
	i.p[0] = p1;
	i.p[1] = p2;
	i.p[2] = p3;
	i.line = current_line;
	i.col = current_col;
	add(i);
}

void InstructionWithParamsList::add2(int inst, const InstructionParam &p1, const InstructionParam &p2)
{
	InstructionWithParams i;
	i.inst = inst;
	i.condition = ARM_COND_ALWAYS;
	i.p[0] = p1;
	i.p[1] = p2;
	i.p[2] = param_none;
	i.line = current_line;
	i.col = current_col;
	add(i);
}

void InstructionWithParamsList::show()
{
	msg_write("--------------");
	state.reset(this);
	foreachi(Asm::InstructionWithParams &i, *this, n){
		for (Label &l: label)
			if (l.inst_no == n)
				msg_write("    " + l.name + ":");
		msg_write(i.str());
	}
}

int InstructionWithParamsList::add_label(const string &name)
{
	so("add_label: " + name);
	// label already in use? (used before declared)
	foreachi(Label &l, label, i)
		if (l.inst_no < 0)
			if (l.name == name){
				if ((l.inst_no >= 0) and (name != "$"))
					SetError("label already declared: " + name);
				l.inst_no = num;
				so("----redecl");
				return i;
			}
	Label l;
	l.name = name;
	l.inst_no = num;
	l.value = -1;
	label.add(l);
	return label.num - 1;
}

int InstructionWithParamsList::get_label(const string &name)
{
	so("add_label: " + name);
	foreachi(Label &l, label, i)
		if (l.name == name){
			so("----reuse");
			return i;
		}
	Label l;
	l.name = name;
	l.inst_no = -1;
	l.value = -1;
	label.add(l);
	return label.num - 1;
}

void *InstructionWithParamsList::get_label_value(const string &name)
{
	for (Label &l: label)
		if (l.name == name)
			return (void*)l.value;
	return NULL;
}


void InstructionWithParamsList::add_wanted_label(int pos, int label_no, int inst_no, bool rel, bool abs, int size)
{
	if ((label_no < 0) or (label_no >= label.num))
		SetError("illegal wanted label request");
	WantedLabel w;
	w.pos = pos;
	w.size = size;
	w.label_no = label_no;
	w.name = label[label_no].name;
	w.relative = rel;
	w.abs = abs;
	w.inst_no = inst_no;
	wanted_label.add(w);
	so("add wanted label");
}

void InstructionWithParamsList::add_func_intro(int stack_alloc_size)
{
	if (InstructionSet.set == INSTRUCTION_SET_ARM)
		return;
	long reg_bp = (InstructionSet.set == INSTRUCTION_SET_AMD64) ? REG_RBP : REG_EBP;
	long reg_sp = (InstructionSet.set == INSTRUCTION_SET_AMD64) ? REG_RSP : REG_ESP;
	int s = InstructionSet.pointer_size;
	add2(INST_PUSH, param_reg(reg_bp));
	add2(INST_MOV, param_reg(reg_bp), param_reg(reg_sp));
	if (stack_alloc_size > 127){
		add2(INST_SUB, param_reg(reg_sp), param_imm(stack_alloc_size, SIZE_32));
	}else if (stack_alloc_size > 0){
		add2(INST_SUB, param_reg(reg_sp), param_imm(stack_alloc_size, SIZE_8));
	}
}

void InstructionWithParamsList::add_func_return(int return_size)
{
	add2(INST_LEAVE);
	if (return_size > 4)
		add2(INST_RET, param_imm(4, SIZE_16));
	else
		add2(INST_RET);
}

// which part of the modr/m byte is used
enum
{
	MRM_NONE,
	MRM_REG,
	MRM_MOD_RM
};

string SizeOut(int size)
{
	if (size == SIZE_8)		return "8";
	if (size == SIZE_16)		return "16";
	if (size == SIZE_32)		return "32";
	if (size == SIZE_48)		return "48";
	if (size == SIZE_64)		return "64";
	if (size == SIZE_128)		return "128";
	return "???";
}


string get_size_name(int size)
{
	if (size == SIZE_8)
		return "byte";
	if (size == SIZE_16)
		return "word";
	if (size == SIZE_32)
		return "dword";
	if (size == SIZE_48)
		return "s48";
	if (size == SIZE_64)
		return "qword";
	if (size == SIZE_128)
		return "dqword";
	return "";
}

// parameter definition (filter for real parameters)
struct InstructionParamFuzzy
{
	bool used;
	bool allow_memory_address;	// [0x12.34...]
	bool allow_memory_indirect;	// [eax]    [eax + ...]
	bool allow_immediate;		// 0x12.34...
	bool allow_register;		// eax
	int _type_;					// approximate type.... (UnFuzzy without mod/rm)
	Register *reg;				// if != NULL  -> force a single register
	int reg_group;
	int mrm_mode;				// which part of the modr/m byte is used?
	int size;
	bool immediate_is_relative;	// for jump


	bool match(InstructionParam &p);
	void print() const;
};

void InstructionParamFuzzy::print() const
{
	string t;
	if (used){
		if (allow_register)
			t += "	Reg";
		if (allow_immediate)
			t += "	Im";
		if (allow_memory_address)
			t += "	[Mem]";
		if (allow_memory_indirect)
			t += "	[Mem + ind]";
		if (reg)
			t += "  " + reg->name;
		if (size != SIZE_UNKNOWN)
			t += "  " + SizeOut(size);
		if (mrm_mode == MRM_REG)
			t += "   /r";
		else if (mrm_mode == MRM_MOD_RM)
			t += "   /m";
	}else{
		t += "	None";
	}
	printf("%s\n", t.c_str());
}

// an instruction/opcode the cpu offers
struct CPUInstruction
{
	int inst;
	int code, code_size, cap;
	bool has_modrm, has_small_param, has_small_addr, has_big_param, has_big_addr, has_fixed_param;
	bool ignore;
	InstructionParamFuzzy param1, param2;
	string name;

	bool match(InstructionWithParams &iwp);
	void print() const
	{
		printf("inst: %s   %.4x (%d) %d  %s\n", name.c_str(), code, code_size, cap, has_modrm ? "modr/m" : "");
		param1.print();
		param2.print();
	}
};

Array<CPUInstruction> CPUInstructions;

bool CPUInstruction::match(InstructionWithParams &iwp)
{
	if (inst != iwp.inst)
		return false;

	//return (param1.match(iwp.p[0])) and (param2.match(iwp.p[1]));
	bool b = (param1.match(iwp.p[0])) and (param2.match(iwp.p[1]));
	/*if (b){
		msg_write("source: " + iwp.p[0].str() + " " + iwp.p[1].str());
		print();
	}*/
	return b;
}

// expands the short instruction parameters
//   returns true if mod/rm byte needed
bool _get_inst_param_(int param, InstructionParamFuzzy &ip)
{
	ip.reg = NULL;
	ip.reg_group = REG_GROUP_NONE;
	ip.mrm_mode = MRM_NONE;
	ip.reg_group = -1;
	ip._type_ = PARAMT_INVALID;
	ip.allow_register = false;
	ip.allow_immediate = false;
	ip.allow_memory_address = false;
	ip.allow_memory_indirect = false;
	ip.immediate_is_relative = false;
	if (param < 0){	ip.used = false;	ip._type_ = PARAMT_NONE;	return false;	}
	ip.used = true;

	// is it a register?
	for (int i=0;i<Registers.num;i++)
		if (Registers[i].id == param){
			ip._type_ = PARAMT_REGISTER;
			ip.reg = &Registers[i];
			ip.allow_register = true;
			ip.reg_group = Registers[i].group;
			ip.size = Registers[i].size;
			return false;
		}
	// general reg / mem
	if ((param == Eb) or (param == Eq) or (param == Ew) or (param == Ed) or (param == E48)){
		ip._type_ = PARAMT_INVALID;//ParamTRegisterOrMem;
		ip.allow_register = true;
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = REG_GROUP_GENERAL;
		ip.mrm_mode = MRM_MOD_RM;
		if (param == Eb)	ip.size = SIZE_8;
		if (param == Ew)	ip.size = SIZE_16;
		if (param == Ed)	ip.size = SIZE_32;
		if (param == Eq)	ip.size = SIZE_64;
		if (param == E48)	ip.size = SIZE_48;
		return true;
	}
	// xmm reg / mem
	if ((param == XMd) or (param == XMq) or (param == XMdq)){
		ip._type_ = PARAMT_INVALID;//ParamTRegisterOrMem;
		ip.allow_register = true;
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = REG_GROUP_XMM;
		ip.mrm_mode = MRM_MOD_RM;
		if (param == XMd)	ip.size = SIZE_32;
		if (param == XMq)	ip.size = SIZE_64;
		if (param == XMdq)	ip.size = SIZE_128;
		return true;
	}
	// general reg (reg)
	if ((param == Gb) or (param == Gq) or (param == Gw) or (param == Gd)){
		ip._type_ = PARAMT_REGISTER;
		ip.allow_register = true;
		ip.reg_group = REG_GROUP_GENERAL;
		ip.mrm_mode = MRM_REG;
		if (param == Gb)	ip.size = SIZE_8;
		if (param == Gw)	ip.size = SIZE_16;
		if (param == Gd)	ip.size = SIZE_32;
		if (param == Gq)	ip.size = SIZE_64;
		return true;
	}
	// general reg (mod)
	if ((param == Rb) or (param == Rq) or (param == Rw) or (param == Rd)){
		ip._type_ = PARAMT_REGISTER;
		ip.allow_register = true;
		ip.reg_group = REG_GROUP_GENERAL;
		ip.mrm_mode = MRM_MOD_RM;
		if (param == Rb)	ip.size = SIZE_8;
		if (param == Rw)	ip.size = SIZE_16;
		if (param == Rd)	ip.size = SIZE_32;
		if (param == Rq)	ip.size = SIZE_64;
		return true;
	}
	// immediate
	if ((param == Ib) or (param == Iq) or (param == Iw) or (param == Id) or (param == I48)){
		ip._type_ = PARAMT_IMMEDIATE;
		ip.allow_immediate = true;
		if (param == Ib)	ip.size = SIZE_8;
		if (param == Iw)	ip.size = SIZE_16;
		if (param == Id)	ip.size = SIZE_32;
		if (param == Iq)	ip.size = SIZE_64;
		if (param == I48)	ip.size = SIZE_48;
		return false;
	}
	// immediate (relative)
	if ((param == Jb) or (param == Jq) or (param == Jw) or (param == Jd)){
		ip._type_ = PARAMT_IMMEDIATE;
		ip.allow_immediate = true;
		ip.immediate_is_relative = true;
		if (param == Jb)	ip.size = SIZE_8;
		if (param == Jw)	ip.size = SIZE_16;
		if (param == Jd)	ip.size = SIZE_32;
		if (param == Jq)	ip.size = SIZE_64;
		return false;
	}
	// mem
	if ((param == Ob) or (param == Oq) or (param == Ow) or (param == Od)){
		ip._type_ = PARAMT_MEMORY;
		ip.allow_memory_address = true;
		if (param == Ob)	ip.size = SIZE_8;
		if (param == Ow)	ip.size = SIZE_16;
		if (param == Od)	ip.size = SIZE_32;
		if (param == Oq)	ip.size = SIZE_64;
		return false;
	}
	// mem
	if ((param == Mb) or (param == Mq) or (param == Mw) or (param == Md)){
		ip._type_ = PARAMT_INVALID; // ...
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = REG_GROUP_GENERAL;
		ip.mrm_mode = MRM_MOD_RM;
		if (param == Mb)	ip.size = SIZE_8;
		if (param == Mw)	ip.size = SIZE_16;
		if (param == Md)	ip.size = SIZE_32;
		if (param == Mq)	ip.size = SIZE_64;
		return true;
	}
	// control reg
	if ((param == Cb) or (param == Cd) or (param == Cw) or (param == Cd)){
		ip._type_ = PARAMT_REGISTER;
		ip.allow_register = true;
		ip.reg_group = REG_GROUP_CONTROL;
		ip.mrm_mode = MRM_REG;
		if (param == Cb)	ip.size = SIZE_8;
		if (param == Cw)	ip.size = SIZE_16;
		if (param == Cd)	ip.size = SIZE_32;
		if (param == Cq)	ip.size = SIZE_64;
		return true;
	}
	// segment reg
	if (param == Sw){
		ip._type_ = PARAMT_REGISTER;
		ip.allow_register = true;
		ip.reg_group = REG_GROUP_SEGMENT;
		ip.mrm_mode = MRM_REG;
		ip.size = SIZE_16;
		return true;
	}
	// xmm reg (reg)
	if (param == Xx){
		ip._type_ = PARAMT_REGISTER;
		ip.allow_register = true;
		ip.reg_group = REG_GROUP_XMM;
		ip.mrm_mode = MRM_REG;
		ip.size = SIZE_128;
		return true;
	}
	msg_error("asm: unknown instparam (call Michi!)");
	msg_write(param);
	exit(0);
	return false;
}

enum
{
	OPT_SMALL_PARAM = 1,
	OPT_SMALL_ADDR,
	OPT_BIG_PARAM,
	OPT_BIG_ADDR,
	OPT_MEDIUM_PARAM,
};

void add_inst(int inst, int code, int code_size, int cap, int param1, int param2, int opt = 0, bool ignore = false)
{
	CPUInstruction i;
	memset(&i.param1, 0, sizeof(i.param1));
	memset(&i.param2, 0, sizeof(i.param2));
	i.inst = inst;
	i.code = code;
	i.code_size = code_size;
	i.cap = cap;
	i.ignore = ignore;
	bool m1 = _get_inst_param_(param1, i.param1);
	bool m2 = _get_inst_param_(param2, i.param2);
	i.has_modrm  = m1 or m2 or (cap >= 0);
	i.has_small_param = (opt == OPT_SMALL_PARAM);
	i.has_small_addr = (opt == OPT_SMALL_ADDR);
	i.has_big_param = (opt == OPT_BIG_PARAM);
	i.has_big_addr = (opt == OPT_BIG_ADDR);
	i.has_fixed_param = (opt != OPT_SMALL_PARAM) and (opt != OPT_MEDIUM_PARAM) and (opt != OPT_BIG_PARAM);
	if ((i.has_big_param) and (InstructionSet.set != INSTRUCTION_SET_AMD64))
		return;

	if (inst == INST_LEA)
		i.param2.size = SIZE_UNKNOWN;
	
	i.name = InstructionNames[NUM_INSTRUCTION_NAMES].name;
	for (int j=0;j<NUM_INSTRUCTION_NAMES;j++)
		if (inst == InstructionNames[j].inst)
			i.name = InstructionNames[j].name;
	CPUInstructions.add(i);
}

enum
{
	AP_NONE,
	AP_REG_12,
	AP_REG_16,
	AP_OFFSET24_0,
	AP_IMM12_0,
	AP_SHIFTED12_0,
};


void add_inst_arm(int inst, int code, int param1, int param2 = AP_NONE, int param3 = AP_NONE)
{
	CPUInstruction i;
	memset(&i.param1, 0, sizeof(i.param1));
	memset(&i.param2, 0, sizeof(i.param2));
	i.inst = inst;
	i.code = code;
	i.code_size = 4;
	i.cap = 0;

	i.name = GetInstructionName(inst);
	CPUInstructions.add(i);
}

const string GetInstructionName(int inst)
{
	if ((inst >= 0) and (inst < NUM_INSTRUCTION_NAMES))
		return Asm::InstructionNames[inst].name;
	return "???";
}

void GetInstructionParamFlags(int inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst){
			p1_read = ((InstructionNames[i].rw1 & 1) > 0);
			p1_write = ((InstructionNames[i].rw1 & 2) > 0);
			p2_read = ((InstructionNames[i].rw2 & 1) > 0);
			p2_write = ((InstructionNames[i].rw2 & 2) > 0);
		}
}

bool GetInstructionAllowConst(int inst)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return ((InstructionNames[i].rw1 & 64) == 0);
	return false;
}

bool GetInstructionAllowGenReg(int inst)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return ((InstructionNames[i].rw1 & 32) == 0);
	return false;
}



int QueryLocalInstructionSet()
{
#ifdef CPU_AMD64
	return INSTRUCTION_SET_AMD64;
#endif
#ifdef CPU_X86
	return INSTRUCTION_SET_X86;
#endif
#ifdef CPU_ARM
	return INSTRUCTION_SET_ARM;
#endif
	msg_error("Asm: unknown instruction set");
	return INSTRUCTION_SET_X86;
}


void InitARM()
{
	Registers.clear();
	for (int i=0; i<16; i++)
		add_reg(format("r%d", i), REG_R0 + i, REG_GROUP_GENERAL, SIZE_32, i);
	for (int i=0; i<32; i++)
		add_reg(format("s%d", i), REG_S0 + i, REG_GROUP_VFP, SIZE_32, 128 + i);

	// create easy to access array
	RegisterByID.clear();
	for (int i=0;i<Registers.num;i++){
		if (RegisterByID.num <= Registers[i].id)
			RegisterByID.resize(Registers[i].id + 1);
		RegisterByID[Registers[i].id] = &Registers[i];
	}

	CPUInstructions.clear();
	add_inst_arm(INST_B,    0x0a000000 ,0);
	add_inst_arm(INST_BL,   0x0b000000 ,0);
}

void InitX86()
{
	int set = InstructionSet.set;

	Registers.clear();
	add_reg("rax",	REG_RAX,	REG_GROUP_GENERAL,	SIZE_64,	0);
	add_reg("eax",	REG_EAX,	REG_GROUP_GENERAL,	SIZE_32,	0);
	add_reg("ax",	REG_AX,	REG_GROUP_GENERAL,	SIZE_16,	0);
	add_reg("ah",	REG_AH,	REG_GROUP_GENERAL,	SIZE_8,	0); // RegResize[] will be overwritten by al
	add_reg("al",	REG_AL,	REG_GROUP_GENERAL,	SIZE_8,	0);
	add_reg("rcx",	REG_RCX,	REG_GROUP_GENERAL,	SIZE_64,	1);
	add_reg("ecx",	REG_ECX,	REG_GROUP_GENERAL,	SIZE_32,	1);
	add_reg("cx",	REG_CX,	REG_GROUP_GENERAL,	SIZE_16,	1);
	add_reg("ch",	REG_CH,	REG_GROUP_GENERAL,	SIZE_8,	1);
	add_reg("cl",	REG_CL,	REG_GROUP_GENERAL,	SIZE_8,	1);
	add_reg("rdx",	REG_RDX,	REG_GROUP_GENERAL,	SIZE_64,	2);
	add_reg("edx",	REG_EDX,	REG_GROUP_GENERAL,	SIZE_32,	2);
	add_reg("dx",	REG_DX,	REG_GROUP_GENERAL,	SIZE_16,	2);
	add_reg("dh",	REG_DH,	REG_GROUP_GENERAL,	SIZE_8,	2);
	add_reg("dl",	REG_DL,	REG_GROUP_GENERAL,	SIZE_8,	2);
	add_reg("rbx",	REG_RBX,	REG_GROUP_GENERAL,	SIZE_64,	3);
	add_reg("ebx",	REG_EBX,	REG_GROUP_GENERAL,	SIZE_32,	3);
	add_reg("bx",	REG_BX,	REG_GROUP_GENERAL,	SIZE_16,	3);
	add_reg("bh",	REG_BH,	REG_GROUP_GENERAL,	SIZE_8,	3);
	add_reg("bl",	REG_BL,	REG_GROUP_GENERAL,	SIZE_8,	3);

	add_reg("rsp",	REG_RSP,	REG_GROUP_GENERAL,	SIZE_64,	4);
	add_reg("esp",	REG_ESP,	REG_GROUP_GENERAL,	SIZE_32,	4);
	add_reg("sp",	REG_SP,	REG_GROUP_GENERAL,	SIZE_16,	4);
	add_reg("rbp",	REG_RBP,	REG_GROUP_GENERAL,	SIZE_64,	5);
	add_reg("ebp",	REG_EBP,	REG_GROUP_GENERAL,	SIZE_32,	5);
	add_reg("bp",	REG_BP,	REG_GROUP_GENERAL,	SIZE_16,	5);
	add_reg("rsi",	REG_RSI,	REG_GROUP_GENERAL,	SIZE_64,	6);
	add_reg("esi",	REG_ESI,	REG_GROUP_GENERAL,	SIZE_32,	6);
	add_reg("si",	REG_SI,	REG_GROUP_GENERAL,	SIZE_16,	6);
	add_reg("rdi",	REG_RDI,	REG_GROUP_GENERAL,	SIZE_64,	7);
	add_reg("edi",	REG_EDI,	REG_GROUP_GENERAL,	SIZE_32,	7);
	add_reg("di",	REG_DI,	REG_GROUP_GENERAL,	SIZE_16,	7);

	add_reg("r8",	REG_R8,	REG_GROUP_GENERAL2,	SIZE_64,	8);
	add_reg("r8d",	REG_R8D,	REG_GROUP_GENERAL2,	SIZE_32,	8);
	add_reg("r9",	REG_R9,	REG_GROUP_GENERAL2,	SIZE_64,	9);
	add_reg("r9d",	REG_R9D,	REG_GROUP_GENERAL2,	SIZE_32,	9);
	add_reg("r10",	REG_R10,	REG_GROUP_GENERAL2,	SIZE_64,	10);
	add_reg("r10d",	REG_R10D,REG_GROUP_GENERAL2,	SIZE_32,	10);
	add_reg("r11",	REG_R11,	REG_GROUP_GENERAL2,	SIZE_64,	10);
	add_reg("r11d",	REG_R11D,REG_GROUP_GENERAL2,	SIZE_32,	11);
	add_reg("r12",	REG_R12,	REG_GROUP_GENERAL2,	SIZE_64,	12);
	add_reg("r12d",	REG_R12D,REG_GROUP_GENERAL2,	SIZE_32,	12);
	add_reg("r13",	REG_R13,	REG_GROUP_GENERAL2,	SIZE_64,	13);
	add_reg("r13d",	REG_R13D,REG_GROUP_GENERAL2,	SIZE_32,	13);
	add_reg("r14",	REG_R14,	REG_GROUP_GENERAL2,	SIZE_64,	14);
	add_reg("r14d",	REG_R14D,REG_GROUP_GENERAL2,	SIZE_32,	14);
	add_reg("r15",	REG_R15,	REG_GROUP_GENERAL2,	SIZE_64,	15);
	add_reg("r15d",	REG_R15D,REG_GROUP_GENERAL2,	SIZE_32,	15);

	add_reg("cs",	REG_CS,	REG_GROUP_SEGMENT,	SIZE_16);
	add_reg("ss",	REG_SS,	REG_GROUP_SEGMENT,	SIZE_16);
	add_reg("ds",	REG_DS,	REG_GROUP_SEGMENT,	SIZE_16);
	add_reg("es",	REG_ES,	REG_GROUP_SEGMENT,	SIZE_16);
	add_reg("fs",	REG_FS,	REG_GROUP_SEGMENT,	SIZE_16);
	add_reg("gs",	REG_GS,	REG_GROUP_SEGMENT,	SIZE_16);

	add_reg("cr0",	REG_CR0,	REG_GROUP_CONTROL,	SIZE_32);
	add_reg("cr1",	REG_CR1,	REG_GROUP_CONTROL,	SIZE_32);
	add_reg("cr2",	REG_RC2,	REG_GROUP_CONTROL,	SIZE_32);
	add_reg("cr3",	REG_CR3,	REG_GROUP_CONTROL,	SIZE_32);
	add_reg("cr4",	REG_CR4,	REG_GROUP_CONTROL,	SIZE_32);

	add_reg("st0",	REG_ST0,	REG_GROUP_X87,	SIZE_32,	16); // ??? 32
	add_reg("st1",	REG_ST1,	REG_GROUP_X87,	SIZE_32,	17);
	add_reg("st2",	REG_ST2,	REG_GROUP_X87,	SIZE_32,	18);
	add_reg("st3",	REG_ST3,	REG_GROUP_X87,	SIZE_32,	19);
	add_reg("st4",	REG_ST4,	REG_GROUP_X87,	SIZE_32,	20);
	add_reg("st5",	REG_ST5,	REG_GROUP_X87,	SIZE_32,	21);
	add_reg("st6",	REG_ST6,	REG_GROUP_X87,	SIZE_32,	22);
	add_reg("st7",	REG_ST7,	REG_GROUP_X87,	SIZE_32,	23);

	add_reg("xmm0",	REG_XMM0,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm1",	REG_XMM1,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm2",	REG_XMM2,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm3",	REG_XMM3,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm4",	REG_XMM4,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm5",	REG_XMM5,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm6",	REG_XMM6,	REG_GROUP_XMM,	SIZE_128);
	add_reg("xmm7",	REG_XMM7,	REG_GROUP_XMM,	SIZE_128);

	// create easy to access array
	RegisterByID.clear();
	for (int i=0;i<Registers.num;i++){
		if (RegisterByID.num <= Registers[i].id)
			RegisterByID.resize(Registers[i].id + 1);
		RegisterByID[Registers[i].id] = &Registers[i];
	}

	CPUInstructions.clear();
	add_inst(INST_DB		,0x00	,0	,-1	,Ib	,-1);
	add_inst(INST_DW		,0x00	,0	,-1	,Iw	,-1);
	add_inst(INST_DD		,0x00	,0	,-1	,Id	,-1);
	add_inst(INST_ADD		,0x00	,1	,-1	,Eb	,Gb);
	add_inst(INST_ADD		,0x01	,1	,-1	,Ew	,Gw, OPT_SMALL_PARAM);
	add_inst(INST_ADD		,0x01	,1	,-1	,Ed	,Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_ADD		,0x01	,1	,-1	,Eq	,Gq, OPT_BIG_PARAM);
	add_inst(INST_ADD		,0x02	,1	,-1	,Gb	,Eb);
	add_inst(INST_ADD		,0x03	,1	,-1	,Gw	,Eq, OPT_SMALL_PARAM);
	add_inst(INST_ADD		,0x03	,1	,-1	,Gd	,Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_ADD		,0x03	,1	,-1	,Gq	,Eq, OPT_BIG_PARAM);
	add_inst(INST_ADD		,0x04	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_ADD		,0x05	,1	,-1	,REG_AX, Iw, OPT_SMALL_PARAM);
	add_inst(INST_ADD		,0x05	,1	,-1	,REG_EAX, Id, OPT_MEDIUM_PARAM);
	add_inst(INST_ADD		,0x05	,1	,-1	,REG_RAX, Id, OPT_BIG_PARAM);
	add_inst(INST_PUSH		,0x06	,1	,-1	,REG_ES	,-1);
	add_inst(INST_POP		,0x07	,1	,-1	,REG_ES	,-1);
	add_inst(INST_OR		,0x08	,1	,-1	,Eb	,Gb);
	add_inst(INST_OR		,0x09	,1	,-1	,Ew	,Gw, OPT_SMALL_PARAM);
	add_inst(INST_OR		,0x09	,1	,-1	,Ed	,Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_OR		,0x09	,1	,-1	,Eq	,Gq, OPT_BIG_PARAM);
	add_inst(INST_OR		,0x0a	,1	,-1	,Gb	,Eb);
	add_inst(INST_OR,	0x0b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_OR,	0x0b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_OR,	0x0b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_OR		,0x0c	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_OR		,0x0d	,1	,-1	,REG_AX,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_OR		,0x0d	,1	,-1	,REG_EAX,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_OR		,0x0d	,1	,-1	,REG_RAX,	Id, OPT_BIG_PARAM);
	add_inst(INST_PUSH		,0x0e	,1	,-1	,REG_CS	,-1);
	add_inst(INST_SLDT		,0x000f	,2	,0	,Ew	,-1);
	add_inst(INST_STR		,0x000f	,2	,1	,Ew	,-1);
	add_inst(INST_LLDT		,0x000f	,2	,2	,Ew	,-1);
	add_inst(INST_LTR		,0x000f	,2	,3	,Ew	,-1);
	add_inst(INST_VERR		,0x000f	,2	,4	,Ew	,-1);
	add_inst(INST_VERW		,0x000f	,2	,5	,Ew	,-1);
	add_inst(INST_SGDT,	0x010f,	2,	0,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(INST_SGDT,	0x010f,	2,	0,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_SGDT,	0x010f,	2,	0,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(INST_SIDT,	0x010f,	2,	1,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(INST_SIDT,	0x010f,	2,	1,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_SIDT,	0x010f,	2,	1,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(INST_LGDT,	0x010f,	2,	2,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(INST_LGDT,	0x010f,	2,	2,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_LGDT,	0x010f,	2,	2,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(INST_LIDT,	0x010f,	2,	3,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(INST_LIDT,	0x010f,	2,	3,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_LIDT,	0x010f,	2,	3,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(INST_SMSW		,0x010f	,2	,4	,Ew	,-1);
	add_inst(INST_LMSW		,0x010f	,2	,6	,Ew	,-1);
	add_inst(INST_MOV		,0x200f	,2	,-1	,Rd	,Cd); // Fehler im Algorhytmus!!!!  (wirklich ???) -> Fehler in Tabelle?!?
	add_inst(INST_MOV		,0x220f	,2	,-1	,Cd	,Rd);
	add_inst(INST_JO		,0x800f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM); // 16/32 bit???
	add_inst(INST_JNO		,0x810f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JB		,0x820f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNB		,0x830f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JZ		,0x840f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNZ		,0x850f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JBE		,0x860f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNBE		,0x870f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JS		,0x880f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNS		,0x890f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JP		,0x8a0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNP		,0x8b0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JL		,0x8c0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNL		,0x8d0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JLE		,0x8e0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JNLE		,0x8f0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(INST_SETO		,0x900f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNO		,0x910f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETB		,0x920f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNB		,0x930f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETZ		,0x940f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNZ		,0x950f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETBE		,0x960f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNBE	,0x970f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETS		,0x980f	,2	,-1	,Eb	,-1); // error in table... "Ev"
	add_inst(INST_SETNS		,0x990f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETP		,0x9a0f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNP		,0x9b0f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETL		,0x9c0f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNL		,0x9d0f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETLE		,0x9e0f	,2	,-1	,Eb	,-1);
	add_inst(INST_SETNLE	,0x9f0f	,2	,-1	,Eb	,-1);
	add_inst(INST_IMUL,	0xaf0f,	2,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_IMUL,	0xaf0f,	2,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_IMUL,	0xaf0f,	2,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_MOVZX,	0xb60f,	2,	-1,	Gw,	Eb, OPT_SMALL_PARAM);
	add_inst(INST_MOVZX,	0xb60f,	2,	-1,	Gd,	Eb, OPT_MEDIUM_PARAM);
	add_inst(INST_MOVZX,	0xb60f,	2,	-1,	Gq,	Eb, OPT_BIG_PARAM);
	add_inst(INST_MOVZX,	0xb70f,	2,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_MOVZX,	0xb70f,	2,	-1,	Gd,	Ew, OPT_MEDIUM_PARAM);
	add_inst(INST_MOVZX,	0xb70f,	2,	-1,	Gq,	Ew, OPT_BIG_PARAM);
	add_inst(INST_MOVSX,	0xbe0f,	2,	-1,	Gw,	Eb, OPT_SMALL_PARAM);
	add_inst(INST_MOVSX,	0xbe0f,	2,	-1,	Gd,	Eb, OPT_MEDIUM_PARAM);
	add_inst(INST_MOVSX,	0xbe0f,	2,	-1,	Gq,	Eb, OPT_BIG_PARAM);
	add_inst(INST_MOVSX,	0xbf0f,	2,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_MOVSX,	0xbf0f,	2,	-1,	Gd,	Ew, OPT_MEDIUM_PARAM);
	add_inst(INST_MOVSX,	0xbf0f,	2,	-1,	Gq,	Ew, OPT_BIG_PARAM);
	add_inst(INST_ADC,	0x10	,1	,-1	,Eb	,Gb);
	add_inst(INST_ADC,	0x11,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_ADC,	0x11,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_ADC,	0x11,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_ADC,	0x12	,1	,-1	,Gb	,Eb);
	add_inst(INST_ADC,	0x13,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_ADC,	0x13,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_ADC,	0x13,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_ADC,	0x14	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_ADC,	0x15	,1	,-1	,REG_AX,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_ADC,	0x15	,1	,-1	,REG_EAX, Id, OPT_MEDIUM_PARAM);
	add_inst(INST_ADC,	0x15	,1	,-1	,REG_RAX, Id, OPT_BIG_PARAM);
	add_inst(INST_PUSH,	0x16	,1	,-1	,REG_SS, -1);
	add_inst(INST_POP,	0x17	,1	,-1	,REG_SS, -1);
	add_inst(INST_SBB,	0x18	,1	,-1	,Eb	,Gb);
	add_inst(INST_SBB,	0x19,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_SBB,	0x19,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_SBB,	0x19,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_SBB,	0x1a	,1	,-1	,Gb	,Eb);
	add_inst(INST_SBB,	0x1b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_SBB,	0x1b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_SBB,	0x1b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_SBB,	0x1c	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_SBB,	0x1d	,1	,-1	,REG_AX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_SBB,	0x1d	,1	,-1	,REG_EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_SBB,	0x1d	,1	,-1	,REG_RAX	,Id, OPT_BIG_PARAM);
	add_inst(INST_PUSH,	0x1e	,1	,-1	,REG_DS	,-1);
	add_inst(INST_POP,	0x1f	,1	,-1	,REG_DS	,-1);
	add_inst(INST_AND,	0x20	,1	,-1	,Eb	,Gb);
	add_inst(INST_AND,	0x21,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_AND,	0x21,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_AND,	0x21,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_AND,	0x22	,1	,-1	,Gb	,Eb);
	add_inst(INST_AND,	0x23,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_AND,	0x23,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_AND,	0x23,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_AND,	0x24	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_AND,	0x25	,1	,-1	,REG_AX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_AND,	0x25	,1	,-1	,REG_EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_AND,	0x25	,1	,-1	,REG_RAX	,Id, OPT_BIG_PARAM);
	add_inst(INST_SUB,	0x28	,1	,-1	,Eb	,Gb);
	add_inst(INST_SUB,	0x29,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_SUB,	0x29,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_SUB,	0x29,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_SUB,	0x2a	,1	,-1	,Gb	,Eb);
	add_inst(INST_SUB,	0x2b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_SUB,	0x2b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_SUB,	0x2b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_SUB,	0x2c	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_SUB,	0x2d	,1	,-1	,REG_AX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_SUB,	0x2d	,1	,-1	,REG_EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_SUB,	0x2d	,1	,-1	,REG_RAX	,Id, OPT_BIG_PARAM);
	add_inst(INST_XOR,	0x30	,1	,-1	,Eb	,Gb);
	add_inst(INST_XOR,	0x31,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_XOR,	0x31,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_XOR,	0x31,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_XOR,	0x32	,1	,-1	,Gb	,Eb);
	add_inst(INST_XOR,	0x33,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_XOR,	0x33,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_XOR,	0x33,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_XOR,	0x34	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_XOR,	0x35	,1	,-1	,REG_AX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_XOR,	0x35	,1	,-1	,REG_EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_XOR,	0x35	,1	,-1	,REG_RAX	,Iq, OPT_BIG_PARAM);
	add_inst(INST_CMP,	0x38	,1	,-1	,Eb	,Gb);
	add_inst(INST_CMP,	0x39,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_CMP,	0x39,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_CMP,	0x39,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_CMP,	0x3a	,1	,-1	,Gb	,Eb);
	add_inst(INST_CMP,	0x3b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_CMP,	0x3b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_CMP,	0x3b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_CMP,	0x3c	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_CMP,	0x3d	,1	,-1	,REG_AX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_CMP,	0x3d	,1	,-1	,REG_EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_CMP,	0x3d	,1	,-1	,REG_RAX	,Id, OPT_BIG_PARAM);
	if (set == INSTRUCTION_SET_X86){
		add_inst(INST_INC		,0x40	,1	,-1	,REG_EAX	,-1);
		add_inst(INST_INC		,0x41	,1	,-1	,REG_ECX	,-1);
		add_inst(INST_INC		,0x42	,1	,-1	,REG_EDX	,-1);
		add_inst(INST_INC		,0x43	,1	,-1	,REG_EBX	,-1);
		add_inst(INST_INC		,0x44	,1	,-1	,REG_ESP	,-1);
		add_inst(INST_INC		,0x45	,1	,-1	,REG_EBP	,-1);
		add_inst(INST_INC		,0x46	,1	,-1	,REG_ESI	,-1);
		add_inst(INST_INC		,0x47	,1	,-1	,REG_EDI	,-1);
		add_inst(INST_DEC		,0x48	,1	,-1	,REG_EAX	,-1);
		add_inst(INST_DEC		,0x49	,1	,-1	,REG_ECX	,-1);
		add_inst(INST_DEC		,0x4a	,1	,-1	,REG_EDX	,-1);
		add_inst(INST_DEC		,0x4b	,1	,-1	,REG_EBX	,-1);
		add_inst(INST_DEC		,0x4c	,1	,-1	,REG_ESP	,-1);
		add_inst(INST_DEC		,0x4d	,1	,-1	,REG_EBP	,-1);
		add_inst(INST_DEC		,0x4e	,1	,-1	,REG_ESI	,-1);
		add_inst(INST_DEC		,0x4f	,1	,-1	,REG_EDI	,-1);
	}
	if (set == INSTRUCTION_SET_X86){
		add_inst(INST_PUSH		,0x50	,1	,-1	,REG_EAX	,-1);
		add_inst(INST_PUSH		,0x51	,1	,-1	,REG_ECX	,-1);
		add_inst(INST_PUSH		,0x52	,1	,-1	,REG_EDX	,-1);
		add_inst(INST_PUSH		,0x53	,1	,-1	,REG_EBX	,-1);
		add_inst(INST_PUSH		,0x54	,1	,-1	,REG_ESP	,-1);
		add_inst(INST_PUSH		,0x55	,1	,-1	,REG_EBP	,-1);
		add_inst(INST_PUSH		,0x56	,1	,-1	,REG_ESI	,-1);
		add_inst(INST_PUSH		,0x57	,1	,-1	,REG_EDI	,-1);
		add_inst(INST_POP		,0x58	,1	,-1	,REG_EAX	,-1);
		add_inst(INST_POP		,0x59	,1	,-1	,REG_ECX	,-1);
		add_inst(INST_POP		,0x5a	,1	,-1	,REG_EDX	,-1);
		add_inst(INST_POP		,0x5b	,1	,-1	,REG_EBX	,-1);
		add_inst(INST_POP		,0x5c	,1	,-1	,REG_ESP	,-1);
		add_inst(INST_POP		,0x5d	,1	,-1	,REG_EBP	,-1);
		add_inst(INST_POP		,0x5e	,1	,-1	,REG_ESI	,-1);
		add_inst(INST_POP		,0x5f	,1	,-1	,REG_EDI	,-1);
	}else if (set == INSTRUCTION_SET_AMD64){
		add_inst(INST_PUSH		,0x50	,1	,-1	,REG_RAX	,-1);
		add_inst(INST_PUSH		,0x51	,1	,-1	,REG_RCX	,-1);
		add_inst(INST_PUSH		,0x52	,1	,-1	,REG_RDX	,-1);
		add_inst(INST_PUSH		,0x53	,1	,-1	,REG_RBX	,-1);
		add_inst(INST_PUSH		,0x54	,1	,-1	,REG_RSP	,-1);
		add_inst(INST_PUSH		,0x55	,1	,-1	,REG_RBP	,-1);
		add_inst(INST_PUSH		,0x56	,1	,-1	,REG_RSI	,-1);
		add_inst(INST_PUSH		,0x57	,1	,-1	,REG_RDI	,-1);
		add_inst(INST_POP		,0x58	,1	,-1	,REG_RAX	,-1);
		add_inst(INST_POP		,0x59	,1	,-1	,REG_RCX	,-1);
		add_inst(INST_POP		,0x5a	,1	,-1	,REG_RDX	,-1);
		add_inst(INST_POP		,0x5b	,1	,-1	,REG_RBX	,-1);
		add_inst(INST_POP		,0x5c	,1	,-1	,REG_RSP	,-1);
		add_inst(INST_POP		,0x5d	,1	,-1	,REG_RBP	,-1);
		add_inst(INST_POP		,0x5e	,1	,-1	,REG_RSI	,-1);
		add_inst(INST_POP		,0x5f	,1	,-1	,REG_RDI	,-1);
	}
	add_inst(INST_PUSHA		,0x60	,1	,-1	,-1	,-1);
	add_inst(INST_POPA		,0x61	,1	,-1	,-1	,-1);
	add_inst(INST_PUSH,	0x68,	1,	-1,	Iw,	-1, OPT_SMALL_PARAM);
	add_inst(INST_PUSH,	0x68,	1,	-1,	Id,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_PUSH,	0x68,	1,	-1,	Iq,	-1, OPT_BIG_PARAM);
	add_inst(INST_IMUL,	0x69,	1,	-1,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_IMUL,	0x69,	1,	-1,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_IMUL,	0x69,	1,	-1,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_PUSH		,0x6a	,1	,-1	,Ib	,-1);
	add_inst(INST_JO		,0x70	,1	,-1	,Jb	,-1);
	add_inst(INST_JNO		,0x71	,1	,-1	,Jb	,-1);
	add_inst(INST_JB		,0x72	,1	,-1	,Jb	,-1);
	add_inst(INST_JNB		,0x73	,1	,-1	,Jb	,-1);
	add_inst(INST_JZ		,0x74	,1	,-1	,Jb	,-1);
	add_inst(INST_JNZ		,0x75	,1	,-1	,Jb	,-1);
	add_inst(INST_JBE		,0x76	,1	,-1	,Jb	,-1);
	add_inst(INST_JNBE		,0x77	,1	,-1	,Jb	,-1);
	add_inst(INST_JS		,0x78	,1	,-1	,Jb	,-1);
	add_inst(INST_JNS		,0x79	,1	,-1	,Jb	,-1);
	add_inst(INST_JP		,0x7a	,1	,-1	,Jb	,-1);
	add_inst(INST_JNP		,0x7b	,1	,-1	,Jb	,-1);
	add_inst(INST_JL		,0x7c	,1	,-1	,Jb	,-1);
	add_inst(INST_JNL		,0x7d	,1	,-1	,Jb	,-1);
	add_inst(INST_JLE		,0x7e	,1	,-1	,Jb	,-1);
	add_inst(INST_JNLE		,0x7f	,1	,-1	,Jb	,-1);
	// Immediate Group 1
	add_inst(INST_ADD		,0x80	,1	,0	,Eb	,Ib);
	add_inst(INST_OR		,0x80	,1	,1	,Eb	,Ib);
	add_inst(INST_ADC		,0x80	,1	,2	,Eb	,Ib);
	add_inst(INST_SBB		,0x80	,1	,3	,Eb	,Ib);
	add_inst(INST_AND		,0x80	,1	,4	,Eb	,Ib);
	add_inst(INST_SUB		,0x80	,1	,5	,Eb	,Ib);
	add_inst(INST_XOR		,0x80	,1	,6	,Eb	,Ib);
	add_inst(INST_CMP		,0x80	,1	,7	,Eb	,Ib);
	add_inst(INST_ADD,	0x81,	1,	0,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_ADD,	0x81,	1,	0,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_ADD,	0x81,	1,	0,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_OR,	0x81,	1,	1,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_OR,	0x81,	1,	1,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_OR,	0x81,	1,	1,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_ADC,	0x81,	1,	2,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_ADC,	0x81,	1,	2,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_ADC,	0x81,	1,	2,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_SBB,	0x81,	1,	3,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_SBB,	0x81,	1,	3,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_SBB,	0x81,	1,	3,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_AND,	0x81,	1,	4,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_AND,	0x81,	1,	4,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_AND,	0x81,	1,	4,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_SUB,	0x81,	1,	5,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_SUB,	0x81,	1,	5,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_SUB,	0x81,	1,	5,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_XOR,	0x81,	1,	6,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_XOR,	0x81,	1,	6,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_XOR,	0x81,	1,	6,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_CMP,	0x81,	1,	7,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_CMP,	0x81,	1,	7,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_CMP,	0x81,	1,	7,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_ADD,	0x83,	1,	0,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_ADD,	0x83,	1,	0,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_ADD,	0x83,	1,	0,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_OR,	0x83,	1,	1,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_OR,	0x83,	1,	1,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_OR,	0x83,	1,	1,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_ADC,	0x83,	1,	2,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_ADC,	0x83,	1,	2,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_ADC,	0x83,	1,	2,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_SBB,	0x83,	1,	3,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_SBB,	0x83,	1,	3,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_SBB,	0x83,	1,	3,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_AND,	0x83,	1,	4,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_AND,	0x83,	1,	4,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_AND,	0x83,	1,	4,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_SUB,	0x83,	1,	5,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_SUB,	0x83,	1,	5,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_SUB,	0x83,	1,	5,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_XOR,	0x83,	1,	6,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_XOR,	0x83,	1,	6,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_XOR,	0x83,	1,	6,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_CMP,	0x83,	1,	7,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_CMP,	0x83,	1,	7,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_CMP,	0x83,	1,	7,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_TEST,	0x84	,1	,-1	,Eb	,Gb);
	add_inst(INST_TEST,	0x85,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_TEST,	0x85,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_TEST,	0x85,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_XCHG,	0x86	,1	,-1	,Eb	,Gb);
	add_inst(INST_XCHG,	0x87,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_XCHG,	0x87,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG,	0x87,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_MOV,	0x88	,1	,-1	,Eb	,Gb);
	add_inst(INST_MOV,	0x89,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(INST_MOV,	0x89,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV,	0x89,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(INST_MOV,	0x8a	,1	,-1	,Gb	,Eb);
	add_inst(INST_MOV,	0x8b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(INST_MOV,	0x8b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV,	0x8b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(INST_MOV,	0x8c	,1	,-1	,Ew	,Sw	);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gw,	Ew, OptSmallParam);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gd,	Ed, OptMediumParam);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(INST_LEA,	0x8d,	1,	-1,	Gw,	Mw, OPT_SMALL_PARAM);
	add_inst(INST_LEA,	0x8d,	1,	-1,	Gd,	Md, OPT_MEDIUM_PARAM);
	add_inst(INST_LEA,	0x8d,	1,	-1,	Gq,	Mq, OPT_BIG_PARAM);
	add_inst(INST_MOV,	0x8e	,1	,-1	,Sw	,Ew, OPT_MEDIUM_PARAM);
	add_inst(INST_POP,	0x8f,	1,	-1,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(INST_POP,	0x8f,	1,	-1,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_POP,	0x8f,	1,	-1,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_NOP		,0x90	,1	,-1	,-1	,-1);
	add_inst(INST_XCHG		,0x91	,1	,-1	,REG_AX	,REG_CX, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x92	,1	,-1	,REG_AX	,REG_DX, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x93	,1	,-1	,REG_AX	,REG_BX, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x94	,1	,-1	,REG_AX	,REG_SP, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x95	,1	,-1	,REG_AX	,REG_BP, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x96	,1	,-1	,REG_AX	,REG_SI, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x97	,1	,-1	,REG_AX	,REG_DI, OPT_SMALL_PARAM);
	add_inst(INST_XCHG		,0x91	,1	,-1	,REG_EAX	,REG_ECX, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x92	,1	,-1	,REG_EAX	,REG_EDX, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x93	,1	,-1	,REG_EAX	,REG_EBX, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x94	,1	,-1	,REG_EAX	,REG_ESP, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x95	,1	,-1	,REG_EAX	,REG_EBP, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x96	,1	,-1	,REG_EAX	,REG_ESI, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x97	,1	,-1	,REG_EAX	,REG_EDI, OPT_MEDIUM_PARAM);
	add_inst(INST_XCHG		,0x91	,1	,-1	,REG_RAX	,REG_RCX, OPT_BIG_PARAM);
	add_inst(INST_XCHG		,0x92	,1	,-1	,REG_RAX	,REG_RDX, OPT_BIG_PARAM);
	add_inst(INST_XCHG		,0x93	,1	,-1	,REG_RAX	,REG_RBX, OPT_BIG_PARAM);
	add_inst(INST_XCHG		,0x94	,1	,-1	,REG_RAX	,REG_RSP, OPT_BIG_PARAM);
	add_inst(INST_XCHG		,0x95	,1	,-1	,REG_RAX	,REG_RBP, OPT_BIG_PARAM);
	add_inst(INST_XCHG		,0x96	,1	,-1	,REG_RAX	,REG_RSI, OPT_BIG_PARAM);
	add_inst(INST_XCHG		,0x97	,1	,-1	,REG_RAX	,REG_RDI, OPT_BIG_PARAM);
	add_inst(INST_CBW_CWDE	,0x98	,1	,-1	,-1 ,-1);
	add_inst(INST_CGQ_CWD	,0x99	,1	,-1	,-1 ,-1);
	add_inst(INST_MOV		,0xa0	,1	,-1	,REG_AL	,Ob, 0, true);
	add_inst(INST_MOV		,0xa1	,1	,-1	,REG_AX	,Ow, OPT_SMALL_PARAM, true);
	add_inst(INST_MOV		,0xa1	,1	,-1	,REG_EAX	,Od, OPT_MEDIUM_PARAM, true);
	add_inst(INST_MOV		,0xa1	,1	,-1	,REG_RAX	,Oq, OPT_BIG_PARAM, true);
	add_inst(INST_MOV		,0xa2	,1	,-1	,Ob	,REG_AL, 0, true);
	add_inst(INST_MOV,	0xa3,	1,	-1,	Ow,	REG_AX, OPT_SMALL_PARAM, true);
	add_inst(INST_MOV,	0xa3,	1,	-1,	Od,	REG_EAX, OPT_MEDIUM_PARAM, true);
	add_inst(INST_MOV,	0xa3,	1,	-1,	Oq,	REG_RAX, OPT_BIG_PARAM, true);
	add_inst(INST_MOVS_B_DS_ESI_ES_EDI	,0xa4	,1	,-1	,-1,-1);
	add_inst(INST_MOVS_DS_ESI_ES_EDI	,0xa5	,1	,-1	,-1,-1);
	add_inst(INST_CMPS_B_DS_ESI_ES_EDI	,0xa6	,1	,-1	,-1,-1);
	add_inst(INST_CMPS_DS_ESI_ES_EDI	,0xa7	,1	,-1	,-1,-1);
	add_inst(INST_MOV		,0xb0	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_MOV		,0xb1	,1	,-1	,REG_CL	,Ib);
	add_inst(INST_MOV		,0xb2	,1	,-1	,REG_DL	,Ib);
	add_inst(INST_MOV		,0xb3	,1	,-1	,REG_BL	,Ib);
	add_inst(INST_MOV		,0xb4	,1	,-1	,REG_AH	,Ib);
	add_inst(INST_MOV		,0xb5	,1	,-1	,REG_CH	,Ib);
	add_inst(INST_MOV		,0xb6	,1	,-1	,REG_DH	,Ib);
	add_inst(INST_MOV		,0xb7	,1	,-1	,REG_BH	,Ib);
	add_inst(INST_MOV		,0xb8	,1	,-1	,REG_EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xb9	,1	,-1	,REG_ECX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xba	,1	,-1	,REG_EDX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xbb	,1	,-1	,REG_EBX	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xbc	,1	,-1	,REG_ESP	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xbd	,1	,-1	,REG_EBP	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xbe	,1	,-1	,REG_ESI	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xbf	,1	,-1	,REG_EDI	,Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV		,0xb8	,1	,-1	,REG_AX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xb9	,1	,-1	,REG_CX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xba	,1	,-1	,REG_DX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xbb	,1	,-1	,REG_BX	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xbc	,1	,-1	,REG_SP	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xbd	,1	,-1	,REG_BP	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xbe	,1	,-1	,REG_SI	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xbf	,1	,-1	,REG_DI	,Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV		,0xb8	,1	,-1	,REG_RAX	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xb9	,1	,-1	,REG_RCX	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xba	,1	,-1	,REG_RDX	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xbb	,1	,-1	,REG_RBX	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xbc	,1	,-1	,REG_RSP	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xbd	,1	,-1	,REG_RBP	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xbe	,1	,-1	,REG_RSI	,Iq, OPT_BIG_PARAM);
	add_inst(INST_MOV		,0xbf	,1	,-1	,REG_RDI	,Iq, OPT_BIG_PARAM);
	// Shift Group 2
	add_inst(INST_ROL		,0xc0	,1	,0	,Eb	,Ib);
	add_inst(INST_ROR		,0xc0	,1	,1	,Eb	,Ib);
	add_inst(INST_RCL		,0xc0	,1	,2	,Eb	,Ib);
	add_inst(INST_RCR		,0xc0	,1	,3	,Eb	,Ib);
	add_inst(INST_SHL		,0xc0	,1	,4	,Eb	,Ib);
	add_inst(INST_SHR		,0xc0	,1	,5	,Eb	,Ib);
	add_inst(INST_SAR		,0xc0	,1	,7	,Eb	,Ib);
	add_inst(INST_ROL,	0xc1,	1,	0,	Ew,	Ib, OPT_SMALL_PARAM); // even though the table says Iv
	add_inst(INST_ROL,	0xc1,	1,	0,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_ROL,	0xc1,	1,	0,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_ROR,	0xc1,	1,	1,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_ROR,	0xc1,	1,	1,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_ROR,	0xc1,	1,	1,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_RCL,	0xc1,	1,	2,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_RCL,	0xc1,	1,	2,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_RCL,	0xc1,	1,	2,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_RCR,	0xc1,	1,	3,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_RCR,	0xc1,	1,	3,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_RCR,	0xc1,	1,	3,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_SHL,	0xc1,	1,	4,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_SHL,	0xc1,	1,	4,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_SHL,	0xc1,	1,	4,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_SHR,	0xc1,	1,	5,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_SHR,	0xc1,	1,	5,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_SHR,	0xc1,	1,	5,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_SAR,	0xc1,	1,	7,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(INST_SAR,	0xc1,	1,	7,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(INST_SAR,	0xc1,	1,	7,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(INST_RET		,0xc2	,1	,-1	,Iw	,-1);
	add_inst(INST_RET		,0xc3	,1	,-1	,-1	,-1);
	add_inst(INST_MOV		,0xc6	,1	,-1	,Eb	,Ib);
	add_inst(INST_MOV,	0xc7,	1,	-1,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_MOV,	0xc7,	1,	-1,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_MOV,	0xc7,	1,	-1,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_LEAVE		,0xc9	,1	,-1	,-1	,-1);
	add_inst(INST_RET_FAR	,0xca	,1	,-1	,Iw	,-1);
	add_inst(INST_RET_FAR	,0xcb	,1	,-1	,-1	,-1);
	add_inst(INST_INT		,0xcd	,1	,-1	,Ib	,-1);
	add_inst(INST_IRET		,0xcf	,1	,-1	,-1	,-1);
	add_inst(INST_ROL,	0xd3,	1,	0,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_ROL,	0xd3,	1,	0,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_ROL,	0xd3,	1,	0,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_ROR,	0xd3,	1,	1,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_ROR,	0xd3,	1,	1,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_ROR,	0xd3,	1,	1,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_RCL,	0xd3,	1,	2,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_RCL,	0xd3,	1,	2,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_RCL,	0xd3,	1,	2,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_RCR,	0xd3,	1,	3,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_RCR,	0xd3,	1,	3,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_RCR,	0xd3,	1,	3,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_SHL,	0xd3,	1,	4,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_SHL,	0xd3,	1,	4,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_SHL,	0xd3,	1,	4,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_SHR,	0xd3,	1,	5,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_SHR,	0xd3,	1,	5,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_SHR,	0xd3,	1,	5,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_SAR,	0xd3,	1,	7,	Ew,	REG_CL, OPT_SMALL_PARAM);
	add_inst(INST_SAR,	0xd3,	1,	7,	Ed,	REG_CL, OPT_MEDIUM_PARAM);
	add_inst(INST_SAR,	0xd3,	1,	7,	Eq,	REG_CL, OPT_BIG_PARAM);
	add_inst(INST_FADD,	0xd8,	1,	0,	Ed,	-1);
	add_inst(INST_FADD,	0xdc,	1,	0,	Eq,	-1);
	add_inst(INST_FMUL,	0xd8,	1,	1,	Ed,	-1);
	add_inst(INST_FMUL,	0xdc,	1,	1,	Eq,	-1);
	add_inst(INST_FSUB,	0xd8,	1,	4,	Ed,	-1);
	add_inst(INST_FSUB,	0xdc,	1,	4,	Eq,	-1);
	add_inst(INST_FDIV,	0xd8,	1,	6,	Ed,	-1);
	add_inst(INST_FDIV,	0xdc,	1,	6,	Eq,	-1);
	add_inst(INST_FLD,	0xd9,	1,	0,	Md,	-1);
	add_inst(INST_FLD,	0xdd,	1,	0,	Mq,	-1);
	add_inst(INST_FLD1,	0xe8d9,	2,	-1,	-1,	-1);
	add_inst(INST_FLDZ,	0xeed9,	2,	-1,	-1,	-1);
	add_inst(INST_FLDPI,	0xebd9,	2,	-1,	-1,	-1);
	add_inst(INST_FST,	0xd9,	1,	2,	Md,	-1);
	add_inst(INST_FST,	0xdd,	1,	2,	Mq,	-1);
	add_inst(INST_FSTP,	0xd9,	1,	3,	Md,	-1);
	add_inst(INST_FSTP,	0xdd,	1,	3,	Mq,	-1);
	add_inst(INST_FLDCW,	0xd9,	1,	5,	Mw,	-1);
	add_inst(INST_FNSTCW,	0xd9,	1,	7,	Mw,	-1);
	add_inst(INST_FXCH		,0xc9d9	,2	,-1	,REG_ST0	,REG_ST1);
	add_inst(INST_FUCOMPP	,0xe9da	,2	,-1	,REG_ST0	,REG_ST1);

	add_inst(INST_FSQRT,	0xfad9,	2,	-1,	-1, -1);
	add_inst(INST_FSIN,	0xfed9,	2,	-1,	-1, -1);
	add_inst(INST_FCOS,	0xffd9,	2,	-1,	-1, -1);
	add_inst(INST_FPTAN,	0xf2d9,	2,	-1,	-1, -1);
	add_inst(INST_FPATAN,	0xf3d9,	2,	-1,	-1, -1);
	add_inst(INST_FYL2X,	0xf1d9,	2,	-1,	-1, -1);
	add_inst(INST_FISTP,	0xdb	,1	,3	,Md	,-1);
	add_inst(INST_FILD,	0xdb,	1,	0,	Ed,	-1);
	add_inst(INST_FADDP,	0xde,	1,	0,	Ed,	-1);
	add_inst(INST_FMULP,	0xde,	1,	1,	Ed,	-1);
	add_inst(INST_FSUBP,	0xde,	1,	5,	Ed,	-1);
	add_inst(INST_FDIVP,	0xde,	1,	7,	Ed,	-1); // de.f9 ohne Parameter...?
	add_inst(INST_FNSTSW	,0xe0df	,2	,-1	,REG_AX	,-1);
	add_inst(INST_LOOPNE	,0xe0	,1	,-1	,Jb	,-1);
	add_inst(INST_LOOPE		,0xe1	,1	,-1	,Jb	,-1);
	add_inst(INST_LOOP		,0xe2	,1	,-1	,Jb	,-1);
	add_inst(INST_IN		,0xe4	,1	,-1	,REG_AL	,Ib);
	add_inst(INST_IN		,0xe5	,1	,-1	,REG_EAX,Ib);
	add_inst(INST_OUT		,0xe6	,1	,-1	,Ib	,REG_AL);
	add_inst(INST_OUT		,0xe7	,1	,-1	,Ib	,REG_EAX);
	add_inst(INST_CALL,	0xe8,	1,	-1,	Jw,	-1, OPT_SMALL_PARAM); // well... "Av" in tyble
	add_inst(INST_CALL,	0xe8,	1,	-1,	Jd,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_CALL,	0xe8,	1,	-1,	Jq,	-1, OPT_BIG_PARAM);
	add_inst(INST_JMP,	0xe9,	1,	-1,	Jw,	-1, OPT_SMALL_PARAM); // miswritten in the table
	add_inst(INST_JMP,	0xe9,	1,	-1,	Jd,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_JMP,	0xe9,	1,	-1,	Jq,	-1, OPT_BIG_PARAM);
//	add_inst(inst_jmp		,0xea	,1	,-1, Ap, -1); TODO
	add_inst(INST_JMP_FAR, 0xea, 1, -1, Id, -1, OPT_SMALL_PARAM);
	add_inst(INST_JMP_FAR, 0xea, 1, -1, I48, -1, OPT_MEDIUM_PARAM);
	add_inst(INST_JMP		,0xeb	,1	,-1, Jb, -1);
	add_inst(INST_IN		,0xec	,1	,-1, REG_AL, REG_DX);
	add_inst(INST_IN		,0xed	,1	,-1, REG_EAX, REG_DX);
	add_inst(INST_OUT		,0xee	,1	,-1, REG_DX, REG_AL);
	add_inst(INST_OUT		,0xef	,1	,-1, REG_DX, REG_EAX);
	add_inst(INST_LOCK		,0xf0	,1	,-1	,-1	,-1);
	/*add_inst(inst_repne		,0xf2	,1	,-1	,-1	,-1);
	add_inst(inst_rep		,0xf3	,1	,-1	,-1	,-1);*/
	add_inst(INST_HLT		,0xf4	,1	,-1	,-1	,-1);
	add_inst(INST_CMC		,0xf5	,1	,-1	,-1	,-1);
	// Unary Group 3
	add_inst(INST_TEST		,0xf6	,1	,0	,Eb	,Ib);
	add_inst(INST_NOT		,0xf6	,1	,2	,Eb	,-1);
	add_inst(INST_NEG		,0xf6	,1	,3	,Eb	,-1);
	add_inst(INST_MUL		,0xf6	,1	,4	,REG_AL	,Eb);
	add_inst(INST_IMUL		,0xf6	,1	,5	,REG_AL	,Eb);
	add_inst(INST_DIV		,0xf6	,1	,6	,REG_AL	,Eb);
	add_inst(INST_IDIV		,0xf6	,1	,7	,Eb	,-1);
	add_inst(INST_TEST,	0xf7,	1,	0,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(INST_TEST,	0xf7,	1,	0,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(INST_TEST,	0xf7,	1,	0,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(INST_NOT,	0xf7,	1,	2,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(INST_NOT,	0xf7,	1,	2,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_NOT,	0xf7,	1,	2,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_NEG,	0xf7,	1,	3,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(INST_NEG,	0xf7,	1,	3,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_NEG,	0xf7,	1,	3,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_MUL		,0xf7	,1	,4	,REG_EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_IMUL		,0xf7	,1	,5	,REG_EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_DIV		,0xf7	,1	,6	,REG_EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_IDIV		,0xf7	,1	,7	,REG_EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(INST_MUL		,0xf7	,1	,4	,REG_AX	,Ew, OPT_SMALL_PARAM);
	add_inst(INST_IMUL		,0xf7	,1	,5	,REG_AX	,Ew, OPT_SMALL_PARAM);
	add_inst(INST_DIV		,0xf7	,1	,6	,REG_AX	,Ew, OPT_SMALL_PARAM);
	add_inst(INST_IDIV		,0xf7	,1	,7	,REG_AX	,Ew, OPT_SMALL_PARAM);
	add_inst(INST_MUL		,0xf7	,1	,4	,REG_RAX	,Eq, OPT_BIG_PARAM);
	add_inst(INST_IMUL		,0xf7	,1	,5	,REG_RAX	,Eq, OPT_BIG_PARAM);
	add_inst(INST_DIV		,0xf7	,1	,6	,REG_RAX	,Eq, OPT_BIG_PARAM);
	add_inst(INST_IDIV		,0xf7	,1	,7	,REG_RAX	,Eq, OPT_BIG_PARAM);
	add_inst(INST_CLC		,0xf8	,1	,-1	,-1	,-1);
	add_inst(INST_STC		,0xf9	,1	,-1	,-1	,-1);
	add_inst(INST_CLI		,0xfa	,1	,-1	,-1	,-1);
	add_inst(INST_STI		,0xfb	,1	,-1	,-1	,-1);
	add_inst(INST_CLD		,0xfc	,1	,-1	,-1	,-1);
	add_inst(INST_STD		,0xfd	,1	,-1	,-1	,-1);
	add_inst(INST_INC		,0xfe	,1	,0	,Eb	,-1);
	add_inst(INST_DEC		,0xfe	,1	,1	,Eb	,-1);
	add_inst(INST_INC,	0xff,	1,	0,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(INST_INC,	0xff,	1,	0,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_INC,	0xff,	1,	0,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_DEC,	0xff,	1,	1,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(INST_DEC,	0xff,	1,	1,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_DEC,	0xff,	1,	1,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_CALL,	0xff,	1,	2,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(INST_CALL,	0xff,	1,	2,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_CALL,	0xff,	1,	2,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_CALL_FAR,	0xff,	1,	3,	Ew,	-1, OPT_SMALL_PARAM); // Ep instead of Ev...
	add_inst(INST_CALL_FAR,	0xff,	1,	3,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(INST_CALL_FAR,	0xff,	1,	3,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(INST_JMP, 0xff, 1,	4, Ew, -1, OPT_SMALL_PARAM);
	add_inst(INST_JMP, 0xff, 1,	4, Ed, -1, OPT_MEDIUM_PARAM);
	add_inst(INST_JMP, 0xff, 1,	4, Eq, -1, OPT_BIG_PARAM);
	add_inst(INST_JMP_FAR, 0xff, 1, 5, Ed, -1, OPT_SMALL_PARAM);
	add_inst(INST_JMP_FAR, 0xff, 1, 5, E48, -1, OPT_MEDIUM_PARAM);
	add_inst(INST_PUSH, 0xff, 1, 6, Ew, -1, OPT_SMALL_PARAM);
	add_inst(INST_PUSH, 0xff, 1, 6, Ed, -1, OPT_MEDIUM_PARAM);
	add_inst(INST_PUSH, 0xff, 1, 6, Eq, -1, OPT_BIG_PARAM);

	// sse
	add_inst(INST_MOVSS,  0x100ff3, 3, -1, Xx, XMd);
	add_inst(INST_MOVSS,  0x110ff3, 3, -1, XMd, Xx);
	add_inst(INST_MOVSD,  0x100ff2, 3, -1, Xx, XMq);
	add_inst(INST_MOVSD,  0x110ff2, 3, -1, XMq, Xx);
	add_inst(INST_MOVUPS, 0x100f,   2, -1, Xx, XMdq);
	add_inst(INST_MOVUPS, 0x110f,   2, -1, XMdq, Xx);
	add_inst(INST_MOVAPS, 0x280f,   2, -1, Xx, XMdq);
	add_inst(INST_MOVAPS, 0x290f,   2, -1, XMdq, Xx);
	add_inst(INST_ADDSS,  0x580ff3, 3, -1, Xx, XMd);
	add_inst(INST_ADDSD,  0x580ff2, 3, -1, Xx, XMq);
	add_inst(INST_ADDPS,  0x580f,   2, -1, Xx, XMdq);
	add_inst(INST_SUBSS,  0x5c0ff3, 3, -1, Xx, XMd);
	add_inst(INST_SUBSD,  0x5c0ff2, 3, -1, Xx, XMq);
	add_inst(INST_MULSS,  0x590ff3, 3, -1, Xx, XMd);
	add_inst(INST_MULSD,  0x590ff2, 3, -1, Xx, XMq);
	add_inst(INST_DIVSS,  0x5e0ff3, 3, -1, Xx, XMd);
	add_inst(INST_DIVSD,  0x5e0ff2, 3, -1, Xx, XMq);
	add_inst(INST_SQRTSS, 0x510ff3, 3, -1, Xx, XMd);
	add_inst(INST_SQRTSD, 0x510ff2, 3, -1, Xx, XMq);
	add_inst(INST_MINSS,  0x5d0ff3, 3, -1, Xx, XMd);
	add_inst(INST_MINSD,  0x5d0ff2, 3, -1, Xx, XMq);
	add_inst(INST_MAXSS,  0x5f0ff3, 3, -1, Xx, XMd);
	add_inst(INST_MAXSD,  0x5f0ff2, 3, -1, Xx, XMq);
	add_inst(INST_CVTTSS2SI, 0x2c0ff3, 3, -1, Rd, XMd);
	add_inst(INST_CVTTSD2SI, 0x2c0ff2, 3, -1, Rq, XMd);
	add_inst(INST_CVTSI2SS,  0x2a0ff3, 3, -1, Xx, Ed);
	add_inst(INST_CVTSI2SD,  0x2a0ff2, 3, -1, Xx, Eq);
	add_inst(INST_COMISS,    0x2f0f,   2, -1, Xx, XMd);
	add_inst(INST_COMISD,    0x2f0f66, 3, -1, Xx, XMq);
	add_inst(INST_UCOMISS,   0x2e0f,   2, -1, Xx, XMd);
	add_inst(INST_UCOMISD,   0x2e0f66, 3, -1, Xx, XMq);
}



void Init(int set)
{
	if (set < 0)
		set = QueryLocalInstructionSet();

	InstructionSet.set = set;
	InstructionSet.pointer_size = 4;
	if (set == INSTRUCTION_SET_AMD64)
		InstructionSet.pointer_size = 8;

	for (int i=0;i<NUM_REG_ROOTS;i++)
		for (int j=0;j<=MAX_REG_SIZE;j++)
			RegResize[i][j] = -1;


	for (int i=0;i<Asm::NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst != i)
			msg_error(string(InstructionNames[i].name) + "  " + i2s(InstructionNames[i].inst) + "  !=   " + i2s(i));

	if (set == INSTRUCTION_SET_ARM)
		InitARM();
	else
		InitX86();
}

InstructionParam::InstructionParam()
{
	type = PARAMT_NONE;
	disp = DISP_MODE_NONE;
	reg = NULL;
	reg2 = NULL;
	deref = false;
	size = SIZE_UNKNOWN;
	value = 0;
	is_label = false;
	write_back = false;
}

// convert an asm parameter into a human readable expression
string InstructionParam::str(bool hide_size)
{
	//msg_write("----");
	//msg_write(p.type);
	if (type == PARAMT_INVALID){
		return "-\?\?\?-";
	}else if (type == PARAMT_NONE){
		return "";
	}else if (type == PARAMT_REGISTER){
		string post;
		if (write_back)
			post = "!";
			//msg_write((long)reg);
			//msg_write((long)disp);
		if (deref){
			//msg_write("deref");
			string ss;
			if (!hide_size)
				ss = get_size_name(size) + " ";
			string s = reg->name;
			if (disp == DISP_MODE_8){
				if (value > 0)
					s += format("+0x%02x", (value & 0xff));
				else
					s += format("-0x%02x", ((-value) & 0xff));
			}else if (disp == DISP_MODE_16)
				s += format("+0x%04x", (value & 0xffff));
			else if (disp == DISP_MODE_32)
				s += format("+0x%08x", value);
			else if (disp == DISP_MODE_SIB)
				return "SIB[...][...]";
			else if (disp == DISP_MODE_8_SIB)
				s += format("::SIB...+0x%02x", value);
			else if (disp == DISP_MODE_8_REG2)
				s += format("%s+0x%02x", reg2->name.c_str(), value);
			else if (disp == DISP_MODE_REG2)
				s += "+" + reg2->name;
			return ss + "[" + s + "]";
		}else
			return reg->name + post;
	}else if (type == PARAMT_REGISTER_SET){
		Array<string> s;
		for (int i=0; i<16; i++)
			if (value & (1<<i))
				s.add(RegisterByID[REG_R0 + i]->name);
		return "{" + implode(s, ",") + "}";
	}else if (type == PARAMT_IMMEDIATE){
		string s = d2h(&value, deref ? state.AddrSize : size);
		if (is_label)
			s = state.get_label(value);
		if (deref)
			return get_size_name(size) + " [" + s + "]";
		return s;
	/*}else if (type == ParamTImmediateExt){
		//msg_write("im");
		return format("%s:%s", d2h(&((char*)&value)[4], 2).c_str(), d2h(&value, state.ParamSize).c_str());*/
	}
	return "\?\?\?";
}

string ARMConditions[16] = {
	"eq",
	"ne",
	"cs",
	"cc",
	"mi",
	"pl",
	"vs",
	"vc",
	"hi",
	"ls",
	"ge",
	"lt",
	"gt",
	"le",
	"al",
	"???",
};

string InstructionWithParams::str(bool hide_size)
{
	string s;
	if (condition != ARM_COND_ALWAYS)
		s += ARMConditions[condition & 0xf] + ":";
	s += GetInstructionName(inst);
	s += "  " + p[0].str(hide_size);
	if (p[1].type != PARAMT_NONE)
		s += ",  " + p[1].str(hide_size);
	if (p[2].type != PARAMT_NONE)
		s += ",  " + p[2].str(hide_size);
	return s;
}

inline void UnfuzzyParam(InstructionParam &p, InstructionParamFuzzy &pf)
{
	p.type = pf._type_;
	p.reg2 = NULL;
	p.disp = DISP_MODE_NONE;
	p.reg = pf.reg;
	if ((p.reg) and (state.ExtendModRMBase)){
		if ((p.reg->id >= REG_RAX) and (p.reg->id <= REG_RBP))
			p.reg = RegisterByID[p.reg->id + REG_R8 - REG_RAX];
	}
	p.size = pf.size;
	p.deref = false; // well... FIXME
	p.value = 0;
	p.is_label = false;
	if (pf._type_ == PARAMT_MEMORY){
		p.type = PARAMT_IMMEDIATE;
		p.deref = true;
	}
}

int GetModRMRegister(int reg, int size, int group)
{
	if (group == REG_GROUP_XMM)
		return REG_XMM0 + reg;
	if (size == SIZE_8){
		if (reg == 0x00)	return REG_AL;
		if (reg == 0x01)	return REG_CL;
		if (reg == 0x02)	return REG_DL;
		if (reg == 0x03)	return REG_BL;
		if (reg == 0x04)	return REG_AH;
		if (reg == 0x05)	return REG_CH;
		if (reg == 0x06)	return REG_DH;
		if (reg == 0x07)	return REG_BH;
	}else if (size == SIZE_16){
		if (reg == 0x00)	return REG_AX;
		if (reg == 0x01)	return REG_CX;
		if (reg == 0x02)	return REG_DX;
		if (reg == 0x03)	return REG_BX;
		if (reg == 0x04)	return REG_SP;
		if (reg == 0x05)	return REG_BP;
		if (reg == 0x06)	return REG_SI;
		if (reg == 0x07)	return REG_DI;
	}else if (size == SIZE_32){
		if (reg == 0x00)	return REG_EAX;
		if (reg == 0x01)	return REG_ECX;
		if (reg == 0x02)	return REG_EDX;
		if (reg == 0x03)	return REG_EBX;
		if (reg == 0x04)	return REG_ESP;
		if (reg == 0x05)	return REG_EBP;
		if (reg == 0x06)	return REG_ESI;
		if (reg == 0x07)	return REG_EDI;
		if (reg == 0x08)	return REG_R8D;
		if (reg == 0x09)	return REG_R9D;
		if (reg == 0x0a)	return REG_R10D;
		if (reg == 0x0b)	return REG_R11D;
		if (reg == 0x0c)	return REG_R12D;
		if (reg == 0x0d)	return REG_R13D;
		if (reg == 0x0e)	return REG_R14D;
		if (reg == 0x0f)	return REG_R15D;
	}else if (size == SIZE_64){
		if (reg == 0x00)	return REG_RAX;
		if (reg == 0x01)	return REG_RCX;
		if (reg == 0x02)	return REG_RDX;
		if (reg == 0x03)	return REG_RBX;
		if (reg == 0x04)	return REG_RSP;
		if (reg == 0x05)	return REG_RBP;
		if (reg == 0x06)	return REG_RSI;
		if (reg == 0x07)	return REG_RDI;
		if (reg == 0x08)	return REG_R8;
		if (reg == 0x09)	return REG_R9;
		if (reg == 0x0a)	return REG_R10;
		if (reg == 0x0b)	return REG_R11;
		if (reg == 0x0c)	return REG_R12;
		if (reg == 0x0d)	return REG_R13;
		if (reg == 0x0e)	return REG_R14;
		if (reg == 0x0f)	return REG_R15;
	}
	msg_error("unhandled mod/rm register: " + i2s(reg) + " (size " + i2s(size) + ")");
	return 0;
}

inline void GetFromModRM(InstructionParam &p, InstructionParamFuzzy &pf, unsigned char modrm)
{
	if (pf.mrm_mode == MRM_REG){
		unsigned char reg = modrm & 0x38; // bits 5, 4, 3
		p.type = PARAMT_REGISTER;
		p.deref = false;
		if (pf.reg_group == REG_GROUP_SEGMENT){
			if (reg == 0x00)	p.reg = RegisterByID[REG_ES];
			if (reg == 0x08)	p.reg = RegisterByID[REG_CS];
			if (reg == 0x10)	p.reg = RegisterByID[REG_SS];
			if (reg == 0x18)	p.reg = RegisterByID[REG_DS];
			if (reg == 0x20)	p.reg = RegisterByID[REG_FS];
			if (reg == 0x28)	p.reg = RegisterByID[REG_GS];
		}else if (pf.reg_group == REG_GROUP_CONTROL){
			if (reg == 0x00)	p.reg = RegisterByID[REG_CR0];
			if (reg == 0x08)	p.reg = RegisterByID[REG_CR1];
			if (reg == 0x10)	p.reg = RegisterByID[REG_RC2];
			if (reg == 0x18)	p.reg = RegisterByID[REG_CR3];
			if (reg == 0x20)	p.reg = RegisterByID[REG_CR4];
		}else if (pf.reg_group == REG_GROUP_XMM){
			p.reg = RegisterByID[REG_XMM0 + (reg >> 3)];
		}else{
			reg = (reg >> 3) | (state.ExtendModRMReg ? 0x08 : 0x00);
			p.reg = RegisterByID[GetModRMRegister(reg, p.size, REG_GROUP_GENERAL)];
		}
	}else if (pf.mrm_mode == MRM_MOD_RM){
		unsigned char mod = modrm & 0xc0; // bits 7, 6
		unsigned char rm = modrm & 0x07; // bits 2, 1, 0
		if (state.ExtendModRMBase)	rm |= 0x08;
		if (mod == 0x00){
			if (state.AddrSize == SIZE_16){
				p.type = PARAMT_REGISTER;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[REG_BX];	p.reg2 = RegisterByID[REG_SI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x01){p.reg = RegisterByID[REG_BX];	p.reg2 = RegisterByID[REG_DI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x02){p.reg = RegisterByID[REG_BP];	p.reg2 = RegisterByID[REG_SI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x03){p.reg = RegisterByID[REG_BP];	p.reg2 = RegisterByID[REG_DI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x04)	p.reg = RegisterByID[REG_SI];
				if (rm == 0x05)	p.reg = RegisterByID[REG_DI];
				if (rm == 0x06){p.reg = NULL;	p.type = PARAMT_IMMEDIATE;	}
				if (rm == 0x07)	p.reg = RegisterByID[REG_BX];
			}else{
				p.type = PARAMT_REGISTER;
				p.deref = true;
				//if (rm == 0x04){p.reg = NULL;	p.disp = DispModeSIB;	p.type = ParamTImmediate;}//p.type = ParamTInvalid;	Error("kein SIB byte...");}
				if (rm == 0x04){p.reg = RegisterByID[REG_EAX];	p.disp = DISP_MODE_SIB;	} // eax = provisoric
				else if (rm == 0x05){p.reg = NULL;	p.type = PARAMT_IMMEDIATE;	}
				else
					p.reg = RegisterByID[GetModRMRegister(rm, SIZE_32, REG_GROUP_GENERAL)];
			}
		}else if ((mod == 0x40) or (mod == 0x80)){
			if (state.AddrSize == SIZE_16){
				p.type = PARAMT_REGISTER;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[REG_BX];	p.reg2 = RegisterByID[REG_SI];	p.disp = (mod == 0x40) ? DISP_MODE_8_REG2 : DISP_MODE_16_REG2;	}
				if (rm == 0x01){p.reg = RegisterByID[REG_BX];	p.reg2 = RegisterByID[REG_DI];	p.disp = (mod == 0x40) ? DISP_MODE_8_REG2 : DISP_MODE_16_REG2;	}
				if (rm == 0x02){p.reg = RegisterByID[REG_BP];	p.reg2 = RegisterByID[REG_SI];	p.disp = (mod == 0x40) ? DISP_MODE_8_REG2 : DISP_MODE_16_REG2;	}
				if (rm == 0x03){p.reg = RegisterByID[REG_BP];	p.reg2 = RegisterByID[REG_DI];	p.disp = (mod == 0x40) ? DISP_MODE_8_REG2 : DISP_MODE_16_REG2;	}
				if (rm == 0x04){p.reg = RegisterByID[REG_SI];	p.disp = (mod == 0x40) ? DISP_MODE_8 : DISP_MODE_16;	}
				if (rm == 0x05){p.reg = RegisterByID[REG_DI];	p.disp = (mod == 0x40) ? DISP_MODE_8 : DISP_MODE_16;	}
				if (rm == 0x06){p.reg = RegisterByID[REG_BP];	p.disp = (mod == 0x40) ? DISP_MODE_8 : DISP_MODE_16;	}
				if (rm == 0x07){p.reg = RegisterByID[REG_BX];	p.disp = (mod == 0x40) ? DISP_MODE_8 : DISP_MODE_16;	}
			}else{
				p.type = PARAMT_REGISTER;
				p.deref = true;
				p.disp = (mod == 0x40) ? DISP_MODE_8 : DISP_MODE_32;
				//if (rm == 0x04){p.reg = NULL;	p.type = ParamTInvalid;	}
				if (rm == 0x04){p.reg = RegisterByID[REG_EAX];	p.disp = DISP_MODE_8_SIB;	} // eax = provisoric
				else
					p.reg = RegisterByID[GetModRMRegister(rm, SIZE_32, REG_GROUP_GENERAL)];
			}
		}else if (mod == 0xc0){
			p.type = PARAMT_REGISTER;
			p.deref = false;
			if (state.ExtendModRMBase)	rm |= 0x08;
			p.reg = RegisterByID[GetModRMRegister(rm, p.size, pf.reg_group)];
		}
	}
}

inline void TryGetSIB(InstructionParam &p, char *&cur)
{
	if ((p.disp == DISP_MODE_SIB) or (p.disp == DISP_MODE_8_SIB)){
		bool disp8 = (p.disp == DISP_MODE_8_SIB);
		char sib = *cur;
		cur++;
		unsigned char ss = (sib & 0xc0); // bits 7, 6
		unsigned char index = (sib & 0x38); // bits 5, 4, 3
		unsigned char base = (sib & 0x07); // bits 2, 1, 0
		/*msg_error("SIB");
		msg_write(ss);
		msg_write(index);
		msg_write(base);*/

		// direct?
		//if (p.disp == DispModeSIB){
			if (ss == 0x00){ // scale factor 1
				p.deref = true;
				p.disp = disp8 ? DISP_MODE_8_REG2 : DISP_MODE_REG2;
				if (base == 0x00)		p.reg = RegisterByID[REG_EAX];
				else if (base == 0x01)	p.reg = RegisterByID[REG_ECX];
				else if (base == 0x02)	p.reg = RegisterByID[REG_EDX];
				else if (base == 0x03)	p.reg = RegisterByID[REG_EBX];
				else if (base == 0x04)	p.reg = RegisterByID[REG_ESP];
				else p.disp = DISP_MODE_SIB; // ...
				if (index == 0x00)		p.reg2 = RegisterByID[REG_EAX];
				else if (index == 0x08)	p.reg2 = RegisterByID[REG_ECX];
				else if (index == 0x10)	p.reg2 = RegisterByID[REG_EDX];
				else if (index == 0x18)	p.reg2 = RegisterByID[REG_EBX];
				else if (index == 0x28)	p.reg2 = RegisterByID[REG_EBP];
				else if (index == 0x30)	p.reg2 = RegisterByID[REG_ESI];
				else if (index == 0x38)	p.reg2 = RegisterByID[REG_EDI];
				else p.disp = disp8 ? DISP_MODE_8 : DISP_MODE_NONE;
			}
		//}
	}
}

inline void ReadParamData(char *&cur, InstructionParam &p, bool has_modrm)
{
	//char *o = cur;
	p.value = 0;
	if (p.type == PARAMT_IMMEDIATE){
		if (p.deref){
			int size = has_modrm ? state.AddrSize : state.FullRegisterSize; // Ov/Mv...
			memcpy(&p.value, cur, size);
			cur += size;
		}else{
			memcpy(&p.value, cur, p.size);
			cur += p.size;
		}
	/*}else if (p.type == ParamTImmediateExt){
		if (state.ParamSize == Size16){ // addr?
			*(short*)&p.value = *(short*)cur;	cur += 2;	((short*)&p.value)[2] = *(short*)cur;	cur += 2;
		}else{
			memcpy(&p.value, cur, 6);		cur += 6;
		}*/
	}else if (p.type == PARAMT_REGISTER){
		if ((p.disp == DISP_MODE_8) or (p.disp == DISP_MODE_8_REG2) or (p.disp == DISP_MODE_8_SIB)){
			*(char*)&p.value = *cur;		cur ++;
		}else if (p.disp == DISP_MODE_16){
			*(short*)&p.value = *(short*)cur;		cur += 2;
		}else if (p.disp == DISP_MODE_32){
			*(int*)&p.value = *(int*)cur;		cur += 4;
		}
	}
	//msg_write((long)cur - (long)o);
}

string show_reg(int r)
{
	return format("r%d", r);
}

int ARM_DATA_INSTRUCTIONS[16] =
{
	INST_AND,
	INST_XOR,
	INST_SUB,
	INST_RSB,
	INST_ADD,
	INST_ADC,
	INST_SBC,
	INST_RSC,
	INST_TST,
	INST_TEQ,
	INST_CMP,
	INST_CMN,
	INST_OR,
	INST_MOV,
	INST_BIC,
	INST_MVN
};

int ARM_VFP_PRIMARY_INSTRUCTIONS[16] =
{
	INST_FMACS,
	INST_FNMACS,
	INST_FMSCS,
	INST_FNMSCS,
	INST_FMULS,
	INST_FNMULS,
	INST_FADDS,
	INST_FSUBS,
	INST_FDIVS,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1
};

int ARM_VFP_EXTENSION_INSTRUCTIONS[32] =
{
	INST_FCPYS,
	INST_FABSS,
	INST_FNEGS,
	INST_FSQRTS,
	-1,
	-1,
	-1,
	-1,
	INST_FCMPS,
	INST_FCMPES,
	INST_FCMPZS,
	INST_FCMPEZS,
	-1,
	-1,
	-1,
	INST_CVTDS,

	INST_FUITOS,
	INST_FSITOS,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	INST_FTOUIS,
	INST_FTOUIZS,
	INST_FTOSIS,
	INST_FTOSIZS,
	-1,
	-1,
	-1,
	-1
};

int arm_decode_imm(int imm)
{
	int r = ((imm >> 8) & 0xf);
	int n = (imm & 0xff);
	return n >> (r*2) | n << (32 - r*2);
}

InstructionParam disarm_shift_reg(int code)
{
	InstructionParam p = param_reg(REG_R0 + (code & 0xf));
	bool by_reg = (code >> 4) & 0x1;
	int r = ((code >> 7) & 0x1f);
	if (!by_reg and r == 0)
		return p;
	/*if (((code >> 5) & 0x3) == 0)
		s += "<<";
	else
		s += ">>";
	if (by_reg){
		s += show_reg((code >> 8) & 0xf);
	}else{
		s += format("%d", r);
	}*/
	return p;
}

InstructionWithParams disarm_data_opcode(int code)
{
	InstructionWithParams i;
	i.inst = ARM_DATA_INSTRUCTIONS[(code >> 21) & 15];
	if (((code >> 20) & 1) and (i.inst != INST_CMP) and (i.inst != INST_CMN) and (i.inst != INST_TEQ) and (i.inst != INST_TST))
		msg_write(GetInstructionName(i.inst) + "[S]");
	i.p[0] = param_reg(REG_R0 + ((code >> 12) & 15));
	i.p[1] = param_reg(REG_R0 + ((code >> 16) & 15));
	if ((code >> 25) & 1)
		i.p[2] = param_imm(arm_decode_imm(code & 0xfff), SIZE_32);
	else
		i.p[2] = disarm_shift_reg(code & 0xfff);
	if ((i.inst == INST_CMP) or (i.inst == INST_CMN) or (i.inst == INST_TST) or (i.inst == INST_TEQ) or (i.inst == INST_MOV)){
		if ((i.inst == INST_CMP) or (i.inst == INST_CMN))
			i.p[0] = i.p[1];
		i.p[1] = i.p[2];
		i.p[2] = param_none;
	}
	return i;
}

InstructionWithParams disarm_data_opcode_mul(int code)
{
	InstructionWithParams i;
	i.inst = INST_MUL;
	if ((code >> 20) & 1)
		msg_write(" [S]");
	i.p[0] = param_reg(REG_R0 + ((code >> 16) & 15));
	i.p[1] = param_reg(REG_R0 + ((code >> 0) & 15));
	i.p[2] = param_reg(REG_R0 + ((code >> 8) & 15));
	return i;
}

InstructionWithParams disarm_branch(int code)
{
	InstructionWithParams i;
	if ((code >> 24) & 1)
		i.inst = INST_BL;
	else
		i.inst = INST_B;
	i.p[0] = param_imm(code & 0x00ffffff, SIZE_32);
	i.p[1] = param_none;
	i.p[2] = param_none;
	return i;
}

InstructionWithParams disarm_blx(int code)
{
	InstructionWithParams i;
	i.inst = INST_BLX;
	i.p[0] = param_reg(REG_R0 + ((code >> 0) & 15));
	i.p[1] = param_none;
	i.p[2] = param_none;
	return i;
}

InstructionWithParams disarm_data_transfer(int code)
{
	InstructionWithParams i;
	bool bb = ((code >> 22) & 1);
	bool ll = ((code >> 20) & 1);
	if (ll)
		i.inst = bb ? INST_LDRB : INST_LDR;
	else
		i.inst = bb ? INST_STRB : INST_STR;
	int Rn = (code >> 16) & 0xf;
	int Rd = (code >> 12) & 0xf;
	i.p[0] = param_reg(REG_R0 + Rd);
	bool imm = ((code >> 25) & 1);
	bool pre = ((code >> 24) & 1);
	bool up = ((code >> 23) & 1);
	bool ww = ((code >> 21) & 1);
	if (imm){
		msg_write( " --shifted reg--");
	}else{
		if (code & 0xfff)
			i.p[1] = param_deref_reg_shift(REG_R0 + Rn, up ? (code & 0xfff) : (-(code & 0xfff)), bb ? SIZE_8 : SIZE_32);
		else
			i.p[1] = param_deref_reg(REG_R0 + Rn, bb ? SIZE_8 : SIZE_32);
	}
	i.p[1].write_back = ww;
	i.p[2] = param_none;
	return i;
}

InstructionWithParams disarm_data_block_transfer(int code)
{
	InstructionWithParams i;
	bool ll = ((code >> 20) & 1);
	bool pp = ((code >> 24) & 1);
	bool uu = ((code >> 23) & 1);
	bool ww = ((code >> 21) & 1);
	if (!pp and uu)
		i.inst = ll ? INST_LDMIA : INST_STMIA;
	else if (pp and uu)
		i.inst = ll ? INST_LDMIB : INST_STMIB;
	else if (!pp and !uu)
		i.inst = ll ? INST_LDMDA : INST_STMDA;
	else if (pp and !uu)
		i.inst = ll ? INST_LDMDB : INST_STMDB;
	int Rn = (code >> 16) & 0xf;
	i.p[0] = param_reg(REG_R0 + Rn);
	i.p[1] = param_reg_set(code & 0xffff);
	i.p[0].write_back = ww;
	i.p[2] = param_none;
	return i;
}

InstructionWithParams disarm_vfp(int code)
{
	InstructionWithParams i;

	int nn = 0;
	if ((code & (1<<6)) != 0)
		nn += 1;
	if ((code & (1<<20)) != 0)
		nn += 2;
	if ((code & (1<<21)) != 0)
		nn += 4;
	if ((code & (1<<23)) != 0)
		nn += 8;

	i.inst = ARM_VFP_PRIMARY_INSTRUCTIONS[nn];
	int cp_num = (code & 0x00000f00) >> 8;

	int fd = (code & 0x0000f000) >> 11;
	if ((code & (1<<22)) != 0)
		fd += 1;
	int fn = (code & 0x000f0000) >> 15;
	if ((code & (1<<7)) != 0)
		fn += 1;
	int fm = (code & 0x0000000f) << 1;
	if ((code & (1<<5)) != 0)
		fm += 1;

	if (nn == 15){
		i.inst = ARM_VFP_EXTENSION_INSTRUCTIONS[fn];
		i.p[0] = param_reg(REG_S0 + fd);
		i.p[1] = param_reg(REG_S0 + fm);
	}else{
		i.p[0] = param_reg(REG_S0 + fd);
		i.p[1] = param_reg(REG_S0 + fn);
		i.p[2] = param_reg(REG_S0 + fm);
	}
	return i;
}

InstructionWithParams disarm_vfp_transfer(int code)
{
	InstructionWithParams i;
	bool ll = ((code >> 20) & 1);
	if (ll)
		i.inst = INST_FLDS;
	else
		i.inst = INST_FSTS;
	int Rn = (code >> 16) & 0xf;
	int Fd = ((code >> 12) & 0xf) * 2;
	if ((code & (1<<22)) != 0)
		Fd += 1;

	i.p[0] = param_reg(REG_S0 + Fd);
	bool up = ((code >> 23) & 1);
	int offset = (code & 0xff) * 4;
	i.p[1] = param_deref_reg_shift(REG_R0 + Rn, up ? offset : -offset, SIZE_32);
	i.p[2] = param_none;
	return i;
}

string DisassembleARM(void *_code_,int length,bool allow_comments)
{
	string buf;
	int *code = (int*)_code_;
	for (int ni=0; ni<length/4; ni++){
		int cur = code[ni];

		int x = (cur >> 25) & 0x7;

		buf += string((char*)&cur, 4).hex(true).substr(2, -1);
		buf += "    ";

		InstructionWithParams iwp;
		iwp.inst = INST_NOP;
		iwp.p[0] = param_none;
		iwp.p[1] = param_none;
		iwp.p[2] = param_none;
		if ((cur & 0x0ff000f0) == 0x01200030){
			iwp = disarm_blx(cur);
		}else if (((cur >> 26) & 3) == 0){
			if ((cur & 0x0fe000f0) == 0x00000090)
				iwp = disarm_data_opcode_mul(cur);
			else
				iwp = disarm_data_opcode(cur);
		}else if (((cur >> 26) & 0x3) == 0b01)
			iwp = disarm_data_transfer(cur);
		else if (x == 0b100)
			iwp = disarm_data_block_transfer(cur);
		else if (x == 0b101)
			iwp = disarm_branch(cur);
		else if ((cur & 0x0f000010) == 0x0e000000)
			iwp = disarm_vfp(cur);
		else if ((cur & 0x0f200000) == 0x0d000000)
			iwp = disarm_vfp_transfer(cur);
		iwp.condition = (cur >> 28) & 0xf;


		buf += iwp.str() + "\n";
	}
	return buf;
}

string DisassembleX86(void *_code_,int length,bool allow_comments)
{
	char *code = (char*)_code_;

	string param;
	char *opcode;
	string bufstr;
	char *end=code+length;
	char *orig=code;
	if (length<0)	end=code+65536;

	// code points to the start of the (current) complete command (dword cs: mov ax, ...)
	// cur points to the currently processed byte
	// opcode points to the start of the instruction (mov)
	char *cur = code;
	state.init();
	state.DefaultSize = SIZE_32;


	while(code < end){
		state.reset(NULL);
		opcode = cur;
		code = cur;

		// done?
		if (code >= end)
			break;

		// special info
		if (CurrentMetaInfo){

			// labels
#if 0
			// TODO
			for (int i=0;i<CurrentMetaInfo->label.num;i++)
				if ((long)code - (long)orig == CurrentMetaInfo->label[i].pos)
					bufstr += "    " + CurrentMetaInfo->label[i].name + ":\n";
#endif

			// data blocks
			bool inserted = false;
			for (int i=0;i<CurrentMetaInfo->data.num;i++){
				//printf("%d  %d  %d  %d\n", CurrentMetaInfo->data[i].Pos, (long)code, (long)orig, (long)code - (long)orig);
				if ((long)code - (long)orig == CurrentMetaInfo->data[i].offset){
					//msg_write("data");
					if (CurrentMetaInfo->data[i].size==1){
						bufstr += "  db\t";
						bufstr += d2h(cur,1);
					}else if (CurrentMetaInfo->data[i].size==2){
						bufstr += "  dw\t";
						bufstr += d2h(cur,2);
					}else if (CurrentMetaInfo->data[i].size==4){
						bufstr += "  dd\t";
						bufstr += d2h(cur,4);
					}else{
						bufstr += "  ds \t...";
					}
					cur += CurrentMetaInfo->data[i].size;
					bufstr += "\n";
					inserted = true;
				}
			}
			if (inserted)
				continue;

			// change of bits (processor mode)
			for (int i=0;i<CurrentMetaInfo->bit_change.num;i++)
				if ((long)code-(long)orig == CurrentMetaInfo->bit_change[i].offset){
					state.DefaultSize = (CurrentMetaInfo->bit_change[i].bits == 16) ? SIZE_16 : SIZE_32;
					state.reset(NULL);
					if (state.DefaultSize == SIZE_16)
						bufstr += "   bits_16\n";
					else
						bufstr += "   bits_32\n";
				}
		}

		// code

		// prefix (size/segment register)
		Register *seg = NULL;
		if (cur[0]==0x67){
			state.AddrSize = (state.DefaultSize == SIZE_32) ? SIZE_16 : SIZE_32;
			cur++;
		}
		if (cur[0]==0x66){
			state.ParamSize = (state.DefaultSize == SIZE_32) ? SIZE_16 : SIZE_32;
			cur++;
		}
		if (InstructionSet.set == INSTRUCTION_SET_AMD64){
			if ((cur[0] & 0xf0) == 0x40){
				if ((cur[0] & 0x08) > 0)
					state.ParamSize = SIZE_64;
				state.ExtendModRMReg = ((cur[0] & 0x04) > 0);
				state.ExtendModRMIndex = ((cur[0] & 0x02) > 0);
				state.ExtendModRMBase = ((cur[0] & 0x01) > 0);
				cur++;
			}
		}
		if (cur[0]==0x2e){	seg = RegisterByID[REG_CS];	cur++;	}
		else if (cur[0]==0x36){	seg = RegisterByID[REG_SS];	cur++;	}
		else if (cur[0]==0x3e){	seg = RegisterByID[REG_DS];	cur++;	}
		else if (cur[0]==0x26){	seg = RegisterByID[REG_ES];	cur++;	}
		else if (cur[0]==0x64){	seg = RegisterByID[REG_FS];	cur++;	}
		else if (cur[0]==0x65){	seg = RegisterByID[REG_GS];	cur++;	}
		opcode=cur;

		// instruction
		CPUInstruction *inst = NULL;
		for (CPUInstruction &ci: CPUInstructions){
			if (ci.code_size == 0)
				continue;
			if (!ci.has_fixed_param){
				if (ci.has_small_param != (state.ParamSize == SIZE_16))
					continue;
				if (ci.has_big_param != (state.ParamSize == SIZE_64))
					continue;
			}
			// opcode correct?
			bool ok = true;
			for (int j=0;j<ci.code_size;j++)
				if (cur[j] != ((char*)&ci.code)[j])
					ok = false;
			// cap correct?
			if (ci.cap >= 0)
				ok &= ((unsigned char)ci.cap == (((unsigned)cur[ci.code_size] >> 3) & 0x07));
			if ((ok) and (ci.has_modrm)){
				InstructionParam p1, p2;
				UnfuzzyParam(p1, ci.param1);
				UnfuzzyParam(p2, ci.param2);

				// modr/m byte
				char modrm = cur[ci.code_size];
				GetFromModRM(p1, ci.param1, modrm);
				GetFromModRM(p2, ci.param2, modrm);
				if ((p1.type == PARAMT_REGISTER) and (!p1.deref) and (!ci.param1.allow_register))
					continue;
				if ((p2.type == PARAMT_REGISTER) and (!p2.deref) and (!ci.param2.allow_register))
					continue;
			}
			if (ok){
				inst = &ci;
				cur += inst->code_size;
				break;
			}
		}
		if (inst){
			InstructionParamFuzzy ip1 = inst->param1;
			InstructionParamFuzzy ip2 = inst->param2;

			
			InstructionParam p1, p2;
			UnfuzzyParam(p1, ip1);
			UnfuzzyParam(p2, ip2);

			// modr/m byte
			if (inst->has_modrm){
				//msg_write("modrm");
				char modrm = *cur;
				cur ++;
				GetFromModRM(p1, ip1, modrm);
				GetFromModRM(p2, ip2, modrm);
				TryGetSIB(p1, cur);
				TryGetSIB(p2, cur);
			}

			// immediate...
			ReadParamData(cur, p1, inst->has_modrm);
			ReadParamData(cur, p2, inst->has_modrm);



		// create asm code
			string str;

			// segment register?
			if (seg)
				str += seg->name + ": ";

			// command
			str += inst->name;

			// parameters
			if ((state.ParamSize != state.DefaultSize) and ((p1.type != PARAMT_REGISTER) or (p1.deref)) and ((p2.type != PARAMT_REGISTER) or p2.deref)){
				if (state.ParamSize == SIZE_16)
					str += " word";
				else if (state.ParamSize == SIZE_32)
					str += " dword";
				else if (state.ParamSize == SIZE_64)
					str += " qword";
			}
			bool hide_size = p2.type != PARAMT_NONE;
			if (p1.type != PARAMT_NONE)
				str += " " + p1.str(hide_size);
			if (p2.type != PARAMT_NONE)
				str += ", " + p2.str(hide_size);
			
			
			if (allow_comments){
				int l = str.num;
				str += " ";
				for (int ii=0;ii<48-l;ii++)
					str += " ";
				str += "// ";
				str += d2h(code,long(cur) - long(code), false);
			}
			//msg_write(str);
			bufstr += str;
			bufstr += "\n";

		}else{
			//msg_write(string2("????? -                          unknown         // %s\n",d2h(code,1+long(cur)-long(code),false)));
			bufstr += format("????? -                          unknown         // %s\n",d2h(code,1+long(cur)-long(code),false).c_str());
			cur ++;
		}

		// done?
		if ((length < 0) and (((unsigned char)opcode[0] == 0xc3) or ((unsigned char)opcode[0] == 0xc2)))
			break;
	}
	return bufstr;
}



// convert some opcode into (human readable) assembler language
string Disassemble(void *code, int length, bool allow_comments)
{
	if (InstructionSet.set == INSTRUCTION_SET_ARM)
		return DisassembleARM(code, length, allow_comments);
	return DisassembleX86(code, length, allow_comments);
}

// skip unimportant code (whitespace/comments)
//    returns true if end of code
bool IgnoreUnimportant(int &pos)
{
	bool CommentLine = false;
	
	// ignore comments and "white space"
	for (int i=0;i<1048576;i++){
		if (code_buffer[pos] == 0){
			state.EndOfCode = true;
			state.EndOfLine = true;
			return true;
		}
		if (code_buffer[pos] == '\n'){
			state.LineNo ++;
			state.ColumnNo = 0;
			CommentLine = false;
		}
		// "white space"
		if ((code_buffer[pos] == '\n') or (code_buffer[pos] == ' ') or (code_buffer[pos] == '\t')){
			pos ++;
			state.ColumnNo ++;
			continue;
		}
		// comments
		if ((code_buffer[pos] == ';') or ((code_buffer[pos] == '/') and (code_buffer[pos] == '/'))){
			CommentLine = true;
			pos ++;
			state.ColumnNo ++;
			continue;
		}
		if (!CommentLine)
			break;
		pos ++;
		state.ColumnNo ++;
	}
	return false;
}

// returns one "word" in the source code
string FindMnemonic(int &pos)
{
	state.EndOfLine = false;
	char mne[128];
	strcpy(mne, "");

	if (IgnoreUnimportant(pos))
		return mne;
	
	bool in_string = false;
	for (int i=0;i<128;i++){
		mne[i] = code_buffer[pos];
		mne[i + 1] = 0;
		
		// string like stuff
		if ((mne[i] == '\'') or (mne[i] == '\"'))
			in_string =! in_string;
		// end of code
		if (code_buffer[pos] == 0){
			mne[i] = 0;
			state.EndOfCode = true;
			state.EndOfLine = true;
			break;
		}
		// end of line
		if (code_buffer[pos] == '\n'){
			mne[i] = 0;
			state.EndOfLine = true;
			break;
		}
		if (!in_string){
			// "white space" -> complete
			if ((code_buffer[pos] == ' ') or (code_buffer[pos] == '\t') or (code_buffer[pos] == ',')){
				mne[i] = 0;
				// end of line?
				for (int j=0;j<128;j++){
					if ((code_buffer[pos+j] != ' ') and (code_buffer[pos+j] != '\t') and (code_buffer[pos+j] != ',')){
						if ((code_buffer[pos + j] == 0) or (code_buffer[pos + j] == '\n'))
							state.EndOfLine = true;
						// comment ending the line
						if ((code_buffer[pos + j] == ';') or ((code_buffer[pos + j] == '/') and (code_buffer[pos + j + 1] == '/')))
							state.EndOfLine = true;
						pos += j;
						state.ColumnNo += j;
						if (code_buffer[pos] == '\n')
							state.ColumnNo = 0;
						break;
					}
				}
				break;
			}
		}
		pos ++;
		state.ColumnNo ++;
	}
	/*msg_write>Write(mne);
	if (EndOfLine)
		msg_write>Write("    eol");*/
	return mne;
}

// interpret an expression from source code as an assembler parameter
void GetParam(InstructionParam &p, const string &param, InstructionWithParamsList &list, int pn)
{
	p.type = PARAMT_INVALID;
	p.reg = NULL;
	p.deref = false;
	p.size = SIZE_UNKNOWN;
	p.disp = DISP_MODE_NONE;
	p.is_label = false;
	//msg_write(param);

	// none
	if (param.num == 0){
		p.type = PARAMT_NONE;

	// deref
	}else if ((param[0] == '[') and (param[param.num-1] == ']')){
		if (DebugAsm)
			printf("deref:   ");
		so("Deref:");
		//bool u16 = use_mode16;
		GetParam(p, param.substr(1, -2), list, pn);
		p.size = SIZE_UNKNOWN;
		p.deref = true;
		//use_mode16 = u16;

	// string
	}else if ((param[0] == '\"') and (param[param.num-1] == '\"')){
		if (DebugAsm)
			printf("String:   ");
		char *ps = new char[param.num - 1];
		strcpy(ps, param.substr(1, -2).c_str());
		p.value = (long)ps;
		p.type = PARAMT_IMMEDIATE;

	// complex...
	}else if (param.find("+") >= 0){
		if (DebugAsm)
			printf("complex:   ");
		InstructionParam sub;
		
		// first part (must be a register)
		string part;
		for (int i=0;i<param.num;i++)
			if ((param[i] == ' ') or (param[i] == '+'))
				break;
			else
				part.add(param[i]);
		int offset = part.num;
		GetParam(sub, part, list, pn);
		if (sub.type == PARAMT_REGISTER){
			//msg_write("reg");
			p.type = PARAMT_REGISTER;
			p.size = SIZE_32;
			p.reg = sub.reg;
		}else
			p.type = PARAMT_INVALID;

		// second part (...up till now only hex)
		for (int i=offset;i<param.num;i++)
			if ((param[i] != ' ') and (param[i] != '+')){
				offset = i;
				break;
			}
		part = param.substr(offset, -1);
		GetParam(sub, part, list, pn);
		if (sub.type == PARAMT_IMMEDIATE){
			//msg_write("c2 = im");
			if (((long)sub.value & 0xffffff00) == 0)
				p.disp = DISP_MODE_8;
			else
				p.disp = DISP_MODE_32;
			p.value = sub.value;
		}else
			p.type = PARAMT_INVALID;

		

	// hex const
	}else if ((param[0] == '0') and (param[1] == 'x')){
		p.type = PARAMT_IMMEDIATE;
		long long v = 0;
		for (int i=2;i<param.num;i++){
			if (param[i] == '.'){
			}else if ((param[i] >= 'a') and (param[i] <= 'f')){
				v *= 16;
				v += param[i] - 'a' + 10;
			}else if ((param[i] >= 'A') and (param[i] <= 'F')){
				v *= 16;
				v += param[i]-'A'+10;
			}else if ((param[i]>='0')and(param[i]<='9')){
				v*=16;
				v+=param[i]-'0';
			/*}else if (param[i]==':'){
				InstructionParam sub;
				GetParam(sub, param.tail(param.num - i - 1), list, pn);
				if (sub.type != ParamTImmediate){
					SetError("error in hex parameter:  " + string(param));
					p.type = PKInvalid;
					return;						
				}
				p.value = (long)v;
				p.value <<= 8 * sub.size;
				p.value += sub.value;
				p.size = sub.size;
				p.type = ParamTImmediate;//Ext;
				break;*/
			}else{
				SetError("evil character in hex parameter:  \"" + param + "\"");
				p.type = PARAMT_INVALID;
				return;
			}
			p.value = (long)v;
			p.size = SIZE_8;
			if (param.num > 4)
				p.size = SIZE_16;
			if (param.num > 6)
				p.size = SIZE_32;
			if (param.num > 10)
				p.size = SIZE_48;
			if (param.num > 14)
				p.size = SIZE_64;
		}
		if (DebugAsm){
			printf("hex const:  %s\n",d2h((char*)&p.value,p.size).c_str());
		}

	// char const
	}else if ((param[0] == '\'') and (param[param.num - 1] == '\'')){
		p = param_imm((long)param[1], SIZE_8);
		if (DebugAsm)
			printf("hex const:  %s\n",d2h((char*)&p.value,1).c_str());

	// label substitude
	}else if (param == "$"){
		p = param_label(list.add_label(param), SIZE_32);
		
	}else{
		// register
		for (int i=0;i<Registers.num;i++)
			if (Registers[i].name == param){
				p = param_reg(Registers[i].id);
				return;
			}
		// existing label
		for (int i=0;i<list.label.num;i++)
			if (list.label[i].name == param){
				p = param_label(i, SIZE_32);
				return;
			}
		// script variable (global)
		for (int i=0;i<CurrentMetaInfo->global_var.num;i++){
			if (CurrentMetaInfo->global_var[i].name == param){
				p = param_deref_imm((long)CurrentMetaInfo->global_var[i].pos, CurrentMetaInfo->global_var[i].size);
				return;
			}
		}
		// not yet existing label...
		if (param[0]=='_'){
			so("label as param:  \"" + param + "\"\n");
			p = param_label(list.get_label(param), SIZE_32);
			return;
		}
	}
	if (p.type == PARAMT_INVALID)
		SetError("unknown parameter:  \"" + param + "\"\n");
}

inline void insert_val(char *oc, int &ocs, long long val, int size)
{
	if (size == SIZE_8)
		oc[ocs] = (char)val;
	else if (size == SIZE_16)
		*(short*)&oc[ocs] = (short)val;
	else if (size == SIZE_24)
		*(int*)&oc[ocs - 1] = (*(int*)&oc[ocs - 1] & 0xff000000) | ((int)val & 0x00ffffff);
	else if (size == SIZE_32)
		*(int*)&oc[ocs] = (int)val;
	else if (size == SIZE_64)
		*(long long int*)&oc[ocs] = val;
	else if (size == SIZE_8L4){
		val = arm_encode_8l4(val);
		*(int*)&oc[ocs - 2] = (*(int*)&oc[ocs - 2] & 0xfffff000) | ((int)val & 0x00000fff);
	}else if (size == SIZE_12){
		*(int*)&oc[ocs - 2] = (*(int*)&oc[ocs - 2] & 0xfffff000) | ((int)val & 0x00000fff);
	}else if (size == SIZE_8S2){
		oc[ocs] = (char)(val >> 2);
	}else if (size > 0)
		memcpy(&oc[ocs], &val, size);
}

inline void append_val(char *oc, int &ocs, long long val, int size)
{
	insert_val(oc, ocs, val, size);
	ocs += size;
}

void OpcodeAddImmideate(char *oc, int &ocs, InstructionParam &p, CPUInstruction &inst, InstructionWithParamsList &list, int next_param_size)
{
	long long value = p.value;
	int size = 0;
	if (p.type == PARAMT_IMMEDIATE){
		size = p.size;
		if (p.deref){
			//---msg_write("deref....");
			size = state.AddrSize; // inst.has_big_addr
			if (InstructionSet.set == INSTRUCTION_SET_AMD64){
				if (inst.has_modrm)
					value -= (long)oc + ocs + size + next_param_size; // amd64 uses RIP-relative addressing!
				else
					size = SIZE_64; // Ov/Mv...
			}
		}
	//}else if (p.type == ParamTImmediateExt){
	//	size = state.ParamSize;  // bits 0-15  /  0-31
	}else if (p.type == PARAMT_REGISTER){
		if (p.disp == DISP_MODE_8)	size = SIZE_8;
		if (p.disp == DISP_MODE_16)	size = SIZE_16;
		if (p.disp == DISP_MODE_32)	size = SIZE_32;
	}else
		return;

	bool rel = ((inst.name[0] == 'j') /*and (inst.param1._type_ != ParamTImmediateDouble)*/) or (inst.name == "call") or (inst.name.find("loop") >= 0);
	if (inst.inst == INST_JMP_FAR)
		rel = false;
	if (p.is_label){
		if ((InstructionSet.set == INSTRUCTION_SET_AMD64) and (p.deref))
			rel = true;
		list.add_wanted_label(ocs, p.value, list.current_inst, rel, false, size);
	}else if (rel){
		value -= CurrentMetaInfo->code_origin + ocs + size + next_param_size; // TODO ...first byte of next opcode
	}

	//---msg_write("imm " + i2s(size));
	append_val(oc, ocs, value, size);
}

void InstructionWithParamsList::LinkWantedLabels(void *oc)
{
	foreachib(WantedLabel &w, wanted_label, i){
		Label &l = label[w.label_no];
		if (l.value == -1)
			continue;
		so("linking label");

		long long value = l.value;
		if (w.relative){
			int size = w.size;
			if ((size == SIZE_8L4) or (size == SIZE_12))
				size = 2;
			if (size == SIZE_8S2)
				size = 1;

			// TODO first byte after command
			if (InstructionSet.set == INSTRUCTION_SET_ARM){
				value -= CurrentMetaInfo->code_origin + w.pos + size + 4;
				int inst = (*this)[w.inst_no].inst;
				if ((inst == INST_BL) or (inst == INST_B) or (inst == INST_CALL) or (inst == INST_JMP)){
					value = value >> 2;
				}
			}else{
				value -= CurrentMetaInfo->code_origin + w.pos + size;
			}
		}
		if ((w.abs) and (value < 0))
			value = - value;

		insert_val((char*)oc, w.pos, value, w.size);


		wanted_label.erase(i);
		_foreach_it_.update();
	}
}

void add_data_inst(InstructionWithParamsList *l, int size)
{
	AsmData d;
	d.cmd_pos = l->num;
	d.size = size;
	CurrentMetaInfo->data.add(d);
}

void InstructionWithParamsList::AppendFromSource(const string &_code)
{
	const char *code = _code.c_str();

	if (!CurrentMetaInfo)
		SetError("no CurrentMetaInfo");

	state.LineNo = CurrentMetaInfo->line_offset;
	state.ColumnNo = 0;

	// CurrentMetaInfo->CurrentOpcodePos // Anfang aktuelle Zeile im gesammten Opcode
	code_buffer = code; // Asm-Source-Puffer

	int pos = 0;
	InstructionParam p1, p2, p3;
	state.DefaultSize = SIZE_32;
	if (CurrentMetaInfo)
		if (CurrentMetaInfo->mode16)
			state.DefaultSize = SIZE_16;
	state.EndOfCode = false;
	while(pos < _code.num - 2){

		string cmd, param1, param2, param3;

		//msg_write("..");
		state.reset(this);


	// interpret asm code (1 line)
		// find command
		cmd = FindMnemonic(pos);
		current_line = state.LineNo;
		current_col = state.ColumnNo;
		//msg_write(cmd);
		if (cmd.num == 0)
			break;
		// find parameters
		if (!state.EndOfLine){
			param1 = FindMnemonic(pos);
			if ((param1 == "dword") or (param1 == "word") or (param1 == "qword")){
				if (param1 == "word")
					state.ParamSize = SIZE_16;
				else if (param1 == "dword")
					state.ParamSize = SIZE_32;
				else if (param1 == "qword")
					state.ParamSize = SIZE_64;
				if (!state.EndOfLine)
					param1 = FindMnemonic(pos);
			}
		}
		if (!state.EndOfLine)
			param2 = FindMnemonic(pos);
		if (!state.EndOfLine)
			param3 = FindMnemonic(pos);
		//msg_write(string2("----: %s %s%s %s", cmd, param1, (strlen(param2)>0)?",":"", param2));
		if (state.EndOfCode)
			break;
		so("------------------------------");
		so(cmd);
		so(param1);
		so(param2);
		so(param3);
		so("------");

		// parameters
		GetParam(p1, param1, *this, 0);
		GetParam(p2, param2, *this, 1);
		GetParam(p3, param3, *this, 1);
		if ((p1.type == PARAMT_INVALID) or (p2.type == PARAMT_INVALID) or (p3.type == PARAMT_INVALID))
			return;

	// special stuff
		if (cmd == "bits_16"){
			so("16 bit Modus!");
			state.DefaultSize = SIZE_16;
			state.reset(this);
			if (CurrentMetaInfo){
				CurrentMetaInfo->mode16 = true;
				BitChange b;
				b.cmd_pos = num;
				b.bits = 16;
				CurrentMetaInfo->bit_change.add(b);
			}
			continue;
		}else if (cmd == "bits_32"){
			so("32 bit Modus!");
			state.DefaultSize = SIZE_32;
			state.reset(this);
			if (CurrentMetaInfo){
				CurrentMetaInfo->mode16 = false;
				BitChange b;
				b.cmd_pos = num;
				b.bits = 32;
				CurrentMetaInfo->bit_change.add(b);
			}
			continue;

		}else if (cmd == "db"){
			so("Daten:   1 byte");
			add_data_inst(this, 1);
		}else if (cmd == "dw"){
			so("Daten:   2 byte");
			add_data_inst(this, 2);
		}else if (cmd == "dd"){
			so("Daten:   4 byte");
			add_data_inst(this, 4);
		}/*else if ((cmd == "ds") or (cmd == "dz")){
			so("Daten:   String");
			char *s = (char*)p1.value;
			int l=strlen(s);
			if (cmd == "dz")
				l ++;
			if (CurrentMetaInfo){
				AsmData d;
				d.cmd_pos = num;
				d.size = l;
				d.data = new char[l];
				memcpy(d.data, s, l);
				CurrentMetaInfo->data.add(d);
			}
			//memcpy(&buffer[CodeLength], s, l);
			//CodeLength += l;
			continue;
		}*/else if (cmd[cmd.num - 1] == ':'){
			so("Label");
			cmd.resize(cmd.num - 1);
			so(cmd);
			add_label(cmd);

			continue;
		}


		InstructionWithParams iwp;
		iwp.condition = ARM_COND_ALWAYS;

		if (cmd.find(":") >= 0){
			iwp.condition = -1;
			Array<string> l = cmd.explode(":");
			for (int i=0; i<16; i++)
				if (l[0] == ARMConditions[i])
					iwp.condition = i;
			if (iwp.condition < 0)
				SetError("unknown condition: " + l[0]);
			cmd = l[1];
		}

		// command
		int inst = -1;
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (string(InstructionNames[i].name) == cmd)
				inst = InstructionNames[i].inst;
		if (inst < 0)
			SetError("unknown instruction:  " + cmd);
		// prefix
		if (state.ParamSize != state.DefaultSize){
			//buffer[CodeLength ++] = 0x66;
			SetError("prefix unhandled:  " + cmd);
		}
		iwp.inst = inst;
		iwp.p[0] = p1;
		iwp.p[1] = p2;
		iwp.p[2] = p3;
		iwp.line = current_line;
		iwp.col = current_col;
		add(iwp);


		if (state.EndOfCode)
			break;
	}
}


// convert human readable asm code into opcode
bool Assemble(const char *code, char *oc, int &ocs)
{
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/

	InstructionWithParamsList list = InstructionWithParamsList(CurrentMetaInfo->line_offset);

	list.AppendFromSource(code);

	list.Optimize(oc, ocs);

	// compile commands
	list.Compile(oc, ocs);

	return true;
}

inline bool _size_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p)
{
	if (inst_p.size == wanted_p.size)
		return true;
	if (wanted_p.reg)
		if ((inst_p.reg_group == REG_GROUP_XMM) and (wanted_p.reg->group == REG_GROUP_XMM))
			return true;
	if ((inst_p.size == SIZE_UNKNOWN) or (wanted_p.size == SIZE_UNKNOWN))
		return true;
/*	if ((inst_p.size == SizeVariable) and ((wanted_p.size == Size16) or (wanted_p.size == Size32)))
		return true;*/
	return false;
}

inline bool _deref_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p)
{
	if (wanted_p.deref)
		return (inst_p.allow_memory_address) or (inst_p.allow_memory_indirect);
	return true;
}

bool InstructionParamFuzzy::match(InstructionParam &wanted_p)
{
	//ParamFuzzyOut(&inst_p);
	
	// none
	if ((wanted_p.type == PARAMT_NONE) or (!used))
		return (wanted_p.type == PARAMT_NONE) and (!used);

	// xmm register...
	if ((allow_register) and (wanted_p.type == PARAMT_REGISTER) and (wanted_p.reg)){
		if ((reg_group == REG_GROUP_XMM) and (wanted_p.reg->group == REG_GROUP_XMM))
			return true;
	}

	// size mismatch?
	if ((size != SIZE_UNKNOWN) and (wanted_p.size != SIZE_UNKNOWN))
		if (size != wanted_p.size)
			return false;

	// immediate
	if (wanted_p.type == PARAMT_IMMEDIATE){
		if ((allow_memory_address) and (wanted_p.deref))
			return true;
		if ((allow_immediate) and (!wanted_p.deref)){
			//msg_write("imm " + SizeOut(inst_p.size) + " " + SizeOut(wanted_p.size));
			return (size == wanted_p.size);
		}
		return false;
	}

	// immediate double
	/*if (wanted_p.type == ParamTImmediateExt){
		msg_write("imx");
		if (allow_memory_address)
			return (size == wanted_p.size);
	}*/

	// reg
	if (wanted_p.type == PARAMT_REGISTER){
		// direct match
		if ((allow_register) and (reg)){
			if (wanted_p.reg){
				if ((reg->id >= REG_RAX) and (reg->id <= REG_RBP) and (wanted_p.reg->id == reg->id + REG_R8 - REG_RAX))
					return true;
			}
			return ((reg == wanted_p.reg) and (_deref_match_(*this, wanted_p)));
		}
		// fuzzy match
		/*if (inst_p.allow_register){
			msg_write("r2");
			
			return ((inst_p.reg_group == wanted_p.reg->group) and (_size_match_(inst_p, wanted_p)) and (_deref_match_(inst_p, wanted_p)));
		}*/
		// very fuzzy match
		if ((allow_register) or (allow_memory_indirect)){
			if (wanted_p.deref){
				if (allow_memory_indirect)
					return ((REG_GROUP_GENERAL == wanted_p.reg->group) and (_deref_match_(*this, wanted_p)));
			}else if (allow_register)
				return ((reg_group == wanted_p.reg->group) and (_size_match_(*this, wanted_p))); // FIXME (correct?)
		}
	}

	return false;
}


int GetModRMReg(Register *r)
{
	int id = r->id;
	if ((id == REG_R8)  or (id == REG_R8D)  or (id == REG_RAX) or (id == REG_EAX) or (id == REG_AX) or (id == REG_AL))	return 0x00;
	if ((id == REG_R9)  or (id == REG_R9D)  or (id == REG_RCX) or (id == REG_ECX) or (id == REG_CX) or (id == REG_CL))	return 0x01;
	if ((id == REG_R10) or (id == REG_R10D) or (id == REG_RDX) or (id == REG_EDX) or (id == REG_DX) or (id == REG_DL))	return 0x02;
	if ((id == REG_R11) or (id == REG_R11D) or (id == REG_RBX) or (id == REG_EBX) or (id == REG_BX) or (id == REG_BL))	return 0x03;
	if ((id == REG_R12) or (id == REG_R12D) or (id == REG_RSP) or (id == REG_ESP) or (id == REG_SP) or (id == REG_AH))	return 0x04;
	if ((id == REG_R13) or (id == REG_R13D) or (id == REG_RBP) or (id == REG_EBP) or (id == REG_BP) or (id == REG_CH))	return 0x05;
	if ((id == REG_R14) or (id == REG_R14D) or (id == REG_RSI) or (id == REG_ESI) or (id == REG_SI) or (id == REG_DH))	return 0x06;
	if ((id == REG_R15) or (id == REG_R15D) or (id == REG_RDI) or (id == REG_EDI) or (id == REG_DI) or (id == REG_BH))	return 0x07;
	if ((id >= REG_XMM0) and (id <= REG_XMM7))	return (id - REG_XMM0);
	SetError("GetModRMReg: register not allowed: " + r->name);
	return 0;
}

inline int CreatePartialModRMByte(InstructionParamFuzzy &pf, InstructionParam &p)
{
	int r = -1;
	if (p.reg)
		r = p.reg->id;
	if (pf.mrm_mode == MRM_REG){
		if (r == REG_ES)	return 0x00;
		if (r == REG_CS)	return 0x08;
		if (r == REG_SS)	return 0x10;
		if (r == REG_DS)	return 0x18;
		if (r == REG_FS)	return 0x20;
		if (r == REG_GS)	return 0x28;
		if (r == REG_CR0)	return 0x00;
		if (r == REG_CR1)	return 0x08;
		if (r == REG_RC2)	return 0x10;
		if (r == REG_CR3)	return 0x18;
		if (r == REG_CR4)	return 0x20;
		int mrm = GetModRMReg(p.reg) << 3;
		if (p.reg->extend_mod_rm)
			mrm += 0x0400; // REXR
		return mrm;
	}else if (pf.mrm_mode == MRM_MOD_RM){
		if (p.deref){
			if (state.AddrSize == SIZE_16){
				if ((p.type == PARAMT_IMMEDIATE) and (p.deref))	return 0x06;
			}else{
				if ((r == REG_EAX) or (r == REG_RAX))	return (p.disp == DISP_MODE_NONE) ? 0x00 : ((p.disp == DISP_MODE_8) ? 0x40 : 0x80); // default = DispMode32
				if ((r == REG_ECX) or (r == REG_RCX))	return (p.disp == DISP_MODE_NONE) ? 0x01 : ((p.disp == DISP_MODE_8) ? 0x41 : 0x81);
				if ((r == REG_EDX) or (r == REG_RDX))	return (p.disp == DISP_MODE_NONE) ? 0x02 : ((p.disp == DISP_MODE_8) ? 0x42 : 0x82);
				if ((r == REG_EBX) or (r == REG_RBX))	return (p.disp == DISP_MODE_NONE) ? 0x03 : ((p.disp == DISP_MODE_8) ? 0x43 : 0x83);
				// sib			return 4;
				// disp32		return 5;
				if ((p.type == PARAMT_IMMEDIATE) and (p.deref))	return 0x05;
				if ((r == REG_EBP) or (r == REG_RBP))	return (p.disp == DISP_MODE_8) ? 0x45 : 0x85;
				if ((r == REG_ESI) or (r == REG_RSI))	return (p.disp == DISP_MODE_NONE) ? 0x06 : ((p.disp == DISP_MODE_8) ? 0x46 : 0x86);
				if ((r == REG_EDI) or (r == REG_RDI))	return (p.disp == DISP_MODE_NONE) ? 0x07 : ((p.disp == DISP_MODE_8) ? 0x47 : 0x87);
			}
		}else{
			int mrm = GetModRMReg(p.reg) | 0xc0;
			if (p.reg->extend_mod_rm)
				mrm += 0x0100; // REXB
			return mrm;
		}
	}
	if (pf.mrm_mode != MRM_NONE)
		SetError(format("unhandled modrm %d %d %s %d %s", pf.mrm_mode, p.type, (p.reg?p.reg->name.c_str():""), p.deref, SizeOut(pf.size).c_str()));
	return 0x00;
}

int CreateModRMByte(CPUInstruction &inst, InstructionParam &p1, InstructionParam &p2)
{
	int mrm = CreatePartialModRMByte(inst.param1, p1) | CreatePartialModRMByte(inst.param2, p2);
	if (inst.cap >= 0)
		mrm |= (inst.cap << 3);
	return mrm;
}

void OpcodeAddInstruction(char *oc, int &ocs, CPUInstruction &inst, InstructionParam &p1, InstructionParam &p2, InstructionWithParamsList &list)
{
	//---msg_write("add inst " + inst.name);

	// 16/32 bit toggle prefix
	if ((!inst.has_fixed_param) and (inst.has_small_param != (state.DefaultSize == SIZE_16)))
		append_val(oc, ocs, 0x66, 1);

	int mod_rm = 0;
	if (inst.has_modrm)
		mod_rm = CreateModRMByte(inst, p1, p2);

	// REX prefix
	char rex = mod_rm >> 8;
	if ((inst.param1.reg) and (p1.reg))
		if ((inst.param1.reg->id >= REG_RAX) and (inst.param1.reg->id <= REG_RBP) and (inst.param1.reg->id == p1.reg->id + REG_RAX - REG_R8))
			rex = 0x01;
	if (inst.has_big_param)//state.ParamSize == Size64)
		rex |= 0x08;
	if (rex != 0)
		append_val(oc, ocs, 0x40 | rex, 1);

	// add opcode
	*(int*)&oc[ocs] = inst.code;
	ocs += inst.code_size;

	// create mod/rm-byte
	if (inst.has_modrm)
		oc[ocs ++] = mod_rm;

	OCParam = ocs;

	int param2_size = 0;
	if (p2.type == PARAMT_IMMEDIATE)
		param2_size = p2.size;

	OpcodeAddImmideate(oc, ocs, p1, inst, list, param2_size);
	OpcodeAddImmideate(oc, ocs, p2, inst, list, 0);
}

void InstructionWithParamsList::AddInstruction(char *oc, int &ocs, int n)
{
	int ocs0 = ocs;
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;
	state.reset(this);

	// test if any instruction matches our wishes
	int ninst = -1;
	bool has_mod_rm = false;
	foreachi(CPUInstruction &c, CPUInstructions, i)
		if ((!c.ignore) and (c.match(iwp))){
			if (((!c.has_modrm) and (has_mod_rm)) or (ninst < 0)){
				has_mod_rm = c.has_modrm;
				ninst = i;
			}
		}

/*	// try again with REX prefix?
 // now done automatically...!
	if ((ninst < 0) and (InstructionSet.set == InstructionSetAMD64)){
		state.ParamSize = Size64;

		for (int i=0;i<CPUInstructions.num;i++)
			if (CPUInstructions[i].match(iwp)){
				if (((!CPUInstructions[i].has_modrm) and (has_mod_rm)) or (ninst < 0)){
					has_mod_rm = CPUInstructions[i].has_modrm;
					ninst = i;
				}
			}

	}*/

	// none found?
	if (ninst < 0){
		state.LineNo = iwp.line;
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (InstructionNames[i].inst == iwp.inst)
				SetError("command not compatible with its parameters\n" + iwp.str());
		SetError(format("instruction unknown: %d", iwp.inst));
	}


	if (DebugAsm)
		CPUInstructions[ninst].print();

	// compile
	OpcodeAddInstruction(oc, ocs, CPUInstructions[ninst], iwp.p[0], iwp.p[1], *this);
	iwp.size = ocs - ocs0;

	//msg_write(d2h(&oc[ocs0], ocs - ocs0, false));
}

int arm_reg_no(Register *r)
{
	if (r)
		if ((r->id >= REG_R0) and (r->id <= REG_R15))
			return r->id - REG_R0;
	SetError("ARM: invalid register: " + r->name);
	return -1;
}

int arm_freg_no(Register *r)
{
	if (r)
		if ((r->id >= REG_S0) and (r->id <= REG_S31))
			return r->id - REG_S0;
	SetError("ARM: invalid vfp register: " + r->name);
	return -1;
}

int arm_encode_8l4(unsigned int value)
{
	for (int ex=0; ex<=30; ex+=2){
		unsigned int mask = (0xffffff00 >> ex) | (0xffffff00 << (32-ex));
		if ((value & mask) == 0){
			unsigned int mant = (value << ex) | (value >> (32 - ex));
			return mant | (ex << (8-1));
		}
	}
	SetError("ARM: immediate value not representable: " + i2s(value));
	return 0;
}

bool inline arm_is_load_store_reg(int inst)
{
	return (inst == INST_LDR) or (inst == INST_LDRB) or (inst == INST_STR) or (inst == INST_STRB);
}

bool inline arm_is_data(int inst, int &nn)
{
	nn = -1;
	for (int i=0; i<16; i++)
		if (inst == ARM_DATA_INSTRUCTIONS[i]){
			nn = i;
			return true;
		}
	return false;
}

bool inline arm_is_vfp_primary(int inst, int &nn)
{
	nn = -1;
	for (int i=0; i<16; i++)
		if (inst == ARM_VFP_PRIMARY_INSTRUCTIONS[i]){
			nn = i;
			return true;
		}
	return false;
}

bool inline arm_is_vfp_extension(int inst, int &nn)
{
	nn = -1;
	for (int i=0; i<32; i++)
		if (inst == ARM_VFP_EXTENSION_INSTRUCTIONS[i]){
			nn = i;
			return true;
		}
	return false;
}

bool inline arm_is_load_store_multi(int inst)
{
	if ((inst == INST_LDMIA) or (inst == INST_LDMIB) or (inst == INST_LDMDA) or (inst == INST_LDMDB))
		return true;
	if ((inst == INST_STMIA) or (inst == INST_STMIB) or (inst == INST_STMDA) or (inst == INST_STMDB))
		return true;
	return false;
}

void arm_expect(InstructionWithParams &c, int type0 = PARAMT_NONE, int type1 = PARAMT_NONE, int type2 = PARAMT_NONE)
{
	int t[3] = {type0, type1, type2};
	for (int i=0; i<3; i++)
		if (c.p[i].type != t[i])
			SetError(format("param #%d expected to be %s: ", i+1, "???") + c.str());
}

inline bool label_after_now(InstructionWithParamsList *list, int label_no, int now)
{
	if (list->label[label_no].inst_no < 0)
		return true;
	return list->label[label_no].inst_no > now;
}

void InstructionWithParamsList::AddInstructionARM(char *oc, int &ocs, int n)
{
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;
	state.reset(this);

	int code = 0;

	code = iwp.condition << 28;
	int nn = -1;
	if (arm_is_data(iwp.inst, nn)){
		if ((iwp.inst == INST_CMP) or (iwp.inst == INST_CMN) or (iwp.inst == INST_TST) or (iwp.inst == INST_TEQ) or (iwp.inst == INST_MOV)){
			iwp.p[2] = iwp.p[1];
			if ((iwp.inst == INST_CMP) or (iwp.inst == INST_CMN)){
				iwp.p[1] = iwp.p[0];
				iwp.p[0] = param_reg(REG_R0);
			}else{
				iwp.p[1] = param_reg(REG_R0);
			}
		}
		bool ss = (iwp.inst == INST_CMP) or (iwp.inst == INST_CMN) or (iwp.inst == INST_TST) or (iwp.inst == INST_TEQ);
		code |= 0x0 << 26;
		code |= (nn << 21);
		if (ss)
			code |= 1 << 20;
		if (iwp.p[2].type == PARAMT_REGISTER){
			arm_expect(iwp, PARAMT_REGISTER, PARAMT_REGISTER, PARAMT_REGISTER);
			code |= arm_reg_no(iwp.p[2].reg) << 0;
			if (iwp.p[2].disp != DISP_MODE_NONE)
				SetError("p3.disp != DISP_MODE_NONE");
		}else if (iwp.p[2].type == PARAMT_IMMEDIATE){
			arm_expect(iwp, PARAMT_REGISTER, PARAMT_REGISTER, PARAMT_IMMEDIATE);
			if (iwp.p[2].is_label){
				add_wanted_label(ocs + 2, iwp.p[2].value, n, false, false, SIZE_8L4);
			}else{
				code |= arm_encode_8l4(iwp.p[2].value) << 0;
				code |= 1 << 25;
			}
		}/*else if (iwp.p[2].type == PARAMT_REGISTER_SHIFT){
			msg_write("TODO reg shift");
			code |= arm_reg_no(iwp.p[2].reg) << 0;
			code |= (iwp.p[2].value & 0xff) << 4;
		}*/ else{
			SetError("unhandled param #3 in " + iwp.str());
		}
		code |= arm_reg_no(iwp.p[0].reg) << 12;
		code |= arm_reg_no(iwp.p[1].reg) << 16;
	}else if (arm_is_load_store_reg(iwp.inst)){
		if (iwp.inst == INST_LDR)
			code |= 0x04100000;
		else if (iwp.inst == INST_LDRB)
			code |= 0x04500000;
		else if (iwp.inst == INST_STR)
			code |= 0x04000000;
		else if (iwp.inst == INST_STRB)
			code |= 0x04400000;

		if ((iwp.p[1].type == PARAMT_IMMEDIATE) and (iwp.p[1].deref) and (iwp.p[1].is_label)){
			add_wanted_label(ocs + 2, iwp.p[1].value, n, true, true, SIZE_12);
			iwp.p[1] = param_deref_reg_shift(REG_R15, label_after_now(this, iwp.p[1].value, n) ? 1 : -1, SIZE_32);
		}

		if (iwp.p[0].reg == iwp.p[1].reg)
			SetError("not allowed to use the same register for destination and addressing: " + iwp.str());

		code |= arm_reg_no(iwp.p[0].reg) << 12; // Rd
		code |= arm_reg_no(iwp.p[1].reg) << 16; // Rn

		if ((iwp.p[1].disp == DISP_MODE_8) or (iwp.p[1].disp == DISP_MODE_32)){
			if ((iwp.p[1].value > 0x0fff) or (iwp.p[1].value < - 0x0fff))
				SetError("offset larger than 12 bit: " + iwp.str());
			if (iwp.p[1].value >= 0)
				code |= 0x01800000 | (iwp.p[1].value & 0x0fff);
			else
				code |= 0x01000000 | ((-iwp.p[1].value) & 0x0fff);
		}else if (iwp.p[1].disp == DISP_MODE_REG2){
			if (iwp.p[1].value >= 0)
				code |= 0x03800000;
			else
				code |= 0x03000000;
			code |= arm_reg_no(iwp.p[1].reg2);
		}
	}else if (arm_is_load_store_multi(iwp.inst)){
		arm_expect(iwp, PARAMT_REGISTER, PARAMT_IMMEDIATE);
		bool ll = ((iwp.inst == INST_LDMIA) or (iwp.inst == INST_LDMIB) or (iwp.inst == INST_LDMDA) or (iwp.inst == INST_LDMDB));
		bool uu = ((iwp.inst == INST_LDMIA) or (iwp.inst == INST_LDMIB) or (iwp.inst == INST_STMIA) or (iwp.inst == INST_STMIB));
		bool pp = ((iwp.inst == INST_LDMIB) or (iwp.inst == INST_LDMDB) or (iwp.inst == INST_STMIB) or (iwp.inst == INST_STMDB));
		bool ww = true;
		if (ll)
			code |= 0x08100000;
		else
			code |= 0x08000000;
		if (uu)
			code |= 0x00800000;
		if (pp)
			code |= 0x01000000;
		if (ww)
			code |= 0x00200000;
		code |= arm_reg_no(iwp.p[0].reg) << 16;
		code |= iwp.p[1].value & 0xffff;
	}else if (iwp.inst == INST_MUL){
		code |= 0x00000090;
		arm_expect(iwp, PARAMT_REGISTER, PARAMT_REGISTER, PARAMT_REGISTER);
		code |= arm_reg_no(iwp.p[0].reg) << 16;
		code |= arm_reg_no(iwp.p[1].reg);
		code |= arm_reg_no(iwp.p[2].reg) << 8;
	}else if ((iwp.inst == INST_BLX) or ((iwp.inst == INST_CALL) and (iwp.p[0].type == PARAMT_REGISTER))){
		arm_expect(iwp, PARAMT_REGISTER);
		code |= 0x012fff30;
		code |= arm_reg_no(iwp.p[0].reg);
	}else if ((iwp.inst == INST_BL) or (iwp.inst == INST_B) or (iwp.inst == INST_JMP) or (iwp.inst == INST_CALL)){
		arm_expect(iwp, PARAMT_IMMEDIATE);
		if ((iwp.inst == INST_BL) or (iwp.inst == INST_CALL))
			code |= 0x0b000000;
		else
			code |= 0x0a000000;
		int value = iwp.p[0].value;
		if (iwp.p[0].is_label){
			add_wanted_label(ocs + 1, value, n, true, false, SIZE_24);
		}else if (iwp.inst == INST_CALL)
			value = (iwp.p[0].value - (long)&oc[ocs] - 8) >> 2;
		code |= (value & 0x00ffffff);
	}else if (iwp.inst == INST_DD){
		arm_expect(iwp, PARAMT_IMMEDIATE);
		code = iwp.p[0].value;
	}else if (arm_is_vfp_primary(iwp.inst, nn)){
		arm_expect(iwp, PARAMT_REGISTER, PARAMT_REGISTER, PARAMT_REGISTER);
		code |= 0x0e000a00; // a=single, b=double
		if ((nn & 0x01) > 0)
			code |= 1 << 6;
		if ((nn & 0x02) > 0)
			code |= 1 << 20;
		if ((nn & 0x04) > 0)
			code |= 1 << 21;
		if ((nn & 0x08) > 0)
			code |= 1 << 23;
		int fd = arm_freg_no(iwp.p[0].reg);
		int fn = arm_freg_no(iwp.p[1].reg);
		int fm = arm_freg_no(iwp.p[2].reg);
		code |= fm >> 1;
		code |= (fd >> 1) << 12;
		code |= (fn >> 1) << 16;
		if ((fm & 0x01) > 0)
			code |= 1 << 5;
		if ((fn & 0x01) > 0)
			code |= 1 << 7;
		if ((fd & 0x01) > 0)
			code |= 1 << 22;
	}else if (arm_is_vfp_extension(iwp.inst, nn)){
		arm_expect(iwp, PARAMT_REGISTER, PARAMT_REGISTER);
		code |= 0x0eb00a40; // a=single, b=double
		int fd = arm_freg_no(iwp.p[0].reg);
		int fn = nn;
		int fm = arm_freg_no(iwp.p[1].reg);
		code |= fm >> 1;
		code |= (fd >> 1) << 12;
		code |= (fn >> 1) << 16;
		if ((fm & 0x01) > 0)
			code |= 1 << 5;
		if ((fn & 0x01) > 0)
			code |= 1 << 7;
		if ((fd & 0x01) > 0)
			code |= 1 << 22;
	}else if ((iwp.inst == INST_FLDS) or (iwp.inst == INST_FSTS)){
		if (iwp.inst == INST_FLDS)
			code |= 0x0d100a00;
		else
			code |= 0x0d000a00;

		if ((iwp.p[1].type == PARAMT_IMMEDIATE) and (iwp.p[1].deref)){
			if (iwp.p[1].is_label){
				add_wanted_label(ocs + 3, iwp.p[1].value, n, true, true, SIZE_8S2);
				iwp.p[1] = param_deref_reg_shift(REG_R15, label_after_now(this, iwp.p[1].value, n) ? 1 : -1, SIZE_32);
			}else{
				iwp.p[1] = param_deref_reg_shift(REG_R15, iwp.p[1].value - (long)&oc[ocs] - 8, SIZE_32);
			}
		}

		int fd = arm_freg_no(iwp.p[0].reg);
		code |= (fd >> 1)<< 12;
		if ((fd & 1) > 0)
			code |= 1 << 22;

		code |= arm_reg_no(iwp.p[1].reg) << 16; // Rn

		if ((iwp.p[1].disp == DISP_MODE_8) or (iwp.p[1].disp == DISP_MODE_32)){
			int v = (iwp.p[1].value >> 2);
			if ((v > 0x00ff) or (v < - 0x00ff))
				SetError("offset larger than 8 bit: " + iwp.str());
			if (v >= 0)
				code |= 0x00800000 | (v & 0x00ff);
			else
				code |= 0x00000000 | ((-v) & 0x00ff);
		}
	}else{
		SetError("cannot assemble instruction: " + iwp.str());
	}

	*(int*)&oc[ocs] = code;
	ocs += 4;
}

void InstructionWithParamsList::ShrinkJumps(void *oc, int ocs)
{
	// first pass compilation (we need real jump distances)
	int _ocs = ocs;
	Compile(oc, _ocs);
	wanted_label.clear();

	// try shrinking
	foreachi(InstructionWithParams &iwp, *this, i){
		if ((iwp.inst == INST_JMP) or (iwp.inst == INST_JZ) or (iwp.inst == INST_JNZ) or (iwp.inst == INST_JL) or (iwp.inst == INST_JNL) or (iwp.inst == INST_JLE) or (iwp.inst == INST_JNLE)){
			if (iwp.p[0].is_label){
				int target = label[(int)iwp.p[0].value].inst_no;

				// jump distance
				int dist = 0;
				for (int j=i+1;j<target;j++)
					dist += (*this)[j].size;
				for (int j=target;j<=i;j++)
					dist += (*this)[j].size;
				//msg_write(format("%d %d   %d", i, target, dist));

				if (dist < 127){
					so("really shrink");
					iwp.p[0].size = SIZE_8;
				}
			}
		}
	}
}

void InstructionWithParamsList::Optimize(void *oc, int ocs)
{
	if (InstructionSet.set != INSTRUCTION_SET_ARM)
		ShrinkJumps(oc, ocs);
}

void InstructionWithParamsList::Compile(void *oc, int &ocs)
{
	state.DefaultSize = SIZE_32;
	state.reset(this);
	if (!CurrentMetaInfo){
		DummyMetaInfo.code_origin = (long)oc;
		CurrentMetaInfo = &DummyMetaInfo;
	}

	for (int i=0;i<num+1;i++){
		// bit change
		for (BitChange &b: CurrentMetaInfo->bit_change)
			if (b.cmd_pos == i){
				state.DefaultSize = SIZE_32;
				if (b.bits == 16)
					state.DefaultSize = SIZE_16;
				state.reset(this);
				b.offset = ocs;
			}

		// data?
		for (AsmData &d: CurrentMetaInfo->data)
			if (d.cmd_pos == i)
				d.offset = ocs;

		// defining a label?
		for (int j=0;j<label.num;j++)
			if (i == label[j].inst_no){
				so("defining found: " + label[j].name);
				label[j].value = CurrentMetaInfo->code_origin + ocs;
			}
		if (i >= num)
			break;

		// opcode
		if (InstructionSet.set == INSTRUCTION_SET_ARM)
			AddInstructionARM((char*)oc, ocs, i);
		else
			AddInstruction((char*)oc, ocs, i);
	}

	LinkWantedLabels(oc);

	for (WantedLabel &l: wanted_label){
		state.LineNo = (*this)[l.inst_no].line;
		state.ColumnNo = (*this)[l.inst_no].col;
		SetError("undeclared label used: " + l.name);
	}
}

void AddInstruction(char *oc, int &ocs, int inst, const InstructionParam &p1, const InstructionParam &p2, const InstructionParam &p3)
{
	/*if (!CPUInstructions)
		SetInstructionSet(InstructionSetDefault);*/
	state.DefaultSize = SIZE_32;
	state.reset(NULL);
	/*msg_write("--------");
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionName[i].inst == inst)
			printf("%s\n", InstructionName[i].name);*/

	OCParam = ocs;
	InstructionWithParamsList list = InstructionWithParamsList(0);
	InstructionWithParams iwp;
	iwp.inst = inst;
	iwp.p[0] = p1;
	iwp.p[1] = p2;
	iwp.p[2] = p3;
	iwp.line = -1;
	list.add(iwp);
	list.AddInstruction(oc, ocs, 0);
}

bool ImmediateAllowed(int inst)
{
	for (CPUInstruction &i: CPUInstructions)
		if (i.inst == inst)
			if ((i.param1.allow_immediate) or (i.param2.allow_immediate))
				return true;
	return false;
}

};
