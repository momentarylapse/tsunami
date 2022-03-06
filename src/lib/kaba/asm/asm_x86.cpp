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

bool reg_between(RegID r, RegID a, RegID b);


// short parameter type
enum {
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
bool _get_inst_param_(int param, InstructionParamFuzzy &ip) {
	ip.reg = nullptr;
	ip.reg_group = RegGroup::NONE;
	ip.mrm_mode = ModRM::NONE;
	ip.reg_group = RegGroup::INVALID;
	ip._type_ = ParamType::INVALID;
	ip.allow_register = false;
	ip.allow_immediate = false;
	ip.allow_memory_address = false;
	ip.allow_memory_indirect = false;
	ip.immediate_is_relative = false;
	if (param < 0) {	ip.used = false;	ip._type_ = ParamType::NONE;	return false;	}
	ip.used = true;

	// is it a register?
	for (int i=0;i<registers.num;i++)
		if (registers[i].id == (RegID)param) {
			ip._type_ = ParamType::REGISTER;
			ip.reg = &registers[i];
			ip.allow_register = true;
			ip.reg_group = registers[i].group;
			ip.size = registers[i].size;
			return false;
		}
	// general reg / mem
	if ((param == Eb) or (param == Eq) or (param == Ew) or (param == Ed) or (param == E48)) {
		ip._type_ = ParamType::INVALID;//ParamTRegisterOrMem;
		ip.allow_register = true;
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroup::GENERAL;
		ip.mrm_mode = ModRM::MOD_RM;
		if (param == Eb)	ip.size = SIZE_8;
		if (param == Ew)	ip.size = SIZE_16;
		if (param == Ed)	ip.size = SIZE_32;
		if (param == Eq)	ip.size = SIZE_64;
		if (param == E48)	ip.size = SIZE_48;
		return true;
	}
	// xmm reg / mem
	if ((param == XMd) or (param == XMq) or (param == XMdq)) {
		ip._type_ = ParamType::INVALID;//ParamTRegisterOrMem;
		ip.allow_register = true;
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroup::XMM;
		ip.mrm_mode = ModRM::MOD_RM;
		if (param == XMd)	ip.size = SIZE_32;
		if (param == XMq)	ip.size = SIZE_64;
		if (param == XMdq)	ip.size = SIZE_128;
		return true;
	}
	// general reg (reg)
	if ((param == Gb) or (param == Gq) or (param == Gw) or (param == Gd)) {
		ip._type_ = ParamType::REGISTER;
		ip.allow_register = true;
		ip.reg_group = RegGroup::GENERAL;
		ip.mrm_mode = ModRM::REG;
		if (param == Gb)	ip.size = SIZE_8;
		if (param == Gw)	ip.size = SIZE_16;
		if (param == Gd)	ip.size = SIZE_32;
		if (param == Gq)	ip.size = SIZE_64;
		return true;
	}
	// general reg (mod)
	if ((param == Rb) or (param == Rq) or (param == Rw) or (param == Rd)) {
		ip._type_ = ParamType::REGISTER;
		ip.allow_register = true;
		ip.reg_group = RegGroup::GENERAL;
		ip.mrm_mode = ModRM::MOD_RM;
		if (param == Rb)	ip.size = SIZE_8;
		if (param == Rw)	ip.size = SIZE_16;
		if (param == Rd)	ip.size = SIZE_32;
		if (param == Rq)	ip.size = SIZE_64;
		return true;
	}
	// immediate
	if ((param == Ib) or (param == Iq) or (param == Iw) or (param == Id) or (param == I48)) {
		ip._type_ = ParamType::IMMEDIATE;
		ip.allow_immediate = true;
		if (param == Ib)	ip.size = SIZE_8;
		if (param == Iw)	ip.size = SIZE_16;
		if (param == Id)	ip.size = SIZE_32;
		if (param == Iq)	ip.size = SIZE_64;
		if (param == I48)	ip.size = SIZE_48;
		return false;
	}
	// immediate (relative)
	if ((param == Jb) or (param == Jq) or (param == Jw) or (param == Jd)) {
		ip._type_ = ParamType::IMMEDIATE;
		ip.allow_immediate = true;
		ip.immediate_is_relative = true;
		if (param == Jb)	ip.size = SIZE_8;
		if (param == Jw)	ip.size = SIZE_16;
		if (param == Jd)	ip.size = SIZE_32;
		if (param == Jq)	ip.size = SIZE_64;
		return false;
	}
	// mem
	if ((param == Ob) or (param == Oq) or (param == Ow) or (param == Od)) {
		ip._type_ = ParamType::MEMORY;
		ip.allow_memory_address = true;
		if (param == Ob)	ip.size = SIZE_8;
		if (param == Ow)	ip.size = SIZE_16;
		if (param == Od)	ip.size = SIZE_32;
		if (param == Oq)	ip.size = SIZE_64;
		return false;
	}
	// mem
	if ((param == Mb) or (param == Mq) or (param == Mw) or (param == Md)) {
		ip._type_ = ParamType::INVALID; // ...
		ip.allow_memory_address = true;
		ip.allow_memory_indirect = true;
		ip.reg_group = RegGroup::GENERAL;
		ip.mrm_mode = ModRM::MOD_RM;
		if (param == Mb)	ip.size = SIZE_8;
		if (param == Mw)	ip.size = SIZE_16;
		if (param == Md)	ip.size = SIZE_32;
		if (param == Mq)	ip.size = SIZE_64;
		return true;
	}
	// control reg
	if ((param == Cb) or (param == Cd) or (param == Cw) or (param == Cd)) {
		ip._type_ = ParamType::REGISTER;
		ip.allow_register = true;
		ip.reg_group = RegGroup::CONTROL;
		ip.mrm_mode = ModRM::REG;
		if (param == Cb)	ip.size = SIZE_8;
		if (param == Cw)	ip.size = SIZE_16;
		if (param == Cd)	ip.size = SIZE_32;
		if (param == Cq)	ip.size = SIZE_64;
		return true;
	}
	// segment reg
	if (param == Sw) {
		ip._type_ = ParamType::REGISTER;
		ip.allow_register = true;
		ip.reg_group = RegGroup::SEGMENT;
		ip.mrm_mode = ModRM::REG;
		ip.size = SIZE_16;
		return true;
	}
	// xmm reg (reg)
	if (param == Xx) {
		ip._type_ = ParamType::REGISTER;
		ip.allow_register = true;
		ip.reg_group = RegGroup::XMM;
		ip.mrm_mode = ModRM::REG;
		ip.size = SIZE_128;
		return true;
	}
	msg_error("asm: unknown instparam (call Michi!)");
	msg_write(param);
	exit(0);
	return false;
}

enum {
	// bits filter / needing prefixes...
	OPT_SMALL_PARAM = 1,
	OPT_SMALL_ADDR,
	OPT_BIG_PARAM,
	OPT_BIG_ADDR,
	OPT_MEDIUM_PARAM,
};


// an instruction/opcode the cpu offers
struct CPUInstruction {
	InstID inst;
	int code, code_size, cap;
	bool has_modrm, has_small_param, has_small_addr, has_medium_param, has_big_param, has_big_addr, has_fixed_param;
	bool ignore;
	InstructionParamFuzzy param1, param2;
	string name;

