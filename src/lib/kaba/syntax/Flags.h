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
	CALL_BY_VALUE = 1<<0,
	//MEMBER = 1<<2,
	CONST = 1<<3,
	PURE_XXX = 1<<4,
	PURE = CONST | PURE_XXX,
	OVERRIDE = 1<<5,
	RAISES_EXCEPTIONS = 1<<6,
	STATIC = 1<<7,
	EXTERN = 1<<8,
	OUT = 1<<9,
	VIRTUAL = 1<<10,
	SELFREF = 1<<11,
	SHARED = 1<<12,
	OWNED = 1<<13,
	AUTO_CAST = 1<<14,
	TEMPLATE = 1<<15,
	NEEDS_OVERRIDE = 1<<16,

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

