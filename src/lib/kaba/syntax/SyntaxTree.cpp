#include "../kaba.h"
#include "../parser/Parser.h"
#include "../dynamic/exception.h"
#include "../../os/msg.h"
#include "../../base/iter.h"

namespace kaba {

//#define ScriptDebug


extern const Class *TypeMat4;
extern const Class *TypeVec2;
extern const Class *TypeSpecialFunctionRef;

bool is_func(shared<Node> n);

static shared_array<Node> _transform_insert_before_;
shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c);

int dict_row_size(const Class *t_val);

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
	if (find(name, -1).num > 0)
		return false;
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

	base_class = new Class(TypeNamespaceT, "-base-", 0, 1, this);
	_base_class = base_class;
	implicit_symbols = new Class(TypeNamespaceT, "-implicit-", 0, 1, this);
	root_of_all_evil = new Function("-root-", TypeVoid, base_class, Flags::Static);
}

void SyntaxTree::default_import() {
	for (auto p: module->context->internal_packages)
		if (p->used_by_default)
			import_data_all(p->base_class(), -1);
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
	auto *c = add_constant(TypeInt32);
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
	for (int i=0; i<(int)OperatorID::_Count_; i++)
		if ((name == abstract_operators[i].name) and ((int)param_flags == (abstract_operators[i].flags & OperatorFlags::Binary)))
			return &abstract_operators[i];

	// old hack
	if (name == "!")
		return &abstract_operators[(int)OperatorID::Negate];

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
		auto n = new Node(NodeKind::STATEMENT, (int_p)s, TypeVoid);
		n->set_num_params(s->num_params);
		return {n};
	}*/

	// operators
	if (auto w = parser->which_abstract_operator(name, OperatorFlags::UnaryRight)) // negate/not...
		return {new Node(NodeKind::AbstractOperator, (int_p)w, TypeUnknown, Flags::None, token_id)};

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

