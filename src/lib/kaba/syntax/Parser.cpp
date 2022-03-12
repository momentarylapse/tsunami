#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include "Parser.h"
#include <stdio.h>

#include "../../config.h"

#ifdef _X_USE_HUI_
#include "../../hui/Application.h"
#elif defined(_X_USE_HUI_MINIMAL_)
#include "../../hui_minimal/Application.h"
#endif



const int MAX_IMPORT_DIRECTORY_PARENTS = 5;

namespace kaba {

void test_node_recursion(shared<Node> root, const Class *ns, const string &message);
shared<Node> create_node_token(Parser *p);

Array<const Class*> func_effective_params(const Function *f);
Array<const Class*> node_call_effective_params(shared<Node> node);
const Class *node_call_return_type(shared<Node> node);

ExpressionBuffer *cur_exp_buf = nullptr;

void crash() {
	int *p = nullptr;
	*p = 4;
}


extern const Class *TypeIntList;
extern const Class *TypeAnyList;
extern const Class *TypeAnyDict;
extern const Class *TypeDynamicArray;
extern const Class *TypeIntDict;
extern const Class *TypeStringAutoCast;
extern const Class *TypePath;

const int TYPE_CAST_NONE = -1;
const int TYPE_CAST_DEREFERENCE = -2;
const int TYPE_CAST_REFERENCE = -3;
const int TYPE_CAST_OWN_STRING = -10;
const int TYPE_CAST_ABSTRACT_LIST = -20;
const int TYPE_CAST_ABSTRACT_TUPLE = -30;
const int TYPE_CAST_TUPLE_AS_CONSTRUCTOR = -31;
const int TYPE_CAST_FUNCTION_AS_CALLABLE = -32;
const int TYPE_CAST_MAKE_SHARED = -40;
const int TYPE_CAST_MAKE_OWNED = -41;



struct CastingData {
	int cast;
	int penalty;
	Function *f;
};

bool type_match(const Class *given, const Class *wanted);
bool type_match_with_cast(shared<Node> node, bool is_modifiable, const Class *wanted, CastingData &cd);



#if 0
bool is_function_pointer(const Class *c) {
	if (c ==  TypeFunctionP)
		return true;
	return is_typed_function_pointer(c);
}
#endif


const Class *give_useful_type(Parser *p, shared<Node> node) {
	if (node->type == TypeUnknown)
		return p->force_concrete_type(node)->type;
	return node->type;
}


// usable for pointer AND Callable class!
Array<const Class*> get_callable_param_types(const Class *fp) {
	if (fp->is_pointer())
		return fp->param[0]->param.sub_ref(0, -1); // skip return value
	return fp->param.sub_ref(0, -1); // skip return value
}

const Class *get_callable_return_type(const Class *fp) {
	if (fp->is_pointer())
		return fp->param[0]->param.back();
	return fp->param.back();
}

Array<const Class*> get_callable_capture_types(const Class *fp) {
	if (fp->is_pointer())
		return get_callable_capture_types(fp->param[0]);
	Array<const Class*> binds;
	for (auto &e: fp->elements)
		if (e.name.head(7) == "capture")
			binds.add(e.type);
	return binds;
}

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

Parser::Parser(SyntaxTree *t) {
	tree = t;
	cur_func = nullptr;
	for_index_count = 0;
	Exp.cur_line = nullptr;
	parser_loop_depth = 0;
	found_dynamic_param = false;
}


void Parser::parse_buffer(const string &buffer, bool just_analyse) {
	Exp.analyse(tree, buffer);

	parse_macros(just_analyse);

	parse();

	Exp.clear();

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

const Class *get_user_friendly_type(shared<Node> operand) {
	const Class *type = operand->type;
	bool deref = false;
	bool only_static = false;

	if (operand->kind == NodeKind::CLASS) {
		// referencing class functions
		return operand->as_class();
	} else if (type->is_some_pointer()) {
		return type->param[0];
	}
	return type;
}

shared<Node> Parser::parse_abstract_operand_extension_element(shared<Node> operand) {
	Exp.next(); // .

	auto el = new Node(NodeKind::ABSTRACT_ELEMENT, 0, TypeUnknown);
	el->token_id = Exp.cur_token();
	el->set_num_params(2);
	el->set_param(0, operand);
	el->set_param(1, create_node_token(this));
	Exp.next();
	return el;
}

shared<Node> Parser::parse_abstract_operand_extension_dict(shared<Node> operand) {
	Exp.next(); // "{"

	if (Exp.cur != "}")
		do_error_exp("'}' expected after dict 'class{'");

	auto node = new Node(NodeKind::ABSTRACT_TYPE_DICT, 0, TypeUnknown);
	node->token_id = Exp.cur_token();
	Exp.next();
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
	node->token_id = Exp.cur_token();
	Exp.next(); // "*"
	node->set_num_params(1);
	node->set_param(0, operand);
	return node;
}

shared<Node> Parser::parse_abstract_operand_extension_array(shared<Node> operand, Block *block) {
	int token0 = Exp.cur_token();
	// array index...
	Exp.next();


	if (Exp.cur == "]") {
		auto node = new Node(NodeKind::ABSTRACT_TYPE_LIST, 0, TypeUnknown, false, token0);
		Exp.next();
		node->set_num_params(1);
		node->set_param(0, operand);
		return node;
	}

	shared<Node> index;
	shared<Node> index2;
	if (Exp.cur == ":") {
		index = tree->add_node_const(tree->add_constant_int(0));
	} else {
		index = parse_abstract_operand_greedy(block);
	}
	if (Exp.cur == ":") {
		Exp.next();
		if (Exp.cur == "]") {
			index2 = tree->add_node_const(tree->add_constant_int(DynamicArray::MAGIC_END_INDEX));
			// magic value (-_-)'
		} else {
			index2 = parse_abstract_operand_greedy(block);
		}
	}
	if (Exp.cur != "]")
		do_error_exp("']' expected after array index");
	Exp.next();

	auto array = tree->add_node_array(operand, index, TypeUnknown);
	if (index2) {
		array->set_num_params(3);
		array->set_param(2, index2);
	}
	return array;
}

shared<Node> Parser::make_func_node_callable(const shared<Node> l) {
	Function *f = l->as_func();
	//auto r = tree->add_node_call(f);
	auto r = l->shallow_copy();
	r->kind = NodeKind::CALL_FUNCTION;
	r->type = f->literal_return_type;
	r->set_num_params(f->num_params);

	// virtual?
	if (f->virtual_index >= 0)
		r->kind = NodeKind::CALL_VIRTUAL;
	return r;
}

shared<Node> Parser::make_func_pointer_node_callable(const shared<Node> l) {
	auto f = l->type->param[0]->get_call();

	shared<Node> c;
	if (f->virtual_index >= 0) {
		c = new Node(NodeKind::CALL_VIRTUAL, (int_p)f, f->literal_return_type, true);
	} else {
		do_error("function pointer call should be virtual???", l);
		c = new Node(NodeKind::CALL_FUNCTION, (int_p)f, f->literal_return_type, true);
	}
	c->set_num_params(f->num_params);
	c->set_instance(l->deref());
	return c;
}

shared<Node> SyntaxTree::make_fake_constructor(const Class *t, const Class *param_type, int token_id) {
	//if ((t == TypeInt) and (param_type == TypeFloat32))
	//	return add_node_call(get_existence("f2i", nullptr, nullptr, false)[0]->as_func());
	if (param_type->is_pointer())
		param_type = param_type->param[0];
		
	string fname = "__" + t->name + "__";
	auto *cf = param_type->get_member_func(fname, t, {});
	if (!cf)
		do_error(format("illegal fake constructor... requires '%s.%s()'", param_type->long_name(), fname), token_id);
	return add_node_member_call(cf, nullptr, token_id); // temp var added later...
		
	auto *dummy = new Node(NodeKind::PLACEHOLDER, 0, TypeVoid);
	return add_node_member_call(cf, dummy, token_id); // temp var added later...
}

shared<Node> SyntaxTree::add_node_constructor(Function *f, int token_id) {
	auto *dummy = new Node(NodeKind::PLACEHOLDER, 0, f->name_space, false);
	auto n = add_node_member_call(f, dummy, token_id); // temp var added later...
	n->kind = NodeKind::CONSTRUCTOR_AS_FUNCTION;
	n->type = f->name_space;
	return n;
}

shared_array<Node> Parser::turn_class_into_constructor(const Class *t, const shared_array<Node> &params, int token_id) {
	if (((t == TypeInt) or (t == TypeFloat32) or (t == TypeInt64) or (t == TypeFloat64) or (t == TypeBool) or (t == TypeChar)) and (params.num == 1))
		return {tree->make_fake_constructor(t, params[0]->type, token_id)};
	
	// constructor
	//auto *vv = block->add_var(block->function->create_slightly_hidden_name(), t);
	//vv->explicitly_constructed = true;
	//shared<Node> dummy = add_node_local(vv);
	shared_array<Node> links;
	for (auto *cf: t->get_constructors())
		if ((params.num >= cf->mandatory_params-1) and (params.num <= cf->num_params-1)) // skip "self"
			links.add(tree->add_node_constructor(cf, token_id));
	if (links.num == 0) {
		for (auto *cf: t->get_constructors()) {
			msg_write(cf->signature(TypeVoid));
			msg_write(cf->mandatory_params);
		}
		do_error(format("class %s does not have a constructor with %d parameters", t->long_name(), params.num), token_id);
	}
	return links;
}

Array<const Class*> Parser::type_list_from_nodes(const shared_array<Node> &nn) {
	Array<const Class*> t;
	for (auto &n: nn)
		t.add(force_concrete_type(n)->type);
	return t;
}

string type_list_to_str(const Array<const Class*> &tt) {
	string s;
	for (auto *t: tt) {
		if (s.num > 0)
			s += ", ";
		if (t)
			s += t->long_name();
		else
			s += "<nil>";
	}
	return "(" + s + ")";
}

shared<Node> check_const_params(SyntaxTree *tree, shared<Node> n) {
	if ((n->kind == NodeKind::CALL_FUNCTION) or (n->kind == NodeKind::CALL_VIRTUAL)) {
		auto f = n->as_func();
		int offset = 0;
		if (f->is_member()) {
			offset = 1;
			if (f->is_selfref()) {
				// const(return) = const(instance)
				n->is_const = n->params[0]->is_const;
			} else if (n->params[0]->is_const and !f->is_const()) {
				//n->show();
				tree->do_error(f->long_name() + ": member function expects a mutable instance, because it is declared without 'const'", n->token_id);
			}
		}
		for (int i=offset; i<f->num_params; i++)
			if (n->params[i]->is_const and !f->var[i]->is_const())
				tree->do_error(format("%s: function parameter %d ('%s') is 'out' and does not accept a constant value", f->long_name(), i+1-offset, f->var[i]->name), n->token_id);
	}
	return n;
}

bool node_is_member_function_with_instance(shared<Node> n) {
	if (!n->is_function())
		return false;
	auto *f = n->as_func();
	if (f->is_static())
		return false;
	return n->params.num == 0 or n->params[0];
}

shared<Node> Parser::try_to_match_apply_params(const shared_array<Node> &links, shared_array<Node> &_params) {

	//force_concrete_types(params);
	auto params = _params;
	if (node_is_member_function_with_instance(links[0])) {
		params.insert(links[0]->params[0], 0);

	}

	// direct match...
	for (shared<Node> operand: links) {
		if (!direct_param_match(operand, params))
			continue;

		return check_const_params(tree, apply_params_direct(operand, params));
	}


	// advanced match...
	Array<CastingData> casts;
	Array<const Class*> wanted;
	int min_penalty = 1000000;
	shared<Node> chosen;
	for (auto operand: links) {
		Array<CastingData> cur_casts;
		Array<const Class*> cur_wanted;
		int cur_penalty;
		if (!param_match_with_cast(operand, params, cur_casts, cur_wanted, &cur_penalty))
			continue;
		if (cur_penalty < min_penalty){
			casts = cur_casts;
			wanted = cur_wanted;
			chosen = operand;
			min_penalty = cur_penalty;
		}
	}

	if (chosen)
		return check_const_params(tree, apply_params_with_cast(chosen, params, casts, wanted));


	// error message

	if (links.num == 0)
		do_error("can not call ...WTF??", -1); //, links[0]);

	if (links.num == 1) {
		param_match_with_cast(links[0], params, casts, wanted, &min_penalty);
		do_error("invalid function parameters: " + param_match_with_cast_error(params, wanted), links[0]);
	}

	string found = type_list_to_str(type_list_from_nodes(params));
	string available;
	for (auto link: links) {
		//auto p = get_wanted_param_types(link);
		//available += format("\n * %s for %s", type_list_to_str(p), link->sig(tree->base_class));
		available += format("\n * %s", link->signature(tree->base_class));
	}
	do_error(format("invalid function parameters: %s given, possible options:%s", found, available), links[0]);
	return shared<Node>();
}

shared<Node> Parser::parse_abstract_operand_extension_call(shared<Node> link, Block *block) {

	// parse all parameters
	auto params = parse_abstract_call_parameters(block);

	auto node = new Node(NodeKind::ABSTRACT_CALL, 0, TypeUnknown);
	node->set_num_params(params.num + 1);
	node->set_param(0, link);
	foreachi (auto p, params, i)
		node->set_param(i + 1, p);

	return node;
}

bool is_type_tuple(const shared<Node> n) {
	if (n->kind != NodeKind::TUPLE)
		return false;
	for (auto p: weak(n->params))
		if (p->kind != NodeKind::CLASS)
			return false;
	return true;
}

Array<const Class*> class_tuple_extract_classes(const shared<Node> n) {
	Array<const Class*> classes;
	for (auto p: weak(n->params))
		classes.add(p->as_class());
	return classes;
}

Array<const Class*> node_extract_param_types(const shared<Node> n) {
	Array<const Class*> classes;
	for (auto p: weak(n->params))
		classes.add(p->type);
	return classes;
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

			return parse_operand_extension({tree->add_node_class(t)}, block, prefer_type);
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

		return parse_operand_extension({tree->add_node_class(t)}, block, prefer_type);
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
	} else if (Exp.cur == IDENTIFIER_SHARED or Exp.cur == IDENTIFIER_OWNED) {
		auto sub = operand;
		if (Exp.cur == IDENTIFIER_SHARED) {
			operand = new Node(NodeKind::ABSTRACT_TYPE_SHARED, 0, TypeUnknown);
		} else { //if (pre == IDENTIFIER_OWNED)
			operand = new Node(NodeKind::ABSTRACT_TYPE_OWNED, 0, TypeUnknown);
		}
		operand->token_id = Exp.cur_token();
		Exp.next();
		operand->set_num_params(1);
		operand->set_param(0, sub);
		return parse_abstract_operand_extension(operand, block, true);
	} else {

		if ((Exp.cur == "*" and (prefer_class or no_identifier_after())) or Exp.cur == "ptr") {
			// FIXME: false positives for "{{pi * 10}}"
			return parse_abstract_operand_extension(parse_abstract_operand_extension_pointer(operand), block, true);
		}
		// unary operator? (++,--)

		auto op = parse_abstract_operator(1);
		if (op) {
			op->set_num_params(1);
			op->set_param(0, operand);
			return parse_abstract_operand_extension(op, block, prefer_class);
		}
		return operand;
	}

	// recursion
	return parse_abstract_operand_extension(operand, block, prefer_class);
}


// when calling ...(...)
Array<const Class*> Parser::get_wanted_param_types(shared<Node> link, int &mandatory_params) {
	if (link->is_function()) {
		auto f = link->as_func();
		mandatory_params = f->mandatory_params;
		auto p = f->literal_param_type;
		/*if (f->is_member() and (link->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION))
			if (link->params.num == 0 or !link->params[0])
				p.insert(f->name_space, 0);*/
		if (link->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION) {
			//msg_write("FIXME CONSTR AS FUNC");
			// TODO: what is better, giving here an "effective" parameter set, or offseting from the outside?
		}
		return p;
	} else if (link->kind == NodeKind::CLASS) {
		// should be caught earlier and turned to func...
		const Class *t = link->as_class();
		for (auto *c: t->get_constructors()) {
			mandatory_params = c->num_params;
			return c->literal_param_type;
		}
	/*} else if (link->kind == NodeKind::CALL_RAW_POINTER) {
		return get_callable_param_types(link->params[0]->type);*/
	} else {
		do_error("evil function...kind: "+kind2str(link->kind), link);
	}

	return {};
}

shared_array<Node> Parser::parse_abstract_call_parameters(Block *block) {
	if (Exp.cur != "(")
		do_error_exp("'(' expected in front of function parameter list");

	Exp.next();

	shared_array<Node> params;

	// list of parameters
	for (int p=0;;p++) {
		if (Exp.cur == ")")
			break;

		// find parameter
		params.add(parse_abstract_operand_greedy(block));

		if (Exp.cur != ",") {
			if (Exp.cur == ")")
				break;
			do_error_exp("',' or ')' expected after parameter for function");
		}
		Exp.next();
	}
	Exp.next(); // ')'
	return params;
}



// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
shared<Node> Parser::check_param_link(shared<Node> link, const Class *wanted, const string &f_name, int param_no, int num_params) {
	// type cast needed and possible?
	const Class *given = link->type;

	if (type_match(given, wanted))
		return link;

	CastingData cast;
	if (type_match_with_cast(link, false, wanted, cast))
		return apply_type_cast(cast, link, wanted);

	Exp.rewind();
	if (num_params > 1)
		do_error(format("command '%s': type '%s' given, '%s' expected for parameter #%d", f_name, given->long_name(), wanted->long_name(), param_no + 1), link);
	else
		do_error(format("command '%s': type '%s' given, '%s' expected", f_name, given->long_name(), wanted->long_name()), link);
	return link;
}

bool Parser::direct_param_match(const shared<Node> operand, const shared_array<Node> &params) {
	int mandatory_params;
	auto wanted_types = get_wanted_param_types(operand, mandatory_params);
	if (wanted_types.num != params.num)
		return false;
	for (auto c: wanted_types)
		if (c == TypeDynamic)
			found_dynamic_param = true;
	for (int p=0; p<params.num; p++) {
		if (!type_match(params[p]->type, wanted_types[p]))
			return false;
	}
	return true;
}

bool Parser::param_match_with_cast(const shared<Node> operand, const shared_array<Node> &params, Array<CastingData> &casts, Array<const Class*> &wanted, int *max_penalty) {
	int mandatory_params;
	wanted = get_wanted_param_types(operand, mandatory_params);
	if ((params.num < mandatory_params) or (params.num > wanted.num))
		return false;
	casts.resize(params.num);
	*max_penalty = 0;
	for (int p=0; p<params.num; p++) {
		int penalty;
		if (!type_match_with_cast(params[p], false, wanted[p], casts[p]))
			return false;
		*max_penalty = max(*max_penalty, casts[p].penalty);
	}
	return true;
}

string Parser::param_match_with_cast_error(const shared_array<Node> &params, const Array<const Class*> &wanted) {
	if (wanted.num != params.num)
		return format("%d parameters given, %d expected", params.num, wanted.num);
	for (int p=0; p<params.num; p++) {
		CastingData cast;
		if (!type_match_with_cast(params[p], false, wanted[p], cast)) {
			auto pt = give_useful_type(this, params[p]);
			if (params.num > 1)
				return format("type '%s' given, '%s' expected for parameter #%d", pt->long_name(), wanted[p]->long_name(), p+1);
			else
				return format("type '%s' given, '%s' expected    %s  %s", pt->long_name(), wanted[p]->long_name(), p2s(pt), p2s(wanted[p]));
		}
	}
	return "";
}

shared<Node> Parser::apply_params_direct(shared<Node> operand, const shared_array<Node> &params, int offset) {
	auto r = operand->shallow_copy();
	for (int p=0; p<params.num; p++)
		r->set_param(p+offset, params[p]);
	return r;
}

shared<Node> Parser::apply_params_with_cast(shared<Node> operand, const shared_array<Node> &params, const Array<CastingData> &casts, const Array<const Class*> &wanted, int offset) {
	auto r = operand->shallow_copy();
	for (int p=0; p<params.num; p++) {
		auto pp = apply_type_cast(casts[p], params[p], wanted[p]);
		r->set_param(p+offset, pp);
	}

	// default values
	if (operand->is_function()) {
		auto f = operand->as_func();
		for (int p=params.num+offset; p<f->num_params; p++) {
			r->set_param(p, f->default_parameters[p]);
		}
	}
	return r;
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

shared<Node> Parser::link_unary_operator(AbstractOperator *po, shared<Node> operand, Block *block, int token_id) {
	Operator *op = nullptr;
	const Class *p1 = operand->type;

	// exact match?
	bool ok=false;
	for (auto *_op: tree->operators)
		if (po == _op->abstract)
			if ((!_op->param_type_2) and (type_match(p1, _op->param_type_1))) {
				op = _op;
				ok = true;
				break;
			}


	// needs type casting?
	if (!ok) {
		CastingData current;
		CastingData best = {-1, 10000};
		const Class *t_best = nullptr;
		for (auto *_op: tree->operators)
			if (po == _op->abstract)
				if ((!_op->param_type_2) and (type_match_with_cast(operand, false, _op->param_type_1, current))) {
					ok = true;
					if (current.penalty < best.penalty) {
						op = _op;
						best = current;
						t_best = _op->param_type_1;
					}
			}
		// cast
		if (ok) {
			operand = apply_type_cast(best, operand, t_best);
		}
	}


	if (!ok)
		do_error(format("unknown unitary operator '%s %s'", po->name, p1->long_name()), token_id);
	return tree->add_node_operator(op, operand, nullptr, token_id);
}

shared<Node> Parser::parse_abstract_set_builder(Block *block) {
	//Exp.next(); // [
	auto n_for = parse_abstract_for_header(block);

	auto n_exp = parse_abstract_operand_greedy(block);
	
	shared<Node> n_cmp;
	if (Exp.cur == IDENTIFIER_IF) {
		Exp.next(); // if
		n_cmp = parse_abstract_operand_greedy(block);
	}

	if (Exp.cur != "]")
		do_error_exp("] expected");
	Exp.next();

	auto n = new Node(NodeKind::ARRAY_BUILDER_FOR, 0, TypeUnknown);
	n->set_num_params(3);
	n->set_param(0, n_for);
	n->set_param(1, n_exp);
	n->set_param(2, n_cmp);
	return n;

}


shared<Node> Parser::apply_format(shared<Node> n, const string &fmt) {
	auto f = n->type->get_member_func("format", TypeString, {TypeString});
	if (!f)
		do_error(format("format string: no '%s.format(string)' function found", n->type->long_name()), n);
	auto *c = tree->add_constant(TypeString);
	c->as_string() = fmt;
	auto nf = tree->add_node_call(f, n->token_id);
	nf->set_instance(n);
	nf->set_param(1, tree->add_node_const(c, n->token_id));
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
			parts.add(tree->add_node_const(c, token_id));
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
			n = deref_if_pointer(n);

			if (fmt != "") {
				n = apply_format(n, fmt);
			} else {
				n = check_param_link(n, TypeStringAutoCast, "", 0, 1);
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
		return tree->add_node_const(c, token_id);
	}
	
	// glue
	while (parts.num > 1) {
		auto b = parts.pop();
		auto a = parts.pop();
		auto n = link_operator_id(OperatorID::ADD, a, b, token_id);
		parts.add(n);
	}
	//parts[0]->show();
	return parts[0];
}

shared<Node> Parser::parse_abstract_list(Block *block) {
	shared_array<Node> el;
	while(true) {
		if (Exp.cur == "]")
			break;
		el.add(parse_abstract_operand_greedy(block));
		if ((Exp.cur != ",") and (Exp.cur != "]"))
			do_error_exp("',' or ']' expected");
		if (Exp.cur == "]")
			break;
		Exp.next();
	}
	Exp.next();
	return build_abstract_list(el);
}

shared<Node> Parser::parse_abstract_dict(Block *block) {
	Exp.next(); // {
	shared_array<Node> el;
	while(true) {
		if (Exp.cur == "}")
			break;
		el.add(parse_abstract_operand_greedy(block)); // key
		if (Exp.cur != ":")
			do_error_exp("':' after key expected");
		Exp.next();
		el.add(parse_abstract_operand_greedy(block)); // value
		if (Exp.cur == "}")
			break;
		Exp.next();
	}
	Exp.next();
	return build_abstract_dict(el);
}

const Class *make_pointer_shared(SyntaxTree *tree, const Class *parent, int token_id) {
	if (!parent->name_space)
		tree->do_error("shared not allowed for: " + parent->long_name(), token_id); // TODO
	return tree->make_class(parent->name + " " + IDENTIFIER_SHARED, Class::Type::POINTER_SHARED, config.pointer_size, 0, nullptr, {parent}, parent->name_space, token_id);
}

const Class *make_pointer_owned(SyntaxTree *tree, const Class *parent, int token_id) {
	if (!parent->name_space)
		tree->do_error("owned not allowed for: " + parent->long_name(), token_id);
	return tree->make_class(parent->name + " " + IDENTIFIER_OWNED, Class::Type::POINTER_OWNED, config.pointer_size, 0, nullptr, {parent}, parent->name_space, token_id);
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
	auto c = const_cast<Class*>(tree->make_class("("+name+")", Class::Type::PRODUCT, size, -1, nullptr, classes, tree->_base_class.get(), token_id));
	if (c->elements.num == 0) {
		int offset = 0;
		foreachi (auto &cc, classes, i) {
			c->elements.add(ClassElement("e" + i2s(i), cc, offset));
			offset += cc->size;
		}
		tree->add_missing_function_headers_for_class(c);
	}
	return c;

}

shared<Node> digest_type(SyntaxTree *tree, shared<Node> n) {
	if (!is_type_tuple(n))
		return n;
	auto classes = class_tuple_extract_classes(n);
	return tree->add_node_class(merge_type_tuple_into_product(tree, classes, n->token_id), n->token_id);
}

shared<Node> create_node_token(Parser *p) {
	auto t = new Node(NodeKind::ABSTRACT_TOKEN, (int_p)&p->Exp, TypeUnknown);
	t->token_id = p->Exp.cur_token();
	return t;
}

// minimal operand
// but with A[...], A(...) etc
shared<Node> Parser::parse_abstract_operand(Block *block, bool prefer_class) {
	shared<Node> operand;

	// ( -> one level down and combine commands
	if (Exp.cur == "(") {
		Exp.next();
		operand = parse_abstract_operand_greedy(block, true);
		if (Exp.cur != ")")
			do_error_exp("')' expected");
		Exp.next();
	} else if (Exp.cur == "&") { // & -> address operator
		Exp.next();
		int token = Exp.cur_token();
		operand = parse_abstract_operand(block)->ref(TypeUnknown);
		operand->token_id = token;
	} else if (Exp.cur == "*") { // * -> dereference
		Exp.next();
		int token = Exp.cur_token();
		operand = parse_abstract_operand(block)->deref(TypeUnknown);
		operand->token_id = token;
	} else if (Exp.cur == "[") {
		Exp.next();
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
		operand = create_node_token(this);
		Exp.next();
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
	cmd->token_id = Exp.cur_token();

	Exp.next();
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

bool type_match_tuple_as_contructor(shared<Node> node, Function *f_constructor, int &penalty) {
	if (f_constructor->literal_param_type.num != node->params.num + 1)
		return false;

	penalty = 20;
	foreachi (auto *e, weak(node->params).sub_ref(1), i) {
		CastingData cast;
		if (!type_match_with_cast(e, false, f_constructor->literal_param_type[i], cast))
			return false;
		penalty += cast.penalty;
	}
	return true;
}

const Class *make_effective_class_callable(shared<Node> node) {
	auto f = node->as_func();
	if (f->is_member() and node->params.num > 0 and node->params[0])
		return f->owner()->make_class_callable_fp(f->literal_param_type.sub_ref(1), f->literal_return_type, node->token_id);
	return f->owner()->make_class_callable_fp(f, node->token_id);
}

bool type_match_with_cast(shared<Node> node, bool is_modifiable, const Class *wanted, CastingData &cd) {
	cd.penalty = 0;
	auto given = node->type;
	cd.cast = TYPE_CAST_NONE;
	if (type_match(given, wanted))
		return true;
	if (wanted == TypeStringAutoCast and given == TypeString)
		return true;
	if (wanted == TypeString and given == TypePath)
		return true;
	if (is_modifiable) // is a variable getting assigned.... better not cast
		return false;
	if (given->is_pointer()) {
		if (type_match(given->param[0], wanted)) {
			cd.penalty = 10;
			cd.cast = TYPE_CAST_DEREFERENCE;
			return true;
		}
	}
	if (wanted->is_pointer_shared() and given->is_pointer()) {
		if (type_match(given->param[0], wanted->param[0])) {
			cd.penalty = 10;
			cd.cast = TYPE_CAST_MAKE_SHARED;
			return true;
		}
	}
	/*if (wanted->is_pointer_owned() and given->is_pointer()) {
		if (type_match(given->param[0], wanted->param[0])) {
			penalty = 10;
			cast = TYPE_CAST_MAKE_OWNED;
			return true;
		}
	}*/
	if (node->kind == NodeKind::ARRAY_BUILDER and given == TypeUnknown) {
		if (wanted->is_super_array()) {
			auto t = wanted->get_array_element();
			CastingData cast;
			for (auto *e: weak(node->params)) {
				if (!type_match_with_cast(e, false, t, cast))
					return false;
				cd.penalty += cast.penalty;
			}
			cd.cast = TYPE_CAST_ABSTRACT_LIST;
			return true;
		}
		if (wanted == TypeDynamicArray) {
			cd.cast = TYPE_CAST_ABSTRACT_LIST;
			return true;
		}
		// TODO: only for tuples
		for (auto *f: wanted->get_constructors()) {
			if (type_match_tuple_as_contructor(node, f, cd.penalty)) {
				cd.cast = TYPE_CAST_TUPLE_AS_CONSTRUCTOR;
				cd.f = f;
				return true;
			}
		}
	}
	if ((node->kind == NodeKind::TUPLE) and (given == TypeUnknown)) {

		for (auto *f: wanted->get_constructors()) {
			if (type_match_tuple_as_contructor(node, f, cd.penalty)) {
				cd.cast = TYPE_CAST_TUPLE_AS_CONSTRUCTOR;
				cd.f = f;
				return true;
			}
		}

		// FIXME this probably doesn't make sense... we should already know the wanted type!
		if (!wanted->is_product())
			return false;
		if (wanted->param.num != node->params.num)
			return false;
		for (int i=0; i<node->params.num; i++)
			if (!type_match(node->params[i]->type, wanted->param[i]))
				return false;
		msg_error("product");
		cd.cast = TYPE_CAST_ABSTRACT_TUPLE;
		return true;
	}
	if (wanted->is_callable() and (given == TypeUnknown)) {
		if (node->kind == NodeKind::FUNCTION) {
			auto ft = make_effective_class_callable(node);
			if (type_match(ft, wanted)) {
				cd.cast = TYPE_CAST_FUNCTION_AS_CALLABLE;
				return true;
			}
		}
	}
	if (wanted == TypeStringAutoCast) {
		//Function *cf = given->get_func(IDENTIFIER_FUNC_STR, TypeString, {});
		//if (cf) {
			cd.penalty = 50;
			cd.cast = TYPE_CAST_OWN_STRING;
			return true;
		//}
	}
	foreachi(auto &c, TypeCasts, i)
		if (type_match(given, c.source) and type_match(c.dest, wanted)) {
			cd.penalty = c.penalty;
			cd.cast = i;
			return true;
		}
	return false;
}

shared<Node> Parser::apply_type_cast(const CastingData &cast, shared<Node> node, const Class *wanted) {
	if (cast.cast == TYPE_CAST_NONE)
		return node;
	if (cast.cast == TYPE_CAST_DEREFERENCE)
		return node->deref();
	if (cast.cast == TYPE_CAST_REFERENCE)
		return node->ref();
	if (cast.cast == TYPE_CAST_OWN_STRING)
		return add_converter_str(node, false);
	if (cast.cast == TYPE_CAST_ABSTRACT_LIST) {
		if (wanted == TypeDynamicArray)
			return force_concrete_type(node);
		CastingData cd2;
		foreachi (auto e, node->params, i) {
			if (!type_match_with_cast(e, false, wanted->get_array_element(), cd2)) {
				do_error("nope????", node);
			}
			node->params[i] = apply_type_cast(cd2, e, wanted->get_array_element());
		}
		node->type = wanted;
		return node;
	}
	if (cast.cast == TYPE_CAST_ABSTRACT_TUPLE) {
		msg_error("AUTO TUPLE");
		node->type = wanted;
		return node;
	}
	if (cast.cast == TYPE_CAST_FUNCTION_AS_CALLABLE) {
		return force_concrete_type(node);
	}

	if (cast.cast == TYPE_CAST_TUPLE_AS_CONSTRUCTOR) {
		Array<CastingData> c;
		c.resize(node->params.num);
		auto f = cast.f;
		foreachi (auto e, node->params, i)
			if (!type_match_with_cast(e, false, f->literal_param_type[i+1], c[i])) { do_error("aaaaa", e); }
		auto cmd = tree->add_node_constructor(f);
		return apply_params_with_cast(cmd, node->params, c, f->literal_param_type, 1);
	}
	if ((cast.cast == TYPE_CAST_MAKE_SHARED) or (cast.cast == TYPE_CAST_MAKE_OWNED)) {
		auto f = wanted->get_func(IDENTIFIER_FUNC_SHARED_CREATE, wanted, {node->type});
		if (!f)
			do_error("make shared... create() missing...", node);
		auto nn = tree->add_node_call(f, node->token_id);
		nn->set_param(0, node);
		return nn;
	}
	
	auto c = tree->add_node_call(TypeCasts[cast.cast].f, node->token_id);
	c->type = TypeCasts[cast.cast].dest;
	c->set_param(0, node);
	return c;
}

shared<Node> Parser::link_special_operator_is(shared<Node> param1, shared<Node> param2, int token_id) {
	if (param2->kind != NodeKind::CLASS)
		do_error("class name expected after 'is'", param2);
	const Class *t2 = param2->as_class();
	if (t2->vtable.num == 0)
		do_error(format("class after 'is' needs to have virtual functions: '%s'", t2->long_name()), param2);

	const Class *t1 = param1->type;
	if (t1->is_pointer()) {
		param1 = param1->deref();
		t1 = t1->param[0];
	}
	if (!t2->is_derived_from(t1))
		do_error(format("'is': class '%s' is not derived from '%s'", t2->long_name(), t1->long_name()), token_id);

	// vtable2
	auto vtable2 = tree->add_node_const(tree->add_constant_pointer(TypePointer, t2->_vtable_location_compiler_), token_id);

	// vtable1
	param1->type = TypePointer;

	return tree->add_node_operator_by_inline(InlineID::POINTER_EQUAL, param1, vtable2, token_id);
}

shared<Node> Parser::link_special_operator_in(shared<Node> param1, shared<Node> param2, int token_id) {
	param2 = force_concrete_type(param2);
	auto *f = param2->type->get_member_func("__contains__", TypeBool, {param1->type});
	if (!f)
		do_error(format("no 'bool %s.__contains__(%s)' found", param2->type->long_name(), param1->type->long_name()), token_id);

	auto n = tree->add_node_member_call(f, param2, token_id);
	n->set_param(1, param1);
	return n;
}

shared<Node> explicit_cast(Parser *p, shared<Node> node, const Class *wanted) {
	auto type = node->type;
	if (type == wanted)
		return node;

	CastingData cast;
	if (type_match_with_cast(node, false, wanted, cast)) {
		if (cast.cast == TYPE_CAST_NONE) {
			auto c = node->shallow_copy();
			c->type = wanted;
			return c;
		}
		return p->apply_type_cast(cast, node, wanted);
	}

	// explicit pointer cast
	if (wanted->is_some_pointer() and type->is_some_pointer()) {
		node->type = wanted;
		return node;
	}

	if (wanted == TypeString)
		return p->add_converter_str(node, false);

	if (type->get_member_func("__" + wanted->name + "__", wanted, {})) {
		auto rrr = p->turn_class_into_constructor(wanted, {node}, node->token_id);
		if (rrr.num > 0) {
			rrr[0]->set_param(0, node);
			return rrr[0];
		}
	}

	p->do_error(format("can not cast expression of type '%s' to type '%s'", node->type->long_name(), wanted->long_name()), node);
	return nullptr;
}

shared<Node> Parser::link_special_operator_as(shared<Node> param1, shared<Node> param2, int token_id) {
	if (param2->kind != NodeKind::CLASS)
		do_error("class name expected after 'as'", param2);
	auto wanted = param2->as_class();
	return explicit_cast(this, param1, wanted);
}

shared<Node> Parser::link_operator_id(OperatorID op_no, shared<Node> param1, shared<Node> param2, int token_id) {
	return link_operator(&abstract_operators[(int)op_no], param1, param2, token_id);
}

shared<Node> Parser::link_operator(AbstractOperator *primop, shared<Node> param1, shared<Node> param2, int token_id) {
	bool left_modifiable = primop->left_modifiable;
	bool order_inverted = primop->order_inverted;
	string op_func_name = primop->function_name;
	shared<Node> op;

	if (primop->left_modifiable and param1->is_const)
		do_error("trying to modify a constant expression", token_id);

	if (primop->id == OperatorID::IS)
		return link_special_operator_is(param1, param2, token_id);
	if (primop->id == OperatorID::IN)
		return link_special_operator_in(param1, param2, token_id);
	if (primop->id == OperatorID::AS)
		return link_special_operator_as(param1, param2, token_id);


	auto *p1 = param1->type;
	auto *p2 = param2->type;

	const Class *pp1 = p1;
	if (pp1->is_pointer())
		pp1 = p1->param[0];

	if (primop->id == OperatorID::ASSIGN) {
		//param1->show();
		if (param1->kind == NodeKind::CALL_FUNCTION) {
			auto f = param1->as_func();
			if (f->name == IDENTIFIER_FUNC_GET) {
				auto inst = param1->params[0];
				auto index = param1->params[1];
				//msg_write(format("[]=...    void %s.__set__(%s, %s)?", inst->type->long_name(), index->type->long_name(), p2->long_name()));
				for (auto *ff: weak(inst->type->functions))
					if (ff->name == IDENTIFIER_FUNC_SET and ff->literal_return_type == TypeVoid and ff->num_params == 3) {
						if (ff->literal_param_type[1] != index->type)
							continue;
						CastingData cast;
						if (!type_match_with_cast(param2, false, ff->literal_param_type[2], cast))
							continue;
						//msg_write(ff->signature());
						auto nn = tree->add_node_member_call(ff, inst, token_id);
						nn->set_param(1, index);
						nn->set_param(2, apply_type_cast(cast, param2, ff->literal_param_type[2]));
						return nn;
					}
			}
		}
	}

	// exact match as class function?
	for (auto *f: weak(pp1->functions))
		if ((f->name == op_func_name) and f->is_member()) {
			// exact match as class function but missing a "&"?
			if (f->literal_param_type.num != 2)
				continue;

			auto type2 = f->literal_param_type[1];
			if (type_match(p2, type2)) {
				auto inst = param1;
				if (p1 == pp1)
					op = tree->add_node_member_call(f, inst, token_id);
				else
					op = tree->add_node_member_call(f, inst->deref(), token_id);
				op->set_param(1, param2);
				return op;
			}
		}

	// exact (operator) match?
	// FIXME don't auto cast into arbitrary crap...
	/*for (auto *op: tree->operators)
		if (primop == op->primitive)
			if (type_match(p1, op->param_type_1) and type_match(p2, op->param_type_2)) {
				// UNUSED ANYWAY???
	//			return tree->add_node_operator(op, param1, param2);
			}*/


	// needs type casting?
	CastingData c1 = {TYPE_CAST_NONE, 0};
	CastingData c2 = {TYPE_CAST_NONE, 0};
	CastingData c1_best = {TYPE_CAST_NONE, 1000};
	CastingData c2_best = {TYPE_CAST_NONE, 1000};
	const Class *t1_best = nullptr, *t2_best = nullptr;
	Operator *op_found = nullptr;
	Function *op_cf_found = nullptr;
	for (auto *op: tree->operators)
		if (primop == op->abstract)
			if (type_match_with_cast(param1, left_modifiable, op->param_type_1,  c1) and type_match_with_cast(param2, false, op->param_type_2, c2))
				if (c1.penalty + c2.penalty < c1_best.penalty + c2_best.penalty) {
					op_found = op;
					c1_best = c1;
					c2_best = c2;
					t1_best = op->param_type_1;
					t2_best = op->param_type_2;
				}
	for (auto *cf: weak(p1->functions))
		if (cf->name == op_func_name)
			if (type_match_with_cast(param2, false, cf->literal_param_type[1], c2))
				if (c2.penalty < c2_best.penalty) {
					op_cf_found = cf;
					c1_best.cast = TYPE_CAST_NONE;
					c2_best = c2;
					t2_best = cf->literal_param_type[1];
				}
	// cast
	if (op_found or op_cf_found) {
		param1 = apply_type_cast(c1_best, param1, t1_best);
		param2 = apply_type_cast(c2_best, param2, t2_best);
		if (op_cf_found) {
			op = tree->add_node_member_call(op_cf_found, param1, token_id);
			op->set_param(1, param2);
		} else {
			return tree->add_node_operator(op_found, param1, param2, token_id);
		}
		return op;
	}

	return nullptr;
}

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

shared<Node> Parser::build_function_pipe(const shared<Node> &input, const shared<Node> &func) {

	if (func->kind != NodeKind::FUNCTION)
		do_error("function expected after '|>", func);
	auto f = func->as_func();
	if (f->num_params != 1)
		do_error("function after '|>' needs exactly 1 parameter (including self)", func);
	//if (f->literal_param_type[0] != input->type)
	//	do_error("pipe type mismatch...");

	auto out = tree->add_node_call(f, func->token_id);

	shared_array<Node> inputs;
	inputs.add(input);

	Array<CastingData> casts;
	Array<const Class*> wanted;
	int penalty;
	if (!param_match_with_cast(out, {input}, casts, wanted, &penalty))
		do_error("pipe: " + param_match_with_cast_error({input}, wanted), func);
	return check_const_params(tree, apply_params_with_cast(out, {input}, casts, wanted));
	//auto out = p->tree->add_node_call(f);
	//out->set_param(0, input);
	//return out;
}


shared<Node> Parser::build_lambda_new(const shared<Node> &param, const shared<Node> &expression) {
	do_error("abstract lambda not implemented yet", param);
	return nullptr;
}

inline void concretify_all_params(shared<Node> &node, Block *block, const Class *ns, Parser *parser) {
	for (int p=0; p<node->params.num; p++)
		if (node->params[p]->type == TypeUnknown) {
			node->params[p] = parser->concretify_node(node->params[p], block, ns);
		}
};


shared<Node> Parser::concretify_call(shared<Node> node, Block *block, const Class *ns) {
	//concretify_all_params(node, block, ns, this);
	auto links = concretify_node_multi(node->params[0], block, ns);
	for (int p=1; p<node->params.num; p++)
		if (node->params[p]->type == TypeUnknown)
			node->params[p] = concretify_node(node->params[p], block, ns);

	auto params = node->params.sub_ref(1);


	// make links callable
	foreachi (auto l, weak(links), i) {
		if (l->kind == NodeKind::FUNCTION) {
			links[i] = make_func_node_callable(l);
		} else if (l->kind == NodeKind::CLASS) {
			auto *t = l->as_class();
			return try_to_match_apply_params(turn_class_into_constructor(t, params, node->token_id), params);
#if 0
		} else if (is_typed_function_pointer(l->type)) {
			auto c = new Node(NodeKind::POINTER_CALL, 0, get_function_pointer_return_type(l->type));
			c->set_num_params(1 + get_function_pointer_param_types(l->type).num);
			c->set_param(0, l);
			return try_to_match_apply_params({c}, params);
		/*} else if (l->type == TypeFunctionCodeP) {
			auto c = new Node(NodeKind::POINTER_CALL, 0, TypeVoid);
			c->set_num_params(1);
			c->set_param(0, l);
			return try_to_match_params({c});*/
#endif
		} else if (l->type->is_callable()) {
			links[i] = make_func_pointer_node_callable(l);
			//return tree->add_node_member_call(l->type->param[0]->get_call(), l->deref(), params);
		} else {
			do_error("can't call " + kind2str(l->kind), l);
		}
	}
	return try_to_match_apply_params(links, params);
}

shared_array<Node> Parser::concretify_element(shared<Node> node, Block *block, const Class *ns) {
	auto base = concretify_node(node->params[0], block, ns);
	auto el = Exp.get_token(node->params[1]->token_id);

	base = force_concrete_type(base);
	auto links = tree->get_element_of(base, el);
	if (links.num > 0)
		return links;

	if (base->kind == NodeKind::CLASS) {
		auto c = tree->add_node_const(tree->add_constant(TypeClassP), node->token_id);
		c->as_const()->as_int64() = (int_p)base->as_class();

		auto links = tree->get_element_of(c, el);
		if (links.num > 0)
			return links;
	}

	if (base->kind == NodeKind::FUNCTION) {
		msg_write("FFF");
	}

	do_error(format("unknown element of '%s'", get_user_friendly_type(base)->long_name()), node->params[1]);
	return {};
}


class TemplateManager {
public:
	static void add_template(Function *f) {
		if (config.verbose)
			msg_write("ADD TEMPLATE");
		Template t;
		t.func = f;
		templates.add(t);
	}

	static Function *full_copy(Parser *parser, Function *f0) {
		auto f = f0->create_dummy_clone(f0->name_space);
		f->block = (Block*)parser->tree->cp_node(f0->block.get()).get();
		f->needs_overriding = false;

		auto convert = [f](shared<Node> n) {
			if (n->kind != NodeKind::BLOCK)
				return n;
			auto b = n->as_block();
			foreachi (auto v, b->vars, vi) {
				int i = weak(b->function->var).find(v);
				//msg_write(i);
				b->vars[vi] = f->var[i].get();
			}
			b->function = f;
			return n;
		};

		convert(f->block.get());
		parser->tree->transform_block(f->block.get(), convert);

		f->abstract_param_types = f0->abstract_param_types;
		f->abstract_return_type = f0->abstract_return_type;
		return f;
	}

	static Function *instantiate(Parser *parser, Function *f0, const Array<const Class*> &params) {
		if (config.verbose)
			msg_write("INSTANTIATE TEMPLATE");
		auto f = full_copy(parser, f0);
		//f->block->show();

		/*for (auto pp: weak(f->abstract_param_types))
			pp->show();
		msg_write(params[0]->name);*/

		// lazy experiment...
		//f->literal_param_type[0] = params[0];
		//f->literal_return_type = params[0];
		//msg_write("INSTANTIATE 01");
		//msg_write(f0->abstract_param_types.num);
		f->abstract_param_types[0] = parser->tree->add_node_class(params[0]);
		//msg_write("INSTANTIATE 02");
		f->abstract_return_type = parser->tree->add_node_class(params[0]);

		//msg_write("INSTANTIATE 2");

		parser->concretify_function_header(f);
		//msg_write("INSTANTIATE 3");

		f->update_parameters_after_parsing();

		//msg_write("INSTANTIATE 4");
		parser->concretify_function_body(f);
		//msg_write("INSTANTIATE 41");

		if (config.verbose)
			f->block->show();
		//msg_write("INSTANTIATE 42");

		auto ns = const_cast<Class*>(f0->name_space);
		ns->add_function(parser->tree, f, false, false);
		parser->tree->functions.add(f);

		//parser->do_error("template instantiate");
		return f;
	}

	struct Instance {
		Function *f;
		Array<const Class*> params;
	};
	struct Template {
		Function *func;
		Array<string> params;
		Array<Instance> instances;
	};
	static Array<Template> templates;
};
Array<TemplateManager::Template> TemplateManager::templates;

shared<Node> Parser::concretify_array(shared<Node> node, Block *block, const Class *ns) {
	auto operand = concretify_node(node->params[0], block, ns);
	auto index = concretify_node(node->params[1], block, ns);
	shared<Node> index2;
	if (node->params.num >= 3)
		index2 = concretify_node(node->params[2], block, ns);

	// int[3]
	if (operand->kind == NodeKind::CLASS) {
		// find array index
		index = tree->transform_node(index, [&](shared<Node> n) { return tree->conv_eval_const_func(n); });

		if ((index->kind != NodeKind::CONSTANT) or (index->type != TypeInt))
			do_error("only constants of type 'int' allowed for size of arrays", index);
		int array_size = index->as_const()->as_int();
		auto t = tree->make_class_array(operand->as_class(), array_size, operand->token_id);
		return tree->add_node_class(t);

	}

	// min[float]()
	if (operand->kind == NodeKind::FUNCTION) {
		auto links = concretify_node_multi(node->params[0], block, ns);
		if (index->kind != NodeKind::CLASS)
			do_error("functions can only be indexed by a type", index);
		auto t = index->as_class();
		for (auto l: weak(links)) {
			auto f = l->as_func();
			if (f->is_abstract) {
				auto ff = TemplateManager::instantiate(this, f, {t});
				if (ff)
					return tree->add_node_func_name(ff);
			} else {
				if (f->num_params >= 1)
					if (f->literal_param_type[0] == t)
						return l;
			}
		}
		do_error(format("function has no version [%s]", t->name), index);
	}


	// auto deref?
	if (operand->type->is_pointer()) {
		if (!operand->type->param[0]->is_array() and !operand->type->param[0]->usable_as_super_array())
			do_error(format("using pointer type '%s' as an array (like in C) is deprecated", operand->type->long_name()), index);
		operand = operand->deref();
	}


	// subarray() ?
	if (index2) {
		auto *cf = operand->type->get_member_func(IDENTIFIER_FUNC_SUBARRAY, operand->type, {index->type, index->type});
		if (cf) {
			auto f = tree->add_node_member_call(cf, operand, operand->token_id);
			f->is_const = operand->is_const;
			f->set_param(1, index);
			f->set_param(2, index2);
			return f;
		} else {
			do_error(format("function '%s.%s(int,int) -> %s' required by '[a:b]' missing", operand->type->name, IDENTIFIER_FUNC_SUBARRAY, operand->type->name), index);
		}
	}

	// __get__() ?
	auto *cf = operand->type->get_get(index->type);
	if (cf) {
		auto f = tree->add_node_member_call(cf, operand, operand->token_id);
		f->is_const = operand->is_const;
		f->set_param(1, index);
		return f;
	}

	// allowed?
	if (!operand->type->is_array() and !operand->type->usable_as_super_array())
		do_error(format("type '%s' is neither an array nor does it have a function __get__(%s)", operand->type->long_name(), index->type->long_name()), index);


	if (index->type != TypeInt) {
		Exp.rewind();
		do_error(format("array index needs to be of type 'int', not '%s'", index->type->long_name()), index);
	}

	shared<Node> array_element;

	// pointer?
	if (operand->type->usable_as_super_array()) {
		array_element = tree->add_node_dyn_array(operand, index);
	} else if (operand->type->is_pointer()) {
		array_element = tree->add_node_parray(operand, index, operand->type->param[0]->param[0]);
	} else {
		array_element = tree->add_node_array(operand, index);
	}
	array_element->is_const = operand->is_const;
	return array_element;

}

shared_array<Node> Parser::concretify_node_multi(shared<Node> node, Block *block, const Class *ns) {
	if (node->type != TypeUnknown)
		return {node};

	if (node->kind == NodeKind::ABSTRACT_TOKEN) {
		string token = Exp.get_token(node->token_id);

		// direct operand
		auto operands = tree->get_existence(token, block, ns);
		if (operands.num > 0) {
			// direct operand
			for (auto &o: weak(operands))
				o->token_id = node->token_id;
			return operands;
		} else {
			auto t = get_constant_type(token);
			if (t == TypeUnknown) {

				msg_write("----");
				for (auto vv: weak(block->function->var))
					msg_write(vv->type->name + " .... " + vv->name);
				for (auto p: block->function->literal_param_type)
					msg_write(p->name);
				//crash();
				do_error("unknown operand", node);
			}

			Value v;
			get_constant_value(token, v);

			if (t == TypeString) {
				return {try_parse_format_string(block, v, node->token_id)};
			} else {
				auto *c = tree->add_constant(t);
				c->set(v);
				return {tree->add_node_const(c, node->token_id)};
			}
		}
	} else if (node->kind == NodeKind::ABSTRACT_ELEMENT) {
		return concretify_element(node, block, ns);
	} else {
		return {concretify_node(node, block, ns)};
	}
	return {};
}


shared<Node> Parser::concretify_statement_return(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns, this);
	if (block->function->literal_return_type == TypeVoid) {
		if (node->params.num > 0)
			do_error("current function has type 'void', can not return a value", node);
	} else {
		if (node->params.num == 0)
			do_error("return value expected", node);
		node->params[0] = check_param_link(node->params[0], block->function->literal_return_type, IDENTIFIER_RETURN);
	}
	node->type = TypeVoid;
	return node;
}

shared<Node> Parser::concretify_statement_if(shared<Node> node, Block *block, const Class *ns) {
	// [COND, TRUE-BLOCK, FALSE-BLOCK]
	concretify_all_params(node, block, ns, this);
	node->type = TypeVoid;
	//node->set_param(0, check_param_link(node->params[0], TypeBool, IDENTIFIER_IF));
	node->params[0] = check_param_link(node->params[0], TypeBool, IDENTIFIER_IF);
	return node;
}

shared<Node> Parser::concretify_statement_while(shared<Node> node, Block *block, const Class *ns) {
	// [COND, BLOCK]
	concretify_all_params(node, block, ns, this);
	node->type = TypeVoid;
	node->params[0] = check_param_link(node->params[0], TypeBool, IDENTIFIER_WHILE);
	return node;
}

shared<Node> Parser::concretify_statement_for_range(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, VALUE0, VALUE1, STEP, BLOCK]

	auto var_name = Exp.get_token(node->params[0]->token_id);
	auto val0 = force_concrete_type(concretify_node(node->params[1], block, ns));
	auto val1 = force_concrete_type(concretify_node(node->params[2], block, ns));
	auto step = node->params[3];
	if (step)
		step = force_concrete_type(concretify_node(step, block, ns));

	// type?
	const Class *t = val0->type;
	if (val1->type == TypeFloat32)
		t = val1->type;
	if (step)
		if (step->type == TypeFloat32)
			t = step->type;

	if (!step) {
		if (t) {
			step = tree->add_node_const(tree->add_constant_int(1));
		} else {
			step = tree->add_node_const(tree->add_constant(TypeFloat32));
			step->as_const()->as_float() = 1.0f;
		}
	}

	val0 = check_param_link(val0, t, "for", 1, 2);
	val1 = check_param_link(val1, t, "for", 1, 2);
	if (step)
		step = check_param_link(step, t, "for", 1, 2);

	node->params[1] = val0;
	node->params[2] = val1;
	node->params[3] = step;

	// variable...
	auto *var = block->add_var(var_name, t);
	node->set_param(0, tree->add_node_local(var));

	// block
	node->params[4] = concretify_node(node->params[4], block, ns);
	post_process_for(node);

	node->type = TypeVoid;
	return node;
}

shared<Node> Parser::concretify_statement_for_array(shared<Node> node, Block *block, const Class *ns) {
	// [VAR, INDEX, ARRAY, BLOCK]

	auto array = force_concrete_type(concretify_node(node->params[2], block, ns));
	array = deref_if_pointer(array);
	if (!array->type->usable_as_super_array() and !array->type->is_array())
		do_error("array or list expected as second parameter in 'for . in .'", array);
	node->params[2] = array;


	// variable...
	auto var_name = Exp.get_token(node->params[0]->token_id);
	auto var_type = array->type->get_array_element();
	auto var = block->add_var(var_name, var_type);
	if (array->is_const)
		flags_set(var->flags, Flags::CONST);
	node->set_param(0, tree->add_node_local(var));

	string index_name = format("-for_index_%d-", for_index_count ++);
	if (node->params[1])
		index_name = Exp.get_token(node->params[1]->token_id);
	auto index = block->add_var(index_name, TypeInt);
	node->set_param(1, tree->add_node_local(index));

	// block
	node->params[3] = concretify_node(node->params[3], block, ns);
	post_process_for(node);

	node->type = TypeVoid;
	return node;
}

shared<Node> Parser::concretify_statement_str(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns, this);
	return add_converter_str(node->params[0], false);
}

shared<Node> Parser::concretify_statement_repr(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns, this);
	return add_converter_str(node->params[0], true);
}

