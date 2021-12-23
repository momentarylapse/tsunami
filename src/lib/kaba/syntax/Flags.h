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
	NONE = 0,
	CALL_BY_VALUE = 0x1,
	//CLASS = 0x4,
	CONST = 0x8,
	PURE_XXX = 0x10,
	PURE = CONST | PURE_XXX,
	OVERRIDE = 0x20,
	RAISES_EXCEPTIONS = 0x40,
	STATIC = 0x80,
	EXTERN = 0x100,
	OUT = 0x200,
	VIRTUAL = 0x400,
	SELFREF = 0x800,
	SHARED = 0x1000,
	OWNED = 0x2000,
	AUTO_CAST = 0x4000,

	AUTO_IMPORT = 1<<24,
	FULLY_PARSED = 1<<25,
	FORCE_CALL_BY_VALUE = 1<<26,
	AMD64_ALLOW_PASS_IN_XMM = 1<<27,

	__INIT_FILL_ALL_PARAMS = 1<<28,

	_STATIC__RAISES_EXCEPTIONS = STATIC | RAISES_EXCEPTIONS,
	_STATIC__PURE = STATIC | PURE,
	_SELFREF__RAISES_EXCEPTIONS = SELFREF | RAISES_EXCEPTIONS
};
bool flags_has(Flags flags, Flags t);
void flags_set(Flags &flags, Flags t);
void flags_clear(Flags &flags, Flags t);
Flags flags_mix(const Array<Flags> &f);


};

