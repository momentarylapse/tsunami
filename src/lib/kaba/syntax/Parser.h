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

namespace Kaba {

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
	~Parser();

	void parse_buffer(const string &buffer, bool just_analyse);

	void pre_compiler(bool just_analyse);
	void handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);

	void do_error(const string &msg, int override_exp_no = -1, int override_line = -1);
	void do_error_implicit(Function *f, const string &msg);
	void expect_no_new_line();
	void expect_new_line();
	void expect_indent();

	Node *link_operator(PrimitiveOperator *primop, Node *param1, Node *param2);
	Node *link_operator_id(OperatorID op_no, Node *param1, Node *param2);

	Array<const Class*> get_wanted_param_types(Node *link);
	Node *check_param_link(Node *link, const Class *type, const string &f_name = "", int param_no = -1);

	Node *apply_type_cast(int tc, Node *param, const Class *wanted);
	Node *apply_params_with_cast(Node *operand, const Array<Node*> &params, const Array<int> &casts, const Array<const Class*> &wanted);
	bool direct_param_match(Node *operand, Array<Node*> &params);
	bool param_match_with_cast(Node *operand, Array<Node*> &params, Array<int> &casts, Array<const Class*> &wanted);
	Node *apply_params_direct(Node *operand, Array<Node*> &params);
	Node *force_concrete_type(Node *node);
	void force_concrete_types(Array<Node*> &nodes);
	Node *deref_if_pointer(Node *node);
	Node *add_converter_str(Node *sub, bool repr);

	void link_most_important_operator(Array<Node*> &operand, Array<Node*> &_operator, Array<int> &op_exp);
	Array<Node*> make_class_node_callable(const Class *t, Block *block, Array<Node*> &params);
	void make_func_node_callable(Node *l);
	Node *link_unary_operator(PrimitiveOperator *op, Node *operand, Block *block);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, Node *cmd);

	Node *link_special_operator_is(Node *param1, Node *param2);
	Node *link_special_operator_in(Node *param1, Node *param2);
	Node *make_dynamical(Node *node);
	Array<const Class*> type_list_from_nodes(const Array<Node*> &nn);


	Node *build_abstract_list(const Array<Node*> &el);
	Node *parse_list(Block *block);
	Node *build_abstract_dict(const Array<Node*> &el);
	Node *parse_dict(Block *block);

	const Class *get_constant_type(const string &str);
	void get_constant_value(const string &str, Value &value);

	void parse();
	void parse_top_level();
	void parse_all_class_names(Class *ns, int indent0);
	void parse_all_function_bodies(const Class *name_space);
	Flags parse_flags(Flags initial);
	void parse_import();
	void parse_enum(Class *_namespace);
	bool parse_class(Class *_namespace);
	void skip_parse_class();
	Function *parse_function_header(Class *name_space, Flags flags);
	void skip_parsing_function_body();
	void parse_function_body(Function *f);
	bool parse_function_command(Function *f, int indent0);
	const Class *parse_type(const Class *ns);
	void parse_global_variable_def(bool single, Block *block, Flags flags);
	void parse_named_const(const string &name, const Class *type, Class *name_space, Block *block);
	static PrimitiveOperator *which_primitive_operator(const string &name, int param_flags = 3);
	static Statement *which_statement(const string &name);

	Node *parse_operand_extension(Array<Node*> operands, Block *block);
	Array<Node*> parse_operand_extension_element(Node *operand);
	Node *parse_operand_extension_array(Node *operand, Block *block);
	Node *parse_operand_extension_call(Array<Node*> operands, Block *block);

	const Class *parse_type_extension_array(const Class *c);
	const Class *parse_type_extension_dict(const Class *c);
	const Class *parse_type_extension_pointer(const Class *c);
	Node *parse_single_func_param(Block *block);
	void parse_complete_command(Block *block);
	void parse_local_definition(Block *block, const Class *type);
	Node *parse_block(Block *parent, Block *block = nullptr);
	Node *parse_operand(Block *block, bool prefer_class = false);
	Node *parse_operand_greedy(Block *block, bool allow_tuples = false, Node *first_operand = nullptr);
	Node *parse_operand_super_greedy(Block *block);
	Node *parse_set_builder(Block *block);

	Node *parse_primitive_operator(Block *block);
	Array<Node*> parse_call_parameters(Block *block);
	Node *try_parse_format_string(Block *block, Value &v);
	Node *apply_format(Node *n, const string &fmt);
	Node *parse_statement(Block *block);
	Node *parse_for_header(Block *block);
	void post_process_for(Node *n);
	Node *parse_statement_for(Block *block);
	Node *parse_statement_while(Block *block);
	Node *parse_statement_break(Block *block);
	Node *parse_statement_continue(Block *block);
	Node *parse_statement_return(Block *block);
	Node *parse_statement_raise(Block *block);
	Node *parse_statement_try(Block *block);
	Node *parse_statement_if(Block *block);
	Node *parse_statement_pass(Block *block);
	Node *parse_statement_new(Block *block);
	Node *parse_statement_delete(Block *block);
	Node *parse_statement_sizeof(Block *block);
	Node *parse_statement_type(Block *block);
	Node *parse_statement_str(Block *block);
	Node *parse_statement_repr(Block *block);
	Node *parse_statement_len(Block *block);
	Node *parse_statement_let(Block *block);
	Node *parse_statement_map(Block *block);
	Node *parse_statement_lambda(Block *block);
	Node *parse_statement_sorted(Block *block);
	Node *parse_statement_dyn(Block *block);
	Node *parse_statement_call(Block *block);


	void auto_implement_add_virtual_table(Node *self, Function *f, const Class *t);
	void auto_implement_add_child_constructors(Node *self, Function *f, const Class *t);
	void auto_implement_constructor(Function *f, const Class *t, bool allow_parent_constructor);
	void auto_implement_destructor(Function *f, const Class *t);
	void auto_implement_assign(Function *f, const Class *t);
	void auto_implement_array_clear(Function *f, const Class *t);
	void auto_implement_array_resize(Function *f, const Class *t);
	void auto_implement_array_add(Function *f, const Class *t);
	void auto_implement_array_remove(Function *f, const Class *t);
	void auto_implement_functions(const Class *t);
	
	SyntaxTree *tree;
	Function *cur_func;
	ExpressionBuffer Exp;

	int for_index_count;

	int parser_loop_depth;
};

} /* namespace Kaba */

#endif /* SRC_LIB_KABA_SYNTAX_PARSER_H_ */
