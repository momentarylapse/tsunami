/*
 * Statement.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

#include "../../base/base.h"


namespace kaba {


#ifdef DELETE
#undef DELETE
#endif

// statements
enum class StatementID {
	RETURN,
	IF,
	IF_ELSE,
	WHILE,
	FOR_ARRAY,
	FOR_RANGE,
	FOR_DIGEST,
	BREAK,
	CONTINUE,
	NEW,
	DELETE,
	SIZEOF,
	TYPEOF,
	ASM,
	//RAISE,
	TRY,
	EXCEPT,
	PASS,
	STR,
	REPR,
	LEN,
	LET,
	VAR,
	MAP,
	LAMBDA,
	FUNC,
	SORTED,
	DYN,
	WEAK,
	RAW_FUNCTION_POINTER
};

class Statement {
public:
	string name;
	int num_params;
	StatementID id;
};
extern Array<Statement*> Statements;

Statement *statement_from_id(StatementID id);
int add_statement(const string &name, StatementID id, int num_params = 0);

}

