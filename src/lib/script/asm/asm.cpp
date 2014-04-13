#include "../../base/base.h"
#include "../../file/file.h"
#include "asm.h"
#include <stdio.h>

namespace Asm
{


int OCParam;



enum{
	Size8 = 1,
	Size16 = 2,
	Size32 = 4,
	Size48 = 6,
	Size64 = 8,
	Size128 = 16,
	/*SizeVariable = -5,
	Size32or48 = -6,*/
	SizeUnknown = -7,
};

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
	void init()
	{
		DefaultSize = Size32;
		FullRegisterSize = InstructionSet.pointer_size;

		if (CurrentMetaInfo)
			if (CurrentMetaInfo->Mode16)
				DefaultSize = Size16;
	}
	void reset()
	{
		ParamSize = DefaultSize;
		AddrSize = DefaultSize;
		ExtendModRMBase = false;
		ExtendModRMReg = false;
		ExtendModRMIndex = false;
	}
};
static ParserState state;

const char *code_buffer;
MetaInfo *CurrentMetaInfo = NULL;
MetaInfo DummyMetaInfo;

Exception::Exception(const string &_message, const string &_expression, int _line, int _column)
{
	if (_expression.num > 0)
		message += "\"" + _expression + "\": ";
	message += _message;
	if (line >= 0)
		message += "\nline " + i2s(_line);

	line = _line;
	column = _column;
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

// penalty:  0 -> max output
#define ASM_DB_LEVEL	10



MetaInfo::MetaInfo()
{
	Mode16 = false;
	CodeOrigin = 0;
	LineOffset = 0;
}



// groups of registers
enum{
	RegGroupNone,
	RegGroupGeneral,
	RegGroupGeneral2,
	RegGroupSegment,
	RegGroupFlags,
	RegGroupControl,
	RegGroupX87,
	RegGroupXmm,
};


struct Register{
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
	if (group == RegGroupGeneral2){
		r.group = RegGroupGeneral;
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
	if ((reg < 0) || (reg >= NUM_REGISTERS))
		return "INVALID REG: " + i2s(reg);
	return RegisterByID[reg]->name;
}

struct InstructionName{
	int inst;
	string name;
	int rw1, rw2; // parameter is read(1), modified(2) or both (3)
};

// rw1/2: 
InstructionName InstructionNames[NUM_INSTRUCTION_NAMES + 1] = {
	{inst_db,		"db"},
	{inst_dw,		"dw"},
	{inst_dd,		"dd"},
	{inst_ds,		"ds"},
	{inst_dz,		"dz"},

	{inst_add,		"add",		3, 1},
	{inst_adc,		"adc",		3, 1},
	{inst_sub,		"sub",		3, 1},
	{inst_sbb,		"sbb",		3, 1},
	{inst_inc,		"inc",		3},
	{inst_dec,		"dec",		3},
	{inst_mul,		"mul",		3, 1},
	{inst_imul,		"imul",		3, 1},
	{inst_div,		"div",		3, 1},
	{inst_idiv,		"idiv",		3, 1},
	{inst_mov,		"mov",		2, 1},
	{inst_movzx,	"movzx",	2, 1},
	{inst_movsx,	"movsx",	2, 1},
	{inst_and,		"and",		3, 1},
	{inst_or,		"or",		3, 1},
	{inst_xor,		"xor",		3, 1},
	{inst_not,		"not",		3},
	{inst_neg,		"neg",		3},
	{inst_pop,		"pop",		2},
	{inst_popa,		"popa",		2},
	{inst_push,		"push",		1},
	{inst_pusha,	"pusha",	1},
	
	{inst_jo,		"jo",		1},
	{inst_jno,		"jno",		1},
	{inst_jb,		"jb",		1},
	{inst_jnb,		"jnb",		1},
	{inst_jz,		"jz",		1},
	{inst_jnz,		"jnz",		1},
	{inst_jbe,		"jbe",		1},
	{inst_jnbe,		"jnbe",		1},
	{inst_js,		"js",		1},
	{inst_jns,		"jns",		1},
	{inst_jp,		"jp",		1},
	{inst_jnp,		"jnp",		1},
	{inst_jl,		"jl",		1},
	{inst_jnl,		"jnl",		1},
	{inst_jle,		"jle",		1},
	{inst_jnle,		"jnle",		1},
	
	{inst_cmp,		"cmp",		1, 1},
	
	{inst_seto,		"seto",		2},
	{inst_setno,	"setno",	2},
	{inst_setb,		"setb",		2},
	{inst_setnb,	"setnb",	2},
	{inst_setz,		"setz",		2},
	{inst_setnz,	"setnz",	2},
	{inst_setbe,	"setbe",	2},
	{inst_setnbe,	"setnbe",	2},
	{inst_sets,		"sets",		2},
	{inst_setns,	"setns",	2},
	{inst_setp,		"setp",		2},
	{inst_setnp,	"setnp",	2},
	{inst_setl,		"setl",		2},
	{inst_setnl,	"setnl",	2},
	{inst_setle,	"setle",	2},
	{inst_setnle,	"setnle",	2},
	
	{inst_sldt,		"sldt"},
	{inst_str,		"str"},
	{inst_lldt,		"lldt"},
	{inst_ltr,		"ltr"},
	{inst_verr,		"verr"},
	{inst_verw,		"verw"},
	{inst_sgdt,		"sgdt"},
	{inst_sidt,		"sidt"},
	{inst_lgdt,		"lgdt"},
	{inst_lidt,		"lidt"},
	{inst_smsw,		"smsw"},
	{inst_lmsw,		"lmsw"},
	
	{inst_test,		"test",		1, 1},
	{inst_xchg,		"xchg",		3, 3},
	{inst_lea,		"lea", 		2, 1},
	{inst_nop,		"nop"},
	{inst_cbw_cwde,	"cbw/cwde"},
	{inst_cgq_cwd,	"cgq/cwd"},
	{inst_movs_ds_esi_es_edi,	"movs_ds:esi,es:edi"},
	{inst_movs_b_ds_esi_es_edi,	"movs.b_ds:esi,es:edi"},
	{inst_cmps_ds_esi_es_edi,	"cmps_ds:esi,es:edi"},
	{inst_cmps_b_ds_esi_es_edi,	"cmps.b_ds:esi,es:edi"},
	{inst_rol,		"rol",		3, 1},
	{inst_ror,		"ror",		3, 1},
	{inst_rcl,		"rcl",		3, 1},
	{inst_rcr,		"rcr",		3, 1},
	{inst_shl,		"shl",		3, 1},
	{inst_shr,		"shr",		3, 1},
	{inst_sar,		"sar",		3, 1},
	{inst_ret,		"ret",		1},
	{inst_leave,	"leave",	1},
	{inst_ret_far,	"ret_far",	1},
	{inst_int,		"int",		1},
	{inst_iret,		"iret",		1},
	
	{inst_fadd,		"fadd",		1},
	{inst_fmul,		"fmul",		1},
	{inst_fsub,		"fsub",		1},
	{inst_fdiv,		"fdiv",		1},
	{inst_fld,		"fld",		1},
	{inst_fld1,		"fld1",		0},
	{inst_fldz,		"fldz",		0},
	{inst_fldpi,	"fldpi",	0},
	{inst_fst,		"fst",		2},
	{inst_fstp,		"fstp",		2},
	{inst_fild,		"fild",		1},
	{inst_faddp,	"faddp",	1},
	{inst_fmulp,	"fmulp",	1},
	{inst_fsubp,	"fsubp",	1},
	{inst_fdivp,	"fdivp",	1},
	{inst_fldcw,	"fldcw",	1},
	{inst_fnstcw,	"fnstcw",	2},
	{inst_fnstsw,	"fnstsw",	2},
	{inst_fistp,	"fistp",	2},
	{inst_fxch,		"fxch",		3, 3},
	{inst_fsqrt,	"fsqrt",	3},
	{inst_fsin,		"fsin",		3},
	{inst_fcos,		"fcos",		3},
	{inst_fptan,	"fptan",	3},
	{inst_fpatan,	"fpatan",	3},
	{inst_fyl2x,	"fyl2x",	3},
	{inst_fchs,		"fchs",		3},
	{inst_fabs,		"fabs",		3},
	{inst_fucompp,	"fucompp",	1, 1},
	
	{inst_loop,		"loop"},
	{inst_loope,	"loope"},
	{inst_loopne,	"loopne"},
	{inst_in,		"in",		2, 1},
	{inst_out,		"out",		1, 1},
	
	{inst_call,		"call",		1},
	{inst_call_far,	"call_far", 1},
	{inst_jmp,		"jmp",		1},
	{inst_jmp_far,	"jmp_far",		1},
	{inst_lock,		"lock"},
	{inst_rep,		"rep"},
	{inst_repne,	"repne"},
	{inst_hlt,		"hlt"},
	{inst_cmc,		"cmc"},
	{inst_clc,		"clc"},
	{inst_stc,		"stc"},
	{inst_cli,		"cli"},
	{inst_sti,		"sti"},
	{inst_cld,		"cld"},
	{inst_std,		"std"},

	{inst_movss,		"movss"},
	{inst_movsd,		"movsd"},
	
	{-1,			"???"}
};



// parameter types
enum{
	ParamTImmediate,
	ParamTRegister,
	ParamTRegisterOrMem, // ...
	ParamTMemory,
	//ParamTSIB,
	ParamTNone,
	ParamTInvalid
};

// short parameter type
enum{
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
};

// displacement for registers
enum{
	DispModeNone,	// reg
	DispMode8,		// reg + 8bit
	DispMode16,		// reg + 16bit
	DispMode32,		// reg + 32bit
	DispModeSIB,	// SIB-byte
	DispMode8SIB,	// SIB-byte + 8bit
	DispModeReg2,	// reg + reg2
	DispMode8Reg2,	// reg + reg2 + 8bit
	DispMode16Reg2	// reg + reg2 + 16bit
};

// a real parameter (usable)
struct InstructionParam{
	int type;
	int disp;
	Register *reg, *reg2;
	bool deref;
	int size;
	long long value; // disp or immediate
	bool is_label;
	bool immediate_is_relative;	// for jump
	string str(bool hide_size = false);
};

struct InstructionWithParams{
	int inst;
	InstructionParam p1, p2;
	int line, col;
	int size;
	int addr_size;
	int param_size;
};


InstructionParam _make_param_(int type, int size, long long param);

InstructionWithParamsList::InstructionWithParamsList(int line_no)
{
	current_line = line_no;
	current_col = 0;
}

InstructionWithParamsList::~InstructionWithParamsList()
{}

void InstructionWithParamsList::add_easy(int inst, int param1_type, int param1_size, void *param1, int param2_type, int param2_size, void *param2)
{
	InstructionWithParams i;
	i.inst = inst;
	i.p1 = _make_param_(param1_type, param1_size, (long)param1);
	i.p2 = _make_param_(param2_type, param2_size, (long)param2);
	i.line = current_line;
	i.col = current_col;
	add(i);
};

int InstructionWithParamsList::add_label(const string &name, bool declaring)
{
	so("add_label: " + name);
	// label already in use? (used before declared)
	if (declaring){
		foreachi(Label &l, label, i)
			if (l.InstNo < 0)
				if (l.Name == name){
					l.InstNo = num;
					so("----redecl");
					return i;
				}
	}else{
		foreachi(Label &l, label, i)
			if (l.Name == name){
				so("----reuse");
				return i;
			}
	}
	Label l;
	l.Name = name;
	l.InstNo = declaring ? num : -1;
	l.Value = -1;
	label.add(l);
	return label.num - 1;
}

void InstructionWithParamsList::add_func_intro(int stack_alloc_size)
{
	long reg_bp = (InstructionSet.set == InstructionSetAMD64) ? RegRbp : RegEbp;
	long reg_sp = (InstructionSet.set == InstructionSetAMD64) ? RegRsp : RegEsp;
	int s = InstructionSet.pointer_size;
	add_easy(inst_push, PKRegister, s, (void*)reg_bp);
	add_easy(inst_mov, PKRegister, s, (void*)reg_bp, PKRegister, s, (void*)reg_sp);
	if (stack_alloc_size > 127){
		add_easy(inst_sub, PKRegister, s, (void*)reg_sp, PKConstant, Size32, (void*)(long)stack_alloc_size);
	}else if (stack_alloc_size > 0){
		add_easy(inst_sub, PKRegister, s, (void*)reg_sp, PKConstant, Size8, (void*)(long)stack_alloc_size);
	}
}

void InstructionWithParamsList::add_func_return(int return_size)
{
	add_easy(inst_leave);
	if (return_size > 4)
		add_easy(inst_ret, PKConstant, Size16, (void*)4);
	else
		add_easy(inst_ret);
}

// which part of the modr/m byte is used
enum{
	MRMNone,
	MRMReg,
	MRMModRM
};

string SizeOut(int size)
{
	if (size == Size8)		return "8";
	if (size == Size16)		return "16";
	if (size == Size32)		return "32";
	if (size == Size48)		return "48";
	if (size == Size64)		return "64";
	if (size == Size128)		return "128";
	return "???";
}


string get_size_name(int size)
{
	if (size == Size8)
		return "byte";
	if (size == Size16)
		return "word";
	if (size == Size32)
		return "dword";
	if (size == Size48)
		return "s48";
	if (size == Size64)
		return "qword";
	if (size == Size128)
		return "dqword";
	return "";
}

// parameter definition (filter for real parameters)
struct InstructionParamFuzzy{
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
		if (size != SizeUnknown)
			t += "  " + SizeOut(size);
		if (mrm_mode == MRMReg)
			t += "   /r";
		else if (mrm_mode == MRMModRM)
			t += "   /m";
	}else{
		t += "	None";
	}
	printf("%s\n", t.c_str());
}

// an instruction/opcode the cpu offers
struct CPUInstruction{
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

	//return (param1.match(iwp.p1)) && (param2.match(iwp.p2));
	bool b = (param1.match(iwp.p1)) && (param2.match(iwp.p2));
	/*if (b){
		msg_write("source: " + iwp.p1.str() + " " + iwp.p2.str());
		print();
	}*/
	return b;
}

// expands the short instruction parameters
//   returns true if mod/rm byte needed
bool _get_inst_param_(int param, InstructionParamFuzzy &ip)
{
	ip.reg = NULL;
	ip.reg_group = RegGroupNone;
	ip.mrm_mode = MRMNone;
	ip.reg_group = -1;
	ip._type_ = ParamTInvalid;
	ip.allow_register = false;
	ip.allow_immediate = false;
	ip.allow_memory_address = false;
	ip.allow_memory_indirect = false;
	ip.immediate_is_relative = false;
	if (param < 0){	ip.used = false;	ip._type_ = ParamTNone;	return false;	}
	ip.used = true;

	// is it a register?
	for (int i=0;i<Registers.num;i++)
		if (Registers[i].id == param){
			ip._type_ = ParamTRegister;
			ip.reg = &Registers[i];
			ip.allow_register = true;
			ip.reg_group = Registers[i].group;
			ip.size = Registers[i].size;
			return false;
		}
	// general reg / mem
	if ((param == Eb) || (param == Eq) || (param == Ew) || (param == Ed) || (param == E48)){
		ip._type_ = ParamTInvalid;//ParamTRegisterOrMem;
		ip.allow_register = true;
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMModRM;
		if (param == Eb)	ip.size = Size8;
		if (param == Ew)	ip.size = Size16;
		if (param == Ed)	ip.size = Size32;
		if (param == Eq)	ip.size = Size64;
		if (param == E48)	ip.size = Size48;
		return true;
	}
	// general reg (reg)
	if ((param == Gb) || (param == Gq) || (param == Gw) || (param == Gd)){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMReg;
		if (param == Gb)	ip.size = Size8;
		if (param == Gw)	ip.size = Size16;
		if (param == Gd)	ip.size = Size32;
		if (param == Gq)	ip.size = Size64;
		return true;
	}
	// general reg (mod)
	if ((param == Rb) || (param == Rq) || (param == Rw) || (param == Rd)){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMModRM;
		if (param == Rb)	ip.size = Size8;
		if (param == Rw)	ip.size = Size16;
		if (param == Rd)	ip.size = Size32;
		if (param == Rq)	ip.size = Size64;
		return true;
	}
	// immediate
	if ((param == Ib) || (param == Iq) || (param == Iw) || (param == Id) || (param == I48)){
		ip._type_ = ParamTImmediate;
		ip.allow_immediate = true;
		if (param == Ib)	ip.size = Size8;
		if (param == Iw)	ip.size = Size16;
		if (param == Id)	ip.size = Size32;
		if (param == Iq)	ip.size = Size64;
		if (param == I48)	ip.size = Size48;
		return false;
	}
	// immediate (relative)
	if ((param == Jb) || (param == Jq) || (param == Jw) || (param == Jd)){
		ip._type_ = ParamTImmediate;
		ip.allow_immediate = true;
		ip.immediate_is_relative = true;
		if (param == Jb)	ip.size = Size8;
		if (param == Jw)	ip.size = Size16;
		if (param == Jd)	ip.size = Size32;
		if (param == Jq)	ip.size = Size64;
		return false;
	}
	// mem
	if ((param == Ob) || (param == Oq) || (param == Ow) || (param == Od)){
		ip._type_ = ParamTMemory;
		ip.allow_memory_address = true;
		if (param == Ob)	ip.size = Size8;
		if (param == Ow)	ip.size = Size16;
		if (param == Od)	ip.size = Size32;
		if (param == Oq)	ip.size = Size64;
		return false;
	}
	// mem
	if ((param == Mb) || (param == Mq) || (param == Mw) || (param == Md)){
		ip._type_ = ParamTInvalid; // ...
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroupGeneral;
		ip.mrm_mode = MRMModRM;
		if (param == Mb)	ip.size = Size8;
		if (param == Mw)	ip.size = Size16;
		if (param == Md)	ip.size = Size32;
		if (param == Mq)	ip.size = Size64;
		return true;
	}
	// control reg
	if ((param == Cb) || (param == Cd) || (param == Cw) || (param == Cd)){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupControl;
		ip.mrm_mode = MRMReg;
		if (param == Cb)	ip.size = Size8;
		if (param == Cw)	ip.size = Size16;
		if (param == Cd)	ip.size = Size32;
		if (param == Cq)	ip.size = Size64;
		return true;
	}
	// segment reg
	if (param == Sw){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupSegment;
		ip.mrm_mode = MRMReg;
		ip.size = Size16;
		return true;
	}
	// xmm reg (reg)
	if (param == Xx){
		ip._type_ = ParamTRegister;
		ip.allow_register = true;
		ip.reg_group = RegGroupXmm;
		ip.mrm_mode = MRMReg;
		ip.size = Size128;
		return true;
	}
	msg_error("asm: unknown instparam (call Michi!)");
	msg_write(param);
	exit(0);
	return false;
}

enum
{
	OptSmallParam = 1,
	OptSmallAddr,
	OptBigParam,
	OptBigAddr,
	OptMediumParam,
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
	i.has_modrm  = m1 || m2 || (cap >= 0);
	i.has_small_param = (opt == OptSmallParam);
	i.has_small_addr = (opt == OptSmallAddr);
	i.has_big_param = (opt == OptBigParam);
	i.has_big_addr = (opt == OptBigAddr);
	i.has_fixed_param = (opt != OptSmallParam) && (opt != OptMediumParam) && (opt != OptBigParam);
	if ((i.has_big_param) && (InstructionSet.set != InstructionSetAMD64))
		return;

	if (inst == inst_lea)
		i.param2.size = SizeUnknown;
	
	i.name = InstructionNames[NUM_INSTRUCTION_NAMES].name;
	for (int j=0;j<NUM_INSTRUCTION_NAMES;j++)
		if (inst == InstructionNames[j].inst)
			i.name = InstructionNames[j].name;
	CPUInstructions.add(i);
}

string GetInstructionName(int inst)
{
	for (int i=0;i<Asm::NUM_INSTRUCTION_NAMES;i++)
		if (inst == InstructionNames[i].inst)
			return Asm::InstructionNames[i].name;
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
	if ((inst == inst_div) || (inst == inst_idiv) || (inst == inst_movss))
		return false;
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return (InstructionNames[i].name[0] != 'f');
	return true;
}

bool GetInstructionAllowGenReg(int inst)
{
	if (inst == inst_lea)
		return false;
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionNames[i].inst == inst)
			return (InstructionNames[i].name[0] != 'f');
	return true;
}

