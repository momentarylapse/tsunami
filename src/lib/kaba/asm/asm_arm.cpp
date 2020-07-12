/*
 * asm_arm.cpp
 *
 *  Created on: 09.09.2019
 *      Author: michi
 */

#include "../../base/base.h"
#include "../../file/msg.h"
#include "asm.h"
#include "internal.h"

namespace Asm {



struct CPUInstructionARM {
	int inst;
	int code, filter;
	int p1, p2, p3;
};
Array<CPUInstructionARM> cpu_instructions_arm;



enum {
	AP_NONE,
	AP_REG_0,
	AP_FREG_0_5,
	AP_REG_8,
	AP_REG_12,
	AP_FREG_12_22,
	AP_REG_16,
	AP_REG_16_W21,
	AP_FREG_16_7,
	AP_REG_SET,
	AP_OFFSET24_0,
	AP_IMM12_0,
	AP_SHIFTED12_0,
	AP_DEREF_REG_16_OFFSET,
	AP_SHIFTER_0X12_I25,
	AP_XX_R12_W21_UPI23,
	AP_XX_R12_W21_UPI23_BYTE,
};


void add_inst_arm(int inst, int code, int filter, int param1, int param2 = AP_NONE, int param3 = AP_NONE) {
	CPUInstructionARM i;
	i.inst = inst;
	i.code = code;
	i.filter = filter;
	i.p1 = param1;
	i.p2 = param2;
	i.p3 = param3;

	//i.name = GetInstructionName(inst);
	cpu_instructions_arm.add(i);
}

void arm_init() {
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

	cpu_instructions_arm.clear();

	add_inst_arm(INST_MUL,  0x00000090, 0x0ff00ff0, AP_REG_16, AP_REG_0, AP_REG_8);
	add_inst_arm(INST_MULS, 0x00100090, 0x0ff00ff0, AP_REG_16, AP_REG_0, AP_REG_8);

	// data
	add_inst_arm(INST_AND,  0x00000000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_ANDS, 0x00100000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_XOR,  0x00200000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_XORS, 0x00300000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_SUB,  0x00400000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_SUBS, 0x00500000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_RSB,  0x00600000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_RSBS, 0x00700000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_ADD,  0x00800000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_ADDS, 0x00900000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_ADC,  0x00a00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_ADCS, 0x00b00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_SBC,  0x00c00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_SBCS, 0x00d00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_RSC,  0x00e00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_RSCS, 0x00f00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_TST,  0x01100000, 0x0df00000, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_TEQ,  0x01300000, 0x0df00000, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_CMP,  0x01500000, 0x0df00000, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_CMN,  0x01700000, 0x0df00000, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_OR,   0x01800000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_ORS,  0x01900000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_MOV,  0x01a00000, 0x0df00000, AP_REG_12, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_MOVS, 0x01b00000, 0x0df00000, AP_REG_12, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_BIC,  0x01c00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_BICS, 0x01d00000, 0x0df00000, AP_REG_12, AP_REG_16, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_MVN,  0x01e00000, 0x0df00000, AP_REG_12, AP_SHIFTER_0X12_I25);
	add_inst_arm(INST_MVNS, 0x01f00000, 0x0df00000, AP_REG_12, AP_SHIFTER_0X12_I25);

	// transfer
	add_inst_arm(INST_LDR,  0x04100000, 0x0c500000, AP_REG_12, AP_XX_R12_W21_UPI23);
	add_inst_arm(INST_LDRB, 0x04500000, 0x0c500000, AP_REG_12, AP_XX_R12_W21_UPI23_BYTE);
	add_inst_arm(INST_STR,  0x04000000, 0x0c500000, AP_REG_12, AP_XX_R12_W21_UPI23); // TODO swap parameters...
	add_inst_arm(INST_STRB, 0x04400000, 0x0c500000, AP_REG_12, AP_XX_R12_W21_UPI23_BYTE);

	// transfer multiple
	add_inst_arm(INST_LDMIA,  0x08900000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_STMIA,  0x08800000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_LDMIB,  0x09900000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_STMIB,  0x09800000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_LDMDA,  0x08100000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_STMDA,  0x08000000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_LDMDB,  0x09100000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);
	add_inst_arm(INST_STMDB,  0x09000000, 0x0f900000, AP_REG_16_W21, AP_REG_SET);

	// branch
	add_inst_arm(INST_BLX, 0x01200030, 0x0ff000f0, AP_REG_0);
	add_inst_arm(INST_B,   0x0a000000 ,0x0f000000, AP_OFFSET24_0);
	add_inst_arm(INST_BL,  0x0b000000 ,0x0f000000, AP_OFFSET24_0);


	// -- float --

	add_inst_arm(INST_FMSR,  0x0e000a10, 0x0ff00f7f, AP_FREG_16_7, AP_REG_12);
	add_inst_arm(INST_FMRS,  0x0e100a10, 0x0ff00f7f, AP_REG_12, AP_FREG_16_7);

	add_inst_arm(INST_FLDS,   0x0d100a00, 0x0f300f00, AP_FREG_12_22, AP_DEREF_REG_16_OFFSET);
	add_inst_arm(INST_FSTS,   0x0d000a00, 0x0f300f00, AP_DEREF_REG_16_OFFSET, AP_FREG_12_22);

	add_inst_arm(INST_FMACS,  0x0e000a00, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FNMACS, 0x0e000a40, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FMSCS,  0x0e100a00, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FNMSCS, 0x0e100a40, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FMULS,  0x0e200a00, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FNMULS, 0x0e200a40, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FADDS,  0x0e300a00, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FSUBS,  0x0e300a40, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);
	add_inst_arm(INST_FDIVS,  0x0e400a00, 0x0fb00f50, AP_FREG_12_22, AP_FREG_16_7, AP_FREG_0_5);

	add_inst_arm(INST_FCPYS,  0x0eb00a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FABSS,  0x0eb00ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FNEGS,  0x0eb10a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FSQRTS, 0x0eb10ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FCMPS,   0x0eb40a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FCMPES,  0x0eb40ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FCMPZS,  0x0eb50a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FCMPEZS, 0x0eb50ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);

	add_inst_arm(INST_CVTDS, 0x0eb70ac0, 0x0fff0fd0, AP_FREG_12_22, AP_FREG_0_5);
	//add_inst_arm(INST_CVTSD, 0x0eb70ac0, 0x0fff0fd0, AP_FREG_12_22, AP_FREG_0_5);

	add_inst_arm(INST_FUITOS, 0x0eb80a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FSITOS, 0x0eb80ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);

	add_inst_arm(INST_FTOUIS,  0x0ebc0a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FTOUIZS, 0x0ebc0ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FTOSIS,  0x0ebd0a40, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
	add_inst_arm(INST_FTOSIZS, 0x0ebd0ac0, 0x0fbf0fd0, AP_FREG_12_22, AP_FREG_0_5);
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


InstructionParam param_xxx(int code, bool bb) {
	InstructionParam p;

	int Rn = (code >> 16) & 0xf;
	bool imm = ((code >> 25) & 1);
	bool pre = ((code >> 24) & 1);
	bool up = ((code >> 23) & 1);
	bool ww = ((code >> 21) & 1);
	if (imm){
		msg_write( " --shifted reg--");
	}else{
		if (code & 0xfff)
			p = param_deref_reg_shift(REG_R0 + Rn, up ? (code & 0xfff) : (-(code & 0xfff)), bb ? SIZE_8 : SIZE_32);
		else
			p = param_deref_reg(REG_R0 + Rn, bb ? SIZE_8 : SIZE_32);
	}
	p.write_back = ww;
	return p;
}

InstructionParam disarm_param(int code, int p) {
	if (p == AP_FREG_0_5) {
		int fm = (code & 0x0000000f) << 1;
		if ((code & (1<<5)) != 0)
			fm += 1;
		return param_reg(REG_S0 + fm);
	} else if (p == AP_FREG_12_22) {
		int fd = (code & 0x0000f000) >> 11;
		if ((code & (1<<22)) != 0)
			fd += 1;
		return param_reg(REG_S0 + fd);
	} else if (p == AP_FREG_16_7) {
		int fn = (code & 0x000f0000) >> 15;
		if ((code & (1<<7)) != 0)
			fn += 1;
		return param_reg(REG_S0 + fn);
	} else if (p == AP_REG_0) {
		int fm = (code & 0x0000000f);
		return param_reg(REG_R0 + fm);
	} else if (p == AP_REG_8) {
		int fm = (code & 0x00000f00) >> 8;
		return param_reg(REG_R0 + fm);
	} else if (p == AP_REG_12) {
		int fd = (code & 0x0000f000) >> 12;
		return param_reg(REG_R0 + fd);
	} else if (p == AP_REG_16) {
		int fn = (code & 0x000f0000) >> 16;
		return param_reg(REG_R0 + fn);
	} else if (p == AP_REG_16_W21) {
		int fm = (code & 0x000f0000) >> 16;
		auto p = param_reg(REG_R0 + fm);
		p.write_back = ((code >> 21) & 1);
		return p;
	} else if (p == AP_DEREF_REG_16_OFFSET) {
		int Rn = (code >> 16) & 0xf;
		bool up = ((code >> 23) & 1);
		int offset = (code & 0xff) * 4;
		return param_deref_reg_shift(REG_R0 + Rn, up ? offset : -offset, SIZE_32);
	} else if (p == AP_SHIFTER_0X12_I25) {
		if ((code >> 25) & 1)
			return param_imm(arm_decode_imm(code & 0xfff), SIZE_32);
		return disarm_shift_reg(code & 0xfff);
	} else if (p == AP_OFFSET24_0) {
		return param_imm((code & 0x00ffffff) * 4, SIZE_32);
	} else if (p == AP_XX_R12_W21_UPI23) {
		return param_xxx(code, false);
	} else if (p == AP_XX_R12_W21_UPI23_BYTE) {
		return param_xxx(code, true);
	} else if (p == AP_REG_SET) {
		return param_reg_set(code & 0xffff);
	} else if (p != AP_NONE) {
		msg_error("disasm_param... unhandled " + i2s(p));
	}
	return InstructionParam();
}

InstructionWithParams disarm_general(int code) {
	InstructionWithParams i;
	i.inst = INST_NOP;
	i.p[0] = param_none;
	i.p[1] = param_none;
	i.p[2] = param_none;
	for (auto &ii: cpu_instructions_arm) {
		if ((code & ii.filter) == ii.code) {
			i.inst = ii.inst;
			i.p[0] = disarm_param(code, ii.p1);
			i.p[1] = disarm_param(code, ii.p2);
			i.p[2] = disarm_param(code, ii.p3);
			return i;
		}
	}
	return i;
}

string arm_disassemble(void *_code_,int length,bool allow_comments)
{
	string buf;
	int *code = (int*)_code_;
	for (int ni=0; ni<length/4; ni++){
		int cur = code[ni];

		int x = (cur >> 25) & 0x7;

		buf += i2h(cur, 4);
		buf += "    ";

		InstructionWithParams iwp;
		iwp = disarm_general(cur);
		iwp.condition = (cur >> 28) & 0xf;


		buf += iwp.str() + "\n";
	}
	return buf;
}

int arm_reg_no(Register *r)
{
	if (r)
		if ((r->id >= REG_R0) and (r->id <= REG_R15))
			return r->id - REG_R0;
	raise_error("ARM: invalid register: " + r->name);
	return -1;
}

int arm_freg_no(Register *r)
{
	if (r)
		if ((r->id >= REG_S0) and (r->id <= REG_S31))
			return r->id - REG_S0;
	raise_error("ARM: invalid vfp register: " + r->name);
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
	raise_error("ARM: immediate value not representable: " + i2s(value));
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
			raise_error(format("param #%d expected to be %s: ", i+1, "???") + c.str());
}


void arm_ass_param(InstructionParam &pp, int p, int &code) {
	if (p == AP_FREG_0_5) {
		int fn = arm_freg_no(pp.reg);
		code |= (fn >> 1);
		if ((fn & 0x01) > 0)
			code |= 1 << 5;
	} else if (p == AP_FREG_12_22) {
		int fn = arm_freg_no(pp.reg);
		code |= (fn >> 1) << 12;
		if ((fn & 0x01) > 0)
			code |= 1 << 22;
	} else if (p == AP_FREG_16_7) {
		int fn = arm_freg_no(pp.reg);
		code |= (fn >> 1) << 16;
		if ((fn & 0x01) > 0)
			code |= 1 << 7;
	} else if (p == AP_REG_0) {
		int fn = arm_reg_no(pp.reg);
		code |= (fn & 0xf);
	} else if (p == AP_REG_8) {
		int fn = arm_reg_no(pp.reg);
		code |= (fn & 0xf) << 8;
	} else if (p == AP_REG_12) {
		int fn = arm_reg_no(pp.reg);
		code |= (fn & 0xf) << 12;
	} else if (p == AP_REG_16) {
		int fn = arm_reg_no(pp.reg);
		code |= (fn & 0xf) << 16;
	}
}

bool arm_ass_gen(InstructionWithParams &iwp, int &code) {
	for (auto &i: cpu_instructions_arm)
		if (i.inst == iwp.inst) {
			code |= i.code;
			arm_ass_param(iwp.p[0], i.p1, code);
			arm_ass_param(iwp.p[1], i.p2, code);
			arm_ass_param(iwp.p[2], i.p3, code);
			return true;
		}
	return false;
}


inline bool label_after_now(InstructionWithParamsList *list, int label_no, int now) {
	if (list->label[label_no].inst_no < 0)
		return true;
	return list->label[label_no].inst_no > now;
}

void InstructionWithParamsList::add_instruction_arm(char *oc, int &ocs, int n)
{
	InstructionWithParams &iwp = (*this)[n];
	current_inst = n;
	state.reset(this);

	if (iwp.inst == INST_ALIGN_OPCODE)
		return;

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
				raise_error("p3.disp != DISP_MODE_NONE");
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
			raise_error("unhandled param #3 in " + iwp.str());
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
			raise_error("not allowed to use the same register for destination and addressing: " + iwp.str());

		code |= arm_reg_no(iwp.p[0].reg) << 12; // Rd
		code |= arm_reg_no(iwp.p[1].reg) << 16; // Rn

		if ((iwp.p[1].disp == DISP_MODE_8) or (iwp.p[1].disp == DISP_MODE_32)){
			if ((iwp.p[1].value > 0x0fff) or (iwp.p[1].value < - 0x0fff))
				raise_error("offset larger than 12 bit: " + iwp.str());
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
			value = (iwp.p[0].value - (int_p)&oc[ocs] - 8) >> 2;
		code |= (value & 0x00ffffff);
	}else if (iwp.inst == INST_DD){
		arm_expect(iwp, PARAMT_IMMEDIATE);
		code = iwp.p[0].value;
	}else if ((iwp.inst == INST_FLDS) or (iwp.inst == INST_FSTS)){
		if (iwp.inst == INST_FLDS)
			code |= 0x0d100a00;
		else
			code |= 0x0d000a00;
		if (iwp.inst == INST_FSTS)
			std::swap(iwp.p[0], iwp.p[1]);

		if ((iwp.p[1].type == PARAMT_IMMEDIATE) and (iwp.p[1].deref)){
			if (iwp.p[1].is_label){
				add_wanted_label(ocs + 3, iwp.p[1].value, n, true, true, SIZE_8S2);
				iwp.p[1] = param_deref_reg_shift(REG_R15, label_after_now(this, iwp.p[1].value, n) ? 1 : -1, SIZE_32);
			}else{
				iwp.p[1] = param_deref_reg_shift(REG_R15, iwp.p[1].value - (int_p)&oc[ocs] - 8, SIZE_32);
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
				raise_error("offset larger than 8 bit: " + iwp.str());
			if (v >= 0)
				code |= 0x00800000 | (v & 0x00ff);
			else
				code |= 0x00000000 | ((-v) & 0x00ff);
		}
	} else if (arm_ass_gen(iwp, code)) {
	}else{
		raise_error("cannot assemble instruction: " + iwp.str());
	}

	*(int*)&oc[ocs] = code;
	ocs += 4;
}

};