shared<Node> Parser::concretify_statement_sizeof(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);

	if (sub->kind == NodeKind::CLASS) {
		return tree->add_node_const(tree->add_constant_int(sub->as_class()->size), node->token_id);
	} else {
		return tree->add_node_const(tree->add_constant_int(sub->type->size), node->token_id);
	}
}

shared<Node> Parser::concretify_statement_typeof(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);

	auto c = tree->add_node_const(tree->add_constant(TypeClassP), node->token_id);
	if (sub->kind == NodeKind::CLASS) {
		return tree->add_node_class(sub->as_class(), node->token_id);
	} else {
		return tree->add_node_class(sub->type, node->token_id);
	}
}

shared<Node> Parser::concretify_statement_len(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	sub = force_concrete_type(sub);
	sub = deref_if_pointer(sub);

	// array?
	if (sub->type->is_array())
		return tree->add_node_const(tree->add_constant_int(sub->type->array_length), node->token_id);

	// __length__() function?
	auto *f = sub->type->get_member_func(IDENTIFIER_FUNC_LENGTH, TypeInt, {});
	if (f)
		return tree->add_node_member_call(f, sub, node->token_id);

	// element "int num/length"?
	for (auto &e: sub->type->elements)
		if (e.type == TypeInt and (e.name == "length" or e.name == "num")) {
			return sub->shift(e.offset, e.type);
		}

	// length() function?
	for (auto f: sub->type->functions)
		if ((f->name == "length") and (f->num_params == 1))
			return tree->add_node_member_call(f.get(), sub, node->token_id);


	do_error(format("don't know how to get the length of an object of class '%s'", sub->type->long_name()), node);
	return nullptr;
}

