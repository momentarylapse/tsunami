/*
 * Statement.cpp
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#include "Statement.h"
#include "Identifier.h"

namespace kaba {

Array<Statement*> Statements;

Statement *statement_from_id(StatementID id) {
	for (auto *s: Statements)
		if (s->id == id)
			return s;
	return nullptr;
}

int add_statement(const string &name, StatementID id, int num_params) {
	Statement *s = new Statement;
	s->name = name;
	s->id = id;
	s->num_params = num_params;
	Statements.add(s);
	return 0;
}



Array<SpecialFunction*> special_functions;

SpecialFunction *special_function_from_id(SpecialFunctionID id) {
	for (auto *s: special_functions)
		if (s->id == id)
			return s;
	return nullptr;
}

int add_special_function(const string &name, SpecialFunctionID id, int min_params, int max_params) {
	auto s = new SpecialFunction;
	s->name = name;
	s->id = id;
	s->min_params = min_params;
	s->max_params = max_params;
	special_functions.add(s);
	return 0;
}

void SIAddStatements() {
	// statements
	add_statement(Identifier::Return, StatementID::Return); // return: ParamType will be defined by the parser!
	add_statement(Identifier::If, StatementID::If, 2); // [CMP, BLOCK, [ELSE-BLOCK]]
	add_statement("-if/unwrap-", StatementID::IfUnwrap, 3); // [EXPR, OUT_VAR, BLOCK, [ELSE-BLOCK]]
	add_statement(Identifier::While, StatementID::While, 2); // [CMP, BLOCK]
	add_statement("-for-con-", StatementID::ForContainer, 4); // [VAR, INDEX, ARRAY, BLOCK]
	add_statement("-for-range-", StatementID::ForRange, 5); // [VAR, START, STOP, STEP, BLOCK]
	add_statement(Identifier::For, StatementID::ForDigest, 4); // [INIT, CMP, BLOCK, INC] internally like a while-loop... but a bit different...
	add_statement(Identifier::Break, StatementID::Break);
	add_statement(Identifier::Continue, StatementID::Continue);
	add_statement(Identifier::New, StatementID::New, 1);
	add_statement(Identifier::Delete, StatementID::Delete, 1);
	add_statement(Identifier::Let, StatementID::Let);
	add_statement(Identifier::Var, StatementID::Var);
	add_statement(Identifier::Asm, StatementID::Asm);
	add_statement(/*Identifier::RAISE*/ "-raise-", StatementID::Raise, 1); // [EXCEPTION (pointer into static array)]
	add_statement("-raise-local-", StatementID::RaiseLocal);
	add_statement(Identifier::Try, StatementID::Try); // return: ParamType will be defined by the parser!
	add_statement(Identifier::Except, StatementID::Except); // return: ParamType will be defined by the parser!
	add_statement(Identifier::Pass, StatementID::Pass);
	add_statement(Identifier::Lambda, StatementID::Lambda);
	add_statement(Identifier::Func, StatementID::Func);
	add_statement(Identifier::RawFunctionPointer, StatementID::RawFunctionPointer, 1);
	add_statement(Identifier::TrustMe, StatementID::TrustMe, 1); // [BLOCK]
	add_statement(Identifier::Match, StatementID::Match, 1); // [TERM, CASE1, INSTR1, ...]

	add_special_function(Identifier::Sizeof, SpecialFunctionID::Sizeof, 1, 1);
	add_special_function(Identifier::Typeof, SpecialFunctionID::Typeof, 1, 1);
	add_special_function(Identifier::Str, SpecialFunctionID::Str, 1, 1);
	add_special_function(Identifier::Repr, SpecialFunctionID::Repr, 1, 1);
	add_special_function(Identifier::Len, SpecialFunctionID::Len, 1, 1);
	add_special_function(Identifier::Sort, SpecialFunctionID::Sort, 1, 2);
	add_special_function(Identifier::Filter, SpecialFunctionID::Filter, 1, 1);
	add_special_function(Identifier::Dyn, SpecialFunctionID::Dyn, 1, 1);
	add_special_function(Identifier::Weak, SpecialFunctionID::Weak, 1, 1);
	add_special_function(Identifier::Give, SpecialFunctionID::Give, 1, 1);
}




}

