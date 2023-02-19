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
	add_statement(Identifier::RETURN, StatementID::RETURN); // return: ParamType will be defined by the parser!
	add_statement(Identifier::IF, StatementID::IF, 2); // [CMP, BLOCK, [ELSE-BLOCK]]
	add_statement("-if/unwrap-", StatementID::IF_UNWRAP, 3); // [EXPR, OUT_VAR, BLOCK, [ELSE-BLOCK]]
	add_statement(Identifier::WHILE, StatementID::WHILE, 2); // [CMP, BLOCK]
	add_statement("-for-con-", StatementID::FOR_CONTAINER, 4); // [VAR, INDEX, ARRAY, BLOCK]
	add_statement("-for-range-", StatementID::FOR_RANGE, 5); // [VAR, START, STOP, STEP, BLOCK]
	add_statement(Identifier::FOR, StatementID::FOR_DIGEST, 4); // [INIT, CMP, BLOCK, INC] internally like a while-loop... but a bit different...
	add_statement(Identifier::BREAK, StatementID::BREAK);
	add_statement(Identifier::CONTINUE, StatementID::CONTINUE);
	add_statement(Identifier::NEW, StatementID::NEW, 1);
	add_statement(Identifier::DELETE, StatementID::DELETE, 1);
	add_statement(Identifier::LET, StatementID::LET);
	add_statement(Identifier::VAR, StatementID::VAR);
	add_statement(Identifier::ASM, StatementID::ASM);
	//add_statement(Identifier::RAISE, StatementID::RAISE); NOPE, now it's a function!
	add_statement(Identifier::TRY, StatementID::TRY); // return: ParamType will be defined by the parser!
	add_statement(Identifier::EXCEPT, StatementID::EXCEPT); // return: ParamType will be defined by the parser!
	add_statement(Identifier::PASS, StatementID::PASS);
	add_statement(Identifier::LAMBDA, StatementID::LAMBDA);
	add_statement(Identifier::FUNC, StatementID::FUNC);
	add_statement(Identifier::RAW_FUNCTION_POINTER, StatementID::RAW_FUNCTION_POINTER, 1);

	add_special_function(Identifier::SIZEOF, SpecialFunctionID::SIZEOF, 1, 1);
	add_special_function(Identifier::TYPEOF, SpecialFunctionID::TYPEOF, 1, 1);
	add_special_function(Identifier::STR, SpecialFunctionID::STR, 1, 1);
	add_special_function(Identifier::REPR, SpecialFunctionID::REPR, 1, 1);
	add_special_function(Identifier::LEN, SpecialFunctionID::LEN, 1, 1);
	add_special_function(Identifier::SORT, SpecialFunctionID::SORT, 1, 2);
	add_special_function(Identifier::FILTER, SpecialFunctionID::FILTER, 1, 1);
	add_special_function(Identifier::DYN, SpecialFunctionID::DYN, 1, 1);
	add_special_function(Identifier::WEAK, SpecialFunctionID::WEAK, 1, 1);
	add_special_function(Identifier::GIVE, SpecialFunctionID::GIVE, 1, 1);
}




}