shared<Node> Parser::concretify_statement_new(shared<Node> node, Block *block, const Class *ns) {
	auto constr = concretify_node(node->params[0], block, block->name_space());
	if (constr->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION)
		do_error("constructor call expected after 'new'", node->params[0]);
	constr->kind = NodeKind::CALL_FUNCTION;
	constr->type = TypeVoid;
	node->params[0] = constr;

	auto ff = constr->as_func();
	auto tt = ff->name_space;
	//do_error("NEW " + tt->long_name());

	node->type = tt->get_pointer();
	return node;
}

shared<Node> Parser::concretify_statement_delete(shared<Node> node, Block *block, const Class *ns) {
	auto p = concretify_node(node->params[0], block, block->name_space());
	if (!p->type->is_pointer())
		do_error("pointer expected after 'del'", node->params[0]);

	// override del operator?
	auto f = p->type->param[0]->get_member_func(IDENTIFIER_FUNC_DELETE_OVERRIDE, TypeVoid, {});
	if (f) {
		auto cmd = tree->add_node_call(f, node->token_id);
		cmd->set_instance(p->deref());
		return cmd;
	}

	// default delete
	node->params[0] = p;
	node->type = TypeVoid;
	return node;
}

shared<Node> Parser::concretify_statement_dyn(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	//sub = force_concrete_type(sub); // TODO
	return make_dynamical(sub);
}

