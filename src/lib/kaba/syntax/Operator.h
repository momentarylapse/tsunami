/*
 * Operator.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"


namespace kaba {

class SyntaxTree;
class Function;
class Class;


#ifdef IN
#undef IN
#endif

//--------------------------------------------------------------------------------------------------
// operators
enum class OperatorID {
	NONE = -1,
	ASSIGN,        //  =
	ADD,           //  +
	SUBTRACT,      //  -
	MULTIPLY,      //  *
	DIVIDE,        //  /
	NEGATIVE,      //  -
	ADDS,          // +=
	SUBTRACTS,     // -=
	MULTIPLYS,     // *=
	DIVIDES,       // /=
	EQUAL,         // ==
	NOTEQUAL,      // !=
	NEGATE,        //  not
	SMALLER,       //  <
	GREATER,       //  >
	SMALLER_EQUAL, // <=
	GREATER_EQUAL, // >=
	AND,           // and
	OR,            // or
	MODULO,        //  %
	BIT_AND,       //  &
	BIT_OR,        //  |
	SHIFT_LEFT,    // <<
	SHIFT_RIGHT,   // >>
	INCREASE,      // ++
	DECREASE,      // --
	IS,            // is
	IN,            // in
	EXTENDS,       // extends
	EXPONENT,      // ^
	COMMA,         // ,
	DEREFERENCE,   // *
	REFERENCE,     // &
	ARRAY,         // [...]
	FUNCTION_PIPE, // |>
	AS,            // as
	_COUNT_
};

class PrimitiveOperator {
public:
	string name;
	OperatorID id;
	bool left_modifiable;
	unsigned char level; // order of operators ("Punkt vor Strich")
	string function_name;
	int param_flags; // 1 = only left, 2 = only right, 3 = both
	bool order_inverted; // (param, instance) instead of (instance, param)
};
extern PrimitiveOperator PrimitiveOperators[];



class Operator {
public:
	PrimitiveOperator *primitive;
	const Class *return_type, *param_type_1, *param_type_2;

	SyntaxTree *owner;
	Function *f;

	string sig(const Class *ns) const;
};


}
