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
	{"+",  OperatorID::ADD,           12, Identifier::Func::ADD, OperatorFlags::BINARY},
	{"-",  OperatorID::SUBTRACT,      12, Identifier::Func::SUBTRACT, OperatorFlags::BINARY},
	{"*",  OperatorID::MULTIPLY,      13, Identifier::Func::MULTIPLY, OperatorFlags::BINARY},
	{"/",  OperatorID::DIVIDE,        13, Identifier::Func::DIVIDE, OperatorFlags::BINARY},
	{"-",  OperatorID::NEGATIVE,      14, Identifier::Func::NEGATIVE, OperatorFlags::UNARY_RIGHT}, // -1 etc
	{"+=", OperatorID::ADDS,          0,  Identifier::Func::ADD_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"-=", OperatorID::SUBTRACTS,     0,  Identifier::Func::SUBTRACT_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"*=", OperatorID::MULTIPLYS,     0,  Identifier::Func::MULTIPLY_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"/=", OperatorID::DIVIDES,       0,  Identifier::Func::DIVIDE_ASSIGN, OperatorFlags::BINARY | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"==", OperatorID::EQUAL,         9,  Identifier::Func::EQUAL, OperatorFlags::BINARY},
	{"!=", OperatorID::NOT_EQUAL,     9,  Identifier::Func::NOT_EQUAL, OperatorFlags::BINARY},
	{Identifier::NOT,OperatorID::NEGATE,3,  Identifier::Func::NOT, OperatorFlags::UNARY_RIGHT},
	{"<",  OperatorID::SMALLER,       10,  Identifier::Func::SMALLER, OperatorFlags::BINARY},
	{">",  OperatorID::GREATER,       10,  Identifier::Func::GREATER, OperatorFlags::BINARY},
	{"<=", OperatorID::SMALLER_EQUAL, 10,  Identifier::Func::SMALLER_EQUAL, OperatorFlags::BINARY},
	{">=", OperatorID::GREATER_EQUAL, 10,  Identifier::Func::GREATER_EQUAL, OperatorFlags::BINARY},
	{Identifier::AND, OperatorID::AND,5,  Identifier::Func::AND, OperatorFlags::BINARY},
	{Identifier::OR,  OperatorID::OR, 4,  Identifier::Func::OR, OperatorFlags::BINARY},
	{"%",  OperatorID::MODULO,        13, Identifier::Func::MODULO, OperatorFlags::BINARY},
	{"&",  OperatorID::BIT_AND,       8, Identifier::Func::BIT_AND, OperatorFlags::BINARY},
	{"|",  OperatorID::BIT_OR,        6, Identifier::Func::BIT_OR, OperatorFlags::BINARY},
	{"<<", OperatorID::SHIFT_LEFT,    11, Identifier::Func::SHIFT_LEFT, OperatorFlags::BINARY},
	{">>", OperatorID::SHIFT_RIGHT,   11, Identifier::Func::SHIFT_RIGHT, OperatorFlags::BINARY},
	{"++", OperatorID::INCREASE,      3, Identifier::Func::INCREASE, OperatorFlags::UNARY_LEFT | OperatorFlags::LEFT_IS_MODIFIABLE},
	{"--", OperatorID::DECREASE,      3, Identifier::Func::DECREASE, OperatorFlags::UNARY_LEFT | OperatorFlags::LEFT_IS_MODIFIABLE},
	{Identifier::IS, OperatorID::IS,  3,  "-none-", OperatorFlags::BINARY},
	{Identifier::IN, OperatorID::IN,  13, Identifier::Func::CONTAINS, OperatorFlags::BINARY | OperatorFlags::ORDER_INVERTED}, // INVERTED
	{Identifier::EXTENDS, OperatorID::EXTENDS, 3,  "-none-", OperatorFlags::BINARY},
	{"^",  OperatorID::EXPONENT,      15,  Identifier::Func::EXPONENT, OperatorFlags::BINARY},
	{",",  OperatorID::COMMA,         0,  "-none-", OperatorFlags::BINARY},
	{"*",  OperatorID::DEREFERENCE,   16,  Identifier::Func::GET, OperatorFlags::UNARY_RIGHT},
	{"&",  OperatorID::REFERENCE,     16,  "-none-", OperatorFlags::UNARY_RIGHT},
	{"[...]",  OperatorID::ARRAY,     17,  "-none-", OperatorFlags::BINARY},
	{"|>",  OperatorID::FUNCTION_PIPE,1,  "-none-", OperatorFlags::BINARY},
	{Identifier::AS, OperatorID::AS,  16,  "-none-", OperatorFlags::BINARY},
	{"=>",  OperatorID::MAPS_TO,      2,  Identifier::Func::MAPS_TO, OperatorFlags::BINARY},
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
