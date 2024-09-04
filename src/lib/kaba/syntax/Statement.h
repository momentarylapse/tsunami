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
	Return,
	If,
	IfUnwrap,
	While,
	ForContainer,
	ForRange,
	ForDigest,
	Break,
	Continue,
	New,
	Delete,
	Asm,
	Raise,
	RaiseLocal,
	Try,
	Except,
	Pass,
	Let,
	Var,
	Lambda,
	Func,
	RawFunctionPointer,
	TrustMe,
	Match
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
	Sizeof,
	Typeof,
	Str,
	Repr,
	Len,
	Sort,
	Filter,
	Dyn,
	Weak,
	Give
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

