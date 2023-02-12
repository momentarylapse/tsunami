/*
 * Operator.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include "Operator.h"
#include "Identifier.h"
#include "Class.h"

namespace kaba {

//   without type information ("abstract")
AbstractOperator abstract_operators[(int)OperatorID::_COUNT_] = {
	{"=",  OperatorID::ASSIGN,        true,   0, Identifier::Func::ASSIGN, 3, false},
	{"+",  OperatorID::ADD,           false, 11, Identifier::Func::ADD, 3, false},
	{"-",  OperatorID::SUBTRACT,      false, 11, Identifier::Func::SUBTRACT, 3, false},
	{"*",  OperatorID::MULTIPLY,      false, 12, Identifier::Func::MULTIPLY, 3, false},
	{"/",  OperatorID::DIVIDE,        false, 12, Identifier::Func::DIVIDE, 3, false},
	{"-",  OperatorID::NEGATIVE,      false, 13, Identifier::Func::NEGATIVE, 2, false}, // -1 etc
	{"+=", OperatorID::ADDS,          true,   0,  Identifier::Func::ADD_ASSIGN, 3, false},
	{"-=", OperatorID::SUBTRACTS,     true,   0,  Identifier::Func::SUBTRACT_ASSIGN, 3, false},
	{"*=", OperatorID::MULTIPLYS,     true,   0,  Identifier::Func::MULTIPLY_ASSIGN, 3, false},
	{"/=", OperatorID::DIVIDES,       true,   0,  Identifier::Func::DIVIDE_ASSIGN, 3, false},
	{"==", OperatorID::EQUAL,         false,  8,  Identifier::Func::EQUAL, 3, false},
	{"!=", OperatorID::NOT_EQUAL,      false,  8,  Identifier::Func::NOT_EQUAL, 3, false},
	{Identifier::NOT,OperatorID::NEGATE,false, 2,  Identifier::Func::NOT, 2, false},
	{"<",  OperatorID::SMALLER,       false,  9,  Identifier::Func::SMALLER, 3, false},
	{">",  OperatorID::GREATER,       false,  9,  Identifier::Func::GREATER, 3, false},
	{"<=", OperatorID::SMALLER_EQUAL, false,  9,  Identifier::Func::SMALLER_EQUAL, 3, false},
	{">=", OperatorID::GREATER_EQUAL, false,  9,  Identifier::Func::GREATER_EQUAL, 3, false},
	{Identifier::AND, OperatorID::AND,false,  4,  Identifier::Func::AND, 3, false},
	{Identifier::OR,  OperatorID::OR, false,  3,  Identifier::Func::OR, 3, false},
	{"%",  OperatorID::MODULO,        false, 12, Identifier::Func::MODULO, 3, false},
	{"&",  OperatorID::BIT_AND,       false,  7, Identifier::Func::BIT_AND, 3, false},
	{"|",  OperatorID::BIT_OR,        false,  5, Identifier::Func::BIT_OR, 3, false},
	{"<<", OperatorID::SHIFT_LEFT,    false, 10, Identifier::Func::SHIFT_LEFT, 3, false},
	{">>", OperatorID::SHIFT_RIGHT,   false, 10, Identifier::Func::SHIFT_RIGHT, 3, false},
	{"++", OperatorID::INCREASE,      true,   2, Identifier::Func::INCREASE, 1, false},
	{"--", OperatorID::DECREASE,      true,   2, Identifier::Func::DECREASE, 1, false},
	{Identifier::IS, OperatorID::IS,  false,  2,  "-none-", 3, false},
	{Identifier::IN, OperatorID::IN,  false, 12, Identifier::Func::CONTAINS, 3, true}, // INVERTED
	{Identifier::EXTENDS, OperatorID::EXTENDS, false, 2,  "-none-", 3, false},
	{"^",  OperatorID::EXPONENT,      false, 14,  Identifier::Func::EXPONENT, 3, false},
	{",",  OperatorID::COMMA,         false,  0,  "-none-", 3, false},
	{"*",  OperatorID::DEREFERENCE,   false, 15,  Identifier::Func::GET, 2, false},
	{"&",  OperatorID::REFERENCE,     false, 15,  "-none-", 2, false},
	{"[...]",  OperatorID::ARRAY,     false, 16,  "-none-", 3, false},
	{"|>",  OperatorID::FUNCTION_PIPE,false,  1,  "-none-", 3, false},
	{Identifier::AS, OperatorID::AS,  false, 15,  "-none-", 3, false},
	{"=>",  OperatorID::MAPS_TO,      false,  1,  Identifier::Func::MAPS_TO, 3, false}
};


string Operator::sig(const Class *ns) const {
	if (param_type_1 and param_type_2)
		return format("(%s) %s (%s)", param_type_1->cname(ns), abstract->name, param_type_2->cname(ns));
	if (param_type_1)
		return format("(%s) %s", param_type_1->cname(ns), abstract->name);
	return format("%s (%s)", abstract->name, param_type_2->cname(ns));
}

}
