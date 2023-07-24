/*
 * Identifier.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

class string;

namespace kaba {

namespace Identifier {
	extern const string CLASS;
	extern const string STRUCT;
	extern const string INTERFACE;
	extern const string FUNC;
	extern const string MACRO;
	extern const string SUPER;
	extern const string SELF;
	extern const string EXTENDS;
	extern const string IMPLEMENTS;
	extern const string STATIC;
	extern const string NEW;
	extern const string DELETE;
	extern const string SIZEOF;
	extern const string TYPEOF;
	extern const string STR;
	extern const string REPR;
	extern const string LEN;
	extern const string LET;
	extern const string VAR;
	extern const string NAMESPACE;
	extern const string RETURN_VAR;
	extern const string VTABLE_VAR;
	extern const string SHARED_COUNT;
	extern const string ENUM;
	extern const string CONST;
	extern const string MUTABLE;
	extern const string OUT;
	extern const string OVERRIDE;
	extern const string VIRTUAL;
	extern const string EXTERN;
	extern const string SELFREF;
	extern const string REF;
	extern const string WEAK;
	extern const string GIVE;
	extern const string SHARED;
	extern const string OWNED;
	extern const string XFER;
	extern const string ALIAS;
	extern const string RAW_POINTER;
	extern const string TRUST_ME;
	extern const string PURE;
	extern const string NOAUTO;
	extern const string NOFRAME;
	extern const string THROWS;
	extern const string USE;
	extern const string IMPORT;
	extern const string RETURN;
	extern const string RAISE;
	extern const string TRY;
	extern const string EXCEPT;
	extern const string IF;
	extern const string ELSE;
	extern const string WHILE;
	extern const string FOR;
	extern const string IN;
	extern const string AS;
	extern const string BREAK;
	extern const string CONTINUE;
	extern const string PASS;
	extern const string AND;
	extern const string OR;
	extern const string XOR;
	extern const string NOT;
	extern const string IS;
	extern const string ASM;
	extern const string LAMBDA;
	extern const string SORT;
	extern const string FILTER;
	extern const string DYN;
	extern const string RAW_FUNCTION_POINTER;
	namespace Func {
		extern const string INIT;
		extern const string DELETE;
		extern const string DELETE_OVERRIDE;
		extern const string ASSIGN;
		extern const string EQUAL;
		extern const string NOT_EQUAL;
		extern const string SMALLER;
		extern const string GREATER;
		extern const string SMALLER_EQUAL;
		extern const string GREATER_EQUAL;
		extern const string ADD;
		extern const string SUBTRACT;
		extern const string MULTIPLY;
		extern const string DIVIDE;
		extern const string ADD_ASSIGN;
		extern const string SUBTRACT_ASSIGN;
		extern const string MULTIPLY_ASSIGN;
		extern const string DIVIDE_ASSIGN;
		extern const string NEGATIVE;
		extern const string NOT;
		extern const string AND;
		extern const string OR;
		extern const string MODULO;
		extern const string SHIFT_LEFT;
		extern const string SHIFT_RIGHT;
		extern const string BIT_AND;
		extern const string BIT_OR;
		extern const string INCREASE;
		extern const string DECREASE;
		extern const string EXPONENT;
		extern const string MAPS_TO;
		extern const string GET;
		extern const string SET;
		extern const string LENGTH;
		extern const string STR;
		extern const string REPR;
		extern const string SUBARRAY;
		extern const string CALL;
		extern const string SHARED_CLEAR;
		extern const string SHARED_CREATE;
		extern const string OWNED_GIVE;
		extern const string CONTAINS;
		extern const string FORMAT;
		extern const string OPTIONAL_HAS_VALUE;
	}
}

}