	bool match(InstructionWithParams &iwp);
	void print() const {
		msg_write(format("inst: %s   %.4x (%d) %d  %s", name, code, code_size, cap, has_modrm ? "modr/m" : ""));
		param1.print();
		param2.print();
	}
};

static Array<CPUInstruction> all_cpu_instructions;
static Array<CPUInstruction> cpu_instructions[(int)InstID::NUM_INSTRUCTION_NAMES];


bool CPUInstruction::match(InstructionWithParams &iwp) {
	if (inst != iwp.inst)
		return false;

	//return (param1.match(iwp.p[0])) and (param2.match(iwp.p[1]));
	bool b = param1.match(iwp.p[0]) and param2.match(iwp.p[1]);
	/*if (b) {
		msg_write("source: " + iwp.p[0].str() + " " + iwp.p[1].str());
		print();
	}*/
	return b;
}

void add_inst(InstID inst, int code, int code_size, int cap, int param1, int param2, int opt = 0, bool ignore = false) {
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
	i.has_medium_param = (opt == OPT_MEDIUM_PARAM);
	i.has_big_param = (opt == OPT_BIG_PARAM);
	i.has_big_addr = (opt == OPT_BIG_ADDR);
	i.has_fixed_param = (opt != OPT_SMALL_PARAM) and (opt != OPT_MEDIUM_PARAM) and (opt != OPT_BIG_PARAM);
	if (i.has_big_param and (instruction_set.set != InstructionSet::AMD64))
		return;

	if (inst == InstID::LEA)
		i.param2.size = SIZE_UNKNOWN;

	i.name = instruction_names[(int)InstID::NUM_INSTRUCTION_NAMES].name;
	for (int j=0;j<(int)InstID::NUM_INSTRUCTION_NAMES;j++)
		if (inst == instruction_names[j].inst)
			i.name = instruction_names[j].name;
	all_cpu_instructions.add(i);
	cpu_instructions[(int)inst].add(i);
}


void x86_init() {
	auto set = instruction_set.set;

	registers.clear();
	add_reg("rax",	RegID::RAX,	RegGroup::GENERAL,	SIZE_64,	RegRoot::A);
	add_reg("eax",	RegID::EAX,	RegGroup::GENERAL,	SIZE_32,	RegRoot::A);
	add_reg("ax",	RegID::AX,	RegGroup::GENERAL,	SIZE_16,	RegRoot::A);
	add_reg("ah",	RegID::AH,	RegGroup::GENERAL,	SIZE_8,	RegRoot::A); // RegResize[] will be overwritten by al
	add_reg("al",	RegID::AL,	RegGroup::GENERAL,	SIZE_8,	RegRoot::A);
	add_reg("rcx",	RegID::RCX,	RegGroup::GENERAL,	SIZE_64,	RegRoot::C);
	add_reg("ecx",	RegID::ECX,	RegGroup::GENERAL,	SIZE_32,	RegRoot::C);
	add_reg("cx",	RegID::CX,	RegGroup::GENERAL,	SIZE_16,	RegRoot::C);
	add_reg("ch",	RegID::CH,	RegGroup::GENERAL,	SIZE_8,	RegRoot::C);
	add_reg("cl",	RegID::CL,	RegGroup::GENERAL,	SIZE_8,	RegRoot::C);
	add_reg("rdx",	RegID::RDX,	RegGroup::GENERAL,	SIZE_64,	RegRoot::D);
	add_reg("edx",	RegID::EDX,	RegGroup::GENERAL,	SIZE_32,	RegRoot::D);
	add_reg("dx",	RegID::DX,	RegGroup::GENERAL,	SIZE_16,	RegRoot::D);
	add_reg("dh",	RegID::DH,	RegGroup::GENERAL,	SIZE_8,	RegRoot::D);
	add_reg("dl",	RegID::DL,	RegGroup::GENERAL,	SIZE_8,	RegRoot::D);
	add_reg("rbx",	RegID::RBX,	RegGroup::GENERAL,	SIZE_64,	RegRoot::B);
	add_reg("ebx",	RegID::EBX,	RegGroup::GENERAL,	SIZE_32,	RegRoot::B);
	add_reg("bx",	RegID::BX,	RegGroup::GENERAL,	SIZE_16,	RegRoot::B);
	add_reg("bh",	RegID::BH,	RegGroup::GENERAL,	SIZE_8,	RegRoot::B);
	add_reg("bl",	RegID::BL,	RegGroup::GENERAL,	SIZE_8,	RegRoot::B);

	add_reg("rsp",	RegID::RSP,	RegGroup::GENERAL,	SIZE_64,	RegRoot::SP);
	add_reg("esp",	RegID::ESP,	RegGroup::GENERAL,	SIZE_32,	RegRoot::SP);
	add_reg("sp",	RegID::SP,	RegGroup::GENERAL,	SIZE_16,	RegRoot::SP);
	add_reg("rbp",	RegID::RBP,	RegGroup::GENERAL,	SIZE_64,	RegRoot::BP);
	add_reg("ebp",	RegID::EBP,	RegGroup::GENERAL,	SIZE_32,	RegRoot::BP);
	add_reg("bp",	RegID::BP,	RegGroup::GENERAL,	SIZE_16,	RegRoot::BP);
	add_reg("rsi",	RegID::RSI,	RegGroup::GENERAL,	SIZE_64,	RegRoot::SI);
	add_reg("esi",	RegID::ESI,	RegGroup::GENERAL,	SIZE_32,	RegRoot::SI);
	add_reg("si",	RegID::SI,	RegGroup::GENERAL,	SIZE_16,	RegRoot::SI);
	add_reg("rdi",	RegID::RDI,	RegGroup::GENERAL,	SIZE_64,	RegRoot::DI);
	add_reg("edi",	RegID::EDI,	RegGroup::GENERAL,	SIZE_32,	RegRoot::DI);
	add_reg("di",	RegID::DI,	RegGroup::GENERAL,	SIZE_16,	RegRoot::DI);

	add_reg("r8",	RegID::R8,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R8);
	add_reg("r8d",	RegID::R8D,	RegGroup::GENERAL2,	SIZE_32,	RegRoot::R8);
	add_reg("r9",	RegID::R9,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R9);
	add_reg("r9d",	RegID::R9D,	RegGroup::GENERAL2,	SIZE_32,	RegRoot::R9);
	add_reg("r10",	RegID::R10,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R10);
	add_reg("r10d",	RegID::R10D,RegGroup::GENERAL2,	SIZE_32,	RegRoot::R10);
	add_reg("r11",	RegID::R11,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R10);
	add_reg("r11d",	RegID::R11D,RegGroup::GENERAL2,	SIZE_32,	RegRoot::R11);
	add_reg("r12",	RegID::R12,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R12);
	add_reg("r12d",	RegID::R12D,RegGroup::GENERAL2,	SIZE_32,	RegRoot::R12);
	add_reg("r13",	RegID::R13,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R13);
	add_reg("r13d",	RegID::R13D,RegGroup::GENERAL2,	SIZE_32,	RegRoot::R13);
	add_reg("r14",	RegID::R14,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R14);
	add_reg("r14d",	RegID::R14D,RegGroup::GENERAL2,	SIZE_32,	RegRoot::R14);
	add_reg("r15",	RegID::R15,	RegGroup::GENERAL2,	SIZE_64,	RegRoot::R15);
	add_reg("r15d",	RegID::R15D,RegGroup::GENERAL2,	SIZE_32,	RegRoot::R15);

	add_reg("cs",	RegID::CS,	RegGroup::SEGMENT,	SIZE_16);
	add_reg("ss",	RegID::SS,	RegGroup::SEGMENT,	SIZE_16);
	add_reg("ds",	RegID::DS,	RegGroup::SEGMENT,	SIZE_16);
	add_reg("es",	RegID::ES,	RegGroup::SEGMENT,	SIZE_16);
	add_reg("fs",	RegID::FS,	RegGroup::SEGMENT,	SIZE_16);
	add_reg("gs",	RegID::GS,	RegGroup::SEGMENT,	SIZE_16);

	add_reg("cr0",	RegID::CR0,	RegGroup::CONTROL,	SIZE_32);
	add_reg("cr1",	RegID::CR1,	RegGroup::CONTROL,	SIZE_32);
	add_reg("cr2",	RegID::RC2,	RegGroup::CONTROL,	SIZE_32);
	add_reg("cr3",	RegID::CR3,	RegGroup::CONTROL,	SIZE_32);
	add_reg("cr4",	RegID::CR4,	RegGroup::CONTROL,	SIZE_32);

	add_reg("st0",	RegID::ST0,	RegGroup::X87,	SIZE_32,	RegRoot::S0); // ??? 32
	add_reg("st1",	RegID::ST1,	RegGroup::X87,	SIZE_32,	RegRoot::S1);
	add_reg("st2",	RegID::ST2,	RegGroup::X87,	SIZE_32,	RegRoot::S2);
	add_reg("st3",	RegID::ST3,	RegGroup::X87,	SIZE_32,	RegRoot::S3);
	add_reg("st4",	RegID::ST4,	RegGroup::X87,	SIZE_32,	RegRoot::S4);
	add_reg("st5",	RegID::ST5,	RegGroup::X87,	SIZE_32,	RegRoot::S5);
	add_reg("st6",	RegID::ST6,	RegGroup::X87,	SIZE_32,	RegRoot::S6);
	add_reg("st7",	RegID::ST7,	RegGroup::X87,	SIZE_32,	RegRoot::S7);

	add_reg("xmm0",	RegID::XMM0,	RegGroup::XMM,	SIZE_128, RegRoot::X0);
	add_reg("xmm1",	RegID::XMM1,	RegGroup::XMM,	SIZE_128, RegRoot::X1);
	add_reg("xmm2",	RegID::XMM2,	RegGroup::XMM,	SIZE_128, RegRoot::X2);
	add_reg("xmm3",	RegID::XMM3,	RegGroup::XMM,	SIZE_128, RegRoot::X3);
	add_reg("xmm4",	RegID::XMM4,	RegGroup::XMM,	SIZE_128, RegRoot::X4);
	add_reg("xmm5",	RegID::XMM5,	RegGroup::XMM,	SIZE_128, RegRoot::X5);
	add_reg("xmm6",	RegID::XMM6,	RegGroup::XMM,	SIZE_128, RegRoot::X6);
	add_reg("xmm7",	RegID::XMM7,	RegGroup::XMM,	SIZE_128, RegRoot::X7);

	// create easy to access array
	register_by_id.clear();
	for (int i=0;i<registers.num;i++) {
		if (register_by_id.num <= (int)registers[i].id)
			register_by_id.resize((int)registers[i].id + 1);
		RegisterByID(registers[i].id) = &registers[i];
	}

	all_cpu_instructions.clear();
	for (int i=0; i<(int)InstID::NUM_INSTRUCTION_NAMES; i++)
		cpu_instructions[i].clear();
	add_inst(InstID::DB		,0x00	,0	,-1	,Ib	,-1);
	add_inst(InstID::DW		,0x00	,0	,-1	,Iw	,-1);
	add_inst(InstID::DD		,0x00	,0	,-1	,Id	,-1);
	add_inst(InstID::DQ		,0x00	,0	,-1	,Iq	,-1);
	add_inst(InstID::ADD		,0x00	,1	,-1	,Eb	,Gb);
	add_inst(InstID::ADD		,0x01	,1	,-1	,Ew	,Gw, OPT_SMALL_PARAM);
	add_inst(InstID::ADD		,0x01	,1	,-1	,Ed	,Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADD		,0x01	,1	,-1	,Eq	,Gq, OPT_BIG_PARAM);
	add_inst(InstID::ADD		,0x02	,1	,-1	,Gb	,Eb);
	add_inst(InstID::ADD		,0x03	,1	,-1	,Gw	,Eq, OPT_SMALL_PARAM);
	add_inst(InstID::ADD		,0x03	,1	,-1	,Gd	,Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADD		,0x03	,1	,-1	,Gq	,Eq, OPT_BIG_PARAM);
	add_inst(InstID::ADD		,0x04	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::ADD		,0x05	,1	,-1	,(int)RegID::AX, Iw, OPT_SMALL_PARAM);
	add_inst(InstID::ADD		,0x05	,1	,-1	,(int)RegID::EAX, Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADD		,0x05	,1	,-1	,(int)RegID::RAX, Id, OPT_BIG_PARAM);
	add_inst(InstID::PUSH		,0x06	,1	,-1	,(int)RegID::ES	,-1);
	add_inst(InstID::POP		,0x07	,1	,-1	,(int)RegID::ES	,-1);
	add_inst(InstID::OR		,0x08	,1	,-1	,Eb	,Gb);
	add_inst(InstID::OR		,0x09	,1	,-1	,Ew	,Gw, OPT_SMALL_PARAM);
	add_inst(InstID::OR		,0x09	,1	,-1	,Ed	,Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::OR		,0x09	,1	,-1	,Eq	,Gq, OPT_BIG_PARAM);
	add_inst(InstID::OR		,0x0a	,1	,-1	,Gb	,Eb);
	add_inst(InstID::OR,	0x0b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::OR,	0x0b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::OR,	0x0b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::OR		,0x0c	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::OR		,0x0d	,1	,-1	,(int)RegID::AX,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::OR		,0x0d	,1	,-1	,(int)RegID::EAX,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::OR		,0x0d	,1	,-1	,(int)RegID::RAX,	Id, OPT_BIG_PARAM);
	add_inst(InstID::PUSH		,0x0e	,1	,-1	,(int)RegID::CS	,-1);
	add_inst(InstID::SLDT		,0x000f	,2	,0	,Ew	,-1);
	add_inst(InstID::STR		,0x000f	,2	,1	,Ew	,-1);
	add_inst(InstID::LLDT		,0x000f	,2	,2	,Ew	,-1);
	add_inst(InstID::LTR		,0x000f	,2	,3	,Ew	,-1);
	add_inst(InstID::VERR		,0x000f	,2	,4	,Ew	,-1);
	add_inst(InstID::VERW		,0x000f	,2	,5	,Ew	,-1);
	add_inst(InstID::SGDT,	0x010f,	2,	0,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::SGDT,	0x010f,	2,	0,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::SGDT,	0x010f,	2,	0,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::SIDT,	0x010f,	2,	1,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::SIDT,	0x010f,	2,	1,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::SIDT,	0x010f,	2,	1,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::LGDT,	0x010f,	2,	2,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::LGDT,	0x010f,	2,	2,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::LGDT,	0x010f,	2,	2,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::LIDT,	0x010f,	2,	3,	Mw,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::LIDT,	0x010f,	2,	3,	Md,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::LIDT,	0x010f,	2,	3,	Mq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::SMSW		,0x010f	,2	,4	,Ew	,-1);
	add_inst(InstID::LMSW		,0x010f	,2	,6	,Ew	,-1);
	add_inst(InstID::MOV		,0x200f	,2	,-1	,Rd	,Cd); // Fehler im Algorhytmus!!!!  (wirklich ???) -> Fehler in Tabelle?!?
	add_inst(InstID::MOV		,0x220f	,2	,-1	,Cd	,Rd);
	add_inst(InstID::JO		,0x800f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM); // 16/32 bit???
	add_inst(InstID::JO		,0x800f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNO		,0x810f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNO		,0x810f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JB		,0x820f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JB		,0x820f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNB		,0x830f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNB		,0x830f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JZ		,0x840f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JZ		,0x840f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNZ		,0x850f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNZ		,0x850f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JBE		,0x860f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JBE		,0x860f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNBE		,0x870f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNBE		,0x870f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JS		,0x880f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JS		,0x880f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNS		,0x890f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNS		,0x890f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JP		,0x8a0f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JP		,0x8a0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNP		,0x8b0f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNP		,0x8b0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JL		,0x8c0f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JL		,0x8c0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNL		,0x8d0f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNL		,0x8d0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JLE		,0x8e0f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JLE		,0x8e0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JNLE		,0x8f0f	,2	,-1	,Iw	,-1, OPT_SMALL_PARAM);
	add_inst(InstID::JNLE		,0x8f0f	,2	,-1	,Id	,-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::SETO		,0x900f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNO		,0x910f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETB		,0x920f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNB		,0x930f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETZ		,0x940f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNZ		,0x950f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETBE		,0x960f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNBE	,0x970f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETS		,0x980f	,2	,-1	,Eb	,-1); // error in table... "Ev"
	add_inst(InstID::SETNS		,0x990f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETP		,0x9a0f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNP		,0x9b0f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETL		,0x9c0f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNL		,0x9d0f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETLE		,0x9e0f	,2	,-1	,Eb	,-1);
	add_inst(InstID::SETNLE	,0x9f0f	,2	,-1	,Eb	,-1);
	add_inst(InstID::IMUL,	0xaf0f,	2,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::IMUL,	0xaf0f,	2,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::IMUL,	0xaf0f,	2,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::MOVZX,	0xb60f,	2,	-1,	Gw,	Eb, OPT_SMALL_PARAM);
	add_inst(InstID::MOVZX,	0xb60f,	2,	-1,	Gd,	Eb, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOVZX,	0xb60f,	2,	-1,	Gq,	Eb, OPT_BIG_PARAM);
	add_inst(InstID::MOVZX,	0xb70f,	2,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::MOVZX,	0xb70f,	2,	-1,	Gd,	Ew, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOVZX,	0xb70f,	2,	-1,	Gq,	Ew, OPT_BIG_PARAM);
	add_inst(InstID::MOVSX,	0xbe0f,	2,	-1,	Gw,	Eb, OPT_SMALL_PARAM);
	add_inst(InstID::MOVSX,	0xbe0f,	2,	-1,	Gd,	Eb, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOVSX,	0xbe0f,	2,	-1,	Gq,	Eb, OPT_BIG_PARAM);
	add_inst(InstID::MOVSX,	0xbf0f,	2,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::MOVSX,	0xbf0f,	2,	-1,	Gd,	Ew, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOVSX,	0xbf0f,	2,	-1,	Gq,	Ew, OPT_BIG_PARAM);
	add_inst(InstID::MOVSXD,	0x63,	1,	-1,	Gq,	Ed, OPT_BIG_PARAM);
	add_inst(InstID::ADC,	0x10	,1	,-1	,Eb	,Gb);
	add_inst(InstID::ADC,	0x11,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::ADC,	0x11,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADC,	0x11,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::ADC,	0x12	,1	,-1	,Gb	,Eb);
	add_inst(InstID::ADC,	0x13,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::ADC,	0x13,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADC,	0x13,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::ADC,	0x14	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::ADC,	0x15	,1	,-1	,(int)RegID::AX,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::ADC,	0x15	,1	,-1	,(int)RegID::EAX, Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADC,	0x15	,1	,-1	,(int)RegID::RAX, Id, OPT_BIG_PARAM);
	add_inst(InstID::PUSH,	0x16	,1	,-1	,(int)RegID::SS, -1);
	add_inst(InstID::POP,	0x17	,1	,-1	,(int)RegID::SS, -1);
	add_inst(InstID::SBB,	0x18	,1	,-1	,Eb	,Gb);
	add_inst(InstID::SBB,	0x19,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::SBB,	0x19,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::SBB,	0x19,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::SBB,	0x1a	,1	,-1	,Gb	,Eb);
	add_inst(InstID::SBB,	0x1b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::SBB,	0x1b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::SBB,	0x1b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::SBB,	0x1c	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::SBB,	0x1d	,1	,-1	,(int)RegID::AX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::SBB,	0x1d	,1	,-1	,(int)RegID::EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::SBB,	0x1d	,1	,-1	,(int)RegID::RAX	,Id, OPT_BIG_PARAM);
	add_inst(InstID::PUSH,	0x1e	,1	,-1	,(int)RegID::DS	,-1);
	add_inst(InstID::POP,	0x1f	,1	,-1	,(int)RegID::DS	,-1);
	add_inst(InstID::AND,	0x20	,1	,-1	,Eb	,Gb);
	add_inst(InstID::AND,	0x21,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::AND,	0x21,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::AND,	0x21,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::AND,	0x22	,1	,-1	,Gb	,Eb);
	add_inst(InstID::AND,	0x23,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::AND,	0x23,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::AND,	0x23,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::AND,	0x24	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::AND,	0x25	,1	,-1	,(int)RegID::AX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::AND,	0x25	,1	,-1	,(int)RegID::EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::AND,	0x25	,1	,-1	,(int)RegID::RAX	,Id, OPT_BIG_PARAM);
	add_inst(InstID::SUB,	0x28	,1	,-1	,Eb	,Gb);
	add_inst(InstID::SUB,	0x29,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::SUB,	0x29,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::SUB,	0x29,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::SUB,	0x2a	,1	,-1	,Gb	,Eb);
	add_inst(InstID::SUB,	0x2b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::SUB,	0x2b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::SUB,	0x2b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::SUB,	0x2c	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::SUB,	0x2d	,1	,-1	,(int)RegID::AX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::SUB,	0x2d	,1	,-1	,(int)RegID::EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::SUB,	0x2d	,1	,-1	,(int)RegID::RAX	,Id, OPT_BIG_PARAM);
	add_inst(InstID::XOR,	0x30	,1	,-1	,Eb	,Gb);
	add_inst(InstID::XOR,	0x31,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::XOR,	0x31,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::XOR,	0x31,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::XOR,	0x32	,1	,-1	,Gb	,Eb);
	add_inst(InstID::XOR,	0x33,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::XOR,	0x33,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::XOR,	0x33,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::XOR,	0x34	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::XOR,	0x35	,1	,-1	,(int)RegID::AX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::XOR,	0x35	,1	,-1	,(int)RegID::EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::XOR,	0x35	,1	,-1	,(int)RegID::RAX	,Id, OPT_BIG_PARAM);
	add_inst(InstID::CMP,	0x38	,1	,-1	,Eb	,Gb);
	add_inst(InstID::CMP,	0x39,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::CMP,	0x39,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::CMP,	0x39,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::CMP,	0x3a	,1	,-1	,Gb	,Eb);
	add_inst(InstID::CMP,	0x3b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::CMP,	0x3b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::CMP,	0x3b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::CMP,	0x3c	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::CMP,	0x3d	,1	,-1	,(int)RegID::AX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::CMP,	0x3d	,1	,-1	,(int)RegID::EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::CMP,	0x3d	,1	,-1	,(int)RegID::RAX	,Id, OPT_BIG_PARAM);
	if (set == InstructionSet::X86) {
		add_inst(InstID::INC		,0x40	,1	,-1	,(int)RegID::EAX	,-1);
		add_inst(InstID::INC		,0x41	,1	,-1	,(int)RegID::ECX	,-1);
		add_inst(InstID::INC		,0x42	,1	,-1	,(int)RegID::EDX	,-1);
		add_inst(InstID::INC		,0x43	,1	,-1	,(int)RegID::EBX	,-1);
		add_inst(InstID::INC		,0x44	,1	,-1	,(int)RegID::ESP	,-1);
		add_inst(InstID::INC		,0x45	,1	,-1	,(int)RegID::EBP	,-1);
		add_inst(InstID::INC		,0x46	,1	,-1	,(int)RegID::ESI	,-1);
		add_inst(InstID::INC		,0x47	,1	,-1	,(int)RegID::EDI	,-1);
		add_inst(InstID::DEC		,0x48	,1	,-1	,(int)RegID::EAX	,-1);
		add_inst(InstID::DEC		,0x49	,1	,-1	,(int)RegID::ECX	,-1);
		add_inst(InstID::DEC		,0x4a	,1	,-1	,(int)RegID::EDX	,-1);
		add_inst(InstID::DEC		,0x4b	,1	,-1	,(int)RegID::EBX	,-1);
		add_inst(InstID::DEC		,0x4c	,1	,-1	,(int)RegID::ESP	,-1);
		add_inst(InstID::DEC		,0x4d	,1	,-1	,(int)RegID::EBP	,-1);
		add_inst(InstID::DEC		,0x4e	,1	,-1	,(int)RegID::ESI	,-1);
		add_inst(InstID::DEC		,0x4f	,1	,-1	,(int)RegID::EDI	,-1);
	}
	if (set == InstructionSet::X86) {
		add_inst(InstID::PUSH		,0x50	,1	,-1	,(int)RegID::EAX	,-1);
		add_inst(InstID::PUSH		,0x51	,1	,-1	,(int)RegID::ECX	,-1);
		add_inst(InstID::PUSH		,0x52	,1	,-1	,(int)RegID::EDX	,-1);
		add_inst(InstID::PUSH		,0x53	,1	,-1	,(int)RegID::EBX	,-1);
		add_inst(InstID::PUSH		,0x54	,1	,-1	,(int)RegID::ESP	,-1);
		add_inst(InstID::PUSH		,0x55	,1	,-1	,(int)RegID::EBP	,-1);
		add_inst(InstID::PUSH		,0x56	,1	,-1	,(int)RegID::ESI	,-1);
		add_inst(InstID::PUSH		,0x57	,1	,-1	,(int)RegID::EDI	,-1);
		add_inst(InstID::POP		,0x58	,1	,-1	,(int)RegID::EAX	,-1);
		add_inst(InstID::POP		,0x59	,1	,-1	,(int)RegID::ECX	,-1);
		add_inst(InstID::POP		,0x5a	,1	,-1	,(int)RegID::EDX	,-1);
		add_inst(InstID::POP		,0x5b	,1	,-1	,(int)RegID::EBX	,-1);
		add_inst(InstID::POP		,0x5c	,1	,-1	,(int)RegID::ESP	,-1);
		add_inst(InstID::POP		,0x5d	,1	,-1	,(int)RegID::EBP	,-1);
		add_inst(InstID::POP		,0x5e	,1	,-1	,(int)RegID::ESI	,-1);
		add_inst(InstID::POP		,0x5f	,1	,-1	,(int)RegID::EDI	,-1);
	} else if (set == InstructionSet::AMD64) {
		add_inst(InstID::PUSH		,0x50	,1	,-1	,(int)RegID::RAX	,-1);
		add_inst(InstID::PUSH		,0x51	,1	,-1	,(int)RegID::RCX	,-1);
		add_inst(InstID::PUSH		,0x52	,1	,-1	,(int)RegID::RDX	,-1);
		add_inst(InstID::PUSH		,0x53	,1	,-1	,(int)RegID::RBX	,-1);
		add_inst(InstID::PUSH		,0x54	,1	,-1	,(int)RegID::RSP	,-1);
		add_inst(InstID::PUSH		,0x55	,1	,-1	,(int)RegID::RBP	,-1);
		add_inst(InstID::PUSH		,0x56	,1	,-1	,(int)RegID::RSI	,-1);
		add_inst(InstID::PUSH		,0x57	,1	,-1	,(int)RegID::RDI	,-1);
		add_inst(InstID::POP		,0x58	,1	,-1	,(int)RegID::RAX	,-1);
		add_inst(InstID::POP		,0x59	,1	,-1	,(int)RegID::RCX	,-1);
		add_inst(InstID::POP		,0x5a	,1	,-1	,(int)RegID::RDX	,-1);
		add_inst(InstID::POP		,0x5b	,1	,-1	,(int)RegID::RBX	,-1);
		add_inst(InstID::POP		,0x5c	,1	,-1	,(int)RegID::RSP	,-1);
		add_inst(InstID::POP		,0x5d	,1	,-1	,(int)RegID::RBP	,-1);
		add_inst(InstID::POP		,0x5e	,1	,-1	,(int)RegID::RSI	,-1);
		add_inst(InstID::POP		,0x5f	,1	,-1	,(int)RegID::RDI	,-1);
	}
	add_inst(InstID::PUSHA		,0x60	,1	,-1	,-1	,-1);
	add_inst(InstID::POPA		,0x61	,1	,-1	,-1	,-1);
	add_inst(InstID::PUSH,	0x68,	1,	-1,	Iw,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::PUSH,	0x68,	1,	-1,	Id,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::PUSH,	0x68,	1,	-1,	Iq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::IMUL,	0x69,	1,	-1,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::IMUL,	0x69,	1,	-1,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::IMUL,	0x69,	1,	-1,	Eq,	Id, OPT_BIG_PARAM); // yes, 32bit imm
	add_inst(InstID::PUSH		,0x6a	,1	,-1	,Ib	,-1);
	add_inst(InstID::JO		,0x70	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNO		,0x71	,1	,-1	,Jb	,-1);
	add_inst(InstID::JB		,0x72	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNB		,0x73	,1	,-1	,Jb	,-1);
	add_inst(InstID::JZ		,0x74	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNZ		,0x75	,1	,-1	,Jb	,-1);
	add_inst(InstID::JBE		,0x76	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNBE		,0x77	,1	,-1	,Jb	,-1);
	add_inst(InstID::JS		,0x78	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNS		,0x79	,1	,-1	,Jb	,-1);
	add_inst(InstID::JP		,0x7a	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNP		,0x7b	,1	,-1	,Jb	,-1);
	add_inst(InstID::JL		,0x7c	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNL		,0x7d	,1	,-1	,Jb	,-1);
	add_inst(InstID::JLE		,0x7e	,1	,-1	,Jb	,-1);
	add_inst(InstID::JNLE		,0x7f	,1	,-1	,Jb	,-1);
	// Immediate Group 1
	add_inst(InstID::ADD		,0x80	,1	,0	,Eb	,Ib);
	add_inst(InstID::OR		,0x80	,1	,1	,Eb	,Ib);
	add_inst(InstID::ADC		,0x80	,1	,2	,Eb	,Ib);
	add_inst(InstID::SBB		,0x80	,1	,3	,Eb	,Ib);
	add_inst(InstID::AND		,0x80	,1	,4	,Eb	,Ib);
	add_inst(InstID::SUB		,0x80	,1	,5	,Eb	,Ib);
	add_inst(InstID::XOR		,0x80	,1	,6	,Eb	,Ib);
	add_inst(InstID::CMP		,0x80	,1	,7	,Eb	,Ib);
	add_inst(InstID::ADD,	0x81,	1,	0,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::ADD,	0x81,	1,	0,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADD,	0x81,	1,	0,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::OR,	0x81,	1,	1,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::OR,	0x81,	1,	1,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::OR,	0x81,	1,	1,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::ADC,	0x81,	1,	2,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::ADC,	0x81,	1,	2,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADC,	0x81,	1,	2,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::SBB,	0x81,	1,	3,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::SBB,	0x81,	1,	3,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::SBB,	0x81,	1,	3,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::AND,	0x81,	1,	4,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::AND,	0x81,	1,	4,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::AND,	0x81,	1,	4,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::SUB,	0x81,	1,	5,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::SUB,	0x81,	1,	5,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::SUB,	0x81,	1,	5,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::XOR,	0x81,	1,	6,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::XOR,	0x81,	1,	6,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::XOR,	0x81,	1,	6,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::CMP,	0x81,	1,	7,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::CMP,	0x81,	1,	7,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::CMP,	0x81,	1,	7,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::ADD,	0x83,	1,	0,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::ADD,	0x83,	1,	0,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADD,	0x83,	1,	0,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::OR,	0x83,	1,	1,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::OR,	0x83,	1,	1,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::OR,	0x83,	1,	1,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::ADC,	0x83,	1,	2,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::ADC,	0x83,	1,	2,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::ADC,	0x83,	1,	2,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::SBB,	0x83,	1,	3,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::SBB,	0x83,	1,	3,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::SBB,	0x83,	1,	3,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::AND,	0x83,	1,	4,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::AND,	0x83,	1,	4,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::AND,	0x83,	1,	4,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::SUB,	0x83,	1,	5,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::SUB,	0x83,	1,	5,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::SUB,	0x83,	1,	5,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::XOR,	0x83,	1,	6,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::XOR,	0x83,	1,	6,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::XOR,	0x83,	1,	6,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::CMP,	0x83,	1,	7,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::CMP,	0x83,	1,	7,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::CMP,	0x83,	1,	7,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::TEST,	0x84	,1	,-1	,Eb	,Gb);
	add_inst(InstID::TEST,	0x85,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::TEST,	0x85,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::TEST,	0x85,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::XCHG,	0x86	,1	,-1	,Eb	,Gb);
	add_inst(InstID::XCHG,	0x87,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG,	0x87,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG,	0x87,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::MOV,	0x88	,1	,-1	,Eb	,Gb);
	add_inst(InstID::MOV,	0x89,	1,	-1,	Ew,	Gw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV,	0x89,	1,	-1,	Ed,	Gd, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV,	0x89,	1,	-1,	Eq,	Gq, OPT_BIG_PARAM);
	add_inst(InstID::MOV,	0x8a	,1	,-1	,Gb	,Eb);
	add_inst(InstID::MOV,	0x8b,	1,	-1,	Gw,	Ew, OPT_SMALL_PARAM);
	add_inst(InstID::MOV,	0x8b,	1,	-1,	Gd,	Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV,	0x8b,	1,	-1,	Gq,	Eq, OPT_BIG_PARAM);
	add_inst(InstID::MOV,	0x8c	,1	,-1	,Ew	,Sw	);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gw,	Ew, OptSmallParam);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gd,	Ed, OptMediumParam);
	//add_inst(inst_lea,	0x8d,	1,	-1,	Gq,	Eq, OptBigParam);
	add_inst(InstID::LEA,	0x8d,	1,	-1,	Gw,	Mw, OPT_SMALL_PARAM);
	add_inst(InstID::LEA,	0x8d,	1,	-1,	Gd,	Md, OPT_MEDIUM_PARAM);
	add_inst(InstID::LEA,	0x8d,	1,	-1,	Gq,	Mq, OPT_BIG_PARAM);
	add_inst(InstID::MOV,	0x8e	,1	,-1	,Sw	,Ew, OPT_SMALL_PARAM);
	add_inst(InstID::MOV,	0x8e	,1	,-1	,Sw	,Ew, OPT_MEDIUM_PARAM); // s registers are always 16 bit!
	add_inst(InstID::POP,	0x8f,	1,	-1,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::POP,	0x8f,	1,	-1,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::POP,	0x8f,	1,	-1,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::NOP		,0x90	,1	,-1	,-1	,-1);
	add_inst(InstID::XCHG		,0x91	,1	,-1	,(int)RegID::AX	,(int)RegID::CX, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x92	,1	,-1	,(int)RegID::AX	,(int)RegID::DX, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x93	,1	,-1	,(int)RegID::AX	,(int)RegID::BX, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x94	,1	,-1	,(int)RegID::AX	,(int)RegID::SP, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x95	,1	,-1	,(int)RegID::AX	,(int)RegID::BP, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x96	,1	,-1	,(int)RegID::AX	,(int)RegID::SI, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x97	,1	,-1	,(int)RegID::AX	,(int)RegID::DI, OPT_SMALL_PARAM);
	add_inst(InstID::XCHG		,0x91	,1	,-1	,(int)RegID::EAX	,(int)RegID::ECX, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x92	,1	,-1	,(int)RegID::EAX	,(int)RegID::EDX, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x93	,1	,-1	,(int)RegID::EAX	,(int)RegID::EBX, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x94	,1	,-1	,(int)RegID::EAX	,(int)RegID::ESP, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x95	,1	,-1	,(int)RegID::EAX	,(int)RegID::EBP, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x96	,1	,-1	,(int)RegID::EAX	,(int)RegID::ESI, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x97	,1	,-1	,(int)RegID::EAX	,(int)RegID::EDI, OPT_MEDIUM_PARAM);
	add_inst(InstID::XCHG		,0x91	,1	,-1	,(int)RegID::RAX	,(int)RegID::RCX, OPT_BIG_PARAM);
	add_inst(InstID::XCHG		,0x92	,1	,-1	,(int)RegID::RAX	,(int)RegID::RDX, OPT_BIG_PARAM);
	add_inst(InstID::XCHG		,0x93	,1	,-1	,(int)RegID::RAX	,(int)RegID::RBX, OPT_BIG_PARAM);
	add_inst(InstID::XCHG		,0x94	,1	,-1	,(int)RegID::RAX	,(int)RegID::RSP, OPT_BIG_PARAM);
	add_inst(InstID::XCHG		,0x95	,1	,-1	,(int)RegID::RAX	,(int)RegID::RBP, OPT_BIG_PARAM);
	add_inst(InstID::XCHG		,0x96	,1	,-1	,(int)RegID::RAX	,(int)RegID::RSI, OPT_BIG_PARAM);
	add_inst(InstID::XCHG		,0x97	,1	,-1	,(int)RegID::RAX	,(int)RegID::RDI, OPT_BIG_PARAM);
	add_inst(InstID::CBW_CWDE	,0x98	,1	,-1	,-1 ,-1);
	add_inst(InstID::CGQ_CWD	,0x99	,1	,-1	,-1 ,-1);
	add_inst(InstID::MOV		,0xa0	,1	,-1	,(int)RegID::AL	,Ob, 0, true);
	add_inst(InstID::MOV		,0xa1	,1	,-1	,(int)RegID::AX	,Ow, OPT_SMALL_PARAM, true);
	add_inst(InstID::MOV		,0xa1	,1	,-1	,(int)RegID::EAX	,Od, OPT_MEDIUM_PARAM, true);
	add_inst(InstID::MOV		,0xa1	,1	,-1	,(int)RegID::RAX	,Oq, OPT_BIG_PARAM, true);
	add_inst(InstID::MOV		,0xa2	,1	,-1	,Ob	,(int)RegID::AL, 0, true);
	add_inst(InstID::MOV,	0xa3,	1,	-1,	Ow,	(int)RegID::AX, OPT_SMALL_PARAM, true);
	add_inst(InstID::MOV,	0xa3,	1,	-1,	Od,	(int)RegID::EAX, OPT_MEDIUM_PARAM, true);
	add_inst(InstID::MOV,	0xa3,	1,	-1,	Oq,	(int)RegID::RAX, OPT_BIG_PARAM, true);
	add_inst(InstID::MOVS_B_DS_ESI_ES_EDI	,0xa4	,1	,-1	,-1,-1);
	add_inst(InstID::MOVS_DS_ESI_ES_EDI	,0xa5	,1	,-1	,-1,-1);
	add_inst(InstID::CMPS_B_DS_ESI_ES_EDI	,0xa6	,1	,-1	,-1,-1);
	add_inst(InstID::CMPS_DS_ESI_ES_EDI	,0xa7	,1	,-1	,-1,-1);
	add_inst(InstID::MOV		,0xb0	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::MOV		,0xb1	,1	,-1	,(int)RegID::CL	,Ib);
	add_inst(InstID::MOV		,0xb2	,1	,-1	,(int)RegID::DL	,Ib);
	add_inst(InstID::MOV		,0xb3	,1	,-1	,(int)RegID::BL	,Ib);
	add_inst(InstID::MOV		,0xb4	,1	,-1	,(int)RegID::AH	,Ib);
	add_inst(InstID::MOV		,0xb5	,1	,-1	,(int)RegID::CH	,Ib);
	add_inst(InstID::MOV		,0xb6	,1	,-1	,(int)RegID::DH	,Ib);
	add_inst(InstID::MOV		,0xb7	,1	,-1	,(int)RegID::BH	,Ib);
	add_inst(InstID::MOV		,0xb8	,1	,-1	,(int)RegID::EAX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xb9	,1	,-1	,(int)RegID::ECX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xba	,1	,-1	,(int)RegID::EDX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xbb	,1	,-1	,(int)RegID::EBX	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xbc	,1	,-1	,(int)RegID::ESP	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xbd	,1	,-1	,(int)RegID::EBP	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xbe	,1	,-1	,(int)RegID::ESI	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xbf	,1	,-1	,(int)RegID::EDI	,Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV		,0xb8	,1	,-1	,(int)RegID::AX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xb9	,1	,-1	,(int)RegID::CX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xba	,1	,-1	,(int)RegID::DX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xbb	,1	,-1	,(int)RegID::BX	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xbc	,1	,-1	,(int)RegID::SP	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xbd	,1	,-1	,(int)RegID::BP	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xbe	,1	,-1	,(int)RegID::SI	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xbf	,1	,-1	,(int)RegID::DI	,Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV		,0xb8	,1	,-1	,(int)RegID::RAX	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xb9	,1	,-1	,(int)RegID::RCX	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xba	,1	,-1	,(int)RegID::RDX	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xbb	,1	,-1	,(int)RegID::RBX	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xbc	,1	,-1	,(int)RegID::RSP	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xbd	,1	,-1	,(int)RegID::RBP	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xbe	,1	,-1	,(int)RegID::RSI	,Iq, OPT_BIG_PARAM);
	add_inst(InstID::MOV		,0xbf	,1	,-1	,(int)RegID::RDI	,Iq, OPT_BIG_PARAM);
	// Shift Group 2
	add_inst(InstID::ROL		,0xc0	,1	,0	,Eb	,Ib);
	add_inst(InstID::ROR		,0xc0	,1	,1	,Eb	,Ib);
	add_inst(InstID::RCL		,0xc0	,1	,2	,Eb	,Ib);
	add_inst(InstID::RCR		,0xc0	,1	,3	,Eb	,Ib);
	add_inst(InstID::SHL		,0xc0	,1	,4	,Eb	,Ib);
	add_inst(InstID::SHR		,0xc0	,1	,5	,Eb	,Ib);
	add_inst(InstID::SAR		,0xc0	,1	,7	,Eb	,Ib);
	add_inst(InstID::ROL,	0xc1,	1,	0,	Ew,	Ib, OPT_SMALL_PARAM); // even though the table says Iv
	add_inst(InstID::ROL,	0xc1,	1,	0,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::ROL,	0xc1,	1,	0,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::ROR,	0xc1,	1,	1,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::ROR,	0xc1,	1,	1,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::ROR,	0xc1,	1,	1,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::RCL,	0xc1,	1,	2,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::RCL,	0xc1,	1,	2,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::RCL,	0xc1,	1,	2,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::RCR,	0xc1,	1,	3,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::RCR,	0xc1,	1,	3,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::RCR,	0xc1,	1,	3,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::SHL,	0xc1,	1,	4,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::SHL,	0xc1,	1,	4,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::SHL,	0xc1,	1,	4,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::SHR,	0xc1,	1,	5,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::SHR,	0xc1,	1,	5,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::SHR,	0xc1,	1,	5,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::SAR,	0xc1,	1,	7,	Ew,	Ib, OPT_SMALL_PARAM);
	add_inst(InstID::SAR,	0xc1,	1,	7,	Ed,	Ib, OPT_MEDIUM_PARAM);
	add_inst(InstID::SAR,	0xc1,	1,	7,	Eq,	Ib, OPT_BIG_PARAM);
	add_inst(InstID::RET		,0xc2	,1	,-1	,Iw	,-1);
	add_inst(InstID::RET		,0xc3	,1	,-1	,-1	,-1);
	add_inst(InstID::MOV		,0xc6	,1	,-1	,Eb	,Ib);
	add_inst(InstID::MOV,	0xc7,	1,	-1,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::MOV,	0xc7,	1,	-1,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::MOV,	0xc7,	1,	-1,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::LEAVE		,0xc9	,1	,-1	,-1	,-1);
	add_inst(InstID::RET_FAR	,0xca	,1	,-1	,Iw	,-1);
	add_inst(InstID::RET_FAR	,0xcb	,1	,-1	,-1	,-1);
	add_inst(InstID::INT		,0xcd	,1	,-1	,Ib	,-1);
	add_inst(InstID::IRET		,0xcf	,1	,-1	,-1	,-1);
	add_inst(InstID::ROL,	0xd3,	1,	0,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::ROL,	0xd3,	1,	0,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::ROL,	0xd3,	1,	0,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::ROR,	0xd3,	1,	1,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::ROR,	0xd3,	1,	1,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::ROR,	0xd3,	1,	1,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::RCL,	0xd3,	1,	2,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::RCL,	0xd3,	1,	2,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::RCL,	0xd3,	1,	2,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::RCR,	0xd3,	1,	3,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::RCR,	0xd3,	1,	3,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::RCR,	0xd3,	1,	3,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::SHL,	0xd3,	1,	4,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::SHL,	0xd3,	1,	4,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::SHL,	0xd3,	1,	4,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::SHR,	0xd3,	1,	5,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::SHR,	0xd3,	1,	5,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::SHR,	0xd3,	1,	5,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::SAR,	0xd3,	1,	7,	Ew,	(int)RegID::CL, OPT_SMALL_PARAM);
	add_inst(InstID::SAR,	0xd3,	1,	7,	Ed,	(int)RegID::CL, OPT_MEDIUM_PARAM);
	add_inst(InstID::SAR,	0xd3,	1,	7,	Eq,	(int)RegID::CL, OPT_BIG_PARAM);
	add_inst(InstID::FADD,	0xd8,	1,	0,	Ed,	-1);
	add_inst(InstID::FADD,	0xdc,	1,	0,	Eq,	-1);
	add_inst(InstID::FMUL,	0xd8,	1,	1,	Ed,	-1);
	add_inst(InstID::FMUL,	0xdc,	1,	1,	Eq,	-1);
	add_inst(InstID::FSUB,	0xd8,	1,	4,	Ed,	-1);
	add_inst(InstID::FSUB,	0xdc,	1,	4,	Eq,	-1);
	add_inst(InstID::FDIV,	0xd8,	1,	6,	Ed,	-1);
	add_inst(InstID::FDIV,	0xdc,	1,	6,	Eq,	-1);
	add_inst(InstID::FLD,	0xd9,	1,	0,	Md,	-1);
	add_inst(InstID::FLD,	0xdd,	1,	0,	Mq,	-1);
	add_inst(InstID::FLD1,	0xe8d9,	2,	-1,	-1,	-1);
	add_inst(InstID::FLDZ,	0xeed9,	2,	-1,	-1,	-1);
	add_inst(InstID::FLDPI,	0xebd9,	2,	-1,	-1,	-1);
	add_inst(InstID::FST,	0xd9,	1,	2,	Md,	-1);
	add_inst(InstID::FST,	0xdd,	1,	2,	Mq,	-1);
	add_inst(InstID::FSTP,	0xd9,	1,	3,	Md,	-1);
	add_inst(InstID::FSTP,	0xdd,	1,	3,	Mq,	-1);
	add_inst(InstID::FLDCW,	0xd9,	1,	5,	Mw,	-1);
	add_inst(InstID::FNSTCW,	0xd9,	1,	7,	Mw,	-1);
	add_inst(InstID::FXCH		,0xc9d9	,2	,-1	,(int)RegID::ST0	,(int)RegID::ST1);
	add_inst(InstID::FUCOMPP	,0xe9da	,2	,-1	,(int)RegID::ST0	,(int)RegID::ST1);

	add_inst(InstID::FSQRT,	0xfad9,	2,	-1,	-1, -1);
	add_inst(InstID::FSIN,	0xfed9,	2,	-1,	-1, -1);
	add_inst(InstID::FCOS,	0xffd9,	2,	-1,	-1, -1);
	add_inst(InstID::FPTAN,	0xf2d9,	2,	-1,	-1, -1);
	add_inst(InstID::FPATAN,	0xf3d9,	2,	-1,	-1, -1);
	add_inst(InstID::FYL2X,	0xf1d9,	2,	-1,	-1, -1);
	add_inst(InstID::FISTP,	0xdb	,1	,3	,Md	,-1);
	add_inst(InstID::FILD,	0xdb,	1,	0,	Ed,	-1);
	add_inst(InstID::FADDP,	0xde,	1,	0,	Ed,	-1);
	add_inst(InstID::FMULP,	0xde,	1,	1,	Ed,	-1);
	add_inst(InstID::FSUBP,	0xde,	1,	5,	Ed,	-1);
	add_inst(InstID::FDIVP,	0xde,	1,	7,	Ed,	-1); // de.f9 ohne Parameter...?
	add_inst(InstID::FNSTSW	,0xe0df	,2	,-1	,(int)RegID::AX	,-1);
	add_inst(InstID::LOOPNE	,0xe0	,1	,-1	,Jb	,-1);
	add_inst(InstID::LOOPE		,0xe1	,1	,-1	,Jb	,-1);
	add_inst(InstID::LOOP		,0xe2	,1	,-1	,Jb	,-1);
	add_inst(InstID::IN		,0xe4	,1	,-1	,(int)RegID::AL	,Ib);
	add_inst(InstID::IN		,0xe5	,1	,-1	,(int)RegID::EAX,Ib);
	add_inst(InstID::OUT		,0xe6	,1	,-1	,Ib	,(int)RegID::AL);
	add_inst(InstID::OUT		,0xe7	,1	,-1	,Ib	,(int)RegID::EAX);
	add_inst(InstID::CALL,	0xe8,	1,	-1,	Jw,	-1, OPT_SMALL_PARAM); // well... "Av" in table
	add_inst(InstID::CALL,	0xe8,	1,	-1,	Jd,	-1, OPT_MEDIUM_PARAM);
