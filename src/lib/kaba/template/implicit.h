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
class Block;

class AutoImplementer {
public:
	AutoImplementer(Parser *parser, SyntaxTree *tree);

	void do_error_implicit(Function *f, const string &msg);

	void _add_missing_function_headers_for_regular(Class *t);
	void _add_missing_function_headers_for_array(Class *t);
	void _add_missing_function_headers_for_list(Class *t);
	void _add_missing_function_headers_for_dict(Class *t);
	void _add_missing_function_headers_for_optional(Class *t);
	void _add_missing_function_headers_for_enum(Class *t);
	void _add_missing_function_headers_for_product(Class *t);
	void _add_missing_function_headers_for_shared(Class *t);
	void _add_missing_function_headers_for_owned(Class *t);
	void _add_missing_function_headers_for_xfer(Class *t);
	void _add_missing_function_headers_for_alias(Class *t);
	void _add_missing_function_headers_for_callable_fp(Class *t);
	void _add_missing_function_headers_for_callable_bind(Class *t);

	Function *add_func_header(Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, Function *cf = nullptr, Flags flags = Flags::NONE, const shared_array<Node> &def_params = {});

	void implement_add_virtual_table(shared<Node> self, Function *f, const Class *t);
	void implement_add_child_constructors(shared<Node> self, Function *f, const Class *t, bool allow_elements_from_parent);
	void implement_regular_constructor(Function *f, const Class *t, bool allow_parent_constructor);
	void implement_regular_destructor(Function *f, const Class *t);
	void implement_regular_assign(Function *f, const Class *t);
	void implement_array_constructor(Function *f, const Class *t);
	void implement_array_destructor(Function *f, const Class *t);
	void implement_array_assign(Function *f, const Class *t);
	void implement_list_constructor(Function *f, const Class *t);
	void implement_list_destructor(Function *f, const Class *t);
	void implement_list_assign(Function *f, const Class *t);
	void implement_list_clear(Function *f, const Class *t);
	void implement_list_resize(Function *f, const Class *t);
	void implement_list_add(Function *f, const Class *t);
	void implement_list_remove(Function *f, const Class *t);
	void implement_list_equal(Function *f, const Class *t);
	void implement_list_give(Function *f, const Class *t);
	void implement_dict_constructor(Function *f, const Class *t);
	void implement_dict_clear(Function *f, const Class *t);
	void implement_dict_assign(Function *f, const Class *t);
	void implement_dict_get(Function *f, const Class *t);
	void implement_dict_set(Function *f, const Class *t);
	void implement_dict_contains(Function *f, const Class *t);
	void implement_shared_constructor(Function *f, const Class *t);
	void implement_shared_destructor(Function *f, const Class *t);
	void implement_shared_assign(Function *f, const Class *t);
	void implement_shared_clear(Function *f, const Class *t);
	void implement_shared_create(Function *f, const Class *t);
	void implement_owned_constructor(Function *f, const Class *t);
	void implement_owned_destructor(Function *f, const Class *t);
	void implement_owned_clear(Function *f, const Class *t);
	void implement_owned_assign_raw(Function *f, const Class *t);
	void implement_owned_assign(Function *f, const Class *t);
	void implement_owned_give(Function *f, const Class *t);
	void implement_callable_constructor(Function *f, const Class *t);
	void implement_callable_fp_call(Function *f, const Class *t);
	void implement_callable_bind_call(Function *f, const Class *t);
	void implement_optional_constructor(Function *f, const Class *t);
	void implement_optional_constructor_wrap(Function *f, const Class *t);
	void implement_optional_destructor(Function *f, const Class *t);
	void implement_optional_assign(Function *f, const Class *t);
	void implement_optional_assign_raw(Function *f, const Class *t);
	void implement_optional_assign_null(Function *f, const Class *t);
	void implement_optional_has_value(Function *f, const Class *t);
	void implement_optional_value(Function *f, const Class *t);
	void implement_optional_equal(Function *f, const Class *t);
	void implement_optional_equal_raw(Function *f, const Class *t);
	void implement_product_equal(Function *f, const Class *t);
	void implement_product_not_equal(Function *f, const Class *t);
	void implement_future_constructor(Function *f, const Class *t);
	void _implement_functions_for_array(const Class *t);
	void _implement_functions_for_list(const Class *t);
	void _implement_functions_for_dict(const Class *t);
	void _implement_functions_for_optional(const Class *t);
	void _implement_functions_for_enum(const Class *t);
	void _implement_functions_for_product(const Class *t);
	void _implement_functions_for_shared(const Class *t);
	void _implement_functions_for_owned(const Class *t);
	void _implement_functions_for_xfer(const Class *t);
	void _implement_functions_for_alias(const Class *t);
	void _implement_functions_for_callable_fp(const Class *t);
	void _implement_functions_for_callable_bind(const Class *t);
	void _implement_functions_for_regular(const Class *t);

	shared<Node> node_false();
	shared<Node> node_true();
	shared<Node> node_nil();
	shared<Node> const_int(int i);

	void db_add_print_node(shared<Block> block, shared<Node> node);
	void db_add_print_p2s_node(shared<Block> block, shared<Node> node);
	void db_add_print_label(shared<Block> block, const string &s);
	void db_add_print_label_node(shared<Block> block, const string &s, shared<Node> node);

	shared<Node> add_assign(Function *f, const string &ctx, shared<Node> a, shared<Node> b);
	shared<Node> add_assign(Function *f, const string &ctx, const string &msg, shared<Node> a, shared<Node> b);

	shared<Node> add_equal(Function *f, const string &ctx, shared<Node> a, shared<Node> b);
	shared<Node> add_not_equal(Function *f, const string &ctx, shared<Node> a, shared<Node> b);


	static bool needs_new(Function *f);
	static Array<string> class_func_param_names(Function *cf);
	static bool has_user_constructors(const Class *t);
	void remove_inherited_constructors(Class *t);
	void redefine_inherited_constructors(Class *t);
	void add_full_constructor(Class *t);
	static bool class_can_fully_construct(const Class *t);
	static bool class_can_default_construct(const Class *t);
	static bool class_can_destruct(const Class *t);
	static bool class_can_assign(const Class *t);
	static bool class_can_elements_assign(const Class *t);
	static bool class_can_equal(const Class *t);

	static Function* prepare_auto_impl(const Class *t, Function *f);


	SyntaxTree *tree;
	Parser *parser;
	Context *context;
};

// TODO split
class AutoImplementerInternal : public AutoImplementer {
public:
	AutoImplementerInternal(Parser *p, SyntaxTree *tree) : AutoImplementer(p, tree) {}
	void implement_functions(const Class *t);
	void add_missing_function_headers_for_class(Class *t);
	void complete_type(Class *t, int array_size, int token_id);
};

}


#endif /* SRC_LIB_KABA_PARSER_IMPLICIT_H_ */