/*
#define NumInstructionsX86		319
CPUInstruction InstructionX86[NumInstructionsX86]={
};

int NumInstructions=0;
sInstruction *Instruction=NULL;

void SetInstructionSet(int set)
{
	msg_db_f("SetInstructionSet", 1+ASM_DB_LEVEL);
	if (set==InstructionSetDefault){
		if (sizeof(long)==8)
			set=InstructionSetAMD64;
		else
			set=InstructionSetX86;
	}
	
	set=InstructionSetX86;
	
	if (set==InstructionSetX86){
		NumInstructions=NumInstructionsX86;
		Instruction=InstructionX86;
		so("--------------------------------x86");
	}
	
	// build name table
	for (int i=0;i<NumInstructions;i++){
		Instruction[i].name==InstructionName[NUM_INSTRUCTION_NAMES].name;
		for (int j=0;j<NUM_INSTRUCTION_NAMES;j++)
			if (Instruction[i].inst==InstructionName[j].inst)
				Instruction[i].name=InstructionName[j].name;
	}
}*/



void Init(int set)
{
	if (set < 0){
		if (sizeof(void*) == 8)
			set = InstructionSetAMD64;
		else if (sizeof(void*) == 4)
			set = InstructionSetX86;
		else{
			msg_error("Asm: unknown instruction set");
			set = InstructionSetX86;
		}
	}
	InstructionSet.set = set;
	InstructionSet.pointer_size = 4;
	if (set == InstructionSetAMD64)
		InstructionSet.pointer_size = 8;

	for (int i=0;i<NUM_REG_ROOTS;i++)
		for (int j=0;j<=MAX_REG_SIZE;j++)
			RegResize[i][j] = -1;

	Registers.clear();
	add_reg("rax",	RegRax,	RegGroupGeneral,	Size64,	0);
	add_reg("eax",	RegEax,	RegGroupGeneral,	Size32,	0);
	add_reg("ax",	RegAx,	RegGroupGeneral,	Size16,	0);
	add_reg("ah",	RegAh,	RegGroupGeneral,	Size8,	0); // RegResize[] will be overwritten by al
	add_reg("al",	RegAl,	RegGroupGeneral,	Size8,	0);
	add_reg("rcx",	RegRcx,	RegGroupGeneral,	Size64,	1);
	add_reg("ecx",	RegEcx,	RegGroupGeneral,	Size32,	1);
	add_reg("cx",	RegCx,	RegGroupGeneral,	Size16,	1);
	add_reg("ch",	RegCh,	RegGroupGeneral,	Size8,	1);
	add_reg("cl",	RegCl,	RegGroupGeneral,	Size8,	1);
	add_reg("rdx",	RegRdx,	RegGroupGeneral,	Size64,	2);
	add_reg("edx",	RegEdx,	RegGroupGeneral,	Size32,	2);
	add_reg("dx",	RegDx,	RegGroupGeneral,	Size16,	2);
	add_reg("dh",	RegDh,	RegGroupGeneral,	Size8,	2);
	add_reg("dl",	RegDl,	RegGroupGeneral,	Size8,	2);
	add_reg("rbx",	RegRbx,	RegGroupGeneral,	Size64,	3);
	add_reg("ebx",	RegEbx,	RegGroupGeneral,	Size32,	3);
	add_reg("bx",	RegBx,	RegGroupGeneral,	Size16,	3);
	add_reg("bh",	RegBh,	RegGroupGeneral,	Size8,	3);
	add_reg("bl",	RegBl,	RegGroupGeneral,	Size8,	3);

	add_reg("rsp",	RegRsp,	RegGroupGeneral,	Size64,	4);
	add_reg("esp",	RegEsp,	RegGroupGeneral,	Size32,	4);
	add_reg("sp",	RegSp,	RegGroupGeneral,	Size16,	4);
	add_reg("rbp",	RegRbp,	RegGroupGeneral,	Size64,	5);
	add_reg("ebp",	RegEbp,	RegGroupGeneral,	Size32,	5);
	add_reg("bp",	RegBp,	RegGroupGeneral,	Size16,	5);
	add_reg("rsi",	RegRsi,	RegGroupGeneral,	Size64,	6);
	add_reg("esi",	RegEsi,	RegGroupGeneral,	Size32,	6);
	add_reg("si",	RegSi,	RegGroupGeneral,	Size16,	6);
	add_reg("rdi",	RegRdi,	RegGroupGeneral,	Size64,	7);
	add_reg("edi",	RegEdi,	RegGroupGeneral,	Size32,	7);
	add_reg("di",	RegDi,	RegGroupGeneral,	Size16,	7);

	add_reg("r8",	RegR8,	RegGroupGeneral2,	Size64,	8);
	add_reg("r8d",	RegR8d,	RegGroupGeneral2,	Size32,	8);
	add_reg("r9",	RegR9,	RegGroupGeneral2,	Size64,	9);
	add_reg("r9d",	RegR9d,	RegGroupGeneral2,	Size32,	9);
	add_reg("r10",	RegR10,	RegGroupGeneral2,	Size64,	10);
	add_reg("r10d",	RegR10d,RegGroupGeneral2,	Size32,	10);
	add_reg("r11",	RegR11,	RegGroupGeneral2,	Size64,	10);
	add_reg("r11d",	RegR11d,RegGroupGeneral2,	Size32,	11);
	add_reg("r12",	RegR12,	RegGroupGeneral2,	Size64,	12);
	add_reg("r12d",	RegR12d,RegGroupGeneral2,	Size32,	12);
	add_reg("r13",	RegR13,	RegGroupGeneral2,	Size64,	13);
	add_reg("r13d",	RegR13d,RegGroupGeneral2,	Size32,	13);
	add_reg("r14",	RegR14,	RegGroupGeneral2,	Size64,	14);
	add_reg("r14d",	RegR14d,RegGroupGeneral2,	Size32,	14);
	add_reg("r15",	RegR15,	RegGroupGeneral2,	Size64,	15);
	add_reg("r15d",	RegR15d,RegGroupGeneral2,	Size32,	15);

	add_reg("cs",	RegCs,	RegGroupSegment,	Size16);
	add_reg("ss",	RegSs,	RegGroupSegment,	Size16);
	add_reg("ds",	RegDs,	RegGroupSegment,	Size16);
	add_reg("es",	RegEs,	RegGroupSegment,	Size16);
	add_reg("fs",	RegFs,	RegGroupSegment,	Size16);
	add_reg("gs",	RegGs,	RegGroupSegment,	Size16);

	add_reg("cr0",	RegCr0,	RegGroupControl,	Size32);
	add_reg("cr1",	RegCr1,	RegGroupControl,	Size32);
	add_reg("cr2",	RegCr2,	RegGroupControl,	Size32);
	add_reg("cr3",	RegCr3,	RegGroupControl,	Size32);

	add_reg("st0",	RegSt0,	RegGroupX87,	Size32,	16); // ??? 32
	add_reg("st1",	RegSt1,	RegGroupX87,	Size32,	17);
	add_reg("st2",	RegSt2,	RegGroupX87,	Size32,	18);
	add_reg("st3",	RegSt3,	RegGroupX87,	Size32,	19);
	add_reg("st4",	RegSt4,	RegGroupX87,	Size32,	20);
	add_reg("st5",	RegSt5,	RegGroupX87,	Size32,	21);
	add_reg("st6",	RegSt6,	RegGroupX87,	Size32,	22);
	add_reg("st7",	RegSt7,	RegGroupX87,	Size32,	23);

	add_reg("xmm0",	RegXmm0,	RegGroupXmm,	Size128);
	add_reg("xmm1",	RegXmm1,	RegGroupXmm,	Size128);
	add_reg("xmm2",	RegXmm2,	RegGroupXmm,	Size128);
	add_reg("xmm3",	RegXmm3,	RegGroupXmm,	Size128);
	add_reg("xmm4",	RegXmm4,	RegGroupXmm,	Size128);
	add_reg("xmm5",	RegXmm5,	RegGroupXmm,	Size128);
	add_reg("xmm6",	RegXmm6,	RegGroupXmm,	Size128);
	add_reg("xmm7",	RegXmm7,	RegGroupXmm,	Size128);

	// create easy to access array
	RegisterByID.clear();
	for (int i=0;i<Registers.num;i++){
		if (RegisterByID.num <= Registers[i].id)
			RegisterByID.resize(Registers[i].id + 1);
		RegisterByID[Registers[i].id] = &Registers[i];
	}

	CPUInstructions.clear();
	add_inst(inst_db		,0x00	,0	,-1	,Ib	,-1);
	add_inst(inst_dw		,0x00	,0	,-1	,Iw	,-1);
	add_inst(inst_dd		,0x00	,0	,-1	,Id	,-1);
	add_inst(inst_add		,0x00	,1	,-1	,Eb	,Gb);
	add_inst(inst_add		,0x01	,1	,-1	,Ew	,Gw, OptSmallParam);
	add_inst(inst_add		,0x01	,1	,-1	,Ed	,Gd, OptMediumParam);
	add_inst(inst_add		,0x01	,1	,-1	,Eq	,Gq, OptBigParam);
	add_inst(inst_add		,0x02	,1	,-1	,Gb	,Eb);
	add_inst(inst_add		,0x03	,1	,-1	,Gw	,Eq, OptSmallParam);
	add_inst(inst_add		,0x03	,1	,-1	,Gd	,Ed, OptMediumParam);
	add_inst(inst_add		,0x03	,1	,-1	,Gq	,Eq, OptBigParam);
	add_inst(inst_add		,0x04	,1	,-1	,RegAl	,Ib);
	add_inst(inst_add		,0x05	,1	,-1	,RegAx, Iw, OptSmallParam);
	add_inst(inst_add		,0x05	,1	,-1	,RegEax, Id, OptMediumParam);
	add_inst(inst_add		,0x05	,1	,-1	,RegRax, Id, OptBigParam);
	add_inst(inst_push		,0x06	,1	,-1	,RegEs	,-1);
	add_inst(inst_pop		,0x07	,1	,-1	,RegEs	,-1);
	add_inst(inst_or		,0x08	,1	,-1	,Eb	,Gb);
	add_inst(inst_or		,0x09	,1	,-1	,Ew	,Gw, OptSmallParam);
	add_inst(inst_or		,0x09	,1	,-1	,Ed	,Gd, OptMediumParam);
	add_inst(inst_or		,0x09	,1	,-1	,Eq	,Gq, OptBigParam);
	add_inst(inst_or		,0x0a	,1	,-1	,Gb	,Eb);
	add_inst(inst_or,	0x0b,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_or,	0x0b,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_or,	0x0b,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_or		,0x0c	,1	,-1	,RegAl	,Ib);
	add_inst(inst_or		,0x0d	,1	,-1	,RegAx,	Iw, OptSmallParam);
	add_inst(inst_or		,0x0d	,1	,-1	,RegEax,	Id, OptMediumParam);
	add_inst(inst_or		,0x0d	,1	,-1	,RegRax,	Id, OptBigParam);
	add_inst(inst_push		,0x0e	,1	,-1	,RegCs	,-1);
	add_inst(inst_sldt		,0x000f	,2	,0	,Ew	,-1);
	add_inst(inst_str		,0x000f	,2	,1	,Ew	,-1);
	add_inst(inst_lldt		,0x000f	,2	,2	,Ew	,-1);
	add_inst(inst_ltr		,0x000f	,2	,3	,Ew	,-1);
	add_inst(inst_verr		,0x000f	,2	,4	,Ew	,-1);
	add_inst(inst_verw		,0x000f	,2	,5	,Ew	,-1);
	add_inst(inst_sgdt,	0x010f,	2,	0,	Mw,	-1, OptSmallParam);
	add_inst(inst_sgdt,	0x010f,	2,	0,	Md,	-1, OptMediumParam);
	add_inst(inst_sgdt,	0x010f,	2,	0,	Mq,	-1, OptBigParam);
	add_inst(inst_sidt,	0x010f,	2,	1,	Mw,	-1, OptSmallParam);
	add_inst(inst_sidt,	0x010f,	2,	1,	Md,	-1, OptMediumParam);
	add_inst(inst_sidt,	0x010f,	2,	1,	Mq,	-1, OptBigParam);
	add_inst(inst_lgdt,	0x010f,	2,	2,	Mw,	-1, OptSmallParam);
	add_inst(inst_lgdt,	0x010f,	2,	2,	Md,	-1, OptMediumParam);
	add_inst(inst_lgdt,	0x010f,	2,	2,	Mq,	-1, OptBigParam);
	add_inst(inst_lidt,	0x010f,	2,	3,	Mw,	-1, OptSmallParam);
	add_inst(inst_lidt,	0x010f,	2,	3,	Md,	-1, OptMediumParam);
	add_inst(inst_lidt,	0x010f,	2,	3,	Mq,	-1, OptBigParam);
	add_inst(inst_smsw		,0x010f	,2	,4	,Ew	,-1);
	add_inst(inst_lmsw		,0x010f	,2	,6	,Ew	,-1);
	add_inst(inst_mov		,0x200f	,2	,-1	,Rd	,Cd); // Fehler im Algorhytmus!!!!  (wirklich ???) -> Fehler in Tabelle?!?
	add_inst(inst_mov		,0x220f	,2	,-1	,Cd	,Rd);
	add_inst(inst_jo		,0x800f	,2	,-1	,Id	,-1, OptMediumParam); // 16/32 bit???
	add_inst(inst_jno		,0x810f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jb		,0x820f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jnb		,0x830f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jz		,0x840f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jnz		,0x850f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jbe		,0x860f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jnbe		,0x870f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_js		,0x880f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jns		,0x890f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jp		,0x8a0f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jnp		,0x8b0f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jl		,0x8c0f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jnl		,0x8d0f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jle		,0x8e0f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_jnle		,0x8f0f	,2	,-1	,Id	,-1, OptMediumParam);
	add_inst(inst_seto		,0x900f	,2	,-1	,Eb	,-1);
	add_inst(inst_setno		,0x910f	,2	,-1	,Eb	,-1);
	add_inst(inst_setb		,0x920f	,2	,-1	,Eb	,-1);
	add_inst(inst_setnb		,0x930f	,2	,-1	,Eb	,-1);
	add_inst(inst_setz		,0x940f	,2	,-1	,Eb	,-1);
	add_inst(inst_setnz		,0x950f	,2	,-1	,Eb	,-1);
	add_inst(inst_setbe		,0x960f	,2	,-1	,Eb	,-1);
	add_inst(inst_setnbe	,0x970f	,2	,-1	,Eb	,-1);
	add_inst(inst_sets		,0x980f	,2	,-1	,Eb	,-1); // error in table... "Ev"
	add_inst(inst_setns		,0x990f	,2	,-1	,Eb	,-1);
	add_inst(inst_setp		,0x9a0f	,2	,-1	,Eb	,-1);
	add_inst(inst_setnp		,0x9b0f	,2	,-1	,Eb	,-1);
	add_inst(inst_setl		,0x9c0f	,2	,-1	,Eb	,-1);
	add_inst(inst_setnl		,0x9d0f	,2	,-1	,Eb	,-1);
	add_inst(inst_setle		,0x9e0f	,2	,-1	,Eb	,-1);
	add_inst(inst_setnle	,0x9f0f	,2	,-1	,Eb	,-1);
	add_inst(inst_imul,	0xaf0f,	2,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_imul,	0xaf0f,	2,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_imul,	0xaf0f,	2,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_movzx,	0xb60f,	2,	-1,	Gw,	Eb, OptSmallParam);
	add_inst(inst_movzx,	0xb60f,	2,	-1,	Gd,	Eb, OptMediumParam);
	add_inst(inst_movzx,	0xb60f,	2,	-1,	Gq,	Eb, OptBigParam);
	add_inst(inst_movzx,	0xb70f,	2,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_movzx,	0xb70f,	2,	-1,	Gd,	Ew, OptMediumParam);
	add_inst(inst_movzx,	0xb70f,	2,	-1,	Gq,	Ew, OptBigParam);
	add_inst(inst_movsx,	0xbe0f,	2,	-1,	Gw,	Eb, OptSmallParam);
	add_inst(inst_movsx,	0xbe0f,	2,	-1,	Gd,	Eb, OptMediumParam);
	add_inst(inst_movsx,	0xbe0f,	2,	-1,	Gq,	Eb, OptBigParam);
	add_inst(inst_movsx,	0xbf0f,	2,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_movsx,	0xbf0f,	2,	-1,	Gd,	Ew, OptMediumParam);
	add_inst(inst_movsx,	0xbf0f,	2,	-1,	Gq,	Ew, OptBigParam);
	add_inst(inst_adc,	0x10	,1	,-1	,Eb	,Gb);
	add_inst(inst_adc,	0x11,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_adc,	0x11,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_adc,	0x11,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_adc,	0x12	,1	,-1	,Gb	,Eb);
	add_inst(inst_adc,	0x13,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_adc,	0x13,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_adc,	0x13,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_adc,	0x14	,1	,-1	,RegAl	,Ib);
	add_inst(inst_adc,	0x15	,1	,-1	,RegAx,	Iw, OptSmallParam);
	add_inst(inst_adc,	0x15	,1	,-1	,RegEax, Id, OptMediumParam);
	add_inst(inst_adc,	0x15	,1	,-1	,RegRax, Id, OptBigParam);
	add_inst(inst_push,	0x16	,1	,-1	,RegSs, -1);
	add_inst(inst_pop,	0x17	,1	,-1	,RegSs, -1);
	add_inst(inst_sbb,	0x18	,1	,-1	,Eb	,Gb);
	add_inst(inst_sbb,	0x19,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_sbb,	0x19,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_sbb,	0x19,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_sbb,	0x1a	,1	,-1	,Gb	,Eb);
	add_inst(inst_sbb,	0x1b,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_sbb,	0x1b,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_sbb,	0x1b,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_sbb,	0x1c	,1	,-1	,RegAl	,Ib);
	add_inst(inst_sbb,	0x1d	,1	,-1	,RegAx	,Iw, OptSmallParam);
	add_inst(inst_sbb,	0x1d	,1	,-1	,RegEax	,Id, OptMediumParam);
	add_inst(inst_sbb,	0x1d	,1	,-1	,RegRax	,Id, OptBigParam);
	add_inst(inst_push,	0x1e	,1	,-1	,RegDs	,-1);
	add_inst(inst_pop,	0x1f	,1	,-1	,RegDs	,-1);
	add_inst(inst_and,	0x20	,1	,-1	,Eb	,Gb);
	add_inst(inst_and,	0x21,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_and,	0x21,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_and,	0x21,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_and,	0x22	,1	,-1	,Gb	,Eb);
	add_inst(inst_and,	0x23,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_and,	0x23,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_and,	0x23,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_and,	0x24	,1	,-1	,RegAl	,Ib);
	add_inst(inst_and,	0x25	,1	,-1	,RegAx	,Iw, OptSmallParam);
	add_inst(inst_and,	0x25	,1	,-1	,RegEax	,Id, OptMediumParam);
	add_inst(inst_and,	0x25	,1	,-1	,RegRax	,Id, OptBigParam);
	add_inst(inst_sub,	0x28	,1	,-1	,Eb	,Gb);
	add_inst(inst_sub,	0x29,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_sub,	0x29,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_sub,	0x29,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_sub,	0x2a	,1	,-1	,Gb	,Eb);
	add_inst(inst_sub,	0x2b,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_sub,	0x2b,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_sub,	0x2b,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_sub,	0x2c	,1	,-1	,RegAl	,Ib);
	add_inst(inst_sub,	0x2d	,1	,-1	,RegAx	,Iw, OptSmallParam);
	add_inst(inst_sub,	0x2d	,1	,-1	,RegEax	,Id, OptMediumParam);
	add_inst(inst_sub,	0x2d	,1	,-1	,RegRax	,Id, OptBigParam);
	add_inst(inst_xor,	0x30	,1	,-1	,Eb	,Gb);
	add_inst(inst_xor,	0x31,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_xor,	0x31,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_xor,	0x31,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_xor,	0x32	,1	,-1	,Gb	,Eb);
	add_inst(inst_xor,	0x33,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_xor,	0x33,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_xor,	0x33,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_xor,	0x34	,1	,-1	,RegAl	,Ib);
	add_inst(inst_xor,	0x35	,1	,-1	,RegAx	,Iw, OptSmallParam);
	add_inst(inst_xor,	0x35	,1	,-1	,RegEax	,Id, OptMediumParam);
	add_inst(inst_xor,	0x35	,1	,-1	,RegRax	,Iq, OptBigParam);
	add_inst(inst_cmp,	0x38	,1	,-1	,Eb	,Gb);
	add_inst(inst_cmp,	0x39,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_cmp,	0x39,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_cmp,	0x39,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_cmp,	0x3a	,1	,-1	,Gb	,Eb);
	add_inst(inst_cmp,	0x3b,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_cmp,	0x3b,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_cmp,	0x3b,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_cmp,	0x3c	,1	,-1	,RegAl	,Ib);
	add_inst(inst_cmp,	0x3d	,1	,-1	,RegAx	,Iw, OptSmallParam);
	add_inst(inst_cmp,	0x3d	,1	,-1	,RegEax	,Id, OptMediumParam);
	add_inst(inst_cmp,	0x3d	,1	,-1	,RegRax	,Id, OptBigParam);
	if (set == InstructionSetX86){
		add_inst(inst_inc		,0x40	,1	,-1	,RegEax	,-1);
		add_inst(inst_inc		,0x41	,1	,-1	,RegEcx	,-1);
		add_inst(inst_inc		,0x42	,1	,-1	,RegEdx	,-1);
		add_inst(inst_inc		,0x43	,1	,-1	,RegEbx	,-1);
		add_inst(inst_inc		,0x44	,1	,-1	,RegEsp	,-1);
		add_inst(inst_inc		,0x45	,1	,-1	,RegEbp	,-1);
		add_inst(inst_inc		,0x46	,1	,-1	,RegEsi	,-1);
		add_inst(inst_inc		,0x47	,1	,-1	,RegEdi	,-1);
		add_inst(inst_dec		,0x48	,1	,-1	,RegEax	,-1);
		add_inst(inst_dec		,0x49	,1	,-1	,RegEcx	,-1);
		add_inst(inst_dec		,0x4a	,1	,-1	,RegEdx	,-1);
		add_inst(inst_dec		,0x4b	,1	,-1	,RegEbx	,-1);
		add_inst(inst_dec		,0x4c	,1	,-1	,RegEsp	,-1);
		add_inst(inst_dec		,0x4d	,1	,-1	,RegEbp	,-1);
		add_inst(inst_dec		,0x4e	,1	,-1	,RegEsi	,-1);
		add_inst(inst_dec		,0x4f	,1	,-1	,RegEdi	,-1);
	}
	if (set == InstructionSetX86){
		add_inst(inst_push		,0x50	,1	,-1	,RegEax	,-1);
		add_inst(inst_push		,0x51	,1	,-1	,RegEcx	,-1);
		add_inst(inst_push		,0x52	,1	,-1	,RegEdx	,-1);
		add_inst(inst_push		,0x53	,1	,-1	,RegEbx	,-1);
		add_inst(inst_push		,0x54	,1	,-1	,RegEsp	,-1);
		add_inst(inst_push		,0x55	,1	,-1	,RegEbp	,-1);
		add_inst(inst_push		,0x56	,1	,-1	,RegEsi	,-1);
		add_inst(inst_push		,0x57	,1	,-1	,RegEdi	,-1);
		add_inst(inst_pop		,0x58	,1	,-1	,RegEax	,-1);
		add_inst(inst_pop		,0x59	,1	,-1	,RegEcx	,-1);
		add_inst(inst_pop		,0x5a	,1	,-1	,RegEdx	,-1);
		add_inst(inst_pop		,0x5b	,1	,-1	,RegEbx	,-1);
		add_inst(inst_pop		,0x5c	,1	,-1	,RegEsp	,-1);
		add_inst(inst_pop		,0x5d	,1	,-1	,RegEbp	,-1);
		add_inst(inst_pop		,0x5e	,1	,-1	,RegEsi	,-1);
		add_inst(inst_pop		,0x5f	,1	,-1	,RegEdi	,-1);
	}else if (set == InstructionSetAMD64){
		add_inst(inst_push		,0x50	,1	,-1	,RegRax	,-1);
		add_inst(inst_push		,0x51	,1	,-1	,RegRcx	,-1);
		add_inst(inst_push		,0x52	,1	,-1	,RegRdx	,-1);
		add_inst(inst_push		,0x53	,1	,-1	,RegRbx	,-1);
		add_inst(inst_push		,0x54	,1	,-1	,RegRsp	,-1);
		add_inst(inst_push		,0x55	,1	,-1	,RegRbp	,-1);
		add_inst(inst_push		,0x56	,1	,-1	,RegRsi	,-1);
		add_inst(inst_push		,0x57	,1	,-1	,RegRdi	,-1);
		add_inst(inst_pop		,0x58	,1	,-1	,RegRax	,-1);
		add_inst(inst_pop		,0x59	,1	,-1	,RegRcx	,-1);
		add_inst(inst_pop		,0x5a	,1	,-1	,RegRdx	,-1);
		add_inst(inst_pop		,0x5b	,1	,-1	,RegRbx	,-1);
		add_inst(inst_pop		,0x5c	,1	,-1	,RegRsp	,-1);
		add_inst(inst_pop		,0x5d	,1	,-1	,RegRbp	,-1);
		add_inst(inst_pop		,0x5e	,1	,-1	,RegRsi	,-1);
		add_inst(inst_pop		,0x5f	,1	,-1	,RegRdi	,-1);
	}
	add_inst(inst_pusha		,0x60	,1	,-1	,-1	,-1);
	add_inst(inst_popa		,0x61	,1	,-1	,-1	,-1);
	add_inst(inst_push,	0x68,	1,	-1,	Iw,	-1, OptSmallParam);
	add_inst(inst_push,	0x68,	1,	-1,	Id,	-1, OptMediumParam);
	add_inst(inst_push,	0x68,	1,	-1,	Iq,	-1, OptBigParam);
	add_inst(inst_imul,	0x69,	1,	-1,	Ew,	Iw, OptSmallParam);
	add_inst(inst_imul,	0x69,	1,	-1,	Ed,	Id, OptMediumParam);
	add_inst(inst_imul,	0x69,	1,	-1,	Eq,	Id, OptBigParam);
	add_inst(inst_push		,0x6a	,1	,-1	,Ib	,-1);
	add_inst(inst_jo		,0x70	,1	,-1	,Jb	,-1);
	add_inst(inst_jno		,0x71	,1	,-1	,Jb	,-1);
	add_inst(inst_jb		,0x72	,1	,-1	,Jb	,-1);
	add_inst(inst_jnb		,0x73	,1	,-1	,Jb	,-1);
	add_inst(inst_jz		,0x74	,1	,-1	,Jb	,-1);
	add_inst(inst_jnz		,0x75	,1	,-1	,Jb	,-1);
	add_inst(inst_jbe		,0x76	,1	,-1	,Jb	,-1);
	add_inst(inst_jnbe		,0x77	,1	,-1	,Jb	,-1);
	add_inst(inst_js		,0x78	,1	,-1	,Jb	,-1);
	add_inst(inst_jns		,0x79	,1	,-1	,Jb	,-1);
	add_inst(inst_jp		,0x7a	,1	,-1	,Jb	,-1);
	add_inst(inst_jnp		,0x7b	,1	,-1	,Jb	,-1);
	add_inst(inst_jl		,0x7c	,1	,-1	,Jb	,-1);
	add_inst(inst_jnl		,0x7d	,1	,-1	,Jb	,-1);
	add_inst(inst_jle		,0x7e	,1	,-1	,Jb	,-1);
	add_inst(inst_jnle		,0x7f	,1	,-1	,Jb	,-1);
	// Immediate Group 1
	add_inst(inst_add		,0x80	,1	,0	,Eb	,Ib);
	add_inst(inst_or		,0x80	,1	,1	,Eb	,Ib);
	add_inst(inst_adc		,0x80	,1	,2	,Eb	,Ib);
	add_inst(inst_sbb		,0x80	,1	,3	,Eb	,Ib);
	add_inst(inst_and		,0x80	,1	,4	,Eb	,Ib);
	add_inst(inst_sub		,0x80	,1	,5	,Eb	,Ib);
	add_inst(inst_xor		,0x80	,1	,6	,Eb	,Ib);
	add_inst(inst_cmp		,0x80	,1	,7	,Eb	,Ib);
	add_inst(inst_add,	0x81,	1,	0,	Ew,	Iw, OptSmallParam);
	add_inst(inst_add,	0x81,	1,	0,	Ed,	Id, OptMediumParam);
	add_inst(inst_add,	0x81,	1,	0,	Eq,	Id, OptBigParam);
	add_inst(inst_or,	0x81,	1,	1,	Ew,	Iw, OptSmallParam);
	add_inst(inst_or,	0x81,	1,	1,	Ed,	Id, OptMediumParam);
	add_inst(inst_or,	0x81,	1,	1,	Eq,	Id, OptBigParam);
	add_inst(inst_adc,	0x81,	1,	2,	Ew,	Iw, OptSmallParam);
	add_inst(inst_adc,	0x81,	1,	2,	Ed,	Id, OptMediumParam);
	add_inst(inst_adc,	0x81,	1,	2,	Eq,	Id, OptBigParam);
	add_inst(inst_sbb,	0x81,	1,	3,	Ew,	Iw, OptSmallParam);
	add_inst(inst_sbb,	0x81,	1,	3,	Ed,	Id, OptMediumParam);
	add_inst(inst_sbb,	0x81,	1,	3,	Eq,	Id, OptBigParam);
	add_inst(inst_and,	0x81,	1,	4,	Ew,	Iw, OptSmallParam);
	add_inst(inst_and,	0x81,	1,	4,	Ed,	Id, OptMediumParam);
	add_inst(inst_and,	0x81,	1,	4,	Eq,	Id, OptBigParam);
	add_inst(inst_sub,	0x81,	1,	5,	Ew,	Iw, OptSmallParam);
	add_inst(inst_sub,	0x81,	1,	5,	Ed,	Id, OptMediumParam);
	add_inst(inst_sub,	0x81,	1,	5,	Eq,	Id, OptBigParam);
	add_inst(inst_xor,	0x81,	1,	6,	Ew,	Iw, OptSmallParam);
	add_inst(inst_xor,	0x81,	1,	6,	Ed,	Id, OptMediumParam);
	add_inst(inst_xor,	0x81,	1,	6,	Eq,	Id, OptBigParam);
	add_inst(inst_cmp,	0x81,	1,	7,	Ew,	Iw, OptSmallParam);
	add_inst(inst_cmp,	0x81,	1,	7,	Ed,	Id, OptMediumParam);
	add_inst(inst_cmp,	0x81,	1,	7,	Eq,	Id, OptBigParam);
	add_inst(inst_add,	0x83,	1,	0,	Ew,	Ib, OptSmallParam);
	add_inst(inst_add,	0x83,	1,	0,	Ed,	Ib, OptMediumParam);
	add_inst(inst_add,	0x83,	1,	0,	Eq,	Ib, OptBigParam);
	add_inst(inst_or,	0x83,	1,	1,	Ew,	Ib, OptSmallParam);
	add_inst(inst_or,	0x83,	1,	1,	Ed,	Ib, OptMediumParam);
	add_inst(inst_or,	0x83,	1,	1,	Eq,	Ib, OptBigParam);
	add_inst(inst_adc,	0x83,	1,	2,	Ew,	Ib, OptSmallParam);
	add_inst(inst_adc,	0x83,	1,	2,	Ed,	Ib, OptMediumParam);
	add_inst(inst_adc,	0x83,	1,	2,	Eq,	Ib, OptBigParam);
	add_inst(inst_sbb,	0x83,	1,	3,	Ew,	Ib, OptSmallParam);
	add_inst(inst_sbb,	0x83,	1,	3,	Ed,	Ib, OptMediumParam);
	add_inst(inst_sbb,	0x83,	1,	3,	Eq,	Ib, OptBigParam);
	add_inst(inst_and,	0x83,	1,	4,	Ew,	Ib, OptSmallParam);
	add_inst(inst_and,	0x83,	1,	4,	Ed,	Ib, OptMediumParam);
	add_inst(inst_and,	0x83,	1,	4,	Eq,	Ib, OptBigParam);
	add_inst(inst_sub,	0x83,	1,	5,	Ew,	Ib, OptSmallParam);
	add_inst(inst_sub,	0x83,	1,	5,	Ed,	Ib, OptMediumParam);
	add_inst(inst_sub,	0x83,	1,	5,	Eq,	Ib, OptBigParam);
	add_inst(inst_xor,	0x83,	1,	6,	Ew,	Ib, OptSmallParam);
	add_inst(inst_xor,	0x83,	1,	6,	Ed,	Ib, OptMediumParam);
	add_inst(inst_xor,	0x83,	1,	6,	Eq,	Ib, OptBigParam);
	add_inst(inst_cmp,	0x83,	1,	7,	Ew,	Ib, OptSmallParam);
	add_inst(inst_cmp,	0x83,	1,	7,	Ed,	Ib, OptMediumParam);
	add_inst(inst_cmp,	0x83,	1,	7,	Eq,	Ib, OptBigParam);
	add_inst(inst_test,	0x84	,1	,-1	,Eb	,Gb);
	add_inst(inst_test,	0x85,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_test,	0x85,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_test,	0x85,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_xchg,	0x86	,1	,-1	,Eb	,Gb);
	add_inst(inst_xchg,	0x87,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_xchg,	0x87,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_xchg,	0x87,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_mov,	0x88	,1	,-1	,Eb	,Gb);
	add_inst(inst_mov,	0x89,	1,	-1,	Ew,	Gw, OptSmallParam);
	add_inst(inst_mov,	0x89,	1,	-1,	Ed,	Gd, OptMediumParam);
	add_inst(inst_mov,	0x89,	1,	-1,	Eq,	Gq, OptBigParam);
	add_inst(inst_mov,	0x8a	,1	,-1	,Gb	,Eb);
	add_inst(inst_mov,	0x8b,	1,	-1,	Gw,	Ew, OptSmallParam);
	add_inst(inst_mov,	0x8b,	1,	-1,	Gd,	Ed, OptMediumParam);
	add_inst(inst_mov,	0x8b,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_mov,	0x8c	,1	,-1	,Ew	,Sw	);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gw,	Ew, OptSmallParam);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gd,	Ed, OptMediumParam);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(inst_lea,	0x8d,	1,	-1,	Gw,	Mw, OptSmallParam);
	add_inst(inst_lea,	0x8d,	1,	-1,	Gd,	Md, OptMediumParam);
	add_inst(inst_lea,	0x8d,	1,	-1,	Gq,	Mq, OptBigParam);
	add_inst(inst_mov,	0x8e	,1	,-1	,Sw	,Ew, OptMediumParam);
	add_inst(inst_pop,	0x8f,	1,	-1,	Ew,	-1, OptSmallParam);
	add_inst(inst_pop,	0x8f,	1,	-1,	Ed,	-1, OptMediumParam);
	add_inst(inst_pop,	0x8f,	1,	-1,	Eq,	-1, OptBigParam);
	add_inst(inst_nop		,0x90	,1	,-1	,-1	,-1);
	add_inst(inst_xchg		,0x91	,1	,-1	,RegAx	,RegCx, OptSmallParam);
	add_inst(inst_xchg		,0x92	,1	,-1	,RegAx	,RegDx, OptSmallParam);
	add_inst(inst_xchg		,0x93	,1	,-1	,RegAx	,RegBx, OptSmallParam);
	add_inst(inst_xchg		,0x94	,1	,-1	,RegAx	,RegSp, OptSmallParam);
	add_inst(inst_xchg		,0x95	,1	,-1	,RegAx	,RegBp, OptSmallParam);
	add_inst(inst_xchg		,0x96	,1	,-1	,RegAx	,RegSi, OptSmallParam);
	add_inst(inst_xchg		,0x97	,1	,-1	,RegAx	,RegDi, OptSmallParam);
	add_inst(inst_xchg		,0x91	,1	,-1	,RegEax	,RegEcx, OptMediumParam);
	add_inst(inst_xchg		,0x92	,1	,-1	,RegEax	,RegEdx, OptMediumParam);
	add_inst(inst_xchg		,0x93	,1	,-1	,RegEax	,RegEbx, OptMediumParam);
	add_inst(inst_xchg		,0x94	,1	,-1	,RegEax	,RegEsp, OptMediumParam);
	add_inst(inst_xchg		,0x95	,1	,-1	,RegEax	,RegEbp, OptMediumParam);
	add_inst(inst_xchg		,0x96	,1	,-1	,RegEax	,RegEsi, OptMediumParam);
	add_inst(inst_xchg		,0x97	,1	,-1	,RegEax	,RegEdi, OptMediumParam);
	add_inst(inst_xchg		,0x91	,1	,-1	,RegRax	,RegRcx, OptBigParam);
	add_inst(inst_xchg		,0x92	,1	,-1	,RegRax	,RegRdx, OptBigParam);
	add_inst(inst_xchg		,0x93	,1	,-1	,RegRax	,RegRbx, OptBigParam);
	add_inst(inst_xchg		,0x94	,1	,-1	,RegRax	,RegRsp, OptBigParam);
	add_inst(inst_xchg		,0x95	,1	,-1	,RegRax	,RegRbp, OptBigParam);
	add_inst(inst_xchg		,0x96	,1	,-1	,RegRax	,RegRsi, OptBigParam);
	add_inst(inst_xchg		,0x97	,1	,-1	,RegRax	,RegRdi, OptBigParam);
	add_inst(inst_cbw_cwde	,0x98	,1	,-1	,-1 ,-1);
	add_inst(inst_cgq_cwd	,0x99	,1	,-1	,-1 ,-1);
	add_inst(inst_mov		,0xa0	,1	,-1	,RegAl	,Ob, 0, true);
	add_inst(inst_mov		,0xa1	,1	,-1	,RegAx	,Ow, OptSmallParam, true);
	add_inst(inst_mov		,0xa1	,1	,-1	,RegEax	,Od, OptMediumParam, true);
	add_inst(inst_mov		,0xa1	,1	,-1	,RegRax	,Oq, OptBigParam, true);
	add_inst(inst_mov		,0xa2	,1	,-1	,Ob	,RegAl, 0, true);
	add_inst(inst_mov,	0xa3,	1,	-1,	Ow,	RegAx, OptSmallParam, true);
	add_inst(inst_mov,	0xa3,	1,	-1,	Od,	RegEax, OptMediumParam, true);
	add_inst(inst_mov,	0xa3,	1,	-1,	Oq,	RegRax, OptBigParam, true);
	add_inst(inst_movs_b_ds_esi_es_edi	,0xa4	,1	,-1	,-1,-1);
	add_inst(inst_movs_ds_esi_es_edi	,0xa5	,1	,-1	,-1,-1);
	add_inst(inst_cmps_b_ds_esi_es_edi	,0xa6	,1	,-1	,-1,-1);
	add_inst(inst_cmps_ds_esi_es_edi	,0xa7	,1	,-1	,-1,-1);
	add_inst(inst_mov		,0xb0	,1	,-1	,RegAl	,Ib);
	add_inst(inst_mov		,0xb1	,1	,-1	,RegCl	,Ib);
	add_inst(inst_mov		,0xb2	,1	,-1	,RegDl	,Ib);
	add_inst(inst_mov		,0xb3	,1	,-1	,RegBl	,Ib);
	add_inst(inst_mov		,0xb4	,1	,-1	,RegAh	,Ib);
	add_inst(inst_mov		,0xb5	,1	,-1	,RegCh	,Ib);
	add_inst(inst_mov		,0xb6	,1	,-1	,RegDh	,Ib);
	add_inst(inst_mov		,0xb7	,1	,-1	,RegBh	,Ib);
	add_inst(inst_mov		,0xb8	,1	,-1	,RegEax	,Id, OptMediumParam);
	add_inst(inst_mov		,0xb9	,1	,-1	,RegEcx	,Id, OptMediumParam);
	add_inst(inst_mov		,0xba	,1	,-1	,RegEdx	,Id, OptMediumParam);
	add_inst(inst_mov		,0xbb	,1	,-1	,RegEbx	,Id, OptMediumParam);
	add_inst(inst_mov		,0xbc	,1	,-1	,RegEsp	,Id, OptMediumParam);
	add_inst(inst_mov		,0xbd	,1	,-1	,RegEbp	,Id, OptMediumParam);
	add_inst(inst_mov		,0xbe	,1	,-1	,RegEsi	,Id, OptMediumParam);
	add_inst(inst_mov		,0xbf	,1	,-1	,RegEdi	,Id, OptMediumParam);
	add_inst(inst_mov		,0xb8	,1	,-1	,RegAx	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xb9	,1	,-1	,RegCx	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xba	,1	,-1	,RegDx	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xbb	,1	,-1	,RegBx	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xbc	,1	,-1	,RegSp	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xbd	,1	,-1	,RegBp	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xbe	,1	,-1	,RegSi	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xbf	,1	,-1	,RegDi	,Iw, OptSmallParam);
	add_inst(inst_mov		,0xb8	,1	,-1	,RegRax	,Iq, OptBigParam);
	add_inst(inst_mov		,0xb9	,1	,-1	,RegRcx	,Iq, OptBigParam);
	add_inst(inst_mov		,0xba	,1	,-1	,RegRdx	,Iq, OptBigParam);
	add_inst(inst_mov		,0xbb	,1	,-1	,RegRbx	,Iq, OptBigParam);
	add_inst(inst_mov		,0xbc	,1	,-1	,RegRsp	,Iq, OptBigParam);
	add_inst(inst_mov		,0xbd	,1	,-1	,RegRbp	,Iq, OptBigParam);
	add_inst(inst_mov		,0xbe	,1	,-1	,RegRsi	,Iq, OptBigParam);
	add_inst(inst_mov		,0xbf	,1	,-1	,RegRdi	,Iq, OptBigParam);
	// Shift Group 2
	add_inst(inst_rol		,0xc0	,1	,0	,Eb	,Ib);
	add_inst(inst_ror		,0xc0	,1	,1	,Eb	,Ib);
	add_inst(inst_rcl		,0xc0	,1	,2	,Eb	,Ib);
	add_inst(inst_rcr		,0xc0	,1	,3	,Eb	,Ib);
	add_inst(inst_shl		,0xc0	,1	,4	,Eb	,Ib);
	add_inst(inst_shr		,0xc0	,1	,5	,Eb	,Ib);
	add_inst(inst_sar		,0xc0	,1	,7	,Eb	,Ib);
	add_inst(inst_rol,	0xc1,	1,	0,	Ew,	Ib, OptSmallParam); // even though the table says Iv
	add_inst(inst_rol,	0xc1,	1,	0,	Ed,	Ib, OptMediumParam);
	add_inst(inst_rol,	0xc1,	1,	0,	Eq,	Ib, OptBigParam);
	add_inst(inst_ror,	0xc1,	1,	1,	Ew,	Ib, OptSmallParam);
	add_inst(inst_ror,	0xc1,	1,	1,	Ed,	Ib, OptMediumParam);
	add_inst(inst_ror,	0xc1,	1,	1,	Eq,	Ib, OptBigParam);
	add_inst(inst_rcl,	0xc1,	1,	2,	Ew,	Ib, OptSmallParam);
	add_inst(inst_rcl,	0xc1,	1,	2,	Ed,	Ib, OptMediumParam);
	add_inst(inst_rcl,	0xc1,	1,	2,	Eq,	Ib, OptBigParam);
	add_inst(inst_rcr,	0xc1,	1,	3,	Ew,	Ib, OptSmallParam);
	add_inst(inst_rcr,	0xc1,	1,	3,	Ed,	Ib, OptMediumParam);
	add_inst(inst_rcr,	0xc1,	1,	3,	Eq,	Ib, OptBigParam);
	add_inst(inst_shl,	0xc1,	1,	4,	Ew,	Ib, OptSmallParam);
	add_inst(inst_shl,	0xc1,	1,	4,	Ed,	Ib, OptMediumParam);
	add_inst(inst_shl,	0xc1,	1,	4,	Eq,	Ib, OptBigParam);
	add_inst(inst_shr,	0xc1,	1,	5,	Ew,	Ib, OptSmallParam);
	add_inst(inst_shr,	0xc1,	1,	5,	Ed,	Ib, OptMediumParam);
	add_inst(inst_shr,	0xc1,	1,	5,	Eq,	Ib, OptBigParam);
	add_inst(inst_sar,	0xc1,	1,	7,	Ew,	Ib, OptSmallParam);
	add_inst(inst_sar,	0xc1,	1,	7,	Ed,	Ib, OptMediumParam);
	add_inst(inst_sar,	0xc1,	1,	7,	Eq,	Ib, OptBigParam);
	add_inst(inst_ret		,0xc2	,1	,-1	,Iw	,-1);
	add_inst(inst_ret		,0xc3	,1	,-1	,-1	,-1);
	add_inst(inst_mov		,0xc6	,1	,-1	,Eb	,Ib);
	add_inst(inst_mov,	0xc7,	1,	-1,	Ew,	Iw, OptSmallParam);
	add_inst(inst_mov,	0xc7,	1,	-1,	Ed,	Id, OptMediumParam);
	add_inst(inst_mov,	0xc7,	1,	-1,	Eq,	Id, OptBigParam);
	add_inst(inst_leave		,0xc9	,1	,-1	,-1	,-1);
	add_inst(inst_ret_far	,0xca	,1	,-1	,Iw	,-1);
	add_inst(inst_ret_far	,0xcb	,1	,-1	,-1	,-1);
	add_inst(inst_int		,0xcd	,1	,-1	,Ib	,-1);
	add_inst(inst_iret		,0xcf	,1	,-1	,-1	,-1);
	add_inst(inst_rol,	0xd3,	1,	0,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_rol,	0xd3,	1,	0,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_rol,	0xd3,	1,	0,	Eq,	RegCl, OptBigParam);
	add_inst(inst_ror,	0xd3,	1,	1,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_ror,	0xd3,	1,	1,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_ror,	0xd3,	1,	1,	Eq,	RegCl, OptBigParam);
	add_inst(inst_rcl,	0xd3,	1,	2,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_rcl,	0xd3,	1,	2,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_rcl,	0xd3,	1,	2,	Eq,	RegCl, OptBigParam);
	add_inst(inst_rcr,	0xd3,	1,	3,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_rcr,	0xd3,	1,	3,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_rcr,	0xd3,	1,	3,	Eq,	RegCl, OptBigParam);
	add_inst(inst_shl,	0xd3,	1,	4,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_shl,	0xd3,	1,	4,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_shl,	0xd3,	1,	4,	Eq,	RegCl, OptBigParam);
	add_inst(inst_shr,	0xd3,	1,	5,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_shr,	0xd3,	1,	5,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_shr,	0xd3,	1,	5,	Eq,	RegCl, OptBigParam);
	add_inst(inst_sar,	0xd3,	1,	7,	Ew,	RegCl, OptSmallParam);
	add_inst(inst_sar,	0xd3,	1,	7,	Ed,	RegCl, OptMediumParam);
	add_inst(inst_sar,	0xd3,	1,	7,	Eq,	RegCl, OptBigParam);
	add_inst(inst_fadd,	0xd8,	1,	0,	Ed,	-1);
	add_inst(inst_fmul,	0xd8,	1,	1,	Ed,	-1);
	add_inst(inst_fsub,	0xd8,	1,	4,	Ed,	-1);
	add_inst(inst_fdiv,	0xd8,	1,	6,	Ed,	-1);
	add_inst(inst_fld,	0xd9,	1,	0,	Md,	-1);
	add_inst(inst_fld1,	0xe8d9,	2,	-1,	-1,	-1);
	add_inst(inst_fldz,	0xeed9,	2,	-1,	-1,	-1);
	add_inst(inst_fldpi,	0xebd9,	2,	-1,	-1,	-1);
	add_inst(inst_fst,	0xd9,	1,	2,	Md,	-1);
	add_inst(inst_fstp,	0xd9,	1,	3,	Md,	-1);
	add_inst(inst_fldcw,	0xd9,	1,	5,	Mw,	-1);
	add_inst(inst_fnstcw,	0xd9,	1,	7,	Mw,	-1);
	add_inst(inst_fxch		,0xc9d9	,2	,-1	,RegSt0	,RegSt1);
	add_inst(inst_fucompp	,0xe9da	,2	,-1	,RegSt0	,RegSt1);

	add_inst(inst_fsqrt,	0xfad9,	2,	-1,	-1, -1);
	add_inst(inst_fsin,	0xfed9,	2,	-1,	-1, -1);
	add_inst(inst_fcos,	0xffd9,	2,	-1,	-1, -1);
	add_inst(inst_fptan,	0xf2d9,	2,	-1,	-1, -1);
	add_inst(inst_fpatan,	0xf3d9,	2,	-1,	-1, -1);
	add_inst(inst_fyl2x,	0xf1d9,	2,	-1,	-1, -1);
	add_inst(inst_fistp,	0xdb	,1	,3	,Md	,-1);
	add_inst(inst_fild,	0xdb,	1,	0,	Ed,	-1);
	add_inst(inst_faddp,	0xde,	1,	0,	Ed,	-1);
	add_inst(inst_fmulp,	0xde,	1,	1,	Ed,	-1);
	add_inst(inst_fsubp,	0xde,	1,	5,	Ed,	-1);
	add_inst(inst_fdivp,	0xde,	1,	7,	Ed,	-1); // de.f9 ohne Parameter...?
	add_inst(inst_fnstsw	,0xe0df	,2	,-1	,RegAx	,-1);
	add_inst(inst_loopne	,0xe0	,1	,-1	,Jb	,-1);
	add_inst(inst_loope		,0xe1	,1	,-1	,Jb	,-1);
	add_inst(inst_loop		,0xe2	,1	,-1	,Jb	,-1);
	add_inst(inst_in		,0xe4	,1	,-1	,RegAl	,Ib);
	add_inst(inst_in		,0xe5	,1	,-1	,RegEax,Ib);
	add_inst(inst_out		,0xe6	,1	,-1	,Ib	,RegAl);
	add_inst(inst_out		,0xe7	,1	,-1	,Ib	,RegEax);
	add_inst(inst_call,	0xe8,	1,	-1,	Jw,	-1, OptSmallParam); // well... "Av" in tyble
	add_inst(inst_call,	0xe8,	1,	-1,	Jd,	-1, OptMediumParam);
	add_inst(inst_call,	0xe8,	1,	-1,	Jq,	-1, OptBigParam);
	add_inst(inst_jmp,	0xe9,	1,	-1,	Jw,	-1, OptSmallParam); // miswritten in the table
	add_inst(inst_jmp,	0xe9,	1,	-1,	Jd,	-1, OptMediumParam);
	add_inst(inst_jmp,	0xe9,	1,	-1,	Jq,	-1, OptBigParam);