//	add_inst(InstID::CALL,	0xe8,	1,	-1,	Jq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::JMP,	0xe9,	1,	-1,	Jw,	-1, OPT_SMALL_PARAM); // miswritten in the table
	add_inst(InstID::JMP,	0xe9,	1,	-1,	Jd,	-1, OPT_MEDIUM_PARAM);
//	add_inst(InstID::JMP,	0xe9,	1,	-1,	Jq,	-1, OPT_BIG_PARAM);
//	add_inst(inst_jmp		,0xea	,1	,-1, Ap, -1); TODO
	add_inst(InstID::JMP_FAR, 0xea, 1, -1, Id, -1, OPT_SMALL_PARAM);
	add_inst(InstID::JMP_FAR, 0xea, 1, -1, I48, -1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JMP		,0xeb	,1	,-1, Jb, -1);
	add_inst(InstID::IN		,0xec	,1	,-1, (int)RegID::AL, (int)RegID::DX);
	add_inst(InstID::IN		,0xed	,1	,-1, (int)RegID::EAX, (int)RegID::DX);
	add_inst(InstID::OUT		,0xee	,1	,-1, (int)RegID::DX, (int)RegID::AL);
	add_inst(InstID::OUT		,0xef	,1	,-1, (int)RegID::DX, (int)RegID::EAX);
	add_inst(InstID::LOCK		,0xf0	,1	,-1	,-1	,-1);
	/*add_inst(inst_repne		,0xf2	,1	,-1	,-1	,-1);
	add_inst(inst_rep		,0xf3	,1	,-1	,-1	,-1);*/
	add_inst(InstID::HLT		,0xf4	,1	,-1	,-1	,-1);
	add_inst(InstID::CMC		,0xf5	,1	,-1	,-1	,-1);
	// Unary Group 3
	add_inst(InstID::TEST		,0xf6	,1	,0	,Eb	,Ib);
	add_inst(InstID::NOT		,0xf6	,1	,2	,Eb	,-1);
	add_inst(InstID::NEG		,0xf6	,1	,3	,Eb	,-1);
	add_inst(InstID::MUL		,0xf6	,1	,4	,(int)RegID::AL	,Eb);
	add_inst(InstID::IMUL		,0xf6	,1	,5	,(int)RegID::AL	,Eb);
	add_inst(InstID::DIV		,0xf6	,1	,6	,(int)RegID::AL	,Eb);
	add_inst(InstID::IDIV		,0xf6	,1	,7	,Eb	,-1);
	add_inst(InstID::TEST,	0xf7,	1,	0,	Ew,	Iw, OPT_SMALL_PARAM);
	add_inst(InstID::TEST,	0xf7,	1,	0,	Ed,	Id, OPT_MEDIUM_PARAM);
	add_inst(InstID::TEST,	0xf7,	1,	0,	Eq,	Id, OPT_BIG_PARAM);
	add_inst(InstID::NOT,	0xf7,	1,	2,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::NOT,	0xf7,	1,	2,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::NOT,	0xf7,	1,	2,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::NEG,	0xf7,	1,	3,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::NEG,	0xf7,	1,	3,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::NEG,	0xf7,	1,	3,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::MUL		,0xf7	,1	,4	,(int)RegID::EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::IMUL		,0xf7	,1	,5	,(int)RegID::EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::DIV		,0xf7	,1	,6	,(int)RegID::EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::IDIV		,0xf7	,1	,7	,(int)RegID::EAX	,Ed, OPT_MEDIUM_PARAM);
	add_inst(InstID::MUL		,0xf7	,1	,4	,(int)RegID::AX	,Ew, OPT_SMALL_PARAM);
	add_inst(InstID::IMUL		,0xf7	,1	,5	,(int)RegID::AX	,Ew, OPT_SMALL_PARAM);
	add_inst(InstID::DIV		,0xf7	,1	,6	,(int)RegID::AX	,Ew, OPT_SMALL_PARAM);
	add_inst(InstID::IDIV		,0xf7	,1	,7	,(int)RegID::AX	,Ew, OPT_SMALL_PARAM);
	add_inst(InstID::MUL		,0xf7	,1	,4	,(int)RegID::RAX	,Eq, OPT_BIG_PARAM);
	add_inst(InstID::IMUL		,0xf7	,1	,5	,(int)RegID::RAX	,Eq, OPT_BIG_PARAM);
	add_inst(InstID::DIV		,0xf7	,1	,6	,(int)RegID::RAX	,Eq, OPT_BIG_PARAM);
	add_inst(InstID::IDIV		,0xf7	,1	,7	,(int)RegID::RAX	,Eq, OPT_BIG_PARAM);
	add_inst(InstID::CLC		,0xf8	,1	,-1	,-1	,-1);
	add_inst(InstID::STC		,0xf9	,1	,-1	,-1	,-1);
	add_inst(InstID::CLI		,0xfa	,1	,-1	,-1	,-1);
	add_inst(InstID::STI		,0xfb	,1	,-1	,-1	,-1);
	add_inst(InstID::CLD		,0xfc	,1	,-1	,-1	,-1);
	add_inst(InstID::STD		,0xfd	,1	,-1	,-1	,-1);
	add_inst(InstID::INC		,0xfe	,1	,0	,Eb	,-1);
	add_inst(InstID::DEC		,0xfe	,1	,1	,Eb	,-1);
	add_inst(InstID::INC,	0xff,	1,	0,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::INC,	0xff,	1,	0,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::INC,	0xff,	1,	0,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::DEC,	0xff,	1,	1,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::DEC,	0xff,	1,	1,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::DEC,	0xff,	1,	1,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::CALL,	0xff,	1,	2,	Ew,	-1, OPT_SMALL_PARAM);
	add_inst(InstID::CALL,	0xff,	1,	2,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::CALL,	0xff,	1,	2,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::CALL_FAR,	0xff,	1,	3,	Ew,	-1, OPT_SMALL_PARAM); // Ep instead of Ev...
	add_inst(InstID::CALL_FAR,	0xff,	1,	3,	Ed,	-1, OPT_MEDIUM_PARAM);
	add_inst(InstID::CALL_FAR,	0xff,	1,	3,	Eq,	-1, OPT_BIG_PARAM);
	add_inst(InstID::JMP, 0xff, 1,	4, Ew, -1, OPT_SMALL_PARAM);
	add_inst(InstID::JMP, 0xff, 1,	4, Ed, -1, OPT_MEDIUM_PARAM);
	add_inst(InstID::JMP, 0xff, 1,	4, Eq, -1, OPT_BIG_PARAM);
	add_inst(InstID::JMP_FAR, 0xff, 1, 5, Ed, -1, OPT_SMALL_PARAM);
	add_inst(InstID::JMP_FAR, 0xff, 1, 5, E48, -1, OPT_MEDIUM_PARAM);
	add_inst(InstID::PUSH, 0xff, 1, 6, Ew, -1, OPT_SMALL_PARAM);
	add_inst(InstID::PUSH, 0xff, 1, 6, Ed, -1, OPT_MEDIUM_PARAM);
	add_inst(InstID::PUSH, 0xff, 1, 6, Eq, -1, OPT_BIG_PARAM);

	// sse
	add_inst(InstID::MOVSS,  0x100ff3, 3, -1, Xx, XMd);
	add_inst(InstID::MOVSS,  0x110ff3, 3, -1, XMd, Xx);
	add_inst(InstID::MOVSD,  0x100ff2, 3, -1, Xx, XMq);
	add_inst(InstID::MOVSD,  0x110ff2, 3, -1, XMq, Xx);
	add_inst(InstID::MOVUPS, 0x100f,   2, -1, Xx, XMdq);
	add_inst(InstID::MOVUPS, 0x110f,   2, -1, XMdq, Xx);
	add_inst(InstID::MOVAPS, 0x280f,   2, -1, Xx, XMdq);
	add_inst(InstID::MOVAPS, 0x290f,   2, -1, XMdq, Xx);
	add_inst(InstID::MOVLPS, 0x120f,   2, -1, Xx, XMq);
	add_inst(InstID::MOVLPS, 0x130f,   2, -1, XMq, Xx);
	add_inst(InstID::MOVHPS, 0x160f,   2, -1, Xx, XMq);
	add_inst(InstID::MOVHPS, 0x170f,   2, -1, XMq, Xx);
	add_inst(InstID::ADDSS,  0x580ff3, 3, -1, Xx, XMd);
	add_inst(InstID::ADDSD,  0x580ff2, 3, -1, Xx, XMq);
	add_inst(InstID::ADDPS,  0x580f,   2, -1, Xx, XMdq);
	add_inst(InstID::SUBSS,  0x5c0ff3, 3, -1, Xx, XMd);
	add_inst(InstID::SUBSD,  0x5c0ff2, 3, -1, Xx, XMq);
	add_inst(InstID::MULSS,  0x590ff3, 3, -1, Xx, XMd);
	add_inst(InstID::MULSD,  0x590ff2, 3, -1, Xx, XMq);
	add_inst(InstID::DIVSS,  0x5e0ff3, 3, -1, Xx, XMd);
	add_inst(InstID::DIVSD,  0x5e0ff2, 3, -1, Xx, XMq);
	add_inst(InstID::SQRTSS, 0x510ff3, 3, -1, Xx, XMd);
	add_inst(InstID::SQRTSD, 0x510ff2, 3, -1, Xx, XMq);
	add_inst(InstID::MINSS,  0x5d0ff3, 3, -1, Xx, XMd);
	add_inst(InstID::MINSD,  0x5d0ff2, 3, -1, Xx, XMq);
	add_inst(InstID::MAXSS,  0x5f0ff3, 3, -1, Xx, XMd);
	add_inst(InstID::MAXSD,  0x5f0ff2, 3, -1, Xx, XMq);
	add_inst(InstID::CVTSS2SD, 0x5a0ff3, 3, -1, Xx, XMd);
	add_inst(InstID::CVTSD2SS, 0x5a0ff2, 3, -1, Xx, XMq);
	add_inst(InstID::CVTTSS2SI, 0x2c0ff3, 3, -1, Rd, XMd);
	add_inst(InstID::CVTTSD2SI, 0x2c0ff2, 3, -1, Rq, XMd);
	add_inst(InstID::CVTSI2SS,  0x2a0ff3, 3, -1, Xx, Ed);
	add_inst(InstID::CVTSI2SD,  0x2a0ff2, 3, -1, Xx, Eq);
	add_inst(InstID::COMISS,    0x2f0f,   2, -1, Xx, XMd);
	add_inst(InstID::COMISD,    0x2f0f66, 3, -1, Xx, XMq);
	add_inst(InstID::UCOMISS,   0x2e0f,   2, -1, Xx, XMd);
	add_inst(InstID::UCOMISD,   0x2e0f66, 3, -1, Xx, XMq);

	add_inst(InstID::WRMSR,     0x300f, 2, -1, -1, -1);
	add_inst(InstID::RDTSC,     0x310f, 2, -1, -1, -1);
	add_inst(InstID::RDMSR,     0x320f, 2, -1, -1, -1);
	add_inst(InstID::RDPMC,     0x330f, 2, -1, -1, -1);
	add_inst(InstID::CPUID,     0xa20f, 2, -1, -1, -1);
	add_inst(InstID::LFENCE,    0xe8ae0f, 3, -1, -1, -1);
	add_inst(InstID::MFENCE,    0xf0ae0f, 3, -1, -1, -1);
	add_inst(InstID::SFENCE,    0xf8ae0f, 3, -1, -1, -1);
	add_inst(InstID::CLFLUSH,   0xae0f, 2, 7, Mb, -1);
	add_inst(InstID::REP,       0xf3, 1, -1, -1, -1);


	if (set == InstructionSet::AMD64) {
		add_inst(InstID::SYSCALL,   0x050f, 2, -1, -1, -1);
		add_inst(InstID::SYSRET,    0x070f, 2, -1, -1, -1);
		add_inst(InstID::SYSENTER,  0x340f, 2, -1, -1, -1);
		add_inst(InstID::SYSEXIT,   0x350f, 2, -1, -1, -1);
	}
}

