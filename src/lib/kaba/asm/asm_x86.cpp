/*
 * asm_x86.cpp
 *
 *  Created on: 09.09.2019
 *      Author: michi
 */

#include "../../base/base.h"
#include "../../file/msg.h"
#include "asm.h"
#include "internal.h"
#include <stdio.h>

namespace Asm {


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



// expands the short instruction parameters
//   returns true if mod/rm byte needed
bool _get_inst_param_(int param, InstructionParamFuzzy &ip)
{
	ip.reg = nullptr;
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
	if ((i.has_big_param) and (instruction_set.set != InstructionSet::AMD64))
		return;

	if (inst == INST_LEA)
		i.param2.size = SIZE_UNKNOWN;

	i.name = InstructionNames[NUM_INSTRUCTION_NAMES].name;
	for (int j=0;j<NUM_INSTRUCTION_NAMES;j++)
		if (inst == InstructionNames[j].inst)
			i.name = InstructionNames[j].name;
	CPUInstructions.add(i);
}


void x86_init()
{
	auto set = instruction_set.set;

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
	add_inst(INST_XOR,	0x35	,1	,-1	,REG_RAX	,Id, OPT_BIG_PARAM);
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
	if (set == InstructionSet::X86){
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
	if (set == InstructionSet::X86){
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
	}else if (set == InstructionSet::AMD64){
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
	add_inst(INST_CALL,	0xe8,	1,	-1,	Jw,	-1, OPT_SMALL_PARAM); // well... "Av" in table
	add_inst(INST_CALL,	0xe8,	1,	-1,	Jd,	-1, OPT_MEDIUM_PARAM);
//	add_inst(INST_CALL,	0xe8,	1,	-1,	Jq,	-1, OPT_BIG_PARAM);
	add_inst(INST_JMP,	0xe9,	1,	-1,	Jw,	-1, OPT_SMALL_PARAM); // miswritten in the table
	add_inst(INST_JMP,	0xe9,	1,	-1,	Jd,	-1, OPT_MEDIUM_PARAM);
//	add_inst(INST_JMP,	0xe9,	1,	-1,	Jq,	-1, OPT_BIG_PARAM);
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
	add_inst(INST_MOVLPS, 0x120f,   2, -1, Xx, XMq);
	add_inst(INST_MOVLPS, 0x130f,   2, -1, XMq, Xx);
	add_inst(INST_MOVHPS, 0x160f,   2, -1, Xx, XMq);
	add_inst(INST_MOVHPS, 0x170f,   2, -1, XMq, Xx);
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
	add_inst(INST_CVTSS2SD, 0x5a0ff3, 3, -1, Xx, XMd);
	add_inst(INST_CVTSD2SS, 0x5a0ff2, 3, -1, Xx, XMq);
	add_inst(INST_CVTTSS2SI, 0x2c0ff3, 3, -1, Rd, XMd);
	add_inst(INST_CVTTSD2SI, 0x2c0ff2, 3, -1, Rq, XMd);
	add_inst(INST_CVTSI2SS,  0x2a0ff3, 3, -1, Xx, Ed);
	add_inst(INST_CVTSI2SD,  0x2a0ff2, 3, -1, Xx, Eq);
	add_inst(INST_COMISS,    0x2f0f,   2, -1, Xx, XMd);
	add_inst(INST_COMISD,    0x2f0f66, 3, -1, Xx, XMq);
	add_inst(INST_UCOMISS,   0x2e0f,   2, -1, Xx, XMd);
	add_inst(INST_UCOMISD,   0x2e0f66, 3, -1, Xx, XMq);

	if (set == InstructionSet::AMD64){
		add_inst(INST_SYSCALL,	0x050f, 2, -1, -1, -1);
		add_inst(INST_SYSRET,	0x070f, 2, -1, -1, -1);
		add_inst(INST_SYSENTER,	0x340f, 2, -1, -1, -1);
		add_inst(INST_SYSEXIT,	0x350f, 2, -1, -1, -1);
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
			reg = (reg >> 3) | (state.extend_mod_rm_reg ? 0x08 : 0x00);
			p.reg = RegisterByID[GetModRMRegister(reg, p.size, REG_GROUP_GENERAL)];
		}
	}else if (pf.mrm_mode == MRM_MOD_RM){
		unsigned char mod = modrm & 0xc0; // bits 7, 6
		unsigned char rm = modrm & 0x07; // bits 2, 1, 0
		if (state.extend_mod_rm_base)	rm |= 0x08;
		if (mod == 0x00){
			if (state.addr_size == SIZE_16){
				p.type = PARAMT_REGISTER;
				p.deref = true;
				if (rm == 0x00){p.reg = RegisterByID[REG_BX];	p.reg2 = RegisterByID[REG_SI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x01){p.reg = RegisterByID[REG_BX];	p.reg2 = RegisterByID[REG_DI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x02){p.reg = RegisterByID[REG_BP];	p.reg2 = RegisterByID[REG_SI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x03){p.reg = RegisterByID[REG_BP];	p.reg2 = RegisterByID[REG_DI];	p.disp = DISP_MODE_REG2;	}
				if (rm == 0x04)	p.reg = RegisterByID[REG_SI];
				if (rm == 0x05)	p.reg = RegisterByID[REG_DI];
				if (rm == 0x06){p.reg = nullptr;	p.type = PARAMT_IMMEDIATE;	}
				if (rm == 0x07)	p.reg = RegisterByID[REG_BX];
			}else{
				p.type = PARAMT_REGISTER;
				p.deref = true;
				//if (rm == 0x04){p.reg = NULL;	p.disp = DispModeSIB;	p.type = ParamTImmediate;}//p.type = ParamTInvalid;	Error("kein SIB byte...");}
				if (rm == 0x04){p.reg = RegisterByID[REG_EAX];	p.disp = DISP_MODE_SIB;	} // eax = provisoric
				else if (rm == 0x05){p.reg = nullptr;	p.type = PARAMT_IMMEDIATE;	}
				else
					p.reg = RegisterByID[GetModRMRegister(rm, SIZE_32, REG_GROUP_GENERAL)];
			}
		}else if ((mod == 0x40) or (mod == 0x80)){
			if (state.addr_size == SIZE_16){
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
			if (state.extend_mod_rm_base)	rm |= 0x08;
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


inline void UnfuzzyParam(InstructionParam &p, InstructionParamFuzzy &pf)
{
	p.type = pf._type_;
	p.reg2 = nullptr;
	p.disp = DISP_MODE_NONE;
	p.reg = pf.reg;
	if ((p.reg) and (state.extend_mod_rm_base)){
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

inline void ReadParamData(char *&cur, InstructionParam &p, bool has_modrm)
{
	//char *o = cur;
	p.value = 0;
	if (p.type == PARAMT_IMMEDIATE){
		if (p.deref){
			int size = has_modrm ? state.addr_size : state.full_register_size; // Ov/Mv...
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
	//msg_write((int_p)cur - (int_p)o);
}


string x86_disassemble(void *_code_,int length,bool allow_comments)
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
	state.default_size = SIZE_32;


	while(code < end){
		state.reset(nullptr);
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
				if ((int_p)code - (int_p)orig == CurrentMetaInfo->label[i].pos)
					bufstr += "    " + CurrentMetaInfo->label[i].name + ":\n";
#endif

			// data blocks
			bool inserted = false;
			for (int i=0;i<CurrentMetaInfo->data.num;i++){
				//printf("%d  %d  %d  %d\n", CurrentMetaInfo->data[i].Pos, (int_p)code, (int_p)orig, (int_p)code - (int_p)orig);
				if ((int_p)code - (int_p)orig == CurrentMetaInfo->data[i].offset){
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
				if ((int_p)code-(int_p)orig == CurrentMetaInfo->bit_change[i].offset){
					state.default_size = (CurrentMetaInfo->bit_change[i].bits == 16) ? SIZE_16 : SIZE_32;
					state.reset(nullptr);
					if (state.default_size == SIZE_16)
						bufstr += "   bits_16\n";
					else
						bufstr += "   bits_32\n";
				}
		}

		// code
		Register *seg = nullptr;

		// prefix
		while (true){

			// prefix (size/segment register)
			if (cur[0]==0x67){
				state.addr_size = (state.default_size == SIZE_32) ? SIZE_16 : SIZE_32;
				cur++;
				continue;
			}
			if (cur[0]==0x66){
				state.param_size = (state.default_size == SIZE_32) ? SIZE_16 : SIZE_32;
				cur++;
				continue;
			}

			// REX
			if (instruction_set.set == InstructionSet::AMD64){
				if ((cur[0] & 0xf0) == 0x40){
					if ((cur[0] & 0x08) > 0)
						state.param_size = SIZE_64;
					state.extend_mod_rm_reg = ((cur[0] & 0x04) > 0);
					state.extend_mod_rm_index = ((cur[0] & 0x02) > 0);
					state.extend_mod_rm_base = ((cur[0] & 0x01) > 0);
					cur++;
					continue;
				}
			}

			// segment registers
			if (cur[0]==0x2e){      seg = RegisterByID[REG_CS]; cur++; continue; }
			else if (cur[0]==0x36){ seg = RegisterByID[REG_SS]; cur++; continue; }
			else if (cur[0]==0x3e){ seg = RegisterByID[REG_DS]; cur++; continue; }
			else if (cur[0]==0x26){ seg = RegisterByID[REG_ES]; cur++; continue; }
			else if (cur[0]==0x64){ seg = RegisterByID[REG_FS]; cur++; continue; }
			else if (cur[0]==0x65){ seg = RegisterByID[REG_GS]; cur++; continue; }

			break;

		}

		opcode=cur;

		// instruction
		CPUInstruction *inst = nullptr;
		for (CPUInstruction &ci: CPUInstructions){
			if (ci.code_size == 0)
				continue;
			if (!ci.has_fixed_param){
				if (ci.has_small_param != (state.param_size == SIZE_16))
					continue;
				if (ci.has_big_param != (state.param_size == SIZE_64))
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
			if ((state.param_size != state.default_size) and ((p1.type != PARAMT_REGISTER) or (p1.deref)) and ((p2.type != PARAMT_REGISTER) or p2.deref)){
				if (state.param_size == SIZE_16)
					str += " word";
				else if (state.param_size == SIZE_32)
					str += " dword";
				else if (state.param_size == SIZE_64)
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
				str += d2h(code, (int_p)cur - (int_p)code);
			}
			//msg_write(str);
			bufstr += str;
			bufstr += "\n";

		}else{
			//msg_write(string2("????? -                          unknown         // %s\n",d2h(code,1+int_p(cur)-int_p(code))));
			bufstr += format("????? -                          unknown         // %s\n",d2h(code,1+int_p(cur)-int_p(code)));
			cur ++;
		}

		// done?
		if ((length < 0) and (((unsigned char)opcode[0] == 0xc3) or ((unsigned char)opcode[0] == 0xc2)))
			break;
	}
	return bufstr;
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
	raise_error("GetModRMReg: register not allowed: " + r->name);
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
			if (state.addr_size == SIZE_16){
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
		raise_error(format("unhandled modrm %d %d %s %d %s", pf.mrm_mode, p.type, (p.reg?p.reg->name:""), p.deref, SizeOut(pf.size)));
	return 0x00;
}

int CreateModRMByte(CPUInstruction &inst, InstructionParam &p1, InstructionParam &p2)
{
	int mrm = CreatePartialModRMByte(inst.param1, p1) | CreatePartialModRMByte(inst.param2, p2);
	if (inst.cap >= 0)
		mrm |= (inst.cap << 3);
	return mrm;
}

inline void append_val(char *oc, int &ocs, int64 val, int size)
{
	insert_val(oc, ocs, val, size);
	ocs += size;
}

void OpcodeAddImmideate(char *oc, int &ocs, InstructionParam &p, CPUInstruction &inst, InstructionWithParamsList &list, int next_param_size)
{
	int64 value = p.value;
	int size = 0;
	if (p.type == PARAMT_IMMEDIATE){
		size = p.size;
		if (p.deref){
			//---msg_write("deref....");
			size = state.addr_size; // inst.has_big_addr
			if (instruction_set.set == InstructionSet::AMD64){
				if (inst.has_modrm){
					value -= (int_p)oc + ocs + size + next_param_size; // amd64 uses RIP-relative addressing!
					if ((value >= 0x80000000) or (-value >= 0x80000000)) {
						msg_write("-----");
						inst.print();
						raise_error(format("RIP relative more than 32 bit: %lx from %p", p.value, &oc[ocs]));
					}
				}else{
					size = SIZE_64; // Ov/Mv...
				}
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

	bool rel = ((inst.name[0] == 'j') /*and (inst.param1._type_ != ParamTImmediateDouble)*/) or (inst.inst == INST_CALL) or (inst.name.find("loop") >= 0);
	if (inst.inst == INST_JMP_FAR)
		rel = false;
	if (p.is_label){
		if ((instruction_set.set == InstructionSet::AMD64) and (p.deref))
			rel = true;
		list.add_wanted_label(ocs, p.value, list.current_inst, rel, false, size);
	}else if (rel){
		value -= CurrentMetaInfo->code_origin + ocs + size + next_param_size; // TODO ...first byte of next opcode
	}

	//---msg_write("imm " + i2s(size));
	append_val(oc, ocs, value, size);
}

void OpcodeAddInstruction(char *oc, int &ocs, CPUInstruction &inst, InstructionParam &p1, InstructionParam &p2, InstructionWithParamsList &list)
{
	//---msg_write("add inst " + inst.name);

	// 16/32 bit toggle prefix
	if ((!inst.has_fixed_param) and (inst.has_small_param != (state.default_size == SIZE_16)))
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

static void align_opcode(char *oc, int &ocs, int granularity)
{
	int mask = granularity - 1;
	if ((ocs & mask) == 0)
		return;
	int ocs_new = (ocs | mask) + 1;
	for (int i=ocs; i<ocs_new; i++)
		oc[i] = 0x90;
	ocs = ocs_new;
}

void InstructionWithParamsList::add_instruction(char *oc, int &ocs, int n)
{
	int ocs0 = ocs;
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;
	state.reset(this);

	if (iwp.inst == INST_ALIGN_OPCODE){
		align_opcode(oc, ocs, 16);
		return;
	}

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
	if ((ninst < 0) and (instruction_set.set == InstructionSetAMD64)){
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
		state.line_no = iwp.line;
		for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
			if (InstructionNames[i].inst == iwp.inst)
				raise_error("command not compatible with its parameters\n" + iwp.str());
		raise_error(format("instruction unknown: %d", iwp.inst));
	}


	if (DebugAsm)
		CPUInstructions[ninst].print();

	// compile
	OpcodeAddInstruction(oc, ocs, CPUInstructions[ninst], iwp.p[0], iwp.p[1], *this);
	iwp.size = ocs - ocs0;

	//msg_write(d2h(&oc[ocs0], ocs - ocs0, false));
}

bool immediate_allowed(int inst)
{
	for (CPUInstruction &i: CPUInstructions)
		if (i.inst == inst)
			if ((i.param1.allow_immediate) or (i.param2.allow_immediate))
				return true;
	return false;
}

};


