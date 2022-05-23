#include "../kaba.h"
#include "Parser.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace kaba {

//#define ScriptDebug


extern const Class *TypeDynamicArray;
extern const Class *TypeDictBase;
extern const Class *TypeSharedPointer;
extern const Class *TypeCallableBase;
extern const Class *TypeMatrix;
extern const Class *TypeVec2;

bool is_func(shared<Node> n);

string class_name_might_need_parantheses(const Class *t);
bool type_needs_alignment(const Class *t);


static shared_array<Node> _transform_insert_before_;
shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c);


string Operator::sig(const Class *ns) const {
	if (param_type_1 and param_type_2)
		return format("(%s) %s (%s)", param_type_1->cname(ns), abstract->name, param_type_2->cname(ns));
	if (param_type_1)
		return format("(%s) %s", param_type_1->cname(ns), abstract->name);
	return format("%s (%s)", abstract->name, param_type_2->cname(ns));
}


// recursive
shared<Node> SyntaxTree::cp_node(shared<Node> c) {
	shared<Node> cmd;
	if (c->kind == NodeKind::BLOCK) {
		cmd = new Block(c->as_block()->function, c->as_block()->parent, c->type);
		cmd->as_block()->vars = c->as_block()->vars;
	} else {
		cmd = new Node(c->kind, c->link_no, c->type, c->is_const);
	}
	cmd->token_id = c->token_id;
	cmd->set_num_params(c->params.num);
	for (int i=0;i<c->params.num;i++)
		if (c->params[i])
			cmd->set_param(i, cp_node(c->params[i]));
	return cmd;
}

const Class *SyntaxTree::make_class_callable_fp(Function *f, int token_id) {
	return make_class_callable_fp(f->literal_param_type, f->literal_return_type, token_id);
}

string make_callable_signature(const Array<const Class*> &params, const Class *ret) {
	// maybe some day...
	string signature;// = param->name;
	for (int i=0; i<params.num; i++) {
		if (i > 0)
			signature += ",";
		signature += class_name_might_need_parantheses(params[i]);
	}
	if (params.num == 0)
		signature = "void";
	if (params.num > 1)
		signature = "(" + signature + ")";
	if (params.num == 0 or (params.num == 1 and params[0] == TypeVoid)) {
		signature = "void";
	}
	return signature + "->" + class_name_might_need_parantheses(ret);
}

// input {}->R  OR  void->void   BOTH create  void->R
const Class *SyntaxTree::make_class_callable_fp(const Array<const Class*> &param, const Class *ret, int token_id) {
	string name = make_callable_signature(param, ret);

	auto params_ret = param;
	if ((param.num == 1) and (param[0] == TypeVoid))
		params_ret = {};
	params_ret.add(ret);

	auto ff = make_class("Callable[" + name + "]", Class::Type::CALLABLE_FUNCTION_POINTER, TypeCallableBase->size, 0, nullptr, params_ret, base_class, token_id);
	return make_class(name, Class::Type::POINTER, config.pointer_size, 0, nullptr, {ff}, base_class, token_id);
	//return make_class(name, Class::Type::CALLABLE_FUNCTION_POINTER, TypeCallableBase->size, 0, nullptr, params_ret, base_class);
}

// inner callable: params [A,B,C,D,E]
// captures: [-,x0,-,-,x1]
// class CallableBind
//     func __init__(f, c0, c1)
//     func call(a,b,c)
//         f(a,x0,b,c,c1)
// (A,C,D) -> R
const Class *SyntaxTree::make_class_callable_bind(const Array<const Class*> &params, const Class *ret, const Array<const Class*> &captures, const Array<bool> &capture_via_ref, int token_id) {

	string name = make_callable_signature(params, ret);

	Array<const Class*> outer_params_ret;
	//if ((params.num == 1) and (params[0] == TypeVoid))
	//	outer_params_ret = {};
	for (int i=0; i<params.num; i++)
		if (!captures[i])
			outer_params_ret.add(params[i]);
	outer_params_ret.add(ret);

	static int unique_bind_counter = 0;

	auto t = (Class*)make_class(format(":bind-%d:", unique_bind_counter++), Class::Type::CALLABLE_BIND, TypeCallableBase->size, 0, nullptr, outer_params_ret, base_class, token_id);
	int offset = t->size;
	foreachi (auto b, captures, i) {
		if (!b)
			continue;
		if (capture_via_ref[i])
			b = b->get_pointer();
		if (type_needs_alignment(b))
			offset = mem_align(offset, 4);
		auto el = ClassElement(format("capture%d%s", i, capture_via_ref[i] ? "_ref" : ""), b, offset);
		offset += b->size;
		t->elements.add(el);
	}
	t->size = offset;

	for (auto &e: t->elements)
		if (e.name == "_fp")
			e.type = make_class_callable_fp(params, ret, token_id);

	add_missing_function_headers_for_class(t);
	return t;
}

shared<Node> SyntaxTree::add_node_statement(StatementID id, int token_id, const Class *type) {
	auto *s = statement_from_id(id);
	auto c = new Node(NodeKind::STATEMENT, (int64)s, type, false, token_id);
	c->set_num_params(s->num_params);
	return c;
}

// virtual call, if func is virtual
shared<Node> SyntaxTree::add_node_member_call(Function *f, const shared<Node> inst, int token_id, const shared_array<Node> &params, bool force_non_virtual) {
	shared<Node> c;
	if ((f->virtual_index >= 0) and !force_non_virtual) {
		c = new Node(NodeKind::CALL_VIRTUAL, (int_p)f, f->literal_return_type, true, token_id);
	} else {
		c = new Node(NodeKind::CALL_FUNCTION, (int_p)f, f->literal_return_type, true, token_id);
	}
	c->set_num_params(f->num_params);
	c->set_instance(inst);
	foreachi (auto p, params, i)
		c->set_param(i + 1, p);
	return c;
}

// non-member!
shared<Node> SyntaxTree::add_node_call(Function *f, int token_id) {
	// FIXME: literal_return_type???
	shared<Node> c = new Node(NodeKind::CALL_FUNCTION, (int_p)f, f->literal_return_type, true, token_id);
		c->set_num_params(f->num_params);
	return c;
}

shared<Node> SyntaxTree::add_node_func_name(Function *f, int token_id) {
	return new Node(NodeKind::FUNCTION, (int_p)f, TypeUnknown, true, token_id);
}

