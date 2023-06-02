/*
 * Operator.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"
#include "../../base/pointer.h"


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
	NOT_EQUAL,     // !=
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
	MAPS_TO,       // =>
	REF_ASSIGN,    // :=
	_COUNT_
};

enum class OperatorFlags {
	NONE = 0,
	UNARY_LEFT = 1,
	UNARY_RIGHT = 2,
	BINARY = 3,
	LEFT_IS_MODIFIABLE = 4,
	ORDER_INVERTED = 8 // (param, instance) instead of (instance, param)
};

OperatorFlags operator|(OperatorFlags a, OperatorFlags b);
int operator&(OperatorFlags a, OperatorFlags b);

class AbstractOperator {
public:
	string name;
	OperatorID id;
	unsigned char level; // order of operators ("Punkt vor Strich")
	string function_name;
	OperatorFlags flags = OperatorFlags::NONE;

	bool is_binary() const;
};
extern AbstractOperator abstract_operators[];



class Operator : public Sharable<base::Empty> {
public:
	AbstractOperator *abstract;
	const Class *return_type, *param_type_1, *param_type_2;

	SyntaxTree *owner;
	Function *f;

	string sig(const Class *ns) const;
};


}
