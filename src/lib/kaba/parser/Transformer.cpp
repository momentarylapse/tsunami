/*
 * Transformer.cpp
 *
 *  Created on: 27 Feb 2023
 *      Author: michi
 */

#include "Transformer.h"

namespace kaba {

Transformer::Transformer() {
}

Transformer::~Transformer() {
}

void Transformer::fully_transform(SyntaxTree *_tree) {
	tree = _tree;
}

}