shared<Node> SyntaxTree::add_node_class(const Class *c, int token_id) {
	return new Node(NodeKind::CLASS, (int_p)c, TypeClassP, true, token_id);
}


shared<Node> SyntaxTree::add_node_operator(Operator *op, const shared<Node> p1, const shared<Node> p2, int token_id, const Class *override_type) {
	if (!override_type)
		override_type = op->return_type;
	shared<Node> cmd = new Node(NodeKind::OPERATOR, (int_p)op, override_type, true, token_id);
	if (op->abstract->param_flags == 3) {
		cmd->set_num_params(2); // binary
		cmd->set_param(0, p1);
		cmd->set_param(1, p2);
	} else {
		cmd->set_num_params(1); // unary
		cmd->set_param(0, p1);
	}
	return cmd;
}

extern Array<Operator*> global_operators;

shared<Node> SyntaxTree::add_node_operator_by_inline(InlineID inline_index, const shared<Node> p1, const shared<Node> p2, int token_id, const Class *override_type) {
	for (auto *op: global_operators)
		if (op->f->inline_no == inline_index)
			return add_node_operator(op, p1, p2, token_id, override_type);

	do_error(format("operator inline index not found: %d", (int)inline_index));
	return nullptr;
}


shared<Node> SyntaxTree::add_node_local(Variable *v, const Class *type, int token_id) {
	return new Node(NodeKind::VAR_LOCAL, (int_p)v, type, v->is_const(), token_id);
}

shared<Node> SyntaxTree::add_node_local(Variable *v, int token_id) {
	return new Node(NodeKind::VAR_LOCAL, (int_p)v, v->type, v->is_const(), token_id);
}

shared<Node> SyntaxTree::add_node_global(Variable *v, int token_id) {
	return new Node(NodeKind::VAR_GLOBAL, (int_p)v, v->type, v->is_const(), token_id);
}

shared<Node> SyntaxTree::add_node_parray(shared<Node> p, shared<Node> index, const Class *type) {
	shared<Node> cmd_el = new Node(NodeKind::POINTER_AS_ARRAY, 0, type, false, index->token_id);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

shared<Node> SyntaxTree::add_node_dyn_array(shared<Node> array, shared<Node> index) {
	shared<Node> cmd_el = new Node(NodeKind::DYNAMIC_ARRAY, 0, array->type->get_array_element(), false, index->token_id);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, array);
	cmd_el->set_param(1, index);
	return cmd_el;
}

shared<Node> SyntaxTree::add_node_array(shared<Node> array, shared<Node> index, const Class *type) {
	if (!type)
		type = array->type->param[0];
	auto *el = new Node(NodeKind::ARRAY, 0, type, false, index->token_id);
	el->set_num_params(2);
	el->set_param(0, array);
	el->set_param(1, index);
	return el;
}

/*shared<Node> SyntaxTree::add_node_block(Block *b)
{
	return new Node(NodeKind::BLOCK, (long long)(int_p)b, TypeVoid);
}*/

SyntaxTree::SyntaxTree(Module *_module) {
	base_class = new Class("-base-", 0, this);
	_base_class = base_class;
	imported_symbols = new Class("-imported-", 0, this);
	root_of_all_evil = new Function("-root-", TypeVoid, base_class, Flags::STATIC);


	flag_string_const_as_cstring = false;
	flag_function_pointer_as_code = false;
	flag_immortal = false;
	module = _module;
	asm_meta_info = new Asm::MetaInfo(config.pointer_size);
	parser = nullptr;
}

void SyntaxTree::default_import() {
	for (auto p: packages)
		if (p->used_by_default)
			import_data(p, false, "");
}


shared<Node> SyntaxTree::make_constructor_static(shared<Node> n, const string &name) {
	for (auto *f: weak(n->type->functions))
		if (f->name == name) {
			auto nn = add_node_call(f, n->token_id);
			for (int i=0; i<n->params.num-1; i++)
				nn->set_param(i, n->params[i+1]);
			//nn->params = n->params.sub(1,-1);
			return nn;
		}
	return n;
}

void SyntaxTree::digest() {
	if (config.verbose)
		show("digest:pre");

	// turn vector(x,y,z) into vector._create(x,y,z)
	// TODO make more universal! maybe general __create__() function as fake constructor?
	transform([&](shared<Node> n){
		if (n->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION)
			return n;
		if ((n->type == TypeVector) or (n->type == TypeVec2) or (n->type == TypeColor) or (n->type == TypeRect) or (n->type == TypeComplex)) {
			return make_constructor_static(n, "_create");
		}
		if (n->type == TypeQuaternion) {
			if (n->params.num == 2 and n->params[1]->type == TypeVector)
				return make_constructor_static(n, "_rotation_v");
			if (n->params.num == 3 and n->params[1]->type == TypeVector)
				return make_constructor_static(n, "_rotation_a");
			if (n->params.num == 2 and n->params[1]->type == TypeMatrix)
				return make_constructor_static(n, "_rotation_m");
		}
		return n;
	});

	transform([&](shared<Node> n){ return conv_class_and_func_to_const(n); });

	if (config.allow_simplify_consts)
		eval_const_expressions(true);

	transformb([&](shared<Node> n, Block* b){ return conv_break_down_high_level(n, b); });
	if (config.verbose)
		show("digest:break-high");


	transform([&](shared<Node> n){ return conv_break_down_med_level(this, n); });
	transform([&](shared<Node> n){ return conv_break_down_low_level(n); });
	if (config.verbose)
		show("digest:break-low");

	simplify_shift_deref();
	simplify_ref_deref();

	if (config.allow_simplify_consts)
		eval_const_expressions(true);

	if (config.verbose)
		show("digest:pre-proc");

	transform([&](shared<Node> n){ return conv_func_inline(n); });
	if (config.verbose)
		show("digest:inline");

	convert_call_by_reference();
	if (config.verbose)
		show("digest:call-by-ref");

	map_local_variables_to_stack();
	if (config.verbose)
		show("digest:map");

	simplify_ref_deref();
	simplify_shift_deref();
	if (config.verbose)
		show("digest:deref");

	if (config.allow_simplify_consts)
		eval_const_expressions(false);
	if (config.verbose)
		show("digest:map");


	//pre_processor_addresses();
}


