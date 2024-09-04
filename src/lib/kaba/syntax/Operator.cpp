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
AbstractOperator abstract_operators[(int)OperatorID::_Count_] = {
	{"=",  OperatorID::Assign,        0, Identifier::func::Assign, OperatorFlags::Binary | OperatorFlags::LeftIsModifiable},
	{"+",  OperatorID::Add,           12, Identifier::func::Add, OperatorFlags::Binary},
	{"-",  OperatorID::Subtract,      12, Identifier::func::Subtract, OperatorFlags::Binary},
	{"*",  OperatorID::Multiply,      13, Identifier::func::Multiply, OperatorFlags::Binary},
	{"/",  OperatorID::Divide,        13, Identifier::func::Divide, OperatorFlags::Binary},
	{"-",  OperatorID::Negative,      14, Identifier::func::Negative, OperatorFlags::UnaryRight}, // -1 etc
	{"+=", OperatorID::AddAssign,          0,  Identifier::func::AddAssign, OperatorFlags::Binary | OperatorFlags::LeftIsModifiable},
	{"-=", OperatorID::SubtractAssign,     0,  Identifier::func::SubtractAssign, OperatorFlags::Binary | OperatorFlags::LeftIsModifiable},
	{"*=", OperatorID::MultiplyAssign,     0,  Identifier::func::MultiplyAssign, OperatorFlags::Binary | OperatorFlags::LeftIsModifiable},
	{"/=", OperatorID::DivideAssign,       0,  Identifier::func::DivideAssign, OperatorFlags::Binary | OperatorFlags::LeftIsModifiable},
	{"==", OperatorID::Equal,         9,  Identifier::func::Equal, OperatorFlags::Binary},
	{"!=", OperatorID::NotEqual,     9,  Identifier::func::NotEqual, OperatorFlags::Binary},
	{Identifier::Not,OperatorID::Negate,3,  Identifier::func::Not, OperatorFlags::UnaryRight},
	{"<",  OperatorID::Smaller,       10,  Identifier::func::Smaller, OperatorFlags::Binary},
	{">",  OperatorID::Greater,       10,  Identifier::func::Greater, OperatorFlags::Binary},
	{"<=", OperatorID::SmallerEqual, 10,  Identifier::func::SmallerEqual, OperatorFlags::Binary},
	{">=", OperatorID::GreaterEqual, 10,  Identifier::func::GreaterEqual, OperatorFlags::Binary},
	{Identifier::And, OperatorID::And,5,  Identifier::func::And, OperatorFlags::Binary},
	{Identifier::Or,  OperatorID::Or, 4,  Identifier::func::Or, OperatorFlags::Binary},
	{"%",  OperatorID::Modulo,        13, Identifier::func::Modulo, OperatorFlags::Binary},
	{"&",  OperatorID::BitAnd,       8, Identifier::func::BitAnd, OperatorFlags::Binary},
	{"|",  OperatorID::BitOr,        6, Identifier::func::BitOr, OperatorFlags::Binary},
	{"<<", OperatorID::ShiftLeft,    11, Identifier::func::ShiftLeft, OperatorFlags::Binary},
	{">>", OperatorID::ShiftRight,   11, Identifier::func::ShiftRight, OperatorFlags::Binary},
	{"++", OperatorID::Increase,      3, Identifier::func::Increase, OperatorFlags::UnaryLeft | OperatorFlags::LeftIsModifiable},
	{"--", OperatorID::Decrease,      3, Identifier::func::Decrease, OperatorFlags::UnaryLeft | OperatorFlags::LeftIsModifiable},
	{Identifier::Is, OperatorID::Is,  3,  "-none-", OperatorFlags::Binary},
	{Identifier::In, OperatorID::In,  13, Identifier::func::Contains, OperatorFlags::Binary | OperatorFlags::OrderInverted}, // INVERTED
	{Identifier::Extends, OperatorID::Extends, 3,  "-none-", OperatorFlags::Binary},
	{"^",  OperatorID::Exponent,      15,  Identifier::func::Exponent, OperatorFlags::Binary},
	{",",  OperatorID::Comma,         0,  "-none-", OperatorFlags::Binary},
	{"*",  OperatorID::Dereference,   16,  Identifier::func::Get, OperatorFlags::UnaryRight},
	{"&",  OperatorID::Reference,     16,  "-none-", OperatorFlags::UnaryRight},
	{"[...]",  OperatorID::Index,     17,  "-none-", OperatorFlags::Binary},
	{"|>",  OperatorID::PipeRight,1,  "-none-", OperatorFlags::Binary},
	{Identifier::As, OperatorID::As,  16,  "-none-", OperatorFlags::Binary},
	{"=>",  OperatorID::MapsTo,      2,  Identifier::func::MapsTo, OperatorFlags::Binary},
	{":=",  OperatorID::RefAssign,   0, "-none-", OperatorFlags::Binary | OperatorFlags::LeftIsModifiable}
};

OperatorFlags operator|(OperatorFlags a, OperatorFlags b) {
	return (OperatorFlags)((int)a | (int)b);
}

int operator&(OperatorFlags a, OperatorFlags b) {
	return (int)a & (int)b;
}

bool AbstractOperator::is_binary() const {
	return (flags & OperatorFlags::Binary) == (int)OperatorFlags::Binary;
}


string Operator::sig(const Class *ns) const {
	if (param_type_1 and param_type_2)
		return format("(%s) %s (%s)", param_type_1->cname(ns), abstract->name, param_type_2->cname(ns));
	if (param_type_1)
		return format("(%s) %s", param_type_1->cname(ns), abstract->name);
	return format("%s (%s)", abstract->name, param_type_2->cname(ns));
}

}
