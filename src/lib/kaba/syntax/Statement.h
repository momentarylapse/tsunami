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
	BLOCK_RETURN,
	IF,
	IF_UNWRAP,
	WHILE,
	FOR_CONTAINER,
	FOR_RANGE,
	FOR_DIGEST,
	BREAK,
	CONTINUE,
	NEW,
	DELETE,
	ASM,
	//RAISE,
	TRY,
	EXCEPT,
	PASS,
	LET,
	VAR,
	LAMBDA,
	FUNC,
	RAW_FUNCTION_POINTER,
	TRUST_ME
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


// special functions
enum class SpecialFunctionID {
	SIZEOF,
	TYPEOF,
	STR,
	REPR,
	LEN,
	SORT,
	FILTER,
	DYN,
	WEAK,
	GIVE
};

class SpecialFunction {
public:
	string name;
	int min_params;
	int max_params;
	SpecialFunctionID id;
};
extern Array<SpecialFunction*> special_functions;

SpecialFunction *special_function_from_id(SpecialFunctionID id);
int add_special_function(const string &name, SpecialFunctionID id, int min_params, int max_params);

}