void SyntaxTree::do_error(const string &str, int override_token_id) {
	parser->do_error(str, override_token_id);
}


void _asm_add_static_vars(Asm::MetaInfo *meta, const Class *c, const Class *base_ns) {
	for (auto *v: weak(c->static_variables)) {
		Asm::GlobalVar vv;
		vv.name = v->name;
		if (c->name.head(1) != "-" and c != base_ns)
			vv.name = c->cname(c->owner->base_class) + "." + v->name;
		vv.size = v->type->size;
		vv.pos = v->memory;
		meta->global_var.add(vv);
	}
	for (auto *cc: weak(c->classes))
		_asm_add_static_vars(meta, cc, base_ns);
}

void SyntaxTree::create_asm_meta_info() {
	asm_meta_info->global_var.clear();
	_asm_add_static_vars(asm_meta_info.get(), base_class, base_class);
}



Constant *SyntaxTree::add_constant(const Class *type, Class *name_space) {
	if (!name_space)
		name_space = base_class;
	auto *c = new Constant(type, this);
	name_space->constants.add(c);
	return c;
}

Constant *SyntaxTree::add_constant_int(int value) {
	auto *c = add_constant(TypeInt);
	c->as_int() = value;
	return c;
}

Constant *SyntaxTree::add_constant_pointer(const Class *type, const void *value) {
	auto *c = add_constant(type);
	c->as_int64() = (int_p)value;
	return c;
}



Function *SyntaxTree::add_function(const string &name, const Class *return_type, const Class *name_space, Flags flags) {
	if (!name_space)
		name_space = base_class;
	Function *f = new Function(name, return_type, name_space, flags);
	functions.add(f);
	return f;
}



shared<Node> SyntaxTree::add_node_const(Constant *c, int token_id) {
	return new Node(NodeKind::CONSTANT, (int_p)c, c->type.get(), true, token_id);
}

/*shared<Node> SyntaxTree::add_node_block(Block *b) {
	return new Node(NodeKind::BLOCK, (int_p)b, TypeVoid);
}*/

AbstractOperator *Parser::which_abstract_operator(const string &name, int param_flags) {
	for (int i=0; i<(int)OperatorID::_COUNT_; i++)
		if ((name == abstract_operators[i].name) and (param_flags == abstract_operators[i].param_flags))
			return &abstract_operators[i];

	// old hack
	if (name == "!")
		return &abstract_operators[(int)OperatorID::NEGATE];

	return nullptr;
}

const Class *SyntaxTree::which_owned_class(const string &name) {
	for (auto *c: weak(base_class->classes))
		if (name == c->name)
			return c;
	return nullptr;
}

Statement *Parser::which_statement(const string &name) {
	for (auto *s: Statements)
		if (name == s->name)
			return s;
	return nullptr;
}

shared_array<Node> SyntaxTree::get_existence_global(const string &name, const Class *ns, int token_id) {
	shared_array<Node> links;


	// recursively up the namespaces
	while (ns) {

		// named constants
		for (auto *c: weak(ns->constants))
			if (name == c->name)
				return {add_node_const(c, token_id)};

		for (auto *v: weak(ns->static_variables))
			if (v->name == name)
				return {add_node_global(v, token_id)};

		// then the (real) functions
		for (auto *f: weak(ns->functions))
			if (f->name == name and f->is_static())
				links.add(add_node_func_name(f, token_id));
		if (links.num > 0)
			return links;

		// types
		for (auto *c: weak(ns->classes))
			if (c->name == name)
				return {add_node_class(c, token_id)};

		// prefer class...
		if (links.num > 0)
			return links;

		ns = ns->name_space;
	}

	// ...unknown
	return {};
}

shared_array<Node> SyntaxTree::get_element_of(shared<Node> operand, const string &name, int token_id) {
	//operand = force_concrete_type(operand);
	const Class *type = operand->type;
	bool deref = false;
	bool only_static = false;

	if (operand->kind == NodeKind::CLASS) {
		// referencing class functions
		type = operand->as_class();
		only_static = true;
	} else if (type->is_some_pointer()) {
		// pointer -> dereference
		type = type->param[0];
		deref = true;
	}

	// super
	if (type->parent and (name == IDENTIFIER_SUPER)) {
		operand->token_id = token_id;
		if (deref) {
			operand->type = type->parent->get_pointer();
			return {operand};
		}
		return {operand->ref(type->parent->get_pointer())};
	}


	// find element
	if (!only_static) {
		for (auto &e: type->elements)
			if (name == e.name) {
				// direct
				if (deref)
					return {operand->deref_shift(e.offset, e.type, token_id)};
				else
					return {operand->shift(e.offset, e.type, token_id)};
			} else if (e.allow_indirect_use) {

				auto v = deref ? operand->deref_shift(e.offset, e.type, token_id) : operand->shift(e.offset, e.type, token_id);
				auto links = get_element_of(v, name, token_id);
				if (links.num > 0)
					return links;

				// indirect ("use")
				/*auto et = e.type;
				if (e.type->is_some_pointer())
					et = e.type->param[0];
				for (auto &ee: et->elements)
					if (name == ee.name) {
						shared<Node> parent;
						if (deref)
							parent = operand->deref_shift(e.offset, e.type);
						else
							parent = operand->shift(e.offset, e.type);
						if (e.type->is_some_pointer())
							return {parent->deref_shift(ee.offset, ee.type)};
						else
							return {parent->shift(ee.offset, ee.type)};

					}*/
			}
	}
	for (auto *c: weak(type->constants))
		if (name == c->name) {
			return {add_node_const(c, token_id)};
		}
	for (auto *v: weak(type->static_variables))
		if (name == v->name) {
			return {add_node_global(v, token_id)};
		}

	// sub-class
	for (auto *c: weak(type->classes))
		if (name == c->name) {
			return {add_node_class(c, token_id)};
		}


	if (deref and !only_static)
		operand = operand->deref();

	// class function?
	shared_array<Node> links;
	for (auto *cf: weak(type->functions))
		if (name == cf->name) {
			links.add(add_node_func_name(cf, token_id));
			if (!cf->is_static() and !only_static)
				links.back()->params.add(cp_node(operand));
		}
	return links;
}

