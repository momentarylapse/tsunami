#include "Class.h"
#include "../kaba.h"
#include "../parser/Parser.h"
#include "../dynamic/exception.h"
#include "../../os/msg.h"
#include "../../base/iter.h"
#include "lib/kaba/parser/Transformer.h"

namespace kaba {

//#define ScriptDebug



bool is_func(shared<Node> n);

shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c);

int dict_row_size(const Class *t_val);



	InlineID __get_pointer_add_int();

shared_array<Node> Scope::find(const string &name, int token_id) const {
	shared_array<Node> r;
	for (auto &e: entries)
		if (e.name == name) {
			if (e.kind == NodeKind::Class)
				r.add(add_node_class(reinterpret_cast<const Class *>(e.p), token_id));
			if (e.kind == NodeKind::Function)
				r.add(add_node_func_name(reinterpret_cast<const Function *>(e.p), token_id));
			if (e.kind == NodeKind::VarGlobal)
				r.add(add_node_global(reinterpret_cast<const Variable *>(e.p), token_id));
			if (e.kind == NodeKind::Constant)
				r.add(add_node_const(reinterpret_cast<const Constant *>(e.p), token_id));
		}
	return r;
}

bool Scope::add_class(const string &name, const Class *c) {
	for (const auto& x: find(name, -1)) {
		if (x->kind == NodeKind::Class and x->as_class() == c)
			return true;
		return false;
	}
	entries.add(Entry{name, NodeKind::Class, c});
	return true;
}

bool Scope::add_function(const string &name, const Function *f) {
	//if (find(name, -1).num > 0)
	//	msg_error("NAME ALREADY IN SCOPE (func): " + name);
	entries.add(Entry{name, NodeKind::Function, f});
	return true;
}

bool Scope::add_variable(const string &name, const Variable *v) {
	if (find(name, -1).num > 0)
		return false;
	entries.add(Entry{name, NodeKind::VarGlobal, v});
	return true;
}

bool Scope::add_const(const string &name, const Constant *c) {
	if (find(name, -1).num > 0)
		return false;
	entries.add(Entry{name, NodeKind::Constant, c});
	return true;
}



const Class *SyntaxTree::request_implicit_class_callable_fp(Function *f, int token_id) {
	return request_implicit_class_callable_fp(f->literal_param_type, f->literal_return_type, token_id);
}

// input {}->R  OR  void->void   BOTH create  void->R
const Class *SyntaxTree::request_implicit_class_callable_fp(const Array<const Class*> &param, const Class *ret, int token_id) {
	return module->context->template_manager->request_callable_fp(this, param, ret, token_id);
}

// inner callable: params [A,B,C,D,E]
// captures: [-,x0,-,-,x1]
// class CallableBind
//     func __init__(f, c0, c1)
//     func call(a,b,c)
//         f(a,x0,b,c,c1)
// (A,C,D) -> R
const Class *SyntaxTree::request_implicit_class_callable_bind(const Array<const Class*> &params, const Class *ret, const Array<const Class*> &captures, const Array<bool> &capture_via_ref, int token_id) {
	return module->context->template_manager->request_callable_bind(this, params, ret, captures, capture_via_ref, token_id);
}

SyntaxTree::SyntaxTree(Module *_module) {
	flag_string_const_as_cstring = false;
	flag_function_pointer_as_code = false;
	flag_immortal = false;
	module = _module;
	asm_meta_info = new Asm::MetaInfo(config.target.pointer_size);

	base_class = new Class(common_types.namespace_t, "-base-", 0, 1, this);
	_base_class = base_class;
	implicit_symbols = new Class(common_types.namespace_t, "-implicit-", 0, 1, this);
	root_of_all_evil = new Function("-root-", common_types._void, base_class, Flags::Static);
}

void SyntaxTree::default_import() {
	for (auto p: weak(module->context->internal_packages))
		if (p->auto_import)
			import_data_all(p->main_module->base_class(), -1);
}