RegID GetModRMRegister(int reg, int size, RegGroup group) {
	if (group == RegGroup::XMM)
		return RegID((int)RegID::XMM0 + reg);
	if (size == SIZE_8) {
		if (reg == 0x00)	return RegID::AL;
		if (reg == 0x01)	return RegID::CL;
		if (reg == 0x02)	return RegID::DL;
		if (reg == 0x03)	return RegID::BL;
		if (reg == 0x04)	return RegID::AH;
		if (reg == 0x05)	return RegID::CH;
		if (reg == 0x06)	return RegID::DH;
		if (reg == 0x07)	return RegID::BH;
	} else if (size == SIZE_16) {
		if (reg == 0x00)	return RegID::AX;
		if (reg == 0x01)	return RegID::CX;
		if (reg == 0x02)	return RegID::DX;
		if (reg == 0x03)	return RegID::BX;
		if (reg == 0x04)	return RegID::SP;
		if (reg == 0x05)	return RegID::BP;
		if (reg == 0x06)	return RegID::SI;
		if (reg == 0x07)	return RegID::DI;
	} else if (size == SIZE_32) {
		if (reg == 0x00)	return RegID::EAX;
		if (reg == 0x01)	return RegID::ECX;
		if (reg == 0x02)	return RegID::EDX;
		if (reg == 0x03)	return RegID::EBX;
		if (reg == 0x04)	return RegID::ESP;
		if (reg == 0x05)	return RegID::EBP;
		if (reg == 0x06)	return RegID::ESI;
		if (reg == 0x07)	return RegID::EDI;
		if (reg == 0x08)	return RegID::R8D;
		if (reg == 0x09)	return RegID::R9D;
		if (reg == 0x0a)	return RegID::R10D;
		if (reg == 0x0b)	return RegID::R11D;
		if (reg == 0x0c)	return RegID::R12D;
		if (reg == 0x0d)	return RegID::R13D;
		if (reg == 0x0e)	return RegID::R14D;
		if (reg == 0x0f)	return RegID::R15D;
	} else if (size == SIZE_64) {
		if (reg == 0x00)	return RegID::RAX;
		if (reg == 0x01)	return RegID::RCX;
		if (reg == 0x02)	return RegID::RDX;
		if (reg == 0x03)	return RegID::RBX;
		if (reg == 0x04)	return RegID::RSP;
		if (reg == 0x05)	return RegID::RBP;
		if (reg == 0x06)	return RegID::RSI;
		if (reg == 0x07)	return RegID::RDI;
		if (reg == 0x08)	return RegID::R8;
		if (reg == 0x09)	return RegID::R9;
		if (reg == 0x0a)	return RegID::R10;
		if (reg == 0x0b)	return RegID::R11;
		if (reg == 0x0c)	return RegID::R12;
		if (reg == 0x0d)	return RegID::R13;
		if (reg == 0x0e)	return RegID::R14;
		if (reg == 0x0f)	return RegID::R15;
	}
	msg_error("unhandled mod/rm register: " + i2s(reg) + " (size " + i2s(size) + ")");
	return RegID::INVALID;
}

