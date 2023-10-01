#include "../kaba.h"
#include "../parser/Parser.h"
#include "../template/template.h"
#include "../asm/asm.h"
#include "../../os/msg.h"
#include "../../base/iter.h"
#include <stdio.h>

namespace kaba {

//#define ScriptDebug


extern const Class *TypeDynamicArray;
extern const Class *TypeDictBase;
extern const Class *TypeCallableBase;
extern const Class *TypeMat4;
extern const Class *TypeVec2;
extern const Class *TypeSpecialFunctionRef;

bool is_func(shared<Node> n);

string class_name_might_need_parantheses(const Class *t);
bool type_needs_alignment(const Class *t);


static shared_array<Node> _transform_insert_before_;
shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c);




const Class *SyntaxTree::request_implicit_class_callable_fp(Function *f, int token_id) {
	return request_implicit_class_callable_fp(f->literal_param_type, f->literal_return_type, token_id);
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
const Class *SyntaxTree::request_implicit_class_callable_fp(const Array<const Class*> &param, const Class *ret, int token_id) {
	string name = make_callable_signature(param, ret);

	auto params_ret = param;
	if ((param.num == 1) and (param[0] == TypeVoid))
		params_ret = {};
	params_ret.add(ret);

	auto ff = request_implicit_class("Callable[" + name + "]", Class::Type::CALLABLE_FUNCTION_POINTER, TypeCallableBase->size, 0, nullptr, params_ret, token_id);
	return request_implicit_class(name, Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, {ff}, token_id);
	//return make_class(name, Class::Type::CALLABLE_FUNCTION_POINTER, TypeCallableBase->size, 0, nullptr, params_ret, base_class);
}

// inner callable: params [A,B,C,D,E]
// captures: [-,x0,-,-,x1]
// class CallableBind
//     func __init__(f, c0, c1)
//     func call(a,b,c)
//         f(a,x0,b,c,c1)
// (A,C,D) -> R
const Class *SyntaxTree::request_implicit_class_callable_bind(const Array<const Class*> &params, const Class *ret, const Array<const Class*> &captures, const Array<bool> &capture_via_ref, int token_id) {

	string name = make_callable_signature(params, ret);

	Array<const Class*> outer_params_ret;
	//if ((params.num == 1) and (params[0] == TypeVoid))
	//	outer_params_ret = {};
	for (int i=0; i<params.num; i++)
		if (!captures[i])
			outer_params_ret.add(params[i]);
	outer_params_ret.add(ret);

	static int unique_bind_counter = 0;

	auto t = (Class*)create_new_class(format(":bind-%d:", unique_bind_counter++), Class::Type::CALLABLE_BIND, TypeCallableBase->size, 0, nullptr, outer_params_ret, base_class, token_id);
	int offset = t->size;
	for (auto [i,b]: enumerate(captures)) {
		if (!b)
			continue;
		auto c = b;
		if (capture_via_ref[i])
			c = get_pointer(c, token_id);
		if (type_needs_alignment(b))
			offset = mem_align(offset, 4);
		auto el = ClassElement(format("capture%d%s", i, capture_via_ref[i] ? "_ref" : ""), c, offset);
		offset += c->size;
		t->elements.add(el);
	}
	t->size = offset;

	for (auto &e: t->elements)
		if (e.name == "_fp")
			e.type = request_implicit_class_callable_fp(params, ret, token_id);

	add_missing_function_headers_for_class(t);
	return t;
}

SyntaxTree::SyntaxTree(Module *_module) {
	flag_string_const_as_cstring = false;
	flag_function_pointer_as_code = false;
	flag_immortal = false;
	module = _module;
	asm_meta_info = new Asm::MetaInfo(config.target.pointer_size);

	base_class = new Class(Class::Type::REGULAR, "-base-", 0, this);
	_base_class = base_class;
	imported_symbols = new Class(Class::Type::REGULAR, "-imported-", 0, this);
	implicit_symbols = new Class(Class::Type::REGULAR, "-implicit-", 0, this);
	root_of_all_evil = new Function("-root-", TypeVoid, base_class, Flags::STATIC);
}

void SyntaxTree::default_import() {
	for (auto p: module->context->packages)
		if (p->used_by_default)
			import_data(p, false, "");
}

void SyntaxTree::digest() {
	if (config.verbose)
		show("digest:pre");

	// turn vector(x,y,z) into vector._create(x,y,z)
	// TODO make more universal! maybe general __create__() function as fake constructor?
	transform([this] (shared<Node> n) {
		return conv_fake_constructors(n);
	});

	transform([this] (shared<Node> n) {
		return conv_class_and_func_to_const(n);
	});

	if (config.allow_simplify_consts)
		eval_const_expressions(true);

	transformb([this] (shared<Node> n, Block* b) {
		return conv_break_down_high_level(n, b);
	});
	
	if (config.verbose)
		show("digest:break-high");


	transform([this] (shared<Node> n) {
		return conv_break_down_med_level(this, n);
	});
	transform([this] (shared<Node> n) {
		return conv_break_down_low_level(n);
	});
	if (config.verbose)
		show("digest:break-low");

	simplify_shift_deref();
	simplify_ref_deref();

	if (config.allow_simplify_consts)
		eval_const_expressions(true);

	if (config.verbose)
		show("digest:pre-proc");

	transform([this] (shared<Node> n) {
		return conv_func_inline(n);
	});
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



AbstractOperator *Parser::which_abstract_operator(const string &name, OperatorFlags param_flags) {
	for (int i=0; i<(int)OperatorID::_COUNT_; i++)
		if ((name == abstract_operators[i].name) and ((int)param_flags == (abstract_operators[i].flags & OperatorFlags::BINARY)))
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

SpecialFunction *Parser::which_special_function(const string &name) {
	for (auto *s: special_functions)
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
		if (name == Identifier::SELF_CLASS)
			return {add_node_class(ns, token_id)};

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
	bool allow_member = true;

	if (operand->kind == NodeKind::CLASS) {
		// referencing class functions
		type = operand->as_class();
		allow_member = false;
	//} else if (type->is_some_pointer()) {
	} else if (type->is_some_pointer_not_null()) {
		// pointer -> dereference
		type = type->param[0];
		deref = true;
	}

	// super
	if (type->parent and (name == Identifier::SUPER)) {
		operand->token_id = token_id;
		auto t_ref = request_implicit_class_reference(type->parent, token_id);
		if (deref) {
			operand->type = t_ref;
			return {operand};
		}
		return {operand->ref(t_ref)};
	}


	// find element
	if (allow_member) {
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
		if (name == c->name)
			return {add_node_const(c, token_id)};

	for (auto *v: weak(type->static_variables))
		if (name == v->name)
			return {add_node_global(v, token_id)};

	// sub-class
	for (auto *c: weak(type->classes))
		if (name == c->name)
			return {add_node_class(c, token_id)};


	if (deref and allow_member)
		operand = operand->deref();

	// class function?
	shared_array<Node> links;
	for (auto *cf: weak(type->functions))
		if (name == cf->name) {
			links.add(add_node_func_name(cf, token_id));
			if (cf->is_member() and allow_member)
				links.back()->params.add(cp_node(operand));
		}
	return links;
}

shared_array<Node> SyntaxTree::get_existence_block(const string &name, Block *block, int token_id) {
	Function *f = block->function;

	// first test local variables
	if (auto *v = block->get_var(name)) {
		if (v->type->is_pointer_alias())
			return {add_node_local(v, token_id)->deref()};
		else
			return {add_node_local(v, token_id)};
	}

	// self.x?
	if (f->is_member()) {
		auto self = add_node_local(f->__get_var(Identifier::SELF), token_id);
		auto links = get_element_of(self, name, token_id);
		if (links.num > 0)
			return links;
	}

	for (auto *v: weak(f->name_space->static_variables))
		if (v->name == name)
			return {add_node_global(v)};

	if (name == Identifier::RETURN_CLASS)
		return {add_node_class(f->literal_return_type)};
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
	/*if (auto s = Parser::which_statement(name)) {
		//return {add_node_statement(s->id)};
		auto n = new Node(NodeKind::STATEMENT, (int_p)s, TypeVoid);
		n->set_num_params(s->num_params);
		return {n};
	}*/

	// operators
	if (auto w = parser->which_abstract_operator(name, OperatorFlags::UNARY_RIGHT)) // negate/not...
		return {new Node(NodeKind::ABSTRACT_OPERATOR, (int_p)w, TypeUnknown, Flags::NONE, token_id)};

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


void SyntaxTree::add_missing_function_headers_for_class(Class *t) {
	AutoImplementer a(nullptr, this);
	a.add_missing_function_headers_for_class(t);
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


Class *SyntaxTree::create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, Class *ns, int token_id) {
	if (find_root_type_by_name(name, ns, false))
		do_error(format("class '%s' already exists", name), token_id);
	return create_new_class_no_check(name, type, size, array_size, parent, params, ns, token_id);
}


Class *SyntaxTree::create_new_class_no_check(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, Class *ns, int token_id) {
	//msg_write("CREATE " + name);

	Class *t = new Class(type, name, size, this, parent, params);
	t->token_id = token_id;
	owned_classes.add(t);
	
	// link namespace
	ns->classes.add(t);
	t->name_space = ns;
	
	AutoImplementer ai(nullptr, this);
	ai.complete_type(t, array_size, token_id);
	return t;
}

// X[], X{}, X*, X shared, (X,Y,Z), X->Y
const Class *SyntaxTree::request_implicit_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, int token_id) {
	//msg_write("make class " + name + " ns=" + ns->long_name());// + " params=" + param->long_name());

	// check if it already exists
	if (auto *tt = module->context->implicit_class_registry->find(name, type, array_size, params))
		return tt;

	// add new class
	auto t = create_new_class_no_check(name, type, size, array_size, parent, params, implicit_symbols.get(), token_id);
	module->context->implicit_class_registry->add(t);
	return t;
}

const Class *SyntaxTree::get_pointer(const Class *base, int token_id) {
//	return request_implicit_class(class_name_might_need_parantheses(base) + "*", Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, {base}, token_id);
	return request_implicit_class_pointer(base, token_id);
}

const Class *SyntaxTree::request_implicit_class_pointer(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("ptr[..] not allowed for: " + base->long_name(), token_id); // TODO
	return request_implicit_class(class_name_might_need_parantheses(base) + "*", Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, {base}, token_id);
//	return request_implicit_class(format("%s[%s]", Identifier::RAW_POINTER, base->name), Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_shared(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("shared[..] not allowed for: " + base->long_name(), token_id); // TODO
	return request_implicit_class(format("%s[%s]", Identifier::SHARED, base->name), Class::Type::POINTER_SHARED, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_shared_not_null(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("shared![..] not allowed for: " + base->long_name(), token_id); // TODO
	return request_implicit_class(format("%s![%s]", Identifier::SHARED, base->name), Class::Type::POINTER_SHARED_NOT_NULL, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_owned(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("owned[..] not allowed for: " + base->long_name(), token_id);
	return request_implicit_class(format("%s[%s]", Identifier::OWNED, base->name), Class::Type::POINTER_OWNED, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_owned_not_null(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("owned![..] not allowed for: " + base->long_name(), token_id);
	return request_implicit_class(format("%s![%s]", Identifier::OWNED, base->name), Class::Type::POINTER_OWNED_NOT_NULL, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_xfer(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("xfer[..] not allowed for: " + base->long_name(), token_id);
	return request_implicit_class(format("%s[%s]", Identifier::XFER, base->name), Class::Type::POINTER_XFER, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_alias(const Class *base, int token_id) {
	if (!base->name_space)
		do_error("@alias[..] not allowed for: " + base->long_name(), token_id);
	return request_implicit_class(format("%s[%s]", Identifier::ALIAS, base->name), Class::Type::POINTER_ALIAS, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_reference(const Class *base, int token_id) {
	return request_implicit_class(class_name_might_need_parantheses(base) + "&", Class::Type::REFERENCE, config.target.pointer_size, 0, nullptr, {base}, token_id);
}

const Class *SyntaxTree::request_implicit_class_list(const Class *element_type, int token_id) {
	string name = class_name_might_need_parantheses(element_type) + "[]";
	return request_implicit_class(name, Class::Type::LIST, config.target.dynamic_array_size, -1, TypeDynamicArray, {element_type}, token_id);
}

const Class *SyntaxTree::request_implicit_class_array(const Class *element_type, int num_elements, int token_id) {
	string name = class_name_might_need_parantheses(element_type) + format("[%d]", num_elements);
	return request_implicit_class(name, Class::Type::ARRAY, element_type->size * num_elements, num_elements, nullptr, {element_type}, token_id);
}

const Class *SyntaxTree::request_implicit_class_dict(const Class *element_type, int token_id) {
	string name = class_name_might_need_parantheses(element_type) + "{}";
	return request_implicit_class(name, Class::Type::DICT, config.target.dynamic_array_size, 0, TypeDictBase, {element_type}, token_id);
}

const Class *SyntaxTree::request_implicit_class_optional(const Class *param, int token_id) {
	string name = class_name_might_need_parantheses(param) + "?";
	return request_implicit_class(name, Class::Type::OPTIONAL, param->size + 1, 0, nullptr, {param}, token_id);
}

shared<Node> SyntaxTree::conv_cbr(shared<Node> c, Variable *var) {
	// convert
	if ((c->kind == NodeKind::VAR_LOCAL) and (c->as_local() == var)) {
		auto r = add_node_local(var);
		r->set_type(get_pointer(c->type, c->token_id));
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
				c->set_param(0, c->params[0]->ref(this));
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
				int param_offset = 0;//f->is_static() ? 0 : 1;
				// TODO does g++ keep const refs to instance?!
				if ((j >= param_offset) and (flags_has(f->var[j]->flags, Flags::OUT)))
					return true;
			}
			return false;
		};

		// parameters, instance: class as reference
		for (int j=0;j<c->params.num;j++)
			if (c->params[j] and needs_conversion(j)) {
				r->set_param(j, c->params[j]->ref(this));
				changed = true;
			}

		// return: array reference (-> dereference)
		if (c->type->is_array() /*or c->type->is_super_array()*/) {
			r->set_type(get_pointer(c->type, c->token_id));
			return r->deref();
		}
		if (changed)
			return r;
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
			auto r = new Node(kind, 0, c->type, c->flags);
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
		if (v->name == Identifier::RETURN_VAR) {
			p_ret = add_node_local(v);
		}
	if (!p_ret)
		do_error("-return- not found...");
	auto ret = p_ret->deref();
	auto cmd_assign = parser->con.link_operator_id(OperatorID::ASSIGN, ret, n->params[0]);
	if (!cmd_assign)
		do_error(format("no operator '%s = %s' for return from function found: '%s'", ret->type->long_name(), n->params[0]->type->long_name(), f->long_name()));
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
				v->type = get_pointer(v->type, -1);

				// usage inside the function
				transform_block(f->block.get(), [this, v](shared<Node> n) {
					return conv_cbr(n, v);
				});
			}
	}

	// convert return...
	for (Function *f: functions)
		if (f->literal_return_type->uses_return_by_memory() and (f->literal_return_type != TypeUnknown))
			//convert_return_by_memory(this, f->block, f);
			transform_block(f->block.get(), [this, f](shared<Node> n) {
				return conv_return_by_memory(n, f);
			});

	// convert function calls
	transform([this](shared<Node> n) {
		return conv_calls(n);
	});
}


void SyntaxTree::simplify_ref_deref() {
	// remove &*
	transform([this] (shared<Node> n) {
		return conv_easyfy_ref_deref(n, 0);
	});
}

void SyntaxTree::simplify_shift_deref() {
	// remove &*
	transform([this] (shared<Node> n) {
		return conv_easyfy_shift_deref(n, 0);
	});
}

InlineID __get_pointer_add_int() {
	if (config.target.instruction_set == Asm::InstructionSet::AMD64)
		return InlineID::INT64_ADD_INT;
	return InlineID::INT_ADD;
}


shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c) {
	if (c->kind == NodeKind::DYNAMIC_ARRAY) {
		return tree->conv_break_down_low_level(
				add_node_parray(
						c->params[0]->change_type(tree->get_pointer(c->type, c->token_id)),
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
				c->params[0]->ref(this), // array
				add_node_operator_by_inline(InlineID::INT_MULTIPLY,
						c->params[1], // ref
						add_node_const(add_constant_int(el_type->size))),
				c->token_id,
				get_pointer(el_type, c->token_id))->deref();

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
				get_pointer(el_type, c->token_id))->deref();
	} else if (c->kind == NodeKind::ADDRESS_SHIFT) {

		auto *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

		//if (c->link_no == 0)
		//	return c->params[0]->ref(this)->deref(el_type);
		// FIXME this causes a bug, probably by omtimizing away *(&x) and changing types

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0]->ref(this), // struct
				add_node_const(add_constant_int(c->link_no)),
				c->token_id,
				get_pointer(el_type, c->token_id))->deref();

	} else if (c->kind == NodeKind::DEREF_ADDRESS_SHIFT) {

		auto *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		//if (c->link_no == 0)
		//	return c->params[0]->deref(el_type);

		// create command for shift constant
		// address = &struct + shift
		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0], // ref struct
				add_node_const(add_constant_int(c->link_no)),
				c->token_id,
				get_pointer(el_type, c->token_id))->deref();
	}
	return c;
}


shared<Node> SyntaxTree::transform_node(shared<Node> n, std::function<shared<Node>(shared<Node>)> F) {
	if (n->kind == NodeKind::BLOCK) {
		transform_block(n->as_block(), F);
		return F(n);
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
		if (!f->is_template() and !f->is_macro()) {
			parser->cur_func = f;
			transform_block(f->block.get(), F);
		}
}
void SyntaxTree::transformb(std::function<shared<Node>(shared<Node>, Block*)> F) {
	for (Function *f: functions)
		if (!f->is_template() and !f->is_macro()) {
			parser->cur_func = f;
			transformb_block(f->block.get(), F);
		}
}

bool node_is_executable(shared<Node> n) {
	if ((n->kind == NodeKind::CONSTANT) or (n->kind == NodeKind::VAR_LOCAL) or (n->kind == NodeKind::VAR_GLOBAL))
		return false;
	if ((n->kind == NodeKind::ADDRESS_SHIFT) or (n->kind == NodeKind::ARRAY) or (n->kind == NodeKind::DYNAMIC_ARRAY)
			or (n->kind == NodeKind::REFERENCE) or (n->kind == NodeKind::DEREFERENCE)
			or (n->kind == NodeKind::DEREF_ADDRESS_SHIFT))
		return node_is_executable(n->params[0]);
	return true;
}

shared<Node> SyntaxTree::conv_fake_constructors(shared<Node> n) {
	if (n->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION)
		return n;
	if ((n->type == TypeVec3) or (n->type == TypeVec2) or (n->type == TypeColor) or (n->type == TypeRect) or (n->type == TypeComplex)) {
		return make_constructor_static(n, "_create");
	}
	if (n->type == TypeQuaternion) {
		if (n->params.num == 2 and n->params[1]->type == TypeVec3)
			return make_constructor_static(n, "_rotation_v");
		if (n->params.num == 3 and n->params[1]->type == TypeVec3)
			return make_constructor_static(n, "_rotation_a");
		if (n->params.num == 2 and n->params[1]->type == TypeMat4)
			return make_constructor_static(n, "_rotation_m");
	}
	return n;
}

shared<Node> SyntaxTree::conv_class_and_func_to_const(shared<Node> n) {
	if (n->kind == NodeKind::CLASS) {
		return add_node_const(add_constant_pointer(TypeClassRef, n->as_class()));
	}
	if (n->kind == NodeKind::SPECIAL_FUNCTION_NAME) {
		return add_node_const(add_constant_pointer(TypeSpecialFunctionRef, n->as_special_function()));
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
	} else if ((n->kind == NodeKind::STATEMENT) and (n->as_statement()->id == StatementID::FOR_CONTAINER)) {

		// [VAR, INDEX, ARRAY, BLOCK]
		auto var = n->params[0];
		auto index = n->params[1];
		auto array = n->params[2];
		auto block = n->params[3];
		
		
		// array needs execution?
		if (node_is_executable(array)) {
			// -> assign into variable before the loop
			auto *v = b->add_var(b->function->create_slightly_hidden_name(), array->type);

			auto assign = parser->con.link_operator_id(OperatorID::ASSIGN, add_node_local(v), array);
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
		if (array->type->usable_as_list()) {
			// array.num
			val1 = array->shift(config.target.pointer_size, TypeInt, array->token_id);
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
		if (array->type->usable_as_list()) {
			el = add_node_dyn_array(array, index);
		} else {
			el = add_node_array(array, index);
		}

		// &for_var = &array[index]
		auto cmd_var_assign = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, var, el->ref(this));
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

	} else if ((n->kind == NodeKind::STATEMENT) and (n->as_statement()->id == StatementID::RAW_FUNCTION_POINTER)) {
		// only extract explicit raw_function_pointer()
		// skip implicit from callable...
		if (n->params[0]->kind == NodeKind::CONSTANT) {
			n->params[0]->as_const()->type = TypeFunctionCodeRef;
			return n->params[0];
		}
	}

	// TODO experimental dynamic type insertion
	if (false and is_func(n)) {
		auto f = n->as_func();
		for (int i=0; i<f->num_params; i++)
			if (f->literal_param_type[i] == TypeDynamic) {
				msg_error("conv dyn!");
				auto c = add_constant(TypeClassRef);
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
			auto r = new Node(NodeKind::CALL_INLINE, n->link_no, n->type, n->flags);
			r->params = n->params;
			return r;
		}
	}
	if (n->kind == NodeKind::OPERATOR) {
		Operator *op = n->as_op();
		if (op->f->inline_no != InlineID::NONE) {
			auto r = new Node(NodeKind::CALL_INLINE, (int_p)op->f, n->type, n->flags);
			r->params = n->params;
			return r;
		} else {
			auto r = new Node(NodeKind::CALL_FUNCTION, (int_p)op->f, n->type, n->flags);
			r->params = n->params;
			return r;
		}
	}
	return n;
}


void MapLVSX86Return(Function *f, int64 &stack_offset) {
	for (auto &v: f->var)
		if (v->name == Identifier::RETURN_VAR) {
			v->_offset = stack_offset;
			stack_offset += config.target.pointer_size;
		}
}

void MapLVSX86Self(Function *f, int64 &stack_offset) {
	for (auto &v: f->var)
		if (v->name == Identifier::SELF) {
			v->_offset = stack_offset;
			stack_offset += config.target.pointer_size;
		}
}

void SyntaxTree::map_local_variables_to_stack() {
	for (Function *f: functions) {

		if (config.target.instruction_set == Asm::InstructionSet::X86) {
			f->_var_size = 0;
			int64 stack_offset = 2 * config.target.pointer_size; // space for eIP and eBP
			// offsets to stack pointer (for push parameters)

			if (config.target.abi == Abi::X86_WINDOWS) {
				// map "self" to the VERY first parameter
				if (f->is_member())
					MapLVSX86Self(f, stack_offset);

				// map "-return-" to the first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);
			} else {
				// map "-return-" to the VERY first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);

				// map "self" to the first parameter
				if (f->is_member())
					MapLVSX86Self(f, stack_offset);
			}

			for (auto&& [i,v]: enumerate(weak(f->var))) {
				if (f->is_member() and (v->name == Identifier::SELF))
					continue;
				if (v->name == Identifier::RETURN_VAR)
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
		} else if (config.target.instruction_set == Asm::InstructionSet::AMD64) {
			f->_var_size = 0;
			int64 stack_offset = 2 * config.target.pointer_size; // space for rIP and rBP
			// offsets to stack pointer (for push parameters)

			if (config.target.abi == Abi::AMD64_WINDOWS) {

				// map "self" to the VERY first parameter
				if (f->is_member())
					MapLVSX86Self(f, stack_offset);

				// map "-return-" to the first parameter
				if (f->literal_return_type->uses_return_by_memory())
					MapLVSX86Return(f, stack_offset);

				for (auto&& [i,v]: enumerate(weak(f->var))) {
					if (f->is_member() and (v->name == Identifier::SELF))
						continue;
					if (v->name == Identifier::RETURN_VAR)
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
				for (auto v: weak(f->var)) {
					int64 s = mem_align(v->type->size, 4);
					f->_var_size += s;
					v->_offset = -f->_var_size;
				}
			}
		} else if (config.target.is_arm()) {
			f->_var_size = 0;

			for (auto v: weak(f->var)) {
				int s = mem_align(v->type->size, 4);
				v->_offset = f->_var_size;// + s;
				f->_var_size += s;
			}
		}
	}
}

void delete_all_constants(Class *c) {
	for (auto cc: weak(c->classes)) {
		auto ccc = const_cast<Class*>(cc);
		ccc->constants.clear();
		delete_all_constants(ccc);
	}
}

// no included modules may be deleted before us!!!
SyntaxTree::~SyntaxTree() {
	// delete all classes, functions etc created by this module

	delete_all_constants(base_class);

	module->context->template_manager->clear_from_module(module);
	module->context->implicit_class_registry->clear_from_module(module);
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
