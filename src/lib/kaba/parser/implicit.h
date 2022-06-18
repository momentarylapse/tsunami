/*
 * implicit.h
 *
 *  Created on: 28 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_IMPLICIT_H_
#define SRC_LIB_KABA_PARSER_IMPLICIT_H_

#include "../../base/pointer.h"

namespace kaba {

class Node;
class Function;
class Class;
class SyntaxTree;
class Parser;

class AutoImplementer {
public:
	AutoImplementer(Parser *parser, SyntaxTree *tree);

	void do_error_implicit(Function *f, const string &msg);

	void auto_implement_add_virtual_table(shared<Node> self, Function *f, const Class *t);
	void auto_implement_add_child_constructors(shared<Node> self, Function *f, const Class *t, bool allow_elements_from_parent);
	void auto_implement_regular_constructor(Function *f, const Class *t, bool allow_parent_constructor);
	void auto_implement_regular_destructor(Function *f, const Class *t);
	void auto_implement_regular_assign(Function *f, const Class *t);
	void auto_implement_array_constructor(Function *f, const Class *t);
	void auto_implement_array_destructor(Function *f, const Class *t);
	void auto_implement_array_assign(Function *f, const Class *t);
	void auto_implement_super_array_constructor(Function *f, const Class *t);
	void auto_implement_super_array_destructor(Function *f, const Class *t);
	void auto_implement_super_array_assign(Function *f, const Class *t);
	void auto_implement_super_array_clear(Function *f, const Class *t);
	void auto_implement_super_array_resize(Function *f, const Class *t);
	void auto_implement_super_array_add(Function *f, const Class *t);
	void auto_implement_super_array_remove(Function *f, const Class *t);
	void auto_implement_dict_constructor(Function *f, const Class *t);
	void auto_implement_shared_constructor(Function *f, const Class *t);
	void auto_implement_shared_destructor(Function *f, const Class *t);
	void auto_implement_shared_assign(Function *f, const Class *t);
	void auto_implement_shared_clear(Function *f, const Class *t);
	void auto_implement_shared_create(Function *f, const Class *t);
	void auto_implement_owned_clear(Function *f, const Class *t);
	void auto_implement_owned_assign(Function *f, const Class *t);
	void auto_implement_callable_constructor(Function *f, const Class *t);
	void auto_implement_callable_fp_call(Function *f, const Class *t);
	void auto_implement_callable_bind_call(Function *f, const Class *t);
	void auto_implement_functions(const Class *t);

	SyntaxTree *tree;
	Parser *parser;
};

}


#endif /* SRC_LIB_KABA_PARSER_IMPLICIT_H_ */