inline void GetFromModRM(InstructionParam &p, InstructionParamFuzzy &pf, unsigned char modrm) {
	if (pf.mrm_mode == ModRM::REG) {
		unsigned char reg = modrm & 0x38; // bits 5, 4, 3
		p.type = ParamType::REGISTER;
		p.deref = false;
		if (pf.reg_group == RegGroup::SEGMENT) {
			if (reg == 0x00)	p.reg = RegisterByID(RegID::ES);
			if (reg == 0x08)	p.reg = RegisterByID(RegID::CS);
			if (reg == 0x10)	p.reg = RegisterByID(RegID::SS);
			if (reg == 0x18)	p.reg = RegisterByID(RegID::DS);
			if (reg == 0x20)	p.reg = RegisterByID(RegID::FS);
			if (reg == 0x28)	p.reg = RegisterByID(RegID::GS);
		} else if (pf.reg_group == RegGroup::CONTROL) {
			if (reg == 0x00)	p.reg = RegisterByID(RegID::CR0);
			if (reg == 0x08)	p.reg = RegisterByID(RegID::CR1);
			if (reg == 0x10)	p.reg = RegisterByID(RegID::RC2);
			if (reg == 0x18)	p.reg = RegisterByID(RegID::CR3);
			if (reg == 0x20)	p.reg = RegisterByID(RegID::CR4);
		} else if (pf.reg_group == RegGroup::XMM) {
			p.reg = register_by_id[(int)RegID::XMM0 + (reg >> 3)];
		} else {
			reg = (reg >> 3) | (state.extend_mod_rm_reg ? 0x08 : 0x00);
			p.reg = RegisterByID(GetModRMRegister(reg, p.size, RegGroup::GENERAL));
		}
	} else if (pf.mrm_mode == ModRM::MOD_RM) {
		unsigned char mod = modrm & 0xc0; // bits 7, 6
		unsigned char rm = modrm & 0x07; // bits 2, 1, 0
		if (state.extend_mod_rm_base)	rm |= 0x08;
		if (mod == 0x00) {
			if (state.addr_size == SIZE_16) {
				p.type = ParamType::REGISTER;
				p.deref = true;
				if (rm == 0x00) {p.reg = RegisterByID(RegID::BX);	p.reg2 = RegisterByID(RegID::SI);	p.disp = DispMode::REG2;	}
				if (rm == 0x01) {p.reg = RegisterByID(RegID::BX);	p.reg2 = RegisterByID(RegID::DI);	p.disp = DispMode::REG2;	}
				if (rm == 0x02) {p.reg = RegisterByID(RegID::BP);	p.reg2 = RegisterByID(RegID::SI);	p.disp = DispMode::REG2;	}
				if (rm == 0x03) {p.reg = RegisterByID(RegID::BP);	p.reg2 = RegisterByID(RegID::DI);	p.disp = DispMode::REG2;	}
				if (rm == 0x04)	p.reg = RegisterByID(RegID::SI);
				if (rm == 0x05)	p.reg = RegisterByID(RegID::DI);
				if (rm == 0x06) {p.reg = nullptr;	p.type = ParamType::IMMEDIATE;	}
				if (rm == 0x07)	p.reg = RegisterByID(RegID::BX);
			} else {
				p.type = ParamType::REGISTER;
				p.deref = true;
				//if (rm == 0x04) {p.reg = NULL;	p.disp = DispModeSIB;	p.type = ParamTImmediate;}//p.type = ParamTInvalid;	Error("kein SIB byte...");}
				if (rm == 0x04) {p.reg = RegisterByID(RegID::EAX);	p.disp = DispMode::SIB;	} // eax = provisoric
				else if (rm == 0x05) {p.reg = nullptr;	p.type = ParamType::IMMEDIATE;	}
				else
					p.reg = RegisterByID(GetModRMRegister(rm, SIZE_32, RegGroup::GENERAL));
			}
		} else if ((mod == 0x40) or (mod == 0x80)) {
			if (state.addr_size == SIZE_16) {
				p.type = ParamType::REGISTER;
				p.deref = true;
				if (rm == 0x00) {p.reg = RegisterByID(RegID::BX);	p.reg2 = RegisterByID(RegID::SI);	p.disp = (mod == 0x40) ? DispMode::_8_REG2 : DispMode::_16_REG2;	}
				if (rm == 0x01) {p.reg = RegisterByID(RegID::BX);	p.reg2 = RegisterByID(RegID::DI);	p.disp = (mod == 0x40) ? DispMode::_8_REG2 : DispMode::_16_REG2;	}
				if (rm == 0x02) {p.reg = RegisterByID(RegID::BP);	p.reg2 = RegisterByID(RegID::SI);	p.disp = (mod == 0x40) ? DispMode::_8_REG2 : DispMode::_16_REG2;	}
				if (rm == 0x03) {p.reg = RegisterByID(RegID::BP);	p.reg2 = RegisterByID(RegID::DI);	p.disp = (mod == 0x40) ? DispMode::_8_REG2 : DispMode::_16_REG2;	}
				if (rm == 0x04) {p.reg = RegisterByID(RegID::SI);	p.disp = (mod == 0x40) ? DispMode::_8 : DispMode::_16;	}
				if (rm == 0x05) {p.reg = RegisterByID(RegID::DI);	p.disp = (mod == 0x40) ? DispMode::_8 : DispMode::_16;	}
				if (rm == 0x06) {p.reg = RegisterByID(RegID::BP);	p.disp = (mod == 0x40) ? DispMode::_8 : DispMode::_16;	}
				if (rm == 0x07) {p.reg = RegisterByID(RegID::BX);	p.disp = (mod == 0x40) ? DispMode::_8 : DispMode::_16;	}
			} else {
				p.type = ParamType::REGISTER;
				p.deref = true;
				p.disp = (mod == 0x40) ? DispMode::_8 : DispMode::_32;
				//if (rm == 0x04) {p.reg = NULL;	p.type = ParamTInvalid;	}
				if (rm == 0x04) {p.reg = RegisterByID(RegID::EAX);	p.disp = DispMode::_8_SIB;	} // eax = provisoric
				else
					p.reg = RegisterByID(GetModRMRegister(rm, SIZE_32, RegGroup::GENERAL));
			}
		} else if (mod == 0xc0) {
			p.type = ParamType::REGISTER;
			p.deref = false;
			if (state.extend_mod_rm_base)	rm |= 0x08;
			p.reg = RegisterByID(GetModRMRegister(rm, p.size, pf.reg_group));
		}
	}
}

