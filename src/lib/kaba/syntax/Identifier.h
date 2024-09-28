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
	extern const string Class;
	extern const string Struct;
	extern const string Interface;
	extern const string Func;
	extern const string Macro;
	extern const string Super;
	extern const string Self;
	extern const string SelfClass;
	extern const string ReturnClass;
	extern const string Extends;
	extern const string Implements;
	extern const string Static;
	extern const string New;
	extern const string Delete;
	extern const string Sizeof;
	extern const string Typeof;
	extern const string Str;
	extern const string Repr;
	extern const string Len;
	extern const string Let;
	extern const string Var;
	extern const string Namespace;
	extern const string ReturnVar;
	extern const string VtableVar;
	extern const string SharedCount;
	extern const string Enum;
	extern const string Const;
	extern const string Mutable;
	extern const string Out;
	extern const string Override;
	extern const string Virtual;
	extern const string Extern;
	extern const string Selfref;
	extern const string Ref;
	extern const string Weak;
	extern const string Give;
	extern const string Shared;
	extern const string Owned;
	extern const string Xfer;
	extern const string Alias;
	extern const string RawPointer;
	extern const string Future;
	extern const string Promise;
	extern const string TrustMe;
	extern const string Pure;
	extern const string Noauto;
	extern const string Noframe;
	extern const string Throws;
	extern const string Compiletime;
	extern const string Use;
	extern const string Import;
	extern const string Return;
	extern const string Raise;
	extern const string Try;
	extern const string Except;
	extern const string If;
	extern const string Else;
	extern const string While;
	extern const string For;
	extern const string In;
	extern const string As;
	extern const string Match;
	extern const string Break;
	extern const string Continue;
	extern const string Pass;
	extern const string And;
	extern const string Or;
	extern const string Xor;
	extern const string Not;
	extern const string Is;
	extern const string Asm;
	extern const string Lambda;
	extern const string Sort;
	extern const string Filter;
	extern const string Dyn;
	extern const string RawFunctionPointer;
	namespace func {
		extern const string Init;
		extern const string Delete;
		extern const string DeleteOverride;
		extern const string Assign;
		extern const string Equal;
		extern const string NotEqual;
		extern const string Smaller;
		extern const string Greater;
		extern const string SmallerEqual;
		extern const string GreaterEqual;
		extern const string Add;
		extern const string Subtract;
		extern const string Multiply;
		extern const string Divide;
		extern const string AddAssign;
		extern const string SubtractAssign;
		extern const string MultiplyAssign;
		extern const string DivideAssign;
		extern const string Negative;
		extern const string Not;
		extern const string And;
		extern const string Or;
		extern const string Modulo;
		extern const string ShiftLeft;
		extern const string ShiftRight;
		extern const string BitAnd;
		extern const string BitOr;
		extern const string Increase;
		extern const string Decrease;
		extern const string Exponent;
		extern const string MapsTo;
		extern const string Get;
		extern const string Set;
		extern const string Length;
		extern const string Str;
		extern const string Repr;
		extern const string Subarray;
		extern const string Call;
		extern const string SharedClear;
		extern const string SharedCreate;
		extern const string OwnedGive;
		extern const string Contains;
		extern const string Format;
		extern const string OptionalHasValue;
	}
}

}