//	add_inst(inst_jmp		,0xea	,1	,-1, Ap, -1); TODO
	add_inst(inst_jmp_far, 0xea, 1, -1, Id, -1, OptSmallParam);
	add_inst(inst_jmp_far, 0xea, 1, -1, I48, -1, OptMediumParam);
	add_inst(inst_jmp		,0xeb	,1	,-1, Jb, -1);
	add_inst(inst_in		,0xec	,1	,-1, RegAl, RegDx);
	add_inst(inst_in		,0xed	,1	,-1, RegEax, RegDx);
	add_inst(inst_out		,0xee	,1	,-1, RegDx, RegAl);
	add_inst(inst_out		,0xef	,1	,-1, RegDx, RegEax);
	add_inst(inst_lock		,0xf0	,1	,-1	,-1	,-1);
	/*add_inst(inst_repne		,0xf2	,1	,-1	,-1	,-1);
	add_inst(inst_rep		,0xf3	,1	,-1	,-1	,-1);*/
	add_inst(inst_hlt		,0xf4	,1	,-1	,-1	,-1);
	add_inst(inst_cmc		,0xf5	,1	,-1	,-1	,-1);
	// Unary Group 3
	add_inst(inst_test		,0xf6	,1	,0	,Eb	,Ib);
	add_inst(inst_not		,0xf6	,1	,2	,Eb	,-1);
	add_inst(inst_neg		,0xf6	,1	,3	,Eb	,-1);
	add_inst(inst_mul		,0xf6	,1	,4	,RegAl	,Eb);
	add_inst(inst_imul		,0xf6	,1	,5	,RegAl	,Eb);
	add_inst(inst_div		,0xf6	,1	,6	,RegAl	,Eb);
	add_inst(inst_idiv		,0xf6	,1	,7	,Eb	,-1);
	add_inst(inst_test,	0xf7,	1,	0,	Ew,	Iw, OptSmallParam);
	add_inst(inst_test,	0xf7,	1,	0,	Ed,	Id, OptMediumParam);
	add_inst(inst_test,	0xf7,	1,	0,	Eq,	Id, OptBigParam);
	add_inst(inst_not,	0xf7,	1,	2,	Ew,	-1, OptSmallParam);
	add_inst(inst_not,	0xf7,	1,	2,	Ed,	-1, OptMediumParam);
	add_inst(inst_not,	0xf7,	1,	2,	Eq,	-1, OptBigParam);
	add_inst(inst_neg,	0xf7,	1,	3,	Ew,	-1, OptSmallParam);
	add_inst(inst_neg,	0xf7,	1,	3,	Ed,	-1, OptMediumParam);
	add_inst(inst_neg,	0xf7,	1,	3,	Eq,	-1, OptBigParam);
	add_inst(inst_mul		,0xf7	,1	,4	,RegEax	,Ed, OptMediumParam);
	add_inst(inst_imul		,0xf7	,1	,5	,RegEax	,Ed, OptMediumParam);
	add_inst(inst_div		,0xf7	,1	,6	,RegEax	,Ed, OptMediumParam);
	add_inst(inst_idiv		,0xf7	,1	,7	,RegEax	,Ed, OptMediumParam);
	add_inst(inst_mul		,0xf7	,1	,4	,RegAx	,Ed, OptSmallParam);
	add_inst(inst_imul		,0xf7	,1	,5	,RegAx	,Ed, OptSmallParam);
	add_inst(inst_div		,0xf7	,1	,6	,RegAx	,Ed, OptSmallParam);
	add_inst(inst_idiv		,0xf7	,1	,7	,RegAx	,Ed, OptSmallParam);
	add_inst(inst_mul		,0xf7	,1	,4	,RegRax	,Eq, OptBigParam);
	add_inst(inst_imul		,0xf7	,1	,5	,RegRax	,Eq, OptBigParam);
	add_inst(inst_div		,0xf7	,1	,6	,RegRax	,Eq, OptBigParam);
	add_inst(inst_idiv		,0xf7	,1	,7	,RegRax	,Eq, OptBigParam);
	add_inst(inst_clc		,0xf8	,1	,-1	,-1	,-1);
	add_inst(inst_stc		,0xf9	,1	,-1	,-1	,-1);
	add_inst(inst_cli		,0xfa	,1	,-1	,-1	,-1);
	add_inst(inst_sti		,0xfb	,1	,-1	,-1	,-1);
	add_inst(inst_cld		,0xfc	,1	,-1	,-1	,-1);
	add_inst(inst_std		,0xfd	,1	,-1	,-1	,-1);
	add_inst(inst_inc		,0xfe	,1	,0	,Eb	,-1);
	add_inst(inst_dec		,0xfe	,1	,1	,Eb	,-1);
	add_inst(inst_inc,	0xff,	1,	0,	Ew,	-1, OptSmallParam);
	add_inst(inst_inc,	0xff,	1,	0,	Ed,	-1, OptMediumParam);
	add_inst(inst_inc,	0xff,	1,	0,	Eq,	-1, OptBigParam);
	add_inst(inst_dec,	0xff,	1,	1,	Ew,	-1, OptSmallParam);
	add_inst(inst_dec,	0xff,	1,	1,	Ed,	-1, OptMediumParam);
	add_inst(inst_dec,	0xff,	1,	1,	Eq,	-1, OptBigParam);
	add_inst(inst_call,	0xff,	1,	2,	Ew,	-1, OptSmallParam);
	add_inst(inst_call,	0xff,	1,	2,	Ed,	-1, OptMediumParam);
	add_inst(inst_call,	0xff,	1,	2,	Eq,	-1, OptBigParam);
	add_inst(inst_call_far,	0xff,	1,	3,	Ew,	-1, OptSmallParam); // Ep instead of Ev...
	add_inst(inst_call_far,	0xff,	1,	3,	Ed,	-1, OptMediumParam);
	add_inst(inst_call_far,	0xff,	1,	3,	Eq,	-1, OptBigParam);
	add_inst(inst_jmp, 0xff, 1,	4, Ew, -1, OptSmallParam);
	add_inst(inst_jmp, 0xff, 1,	4, Ed, -1, OptMediumParam);
	add_inst(inst_jmp, 0xff, 1,	4, Eq, -1, OptBigParam);
	add_inst(inst_jmp_far, 0xff, 1, 5, Ed, -1, OptSmallParam);
	add_inst(inst_jmp_far, 0xff, 1, 5, E48, -1, OptMediumParam);
	add_inst(inst_push, 0xff, 1, 6, Ew, -1, OptSmallParam);
	add_inst(inst_push, 0xff, 1, 6, Ed, -1, OptMediumParam);
	add_inst(inst_push, 0xff, 1, 6, Eq, -1, OptBigParam);

	// sse
	add_inst(inst_movss,	0x100ff3,	3,	-1,	Xx, Ed);
	add_inst(inst_movss,	0x110ff3,	3,	-1,	Ed, Xx);
	add_inst(inst_movsd,	0x100ff2,	3,	-1,	Xx, Eq);
	add_inst(inst_movsd,	0x110ff2,	3,	-1,	Eq, Xx);
}