shared<Node> Parser::concretify_statement_sorted(shared<Node> node, Block *block, const Class *ns) {
	concretify_all_params(node, block, ns, this);
	auto array = force_concrete_type(node->params[0]);
	auto crit = force_concrete_type(node->params[1]);

	if (!array->type->is_super_array())
		do_error("sorted(): first parameter must be a list[]", array);
	if (crit->type != TypeString)
		do_error("sorted(): second parameter must be a string", crit);

	Function *f = tree->required_func_global("@sorted", node->token_id);

	auto cmd = tree->add_node_call(f, node->token_id);
	cmd->set_param(0, array);
	cmd->set_param(1, tree->add_node_class(array->type));
	cmd->set_param(2, crit);
	cmd->type = array->type;
	return cmd;
}

shared<Node> Parser::concretify_statement_weak(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());

	auto t = sub->type;
	while (true) {
		if (t->is_pointer_shared() or t->is_pointer_owned()) {
			auto tt = t->param[0]->get_pointer();
			return sub->shift(0, tt);
		} else if (t->is_super_array() and t->get_array_element()->is_pointer_shared()) {
			auto tt = tree->make_class_super_array(t->param[0]->param[0]->get_pointer(), node->token_id);
			return sub->shift(0, tt);
		}
		if (t->parent)
			t = t->parent;
		else
			break;
	}
	do_error("weak() expects either a shared pointer, an owned pointer, or a shared pointer array", sub);
	return nullptr;
}

