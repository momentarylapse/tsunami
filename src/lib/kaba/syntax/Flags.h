/*
 * Flags.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */


#pragma once

namespace kaba {

enum class InlineID;
enum class OperatorID;

#ifdef CONST
#undef CONST
#endif
#ifdef PURE
#undef PURE
#endif
#ifdef OUT
#undef OUT
#endif

enum class Flags {
	None = 0,
	Mutable = 1<<1,
	//CALL_BY_VALUE = 1<<0,
	//MEMBER = 1<<2,
	Pure = 1<<4,
	Override = 1<<5,
	RaisesExceptions = 1<<6,
	Static = 1<<7,
	Extern = 1<<8,
	Out = 1<<9,
	Virtual = 1<<10,
	Ref = 1<<11,
	Shared = 1<<12,
	Owned = 1<<13,
	AutoCast = 1<<14,
	Template = 1<<15,
	Unimplemented = 1<<16,
	Noauto = 1 << 17,
	Noframe = 1 << 18,
	Macro = 1 << 19,
	TrustMe = 1 << 20,
	Try = 1 << 21,
	CompileTime = 1 << 22,

	AutoImport = 1<<24,
	FullyParsed = 1<<25,
	ForceCallByValue = 1<<26,
	ReturnInFloatRegisters = 1<<27,

	__InitFillAllParams = 1<<28,
};
Flags operator|(Flags a, Flags b);
bool flags_has(Flags flags, Flags t);
void flags_set(Flags &flags, Flags t);
void flags_clear(Flags &flags, Flags t);
Flags flags_mix(const Array<Flags> &f);


};

