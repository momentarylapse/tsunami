#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Kaba{

//#define ScriptDebug


extern const Class *TypeDynamicArray;
extern const Class *TypeDictBase;
extern const Class *TypeMatrix;



static Array<Node*> _transform_insert_before_;
Node *conv_break_down_med_level(SyntaxTree *tree, Node *c);


string Operator::sig() const {
	if (param_type_1 and param_type_2)
		return "(" + param_type_1->name + ") " + primitive->name + " (" + param_type_2->name + ")";
	if (param_type_1)
		return "(" + param_type_1->name + ") " + primitive->name;
	return primitive->name + " (" + param_type_2->name + ")";
}


Node *SyntaxTree::cp_node(Node *c) {
	Node *cmd;
	if (c->kind == NodeKind::BLOCK)
		cmd = new Block(c->as_block()->function, c->as_block()->parent);
	else
		cmd = new Node(c->kind, c->link_no, c->type);
	cmd->is_const = c->is_const;
	cmd->set_num_params(c->params.num);
	for (int i=0;i<c->params.num;i++)
		if (c->params[i])
			cmd->set_param(i, cp_node(c->params[i]));
	return cmd;
}

const Class *SyntaxTree::make_class_func(Function *f) {
	return TypeFunctionCodeP;
	string params;
	for (int i=0; i<f->num_params; i++) {
		if (i > 0)
			params += ",";
		params += f->literal_param_type[i]->name;
	}
	return make_class("func(" + params + ")->" + f->return_type->name, Class::Type::POINTER, config.pointer_size, 0, nullptr, TypeVoid, base_class);

	return TypePointer;
}