inline void TryGetSIB(InstructionParam &p, char *&cur) {
	if ((p.disp == DispMode::SIB) or (p.disp == DispMode::_8_SIB)) {
		bool disp8 = (p.disp == DispMode::_8_SIB);
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
		//if (p.disp == DispModeSIB) {
			if (ss == 0x00) { // scale factor 1
				p.deref = true;
				p.disp = disp8 ? DispMode::_8_REG2 : DispMode::REG2;
				if (base == 0x00)		p.reg = RegisterByID(RegID::EAX);
				else if (base == 0x01)	p.reg = RegisterByID(RegID::ECX);
				else if (base == 0x02)	p.reg = RegisterByID(RegID::EDX);
				else if (base == 0x03)	p.reg = RegisterByID(RegID::EBX);
				else if (base == 0x04)	p.reg = RegisterByID(RegID::ESP);
				else p.disp = DispMode::SIB; // ...
				if (index == 0x00)		p.reg2 = RegisterByID(RegID::EAX);
				else if (index == 0x08)	p.reg2 = RegisterByID(RegID::ECX);
				else if (index == 0x10)	p.reg2 = RegisterByID(RegID::EDX);
				else if (index == 0x18)	p.reg2 = RegisterByID(RegID::EBX);
				else if (index == 0x28)	p.reg2 = RegisterByID(RegID::EBP);
				else if (index == 0x30)	p.reg2 = RegisterByID(RegID::ESI);
				else if (index == 0x38)	p.reg2 = RegisterByID(RegID::EDI);
				else p.disp = disp8 ? DispMode::_8 : DispMode::NONE;
			}
		//}
	}
}


