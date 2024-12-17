/*
 * Identifier.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include "Identifier.h"
#include "../../base/base.h"

namespace kaba {

namespace Identifier {
	const string Class = "class";
	const string Struct = "struct";
	const string Interface = "interface";
	const string Func = "func";
	const string Macro = "macro";
	const string Super = "super";
	const string Self = "self";
	const string SelfClass = "Self";
	const string ReturnClass = "Ret";
	const string Extends = "extends";
	const string Implements = "implements";
	const string Static = "static";
	const string New = "new";
	const string Delete = "del";
	const string Sizeof = "sizeof";
	const string Typeof = "typeof";
	const string Str = "str";
	const string Repr = "repr";
	const string Len = "len";
	const string Let = "let";
	const string Var = "var";
	const string Namespace = "namespace";
	const string ReturnVar = "-return-";
	const string VtableVar = "-vtable-";
	const string SharedCount = "_shared_ref_count";
	const string Enum = "enum";
	const string Const = "const";
	const string Mutable = "mut";
	const string Out = "out";
	const string Override = "override";
	const string Virtual = "virtual";
	const string Extern = "extern";
	//const string ACCESSOR = "accessor";
	const string Selfref = "selfref";
	const string Ref = "ref";
	const string Weak = "weak";
	const string Give = "give";
	const string Shared = "shared";
	const string Owned = "owned";
	const string Xfer = "xfer";
	const string Alias = "@alias";
	const string RawPointer = "ptr";
	const string Future = "future";
	const string Promise = "promise";
	const string TrustMe = "trust_me";
	const string Pure = "pure";
	const string Noauto = "@noauto";
	const string Noframe = "@noframe";
	const string Throws = "throws";
	const string Compiletime = "@compiletime";
	const string Use = "use";
	const string Import = "import";
	const string Return = "return";
	const string Raise = "raise";
	const string Try = "try";
	const string Except = "except";
	const string Assert = "assert";
	const string If = "if";
	const string Else = "else";
	const string While = "while";
	const string For = "for";
	const string In = "in";
	const string As = "as";
	const string Match = "match";
	const string Break = "break";
	const string Continue = "continue";
	const string Pass = "pass";
	const string And = "and";
	const string Or = "or";
	const string Xor = "xor";
	const string Not = "not";
	const string Is = "is";
	const string Asm = "asm";
	const string Lambda = "lambda";
	const string Sort = "sort";
	const string Filter = "filter";
	const string Dyn = "dyn";
	const string RawFunctionPointer = "raw_function_pointer";
	namespace func {
		const string Init = "__init__";
		const string Delete = "__delete__";
		const string DeleteOverride = "__del_override__";
		const string Assign = "__assign__";
		const string Equal = "__eq__";
		const string NotEqual = "__neq__";
		const string Smaller = "__lt__";
		const string Greater = "__gt__";
		const string SmallerEqual = "__le__";
		const string GreaterEqual = "__ge__";
		const string Add = "__add__";
		const string Subtract = "__sub__";
		const string Multiply = "__mul__";
		const string Divide = "__div__";
		const string AddAssign = "__iadd__";
		const string SubtractAssign = "__isub__";
		const string MultiplyAssign = "__imul__";
		const string DivideAssign = "__idiv__";
		const string Negative = "__neg__";
		const string Not = "__not__";
		const string And = "__and__";
		const string Or = "__or__";
		const string Modulo = "__mod__";
		const string ShiftLeft = "__lshift__";
		const string ShiftRight = "__rshift__";
		const string BitAnd = "__bitand__";
		const string BitOr = "__bitor__";
		const string Increase = "__inc__";
		const string Decrease = "__dec__";
		const string Exponent = "__exp__";
		const string MapsTo = "__mapsto__";
		const string Get = "__get__";
		const string Set = "__set__";
		const string Length = "__length__";
		const string Str = "__str__";
		const string Repr = "__repr__";
		const string Subarray = "__subarray__";
		const string Call = "call";
		const string SharedClear = "_clear";
		const string SharedCreate = "_create";
		const string OwnedGive = "_give";
		const string Contains = "__contains__";
		const string Format = "format";
		const string OptionalHasValue = "has_value";
	}
}


}