void SyntaxTree::digest() {
	Transformer t(this);
	t.fully_transform();
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



Constant *SyntaxTree::add_constant(const Class* type, int token_id, Class* name_space) {
	if (!name_space)
		name_space = base_class;
	auto *c = new Constant(type, this, token_id);
	name_space->constants.add(c);
	return c;
}

Constant *SyntaxTree::add_constant_int(int value, int token_id) {
	auto *c = add_constant(common_types.i32, token_id);
	c->as_int() = value;
	return c;
}

Constant *SyntaxTree::add_constant_pointer(const Class *type, const void *value, int token_id) {
	auto *c = add_constant(type, token_id);
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

const Class *SyntaxTree::which_owned_class(const string &name) {
	for (auto *c: weak(base_class->classes))
		if (name == c->name)
			return c;
	return nullptr;
}

shared_array<Node> SyntaxTree::get_existence_global(const string &name, const Class *ns, int token_id) {
	shared_array<Node> links;


	// recursively up the namespaces
	while (ns) {
		/*for (const auto&& [_name, t]: ns->type_aliases)
			if (_name == name)
				return {add_node_class(t, token_id)};*/

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
		if (name == Identifier::SelfClass)
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

	if (operand->kind == NodeKind::Class) {
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
	if (type->parent and (name == Identifier::Super)) {
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
		auto self = add_node_local(f->__get_var(Identifier::Self), token_id);
		auto links = get_element_of(self, name, token_id);
		if (links.num > 0)
			return links;
	}

	for (auto *v: weak(f->name_space->static_variables))
		if (v->name == name)
			return {add_node_global(v)};

	if (name == Identifier::ReturnClass)
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
		auto n = new Node(NodeKind::STATEMENT, (int_p)s, common_types._void);
		n->set_num_params(s->num_params);
		return {n};
	}*/

	// operators
	if (auto w = parser->which_abstract_operator(name, OperatorFlags::UnaryRight)) // negate/not...
		return {new Node(NodeKind::AbstractOperator, (int_p)w, common_types.unknown, Flags::None, token_id)};

	// in include files (only global)...
	links.append(global_scope.find(name, token_id));

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
		// FIXME double checking non-imported...
		for (auto n: global_scope.find(name, -1))
			if (n->kind == NodeKind::Class)
				return n->as_class();
	} else if (_namespace->name_space and allow_recursion) {
		// parent namespace
		return find_root_type_by_name(name, _namespace->name_space, true);
	}
	return nullptr;
}

// used by "class/enum XYZ"
Class *SyntaxTree::create_new_class(const string &name, const Class* from_template, int size, int array_size, const Class *parent, const Array<const Class*> &params, Class *ns, int token_id) {
	if (find_root_type_by_name(name, ns, false))
		do_error(format("class '%s' already exists", name), token_id);
	return create_new_class_no_check(name, from_template, size, array_size, parent, params, ns, token_id);
}


Class *SyntaxTree::create_new_class_no_check(const string &name, const Class* from_template, int size, int array_size, const Class *parent, const Array<const Class*> &params, Class *ns, int token_id) {
	//msg_write("CREATE " + name);

	Class *t = new Class(from_template, name, size, 1, this, parent, params);
	t->token_id = token_id;
	t->array_length = array_size;
	owned_classes.add(t);
	
	// link namespace
	ns->classes.add(t);
	t->name_space = ns;
	return t;
}

const Class *SyntaxTree::get_pointer(const Class *base, int token_id) {
	return request_implicit_class_pointer(base, token_id);
}

const Class *SyntaxTree::type_ref(const Class *base, int token_id) {
	return request_implicit_class_reference(base, token_id);
}

const Class *SyntaxTree::request_implicit_class_pointer(const Class *base, int token_id) {
	return module->context->template_manager->request_pointer(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_shared(const Class *base, int token_id) {
	return module->context->template_manager->request_shared(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_shared_not_null(const Class *base, int token_id) {
	return module->context->template_manager->request_shared_not_null(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_owned(const Class *base, int token_id) {
	return module->context->template_manager->request_owned(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_owned_not_null(const Class *base, int token_id) {
	return module->context->template_manager->request_owned_not_null(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_xfer(const Class *base, int token_id) {
	return module->context->template_manager->request_xfer(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_alias(const Class *base, int token_id) {
	return module->context->template_manager->request_alias(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_reference(const Class *base, int token_id) {
	return module->context->template_manager->request_reference(this, base, token_id);
}

const Class *SyntaxTree::request_implicit_class_list(const Class *element_type, int token_id) {
	return module->context->template_manager->request_list(this, element_type, token_id);
}

const Class *SyntaxTree::request_implicit_class_array(const Class *element_type, int num_elements, int token_id) {
	return module->context->template_manager->request_array(this, element_type, num_elements, token_id);
}

const Class *SyntaxTree::request_implicit_class_dict(const Class *element_type, int token_id) {
	return module->context->template_manager->request_dict(this, element_type, token_id);
}

const Class *SyntaxTree::request_implicit_class_optional(const Class *param, int token_id) {
	return module->context->template_manager->request_optional(this, param, token_id);
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
