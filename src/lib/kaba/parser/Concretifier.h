/*
 * Concretifier.h
 *
 *  Created on: 23 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_CONCRETIFIER_H_
#define SRC_LIB_KABA_PARSER_CONCRETIFIER_H_

#include "../syntax/SyntaxTree.h"
#include "lexical.h"

namespace kaba {

class Class;
class Function;
class Block;
class SyntaxTree;
class Statement;
enum class Flags;
class Parser;
class AutoImplementer;

struct CastingData {
	int cast;
	int penalty;
	Function *f;
};

class Concretifier {
public:
	Concretifier(Parser *parser, SyntaxTree *tree);

	void concretify_all_params(shared<Node> &node, Block *block, const Class *ns);

	shared<Node> concretify_node(shared<Node> node, Block *block, const Class *ns);
	shared_array<Node> concretify_node_multi(shared<Node> node, Block *block, const Class *ns);
	const Class *concretify_as_type(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_array(shared<Node> node, Block *block, const Class *ns);
	shared_array<Node> concretify_element(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_var_declaration(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_array_builder_for(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_operator(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_call(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_return(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_if(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_while(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_range(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_array(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_str(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_repr(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_sizeof(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_typeof(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_len(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_new(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_delete(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_dyn(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_sorted(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_weak(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_map(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_raw_function_pointer(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_try(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_lambda(shared<Node> node, Block *block, const Class *ns);
	shared<Node> force_concrete_type(shared<Node> node);
	shared<Node> force_concrete_type_if_function(shared<Node> node);
	void force_concrete_types(shared_array<Node> &nodes);
	void concretify_function_header(Function *f);
	void concretify_function_body(Function *f);


	shared_array<Node> turn_class_into_constructor(const Class *t, const shared_array<Node> &params, int token_id);
	shared<Node> make_func_node_callable(const shared<Node> l);
	shared<Node> make_func_pointer_node_callable(const shared<Node> l);
	shared<Node> link_unary_operator(AbstractOperator *op, shared<Node> operand, Block *block, int token_id);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, shared<Node> cmd);

	shared<Node> wrap_function_into_callable(Function *f, int token_id);
	shared<Node> wrap_node_into_callable(shared<Node> node);

	shared<Node> apply_type_cast(const CastingData &cast, shared<Node> param, const Class *wanted);
	shared<Node> apply_params_with_cast(shared<Node> operand, const shared_array<Node> &params, const Array<CastingData> &casts, const Array<const Class*> &wanted, int offset = 0);
	bool direct_param_match(const shared<Node> operand, const shared_array<Node> &params);
	bool param_match_with_cast(const shared<Node> operand, const shared_array<Node> &params, Array<CastingData> &casts, Array<const Class*> &wanted, int *max_penalty);
	string param_match_with_cast_error(const shared_array<Node> &params, const Array<const Class*> &wanted);
	shared<Node> apply_params_direct(shared<Node> operand, const shared_array<Node> &params, int offset = 0);
	shared<Node> explicit_cast(shared<Node> node, const Class *wanted);

	shared<Node> link_operator(AbstractOperator *primop, shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_operator_id(OperatorID op_no, shared<Node> param1, shared<Node> param2, int token_id = -1);

	Array<const Class*> get_wanted_param_types(shared<Node> link, int &mandatory_params);
	shared<Node> check_param_link(shared<Node> link, const Class *type, const string &f_name = "", int param_no = -1, int num_params = 1);


	shared<Node> deref_if_pointer(shared<Node> node);
	shared<Node> add_converter_str(shared<Node> sub, bool repr);

	shared<Node> link_special_operator_is(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_in(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_as(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_tuple_extract(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> make_dynamical(shared<Node> node);
	Array<const Class*> type_list_from_nodes(const shared_array<Node> &nn);


	shared<Node> try_to_match_apply_params(const shared_array<Node> &links, shared_array<Node> &params);

	shared<Node> build_function_pipe(const shared<Node> &input, const shared<Node> &func);
	shared<Node> build_lambda_new(const shared<Node> &param, const shared<Node> &expression);



	void do_error(const string &msg, shared<Node> node);
	void do_error(const string &msg, int token_id);


	SyntaxTree *tree;
	Parser *parser;
	AutoImplementer *auto_implementer;
	int for_index_count = 0;
};

}

#endif /* SRC_LIB_KABA_PARSER_CONCRETIFIER_H_ */