inline void UnfuzzyParam(InstructionParam &p, InstructionParamFuzzy &pf) {
	p.type = pf._type_;
	p.reg2 = nullptr;
	p.disp = DispMode::NONE;
	p.reg = pf.reg;
	if (p.reg and state.extend_mod_rm_base) {
		if (reg_between(p.reg->id, RegID::RAX, RegID::RBP))
			p.reg = register_by_id[(int)p.reg->id + (int)RegID::R8 - (int)RegID::RAX];
	}
	p.size = pf.size;
	p.deref = false; // well... FIXME
	p.value = 0;
	p.is_label = false;
	if (pf._type_ == ParamType::MEMORY) {
		p.type = ParamType::IMMEDIATE;
		p.deref = true;
	}
}

inline void ReadParamData(char *&cur, InstructionParam &p, bool has_modrm) {
	//char *o = cur;
	p.value = 0;
	if (p.type == ParamType::IMMEDIATE) {
		if (p.deref) {
			int size = has_modrm ? state.addr_size : state.full_register_size; // Ov/Mv...
			memcpy(&p.value, cur, size);
			cur += size;
		} else {
			memcpy(&p.value, cur, p.size);
			cur += p.size;
		}
	/*} else if (p.type == ParamTImmediateExt) {
		if (state.ParamSize == Size16) { // addr?
			*(short*)&p.value = *(short*)cur;	cur += 2;	((short*)&p.value)[2] = *(short*)cur;	cur += 2;
		} else {
			memcpy(&p.value, cur, 6);		cur += 6;
		}*/
	} else if (p.type == ParamType::REGISTER) {
		if ((p.disp == DispMode::_8) or (p.disp == DispMode::_8_REG2) or (p.disp == DispMode::_8_SIB)) {
			*(char*)&p.value = *cur;		cur ++;
		} else if (p.disp == DispMode::_16) {
			*(short*)&p.value = *(short*)cur;		cur += 2;
		} else if (p.disp == DispMode::_32) {
			*(int*)&p.value = *(int*)cur;		cur += 4;
		}
	}
	//msg_write((int_p)cur - (int_p)o);
}


