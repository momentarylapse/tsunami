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
class Context;

struct CastingDataSingle {
	int cast;
	int penalty;
	Function *f; // casting runction
	unsigned char pre_deref_count;
};

struct CastingDataCall {
	Array<CastingDataSingle> params;
	Function *func_override = nullptr; // for template instantiation
	Array<const Class*> wanted;
	int penalty = 0;
};

class Concretifier {
public:
	Concretifier(Context *c, Parser *parser, SyntaxTree *tree);

	enum TypeCastId {
		NONE = -1,
		DEREFERENCE = -2,
		REFERENCE = -3,
		OWN_STRING = -10,
		ABSTRACT_LIST = -20,
		ABSTRACT_TUPLE = -21,
		ABSTRACT_DICT = -22,
		TUPLE_AS_CONSTRUCTOR = -23,
		AUTO_CONSTRUCTOR = -24,
		FUNCTION_AS_CALLABLE = -30,
		MAKE_SHARED = -40, // TODO use auto constructor instead
		MAKE_OWNED = -41,
		WEAK_POINTER = -42
	};


	void concretify_all_params(shared<Node> &node, Block *block, const Class *ns);

	shared<Node> concretify_node(shared<Node> node, Block *block, const Class *ns);
	shared_array<Node> concretify_node_multi(shared<Node> node, Block *block, const Class *ns);
	shared_array<Node> concretify_token(shared<Node> node, Block *block, const Class *ns);
	const Class *concretify_as_type(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_array(shared<Node> node, Block *block, const Class *ns);
	shared_array<Node> concretify_element(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_var_declaration(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_array_builder_for(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_array_builder_for_inner(shared<Node> n_for, shared<Node> n_exp, shared<Node> n_cmp, const Class *type_el, Block *block, const Class *ns, int token_id);
	shared<Node> concretify_operator(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_block(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_definitely(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_call(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_return(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_if(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_while(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_range(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_container(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_array(shared<Node> node, shared<Node> container, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_dict(shared<Node> node, shared<Node> container, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_unwrap_pointer(shared<Node> node, shared<Node> container, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_unwrap_pointer_shared(shared<Node> node, shared<Node> container, Block *block, const Class *ns);
	shared<Node> concretify_statement_for_unwrap_optional(shared<Node> node, shared<Node> container, Block *block, const Class *ns);
	shared<Node> concretify_statement_new(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_delete(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_raw_function_pointer(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_try(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_raise(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_lambda(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_statement_match(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_call(shared<Node> node, SpecialFunction *s, Block *block, const Class *ns);
	shared<Node> concretify_special_function_str(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_repr(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_sizeof(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_typeof(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_len(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_dyn(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_sort(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_filter(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_weak(shared<Node> node, Block *block, const Class *ns);
	shared<Node> concretify_special_function_give(shared<Node> node, Block *block, const Class *ns);
	shared<Node> force_concrete_type(shared<Node> node);
	shared<Node> force_concrete_type_if_function(shared<Node> node);
	void force_concrete_types(shared_array<Node> &nodes);
	void concretify_function_header(Function *f);
	void concretify_function_body(Function *f);


	shared_array<Node> turn_class_into_constructor(const Class *t, const shared_array<Node> &params, int token_id);
	shared<Node> make_func_node_callable(const shared<Node> l);
	shared<Node> match_template_params(const shared<Node> l, const shared_array<Node> &params, bool allow_fail);
	shared<Node> make_func_pointer_node_callable(const shared<Node> l);
	shared<Node> link_unary_operator(AbstractOperator *op, shared<Node> operand, Block *block, int token_id);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, shared<Node> cmd);
	const Class *make_effective_class_callable(shared<Node> node);

	shared<Node> wrap_function_into_callable(Function *f, int token_id);
	shared<Node> wrap_node_into_callable(shared<Node> node);

	bool type_match_with_cast(shared<Node> node, bool is_modifiable, const Class *wanted, CastingDataSingle &cd);
	bool type_match_tuple_as_contructor(shared<Node> node, Function *f_constructor, int &penalty);

	shared<Node> apply_type_cast_basic(const CastingDataSingle &cast, shared<Node> param, const Class *wanted);
	shared<Node> apply_type_cast(const CastingDataSingle &cast, shared<Node> param, const Class *wanted);
	shared<Node> apply_params_with_cast(shared<Node> operand, const shared_array<Node> &params, const CastingDataCall &casts, int offset = 0);
	bool direct_param_match(const shared<Node> operand, const shared_array<Node> &params);
	bool param_match_with_cast(const shared<Node> operand, const shared_array<Node> &params, CastingDataCall &casts);
	string param_match_with_cast_error(const shared_array<Node> &params, const Array<const Class*> &wanted);
	shared<Node> apply_params_direct(shared<Node> operand, const shared_array<Node> &params, int offset = 0);
	shared<Node> explicit_cast(shared<Node> node, const Class *wanted);

	shared<Node> link_operator(AbstractOperator *primop, shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_operator_id(OperatorID op_no, shared<Node> param1, shared<Node> param2, int token_id = -1);

	Array<const Class*> get_wanted_param_types(shared<Node> link, int &mandatory_params);
	shared<Node> check_param_link(shared<Node> link, const Class *type, const string &f_name = "", int param_no = -1, int num_params = 1);


	shared<Node> deref_if_reference(shared<Node> node);
	shared<Node> add_converter_str(shared<Node> sub, bool repr);

	shared<Node> link_special_operator_is(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_in(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_as(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_tuple_extract(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> link_special_operator_ref_assign(shared<Node> param1, shared<Node> param2, int token_id);
	shared<Node> make_dynamical(shared<Node> node);
	Array<const Class*> type_list_from_nodes(const shared_array<Node> &nn);


	shared<Node> try_to_match_apply_params(const shared_array<Node> &links, shared_array<Node> &params);

	shared<Node> build_function_pipe(const shared<Node> &input, const shared<Node> &func, Block *block, const Class *ns, int token_id);
	shared<Node> build_pipe_sort(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id);
	shared<Node> build_pipe_filter(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id);
	shared<Node> build_pipe_len(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id);
	shared<Node> build_pipe_map(const shared<Node> &input, const shared<Node> &rhs, Block *block, const Class *ns, int token_id);
	shared<Node> build_lambda_new(const shared<Node> &param, const shared<Node> &expression, Block *block, const Class *ns, int token_id);
	shared<Node> try_build_pipe_map_array_unwrap(const shared<Node> &input, Node *f, const Class *rt, const Class *pt, Block *block, const Class *ns, int token_id);
	shared<Node> try_build_pipe_map_optional_unwrap(const shared<Node> &input, Node *f, const Class *rt, const Class *pt, Block *block, const Class *ns, int token_id);
	shared<Node> try_build_pipe_map_direct(const shared<Node> &input, Node *f, const Class *rt, const Class *pt, Block *block, const Class *ns, int token_id);



	void do_error(const string &msg, shared<Node> node);
	void do_error(const string &msg, int token_id);


	Context *context;
	SyntaxTree *tree;
	Parser *parser;
	AutoImplementer *auto_implementer;
	int for_index_count = 0;
};

}

#endif /* SRC_LIB_KABA_PARSER_CONCRETIFIER_H_ */
