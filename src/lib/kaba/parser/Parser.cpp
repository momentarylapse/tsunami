#include "../kaba.h"
#include "../asm/asm.h"
#include "../../os/msg.h"
#include "../../base/set.h"
#include "../../base/algo.h"
#include "../../base/iter.h"
#include "Parser.h"
#include "import.h"
#include "../template/template.h"


namespace kaba {

void test_node_recursion(shared<Node> root, const Class *ns, const string &message);

shared<Module> get_import_module(Parser *parser, const string &name, int token);

void add_enum_label(const Class *type, int value, const string &label);

shared<Node> build_abstract_tuple(const Array<shared<Node>> &el);

void crash() {
	int *p = nullptr;
	*p = 4;
}





#if 0
bool is_function_pointer(const Class *c) {
	if (c ==  common_types.functionP)
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
	AbstractParser(t),
	con(t->module->context, this, t),
	auto_implementer(this, t)
{
	context = t->module->context;
	tree = t;
	cur_func = nullptr;
	parser_loop_depth = 0;
	found_dynamic_param = false;
	f_parse_import = [this] {
		parse_import();
	};
	f_add_asm_block = [this] {
		auto a = add_node_statement(StatementID::Asm);
		a->params.add(auto_implementer.const_int(tree->asm_blocks[next_asm_block ++].uuid));
		return a;
	};
}


void Parser::parse_buffer(const string &buffer, bool just_analyse) {
	Exp.analyse(tree, buffer);

	parse_legacy_macros(just_analyse);

	parse();

	if (config.verbose)
		tree->show("parse:a");

}

// find the type of a (potential) constant
//  "1.2" -> float
const Class *Parser::get_constant_type(const string &str) {
	// character '...'
	if ((str[0] == '\'') and (str.back() == '\''))
		return common_types.u8;

	// string "..."
	if ((str[0] == '"') and (str.back() == '"'))
		return tree->flag_string_const_as_cstring ? common_types.cstring : common_types.string;

	// numerical (int/float)
	const Class *type = common_types.i32;
	bool hex = (str.num > 1) and (str[0] == '0') and (str[1] == 'x');
	char last = 0;
	for (int ic=0;ic<str.num;ic++) {
		char c = str[ic];
		if ((c < '0') or (c > '9')) {
			if (hex) {
				if ((ic >= 2) and (c < 'a') and (c > 'f'))
					return common_types.unknown;
			} else if (c == '.') {
				type = common_types.f32;
			} else {
				if ((ic != 0) or (c != '-')) { // allow sign
					if ((c != 'e') and (c != 'E'))
						if (((c != '+') and (c != '-')) or ((last != 'e') and (last != 'E')))
							return common_types.unknown;
				}
			}
		}
		last = c;
	}
	if (type == common_types.i32) {
		if (hex) {
			if (str.num == 4)
				type = common_types.u8;
			if (str.num > 10)
				type = common_types.i64;
		} else {
			if ((s2i2(str) >= 0x80000000) or (-s2i2(str) > 0x80000000))
				type = common_types.i64;
		}
	}
	return type;
}

void Parser::get_constant_value(const string &str, Value &value) {
	value.init(get_constant_type(str));
// literal
	if (value.type == common_types.u8) {
		if (str[0] == '\'') // 'bla'
			value.as_int() = str.unescape()[1];
		else // 0x12
			value.as_int() = (int)s2i2(str);
	} else if (value.type == common_types.string) {
		value.as_string() = str.sub(1, -1).unescape();
	} else if (value.type == common_types.cstring) {
		strcpy((char*)value.p(), str.sub(1, -1).unescape().c_str());
	} else if (value.type == common_types.i32) {
		value.as_int() = (int)s2i2(str);
	} else if (value.type == common_types.i64) {
		value.as_int64() = s2i2(str);
	} else if (value.type == common_types.f32) {
		value.as_float() = str._float();
	} else if (value.type == common_types.f64) {
		value.as_float64() = str._float();
	}
}


shared<Node> Parser::apply_format(shared<Node> n, const string &fmt) {
	auto f = n->type->get_member_func(Identifier::func::Format, common_types.string, {common_types.string});
	if (!f)
		do_error(format("format string: no '%s.%s(string)' function found", n->type->long_name(), Identifier::func::Format), n);
	auto *c = tree->add_constant(common_types.string);
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
			auto *c = tree->add_constant(common_types.string);
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
		ee.lines[0].physical_line = Exp.token_logical_line(token_id)->physical_line;
		for (auto& t: ee.lines[0].tokens)
			t.pos += Exp.token_line_offset(token_id) + p0 + 2;
		
		int token0 = Exp.cur_token();
		//int cl = Exp.get_line_no();
		//int ce = Exp.cur_exp;
		Exp.lines.add(ee.lines[0]);
		Exp.update_meta_data();
		Exp.jump(Exp.lines.back().token_ids[0]);
		
		//try {
			auto n = parse_operand_greedy(block, false);
			n = con.deref_if_reference(n);

			if (fmt != "") {
				n = apply_format(n, fmt);
			} else {
				n = con.check_param_link(n, common_types.string_auto_cast, "", 0, 1);
			}
			//n->show();
			parts.add(n);
		/*} catch (Exception &e) {
			msg_write(e.line);
			msg_write(e.column);
			throw;
			//e.line += cl;
			//e.column += Exp.
			
			// not perfect (e has physical line-no etc and e.text has filenames baked in)
			do_error(e.text, token_id);
		}*/
		
		Exp.lines.pop();
		Exp.update_meta_data();
		Exp.jump(token0);
		
		pos = p1 + 2;
	
	}
	
	// empty???
	if (parts.num == 0) {
		auto c = tree->add_constant(common_types.string);
		return add_node_const(c, token_id);
	}
	
	// glue
	while (parts.num > 1) {
		auto b = parts.pop();
		auto a = parts.pop();
		auto n = con.link_operator_id(OperatorID::Add, a, b, token_id);
		parts.add(n);
	}
	//parts[0]->show();
	return parts[0];
}

const Class *merge_type_tuple_into_product(SyntaxTree *tree, const Array<const Class*> &classes, int token_id) {
	return tree->module->context->template_manager->request_product(tree, classes, token_id);
}

/*inline int find_operator(int primitive_id, Type *param_type1, Type *param_type2) {
	for (int i=0;i<PreOperator.num;i++)
		if (PreOperator[i].PrimitiveID == primitive_id)
			if ((PreOperator[i].ParamType1 == param_type1) and (PreOperator[i].ParamType2 == param_type2))
				return i;
	//_do_error_("");
	return 0;
	}*/

// greedily parse AxBxC...(operand, operator)
shared<Node> Parser::parse_operand_greedy(Block *block, bool allow_tuples) {
	auto tree = parse_abstract_operand_greedy(allow_tuples);
	if (config.verbose)
		tree->show();
	return con.concretify_node(tree, block, block->name_space());
}

void analyse_func(Function *f) {
	msg_write("----------------------------");
	msg_write(f->signature());
	msg_write(f->num_params);
	msg_write(b2s(f->is_member()));
	for (auto p: f->literal_param_type)
		msg_write("  LPT: " + p->long_name());
	for (int i=0; i<f->num_params; i++)
		if (auto p = f->abstract_param_type(i))
			msg_write("  APT: " + p->str());
		else
			msg_write("  APT: <nil>");
}

void Parser::post_process_for(shared<Node> cmd_for) {
	auto *n_var = cmd_for->params[0].get();
	auto *var = n_var->as_local();

	/*if (cmd_for->as_statement()->id == StatementID::FOR_CONTAINER) {
		auto *loop_block = cmd_for->params[3].get();

	// ref.
		var->type = tree->get_pointer(var->type);
		n_var->type = var->type;
		tree->transform_node(loop_block, [this, var] (shared<Node> n) {
			return tree->conv_cbr(n, var);
		});
	}*/

	// force for_var out of scope...
	var->name = ":" + var->name;
	if (cmd_for->as_statement()->id == StatementID::ForContainer) {
		auto *index = cmd_for->params[1]->as_local();
		index->name = ":" + index->name;
	}
}



Array<string> parse_comma_sep_list(Parser *p) {
	Array<string> names;

	names.add(p->Exp.consume());

	while (p->try_consume(","))
		names.add(p->Exp.consume());

	return names;
}

Function* Parser::realize_lambda(shared<Node> node, Class* name_space) {
	auto f = realize_function_header(node->params[1], common_types.unknown, name_space);

	f->block_node = node->params[1]->params[4];
	f->block_node->link_no = (int_p)f->block;

	node->set_param(0, add_node_func_name(f));
	return f;
}

void Parser::parse_import() {
	Exp.next(); // 'use'

	[[maybe_unused]] bool also_export = false;
	if (try_consume("@export") or try_consume("out"))
		also_export = true;

	// parse source name (a.b.c, might contain leading dots)
	// "..a.b.c" -> ["", "", "a", "b", "c"]
	int token = Exp.cur_token();
	Array<string> name = {};
	bool recursively = false;
	while (try_consume("."))
		name.add("");
	name.add(Exp.consume());
	while (!Exp.end_of_line()) {
		if (!try_consume("."))
			break;
		expect_no_new_line();
		if (try_consume("*")) {
			recursively = true;
			break;
		} else {
			name.add(Exp.consume());
		}
	}

	// alias
	string as_name;
	if (try_consume(Identifier::As)) {
		expect_no_new_line("name expected after 'as'");
		if (recursively)
			do_error_exp("'as' not allowed after 'use module.*'");
		as_name = Exp.cur;
	}

	// resolve
	auto source = resolve_import_source(this, name, token);

	if (as_name == "")
		as_name = name.back();
	if (recursively)
		tree->import_data_all(source._class, token);
	else
		tree->import_data_selective(source._class, source.func, source.var, source._const, as_name, token);
}

void Parser::realize_enum(shared<Node> node, Class *_namespace) {
	auto _class = tree->create_new_class(node->params[0]->as_token(), common_types.enum_t, sizeof(int), -1, nullptr, {}, _namespace, node->token_id);
	_class->flags = node->flags;

	context->template_manager->request_class_instance(tree, common_types.enum_t, {_class}, node->token_id);

	int next_value = 0;

	for (int i=0; i<node->params.num/3; i++) {

		auto *c = tree->add_constant(_class, _class);
		c->name = node->params[i*3+1]->as_token();

		// explicit value
		if (node->params[i*3+2]) {
			auto cv = eval_to_const(node->params[i*3+2], tree->root_of_all_evil->block, common_types.i32);
			next_value = cv->as_const()->as_int();
		} else {
			// linked from host program?
			next_value = context->external->process_class_offset(_class->cname(_namespace), c->name, next_value);
		}
		c->as_int() = (next_value ++);

		if (node->params[i*3+3]) {
			auto cn = eval_to_const(node->params[i*3+3], tree->root_of_all_evil->block, common_types.string);
			auto label = cn->as_const()->as_string();
			add_enum_label(_class, c->as_int(), label);
		}
	}

	flags_set(_class->flags, Flags::FullyParsed);
}


bool is_same_kind_of_pointer(const Class *a, const Class *b);

void parser_class_add_element(Parser *p, Class *_class, const string &name, const Class *type, Flags flags, int64 &_offset, int token_id) {

	// override?
	ClassElement *orig = base::find_by_element(_class->elements, &ClassElement::name, name);

	bool override = flags_has(flags, Flags::Override);
	if (override and ! orig)
		p->do_error(format("can not override element '%s', no previous definition", name), token_id);
	if (!override and orig)
		p->do_error(format("element '%s' is already defined, use '%s' to override", name, Identifier::Override), token_id);
	if (override) {
		if (is_same_kind_of_pointer(orig->type, type))
			orig->type = type;
		else
			p->do_error("can only override pointer elements with other pointer type", token_id);
		return;
	}

	// check parsing dependencies
	if (!type->is_size_known())
		p->do_error(format("size of type '%s' is not known at this point", type->long_name()), token_id);


	// add element
	if (flags_has(flags, Flags::Static)) {
		auto v = new Variable(name, type);
		flags_set(v->flags, flags);
		_class->static_variables.add(v);
	} else {
		_offset = mem_align(_offset, type->alignment);
		_offset = p->context->external->process_class_offset(_class->cname(p->tree->base_class), name, _offset);
		auto el = ClassElement(name, type, _offset);
		_class->elements.add(el);
		_offset += (int)type->size;
	}
}

const Class* parse_class_type(const string& e) {
	if (e == Identifier::Interface)
		return common_types.interface_t;
	if (e == Identifier::Namespace)
		return common_types.namespace_t;
	if (e == Identifier::Struct)
		return common_types.struct_t;
	return nullptr;
}

Class *Parser::realize_class_header(shared<Node> node, Class* _namespace, int64& var_offset0, const string& name_overwrite) {
	var_offset0 = 0;

	string name = name_overwrite;
	if (name == "" and node->params[1])
		name = node->params[1]->as_token();

	Class *_class = nullptr;
	if (flags_has(node->flags, Flags::Override)) {
		// class override X
		_class = const_cast<Class*>(con.concretify_as_type(node->params[1], tree->root_of_all_evil->block, _namespace));
		var_offset0 = _class->size;
		restore_namespace_mapping.add({_class, _class->name_space});
		_class->name_space = _namespace;
	} else {
		// create class
		_class = const_cast<Class*>(tree->find_root_type_by_name(name, _namespace, false));
		// already created...
		if (!_class)
			tree->module->do_error_internal("class declaration ...not found " + name);
		_class->token_id = node->token_id;
		_class->from_template = parse_class_type(node->params[0]->as_token()); // class/struct/interface;
	}

	// template?
	Array<string> template_param_names;
	if (flags_has(node->flags, Flags::Template)) {
		template_param_names.add(node->params[3]->as_token());
		flags_set(_class->flags, Flags::Template);
		context->template_manager->add_class_template(_class, template_param_names, [_nn = node, template_param_names, _namespace] (SyntaxTree* tree, const Array<const Class*>& tparams, int) -> Class* {
			auto nn = cp_node(_nn);
			nn->link_no = 0;
			flags_clear(nn->flags, Flags::Template);

			auto convert = [template_param_names, tparams] (shared<Node> n) {
				if (n->kind == NodeKind::AbstractToken) {
					for (int i=0; i<tparams.num; i++)
						if (n->as_token() == template_param_names[i])
							return add_node_class(tparams[i], n->token_id);
				}
				return n;
			};

			nn = SyntaxTree::transform_node(nn, convert);

			string _name = format("%s[%s]", _nn->params[1]->as_token(), tparams[0]->name);

			Class *t = tree->create_new_class(_name, _nn->as_class(), 0, 0, nullptr, {}, _namespace, _nn->token_id);
			flags_clear(t->flags, Flags::FullyParsed);

			tree->parser->realize_class(nn, _namespace, _name);
			return t;

			tree->do_error("TEMPLATE INSTANCE...", -1);
			return nullptr;
		});
	}

	// parent class
	if (node->params[2]) {
		auto parent = con.concretify_as_type(node->params[2], tree->root_of_all_evil->block, _namespace); // force
		if (!parent->fully_parsed())
			return nullptr;
			//do_error(format("parent class '%s' not fully parsed yet", parent->long_name()));
		_class->derive_from(parent, DeriveFlags::SET_SIZE | DeriveFlags::KEEP_CONSTRUCTORS | DeriveFlags::COPY_VTABLE);
		_class->flags = parent->flags;
		var_offset0 = _class->size;
	}

	/*if (try_consume(Identifier::Implements)) {
		auto parent = parse_type(_namespace); // force
		if (!parent->fully_parsed())
			return nullptr;
		_class->derive_from(parent, DeriveFlags::SET_SIZE | DeriveFlags::KEEP_CONSTRUCTORS | DeriveFlags::COPY_VTABLE);
		var_offset0 = _class->size;
	}*/

	// as shared|@noauto
	/*Flags explicit_flags = Flags::None;
	if (try_consume(Identifier::As)) {
		explicit_flags = parse_flags(explicit_flags);
		flags_set(_class->flags, explicit_flags);
	}

	expect_new_line();*/

	flags_set(_class->flags, node->flags);

	if (flags_has(node->flags, Flags::Shared)) {
		parser_class_add_element(this, _class, Identifier::SharedCount, common_types.i32, Flags::None, var_offset0, _class->token_id);
	}

	return _class;
}

Class* Parser::realize_class(shared<Node> node, Class* name_space, const string& name_overwrite) {
	int64 var_offset = 0;
	auto _class = realize_class_header(node, name_space, var_offset, name_overwrite);
	if (!_class) // in case, not fully parsed
		return nullptr;
	node->link_no = (int_p)_class;

	if (_class->is_template()) // realize body later...
		return _class;
	Array<int> sub_class_ids;

	for (auto&& [i,n]: enumerate(weak(node->params))) {
		if (!n)
			continue;
		if (n->kind == NodeKind::AbstractEnum) {
			realize_enum(n, _class);
		} else if (n->kind == NodeKind::AbstractClass) {
			if (!realize_class(n, _class))
				sub_class_ids.add(i); // try again later...
		} else if (n->kind == NodeKind::AbstractFunction) {
			realize_function(n, _class);
		} else if (n->kind == NodeKind::AbstractLet) {
			realize_named_const(n, _class, tree->root_of_all_evil->block);
		} else if (n->kind == NodeKind::AbstractVar) {
			realize_class_variable_declaration(n, _class, tree->root_of_all_evil->block, var_offset);
		} else if (n->kind == NodeKind::AbstractUseClassElement) {
			realize_class_use_statement(n, _class);
		}
	}


	post_process_newly_parsed_class(_class, (int)var_offset);


	for (int id: sub_class_ids) {
		auto nn = realize_class(node->params[id], _class);
		if (!nn)
			do_error(format("parent class not fully parsed yet"), id);
	}

	return _class;
}

void Parser::post_process_newly_parsed_class(Class *_class, int size) {
	auto external = context->external.get();

	for (const auto [c,n]: restore_namespace_mapping)
		if (c == _class)
			_class->name_space = n;

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
				e.offset = external->process_class_offset(_class->cname(tree->base_class), e.name, e.offset + config.target.pointer_size);

			auto el = ClassElement(Identifier::VtableVar, common_types.pointer, 0);
			_class->elements.insert(el, 0);
			size += config.target.pointer_size;

			for (auto &i: _class->initializers)
				i.element ++;
		}
	}

	int align = 1;
	if (_class->parent)
		align = _class->parent->alignment;
	for (auto &e: _class->elements)
		align = max(align, e.type->alignment);
	size = mem_align(size, align);
	_class->size = external->process_class_size(_class->cname(tree->base_class), size);
	_class->alignment = align;


	auto_implementer.add_missing_function_headers_for_class(_class);

	flags_set(_class->flags, Flags::FullyParsed);
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

