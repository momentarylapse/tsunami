/*
 * Transformer.h
 *
 *  Created on: 27 Feb 2023
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_TRANSFORMER_H_
#define SRC_LIB_KABA_PARSER_TRANSFORMER_H_

namespace kaba {

class SyntaxTree;

class Transformer {
public:
	Transformer();
	~Transformer();


	void fully_transform(SyntaxTree *tree);

	SyntaxTree *tree = nullptr;
};

}

#endif /* SRC_LIB_KABA_PARSER_TRANSFORMER_H_ */
