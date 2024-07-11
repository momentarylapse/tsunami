/*
 * internal.h
 *
 *  Created on: 09.09.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_ASM_INTERNAL_H_
#define SRC_LIB_KABA_ASM_INTERNAL_H_

#include "../../base/base.h"


namespace Asm {




// groups of registers
enum class RegGroup {
	INVALID = -1,
	NONE,
	GENERAL,
	GENERAL2,
	SEGMENT,
	FLAGS,
	CONTROL,
	X87,
	XMM,
	VFP, // ARM-float
};

// parameter types
enum class ParamType {
	IMMEDIATE,
	REGISTER,
	REGISTER_OR_MEM, // ...
	MEMORY,
	REGISTER_SET,
	//SIB,
	NONE,
	INVALID
};

// displacement for registers
enum class DispMode {
	NONE,     // reg
	_8,        // reg + 8bit
	_16,       // reg + 16bit
	_32,       // reg + 32bit
	SIB,      // SIB-byte
	_8_SIB,    // SIB-byte + 8bit
	REG2,     // reg + reg2
	_8_REG2,   // reg + reg2 + 8bit
	_16_REG2   // reg + reg2 + 16bit
};



struct Register {
	string name;
	RegID id;
	RegGroup group;
	int size;
	bool extend_mod_rm;
};
extern Array<Register> registers;
extern Array<Register*> register_by_id;
#define RegisterByID(r)  register_by_id[(int)r]





struct ParserState {
	bool end_of_line;
	bool end_of_code;
	int line_no;
	int column_no;
	int default_param_size;
	int default_addr_size;
	int param_size, addr_size;
	int explicit_size;
	bool extend_mod_rm_base;
	bool extend_mod_rm_reg;
	bool extend_mod_rm_index;
	int full_register_size;
	InstructionWithParamsList *list;
	void init();
	void set_bits(int bits_size);
	void reset(InstructionWithParamsList *_list);
	string get_label(int i);
};
extern ParserState state;


void arm32_init();
void arm64_init();
string arm_disassemble(void *_code_,int length,bool allow_comments);

void x86_init();
string x86_disassemble(void *_code_,int length,bool allow_comments);

void raise_error(const string &str);

void add_reg(const string &name, RegID id, RegGroup group, int size, RegRoot root = RegRoot::NONE);

void insert_val(char *oc, int &ocs, int64 val, int size);
string size_out(int size);

extern bool DebugAsm;

// which part of the modr/m byte is used
enum class ModRM {
	NONE,
	REG,
	MOD_RM
};


// parameter definition (filter for real parameters)
struct InstructionParamFuzzy {
	bool used;
	bool allow_memory_address;	// [0x12.34...]
	bool allow_memory_indirect;	// [eax]    [eax + ...]
	bool allow_immediate;		// 0x12.34...
	bool allow_register;		// eax
	ParamType _type_;			// approximate type.... (UnFuzzy without mod/rm)
	Register *reg;				// if != NULL  -> force a single register
	RegGroup reg_group;
	ModRM mrm_mode;				// which part of the modr/m byte is used?
	int size;
	bool immediate_is_relative;	// for jump


	bool match(InstructionParam &p);
	void print() const;
};



struct InstructionName {
	InstID inst;
	//const string name;
	const char *name;
	int rw1, rw2; // parameter is read(1), modified(2) or both (3)
	// 32 -> don't allow gen reg
	// 64 -> don't allow immediate
};
extern const InstructionName instruction_names[];


bool arm_encode_imm(unsigned int&code, int pf, int64 value, bool already_relative);

// (ARM) parameter encodings
enum {
	AP_NONE,
	AP_REG_0 = 10000, // allow to mix with SIZE_XXX
	AP_REG_0P5,
	AP_WREG_0P5,
	AP_FREG_0_5,
	AP_SREG_0P5,
	AP_DREG_0P5,
	AP_REG_5P5,
	AP_WREG_5P5,
	AP_SREG_5P5,
	AP_DREG_5P5,
	AP_REG_8,
	AP_REG_10P5,
	AP_REG_12,
	AP_FREG_12_22,
	AP_REG_16, // arm32
	AP_REG_16P5, // arm64
	AP_REG_16_W21,
	AP_WREG_16P5,
	AP_SREG_16P5,
	AP_DREG_16P5,
	AP_FREG_16_7,
	AP_REG_SET,
	AP_OFFSET24_0,
	AP_IMM12_0,
	AP_IMM12_10,
	AP_IMM12_10SH,
	AP_IMM9_12,
	AP_IMM16E2_5,
	AP_IMM4_0,
	AP_IMM4_12,
	AP_IMM4_19,
	AP_IMM8F32_13,
	AP_IMM8F64_13,
	AP_IMM26X4REL_0, // relative to rip x4
	AP_IMM14X4REL_5, // relative to rip x4
	AP_IMM19X4REL_5, // ..
	AP_IMM13SRNMASK_10,
	AP_SHIFTED12_0,
	AP_DEREF_REG_16_OFFSET,
	AP_DEREF_S8_REG_5P5_PLUS_IMM12P10, // imm NOT scaled
	AP_DEREF_S32_REG_5P5_PLUS_IMM12P10, // imm scaled x 4
	AP_DEREF_S64_REG_5P5_PLUS_IMM12P10, // imm scaled x 8
	AP_DEREF_S128_REG_5P5_PLUS_IMM7P15, // imm scaled x 16
	AP_DEREF_S32_REG_5P5_PLUS_IMM9P12, // imm NOT scaled!
	AP_DEREF_S64_REG_5P5_PLUS_IMM9P12, // imm NOT scaled!
	AP_SHIFTER_0X12_I25,
	AP_XX_R12_W21_UPI23,
	AP_XX_R12_W21_UPI23_BYTE,
};


}




#endif /* SRC_LIB_KABA_ASM_INTERNAL_H_ */