Node *SyntaxTree::ref_node(Node *sub, const Class *override_type) {
	const Class *t = override_type ? override_type : sub->type->get_pointer();

	/*if (sub->kind == NodeKind::DEREF_ADDRESS_SHIFT) {
		sub->kind = NodeKind::ADDRESS_SHIFT;
		sub->type = t;
		return sub;
	}*/

	Node *c = new Node(NodeKind::REFERENCE, 0, t);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Node *SyntaxTree::deref_node(Node *sub, const Class *override_type) {
	Node *c = new Node(NodeKind::UNKNOWN, 0, TypeVoid, sub->is_const);
	c->kind = NodeKind::DEREFERENCE;
	c->set_num_params(1);
	c->set_param(0, sub);
	if (override_type)
		c->type = override_type;
	else
		c->type = sub->type->param;
	return c;
}

Node *SyntaxTree::shift_node(Node *sub, bool deref, int shift, const Class *type) {
	Node *c = new Node(deref ? NodeKind::DEREF_ADDRESS_SHIFT : NodeKind::ADDRESS_SHIFT, shift, type, sub->is_const);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Node *SyntaxTree::add_node_statement(StatementID id) {
	auto *s = statement_from_id(id);
	Node *c = new Node(NodeKind::STATEMENT, (int64)s, TypeVoid);
	c->set_num_params(s->num_params);
	return c;
}

// virtual call, if func is virtual
Node *SyntaxTree::add_node_member_call(Function *f, Node *inst, bool force_non_virtual) {
	Node *c;
	if ((f->virtual_index >= 0) and (!force_non_virtual)) {
		c = new Node(NodeKind::VIRTUAL_CALL, (int_p)f, f->literal_return_type, true);
	} else {
		c = new Node(NodeKind::FUNCTION_CALL, (int_p)f, f->literal_return_type, true);
	}
	c->set_num_params(f->num_params + 1);
	c->set_instance(inst);
	return c;
}

// non-member!
Node *SyntaxTree::add_node_call(Function *f) {
	// FIXME: literal_return_type???
	Node *c = new Node(NodeKind::FUNCTION_CALL, (int_p)f, f->return_type, true);
	if (f->is_static())
		c->set_num_params(f->num_params);
	else
		c->set_num_params(f->num_params + 1);
	return c;
}

Node *SyntaxTree::add_node_func_name(Function *f) {
	return new Node(NodeKind::FUNCTION, (int_p)f, TypeFunctionP, true);
}

Node *SyntaxTree::add_node_class(const Class *c) {
	return new Node(NodeKind::CLASS, (int_p)c, TypeClassP, true);
}


Node *SyntaxTree::add_node_operator(Node *p1, Node *p2, Operator *op) {
	Node *cmd = new Node(NodeKind::OPERATOR, (int_p)op, op->return_type, true);
	if (op->primitive->param_flags == 3) {
		cmd->set_num_params(2); // binary
		cmd->set_param(0, p1);
		cmd->set_param(1, p2);
	} else {
		cmd->set_num_params(1); // unary
		cmd->set_param(0, p1);
	}
	return cmd;
}

Node *SyntaxTree::add_node_operator_by_inline(Node *p1, Node *p2, InlineID inline_index) {
	for (auto *op: operators)
		if (op->f->inline_no == inline_index)
			return add_node_operator(p1, p2, op);

	do_error("operator inline index not found: " + i2s((int)inline_index));
	return nullptr;
}


Node *SyntaxTree::add_node_local(Variable *v, const Class *type) {
	return new Node(NodeKind::VAR_LOCAL, (int_p)v, type, v->is_const);
}

Node *SyntaxTree::add_node_local(Variable *v) {
	return new Node(NodeKind::VAR_LOCAL, (int_p)v, v->type, v->is_const);
}

Node *SyntaxTree::add_node_global(Variable *v) {
	return new Node(NodeKind::VAR_GLOBAL, (int_p)v, v->type, v->is_const);
}

Node *SyntaxTree::add_node_parray(Node *p, Node *index, const Class *type) {
	Node *cmd_el = new Node(NodeKind::POINTER_AS_ARRAY, 0, type);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

Node *SyntaxTree::add_node_dyn_array(Node *array, Node *index) {
	Node *cmd_el = new Node(NodeKind::DYNAMIC_ARRAY, 0, array->type->get_array_element());
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, array);
	cmd_el->set_param(1, index);
	return cmd_el;
	//auto *t = array->type;
	//return add_node_parray(shift_node(array, false, 0, t->get_pointer()), index, t->get_array_element());
}

Node *SyntaxTree::add_node_array(Node *array, Node *index) {
	auto *el = new Node(NodeKind::ARRAY, 0, array->type->param);
	el->set_num_params(2);
	el->set_param(0, array);
	el->set_param(1, index);
	return el;
}

/*Node *SyntaxTree::add_node_block(Block *b)
{
	return new Node(NodeKind::BLOCK, (long long)(int_p)b, TypeVoid);
}*/

SyntaxTree::SyntaxTree(Script *_script) {
	base_class = new Class("-base-", 0, this);
	root_of_all_evil = new Function("RootOfAllEvil", TypeVoid, base_class);

	flag_string_const_as_cstring = false;
	flag_function_pointer_as_code = false;
	flag_immortal = false;
	cur_func = nullptr;
	script = _script;
	asm_meta_info = new Asm::MetaInfo;
	for_index_count = 0;
	Exp.cur_line = nullptr;
	parser_loop_depth = 0;

	// "include" default stuff
	for (Script *p: Packages)
		if (p->used_by_default)
			add_include_data(p);
}


void SyntaxTree::parse_buffer(const string &buffer, bool just_analyse) {
	Exp.analyse(this, buffer);
	
	pre_compiler(just_analyse);

	parse();

	Exp.clear();

	if (config.verbose)
		show("parse:a");

}

Node *SyntaxTree::make_constructor_static(Node *n, const string &name) {
	for (auto *f: n->type->functions)
		if (f->name == name) {
			auto nn = add_node_call(f);
			nn->params = n->params.sub(1,-1);
			return nn;
		}
	return n;
}

void SyntaxTree::digest() {
	if (config.verbose)
		show("digest:pre");

	// turn vector(x,y,z) into vector._create(x,y,z)
	transform([&](Node* n){
		if (n->kind != NodeKind::CONSTRUCTOR_AS_FUNCTION)
			return n;
		if ((n->type == TypeVector) or (n->type == TypeColor) or (n->type == TypeRect) or (n->type == TypeComplex)) {
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

	transform([&](Node* n){ return conv_class_and_func_to_const(n); });

	eval_const_expressions();

	transformb([&](Node* n, Block* b){ return conv_break_down_high_level(n, b); });
	if (config.verbose)
		show("digest:break-high");


	transform([&](Node* n){ return conv_break_down_med_level(this, n); });
	transform([&](Node* n){ return conv_break_down_low_level(n); });
	if (config.verbose)
		show("digest:break-low");

	simplify_shift_deref();
	simplify_ref_deref();

	eval_const_expressions();

	if (config.verbose)
		show("digest:pre-proc");

	transform([&](Node* n){ return conv_func_inline(n); });
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

	eval_const_expressions();
	if (config.verbose)
		show("digest:map");


	//pre_processor_addresses();
}


// override_line is logical! not physical
void SyntaxTree::do_error(const string &str, int override_exp_no, int override_line) {
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
	throw Exception(str, expr, physical_line, pos, script);
}

void SyntaxTree::do_error_implicit(Function *f, const string &str) {
	int line = max(f->_logical_line_no, f->name_space->_logical_line_no);
	int ex = max(f->_exp_no, f->name_space->_exp_no);
	do_error("[auto generating " + f->signature() + "] : " + str, ex, line);
}

void SyntaxTree::create_asm_meta_info() {
	asm_meta_info->global_var.clear();
	for (auto *v: base_class->static_variables){
		Asm::GlobalVar vv;
		vv.name = v->name;
		vv.size = v->type->size;
		vv.pos = v->memory;
		asm_meta_info->global_var.add(vv);
	}
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



Node *SyntaxTree::add_node_const(Constant *c) {
	return new Node(NodeKind::CONSTANT, (int_p)c, c->type, true);
}

/*Node *SyntaxTree::add_node_block(Block *b) {
	return new Node(NodeKind::BLOCK, (int_p)b, TypeVoid);
}*/

PrimitiveOperator *SyntaxTree::which_primitive_operator(const string &name, int param_flags) {
	for (int i=0; i<(int)OperatorID::_COUNT_; i++)
		if (name == PrimitiveOperators[i].name and param_flags == PrimitiveOperators[i].param_flags)
			return &PrimitiveOperators[i];

	// old hack
	if (name == "!")
		return &PrimitiveOperators[(int)OperatorID::NEGATE];

	return nullptr;
}

const Class *SyntaxTree::which_owned_class(const string &name) {
	for (auto *c: base_class->classes)
		if (name == c->name)
			return c;
	return nullptr;
}

Statement *SyntaxTree::which_statement(const string &name) {
	for (auto *s: Statements)
		if (name == s->name)
			return s;

	return nullptr;
}


// xxxxx FIXME

Node *SyntaxTree::exlink_add_element(Function *f, ClassElement &e) {
	Node *self = add_node_local(f->__get_var(IDENTIFIER_SELF));
	Node *link = new Node(NodeKind::ADDRESS_SHIFT, e.offset, e.type);
	link->set_num_params(1);
	link->params[0] = self;
	return link;
}

// functions of our "self" class
Node *SyntaxTree::exlink_add_class_func(Function *f, Function *cf) {
	Node *link = add_node_func_name(cf);
	Node *self = add_node_local(f->__get_var(IDENTIFIER_SELF));
	if (!f->is_static()) {
		link->set_num_params(1);
		link->set_instance(self);
	}
	return link;
}

Array<Node*> SyntaxTree::get_existence_global(const string &name, const Class *ns, bool prefer_class) {
	Array<Node*> links;

	if (!prefer_class) {
		// global variables (=local variables in "RootOfAllEvil")
		for (auto *v: base_class->static_variables)
			if (v->name == name)
				return {add_node_global(v)};
		// TODO.... namespace...
	}


	// recursively up the namespaces
	while (ns) {

		if (!prefer_class) {
			// named constants
			for (auto *c: ns->constants)
				if (name == c->name)
					return {add_node_const(c)};

			// then the (real) functions
			for (auto *f: ns->functions)
				if (f->name == name and f->is_static())
					links.add(add_node_func_name(f));
			if (links.num > 0 and !prefer_class)
				return links;
		}

		// types
		for (const Class *c: ns->classes)
			if (c->name == name)
				return {add_node_class(c)};

		// prefer class...
		if (links.num > 0)
			return links;

		ns = ns->name_space;
	}

	// ...unknown
	return {};
}

Node* SyntaxTree::get_existence_block(const string &name, Block *block) {
	Function *f = block->function;

	// first test local variables
	auto *v = block->get_var(name);
	if (v)
		return add_node_local(v);
	if (!f->is_static()){
		if ((name == IDENTIFIER_SUPER) and (f->name_space->parent))
			return add_node_local(f->__get_var(IDENTIFIER_SELF), f->name_space->parent);
		// class elements (within a class function)
		for (auto &e: f->name_space->elements)
			if (e.name == name)
				return exlink_add_element(f, e);
		for (auto *cf: f->name_space->functions)
			if (cf->name == name)
				return exlink_add_class_func(f, cf);
	}
	return nullptr;
}

Array<Node*> SyntaxTree::get_existence(const string &name, Block *block, const Class *ns, bool prefer_class)
{
	if (block and !prefer_class) {
		Node *n = get_existence_block(name, block);
		if (n)
			return {n};
	}

	// shared stuff (global variables, functions)
	auto links = get_existence_global(name, ns, prefer_class);
	if (links.num > 0)
		return links;

	if (!prefer_class) {
		// then the statements
		auto s = which_statement(name);
		if (s){
			//return {add_node_statement(s->id)};
			Node *n = new Node(NodeKind::STATEMENT, (int64)s, TypeVoid);
			n->set_num_params(s->num_params);
			return {n};
		}

		// operators
		auto w = which_primitive_operator(name, 2); // negate/not...
		if (w)
			return {new Node(NodeKind::PRIMITIVE_OPERATOR, (int_p)w, TypeUnknown)};
	}

	// in include files (only global)...
	for (Script *i: includes)
		links.append(i->syntax->get_existence_global(name, i->syntax->base_class, prefer_class));


	if (links.num == 0 and prefer_class)
		return get_existence(name, block, ns, false);

	// ...unknown
	return links;
}

// expression naming a type
// we are currently in <namespace>... (no explicit namespace for <name>)
const Class *SyntaxTree::find_root_type_by_name(const string &name, const Class *_namespace, bool allow_recursion) {
	for (auto *c: _namespace->classes)
		if (name == c->name)
			return c;
	if (_namespace == base_class) {
		for (Script *inc: includes)
			for (auto *c: inc->syntax->base_class->classes)
				if (name == c->name)
					return c;
	} else if (_namespace->name_space and allow_recursion) {
		// parent namespace
		return find_root_type_by_name(name, _namespace->name_space, true);
	}
	return nullptr;
}

Class *SyntaxTree::create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Class *param, const Class *ns) {
	if (find_root_type_by_name(name, ns, false))
		do_error("class already exists");

	Class *t = new Class(name, size, this, parent, param);
	t->type = type;
	t->_logical_line_no = Exp.get_line_no();
	t->_exp_no = Exp.cur_exp;
	owned_classes.add(t);
	
	// link namespace
	(const_cast<Class*>(ns))->classes.add(t);
	t->name_space = ns;
	
	t->array_length = max(array_size, 0);
	if (t->is_super_array() or t->is_dict()) {
		t->derive_from(TypeDynamicArray, false); // we already set its size!
		if (param->needs_constructor() and !param->get_default_constructor())
			do_error(format("can not create a dynamic array from type %s, missing default constructor", param->long_name().c_str()));
		t->param = param;
		add_missing_function_headers_for_class(t);
	} else if (t->is_array()) {
		if (param->needs_constructor() and !param->get_default_constructor())
			do_error(format("can not create an array from type %s, missing default constructor", param->long_name().c_str()));
		add_missing_function_headers_for_class(t);
	}
	return t;
}

const Class *SyntaxTree::make_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Class *param, const Class *ns) {
	//msg_write("make class " + name + " ns=" + ns->long_name() + " param=" + param->long_name());
	
	// check if it already exists
	auto *tt = find_root_type_by_name(name, ns, false);
	if (tt)
		return tt;

	// add new class
	return create_new_class(name, type, size, array_size, parent, param, ns);
}

const Class *SyntaxTree::make_class_super_array(const Class *element_type) {
	string name = element_type->name + "[]";
	return make_class(name, Class::Type::SUPER_ARRAY, config.super_array_size, -1, TypeDynamicArray, element_type, element_type->name_space);
}

const Class *SyntaxTree::make_class_array(const Class *element_type, int num_elements) {
	string name = element_type->name + format("[%d]", num_elements);
	return make_class(name, Class::Type::ARRAY, element_type->size * num_elements, num_elements, nullptr, element_type, element_type->name_space);
}

const Class *SyntaxTree::make_class_dict(const Class *element_type) {
	string name = element_type->name + "{}";
	return make_class(name, Class::Type::DICT, config.super_array_size, 0, TypeDictBase, element_type, element_type->name_space);
}

Node *SyntaxTree::conv_cbr(Node *c, Variable *var) {
	// convert
	if ((c->kind == NodeKind::VAR_LOCAL) and (c->as_local() == var)) {
		c->type = c->type->get_pointer();
		return deref_node(c);
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


Node *SyntaxTree::conv_calls(Node *c) {
	if ((c->kind == NodeKind::STATEMENT) and (c->as_statement()->id == StatementID::RETURN))
		if (c->params.num > 0) {
			if ((c->params[0]->type->is_array()) /*or (c->Param[j]->Type->IsSuperArray)*/) {
				c->set_param(0, ref_node(c->params[0]));
			}
			return c;
		}

	if ((c->kind == NodeKind::FUNCTION_CALL) or (c->kind == NodeKind::VIRTUAL_CALL) or (c->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION)) {

		// parameters, instance: class as reference
		for (int j=0;j<c->params.num;j++)
			if (c->params[j] and c->params[j]->type->uses_call_by_reference()) {
				c->set_param(j, ref_node(c->params[j]));
			}

		// return: array reference (-> dereference)
		if ((c->type->is_array()) /*or (c->Type->IsSuperArray)*/) {
			c->type = c->type->get_pointer();
			return deref_node(c);
			//deref_command_old(this, c);
		}
	}

	// special string / list operators
	if (c->kind == NodeKind::OPERATOR) {
		// parameters: super array as reference
		for (int j=0;j<c->params.num;j++)
			if ((c->params[j]->type->is_array()) or (c->params[j]->type->is_super_array())) {
				c->set_param(j, ref_node(c->params[j]));
				// REALLY ?!?!?!?  FIXME?!?!?
				msg_write("this might be bad");
			}
  	}
	return c;
}


// remove &*x
Node *SyntaxTree::conv_easyfy_ref_deref(Node *c, int l) {
	if (c->kind == NodeKind::REFERENCE) {
		if (c->params[0]->kind == NodeKind::DEREFERENCE) {
			// remove 2 knots...
			return c->params[0]->params[0];
		}
	}
	return c;
}

// remove (*x)[] and (*x).y
Node *SyntaxTree::conv_easyfy_shift_deref(Node *c, int l) {
	if ((c->kind == NodeKind::ADDRESS_SHIFT) or (c->kind == NodeKind::ARRAY)) {
		if (c->params[0]->kind == NodeKind::DEREFERENCE) {
			// unify 2 knots (remove 1)
			Node *t = c->params[0]->params[0];
			c->kind = (c->kind == NodeKind::ADDRESS_SHIFT) ? NodeKind::DEREF_ADDRESS_SHIFT : NodeKind::POINTER_AS_ARRAY;
			c->set_param(0, t);
			return c;
		}
	}

	return c;
}


Node *SyntaxTree::conv_return_by_memory(Node *n, Function *f) {
	script->cur_func = f;

	if ((n->kind != NodeKind::STATEMENT) or (n->as_statement()->id != StatementID::RETURN))
		return n;

	// convert into   *-return- = param
	Node *p_ret = nullptr;
	for (Variable *v: f->var)
		if (v->name == IDENTIFIER_RETURN_VAR) {
			p_ret = add_node_local(v);
		}
	if (!p_ret)
		do_error("-return- not found...");
	Node *ret = deref_node(p_ret);
	Node *cmd_assign = link_operator_id(OperatorID::ASSIGN, ret, n->params[0]);
	if (!cmd_assign)
		do_error("no = operator for return from function found: " + f->long_name());
	_transform_insert_before_.add(cmd_assign);

	n->set_num_params(0);
	return n;
}


// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void SyntaxTree::convert_call_by_reference() {
	if (config.verbose)
		msg_write("ConvertCallByReference");
	// convert functions
	for (Function *f: functions) {
		
		// TODO: convert self...
		if (!f->is_static() and f->name_space->uses_call_by_reference()) {
			for (auto *v: f->var)
				if (v->name == IDENTIFIER_SELF) {
					//msg_write("CONV SELF....");
					v->type = v->type->get_pointer();

					// internal usage...
					transform_block(f->block, [&](Node *n){ return conv_cbr(n, v); });
				}
		}

		// parameter: array/class as reference
		for (int j=0;j<f->num_params;j++)
			if (f->var[j]->type->uses_call_by_reference()) {
				f->var[j]->type = f->var[j]->type->get_pointer();

				// internal usage...
				transform_block(f->block, [&](Node *n){ return conv_cbr(n, f->var[j]); });
			}

		// return: array as reference
#if 0
		if ((func->return_type->is_array) /*or (f->Type->IsSuperArray)*/) {
			func->return_type = GetPointerType(func->return_type);
			/*for (int k=0;k<f->Block->Command.num;k++)
				conv_return(this, f->Block->Command[k]);*/
			// no need... return gets converted automatically (all calls...)
		}
#endif
	}

	// convert return...
	for (Function *f: functions)
		if (f->return_type->uses_return_by_memory())
			//convert_return_by_memory(this, f->block, f);
			transform_block(f->block, [&](Node *n){ return conv_return_by_memory(n, f); });

	// convert function calls
	transform([&](Node *n){ return conv_calls(n); });
}


void SyntaxTree::simplify_ref_deref() {
	// remove &*
	transform([&](Node *n){ return conv_easyfy_ref_deref(n, 0); });
}

void SyntaxTree::simplify_shift_deref() {
	// remove &*
	transform([&](Node *n){ return conv_easyfy_shift_deref(n, 0); });
}

InlineID __get_pointer_add_int() {
	if (config.instruction_set == Asm::InstructionSet::AMD64)
		return InlineID::INT64_ADD_INT;
	return InlineID::INT_ADD;
}


Node *conv_break_down_med_level(SyntaxTree *tree, Node *c) {
	if (c->kind == NodeKind::DYNAMIC_ARRAY) {
		return tree->conv_break_down_low_level(tree->add_node_parray(tree->shift_node(c->params[0], false, 0, c->type->get_pointer()), c->params[1], c->type));
	}
	return c;
}

Node *SyntaxTree::conv_break_down_low_level(Node *c) {

	if (c->kind == NodeKind::ARRAY) {

		auto *el_type = c->type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

		Node *c_index = c->params[1];
		// & array
		Node *c_ref_array = ref_node(c->params[0]);
		// create command for size constant
		Node *c_size = add_node_const(add_constant_int(el_type->size));
		// offset = size * index
		Node *c_offset = add_node_operator_by_inline(c_index, c_size, InlineID::INT_MULTIPLY);
		c_offset->type = TypeInt;//TypePointer;
		// address = &array + offset
		Node *c_address = add_node_operator_by_inline(c_ref_array, c_offset, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	} else if (c->kind == NodeKind::POINTER_AS_ARRAY) {

		auto *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

		Node *c_index = c->params[1];
		Node *c_ref_array = c->params[0];
		// create command for size constant
		Node *c_size = add_node_const(add_constant_int(el_type->size));
		// offset = size * index
		Node *c_offset = add_node_operator_by_inline(c_index, c_size, InlineID::INT_MULTIPLY);
		c_offset->type = TypeInt;
		// address = &array + offset
		Node *c_address = add_node_operator_by_inline(c_ref_array, c_offset, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	} else if (c->kind == NodeKind::ADDRESS_SHIFT) {

		auto *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

		// & struct
		Node *c_ref_struct = ref_node(c->params[0]);
		// create command for shift constant
		Node *c_shift = add_node_const(add_constant_int(c->link_no));
		// address = &struct + shift
		Node *c_address = add_node_operator_by_inline(c_ref_struct, c_shift, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	} else if (c->kind == NodeKind::DEREF_ADDRESS_SHIFT) {

		auto *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		Node *c_ref_struct = c->params[0];
		// create command for shift constant
		Node *c_shift = add_node_const(add_constant_int(c->link_no));
		// address = &struct + shift
		Node *c_address = add_node_operator_by_inline(c_ref_struct, c_shift, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	}
	return c;
}


Node* SyntaxTree::transform_node(Node *n, std::function<Node*(Node*)> F) {
	if (n->kind == NodeKind::BLOCK) {
		transform_block(n->as_block(), F);
	} else {
		for (int i=0; i<n->params.num; i++)
			n->set_param(i, transform_node(n->params[i], F));
	}
	return F(n);
}

Node* SyntaxTree::transformb_node(Node *n, Block *b, std::function<Node*(Node*, Block*)> F) {
	if (n->kind == NodeKind::BLOCK) {
		transformb_block(n->as_block(), F);
	} else {
		for (int i=0; i<n->params.num; i++)
			n->set_param(i, transformb_node(n->params[i], b, F));
	}
	return F(n, b);
}

// preventing a sub-block to handle insertions of an outer block
#define PUSH_BLOCK_INSERT \
	auto XXX = _transform_insert_before_; \
	_transform_insert_before_.clear();
#define POP_BLOCK_INSERT \
	_transform_insert_before_ = XXX;

void handle_insert_before(Block *block, int &i) {
	if (_transform_insert_before_.num > 0) {
		for (auto *ib: _transform_insert_before_) {
			if (config.verbose)
				msg_error("INSERT BEFORE...2");
			block->params.insert(ib, i);
			i ++;
		}
		_transform_insert_before_.clear();
	}
}


void SyntaxTree::transform_block(Block *block, std::function<Node*(Node*)> F) {
	PUSH_BLOCK_INSERT;
	//foreachi (Node *n, block->nodes, i){
	for (int i=0; i<block->params.num; i++) {
		block->params[i] = transform_node(block->params[i], F);
		handle_insert_before(block, i);
	}
	POP_BLOCK_INSERT;
}

void SyntaxTree::transformb_block(Block *block, std::function<Node*(Node*, Block*)> F) {
	PUSH_BLOCK_INSERT;
	//foreachi (Node *n, block->nodes, i){
	for (int i=0; i<block->params.num; i++) {
		block->params[i] = transformb_node(block->params[i], block, F);
		handle_insert_before(block, i);
	}
	POP_BLOCK_INSERT;
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::transform(std::function<Node*(Node*)> F) {
	for (Function *f: functions) {
		cur_func = f;
		transform_block(f->block, F);
	}
}
void SyntaxTree::transformb(std::function<Node*(Node*, Block*)> F) {
	for (Function *f: functions) {
		cur_func = f;
		transformb_block(f->block, F);
	}
}

bool node_is_executable(Node *n) {
	if ((n->kind == NodeKind::CONSTANT) or (n->kind == NodeKind::VAR_LOCAL) or (n->kind == NodeKind::VAR_GLOBAL))
		return false;
	if ((n->kind == NodeKind::ADDRESS_SHIFT) or (n->kind == NodeKind::ARRAY) or (n->kind == NodeKind::DYNAMIC_ARRAY) or (n->kind == NodeKind::REFERENCE) or (n->kind == NodeKind::DEREFERENCE) or (n->kind == NodeKind::DEREF_ADDRESS_SHIFT))
		return node_is_executable(n->params[0]);
	return true;
}

Node *SyntaxTree::conv_class_and_func_to_const(Node *n) {
	if (n->kind == NodeKind::FUNCTION) {
		return add_node_const(add_constant_pointer(TypeFunctionP, n->as_func()));
	} else if (n->kind == NodeKind::CLASS) {
		return add_node_const(add_constant_pointer(TypeClassP, n->as_class()));
	}
	return n;
}


Node *SyntaxTree::conv_break_down_high_level(Node *n, Block *b) {
	if (n->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION) {
		if (config.verbose) {
			msg_error("constr func....");
			n->show();
		}
		
		// temp var
		auto *f = cur_func;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type);
		vv->explicitly_constructed = true;
		Node *dummy = add_node_local(vv);
		
		auto *ib = cp_node(n);
		ib->kind = NodeKind::FUNCTION_CALL;
		ib->type = TypeVoid;
		ib->set_instance(dummy);
		if (config.verbose)
			ib->show();

		_transform_insert_before_.add(ib);

		return cp_node(dummy);
	} else if (n->kind == NodeKind::ARRAY_BUILDER) {
		auto *t_el = n->type->get_array_element();
		Function *cf = n->type->get_func("add", TypeVoid, {t_el});
		if (!cf)
			do_error(format("[..]: can not find %s.add(%s) function???", n->type->long_name().c_str(), t_el->long_name().c_str()));

		// temp var
		auto *f = cur_func;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type);
		Node *array = add_node_local(vv);

		Block *bb = new Block(f, b);
		for (int i=0; i<n->params.num; i++){
			auto *cc = add_node_member_call(cf, cp_node(array));
			cc->set_param(1, n->params[i]);
			bb->add(cc);
		}
		_transform_insert_before_.add(bb);
		return array;
	} else if (n->kind == NodeKind::DICT_BUILDER) {
		auto *t_el = n->type->get_array_element();
		Function *cf = n->type->get_func("set", TypeVoid, {TypeString, t_el});
		if (!cf)
			do_error(format("[..]: can not find %s.set(string,%s) function???", n->type->long_name().c_str(), t_el->long_name().c_str()));

		// temp var
		auto *f = cur_func;
		auto *vv = b->add_var(f->create_slightly_hidden_name(), n->type);
		Node *array = add_node_local(vv);

		Block *bb = new Block(f, b);
		for (int i=0; i<n->params.num/2; i++){
			auto *cc = add_node_member_call(cf, cp_node(array));
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

		n->link_no = (int_p)statement_from_id(StatementID::FOR_DIGEST);
		n->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]

		Node *cmd_assign = add_node_operator_by_inline(var, val0, InlineID::INT_ASSIGN);
		n->set_param(0, cmd_assign);

		// while(for_var < val1)
		Node *cmd_cmp = add_node_operator_by_inline(cp_node(var), val1, InlineID::INT_SMALLER);
		n->set_param(1, cmd_cmp);

		n->set_param(2, block);


		// ...for_var += 1
		Node *cmd_inc;
		if (var->type == TypeInt) {
			if (step->as_const()->as_int() == 1)
				cmd_inc = add_node_operator_by_inline(cp_node(var), nullptr, InlineID::INT_INCREASE);
			else
				cmd_inc = add_node_operator_by_inline(cp_node(var), step, InlineID::INT_ADD_ASSIGN);
		} else {
			cmd_inc = add_node_operator_by_inline(cp_node(var), step, InlineID::FLOAT_ADD_ASSIGN);
		}
		n->set_param(3, cmd_inc); // add to loop-block

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

			auto *assign = link_operator_id(OperatorID::ASSIGN, add_node_local(v), array);
			_transform_insert_before_.add(assign);

			array = add_node_local(v);
		}

		n->link_no = (int_p)statement_from_id(StatementID::FOR_DIGEST);
		n->set_num_params(4);
		// [INIT, CMP, BLOCK, INC]


		// 0
		Node *val0 = add_node_const(add_constant_int(0));

		// implement
		// for_index = 0
		Node *cmd_assign = add_node_operator_by_inline(index, val0, InlineID::INT_ASSIGN);
		n->set_param(0, cmd_assign);

		Node *val1;
		if (array->type->usable_as_super_array()) {
			// array.num
			val1 = new Node(NodeKind::ADDRESS_SHIFT, config.pointer_size, TypeInt);
			val1->set_num_params(1);
			val1->set_param(0, cp_node(array));
		} else {
			// array.size
			val1 = add_node_const(add_constant_int(array->type->array_length));
		}

		// while(for_index < val1)
		Node *cmd_cmp = add_node_operator_by_inline(cp_node(index), val1, InlineID::INT_SMALLER);
		n->set_param(1, cmd_cmp);

		// ...block
		n->set_param(2, block);

		// ...for_index += 1
		Node *cmd_inc = add_node_operator_by_inline(cp_node(index), nullptr, InlineID::INT_INCREASE);
		n->set_param(3, cmd_inc);

		// array[index]
		Node *el;
		if (array->type->usable_as_super_array()) {
			el = add_node_dyn_array(array, cp_node(index));
		} else {
			el = add_node_array(array, cp_node(index));
		}

		// &for_var = &array[index]
		Node *cmd_var_assign = add_node_operator_by_inline(var, ref_node(el), InlineID::POINTER_ASSIGN);
		block->params.insert(cmd_var_assign, 0);

	} else if (n->kind == NodeKind::ARRAY_BUILDER_FOR) {

		_transform_insert_before_.add(n->params[0]);
		return n->params[1];
	}
	return n;
}

Node* SyntaxTree::conv_func_inline(Node *n) {
	if (n->kind == NodeKind::FUNCTION_CALL) {
		if (n->as_func()->inline_no != InlineID::NONE) {
			n->kind = NodeKind::INLINE_CALL;
			return n;
		}
	}
	if (n->kind == NodeKind::OPERATOR) {
		Operator *op = n->as_op();
		n->kind = NodeKind::INLINE_CALL; // FIXME
		n->link_no = (int_p)op->f;
		return n;
	}
	return n;
}


void MapLVSX86Return(Function *f) {
	if (f->return_type->uses_return_by_memory()) {
		foreachi(Variable *v, f->var, i)
			if (v->name == IDENTIFIER_RETURN_VAR) {
				v->_offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void MapLVSX86Self(Function *f) {
	if (!f->is_static()){
		foreachi(Variable *v, f->var, i)
			if (v->name == IDENTIFIER_SELF) {
				v->_offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void SyntaxTree::map_local_variables_to_stack() {
	for (Function *f: functions) {
		f->_param_size = 2 * config.pointer_size; // space for return value and eBP
		if (config.instruction_set == Asm::InstructionSet::X86) {
			f->_var_size = 0;

			if (config.abi == Abi::WINDOWS_32) {
				// map "self" to the VERY first parameter
				MapLVSX86Self(f);

				// map "-return-" to the first parameter
				MapLVSX86Return(f);
			} else {
				// map "-return-" to the VERY first parameter
				MapLVSX86Return(f);

				// map "self" to the first parameter
				MapLVSX86Self(f);
			}

			foreachi(Variable *v, f->var, i) {
				if (!f->is_static() and (v->name == IDENTIFIER_SELF))
					continue;
				if (v->name == IDENTIFIER_RETURN_VAR)
					continue;
				int s = mem_align(v->type->size, 4);
				if (i < f->num_params) {
					// parameters
					v->_offset = f->_param_size;
					f->_param_size += s;
				} else {
					// "real" local variables
					v->_offset = - f->_var_size - s;
					f->_var_size += s;
				}
			}
		} else if (config.instruction_set == Asm::InstructionSet::AMD64) {
			f->_var_size = 0;

			foreachi(Variable *v, f->var, i) {
				long long s = mem_align(v->type->size, 4);
				v->_offset = - f->_var_size - s;
				f->_var_size += s;
			}
		} else if (config.instruction_set == Asm::InstructionSet::ARM) {
			f->_var_size = 0;

			foreachi(Variable *v, f->var, i) {
				int s = mem_align(v->type->size, 4);
				v->_offset = f->_var_size;// + s;
				f->_var_size += s;
			}
		}
	}
}


// no included scripts may be deleted before us!!!
SyntaxTree::~SyntaxTree() {
	// delete all classes, functions etc created by this script

	if (asm_meta_info)
		delete asm_meta_info;

	delete root_of_all_evil;
	delete base_class;
}

void SyntaxTree::show(const string &stage) {
	if (!config.allow_output_stage(stage))
		return;
	msg_write("--------- Syntax of " + script->filename + "  " + stage + " ---------");
	msg_right();
	for (auto *f: functions)
		if (!f->is_extern())
			f->show(stage);
	msg_left();
	msg_write("\n\n");
}

};