shared_array<Node> SyntaxTree::get_existence_block(const string &name, Block *block, int token_id) {
	Function *f = block->function;

	// first test local variables
	auto *v = block->get_var(name);
	if (v)
		return {add_node_local(v, token_id)};

	// self.x?
	if (!f->is_static()) {
		auto self = add_node_local(f->__get_var(IDENTIFIER_SELF), token_id);
		auto links = get_element_of(self, name, token_id);
		if (links.num > 0)
			return links;
	}

	for (auto *v: weak(f->name_space->static_variables))
		if (v->name == name)
			return {add_node_global(v)};
	return {};
}

shared_array<Node> SyntaxTree::get_existence(const string &name, Block *block, const Class *ns, int token_id) {
	if (block) {
		auto n = get_existence_block(name, block, token_id);
		if (n.num > 0)
			return n;
	}

	// shared stuff (global variables, functions)
	auto links = get_existence_global(name, ns, token_id);
	if (links.num > 0)
		return links;

	// then the statements
	/*auto s = Parser::which_statement(name);
	if (s) {
		//return {add_node_statement(s->id)};
		auto n = new Node(NodeKind::STATEMENT, (int_p)s, TypeVoid);
		n->set_num_params(s->num_params);
		return {n};
	}*/

	// operators
	auto w = parser->which_abstract_operator(name, 2); // negate/not...
	if (w)
		return {new Node(NodeKind::ABSTRACT_OPERATOR, (int_p)w, TypeUnknown, token_id)};

	// in include files (only global)...
	links.append(get_existence_global(name, imported_symbols.get(), token_id));


	// ...unknown
	return links;
}

Function *SyntaxTree::required_func_global(const string &name, int token_id) {
	auto links = get_existence(name, nullptr, base_class, token_id);
	if (links.num == 0)
		do_error(format("internal error: '%s()' not found????", name), token_id);
	return links[0]->as_func();
}

// expression naming a type
// we are currently in <namespace>... (no explicit namespace for <name>)
const Class *SyntaxTree::find_root_type_by_name(const string &name, const Class *_namespace, bool allow_recursion) {
	for (auto *c: weak(_namespace->classes))
		if (name == c->name)
			return c;
	if (_namespace == base_class) {
		for (auto *c: weak(imported_symbols->classes))
			if (name == c->name)
				return c;
	} else if (_namespace->name_space and allow_recursion) {
		// parent namespace
		return find_root_type_by_name(name, _namespace->name_space, true);
	}
	return nullptr;
}


int kaba_int_passthrough(int i);
int op_int_add(int a, int b);
bool op_int_eq(int a, int b);
bool op_int_neq(int a, int b);
int enum_parse(const string&, const Class*);
Array<int> enum_all(const Class*);


Class *SyntaxTree::create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, const Class *ns, int token_id) {
	if (find_root_type_by_name(name, ns, false))
		do_error("class already exists", token_id);

	Class *t = new Class(name, size, this, parent, params);
	t->type = type;
	t->token_id = token_id;
	owned_classes.add(t);
	
	// link namespace
	(const_cast<Class*>(ns))->classes.add(t);
	t->name_space = ns;
	
	// ->derive_from() will overwrite params!!!

	t->array_length = max(array_size, 0);
	if (t->is_super_array() or t->is_dict()) {
		t->derive_from(TypeDynamicArray, false); // we already set its size!
		if (params[0]->needs_constructor() and !params[0]->get_default_constructor())
			do_error(format("can not create a dynamic array from type '%s', missing default constructor", params[0]->long_name()), token_id);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_array()) {
		if (params[0]->needs_constructor() and !params[0]->get_default_constructor())
			do_error(format("can not create an array from type '%s', missing default constructor", params[0]->long_name()), token_id);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_pointer_shared() or t->is_pointer_owned()) {
		//t->derive_from(TypeSharedPointer, true);
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->type == Class::Type::FUNCTION) {
		t->derive_from(TypeFunction, true);
		t->param = params;
	} else if (t->is_callable_fp()) {
		t->derive_from(TypeCallableBase, true);
		t->functions.clear(); // don't inherit call() with specific types!
		t->param = params;
		add_missing_function_headers_for_class(t);
	} else if (t->is_callable_bind()) {
		t->derive_from(TypeCallableBase, true);
		t->functions.clear(); // don't inherit call() with specific types!
		t->param = params;
		//add_missing_function_headers_for_class(t); // later... depending on the bind variables
	//} else if (t->type == Class::Type::PRODUCT) {
	//	add_missing_function_headers_for_class(t);
	} else if (t->is_enum()) {
		t->flags = Flags::FORCE_CALL_BY_VALUE; // FORCE_CALL_BY_VALUE

		kaba::add_class(t);
		class_add_func("from_int", t, &kaba_int_passthrough, Flags::_STATIC__PURE);
			func_set_inline(InlineID::PASSTHROUGH);
			func_add_param("i", TypeInt);
		//class_add_func(IDENTIFIER_FUNC_STR, TypeString, &i2s, Flags::PURE);
		class_add_func("__int__", TypeInt, &kaba_int_passthrough, Flags::PURE);
			func_set_inline(InlineID::PASSTHROUGH);
		class_add_func("parse", t, &enum_parse, Flags::_STATIC__PURE);
			func_add_param("label", TypeString);
			func_add_param("type", TypeClassP);
		class_add_func("all", TypeDynamicArray, &enum_all, Flags::_STATIC__PURE);
			func_add_param("type", TypeClassP);
		add_operator(OperatorID::ASSIGN, TypeVoid, t, t, InlineID::INT_ASSIGN);
		add_operator(OperatorID::ADD, t, t, t, InlineID::INT_ADD, &op_int_add);
		add_operator(OperatorID::ADDS, TypeVoid, t, t, InlineID::INT_ADD_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, t, t, InlineID::INT_EQUAL, &op_int_eq);
		add_operator(OperatorID::NOTEQUAL, TypeBool, t, t, InlineID::INT_NOT_EQUAL, &op_int_neq);
		add_operator(OperatorID::BIT_AND, t, t, t, InlineID::INT_AND);
		add_operator(OperatorID::BIT_OR, t, t, t, InlineID::INT_OR);

		for (auto f: weak(t->functions)) {
			if (f->name == "parse") {
				f->default_parameters.resize(2);
				auto c = add_constant(TypeClassP, t);
				c->as_int64() = (int_p)t;
				f->mandatory_params = 1;
				f->default_parameters[1] = add_node_const(c, t->token_id);
			} else if (f->name == "all") {
				f->literal_return_type = make_class_super_array(t, t->token_id);
				f->default_parameters.resize(1);
				auto c = add_constant(TypeClassP, t);
				c->as_int64() = (int_p)t;
				f->mandatory_params = 0;
				f->default_parameters[0] = add_node_const(c, t->token_id);
			}
		}
	}
	return t;
}

