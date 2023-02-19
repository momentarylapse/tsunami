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
	{"=",  OperatorID::ASSIGN,        0, Identifier::Func::ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"+",  OperatorID::ADD,           11, Identifier::Func::ADD, OperatorFlags::BINARY},
	{"-",  OperatorID::SUBTRACT,      11, Identifier::Func::SUBTRACT, OperatorFlags::BINARY},
	{"*",  OperatorID::MULTIPLY,      12, Identifier::Func::MULTIPLY, OperatorFlags::BINARY},
	{"/",  OperatorID::DIVIDE,        12, Identifier::Func::DIVIDE, OperatorFlags::BINARY},
	{"-",  OperatorID::NEGATIVE,      13, Identifier::Func::NEGATIVE, OperatorFlags::UNARY_RIGHT}, // -1 etc
	{"+=", OperatorID::ADDS,          0,  Identifier::Func::ADD_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"-=", OperatorID::SUBTRACTS,     0,  Identifier::Func::SUBTRACT_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"*=", OperatorID::MULTIPLYS,     0,  Identifier::Func::MULTIPLY_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"/=", OperatorID::DIVIDES,       0,  Identifier::Func::DIVIDE_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"==", OperatorID::EQUAL,         8,  Identifier::Func::EQUAL, OperatorFlags::BINARY},
	{"!=", OperatorID::NOT_EQUAL,     8,  Identifier::Func::NOT_EQUAL, OperatorFlags::BINARY},
	{Identifier::NOT,OperatorID::NEGATE,2,  Identifier::Func::NOT, OperatorFlags::UNARY_RIGHT},
	{"<",  OperatorID::SMALLER,       9,  Identifier::Func::SMALLER, OperatorFlags::BINARY},
	{">",  OperatorID::GREATER,       9,  Identifier::Func::GREATER, OperatorFlags::BINARY},
	{"<=", OperatorID::SMALLER_EQUAL, 9,  Identifier::Func::SMALLER_EQUAL, OperatorFlags::BINARY},
	{">=", OperatorID::GREATER_EQUAL, 9,  Identifier::Func::GREATER_EQUAL, OperatorFlags::BINARY},
	{Identifier::AND, OperatorID::AND,4,  Identifier::Func::AND, OperatorFlags::BINARY},
	{Identifier::OR,  OperatorID::OR, 3,  Identifier::Func::OR, OperatorFlags::BINARY},
	{"%",  OperatorID::MODULO,        12, Identifier::Func::MODULO, OperatorFlags::BINARY},
	{"&",  OperatorID::BIT_AND,       7, Identifier::Func::BIT_AND, OperatorFlags::BINARY},
	{"|",  OperatorID::BIT_OR,        5, Identifier::Func::BIT_OR, OperatorFlags::BINARY},
	{"<<", OperatorID::SHIFT_LEFT,    10, Identifier::Func::SHIFT_LEFT, OperatorFlags::BINARY},
	{">>", OperatorID::SHIFT_RIGHT,   10, Identifier::Func::SHIFT_RIGHT, OperatorFlags::BINARY},
	{"++", OperatorID::INCREASE,      2, Identifier::Func::INCREASE, OperatorFlags::UNARY_LEFT | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"--", OperatorID::DECREASE,      2, Identifier::Func::DECREASE, OperatorFlags::UNARY_LEFT | OperatorFlags::LEFT_IS_MODIFIABLE},
	{Identifier::IS, OperatorID::IS,  2,  "-none-", OperatorFlags::BINARY},
	{Identifier::IN, OperatorID::IN,  12, Identifier::Func::CONTAINS, OperatorFlags::BINARY | OperatorFlags::ORDER_INVERTED}, // INVERTED
	{Identifier::EXTENDS, OperatorID::EXTENDS, 2,  "-none-", OperatorFlags::BINARY},
	{"^",  OperatorID::EXPONENT,      14,  Identifier::Func::EXPONENT, OperatorFlags::BINARY},
	{",",  OperatorID::COMMA,         0,  "-none-", OperatorFlags::BINARY},
	{"*",  OperatorID::DEREFERENCE,   15,  Identifier::Func::GET, OperatorFlags::UNARY_RIGHT},
	{"&",  OperatorID::REFERENCE,     15,  "-none-", OperatorFlags::UNARY_RIGHT},
	{"[...]",  OperatorID::ARRAY,     16,  "-none-", OperatorFlags::BINARY},
	{"|>",  OperatorID::FUNCTION_PIPE,1,  "-none-", OperatorFlags::BINARY},
	{Identifier::AS, OperatorID::AS,  15,  "-none-", OperatorFlags::BINARY},
	{"=>",  OperatorID::MAPS_TO,      1,  Identifier::Func::MAPS_TO, OperatorFlags::BINARY},
	{":=",  OperatorID::REF_ASSIGN,   0, "-none-", OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE}
};

OperatorFlags operator|(OperatorFlags a, OperatorFlags b) {
	return (OperatorFlags)((int)a | (int)b);
}

int operator&(OperatorFlags a, OperatorFlags b) {
	return (int)a & (int)b;
}

bool AbstractOperator::is_binary() const {
	return (flags & OperatorFlags::BINARY) == (int)OperatorFlags::BINARY;
}


string Operator::sig(const Class *ns) const {
	if (param_type_1 and param_type_2)
		return format("(%s) %s (%s)", param_type_1->cname(ns), abstract->name, param_type_2->cname(ns));
	if (param_type_1)
		return format("(%s) %s", param_type_1->cname(ns), abstract->name);
	return format("%s (%s)", abstract->name, param_type_2->cname(ns));
}

}
