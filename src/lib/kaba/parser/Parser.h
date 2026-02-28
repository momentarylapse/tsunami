/*
 * Parser.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_PARSER_H_
#define SRC_LIB_KABA_PARSER_PARSER_H_

#include "lexical.h"
#include "AbstractParser.h"
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

class Parser : public AbstractParser {
public:
	explicit Parser(SyntaxTree *syntax);

	void parse_buffer(const string &buffer, bool just_analyse);

	void parse_legacy_macros(bool just_analyse);
	void handle_legacy_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);

	const Class *get_constant_type(const string &str);
	void get_constant_value(const string &str, Value &value);

	void parse();
	void realize_tree(shared<Node> node);
	void prerealize_all_class_names_in_block(shared<Node> node, Class *ns);
	void concretify_all_functions();
	void parse_import();
	void realize_enum(shared<Node> node, Class *_namespace);
	Class *realize_class(shared<Node>, Class *name_space, const string& name_overwrite = "");
	Class *realize_class_header(shared<Node>, Class* _namespace, int64& var_offset0, const string& name_overwrite = "");
	void post_process_newly_parsed_class(Class *c, int size);
	void skip_parse_class();
	Function *realize_function_header(shared<Node> node, const Class *default_type, Class *name_space);
	void realize_function(shared<Node> node, Class* name_space);
	void post_process_function_header(Function *f, const Array<string> &template_param_names, Class *name_space, Flags flags);
	Function* realize_lambda(shared<Node> node, Class* name_space);
	void realize_class_variable_declaration(shared<Node> node, const Class *ns, Block *block, int64 &_offset, Flags flags0 = Flags::None);
	void realize_class_use_statement(shared<Node> node, const Class *c);
	void realize_named_const(shared<Node> node, Class *name_space, Block *block);
	shared<Node> parse_and_eval_const(Block *block, const Class *type);
	shared<Node> eval_to_const(shared<Node> node, Block *block, const Class *type);

	shared<Node> parse_operand_extension(const shared_array<Node> &operands, Block *block, bool prefer_type);
	shared_array<Node> parse_operand_extension_element(shared<Node> operand);
	shared<Node> parse_operand_extension_array(shared<Node> operand, Block *block);
	shared<Node> parse_operand_extension_call(const shared_array<Node> &operands, Block *block);

	shared<Node> try_parse_format_string(Block *block, Value &v, int token_id);
	shared<Node> apply_format(shared<Node> n, const string &fmt);
	void post_process_for(shared<Node> n);

	shared<Node> parse_operand_greedy(Block *block, bool allow_tuples);


	//Context *context;
	//SyntaxTree *tree;
	Function *cur_func;
	//ExpressionBuffer &Exp;
	int next_asm_block = 0;

	Concretifier con;
	AutoImplementerInternal auto_implementer;

	bool found_dynamic_param;

	Array<Function*> functions_to_concretify;

	struct NamespaceFix {
		const Class *_class, *_namespace;
	};
	Array<NamespaceFix> restore_namespace_mapping;
};

} /* namespace kaba */

#endif /* SRC_LIB_KABA_SYNTAX_PARSER_H_ */
