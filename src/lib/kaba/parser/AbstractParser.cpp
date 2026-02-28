#include "../kaba.h"
#include "AbstractParser.h"
#include <lib/base/iter.h>
#include <lib/os/msg.h>

namespace kaba {

shared<Node> build_abstract_tuple(const Array<shared<Node>> &el);

AbstractParser::AbstractParser(SyntaxTree* t):
		Exp(t->expressions) {

	context = t->module->context;
	tree = t;
	Exp.cur_line = nullptr;
}

void AbstractParser::do_error(const string &str, shared<Node> node) {
	do_error_exp(str, node->token_id);
}

void AbstractParser::do_error(const string &str, int token_id) {
	do_error_exp(str, token_id);
}

void AbstractParser::do_error_exp(const string &str, int override_token_id) {
	if (Exp.lines.num == 0)
		throw Exception(str, "", 0, 0, tree->module);

	// what data do we have?
	int token_id = Exp.cur_token();

	// override?
	if (override_token_id >= 0)
		token_id = override_token_id;

	int physical_line = Exp.token_physical_line_no(token_id);
	int pos = Exp.token_line_offset(token_id);
	string expr = Exp.get_token(token_id);

#ifdef CPU_ARM
	msg_error(str);
#endif
	throw Exception(str, expr, physical_line, pos, tree->module);
}

void AbstractParser::expect_no_new_line(const string &error_msg) {
	if (Exp.end_of_line())
		do_error_exp(error_msg.num > 0 ? error_msg : "unexpected newline");
}

void AbstractParser::expect_new_line(const string &error_msg) {
	if (!Exp.end_of_line())
		do_error_exp(error_msg.num > 0 ? error_msg : "newline expected");
}

void AbstractParser::expect_new_line_with_indent() {
	if (!Exp.end_of_line())
		do_error_exp("newline expected");
	if (!Exp.next_line_is_indented())
		do_error_exp("additional indent expected");
}

void AbstractParser::expect_identifier(const string &identifier, const string &error_msg, bool consume) {
	if (Exp.cur != identifier)
		do_error_exp(error_msg);
	if (consume)
		Exp.next();
}

bool AbstractParser::try_consume(const string &identifier) {
	if (Exp.cur == identifier) {
		Exp.next();
		return true;
	}
	return false;
}

AbstractOperator* AbstractParser::which_abstract_operator(const string& name, OperatorFlags param_flags) {
	for (int i=0; i<(int)OperatorID::_Count_; i++)
		if ((name == abstract_operators[i].name) and ((int)param_flags == (abstract_operators[i].flags & OperatorFlags::Binary)))
			return &abstract_operators[i];

	// old hack
	if (name == "!")
		return &abstract_operators[(int)OperatorID::Negate];

	return nullptr;
}

Statement* AbstractParser::which_statement(const string& name) {
	for (auto s: Statements)
		if (name == s->name)
			return s;
	return nullptr;
}

SpecialFunction* AbstractParser::which_special_function(const string& name) {
	for (auto s: special_functions)
		if (name == s->name)
			return s;
	return nullptr;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_element(shared<Node> operand) {
	Exp.next(); // .

	auto el = new Node(NodeKind::AbstractElement, 0, common_types.unknown);
	el->token_id = Exp.cur_token();
	el->set_num_params(2);
	el->set_param(0, operand);
	el->set_param(1, parse_abstract_token());
	return el;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_definitely(shared<Node> operand) {
	auto node = new Node(NodeKind::Definitely, 0, common_types.unknown);
	node->token_id = Exp.consume_token(); // "!"
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_dict(shared<Node> operand) {
	Exp.next(); // "{"

	auto node = new Node(NodeKind::AbstractTypeDict, 0, common_types.unknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(1);
	node->set_param(0, operand);

	expect_identifier("}", "'}' expected after dict 'class{'");
	return node;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_optional(shared<Node> operand) {
	Exp.next(); // "?"

	auto node = new Node(NodeKind::AbstractTypeOptional, 0, common_types.unknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_callable(shared<Node> operand) {
	Exp.next(); // "->"

	auto node = new Node(NodeKind::AbstractTypeCallable, 0, common_types.unknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(2);
	node->set_param(0, operand);
	node->set_param(1, parse_abstract_operand());
	return node;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_pointer(shared<Node> operand) {
	auto node = new Node(NodeKind::AbstractTypeStar, 0, common_types.unknown);
	node->token_id = Exp.consume_token(); // "*"
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_reference(shared<Node> operand) {
	auto node = new Node(NodeKind::AbstractTypeReference, 0, common_types.unknown);
	node->token_id = Exp.consume_token(); // "&"
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> AbstractParser::parse_abstract_operand_extension_array(shared<Node> operand) {
	int token0 = Exp.consume_token();
	// array index...


	if (try_consume("]")) {
		auto node = new Node(NodeKind::AbstractTypeList, 0, common_types.unknown, Flags::None, token0);
		node->set_num_params(1);
		node->set_param(0, operand);
		return node;
	}

	auto parse_value_or_slice = [this] () {
		shared<Node> index;
		if (Exp.cur == ":") {
			index = add_node_const(tree->add_constant_int(0));
		} else {
			index = parse_abstract_operand_greedy();
		}
		if (try_consume(":")) {
			shared<Node> index2;
			if (Exp.cur == "]" or Exp.cur == ",") {
				index2 = add_node_const(tree->add_constant_int(DynamicArray::MAGIC_END_INDEX));
				// magic value (-_-)'
			} else {
				index2 = parse_abstract_operand_greedy();
			}
			index = add_node_slice(index, index2);
		}
		return index;
	};

	shared<Node> index = parse_value_or_slice();
	if (try_consume(",")) {
		// TODO ...more
		auto index_b = parse_abstract_operand_greedy();
		index = build_abstract_tuple({index, index_b});
	}
	expect_identifier("]", "']' expected after array index");

	return add_node_array(operand, index, common_types.unknown);
}

shared<Node> AbstractParser::parse_abstract_operand_extension_call(shared<Node> link) {

	// parse all parameters
	auto params = parse_abstract_call_parameters();

	auto node = new Node(NodeKind::AbstractCall, 0, common_types.unknown, Flags::None, link->token_id);
	node->set_num_params(params.num + 1);
	node->set_param(0, link);
	for (auto&& [i,p]: enumerate(params))
		node->set_param(i + 1, p);

	return node;
}


// find any ".", or "[...]"'s    or operators?
shared<Node> AbstractParser::parse_abstract_operand_extension(shared<Node> operand, bool prefer_class) {



#if 0
	// special
	if (/*(operands[0]->kind == NodeKind::CLASS) and*/ ((Exp.cur == "*") or (Exp.cur == "[") or (Exp.cur == "{") or (Exp.cur == "->"))) {

		if (Exp.cur == "*") {
			operand = parse_type_extension_pointer(operand);
		} else if (Exp.cur == "[") {
			t = parse_type_extension_array(t);
		} else if (Exp.cur == "{") {
			t = parse_type_extension_dict(t);
		} else if (Exp.cur == "->") {
			t = parse_type_extension_func(t, block->name_space());
		} else if (Exp.cur == ".") {
			t = parse_type_extension_child(t);
		}

		return parse_operand_extension({add_node_class(t)}, block, prefer_type);
	}
#endif

	auto no_identifier_after = [this] {
		if (Exp.almost_end_of_line())
			return true;
		string next = Exp.peek_next();
		if ((next == ",") or (next == "=") or /*(next == "[") or (next == "{") or*/ (next == "->") or (next == ")") or (next == "*"))
			return true;
		return false;
	};
	/*auto might_declare_pointer_variable = [this] {
		// a line of "int *p = ..."
		if (Exp._cur_exp != 1)
			return false;
		if (is_number(Exp.cur_line->tokens[0].name[0]))
			return false;
		return true;
	};*/

	if (Exp.cur == ".") {
		// element?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_element(operand), prefer_class);
	} else if (Exp.cur == "[") {
		// array?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_array(operand), prefer_class);
	} else if (Exp.cur == "(") {
		// call?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_call(operand), prefer_class);
	} else if (Exp.cur == "{") {
		// dict?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_dict(operand), prefer_class);
	} else if (Exp.cur == "->") {
		// A->B?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_callable(operand), true);
	} else if (Exp.cur == "!") {
		// definitely?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_definitely(operand), prefer_class);
	} else if (Exp.cur == "?") {
		// optional?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_optional(operand), true);
	/*} else if (Exp.cur == Identifier::SHARED or Exp.cur == Identifier::OWNED) {
		auto sub = operand;
		if (Exp.cur == Identifier::SHARED) {
			operand = new Node(NodeKind::ABSTRACT_TYPE_SHARED, 0, common_types.unknown);
		} else { //if (pre == Identifier::OWNED)
			operand = new Node(NodeKind::ABSTRACT_TYPE_OWNED, 0, common_types.unknown);
		}
		operand->token_id = Exp.consume_token();
		operand->set_num_params(1);
		operand->set_param(0, sub);
		return parse_abstract_operand_extension(operand, true);*/
	} else {

		if (Exp.cur == "*" and (prefer_class or no_identifier_after())) {
			// FIXME: false positives for "{{pi * 10}}"
			return parse_abstract_operand_extension(parse_abstract_operand_extension_pointer(operand), true);
		}
		if (Exp.cur == "&" and (prefer_class or no_identifier_after())) {
			return parse_abstract_operand_extension(parse_abstract_operand_extension_reference(operand), true);
		}
		// unary operator? (++,--)

		if (auto op = parse_abstract_operator(OperatorFlags::UnaryLeft)) {
			op->set_num_params(1);
			op->set_param(0, operand);
			return parse_abstract_operand_extension(op, prefer_class);
		}
		return operand;
	}

	// recursion
	return parse_abstract_operand_extension(operand, prefer_class);
}

shared_array<Node> AbstractParser::parse_abstract_call_parameters() {
	expect_identifier("(", "'(' expected in front of function parameter list");

	shared_array<Node> params;

	// list of parameters
	if (try_consume(")"))
		return params;

	bool has_named = false;
	while (true) {

		// find parameter
		if (Exp.peek_next() == "=") {
			// names parameter:  name=...
			int name_token = Exp.cur_token();
			Exp.next();
			Exp.next(); // =
			params.add(add_node_named_parameter(tree, name_token, parse_abstract_operand_greedy()));
			has_named = true;
		} else {
			if (has_named)
				do_error("can not mix named and unnamed parameters", Exp.cur_token());
			params.add(parse_abstract_operand_greedy());
		}

		if (!try_consume(",")) {
			expect_identifier(")", "',' or ')' expected after parameter for function");
			break;
		}
	}
	return params;
}



shared<Node> build_abstract_list(const Array<shared<Node>> &el) {
	auto c = new Node(NodeKind::ArrayBuilder, 0, common_types.unknown);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

shared<Node> build_abstract_dict(const Array<shared<Node>> &el) {
	auto c = new Node(NodeKind::DictBuilder, 0, common_types.unknown);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

shared<Node> build_abstract_tuple(const Array<shared<Node>> &el) {
	auto c = new Node(NodeKind::Tuple, 0, common_types.unknown);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

shared<Node> AbstractParser::parse_abstract_set_builder() {
	//Exp.next(); // [
	auto n_for = parse_abstract_for_header();

	auto n_exp = parse_abstract_operand_greedy();

	shared<Node> n_cmp;
	if (try_consume(Identifier::If))
		n_cmp = parse_abstract_operand_greedy();

	expect_identifier("]", "] expected");

	auto n = new Node(NodeKind::ArrayBuilderFor, 0, common_types.unknown);
	n->set_num_params(3);
	n->set_param(0, n_for);
	n->set_param(1, n_exp);
	n->set_param(2, n_cmp);
	return n;
}


	shared<Node> AbstractParser::parse_abstract_list() {
	shared_array<Node> el;
	if (!try_consume("]"))
		while (true) {
			el.add(parse_abstract_operand_greedy());
			if (try_consume("]"))
				break;
			expect_identifier(",", "',' or ']' expected");
		}
	return build_abstract_list(el);
}

	shared<Node> AbstractParser::parse_abstract_dict() {
	Exp.next(); // {
	shared_array<Node> el;
	if (!try_consume("}"))
		while (true) {
			el.add(parse_abstract_operand_greedy()); // key
			expect_identifier(":", "':' after key expected");
			el.add(parse_abstract_operand_greedy()); // value
			if (try_consume("}"))
				break;
			expect_identifier(",", "',' or '}' expected in dict");
		}
	return build_abstract_dict(el);
}

shared<Node> AbstractParser::parse_abstract_token() {
	return add_node_token(tree, Exp.consume_token());
}

shared<Node> AbstractParser::parse_abstract_type() {
	return parse_abstract_operand(true);
}

// minimal operand
// but with A[...], A(...) etc
shared<Node> AbstractParser::parse_abstract_operand(bool prefer_class) {
	shared<Node> operand;

	// ( -> one level down and combine commands
	if (try_consume("(")) {
		operand = parse_abstract_operand_greedy(true);
		expect_identifier(")", "')' expected");
	} else if (try_consume("&")) { // & -> address operator
		int token = Exp.cur_token();
		operand = parse_abstract_operand()->ref(common_types.unknown);
		operand->token_id = token;
	} else if (try_consume("*")) { // * -> dereference
		int token = Exp.cur_token();
		operand = parse_abstract_operand()->deref(common_types.unknown);
		operand->token_id = token;
	} else if (try_consume("[")) {
		if (Exp.cur == "for") {
			operand = parse_abstract_set_builder();
		} else {
			operand = parse_abstract_list();
		}
	} else if (Exp.cur == "{") {
		operand = parse_abstract_dict();
	} else if ([[maybe_unused]] auto s = which_statement(Exp.cur)) {
		operand = parse_abstract_statement();
	//} else if (auto s = which_special_function(Exp.cur)) {
	//	operand = parse_abstract_special_function(block, s);
	} else if (auto w = which_abstract_operator(Exp.cur, OperatorFlags::UnaryRight)) { // negate/not...
		operand = new Node(NodeKind::AbstractOperator, (int_p)w, common_types.unknown, Flags::None, Exp.cur_token());
		Exp.next();
		operand->set_num_params(1);
		operand->set_param(0, parse_abstract_operand_greedy(false, 13)); // allow '*', don't allow '+'
	} else {
		operand = parse_abstract_token();
	}

	if (Exp.end_of_line())
		return operand;

	//return operand;
	// resolve arrays, structures, calls...
	return parse_abstract_operand_extension(operand, prefer_class);
}

// no type information
shared<Node> AbstractParser::parse_abstract_operator(OperatorFlags param_flags, int min_op_level) {
	auto op = which_abstract_operator(Exp.cur, param_flags);
	if (!op or op->level < min_op_level)
		return nullptr;

	auto cmd = new Node(NodeKind::AbstractOperator, (int_p)op, common_types.unknown);
	cmd->token_id = Exp.consume_token();

	return cmd;
}

void get_comma_range(shared_array<Node> &_operators, int mio, int &first, int &last) {
	first = mio;
	last = mio+1;
	for (int i=mio; i>=0; i--) {
		if (_operators[i]->as_abstract_op()->id == OperatorID::Comma)
			first = i;
		else
			break;
	}
	for (int i=mio; i<_operators.num; i++) {
		if (_operators[i]->as_abstract_op()->id == OperatorID::Comma)
			last = i+1;
		else
			break;
	}
}

// A+B*C  ->  +(A, *(B, C))
shared<Node> digest_operator_list_to_tree(shared_array<Node> &operands, shared_array<Node> &operators) {
	while (operators.num > 0) {

		// find the most important operator (mio)
		int mio = 0;
		int level_max = -10000;
		for (int i=0;i<operators.num;i++) {
			if (operators[i]->as_abstract_op()->level > level_max) {
				mio = i;
				level_max = operators[i]->as_abstract_op()->level;
			}
		}
		auto op_no = operators[mio]->as_abstract_op();

		if (op_no->id == OperatorID::Comma) {
			int first = mio, last = mio;
			get_comma_range(operators, mio, first, last);
			auto n = build_abstract_tuple(operands.sub_ref(first, last + 1));
			operands[first] = n;
			for (int i=last-1; i>=first; i--) {
				operators.erase(i);
				//op_tokens.erase(i);
				operands.erase(i + 1);
			}
			continue;
		}

		// link it
		operators[mio]->set_num_params(2);
		operators[mio]->set_param(0, operands[mio]);
		operators[mio]->set_param(1, operands[mio+1]);

		// remove from list
		operands[mio] = operators[mio];
		operators.erase(mio);
		//op_tokens.erase(mio);
		operands.erase(mio + 1);
	}
	return operands[0];
}

// greedily parse AxBxC...(operand, operator)
shared<Node> AbstractParser::parse_abstract_operand_greedy(bool allow_tuples, int min_op_level) {
	shared_array<Node> operands;
	shared_array<Node> operators;

	// find the first operand
	auto first_operand = parse_abstract_operand();
	if (config.verbose) {
		msg_write("---first:");
		first_operand->show();
	}
	operands.add(first_operand);

	// find pairs of operators and operands
	while (true) {
		if (!allow_tuples and Exp.cur == ",")
			break;
		if (auto op = parse_abstract_operator(OperatorFlags::Binary, min_op_level)) {
			operators.add(op);
			expect_no_new_line("unexpected end of line after operator");
			operands.add(parse_abstract_operand());
		} else {
			break;
		}
	}

	return digest_operator_list_to_tree(operands, operators);
}

// TODO later...
//  J make Node=Block
//  J for with p[0]=set init
//  * for_var in for "Block"

// Node structure
//  p = [VAR, START, STOP, STEP]
//  p = [REF_VAR, KEY, ARRAY]
shared<Node> AbstractParser::parse_abstract_for_header() {

	// variable name
	int token0 = Exp.consume_token(); // for
	auto flags = parse_flags(Flags::None);
	auto var = parse_abstract_token();

	// index
	shared<Node> key;
	if (try_consume("=>")) {
		// key => value
		key = var;
		var = parse_abstract_token();
	}


	expect_identifier(Identifier::In, "'in' expected after variable in 'for ...'");

	// first value/array
	auto val0 = parse_abstract_operand_greedy();


	if (try_consume(":")) {
		// range

		if (key)
			do_error("no key=>value allowed in START:END for loop", key);

		auto val1 = parse_abstract_operand_greedy();

		shared<Node> val_step;
		if (try_consume(":"))
			val_step = parse_abstract_operand_greedy();

		auto cmd_for = add_node_statement(StatementID::ForRange, token0, common_types.unknown);
		cmd_for->set_param(0, var);
		cmd_for->set_param(1, val0);
		cmd_for->set_param(2, val1);
		cmd_for->set_param(3, val_step);
		//cmd_for->set_uparam(4, loop_block);

		return cmd_for;

	} else {
		// array

		auto array = val0;


		auto cmd_for = add_node_statement(StatementID::ForContainer, token0, common_types.unknown);
		// [REF_VAR (token), KEY? (token), ARRAY, BLOCK]

		cmd_for->set_param(0, var);
		cmd_for->set_param(1, key);
		cmd_for->set_param(2, array);
		//cmd_for->set_uparam(3, loop_block);

		flags_set(cmd_for->flags, flags);
		return cmd_for;
	}
}

// Node structure
shared<Node> AbstractParser::parse_abstract_statement_for() {
	int ind0 = Exp.cur_line->indent;

	auto cmd_for = parse_abstract_for_header();

	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	parser_loop_depth ++;
	auto loop_block = parse_abstract_block();
	parser_loop_depth --;

	cmd_for->set_param(cmd_for->params.num - 1, loop_block);

	// else?
	int token_id = Exp.cur_token();
	Exp.next_line();
	if (!Exp.end_of_file() and (Exp.cur == Identifier::Else) and (Exp.cur_line->indent >= ind0)) {
		Exp.next();
		// ...block
		expect_new_line_with_indent();
		Exp.next_line();
		cmd_for->params.add(parse_abstract_block());
	} else {
		Exp.jump(token_id);
	}

	//post_process_for(cmd_for);

	return cmd_for;
}

// Node structure
//  p[0]: test
//  p[1]: loop block
shared<Node> AbstractParser::parse_abstract_statement_while() {
	int token0 = Exp.consume_token(); // "while"
	auto cmd_cmp = parse_abstract_operand_greedy();

	auto cmd_while = add_node_statement(StatementID::While, token0, common_types.unknown);
	cmd_while->set_param(0, cmd_cmp);

	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	parser_loop_depth ++;
	cmd_while->set_num_params(2);
	cmd_while->set_param(1, parse_abstract_block());
	parser_loop_depth --;
	return cmd_while;
}

// [INPUT, PATTERN1, RESULT1, ...]
shared<Node> AbstractParser::parse_abstract_statement_match() {
	int token0 = Exp.consume_token(); // "match"
	auto cmd_input = parse_abstract_operand_greedy();

	auto cmd_match = add_node_statement(StatementID::Match, token0, common_types.unknown);
	cmd_match->set_param(0, cmd_input);

	expect_new_line_with_indent();
	int indent = Exp.next_line_indent();
	Exp.next_line();
	for (int i=0;; i++) {

		// pattern
		shared<Node> pattern;
		if (try_consume(Identifier::Else) or try_consume("*")) {
			if (i == 0)
				do_error("'match' must not begin with the default pattern", Exp.cur_token() - 1);
			pattern = add_node_statement(StatementID::Pass);
		} else {
			pattern = parse_abstract_operand();
		}

		// =>
		expect_identifier("=>", "'=>' expected after 'match' pattern");

		// result
		shared<Node> result;
		if (Exp.end_of_line()) {
			Exp.next_line();
			// indented block
			result = parse_abstract_block();
		} else {
			// single expression
			result = parse_abstract_operand_greedy();
		}

		cmd_match->set_num_params(3 + 2*i);
		cmd_match->set_param(1 + 2*i, pattern);
		cmd_match->set_param(2 + 2*i, result);

		if (Exp.next_line_indent() < indent)
			break;
		Exp.next_line();
		if (pattern->kind == NodeKind::Statement)
			do_error("no more 'match' patterns allowed after default pattern", Exp.cur_token());
	}

	return cmd_match;
}

shared<Node> AbstractParser::parse_abstract_statement_break() {
	if (parser_loop_depth == 0)
		do_error_exp("'break' only allowed inside a loop");
	int token0 = Exp.consume_token(); // "break"
	return add_node_statement(StatementID::Break, token0);
}

shared<Node> AbstractParser::parse_abstract_statement_continue() {
	if (parser_loop_depth == 0)
		do_error_exp("'continue' only allowed inside a loop");
	int token0 = Exp.consume_token(); // "continue"
	return add_node_statement(StatementID::Continue, token0);
}

// Node structure
//  p[0]: value (if not void)
shared<Node> AbstractParser::parse_abstract_statement_return() {
	int token0 = Exp.consume_token(); // "return"
	auto cmd = add_node_statement(StatementID::Return, token0, common_types.unknown);
	if (Exp.end_of_line()) {
		cmd->set_num_params(0);
	} else {
		cmd->set_num_params(1);
		cmd->set_param(0, parse_abstract_operand_greedy(true));
	}
	expect_new_line();
	return cmd;
}

// IGNORE!!! raise() is a function :P
shared<Node> AbstractParser::parse_abstract_statement_raise() {
	throw "jhhhh";
#if 0
	Exp.next();
	auto cmd = add_node_statement(StatementID::RAISE);

	auto cmd_ex = check_param_link(parse_operand_greedy(block), TypeExceptionP, Identifier::RAISE, 0);
	cmd->set_num_params(1);
	cmd->set_param(0, cmd_ex);

	/*if (block->function->return_type == common_types._void) {
		cmd->set_num_params(0);
	} else {
		auto cmd_value = CheckParamLink(GetCommand(block), block->function->return_type, Identifier::RETURN, 0);
		cmd->set_num_params(1);
		cmd->set_param(0, cmd_value);
	}*/
	expect_new_line();
	return cmd;
#endif
	return nullptr;
}

// Node structure
//  p[0]: try block
//  p[1]: statement except (with type of Exception filter...)
//  p[2]: except block
shared<Node> AbstractParser::parse_abstract_statement_try() {
	int ind = Exp.cur_line->indent;
	int token0 = Exp.consume_token(); // "try"
	auto cmd_try = add_node_statement(StatementID::Try, token0, common_types.unknown);
	cmd_try->set_num_params(1);
	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	cmd_try->set_param(0, parse_abstract_block());
	token0 = Exp.cur_token();
	Exp.next_line();

	int num_excepts = 0;

	// except?
	while (!Exp.end_of_file() and (Exp.cur == Identifier::Except) and (Exp.cur_line->indent == ind)) {
		int token1 = Exp.consume_token(); // "except"

		auto cmd_ex = add_node_statement(StatementID::Except, token1, common_types.unknown);

		if (!Exp.end_of_line()) {
			auto ex_type = parse_abstract_operand(true); // type
			if (!ex_type)
				do_error_exp("Exception class expected");
			cmd_ex->params.add(ex_type);
			if (!Exp.end_of_line()) {
				expect_identifier(Identifier::As, "'as' expected");
				cmd_ex->params.add(parse_abstract_token()); // var name
			}
		}

		// ...block
		expect_new_line_with_indent();
		Exp.next_line();

		auto cmd_ex_block = parse_abstract_block();

		num_excepts ++;
		cmd_try->set_num_params(1 + num_excepts * 2);
		cmd_try->set_param(num_excepts*2 - 1, cmd_ex);
		cmd_try->set_param(num_excepts*2, cmd_ex_block);

		token0 = Exp.cur_token();
		Exp.next_line();
	}

	Exp.jump(token0); // undo next_line()
	return cmd_try;
}

// Node structure (IF):
//  p[0]: test
//  p[1]: true block
// [p[2]: false block]
shared<Node> AbstractParser::parse_abstract_statement_if() {
	int ind0 = Exp.cur_line->indent;
	int token0 = Exp.consume_token(); // "if"

	bool is_compiletime = try_consume(Identifier::Compiletime);

	auto cmd_cmp = parse_abstract_operand_greedy();

	auto cmd_if = add_node_statement(is_compiletime ? StatementID::IfCompiletime : StatementID::If, token0, common_types.unknown);
	cmd_if->set_param(0, cmd_cmp);
	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	cmd_if->set_param(1, parse_abstract_block());

	// else?
	int token_id = Exp.cur_token();
	Exp.next_line();
	if (!Exp.end_of_file() and (Exp.cur == Identifier::Else) and (Exp.cur_line->indent >= ind0)) {
		cmd_if->set_num_params(3);
		Exp.next();
		// iterative if
		if (Exp.cur == Identifier::If) {
			// sub-if's in a new block
			auto cmd_block = add_node_block(nullptr, common_types.unknown);
			cmd_if->set_param(2, cmd_block);
			// parse the next if
			parse_abstract_complete_command_into_block(cmd_block.get());
			return cmd_if;
		}
		// ...block
		expect_new_line_with_indent();
		Exp.next_line();
		cmd_if->set_param(2, parse_abstract_block());
	} else {
		Exp.jump(token_id);
	}
	return cmd_if;
}

shared<Node> AbstractParser::parse_abstract_statement_pass() {
	int token0 = Exp.consume_token(); // "pass"
	expect_new_line();

	return add_node_statement(StatementID::Pass, token0);
}

// Node structure
//  type: class
//  p[0]: call to constructor (optional)
shared<Node> AbstractParser::parse_abstract_statement_new() {
	const int token0 = Exp.consume_token(); // "new"
	const auto flags = parse_flags();
	auto cmd = add_node_statement(StatementID::New, token0, common_types.unknown);
	cmd->set_param(0, parse_abstract_operand());
	flags_set(cmd->flags, flags);
	return cmd;
}

// Node structure
//  p[0]: operand
shared<Node> AbstractParser::parse_abstract_statement_delete() {
	int token0 = Exp.consume_token(); // "del"
	auto cmd = add_node_statement(StatementID::Delete, token0, common_types.unknown);
	cmd->set_param(0, parse_abstract_operand());
	return cmd;
}

shared<Node> AbstractParser::parse_abstract_single_func_param() {
	string func_name = Exp.get_token(Exp.cur_token() - 1);
	expect_identifier("(", format("'(' expected after '%s'", func_name));
	auto n = parse_abstract_operand_greedy();
	expect_identifier(")", format("')' expected after parameter of '%s'", func_name));
	return n;
}

// local (variable) definitions...
shared<Node> AbstractParser::parse_abstract_statement_let() {
	do_error_exp("'let' is deprecated, will change it's meaning soon...");
	return nullptr;
}

shared_array<Node> parse_comma_sep_token_list(AbstractParser *p) {
	shared_array<Node> names;

	names.add(p->parse_abstract_token());

	while (p->try_consume(","))
		names.add(p->parse_abstract_token());

	return names;
}

// local (variable) definitions...
shared<Node> AbstractParser::parse_abstract_statement_var() {
	auto flags = Flags::Mutable;
	if (Exp.cur == Identifier::Let)
		flags = Flags::None;
	Exp.next(); // "var"/"let"

	// tuple "var (x,y) = ..."
	if (try_consume("(")) {
		auto names = parse_comma_sep_token_list(this);
		expect_identifier(")", "')' expected after tuple declaration");

		expect_identifier("=", "'=' expected after tuple declaration", false);

		auto tuple = build_abstract_tuple(names);

		auto assign = parse_abstract_operator(OperatorFlags::Binary);

		auto rhs = parse_abstract_operand_greedy(true);

		assign->set_num_params(2);
		assign->set_param(0, tuple);
		assign->set_param(1, rhs);

		auto node = new Node(NodeKind::AbstractVar, 0, common_types.unknown, flags);
		node->set_num_params(3);
		//node->set_param(0, type); // no type
		node->set_param(1, cp_node(tuple));
		node->set_param(2, assign);
		expect_new_line();
		return node;
	}

	auto names = parse_comma_sep_token_list(this);
	shared<Node> type;

	// explicit type?
	if (try_consume(":")) {
		type = parse_abstract_operand(true);
	} else {
		expect_identifier("=", "':' or '=' expected after 'var' declaration", false);
	}

	if (Exp.cur == "=") {
		if (names.num != 1)
			do_error_exp(format("'var' declaration with '=' only allowed with a single variable name, %d given", names.num));

		auto assign = parse_abstract_operator(OperatorFlags::Binary);

		auto rhs = parse_abstract_operand_greedy(true);

		assign->set_num_params(2);
		assign->set_param(0, names[0]);
		assign->set_param(1, rhs);

		auto node = new Node(NodeKind::AbstractVar, 0, common_types.unknown, flags);
		node->set_num_params(3);
		node->set_param(0, type); // type
		node->set_param(1, names[0]->shallow_copy()); // name
		node->set_param(2, assign);
		expect_new_line();
		return node;
	}

	expect_new_line();

	/*if (names.num > 1)
		do_error("FIXME allow multi var statement", Exp.cur_token());*/

	auto group = new Node(NodeKind::Group, 0, common_types.unknown);

	for (auto &n: names) {
		auto node = new Node(NodeKind::AbstractVar, 0, common_types.unknown, flags);
		node->set_num_params(2);
		node->set_param(0, type); // type
		node->set_param(1, n); // name
		group->add(node);
		//return node;
	}
	return group;//add_node_statement(StatementID::Pass);
}

shared<Node> AbstractParser::parse_abstract_statement_lambda() {
	auto n = parse_abstract_function_header(Flags::Static);

	// lambda body
	if (Exp.end_of_line()) {
		// indented block
		Exp.next_line();
		n->set_param(4, parse_abstract_block());
	} else {
		// single expression
		auto b = add_node_block(nullptr, common_types.unknown);
		b->add(parse_abstract_operand_greedy());
		n->set_param(4, b);
	}

	auto node = add_node_statement(StatementID::Lambda, n->token_id, common_types.unknown);
	node->set_num_params(2);
	node->set_param(1, n);

	return node;
}

shared<Node> AbstractParser::parse_abstract_statement_raw_function_pointer() {
	int token0 = Exp.consume_token(); // "raw_function_pointer"
	auto node = add_node_statement(StatementID::RawFunctionPointer, token0, common_types.unknown);
	node->set_param(0, parse_abstract_single_func_param());
	return node;
}

shared<Node> AbstractParser::parse_abstract_statement_trust_me() {
	[[maybe_unused]] int token0 = Exp.consume_token(); // "trust_me"
	auto node = add_node_statement(StatementID::TrustMe, token0, common_types.unknown);
	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	node->set_param(0, parse_abstract_block());
	//flags_set(node->flags, Flags::TrustMe);
	return node;

	/*expect_new_line_with_indent();
	Exp.next_line();
	auto b = parse_abstract_block();
	flags_set(b->flags, Flags::TrustMe);
	return b;*/
}

shared<Node> AbstractParser::parse_abstract_statement() {
	if (Exp.cur == Identifier::For) {
		return parse_abstract_statement_for();
	} else if (Exp.cur == Identifier::While) {
		return parse_abstract_statement_while();
 	} else if (Exp.cur == Identifier::Break) {
 		return parse_abstract_statement_break();
	} else if (Exp.cur == Identifier::Continue) {
		return parse_abstract_statement_continue();
	} else if (Exp.cur == Identifier::Return) {
		return parse_abstract_statement_return();
	//} else if (Exp.cur == Identifier::RAISE) {
	//	ParseStatementRaise();
	} else if (Exp.cur == Identifier::Try) {
		return parse_abstract_statement_try();
	} else if (Exp.cur == Identifier::If) {
		return parse_abstract_statement_if();
	} else if (Exp.cur == Identifier::Pass) {
		return parse_abstract_statement_pass();
	} else if (Exp.cur == Identifier::New) {
		return parse_abstract_statement_new();
	} else if (Exp.cur == Identifier::Delete) {
		return parse_abstract_statement_delete();
	} else if (Exp.cur == Identifier::Let or Exp.cur == Identifier::Var) {
		return parse_abstract_statement_var();
	} else if (Exp.cur == Identifier::Lambda or Exp.cur == Identifier::Func) {
		return parse_abstract_statement_lambda();
	} else if (Exp.cur == Identifier::RawFunctionPointer) {
		return parse_abstract_statement_raw_function_pointer();
	} else if (Exp.cur == Identifier::TrustMe) {
		return parse_abstract_statement_trust_me();
	} else if (Exp.cur == Identifier::Match) {
		return parse_abstract_statement_match();
	}
	do_error_exp("unhandled statement: " + Exp.cur);
	return nullptr;
}

// unused
shared<Node> AbstractParser::parse_abstract_special_function(SpecialFunction *s) {
	int token0 = Exp.consume_token(); // name

	// no call, just the name
	if (Exp.cur != "(") {
		auto node = add_node_special_function_name(s->id, token0, common_types.special_function_ref);
		node->set_num_params(0);
		return node;
	}

	auto node = add_node_special_function_call(s->id, token0, common_types.unknown);
	auto params = parse_abstract_call_parameters();
	node->params = params;
	if (params.num < s->min_params or params.num > s->max_params) {
		if (s->min_params == s->max_params)
			do_error_exp(format("%s() expects %d parameters", s->name, s->min_params));
		else
			do_error_exp(format("%s() expects %d-%d parameters", s->name, s->min_params, s->max_params));
	}
	/*node->set_param(0, params[0]);
	if (params.num >= 2) {
		node->set_param(1, params[1]);
	} else {
		// empty string
		node->set_param(1, add_node_const(tree->add_constant(common_types.string)));
	}*/
	return node;
}

shared<Node> AbstractParser::parse_abstract_block() {
	int indent0 = Exp.cur_line->indent;

	auto block = add_node_block(nullptr, common_types.unknown);

	while (!Exp.end_of_file()) {

		parse_abstract_complete_command_into_block(block.get());

		if (Exp.next_line_indent() < indent0)
			break;
		Exp.next_line();
	}

	return block;
}

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void AbstractParser::parse_abstract_complete_command_into_block(Node *block) {
	// beginning of a line!

	//bool is_type = tree->find_root_type_by_name(Exp.cur, block->name_space(), true);

	// assembler block
	if (try_consume("-asm-")) {
		block->add(f_add_asm_block());
	} else {
		// commands (the actual code!)
		block->add( parse_abstract_operand_greedy(true));
	}

	expect_new_line();
}


shared<Node> AbstractParser::parse_abstract_enum() {
	Exp.next(); // 'enum'

	expect_no_new_line("name expected (anonymous enum is deprecated)");

	auto node = new Node(NodeKind::AbstractEnum, 0, common_types.unknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(1);

	// class name
	node->set_param(0, parse_abstract_token());

	// as shared|@noauto
	if (try_consume(Identifier::As))
		node->flags = parse_flags();

	expect_new_line_with_indent();
	Exp.next_line();
	int indent0 = Exp.cur_line->indent;

	while (!Exp.end_of_file()) {
		while (!Exp.end_of_line()) {
			node->add(parse_abstract_token());

			// explicit value
			if (try_consume("=")) {
				expect_no_new_line();
				node->add(parse_abstract_operand_greedy());
			} else {
				node->add(nullptr);
			}

			if (try_consume(Identifier::As)) {
				expect_no_new_line();
				node->add(parse_abstract_operand());
			} else {
				node->add(nullptr);
			}

			if (Exp.end_of_line())
				break;
			expect_identifier(",", "',' or newline expected after enum definition");
			expect_no_new_line();
		}
		if (Exp.next_line_indent() < indent0)
			break;
		Exp.next_line();
	}

	return node;
}

// [METACLASS, NAME, PARENT, TEMPLATEPARAM] + flags
shared<Node> AbstractParser::parse_abstract_class_header() {
	auto node = new Node(NodeKind::AbstractClass, 0, common_types.unknown, Flags::None, Exp.cur_token());
	node->set_num_params(4);

	// metaclass
	node->set_param(0, parse_abstract_token()); // class/struct/interface

	if (try_consume(Identifier::Override))
		flags_set(node->flags, Flags::Override);

	// name
	string name = Exp.cur;
	// TODO check name validity
	node->set_param(1, parse_abstract_token());

	// template?
	Array<string> template_param_names;
	if (try_consume("[")) {
		flags_set(node->flags, Flags::Template);
		while (true) {
			node->set_param(3, parse_abstract_token());
			if (!try_consume(","))
				break;
			do_error("template classes only allowed with single parameter", Exp.cur_token());
		}
		expect_identifier("]", "']' expected after template parameter");
	}

	// parent class / interface
	if (try_consume(Identifier::Extends))
		node->set_param(2, parse_abstract_type());
	else if (try_consume(Identifier::Implements))
		node->set_param(2, parse_abstract_type());

	// as shared|@noauto
	Flags explicit_flags = Flags::None;
	if (try_consume(Identifier::As))
		flags_set(node->flags, parse_flags(explicit_flags));

	expect_new_line();

	return node;
}

shared<Node> AbstractParser::parse_abstract_class() {
	int indent0 = Exp.cur_line->indent;
	auto node = parse_abstract_class_header();
	string meta = node->params[0]->as_token();


	// body
	while (!Exp.end_of_file()) {
		Exp.next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
		if (Exp.end_of_file())
			break;

		if (Exp.cur == Identifier::Enum) {
			node->add(parse_abstract_enum());
		} else if ((Exp.cur == Identifier::Class) or (Exp.cur == Identifier::Struct) or (Exp.cur == Identifier::Interface) or (Exp.cur == Identifier::Namespace)) {
			auto n = parse_abstract_class();
			node->add(n);
		} else if (Exp.cur == Identifier::Func) {
			auto flags = Flags::None;
			if (meta == "interface")
				flags_set(flags, Flags::Virtual);
			if (meta == "namespace")
				flags_set(flags, Flags::Static);
			node->add(parse_abstract_function(flags));
		} else if ((Exp.cur == Identifier::Const) or (Exp.cur == Identifier::Let)) {
			node->add(parse_abstract_named_const());
		} else if (Exp.cur == Identifier::Var) {
			auto nodes = parse_abstract_variable_declaration();
			for (auto n: weak(nodes))
				node->add(n);
		} else if (Exp.cur == Identifier::Use) {
			node->add(parse_abstract_class_use_statement());
		} else {
			auto nodes = parse_abstract_variable_declaration();
			for (auto n: weak(nodes))
				node->add(n);
		}
	}
	Exp.rewind();

	return node;
}

shared<Node> AbstractParser::parse_abstract_named_const() {
	Exp.next(); // 'const' / 'let'

	auto node = new Node(NodeKind::AbstractLet, 0, common_types.unknown);
	node->set_num_params(3);
	node->set_param(0, parse_abstract_token());
	node->token_id = node->params[0]->token_id;

	if (try_consume(":"))
		node->set_param(1, parse_abstract_type());

	expect_identifier("=", "'=' expected after const name");
	node->set_param(2, parse_abstract_operand_greedy());

	expect_new_line();
	return node;
}

// [[NAME, TYPE, VALUE], ...]
shared_array<Node> AbstractParser::parse_abstract_variable_declaration(Flags flags0) {
	shared_array<Node> nodes;
	shared<Node> type, value;

	try_consume("var");
	flags_set(flags0, Flags::Mutable);

	Flags flags = parse_flags(flags0);

	auto names = parse_comma_sep_token_list(this);

	// explicit type?
	if (try_consume(":")) {
		type = parse_abstract_type();
	} else {
		expect_identifier("=", "':' or '=' expected after 'var' declaration", false);
	}

	if (try_consume("=")) {

		//if (names.num != 1)
		//	do_error(format("'var' declaration with '=' only allowed with a single variable name, %d given", names.num));

		value = parse_abstract_operand_greedy();
	}

	expect_new_line();

	for (auto& name: names) {
		auto node = new Node(NodeKind::AbstractVar, 0, common_types.unknown, flags, name->token_id);
		node->set_num_params(3);
		node->set_param(0, name);
		node->set_param(1, type);
		node->set_param(2, value);

		nodes.add(node);
	}

	return nodes;
}

shared<Node> AbstractParser::parse_abstract_class_use_statement() {
	Exp.next(); // "use"
	auto node = new Node(NodeKind::AbstractUseClassElement, 0, common_types.unknown, Flags::None, Exp.cur_token());
	node->add(parse_abstract_token());
	expect_new_line();
	return node;
}

// [NAME?, RETURN?, [PARAMS]?, [TEMPLATEARGS]?, BLOCK]
shared<Node> AbstractParser::parse_abstract_function_header(Flags flags0) {
	auto node = new Node(NodeKind::AbstractFunction, 0, common_types.unknown, flags0);
	node->token_id = Exp.cur_token();
	node->set_num_params(5);

	Exp.next(); // "func"

	node->flags = parse_flags(node->flags);

	// name?
	if (Exp.cur == "(" or Exp.cur == "[") {
		// lambda -> generate name later
	} else {
		node->set_param(0, parse_abstract_token());
	}

	// template?
	Array<string> template_param_names;
	if (try_consume("[")) {
		auto tnode = new Node(NodeKind::AbstractTypeList, 0, common_types.unknown);
		tnode->params = parse_comma_sep_token_list(this);
		expect_identifier("]", "']' expected after template parameter");
		node->set_param(3, tnode);
		flags_set(node->flags, Flags::Template);
	}

	// parameter list
	expect_identifier("(", "'(' expected after function name");
	auto pnode = new Node(NodeKind::AbstractTypeList, 0, common_types.unknown);
	if (!try_consume(")")) {
		int nn = 0;
		while (true) {
			nn ++;
			pnode->params.resize(nn * 3);
			// like variable definitions

			auto param_flags = parse_flags();

			pnode->set_param(nn*3-3, parse_abstract_token());
			pnode->params[nn*3-3]->flags = param_flags;

			//expect_identifier(":", "':' expected after parameter name");

			// type of parameter variable
			if (try_consume(":"))
				pnode->set_param(nn*3-2, parse_abstract_operand(true));

			// default parameter?
			if (try_consume("="))
				pnode->set_param(nn*3-1, parse_abstract_operand_greedy(false, 1));

			if (!pnode->params[nn*3-2] and !pnode->params[nn*3-1])
				do_error("':' or '=' expected after parameter name", Exp.cur_token());

			if (try_consume(")"))
				break;

			expect_identifier(",", "',' or ')' expected after parameter");
		}
	}
	node->set_param(2, pnode);

	// return type
	if (try_consume("->"))
		node->set_param(1, parse_abstract_operand(true));

	return node;
}

shared<Node> AbstractParser::parse_abstract_function(Flags flags0) {
	auto n = parse_abstract_function_header(flags0);
	expect_new_line("newline expected after parameter list");

	if (!flags_has(n->flags, Flags::Extern) and Exp.next_line_is_indented()) {
		parser_loop_depth = 0;
		Exp.next_line();

		n->set_param(4, parse_abstract_block());
	}
	return n;
}

Flags AbstractParser::parse_flags(Flags initial) {
	Flags flags = initial;

	while (true) {
		if (Exp.cur == Identifier::Static) {
			flags_set(flags, Flags::Static);
		} else if (Exp.cur == Identifier::Extern) {
			flags_set(flags, Flags::Extern);
		} else if (Exp.cur == Identifier::Const) {
			do_error("'const' deprecated", Exp.cur_token());
		} else if (Exp.cur == Identifier::Mutable) {
			flags_set(flags, Flags::Mutable);
		} else if (Exp.cur == Identifier::Virtual) {
			flags_set(flags, Flags::Virtual);
		} else if (Exp.cur == Identifier::Override) {
			flags_set(flags, Flags::Override);
		} else if (Exp.cur == Identifier::Selfref or Exp.cur == Identifier::Ref) {
			flags_set(flags, Flags::Ref);
		} else if (Exp.cur == Identifier::Globalref) {
			flags_set(flags, Flags::Globalref);
		} else if (Exp.cur == Identifier::Shared) {
			flags_set(flags, Flags::Shared);
		} else if (Exp.cur == Identifier::Owned) {
			flags_set(flags, Flags::Owned);
		} else if (Exp.cur == Identifier::Out) {
			flags_set(flags, Flags::Out);
		} else if (Exp.cur == Identifier::Throws) {
			flags_set(flags, Flags::RaisesExceptions);
		} else if (Exp.cur == Identifier::Pure) {
			flags_set(flags, Flags::Pure);
		} else if (Exp.cur == Identifier::Noauto) {
			flags_set(flags, Flags::Noauto);
		} else if (Exp.cur == Identifier::Noframe) {
			flags_set(flags, Flags::Noframe);
		} else {
			break;
		}
		Exp.next();
	}
	return flags;
}

shared<Node> AbstractParser::parse_abstract_top_level() {
	shared<Node> node = new Node(NodeKind::AbstractRoot, 0, common_types.unknown);

	// syntax analysis

	Exp.reset_walker();

	// global definitions (enum, class, variables and functions)
	while (!Exp.end_of_file()) {

		if (Exp.cur == Identifier::Use) {
			// TODO make abstract...?
			f_parse_import();

		// enum
		} else if (Exp.cur == Identifier::Enum) {
			node->add(parse_abstract_enum());

		// class
		} else if ((Exp.cur == Identifier::Class) or (Exp.cur == Identifier::Struct) or (Exp.cur == Identifier::Interface) or (Exp.cur == Identifier::Namespace)) {
			node->add(parse_abstract_class());

		// func
		} else if (Exp.cur == Identifier::Func) {
			node->add(parse_abstract_function(Flags::Static));

		// macro
		} else if (Exp.cur == Identifier::Macro) {
			node->add(parse_abstract_function(Flags::Static | Flags::Macro));

		} else if ((Exp.cur == Identifier::Const) or (Exp.cur == Identifier::Let)) {
			node->add(parse_abstract_named_const());

		} else if (Exp.cur == Identifier::Var) {
			auto nodes = parse_abstract_variable_declaration();
			for (auto n: weak(nodes))
				node->add(n);

		} else {
			do_error_exp("unknown top level definition");
		}
		if (!Exp.end_of_file())
			Exp.next_line();
	}
	return node;
}

} // kaba