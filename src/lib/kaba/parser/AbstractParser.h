//
// Created by michi on 2/16/26.
//

#ifndef KABA_ABSTRACTPARSER_H
#define KABA_ABSTRACTPARSER_H

namespace kaba {

class ExpressionBuffer;
class Context;
class SyntaxTree;

class AbstractParser {
public:
	explicit AbstractParser(SyntaxTree* tree);

	void do_error(const string &msg, shared<Node> node);
	void do_error(const string &msg, int token_id);
	void do_error_exp(const string &msg, int override_token_id = -1);
	void expect_no_new_line(const string &error_msg = "");
	void expect_new_line(const string &error_msg = "");
	void expect_new_line_with_indent();
	void expect_identifier(const string &identifier, const string &error_msg, bool consume = true);
	bool try_consume(const string &identifier);

	Flags parse_flags(Flags initial = Flags::None);

	shared<Node> parse_abstract_list();
	shared<Node> parse_abstract_dict();
	shared<Node> parse_abstract_top_level();
	shared<Node> parse_abstract_enum();
	shared<Node> parse_abstract_class();
	shared<Node> parse_abstract_class_header();
	shared<Node> parse_abstract_function(Flags flags0);
	shared<Node> parse_abstract_function_header(Flags flags0);
	shared_array<Node> parse_abstract_variable_declaration(Flags flags0 = Flags::None);
	shared<Node> parse_abstract_class_use_statement();
	shared<Node> parse_abstract_named_const();

	shared<Node> parse_abstract_operand_extension(shared<Node> operands, bool prefer_class);
	shared<Node> parse_abstract_operand_extension_element(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_definitely(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_array(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_pointer(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_reference(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_dict(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_optional(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_callable(shared<Node> operand);
	shared<Node> parse_abstract_operand_extension_call(shared<Node> operand);

	shared<Node> parse_abstract_single_func_param();
	void parse_abstract_complete_command_into_block(Node* block);
	shared<Node> parse_abstract_block();
	shared<Node> parse_abstract_operand(bool prefer_class = false);
	shared<Node> parse_operand_greedy(Block *block, bool allow_tuples = false);
	shared<Node> parse_abstract_operand_greedy(bool allow_tuples = false, int min_op_level = -999);
	shared<Node> parse_abstract_set_builder();
	shared<Node> parse_abstract_token();
	shared<Node> parse_abstract_type();

	shared<Node> parse_abstract_operator(OperatorFlags param_flags, int min_op_level = -999);
	shared_array<Node> parse_abstract_call_parameters();
	shared<Node> parse_abstract_special_function(SpecialFunction *s);

	shared<Node> parse_abstract_statement();
	shared<Node> parse_abstract_for_header();
	shared<Node> parse_abstract_statement_for();
	shared<Node> parse_abstract_statement_while();
	shared<Node> parse_abstract_statement_break();
	shared<Node> parse_abstract_statement_continue();
	shared<Node> parse_abstract_statement_return();
	shared<Node> parse_abstract_statement_raise();
	shared<Node> parse_abstract_statement_try();
	shared<Node> parse_abstract_statement_if();
	shared<Node> parse_abstract_statement_pass();
	shared<Node> parse_abstract_statement_new();
	shared<Node> parse_abstract_statement_delete();
	shared<Node> parse_abstract_statement_let();
	shared<Node> parse_abstract_statement_var();
	shared<Node> parse_abstract_statement_lambda();
	shared<Node> parse_abstract_statement_raw_function_pointer();
	shared<Node> parse_abstract_statement_trust_me();
	shared<Node> parse_abstract_statement_match();

	static AbstractOperator* which_abstract_operator(const string& name, OperatorFlags param_flags = OperatorFlags::Binary);
	static Statement* which_statement(const string& name);
	static SpecialFunction* which_special_function(const string &name);

	Context* context;
	SyntaxTree* tree;
	ExpressionBuffer& Exp;
	int parser_loop_depth;

	std::function<void()> f_parse_import;
	std::function<shared<Node>()> f_add_asm_block;
};

}

#endif //KABA_ABSTRACTPARSER_H