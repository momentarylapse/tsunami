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
	None = -1,
	Assign,        //  =
	Add,           //  +
	Subtract,      //  -
	Multiply,      //  *
	Divide,        //  /
	Negative,      //  -
	AddAssign,     // +=
	SubtractAssign,// -=
	MultiplyAssign,// *=
	DivideAssign,  // /=
	Equal,         // ==
	NotEqual,      // !=
	Negate,        //  not
	Smaller,       //  <
	Greater,       //  >
	SmallerEqual,  // <=
	GreaterEqual,  // >=
	And,           // and
	Or,            // or
	Modulo,        //  %
	BitAnd,        //  &
	BitOr,         //  |
	ShiftLeft,     // <<
	ShiftRight,    // >>
	Increase,      // ++
	Decrease,      // --
	Is,            // is
	In,            // in
	Extends,       // extends
	Exponent,      // ^
	Comma,         // ,
	Dereference,   // *
	Reference,     // &
	Index,         // [...]
	PipeRight,     // |>
	As,            // as
	MapsTo,        // =>
	RefAssign,     // :=
	_Count_
};

enum class OperatorFlags {
	None = 0,
	UnaryLeft = 1,
	UnaryRight = 2,
	Binary = 3,
	LeftIsModifiable = 4,
	OrderInverted = 8 // (param, instance) instead of (instance, param)
};

OperatorFlags operator|(OperatorFlags a, OperatorFlags b);
int operator&(OperatorFlags a, OperatorFlags b);

class AbstractOperator {
public:
	string name;
	OperatorID id;
	unsigned char level; // order of operators ("Punkt vor Strich")
	string function_name;
	OperatorFlags flags = OperatorFlags::None;

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