// convert an asm parameter into a human readable expression
string InstructionParam::str(bool hide_size)
{
	msg_db_f("AddParam", 1+ASM_DB_LEVEL);
	//msg_write("----");
	//msg_write(p.type);
	if (type == ParamTInvalid){
		return "-\?\?\?-";
	}else if (type == ParamTNone){
		return "";
	}else if (type == ParamTRegister){
			//msg_write((long)reg);
			//msg_write((long)disp);
		if (deref){
			//msg_write("deref");
			string ss;
			if (!hide_size)
				ss = get_size_name(size) + " ";
			if (disp == DispModeNone)
				return ss + "[" + reg->name + "]";
			else if (disp == DispMode8)
				return ss + format("[%s + 0x%02x]", reg->name.c_str(), value);
			else if (disp == DispMode16)
				return ss + format("[%s + 0x%04x]", reg->name.c_str(), value);
			else if (disp == DispMode32)
				return ss + format("[%s + 0x%08x]", reg->name.c_str(), value);
			else if (disp == DispModeSIB)
				return "SIB[...][...]";
			else if (disp == DispMode8SIB)
				return ss + format("[SIB... + 0x%02x]", value);
			else if (disp == DispMode8Reg2)
				return ss + format("[%s + %s + 0x%02x]", reg->name.c_str(), reg2->name.c_str(), value);
			else if (disp == DispModeReg2)
				return ss + "[" + reg->name + " + " + reg2->name + "]";
		}else
			return reg->name;
	}else if (type == ParamTImmediate){
		//msg_write("im");
		if (deref)
			return format("[%s]", d2h(&value, state.AddrSize).c_str());
		return d2h(&value, size);
	/*}else if (type == ParamTImmediateExt){
		//msg_write("im");
		return format("%s:%s", d2h(&((char*)&value)[4], 2).c_str(), d2h(&value, state.ParamSize).c_str());*/
	}
#if 0
	for (int i=0;i<Registers.num;i++)
		if (param==Registers[i].reg){
			strcat(str,Registers[i].name);
			return;
		}
	switch(param){
		case peAX:	strcat(str,"[eax]");	break;
		case peCX:	strcat(str,"[ecx]");	break;
		case peDX:	strcat(str,"[edx]");	break;
		case peBX:	strcat(str,"[ebx]");	break;
		case pp:	strcat(str,"[--][--]");	break;
		case disp8:	strcat(str,string("[",d2h((char*)&disp,1),"]"));	break;
		case disp16:	strcat(str,string("[",d2h((char*)&disp,2),"]"));	break;
		case disp32:	strcat(str,string("[",d2h((char*)&disp,4),"]"));	break;
		case peSI:	strcat(str,"[esi]");	break;
		case peDI:	strcat(str,"[edi]");	break;
		case pBX_pSI:	strcat(str,"[bx + si]");		break;
		case pBX_pDI:	strcat(str,"[bx + di]");		break;
		case pBP_pSI:	strcat(str,"[bp + si]");		break;
		case pBP_pDI:	strcat(str,"[bp + di]");		break;
		case pBX:	strcat(str,"[bx]");		break;
		case pSI:	strcat(str,"[si]");		break;
		case pDI:	strcat(str,"[di]");		break;

		case d8_peAX:	strcat(str,string("[eax + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peCX:	strcat(str,string("[ecx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peDX:	strcat(str,string("[edx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peBX:	strcat(str,string("[ebx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pp:	strcat(str,string("[--][-- + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peBP:	strcat(str,string("[ebp + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peSI:	strcat(str,string("[esi + ",d2h((char*)&disp,1),"]"));	break;
		case d8_peDI:	strcat(str,string("[edi + ",d2h((char*)&disp,1),"]"));	break;

		case d8_pBX:	strcat(str,string("[bx + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pBP:	strcat(str,string("[bp + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pSI:	strcat(str,string("[si + ",d2h((char*)&disp,1),"]"));	break;
		case d8_pDI:	strcat(str,string("[di + ",d2h((char*)&disp,1),"]"));	break;
	
		case d16_pBX_pSI:	strcat(str,string("[bx + si + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBX_pDI:	strcat(str,string("[bx + di + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBP_pSI:	strcat(str,string("[bp + si + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBP_pDI:	strcat(str,string("[bp + di + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBX:	strcat(str,string("[bx + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pBP:	strcat(str,string("[bp + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pSI:	strcat(str,string("[si + ",d2h((char*)&disp,2),"]"));	break;
		case d16_pDI:	strcat(str,string("[di + ",d2h((char*)&disp,2),"]"));	break;

		case d32_peAX:	strcat(str,string("[eax + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peCX:	strcat(str,string("[ecx + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peDX:	strcat(str,string("[edx + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peBX:	strcat(str,string("[ebx + ",d2h((char*)&disp,4),"]"));	break;
		case d32_pp:	strcat(str,string("[--][-- + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peBP:	strcat(str,string("[ebp + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peSI:	strcat(str,string("[esi + ",d2h((char*)&disp,4),"]"));	break;
		case d32_peDI:	strcat(str,string("[edi + ",d2h((char*)&disp,4),"]"));	break;

		case Ib:	strcat(str,d2h((char*)&disp,1));	break;
		case Iw:	strcat(str,d2h((char*)&disp,2));	break;
		case Id:	strcat(str,d2h((char*)&disp,4));	break;
		case Ip:
			if ((small_param)&&(!mode16))	strcat(str,"word ");
			if ((!small_param)&&(mode16))	strcat(str,"dword ");
			strcat(str,d2h((char*)&ParamConstantDouble,2));
			strcat(str,":");
			if (small_param)	strcat(str,d2h((char*)&disp,2));
			else				strcat(str,d2h((char*)&disp,4));
			break;

		default:	strcat(str,string(i2s(param),": -\?\?- "));	break;
	};
#endif
	return "\?\?\?";
}

inline void UnfuzzyParam(InstructionParam &p, InstructionParamFuzzy &pf)
{
	msg_db_f("UnfuzzyParam", 2+ASM_DB_LEVEL);
	p.type = pf._type_;
	p.reg2 = NULL;
	p.disp = DispModeNone;
	p.reg = pf.reg;
	if ((p.reg) && (state.ExtendModRMBase)){
		if ((p.reg->id >= RegRax) && (p.reg->id <= RegRbp))
			p.reg = RegisterByID[p.reg->id + RegR8 - RegRax];
	}
	p.size = pf.size;
	p.deref = false; // well... FIXME
	p.value = 0;
	p.is_label = false;
	if (pf._type_ == ParamTMemory){
		p.type = ParamTImmediate;
		p.deref = true;
	}
}

int GetModRMRegister(int reg, int size)
{
	if (size == Size8){
		if (reg == 0x00)	return RegAl;
		if (reg == 0x01)	return RegCl;
		if (reg == 0x02)	return RegDl;
		if (reg == 0x03)	return RegBl;
		if (reg == 0x04)	return RegAh;
		if (reg == 0x05)	return RegCh;
		if (reg == 0x06)	return RegDh;
		if (reg == 0x07)	return RegBh;
	}else if (size == Size16){
		if (reg == 0x00)	return RegAx;
		if (reg == 0x01)	return RegCx;
		if (reg == 0x02)	return RegDx;
		if (reg == 0x03)	return RegBx;
		if (reg == 0x04)	return RegSp;
		if (reg == 0x05)	return RegBp;
		if (reg == 0x06)	return RegSi;
		if (reg == 0x07)	return RegDi;
	}else if (size == Size32){
		if (reg == 0x00)	return RegEax;
		if (reg == 0x01)	return RegEcx;
		if (reg == 0x02)	return RegEdx;
		if (reg == 0x03)	return RegEbx;
		if (reg == 0x04)	return RegEsp;
		if (reg == 0x05)	return RegEbp;
		if (reg == 0x06)	return RegEsi;
		if (reg == 0x07)	return RegEdi;
		if (reg == 0x08)	return RegR8d;
		if (reg == 0x09)	return RegR9d;
		if (reg == 0x0a)	return RegR10d;
		if (reg == 0x0b)	return RegR11d;
		if (reg == 0x0c)	return RegR12d;
		if (reg == 0x0d)	return RegR13d;
		if (reg == 0x0e)	return RegR14d;
		if (reg == 0x0f)	return RegR15d;
	}else if (size == Size64){
		if (reg == 0x00)	return RegRax;
		if (reg == 0x01)	return RegRcx;
		if (reg == 0x02)	return RegRdx;
		if (reg == 0x03)	return RegRbx;
		if (reg == 0x04)	return RegRsp;
		if (reg == 0x05)	return RegRbp;
		if (reg == 0x06)	return RegRsi;
		if (reg == 0x07)	return RegRdi;
		if (reg == 0x08)	return RegR8;
		if (reg == 0x09)	return RegR9;
		if (reg == 0x0a)	return RegR10;
		if (reg == 0x0b)	return RegR11;
		if (reg == 0x0c)	return RegR12;
		if (reg == 0x0d)	return RegR13;
		if (reg == 0x0e)	return RegR14;
		if (reg == 0x0f)	return RegR15;
	}
	msg_error("unhandled mod/rm register: " + i2s(reg) + " (size " + i2s(size) + ")");
	return 0;
}

inline void GetFromModRM(InstructionParam &p, InstructionParamFuzzy &pf, unsigned char modrm)
{
	msg_db_f("GetFromModRM", 2+ASM_DB_LEVEL);
	if (pf.mrm_mode == MRMReg){
		unsigned char reg = modrm & 0x38; // bits 5, 4, 3
		p.type = ParamTRegister;
		p.deref = false;
		if (pf.reg_group == RegGroupSegment){
			if (reg == 0x00)	p.reg = RegisterByID[RegEs];
			if (reg == 0x08)	p.reg = RegisterByID[RegCs];
			if (reg == 0x10)	p.reg = RegisterByID[RegSs];
			if (reg == 0x18)	p.reg = RegisterByID[RegDs];
			if (reg == 0x20)	p.reg = RegisterByID[RegFs];
			if (reg == 0x28)	p.reg = RegisterByID[RegGs];
		}else if (pf.reg_group == RegGroupControl){
			if (reg == 0x00)	p.reg = RegisterByID[RegCr0];
			if (reg == 0x08)	p.reg = RegisterByID[RegCr1];
			if (reg == 0x10)	p.reg = RegisterByID[RegCr2];
			if (reg == 0x18)	p.reg = RegisterByID[RegCr3];
		}else if (pf.reg_group == RegGroupXmm){
			p.reg = RegisterByID[RegXmm0 + (reg >> 3)];
		}else{
			reg = (reg >> 3) | (state.ExtendModRMReg ? 0x08 : 0x00);
			p.reg = RegisterByID[GetModRMRegister(reg, p.size)];
		}
	}else if (pf.mrm_mode == MRMModRM){
		unsigned char mod = modrm & 0xc0; // bits 7, 6
		unsigned char rm = modrm & 0x07; // bits 2, 1, 0
		if (state.ExtendModRMBase)	rm |= 0x08;
		if (mod == 0x00){
			if (state.AddrSize == Size16){
				p.type = ParamTRegister;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegSi];	p.disp = DispModeReg2;	}
				if (rm == 0x01){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegDi];	p.disp = DispModeReg2;	}
				if (rm == 0x02){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegSi];	p.disp = DispModeReg2;	}
				if (rm == 0x03){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegDi];	p.disp = DispModeReg2;	}
				if (rm == 0x04)	p.reg = RegisterByID[RegSi];
				if (rm == 0x05)	p.reg = RegisterByID[RegDi];
				if (rm == 0x06){p.reg = NULL;	p.type = ParamTImmediate;	}
				if (rm == 0x07)	p.reg = RegisterByID[RegBx];
			}else{
				p.type = ParamTRegister;
				p.deref = true;
				//if (rm == 0x04){p.reg = NULL;	p.disp = DispModeSIB;	p.type = ParamTImmediate;}//p.type = ParamTInvalid;	Error("kein SIB byte...");}
				if (rm == 0x04){p.reg = RegisterByID[RegEax];	p.disp = DispModeSIB;	} // eax = provisoric
				else if (rm == 0x05){p.reg = NULL;	p.type = ParamTImmediate;	}
				else
					p.reg = RegisterByID[GetModRMRegister(rm, Size32)];
			}
		}else if ((mod == 0x40) || (mod == 0x80)){
			if (state.AddrSize == Size16){
				p.type = ParamTRegister;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegSi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x01){p.reg = RegisterByID[RegBx];	p.reg2 = RegisterByID[RegDi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x02){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegSi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x03){p.reg = RegisterByID[RegBp];	p.reg2 = RegisterByID[RegDi];	p.disp = (mod == 0x40) ? DispMode8Reg2 : DispMode16Reg2;	}
				if (rm == 0x04){p.reg = RegisterByID[RegSi];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
				if (rm == 0x05){p.reg = RegisterByID[RegDi];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
				if (rm == 0x06){p.reg = RegisterByID[RegBp];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
				if (rm == 0x07){p.reg = RegisterByID[RegBx];	p.disp = (mod == 0x40) ? DispMode8 : DispMode16;	}
			}else{
				p.type = ParamTRegister;
				p.deref = true;
				p.disp = (mod == 0x40) ? DispMode8 : DispMode32;
				//if (rm == 0x04){p.reg = NULL;	p.type = ParamTInvalid;	}
				if (rm == 0x04){p.reg = RegisterByID[RegEax];	p.disp = DispMode8SIB;	} // eax = provisoric
				else
					p.reg = RegisterByID[GetModRMRegister(rm, Size32)];
			}
		}else if (mod == 0xc0){
			p.type = ParamTRegister;
			p.deref = false;
			if (state.ExtendModRMBase)	rm |= 0x08;
			p.reg = RegisterByID[GetModRMRegister(rm, p.size)];
		}
	}
}

inline void TryGetSIB(InstructionParam &p, char *&cur)
{
	if ((p.disp == DispModeSIB) || (p.disp == DispMode8SIB)){
		bool disp8 = (p.disp == DispMode8SIB);
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
				p.disp = disp8 ? DispMode8Reg2 : DispModeReg2;
				if (base == 0x00)		p.reg = RegisterByID[RegEax];
				else if (base == 0x01)	p.reg = RegisterByID[RegEcx];
				else if (base == 0x02)	p.reg = RegisterByID[RegEdx];
				else if (base == 0x03)	p.reg = RegisterByID[RegEbx];
				else if (base == 0x04)	p.reg = RegisterByID[RegEsp];
				else p.disp = DispModeSIB; // ...
				if (index == 0x00)		p.reg2 = RegisterByID[RegEax];
				else if (index == 0x08)	p.reg2 = RegisterByID[RegEcx];
				else if (index == 0x10)	p.reg2 = RegisterByID[RegEdx];
				else if (index == 0x18)	p.reg2 = RegisterByID[RegEbx];
				else if (index == 0x28)	p.reg2 = RegisterByID[RegEbp];
				else if (index == 0x30)	p.reg2 = RegisterByID[RegEsi];
				else if (index == 0x38)	p.reg2 = RegisterByID[RegEdi];
				else p.disp = disp8 ? DispMode8 : DispModeNone;
			}
		//}
	}
}

inline void ReadParamData(char *&cur, InstructionParam &p, bool has_modrm)
{
	msg_db_f("ReadParamData", 2+ASM_DB_LEVEL);
	//char *o = cur;
	p.value = 0;
	if (p.type == ParamTImmediate){
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
	}else if (p.type == ParamTRegister){
		if ((p.disp == DispMode8) || (p.disp == DispMode8Reg2) || (p.disp == DispMode8SIB)){
			*(char*)&p.value = *cur;		cur ++;
		}else if (p.disp == DispMode16){
			*(short*)&p.value = *(short*)cur;		cur += 2;
		}else if (p.disp == DispMode32){
			*(int*)&p.value = *(int*)cur;		cur += 4;
		}
	}
	//msg_write((long)cur - (long)o);
}

// convert some opcode into (human readable) assembler language
string Disassemble(void *_code_,int length,bool allow_comments)
{
	msg_db_f("Disassemble", 1+ASM_DB_LEVEL);

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
	state.DefaultSize = Size32;


	while(code < end){
		state.reset();
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
				if ((long)code - (long)orig == CurrentMetaInfo->label[i].Pos)
					bufstr += "    " + CurrentMetaInfo->label[i].Name + ":\n";
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
					state.DefaultSize = (CurrentMetaInfo->bit_change[i].bits == 16) ? Size16 : Size32;
					state.reset();
					if (state.DefaultSize == Size16)
						bufstr += "   bits_16\n";
					else
						bufstr += "   bits_32\n";
				}
		}

		// code

		// prefix (size/segment register)
		Register *seg = NULL;
		if (cur[0]==0x67){
			state.AddrSize = (state.DefaultSize == Size32) ? Size16 : Size32;
			cur++;
		}
		if (cur[0]==0x66){
			state.ParamSize = (state.DefaultSize == Size32) ? Size16 : Size32;
			cur++;
		}
		if (InstructionSet.set == InstructionSetAMD64){
			if ((cur[0] & 0xf0) == 0x40){
				if ((cur[0] & 0x08) > 0)
					state.ParamSize = Size64;
				state.ExtendModRMReg = ((cur[0] & 0x04) > 0);
				state.ExtendModRMIndex = ((cur[0] & 0x02) > 0);
				state.ExtendModRMBase = ((cur[0] & 0x01) > 0);
				cur++;
			}
		}
		if (cur[0]==0x2e){	seg = RegisterByID[RegCs];	cur++;	}
		else if (cur[0]==0x36){	seg = RegisterByID[RegSs];	cur++;	}
		else if (cur[0]==0x3e){	seg = RegisterByID[RegDs];	cur++;	}
		else if (cur[0]==0x26){	seg = RegisterByID[RegEs];	cur++;	}
		else if (cur[0]==0x64){	seg = RegisterByID[RegFs];	cur++;	}
		else if (cur[0]==0x65){	seg = RegisterByID[RegGs];	cur++;	}
		opcode=cur;

		// instruction
		CPUInstruction *inst = NULL;
		foreach(CPUInstruction &ci, CPUInstructions){
			if (ci.code_size == 0)
				continue;
			if (!ci.has_fixed_param){
				if (ci.has_small_param != (state.ParamSize == Size16))
					continue;
				if (ci.has_big_param != (state.ParamSize == Size64))
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
			if ((ok) && (ci.has_modrm)){
				InstructionParam p1, p2;
				UnfuzzyParam(p1, ci.param1);
				UnfuzzyParam(p2, ci.param2);

				// modr/m byte
				char modrm = cur[ci.code_size];
				GetFromModRM(p1, ci.param1, modrm);
				GetFromModRM(p2, ci.param2, modrm);
				if ((p1.type == ParamTRegister) && (!p1.deref) && (!ci.param1.allow_register))
					continue;
				if ((p2.type == ParamTRegister) && (!p2.deref) && (!ci.param2.allow_register))
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
			if ((state.ParamSize != state.DefaultSize) && ((p1.type != ParamTRegister) || (p1.deref)) && ((p2.type != ParamTRegister) || p2.deref)){
				if (state.ParamSize == Size16)
					str += " word";
				else if (state.ParamSize == Size32)
					str += " dword";
				else if (state.ParamSize == Size64)
					str += " qword";
			}
			bool hide_size = p2.type != ParamTNone;
			if (p1.type != ParamTNone)
				str += " " + p1.str(hide_size);
			if (p2.type != ParamTNone)
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

#if 0
			
			bool e_first=false;
			char modRM;
			int mod,Reg,rm;
			if ((p1>=0)||(p2>=0)){
				if ((p1==Ed)||(p1==Ew)||(p1==Eb)||(p1==Sw)||(p2==Ed)||(p2==Ew)||(p2==Eb)||(p2==Sw)){
					e_first=((p1==Ed)||(p1==Ew)||(p1==Eb)) && (p1!=Sw);
					if (!e_first){	int t=p1;		p1=p2;			p2=t;	}
					if (!e_first){	int t=pp1;		pp1=pp2;		pp2=t;	}
					if (!e_first){	int t=disp1;	disp1=disp2;	disp2=t;	}
					cur++;
					modRM=cur[0];
					mod=(unsigned char)modRM/64;
					Reg=((unsigned char)modRM/8)%8;
					rm=(unsigned char)modRM%8;
					if (mod==0){
						if (mode16){
							if (rm==0)	pp1=pBX_pSI;
							if (rm==1)	pp1=pBX_pDI;
							if (rm==2)	pp1=pBP_pSI;
							if (rm==3)	pp1=pBP_pDI;
							if (rm==4)	pp1=pSI;
							if (rm==5)	pp1=pDI;
							if (rm==6){	pp1=disp16;	disp1=*(short*)&cur[1];	cur+=2;	}
							if (rm==7)	pp1=pBX;
						}else{
							if (rm==0)	pp1=peAX;
							if (rm==1)	pp1=peCX;
							if (rm==2)	pp1=peDX;
							if (rm==3)	pp1=peBX;
							if (rm==4){	pp1=pp;		disp1=cur[1];		cur++;	}
							if (rm==5){	pp1=disp32;	disp1=*(int*)&cur[1];	cur+=4; }
							if (rm==6)	pp1=peSI;
							if (rm==7)	pp1=peDI;
						}
					}else if (mod==1){
						if (mode16){
							if (rm==0)	pp1=d8_pBX_pSI;
							if (rm==1)	pp1=d8_pBX_pDI;
							if (rm==2)	pp1=d8_pBP_pSI;
							if (rm==3)	pp1=d8_pBP_pDI;
							if (rm==4)	pp1=d8_pSI;
							if (rm==5)	pp1=d8_pDI;
							if (rm==6)	pp1=d8_pBP;
							if (rm==7)	pp1=d8_pBX;
							disp1=cur[1];	cur++;
						}else{
							if (rm==0)	pp1=d8_peAX;
							if (rm==1)	pp1=d8_peCX;
							if (rm==2)	pp1=d8_peDX;
							if (rm==3)	pp1=d8_peBX;
							if (rm==4){	pp1=d8_pp;		disp1=cur[1];		cur++;	}
							if (rm==5)	pp1=d8_peBP;
							if (rm==6)	pp1=d8_peSI;
							if (rm==7)	pp1=d8_peDI;
							disp1=cur[1];	cur++;
						}
					}else if (mod==2){
						if (mode16){
							if (rm==0)	pp1=d16_pBX_pSI;
							if (rm==1)	pp1=d16_pBX_pDI;
							if (rm==2)	pp1=d16_pBP_pSI;
							if (rm==3)	pp1=d16_pBP_pDI;
							if (rm==4)	pp1=d16_pSI;
							if (rm==5)	pp1=d16_pDI;
							if (rm==6)	pp1=d16_pBP;
							if (rm==7)	pp1=d16_pBX;
							disp1=*(short*)&cur[1];	cur+=2;
						}else{
							if (rm==0)	pp1=d32_peAX;
							if (rm==1)	pp1=d32_peCX;
							if (rm==2)	pp1=d32_peDX;
							if (rm==3)	pp1=d32_peBX;
							if (rm==4){	pp1=d32_pp;		disp1=cur[1];		cur++;	}
							if (rm==5)	pp1=d32_peBP;
							if (rm==6)	pp1=d32_peSI;
							if (rm==7)	pp1=d32_peDI;
							disp1=*(int*)&cur[1];	cur+=4;
						}
					}else if (mod==3){
						if (p1==Eb){
							if (rm==0)	pp1=AL;
							if (rm==1)	pp1=CL;
							if (rm==2)	pp1=DL;
							if (rm==3)	pp1=BL;
							if (rm==4)	pp1=AH;
							if (rm==5)	pp1=CH;
							if (rm==6)	pp1=DH;
							if (rm==7)	pp1=BH;
						}else if (p1==Ew){
							if (rm==0)	pp1=AX;
							if (rm==1)	pp1=CX;
							if (rm==2)	pp1=DX;
							if (rm==3)	pp1=BX;
							if (rm==4)	pp1=SP;
							if (rm==5)	pp1=BP;
							if (rm==6)	pp1=SI;
							if (rm==7)	pp1=DI;
						}else if (p1==Ed){
							if (rm==0)	pp1=eAX;
							if (rm==1)	pp1=eCX;
							if (rm==2)	pp1=eDX;
							if (rm==3)	pp1=eBX;
							if (rm==4)	pp1=eSP;
							if (rm==5)	pp1=eBP;
							if (rm==6)	pp1=eSI;
							if (rm==7)	pp1=eDI;
						}
					}
					if (p2==Gb){
						if (Reg==0)	pp2=AL;
						if (Reg==1)	pp2=CL;
						if (Reg==2)	pp2=DL;
						if (Reg==3)	pp2=BL;
						if (Reg==4)	pp2=AH;
						if (Reg==5)	pp2=CH;
						if (Reg==6)	pp2=DH;
						if (Reg==7)	pp2=BH;
					}else if (p2==Gw){
						if (Reg==0)	pp2=AX;
						if (Reg==1)	pp2=CX;
						if (Reg==2)	pp2=DX;
						if (Reg==3)	pp2=BX;
						if (Reg==4)	pp2=SP;
						if (Reg==5)	pp2=BP;
						if (Reg==6)	pp2=SI;
						if (Reg==7)	pp2=DI;
					}else if (p2==Gd){
						if (Reg==0)	pp2=eAX;
						if (Reg==1)	pp2=eCX;
						if (Reg==2)	pp2=eDX;
						if (Reg==3)	pp2=eBX;
						if (Reg==4)	pp2=eSP;
						if (Reg==5)	pp2=eBP;
						if (Reg==6)	pp2=eSI;
						if (Reg==7)	pp2=eDI;
					}else if (p2==Sw){
						if (Reg==0)	pp2=ES;
						if (Reg==1)	pp2=CS;
						if (Reg==2)	pp2=SS;
						if (Reg==3)	pp2=DS;
						if (Reg==4)	pp2=FS;
						if (Reg==5)	pp2=GS;
					}else if (p2==Cd){
						if (Reg==0)	pp2=CR0;
						if (Reg==1)	pp2=CR1;
						if (Reg==2)	pp2=CR2;
						if (Reg==3)	pp2=CR3;
					}else if ((p2==Ob)||(p2==Ow)||(p2==Od)){
						if (small_param){
							pp2=disp16;
							disp2=*(short*)&cur[1];	cur+=2;
						}else{
							pp2=disp32;
							disp2=*(int*)&cur[1];	cur+=4;
						}
					}

					if (!e_first){	int t=pp1;	pp1=pp2;	pp2=t;	}
					if (!e_first){	int t=p1;	p1=p2;		p2=t;	}
					if (!e_first){	int t=disp1;	disp1=disp2;	disp2=t;	}
				}else if ((p1==Ob)||(p1==Ow)||(p1==Od)){
					if (small_param){
						pp1=disp16;
						disp1=*(short*)&cur[1];	cur+=2;
					}else{
						pp1=disp32;
						disp1=*(int*)&cur[1];	cur+=4;
					}
				}else if (p1==Ip){
					if (small_param){	disp1=*(short*)&cur[1];	cur+=2;	}
					else{				disp1=*(int*)&cur[1];	cur+=4;	}
					ParamConstantDouble=*(short*)&cur[1];	cur+=2;
				}else if (p1==Id){
					disp1=*(int*)&cur[1];	cur+=4;
				}else if (p1==Iw){
					disp1=*(short*)&cur[1];	cur+=2;
				}else if (p1==Ib){
					disp1=cur[1];	cur++;
				}
				// Param2
				if ((p2==Ob)||(p2==Ow)||(p2==Od)){
					if (small_param){
						pp2=disp16;
						disp2=*(short*)&cur[1];	cur+=2;
					}else{
						pp2=disp32;
						disp2=*(int*)&cur[1];	cur+=4;
					}
				}else if (p2==Id){
					disp2=*(int*)&cur[1];	cur+=4;
				}else if (p2==Iw){
					disp2=*(short*)&cur[1];	cur+=2;
				}else if (p2==Ib){
					disp2=cur[1];	cur++;
				}
				//for (int i=0;i<32-l;i++)
				//	strcat(str," ");
				strcat(param," ");
				AddParam(param,pp1,disp1);
				if (p2>=0){
					strcat(param,", ");
					AddParam(param,pp2,disp2);
				}
			}
			char str[128];		strcpy(str,"");
			if (seg==CS)	strcat(str,"CS: ");
			if (seg==SS)	strcat(str,"SS: ");
			if (seg==DS)	strcat(str,"DS: ");
			if (seg==ES)	strcat(str,"ES: ");
			if (seg==FS)	strcat(str,"FS: ");
			if (seg==GS)	strcat(str,"GS: ");
			strcat(str,string(CPUInstructions[ae].name,param));
			strcat(buffer,str);
			if (allow_comments){
				int l=strlen(str);
				strcat(buffer," ");
				for (int ii=0;ii<48-l;ii++)
					strcat(buffer," ");
				strcat(buffer,"// ");
				strcat(buffer,d2h(code,1+long(cur)-long(code),false));
			}
#endif
		}else{
			//msg_write(string2("????? -                          unknown         // %s\n",d2h(code,1+long(cur)-long(code),false)));
			bufstr += format("????? -                          unknown         // %s\n",d2h(code,1+long(cur)-long(code),false).c_str());
			cur ++;
		}

		// done?
		if ((length < 0) && (((unsigned char)opcode[0] == 0xc3) || ((unsigned char)opcode[0] == 0xc2)))
			break;
	}
	return bufstr;
}

// skip unimportant code (whitespace/comments)
//    returns true if end of code
bool IgnoreUnimportant(int &pos)
{
	msg_db_f("IgnoreUnimportant", 4+ASM_DB_LEVEL);
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
		if ((code_buffer[pos] == '\n') || (code_buffer[pos] == ' ') || (code_buffer[pos] == '\t')){
			pos ++;
			state.ColumnNo ++;
			continue;
		}
		// comments
		if ((code_buffer[pos] == ';') || ((code_buffer[pos] == '/') && (code_buffer[pos] == '/'))){
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
	msg_db_f("GetMne", 1+ASM_DB_LEVEL);
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
		if ((mne[i] == '\'') || (mne[i] == '\"'))
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
			if ((code_buffer[pos] == ' ') || (code_buffer[pos] == '\t') || (code_buffer[pos] == ',')){
				mne[i] = 0;
				// end of line?
				for (int j=0;j<128;j++){
					if ((code_buffer[pos+j] != ' ') && (code_buffer[pos+j] != '\t') && (code_buffer[pos+j] != ',')){
						if ((code_buffer[pos + j] == 0) || (code_buffer[pos + j] == '\n'))
							state.EndOfLine = true;
						// comment ending the line
						if ((code_buffer[pos + j] == ';') || ((code_buffer[pos + j] == '/') && (code_buffer[pos + j + 1] == '/')))
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
	msg_db_f("GetParam", 1+ASM_DB_LEVEL);
	p.type = ParamTInvalid;
	p.reg = NULL;
	p.deref = false;
	p.size = SizeUnknown;
	p.disp = DispModeNone;
	p.is_label = false;
	//msg_write(param);

	// none
	if (param.num == 0){
		p.type = ParamTNone;

	// deref
	}else if ((param[0] == '[') && (param[param.num-1] == ']')){
		if (DebugAsm)
			printf("deref:   ");
		so("Deref:");
		//bool u16 = use_mode16;
		GetParam(p, param.substr(1, -2), list, pn);
		p.size = SizeUnknown;
		p.deref = true;
		//use_mode16 = u16;

	// string
	}else if ((param[0] == '\"') && (param[param.num-1] == '\"')){
		if (DebugAsm)
			printf("String:   ");
		char *ps = new char[param.num - 1];
		strcpy(ps, param.substr(1, -2).c_str());
		p.value = (long)ps;
		p.type = ParamTImmediate;

	// complex...
	}else if (param.find("+") >= 0){
		if (DebugAsm)
			printf("complex:   ");
		InstructionParam sub;
		
		// first part (must be a register)
		string part;
		for (int i=0;i<param.num;i++)
			if ((param[i] == ' ') || (param[i] == '+'))
				break;
			else
				part.add(param[i]);
		int offset = part.num;
		GetParam(sub, part, list, pn);
		if (sub.type == ParamTRegister){
			//msg_write("reg");
			p.type = ParamTRegister;
			p.size = Size32;
			p.reg = sub.reg;
		}else
			p.type = ParamTInvalid;

		// second part (...up till now only hex)
		for (int i=offset;i<param.num;i++)
			if ((param[i] != ' ') && (param[i] != '+')){
				offset = i;
				break;
			}
		part = param.substr(offset, -1);
		GetParam(sub, part, list, pn);
		if (sub.type == ParamTImmediate){
			//msg_write("c2 = im");
			if (((long)sub.value & 0xffffff00) == 0)
				p.disp = DispMode8;
			else
				p.disp = DispMode32;
			p.value = sub.value;
		}else
			p.type = ParamTInvalid;

		

	// hex const
	}else if ((param[0] == '0') && (param[1] == 'x')){
		p.type = ParamTImmediate;
		long long v = 0;
		for (int i=2;i<param.num;i++){
			if (param[i] == '.'){
			}else if ((param[i] >= 'a') && (param[i] <= 'f')){
				v *= 16;
				v += param[i] - 'a' + 10;
			}else if ((param[i] >= 'A') && (param[i] <= 'F')){
				v *= 16;
				v += param[i]-'A'+10;
			}else if ((param[i]>='0')&&(param[i]<='9')){
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
				p.type = ParamTInvalid;
				return;
			}
			p.value = (long)v;
			p.size = Size8;
			if (param.num > 4)
				p.size = Size16;
			if (param.num > 6)
				p.size = Size32;
			if (param.num > 10)
				p.size = Size48;
			if (param.num > 14)
				p.size = Size64;
		}
		if (DebugAsm){
			printf("hex const:  %s\n",d2h((char*)&p.value,p.size).c_str());
		}

	// char const
	}else if ((param[0] == '\'') && (param[param.num - 1] == '\'')){
		p.value = (long)param[1];
		p.type = ParamTImmediate;
		p.size = Size8;
		if (DebugAsm)
			printf("hex const:  %s\n",d2h((char*)&p.value,1).c_str());

	// label substitude
	}else if (param == "$"){
		p.value = list.add_label(param, true);
		p.type = ParamTImmediate;
		p.size = Size32;
		p.is_label = true;
		so("label:  " + param + "\n");
		
	}else{
		// register
		for (int i=0;i<Registers.num;i++)
			if (Registers[i].name == param){
				p.type = ParamTRegister;
				p.reg = &Registers[i];
				p.size = Registers[i].size;
				so("Register:  " + Registers[i].name + "\n");
				return;
			}
		// existing label
		for (int i=0;i<list.label.num;i++)
			if (list.label[i].Name == param){
				p.value = i;
				p.type = ParamTImmediate;
				p.size = Size32;
				p.is_label = true;
				so("label:  " + param + "\n");
				return;
			}
		// script variable (global)
		for (int i=0;i<CurrentMetaInfo->global_var.num;i++){
			if (CurrentMetaInfo->global_var[i].Name == param){
				p.value = (long)CurrentMetaInfo->global_var[i].Pos;
				p.type = ParamTImmediate;
				p.size = CurrentMetaInfo->global_var[i].Size;
				p.deref = true;
				so("global variable:  \"" + param + "\"\n");
				return;
			}
		}
		// not yet existing label...
		if (param[0]=='_'){
			so("label as param:  \"" + param + "\"\n");
			p.value = list.add_label(param, false);
			p.type = ParamTImmediate;
			p.is_label = true;
			p.size = Size32;
			return;
		}
	}
	if (p.type == ParamTInvalid)
		SetError("unknown parameter:  \"" + param + "\"\n");
}

inline void insert_val(char *oc, int &ocs, long long val, int size)
{
	if (size == 1)
		oc[ocs] = (char)val;
	else if (size == 2)
		*(short*)&oc[ocs] = (short)val;
	else if (size == 4)
		*(int*)&oc[ocs] = (int)val;
	else if (size == 8)
		*(long long int*)&oc[ocs] = val;
	else
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
	if (p.type == ParamTImmediate){
		size = p.size;
		if (p.deref){
			//---msg_write("deref....");
			size = state.AddrSize; // inst.has_big_addr
			if (InstructionSet.set == InstructionSetAMD64){
				if (inst.has_modrm)
					value -= (long)oc + ocs + size + next_param_size; // amd64 uses RIP-relative addressing!
				else
					size = Size64; // Ov/Mv...
			}
		}
	//}else if (p.type == ParamTImmediateExt){
	//	size = state.ParamSize;  // bits 0-15  /  0-31
	}else if (p.type == ParamTRegister){
		if (p.disp == DispMode8)	size = Size8;
		if (p.disp == DispMode16)	size = Size16;
		if (p.disp == DispMode32)	size = Size32;
	}else
		return;

	bool rel = ((inst.name[0] == 'j') /*&& (inst.param1._type_ != ParamTImmediateDouble)*/) || (inst.name == "call") || (inst.name.find("loop") >= 0);
	if (inst.inst == inst_jmp_far)
		rel = false;
	if (p.is_label){
		WantedLabel w;
		w.Pos = ocs;
		w.Size = size;
		w.LabelNo = value;
		w.Name = list.label[p.value].Name;
		w.Relative = rel;
		w.InstNo = list.current_inst;
		list.wanted_label.add(w);
		so("add wanted label");
	}else if (rel){
		value -= CurrentMetaInfo->CodeOrigin + ocs + size + next_param_size; // TODO ...first byte of next opcode
	}

	//---msg_write("imm " + i2s(size));
	append_val(oc, ocs, value, size);
}

void InstructionWithParamsList::LinkWantedLabels(void *oc)
{
	foreachib(WantedLabel &w, wanted_label, i){
		Label &l = label[w.LabelNo];
		if (l.Value == -1)
			continue;
		so("linking label");

		int value = l.Value;
		if (w.Relative)
			value -= CurrentMetaInfo->CodeOrigin + w.Pos + w.Size; // TODO first byte after command

		insert_val((char*)oc, w.Pos, value, w.Size);


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
	msg_db_f("AppendFromSource", 1+ASM_DB_LEVEL);

	const char *code = _code.c_str();

	if (!CurrentMetaInfo)
		SetError("no CurrentMetaInfo");

	state.LineNo = CurrentMetaInfo->LineOffset;
	state.ColumnNo = 0;

	// CurrentMetaInfo->CurrentOpcodePos // Anfang aktuelle Zeile im gesammten Opcode
	code_buffer = code; // Asm-Source-Puffer

	int pos = 0;
	InstructionParam p1, p2;
	state.DefaultSize = Size32;
	if (CurrentMetaInfo)
		if (CurrentMetaInfo->Mode16)
			state.DefaultSize = Size16;
	state.EndOfCode = false;
	while(pos < _code.num - 2){

		string cmd, param1, param2;

		//msg_write("..");
		state.reset();


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
			if ((param1 == "dword") || (param1 == "word") || (param1 == "qword")){
				if (param1 == "word")
					state.ParamSize = Size16;
				else if (param1 == "dword")
					state.ParamSize = Size32;
				else if (param1 == "qword")
					state.ParamSize = Size64;
				if (!state.EndOfLine)
					param1 = FindMnemonic(pos);
			}
		}
		if (!state.EndOfLine)
			param2 = FindMnemonic(pos);
		//msg_write(string2("----: %s %s%s %s", cmd, param1, (strlen(param2)>0)?",":"", param2));
		if (state.EndOfCode)
			break;
		so("------------------------------");
		so(cmd);
		so(param1);
		so(param2);
		so("------");

		// parameters
		GetParam(p1, param1, *this, 0);
		GetParam(p2, param2, *this, 1);
		if ((p1.type == ParamTInvalid) || (p2.type == ParamTInvalid))
			return;

	// special stuff
		if (cmd == "bits_16"){
			so("16 bit Modus!");
			state.DefaultSize = Size16;
			state.reset();
			if (CurrentMetaInfo){
				CurrentMetaInfo->Mode16 = true;
				BitChange b;
				b.cmd_pos = num;
				b.bits = 16;
				CurrentMetaInfo->bit_change.add(b);
			}
			continue;
		}else if (cmd == "bits_32"){
			so("32 bit Modus!");
			state.DefaultSize = Size32;
			state.reset();
			if (CurrentMetaInfo){
				CurrentMetaInfo->Mode16 = false;
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
		}/*else if ((cmd == "ds") || (cmd == "dz")){
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
			add_label(cmd, true);

			continue;
		}

		// command
		int inst = -1;
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (InstructionNames[i].name == cmd)
				inst = InstructionNames[i].inst;
		if (inst < 0)
			SetError("unknown instruction:  " + cmd);
		// prefix
		if (state.ParamSize != state.DefaultSize){
			//buffer[CodeLength ++] = 0x66;
			SetError("prefix unhandled:  " + cmd);
		}
		InstructionWithParams iwp;
		iwp.inst = inst;
		iwp.p1 = p1;
		iwp.p2 = p2;
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
	msg_db_f("Assemble", 1+ASM_DB_LEVEL);
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/

	InstructionWithParamsList list = InstructionWithParamsList(CurrentMetaInfo->LineOffset);

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
	if ((inst_p.size == SizeUnknown) || (wanted_p.size == SizeUnknown))
		return true;
/*	if ((inst_p.size == SizeVariable) && ((wanted_p.size == Size16) || (wanted_p.size == Size32)))
		return true;*/
	return false;
}

inline bool _deref_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p)
{
	if (wanted_p.deref)
		return (inst_p.allow_memory_address) || (inst_p.allow_memory_indirect);
	return true;
}

bool InstructionParamFuzzy::match(InstructionParam &wanted_p)
{
	//ParamFuzzyOut(&inst_p);
	
	// none
	if ((wanted_p.type == ParamTNone) || (!used))
		return (wanted_p.type == ParamTNone) && (!used);

	if ((size != SizeUnknown) && (wanted_p.size != SizeUnknown))
		if (size != wanted_p.size)
			return false;

	// immediate
	if (wanted_p.type == ParamTImmediate){
		if ((allow_memory_address) && (wanted_p.deref))
			return true;
		if ((allow_immediate) && (!wanted_p.deref)){
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
	if (wanted_p.type == ParamTRegister){
		// direct match
		if ((allow_register) && (reg)){
			if (wanted_p.reg)
				if ((reg->id >= RegRax) && (reg->id <= RegRbp) && (wanted_p.reg->id == reg->id + RegR8 - RegRax))
					return true;
			return ((reg == wanted_p.reg) && (_deref_match_(*this, wanted_p)));
		}
		// fuzzy match
		/*if (inst_p.allow_register){
			msg_write("r2");
			
			return ((inst_p.reg_group == wanted_p.reg->group) && (_size_match_(inst_p, wanted_p)) && (_deref_match_(inst_p, wanted_p)));
		}*/
		// very fuzzy match
		if ((allow_register) || (allow_memory_indirect)){
			if (wanted_p.deref){
				if (allow_memory_indirect)
					return ((reg_group == wanted_p.reg->group) && (_deref_match_(*this, wanted_p)));
			}else if (allow_register)
				return ((reg_group == wanted_p.reg->group) && (_size_match_(*this, wanted_p))); // FIXME (correct?)
		}
	}

	return false;
}

// translate from easy parameters to assembler usable parameters
InstructionParam _make_param_(int type, int size, long long param)
{
	InstructionParam i;
	i.reg = NULL;
	i.size = SizeUnknown;
	i.deref = false;
	i.value = 0;
	i.disp = DispModeNone;
	i.is_label = false;
	if (type == PKNone){
		i.type = ParamTNone;
	}else if (type == PKConstant){
		i.type = ParamTImmediate;
		i.size = size;
		i.value = param;
	}else if (type == PKLabel){
		i.type = ParamTImmediate;
		i.size = Size32;
		i.value = param;
		i.is_label = true;
	}else if (type == PKDerefConstant){
		i.type = ParamTImmediate;
		i.deref = true;
		i.size = size;
		i.value = param;
	}else if (type == PKEdxRel){
		i.type = ParamTRegister;
		i.reg = RegisterByID[RegEdx];
		i.deref = true;
		i.disp = ((param < 120) && (param > -120)) ? DispMode8 : DispMode32;
		i.value = param;
	}else if (type == PKLocal){
		i.type = ParamTRegister;
		i.reg = RegisterByID[RegEbp];
		i.deref = true;
		i.size = size;
		i.disp = ((param < 120) && (param > -120)) ? DispMode8 : DispMode32;
		i.value = param;
	}else if (type == PKRegister){
		i.type = ParamTRegister;
		i.reg = RegisterByID[(long)param];
		i.size = i.reg->size;
	}else if (type == PKDerefRegister){
		i.type = ParamTRegister;
		i.reg = RegisterByID[(long)param];
		i.size = size;
		i.deref = true;
	}
	return i;
}

int GetModRMReg(Register *r)
{
	int id = r->id;
	if ((id == RegR8)  || (id == RegR8d)  || (id == RegRax) || (id == RegEax) || (id == RegAx) || (id == RegAl))	return 0x00;
	if ((id == RegR9)  || (id == RegR9d)  || (id == RegRcx) || (id == RegEcx) || (id == RegCx) || (id == RegCl))	return 0x01;
	if ((id == RegR10) || (id == RegR10d) || (id == RegRdx) || (id == RegEdx) || (id == RegDx) || (id == RegDl))	return 0x02;
	if ((id == RegR11) || (id == RegR11d) || (id == RegRbx) || (id == RegEbx) || (id == RegBx) || (id == RegBl))	return 0x03;
	if ((id == RegR12) || (id == RegR12d) || (id == RegRsp) || (id == RegEsp) || (id == RegSp) || (id == RegAh))	return 0x04;
	if ((id == RegR13) || (id == RegR13d) || (id == RegRbp) || (id == RegEbp) || (id == RegBp) || (id == RegCh))	return 0x05;
	if ((id == RegR14) || (id == RegR14d) || (id == RegRsi) || (id == RegEsi) || (id == RegSi) || (id == RegDh))	return 0x06;
	if ((id == RegR15) || (id == RegR15d) || (id == RegRdi) || (id == RegEdi) || (id == RegDi) || (id == RegBh))	return 0x07;
	if ((id >= RegXmm0) && (id <= RegXmm7))	return (id - RegXmm0);
	SetError("GetModRMReg: register not allowed: " + r->name);
	return 0;
}

inline int CreatePartialModRMByte(InstructionParamFuzzy &pf, InstructionParam &p)
{
	int r = -1;
	if (p.reg)
		r = p.reg->id;
	if (pf.mrm_mode == MRMReg){
		if (r == RegEs)	return 0x00;
		if (r == RegCs)	return 0x08;
		if (r == RegSs)	return 0x10;
		if (r == RegDs)	return 0x18;
		if (r == RegFs)	return 0x20;
		if (r == RegGs)	return 0x28;
		if (r == RegCr0)	return 0x00;
		if (r == RegCr1)	return 0x08;
		if (r == RegCr2)	return 0x10;
		if (r == RegCr3)	return 0x18;
		int mrm = GetModRMReg(p.reg) << 3;
		if (p.reg->extend_mod_rm)
			mrm += 0x0400; // REXR
		return mrm;
	}else if (pf.mrm_mode == MRMModRM){
		if (p.deref){
			if (state.AddrSize == Size16){
				if ((p.type == ParamTImmediate) && (p.deref))	return 0x06;
			}else{
				if ((r == RegEax) || (r == RegRax))	return (p.disp == DispModeNone) ? 0x00 : ((p.disp == DispMode8) ? 0x40 : 0x80); // default = DispMode32
				if ((r == RegEcx) || (r == RegRcx))	return (p.disp == DispModeNone) ? 0x01 : ((p.disp == DispMode8) ? 0x41 : 0x81);
				if ((r == RegEdx) || (r == RegRdx))	return (p.disp == DispModeNone) ? 0x02 : ((p.disp == DispMode8) ? 0x42 : 0x82);
				if ((r == RegEbx) || (r == RegRbx))	return (p.disp == DispModeNone) ? 0x03 : ((p.disp == DispMode8) ? 0x43 : 0x83);
				// sib			return 4;
				// disp32		return 5;
				if ((p.type == ParamTImmediate) && (p.deref))	return 0x05;
				if ((r == RegEbp) || (r == RegRbp))	return (p.disp == DispMode8) ? 0x45 : 0x85;
				if ((r == RegEsi) || (r == RegRsi))	return (p.disp == DispModeNone) ? 0x06 : ((p.disp == DispMode8) ? 0x46 : 0x86);
				if ((r == RegEdi) || (r == RegRdi))	return (p.disp == DispModeNone) ? 0x07 : ((p.disp == DispMode8) ? 0x47 : 0x87);
			}
		}else{
			int mrm = GetModRMReg(p.reg) | 0xc0;
			if (p.reg->extend_mod_rm)
				mrm += 0x0100; // REXB
			return mrm;
		}
	}
	if (pf.mrm_mode != MRMNone)
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
	msg_db_f("OpcodeAddInstruction", 1+ASM_DB_LEVEL);
	//---msg_write("add inst " + inst.name);

	// 16/32 bit toggle prefix
	if ((!inst.has_fixed_param) && (inst.has_small_param != (state.DefaultSize == Size16)))
		append_val(oc, ocs, 0x66, 1);

	int mod_rm = 0;
	if (inst.has_modrm)
		mod_rm = CreateModRMByte(inst, p1, p2);

	// REX prefix
	char rex = mod_rm >> 8;
	if ((inst.param1.reg) && (p1.reg))
		if ((inst.param1.reg->id >= RegRax) && (inst.param1.reg->id <= RegRbp) && (inst.param1.reg->id == p1.reg->id + RegRax - RegR8))
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
	if (p2.type == ParamTImmediate)
		param2_size = p2.size;

	OpcodeAddImmideate(oc, ocs, p1, inst, list, param2_size);
	OpcodeAddImmideate(oc, ocs, p2, inst, list, 0);
}

void InstructionWithParamsList::AddInstruction(char *oc, int &ocs, int n)
{
	msg_db_f("AsmAddInstructionLow", 1+ASM_DB_LEVEL);

	int ocs0 = ocs;
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;
	state.reset();

	// test if any instruction matches our wishes
	int ninst = -1;
	bool has_mod_rm = false;
	foreachi(CPUInstruction &c, CPUInstructions, i)
		if ((!c.ignore) && (c.match(iwp))){
			if (((!c.has_modrm) && (has_mod_rm)) || (ninst < 0)){
				has_mod_rm = c.has_modrm;
				ninst = i;
			}
		}

/*	// try again with REX prefix?
 // now done automatically...!
	if ((ninst < 0) && (InstructionSet.set == InstructionSetAMD64)){
		state.ParamSize = Size64;

		for (int i=0;i<CPUInstructions.num;i++)
			if (CPUInstructions[i].match(iwp)){
				if (((!CPUInstructions[i].has_modrm) && (has_mod_rm)) || (ninst < 0)){
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
				SetError("command not compatible with its parameters\n" + InstructionNames[i].name + " " + iwp.p1.str() + ", " + iwp.p2.str());
		SetError(format("instruction unknown: %d", iwp.inst));
	}


	if (DebugAsm)
		CPUInstructions[ninst].print();

	// compile
	OpcodeAddInstruction(oc, ocs, CPUInstructions[ninst], iwp.p1, iwp.p2, *this);
	iwp.size = ocs - ocs0;

	//msg_write(d2h(&oc[ocs0], ocs - ocs0, false));
}

void InstructionWithParamsList::ShrinkJumps(void *oc, int ocs)
{
	// first pass compilation (we need real jump distances)
	int _ocs = ocs;
	Compile(oc, _ocs);
	wanted_label.clear();

	// try shrinking
	foreachi(InstructionWithParams &iwp, *this, i){
		if ((iwp.inst == inst_jmp) || (iwp.inst == inst_jz) || (iwp.inst == inst_jnz) || (iwp.inst == inst_jl) || (iwp.inst == inst_jnl) || (iwp.inst == inst_jle) || (iwp.inst == inst_jnle)){
			if (iwp.p1.is_label){
				int target = label[iwp.p1.value].InstNo;

				// jump distance
				int dist = 0;
				for (int j=i+1;j<target;j++)
					dist += (*this)[j].size;
				for (int j=target;j<=i;j++)
					dist += (*this)[j].size;
				//msg_write(format("%d %d   %d", i, target, dist));

				if (dist < 127){
					so("really shrink");
					iwp.p1.size = Size8;
				}
			}
		}
	}
}

void InstructionWithParamsList::Optimize(void *oc, int ocs)
{
	ShrinkJumps(oc, ocs);
}

void InstructionWithParamsList::Compile(void *oc, int &ocs)
{
	state.DefaultSize = Size32;
	state.reset();
	if (!CurrentMetaInfo){
		DummyMetaInfo.CodeOrigin = (long)oc;
		CurrentMetaInfo = &DummyMetaInfo;
	}

	for (int i=0;i<num+1;i++){
		// bit change
		foreach(BitChange &b, CurrentMetaInfo->bit_change)
			if (b.cmd_pos == i){
				state.DefaultSize = Size32;
				if (b.bits == 16)
					state.DefaultSize = Size16;
				state.reset();
				b.offset = ocs;
			}

		// data?
		foreach(AsmData &d, CurrentMetaInfo->data)
			if (d.cmd_pos == i)
				d.offset = ocs;

		// defining a label?
		for (int j=0;j<label.num;j++)
			if (i == label[j].InstNo){
				so("defining found: " + label[j].Name);
				label[j].Value = CurrentMetaInfo->CodeOrigin + ocs;
			}
		if (i >= num)
			break;

		// opcode
		AddInstruction((char*)oc, ocs, i);
	}

	LinkWantedLabels(oc);

	foreach(WantedLabel &l, wanted_label){
		if (l.Name.head(10) == "kaba-func:")
			continue;
		state.LineNo = (*this)[l.InstNo].line;
		state.ColumnNo = (*this)[l.InstNo].col;
		SetError("undeclared label used: " + l.Name);
	}
}

// only used for error messages
void param2str(string &str, int type, int size, void *param)
{
	switch(type){
		case PKNone:
			str = "---";
			break;
		case PKRegister:
			str = "reg ";
			if (((long)param >= 0) && ((long)param < RegisterByID.num)){
				if (RegisterByID[(long)param])
					str += RegisterByID[(long)param]->name;
				else
					str += "-----evil----";
			}else
				str += "-----evil----";
			break;
		case PKDerefRegister:
			str = get_size_name(size) + " deref reg ";
			if (((long)param >= 0) && ((long)param < RegisterByID.num)){
				if (RegisterByID[(long)param])
					str += RegisterByID[(long)param]->name;
				else
					str += "-----evil----";
			}else
				str += "-----evil----";
			break;
		case PKLocal:
			str = get_size_name(size) + " local " + d2h(&param, 4);
			break;
		case PKEdxRel:
			str = "edx rel " + d2h(&param, 4);
			break;
		case PKConstant:
			str = "const " + d2h(&param, size);
			break;
		/*case PKConstantDouble:
			str = "const 2x " + d2h(&param, 6);
			break;*/
		case PKDerefConstant:
			str = get_size_name(size) + "deref const [" + d2h(&param, 4) + "]";
			break;
		default:
			str = format("??? (%d)", type);
			break;
	}
}

void AddInstruction(char *oc, int &ocs, int inst, int param1_type, int param1_size, void *param1, int param2_type, int param2_size, void *param2)
{
	msg_db_f("AsmAddInstruction", 1+ASM_DB_LEVEL);
	/*if (!CPUInstructions)
		SetInstructionSet(InstructionSetDefault);*/
	state.DefaultSize = Size32;
	state.reset();
	/*msg_write("--------");
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionName[i].inst == inst)
			printf("%s\n", InstructionName[i].name);*/

	InstructionParam wp1 = _make_param_(param1_type, param1_size, (long)param1);
	InstructionParam wp2 = _make_param_(param2_type, param2_size, (long)param2);

	OCParam = ocs;
	InstructionWithParamsList list = InstructionWithParamsList(0);
	InstructionWithParams iwp;
	iwp.inst = inst;
	iwp.p1 = wp1;
	iwp.p2 = wp2;
	iwp.line = -1;
	list.add(iwp);
	list.AddInstruction(oc, ocs, 0);
}

bool ImmediateAllowed(int inst)
{
	for (int i=0;i<CPUInstructions.num;i++)
		if (CPUInstructions[i].inst == inst)
			if ((CPUInstructions[i].param1.allow_immediate) || (CPUInstructions[i].param2.allow_immediate))
				return true;
	return false;
}

};