shared<Node> Parser::eval_to_const(shared<Node> cv, Block *block, const Class *type) {
	cv = con.concretify_node(cv, block, block->name_space());
	if (type) {
		CastingDataSingle cast;
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

	if (cv->kind != NodeKind::Constant) {
		//cv->show(common_types._void);
		do_error("constant value expected, but expression can not be evaluated at compile time", cv);
	}
	return cv;
}

shared<Node> Parser::parse_and_eval_const(Block *block, const Class *type) {
	// find const value
	auto cv = parse_abstract_operand_greedy(true);
	if (config.verbose)
		cv->show();
	return eval_to_const(cv, block, type);
}

void Parser::realize_named_const(shared<Node> node, Class *name_space, Block *block) {

	// explicit type?
	const Class *type = nullptr;
	if (node->params[1])
		type = con.concretify_as_type(node->params[1], tree->root_of_all_evil->block, name_space);

	// find const value
	auto cv = eval_to_const(node->params[2], block, type);
	Constant *c_value = cv->as_const();

	auto *c = tree->add_constant(c_value->type.get(), name_space);
	c->set(*c_value);
	c->name = node->params[0]->as_token();
}

void Parser::realize_class_variable_declaration(shared<Node> node, const Class *ns, Block *block, int64 &_offset, Flags flags0) {
	if (ns->is_interface())
		do_error_exp("interfaces can not have data elements");

	Flags flags = parse_flags(flags0);
	flags_set(flags, node->flags);
	if (ns->is_namespace())
		flags_set(flags, Flags::Static);

	auto cc = const_cast<Class*>(ns);

	// explicit type?
	const Class *type = nullptr;
	if (node->params[1])
		type = con.concretify_as_type(node->params[1], tree->root_of_all_evil->block, ns);

	shared<Node> value;
	if (node->params[2]) {

		//if (nodes.num != 1)
		//	do_error(format("'var' declaration with '=' only allowed with a single variable name, %d given", names.num));


		//auto cv = eval_to_const(node->params[2], block, type);
		value = con.force_concrete_type(con.concretify_node(node->params[2], block, ns));
		if (!type)
			type = value->type;
	}


	parser_class_add_element(this, cc, node->params[0]->as_token(), type, flags, _offset, node->params[0]->token_id);

	if (node->params[2]) {
		ClassInitializers init = {ns->elements.num - 1, value};
		cc->initializers.add(init);
	}
}

void Parser::realize_class_use_statement(shared<Node> node, const Class *c) {
	string name = node->params[0]->as_token();
	bool found = false;
	for (auto &e: c->elements)
		if (e.name == name) {
			e.allow_indirect_use = true;
			found = true;
		}
	if (!found)
		do_error_exp(format("use: class '%s' does not have an element '%s'", c->name, name));
}

Function *Parser::realize_function_header(shared<Node> node, const Class *default_type, Class *name_space) {
	// name?
	string name;
	if (!node->params[0]) {
		static int lambda_count = 0;
		name = format(":lambda-%d:", lambda_count ++);
	} else {
		name = node->params[0]->as_token();
		if ((name == Identifier::func::Init) or (name == Identifier::func::Delete) or (name == Identifier::func::Assign))
			flags_set(node->flags, Flags::Mutable);
	}

	Function *f = tree->add_function(name, default_type, name_space, node->flags);

	// template?
	Array<string> template_param_names;
	if (node->params[3]) {
		for (auto t: weak(node->params[3]->params))
			template_param_names.add(t->as_token());
		flags_set(f->flags, Flags::Template);
	}

	//if (config.verbose)
	//	msg_write("PARSE HEAD  " + f->signature());
	f->token_id = node->token_id;
	cur_func = f;

	// parameter list
	if (auto pnode = node->params[2]) {
		for (int i=0; i<pnode->params.num/3; i++) {
			[[maybe_unused]] auto v = f->add_param(pnode->params[i*3]->as_token(), common_types.unknown, pnode->params[i*3]->flags);
		}
	}

	f->abstract_node = node;

	post_process_function_header(f, template_param_names, name_space, node->flags);
	cur_func = nullptr;

	return f;
}

void Parser::post_process_function_header(Function *f, const Array<string> &template_param_names, Class *name_space, Flags flags) {
	if (f->is_template()) {
		context->template_manager->add_function_template(f, template_param_names, nullptr);
		name_space->add_template_function(tree, f, flags_has(flags, Flags::Virtual), flags_has(flags, Flags::Override));
	} else if (f->is_macro()) {
		name_space->add_function(tree, f, false, flags_has(flags, Flags::Override));
	} else {
		con.concretify_function_header(f);

		f->update_parameters_after_parsing();

		name_space->add_function(tree, f, flags_has(flags, Flags::Virtual), flags_has(flags, Flags::Override));
	}
}

void Parser::realize_function(shared<Node> node, Class* name_space) {
	auto f = realize_function_header(node, common_types._void, name_space);
	if (node->params[4]) {
		f->block_node = node->params[4];
		f->block_node->link_no = (int_p)f->block;

		if (config.verbose) {
			msg_write("ABSTRACT:");
			f->block_node->show();
		}
	}

	functions_to_concretify.add(f);
}

void Parser::prerealize_all_class_names_in_block(shared<Node> node, Class *ns) {
	for (auto n: weak(node->params)) {
		if (n and n->kind == NodeKind::AbstractClass and !flags_has(n->flags, Flags::Override)) {
			Class *t = tree->create_new_class(n->params[1]->as_token(), nullptr, 0, 0, nullptr, {}, ns, n->token_id);
			flags_clear(t->flags, Flags::FullyParsed);

			prerealize_all_class_names_in_block(n, t);
		}
	}
}

void Parser::concretify_all_functions() {
	//for (auto *f: function_needs_parsing)   might add lambda functions...
	for (int i=0; i<functions_to_concretify.num; i++) {
		auto f = functions_to_concretify[i];
		if (!f->is_extern() and (f->token_id >= 0)) {
			if (!f->is_template() and !f->is_macro())
				con.concretify_function_body(f);
		}
	}

	cur_func = nullptr;
}

void Parser::realize_tree(shared<Node> node) {
	cur_func = nullptr;
	prerealize_all_class_names_in_block(node, tree->base_class);

	// realize all lambdas
	node = tree->transform_node(node, [this] (shared<Node> n) {
		if (n->kind == NodeKind::Statement and n->as_statement()->id == StatementID::Lambda) {
			realize_lambda(n, tree->base_class);
		}
		return n;
	});


	for (auto&& [i,n]: enumerate(weak(node->params))) {
		if (!n)
			continue;
		if (n->kind == NodeKind::AbstractEnum) {
			realize_enum(n, tree->base_class);
		} else if (n->kind == NodeKind::AbstractClass) {
			realize_class(n, tree->base_class);
		} else if (n->kind == NodeKind::AbstractFunction) {
			realize_function(n, tree->base_class);
		} else if (n->kind == NodeKind::AbstractLet) {
			realize_named_const(n, tree->base_class, tree->root_of_all_evil->block);
		} else if (n->kind == NodeKind::AbstractVar) {
			int64 var_offset = 0;
			realize_class_variable_declaration(n, tree->base_class, tree->root_of_all_evil->block, var_offset);
		}
	}
}

// convert text into script data
void Parser::parse() {
	Exp.reset_walker();
	Exp.do_error_endl = [this] {
		do_error_exp("unexpected newline");
	};

	tree->root_node = parse_abstract_top_level();

	realize_tree(tree->root_node);

	concretify_all_functions();
	
	tree->show("aaa");

	for (auto *f: tree->functions)
		test_node_recursion(f->block_node.get(), tree->base_class, "a " + f->long_name());

	for (int i=0; i<tree->owned_classes.num; i++) // array might change...
		auto_implementer.implement_functions(tree->owned_classes[i]);

	for (auto *f: tree->functions)
		test_node_recursion(f->block_node.get(), tree->base_class, "b " + f->long_name());
}

}
