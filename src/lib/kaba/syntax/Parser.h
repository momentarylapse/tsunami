/*
 * Parser.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_PARSER_H_
#define SRC_LIB_KABA_SYNTAX_PARSER_H_

#include "lexical.h"
#include "SyntaxTree.h"

//#include "lexical.h"

namespace kaba {

class Class;
class Function;
class Block;
class SyntaxTree;
class Statement;
class PrimitiveOperator;
enum class Flags;

class Parser {
public:
	Parser(SyntaxTree *syntax);

	void parse_buffer(const string &buffer, bool just_analyse);

	void pre_compiler(bool just_analyse);
	void handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);

	void do_error(const string &msg, int override_exp_no = -1, int override_line = -1);
	void do_error_implicit(Function *f, const string &msg);
	void expect_no_new_line();
	void expect_new_line();
	void expect_indent();

	shared<Node> link_operator(PrimitiveOperator *primop, shared<Node> param1, shared<Node> param2);
	shared<Node> link_operator_id(OperatorID op_no, shared<Node> param1, shared<Node> param2);

	Array<const Class*> get_wanted_param_types(shared<Node> link);
	shared<Node> check_param_link(shared<Node> link, const Class *type, const string &f_name = "", int param_no = -1, int num_params = 1);

	shared<Node> apply_type_cast(int tc, shared<Node> param, const Class *wanted);
	shared<Node> apply_params_with_cast(shared<Node> operand, const shared_array<Node> &params, const Array<int> &casts, const Array<const Class*> &wanted);
	bool direct_param_match(const shared<Node> operand, const shared_array<Node> &params);
	bool param_match_with_cast(const shared<Node> operand, const shared_array<Node> &params, Array<int> &casts, Array<const Class*> &wanted, int *max_penalty);
	string param_match_with_cast_error(const shared_array<Node> &params, Array<const Class*> &wanted);
	shared<Node> apply_params_direct(shared<Node> operand, shared_array<Node> &params);
	shared<Node> force_concrete_type(shared<Node> node);
	void force_concrete_types(shared_array<Node> &nodes);
	shared<Node> deref_if_pointer(shared<Node> node);
	shared<Node> add_converter_str(shared<Node> sub, bool repr);

	void link_most_important_operator(shared_array<Node> &operand, shared_array<Node> &_operator, Array<int> &op_exp);
	shared_array<Node> make_class_node_callable(const Class *t, shared_array<Node> &params);
	shared<Node> make_func_node_callable(const shared<Node> l);
	shared<Node> link_unary_operator(PrimitiveOperator *op, shared<Node> operand, Block *block);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, shared<Node> cmd);

	shared<Node> link_special_operator_is(shared<Node> param1, shared<Node> param2);
	shared<Node> link_special_operator_in(shared<Node> param1, shared<Node> param2);
	shared<Node> make_dynamical(shared<Node> node);
	Array<const Class*> type_list_from_nodes(const shared_array<Node> &nn);


	shared<Node> build_abstract_list(const Array<shared<Node>> &el);
	shared<Node> parse_list(Block *block);
	shared<Node> build_abstract_dict(const Array<shared<Node>> &el);
	shared<Node> parse_dict(Block *block);
	shared<Node> build_abstract_tuple(const Array<shared<Node>> &el);
	shared<Node> build_function_pipe(const shared<Node> &input, const shared<Node> &func);

	const Class *get_constant_type(const string &str);
	void get_constant_value(const string &str, Value &value);

	void parse();
	void parse_top_level();
	void parse_all_class_names(Class *ns, int indent0);
	void parse_all_function_bodies();
	Flags parse_flags(Flags initial = Flags::NONE);
	void parse_import();
	void parse_enum(Class *_namespace);
	bool parse_class(Class *_namespace);
	Class *parse_class_header(Class *_namespace, int &offset0);
	void post_process_newly_parsed_class(Class *c, int size);
	void skip_parse_class();
	Function *parse_function_header_old(Class *name_space, Flags flags0);
	Function *parse_function_header_new(Class *name_space, Flags flags0);
	void skip_parsing_function_body(Function *f);
	void parse_function_body(Function *f);
	bool parse_function_command(Function *f, int indent0);
	const Class *parse_type(const Class *ns);
	//const Class *parse_product_type(const Class *ns);
	void parse_class_variable_declaration(const Class *ns, Block *block, int &_offset, Flags flags0 = Flags::NONE);
	void parse_class_use_statement(const Class *c);
	void parse_named_const(Class *name_space, Block *block);
	shared<Node> parse_and_eval_const(Block *block, const Class *type);
	static PrimitiveOperator *which_primitive_operator(const string &name, int param_flags = 3);
	static Statement *which_statement(const string &name);

	shared<Node> parse_operand_extension(const shared_array<Node> &operands, Block *block, bool prefer_type);
	shared_array<Node> parse_operand_extension_element(shared<Node> operand);
	shared<Node> parse_operand_extension_array(shared<Node> operand, Block *block);
	shared<Node> parse_operand_extension_call(const shared_array<Node> &operands, Block *block);

	shared<Node> try_to_match_apply_params(const shared_array<Node> &links, shared_array<Node> &params);

	const Class *parse_type_extension_array(const Class *c);
	const Class *parse_type_extension_dict(const Class *c);
	const Class *parse_type_extension_pointer(const Class *c);
	const Class *parse_type_extension_func(const Class *c);
	const Class *parse_type_extension_child(const Class *c);
	shared<Node> parse_single_func_param(Block *block);
	void parse_complete_command(Block *block);
	void parse_local_definition(Block *block, const Class *type);
	shared<Node> parse_block(Block *parent, Block *block = nullptr);
	shared<Node> parse_operand(Block *block, const Class *ns, bool prefer_class = false);
	shared<Node> parse_operand_greedy(Block *block, bool allow_tuples = false, shared<Node> first_operand = nullptr);
	shared<Node> parse_operand_super_greedy(Block *block);
	shared<Node> parse_set_builder(Block *block);

	shared<Node> parse_primitive_operator(Block *block);
	shared_array<Node> parse_call_parameters(Block *block);
	shared<Node> try_parse_format_string(Block *block, Value &v);
	shared<Node> apply_format(shared<Node> n, const string &fmt);
	shared<Node> parse_statement(Block *block);
	shared<Node> parse_for_header(Block *block);
	void post_process_for(shared<Node> n);
	shared<Node> parse_statement_for(Block *block);
	shared<Node> parse_statement_while(Block *block);
	shared<Node> parse_statement_break(Block *block);
	shared<Node> parse_statement_continue(Block *block);
	shared<Node> parse_statement_return(Block *block);
	shared<Node> parse_statement_raise(Block *block);
	shared<Node> parse_statement_try(Block *block);
	shared<Node> parse_statement_if(Block *block);
	shared<Node> parse_statement_pass(Block *block);
	shared<Node> parse_statement_new(Block *block);
	shared<Node> parse_statement_delete(Block *block);
	shared<Node> parse_statement_sizeof(Block *block);
	shared<Node> parse_statement_type(Block *block);
	shared<Node> parse_statement_str(Block *block);
	shared<Node> parse_statement_repr(Block *block);
	shared<Node> parse_statement_len(Block *block);
	shared<Node> parse_statement_let(Block *block);
	shared<Node> parse_statement_var(Block *block);
	shared<Node> parse_statement_map(Block *block);
	shared<Node> parse_statement_lambda(Block *block);
	shared<Node> parse_statement_sorted(Block *block);
	shared<Node> parse_statement_dyn(Block *block);
	shared<Node> parse_statement_call(Block *block);
	shared<Node> parse_statement_weak(Block *block);


	void auto_implement_add_virtual_table(shared<Node> self, Function *f, const Class *t);
	void auto_implement_add_child_constructors(shared<Node> self, Function *f, const Class *t, bool allow_elements_from_parent);
	void auto_implement_constructor(Function *f, const Class *t, bool allow_parent_constructor);
	void auto_implement_destructor(Function *f, const Class *t);
	void auto_implement_assign(Function *f, const Class *t);
	void auto_implement_array_clear(Function *f, const Class *t);
	void auto_implement_array_resize(Function *f, const Class *t);
	void auto_implement_array_add(Function *f, const Class *t);
	void auto_implement_array_remove(Function *f, const Class *t);
	void auto_implement_shared_assign(Function *f, const Class *t);
	void auto_implement_shared_clear(Function *f, const Class *t);
	void auto_implement_shared_create(Function *f, const Class *t);
	void auto_implement_owned_clear(Function *f, const Class *t);
	void auto_implement_owned_assign(Function *f, const Class *t);
	void auto_implement_functions(const Class *t);
	
	SyntaxTree *tree;
	Function *cur_func;
	ExpressionBuffer Exp;

	int for_index_count;
	int parser_loop_depth;
	bool found_dynamic_param;

	Array<Function*> function_needs_parsing;
};

} /* namespace kaba */

#endif /* SRC_LIB_KABA_SYNTAX_PARSER_H_ */