shared<Node> SyntaxTree::conv_cbr(shared<Node> c, Variable *var) {
	// convert
	if ((c->kind == NodeKind::VarLocal) and (c->as_local() == var)) {
		auto r = add_node_local(var);
		r->set_type(type_ref(c->type, c->token_id));
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
	if ((c->kind == NodeKind::Statement) and (c->as_statement()->id == StatementID::Return))
		if (c->params.num > 0) {
			if ((c->params[0]->type->is_array()) /*or (c->Param[j]->Type->IsSuperArray)*/) {
				c->set_param(0, c->params[0]->ref(this));
			}
			return c;
		}

	if (c->is_call() or (c->kind == NodeKind::ConstructorAsFunction)) {
		auto r = c->shallow_copy();
		bool changed = false;

		auto needs_conversion = [&c] (int j) {
			if (c->params[j]->type->uses_call_by_reference())
				return true;
			if (c->is_function()) {
				auto f = c->as_func();
				int param_offset = 0;//f->is_static() ? 0 : 1;
				// TODO does g++ keep const refs to instance?!
				if ((j >= param_offset) and (flags_has(f->var[j]->flags, Flags::Out)))
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
			r->set_type(type_ref(c->type, c->token_id));
			return r->deref();
		}
		if (changed)
			return r;
	}
	return c;
}


// remove &*x
shared<Node> SyntaxTree::conv_easyfy_ref_deref(shared<Node> c, int l) {
	if (c->kind == NodeKind::Reference) {
		if (c->params[0]->kind == NodeKind::Dereference) {
			// remove 2 knots...
			return c->params[0]->params[0];
		}
	}
	return c;
}

// remove (*x)[] and (*x).y
shared<Node> SyntaxTree::conv_easyfy_shift_deref(shared<Node> c, int l) {
	if ((c->kind == NodeKind::AddressShift) or (c->kind == NodeKind::Array)) {
		if (c->params[0]->kind == NodeKind::Dereference) {
			// unify 2 knots (remove 1)
			c->show(TypeVoid);
			auto kind = (c->kind == NodeKind::AddressShift) ? NodeKind::DereferenceAddressShift : NodeKind::PointerAsArray;
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

	if ((n->kind != NodeKind::Statement) or (n->as_statement()->id != StatementID::Return))
		return n;

	// convert into   *-return- = param
	shared<Node> p_ret;
	for (Variable *v: weak(f->var))
		if (v->name == Identifier::ReturnVar) {
			p_ret = add_node_local(v);
		}
	if (!p_ret)
		do_error("-return- not found...");
	auto ret = p_ret->deref();
	auto cmd_assign = parser->con.link_operator_id(OperatorID::Assign, ret, n->params[0]);
	if (!cmd_assign)
		do_error(format("no operator '%s = %s' for return from function found: '%s'", ret->type->long_name(), n->params[0]->type->long_name(), f->long_name()));
	_transform_insert_before_.add(cmd_assign);

	return add_node_statement(StatementID::Return);
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
			if (v->type->uses_call_by_reference() or flags_has(v->flags, Flags::Out)) {
				v->type = type_ref(v->type, -1);

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
	if (config.target.pointer_size == 8)
		return InlineID::Int64AddInt32;
	return InlineID::Int32Add;
}


shared<Node> conv_break_down_med_level(SyntaxTree *tree, shared<Node> c) {
	if (c->kind == NodeKind::DynamicArray) {
		return tree->conv_break_down_low_level(
				add_node_parray(
						c->params[0]->change_type(tree->type_ref(c->type, c->token_id)),
						c->params[1], c->type));
	}
	return c;
}

shared<Node> SyntaxTree::conv_break_down_low_level(shared<Node> c) {

	if (c->kind == NodeKind::Array) {

		auto *el_type = c->type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0]->ref(this), // array
				add_node_operator_by_inline(InlineID::Int32Multiply,
						c->params[1], // ref
						add_node_const(add_constant_int(el_type->size))),
				c->token_id,
				type_ref(el_type, c->token_id))->deref();

	} else if (c->kind == NodeKind::PointerAsArray) {

		auto *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

		return add_node_operator_by_inline(__get_pointer_add_int(),
				c->params[0], // ref array
				add_node_operator_by_inline(InlineID::Int32Multiply,
						c->params[1], // index
						add_node_const(add_constant_int(el_type->size))),
				c->token_id,
				type_ref(el_type, c->token_id))->deref();
	} else if (c->kind == NodeKind::AddressShift) {

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
				type_ref(el_type, c->token_id))->deref();

	} else if (c->kind == NodeKind::DereferenceAddressShift) {

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
				type_ref(el_type, c->token_id))->deref();
	}
	return c;
}


shared<Node> SyntaxTree::transform_node(shared<Node> n, std::function<shared<Node>(shared<Node>)> F) {
	if (n->kind == NodeKind::Block) {
		transform_block(n->as_block(), F);
		return F(n);
	} else {
		shared<Node> r = n;
		for (int i=0; i<n->params.num; i++)
			if (n->params[i]) {
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
	if (n->kind == NodeKind::Block) {
		transformb_block(n->as_block(), F);
		return F(n, b);
	} else {
		shared<Node> r = n;
		for (int i=0; i<n->params.num; i++)
			if (n->params[i]) {
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
	if ((n->kind == NodeKind::Constant) or (n->kind == NodeKind::VarLocal) or (n->kind == NodeKind::VarGlobal))
		return false;
	if ((n->kind == NodeKind::AddressShift) or (n->kind == NodeKind::Array) or (n->kind == NodeKind::DynamicArray)
			or (n->kind == NodeKind::Reference) or (n->kind == NodeKind::Dereference)
			or (n->kind == NodeKind::DereferenceAddressShift))
		return node_is_executable(n->params[0]);
	return true;
}

shared<Node> SyntaxTree::conv_fake_constructors(shared<Node> n) {
	if (n->kind != NodeKind::ConstructorAsFunction)
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
	if (n->kind == NodeKind::Class) {
		return add_node_const(add_constant_pointer(TypeClassRef, n->as_class()));
	}
	if (n->kind == NodeKind::SpecialFunctionName) {
		return add_node_const(add_constant_pointer(TypeSpecialFunctionRef, n->as_special_function()));
	}
	return n;
}


shared<Node> SyntaxTree::conv_break_down_high_level(shared<Node> n, Block *b) {
	if (n->kind == NodeKind::ConstructorAsFunction) {
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
	} else if (n->kind == NodeKind::ArrayBuilder) {
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
	} else if (n->kind == NodeKind::DictBuilder) {
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
	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::ForRange)) {

		// [VAR, START, STOP, STEP, BLOCK]
		auto var = n->params[0];
		auto val0 = n->params[1];
		auto val1 = n->params[2];
		auto step = n->params[3];
		auto block = n->params[4];


		auto nn = add_node_statement(StatementID::ForDigest);
		nn->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]

		// assign
		nn->set_param(0, add_node_operator_by_inline(InlineID::Int32Assign, var, val0));

		// while(for_var < val1)
		nn->set_param(1, add_node_operator_by_inline(InlineID::Int32Smaller, var, val1));

		nn->set_param(2, block);


		// ...for_var += 1
		shared<Node> cmd_inc;
		if (var->type == TypeInt32) {
			if (step->as_const()->as_int() == 1)
				cmd_inc = add_node_operator_by_inline(InlineID::Int32Increase, var, nullptr);
			else
				cmd_inc = add_node_operator_by_inline(InlineID::Int32AddAssign, var, step);
		} else {
			cmd_inc = add_node_operator_by_inline(InlineID::Float32AddAssign, var, step);
		}
		nn->set_param(3, cmd_inc); // add to loop-block

		return nn;
	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::ForContainer)) {

		// [VAR, INDEX, ARRAY, BLOCK]
		auto var = n->params[0];
		auto key = n->params[1];
		auto array = n->params[2];
		auto block = n->params[3];
		auto index = key;
		
		
		// array needs execution?
		if (node_is_executable(array)) {
			// -> assign into variable before the loop
			auto *v = b->add_var(b->function->create_slightly_hidden_name(), array->type);

			auto assign = parser->con.link_operator_id(OperatorID::Assign, add_node_local(v), array);
			_transform_insert_before_.add(assign);

			array = add_node_local(v);
		}

		auto nn = add_node_statement(StatementID::ForDigest);
		nn->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]

		if (array->type->is_dict()) {
			static int for_index_count = 0;
			string index_name = format("-for_dict_index_%d-", for_index_count++);
			index = add_node_local(b->add_var(index_name, TypeInt32));
		}


		// 0
		auto val0 = add_node_const(add_constant_int(0));

		// implement
		// for_index = 0
		nn->set_param(0, add_node_operator_by_inline(InlineID::Int32Assign, index, val0));

		shared<Node> val1;
		if (array->type->usable_as_list() or array->type->is_dict()) {
			// array.num
			val1 = array->shift(config.target.pointer_size, TypeInt32, array->token_id);
		} else {
			// array.size
			val1 = add_node_const(add_constant_int(array->type->array_length));
		}

		// while(for_index < val1)
		nn->set_param(1, add_node_operator_by_inline(InlineID::Int32Smaller, index, val1));

		// ...block
		nn->set_param(2, block);

		// ...for_index += 1
		nn->set_param(3, add_node_operator_by_inline(InlineID::Int32Increase, index, nullptr));

		if (array->type->is_dict()) {

			auto row = add_node_operator_by_inline(__get_pointer_add_int(),
					array->change_type(TypeReference),
					add_node_operator_by_inline(InlineID::Int32Multiply,
							index,
							add_node_const(add_constant_int(dict_row_size(array->type->param[0])))),
					n->token_id,
					TypeReference)->deref();


			// &for_var = &row.value
			auto cmd_var_assign = add_node_operator_by_inline(InlineID::PointerAssign, var, row->shift(TypeString->size, array->type->param[0])->ref(this));
			block->params.insert(cmd_var_assign, 0);

			// &for_var = &row.value
			auto cmd_key_assign = add_node_operator_by_inline(InlineID::PointerAssign, key, row->change_type(TypeString)->ref(this));
			block->params.insert(cmd_key_assign, 0);
		} else {

			// array[index]
			shared<Node> el;
			if (array->type->usable_as_list()) {
				el = add_node_dyn_array(array, index);
			} else {
				el = add_node_array(array, index);
			}

			// &for_var = &array[index]
			auto cmd_var_assign = add_node_operator_by_inline(InlineID::PointerAssign, var, el->ref(this));
			block->params.insert(cmd_var_assign, 0);
		}

		return nn;
	} else if (n->kind == NodeKind::ArrayBuilderFor) {

		_transform_insert_before_.add(n->params[0]);
		return n->params[1];
	} else if (n->kind == NodeKind::TupleExtraction) {

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

	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::Match)) {

		auto *vv = b->add_var(b->function->create_slightly_hidden_name(), n->params[0]->type);
		auto temp = add_node_local(vv);

		Block *bb = new Block(b->function, b);
		bb->type = n->type;
		auto& ai = parser->auto_implementer;
		bb->add(ai.add_assign(b->function, "", temp, n->params[0]));

		const int ncases = (n->params.num - 1) / 2;
		shared<Node> cmd;
		for (int i=ncases-1; i>=0; i--) {
			const bool is_last = (i == ncases - 1);
			if (is_last and n->params[1 + 2*i]->kind == NodeKind::Statement) {
				// default?
				cmd = n->params[2 + 2*i];
				continue;
			}
			auto cmd_if = add_node_statement(StatementID::If, n->token_id, n->type);
			cmd_if->set_num_params(is_last ? 2 : 3);
			cmd_if->set_param(0, ai.add_equal(b->function, "", temp, n->params[1 + 2*i]));
			cmd_if->set_param(1, n->params[2 + 2*i]);
			// BLOCKS!!!!!
			if (!is_last)
				cmd_if->set_param(2, cmd);
			cmd = cmd_if;
		}
		bb->add(cmd);

		return bb;

	} else if ((n->kind == NodeKind::Statement) and (n->as_statement()->id == StatementID::RawFunctionPointer)) {
		// only extract explicit raw_function_pointer()
		// skip implicit from callable...
		if (n->params[0]->kind == NodeKind::Constant) {
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
	if (n->kind == NodeKind::CallFunction) {
		if (n->as_func()->inline_no != InlineID::None) {
			auto r = new Node(NodeKind::CallInline, n->link_no, n->type, n->flags);
			r->params = n->params;
			return r;
		}
	}
	if (n->kind == NodeKind::Operator) {
		Operator *op = n->as_op();
		if (op->f->inline_no != InlineID::None) {
			auto r = new Node(NodeKind::CallInline, (int_p)op->f, n->type, n->flags);
			r->params = n->params;
			return r;
		} else {
			auto r = new Node(NodeKind::CallFunction, (int_p)op->f, n->type, n->flags);
			r->params = n->params;
			return r;
		}
	}
	return n;
}


void MapLVSX86Return(Function *f, int64 &stack_offset) {
	for (auto &v: f->var)
		if (v->name == Identifier::ReturnVar) {
			v->_offset = stack_offset;
			stack_offset += config.target.pointer_size;
		}
}

void MapLVSX86Self(Function *f, int64 &stack_offset) {
	for (auto &v: f->var)
		if (v->name == Identifier::Self) {
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
				if (f->is_member() and (v->name == Identifier::Self))
					continue;
				if (v->name == Identifier::ReturnVar)
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
					if (f->is_member() and (v->name == Identifier::Self))
						continue;
					if (v->name == Identifier::ReturnVar)
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
				int align = (v->type->size > 4) ? 8 : 4;
				int s = mem_align((int)v->type->size, align);
				v->_offset = mem_align(f->_var_size, align);
				f->_var_size = v->_offset + s;
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