shared<Node> Parser::concretify_statement_map(shared<Node> node, Block *block, const Class *ns) {
	auto func = concretify_node(node->params[0], block, block->name_space());
	auto array = concretify_node(node->params[1], block, block->name_space());
	func = force_concrete_type(func);
	array = force_concrete_type(array);


	if (!func->type->is_callable())
		do_error("map(): first parameter must be callable", func);
	if (!array->type->is_super_array())
		do_error("map(): second parameter must be a list[]", array);

	auto p = node_call_effective_params(func);
	auto rt = node_call_return_type(func);
	if (p.num != 1)
		do_error("map(): function must have exactly one parameter", func);
	if (p[0] != array->type->param[0])
		do_error("map(): function parameter does not match list type", array);

	auto *f = tree->required_func_global("@xmap", node->token_id);

	auto cmd = tree->add_node_call(f, node->token_id);
	cmd->set_param(0, func);
	cmd->set_param(1, array);
	cmd->set_param(2, tree->add_node_class(p[0]));
	cmd->set_param(3, tree->add_node_class(p[1]));
	cmd->type = tree->make_class_super_array(rt, node->token_id);
	return cmd;
}

shared<Node> Parser::concretify_statement_raw_function_pointer(shared<Node> node, Block *block, const Class *ns) {
	auto sub = concretify_node(node->params[0], block, block->name_space());
	if (sub->kind != NodeKind::FUNCTION)
		do_error("raw_function_pointer() expects a function name", sub);
	auto func = tree->add_node_const(tree->add_constant(TypeFunctionP), node->token_id);
	func->as_const()->as_int64() = (int_p)sub->as_func();

	node->type = TypeFunctionCodeP;
	node->set_param(0, func);
	return node;
}

shared<Node> Parser::concretify_statement_try(shared<Node> node, Block *block, const Class *ns) {
	// [TRY-BLOCK, EX:[TYPE, NAME], EX-BLOCK, ...]

	auto try_block = concretify_node(node->params[0], block, block->name_space());
	node->params[0] = try_block;

	int num_exceptions = (node->params.num - 1) / 2;

	for (int i=0; i<num_exceptions; i++) {

		auto ex = node->params[1 + 2*i];

		auto ex_block = node->params[2 + 2*i];

		if (ex->params.num > 0) {
			auto ex_type = ex->params[0];
			ex_type = concretify_node(ex_type, block, block->name_space());
			ex_type = digest_type(tree, ex_type);
			auto var_name = Exp.get_token(ex->params[1]->token_id);

			ex->params.resize(1);


			if (ex_type->kind != NodeKind::CLASS)
				do_error("Exception class expected", ex_type);
			auto type = ex_type->as_class();
			if (!type->is_derived_from(TypeException))
				do_error("Exception class expected", ex_type);
			ex->type = type;

			auto *v = ex_block->as_block()->add_var(var_name, type->get_pointer());
			ex->set_param(0, tree->add_node_local(v));
		} else {
			ex->type = TypeVoid;
		}

		// find types AFTER creating the variable
		ex_block = concretify_node(ex_block, block, block->name_space());
		node->params[2 + 2*i] = ex_block;
	}
	node->type = TypeVoid;
	return node;
}

// inner_callable: (A,B,C,D,E)->R
// captures:       [-,x0,-,-,x1]
shared<Node> create_bind(Parser *p, shared<Node> inner_callable, const shared_array<Node> &captures, const Array<bool> &capture_via_ref) {
	SyntaxTree *tree = p->tree;
	int token_id = inner_callable->token_id;

	Array<const Class*> capture_types;
	for (auto c: weak(captures))
		if (c)
			capture_types.add(c->type);
		else
			capture_types.add(nullptr);

	auto param_types = get_callable_param_types(inner_callable->type);
	auto return_type = get_callable_return_type(inner_callable->type);

	Array<const Class*> outer_call_types;
	for (int i=0; i<param_types.num; i++)
		if (!captures[i])
			outer_call_types.add(param_types[i]);

	auto bind_wrapper_type = tree->make_class_callable_bind(param_types, return_type, capture_types, capture_via_ref, token_id);

	// "new bind(f, x0, x1, ...)"
	for (auto *cf: bind_wrapper_type->get_constructors()) {
		auto cmd_new = tree->add_node_statement(StatementID::NEW);
		auto con = tree->add_node_constructor(cf);
		shared_array<Node> params = {inner_callable.get()};
		for (auto c: weak(captures))
			if (c)
				params.add(c);
		con = p->apply_params_direct(con, params, 1);
		con->kind = NodeKind::CALL_FUNCTION;
		con->type = TypeVoid;

		cmd_new->type = tree->make_class_callable_fp(outer_call_types, return_type, token_id);
		cmd_new->set_param(0, con);
		cmd_new->token_id = inner_callable->token_id;
		return cmd_new;
	}

	p->do_error("bind failed...", inner_callable);
	return nullptr;
}

shared<Node> Parser::concretify_statement_lambda(shared<Node> node, Block *block, const Class *ns) {
	auto f = node->params[0]->as_func();


	auto *prev_func = cur_func;

	f->block->parent = block; // to allow captured variable lookup
	if (block->function->is_member())
		flags_clear(f->flags, Flags::STATIC); // allow finding "self.x" via "x"

	cur_func = f;

	if (f->block->params.num == 1) {
		// func(i)              (multi line)
		//     bla..
		//     return i*i       (explicit return)

		auto cmd = f->block->params[0];
		cmd = concretify_node(cmd, f->block.get(), block->name_space());

		f->literal_return_type = cmd->type;
		f->effective_return_type = cmd->type;

		if (cmd->type == TypeVoid) {
			f->block->params[0] = cmd;
		} else {
			auto ret = tree->add_node_statement(StatementID::RETURN);
			ret->set_num_params(1);
			ret->params[0] = cmd;
			f->block->params[0] = ret;
		}

	} else {
		// func(i) i*i      (single line, direct return)
		f->block->type = TypeUnknown;
		f->literal_return_type = TypeVoid;
		f->effective_return_type = TypeVoid;
		concretify_node(f->block.get(), f->block.get(), f->name_space);
	}

	cur_func = prev_func;


	f->block->parent = nullptr;
	flags_set(f->flags, Flags::STATIC);

	tree->base_class->add_function(tree, f, false, false);

	// find captures
	Set<Variable*> captures;
	auto find_captures = [block, &captures](shared<Node> n) {
		if (n->kind == NodeKind::VAR_LOCAL) {
			auto v = n->as_local();
			for (auto vv: block->function->var)
				if (v == vv)
					captures.add(v);
		}
		return n;
	};
	tree->transform_block(f->block.get(), find_captures);


	// no captures?
	if (captures.num == 0) {
		f->update_parameters_after_parsing();
		return tree->add_node_func_name(f);
	}

	auto explicit_param_types = f->literal_param_type;


	if (config.verbose)
		msg_write("CAPTURES:");

	Array<bool> capture_via_ref;

	auto should_capture_via_ref = [this, node] (Variable *v) {
		if (v->name == IDENTIFIER_SELF)
			return true;
		if (v->type->can_memcpy() or v->type == TypeString)
			return false;
		do_error(format("currently not supported to capture variable '%s' of type '%s'", v->name, v->type->long_name()), node);
		return true;
	};

	// replace captured variables by adding more parameters to f
	for (auto v: captures) {
		if (config.verbose)
			msg_write("  * " + v->name);

		bool via_ref = should_capture_via_ref(v);
		capture_via_ref.add(via_ref);
		auto cap_type = via_ref ? v->type->get_pointer() : v->type;


		auto vvv = f->add_param(v->name, cap_type, Flags::NONE);
		//if (!flags_has(flags, Flags::OUT))
		//flags_set(v->flags, Flags::CONST);



		auto replace_local = [v,vvv,cap_type,via_ref](shared<Node> n) {
			if (n->kind == NodeKind::VAR_LOCAL)
				if (n->as_local() == v) {
					if (via_ref) {
						n->link_no = (int_p)vvv;
						n->type = cap_type;
						return n->deref();
					} else {
						n->link_no = (int_p)vvv;
					}
				}
			return n;
		};
		tree->transform_block(f->block.get(), replace_local);
	}

	f->update_parameters_after_parsing();

	auto create_inner_lambda = wrap_function_into_callable(f, node->token_id);

	shared_array<Node> capture_nodes;
	foreachi (auto &c, captures, i) {
		if (capture_via_ref[i])
			capture_nodes.add(tree->add_node_local(c)->ref());
		else
			capture_nodes.add(tree->add_node_local(c));
	}
	for (auto e: explicit_param_types) {
		capture_nodes.insert(nullptr, 0);
		capture_via_ref.insert(false, 0);
	}

	return create_bind(this, create_inner_lambda, capture_nodes, capture_via_ref);
}

shared<Node> Parser::concretify_statement(shared<Node> node, Block *block, const Class *ns) {
	auto s = node->as_statement();
	if (s->id == StatementID::RETURN) {
		return concretify_statement_return(node, block, ns);
	} else if ((s->id == StatementID::IF) or (s->id == StatementID::IF_ELSE)) {
		return concretify_statement_if(node, block, ns);
	} else if (s->id == StatementID::WHILE) {
		return concretify_statement_while(node, block, ns);
	} else if (s->id == StatementID::FOR_RANGE) {
		return concretify_statement_for_range(node, block, ns);
	} else if (s->id == StatementID::FOR_ARRAY) {
		return concretify_statement_for_array(node, block, ns);
	} else if (s->id == StatementID::STR) {
		return concretify_statement_str(node, block, ns);
	} else if (s->id == StatementID::REPR) {
		return concretify_statement_repr(node, block, ns);
	} else if (s->id == StatementID::SIZEOF) {
		return concretify_statement_sizeof(node, block, ns);
	} else if (s->id == StatementID::TYPEOF) {
		return concretify_statement_typeof(node, block, ns);
	} else if (s->id == StatementID::LEN) {
		return concretify_statement_len(node, block, ns);
	} else if (s->id == StatementID::NEW) {
		return concretify_statement_new(node, block, ns);
	} else if (s->id == StatementID::DELETE) {
		return concretify_statement_delete(node, block, ns);
	} else if (s->id == StatementID::DYN) {
		return concretify_statement_dyn(node, block, ns);
	} else if (s->id == StatementID::RAW_FUNCTION_POINTER) {
		return concretify_statement_raw_function_pointer(node, block, ns);
	} else if (s->id == StatementID::WEAK) {
		return concretify_statement_weak(node, block, ns);
	} else if (s->id == StatementID::SORTED) {
		return concretify_statement_sorted(node, block, ns);
	} else if (s->id == StatementID::MAP) {
		return concretify_statement_map(node, block, ns);
	} else if (s->id == StatementID::TRY) {
		return concretify_statement_try(node, block, ns);
	} else if (s->id == StatementID::LAMBDA) {
		return concretify_statement_lambda(node, block, ns);
	} else {
		node->show();
		do_error("INTERNAL: unexpected statement", node);
	}
	return nullptr;
}

