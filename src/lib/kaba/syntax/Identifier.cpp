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
	const string CLASS = "class";
	const string STRUCT = "struct";
	const string INTERFACE = "interface";
	const string FUNC = "func";
	const string MACRO = "macro";
	const string SUPER = "super";
	const string SELF = "self";
	const string EXTENDS = "extends";
	const string IMPLEMENTS = "implements";
	const string STATIC = "static";
	const string NEW = "new";
	const string DELETE = "del";
	const string SIZEOF = "sizeof";
	const string TYPEOF = "typeof";
	const string STR = "str";
	const string REPR = "repr";
	const string LEN = "len";
	const string LET = "let";
	const string VAR = "var";
	const string NAMESPACE = "namespace";
	const string RETURN_VAR = "-return-";
	const string VTABLE_VAR = "-vtable-";
	const string SHARED_COUNT = "_shared_ref_count";
	const string ENUM = "enum";
	const string CONST = "const";
	const string MUTABLE = "mut";
	const string OUT = "out";
	const string OVERRIDE = "override";
	const string VIRTUAL = "virtual";
	const string EXTERN = "extern";
	//const string ACCESSOR = "accessor";
	const string SELFREF = "selfref";
	const string REF = "ref";
	const string WEAK = "weak";
	const string GIVE = "give";
	const string SHARED = "shared";
	const string OWNED = "owned";
	const string XFER = "xfer";
	const string PURE = "pure";
	const string NOAUTO = "@noauto";
	const string NOFRAME = "@noframe";
	const string THROWS = "throws";
	const string USE = "use";
	const string IMPORT = "import";
	const string RETURN = "return";
	const string RAISE = "raise";
	const string TRY = "try";
	const string EXCEPT = "except";
	const string IF = "if";
	const string ELSE = "else";
	const string WHILE = "while";
	const string FOR = "for";
	const string IN = "in";
	const string AS = "as";
	const string BREAK = "break";
	const string CONTINUE = "continue";
	const string PASS = "pass";
	const string AND = "and";
	const string OR = "or";
	const string XOR = "xor";
	const string NOT = "not";
	const string IS = "is";
	const string ASM = "asm";
	const string LAMBDA = "lambda";
	const string SORT = "sort";
	const string FILTER = "filter";
	const string DYN = "dyn";
	const string RAW_FUNCTION_POINTER = "raw_function_pointer";
	namespace Func {
		const string INIT = "__init__";
		const string DELETE = "__delete__";
		const string DELETE_OVERRIDE = "__del_override__";
		const string ASSIGN = "__assign__";
		const string EQUAL = "__eq__";
		const string NOT_EQUAL = "__neq__";
		const string SMALLER = "__lt__";
		const string GREATER = "__gt__";
		const string SMALLER_EQUAL = "__le__";
		const string GREATER_EQUAL = "__ge__";
		const string ADD = "__add__";
		const string SUBTRACT = "__sub__";
		const string MULTIPLY = "__mul__";
		const string DIVIDE = "__div__";
		const string ADD_ASSIGN = "__iadd__";
		const string SUBTRACT_ASSIGN = "__isub__";
		const string MULTIPLY_ASSIGN = "__imul__";
		const string DIVIDE_ASSIGN = "__idiv__";
		const string NEGATIVE = "__neg__";
		const string NOT = "__not__";
		const string AND = "__and__";
		const string OR = "__or__";
		const string MODULO = "__mod__";
		const string SHIFT_LEFT = "__lshift__";
		const string SHIFT_RIGHT = "__rshift__";
		const string BIT_AND = "__bitand__";
		const string BIT_OR = "__bitor__";
		const string INCREASE = "__inc__";
		const string DECREASE = "__dec__";
		const string EXPONENT = "__exp__";
		const string MAPS_TO = "__mapsto__";
		const string GET = "__get__";
		const string SET = "__set__";
		const string LENGTH = "__length__";
		const string STR = "__str__";
		const string REPR = "__repr__";
		const string SUBARRAY = "__subarray__";
		const string CALL = "call";
		const string SHARED_CLEAR = "_clear";
		const string SHARED_CREATE = "_create";
		const string OWNED_GIVE = "_give";
		const string CONTAINS = "__contains__";
		const string FORMAT = "format";
		const string OPTIONAL_HAS_VALUE = "has_value";
	}
}


}