const Class *SyntaxTree::make_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, const Class *ns, int token_id) {
	//msg_write("make class " + name + " ns=" + ns->long_name() + " param=" + param->long_name());

	// check if it already exists
	auto *tt = find_root_type_by_name(name, ns, false);
	if (tt)
		return tt;

	// add new class
	return create_new_class(name, type, size, array_size, parent, params, ns, token_id);
}

const Class *SyntaxTree::make_class_super_array(const Class *element_type, int token_id) {
	string name = class_name_might_need_parantheses(element_type) + "[]";
	return make_class(name, Class::Type::SUPER_ARRAY, config.super_array_size, -1, TypeDynamicArray, {element_type}, element_type->name_space, token_id);
}

const Class *SyntaxTree::make_class_array(const Class *element_type, int num_elements, int token_id) {
	string name = class_name_might_need_parantheses(element_type) + format("[%d]", num_elements);
	return make_class(name, Class::Type::ARRAY, element_type->size * num_elements, num_elements, nullptr, {element_type}, element_type->name_space, token_id);
}

const Class *SyntaxTree::make_class_dict(const Class *element_type, int token_id) {
	string name = class_name_might_need_parantheses(element_type) + "{}";
	return make_class(name, Class::Type::DICT, config.super_array_size, 0, TypeDictBase, {element_type}, element_type->name_space, token_id);
}

shared<Node> SyntaxTree::conv_cbr(shared<Node> c, Variable *var) {
	// convert
	if ((c->kind == NodeKind::VAR_LOCAL) and (c->as_local() == var)) {
		auto r = add_node_local(var);
		r->set_type(c->type->get_pointer());
		return r->deref();
	}
	return c;
}

#if 0
void conv_return(SyntaxTree *ps, nodes *c) {
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->params[i]);
	
	if ((c->kind == NodeKind::STATEMENT) and (c->link_no == COMMAND_RETURN)) {
		msg_write("conv ret");
		ref_command_old(ps, c);
	}
}
#endif


shared<Node> SyntaxTree::conv_calls(shared<Node> c) {
	if ((c->kind == NodeKind::STATEMENT) and (c->as_statement()->id == StatementID::RETURN))
		if (c->params.num > 0) {
			if ((c->params[0]->type->is_array()) /*or (c->Param[j]->Type->IsSuperArray)*/) {
				c->set_param(0, c->params[0]->ref());
			}
			return c;
		}

	if (c->is_call() or (c->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION)) {
		auto r = c->shallow_copy();
		bool changed = false;

		auto needs_conversion = [&c] (int j) {
			if (c->params[j]->type->uses_call_by_reference())
				return true;
			if (c->is_function()) {
				auto f = c->as_func();
				int param_offset = f->is_static() ? 0 : 1;
				if ((j >= param_offset) and (flags_has(f->var[j]->flags, Flags::OUT)))
					return true;
			}
			return false;
		};

		// parameters, instance: class as reference
		for (int j=0;j<c->params.num;j++)
			if (c->params[j] and needs_conversion(j)) {
				r->set_param(j, c->params[j]->ref());
				changed = true;
			}

		// return: array reference (-> dereference)
		if ((c->type->is_array()) /*or (c->Type->IsSuperArray)*/) {
			r->set_type(c->type->get_pointer());
			return r->deref();
		}
		if (changed)
			return r;
	}

	// special string / list operators
	if (c->kind == NodeKind::OPERATOR) {
		// parameters: super array as reference
		for (int j=0;j<c->params.num;j++)
			if ((c->params[j]->type->is_array()) or (c->params[j]->type->is_super_array())) {
				c->set_param(j, c->params[j]->ref());
				// REALLY ?!?!?!?  FIXME?!?!?
				msg_error("this might be bad");
			}
  	}
	return c;
}


// remove &*x
shared<Node> SyntaxTree::conv_easyfy_ref_deref(shared<Node> c, int l) {
	if (c->kind == NodeKind::REFERENCE) {
		if (c->params[0]->kind == NodeKind::DEREFERENCE) {
			// remove 2 knots...
			return c->params[0]->params[0];
		}
	}
	return c;
}

// remove (*x)[] and (*x).y
shared<Node> SyntaxTree::conv_easyfy_shift_deref(shared<Node> c, int l) {
	if ((c->kind == NodeKind::ADDRESS_SHIFT) or (c->kind == NodeKind::ARRAY)) {
		if (c->params[0]->kind == NodeKind::DEREFERENCE) {
			// unify 2 knots (remove 1)
			c->show(TypeVoid);
			auto kind = (c->kind == NodeKind::ADDRESS_SHIFT) ? NodeKind::DEREF_ADDRESS_SHIFT : NodeKind::POINTER_AS_ARRAY;
			auto r = new Node(kind, 0, c->type, c->is_const);
			r->set_param(0, c->params[0]->params[0]);
			r->set_param(1, c->params[1]);
			r->show(TypeVoid);
			return r;

			/*auto t = c->params[0]->params[0];
			c->kind = (c->kind == NodeKind::ADDRESS_SHIFT) ? NodeKind::DEREF_ADDRESS_SHIFT : NodeKind::POINTER_AS_ARRAY;
			c->set_param(0, t);
			return c;*/
		}
	}

	return c;
}


shared<Node> SyntaxTree::conv_return_by_memory(shared<Node> n, Function *f) {
	parser->cur_func = f;

	if ((n->kind != NodeKind::STATEMENT) or (n->as_statement()->id != StatementID::RETURN))
		return n;

	// convert into   *-return- = param
	shared<Node> p_ret;
	for (Variable *v: weak(f->var))
		if (v->name == IDENTIFIER_RETURN_VAR) {
			p_ret = add_node_local(v);
		}
	if (!p_ret)
		do_error("-return- not found...");
	auto ret = p_ret->deref();
	auto cmd_assign = parser->link_operator_id(OperatorID::ASSIGN, ret, n->params[0]);
	if (!cmd_assign)
		do_error(format("no '=' operator for return from function found: '%s'", f->long_name()));
	_transform_insert_before_.add(cmd_assign);

	return add_node_statement(StatementID::RETURN);
}


// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void SyntaxTree::convert_call_by_reference() {
	if (config.verbose)
		msg_write("ConvertCallByReference");
	// convert functions
	for (Function *f: functions) {
		if (f->literal_return_type == TypeUnknown)
			continue;

		// parameter: array/class as reference
		for (auto v: weak(f->var).sub_ref(0, f->num_params))
			if (v->type->uses_call_by_reference() or flags_has(v->flags, Flags::OUT)) {
				v->type = v->type->get_pointer();

				// usage inside the function
				transform_block(f->block.get(), [&](shared<Node> n){ return conv_cbr(n, v); });
			}
	}

	// convert return...
	for (Function *f: functions)
		if (f->literal_return_type->uses_return_by_memory() and (f->literal_return_type != TypeUnknown))
			//convert_return_by_memory(this, f->block, f);
			transform_block(f->block.get(), [&](shared<Node> n){ return conv_return_by_memory(n, f); });

	// convert function calls
	transform([&](shared<Node> n){ return conv_calls(n); });
}


void SyntaxTree::simplify_ref_deref() {
	// remove &*
	transform([&](shared<Node> n){ return conv_easyfy_ref_deref(n, 0); });
}

void SyntaxTree::simplify_shift_deref() {
	// remove &*
	transform([&](shared<Node> n){ return conv_easyfy_shift_deref(n, 0); });
}

InlineID __get_pointer_add_int() {
	if (config.instruction_set == Asm::InstructionSet::AMD64)
		return InlineID::INT64_ADD_INT;
	return InlineID::INT_ADD;
}


shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c) {
	if (c->kind == NodeKind::DYNAMIC_ARRAY) {
		return tree->conv_break_down_low_level(
				tree->add_node_parray(
						c->params[0]->shift(0, c->type->get_pointer()),
						c->params[1], c->type));
	}
	return c;
}

shared<Node> SyntaxTree::conv_break_down_low_level(shared<Node> c) {

	if (c->kind == NodeKind::ARRAY) {

		auto *el_type = c->type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0]->ref(), // array
				add_node_operator_by_inline(InlineID::INT_MULTIPLY,
						c->params[1], // ref
						add_node_const(add_constant_int(el_type->size))),
				c->token_id,
				el_type->get_pointer())->deref();

	} else if (c->kind == NodeKind::POINTER_AS_ARRAY) {

		auto *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0], // ref array
				add_node_operator_by_inline(InlineID::INT_MULTIPLY,
						c->params[1], // index
						add_node_const(add_constant_int(el_type->size))),
				c->token_id,
				el_type->get_pointer())->deref();
	} else if (c->kind == NodeKind::ADDRESS_SHIFT) {

		auto *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0]->ref(), // struct
				add_node_const(add_constant_int(c->link_no)),
				c->token_id,
				el_type->get_pointer())->deref();

	} else if (c->kind == NodeKind::DEREF_ADDRESS_SHIFT) {

		auto *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		// create command for shift constant
		// address = &struct + shift
		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0], // ref struct
				add_node_const(add_constant_int(c->link_no)),
				c->token_id,
				el_type->get_pointer())->deref();
	}
	return c;
}


shared<Node> SyntaxTree::transform_node(shared<Node> n, std::function<shared<Node>(shared<Node>)> F) {
	if (n->kind == NodeKind::BLOCK) {
		transform_block(n->as_block(), F);
		return n;
	} else {
		shared<Node> r = n;
		for (int i=0; i<n->params.num; i++) {
			auto rr = transform_node(n->params[i], F);
			if (rr != n->params[i].get()) {
				if (r.get() == n.get())
					r = n->shallow_copy();
				r->set_param(i, rr);
			}
		}
		return F(r);
	}
}

shared<Node> SyntaxTree::transformb_node(shared<Node> n, Block *b, std::function<shared<Node>(shared<Node>, Block*)> F) {
	if (n->kind == NodeKind::BLOCK) {
		transformb_block(n->as_block(), F);
		return F(n, b);
	} else {
		shared<Node> r = n;
		for (int i=0; i<n->params.num; i++) {
			auto rr = transformb_node(n->params[i], b, F);
			if (rr != n->params[i].get()) {
				if (r.get() == n.get())
					r = n->shallow_copy();
				r->set_param(i, rr);
			}
		}
		return F(r, b);
	}
}

// preventing a sub-block to handle insertions of an outer block
#define PUSH_BLOCK_INSERT \
	auto XXX = _transform_insert_before_; \
	_transform_insert_before_.clear();
#define POP_BLOCK_INSERT \
	_transform_insert_before_ = XXX;

void handle_insert_before(Block *block, int &i) {
	if (_transform_insert_before_.num > 0) {
		for (auto *ib: weak(_transform_insert_before_)) {
			if (config.verbose)
				msg_error("INSERT BEFORE...2");
			block->params.insert(ib, i);
			i ++;
		}
		_transform_insert_before_.clear();
	}
}


void SyntaxTree::transform_block(Block *block, std::function<shared<Node>(shared<Node>)> F) {
	PUSH_BLOCK_INSERT;
	//foreachi (shared<Node> n, block->nodes, i){
	for (int i=0; i<block->params.num; i++) {
		block->params[i] = transform_node(block->params[i], F);
		handle_insert_before(block, i);
	}
	POP_BLOCK_INSERT;
}

