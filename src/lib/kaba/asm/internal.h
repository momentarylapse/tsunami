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
enum {
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

// parameter types
enum {
	PARAMT_IMMEDIATE,
	PARAMT_REGISTER,
	PARAMT_REGISTER_OR_MEM, // ...
	PARAMT_MEMORY,
	PARAMT_REGISTER_SET,
	//PARAMT_SIB,
	PARAMT_NONE,
	PARAMT_INVALID
};

// displacement for registers
enum {
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



struct Register {
	string name;
	int id, group, size;
	bool extend_mod_rm;
};
extern Array<Register> Registers;
extern Array<Register*> RegisterByID;





struct ParserState {
	bool end_of_line;
	bool end_of_code;
	int line_no;
	int column_no;
	int default_size;
	int param_size, addr_size;
	bool extend_mod_rm_base;
	bool extend_mod_rm_reg;
	bool extend_mod_rm_index;
	int full_register_size;
	InstructionWithParamsList *list;
	void init();
	void reset(InstructionWithParamsList *_list);
	string get_label(int i);
};
extern ParserState state;


void arm_init();
string arm_disassemble(void *_code_,int length,bool allow_comments);

void x86_init();
string x86_disassemble(void *_code_,int length,bool allow_comments);

void raise_error(const string &str);

void add_reg(const string &name, int id, int group, int size, int root = -1);

void insert_val(char *oc, int &ocs, int64 val, int size);
string SizeOut(int size);

extern bool DebugAsm;

// which part of the modr/m byte is used
enum {
	MRM_NONE,
	MRM_REG,
	MRM_MOD_RM
};


// parameter definition (filter for real parameters)
struct InstructionParamFuzzy {
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



struct InstructionName {
	int inst;
	//const string name;
	const char *name;
	int rw1, rw2; // parameter is read(1), modified(2) or both (3)
	// 32 -> don't allow gen reg
	// 64 -> don't allow immediate
};
extern const InstructionName InstructionNames[];



}




#endif /* SRC_LIB_KABA_ASM_INTERNAL_H_ */
