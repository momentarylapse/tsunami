/*
 * import.h
 *
 *  Created on: 07 Jan 2024
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_IMPORT_H_
#define SRC_LIB_KABA_PARSER_IMPORT_H_

#include "../../base/pointer.h"

namespace kaba {

class Module;
class Class;
class Function;
class Variable;
class Constant;

struct ImportSource {
	shared<Module> module;
	const Class *_class = nullptr;
	const Function *func = nullptr;
	const Variable *var = nullptr;
	const Constant *_const = nullptr;
};


ImportSource resolve_import_source(Parser *parser, const Array<string> &name, int token);


}

#endif /* SRC_LIB_KABA_PARSER_IMPORT_H_ */