void SyntaxTree::transformb_block(Block *block, std::function<shared<Node>(shared<Node>, Block*)> F) {
	PUSH_BLOCK_INSERT;
	for (int i=0; i<block->params.num; i++) {
		block->params[i] = transformb_node(block->params[i], block, F);
		handle_insert_before(block, i);
	}
	POP_BLOCK_INSERT;
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::transform(std::function<shared<Node>(shared<Node>)> F) {
	for (Function *f: functions)
		if (!f->is_template()) {
			parser->cur_func = f;
			transform_block(f->block.get(), F);
		}
}
void SyntaxTree::transformb(std::function<shared<Node>(shared<Node>, Block*)> F) {
	for (Function *f: functions)
		if (!f->is_template()) {
			parser->cur_func = f;
			transformb_block(f->block.get(), F);
		}
}

bool node_is_executable(shared<Node> n) {
	if ((n->kind == NodeKind::CONSTANT) or (n->kind == NodeKind::VAR_LOCAL) or (n->kind == NodeKind::VAR_GLOBAL))
		return false;
	if ((n->kind == NodeKind::ADDRESS_SHIFT) or (n->kind == NodeKind::ARRAY) or (n->kind == NodeKind::DYNAMIC_ARRAY) or (n->kind == NodeKind::REFERENCE) or (n->kind == NodeKind::DEREFERENCE) or (n->kind == NodeKind::DEREF_ADDRESS_SHIFT))
		return node_is_executable(n->params[0]);
	return true;
}

shared<Node> SyntaxTree::conv_class_and_func_to_const(shared<Node> n) {
	if (n->kind == NodeKind::CLASS) {
		return add_node_const(add_constant_pointer(TypeClassP, n->as_class()));
	}
	return n;
}


shared<Node> SyntaxTree::conv_break_down_high_level(shared<Node> n, Block *b) {
	if (n->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION) {
		if (config.verbose) {
			msg_error("constr func....");
			n->show(base_class);
		}
		
		// TODO later in serializer!

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type);
		vv->explicitly_constructed = true;
		auto dummy = add_node_local(vv);
		
		auto ib = add_node_call(n->as_func(), n->token_id);
		ib->params = n->params;
		ib->set_instance(dummy);
		if (config.verbose)
			ib->show(base_class);

		_transform_insert_before_.add(ib);

		return dummy;
	} else if (n->kind == NodeKind::ARRAY_BUILDER) {
		auto *t_el = n->type->get_array_element();
		Function *cf = n->type->get_member_func("add", TypeVoid, {t_el});
		if (!cf) {
			msg_error("AAA");
			n->show();
			msg_write(p2s(n->type));
			msg_write(n->type->name);
			do_error(format("[..]: can not find '%s.add(%s)' function???", n->type->long_name(), t_el->long_name()));
		}

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type);
		auto array = add_node_local(vv);

		Block *bb = new Block(f, b);
		for (int i=0; i<n->params.num; i++){
			auto cc = add_node_member_call(cf, array, n->token_id);
			cc->set_param(1, n->params[i]);
			bb->add(cc);
		}
		_transform_insert_before_.add(bb);
		return array;
	} else if (n->kind == NodeKind::DICT_BUILDER) {
		auto *t_el = n->type->get_array_element();
		Function *cf = n->type->get_member_func("__set__", TypeVoid, {TypeString, t_el});
		if (!cf)
			do_error(format("[..]: can not find '%s.__set__(string,%s)' function???", n->type->long_name(), t_el->long_name()));

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type);
		auto array = add_node_local(vv);

		Block *bb = new Block(f, b);
		for (int i=0; i<n->params.num/2; i++){
			auto cc = add_node_member_call(cf, array, n->token_id);
			cc->set_param(1, n->params[i*2]);
			cc->set_param(2, n->params[i*2+1]);
			bb->add(cc);
		}
		_transform_insert_before_.add(bb);
		return array;
	} else if ((n->kind == NodeKind::STATEMENT) and (n->as_statement()->id == StatementID::FOR_RANGE)) {

		// [VAR, START, STOP, STEP, BLOCK]
		auto var = n->params[0];
		auto val0 = n->params[1];
		auto val1 = n->params[2];
		auto step = n->params[3];
		auto block = n->params[4];


		auto nn = add_node_statement(StatementID::FOR_DIGEST);
		nn->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]

		// assign
		nn->set_param(0, add_node_operator_by_inline(InlineID::INT_ASSIGN, var, val0));

		// while(for_var < val1)
		nn->set_param(1, add_node_operator_by_inline(InlineID::INT_SMALLER, var, val1));

		nn->set_param(2, block);


		// ...for_var += 1
		shared<Node> cmd_inc;
		if (var->type == TypeInt) {
			if (step->as_const()->as_int() == 1)
				cmd_inc = add_node_operator_by_inline(InlineID::INT_INCREASE, var, nullptr);
			else
				cmd_inc = add_node_operator_by_inline(InlineID::INT_ADD_ASSIGN, var, step);
		} else {
			cmd_inc = add_node_operator_by_inline(InlineID::FLOAT_ADD_ASSIGN, var, step);
		}
		nn->set_param(3, cmd_inc); // add to loop-block

		return nn;
	} else if ((n->kind == NodeKind::STATEMENT) and (n->as_statement()->id == StatementID::FOR_ARRAY)) {

		// [VAR, INDEX, ARRAY, BLOCK]
		auto var = n->params[0];
		auto index = n->params[1];
		auto array = n->params[2];
		auto block = n->params[3];
		
		
		// array needs execution?
		if (node_is_executable(array)) {
			// -> assign into variable before the loop
			auto *v = b->add_var(b->function->create_slightly_hidden_name(), array->type);

			auto assign = parser->link_operator_id(OperatorID::ASSIGN, add_node_local(v), array);
			_transform_insert_before_.add(assign);

			array = add_node_local(v);
		}

		auto nn = add_node_statement(StatementID::FOR_DIGEST);
		nn->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]


		// 0
		auto val0 = add_node_const(add_constant_int(0));

		// implement
		// for_index = 0
		nn->set_param(0, add_node_operator_by_inline(InlineID::INT_ASSIGN, index, val0));

		shared<Node> val1;
		if (array->type->usable_as_super_array()) {
			// array.num
			val1 = array->shift(config.pointer_size, TypeInt, array->token_id);
		} else {
			// array.size
			val1 = add_node_const(add_constant_int(array->type->array_length));
		}

		// while(for_index < val1)
		nn->set_param(1, add_node_operator_by_inline(InlineID::INT_SMALLER, index, val1));

		// ...block
		nn->set_param(2, block);

		// ...for_index += 1
		nn->set_param(3, add_node_operator_by_inline(InlineID::INT_INCREASE, index, nullptr));

		// array[index]
		shared<Node> el;
		if (array->type->usable_as_super_array()) {
			el = add_node_dyn_array(array, index);
		} else {
			el = add_node_array(array, index);
		}

		// &for_var = &array[index]
		auto cmd_var_assign = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, var, el->ref());
		block->params.insert(cmd_var_assign, 0);

		return nn;
	} else if (n->kind == NodeKind::ARRAY_BUILDER_FOR) {

		_transform_insert_before_.add(n->params[0]);
		return n->params[1];
	} else if (n->kind == NodeKind::TUPLE_EXTRACTION) {

		// temp var
		auto *f = b->function;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->params[0]->type);
		auto temp = add_node_local(vv);

		Block *bb = new Block(f, b);

		// tuple assign -> temp
		Function *cf = n->params[0]->type->get_assign();
		if (!cf)
			do_error(format("tuple-extract: don't know how to assign tuple of type '%s'", n->params[0]->type->long_name()), n->token_id);
		auto assign = add_node_member_call(cf, temp, n->token_id, {n->params[0]});
		bb->add(assign);

		for (int i=1; i<n->params.num; i++) {
			// element assign

			Function *ecf = n->params[i]->type->get_assign();
			if (!ecf)
				do_error(format("tuple-extract: don't know how to assign tuple element %d of type '%s'", i, n->params[i]->type->long_name()), n->token_id);
			auto &e = n->params[0]->type->elements[i-1];
			auto assign = add_node_member_call(ecf, n->params[i], n->token_id, {temp->shift(e.offset, e.type, n->token_id)});
			bb->add(assign);
		}

		//bb->show();
		return bb;

	}

	// TODO experimental dynamic type insertion
	if (false and is_func(n)) {
		auto f = n->as_func();
		for (int i=0; i<f->num_params; i++)
			if (f->literal_param_type[i] == TypeDynamic) {
				msg_error("conv dyn!");
				auto c = add_constant(TypeClassP);
				c->as_int64() = (int64)(int_p)n->params[i]->type;
				n->params.insert(add_node_const(c), i+1);
				n->show(base_class);
			}
	}
	return n;
}