string x86_disassemble(void *_code_, int length, bool allow_comments) {
	char *code = (char*)_code_;

	string param;
	char *opcode;
	string bufstr;
	char *end = code + length;
	char *orig = code;
	if (length < 0)
		end = code + 65536;

	// code points to the start of the (current) complete command (dword cs: mov ax, ...)
	// cur points to the currently processed byte
	// opcode points to the start of the instruction (mov)
	char *cur = code;
	state.init();
	state.set_bits(instruction_set.pointer_size);


	while (code < end) {
		state.reset(nullptr);
		opcode = cur;
		code = cur;

		// done?
		if (code >= end)
			break;

		// special info
		if (CurrentMetaInfo) {

			// labels
#if 0
			// TODO
			for (int i=0;i<CurrentMetaInfo->label.num;i++)
				if ((int_p)code - (int_p)orig == CurrentMetaInfo->label[i].pos)
					bufstr += "    " + CurrentMetaInfo->label[i].name + ":\n";
#endif

			// data blocks
			bool inserted = false;
			for (auto &data: CurrentMetaInfo->data) {
				//printf("%d  %d  %d  %d\n", CurrentMetaInfo->data[i].Pos, (int_p)code, (int_p)orig, (int_p)code - (int_p)orig);
				if ((int_p)code - (int_p)orig == data.offset) {
					//msg_write("data");
					if (data.size == SIZE_8) {
						bufstr += "  db\t";
						bufstr += d2h(cur, data.size);
					} else if (data.size == SIZE_16) {
						bufstr += "  dw\t";
						bufstr += d2h(cur, data.size);
					} else if (data.size == SIZE_32) {
						bufstr += "  dd\t";
						bufstr += d2h(cur, data.size);
					} else if (data.size == SIZE_64) {
						bufstr += "  dq\t";
						bufstr += d2h(cur, data.size);
					} else {
						bufstr += "  ds \t...";
					}
					cur += data.size;
					bufstr += "\n";
					inserted = true;
				}
			}
			if (inserted)
				continue;

			// change of bits (processor mode)
			for (auto &bc: CurrentMetaInfo->bit_change)
				if ((int_p)code - (int_p)orig == bc.offset) {
					state.set_bits(bc.bits_size);
					bufstr += format("   bits_%s\n", size_out(bc.bits_size));
				}
		}

		// code
		Register *seg = nullptr;

		// prefix
		while (true) {

			// prefix (size/segment register)
			if (cur[0] == 0x67) {
				state.addr_size = (state.default_addr_size == SIZE_32) ? SIZE_16 : SIZE_32;
				cur ++;
				continue;
			}
			if (cur[0] == 0x66) {
				state.param_size = (state.default_param_size == SIZE_32) ? SIZE_16 : SIZE_32;
				cur ++;
				continue;
			}

			// REX
			if (state.full_register_size == SIZE_64) { // amd64
				if ((cur[0] & 0xf0) == 0x40) {
					if ((cur[0] & 0x08) > 0)
						state.param_size = SIZE_64;
					state.extend_mod_rm_reg = ((cur[0] & 0x04) > 0);
					state.extend_mod_rm_index = ((cur[0] & 0x02) > 0);
					state.extend_mod_rm_base = ((cur[0] & 0x01) > 0);
					cur ++;
					continue;
				}
			}

			// segment registers
			if (cur[0]==0x2e) {      seg = RegisterByID(RegID::CS); cur++; continue; }
			else if (cur[0]==0x36) { seg = RegisterByID(RegID::SS); cur++; continue; }
			else if (cur[0]==0x3e) { seg = RegisterByID(RegID::DS); cur++; continue; }
			else if (cur[0]==0x26) { seg = RegisterByID(RegID::ES); cur++; continue; }
			else if (cur[0]==0x64) { seg = RegisterByID(RegID::FS); cur++; continue; }
			else if (cur[0]==0x65) { seg = RegisterByID(RegID::GS); cur++; continue; }

			break;

		}

		opcode=cur;

		// instruction
		CPUInstruction *inst = nullptr;
		for (CPUInstruction &ci: all_cpu_instructions) {
			if (ci.code_size == 0)
				continue;
			if (!ci.has_fixed_param) {
				if (ci.has_small_param and (state.param_size != SIZE_16))
					continue;
				if (ci.has_medium_param and (state.param_size != SIZE_32))
					continue;
				if (ci.has_big_param and (state.param_size != SIZE_64))
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
			if (ok and ci.has_modrm) {
				InstructionParam p1, p2;
				UnfuzzyParam(p1, ci.param1);
				UnfuzzyParam(p2, ci.param2);

				// modr/m byte
				char modrm = cur[ci.code_size];
				GetFromModRM(p1, ci.param1, modrm);
				GetFromModRM(p2, ci.param2, modrm);
				if ((p1.type == ParamType::REGISTER) and (!p1.deref) and (!ci.param1.allow_register))
					continue;
				if ((p2.type == ParamType::REGISTER) and (!p2.deref) and (!ci.param2.allow_register))
					continue;
			}
			if (ok) {
				inst = &ci;
				cur += inst->code_size;
				break;
			}
		}
		if (inst) {
			InstructionParamFuzzy ip1 = inst->param1;
			InstructionParamFuzzy ip2 = inst->param2;


			InstructionParam p1, p2;
			UnfuzzyParam(p1, ip1);
			UnfuzzyParam(p2, ip2);

			// modr/m byte
			if (inst->has_modrm) {
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
			/*if ((state.param_size != state.default_param_size) and ((p1.type != ParamType::REGISTER) or (p1.deref)) and ((p2.type != ParamType::REGISTER) or p2.deref)) {
				if (state.param_size == SIZE_16)
					str += " word";
				else if (state.param_size == SIZE_32)
					str += " dword";
				else if (state.param_size == SIZE_64)
					str += " qword";
			}*/
			bool hide_size = p2.type != ParamType::NONE;
			if (p1.type != ParamType::NONE)
				str += " " + p1.str(hide_size);
			if (p2.type != ParamType::NONE)
				str += ", " + p2.str(hide_size);


			if (allow_comments) {
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

		} else {
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

int GetModRMReg(Register *r) {
	RegID id = r->id;
	if ((id == RegID::R8)  or (id == RegID::R8D)  or (id == RegID::RAX) or (id == RegID::EAX) or (id == RegID::AX) or (id == RegID::AL))	return 0x00;
	if ((id == RegID::R9)  or (id == RegID::R9D)  or (id == RegID::RCX) or (id == RegID::ECX) or (id == RegID::CX) or (id == RegID::CL))	return 0x01;
	if ((id == RegID::R10) or (id == RegID::R10D) or (id == RegID::RDX) or (id == RegID::EDX) or (id == RegID::DX) or (id == RegID::DL))	return 0x02;
	if ((id == RegID::R11) or (id == RegID::R11D) or (id == RegID::RBX) or (id == RegID::EBX) or (id == RegID::BX) or (id == RegID::BL))	return 0x03;
	if ((id == RegID::R12) or (id == RegID::R12D) or (id == RegID::RSP) or (id == RegID::ESP) or (id == RegID::SP) or (id == RegID::AH))	return 0x04;
	if ((id == RegID::R13) or (id == RegID::R13D) or (id == RegID::RBP) or (id == RegID::EBP) or (id == RegID::BP) or (id == RegID::CH))	return 0x05;
	if ((id == RegID::R14) or (id == RegID::R14D) or (id == RegID::RSI) or (id == RegID::ESI) or (id == RegID::SI) or (id == RegID::DH))	return 0x06;
	if ((id == RegID::R15) or (id == RegID::R15D) or (id == RegID::RDI) or (id == RegID::EDI) or (id == RegID::DI) or (id == RegID::BH))	return 0x07;
	if ((id >= RegID::XMM0) and (id <= RegID::XMM7))	return ((int)id - (int)RegID::XMM0);
	raise_error("GetModRMReg: register not allowed: " + r->name);
	return 0;
}

inline int CreatePartialModRMByte(InstructionParamFuzzy &pf, InstructionParam &p) {
	RegID r = RegID::INVALID;
	if (p.reg)
		r = p.reg->id;
	if (pf.mrm_mode == ModRM::REG) {
		if (r == RegID::ES)	return 0x00;
		if (r == RegID::CS)	return 0x08;
		if (r == RegID::SS)	return 0x10;
		if (r == RegID::DS)	return 0x18;
		if (r == RegID::FS)	return 0x20;
		if (r == RegID::GS)	return 0x28;
		if (r == RegID::CR0)	return 0x00;
		if (r == RegID::CR1)	return 0x08;
		if (r == RegID::RC2)	return 0x10;
		if (r == RegID::CR3)	return 0x18;
		if (r == RegID::CR4)	return 0x20;
		int mrm = GetModRMReg(p.reg) << 3;
		if (p.reg->extend_mod_rm)
			mrm += 0x0400; // REXR
		return mrm;
	} else if (pf.mrm_mode == ModRM::MOD_RM) {
		if (p.deref) {
			if (state.addr_size == SIZE_16) {
				if ((p.type == ParamType::IMMEDIATE) and (p.deref))	return 0x06;
			} else {
				if ((r == RegID::EAX) or (r == RegID::RAX))	return (p.disp == DispMode::NONE) ? 0x00 : ((p.disp == DispMode::_8) ? 0x40 : 0x80); // default = DispMode32
				if ((r == RegID::ECX) or (r == RegID::RCX))	return (p.disp == DispMode::NONE) ? 0x01 : ((p.disp == DispMode::_8) ? 0x41 : 0x81);
				if ((r == RegID::EDX) or (r == RegID::RDX))	return (p.disp == DispMode::NONE) ? 0x02 : ((p.disp == DispMode::_8) ? 0x42 : 0x82);
				if ((r == RegID::EBX) or (r == RegID::RBX))	return (p.disp == DispMode::NONE) ? 0x03 : ((p.disp == DispMode::_8) ? 0x43 : 0x83);
				// sib			return 4;
				// disp32		return 5;
				if ((p.type == ParamType::IMMEDIATE) and (p.deref))	return 0x05;
				if ((r == RegID::EBP) or (r == RegID::RBP))	return (p.disp == DispMode::_8) ? 0x45 : 0x85;
				if ((r == RegID::ESI) or (r == RegID::RSI))	return (p.disp == DispMode::NONE) ? 0x06 : ((p.disp == DispMode::_8) ? 0x46 : 0x86);
				if ((r == RegID::EDI) or (r == RegID::RDI))	return (p.disp == DispMode::NONE) ? 0x07 : ((p.disp == DispMode::_8) ? 0x47 : 0x87);
			}
		} else {
			int mrm = GetModRMReg(p.reg) | 0xc0;
			if (p.reg->extend_mod_rm)
				mrm += 0x0100; // REXB
			return mrm;
		}
	}
	if (pf.mrm_mode != ModRM::NONE)
		raise_error(format("unhandled modrm %d %d %s %d %s", (int)pf.mrm_mode, (int)p.type, (p.reg?p.reg->name:""), p.deref, size_out(pf.size)));
	return 0x00;
}

int CreateModRMByte(CPUInstruction &inst, InstructionParam &p1, InstructionParam &p2) {
	int mrm = CreatePartialModRMByte(inst.param1, p1) | CreatePartialModRMByte(inst.param2, p2);
	if (inst.cap >= 0)
		mrm |= (inst.cap << 3);
	return mrm;
}

inline void append_val(char *oc, int &ocs, int64 val, int size) {
	insert_val(oc, ocs, val, size);
	ocs += size;
}

void OpcodeAddImmideate(char *oc, int &ocs, InstructionParam &p, CPUInstruction &inst, InstructionWithParamsList &list, int next_param_size) {
	int64 value = p.value;
	int size = 0;
	if (p.type == ParamType::IMMEDIATE) {
		size = p.size;
		if (p.deref) {
			//---msg_write("deref....");
			size = state.addr_size; // inst.has_big_addr
			if (instruction_set.set == InstructionSet::AMD64) {
				if (inst.has_modrm) {
					value -= CurrentMetaInfo->code_origin + ocs + size + next_param_size; // amd64 uses RIP-relative addressing!
					if ((value >= 0x80000000) or (-value >= 0x80000000)) {
						//msg_write("-----");
						//inst.print();
						raise_error(format("RIP relative more than 32 bit: %x from %x", p.value, CurrentMetaInfo->code_origin));
					}
				} else {
					size = SIZE_64; // Ov/Mv...
				}
			}
		}
	//} else if (p.type == ParamTImmediateExt) {
	//	size = state.ParamSize;  // bits 0-15  /  0-31
	} else if (p.type == ParamType::REGISTER) {
		if (p.disp == DispMode::_8)	size = SIZE_8;
		if (p.disp == DispMode::_16)	size = SIZE_16;
		if (p.disp == DispMode::_32)	size = SIZE_32;
	} else {
		return;
	}

	bool rel = ((inst.name[0] == 'j') /*and (inst.param1._type_ != ParamTImmediateDouble)*/) or (inst.inst == InstID::CALL) or (inst.name.find("loop") >= 0);
	if (inst.inst == InstID::JMP_FAR)
		rel = false;
	if (p.is_label) {
		if ((instruction_set.set == InstructionSet::AMD64) and (p.deref))
			rel = true;
		list.add_wanted_label(ocs, p.value, list.current_inst, rel, false, size);
	} else if (rel) {
		value -= CurrentMetaInfo->code_origin + ocs + size + next_param_size; // TODO ...first byte of next opcode
	}

	//---msg_write("imm " + i2s(size));
	append_val(oc, ocs, value, size);
}

void OpcodeAddInstruction(char *oc, int &ocs, CPUInstruction &inst, InstructionParam &p1, InstructionParam &p2, InstructionWithParamsList &list) {
	//---msg_write("add inst " + inst.name);

	// 16/32 bit toggle prefix
	if (!inst.has_fixed_param) {
		if (inst.has_small_param and (state.default_param_size != SIZE_16))
			append_val(oc, ocs, 0x66, 1);
		else if (inst.has_medium_param and (state.default_param_size != SIZE_32))
			append_val(oc, ocs, 0x66, 1);
	}

	int mod_rm = 0;
	if (inst.has_modrm)
		mod_rm = CreateModRMByte(inst, p1, p2);

	// REX prefix
	char rex = mod_rm >> 8;
	if (inst.param1.reg and p1.reg)
		if (reg_between(inst.param1.reg->id, RegID::RAX, RegID::RBP) and ((int)inst.param1.reg->id == (int)p1.reg->id + (int)RegID::RAX - (int)RegID::R8))
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
	if (p2.type == ParamType::IMMEDIATE)
		param2_size = p2.size;

	OpcodeAddImmideate(oc, ocs, p1, inst, list, param2_size);
	OpcodeAddImmideate(oc, ocs, p2, inst, list, 0);
}

static void align_opcode(char *oc, int &ocs, int granularity) {
	int mask = granularity - 1;
	if ((ocs & mask) == 0)
		return;
	int ocs_new = (ocs | mask) + 1;
	for (int i=ocs; i<ocs_new; i++)
		oc[i] = 0x90;
	ocs = ocs_new;
}

void InstructionWithParamsList::add_instruction(char *oc, int &ocs, int n) {
	int ocs0 = ocs;
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;
	state.reset(this);

	if (iwp.inst == InstID::ALIGN_OPCODE) {
		align_opcode(oc, ocs, 16);
		return;
	}

	// test if any instruction matches our wishes
	int ninst = -1;
	bool has_mod_rm = false;
	foreachi(CPUInstruction &c, cpu_instructions[(int)iwp.inst], i)
		if (!c.ignore and c.match(iwp)) {
			if ((!c.has_modrm and has_mod_rm) or (ninst < 0)) {
				has_mod_rm = c.has_modrm;
				ninst = i;
			}
		}

/*	// try again with REX prefix?
 // now done automatically...!
	if ((ninst < 0) and (instruction_set.set == InstructionSetAMD64)) {
		state.ParamSize = Size64;

		for (int i=0;i<CPUInstructions.num;i++)
			if (CPUInstructions[i].match(iwp)) {
				if (((!CPUInstructions[i].has_modrm) and (has_mod_rm)) or (ninst < 0)) {
					has_mod_rm = CPUInstructions[i].has_modrm;
					ninst = i;
				}
			}

	}*/

	// none found?
	if (ninst < 0) {
		state.line_no = iwp.line;
		for (int i=0;i<(int)InstID::NUM_INSTRUCTION_NAMES;i++)
			if (instruction_names[i].inst == iwp.inst)
				raise_error("command not compatible with its parameters\n" + iwp.str());
		raise_error(format("instruction unknown: %d", (int)iwp.inst));
	}


	if (DebugAsm)
		cpu_instructions[(int)iwp.inst][ninst].print();

	// compile
	OpcodeAddInstruction(oc, ocs, cpu_instructions[(int)iwp.inst][ninst], iwp.p[0], iwp.p[1], *this);
	iwp.size = ocs - ocs0;

	//msg_write(d2h(&oc[ocs0], ocs - ocs0, false));
}

bool immediate_allowed(InstID inst) {
	for (CPUInstruction &i: cpu_instructions[(int)inst])
		if ((i.param1.allow_immediate) or (i.param2.allow_immediate))
			return true;
	return false;
}

};