shared<Node> Parser::concretify_node(shared<Node> node, Block *block, const Class *ns) {
	if (node->type != TypeUnknown)
		return node;

	if (node->kind == NodeKind::ABSTRACT_OPERATOR) {
		auto op_no = node->as_abstract_op();

		if (op_no->id == OperatorID::FUNCTION_PIPE) {
			concretify_all_params(node, block, ns, this);
			// well... we're abusing that we will always get the FIRST 2 pipe elements!!!
			return build_function_pipe(node->params[0], node->params[1]);
		} else if (op_no->id == OperatorID::MAPS_TO) {
			return build_lambda_new(node->params[0], node->params[1]);
		}
		concretify_all_params(node, block, ns, this);

		if (node->params.num == 2) {
			// binary operator A+B
			auto param1 = node->params[0];
			auto param2 = force_concrete_type_if_function(node->params[1]);
			auto op = link_operator(op_no, param1, param2);
			if (!op)
				do_error(format("no operator found: '%s %s %s'", param1->type->long_name(), op_no->name, give_useful_type(this, param2)->long_name()), node);
			return op;
		} else {
			return link_unary_operator(op_no, node->params[0], block, node->token_id);
		}
	} else if (node->kind == NodeKind::DEREFERENCE) {
		concretify_all_params(node, block, ns, this);
		auto sub = node->params[0];
		if (!sub->type->is_pointer())
			do_error("only pointers can be dereferenced using '*'", node);
		node->type = sub->type->param[0];
	} else if (node->kind == NodeKind::REFERENCE) {
		concretify_all_params(node, block, ns, this);
		auto sub = node->params[0];
		node->type = sub->type->get_pointer();
	} else if (node->kind == NodeKind::ABSTRACT_CALL) {
		return concretify_call(node, block, ns);
	} else if (node->kind == NodeKind::ARRAY) {
		return concretify_array(node, block, ns);
	} else if (node->kind == NodeKind::TUPLE) {
		concretify_all_params(node, block, ns, this);
		return node;
	} else if (node->kind == NodeKind::ARRAY_BUILDER) {
		concretify_all_params(node, block, ns, this);
		return node;
	} else if (node->kind == NodeKind::DICT_BUILDER) {
		concretify_all_params(node, block, ns, this);
		for (int p=0; p<node->params.num; p+=2)
			if (node->params[p]->type != TypeString or node->params[p]->kind != NodeKind::CONSTANT)
				do_error("key needs to be a constant string", node->params[p]);
		return node;
	} else if (node->kind == NodeKind::FUNCTION) {
		return node;
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_POINTER) {
		concretify_all_params(node, block, ns, this);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected before '*'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		return tree->add_node_class(t->get_pointer());
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_LIST) {
		concretify_all_params(node, block, ns, this);
		auto n = digest_type(tree, node->params[0]);
		if (n->kind != NodeKind::CLASS)
			do_error("type expected before '[]'", n);
		const Class *t = n->as_class();
		t = tree->make_class_super_array(t, node->token_id);
		return tree->add_node_class(t);
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_DICT) {
		concretify_all_params(node, block, ns, this);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected before '{}'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		t = tree->make_class_dict(t, node->token_id);
		return tree->add_node_class(t);
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_CALLABLE) {
		concretify_all_params(node, block, ns, this);
		node->params[0] = digest_type(tree, node->params[0]);
		node->params[1] = digest_type(tree, node->params[1]);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected before '->'", node->params[0]);
		if (node->params[1]->kind != NodeKind::CLASS)
			do_error("type expected before '->'", node->params[1]);
		const Class *t0 = node->params[0]->as_class();
		const Class *t1 = node->params[1]->as_class();
		return tree->add_node_class(tree->make_class_callable_fp({t0}, t1, node->token_id));
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_SHARED) {
		concretify_all_params(node, block, ns, this);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected after 'shared'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		t = make_pointer_shared(tree, t, node->token_id);
		return tree->add_node_class(t);
	} else if (node->kind == NodeKind::ABSTRACT_TYPE_OWNED) {
		concretify_all_params(node, block, ns, this);
		if (node->params[0]->kind != NodeKind::CLASS)
			do_error("type expected after 'owned'", node->params[0]);
		const Class *t = node->params[0]->as_class();
		t = make_pointer_owned(tree, t, node->token_id);
		return tree->add_node_class(t);

	} else if ((node->kind == NodeKind::ABSTRACT_TOKEN) or (node->kind == NodeKind::ABSTRACT_ELEMENT)) {
		auto operands = concretify_node_multi(node, block, ns);
		if (operands.num > 1)
			msg_write(format("WARNING: node not unique:  %s  -  line %d", Exp.get_token(node->token_id), Exp.token_physical_line_no(node->token_id) + 1));
		if (operands.num > 0)
			return operands[0];
	} else if (node->kind == NodeKind::STATEMENT) {
		return concretify_statement(node, block, ns);
	} else if (node->kind == NodeKind::BLOCK) {
		for (int i=0; i<node->params.num; i++)
			node->params[i] = concretify_node(node->params[i], node->as_block(), ns);
		//concretify_all_params(node, node->as_block(), ns, this);
		node->type = TypeVoid;
		for (int i=node->params.num-1; i>=0; i--)
			if (node->params[i]->kind == NodeKind::ABSTRACT_VAR)
				node->params.erase(i);
	} else if (node->kind == NodeKind::ABSTRACT_VAR) {
		const Class *type = nullptr;
		if (node->params[0]) {
			auto t = digest_type(tree, force_concrete_type(concretify_node(node->params[0], block, ns)));
			if (t->kind != NodeKind::CLASS)
				do_error("variable declaration requires a type", t);
			type = t->as_class();
		} else {
			//assert(node->params[2]);
			auto rhs = force_concrete_type(concretify_node(node->params[2]->params[1], block, ns));
			node->params[2]->params[1] = rhs;
			type = rhs->type;
		}
		if (type->needs_constructor() and !type->get_default_constructor())
			do_error(format("declaring a variable of type '%s' requires a constructor but no default constructor exists", type->long_name()), node);
		block->add_var(Exp.get_token(node->params[1]->token_id), type);

		if (node->params.num == 3)
			return concretify_node(node->params[2], block, ns);

	} else if (node->kind == NodeKind::ARRAY_BUILDER_FOR) {
		// IN:  [FOR, EXP, IF]
		// OUT: [FOR, VAR]

		auto n_for = node->params[0];
		auto n_exp = node->params[1];
		auto n_cmp = node->params[2];

		// first pass: find types in the for loop
		auto fake_for = tree->cp_node(n_for); //->shallow_copy();
		fake_for->set_param(fake_for->params.num - 1, tree->cp_node(n_exp)); //->shallow_copy());
		fake_for = concretify_node(fake_for, block, ns);
		// TODO: remove new variables!

		// create an array
		auto type_el = fake_for->params.back()->type;
		auto type_array = tree->make_class_super_array(type_el, node->token_id);
		auto *var = block->add_var(block->function->create_slightly_hidden_name(), type_array);



		// array.add(exp)
		auto *f_add = type_array->get_member_func("add", TypeVoid, {type_el});
		if (!f_add)
			do_error("...add() ???", node);
		auto n_add = tree->add_node_member_call(f_add, tree->add_node_local(var));
		n_add->set_param(1, n_exp);
		n_add->type = TypeUnknown; // mark abstract so n_exp will be concretified

		// add new code to the loop
		Block *b;
		if (n_cmp) {
			auto b_if = new Block(block->function, block, TypeUnknown);
			auto b_add = new Block(block->function, b_if, TypeUnknown);
			b_add->add(n_add);

			auto n_if = tree->add_node_statement(StatementID::IF, node->token_id, TypeUnknown);
			n_if->set_param(0, n_cmp);
			n_if->set_param(1, b_add);

			b_if->add(n_if);
			b = b_if;
		} else {
			b = new Block(block->function, block, TypeUnknown);
			b->add(n_add);
		}

		n_for->set_param(n_for->params.num - 1, b);

		// NOW we can set the types
		n_for = concretify_node(n_for, block, ns);

		auto n = new Node(NodeKind::ARRAY_BUILDER_FOR, 0, type_array);
		n->set_num_params(2);
		n->set_param(0, n_for);
		n->set_param(1, tree->add_node_local(var));
		return n;

	} else if (node->kind == NodeKind::NONE) {
	} else if (node->kind == NodeKind::CALL_FUNCTION) {
		concretify_all_params(node, block, ns, this);
		node->type = node->as_func()->literal_return_type;
	} else {
		node->show();
		do_error("INTERNAL ERROR: unexpected node", node);
	}

	return node;
}


