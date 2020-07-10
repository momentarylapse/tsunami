/*
 * Parser.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_PARSER_H_
#define SRC_LIB_KABA_SYNTAX_PARSER_H_

//#include "lexical.h"

namespace Kaba {
	
class SyntaxTree;

class Parser {
public:
	Parser(SyntaxTree *syntax);
	~Parser();
	
	SyntaxTree *syntax;
	//ExpressionBuffer Exp;
};

} /* namespace Kaba */

#endif /* SRC_LIB_KABA_SYNTAX_PARSER_H_ */
