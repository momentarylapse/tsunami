#include "../kaba.h"
#include "../asm/asm.h"
#include "../../os/msg.h"
#include "../../base/set.h"
#include "../../base/algo.h"
#include "../../base/iter.h"
#include "Parser.h"
#include "template.h"


namespace kaba {

void test_node_recursion(shared<Node> root, const Class *ns, const string &message);

shared<Module> get_import(Parser *parser, const string &name, int token);

void add_enum_label(const Class *type, int value, const string &label);

void crash() {
	int *p = nullptr;
	*p = 4;
}

extern const Class *TypeStringAutoCast;





#if 0
bool is_function_pointer(const Class *c) {
	if (c ==  TypeFunctionP)
		return true;
	return is_typed_function_pointer(c);
}
#endif

int64 s2i2(const string &str) {
	if ((str.num > 1) and (str[0]=='0') and (str[1]=='x')) {
		int64 r=0;
		for (int i=2;i<str.num;i++) {
			r *= 16;
			if ((str[i]>='0') and (str[i]<='9'))
				r+=str[i]-48;
			if ((str[i]>='a') and (str[i]<='f'))
				r+=str[i]-'a'+10;
			if ((str[i]>='A') and (str[i]<='F'))
				r+=str[i]-'A'+10;
		}
		return r;
	} else
		return	str.i64();
}

Parser::Parser(SyntaxTree *t) :
	Exp(t->expressions),
	con(t->module->context, this, t),
	auto_implementer(this, t)
{
	context = t->module->context;
	tree = t;
	cur_func = nullptr;
	Exp.cur_line = nullptr;
	parser_loop_depth = 0;
	found_dynamic_param = false;
}


void Parser::parse_buffer(const string &buffer, bool just_analyse) {
	Exp.analyse(tree, buffer);

	parse_macros(just_analyse);

	parse();

	if (config.verbose)
		tree->show("parse:a");

}

// find the type of a (potential) constant
//  "1.2" -> float
const Class *Parser::get_constant_type(const string &str) {
	// character '...'
	if ((str[0] == '\'') and (str.back() == '\''))
		return TypeChar;

	// string "..."
	if ((str[0] == '"') and (str.back() == '"'))
		return tree->flag_string_const_as_cstring ? TypeCString : TypeString;

	// numerical (int/float)
	const Class *type = TypeInt;
	bool hex = (str.num > 1) and (str[0] == '0') and (str[1] == 'x');
	char last = 0;
	for (int ic=0;ic<str.num;ic++) {
		char c = str[ic];
		if ((c < '0') or (c > '9')) {
			if (hex) {
				if ((ic >= 2) and (c < 'a') and (c > 'f'))
					return TypeUnknown;
			} else if (c == '.') {
				type = TypeFloat32;
			} else {
				if ((ic != 0) or (c != '-')) { // allow sign
					if ((c != 'e') and (c != 'E'))
						if (((c != '+') and (c != '-')) or ((last != 'e') and (last != 'E')))
							return TypeUnknown;
				}
			}
		}
		last = c;
	}
	if (type == TypeInt) {
		if (hex) {
			if ((s2i2(str) >= 0x100000000) or (-s2i2(str) > 0x00000000))
				type = TypeInt64;
		} else {
			if ((s2i2(str) >= 0x80000000) or (-s2i2(str) > 0x80000000))
				type = TypeInt64;
		}
	}
	return type;
}

void Parser::get_constant_value(const string &str, Value &value) {
	value.init(get_constant_type(str));
// literal
	if (value.type == TypeChar) {
		value.as_int() = str.unescape()[1];
	} else if (value.type == TypeString) {
		value.as_string() = str.sub(1, -1).unescape();
	} else if (value.type == TypeCString) {
		strcpy((char*)value.p(), str.sub(1, -1).unescape().c_str());
	} else if (value.type == TypeInt) {
		value.as_int() = (int)s2i2(str);
	} else if (value.type == TypeInt64) {
		value.as_int64() = s2i2(str);
	} else if (value.type == TypeFloat32) {
		value.as_float() = str._float();
	} else if (value.type == TypeFloat64) {
		value.as_float64() = str._float();
	}
}

void Parser::do_error(const string &str, shared<Node> node) {
	do_error_exp(str, node->token_id);
}

void Parser::do_error(const string &str, int token_id) {
	do_error_exp(str, token_id);
}

void Parser::do_error_exp(const string &str, int override_token_id) {
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

shared<Node> Parser::parse_abstract_operand_extension_element(shared<Node> operand) {
	Exp.next(); // .

	auto el = new Node(NodeKind::ABSTRACT_ELEMENT, 0, TypeUnknown);
	el->token_id = Exp.cur_token();
	el->set_num_params(2);
	el->set_param(0, operand);
	el->set_param(1, parse_abstract_token());
	return el;
}

shared<Node> Parser::parse_abstract_operand_extension_dict(shared<Node> operand) {
	Exp.next(); // "{"

	auto node = new Node(NodeKind::ABSTRACT_TYPE_DICT, 0, TypeUnknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(1);
	node->set_param(0, operand);

	expect_identifier("}", "'}' expected after dict 'class{'");
	return node;
}

shared<Node> Parser::parse_abstract_operand_extension_optional(shared<Node> operand) {
	Exp.next(); // "?"

	auto node = new Node(NodeKind::ABSTRACT_TYPE_OPTIONAL, 0, TypeUnknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> Parser::parse_abstract_operand_extension_callable(shared<Node> operand, Block *block) {
	Exp.next(); // "->"

	auto node = new Node(NodeKind::ABSTRACT_TYPE_CALLABLE, 0, TypeUnknown);
	node->token_id = Exp.cur_token();
	node->set_num_params(2);
	node->set_param(0, operand);
	node->set_param(1, parse_abstract_operand(block));
	return node;
}

shared<Node> Parser::parse_abstract_operand_extension_pointer(shared<Node> operand) {
	auto node = new Node(NodeKind::ABSTRACT_TYPE_POINTER, 0, TypeUnknown);
	node->token_id = Exp.consume_token(); // "*"
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> Parser::parse_abstract_operand_extension_array(shared<Node> operand, Block *block) {
	int token0 = Exp.consume_token();
	// array index...


	if (try_consume("]")) {
		auto node = new Node(NodeKind::ABSTRACT_TYPE_LIST, 0, TypeUnknown, false, token0);
		node->set_num_params(1);
		node->set_param(0, operand);
		return node;
	}

	shared<Node> index;
	shared<Node> index2;
	if (Exp.cur == ":") {
		index = add_node_const(tree->add_constant_int(0));
	} else {
		index = parse_abstract_operand_greedy(block);
	}
	if (try_consume(":")) {
		if (Exp.cur == "]") {
			index2 = add_node_const(tree->add_constant_int(DynamicArray::MAGIC_END_INDEX));
			// magic value (-_-)'
		} else {
			index2 = parse_abstract_operand_greedy(block);
		}
	}
	expect_identifier("]", "']' expected after array index");

	auto array = add_node_array(operand, index, TypeUnknown);
	if (index2) {
		array->set_num_params(3);
		array->set_param(2, index2);
	}
	return array;
}

shared<Node> Parser::parse_abstract_operand_extension_call(shared<Node> link, Block *block) {

	// parse all parameters
	auto params = parse_abstract_call_parameters(block);

	auto node = new Node(NodeKind::ABSTRACT_CALL, 0, TypeUnknown);
	node->set_num_params(params.num + 1);
	node->set_param(0, link);
	for (auto&& [i,p]: enumerate(params))
		node->set_param(i + 1, p);

	return node;
}


// find any ".", or "[...]"'s    or operators?
shared<Node> Parser::parse_abstract_operand_extension(shared<Node> operand, Block *block, bool prefer_class) {

#if 0
	// special
	if (false) {
		if (is_type_tuple(operand) and (Exp.cur == "->")) {
			do_error("do we ever reach this point?");
			Exp.next();
			auto ret = parse_type(block->name_space());
			auto t = tree->make_class_callable_fp(class_tuple_extract_classes(operands[0]), ret);

			return parse_operand_extension({add_node_class(t)}, block, prefer_type);
		}
	}
#endif


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
	auto might_declare_pointer_variable = [this] {
		// a line of "int *p = ..."
		if (Exp._cur_exp != 1)
			return false;
		if (is_number(Exp.cur_line->tokens[0].name[0]))
			return false;
		return true;
	};

	if (Exp.cur == ".") {
		// element?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_element(operand), block, prefer_class);
	} else if (Exp.cur == "[") {
		// array?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_array(operand, block), block, prefer_class);
	} else if (Exp.cur == "(") {
		// call?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_call(operand, block), block, prefer_class);
	} else if (Exp.cur == "{") {
		// dict?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_dict(operand), block, prefer_class);
	} else if (Exp.cur == "->") {
		// A->B?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_callable(operand, block), block, true);
	} else if (Exp.cur == "?") {
		// optional?
		return parse_abstract_operand_extension(parse_abstract_operand_extension_optional(operand), block, true);
	} else if (Exp.cur == IDENTIFIER_SHARED or Exp.cur == IDENTIFIER_OWNED) {
		auto sub = operand;
		if (Exp.cur == IDENTIFIER_SHARED) {
			operand = new Node(NodeKind::ABSTRACT_TYPE_SHARED, 0, TypeUnknown);
		} else { //if (pre == IDENTIFIER_OWNED)
			operand = new Node(NodeKind::ABSTRACT_TYPE_OWNED, 0, TypeUnknown);
		}
		operand->token_id = Exp.consume_token();
		operand->set_num_params(1);
		operand->set_param(0, sub);
		return parse_abstract_operand_extension(operand, block, true);
	} else {

		if ((Exp.cur == "*" and (prefer_class or no_identifier_after())) or Exp.cur == "ptr") {
			// FIXME: false positives for "{{pi * 10}}"
			return parse_abstract_operand_extension(parse_abstract_operand_extension_pointer(operand), block, true);
		}
		// unary operator? (++,--)

		if (auto op = parse_abstract_operator(1)) {
			op->set_num_params(1);
			op->set_param(0, operand);
			return parse_abstract_operand_extension(op, block, prefer_class);
		}
		return operand;
	}

	// recursion
	return parse_abstract_operand_extension(operand, block, prefer_class);
}

shared_array<Node> Parser::parse_abstract_call_parameters(Block *block) {
	expect_identifier("(", "'(' expected in front of function parameter list");

	shared_array<Node> params;

	// list of parameters
	if (try_consume(")"))
		return params;
	for (int p=0;;p++) {
		// find parameter
		params.add(parse_abstract_operand_greedy(block));

		if (!try_consume(",")) {
			expect_identifier(")", "',' or ')' expected after parameter for function");
			break;
		}
	}
	return params;
}



shared<Node> build_abstract_list(const Array<shared<Node>> &el) {
	auto c = new Node(NodeKind::ARRAY_BUILDER, 0, TypeUnknown, true);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

shared<Node> build_abstract_dict(const Array<shared<Node>> &el) {
	auto c = new Node(NodeKind::DICT_BUILDER, 0, TypeUnknown, true);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

shared<Node> build_abstract_tuple(const Array<shared<Node>> &el) {
	auto c = new Node(NodeKind::TUPLE, 0, TypeUnknown, true);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

shared<Node> Parser::parse_abstract_set_builder(Block *block) {
	//Exp.next(); // [
	auto n_for = parse_abstract_for_header(block);

	auto n_exp = parse_abstract_operand_greedy(block);
	
	shared<Node> n_cmp;
	if (try_consume(IDENTIFIER_IF))
		n_cmp = parse_abstract_operand_greedy(block);

	expect_identifier("]", "] expected");

	auto n = new Node(NodeKind::ARRAY_BUILDER_FOR, 0, TypeUnknown);
	n->set_num_params(3);
	n->set_param(0, n_for);
	n->set_param(1, n_exp);
	n->set_param(2, n_cmp);
	return n;

}


shared<Node> Parser::apply_format(shared<Node> n, const string &fmt) {
	auto f = n->type->get_member_func(IDENTIFIER_FUNC_FORMAT, TypeString, {TypeString});
	if (!f)
		do_error(format("format string: no '%s.%s(string)' function found", n->type->long_name(), IDENTIFIER_FUNC_FORMAT), n);
	auto *c = tree->add_constant(TypeString);
	c->as_string() = fmt;
	auto nf = add_node_call(f, n->token_id);
	nf->set_instance(n);
	nf->set_param(1, add_node_const(c, n->token_id));
	return nf;
}

shared<Node> Parser::try_parse_format_string(Block *block, Value &v, int token_id) {
	string s = v.as_string();
	
	shared_array<Node> parts;
	int pos = 0;
	
	while (pos < s.num) {
	
		int p0 = s.find("{{", pos);
		
		// constant part before the next {{insert}}
		int pe = (p0 < 0) ? s.num : p0;
		if (pe > pos) {
			auto *c = tree->add_constant(TypeString);
			c->as_string() = s.sub(pos, pe);
			parts.add(add_node_const(c, token_id));
		}
		if (p0 < 0)
			break;
			
		int p1 = s.find("}}", p0);
		if (p1 < 0)
			do_error("string interpolation '{{' not ending with '}}'", token_id);
			
		string xx = s.sub(p0+2, p1);

		// "expr|format" ?
		string fmt;
		int pp = xx.find("|");
		if (pp >= 0) {
			fmt = xx.sub(pp + 1);
			xx = xx.head(pp);
		}

		//msg_write("format:  " + xx);
		ExpressionBuffer ee;
		ee.analyse(tree, xx);
		ee.cur_line->physical_line = Exp.cur_line->physical_line;
		//ee.show();
		
		int token0 = Exp.cur_token();
		//int cl = Exp.get_line_no();
		//int ce = Exp.cur_exp;
		Exp.lines.add(ee.lines[0]);
		Exp.update_meta_data();
		Exp.jump(Exp.lines.back().token_ids[0]);
		
		try {
			auto n = parse_operand_greedy(block, false);
			n = con.deref_if_pointer(n);

			if (fmt != "") {
				n = apply_format(n, fmt);
			} else {
				n = con.check_param_link(n, TypeStringAutoCast, "", 0, 1);
			}
			//n->show();
			parts.add(n);
		} catch (Exception &e) {
			//e.line += cl;
			//e.column += Exp.
			
			// not perfect (e has physical line-no etc and e.text has filenames baked in)
			do_error(e.text, token0);
		}
		
		Exp.lines.pop();
		Exp.update_meta_data();
		Exp.jump(token0);
		
		pos = p1 + 2;
	
	}
	
	// empty???
	if (parts.num == 0) {
		auto c = tree->add_constant(TypeString);
		return add_node_const(c, token_id);
	}
	
	// glue
	while (parts.num > 1) {
		auto b = parts.pop();
		auto a = parts.pop();
		auto n = con.link_operator_id(OperatorID::ADD, a, b, token_id);
		parts.add(n);
	}
	//parts[0]->show();
	return parts[0];
}

shared<Node> Parser::parse_abstract_list(Block *block) {
	shared_array<Node> el;
	if (!try_consume("]"))
		while (true) {
			el.add(parse_abstract_operand_greedy(block));
			if (try_consume("]"))
				break;
			expect_identifier(",", "',' or ']' expected");
		}
	return build_abstract_list(el);
}

shared<Node> Parser::parse_abstract_dict(Block *block) {
	Exp.next(); // {
	shared_array<Node> el;
	if (!try_consume("}"))
		while (true) {
			el.add(parse_abstract_operand_greedy(block)); // key
			expect_identifier(":", "':' after key expected");
			el.add(parse_abstract_operand_greedy(block)); // value
			if (try_consume("}"))
				break;
			expect_identifier(",", "',' or '}' expected in dict");
		}
	return build_abstract_dict(el);
}

const Class *merge_type_tuple_into_product(SyntaxTree *tree, const Array<const Class*> &classes, int token_id) {
	string name;
	int size = 0;
	for (auto &c: classes) {
		size += c->size;
		if (name != "")
			name += ",";
		name += c->name;
	}
	return tree->request_implicit_class("("+name+")", Class::Type::PRODUCT, size, -1, nullptr, classes, token_id);
}

shared<Node> Parser::parse_abstract_token() {
	return new Node(NodeKind::ABSTRACT_TOKEN, (int_p)tree, TypeUnknown, false, Exp.consume_token());
}

// minimal operand
// but with A[...], A(...) etc
shared<Node> Parser::parse_abstract_operand(Block *block, bool prefer_class) {
	shared<Node> operand;

	// ( -> one level down and combine commands
	if (try_consume("(")) {
		operand = parse_abstract_operand_greedy(block, true);
		expect_identifier(")", "')' expected");
	} else if (try_consume("&")) { // & -> address operator
		int token = Exp.cur_token();
		operand = parse_abstract_operand(block)->ref(TypeUnknown);
		operand->token_id = token;
	} else if (try_consume("*")) { // * -> dereference
		int token = Exp.cur_token();
		operand = parse_abstract_operand(block)->deref(TypeUnknown);
		operand->token_id = token;
	} else if (try_consume("[")) {
		if (Exp.cur == "for") {
			operand = parse_abstract_set_builder(block);
		} else {
			operand = parse_abstract_list(block);
		}
	} else if (Exp.cur == "{") {
		operand = parse_abstract_dict(block);
	} else if (auto s = which_statement(Exp.cur)) {
		operand = parse_abstract_statement(block);
	} else if (auto w = which_abstract_operator(Exp.cur, 2)) { // negate/not...
		operand = new Node(NodeKind::ABSTRACT_OPERATOR, (int_p)w, TypeUnknown);
		Exp.next();
		operand->set_num_params(1);
		operand->set_param(0, parse_abstract_operand(block));
	} else {
		operand = parse_abstract_token();
	}

	if (Exp.end_of_line())
		return operand;

	//return operand;
	// resolve arrays, structures, calls...
	return parse_abstract_operand_extension(operand, block, prefer_class);
}

// no type information
shared<Node> Parser::parse_abstract_operator(int param_flags) {
	auto op = which_abstract_operator(Exp.cur, param_flags);
	if (!op)
		return nullptr;

	auto cmd = new Node(NodeKind::ABSTRACT_OPERATOR, (int_p)op, TypeUnknown);
	cmd->token_id = Exp.consume_token();

	return cmd;
}

/*inline int find_operator(int primitive_id, Type *param_type1, Type *param_type2) {
	for (int i=0;i<PreOperator.num;i++)
		if (PreOperator[i].PrimitiveID == primitive_id)
			if ((PreOperator[i].ParamType1 == param_type1) and (PreOperator[i].ParamType2 == param_type2))
				return i;
	//_do_error_("");
	return 0;
}*/

void get_comma_range(shared_array<Node> &_operators, int mio, int &first, int &last) {
	first = mio;
	last = mio+1;
	for (int i=mio; i>=0; i--) {
		if (_operators[i]->as_abstract_op()->id == OperatorID::COMMA)
			first = i;
		else
			break;
	}
	for (int i=mio; i<_operators.num; i++) {
		if (_operators[i]->as_abstract_op()->id == OperatorID::COMMA)
			last = i+1;
		else
			break;
	}
}

void analyse_func(Function *f) {
	msg_write("----------------------------");
	msg_write(f->signature());
	msg_write(f->num_params);
	msg_write(b2s(f->is_member()));
	for (auto p: f->literal_param_type)
		msg_write("  LPT: " + p->long_name());
	for (auto p: f->abstract_param_types)
		if (p)
			msg_write("  APT: " + p->str());
		else
			msg_write("  APT: <nil>");
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

		if (op_no->id == OperatorID::COMMA) {
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
shared<Node> Parser::parse_operand_greedy(Block *block, bool allow_tuples, shared<Node> first_operand) {
	auto tree = parse_abstract_operand_greedy(block, allow_tuples, first_operand);
	if (config.verbose)
		tree->show();
	return con.concretify_node(tree, block, block->name_space());
}

// greedily parse AxBxC...(operand, operator)
shared<Node> Parser::parse_abstract_operand_greedy(Block *block, bool allow_tuples, shared<Node> first_operand) {
	shared_array<Node> operands;
	shared_array<Node> operators;

	// find the first operand
	if (!first_operand)
		first_operand = parse_abstract_operand(block);
	if (config.verbose) {
		msg_write("---first:");
		first_operand->show();
	}
	operands.add(first_operand);

	// find pairs of operators and operands
	while (true) {
		if (!allow_tuples and Exp.cur == ",")
			break;
		if (auto op = parse_abstract_operator(3)) {
			operators.add(op);
			expect_no_new_line("unexpected end of line after operator");
			operands.add(parse_abstract_operand(block));
		} else {
			break;
		}
	}

	return digest_operator_list_to_tree(operands, operators);
}

// greedily parse AxBxC...(operand, operator)
shared<Node> Parser::parse_operand_super_greedy(Block *block) {
	return parse_operand_greedy(block, true);
}

// TODO later...
//  J make Node=Block
//  J for with p[0]=set init
//  * for_var in for "Block"

// Node structure
//  p = [VAR, START, STOP, STEP, BLOCK]
shared<Node> Parser::parse_abstract_for_header(Block *block) {

	// variable name
	int token0 = Exp.consume_token(); // for
	auto var = parse_abstract_token();

	// index
	shared<Node> key;
	if (try_consume("=>")) {
		// key => value
		key = var;
		var = parse_abstract_token();
	} else if (try_consume(",")) {
		key = parse_abstract_token();
	}


	expect_identifier(IDENTIFIER_IN, "'in' expected after variable in 'for ...'");

	// first value/array
	auto val0 = parse_abstract_operand_greedy(block);


	if (try_consume(":")) {
		// range

		if (key)
			do_error("no key=>value allowed in START:END for loop", key);

		auto val1 = parse_abstract_operand_greedy(block);

		shared<Node> val_step;
		if (try_consume(":"))
			val_step = parse_abstract_operand_greedy(block);

		auto cmd_for = add_node_statement(StatementID::FOR_RANGE, token0, TypeUnknown);
		cmd_for->set_param(0, var);
		cmd_for->set_param(1, val0);
		cmd_for->set_param(2, val1);
		cmd_for->set_param(3, val_step);
		//cmd_for->set_uparam(4, loop_block);

		return cmd_for;

	} else {
		// array

		auto array = val0;


		auto cmd_for = add_node_statement(StatementID::FOR_ARRAY, token0, TypeUnknown);
		// [VAR, KEY, ARRAY, BLOCK]

		cmd_for->set_param(0, var);
		cmd_for->set_param(1, key);
		cmd_for->set_param(2, array);
		//cmd_for->set_uparam(3, loop_block);

		return cmd_for;
	}
}

void Parser::post_process_for(shared<Node> cmd_for) {
	auto *n_var = cmd_for->params[0].get();
	auto *var = n_var->as_local();

	if (cmd_for->as_statement()->id == StatementID::FOR_ARRAY) {
		auto *loop_block = cmd_for->params[3].get();

	// ref.
		var->type = tree->get_pointer(var->type);
		n_var->type = var->type;
		tree->transform_node(loop_block, [this, var] (shared<Node> n) {
			return tree->conv_cbr(n, var);
		});
	}

	// force for_var out of scope...
	var->name = ":" + var->name;
	if (cmd_for->as_statement()->id == StatementID::FOR_ARRAY) {
		auto *index = cmd_for->params[1]->as_local();
		index->name = ":" + index->name;
	}
}



// Node structure
shared<Node> Parser::parse_abstract_statement_for(Block *block) {

	auto cmd_for = parse_abstract_for_header(block);

	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	parser_loop_depth ++;
	auto loop_block = parse_abstract_block(block);
	parser_loop_depth --;

	cmd_for->set_param(cmd_for->params.num - 1, loop_block);

	//post_process_for(cmd_for);

	return cmd_for;
}

// Node structure
//  p[0]: test
//  p[1]: loop block
shared<Node> Parser::parse_abstract_statement_while(Block *block) {
	int token0 = Exp.consume_token(); // "while"
	auto cmd_cmp = parse_abstract_operand_greedy(block);

	auto cmd_while = add_node_statement(StatementID::WHILE, token0, TypeUnknown);
	cmd_while->set_param(0, cmd_cmp);

	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	parser_loop_depth ++;
	cmd_while->set_num_params(2);
	cmd_while->set_param(1, parse_abstract_block(block));
	parser_loop_depth --;
	return cmd_while;
}

shared<Node> Parser::parse_abstract_statement_break() {
	if (parser_loop_depth == 0)
		do_error_exp("'break' only allowed inside a loop");
	int token0 = Exp.consume_token(); // "break"
	return add_node_statement(StatementID::BREAK, token0);
}

shared<Node> Parser::parse_abstract_statement_continue() {
	if (parser_loop_depth == 0)
		do_error_exp("'continue' only allowed inside a loop");
	int token0 = Exp.consume_token(); // "continue"
	return add_node_statement(StatementID::CONTINUE, token0);
}

// Node structure
//  p[0]: value (if not void)
shared<Node> Parser::parse_abstract_statement_return(Block *block) {
	int token0 = Exp.consume_token(); // "return"
	auto cmd = add_node_statement(StatementID::RETURN, token0, TypeUnknown);
	if (Exp.end_of_line()) {
		cmd->set_num_params(0);
	} else {
		cmd->set_num_params(1);
		cmd->set_param(0, parse_abstract_operand_greedy(block, true));
	}
	expect_new_line();
	return cmd;
}

// IGNORE!!! raise() is a function :P
shared<Node> Parser::parse_abstract_statement_raise(Block *block) {
	throw "jhhhh";
#if 0
	Exp.next();
	auto cmd = add_node_statement(StatementID::RAISE);

	auto cmd_ex = check_param_link(parse_operand_greedy(block), TypeExceptionP, IDENTIFIER_RAISE, 0);
	cmd->set_num_params(1);
	cmd->set_param(0, cmd_ex);

	/*if (block->function->return_type == TypeVoid) {
		cmd->set_num_params(0);
	} else {
		auto cmd_value = CheckParamLink(GetCommand(block), block->function->return_type, IDENTIFIER_RETURN, 0);
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
shared<Node> Parser::parse_abstract_statement_try(Block *block) {
	int ind = Exp.cur_line->indent;
	int token0 = Exp.consume_token(); // "try"
	auto cmd_try = add_node_statement(StatementID::TRY, token0, TypeUnknown);
	cmd_try->set_num_params(3);
	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	cmd_try->set_param(0, parse_abstract_block(block));
	token0 = Exp.cur_token();
	Exp.next_line();


	int num_excepts = 0;

	// except?
	while (!Exp.end_of_file() and (Exp.cur == IDENTIFIER_EXCEPT) and (Exp.cur_line->indent == ind)) {
		int token1 = Exp.consume_token(); // "except"

		auto cmd_ex = add_node_statement(StatementID::EXCEPT, token1, TypeUnknown);

		auto except_block = new Block(block->function, block, TypeUnknown);

		if (!Exp.end_of_line()) {
			auto ex_type = parse_abstract_operand(block, true); // type
			if (!ex_type)
				do_error_exp("Exception class expected");
			cmd_ex->params.add(ex_type);
			if (!Exp.end_of_line()) {
				expect_identifier(IDENTIFIER_AS, "'as' expected");
				cmd_ex->params.add(parse_abstract_token()); // var name
			}
		}

		//int last_indent = Exp.indent_0;

		// ...block
		expect_new_line_with_indent();
		Exp.next_line();
		//ParseCompleteCommand(block);
		//Exp.next_line();

		//auto n = block->nodes.back();
		//n->as_block()->

		auto cmd_ex_block = parse_abstract_block(block, except_block);

		num_excepts ++;
		cmd_try->set_num_params(1 + num_excepts * 2);
		cmd_try->set_param(num_excepts*2 - 1, cmd_ex);
		cmd_try->set_param(num_excepts*2, cmd_ex_block);

		token0 = Exp.cur_token();
		Exp.next_line();
	}

	Exp.jump(token0); // undo next_line()



	//shared<Node> cmd_ex_block = add_node_block(new_block);
	/*block->nodes.add(cmd_ex_block);

	Exp.indented = false;

	for (int i=0;true;i++) {
		if (((i > 0) and (Exp.cur_line->indent <= last_indent)) or (Exp.end_of_file()))
			break;

		ParseCompleteCommand(new_block);
		Exp.next_line();
	}
	Exp.cur_line --;
	Exp.indent_0 = Exp.cur_line->indent;
	Exp.indented = false;
	Exp.cur_exp = Exp.cur_line->exp.num - 1;
	Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;*/
	return cmd_try;
}

// Node structure (IF):
//  p[0]: test
//  p[1]: true block
// Node structure (IF_ELSE):
//  p[0]: test
//  p[1]: true block
//  p[2]: false block
shared<Node> Parser::parse_abstract_statement_if(Block *block) {
	int ind = Exp.cur_line->indent;
	int token0 = Exp.consume_token(); // "if"
	auto cmd_cmp = parse_abstract_operand_greedy(block);

	auto cmd_if = add_node_statement(StatementID::IF, token0, TypeUnknown);
	cmd_if->set_param(0, cmd_cmp);
	// ...block
	expect_new_line_with_indent();
	Exp.next_line();
	cmd_if->set_param(1, parse_abstract_block(block));
	int token_id = Exp.cur_token();
	Exp.next_line();

	// else?
	if (!Exp.end_of_file() and (Exp.cur == IDENTIFIER_ELSE) and (Exp.cur_line->indent >= ind)) {
		cmd_if->link_no = (int64)statement_from_id(StatementID::IF_ELSE);
		cmd_if->set_num_params(3);
		Exp.next();
		// iterative if
		if (Exp.cur == IDENTIFIER_IF) {
			// sub-if's in a new block
			auto cmd_block = new Block(block->function, block, TypeUnknown);
			cmd_if->set_param(2, cmd_block);
			// parse the next if
			parse_abstract_complete_command(cmd_block);
			return cmd_if;
		}
		// ...block
		expect_new_line_with_indent();
		Exp.next_line();
		cmd_if->set_param(2, parse_abstract_block(block));
	} else {
		Exp.jump(token_id);
	}
	return cmd_if;
}

shared<Node> Parser::parse_abstract_statement_pass(Block *block) {
	int token0 = Exp.consume_token(); // "pass"
	expect_new_line();

	return add_node_statement(StatementID::PASS, token0);
}

// Node structure
//  type: class
//  p[0]: call to constructor (optional)
shared<Node> Parser::parse_abstract_statement_new(Block *block) {
	int token0 = Exp.consume_token(); // "new"
	auto cmd = add_node_statement(StatementID::NEW, token0, TypeUnknown);
	cmd->set_param(0, parse_abstract_operand(block));
	return cmd;
}

// Node structure
//  p[0]: operand
shared<Node> Parser::parse_abstract_statement_delete(Block *block) {
	int token0 = Exp.consume_token(); // "del"
	auto cmd = add_node_statement(StatementID::DELETE, token0, TypeUnknown);
	cmd->set_param(0, parse_abstract_operand(block));
	return cmd;
}

shared<Node> Parser::parse_abstract_single_func_param(Block *block) {
	string func_name = Exp.get_token(Exp.cur_token() - 1);
	expect_identifier("(", format("'(' expected after '%s'", func_name));
	auto n = parse_abstract_operand_greedy(block);
	expect_identifier(")", format("')' expected after parameter of '%s'", func_name));
	return n;
}

shared<Node> Parser::parse_abstract_statement_sizeof(Block *block) {
	int token0 = Exp.consume_token(); // "sizeof"
	auto node = add_node_statement(StatementID::SIZEOF, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;

}

shared<Node> Parser::parse_abstract_statement_type(Block *block) {
	int token0 = Exp.consume_token(); // typeof
	auto node = add_node_statement(StatementID::TYPEOF, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_len(Block *block) {
	int token0 = Exp.consume_token(); // len
	auto node = add_node_statement(StatementID::LEN, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_str(Block *block) {
	int token0 = Exp.consume_token(); // str
	auto node = add_node_statement(StatementID::STR, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_repr(Block *block) {
	int token0 = Exp.consume_token(); // repr
	auto node = add_node_statement(StatementID::REPR, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

// local (variable) definitions...
shared<Node> Parser::parse_abstract_statement_let(Block *block) {
	do_error_exp("'let' is deprecated, will change it's meaning soon...");
	return nullptr;
}

Array<string> parse_comma_sep_list(Parser *p) {
	Array<string> names;

	names.add(p->Exp.consume());

	while (p->try_consume(","))
		names.add(p->Exp.consume());

	return names;
}

shared_array<Node> parse_comma_sep_token_list(Parser *p) {
	shared_array<Node> names;

	names.add(p->parse_abstract_token());

	while (p->try_consume(","))
		names.add(p->parse_abstract_token());

	return names;
}

// local (variable) definitions...
shared<Node> Parser::parse_abstract_statement_var(Block *block) {
	bool is_let = (Exp.cur == IDENTIFIER_LET);
	Exp.next(); // "var"/"let"

	// tuple "var (x,y) = ..."
	if (try_consume("(")) {
		auto names = parse_comma_sep_token_list(this);
		expect_identifier(")", "')' expected after tuple declaration");

		expect_identifier("=", "'=' expected after tuple declaration", false);

		auto tuple = build_abstract_tuple(names);

		auto assign = parse_abstract_operator(3);

		auto rhs = parse_abstract_operand_greedy(block, true);

		assign->set_num_params(2);
		assign->set_param(0, tuple);
		assign->set_param(1, rhs);

		auto node = new Node(NodeKind::ABSTRACT_VAR, (int)is_let, TypeUnknown);
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
		type = parse_abstract_operand(block, true);
	} else {
		expect_identifier("=", "':' or '=' expected after 'var' declaration", false);
	}

	if (Exp.cur == "=") {
		if (names.num != 1)
			do_error_exp(format("'var' declaration with '=' only allowed with a single variable name, %d given", names.num));

		auto assign = parse_abstract_operator(3);

		auto rhs = parse_abstract_operand_greedy(block, true);

		assign->set_num_params(2);
		assign->set_param(0, names[0]);
		assign->set_param(1, rhs);

		auto node = new Node(NodeKind::ABSTRACT_VAR, (int)is_let, TypeUnknown);
		node->set_num_params(3);
		node->set_param(0, type); // type
		node->set_param(1, names[0]->shallow_copy()); // name
		node->set_param(2, assign);
		expect_new_line();
		return node;
	}

	expect_new_line();

	for (auto &n: names) {
		auto node = new Node(NodeKind::ABSTRACT_VAR, (int)is_let, TypeUnknown);
		node->set_num_params(2);
		node->set_param(0, type); // type
		node->set_param(1, n); // name
		block->add(node);
	}
	return add_node_statement(StatementID::PASS);
}

shared<Node> Parser::parse_abstract_statement_map(Block *block) {
	int token0 = Exp.consume_token(); // "map"

	auto params = parse_abstract_call_parameters(block);
	if (params.num != 2)
		do_error_exp("map() expects 2 parameters");

	auto node = add_node_statement(StatementID::MAP, token0, TypeUnknown);
	node->set_param(0, params[0]);
	node->set_param(1, params[1]);
	return node;
}


shared<Node> Parser::parse_abstract_statement_lambda(Block *block) {
	int token0 = Exp.consume_token(); // "lambda"

	static int unique_lambda_counter = 0;

	auto *f = tree->add_function(format(":lambda-%d:", unique_lambda_counter ++), TypeUnknown, tree->base_class, Flags::STATIC);
	f->token_id = token0;

	expect_identifier("(", "");

	// parameter list
	if (!try_consume(")"))
		while (true) {
			// like variable definitions

			Flags flags = parse_flags();

			string param_name = Exp.consume();
			expect_identifier(":", "':' after parameter name expected");

			// type of parameter variable
			auto param_type = parse_type(tree->base_class); // force
			auto v = f->add_param(param_name, param_type, flags);

			if (try_consume(")"))
				break;

			expect_identifier(",", "',' or ')' expected after parameter");
		}

	// lambda body
	if (Exp.end_of_line()) {
		//parse_abstract_block(parent, f->block.get());

		int indent0 = Exp.cur_line->indent;
		bool more_to_parse = true;

	// instructions
		while (more_to_parse) {
			more_to_parse = parse_abstract_function_command(f, indent0);
		}
		Exp.rewind();

	} else {
		// single expression
		auto cmd = parse_abstract_operand_greedy(f->block.get());
		f->block->add(cmd);
	}

	auto node = add_node_statement(StatementID::LAMBDA, token0, TypeUnknown);
	node->set_num_params(1);
	node->set_param(0, add_node_func_name(f));
	return node;
}

shared<Node> Parser::parse_abstract_statement_sorted(Block *block) {
	int token0 = Exp.consume_token(); // "sorted"

	auto node = add_node_statement(StatementID::SORTED, token0, TypeUnknown);

	if (Exp.cur != "(") {
		node->set_num_params(0);
		return node;
	}

	auto params = parse_abstract_call_parameters(block);
	node->set_param(0, params[0]);
	if (params.num < 0 or params.num > 2)
		do_error_exp("sorted(array, criterion=\"\") expects 1 or 2 parameters");
	if (params.num >= 2) {
		node->set_param(1, params[1]);
	} else {
		// empty string
		node->set_param(1, add_node_const(tree->add_constant(TypeString)));
	}
	return node;
}

shared<Node> Parser::parse_abstract_statement_dyn(Block *block) {
	int token0 = Exp.consume_token(); // "dyn"
	auto node = add_node_statement(StatementID::DYN, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_raw_function_pointer(Block *block) {
	int token0 = Exp.consume_token(); // "raw_function_pointer"
	auto node = add_node_statement(StatementID::RAW_FUNCTION_POINTER, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_weak(Block *block) {
	int token0 = Exp.consume_token(); // "weak"
	auto node = add_node_statement(StatementID::WEAK, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement(Block *block) {
	if (Exp.cur == IDENTIFIER_FOR) {
		return parse_abstract_statement_for(block);
	} else if (Exp.cur == IDENTIFIER_WHILE) {
		return parse_abstract_statement_while(block);
 	} else if (Exp.cur == IDENTIFIER_BREAK) {
 		return parse_abstract_statement_break();
	} else if (Exp.cur == IDENTIFIER_CONTINUE) {
		return parse_abstract_statement_continue();
	} else if (Exp.cur == IDENTIFIER_RETURN) {
		return parse_abstract_statement_return(block);
	//} else if (Exp.cur == IDENTIFIER_RAISE) {
	//	ParseStatementRaise(block);
	} else if (Exp.cur == IDENTIFIER_TRY) {
		return parse_abstract_statement_try(block);
	} else if (Exp.cur == IDENTIFIER_IF) {
		return parse_abstract_statement_if(block);
	} else if (Exp.cur == IDENTIFIER_PASS) {
		return parse_abstract_statement_pass(block);
	} else if (Exp.cur == IDENTIFIER_NEW) {
		return parse_abstract_statement_new(block);
	} else if (Exp.cur == IDENTIFIER_DELETE) {
		return parse_abstract_statement_delete(block);
	} else if (Exp.cur == IDENTIFIER_SIZEOF) {
		return parse_abstract_statement_sizeof(block);
	} else if (Exp.cur == IDENTIFIER_TYPEOF) {
		return parse_abstract_statement_type(block);
	} else if (Exp.cur == IDENTIFIER_STR) {
		return parse_abstract_statement_str(block);
	} else if (Exp.cur == IDENTIFIER_REPR) {
		return parse_abstract_statement_repr(block);
	} else if (Exp.cur == IDENTIFIER_LEN) {
		return parse_abstract_statement_len(block);
	} else if (Exp.cur == IDENTIFIER_LET or Exp.cur == IDENTIFIER_VAR) {
		return parse_abstract_statement_var(block);
	} else if (Exp.cur == IDENTIFIER_MAP) {
		return parse_abstract_statement_map(block);
	} else if (Exp.cur == IDENTIFIER_LAMBDA or Exp.cur == IDENTIFIER_FUNC) {
		return parse_abstract_statement_lambda(block);
	} else if (Exp.cur == IDENTIFIER_SORTED) {
		return parse_abstract_statement_sorted(block);
	} else if (Exp.cur == IDENTIFIER_DYN) {
		return parse_abstract_statement_dyn(block);
	} else if (Exp.cur == IDENTIFIER_RAW_FUNCTION_POINTER) {
		return parse_abstract_statement_raw_function_pointer(block);
	} else if (Exp.cur == IDENTIFIER_WEAK) {
		return parse_abstract_statement_weak(block);
	}
	do_error_exp("unhandled statement: " + Exp.cur);
	return nullptr;
}

shared<Node> Parser::parse_abstract_block(Block *parent, Block *block) {
	int indent0 = Exp.cur_line->indent;

	if (!block)
		block = new Block(parent->function, parent, TypeUnknown);
	block->type = TypeUnknown;

	while (!Exp.end_of_file()) {

		parse_abstract_complete_command(block);

		if (Exp.next_line_indent() < indent0)
			break;
		Exp.next_line();
	}

	return block;
}

// local (variable) definitions...
void Parser::parse_abstract_local_definition_old(Block *block, shared<Node> first) {
	while (!Exp.end_of_line()) {
		auto node = new Node(NodeKind::ABSTRACT_VAR, 0, TypeUnknown);
		node->set_num_params(2);
		node->set_param(0, first); // type
		node->set_param(1, parse_abstract_token()); // name
		block->add(node);

		// assignment?
		if (Exp.cur == "=") {
			Exp.rewind();
			// parse assignment
			node->set_num_params(3);
			node->set_param(2, parse_abstract_operand_greedy(block, true));
		}
		if (Exp.end_of_line())
			break;
		expect_identifier(",", "',', '=' or newline expected after declaration of local variable");
	}
}

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void Parser::parse_abstract_complete_command(Block *block) {
	// beginning of a line!

	//bool is_type = tree->find_root_type_by_name(Exp.cur, block->name_space(), true);

	// assembler block
	if (try_consume("-asm-")) {
		block->add(add_node_statement(StatementID::ASM));

	} else {

		auto first = parse_abstract_operand(block);

		if (is_letter(Exp.cur[0])) {
		//if ((first->kind == NodeKind::CLASS) and !Exp.end_of_line()) {
			parse_abstract_local_definition_old(block, first);

		} else {

			// commands (the actual code!)
			block->add(parse_abstract_operand_greedy(block, true, first));
		}
	}

	expect_new_line();
}

void Parser::parse_import() {
	string command = Exp.cur; // 'use' / 'import'
	bool indirect = (command == IDENTIFIER_IMPORT);
	Exp.next();

	// parse import name
	string name = Exp.cur;
	int token = Exp.consume_token();

	string as_name;
	while (!Exp.end_of_line()) {
		if (try_consume(IDENTIFIER_AS)) {
			expect_no_new_line("name expected after 'as'");
			as_name = Exp.cur;
			break;
		} else {
			expect_identifier(".", "'.' expected in import name");
		}
		name += ".";
		expect_no_new_line();
		name += Exp.consume();
	}
	
	// "old/style.kaba" -> old.style
	if (name.match("\"*\""))
		name = name.sub(1, -1).replace(".kaba", "").replace("/", ".");
		
	auto import = get_import(this, name, token);

	if (as_name == "")
		as_name = name;
	tree->import_data(import, indirect, as_name);
}


void Parser::parse_enum(Class *_namespace) {
	Exp.next(); // 'enum'

	expect_no_new_line("name expected (anonymous enum is deprecated)");

	// class name
	int token0 = Exp.cur_token();
	auto _class = tree->create_new_class(Exp.consume(), Class::Type::ENUM, sizeof(int), -1, nullptr, {}, _namespace, token0);

	expect_new_line_with_indent();
	Exp.next_line();
	int indent0 = Exp.cur_line->indent;

	int next_value = 0;

	for (int i=0;!Exp.end_of_file();i++) {
		for (int j=0;!Exp.end_of_line();j++) {
			auto *c = tree->add_constant(_class, _class);
			c->name = Exp.consume();

			// explicit value
			if (try_consume("=")) {
				expect_no_new_line();

				auto cv = parse_and_eval_const(tree->root_of_all_evil->block.get(), TypeInt);
				next_value = cv->as_const()->as_int();
			}
			c->as_int() = (next_value ++);

			if (try_consume(IDENTIFIER_AS)) {
				expect_no_new_line();

				auto cn = parse_and_eval_const(tree->root_of_all_evil->block.get(), TypeString);
				auto label = cn->as_const()->as_string();
				add_enum_label(_class, c->as_int(), label);
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

	flags_set(_class->flags, Flags::FULLY_PARSED);
}

bool type_needs_alignment(const Class *t) {
	if (t->is_array())
		return type_needs_alignment(t->get_array_element());
	return (t->size >= 4);
}

void parser_class_add_element(Parser *p, Class *_class, const string &name, const Class *type, Flags flags, int &_offset, int token_id) {

	// override?
	ClassElement *orig = base::find_by_element(_class->elements, &ClassElement::name, name);

	bool override = flags_has(flags, Flags::OVERRIDE);
	if (override and ! orig)
		p->do_error(format("can not override element '%s', no previous definition", name), token_id);
	if (!override and orig)
		p->do_error(format("element '%s' is already defined, use '%s' to override", name, IDENTIFIER_OVERRIDE), token_id);
	if (override) {
		if (orig->type->is_pointer() and type->is_pointer())
			orig->type = type;
		else
			p->do_error("can only override pointer elements with other pointer type", token_id);
		return;
	}

	// check parsing dependencies
	if (!type->is_size_known())
		p->do_error(format("size of type '%s' is not known at this point", type->long_name()), token_id);


	// add element
	if (flags_has(flags, Flags::STATIC)) {
		auto v = new Variable(name, type);
		flags_set(v->flags, flags);
		_class->static_variables.add(v);
	} else {
		if (type_needs_alignment(type))
			_offset = mem_align(_offset, 4);
		_offset = p->context->external->process_class_offset(_class->cname(p->tree->base_class), name, _offset);
		auto el = ClassElement(name, type, _offset);
		_offset += type->size;
		_class->elements.add(el);
	}
}

Class *Parser::parse_class_header(Class *_namespace, int &offset0) {
	offset0 = 0;
	bool as_interface = (Exp.consume() == IDENTIFIER_INTERFACE); // 'class'/'interface'
	string name = Exp.cur;
	int token_id = Exp.consume_token();

	// create class
	Class *_class = const_cast<Class*>(tree->find_root_type_by_name(name, _namespace, false));
	// already created...
	if (!_class)
		tree->module->do_error_internal("class declaration ...not found " + name);
	_class->token_id = token_id;
	if (as_interface)
		_class->type = Class::Type::INTERFACE;

	if (try_consume(IDENTIFIER_AS)) {
		if (try_consume(IDENTIFIER_SHARED))
			flags_set(_class->flags, Flags::SHARED);
		else
			do_error_exp("'shared' extected after 'as'");
	}

	// parent class
	if (try_consume(IDENTIFIER_EXTENDS)) {
		auto parent = parse_type(_namespace); // force
		if (!parent->fully_parsed())
			return nullptr;
			//do_error(format("parent class '%s' not fully parsed yet", parent->long_name()));
		_class->derive_from(parent, true);
		offset0 = parent->size;
	}

	if (try_consume(IDENTIFIER_IMPLEMENTS)) {
		auto parent = parse_type(_namespace); // force
		if (!parent->fully_parsed())
			return nullptr;
		_class->derive_from(parent, true);
		offset0 = parent->size;
	}
	expect_new_line();

	if (flags_has(_class->flags, Flags::SHARED)) {
		parser_class_add_element(this, _class, IDENTIFIER_SHARED_COUNT, TypeInt, Flags::NONE, offset0, _class->token_id);
	}

	//msg_write("parse " + _class->long_name());
	return _class;
}

bool Parser::parse_class(Class *_namespace) {
	int indent0 = Exp.cur_line->indent;
	int _offset = 0;

	auto _class = parse_class_header(_namespace, _offset);
	if (!_class) // in case, not fully parsed
		return false;

	Array<int> sub_class_token_ids;

	// body
	while (!Exp.end_of_file()) {
		Exp.next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
		if (Exp.end_of_file())
			break;

		if (Exp.cur == IDENTIFIER_ENUM) {
			parse_enum(_class);
		} else if ((Exp.cur == IDENTIFIER_CLASS) or (Exp.cur == IDENTIFIER_INTERFACE)) {
			//msg_write("sub....");
			int cur_token = Exp.cur_token();
			if (!parse_class(_class)) {
				sub_class_token_ids.add(cur_token);
				skip_parse_class();
			}
			//msg_write(">>");
		} else if (Exp.cur == IDENTIFIER_FUNC) {
			auto flags = Flags::CONST;
			if (_class->is_interface())
				flags_set(flags, Flags::VIRTUAL);
			auto f = parse_function_header(_class, flags);
			skip_parsing_function_body(f);
		} else if ((Exp.cur == IDENTIFIER_CONST) or (Exp.cur == IDENTIFIER_LET)) {
			parse_named_const(_class, tree->root_of_all_evil->block.get());
		} else if (try_consume(IDENTIFIER_VAR)) {
			parse_class_variable_declaration(_class, tree->root_of_all_evil->block.get(), _offset);
		} else if (Exp.cur == IDENTIFIER_USE) {
			parse_class_use_statement(_class);
		} else {
			parse_class_variable_declaration(_class, tree->root_of_all_evil->block.get(), _offset);
			//do_error("unknown definition inside a class");
		}
	}

	post_process_newly_parsed_class(_class, _offset);


	int cur_token = Exp.cur_token();

	//msg_write(ia2s(sub_class_line_offsets));
	for (int id: sub_class_token_ids) {
		//msg_write("SUB...");
		Exp.jump(id);
		//.add(Exp.get_line_no());
		if (!parse_class(_class))
			do_error(format("parent class not fully parsed yet"), id);
			//do_error(format("parent class '%s' not fully parsed yet", parent->long_name()));
	}

	Exp.jump(cur_token-1);
	return true;
}

void Parser::post_process_newly_parsed_class(Class *_class, int size) {
	auto external = context->external.get();

	// virtual functions?     (derived -> _class->num_virtual)
//	_class->vtable = cur_virtual_index;
	//for (ClassFunction &cf, _class->function)
	//	_class->num_virtual = max(_class->num_virtual, cf.virtual_index);
	if (_class->vtable.num > 0) {
		if (_class->parent) {
			if (_class->parent->vtable.num == 0)
				do_error("no virtual functions allowed when inheriting from class without virtual functions", _class->token_id);
			// element "-vtable-" being derived
		} else {
			for (ClassElement &e: _class->elements)
				e.offset = external->process_class_offset(_class->cname(tree->base_class), e.name, e.offset + config.pointer_size);

			auto el = ClassElement(IDENTIFIER_VTABLE_VAR, TypePointer, 0);
			_class->elements.insert(el, 0);
			size += config.pointer_size;

			for (auto &i: _class->initializers)
				i.element ++;
		}
	}

	for (auto &e: _class->elements)
		if (type_needs_alignment(e.type))
			size = mem_align(size, 4);
	_class->size = external->process_class_size(_class->cname(tree->base_class), size);


	tree->add_missing_function_headers_for_class(_class);

	flags_set(_class->flags, Flags::FULLY_PARSED);
}

void Parser::skip_parse_class() {
	int indent0 = Exp.cur_line->indent;

	// elements
	while (!Exp.end_of_file()) {
		if (Exp.next_line_indent() <= indent0)
			break;
		Exp.next_line();
	}
	Exp.jump(Exp.cur_line->token_ids.back());
}

void Parser::expect_no_new_line(const string &error_msg) {
	if (Exp.end_of_line())
		do_error_exp(error_msg.num > 0 ? error_msg : "unexpected newline");
}

void Parser::expect_new_line(const string &error_msg) {
	if (!Exp.end_of_line())
		do_error_exp(error_msg.num > 0 ? error_msg : "newline expected");
}

void Parser::expect_new_line_with_indent() {
	if (!Exp.end_of_line())
		do_error_exp("newline expected");
	if (Exp.next_line_indent() <= Exp.cur_line->indent)
		do_error_exp("additional indent expected");
}

void Parser::expect_identifier(const string &identifier, const string &error_msg, bool consume) {
	if (Exp.cur != identifier)
		do_error_exp(error_msg);
	if (consume)
		Exp.next();
}

bool Parser::try_consume(const string &identifier) {
	if (Exp.cur == identifier) {
		Exp.next();
		return true;
	}
	return false;
}



shared<Node> Parser::parse_and_eval_const(Block *block, const Class *type) {

	// find const value
	auto cv = parse_operand_greedy(block, true);

	if (type) {
		CastingData cast;
		if (con.type_match_with_cast(cv, false, type, cast)) {
			cv = con.apply_type_cast(cast, cv, type);
		} else {
			do_error(format("constant value of type '%s' expected", type->long_name()), cv);
		}
	} else {
		cv = con.force_concrete_type(cv);
		type = cv->type;
	}

	cv = tree->transform_node(cv, [this] (shared<Node> n) {
		return tree->conv_eval_const_func(tree->conv_fake_constructors(n));
	});

	if (cv->kind != NodeKind::CONSTANT) {
		//cv->show(TypeVoid);
		do_error("constant value expected, but expression can not be evaluated at compile time", cv);
	}
	return cv;
}

void Parser::parse_named_const(Class *name_space, Block *block) {
	Exp.next(); // 'const' / 'let'
	string name = Exp.consume();

	const Class *type = nullptr;
	if (try_consume(":"))
		type = parse_type(name_space);

	expect_identifier("=", "'=' expected after const name");

	// find const value
	auto cv = parse_and_eval_const(block, type);
	Constant *c_value = cv->as_const();

	auto *c = tree->add_constant(c_value->type.get(), name_space);
	c->set(*c_value);
	c->name = name;
}

void Parser::parse_class_variable_declaration(const Class *ns, Block *block, int &_offset, Flags flags0) {
	if (ns->is_interface())
		do_error_exp("interfaces can not have data elements");

	int token0 = Exp.cur_token();
	Flags flags = parse_flags(flags0);

	auto names = parse_comma_sep_list(this);
	const Class *type = nullptr;

	// explicit type?
	if (try_consume(":")) {
		type = parse_type(ns);
	} else {
		expect_identifier("=", "':' or '=' expected after 'var' declaration", false);
	}

	Constant *c_value = nullptr;
	if (try_consume("=")) {

		//if (names.num != 1)
		//	do_error(format("'var' declaration with '=' only allowed with a single variable name, %d given", names.num));

		auto cv = parse_and_eval_const(block, type);
		c_value = cv->as_const();
		if (!type)
			type = cv->type;

		/*auto rhs = parse_operand_super_greedy(block);
		if (!type) {
			rhs = force_concrete_type(rhs);
			type = rhs->type;
		}
		auto *var = block->add_var(names[0], type);
		auto cmd = link_operator_id(OperatorID::ASSIGN, add_node_local(var), rhs);
		if (!cmd)
			do_error(format("var: no operator '%s' = '%s'", type->long_name(), rhs->type->long_name()));
		return cmd;*/
	}

	expect_new_line();

	for (auto &n: names) {
		auto cc = const_cast<Class*>(ns);
		//block->add_var(n, type);
		parser_class_add_element(this, cc, n, type, flags, _offset, token0);
		/*auto *v = new Variable(n, type);
		flags_set(v->flags, flags);
		tree->base_class->static_variables.add(v);*/

		if (c_value) {
			ClassInitializers init = {ns->elements.num - 1, c_value};
			cc->initializers.add(init);
		}
	}
}

void Parser::parse_class_use_statement(const Class *c) {
	Exp.next(); // "use"
	string name = Exp.consume();
	bool found = false;
	for (auto &e: c->elements)
		if (e.name == name) {
			e.allow_indirect_use = true;
			found = true;
		}
	if (!found)
		do_error_exp(format("use: class '%s' does not have an element '%s'", c->name, name));

	expect_new_line();
}

bool peek_commands_super(ExpressionBuffer &Exp) {
	ExpressionBuffer::Line *l = Exp.cur_line + 1;
	if (l->tokens.num < 3)
		return false;
	if ((l->tokens[0].name == IDENTIFIER_SUPER) and (l->tokens[1].name == ".") and (l->tokens[2].name == IDENTIFIER_FUNC_INIT))
		return true;
	return false;
}

bool Parser::parse_abstract_function_command(Function *f, int indent0) {
	if (Exp.end_of_file())
		return false;

	Exp.next_line();

	// end of file
	if (Exp.end_of_file())
		return false;

	// end of function
	if (Exp.cur_line->indent <= indent0)
		return false;

	// command or local definition
	parse_abstract_complete_command(f->block.get());
	return true;
}



/*const Class *Parser::parse_product_type(const Class *ns) {
	Exp.next(); // (
	Array<const Class*> types;
	types.add(parse_type(ns));

	while (Exp.cur == ",") {
		Exp.next();
		types.add(parse_type(ns));
	}
	if (Exp.cur != ")")
		do_error("',' or ')' in type list expected");
	Exp.next();
	if (types.num == 1)
		return types[0];

	int size = 0;
	string name = types[0]->name;
	for (int i=1; i<types.num; i++) {
		name += "," + types[i]->name;
	}

	auto t = tree->make_class("(" + name + ")", Class::Type::OTHER, size, -1, nullptr, nullptr, ns);
	return t;
}*/

// complicated types like "int[]*[4]" etc
// greedy
const Class *Parser::parse_type(const Class *ns) {
	auto cc = parse_abstract_operand(tree->root_of_all_evil->block.get(), true);
	return con.concretify_as_type(cc, tree->root_of_all_evil->block.get(), ns);
}

Function *Parser::parse_function_header(Class *name_space, Flags flags) {
	Exp.next(); // "func"

	auto block = tree->root_of_all_evil->block.get();

	flags = parse_flags(flags);

	int token = Exp.cur_token();
	string name;
	if (Exp.cur == "(") {
		static int lambda_count = 0;
		name = format(":lambda-%d:", lambda_count ++);
	} else {
		name = Exp.consume();
		if ((name == IDENTIFIER_FUNC_INIT) or (name == IDENTIFIER_FUNC_DELETE) or (name == IDENTIFIER_FUNC_ASSIGN))
			flags_clear(flags, Flags::CONST);
	}

	Function *f = tree->add_function(name, TypeVoid, name_space, flags);

	// template?
	Array<string> template_param_names;
	if (try_consume("[")) {
		template_param_names.add(Exp.consume());
		expect_identifier("]", "']' expected after template parameter");
		flags_set(f->flags, Flags::TEMPLATE);
	}

	//if (config.verbose)
	//	msg_write("PARSE HEAD  " + f->signature());
	f->token_id = token;
	cur_func = f;

	expect_identifier("(", "'(' expected after function name");

// parameter list

	if (!try_consume(")")) {
		for (int k=0;;k++) {
			// like variable definitions

			auto param_flags = parse_flags();

			string param_name = Exp.consume();

			expect_identifier(":", "':' expected after parameter name");

			// type of parameter variable
			f->abstract_param_types.add(parse_abstract_operand(block, true));
			auto v = f->add_param(param_name, TypeUnknown, param_flags);


			// default parameter?
			if (try_consume("=")) {
				f->default_parameters.resize(f->num_params - 1);
				auto dp = parse_abstract_operand(block);
				f->default_parameters.add(dp);
			}

			if (try_consume(")"))
				break;

			expect_identifier(",", "',' or ')' expected after parameter");
		}
	}

	if (try_consume("->")) {
		// return type
		f->abstract_return_type = parse_abstract_operand(tree->root_of_all_evil->block.get(), true);
	}

	expect_new_line("newline expected after parameter list");

	if (f->is_template()) {
		context->template_manager->add_template(f, template_param_names);
		name_space->add_template_function(tree, f, flags_has(flags, Flags::VIRTUAL), flags_has(flags, Flags::OVERRIDE));
	} else {
		con.concretify_function_header(f);

		f->update_parameters_after_parsing();

		name_space->add_function(tree, f, flags_has(flags, Flags::VIRTUAL), flags_has(flags, Flags::OVERRIDE));
	}

	cur_func = nullptr;

	return f;
}

void Parser::skip_parsing_function_body(Function *f) {
	int indent0 = Exp.cur_line->indent;
	while (!Exp.end_of_file()) {
		if (Exp.next_line_indent() <= indent0)
			break;
		Exp.next_line();
	}

	// jump to end of line
	Exp.jump(Exp.cur_line->token_ids.back());
	function_needs_parsing.add(f);
}

void Parser::parse_abstract_function_body(Function *f) {
	Exp.jump(f->token_id);
	f->block->type = TypeUnknown; // abstract parsing

	int indent0 = Exp.cur_line->indent;
	bool more_to_parse = true;

	// auto implement constructor?
	if (f->name == IDENTIFIER_FUNC_INIT) {
		if (peek_commands_super(Exp)) {
			more_to_parse = parse_abstract_function_command(f, indent0);

			auto_implementer.auto_implement_regular_constructor(f, f->name_space, false);
		} else {
			auto_implementer.auto_implement_regular_constructor(f, f->name_space, true);
		}
	}

	parser_loop_depth = 0;

// instructions
	while (more_to_parse) {
		more_to_parse = parse_abstract_function_command(f, indent0);
	}

	if (config.verbose) {
		msg_write("ABSTRACT:");
		f->block->show();
	}

	if (!f->is_template())
		con.concretify_function_body(f);

	cur_func = nullptr;
}

void Parser::parse_all_class_names_in_block(Class *ns, int indent0) {
	while (!Exp.end_of_file()) {
		if ((Exp.cur_line->indent == indent0) and (Exp.cur_line->tokens.num >= 2)) {
			if ((Exp.cur == IDENTIFIER_CLASS) or (Exp.cur == IDENTIFIER_INTERFACE)) {
				Exp.next();
//				if (Exp.cur.num == 1)
//					do_error("class names must be at least 2 characters long", Exp.cur_token());
				Class *t = tree->create_new_class(Exp.cur, Class::Type::REGULAR, 0, 0, nullptr, {}, ns, Exp.cur_token());
				flags_clear(t->flags, Flags::FULLY_PARSED);

				Exp.next_line();
				parse_all_class_names_in_block(t, indent0 + 1);
				continue;
			}
		}
		if (Exp.end_of_file())
			break;
		if (Exp.cur_line->indent < indent0)
			break;
		Exp.next_line();
	}
}

void Parser::parse_all_function_bodies() {
	//for (auto *f: function_needs_parsing)   might add lambda functions...
	for (int i=0; i<function_needs_parsing.num; i++) {
		auto f = function_needs_parsing[i];
		if (!f->is_extern() and (f->token_id >= 0))
			parse_abstract_function_body(f);
	}
}

Flags Parser::parse_flags(Flags initial) {
	Flags flags = initial;

	while (true) {
		if (Exp.cur == IDENTIFIER_STATIC) {
			flags_set(flags, Flags::STATIC);
		} else if (Exp.cur == IDENTIFIER_EXTERN) {
			flags_set(flags, Flags::EXTERN);
		} else if (Exp.cur == IDENTIFIER_CONST) {
			flags_set(flags, Flags::CONST);
		} else if (Exp.cur == IDENTIFIER_MUTABLE) {
			flags_clear(flags, Flags::CONST);
		} else if (Exp.cur == IDENTIFIER_CONST) {
			flags_set(flags, Flags::CONST);
		} else if (Exp.cur == IDENTIFIER_VIRTUAL) {
			flags_set(flags, Flags::VIRTUAL);
		} else if (Exp.cur == IDENTIFIER_OVERRIDE) {
			flags_set(flags, Flags::OVERRIDE);
		} else if (Exp.cur == IDENTIFIER_SELFREF or Exp.cur == IDENTIFIER_REF) {
			flags_set(flags, Flags::REF);
		//} else if (Exp.cur == IDENTIFIER_SHARED) {
		//	flags = flags_mix({flags, Flags::SHARED});
		//} else if (Exp.cur == IDENTIFIER_OWNED) {
		//	flags = flags_mix({flags, Flags::OWNED});
		} else if (Exp.cur == IDENTIFIER_OUT) {
			flags_set(flags, Flags::OUT);
		} else if (Exp.cur == IDENTIFIER_THROWS) {
			flags_set(flags, Flags::RAISES_EXCEPTIONS);
		} else if (Exp.cur == IDENTIFIER_PURE) {
			flags_set(flags, Flags::PURE);
		} else {
			break;
		}
		Exp.next();
	}
	return flags;
}

void Parser::parse_top_level() {
	cur_func = nullptr;

	// syntax analysis

	Exp.reset_walker();
	parse_all_class_names_in_block(tree->base_class, 0);

	Exp.reset_walker();

	// global definitions (enum, class, variables and functions)
	while (!Exp.end_of_file()) {

		if ((Exp.cur == IDENTIFIER_IMPORT) or (Exp.cur == IDENTIFIER_USE)) {
			parse_import();

		// enum
		} else if (Exp.cur == IDENTIFIER_ENUM) {
			parse_enum(tree->base_class);

		// class
		} else if ((Exp.cur == IDENTIFIER_CLASS) or (Exp.cur == IDENTIFIER_INTERFACE)) {
			parse_class(tree->base_class);

		// func
		} else if (Exp.cur == IDENTIFIER_FUNC) {
			auto f = parse_function_header(tree->base_class, Flags::STATIC);
			skip_parsing_function_body(f);

		} else if ((Exp.cur == IDENTIFIER_CONST) or (Exp.cur == IDENTIFIER_LET)) {
			parse_named_const(tree->base_class, tree->root_of_all_evil->block.get());

		} else if (try_consume(IDENTIFIER_VAR)) {
			int offset = 0;
			parse_class_variable_declaration(tree->base_class, tree->root_of_all_evil->block.get(), offset, Flags::STATIC);

		} else {
			do_error_exp("unknown top level definition");
		}
		if (!Exp.end_of_file())
			Exp.next_line();
	}
}

// convert text into script data
void Parser::parse() {
	Exp.reset_walker();
	Exp.do_error_endl = [this] {
		do_error_exp("unexpected newline");
	};

	parse_top_level();

	parse_all_function_bodies();
	
	tree->show("aaa");

	for (auto *f: tree->functions)
		test_node_recursion(f->block.get(), tree->base_class, "a " + f->long_name());

	for (int i=0; i<tree->owned_classes.num; i++) // array might change...
		auto_implementer.auto_implement_functions(tree->owned_classes[i]);

	for (auto *f: tree->functions)
		test_node_recursion(f->block.get(), tree->base_class, "b " + f->long_name());
}

}