shared<Node> SyntaxTree::conv_func_inline(shared<Node> n) {
	if (n->kind == NodeKind::CALL_FUNCTION) {
		if (n->as_func()->inline_no != InlineID::NONE) {
			auto r = new Node(NodeKind::CALL_INLINE, n->link_no, n->type, n->is_const);
			r->params = n->params;
			return r;
		}
	}
	if (n->kind == NodeKind::OPERATOR) {
		Operator *op = n->as_op();
		if (op->f->inline_no != InlineID::NONE) {
			auto r = new Node(NodeKind::CALL_INLINE, (int_p)op->f, n->type, n->is_const);
			r->params = n->params;
			return r;
		} else {
			auto r = new Node(NodeKind::CALL_FUNCTION, (int_p)op->f, n->type, n->is_const);
			r->params = n->params;
			return r;
		}
	}
	return n;
}


void MapLVSX86Return(Function *f, int64 &stack_offset) {
	foreachi (auto v, f->var, i)
		if (v->name == IDENTIFIER_RETURN_VAR) {
			v->_offset = stack_offset;
			stack_offset += config.pointer_size;
		}
}

void MapLVSX86Self(Function *f, int64 &stack_offset) {
	foreachi (auto v, f->var, i)
		if (v->name == IDENTIFIER_SELF) {
			v->_offset = stack_offset;
			stack_offset += config.pointer_size;
		}
}

void SyntaxTree::map_local_variables_to_stack() {
	for (Function *f: functions) {

		if (config.instruction_set == Asm::InstructionSet::X86) {
			f->_var_size = 0;
			int64 stack_offset = 2 * config.pointer_size; // space for eIP and eBP
			// offsets to stack pointer (for push parameters)

			if (config.abi == Abi::X86_WINDOWS) {
				// map "self" to the VERY first parameter
				if (!f->is_static())
					MapLVSX86Self(f, stack_offset);

				// map "-return-" to the first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);
			} else {
				// map "-return-" to the VERY first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);

				// map "self" to the first parameter
				if (!f->is_static())
					MapLVSX86Self(f, stack_offset);
			}

			foreachi (auto v, f->var, i) {
				if (!f->is_static() and (v->name == IDENTIFIER_SELF))
					continue;
				if (v->name == IDENTIFIER_RETURN_VAR)
					continue;
				int s = mem_align(v->type->size, 4);
				if (i < f->num_params) {
					// parameters
					v->_offset = stack_offset;
					stack_offset += s;
				} else {
					// "real" local variables
					v->_offset = - f->_var_size - s;
					f->_var_size += s;
				}
			}
		} else if (config.instruction_set == Asm::InstructionSet::AMD64) {
			f->_var_size = 0;
			int64 stack_offset = 2 * config.pointer_size; // space for rIP and rBP
			// offsets to stack pointer (for push parameters)

			if (config.abi == Abi::AMD64_WINDOWS) {

				// map "self" to the VERY first parameter
				if (!f->is_static())
					MapLVSX86Self(f, stack_offset);

				// map "-return-" to the first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);

				foreachi(auto v, f->var, i) {
					if (!f->is_static() and (v->name == IDENTIFIER_SELF))
						continue;
					if (v->name == IDENTIFIER_RETURN_VAR)
						continue;
					if (i < f->num_params) {
						// parameters
						int s = 8;
						v->_offset = stack_offset;
						stack_offset += s;
					} else {
						// "real" local variables
						int64 s = mem_align(v->type->size, 4);
						f->_var_size += s;
						v->_offset = -f->_var_size;
					}
				}
			} else {
				// TODO map push parameters...
				foreachi (auto v, f->var, i) {
					int64 s = mem_align(v->type->size, 4);
					f->_var_size += s;
					v->_offset = -f->_var_size;
				}
			}
		} else if (config.instruction_set == Asm::InstructionSet::ARM) {
			f->_var_size = 0;

			foreachi (auto v, f->var, i) {
				int s = mem_align(v->type->size, 4);
				v->_offset = f->_var_size;// + s;
				f->_var_size += s;
			}
		}
	}
}


// no included modules may be deleted before us!!!
SyntaxTree::~SyntaxTree() {
	// delete all classes, functions etc created by this module
}

void SyntaxTree::show(const string &stage) {
	if (!config.allow_output_stage(stage))
		return;
	msg_write(format("--------- Syntax of %s  %s ---------", module->filename, stage));
	msg_right();
	for (auto *f: functions)
		if (!f->is_extern())
			f->show(stage);
	msg_left();
	msg_write("\n\n");
}

};