const Class *Parser::concretify_as_type(shared<Node> node, Block *block, const Class *ns) {
	auto cc = concretify_node(node, block, ns);
	cc = digest_type(tree, cc);
	if (cc->kind != NodeKind::CLASS) {
		cc->show(TypeVoid);
		do_error("type expected", cc);
	}
	return cc->as_class();
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
	return concretify_node(tree, block, block->name_space());
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
	for (int i=0;true;i++) {
		if (!allow_tuples and Exp.cur == ",")
			break;
		auto op = parse_abstract_operator(3);
		if (!op)
			break;
		op->token_id = Exp.cur_token() - 1;
		operators.add(op);
		if (Exp.end_of_line())
			do_error_exp("unexpected end of line after operator");
		operands.add(parse_abstract_operand(block));
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
	int token0 = Exp.cur_token();
	Exp.next(); // for
	auto var = create_node_token(this);
	Exp.next();

	// index
	shared<Node> index;
	if (Exp.cur == ",") {
		Exp.next();
		index = create_node_token(this);
		Exp.next();
	}


	if (Exp.cur != "in")
		do_error_exp("'in' expected after variable in 'for ...'");
	Exp.next();

	// first value/array
	auto val0 = parse_abstract_operand_greedy(block);


	if (Exp.cur == ":") {
		// range

		Exp.next(); // :
		auto val1 = parse_abstract_operand_greedy(block);

		shared<Node> val_step;
		if (Exp.cur == ":") {
			Exp.next();
			val_step = parse_abstract_operand_greedy(block);
		}

		auto cmd_for = tree->add_node_statement(StatementID::FOR_RANGE, token0, TypeUnknown);
		cmd_for->set_param(0, var);
		cmd_for->set_param(1, val0);
		cmd_for->set_param(2, val1);
		cmd_for->set_param(3, val_step);
		//cmd_for->set_uparam(4, loop_block);

		return cmd_for;

	} else {
		// array

		auto array = val0;


		auto cmd_for = tree->add_node_statement(StatementID::FOR_ARRAY, token0, TypeUnknown);
		// [VAR, INDEX, ARRAY, BLOCK]

		cmd_for->set_param(0, var);
		cmd_for->set_param(1, index);
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
		var->type = var->type->get_pointer();
		n_var->type = var->type;
		tree->transform_node(loop_block, [&](shared<Node> n) { return tree->conv_cbr(n, var); });
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
	int token0 = Exp.cur_token();
	Exp.next(); // while
	auto cmd_cmp = parse_abstract_operand_greedy(block);

	auto cmd_while = tree->add_node_statement(StatementID::WHILE, token0, TypeUnknown);
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
	Exp.next();
	return tree->add_node_statement(StatementID::BREAK, Exp.cur_token() - 1);
}

shared<Node> Parser::parse_abstract_statement_continue() {
	if (parser_loop_depth == 0)
		do_error_exp("'continue' only allowed inside a loop");
	Exp.next();
	return tree->add_node_statement(StatementID::CONTINUE, Exp.cur_token() - 1);
}

// Node structure
//  p[0]: value (if not void)
shared<Node> Parser::parse_abstract_statement_return(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next();
	auto cmd = tree->add_node_statement(StatementID::RETURN, token0, TypeUnknown);
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
	int token0 = Exp.cur_token();
	Exp.next();
	auto cmd_try = tree->add_node_statement(StatementID::TRY, token0, TypeUnknown);
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
		int token1 = Exp.cur_token();


	//	if (Exp.cur != IDENTIFIER_EXCEPT)
	//		do_error("except after try expected");
	//	if (Exp.cur_line->indent != ind)
	//		do_error("wrong indentation for except");
		Exp.next(); // except

		auto cmd_ex = tree->add_node_statement(StatementID::EXCEPT, token1, TypeUnknown);

		auto except_block = new Block(block->function, block, TypeUnknown);

		if (!Exp.end_of_line()) {
			auto ex_type = parse_abstract_operand(block, true); // type
			if (!ex_type)
				do_error_exp("Exception class expected");
			cmd_ex->params.add(ex_type);
			if (!Exp.end_of_line()) {
				if (Exp.cur != IDENTIFIER_AS)
					do_error_exp("'as' expected");
				Exp.next(); // 'as'
				cmd_ex->params.add(create_node_token(this)); // var name
				Exp.next();
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
	int token0 = Exp.cur_token();
	Exp.next();
	auto cmd_cmp = parse_abstract_operand_greedy(block);

	auto cmd_if = tree->add_node_statement(StatementID::IF, token0, TypeUnknown);
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
	Exp.next(); // pass
	expect_new_line();

	return tree->add_node_statement(StatementID::PASS);
}

// Node structure
//  type: class
//  p[0]: call to constructor (optional)
shared<Node> Parser::parse_abstract_statement_new(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // new
	auto cmd = tree->add_node_statement(StatementID::NEW, token0, TypeUnknown);
	cmd->set_param(0, parse_abstract_operand(block));
	return cmd;
}

// Node structure
//  p[0]: operand
shared<Node> Parser::parse_abstract_statement_delete(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // del
	auto cmd = tree->add_node_statement(StatementID::DELETE, token0, TypeUnknown);
	cmd->set_param(0, parse_abstract_operand(block));
	return cmd;
}

shared<Node> Parser::parse_abstract_single_func_param(Block *block) {
	string func_name = Exp.get_token(Exp.cur_token() - 1);
	if (Exp.cur != "(")
		do_error_exp(format("'(' expected after '%s'", func_name));
	Exp.next(); // "("
	auto n = parse_abstract_operand_greedy(block);
	if (Exp.cur != ")")
		do_error_exp(format("')' expected after parameter of '%s'", func_name));
	Exp.next(); // ")"
	return n;
}

shared<Node> Parser::parse_abstract_statement_sizeof(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // sizeof
	auto node = tree->add_node_statement(StatementID::SIZEOF, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;

}

shared<Node> Parser::parse_abstract_statement_type(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // typeof
	auto node = tree->add_node_statement(StatementID::TYPEOF, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_len(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // len
	auto node = tree->add_node_statement(StatementID::LEN, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

const Class *type_more_abstract(const Class *a, const Class *b) {
	if (a == b)
		return a;
	if (a == TypeInt and b == TypeFloat32)
		return TypeFloat32;
	if (a == TypeFloat32 and b == TypeInt)
		return TypeFloat32;
	return nullptr;
}

shared<Node> Parser::wrap_node_into_callable(shared<Node> node) {
	if (node->kind != NodeKind::FUNCTION)
		return node;
	auto f = node->as_func();
	auto callable = wrap_function_into_callable(f, node->token_id);
	if (f->is_member() and node->params.num > 0 and node->params[0]) {
		//if (f->literal_param_type.num > 1)
		//	do_error("wrapping member functions with parameters into callables currently not implemented...", node);
		shared_array<Node> captures = {node->params[0]};
		Array<bool> capture_via_ref = {true};
		for (int i=1; i<f->literal_param_type.num; i++) {
			captures.add(nullptr);
			capture_via_ref.add(false);
		}
		auto b = create_bind(this, callable, captures, capture_via_ref);
		return b;
	}
	return callable;
}

// f : (A,B,...)->R  =>  new Callable[](f) : (A,B,...)->R
shared<Node> Parser::wrap_function_into_callable(Function *f, int token_id) {
	auto t = tree->make_class_callable_fp(f, token_id);

	for (auto *cf: t->param[0]->get_constructors()) {
		if (cf->num_params == 2) {
			auto cmd = tree->add_node_statement(StatementID::NEW, token_id);
			auto con = tree->add_node_constructor(cf);
			auto fp = tree->add_constant(TypeFunctionP);
			fp->as_int64() = (int_p)f;
			con = apply_params_direct(con, {tree->add_node_const(fp, token_id)}, 1);
			con->kind = NodeKind::CALL_FUNCTION;
			con->type = TypeVoid;

			cmd->type = t;
			cmd->set_param(0, con);
			return cmd;
		}
	}
	do_error("wrap_function_into_callable() failed? " + f->signature(), token_id);
	return nullptr;
}

void Parser::force_concrete_types(shared_array<Node> &nodes) {
	for (int i=0; i<nodes.num; i++)
		nodes[i] = force_concrete_type(nodes[i].get());
}

shared<Node> Parser::force_concrete_type_if_function(shared<Node> node) {
	if (node->kind == NodeKind::FUNCTION)
		return wrap_node_into_callable(node);
	return node;
}

shared<Node> Parser::force_concrete_type(shared<Node> node) {
	if (node->type != TypeUnknown)
		return node;

	if (node->kind == NodeKind::ARRAY_BUILDER) {
		if (node->params.num == 0) {
			node->type = TypeIntList;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[0]->type;
		for (int i=1; i<node->params.num; i++)
			t = type_more_abstract(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract array", node);

		for (int i=0; i<node->params.num; i++) {
			CastingData cast;
			type_match_with_cast(node->params[i].get(), false, t, cast);
			node->params[i] = apply_type_cast(cast, node->params[i].get(), t);
		}

		node->type = tree->make_class_super_array(t, node->token_id);
		return node;
	} else if (node->kind == NodeKind::DICT_BUILDER) {
		if (node->params.num == 0) {
			node->type = TypeIntDict;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[1]->type;
		for (int i=3; i<node->params.num; i+=2)
			t = type_more_abstract(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract dict", node);

		for (int i=1; i<node->params.num; i+=2) {
			CastingData cast;
			type_match_with_cast(node->params[i].get(), false, t, cast);
			node->params[i] = apply_type_cast(cast, node->params[i].get(), t);
		}

		node->type = tree->make_class_dict(t, node->token_id);
		return node;
	} else if (node->kind == NodeKind::TUPLE) {
		auto type = merge_type_tuple_into_product(tree, node_extract_param_types(node), node->token_id);
		auto xx = turn_class_into_constructor(type, node->params, node->token_id);
		return try_to_match_apply_params(xx, node->params);
	} else if (node->kind == NodeKind::FUNCTION) {
		return wrap_node_into_callable(node);
	} else {
		do_error("unhandled abstract type: " + kind2str(node->kind), node);
	}
	return node;
}

shared<Node> Parser::deref_if_pointer(shared<Node> node) {
	if (node->type->is_some_pointer())
		return node->deref();
	return node;
}


shared<Node> Parser::add_converter_str(shared<Node> sub, bool repr) {
	sub = force_concrete_type(sub);
	// evil shortcut for pointers (carefull with nil!!)
	if (!repr)
		sub = deref_if_pointer(sub);
	
	auto *t = sub->type;

	Function *cf = nullptr;
	if (repr)
		cf = t->get_member_func(IDENTIFIER_FUNC_REPR, TypeString, {});
	if (!cf)
		cf = t->get_member_func(IDENTIFIER_FUNC_STR, TypeString, {});
	if (cf)
		return tree->add_node_member_call(cf, sub, sub->token_id);

	// "universal" var2str() or var_repr()
	auto *c = tree->add_constant_pointer(TypeClassP, t);

	Function *f = tree->required_func_global(repr ? "@var_repr" : "@var2str", sub->token_id);

	auto cmd = tree->add_node_call(f, sub->token_id);
	cmd->set_param(0, sub->ref());
	cmd->set_param(1, tree->add_node_const(c));
	return cmd;
}

shared<Node> Parser::parse_abstract_statement_str(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // str
	auto node = tree->add_node_statement(StatementID::STR, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_repr(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // repr
	auto node = tree->add_node_statement(StatementID::REPR, token0, TypeUnknown);
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

	names.add(p->Exp.cur);
	p->Exp.next();

	while (p->Exp.cur == ",") {
		p->Exp.next(); // ","
		names.add(p->Exp.cur);
		p->Exp.next();
	}
	return names;
}

shared_array<Node> parse_comma_sep_token_list(Parser *p) {
	shared_array<Node> names;

	names.add(create_node_token(p));
	p->Exp.next();

	while (p->Exp.cur == ",") {
		p->Exp.next(); // ","
		names.add(create_node_token(p));
		p->Exp.next();
	}
	return names;
}

// local (variable) definitions...
shared<Node> Parser::parse_abstract_statement_var(Block *block) {
	Exp.next(); // "var"

	auto names = parse_comma_sep_token_list(this);
	shared<Node> type;

	// explicit type?
	if (Exp.cur == ":") {
		Exp.next();
		type = parse_abstract_operand(block, true);
	} else if (Exp.cur != "=") {
		do_error_exp("':' or '=' expected after 'var' declaration");
	}

	if (Exp.cur == "=") {
		if (names.num != 1)
			do_error_exp(format("'var' declaration with '=' only allowed with a single variable name, %d given", names.num));

		auto assign = parse_abstract_operator(3);

		auto rhs = parse_abstract_operand_greedy(block, true);

		assign->set_num_params(2);
		assign->set_param(0, names[0]);
		assign->set_param(1, rhs);

		auto node = new Node(NodeKind::ABSTRACT_VAR, 0, TypeUnknown);
		node->set_num_params(3);
		node->set_param(0, type); // type
		node->set_param(1, names[0]->shallow_copy()); // name
		node->set_param(2, assign);
		expect_new_line();
		return node;



		/*if (type) {
			rhs = force_concrete_type_if_function(rhs);
		} else {
			rhs = force_concrete_type(rhs);
			type = rhs->type;
		}
		auto *var = block->add_var(names[0], type);
		auto cmd = link_operator_id(OperatorID::ASSIGN, tree->add_node_local(var), rhs);
		if (!cmd)
			do_error(format("var: no operator '%s' = '%s'", type->long_name(), rhs->type->long_name()));
		return cmd;*/
	}

	expect_new_line();

	for (auto &n: names) {
		auto node = new Node(NodeKind::ABSTRACT_VAR, 0, TypeUnknown);
		node->set_num_params(2);
		node->set_param(0, type); // type
		node->set_param(1, n); // name
		block->add(node);
	}
	return tree->add_node_statement(StatementID::PASS);
}

Array<const Class*> func_effective_params(const Function *f) {
	return f->literal_param_type;
}

Array<const Class*> node_call_effective_params(shared<Node> node) {
	if (node->type->is_callable())
		return get_callable_param_types(node->type);
	if (node->kind == NodeKind::FUNCTION)
		return func_effective_params(node->as_func());
	return get_callable_param_types(node->type);
}

const Class *node_call_return_type(shared<Node> node) {
	if (node->type->is_callable())
		return get_callable_return_type(node->type);
	if (node->kind == NodeKind::FUNCTION)
		return node->as_func()->literal_return_type;
	return get_callable_return_type(node->type);
}

shared<Node> Parser::parse_abstract_statement_map(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // "map"

	auto params = parse_abstract_call_parameters(block);
	if (params.num != 2)
		do_error_exp("map() expects 2 parameters");

	auto node = tree->add_node_statement(StatementID::MAP, token0, TypeUnknown);
	node->set_param(0, params[0]);
	node->set_param(1, params[1]);
	return node;
}


shared<Node> Parser::parse_abstract_statement_lambda(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // "lambda"

	static int unique_lambda_counter = 0;

	auto *f = tree->add_function("<lambda-" + i2s(unique_lambda_counter ++) + ">", TypeUnknown, tree->base_class, Flags::STATIC);
	f->token_id = Exp.cur_token();

	Exp.next(); // '('

	// parameter list
	if (Exp.cur != ")")
		while (true) {
			// like variable definitions

			Flags flags = parse_flags();

			string param_name = Exp.cur;
			Exp.next();
			if (Exp.cur != ":")
				do_error_exp("':' after parameter name expected");
			Exp.next();

			// type of parameter variable
			auto param_type = parse_type(tree->base_class); // force
			auto v = f->add_param(param_name, param_type, flags);

			if (Exp.cur == ")")
				break;

			if (Exp.cur != ",")
				do_error_exp("',' or ')' expected after parameter");
			Exp.next(); // ','
		}
	Exp.next(); // ')'

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

	auto node = tree->add_node_statement(StatementID::LAMBDA, token0, TypeUnknown);
	node->set_num_params(1);
	node->set_param(0, tree->add_node_func_name(f));
	return node;
}

shared<Node> Parser::parse_abstract_statement_sorted(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // "sorted"
	auto params = parse_abstract_call_parameters(block);
	if (params.num != 2)
		do_error_exp("sorted(array, criterion) expects 2 parameters");
	auto node = tree->add_node_statement(StatementID::SORTED, token0, TypeUnknown);
	node->set_param(0, params[0]);
	node->set_param(1, params[1]);
	return node;
}

shared<Node> Parser::make_dynamical(shared<Node> node) {
	if (node->kind == NodeKind::ARRAY_BUILDER and node->type == TypeUnknown) {
		for (int i=0; i<node->params.num; i++)
			node->params[i] = make_dynamical(node->params[i].get());
		// TODO create...
		node->type = TypeAnyList;
		//return node;
	} else  if (node->kind == NodeKind::DICT_BUILDER and node->type == TypeUnknown) {
		for (int i=1; i<node->params.num; i+=2)
			node->params[i] = make_dynamical(node->params[i].get());
		// TODO create...
		node->type = TypeAnyDict;
		//return node;
	}
	//node = force_concrete_type(tree, node);

	auto *c = tree->add_constant_pointer(TypeClassP, node->type);

	Function *f = tree->required_func_global("@dyn", node->token_id);

	auto cmd = tree->add_node_call(f, node->token_id);
	cmd->set_param(0, node->ref());
	cmd->set_param(1, tree->add_node_const(c));
	return cmd;
}

shared<Node> Parser::parse_abstract_statement_dyn(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // dyn
	auto node = tree->add_node_statement(StatementID::DYN, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_raw_function_pointer(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // "raw_function_pointer"
	auto node = tree->add_node_statement(StatementID::RAW_FUNCTION_POINTER, token0, TypeUnknown);
	node->set_param(0, parse_abstract_single_func_param(block));
	return node;
}

shared<Node> Parser::parse_abstract_statement_weak(Block *block) {
	int token0 = Exp.cur_token();
	Exp.next(); // "weak"
	auto node = tree->add_node_statement(StatementID::WEAK, token0, TypeUnknown);
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
	} else if (Exp.cur == IDENTIFIER_LET) {
		return parse_abstract_statement_let(block);
	} else if (Exp.cur == IDENTIFIER_VAR) {
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
		node->set_param(1, create_node_token(this)); // name
		block->add(node);
		Exp.next();

		// assignment?
		if (Exp.cur == "=") {
			Exp.rewind();
			// parse assignment
			node->set_num_params(3);
			node->set_param(2, parse_abstract_operand_greedy(block, true));
		}
		if (Exp.end_of_line())
			break;
		if ((Exp.cur != ",") and !Exp.end_of_line())
			do_error_exp("',', '=' or newline expected after declaration of local variable");
		Exp.next();
	}
}

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void Parser::parse_abstract_complete_command(Block *block) {
	// beginning of a line!

	//bool is_type = tree->find_root_type_by_name(Exp.cur, block->name_space(), true);

	// assembler block
	if (Exp.cur == "-asm-") {
		Exp.next();
		block->add(tree->add_node_statement(StatementID::ASM));

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

extern Array<shared<Module>> loading_module_stack;

string canonical_import_name(const string &s) {
	return s.lower().replace(" ", "").replace("_", "");
}

string dir_has(const Path &dir, const string &name) {
	auto list = dir_search(dir, "*", "fd");
	for (auto &e: list)
		if (canonical_import_name(e.str()) == name)
			return e.str();
	return "";
}

Path import_dir_match(const Path &dir0, const string &name) {
	auto xx = name.explode("/");
	Path filename = dir0;

	for (int i=0; i<xx.num; i++) {
		string e = dir_has(filename, canonical_import_name(xx[i]));
		if (e == "")
			return Path::EMPTY;
		filename <<= e;
	}
	return filename;

	if (file_exists(dir0 << name))
		return dir0 << name;
	return Path::EMPTY;
}

Path find_installed_lib_import(const string &name) {
	Path kaba_dir = hui::Application::directory.parent() << "kaba";
	if (hui::Application::directory.basename()[0] == '.')
		kaba_dir = hui::Application::directory.parent() << ".kaba";
	Path kaba_dir_static = hui::Application::directory_static.parent() << "kaba";
	for (auto &dir: Array<Path>({kaba_dir, kaba_dir_static})) {
		auto path = (dir << "lib" << name).canonical();
		if (file_exists(path))
			return path;
	}
	return Path::EMPTY;
}

Path find_import(Module *s, const string &_name) {
	string name = _name.replace(".kaba", "");
	name = name.replace(".", "/") + ".kaba";

	if (name.head(2) == "@/")
		return find_installed_lib_import(name.sub(2));

	for (int i=0; i<MAX_IMPORT_DIRECTORY_PARENTS; i++) {
		Path filename = import_dir_match((s->filename.parent() << string("../").repeat(i)).canonical(), name);
		if (!filename.is_empty())
			return filename;
	}

	return find_installed_lib_import(name);

	return Path::EMPTY;
}

void Parser::parse_import() {
	string command = Exp.cur; // 'use' / 'import'
	bool indirect = (command == IDENTIFIER_IMPORT);
	Exp.next();

	// parse import name
	string name = Exp.cur;
	Exp.next();
	while (!Exp.end_of_line()) {
		if (Exp.cur != ".")
			do_error_exp("'.' expected in import name");
		name += ".";
		expect_no_new_line();
		Exp.next();
		name += Exp.cur;
		Exp.next();
	}
	
	if (name.match("\"*\""))
		name = name.sub(1, -1); // remove ""
		
	
	// internal packages?
	for (auto p: packages)
		if (p->filename.str() == name) {
			tree->add_include_data(p, indirect);
			return;
		}

	Path filename = find_import(tree->module, name);
	if (filename.is_empty())
		do_error_exp(format("can not find import '%s'", name));

	for (auto ss: weak(loading_module_stack))
		if (ss->filename == filename)
			do_error_exp("recursive include");

	msg_right();
	shared<Module> include;
	try {
		include = load(filename, tree->module->just_analyse or config.compile_os);
		// os-includes will be appended to syntax_tree... so don't compile yet
	} catch (Exception &e) {
		msg_left();

		int token_id = Exp.cur_token();
		string expr = Exp.get_token(token_id);
		e.line = Exp.token_physical_line_no(token_id);
		e.column = Exp.token_line_offset(token_id);
		e.text += format("\n...imported from:\nline %d, %s", e.line+1, tree->module->filename);
		throw e;
		//msg_write(e.message);
		//msg_write("...");
		string msg = e.message() + "\nimported file:";
		//string msg = "in imported file:\n\"" + e.message + "\"";
		do_error_exp(msg);
	}
	cur_exp_buf = &Exp;

	msg_left();
	tree->add_include_data(include, indirect);
}


void Parser::parse_enum(Class *_namespace) {
	Exp.next(); // 'enum'

	// class name?
	if (!Exp.end_of_line()) {
		_namespace = tree->create_new_class(Exp.cur, Class::Type::OTHER, 0, -1, nullptr, {}, _namespace, Exp.cur_token());
		Exp.next();
	}

	expect_new_line_with_indent();
	Exp.next_line();
	int indent0 = Exp.cur_line->indent;

	int next_value = 0;

	for (int i=0;!Exp.end_of_file();i++) {
		for (int j=0;!Exp.end_of_line();j++) {
			auto *c = tree->add_constant(TypeInt, _namespace);
			c->name = Exp.cur;
			Exp.next();

			// explicit value
			if (Exp.cur == "=") {
				Exp.next();
				expect_no_new_line();

				auto cv = parse_and_eval_const(tree->root_of_all_evil->block.get(), TypeInt);
				next_value = cv->as_const()->as_int();
			}
			c->as_int() = (next_value ++);

			if (Exp.end_of_line())
				break;
			if (Exp.cur != ",")
				do_error_exp("',' or newline expected after enum definition");
			Exp.next();
			expect_no_new_line();
		}
		if (Exp.next_line_indent() < indent0)
			break;
		Exp.next_line();
	}
}

bool type_needs_alignment(const Class *t) {
	if (t->is_array())
		return type_needs_alignment(t->get_array_element());
	return (t->size >= 4);
}

void parser_class_add_element(Parser *p, Class *_class, const string &name, const Class *type, Flags flags, int &_offset, int token_id) {

	// override?
	ClassElement *orig = nullptr;
	for (auto &e: _class->elements)
		if (e.name == name) //and e.type->is_pointer and el.type->is_pointer)
			orig = &e;
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
		_offset = process_class_offset(_class->cname(p->tree->base_class), name, _offset);
		auto el = ClassElement(name, type, _offset);
		_offset += type->size;
		_class->elements.add(el);
	}
}

Class *Parser::parse_class_header(Class *_namespace, int &offset0) {
	offset0 = 0;
	bool as_interface = (Exp.cur == IDENTIFIER_INTERFACE);
	Exp.next(); // 'class'/'interface'
	string name = Exp.cur;
	int token_id = Exp.cur_token();
	Exp.next();

	// create class
	Class *_class = const_cast<Class*>(tree->find_root_type_by_name(name, _namespace, false));
	// already created...
	if (!_class)
		tree->module->do_error_internal("class declaration ...not found " + name);
	_class->token_id = token_id;
	if (as_interface)
		_class->type = Class::Type::INTERFACE;

	if (Exp.cur == IDENTIFIER_AS) {
		Exp.next();
		if (Exp.cur == IDENTIFIER_SHARED)
			flags_set(_class->flags, Flags::SHARED);
		else
			do_error_exp("'shared' extected after 'as'");
		Exp.next();
	}

	// parent class
	if (Exp.cur == IDENTIFIER_EXTENDS) {
		Exp.next();
		auto parent = parse_type(_namespace); // force
		if (!parent->fully_parsed())
			return nullptr;
			//do_error(format("parent class '%s' not fully parsed yet", parent->long_name()));
		_class->derive_from(parent, true);
		offset0 = parent->size;
	}

	if (Exp.cur == IDENTIFIER_IMPLEMENTS) {
		Exp.next();
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
			auto f = parse_function_header(_class, _class->is_interface() ? Flags::VIRTUAL : Flags::NONE);
			skip_parsing_function_body(f);
		} else if (Exp.cur == IDENTIFIER_CONST) {
			parse_named_const(_class, tree->root_of_all_evil->block.get());
		} else if (Exp.cur == IDENTIFIER_VAR) {
			Exp.next(); // "var"
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

	// virtual functions?     (derived -> _class->num_virtual)
//	_class->vtable = cur_virtual_index;
	//foreach(ClassFunction &cf, _class->function)
	//	_class->num_virtual = max(_class->num_virtual, cf.virtual_index);
	if (_class->vtable.num > 0) {
		if (_class->parent) {
			if (_class->parent->vtable.num == 0)
				do_error("no virtual functions allowed when inheriting from class without virtual functions", _class->token_id);
			// element "-vtable-" being derived
		} else {
			for (ClassElement &e: _class->elements)
				e.offset = process_class_offset(_class->cname(tree->base_class), e.name, e.offset + config.pointer_size);

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
	_class->size = process_class_size(_class->cname(tree->base_class), size);


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

void Parser::expect_no_new_line() {
	if (Exp.end_of_line())
		do_error_exp("unexpected newline");
}

void Parser::expect_new_line() {
	if (!Exp.end_of_line())
		do_error_exp("newline expected");
}

void Parser::expect_new_line_with_indent() {
	if (!Exp.end_of_line())
		do_error_exp("newline expected");
	if (Exp.next_line_indent() <= Exp.cur_line->indent)
		do_error_exp("additional indent expected");
}

shared<Node> Parser::parse_and_eval_const(Block *block, const Class *type) {

	// find const value
	auto cv = parse_operand_greedy(block, true);

	if (type) {
		CastingData cast;
		if (type_match_with_cast(cv, false, type, cast)) {
			cv = apply_type_cast(cast, cv, type);
		} else {
			do_error(format("constant value of type '%s' expected", type->long_name()), cv);
		}
	} else {
		cv = force_concrete_type(cv);
		type = cv->type;
	}

	cv = tree->transform_node(cv, [&](shared<Node> n) { return tree->conv_eval_const_func(n); });

	if (cv->kind != NodeKind::CONSTANT) {
		//cv->show(TypeVoid);
		do_error("constant value expected", cv);
	}
	return cv;
}

void Parser::parse_named_const(Class *name_space, Block *block) {
	Exp.next(); // 'const'
	string name = Exp.cur;
	Exp.next();

	const Class *type = nullptr;
	if (Exp.cur == ":") {
		Exp.next();
		type = parse_type(name_space);
	}

	if (Exp.cur != "=")
		do_error_exp("'=' expected after const name");
	Exp.next();

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
	if (Exp.cur == ":") {
		Exp.next();
		type = parse_type(ns);
	} else if (Exp.cur != "=") {
		do_error_exp("':' or '=' expected after 'var' declaration");
	}

	Constant *c_value = nullptr;
	if (Exp.cur == "=") {
		Exp.next();

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
		auto cmd = link_operator_id(OperatorID::ASSIGN, tree->add_node_local(var), rhs);
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
	string name = Exp.cur;
	bool found = false;
	for (auto &e: c->elements)
		if (e.name == name) {
			e.allow_indirect_use = true;
			found = true;
		}
	if (!found)
		do_error_exp(format("use: class '%s' does not have an element '%s'", c->name, name));

	Exp.next();
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
	return concretify_as_type(cc, tree->root_of_all_evil->block.get(), ns);
}

bool node_is_template_type_name(Parser *parser, shared<Node> node) {
	if (node->kind != NodeKind::ABSTRACT_TOKEN)
		return false;
	auto t = parser->Exp.get_token(node->token_id);
	return (t.num == 1) and (t[0] >= 'A') and (t[0] <= 'Z');
}

bool func_is_template(Parser *parser, Function *f) {
	if (f->abstract_return_type)
		if (node_is_template_type_name(parser, f->abstract_return_type))
			return true;
	for (auto p: weak(f->abstract_param_types))
		if (node_is_template_type_name(parser, p))
			return true;
	return false;
}

Function *Parser::parse_function_header(Class *name_space, Flags flags) {
	Exp.next(); // "func"

	auto block = tree->root_of_all_evil->block.get();

	flags = parse_flags(flags);

	string name = Exp.cur;
	if (Exp.cur == "(") {
		static int lambda_count = 0;
		name = format("-lambda-%d-", lambda_count ++);
	} else {
		Exp.next();
	}

	Function *f = tree->add_function(name, TypeVoid, name_space, flags);
	f->is_abstract = true;
	//if (config.verbose)
	//	msg_write("PARSE HEAD  " + f->signature());
	f->token_id = Exp.cur_token();
	cur_func = f;

	if (Exp.cur != "(")
		do_error_exp("'(' expected after function name");
	Exp.next(); // '('

// parameter list

	if (Exp.cur != ")")
		for (int k=0;;k++) {
			// like variable definitions

			auto param_flags = parse_flags();

			string param_name = Exp.cur;
			Exp.next();

			if (Exp.cur != ":")
				do_error_exp("':' expected after parameter name");
			Exp.next();

			// type of parameter variable
			f->abstract_param_types.add(parse_abstract_operand(block, true));
			auto v = f->add_param(param_name, TypeUnknown, param_flags);


			// default parameter?
			if (Exp.cur == "=") {
				Exp.next();
				f->default_parameters.resize(f->num_params - 1);
				auto dp = parse_abstract_operand(block);
				f->default_parameters.add(dp);
			}

			if (Exp.cur == ")")
				break;

			if (Exp.cur != ",")
				do_error_exp("',' or ')' expected after parameter");
			Exp.next(); // ','
		}
	Exp.next(); // ')'

	if (Exp.cur == "->") {
		// return type
		Exp.next();
		f->abstract_return_type = parse_abstract_operand(tree->root_of_all_evil->block.get(), true);
	}

	if (!Exp.end_of_line())
		do_error_exp("newline expected after parameter list");

	if (func_is_template(this, f)) {
		TemplateManager::add_template(f);
		name_space->add_abstract_function(tree, f, flags_has(flags, Flags::VIRTUAL), flags_has(flags, Flags::OVERRIDE));
	} else {
		concretify_function_header(f);

		f->update_parameters_after_parsing();

		name_space->add_function(tree, f, flags_has(flags, Flags::VIRTUAL), flags_has(flags, Flags::OVERRIDE));
	}

	cur_func = nullptr;

	return f;
}

void Parser::concretify_function_header(Function *f) {
	auto block = tree->root_of_all_evil->block.get();

	if (f->abstract_return_type) {
		f->set_return_type(concretify_as_type(f->abstract_return_type, block, f->name_space));
	}
	f->literal_param_type.resize(f->abstract_param_types.num);
	foreachi (auto at, weak(f->abstract_param_types), i) {
		auto t = concretify_as_type(at, block, f->name_space);
		auto v = f->var[i];
		v->type = t;
		f->literal_param_type[i] = t;

		// mandatory_params not yet
		if ((i < f->default_parameters.num) and f->default_parameters[i]) {
			f->default_parameters[i] = concretify_node(f->default_parameters[i], block, f->name_space);
			if (f->default_parameters[i]->type != t)
				do_error(format("trying to set a default value of type '%s' for a parameter of type '%s'", f->default_parameters[i]->type->name, t->name), f->default_parameters[i]);
		}
	}
	f->is_abstract = false;
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

			auto_implement_regular_constructor(f, f->name_space, false);
		} else {
			auto_implement_regular_constructor(f, f->name_space, true);
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

	if (f->is_abstract) {
		//msg_error("ABSTRACT FUNC");
	} else {
		concretify_function_body(f);
	}

	cur_func = nullptr;
}

void Parser::concretify_function_body(Function *f) {
	concretify_node(f->block.get(), f->block.get(), f->name_space);

	// auto implement destructor?
	if (f->name == IDENTIFIER_FUNC_DELETE)
		auto_implement_regular_destructor(f, f->name_space);
}

void Parser::parse_all_class_names_in_block(Class *ns, int indent0) {
	while (!Exp.end_of_file()) {
		if ((Exp.cur_line->indent == indent0) and (Exp.cur_line->tokens.num >= 2)) {
			if ((Exp.cur == IDENTIFIER_CLASS) or (Exp.cur == IDENTIFIER_INTERFACE)) {
				Exp.next();
//				if (Exp.cur.num == 1)
//					do_error("class names must be at least 2 characters long", Exp.cur_token());
				Class *t = tree->create_new_class(Exp.cur, Class::Type::OTHER, 0, 0, nullptr, {}, ns, Exp.cur_token());
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
			flags = flags_mix({flags, Flags::STATIC});
		} else if (Exp.cur == IDENTIFIER_EXTERN) {
			flags = flags_mix({flags, Flags::EXTERN});
		} else if (Exp.cur == IDENTIFIER_CONST) {
			flags = flags_mix({flags, Flags::CONST});
		} else if (Exp.cur == IDENTIFIER_VIRTUAL) {
			flags = flags_mix({flags, Flags::VIRTUAL});
		} else if (Exp.cur == IDENTIFIER_OVERRIDE) {
			flags = flags_mix({flags, Flags::OVERRIDE});
		} else if (Exp.cur == IDENTIFIER_SELFREF) {
			flags = flags_mix({flags, Flags::SELFREF});
		//} else if (Exp.cur == IDENTIFIER_SHARED) {
		//	flags = flags_mix({flags, Flags::SHARED});
		//} else if (Exp.cur == IDENTIFIER_OWNED) {
		//	flags = flags_mix({flags, Flags::OWNED});
		} else if (Exp.cur == IDENTIFIER_OUT) {
			flags = flags_mix({flags, Flags::OUT});
		} else if (Exp.cur == IDENTIFIER_THROWS) {
			flags = flags_mix({flags, Flags::RAISES_EXCEPTIONS});
		} else if (Exp.cur == IDENTIFIER_PURE) {
			flags = flags_mix({flags, Flags::PURE});
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

		} else if (Exp.cur == IDENTIFIER_CONST) {
			parse_named_const(tree->base_class, tree->root_of_all_evil->block.get());

		} else if (Exp.cur == IDENTIFIER_VAR) {
			Exp.next(); // "var"
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
	cur_exp_buf = &Exp;
	Exp.reset_walker();

	parse_top_level();

	parse_all_function_bodies();
	
	tree->show("aaa");

	for (auto *f: tree->functions)
		test_node_recursion(f->block.get(), tree->base_class, "a " + f->long_name());

	for (int i=0; i<tree->owned_classes.num; i++) // array might change...
		auto_implement_functions(tree->owned_classes[i]);

	for (auto *f: tree->functions)
		test_node_recursion(f->block.get(), tree->base_class, "b " + f->long_name());
}

}
