/*
 * Operator.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include "Operator.h"
#include "Identifier.h"

namespace kaba {

//   without type information ("primitive")

PrimitiveOperator PrimitiveOperators[(int)OperatorID::_COUNT_] = {
	{"=",  OperatorID::ASSIGN,        true,  0, IDENTIFIER_FUNC_ASSIGN, 3, false},
	{"+",  OperatorID::ADD,           false, 11, "__add__", 3, false},
	{"-",  OperatorID::SUBTRACT,      false, 11, "__sub__", 3, false},
	{"*",  OperatorID::MULTIPLY,      false, 12, "__mul__", 3, false},
	{"/",  OperatorID::DIVIDE,        false, 12, "__div__", 3, false},
	{"-",  OperatorID::NEGATIVE,      false, 13, "__neg__", 2, false}, // -1 etc
	{"+=", OperatorID::ADDS,          true,  0,  "__iadd__", 3, false},
	{"-=", OperatorID::SUBTRACTS,     true,  0,  "__isub__", 3, false},
	{"*=", OperatorID::MULTIPLYS,     true,  0,  "__imul__", 3, false},
	{"/=", OperatorID::DIVIDES,       true,  0,  "__idiv__", 3, false},
	{"==", OperatorID::EQUAL,         false, 8,  "__eq__", 3, false},
	{"!=", OperatorID::NOTEQUAL,      false, 8,  "__ne__", 3, false},
	{IDENTIFIER_NOT,OperatorID::NEGATE,false, 2,  "__not__", 2, false},
	{"<",  OperatorID::SMALLER,       false, 9,  "__lt__", 3, false},
	{">",  OperatorID::GREATER,       false, 9,  "__gt__", 3, false},
	{"<=", OperatorID::SMALLER_EQUAL, false, 9,  "__le__", 3, false},
	{">=", OperatorID::GREATER_EQUAL, false, 9,  "__ge__", 3, false},
	{IDENTIFIER_AND, OperatorID::AND, false, 4,  "__and__", 3, false},
	{IDENTIFIER_OR,  OperatorID::OR,  false, 3,  "__or__", 3, false},
	{"%",  OperatorID::MODULO,        false, 12, "__mod__", 3, false},
	{"&",  OperatorID::BIT_AND,       false, 7, "__bitand__", 3, false},
	{"|",  OperatorID::BIT_OR,        false, 5, "__bitor__", 3, false},
	{"<<", OperatorID::SHIFT_LEFT,    false, 10, "__lshift__", 3, false},
	{">>", OperatorID::SHIFT_RIGHT,   false, 10, "__rshift__", 3, false},
	{"++", OperatorID::INCREASE,      true,  2, "__inc__", 1, false},
	{"--", OperatorID::DECREASE,      true,  2, "__dec__", 1, false},
	{IDENTIFIER_IS, OperatorID::IS,   false, 2,  "-none-", 3, false},
	{IDENTIFIER_IN, OperatorID::IN,   false, 12, "__contains__", 3, true}, // INVERTED
	{IDENTIFIER_EXTENDS, OperatorID::EXTENDS, false, 2,  "-none-", 3, false},
	{"^",  OperatorID::EXPONENT,      false, 14,  "__exp__", 3, false},
	{",",  OperatorID::COMMA,      false, 1,  "-none-", 3, false},
	{"*",  OperatorID::DEREFERENCE,   false, 15,  "__get__", 2, false},
	{"&",  OperatorID::REFERENCE,     false, 15,  "-none-", 2, false},
	{"[...]",  OperatorID::ARRAY,     false, 16,  "-none-", 3, false},
	{"|>",  OperatorID::FUNCTION_PIPE,false, 0,  "-none-", 3, false},
	{IDENTIFIER_AS, OperatorID::AS,   false, 15,  "-none-", 3, false}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

}
