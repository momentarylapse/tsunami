/*
 * Parser.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_PARSER_H_
#define SRC_LIB_KABA_PARSER_PARSER_H_

#include "lexical.h"
#include "Concretifier.h"
#include "../template/implicit.h"
#include "../syntax/SyntaxTree.h"

namespace kaba {

class Class;
class Function;
class Block;
class SyntaxTree;
class Statement;
class SpecialFunction;
class AbstractOperator;
enum class Flags;
struct CastingData;
class Context;

class Parser {
public:
	Parser(SyntaxTree *syntax);

	void parse_buffer(const string &buffer, bool just_analyse);

	void parse_macros(bool just_analyse);
	void handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);

	void do_error(const string &msg, shared<Node> node);
	void do_error(const string &msg, int token_id);
	void do_error_exp(const string &msg, int override_token_id = -1);
	void expect_no_new_line(const string &error_msg = "");
	void expect_new_line(const string &error_msg = "");
	void expect_new_line_with_indent();
	void expect_identifier(const string &identifier, const string &error_msg, bool consume = true);
	bool try_consume(const string &identifier);

	shared<Node> parse_abstract_list(Block *block);
	shared<Node> parse_abstract_dict(Block *block);

	const Class *get_constant_type(const string &str);
	void get_constant_value(const string &str, Value &value);

	void parse();
	void parse_top_level();
	void parse_all_class_names_in_block(Class *ns, int indent0);
	void parse_all_function_bodies();
	Flags parse_flags(Flags initial = Flags::NONE);
	void parse_import();
	void parse_enum(Class *_namespace);
	bool parse_class(Class *_namespace);
	Class *parse_class_header(Class *_namespace, int &offset0);
	void post_process_newly_parsed_class(Class *c, int size);
	void skip_parse_class();
	Function *parse_function_header(const Class *default_type, Class *name_space, Flags flags0);
	void post_process_function_header(Function *f, const Array<string> &template_param_names, Class *name_space, Flags flags);
	void skip_parsing_function_body(Function *f);
	void parse_abstract_function_body(Function *f);
	bool parse_abstract_function_command(Function *f, int indent0);
	const Class *parse_type(const Class *ns);
	//const Class *parse_product_type(const Class *ns);
	void parse_class_variable_declaration(const Class *ns, Block *block, int &_offset, Flags flags0 = Flags::NONE);
	void parse_class_use_statement(const Class *c);
	void parse_named_const(Class *name_space, Block *block);
	shared<Node> parse_and_eval_const(Block *block, const Class *type);
	static AbstractOperator *which_abstract_operator(const string &name, OperatorFlags param_flags = OperatorFlags::BINARY);
	static Statement *which_statement(const string &name);
	static SpecialFunction *which_special_function(const string &name);


	shared<Node> parse_abstract_operand_extension(shared<Node> operands, Block *block, bool prefer_class);
	shared<Node> parse_abstract_operand_extension_element(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_array(shared<Node> operand, Block *block);
	shared<Node> parse_abstract_operand_extension_pointer(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_reference(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_dict(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_optional(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_callable(shared<Node> operand, Block *block);
	shared<Node> parse_abstract_operand_extension_call(shared<Node> operand, Block *block);

	shared<Node> parse_operand_extension(const shared_array<Node> &operands, Block *block, bool prefer_type);
	shared_array<Node> parse_operand_extension_element(shared<Node> operand);
	shared<Node> parse_operand_extension_array(shared<Node> operand, Block *block);
	shared<Node> parse_operand_extension_call(const shared_array<Node> &operands, Block *block);

	shared<Node> parse_abstract_single_func_param(Block *block);
	void parse_abstract_complete_command(Block *block);
	shared<Node> parse_abstract_block(Block *parent, Block *block = nullptr);
	shared<Node> parse_abstract_operand(Block *block, bool prefer_class = false);
	shared<Node> parse_operand_greedy(Block *block, bool allow_tuples = false);
	shared<Node> parse_abstract_operand_greedy(Block *block, bool allow_tuples = false);
	shared<Node> parse_operand_super_greedy(Block *block);
	shared<Node> parse_abstract_set_builder(Block *block);
	shared<Node> parse_abstract_token();

	shared<Node> parse_abstract_operator(OperatorFlags param_flags);
	shared_array<Node> parse_abstract_call_parameters(Block *block);
	shared<Node> try_parse_format_string(Block *block, Value &v, int token_id);
	shared<Node> apply_format(shared<Node> n, const string &fmt);
	void post_process_for(shared<Node> n);

	shared<Node> parse_abstract_statement(Block *block);
	shared<Node> parse_abstract_for_header(Block *block);
	shared<Node> parse_abstract_statement_for(Block *block);
	shared<Node> parse_abstract_statement_while(Block *block);
	shared<Node> parse_abstract_statement_break();
	shared<Node> parse_abstract_statement_continue();
	shared<Node> parse_abstract_statement_return(Block *block);
	shared<Node> parse_abstract_statement_raise(Block *block);
	shared<Node> parse_abstract_statement_try(Block *block);
	shared<Node> parse_abstract_statement_if(Block *block);
	shared<Node> parse_abstract_statement_if_unwrap(Block *block);
	shared<Node> parse_abstract_statement_pass(Block *block);
	shared<Node> parse_abstract_statement_new(Block *block);
	shared<Node> parse_abstract_statement_delete(Block *block);
	shared<Node> parse_abstract_statement_let(Block *block);
	shared<Node> parse_abstract_statement_var(Block *block);
	shared<Node> parse_abstract_statement_lambda(Block *block);
	shared<Node> parse_abstract_statement_raw_function_pointer(Block *block);
	shared<Node> parse_abstract_statement_trust_me(Block *block);


	shared<Node> parse_abstract_special_function(Block *block, SpecialFunction *s);


	Context *context;
	SyntaxTree *tree;
	Function *cur_func;
	ExpressionBuffer &Exp;
	int next_asm_block = 0;

	Concretifier con;
	AutoImplementerInternal auto_implementer;

	int parser_loop_depth;
	bool found_dynamic_param;

	Array<Function*> function_needs_parsing;

	struct NamespaceFix {
		const Class *_class, *_namespace;
	};
	Array<NamespaceFix> restore_namespace_mapping;
};

} /* namespace kaba */

#endif /* SRC_LIB_KABA_SYNTAX_PARSER_H_ */
