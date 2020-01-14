#include "../../base/base.h"
#include "../../file/file.h"
#include "asm.h"
#include "internal.h"
#include <stdio.h>

namespace Asm
{


int OCParam;

ParserState state;


InstructionSetData instruction_set;

void ParserState::init()
{
	default_size = SIZE_32;
	full_register_size = instruction_set.pointer_size;

	if (CurrentMetaInfo)
		if (CurrentMetaInfo->mode16)
			default_size = SIZE_16;

	list = nullptr;
}
void ParserState::reset(InstructionWithParamsList *_list)
{
	param_size = default_size;
	addr_size = default_size;
	extend_mod_rm_base = false;
	extend_mod_rm_reg = false;
	extend_mod_rm_index = false;
	list = _list;
}
string ParserState::get_label(int i)
{
	if (list)
		if ((i >= 0) and (i < list->label.num))
			return list->label[i].name;
	return "_label_" + i2s(i);
}

const char *code_buffer;
MetaInfo *CurrentMetaInfo = nullptr;
MetaInfo DummyMetaInfo;

int arm_encode_8l4(unsigned int value);

Exception::Exception(const string &_message, const string &_expression, int _line, int _column)
{
	if (_expression.num > 0)
		text += "\"" + _expression + "\": ";
	text += _message;
	line = _line;
	column = _column;
	if (line >= 0)
		text += "\nline " + i2s(line);
}

Exception::~Exception(){}

void Exception::print() const
{
	msg_error(message());
}

void raise_error(const string &str)
{
	msg_error(str);
	//msg_error(str + format("\nline %d", LineNo + 1));
	throw Exception(str, "", state.line_no, state.column_no);
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



MetaInfo::MetaInfo() {
	mode16 = false;
	code_origin = 0;
	line_offset = 0;
}




Array<Register> Registers;
Array<Register*> RegisterByID;
int RegRoot[NUM_REGISTERS];
int RegResize[NUM_REG_ROOTS][MAX_REG_SIZE + 1];

void add_reg(const string &name, int id, int group, int size, int root) {
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

string get_reg_name(int reg) {
	if ((reg < 0) or (reg >= NUM_REGISTERS))
		return "INVALID REG: " + i2s(reg);
	return RegisterByID[reg]->name;
}

// rw1/2: 
const InstructionName InstructionNames[NUM_INSTRUCTION_NAMES + 1] = {
	{INST_DB,		"db"},
	{INST_DW,		"dw"},
	{INST_DD,		"dd"},
	{INST_DS,		"ds"},
	{INST_DZ,		"dz"},
	{INST_ALIGN_OPCODE,	":align:"},

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
	{INST_MOVLPS, "movlps", 64+3, 64+1},
	{INST_MOVHPS, "movhps", 64+3, 64+1},
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
	{INST_CVTSS2SD,  "cvtss2sd",  64+3, 64+1},
	{INST_CVTSD2SS,  "cvtsd2ss",  64+3, 64+1},
	{INST_CVTTSS2SI, "cvttss2si", 64+3, 64+1},
	{INST_CVTTSD2SI, "cvttsd2si", 64+3, 64+1},
	{INST_CVTSI2SS,  "cvtsi2ss",  64+3, 64+1},
	{INST_CVTSI2SD,  "cvtsi2sd",  64+3, 64+1},
	{INST_COMISS,    "comiss",    64+3, 64+1},
	{INST_COMISD,    "comisd",    64+3, 64+1},
	{INST_UCOMISS,   "ucomiss",   64+3, 64+1},
	{INST_UCOMISD,   "ucomisd",   64+3, 64+1},

	// amd64
	{INST_SYSCALL,	"syscall"},
	{INST_SYSRET,	"sysret"},
	{INST_SYSENTER,	"sysenter"},
	{INST_SYSEXIT,	"sysexit"},

	{INST_B,		"b"},
	{INST_BL,		"bl"},
	{INST_BLX,		"blx"},

	{INST_MULS, "muls"},
	{INST_ADDS, "adds"},
	{INST_SUBS, "subs"},
	{INST_RSBS, "rsbs"},
	{INST_ADCS, "adcs"},
	{INST_SBCS, "sbcs"},
	{INST_RSCS, "rscs"},
	{INST_ANDS, "ands"},
	{INST_BICS, "bics"},
	{INST_XORS, "xors"},
	{INST_ORS, "ors"},
	{INST_MOVS, "movs"},
	{INST_MVNS, "movns"},

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





InstructionParam param_none;


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
		raise_error("invalid register index: " + i2s(reg));
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

InstructionParam param_imm(int64 value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	return p;
}

InstructionParam param_deref_imm(int64 value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	p.deref = true;
	return p;
}

InstructionParam param_label(int64 value, int size)
{
	InstructionParam p;
	p.type = PARAMT_IMMEDIATE;
	p.size = size;
	p.value = value;
	p.is_label = true;
	return p;
}

InstructionParam param_deref_label(int64 value, int size)
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


int InstructionWithParamsList::create_label(const string &name)
{
	/*if (name == "$")
		return -1;*/
	Label l;
	l.name = name;
	l.inst_no = -1;
	l.value = -1;
	label.add(l);
	return label.num - 1;
}

int InstructionWithParamsList::_find_label(const string &name)
{
	foreachi (Label &l, label, i)
		if (l.name == name)
			return i;
	return -1;

}

// really declare an existing one now
void InstructionWithParamsList::insert_label(int index)
{
	if (index < 0)
		return;
	Label &l = label[index];
	if (l.inst_no >= 0 and l.name != "$")
		raise_error("label already declared: " + l.name);
	l.inst_no = num;
	so("----redecl");

}
int64 InstructionWithParamsList::_label_value(int index)
{
	if (index < 0)
		return 0;
	Label &l = label[index];
	return l.value;

}

// declare
int InstructionWithParamsList::add_label(const string &name)
{
	so("add_label: " + name);
	// label already in use? (used before declared)
	int l = _find_label(name);
	if (l < 0)
		l = create_label(name);
	insert_label(l);
	return l;
}

// good
int InstructionWithParamsList::get_label(const string &name)
{
	so("add_label: " + name);
	int l = _find_label(name);
	if (l >= 0)
		return l;
	return create_label(name);
}

void *InstructionWithParamsList::get_label_value(const string &name)
{
	return (void*)_label_value(_find_label(name));
}


void InstructionWithParamsList::add_wanted_label(int pos, int label_no, int inst_no, bool rel, bool abs, int size)
{
	if ((label_no < 0) or (label_no >= label.num))
		raise_error("illegal wanted label request");
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

string SizeOut(int size)
{
	if (size == SIZE_8)
		return "8";
	if (size == SIZE_16)
		return "16";
	if (size == SIZE_32)
		return "32";
	if (size == SIZE_48)
		return "48";
	if (size == SIZE_64)
		return "64";
	if (size == SIZE_128)
		return "128";
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

const string GetInstructionName(int inst)
{
	if ((inst >= 0) and (inst < NUM_INSTRUCTION_NAMES))
		return Asm::InstructionNames[inst].name;
	return "???";
}

void get_instruction_param_flags(int inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst){
			p1_read = ((InstructionNames[i].rw1 & 1) > 0);
			p1_write = ((InstructionNames[i].rw1 & 2) > 0);
			p2_read = ((InstructionNames[i].rw2 & 1) > 0);
			p2_write = ((InstructionNames[i].rw2 & 2) > 0);
		}
}

bool get_instruction_allow_const(int inst)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return ((InstructionNames[i].rw1 & 64) == 0);
	return false;
}

bool get_instruction_allow_gen_reg(int inst)
{
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return ((InstructionNames[i].rw1 & 32) == 0);
	return false;
}



InstructionSet QueryLocalInstructionSet() {
#ifdef CPU_AMD64
	return InstructionSet::AMD64;
#endif
#ifdef CPU_X86
	return InstructionSet::X86;
#endif
#ifdef CPU_ARM
	return InstructionSet::ARM;
#endif
	msg_error("Asm: unknown instruction set");
	return InstructionSet::X86;
}



void init(InstructionSet set) {
	if (set == InstructionSet::NATIVE)
		set = QueryLocalInstructionSet();

	instruction_set.set = set;
	instruction_set.pointer_size = 4;
	if (set == InstructionSet::AMD64)
		instruction_set.pointer_size = 8;

	for (int i=0;i<NUM_REG_ROOTS;i++)
		for (int j=0;j<=MAX_REG_SIZE;j++)
			RegResize[i][j] = -1;


	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst != i)
			msg_error(string(InstructionNames[i].name) + "  " + i2s(InstructionNames[i].inst) + "  !=   " + i2s(i));

	if (set == InstructionSet::ARM)
		arm_init();
	else
		x86_init();
}

InstructionParam::InstructionParam()
{
	type = PARAMT_NONE;
	disp = DISP_MODE_NONE;
	reg = nullptr;
	reg2 = nullptr;
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
			//msg_write((int_p)reg);
			//msg_write((int_p)disp);
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
			return ss + "[" + s + "]" + post;
		}else
			return reg->name + post;
	}else if (type == PARAMT_REGISTER_SET){
		Array<string> s;
		for (int i=0; i<16; i++)
			if (value & (1<<i))
				s.add(RegisterByID[REG_R0 + i]->name);
		return "{" + implode(s, ",") + "}";
	}else if (type == PARAMT_IMMEDIATE){
		string s = d2h(&value, deref ? state.addr_size : size);
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

string show_reg(int r)
{
	return format("r%d", r);
}



// convert some opcode into (human readable) assembler language
string disassemble(void *code, int length, bool allow_comments) {
	if (instruction_set.set == InstructionSet::ARM)
		return arm_disassemble(code, length, allow_comments);
	return x86_disassemble(code, length, allow_comments);
}

// skip unimportant code (whitespace/comments)
//    returns true if end of code
bool IgnoreUnimportant(int &pos)
{
	bool CommentLine = false;
	
	// ignore comments and "white space"
	for (int i=0;i<1048576;i++){
		if (code_buffer[pos] == 0){
			state.end_of_code = true;
			state.end_of_line = true;
			return true;
		}
		if (code_buffer[pos] == '\n'){
			state.line_no ++;
			state.column_no = 0;
			CommentLine = false;
		}
		// "white space"
		if ((code_buffer[pos] == '\n') or (code_buffer[pos] == ' ') or (code_buffer[pos] == '\t')){
			pos ++;
			state.column_no ++;
			continue;
		}
		// comments
		if ((code_buffer[pos] == ';') or ((code_buffer[pos] == '/') and (code_buffer[pos] == '/'))){
			CommentLine = true;
			pos ++;
			state.column_no ++;
			continue;
		}
		if (!CommentLine)
			break;
		pos ++;
		state.column_no ++;
	}
	return false;
}

// returns one "word" in the source code
string FindMnemonic(int &pos)
{
	state.end_of_line = false;
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
			state.end_of_code = true;
			state.end_of_line = true;
			break;
		}
		// end of line
		if (code_buffer[pos] == '\n'){
			mne[i] = 0;
			state.end_of_line = true;
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
							state.end_of_line = true;
						// comment ending the line
						if ((code_buffer[pos + j] == ';') or ((code_buffer[pos + j] == '/') and (code_buffer[pos + j + 1] == '/')))
							state.end_of_line = true;
						pos += j;
						state.column_no += j;
						if (code_buffer[pos] == '\n')
							state.column_no = 0;
						break;
					}
				}
				break;
			}
		}
		pos ++;
		state.column_no ++;
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
	p.reg = nullptr;
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
		p.value = (int_p)ps;
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
			if (((int_p)sub.value & 0xffffff00) == 0)
				p.disp = DISP_MODE_8;
			else
				p.disp = DISP_MODE_32;
			p.value = sub.value;
		}else
			p.type = PARAMT_INVALID;

		

	// hex const
	}else if ((param[0] == '0') and (param[1] == 'x')){
		p.type = PARAMT_IMMEDIATE;
		int64 v = 0;
		for (int i=2;i<param.num;i++){
			if (param[i] == '.'){
			}else if ((param[i] >= 'a') and (param[i] <= 'f')){
				v *= 16;
				v += param[i] - 'a' + 10;
			}else if ((param[i] >= 'A') and (param[i] <= 'F')){
				v *= 16;
				v += param[i]-'A'+10;
			}else if ((param[i]>='0') and (param[i]<='9')){
				v*=16;
				v+=param[i]-'0';
			/*}else if (param[i]==':'){
				InstructionParam sub;
				GetParam(sub, param.tail(param.num - i - 1), list, pn);
				if (sub.type != ParamTImmediate){
					raise_error("error in hex parameter:  " + string(param));
					p.type = PKInvalid;
					return;						
				}
				p.value = (int_p)v;
				p.value <<= 8 * sub.size;
				p.value += sub.value;
				p.size = sub.size;
				p.type = ParamTImmediate;//Ext;
				break;*/
			}else{
				raise_error("evil character in hex parameter:  \"" + param + "\"");
				p.type = PARAMT_INVALID;
				return;
			}
			p.value = (int_p)v;
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
		p = param_imm((int_p)param[1], SIZE_8);
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
				p = param_deref_imm((int_p)CurrentMetaInfo->global_var[i].pos, CurrentMetaInfo->global_var[i].size);
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
		raise_error("unknown parameter:  \"" + param + "\"\n");
}

void insert_val(char *oc, int &ocs, int64 val, int size)
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
		*(int64*)&oc[ocs] = val;
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

void InstructionWithParamsList::link_wanted_labels(void *oc)
{
	foreachib(WantedLabel &w, wanted_label, i){
		Label &l = label[w.label_no];
		if (l.value == -1)
			continue;
		so("linking label");

		int64 value = l.value;
		if (w.relative){
			int size = w.size;
			if ((size == SIZE_8L4) or (size == SIZE_12))
				size = 2;
			if (size == SIZE_8S2)
				size = 1;

			// TODO first byte after command
			if (instruction_set.set == InstructionSet::ARM){
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

void InstructionWithParamsList::append_from_source(const string &_code)
{
	const char *code = _code.c_str();

	if (!CurrentMetaInfo)
		raise_error("no CurrentMetaInfo");

	state.line_no = CurrentMetaInfo->line_offset;
	state.column_no = 0;

	// CurrentMetaInfo->CurrentOpcodePos // Anfang aktuelle Zeile im gesammten Opcode
	code_buffer = code; // Asm-Source-Puffer

	int pos = 0;
	InstructionParam p1, p2, p3;
	state.default_size = SIZE_32;
	if (CurrentMetaInfo)
		if (CurrentMetaInfo->mode16)
			state.default_size = SIZE_16;
	state.end_of_code = false;
	while(pos < _code.num - 2){

		string cmd, param1, param2, param3;

		//msg_write("..");
		state.reset(this);


	// interpret asm code (1 line)
		// find command
		cmd = FindMnemonic(pos);
		current_line = state.line_no;
		current_col = state.column_no;
		//msg_write(cmd);
		if (cmd.num == 0)
			break;
		// find parameters
		if (!state.end_of_line){
			param1 = FindMnemonic(pos);
			if ((param1 == "dword") or (param1 == "word") or (param1 == "qword")){
				if (param1 == "word")
					state.param_size = SIZE_16;
				else if (param1 == "dword")
					state.param_size = SIZE_32;
				else if (param1 == "qword")
					state.param_size = SIZE_64;
				if (!state.end_of_line)
					param1 = FindMnemonic(pos);
			}
		}
		if (!state.end_of_line)
			param2 = FindMnemonic(pos);
		if (!state.end_of_line)
			param3 = FindMnemonic(pos);
		//msg_write(string2("----: %s %s%s %s", cmd, param1, (strlen(param2)>0)?",":"", param2));
		if (state.end_of_code)
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
			state.default_size = SIZE_16;
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
			state.default_size = SIZE_32;
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
				raise_error("unknown condition: " + l[0]);
			cmd = l[1];
		}

		// command
		int inst = -1;
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (string(InstructionNames[i].name) == cmd)
				inst = InstructionNames[i].inst;
		if (inst < 0)
			raise_error("unknown instruction:  " + cmd);
		// prefix
		if (state.param_size != state.default_size){
			//buffer[CodeLength ++] = 0x66;
			raise_error("prefix unhandled:  " + cmd);
		}
		iwp.inst = inst;
		iwp.p[0] = p1;
		iwp.p[1] = p2;
		iwp.p[2] = p3;
		iwp.line = current_line;
		iwp.col = current_col;
		add(iwp);


		if (state.end_of_code)
			break;
	}
}


// convert human readable asm code into opcode
bool assemble(const char *code, char *oc, int &ocs) {
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/

	InstructionWithParamsList list = InstructionWithParamsList(CurrentMetaInfo->line_offset);

	list.append_from_source(code);

	list.optimize(oc, ocs);

	// compile commands
	list.compile(oc, ocs);

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



void InstructionWithParamsList::shrink_jumps(void *oc, int ocs)
{
	// first pass compilation (we need real jump distances)
	int _ocs = ocs;
	compile(oc, _ocs);
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

void InstructionWithParamsList::optimize(void *oc, int ocs) {
	if (instruction_set.set != InstructionSet::ARM)
		shrink_jumps(oc, ocs);
}

void InstructionWithParamsList::compile(void *oc, int &ocs)
{
	state.default_size = SIZE_32;
	state.reset(this);
	if (!CurrentMetaInfo){
		DummyMetaInfo.code_origin = (int_p)oc;
		CurrentMetaInfo = &DummyMetaInfo;
	}

	for (int i=0;i<num+1;i++){
		state.line_no = i;
		// bit change
		for (BitChange &b: CurrentMetaInfo->bit_change)
			if (b.cmd_pos == i){
				state.default_size = SIZE_32;
				if (b.bits == 16)
					state.default_size = SIZE_16;
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
		if (instruction_set.set == InstructionSet::ARM)
			add_instruction_arm((char*)oc, ocs, i);
		else
			add_instruction((char*)oc, ocs, i);
	}

	link_wanted_labels(oc);

	for (WantedLabel &l: wanted_label){
		state.line_no = (*this)[l.inst_no].line;
		state.column_no = (*this)[l.inst_no].col;
		raise_error("undeclared label used: " + l.name);
	}
}

void add_instruction(char *oc, int &ocs, int inst, const InstructionParam &p1, const InstructionParam &p2, const InstructionParam &p3)
{
	/*if (!CPUInstructions)
		SetInstructionSet(InstructionSetDefault);*/
	state.default_size = SIZE_32;
	state.reset(nullptr);
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
	list.add_instruction(oc, ocs, 0);
}

};
