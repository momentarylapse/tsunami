#include "../kaba.h"
#include "../lib/common.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include "../../hui/Application.h"
#include "Parser.h"
#include <stdio.h>


namespace Kaba{

void test_node_recursion(Node *root, const Class *ns, const string &message);

ExpressionBuffer *cur_exp_buf = nullptr;


extern const Class *TypeAbstractList;
extern const Class *TypeAbstractDict;
extern const Class *TypeIntList;
extern const Class *TypeAnyList;
extern const Class *TypeAnyDict;
extern const Class *TypeDynamicArray;
extern const Class *TypeIntDict;

const int TYPE_CAST_NONE = -1;
const int TYPE_CAST_DEREFERENCE = -2;
const int TYPE_CAST_REFERENCE = -3;
const int TYPE_CAST_OWN_STRING = -10;
const int TYPE_CAST_ABSTRACT_LIST = -20;
const int TYPE_CAST_CLASSIFY = -30;

bool type_match(const Class *given, const Class *wanted);
bool type_match_with_cast(Node *node, bool is_modifiable, const Class *wanted, int &penalty, int &cast);



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
}


void Parser::parse_buffer(const string &buffer, bool just_analyse) {
	Exp.analyse(tree, buffer);

	pre_compiler(just_analyse);

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
		value.as_string() = str.substr(1, -2).unescape();
	} else if (value.type == TypeCString) {
		strcpy((char*)value.p(), str.substr(1, -2).unescape().c_str());
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


// override_line is logical! not physical
void Parser::do_error(const string &str, int override_exp_no, int override_line) {
	// what data do we have?
	int logical_line = Exp.get_line_no();
	int exp_no = Exp.cur_exp;
	int physical_line = 0;
	int pos = 0;
	string expr;

	// override?
	if (override_line >= 0) {
		logical_line = override_line;
		exp_no = 0;
	}
	if (override_exp_no >= 0)
		exp_no = override_exp_no;

	// logical -> physical
	if ((logical_line >= 0) and (logical_line < Exp.line.num)) {
		physical_line = Exp.line[logical_line].physical_line;
		pos = Exp.line[logical_line].exp[exp_no].pos;
		expr = Exp.line[logical_line].exp[exp_no].name;
	}

#ifdef CPU_ARM
	msg_error(str);
#endif
	throw Exception(str, expr, physical_line, pos, tree->script);
}

Array<Node*> Parser::parse_operand_extension_element(Node *operand) {
	Exp.next();
	const Class *type = operand->type;
	bool deref = false;
	bool only_static = false;

	if (operand->kind == NodeKind::CLASS) {
		// referencing class functions
		type = operand->as_class();
		only_static = true;
	} else if (type->is_pointer()) {
		// pointer -> dereference
		type = type->param;
		deref = true;
	}

	// super
	if ((type->parent) and (Exp.cur == IDENTIFIER_SUPER)) {
		Exp.next();
		if (deref) {
			operand->type = type->parent->get_pointer();
			return {operand};
		}
		return {tree->ref_node(operand, type->parent->get_pointer())};
	}


	// find element
	if (!only_static) {
		for (auto &e: type->elements)
			if (Exp.cur == e.name) {
				Exp.next();
				return {tree->shift_node(operand, deref, e.offset, e.type)};
			}
	}
	for (auto *c: type->constants)
		if (Exp.cur == c->name) {
			Exp.next();
			return {tree->add_node_const(c)};
		}
	for (auto *v: type->static_variables)
		if (Exp.cur == v->name) {
			Exp.next();
			return {tree->add_node_global(v)};
		}
		
	// sub-class
	for (auto *c: type->classes)
		if (Exp.cur == c->name) {
			Exp.next();
			return {tree->add_node_class(c)};
		}


	if (deref and !only_static)
		operand = tree->deref_node(operand);

	string f_name = Exp.cur;

	// class function?
	Array<Node*> links;
	for (auto *cf: type->functions)
		if (f_name == cf->name) {
			links.add(tree->add_node_func_name(cf));
			if (!cf->is_static() and !only_static)
				links.back()->params.add(tree->cp_node(operand));
		}
	if (links.num > 0) {
		Exp.next();
		return links;
	}

	do_error(format("unknown element of '%s'", type->long_name()));
	return {};
}

Node *Parser::parse_operand_extension_array(Node *operand, Block *block) {
	operand = force_concrete_type(operand);
	operand = deref_if_pointer(operand);

	// array index...
	Exp.next();
	Node *index = nullptr;
	Node *index2 = nullptr;
	if (Exp.cur == ":") {
		index = tree->add_node_const(tree->add_constant_int(0));
	} else {
		index = parse_operand_super_greedy(block);
	}
	if (Exp.cur == ":") {
		Exp.next();
		if (Exp.cur == "]") {
			index2 = tree->add_node_const(tree->add_constant_int(0x81234567));
			// magic value (-_-)'
		} else {
			index2 = parse_operand_greedy(block);
		}
	}
	if (Exp.cur != "]")
		do_error("']' expected after array index");
	Exp.next();

	// subarray() ?
	if (index2) {
		auto *cf = operand->type->get_func(IDENTIFIER_FUNC_SUBARRAY, operand->type, {index->type, index->type});
		if (cf) {
			Node *f = tree->add_node_member_call(cf, operand);
			f->is_const = operand->is_const;
			f->set_param(1, index);
			f->set_param(2, index2);
			return f;
		}
	}

	// __get__() ?
	auto *cf = operand->type->get_get(index->type);
	if (cf) {
		Node *f = tree->add_node_member_call(cf, operand);
		f->is_const = operand->is_const;
		f->set_param(1, index);
		return f;
	}

	// allowed?
	bool allowed = ((operand->type->is_array()) or (operand->type->usable_as_super_array()));
	bool pparray = false;
	if (!allowed)
		if (operand->type->is_pointer()) {
			if ((!operand->type->param->is_array()) and (!operand->type->param->usable_as_super_array()))
				do_error(format("using pointer type '%s' as an array (like in C) is not allowed any more", operand->type->long_name()));
			allowed = true;
			pparray = (operand->type->param->usable_as_super_array());
		}
	if (!allowed)
		do_error(format("type '%s' is neither an array nor a pointer to an array nor does it have a function __get__(%s)", operand->type->long_name(), index->type->long_name()));


	if (index->type != TypeInt) {
		Exp.rewind();
		do_error(format("type of index for an array needs to be 'int', not '%s'", index->type->long_name()));
	}

	Node *array = nullptr;

	// pointer?
	if (pparray) {
		do_error("test... anscheinend gibt es [] auf * super array");
		//array = cp_command(this, Operand);
/*		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent;
		deref_command_old(this, Operand);
		array = Operand->param[0];*/
	} else if (operand->type->usable_as_super_array()) {
		array = tree->add_node_dyn_array(operand, index);
	} else if (operand->type->is_pointer()) {
		array = tree->add_node_parray(operand, index, operand->type->param->param);
	} else {
		array = tree->add_node_array(operand, index);
	}
	array->is_const = operand->is_const;
	return array;
}

void Parser::make_func_node_callable(Node *l) {
	Function *f = l->as_func();
	l->kind = NodeKind::FUNCTION_CALL;
	l->type = f->literal_return_type;
	if (f->is_static())
		l->set_num_params(f->num_params);
	else
		l->set_num_params(f->num_params + 1);

	// virtual?
	if (f->virtual_index >= 0)
		l->kind = NodeKind::VIRTUAL_CALL;
}

Node *SyntaxTree::make_fake_constructor(const Class *t, Block *block, const Class *param_type) {
	//if ((t == TypeInt) and (param_type == TypeFloat32))
	//	return add_node_call(get_existence("f2i", nullptr, nullptr, false)[0]->as_func());
	if (param_type->is_pointer())
		param_type = param_type->param;
		
	auto *cf = param_type->get_func("__" + t->name + "__", t, {});
	if (!cf)
		do_error(format("illegal fake constructor... requires '%s.%s()'", param_type->long_name(), t->long_name()));
	return add_node_member_call(cf, nullptr); // temp var added later...
		
	auto *dummy = new Node(NodeKind::PLACEHOLDER, 0, TypeVoid);
	return add_node_member_call(cf, dummy); // temp var added later...
}

Node *SyntaxTree::add_node_constructor(Function *f) {
	auto *dummy = new Node(NodeKind::PLACEHOLDER, 0, TypeVoid);
	Node *n = add_node_member_call(f, dummy); // temp var added later...
	n->kind = NodeKind::CONSTRUCTOR_AS_FUNCTION;
	n->type = f->name_space;
	return n;
}

Array<Node*> Parser::make_class_node_callable(const Class *t, Block *block, Array<Node*> &params) {
	if (((t == TypeInt) or (t == TypeFloat32) or (t == TypeInt64) or (t == TypeFloat64) or (t == TypeBool)) and (params.num == 1))
		return {tree->make_fake_constructor(t, block, params[0]->type)};
	
	// constructor
	//auto *vv = block->add_var(block->function->create_slightly_hidden_name(), t);
	//vv->explicitly_constructed = true;
	//Node *dummy = add_node_local(vv);
	Array<Node*> links;
	for (auto *cf: t->get_constructors())
		links.add(tree->add_node_constructor(cf));
	return links;
}

Array<const Class*> Parser::type_list_from_nodes(const Array<Node*> &nn) {
	Array<const Class*> t;
	for (auto *n: nn)
		t.add(force_concrete_type(n)->type);
	return t;
}

string type_list_to_str(const Array<const Class*> &tt) {
	string s;
	for (auto *t: tt) {
		if (s.num > 0)
			s += ", ";
		s += t->long_name();
	}
	return "(" + s + ")";
}

Node *check_const_params(SyntaxTree *tree, Node *n) {
	if (n->kind == NodeKind::FUNCTION_CALL) {
		auto f = n->as_func();
		int offset = 0;
		if (!f->is_static()) {
			offset = 1;
			if (f->is_selfref()) {
				// const(return) = const(instance)
				n->is_const = n->params[0]->is_const;
			} else if (n->params[0]->is_const and !f->is_const()){
				//n->show();
				tree->do_error(f->long_name() + ": member function expects a mutable instance, because it is declared without 'const'");
			}
		}
		for (int i=0; i<f->num_params; i++)
			if (n->params[i+offset]->is_const and !f->var[i]->is_const)
				tree->do_error(format("%s: function parameter %d ('%s') is 'out' and does not accept a constant value", f->long_name(), i+1, f->var[i]->name));
	}
	return n;
}

Node *Parser::parse_operand_extension_call(Array<Node*> links, Block *block) {
	// parse all parameters
	auto params = parse_call_parameters(block);

	// make links callable
	for (Node *l: links) {
		if (l->kind == NodeKind::FUNCTION) {
			make_func_node_callable(l);
		} else if (l->kind == NodeKind::CLASS) {
			auto *t = links[0]->as_class();
			clear_nodes(links);
			links = make_class_node_callable(t, block, params);
			break;
		} else if (l->type == TypeFunctionCodeP) {
			Node *p = links[0];
			clear_nodes(links, p);
			Node *c = new Node(NodeKind::POINTER_CALL, 0, TypeVoid);
			c->set_num_params(1);
			c->set_param(0, p);
			links = {c};
			//do_error("calling pointer...");
		} else {
			do_error("can't call " + kind2str(l->kind));
		}
	}


	// find (and provisional link) the parameters in the source

	/*bool needs_brackets = ((Operand->type != TypeVoid) or (Operand->param.num != 1));
	if (needs_brackets) {
		FindFunctionParameters(wanted_type, block, Operand);

	} else {
		wanted_type.add(TypeUnknown);
		FindFunctionSingleParameter(0, wanted_type, block, Operand);
	}*/

	// direct match...
	for (Node *operand: links) {
		if (!direct_param_match(operand, params))
			continue;

		clear_nodes(links, operand);
		return check_const_params(tree, apply_params_direct(operand, params));
	}


	// advanced match...
	for (Node *operand: links) {
		Array<int> casts;
		Array<const Class*> wanted;
		if (!param_match_with_cast(operand, params, casts, wanted))
			continue;

		clear_nodes(links, operand);
		return check_const_params(tree, apply_params_with_cast(operand, params, casts, wanted));
	}


	// error message
	
	if (links.num == 0)
		do_error("can not call ...");

	string found = type_list_to_str(type_list_from_nodes(params));
	string available;
	for (Node *link: links) {
		auto p = get_wanted_param_types(link);
		available += format("\n%s: %s", link->sig(tree->base_class), type_list_to_str(p));
	}
	do_error(format("invalid function parameters: %s, expected: %s", found, available));
	return nullptr;
}

const Class *Parser::parse_type_extension_array(const Class *t) {
	Exp.next(); // "["

	// no index -> super array
	if (Exp.cur == "]") {
		t = tree->make_class_super_array(t);
	} else {

		// find array index
		Node *c = parse_operand_super_greedy(tree->root_of_all_evil->block);
		c = tree->transform_node(c, [&](Node *n) { return tree->conv_eval_const_func(n); });

		if ((c->kind != NodeKind::CONSTANT) or (c->type != TypeInt))
			do_error("only constants of type 'int' allowed for size of arrays");
		int array_size = c->as_const()->as_int();
		//Exp.next();
		if (Exp.cur != "]")
			do_error("']' expected after array size");
		t = tree->make_class_array(t, array_size);
	}

	Exp.next();
	return t;
}

const Class *Parser::parse_type_extension_dict(const Class *c) {
	Exp.next(); // "{"

	if (Exp.cur != "}")
		do_error("'}' expected after dict 'class{'");

	Exp.next();

	return tree->make_class_dict(c);
}


const Class *Parser::parse_type_extension_pointer(const Class *c) {
	Exp.next(); // "*"
	return c->get_pointer();
}


// find any ".", or "[...]"'s    or operators?
Node *Parser::parse_operand_extension(Array<Node*> operands, Block *block) {

	// special
	if ((operands[0]->kind == NodeKind::CLASS) and ((Exp.cur == "*") or (Exp.cur == "[") or (Exp.cur == "{"))) {
		if (operands.num > 1)
			do_error("ambiguous class?!?!?");
		auto *t = operands[0]->as_class();

		if (Exp.cur == "*") {
			t = parse_type_extension_pointer(t);
		} else if (Exp.cur == "[") {
			t = parse_type_extension_array(t);
		} else if (Exp.cur == "{") {
			t = parse_type_extension_dict(t);
		}

		return parse_operand_extension({tree->add_node_class(t)}, block);
	}

	// nothing?
	auto primop = which_primitive_operator(Exp.cur, 1); // ++,--
	if ((Exp.cur != ".") and (Exp.cur != "[") and (Exp.cur != "(") and (!primop))
		return operands[0];

	if (Exp.cur == ".") {
		if (operands.num > 1)
			do_error("left side of '.' is ambiguous");
		// class element?

		operands = parse_operand_extension_element(operands[0]);

	} else if (Exp.cur == "[") {
		if (operands.num > 1)
			do_error("left side of '[' is ambiguous");
			
		// array?
		operands = {parse_operand_extension_array(operands[0], block)};

	} else if (Exp.cur == "(") {

		operands = {parse_operand_extension_call(operands, block)};


	} else if (primop) {
		if (operands.num > 1)
			do_error("left side of ++/-- is ambiguous");
		// unary operator? (++,--)

		for (auto *op: tree->operators)
			if (op->primitive == primop)
				if ((op->param_type_1 == operands[0]->type) and (!op->param_type_2)) {
					Exp.next();
					return tree->add_node_operator(operands[0], nullptr, op);
				}
		return operands[0];
	}

	// recursion
	return parse_operand_extension(operands, block);
}

void clear_nodes(Array<Node*> &nodes) {
	for (auto *n: nodes) {
		if (n->kind == NodeKind::FUNCTION and !n->as_func()->is_static())
			n->params[0] = nullptr;
		delete n;
	}
	nodes.clear();
}

void clear_nodes(Array<Node*> &nodes, Node *keep) {
	for (auto *n: nodes)
		if (n != keep) {
			if (n->kind == NodeKind::FUNCTION and !n->as_func()->is_static())
				n->params[0] = nullptr;
			delete n;
		}
	nodes.clear();
}

// when calling ...(...)
Array<const Class*> Parser::get_wanted_param_types(Node *link) {
	if ((link->kind == NodeKind::FUNCTION_CALL) or (link->kind == NodeKind::FUNCTION) or (link->kind == NodeKind::VIRTUAL_CALL) or (link->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION)) {
		auto f = link->as_func();
		auto p = f->literal_param_type;
		if (!f->is_static() and (link->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION))
			if (link->params.num == 0 or !link->params[0])
				p.insert(f->name_space, 0);
		return p;
	} else if (link->kind == NodeKind::CLASS) {
		// should be caught earlier and turned to func...
		const Class *t = link->as_class();
		for (auto *c: t->get_constructors())
			return c->literal_param_type;
	} else if (link->kind == NodeKind::POINTER_CALL) {
	//} else if (link->type == TypeFunctionCodeP) {
		return {}; // so far only void() pointers...)
	} else {
		do_error("evil function...kind: "+kind2str(link->kind));
	}

	return {};
}

Array<Node*> Parser::parse_call_parameters(Block *block) {
	if (Exp.cur != "(")
		do_error("'(' expected in front of function parameter list");

	Exp.next();

	Array<Node*> params;

	// list of parameters
	for (int p=0;;p++) {
		if (Exp.cur == ")")
			break;

		// find parameter
		params.add(parse_operand_greedy(block));

		if (Exp.cur != ",") {
			if (Exp.cur == ")")
				break;
			do_error("',' or ')' expected after parameter for function");
		}
		Exp.next();
	}
	Exp.next(); // ')'
	return params;
}



// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
Node *Parser::check_param_link(Node *link, const Class *wanted, const string &f_name, int param_no) {
	// type cast needed and possible?
	const Class *given = link->type;

	if (type_match(given, wanted))
		return link;

	if (wanted->is_pointer_silent()) {
		// "silent" pointer (&)?
		if (type_match(given, wanted->param)) {

			return tree->ref_node(link);
		} else if ((given->is_pointer()) and (type_match(given->param, wanted->param))) {
			// silent reference & of *

			return link;
		} else {
			Exp.rewind();
			do_error(format("(c) parameter %d in command '%s' has type '%s', '%s' expected", param_no + 1, f_name, given->long_name(), wanted->long_name()));
		}

	} else {
		// normal type cast
		int pen, tc;

		if (type_match_with_cast(link, false, wanted, pen, tc))
			return apply_type_cast(tc, link, wanted);

		Exp.rewind();
		do_error(format("parameter %d in command '%s' has type '%s', '%s' expected", param_no + 1, f_name, given->long_name(), wanted->long_name()));
	}
	return link;
}

bool Parser::direct_param_match(Node *operand, Array<Node*> &params) {
	auto wanted_types = get_wanted_param_types(operand);
	if (wanted_types.num != params.num)
		return false;
	for (int p=0; p<params.num; p++)
		if (!type_match(params[p]->type, wanted_types[p]))
			return false;
	return true;
}

bool Parser::param_match_with_cast(Node *operand, Array<Node*> &params, Array<int> &casts, Array<const Class*> &wanted) {
	wanted = get_wanted_param_types(operand);
	if (wanted.num != params.num)
		return false;
	casts.resize(params.num);
	for (int p=0; p<params.num; p++) {
		int penalty;
		if (!type_match_with_cast(params[p], false, wanted[p], penalty, casts[p]))
			return false;
	}
	return true;
}

bool node_is_function(Node *n) {
	return n->kind == NodeKind::FUNCTION_CALL or n->kind == NodeKind::VIRTUAL_CALL or n->kind == NodeKind::INLINE_CALL or n->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION;
}

bool node_is_member_function_with_instance(Node *n) {
	if (!node_is_function(n))
		return false;
	auto *f = n->as_func();
	if (f->is_static())
		return false;
	return n->params.num == 0 or n->params[0];
}

Node *Parser::apply_params_direct(Node *operand, Array<Node*> &params) {
	int offset = 0;
	if (node_is_member_function_with_instance(operand))
		offset = 1;
	for (int p=0; p<params.num; p++)
		operand->set_param(p + offset, params[p]);
	return operand;
}

Node *Parser::apply_params_with_cast(Node *operand, const Array<Node*> &params, const Array<int> &casts, const Array<const Class*> &wanted) {
	int offset = 0;
	if (node_is_member_function_with_instance(operand))
		offset = 1;
	for (int p=0; p<params.num; p++) {
		auto pp = apply_type_cast(casts[p], params[p], wanted[p]);
		operand->set_param(p + offset, pp);
	}
	return operand;
}

Node *Parser::build_abstract_list(const Array<Node*> &el) {
	Node *c = new Node(NodeKind::ARRAY_BUILDER, 0, TypeAbstractList, true);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

Node *Parser::build_abstract_dict(const Array<Node*> &el) {
	Node *c = new Node(NodeKind::DICT_BUILDER, 0, TypeAbstractDict, true);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++)
		c->set_param(i, el[i]);
	return c;
}

Node *Parser::link_unary_operator(PrimitiveOperator *po, Node *operand, Block *block) {
	int _ie = Exp.cur_exp - 1;
	Operator *op = nullptr;
	const Class *p2 = operand->type;

	// exact match?
	bool ok=false;
	for (auto *_op: tree->operators)
		if (po == _op->primitive)
			if ((!_op->param_type_1) and (type_match(p2, _op->param_type_2))) {
				op = _op;
				ok = true;
				break;
			}


	// needs type casting?
	if (!ok) {
		int pen2 = 0;
		int c2 = -1, c2_best = -1;
		int pen_min = 100;
		const Class *t_best = nullptr;
		for (auto *_op: tree->operators)
			if (po == _op->primitive)
				if ((!_op->param_type_1) and (type_match_with_cast(operand, false, _op->param_type_2, pen2, c2))) {
					ok = true;
					if (pen2 < pen_min) {
						op = _op;
						pen_min = pen2;
						c2_best = c2;
						t_best = _op->param_type_2;
					}
			}
		// cast
		if (ok) {
			operand = apply_type_cast(c2_best, operand, t_best);
		}
	}


	if (!ok)
		do_error(format("unknown unitary operator '%s %s'", po->name, p2->long_name()), _ie);
	return tree->add_node_operator(operand, nullptr, op);
}

Node *Parser::parse_set_builder(Block *block) {
	//Exp.next(); // [
	Node *n_for = parse_for_header(block);

	Node *n_exp = parse_operand_greedy(block);
	
	Node *n_cmp = nullptr;
	if (Exp.cur == IDENTIFIER_IF) {
		Exp.next(); // if
		n_cmp = parse_operand_greedy(block);
	}


	if (Exp.cur != "]")
		do_error("] expected");
	Exp.next();


	const Class *el_type = n_exp->type;
	const Class *type = tree->make_class_super_array(el_type);
	auto *var = block->add_var(block->function->create_slightly_hidden_name(), type);

	// array.add(exp)
	auto *f_add = type->get_func("add", TypeVoid, {el_type});
	if (!f_add)
		do_error("...add() ???");
	auto *n_add = tree->add_node_member_call(f_add, tree->add_node_local(var));
	n_add->set_param(1, n_exp);

	Block *b;
	if (n_cmp) {
		Block *b_if = new Block(block->function, block);
		Block *b_add = new Block(block->function, b_if);
		b_add->add(n_add);
	
		Node *n_if = tree->add_node_statement(StatementID::IF);
		n_if->set_param(0, n_cmp);
		n_if->set_param(1, b_add);
	
		b_if->add(n_if);
		b = b_if;
	} else {
		b = new Block(block->function, block);
		b->add(n_add);
	}

	n_for->set_param(n_for->params.num - 1, b);

	post_process_for(n_for);

	Node *n = new Node(NodeKind::ARRAY_BUILDER_FOR, 0, type);
	n->set_num_params(2);
	n->set_param(0, n_for);
	n->set_param(1, tree->add_node_local(var));
	return n;

}


Node *Parser::apply_format(Node *n, const string &fmt) {
	auto f = n->type->get_func("format", TypeString, {TypeString});
	if (!f)
		do_error(format("format string: no '%s.format(string)' function found", n->type->long_name()));
	auto *c = tree->add_constant(TypeString);
	c->as_string() = fmt;
	auto nf = tree->add_node_call(f);
	nf->set_instance(n);
	nf->set_param(1, tree->add_node_const(c));
	return nf;
}

Node *Parser::try_parse_format_string(Block *block, Value &v) {
	string s = v.as_string();
	
	Array<Node*> parts;
	int pos = 0;
	
	while (pos < s.num) {
	
		int p0 = s.find("{{", pos);
		
		// constant part before the next {{insert}}
		int pe = (p0 < 0) ? s.num : p0;
		if (pe > pos) {
			auto *c = tree->add_constant(TypeString);
			c->as_string() = s.substr(pos, pe-pos);
			parts.add(tree->add_node_const(c));
		}
		if (p0 < 0)
			break;
			
		int p1 = s.find("}}", p0);
		if (p1 < 0)
			do_error("string interpolation '{{' not ending with '}}'");
			
		string xx = s.substr(p0+2, p1 - p0 - 2);

		// "expr|format" ?
		string fmt;
		int pp = xx.find("|");
		if (pp >= 0) {
			fmt = xx.substr(pp + 1, -1);
			xx = xx.head(pp);
		}

		//msg_write("format:  " + xx);
		ExpressionBuffer ee;
		ee.analyse(tree, xx);
		ee.cur_line->physical_line = Exp.cur_line->physical_line;
		//ee.show();
		
		int cl = Exp.get_line_no();
		int ce = Exp.cur_exp;
		Exp.line.add(ee.line[0]);
		Exp.set(0, Exp.line.num-1);
		
		try {
			Node *n = parse_operand_super_greedy(block);
			n = deref_if_pointer(n);

			if (fmt != "") {
				n = apply_format(n, fmt);
			} else {
				n = check_param_link(n, TypeString, "", 0);
			}
			//n->show();
			parts.add(n);
		} catch (Exception &e) {
			Exp.set(ce, cl);
			//e.line += cl;
			//e.column += Exp.
			
			// not perfect (e has physical line-no etc and e.text has filenames baked in)
			do_error(e.text);
		}
		
		Exp.line.pop();
		Exp.set(ce, cl);
		
		pos = p1 + 2;
	
	}
	
	// empty???
	if (parts.num == 0) {
		auto *c = tree->add_constant(TypeString);
		return tree->add_node_const(c);
	}
	
	// glue
	while (parts.num > 1) {
		auto *b = parts.pop();
		auto *a = parts.pop();
		auto *n = link_operator_id(OperatorID::ADD, a, b);
		parts.add(n);
	}
	//parts[0]->show();
	return parts[0];
}

Node *Parser::parse_list(Block *block) {
	Array<Node*> el;
	while(true) {
		if (Exp.cur == "]")
			break;
		el.add(parse_operand_greedy(block));
		if ((Exp.cur != ",") and (Exp.cur != "]"))
			do_error("',' or ']' expected");
		if (Exp.cur == "]")
			break;
		Exp.next();
	}
	Exp.next();
	return build_abstract_list(el);
}

Node *Parser::parse_dict(Block *block) {
	Array<Node*> el;
	while(true) {
		if (Exp.cur == "}")
			break;
		auto key = parse_operand_greedy(block);
		if (key->type != TypeString or key->kind != NodeKind::CONSTANT)
			do_error("key needs to be a constant string");
		el.add(key);
		if (Exp.cur != ":")
			do_error("':' after key expected");
		Exp.next();
		auto value = parse_operand_greedy(block);
		if ((Exp.cur != ",") and (Exp.cur != "}"))
			do_error("',' or '}' expected");
		el.add(value);
		if (Exp.cur == "}")
			break;
		Exp.next();
	}
	Exp.next();
	return build_abstract_dict(el);
}

// minimal operand
// but with A[...], A(...) etc
Node *Parser::parse_operand(Block *block, bool prefer_class) {
	Array<Node*> operands;

	// ( -> one level down and combine commands
	if (Exp.cur == "(") {
		Exp.next();
		operands = {parse_operand_super_greedy(block)};
		if (Exp.cur != ")")
			do_error("')' expected");
		Exp.next();
	} else if (Exp.cur == "&") { // & -> address operator
		Exp.next();
		operands = {tree->ref_node(parse_operand(block))};
	} else if (Exp.cur == "*") { // * -> dereference
		Exp.next();
		Node *sub = parse_operand(block);
		if (!sub->type->is_pointer()) {
			Exp.rewind();
			do_error("only pointers can be dereferenced using '*'");
		}
		operands = {tree->deref_node(sub)};
	} else if (Exp.cur == "[") {
		Exp.next();
		if (Exp.cur == "for") {
			operands = {parse_set_builder(block)};
		} else {
			operands = {parse_list(block)};
		}
	} else if (Exp.cur == "{") {
		Exp.next();
		operands = {parse_dict(block)};
	} else {
		// direct operand
		operands = tree->get_existence(Exp.cur, block, block->name_space(), prefer_class);
		if (operands.num > 0) {

			if (operands[0]->kind == NodeKind::STATEMENT) {
				operands = {parse_statement(block)};

			} else if (operands[0]->kind == NodeKind::PRIMITIVE_OPERATOR) {
				// unary operator
				Exp.next();
				auto po = operands[0]->as_prim_op();
				clear_nodes(operands);
				Node *sub_command = parse_operand(block);
				return link_unary_operator(po, sub_command, block);
			} else {
				Exp.next();
				// direct operand!

			}
		} else {
			const Class *t = get_constant_type(Exp.cur);
			if (t == TypeUnknown)
				do_error("unknown operand");

			Value v;
			get_constant_value(Exp.cur, v);
			Exp.next();
			
			if (t == TypeString) {
				operands = {try_parse_format_string(block, v)};
			} else {
				auto *c = tree->add_constant(t);
				c->set(v);
				operands = {tree->add_node_const(c)};
			}
		}

	}
	if (Exp.end_of_line())
		return operands[0];

	// resolve arrays, structures, calls...
	return parse_operand_extension(operands, block);
}

// only "primitive" operator -> no type information
Node *Parser::parse_primitive_operator(Block *block) {
	auto op = which_primitive_operator(Exp.cur, 3);
	if (!op)
		return nullptr;

	// command from operator
	Node *cmd = new Node(NodeKind::PRIMITIVE_OPERATOR, (int_p)op, TypeUnknown);
	// only provisional (only operator sign, parameters and their types by GetCommand!!!)

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


bool type_match_with_cast(Node *node, bool is_modifiable, const Class *wanted, int &penalty, int &cast) {
	penalty = 0;
	auto given = node->type;
	cast = TYPE_CAST_NONE;
	if (type_match(given, wanted))
		return true;
	if (is_modifiable) // is a variable getting assigned.... better not cast
		return false;
	if (given->is_pointer()) {
		if (type_match(given->param, wanted)) {
			penalty = 10;
			cast = TYPE_CAST_DEREFERENCE;
			return true;
		}
	}
	if (wanted->is_pointer_silent()) {
		// "silent" pointer (&)?
		if (type_match(given, wanted->param)) {
			cast = TYPE_CAST_REFERENCE;
			return true;
		} else if ((given->is_pointer()) and (type_match(given->param, wanted->param))) {
			// silent reference & of *
			return true;
		}
	}
	if (node->kind == NodeKind::ARRAY_BUILDER and given == TypeAbstractList) {
		if (wanted->is_super_array()) {
			auto t = wanted->get_array_element();
			int pen, c;
			for (auto *e: node->params)
				if (!type_match_with_cast(e, false, t, pen, c))
					return false;
			cast = TYPE_CAST_ABSTRACT_LIST;
			return true;
		}
		if (wanted == TypeDynamicArray) {
			cast = TYPE_CAST_ABSTRACT_LIST;
			return true;
		}
		for (auto *f: wanted->get_constructors()) {
			if (f->literal_param_type.num != node->params.num)
				continue;

			int pen, c;
			foreachi (auto *e, node->params, i)
				if (!type_match_with_cast(e, false, f->literal_param_type[i], pen, c))
					return false;
			cast = TYPE_CAST_CLASSIFY;
			return true;
		}
	}
	if (wanted == TypeString) {
		Function *cf = given->get_func(IDENTIFIER_FUNC_STR, TypeString, {});
		if (cf) {
			penalty = 50;
			cast = TYPE_CAST_OWN_STRING;
			return true;
		}
	}
	foreachi(auto &c, TypeCasts, i)
		if ((type_match(given, c.source)) and (type_match(c.dest, wanted))) {
			penalty = c.penalty;
			cast = i;
			return true;
		}
	return false;
}

Node *Parser::apply_type_cast(int tc, Node *node, const Class *wanted) {
	if (tc == TYPE_CAST_NONE)
		return node;
	if (tc == TYPE_CAST_DEREFERENCE)
		return tree->deref_node(node);
	if (tc == TYPE_CAST_REFERENCE)
		return tree->ref_node(node);
	if (tc == TYPE_CAST_OWN_STRING) {
		Function *cf = node->type->get_func(IDENTIFIER_FUNC_STR, TypeString, {});
		if (cf)
			return tree->add_node_member_call(cf, node);
		do_error("automatic .str() not implemented yet");
		return node;
	}
	if (tc == TYPE_CAST_ABSTRACT_LIST) {
		if (wanted == TypeDynamicArray)
			return force_concrete_type(node);
		int pen, c;
		foreachi (auto *e, node->params, i) {
			if (!type_match_with_cast(e, false, wanted->get_array_element(), pen, c)) {
				do_error("nope????");
			}
			node->params[i] = apply_type_cast(c, e, wanted->get_array_element());
		}
		node->type = wanted;
		return node;
	}

	if (tc == TYPE_CAST_CLASSIFY) {
		Array<int> c;
		c.resize(node->params.num);
		for (auto *f: wanted->get_constructors()) {
			if (f->literal_param_type.num != node->params.num)
				continue;
			int pen;
			bool ok = true;
			foreachi (auto *e, node->params, i)
				if (!type_match_with_cast(e, false, f->literal_param_type[i], pen, c[i]))
					ok = false;
			if (!ok)
				continue;
			auto cmd = tree->add_node_constructor(f);
			return apply_params_with_cast(cmd, node->params, c, f->literal_param_type);
		}
		do_error("classify...");
	}
	
	Node *c = tree->add_node_call(TypeCasts[tc].f);
	c->type = TypeCasts[tc].dest;
	c->set_param(0, node);
	return c;
}

Node *Parser::link_special_operator_is(Node *param1, Node *param2) {
	if (param2->kind != NodeKind::CLASS)
		do_error("class name expected after 'is'");
	const Class *t2 = param2->as_class();
	if (t2->vtable.num == 0)
		do_error(format("class after 'is' needs to have virtual functions: '%s'", t2->long_name()));

	const Class *t1 = param1->type;
	if (t1->is_pointer()) {
		param1 = tree->deref_node(param1);
		t1 = t1->param;
	}
	if (!t2->is_derived_from(t1))
		do_error(format("'is': class '%s' is not derived from '%s'", t2->long_name(), t1->long_name()));

	// vtable2
	Node *vtable2 = tree->add_node_const(tree->add_constant_pointer(TypePointer, t2->_vtable_location_compiler_));

	// vtable1
	param1->type = TypePointer;

	return tree->add_node_operator_by_inline(param1, vtable2, InlineID::POINTER_EQUAL);
}

Node *Parser::link_special_operator_in(Node *param1, Node *param2) {
	param2 = force_concrete_type(param2);
	auto *f = param2->type->get_func("__contains__", TypeBool, {param1->type});
	if (!f)
		do_error(format("no 'bool %s.__contains__(%s)' found", param2->type->long_name(), param1->type->long_name()));

	Node *n = tree->add_node_member_call(f, param2);
	n->set_param(1, param1);
	return n;
}

Node *Parser::link_operator_id(OperatorID op_no, Node *param1, Node *param2) {
	return link_operator(&PrimitiveOperators[(int)op_no], param1, param2);
}

Node *Parser::link_operator(PrimitiveOperator *primop, Node *param1, Node *param2) {
	bool left_modifiable = primop->left_modifiable;
	bool order_inverted = primop->order_inverted;
	string op_func_name = primop->function_name;
	Node *op = nullptr;

	if (primop->left_modifiable and param1->is_const)
		do_error("trying to modify a constant expression");

	if (primop->id == OperatorID::IS)
		return link_special_operator_is(param1, param2);
	if (primop->id == OperatorID::IN)
		return link_special_operator_in(param1, param2);

	auto *p1 = param1->type;
	auto *p2 = param2->type;

	const Class *pp1 = p1;
	if (pp1->is_pointer())
		pp1 = p1->param;

	if (primop->id == OperatorID::ASSIGN) {
		//param1->show();
		if (param1->kind == NodeKind::FUNCTION_CALL) {
			auto f = param1->as_func();
			if (f->name == "__get__") {
				auto inst = param1->params[0];
				auto index = param1->params[1];
				//msg_write(format("[]=...    void %s.__set__(%s, %s)?", inst->type->long_name(), index->type->long_name(), p2->long_name()));
				for (auto *ff: inst->type->functions)
					if (ff->name == "__set__" and ff->return_type == TypeVoid and ff->num_params == 2) {
						if (ff->literal_param_type[0] != index->type)
							continue;
						int pen, cast;
						if (!type_match_with_cast(param2, false, ff->literal_param_type[1], pen, cast))
							continue;
						//msg_write(ff->signature());
						auto nn = tree->add_node_member_call(ff, inst);
						nn->set_param(1, index);
						nn->set_param(2, apply_type_cast(cast, param2, ff->literal_param_type[1]));
						return nn;
					}
			}
		}
	}

	// exact match as class function?
	for (Function *f: pp1->functions)
		if ((f->name == op_func_name) and !f->is_static()) {
			// exact match as class function but missing a "&"?
			auto type1 = f->literal_param_type[0];
			if (type1->is_pointer_silent()) {
				if (type_match(p2, type1->param)) {
					Node *inst = param1;
					if (p1 == pp1)
						op = tree->add_node_member_call(f, inst);
					else
						op = tree->add_node_member_call(f, tree->deref_node(inst));
					op->set_param(1, tree->ref_node(param2));
					return op;
				}
			} else if (type_match(p2, type1)) {
				Node *inst = param1;
				if (p1 == pp1)
					op = tree->add_node_member_call(f, inst);
				else
					op = tree->add_node_member_call(f, tree->deref_node(inst));
				op->set_param(1, param2);
				return op;
			}
		}

	// exact (operator) match?
	for (auto *op: tree->operators)
		if (primop == op->primitive)
			if (type_match(p1, op->param_type_1) and type_match(p2, op->param_type_2)) {
				return tree->add_node_operator(param1, param2, op);
			}


	// needs type casting?
	int pen1 = 0, pen2 = 0;
	int c1 = TYPE_CAST_NONE, c2 = TYPE_CAST_NONE;
	int c1_best = TYPE_CAST_NONE, c2_best = TYPE_CAST_NONE;
	const Class *t1_best = nullptr, *t2_best = nullptr;
	int pen_min = 2000;
	Operator *op_found = nullptr;
	Function *op_cf_found = nullptr;
	for (auto *op: tree->operators)
		if (primop == op->primitive)
			if (type_match_with_cast(param1, left_modifiable, op->param_type_1, pen1, c1) and type_match_with_cast(param2, false, op->param_type_2, pen2, c2))
				if (pen1 + pen2 < pen_min) {
					op_found = op;
					pen_min = pen1 + pen2;
					c1_best = c1;
					c2_best = c2;
					t1_best = op->param_type_1;
					t2_best = op->param_type_2;
				}
	for (auto *cf: p1->functions)
		if (cf->name == op_func_name)
			if (type_match_with_cast(param2, false, cf->literal_param_type[0], pen2, c2))
				if (pen2 < pen_min) {
					op_cf_found = cf;
					pen_min = pen2;
					c1_best = -1;
					c2_best = c2;
					t2_best = cf->literal_param_type[0];
				}
	// cast
	if (op_found or op_cf_found) {
		param1 = apply_type_cast(c1_best, param1, t1_best);
		param2 = apply_type_cast(c2_best, param2, t2_best);
		if (op_cf_found) {
			op = tree->add_node_member_call(op_cf_found, param1);
			op->set_param(1, param2);
		} else {
			return tree->add_node_operator(param1, param2, op_found);
		}
		return op;
	}

	return nullptr;
}

void get_comma_range(Array<Node*> &_operators, int mio, int &first, int &last) {
	first = mio;
	last = mio+1;
	for (int i=mio; i>=0; i--) {
		if (_operators[i]->as_prim_op()->id == OperatorID::COMMA)
			first = i;
		else
			break;
	}
	for (int i=mio; i<_operators.num; i++) {
		if (_operators[i]->as_prim_op()->id == OperatorID::COMMA)
			last = i+1;
		else
			break;
	}
}

void Parser::link_most_important_operator(Array<Node*> &operands, Array<Node*> &_operators, Array<int> &op_exp) {
	//force_concrete_types(operands);

// find the most important operator (mio)
	int mio = 0;
	for (int i=0;i<_operators.num;i++) {
		if (_operators[i]->as_prim_op()->level > _operators[mio]->as_prim_op()->level)
			mio = i;
	}

// link it
	Node *param1 = operands[mio];
	Node *param2 = operands[mio + 1];
	auto op_no = _operators[mio]->as_prim_op();

	if (op_no->id == OperatorID::COMMA) {
		int first = mio, last = mio;
		get_comma_range(_operators, mio, first, last);
		auto n = build_abstract_list(operands.sub(first, last - first + 1));
		operands[first] = n;
		for (int i=last-1; i>=first; i--) {
			_operators.erase(i);
			op_exp.erase(i);
			operands.erase(i + 1);
		}
		return;
	}

	_operators[mio] = link_operator(op_no, param1, param2);
	if (!_operators[mio])
		do_error(format("no operator found: '%s %s %s'", param1->type->long_name(), op_no->name, param2->type->long_name()), op_exp[mio]);

// remove from list
	operands[mio] = _operators[mio];
	_operators.erase(mio);
	op_exp.erase(mio);
	operands.erase(mio + 1);
}

// greedily parse AxBxC...(operand, operator)
Node *Parser::parse_operand_greedy(Block *block, bool allow_tuples, Node *first_operand) {
	Array<Node*> operands;
	Array<Node*> operators;
	Array<int> op_exp;

	// find the first operand
	if (!first_operand)
		first_operand = parse_operand(block);
	operands.add(first_operand);

	// find pairs of operators and operands
	for (int i=0;true;i++) {
		op_exp.add(Exp.cur_exp);
		if (!allow_tuples and Exp.cur == ",")
			break;
		Node *op = parse_primitive_operator(block);
		if (!op)
			break;
		operators.add(op);
		if (Exp.end_of_line()) {
			//Exp.rewind();
			do_error("unexpected end of line after operator");
		}
		operands.add(parse_operand(block));
	}


	// in each step remove/link the most important operator
	while (operators.num > 0)
		link_most_important_operator(operands, operators, op_exp);

	// complete command is now collected in operand[0]
	return operands[0];
}

// greedily parse AxBxC...(operand, operator)
Node *Parser::parse_operand_super_greedy(Block *block) {
	return parse_operand_greedy(block, true);
}

// TODO later...
//  J make Node=Block
//  J for with p[0]=set init
//  * for_var in for "Block"

// Node structure
//  p = [VAR, START, STOP, STEP, BLOCK]
Node *Parser::parse_for_header(Block *block) {

	// variable name
	Exp.next(); // for
	string var_name = Exp.cur;
	Exp.next();

	// index
	string index_name = format("-for_index_%d-", for_index_count ++);
	if (Exp.cur == ",") {
		Exp.next();
		index_name = Exp.cur;
		Exp.next();
	}


	if (Exp.cur != "in")
		do_error("'in' expected after variable in 'for ...'");
	Exp.next();

	// first value/array
	Node *val0 = parse_operand_super_greedy(block);
	val0 = force_concrete_type(val0);
	val0 = deref_if_pointer(val0);


	if (Exp.cur == ":") {
		// range

		Exp.next(); // :
		Node *val1 = parse_operand_greedy(block);

		Node *val_step = nullptr;
		if (Exp.cur == ":") {
			Exp.next();
			val_step = parse_operand_greedy(block);
		}

		// type?
		const Class *t = val0->type;
		if (val1->type == TypeFloat32)
			t = val1->type;
		if (val_step)
			if (val_step->type == TypeFloat32)
				t = val_step->type;
		val0 = check_param_link(val0, t, "for", 1);
		val1 = check_param_link(val1, t, "for", 1);
		if (val_step)
			val_step = check_param_link(val_step, t, "for", 1);


		if (!val_step) {
			if (val0->type == TypeInt) {
				val_step = tree->add_node_const(tree->add_constant_int(1));
			} else {
				val_step = tree->add_node_const(tree->add_constant(TypeFloat32));
				val_step->as_const()->as_float() = 1.0f;
			}
		}

		// variable
		auto *var_no = block->add_var(var_name, t);

		Node *cmd_for = tree->add_node_statement(StatementID::FOR_RANGE);
		cmd_for->set_param(0, tree->add_node_local(var_no));
		cmd_for->set_param(1, val0);
		cmd_for->set_param(2, val1);
		cmd_for->set_param(3, val_step);
		//cmd_for->set_uparam(4, loop_block);

		return cmd_for;

	} else {
		// array

		Node *for_array = val0;//parse_operand(block);
		if ((!for_array->type->usable_as_super_array()) and (!for_array->type->is_array()))
			do_error("array or list expected as second parameter in 'for . in .'");
		//Exp.next();


		// variable...
		const Class *var_type = for_array->type->get_array_element();
		auto *var = block->add_var(var_name, var_type);
		var->is_const = for_array->is_const;

		// for index
		auto *index = block->add_var(index_name, TypeInt);


		Node *cmd_for = tree->add_node_statement(StatementID::FOR_ARRAY);
		// [VAR, INDEX, ARRAY, BLOCK]

		cmd_for->set_param(0, tree->add_node_local(var));
		cmd_for->set_param(1, tree->add_node_local(index));
		cmd_for->set_param(2, for_array);
		//cmd_for->set_uparam(3, loop_block);

		return cmd_for;
	}
}

void Parser::post_process_for(Node *cmd_for) {
	auto *n_var = cmd_for->params[0];
	auto *var = n_var->as_local();

	if (cmd_for->as_statement()->id == StatementID::FOR_ARRAY) {
		auto *loop_block = cmd_for->params[3];

	// ref.
		var->type = var->type->get_pointer();
		n_var->type = var->type;
		tree->transform_node(loop_block, [&](Node *n) { return tree->conv_cbr(n, var); });
	}

	// force for_var out of scope...
	var->name = ":" + var->name;
	if (cmd_for->as_statement()->id == StatementID::FOR_ARRAY) {
		auto *index = cmd_for->params[1]->as_local();
		index->name = ":" + index->name;
	}
}



// Node structure
Node *Parser::parse_statement_for(Block *block) {

	auto *cmd_for = parse_for_header(block);

	expect_new_line();
	// ...block
	Exp.next_line();
	expect_indent();
	parser_loop_depth ++;
	auto *loop_block = parse_block(block);
	parser_loop_depth --;

	cmd_for->set_param(cmd_for->params.num - 1, loop_block);

	post_process_for(cmd_for);

	return cmd_for;
}

// Node structure
//  p[0]: test
//  p[1]: loop block
Node *Parser::parse_statement_while(Block *block) {
	Exp.next();
	Node *cmd_cmp = check_param_link(parse_operand_greedy(block), TypeBool, "while", 0);
	expect_new_line();

	Node *cmd_while = tree->add_node_statement(StatementID::WHILE);
	cmd_while->set_param(0, cmd_cmp);

	// ...block
	Exp.next_line();
	expect_indent();
	parser_loop_depth ++;
	cmd_while->set_num_params(2);
	cmd_while->set_param(1, parse_block(block));
	parser_loop_depth --;
	return cmd_while;
}

Node *Parser::parse_statement_break(Block *block) {
	if (parser_loop_depth == 0)
		do_error("'break' only allowed inside a loop");
	Exp.next();
	return tree->add_node_statement(StatementID::BREAK);
}

Node *Parser::parse_statement_continue(Block *block) {
	if (parser_loop_depth == 0)
		do_error("'continue' only allowed inside a loop");
	Exp.next();
	return tree->add_node_statement(StatementID::CONTINUE);
}

// Node structure
//  p[0]: value (if not void)
Node *Parser::parse_statement_return(Block *block) {
	Exp.next();
	Node *cmd = tree->add_node_statement(StatementID::RETURN);
	if (block->function->return_type == TypeVoid) {
		cmd->set_num_params(0);
	} else {
		Node *cmd_value = check_param_link(parse_operand_super_greedy(block), block->function->return_type, IDENTIFIER_RETURN, 0);
		cmd->set_num_params(1);
		cmd->set_param(0, cmd_value);
	}
	expect_new_line();
	return cmd;
}

// IGNORE!!! raise() is a function :P
Node *Parser::parse_statement_raise(Block *block) {
	throw "jhhhh";
#if 0
	Exp.next();
	Node *cmd = add_node_statement(StatementID::RAISE);

	Node *cmd_ex = check_param_link(parse_operand_greedy(block), TypeExceptionP, IDENTIFIER_RAISE, 0);
	cmd->set_num_params(1);
	cmd->set_param(0, cmd_ex);

	/*if (block->function->return_type == TypeVoid) {
		cmd->set_num_params(0);
	} else {
		Node *cmd_value = CheckParamLink(GetCommand(block), block->function->return_type, IDENTIFIER_RETURN, 0);
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
Node *Parser::parse_statement_try(Block *block) {
	int ind = Exp.cur_line->indent;
	Exp.next();
	Node *cmd_try = tree->add_node_statement(StatementID::TRY);
	cmd_try->set_num_params(3);
	expect_new_line();
	// ...block
	Exp.next_line();
	expect_indent();
	cmd_try->set_param(0, parse_block(block));
	Exp.next_line();

	if (Exp.cur != IDENTIFIER_EXCEPT)
		do_error("except after try expected");
	if (Exp.cur_line->indent != ind)
		do_error("wrong indentation for except");
	Exp.next();

	Node *cmd_ex = tree->add_node_statement(StatementID::EXCEPT);
	cmd_try->set_param(1, cmd_ex);

	Block *except_block = new Block(block->function, block);

	if (!Exp.end_of_line()) {
		auto *ex_type = parse_type(block->name_space());
		if (!ex_type)
			do_error("Exception class expected");
		if (!ex_type->is_derived_from(TypeException))
			do_error("Exception class expected");
		cmd_ex->type = ex_type;
		ex_type = ex_type->get_pointer();
		if (!Exp.end_of_line()) {
			if (Exp.cur != "as")
				do_error("'as' expected");
			Exp.next();
			string ex_name = Exp.cur;
			auto *v = except_block->add_var(ex_name, ex_type);
			cmd_ex->params.add(tree->add_node_local(v));
			Exp.next();
		}
	}

	//int last_indent = Exp.indent_0;

	expect_new_line();
	// ...block
	Exp.next_line();
	expect_indent();
	//ParseCompleteCommand(block);
	//Exp.next_line();

	//auto n = block->nodes.back();
	//n->as_block()->

	cmd_try->set_param(2, parse_block(block, except_block));

	//Node *cmd_ex_block = add_node_block(new_block);
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
Node *Parser::parse_statement_if(Block *block) {
	int ind = Exp.cur_line->indent;
	Exp.next();
	Node *cmd_cmp = check_param_link(parse_operand_greedy(block), TypeBool, IDENTIFIER_IF, 0);
	expect_new_line();

	Node *cmd_if = tree->add_node_statement(StatementID::IF);
	cmd_if->set_param(0, cmd_cmp);
	// ...block
	Exp.next_line();
	expect_indent();
	//block->nodes.add(ParseBlock(block));
	cmd_if->set_param(1, parse_block(block));
	Exp.next_line();

	// else?
	if ((!Exp.end_of_file()) and (Exp.cur == IDENTIFIER_ELSE) and (Exp.cur_line->indent >= ind)) {
		cmd_if->link_no = (int64)statement_from_id(StatementID::IF_ELSE);
		cmd_if->set_num_params(3);
		Exp.next();
		// iterative if
		if (Exp.cur == IDENTIFIER_IF) {
			//DoError("else if...");
			// sub-if's in a new block
			Block *cmd_block = new Block(block->function, block);
			cmd_if->set_param(2, cmd_block);
			// parse the next if
			parse_complete_command(cmd_block);
			return cmd_if;
		}
		expect_new_line();
		// ...block
		Exp.next_line();
		expect_indent();
		cmd_if->set_param(2, parse_block(block));
		//Exp.next_line();
	} else {
		int line = Exp.get_line_no() - 1;
		Exp.set(Exp.line[line].exp.num - 1, line);
	}
	return cmd_if;
}

Node *Parser::parse_statement_pass(Block *block) {
	Exp.next(); // pass
	expect_new_line();

	return tree->add_node_statement(StatementID::PASS);
}

// Node structure
//  type: class
//  p[0]: call to constructor (optional)
Node *Parser::parse_statement_new(Block *block) {
	Exp.next(); // new
	const Class *t = parse_type(block->name_space());
	Node *cmd = tree->add_node_statement(StatementID::NEW);
	cmd->type = t->get_pointer();
	if (Exp.cur != "(")
		do_error("'(' expected after 'new Type'");
	/*cmd->set_num_uparams(1);
	cmd->set_uparam(0, parse_operand_extension_call(, block, false));
	cmd->uparams[0]->set_instance(new Node(NodeKind::PLACEHOLDER, 0, TypeVoid));*/

	Array<Function*> cfs = t->get_constructors();
	Array<Node*> funcs;
	if (cfs.num == 0)
		do_error(format("class '%s' does not have a constructor", t->long_name()));
	for (auto *cf: cfs) {
		funcs.add(tree->add_node_func_name(cf));
		funcs.back()->params.add(new Node(NodeKind::PLACEHOLDER, 0, TypeVoid));
	}
	cmd->set_num_params(1);
	cmd->set_param(0, parse_operand_extension_call(funcs, block));
	return cmd;
}

// Node structure
//  p[0]: operand
Node *Parser::parse_statement_delete(Block *block) {
	Exp.next(); // del
	auto p = parse_operand(block);
	if (!p->type->is_pointer())
		do_error("pointer expected after 'del'");

	// override del operator?
	auto f = p->type->param->get_func("__del_override__", TypeVoid, {});
	if (f) {
		auto cmd = tree->add_node_call(f);
		cmd->set_instance(tree->deref_node(p));
		return cmd;
	}

	// default delete
	Node *cmd = tree->add_node_statement(StatementID::DELETE);
	cmd->set_param(0, p);
	return cmd;
}

Node *Parser::parse_single_func_param(Block *block) {
	string func_name = Exp.cur_line->exp[Exp.cur_exp-1].name;
	if (Exp.cur != "(")
		do_error(format("'(' expected after '%s'", func_name));
	Exp.next(); // "("
	Node *n = parse_operand_greedy(block);
	if (Exp.cur != ")")
		do_error(format("')' expected after parameter of '%s'", func_name));
	Exp.next(); // ")"
	return n;
}

Node *Parser::parse_statement_sizeof(Block *block) {
	Exp.next(); // sizeof
	Node* sub = parse_single_func_param(block);
	sub = force_concrete_type(sub);
	Node *c;

	if (sub->kind == NodeKind::CLASS) {
		c = tree->add_node_const(tree->add_constant_int(sub->as_class()->size));
	} else {
		c = tree->add_node_const(tree->add_constant_int(sub->type->size));
	}
	delete sub;
	return c;

}

Node *Parser::parse_statement_type(Block *block) {
	Exp.next(); // type
	Node* sub = parse_single_func_param(block);
	sub = force_concrete_type(sub);

	Node *c = tree->add_node_const(tree->add_constant(TypeClassP));

	if (sub->kind == NodeKind::CLASS) {
		c->as_const()->as_int64() = (int_p)sub->as_class();
	} else {
		c->as_const()->as_int64() = (int_p)sub->type;
	}
	delete sub;
	return c;
}

Node *Parser::parse_statement_len(Block *block) {
	Exp.next(); // len
	Node *sub = parse_single_func_param(block);
	sub = force_concrete_type(sub);
	sub = deref_if_pointer(sub);

	// array?
	if (sub->type->is_array())
		return tree->add_node_const(tree->add_constant_int(sub->type->array_length));

	// element "int num/length"?
	for (auto &e: sub->type->elements)
		if (e.type == TypeInt and (e.name == "length" or e.name == "num")) {
			return tree->shift_node(sub, false, e.offset, e.type);
		}
		
	// __length__() function?
	auto *f = sub->type->get_func(IDENTIFIER_FUNC_LENGTH, TypeInt, {});
	if (f)
		return tree->add_node_member_call(f, sub);


	do_error(format("don't know how to get the length of an object of class '%s'", sub->type->long_name()));
	return nullptr;
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

void Parser::force_concrete_types(Array<Node*> &nodes) {
	for (int i=0; i<nodes.num; i++)
		nodes[i] = force_concrete_type(nodes[i]);
}

Node *Parser::force_concrete_type(Node *node) {
	if (node->type != TypeAbstractList and node->type != TypeAbstractDict)
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
			do_error("inhomogeneous abstract array");

		for (int i=0; i<node->params.num; i++) {
			int pen, tc;
			type_match_with_cast(node->params[i], false, t, pen, tc);
			node->params[i] = apply_type_cast(tc, node->params[i], t);
		}

		node->type = tree->make_class_super_array(t);
		return node;
	}
	if (node->kind == NodeKind::DICT_BUILDER) {
		if (node->params.num == 0) {
			node->type = TypeIntDict;
			return node;
		}

		force_concrete_types(node->params);

		auto t = node->params[1]->type;
		for (int i=3; i<node->params.num; i+=2)
			t = type_more_abstract(t, node->params[i]->type);
		if (!t)
			do_error("inhomogeneous abstract dict");

		for (int i=1; i<node->params.num; i+=2) {
			int pen, tc;
			type_match_with_cast(node->params[i], false, t, pen, tc);
			node->params[i] = apply_type_cast(tc, node->params[i], t);
		}

		node->type = tree->make_class_dict(t);
		return node;
	}
	do_error("unhandled abstract type...");
	return node;
}

Node *Parser::deref_if_pointer(Node *node) {
	if (node->type->is_pointer())
		return tree->deref_node(node);
	return node;
}


Node *Parser::add_converter_str(Node *sub, bool repr) {
	sub = force_concrete_type(sub);
	// evil shortcut for pointers (carefull with nil!!)
	if (!repr)
		sub = deref_if_pointer(sub);
	
	auto *t = sub->type;

	Function *cf = nullptr;	
	if (repr)
		cf = t->get_func(IDENTIFIER_FUNC_REPR, TypeString, {});
	if (!cf)
		cf = t->get_func(IDENTIFIER_FUNC_STR, TypeString, {});
	if (cf)
		return tree->add_node_member_call(cf, sub);

	// "universal" var2str() or var_repr()
	auto *c = tree->add_constant_pointer(TypeClassP, sub->type);

	Array<Node*> links = tree->get_existence(repr ? "@var_repr" : "@var2str", nullptr, nullptr, false);
	Function *f = links[0]->as_func();

	Node *cmd = tree->add_node_call(f);
	cmd->set_param(0, tree->ref_node(sub));
	cmd->set_param(1, tree->add_node_const(c));
	return cmd;
}

Node *Parser::parse_statement_str(Block *block) {
	Exp.next(); // str
	Node *sub = parse_single_func_param(block);
	
	return add_converter_str(sub, false);
}

Node *Parser::parse_statement_repr(Block *block) {
	Exp.next(); // repr
	Node *sub = parse_single_func_param(block);

	return add_converter_str(sub, true);
}

// local (variable) definitions...
Node *Parser::parse_statement_let(Block *block) {
	Exp.next(); // "let"
	string name = Exp.cur;
	Exp.next();

	if (Exp.cur != "=")
		do_error("'=' required after 'let' declaration");
	Exp.next();

	auto* rhs = parse_operand_super_greedy(block);
	rhs = force_concrete_type(rhs);
	auto *var = block->add_var(name, rhs->type);
	auto cmd = link_operator_id(OperatorID::ASSIGN, tree->add_node_local(var), rhs);
	if (!cmd)
		do_error("let: no assignment operator for type " + rhs->type->long_name());
	return cmd;
}

Array<const Class*> func_effective_params(const Function *f) {
	auto p = f->literal_param_type;
	if (!f->is_static())
		p.insert(f->name_space, 0);
	return p;
}

Node *Parser::parse_statement_map(Block *block) {
	Exp.next(); // "map"
	string name = Exp.cur;

	auto params = parse_call_parameters(block);
	if (params.num != 2)
		do_error("map() expects 2 parameters");
	if (params[0]->kind != NodeKind::FUNCTION)
		do_error("map(): first parameter must be a function name");
	params[1] = force_concrete_type(params[1]);
	if (!params[1]->type->is_super_array())
		do_error("map(): second parameter must be a list[]");

	auto p = func_effective_params(params[0]->as_func());
	if (p.num != 1)
		do_error("map(): function must have exactly one parameter");
	if (p[0] != params[1]->type->param)
		do_error("map(): function parameter does not match list type");

	auto links = tree->get_existence("@map", nullptr, nullptr, false);
	Function *f = links[0]->as_func();

	auto *c = tree->add_constant_pointer(TypeFunctionP, params[0]->as_func());

	Node *cmd = tree->add_node_call(f);
	cmd->set_param(0, tree->add_node_const(c));
	cmd->set_param(1, params[1]);
	cmd->type = tree->make_class_super_array(params[0]->as_func()->literal_return_type);
	return cmd;
}

Node *Parser::parse_statement_lambda(Block *block) {
	Exp.next(); // "lambda"
	auto *prev_func = cur_func;

	auto *f = tree->add_function("-lambda-", TypeUnknown, tree->base_class, Flags::STATIC);
	f->_logical_line_no = Exp.get_line_no();
	f->_exp_no = Exp.cur_exp;

	cur_func = f;

	Exp.next(); // '('

	// parameter list
	if (Exp.cur != ")")
		for (int k=0;;k++) {
			// like variable definitions

			bool rw = false;
			if (Exp.cur == IDENTIFIER_OUT) {
				rw = true;
				Exp.next();
			}

			// type of parameter variable
			const Class *param_type = parse_type(tree->base_class); // force
			auto v = f->block->add_var(Exp.cur, param_type);
			v->is_const = !rw;
			f->literal_param_type.add(param_type);
			Exp.next();
			f->num_params ++;

			if (Exp.cur == ")")
				break;

			if (Exp.cur != ",")
				do_error("',' or ')' expected after parameter");
			Exp.next(); // ','
		}
	Exp.next(); // ')'

	cur_func = prev_func;

	auto *cmd = parse_operand_greedy(f->block);
	f->return_type = cmd->type;
	f->literal_return_type = cmd->type;

	f->update_parameters_after_parsing();

	if (cmd->type == TypeVoid) {
		f->block->add(cmd);
	} else {
		auto *ret = tree->add_node_statement(StatementID::RETURN);
		ret->set_num_params(1);
		ret->params[0] = cmd;
		f->block->add(ret);
	}

	tree->base_class->add_function(tree, f, false, false);

	return tree->add_node_func_name(f);
}

Node *Parser::parse_statement_sorted(Block *block) {
	Exp.next(); // "sorted"
	string name = Exp.cur;

	auto params = parse_call_parameters(block);
	if (params.num != 2)
		do_error("sorted() expects 2 parameters");
	params[0] = force_concrete_type(params[0]);
	if (!params[0]->type->is_super_array())
		do_error("sorted(): first parameter must be a list[]");
	if (params[1]->type != TypeString)
		do_error("sorted(): second parameter must be a string");

	auto links = tree->get_existence("@sorted", nullptr, nullptr, false);
	Function *f = links[0]->as_func();

	Node *cmd = tree->add_node_call(f);
	cmd->set_param(0, params[0]);
	cmd->set_param(1, tree->add_node_class(params[0]->type));
	cmd->set_param(2, params[1]);
	cmd->type = params[0]->type;
	return cmd;
}

Node *Parser::make_dynamical(Node *node) {
	if (node->kind == NodeKind::ARRAY_BUILDER and node->type == TypeAbstractList) {
		for (int i=0; i<node->params.num; i++)
			node->params[i] = make_dynamical(node->params[i]);
		// TODO create...
		node->type = TypeAnyList;
		//return node;
	} else  if (node->kind == NodeKind::DICT_BUILDER and node->type == TypeAbstractDict) {
		for (int i=1; i<node->params.num; i+=2)
			node->params[i] = make_dynamical(node->params[i]);
		// TODO create...
		node->type = TypeAnyDict;
		//return node;
	}
	//node = force_concrete_type(tree, node);

	auto *c = tree->add_constant_pointer(TypeClassP, node->type);

	auto links = tree->get_existence("@dyn", nullptr, nullptr, false);
	Function *f = links[0]->as_func();

	Node *cmd = tree->add_node_call(f);
	cmd->set_param(0, tree->ref_node(node));
	cmd->set_param(1, tree->add_node_const(c));
	return cmd;
}

Node *Parser::parse_statement_dyn(Block *block) {
	Exp.next(); // dyn
	Node *sub = parse_single_func_param(block);
	//sub = force_concrete_type(sub); // TODO

	return make_dynamical(sub);
}

Node *Parser::parse_statement_call(Block *block) {
	Exp.next(); // "call"
	string name = Exp.cur;

	auto params = parse_call_parameters(block);
	if (params.num == 0)
		do_error("call() expects at least 1 parameter");
	if (params[0]->type != TypeFunctionP)
		do_error("call(): first parameter must be a function pointer ..." + params[0]->type->long_name());

	int np = params.num-1;
	for (int i=0; i<np; i++)
		params[i+1] = force_concrete_type(params[i+1]);

	auto links = tree->get_existence("@call" + i2s(np), nullptr, nullptr, false);
	Function *f = links[0]->as_func();

	Node *cmd = tree->add_node_call(f);
	cmd->set_param(0, params[0]);
	for (int i=0; i<np; i++)
		cmd->set_param(i+1, tree->ref_node(params[i+1]));
	return cmd;
}

Node *Parser::parse_statement(Block *block) {
	if (Exp.cur == IDENTIFIER_FOR) {
		return parse_statement_for(block);
	} else if (Exp.cur == IDENTIFIER_WHILE) {
		return parse_statement_while(block);
 	} else if (Exp.cur == IDENTIFIER_BREAK) {
 		return parse_statement_break(block);
	} else if (Exp.cur == IDENTIFIER_CONTINUE) {
		return parse_statement_continue(block);
	} else if (Exp.cur == IDENTIFIER_RETURN) {
		return parse_statement_return(block);
	//} else if (Exp.cur == IDENTIFIER_RAISE) {
	//	ParseStatementRaise(block);
	} else if (Exp.cur == IDENTIFIER_TRY) {
		return parse_statement_try(block);
	} else if (Exp.cur == IDENTIFIER_IF) {
		return parse_statement_if(block);
	} else if (Exp.cur == IDENTIFIER_PASS) {
		return parse_statement_pass(block);
	} else if (Exp.cur == IDENTIFIER_NEW) {
		return parse_statement_new(block);
	} else if (Exp.cur == IDENTIFIER_DELETE or Exp.cur == "delete") {
		return parse_statement_delete(block);
	} else if (Exp.cur == IDENTIFIER_SIZEOF) {
		return parse_statement_sizeof(block);
	} else if (Exp.cur == IDENTIFIER_TYPE) {
		return parse_statement_type(block);
	} else if (Exp.cur == IDENTIFIER_STR) {
		return parse_statement_str(block);
	} else if (Exp.cur == IDENTIFIER_REPR) {
		return parse_statement_repr(block);
	} else if (Exp.cur == IDENTIFIER_LEN) {
		return parse_statement_len(block);
	} else if (Exp.cur == IDENTIFIER_LET) {
		return parse_statement_let(block);
	} else if (Exp.cur == IDENTIFIER_MAP) {
		return parse_statement_map(block);
	} else if (Exp.cur == IDENTIFIER_LAMBDA) {
		return parse_statement_lambda(block);
	} else if (Exp.cur == IDENTIFIER_SORTED) {
		return parse_statement_sorted(block);
	} else if (Exp.cur == IDENTIFIER_DYN) {
		return parse_statement_dyn(block);
	} else if (Exp.cur == IDENTIFIER_CALL) {
		return parse_statement_call(block);
	}
	do_error("unhandled statement..." + Exp.cur);
	return nullptr;
}

Node *Parser::parse_block(Block *parent, Block *block) {
	int last_indent = Exp.indent_0;

	Exp.indented = false;
	Exp.set(0); // bad hack...
	if (!block)
		block = new Block(parent->function, parent);

	for (int i=0;true;i++) {
		if (((i > 0) and (Exp.cur_line->indent < last_indent)) or (Exp.end_of_file()))
			break;


		parse_complete_command(block);
		Exp.next_line();
	}
	Exp.cur_line --;
	Exp.indent_0 = Exp.cur_line->indent;
	Exp.indented = false;
	Exp.cur_exp = Exp.cur_line->exp.num - 1;
	Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

	return block;
}

// local (variable) definitions...
void Parser::parse_local_definition(Block *block, const Class *type) {
	// type of variable
	if (!type)
		type = parse_type(block->name_space());

	if (type->needs_constructor() and !type->get_default_constructor())
		do_error(format("declaring a variable of type '%s' requires a constructor but no default constructor exists", type->long_name()));

	for (int l=0;!Exp.end_of_line();l++) {
		// name
		block->add_var(Exp.cur, type);
		Exp.next();

		// assignment?
		if (Exp.cur == "=") {
			Exp.rewind();
			// parse assignment
			block->add(parse_operand_super_greedy(block));
		}
		if (Exp.end_of_line())
			break;
		if ((Exp.cur != ",") and (!Exp.end_of_line()))
			do_error("',', '=' or newline expected after declaration of local variable");
		Exp.next();
	}
}

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void Parser::parse_complete_command(Block *block) {
	// cur_exp = 0!

	//bool is_type = tree->find_root_type_by_name(Exp.cur, block->name_space(), true);

	// block?  <- indent
	if (Exp.indented) {
		block->add(parse_block(block));

	// assembler block
	} else if (Exp.cur == "-asm-") {
		Exp.next();
		block->add(tree->add_node_statement(StatementID::ASM));

	} else {

		Node *first = parse_operand(block, true);

		if ((first->kind == NodeKind::CLASS) and !Exp.end_of_line()) {
			parse_local_definition(block, first->as_class());

		} else {

			// commands (the actual code!)
			block->add(parse_operand_greedy(block, true, first));
		}
	}

	expect_new_line();
}

extern Array<Script*> loading_script_stack;

string canonical_import_name(const string &s) {
	return s.lower().replace(" ", "").replace("_", "");
}

string dir_has(const Path &dir, const string &name) {
	auto list = dir_search(dir, "*", true);
	for (auto &e: list)
		if (canonical_import_name(e) == name)
			return e;
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


Path find_import(Script *s, const string &_name) {
	string name = _name.replace(".kaba", "");
	name = name.replace(".", "/") + ".kaba";

	if (name.head(2) == "@/")
		return (hui::Application::directory_static << "lib" << name.substr(2, -1)).canonical(); // TODO...

	for (int i=0; i<5; i++) {
		Path filename = import_dir_match((s->filename.parent() << str_repeat("../", i)).canonical(), name);
		if (!filename.is_empty())
			return filename;
	}

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
			do_error("'.' expected in import name");
		name += ".";
		expect_no_new_line();
		Exp.next();
		name += Exp.cur;
		Exp.next();
	}
	
	if (name.match("\"*\""))
		name = name.substr(1, name.num - 2); // remove ""
		
	
	// internal packages?
	for (Script *p: packages)
		if (p->filename.str() == name) {
			tree->add_include_data(p, indirect);
			return;
		}

	Path filename = find_import(tree->script, name);
	if (filename.is_empty())
		do_error(format("can not find import '%s'", name));

	for (Script *ss: loading_script_stack)
		if (ss->filename == filename)
			do_error("recursive include");

	msg_right();
	Script *include;
	try {
		include = Load(filename, tree->script->just_analyse or config.compile_os);
		// os-includes will be appended to syntax_tree... so don't compile yet
	} catch(Exception &e) {
		msg_left();

		int logical_line = Exp.get_line_no();
		int exp_no = Exp.cur_exp;
		int physical_line = Exp.line[logical_line].physical_line;
		int pos = Exp.line[logical_line].exp[exp_no].pos;
		string expr = Exp.line[logical_line].exp[exp_no].name;
		e.line = physical_line;
		e.column = pos;
		e.text += format("\n...imported from:\nline %d, %s", physical_line, tree->script->filename);
		throw e;
		//msg_write(e.message);
		//msg_write("...");
		string msg = e.message() + "\nimported file:";
		//string msg = "in imported file:\n\"" + e.message + "\"";
		do_error(msg);
	}
	cur_exp_buf = &Exp;

	msg_left();
	tree->add_include_data(include, indirect);
}


void Parser::parse_enum(Class *_namespace) {
	Exp.next(); // 'enum'
	expect_new_line();
	Exp.next_line();
	expect_indent();

	int value = 0;

	for (int i=0;!Exp.end_of_file();i++) {
		for (int j=0;!Exp.end_of_line();j++) {
			auto *c = tree->add_constant(TypeInt, _namespace);
			c->name = Exp.cur;
			Exp.next();

			// explicit value
			if (Exp.cur == "=") {
				Exp.next();
				expect_no_new_line();
				Value v;
				get_constant_value(Exp.cur, v);
				if (v.type == TypeInt)
					value = v.as_int();
				else
					do_error("integer constant expected after '=' for explicit value of enum");
				Exp.next();
			}
			c->as_int() = (value ++);

			if (Exp.end_of_line())
				break;
			if (Exp.cur != ",")
				do_error("',' or newline expected after enum definition");
			Exp.next();
			expect_no_new_line();
		}
		Exp.next_line();
		if (Exp.unindented)
			break;
	}
	Exp.cur_line --;
}

inline bool type_needs_alignment(const Class *t) {
	if (t->is_array())
		return type_needs_alignment(t->param);
	return (t->size >= 4);
}

bool Parser::parse_class(Class *_namespace) {
	int indent0 = Exp.cur_line->indent;
	int _offset = 0;
	Exp.next(); // 'class'
	string name = Exp.cur;
	Exp.next();

	// create class
	Class *_class = const_cast<Class*>(tree->find_root_type_by_name(name, _namespace, false));
	// already created...
	if (!_class)
		tree->script->do_error_internal("class declaration ...not found " + name);

	// parent class
	if (Exp.cur == IDENTIFIER_EXTENDS) {
		Exp.next();
		const Class *parent = parse_type(_namespace); // force
		if (!parent->fully_parsed)
			return false;
			//do_error(format("parent class '%s' not fully parsed yet", parent->long_name()));
		_class->derive_from(parent, true);
		_offset = parent->size;
	}
	expect_new_line();

	//msg_write("parse " + _class->long_name());

	Array<int> sub_class_line_offsets;

	// elements
	while (!Exp.end_of_file()) {
		Exp.next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
		if (Exp.end_of_file())
			break;

		Flags flags = parse_flags(Flags::NONE);

		int ie = Exp.cur_exp;

		if (Exp.cur == IDENTIFIER_ENUM) {
			parse_enum(_class);
			continue;
		}

		if (Exp.cur == IDENTIFIER_CLASS) {
			//msg_write("sub....");
			int cur_line = Exp.get_line_no();
			if (!parse_class(_class)) {
				sub_class_line_offsets.add(cur_line);
				skip_parse_class();
			}
			//msg_write(">>");
			continue;
		}

		const Class *type = parse_type(_class); // force
		while (!Exp.end_of_line()) {
			//int indent = Exp.cur_line->indent;
			
			string name = Exp.cur;
			Exp.next();

			// is a function?
			bool is_function = false;
			if (Exp.cur == "(")
			    is_function = true;
			if (is_function) {
				Exp.set(ie);
				parse_function_header(_class, flags);
				skip_parsing_function_body();
				break;
			}

			if (flags_has(flags, Flags::CONST)) {
				parse_named_const(name, type, _class, tree->root_of_all_evil->block);
				break;
			}

			// override?
			ClassElement *orig = nullptr;
			for (auto &e: _class->elements)
				if (e.name == name) //and e.type->is_pointer and el.type->is_pointer)
					orig = &e;
			bool override = flags_has(flags, Flags::OVERRIDE);
			if (override and ! orig)
				do_error(format("can not override element '%s', no previous definition", name));
			if (!override and orig)
				do_error(format("element '%s' is already defined, use '%s' to override", name, IDENTIFIER_OVERRIDE));
			if (override) {
				if (orig->type->is_pointer() and type->is_pointer())
					orig->type = type;
				else
					do_error("can only override pointer elements with other pointer type");
				continue;
			}

			// check parsing dependencies
			if (!type->is_size_known())
				do_error(format("size of type '%s' is not known at this point", type->long_name()));


			// add element
			if (flags_has(flags, Flags::STATIC)) {
				auto v = new Variable(name, type);
				v->is_extern = flags_has(flags, Flags::EXTERN);
				_class->static_variables.add(v);
			} else {
				if (type_needs_alignment(type))
					_offset = mem_align(_offset, 4);
				_offset = process_class_offset(_class->cname(tree->base_class), name, _offset);
				auto el = ClassElement(name, type, _offset);
				_offset += type->size;
				_class->elements.add(el);
			}
			if ((Exp.cur != ",") and !Exp.end_of_line())
				do_error("',' or newline expected after class element");
			if (Exp.end_of_line())
				break;
			Exp.next();
		}
	}



	// virtual functions?     (derived -> _class->num_virtual)
//	_class->vtable = cur_virtual_index;
	//foreach(ClassFunction &cf, _class->function)
	//	_class->num_virtual = max(_class->num_virtual, cf.virtual_index);
	if (_class->vtable.num > 0) {
		if (_class->parent) {
			if (_class->parent->vtable.num == 0)
				do_error("no virtual functions allowed when inheriting from class without virtual functions");
			// element "-vtable-" being derived
		} else {
			for (ClassElement &e: _class->elements)
				e.offset = process_class_offset(_class->cname(tree->base_class), e.name, e.offset + config.pointer_size);

			auto el = ClassElement(IDENTIFIER_VTABLE_VAR, TypePointer, 0);
			_class->elements.insert(el, 0);
			_offset += config.pointer_size;
		}
	}

	for (auto &e: _class->elements)
		if (type_needs_alignment(e.type))
			_offset = mem_align(_offset, 4);
	_class->size = process_class_size(_class->cname(tree->base_class), _offset);


	tree->add_missing_function_headers_for_class(_class);

	_class->fully_parsed = true;


	int cur_line = Exp.get_line_no();

	//msg_write(ia2s(sub_class_line_offsets));
	for (int l: sub_class_line_offsets) {
		//msg_write("SUB...");
		Exp.set(0, l);
		//.add(Exp.get_line_no());
		if (!parse_class(_class))
			do_error(format("parent class not fully parsed yet"));
			//do_error(format("parent class '%s' not fully parsed yet", parent->long_name()));
	}

	Exp.set(0, cur_line);
	Exp.cur_line --;
	return true;
}

void Parser::skip_parse_class() {
	int indent0 = Exp.cur_line->indent;

	// elements
	while (!Exp.end_of_file()) {
		Exp.next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
	}
	Exp.cur_line --;
}

void Parser::expect_no_new_line() {
	if (Exp.end_of_line())
		do_error("unexpected newline");
}

void Parser::expect_new_line() {
	if (!Exp.end_of_line())
		do_error("newline expected");
}

void Parser::expect_indent() {
	if (!Exp.indented)
		do_error("additional indent expected");
}

void Parser::parse_named_const(const string &name, const Class *type, Class *name_space, Block *block) {
	if (Exp.cur != "=")
		do_error("'=' expected after const name");
	Exp.next();

	// find const value
	Node *cv = parse_operand_super_greedy(block);
	cv = force_concrete_type(cv);
	cv = tree->transform_node(cv, [&](Node *n) { return tree->conv_eval_const_func(n); });

	if ((cv->kind != NodeKind::CONSTANT) or (cv->type != type))
		do_error(format("only constants of type '%s' allowed as value for this constant", type->long_name()));
	Constant *c_value = cv->as_const();

	auto *c = tree->add_constant(type, name_space);
	c->set(*c_value);
	c->name = name;
}

void Parser::parse_global_variable_def(bool single, Block *block, Flags flags) {
	const Class *type = parse_type(block->name_space()); // force

	for (int j=0;true;j++) {
		expect_no_new_line();

		// name
		string name = Exp.cur;
		Exp.next();

		if (flags_has(flags, Flags::CONST)) {
			parse_named_const(name, type, tree->base_class, block);
		} else {
			auto *v = new Variable(name, type);
			v->is_extern = flags_has(flags, Flags::EXTERN);
			tree->base_class->static_variables.add(v);
		}

		if ((Exp.cur != ",") and !Exp.end_of_line())
			do_error("',' or newline expected after definition of a global variable");

		// last one?
		if (Exp.end_of_line())
			break;

		Exp.next(); // ','
	}
}

bool peek_commands_super(ExpressionBuffer &Exp) {
	ExpressionBuffer::Line *l = Exp.cur_line + 1;
	if (l->exp.num < 3)
		return false;
	if ((l->exp[0].name == IDENTIFIER_SUPER) and (l->exp[1].name == ".") and (l->exp[2].name == IDENTIFIER_FUNC_INIT))
		return true;
	return false;
}

bool Parser::parse_function_command(Function *f, int indent0) {
	if (Exp.end_of_file())
		return false;

	Exp.next_line();
	Exp.indented = false;

	// end of file
	if (Exp.end_of_file())
		return false;

	// end of function
	if (Exp.cur_line->indent <= indent0)
		return false;

	// command or local definition
	parse_complete_command(f->block);
	return true;
}


// complicated types like "int[]*[4]" etc
// greedy
const Class *Parser::parse_type(const Class *ns) {
	// base type
	const Class *t = tree->find_root_type_by_name(Exp.cur, ns, true);
	if (!t)
		do_error("unknown type");
	Exp.next();

	// extensions *,[],{},.
	while (true) {

		// pointer?
		if (Exp.cur == "*") {
			Exp.next();
			t = t->get_pointer();
		} else if (Exp.cur == "[") {
			Exp.next();

			// no index -> super array
			if (Exp.cur == "]") {
				t = tree->make_class_super_array(t);
			} else {

				// find array index
				Node *c = tree->transform_node(parse_operand_greedy(tree->root_of_all_evil->block), [&](Node *n) { return tree->conv_eval_const_func(n); });

				if ((c->kind != NodeKind::CONSTANT) or (c->type != TypeInt))
					do_error("only constants of type 'int' allowed for size of arrays");
				int array_size = c->as_const()->as_int();
				//Exp.next();
				if (Exp.cur != "]")
					do_error("']' expected after array size");
				t = tree->make_class_array(t, array_size);
			}

			Exp.next();
		} else if (Exp.cur == "{") {
			Exp.next();

			if (Exp.cur != "}")
				do_error("'}' expected after dict{");

			Exp.next();

			t = tree->make_class_dict(t);
		} else if (Exp.cur == ".") {
			Exp.next();
			const Class *sub = nullptr;
			for (auto *c: t->classes)
				if (c->name == Exp.cur)
					sub = c;
			if (!sub)
				do_error(format("class '%s' does not have a sub-class '%s'", t->long_name(), Exp.cur));
			t = sub;
			Exp.next();
		} else {
			break;
		}
	}

	return t;
}

Function *Parser::parse_function_header(Class *name_space, Flags flags) {
	
// return type
	const Class *return_type = parse_type(name_space); // force...

	Function *f = tree->add_function(Exp.cur, return_type, name_space, flags);
	if (config.verbose)
		msg_write("PARSE HEAD  " + f->signature());
	f->_logical_line_no = Exp.get_line_no();
	f->_exp_no = Exp.cur_exp;
	cur_func = f;

	Exp.next();
	Exp.next(); // '('

// parameter list

	if (Exp.cur != ")")
		for (int k=0;;k++) {
			// like variable definitions

			bool rw = false;
			if (Exp.cur == IDENTIFIER_OUT) {
				rw = true;
				Exp.next();
			}

			// type of parameter variable
			const Class *param_type = parse_type(name_space); // force
			auto v = f->block->add_var(Exp.cur, param_type);
			v->is_const = !rw;
			f->literal_param_type.add(param_type);
			Exp.next();
			f->num_params ++;

			if (Exp.cur == ")")
				break;

			if (Exp.cur != ",")
				do_error("',' or ')' expected after parameter");
			Exp.next(); // ','
		}
	Exp.next(); // ')'

	if (!Exp.end_of_line())
		do_error("newline expected after parameter list");

	f->update_parameters_after_parsing();

	cur_func = nullptr;

	name_space->add_function(tree, f, flags_has(flags, Flags::VIRTUAL), flags_has(flags, Flags::OVERRIDE));

	return f;
}

void Parser::skip_parsing_function_body() {
	int indent0 = Exp.cur_line->indent;
	while (!Exp.end_of_file()) {
		if (Exp.cur_line[1].indent <= indent0)
			break;
		Exp.next_line();
	}
}

void Parser::parse_function_body(Function *f) {
	Exp.cur_line = &Exp.line[f->_logical_line_no];

	int indent0 = Exp.cur_line->indent;
	bool more_to_parse = true;

	// auto implement constructor?
	if (f->name == IDENTIFIER_FUNC_INIT) {
		if (peek_commands_super(Exp)) {
			more_to_parse = parse_function_command(f, indent0);

			auto_implement_constructor(f, f->name_space, false);
		} else {
			auto_implement_constructor(f, f->name_space, true);
		}
	}

	parser_loop_depth = 0;

// instructions
	while (more_to_parse) {
		more_to_parse = parse_function_command(f, indent0);
	}

	// auto implement destructor?
	if (f->name == IDENTIFIER_FUNC_DELETE)
		auto_implement_destructor(f, f->name_space);
	cur_func = nullptr;

	Exp.cur_line --;
}

void Parser::parse_all_class_names(Class *ns, int indent0) {
	if (indent0 == 0)
		Exp.reset_parser();
	while (!Exp.end_of_file()) {
		if ((Exp.cur_line->indent == indent0) and (Exp.cur_line->exp.num >= 2)) {
			if (Exp.cur == IDENTIFIER_CLASS) {
				Exp.next();
				Class *t = tree->create_new_class(Exp.cur, Class::Type::OTHER, 0, 0, nullptr, nullptr, ns);
				t->fully_parsed = false;

				Exp.next_line();
				parse_all_class_names(t, indent0 + 1);
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

void Parser::parse_all_function_bodies(const Class *name_space) {
	//for (auto *f: name_space->functions)   might add lambda functions...
	for (int i=0; i<name_space->functions.num; i++) {
		auto f = name_space->functions[i];
		if ((!f->is_extern()) and (f->_logical_line_no >= 0) and (f->name_space == name_space))
			parse_function_body(f);
	}

	// recursion
	//for (auto *c: name_space->classes)   NO... might encounter new classes creating new functions!
	for (int i=0; i<name_space->classes.num; i++)
		if (name_space->classes[i]->name_space == name_space)
			parse_all_function_bodies(name_space->classes[i]);
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
		} else if (Exp.cur == IDENTIFIER_OUT) {
			flags = flags_mix({flags, Flags::OUT});
		} else if (Exp.cur == "throws") {
			flags = flags_mix({flags, Flags::RAISES_EXCEPTIONS});
		} else if (Exp.cur == "pure") {
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

	parse_all_class_names(tree->base_class, 0);

	Exp.reset_parser();

	// global definitions (enum, class, variables and functions)
	while (!Exp.end_of_file()) {
		Flags flags = parse_flags(Flags::STATIC);


		/*if ((Exp.cur == "import") or (Exp.cur == "use")) {
			ParseImport();

		// enum
		} else*/ if (Exp.cur == IDENTIFIER_ENUM) {
			parse_enum(tree->base_class);

		// class
		} else if (Exp.cur == IDENTIFIER_CLASS) {
			parse_class(tree->base_class);

		} else {

			// type of definition
			bool is_function = false;
			for (int j=1;j<Exp.cur_line->exp.num-1;j++)
				if (Exp.cur_line->exp[j].name == "(")
				    is_function = true;

			// function?
			if (is_function) {
				parse_function_header(tree->base_class, flags);
				skip_parsing_function_body();

			// global variables/consts
			} else {
				parse_global_variable_def(false, tree->root_of_all_evil->block, flags);
			}
		}
		if (!Exp.end_of_file())
			Exp.next_line();
	}
}

// convert text into script data
void Parser::parse() {
	cur_exp_buf = &Exp;
	Exp.reset_parser();

	parse_top_level();

	parse_all_function_bodies(tree->base_class);
	
	tree->show("aaa");

	for (auto *f: tree->functions)
		test_node_recursion(f->block, tree->base_class, "a " + f->long_name());

	for (int i=0; i<tree->owned_classes.num; i++) // array might change...
		auto_implement_functions(tree->owned_classes[i]);

	for (auto *f: tree->functions)
		test_node_recursion(f->block, tree->base_class, "b " + f->long_name());
}

}
