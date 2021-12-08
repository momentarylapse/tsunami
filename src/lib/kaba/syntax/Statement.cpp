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


void SIAddStatements() {
	// statements
	add_statement(IDENTIFIER_RETURN, StatementID::RETURN); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_IF, StatementID::IF, 2); // [CMP, BLOCK]
	add_statement("-if/else-", StatementID::IF_ELSE, 3); // [CMP, BLOCK, ELSE-BLOCK]
	add_statement(IDENTIFIER_WHILE, StatementID::WHILE, 2); // [CMP, BLOCK]
	add_statement("-for-array-", StatementID::FOR_ARRAY, 4); // [VAR, INDEX, ARRAY, BLOCK]
	add_statement("-for-range-", StatementID::FOR_RANGE, 5); // [VAR, START, STOP, STEP, BLOCK]
	add_statement(IDENTIFIER_FOR, StatementID::FOR_DIGEST, 4); // [INIT, CMP, BLOCK, INC] internally like a while-loop... but a bit different...
	add_statement(IDENTIFIER_BREAK, StatementID::BREAK);
	add_statement(IDENTIFIER_CONTINUE, StatementID::CONTINUE);
	add_statement(IDENTIFIER_NEW, StatementID::NEW, 1);
	add_statement(IDENTIFIER_DELETE, StatementID::DELETE, 1);
	add_statement(IDENTIFIER_SIZEOF, StatementID::SIZEOF, 1);
	add_statement(IDENTIFIER_TYPE, StatementID::TYPE, 1);
	add_statement(IDENTIFIER_STR, StatementID::STR, 1);
	add_statement(IDENTIFIER_REPR, StatementID::REPR, 1);
	add_statement(IDENTIFIER_LEN, StatementID::LEN, 1);
	add_statement(IDENTIFIER_LET, StatementID::LET);
	add_statement(IDENTIFIER_VAR, StatementID::VAR);
	add_statement(IDENTIFIER_ASM, StatementID::ASM);
	//add_statement(IDENTIFIER_RAISE, StatementID::RAISE); NOPE, now it's a function!
	add_statement(IDENTIFIER_TRY, StatementID::TRY); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_EXCEPT, StatementID::EXCEPT); // return: ParamType will be defined by the parser!
	add_statement(IDENTIFIER_PASS, StatementID::PASS);
	add_statement(IDENTIFIER_MAP, StatementID::MAP);
	add_statement(IDENTIFIER_LAMBDA, StatementID::LAMBDA);
	add_statement(IDENTIFIER_SORTED, StatementID::SORTED);
	add_statement(IDENTIFIER_DYN, StatementID::DYN);
	//add_statement(IDENTIFIER_CALL, StatementID::CALL);
	add_statement(IDENTIFIER_WEAK, StatementID::WEAK, 1);
}




}

