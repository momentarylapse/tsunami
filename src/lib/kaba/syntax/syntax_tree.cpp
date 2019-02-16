#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Kaba{

//#define ScriptDebug


extern Class *TypeDynamicArray;


bool next_extern = false;
bool next_const = false;

Node *conv_cbr(SyntaxTree *ps, Node *c, int var);

Value::Value()
{
	type = TypeVoid;
}

Value::~Value()
{
	clear();
}

void Value::init(Class *_type)
{
	clear();
	type = _type;

	if (type->is_super_array()){
		value.resize(sizeof(DynamicArray));
		as_array().init(type->parent->size);
	}else{
		value.resize(max(type->size, (long long)16));
	}
}

void Value::clear()
{
	if (type->is_super_array())
		as_array().clear();

	value.clear();
	type = TypeVoid;
}

void Value::set(const Value &v)
{
	init(v.type);
	if (type->is_super_array()){
		as_array().resize(v.as_array().num);
		memcpy(as_array().data, v.as_array().data, as_array().num * type->parent->size);

	}else{
		// plain old data
		memcpy(p(), v.p(), type->size);
	}
}

void* Value::p() const
{
	return value.data;
}

int& Value::as_int() const
{
	return *(int*)value.data;
}

long long& Value::as_int64() const
{
	return *(long long*)value.data;
}

float& Value::as_float() const
{
	return *(float*)value.data;
}

double& Value::as_float64() const
{
	return *(double*)value.data;
}

complex& Value::as_complex() const
{
	return *(complex*)value.data;
}

string& Value::as_string() const
{
	return *(string*)value.data;
}

DynamicArray& Value::as_array() const
{
	return *(DynamicArray*)p();
}

int Value::mapping_size() const
{
	if (type->is_super_array())
		return config.super_array_size + (as_array().num * type->parent->size);
	if (type == TypeCString)
		return strlen((char*)p()) + 1;

	// plain old data
	return type->size;
}

void Value::map_into(char *memory, char *addr) const
{
	if (type->is_super_array()){
		// const string -> variable length
		int size = as_array().element_size * as_array().num;
		int data_offset = config.super_array_size;

		*(void**)&memory[0] = addr + data_offset; // .data
		*(int*)&memory[config.pointer_size    ] = as_array().num;
		*(int*)&memory[config.pointer_size + 4] = 0; // .reserved
		*(int*)&memory[config.pointer_size + 8] = as_array().element_size;
		memcpy(&memory[data_offset], as_array().data, size);
	}else if (type == TypeCString){
		strcpy(memory, (char*)p());
	}else{
		memcpy(memory, p(), type->size);
	}
}

string Value::str() const
{
	return type->var2str(value.data);
}

Constant::Constant(Class *_type)
{
	init(_type);
	name = "-none-";
}

string Constant::str() const
{
	return Value::str();
}

Node *SyntaxTree::cp_node(Node *c)
{
	Node *cmd = AddNode(c->kind, c->link_no, c->type, c->script);
	cmd->set_num_params(c->params.num);
	for (int i=0;i<c->params.num;i++)
		if (c->params[i])
			cmd->set_param(i, cp_node(c->params[i]));
	if (c->instance)
		cmd->set_instance(cp_node(c->instance));
	return cmd;
}

Node *SyntaxTree::ref_node(Node *sub, Class *override_type)
{
	Class *t = override_type ? override_type : sub->type->get_pointer();

	if (sub->kind == KIND_TYPE){
		// Class pointer
		int nc = AddConstant(TypeClassP);
		constants[nc]->as_int64() = (long long)(long)sub->as_class();
		return add_node_const(nc);
	}

	Node *c = AddNode(KIND_REFERENCE, 0, t);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Node *SyntaxTree::deref_node(Node *sub, Class *override_type)
{
	Node *c = AddNode(KIND_UNKNOWN, 0, TypeVoid);
	c->kind = KIND_DEREFERENCE;
	c->set_num_params(1);
	c->set_param(0, sub);
	if (override_type)
		c->type = override_type;
	else
		c->type = sub->type->parent;
	return c;
}

Node *SyntaxTree::shift_node(Node *sub, bool deref, int shift, Class *type)
{
	Node *c= AddNode(deref ? KIND_DEREF_ADDRESS_SHIFT : KIND_ADDRESS_SHIFT, shift, type);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Node *SyntaxTree::add_node_statement(int index)
{
	Node *c = AddNode(KIND_STATEMENT, index, TypeVoid);

	c->script = Packages[0].script;
	c->instance = nullptr;
	c->set_num_params(Statements[index].num_params);

	return c;
}

// virtual call, if func is virtual
Node *SyntaxTree::add_node_classfunc(ClassFunction *f, Node *inst, bool force_non_virtual)
{
	Node *c;
	if ((f->virtual_index >= 0) and (!force_non_virtual))
		c = AddNode(KIND_VIRTUAL_FUNCTION, f->virtual_index, f->return_type);
	else
		c = AddNode(KIND_FUNCTION, f->nr, f->return_type);
	c->script = f->script;
	c->set_instance(inst);
	c->set_num_params(f->param_types.num);
	return c;
}

Node *SyntaxTree::add_node_func(Script *script, int no, Class *return_type)
{
	Node *c = AddNode(KIND_FUNCTION, no, return_type);
	c->script = script;
	c->set_num_params(script->syntax->functions[no]->num_params);
	return c;
}


Node *SyntaxTree::add_node_operator_by_index(Node *p1, Node *p2, int op)
{
	Node *cmd = AddNode(KIND_OPERATOR, op, operators[op].return_type);
	bool unitary = ((operators[op].param_type_1 == TypeVoid) or (operators[op].param_type_2 == TypeVoid));
	cmd->set_num_params( unitary ? 1 : 2); // unary / binary
	cmd->set_param(0, p1);
	if (!unitary)
		cmd->set_param(1, p2);
	return cmd;
}

Node *SyntaxTree::add_node_operator_by_inline(Node *p1, Node *p2, int inline_index)
{
	foreachi (Operator &o, operators, i)
		if (o.inline_index == inline_index)
			return add_node_operator_by_index(p1, p2, i);

	DoError("operator inline index not found: " + i2s(inline_index));
	return nullptr;
}


Node *SyntaxTree::add_node_local_var(int no, Class *type)
{
	if (no < 0)
		script->DoErrorInternal("negative local variable index");
	return AddNode(KIND_VAR_LOCAL, no, type);
}

Node *SyntaxTree::add_node_parray(Node *p, Node *index, Class *type)
{
	Node *cmd_el = AddNode(KIND_POINTER_AS_ARRAY, 0, type);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

Node *SyntaxTree::add_node_block(Block *b)
{
	return AddNode(KIND_BLOCK, (long long)(int_p)b, TypeVoid);
}

SyntaxTree::SyntaxTree(Script *_script) :
	root_of_all_evil(this, "RootOfAllEvil", TypeVoid)
{
	root_of_all_evil.block = new Block(&root_of_all_evil, nullptr);

	flag_string_const_as_cstring = false;
	flag_immortal = false;
	cur_func = nullptr;
	script = _script;
	asm_meta_info = new Asm::MetaInfo;
	for_index_count = 0;
	Exp.cur_line = nullptr;
	parser_loop_depth = 0;

	// "include" default stuff
	for (Package &p: Packages)
		if (p.used_by_default)
			AddIncludeData(p.script);
}


void SyntaxTree::ParseBuffer(const string &buffer, bool just_analyse)
{
	Exp.Analyse(this, buffer + string("\0", 1)); // compatibility... expected by lexical
	
	PreCompiler(just_analyse);

	Parser();

	Exp.clear();

	if (config.verbose)
		Show("par:a");

	ConvertCallByReference();

	/*if (FlagShow)
		Show();*/

	SimplifyShiftDeref();
	SimplifyRefDeref();
	
	PreProcessor();

	if (config.verbose)
		Show("par:b");
}

string Kind2Str(int kind)
{
	if (kind == KIND_VAR_LOCAL)			return "local variable";
	if (kind == KIND_VAR_GLOBAL)			return "global variable";
	if (kind == KIND_VAR_FUNCTION)		return "function as variable";
	if (kind == KIND_CONSTANT)			return "constant";
	if (kind == KIND_REF_TO_CONST)			return "reference to const";
	if (kind == KIND_FUNCTION)			return "function";
	if (kind == KIND_INLINE_FUNCTION)			return "inline";
	if (kind == KIND_VIRTUAL_FUNCTION)	return "virtual function";
	if (kind == KIND_STATEMENT)			return "statement";
	if (kind == KIND_OPERATOR)			return "operator";
	if (kind == KIND_PRIMITIVE_OPERATOR)	return "PRIMITIVE operator";
	if (kind == KIND_BLOCK)				return "command block";
	if (kind == KIND_ADDRESS_SHIFT)		return "address shift";
	if (kind == KIND_ARRAY)				return "array element";
	if (kind == KIND_POINTER_AS_ARRAY)		return "pointer as array element";
	if (kind == KIND_REFERENCE)			return "address operator";
	if (kind == KIND_DEREFERENCE)		return "dereferencing";
	if (kind == KIND_DEREF_ADDRESS_SHIFT)	return "deref address shift";
	if (kind == KIND_TYPE)				return "type";
	if (kind == KIND_ARRAY_BUILDER)		return "array builder";
	if (kind == KIND_CONSTRUCTOR_AS_FUNCTION)		return "constructor function";
	if (kind == KIND_VAR_TEMP)			return "temp";
	if (kind == KIND_DEREF_VAR_TEMP)		return "deref temp";
	if (kind == KIND_REGISTER)			return "register";
	if (kind == KIND_ADDRESS)			return "address";
	if (kind == KIND_MEMORY)				return "memory";
	if (kind == KIND_LOCAL_ADDRESS)		return "local address";
	if (kind == KIND_LOCAL_MEMORY)		return "local memory";
	if (kind == KIND_DEREF_REGISTER)		return "deref register";
	if (kind == KIND_MARKER)				return "marker";
	if (kind == KIND_DEREF_MARKER)		return "deref marker";
	if (kind == KIND_GLOBAL_LOOKUP)		return "global lookup";
	if (kind == KIND_DEREF_GLOBAL_LOOKUP)	return "deref global lookup";
	if (kind == KIND_IMMEDIATE)			return "immediate";
	if (kind == KIND_REF_TO_LOCAL)			return "ref to local";
	if (kind == KIND_REF_TO_GLOBAL)		return "ref to global";
	if (kind == KIND_REF_TO_CONST)			return "ref to const";
	if (kind == KIND_DEREF_VAR_LOCAL)		return "deref local";
	return format("UNKNOWN KIND: %d", kind);
}


string op_to_str(const Operator &op)
{
	return "(" + op.param_type_1->name + ") " + PrimitiveOperators[op.primitive_id].name + " (" + op.param_type_2->name + ")";
}

string LinkNr2Str(SyntaxTree *s, Function *f, int kind, long long nr)
{
	if (kind == KIND_VAR_LOCAL)			return /*"#" + i2s(nr) + ": " +*/ f->var[nr]->name;
	if (kind == KIND_VAR_GLOBAL)			return s->root_of_all_evil.var[nr]->name;
	if (kind == KIND_VAR_FUNCTION)		return s->functions[nr]->name;
	if (kind == KIND_CONSTANT)			return /*"#" + i2s(nr) + ": " +*/ s->constants[nr]->str();
	if (kind == KIND_FUNCTION)			return s->functions[nr]->name;
	if (kind == KIND_VIRTUAL_FUNCTION)	return i2s(nr);//s->Functions[nr]->name;
	if (kind == KIND_STATEMENT)	return Statements[nr].name;
	if (kind == KIND_OPERATOR)			return op_to_str(s->operators[nr]);
	if (kind == KIND_PRIMITIVE_OPERATOR)	return PrimitiveOperators[nr].name;
	if (kind == KIND_BLOCK)				return i2s(nr);
	if (kind == KIND_ADDRESS_SHIFT)		return i2s(nr);
	if (kind == KIND_ARRAY)				return "(no LinkNr)";
	if (kind == KIND_POINTER_AS_ARRAY)		return "(no LinkNr)";
	if (kind == KIND_REFERENCE)			return "(no LinkNr)";
	if (kind == KIND_DEREFERENCE)		return "(no LinkNr)";
	if (kind == KIND_DEREF_ADDRESS_SHIFT)	return i2s(nr);
	if (kind == KIND_TYPE)				return s->classes[nr]->name;
	if (kind == KIND_REGISTER)			return Asm::GetRegName(nr);
	if (kind == KIND_ADDRESS)			return d2h(&nr, config.pointer_size);
	if (kind == KIND_MEMORY)				return d2h(&nr, config.pointer_size);
	if (kind == KIND_LOCAL_ADDRESS)		return d2h(&nr, config.pointer_size);
	if (kind == KIND_LOCAL_MEMORY)		return d2h(&nr, config.pointer_size);
	return i2s(nr);
}

// override_line is logical! not physical
void SyntaxTree::DoError(const string &str, int override_exp_no, int override_line)
{
	// what data do we have?
	int logical_line = Exp.get_line_no();
	int exp_no = Exp.cur_exp;
	int physical_line = 0;
	int pos = 0;
	string expr;

	// override?
	if (override_line >= 0){
		logical_line = override_line;
		exp_no = 0;
	}
	if (override_exp_no >= 0)
		exp_no = override_exp_no;

	// logical -> physical
	if ((logical_line >= 0) and (logical_line < Exp.line.num)){
		physical_line = Exp.line[logical_line].physical_line;
		pos = Exp.line[logical_line].exp[exp_no].pos;
		expr = Exp.line[logical_line].exp[exp_no].name;
	}

	throw Exception(str, expr, physical_line, pos, script);
}

void SyntaxTree::CreateAsmMetaInfo()
{
	asm_meta_info->global_var.clear();
	for (int i=0;i<root_of_all_evil.var.num;i++){
		Asm::GlobalVar v;
		v.name = root_of_all_evil.var[i]->name;
		v.size = root_of_all_evil.var[i]->type->size;
		v.pos = script->g_var[i];
		asm_meta_info->global_var.add(v);
	}
}



// constants

int SyntaxTree::AddConstant(Class *type)
{
	constants.add(new Constant(type));
	return constants.num - 1;
}

Block::Block(Function *f, Block *_parent)
{
	level = 0;
	function = f;
	parent = _parent;
	if (parent)
		level = parent->level + 1;
	_start = _end = nullptr;
}

Block::~Block()
{
	for (Node *n: nodes)
		if (n->kind == KIND_BLOCK)
			delete (n->as_block());
}


inline void set_command(Node *&a, Node *b)
{
	if (a == b)
		return;
	if (a)
		a->ref_count --;
	if (b){
		if (b->ref_count > 0){
			//msg_write(">> " + Kind2Str(b->kind));
		}
		b->ref_count ++;
	}
	a = b;
}

void Block::add(Node *c)
{
	nodes.add(c);
	c->ref_count ++;
}

void Block::set(int index, Node *c)
{
	set_command(nodes[index], c);
}


Variable::Variable(const string &_name, Class *_type)
{
	name = _name;
	type = _type;
	_offset = 0;
	is_extern = false;
	dont_add_constructor = false;
}

int Block::add_var(const string &name, Class *type)
{
	if (get_var(name) >= 0)
		function->tree->DoError(format("variable '%s' already declared in this context", name.c_str()));
	Variable *v = new Variable(name, type);
	v->is_extern = next_extern;
	function->var.add(v);
	int n = function->var.num - 1;
	vars.add(n);
	return n;
}

int Block::get_var(const string &name)
{
	for (int i: vars)
		if (function->var[i]->name == name)
			return i;
	if (parent)
		return parent->get_var(name);
	return -1;
}

// functions


Function::Function(SyntaxTree *_tree, const string &_name, Class *_return_type)
{
	tree = _tree;
	name = _name;
	block = nullptr;
	num_params = 0;
	return_type = _return_type;
	literal_return_type = _return_type;
	_class = nullptr;
	is_extern = false;
	auto_declared = false;
	is_pure = false;
	_param_size = 0;
	_var_size = 0;
	_logical_line_no = -1;
	_exp_no = -1;
	inline_no = -1;
	throws_exceptions = false;
	num_slightly_hidden_vars = 0;
}

Function::~Function()
{
	for (Variable* v: var)
		delete v;
	if (block)
		delete block;
}

string Function::create_slightly_hidden_name()
{
	return format("-temp-%d-", ++ num_slightly_hidden_vars);
}

int Function::__get_var(const string &name) const
{
	return block->get_var(name);
}

string Function::signature(bool include_class) const
{
	string r = literal_return_type->name + " ";
	if ((name.find(".") >= 0) and !include_class)
		r += name.explode(".").back() + "(";
	else
		r += name + "(";
	for (int i=0; i<num_params; i++){
		if (i > 0)
			r += ", ";
		r += literal_param_type[i]->name;
	}
	return r + ")";
}

void blocks_add_recursive(Array<Block*> &blocks, Block *block)
{
	blocks.add(block);
	for (Node* n: block->nodes)
		if (n->kind == KIND_BLOCK)
			blocks_add_recursive(blocks, n->as_block());
}

Array<Block*> Function::all_blocks()
{
	Array<Block*> blocks;
	if (block)
		blocks_add_recursive(blocks, block);
	return blocks;
}

Function *SyntaxTree::AddFunction(const string &name, Class *type)
{
	Function *f = new Function(this, name, type);
	functions.add(f);
	f->block = new Block(f, nullptr);
	return f;
}

Node::Node(int _kind, long long _link_no, Script *_script, Class *_type)
{
	type = _type;
	kind = _kind;
	link_no = _link_no;
	instance = nullptr;
	script = _script;
	ref_count = 0;
}

Block *Node::as_block() const
{
	return (Block*)(int_p)link_no;
}

Function *Node::as_func() const
{
	return script->syntax->functions[link_no];
}

Class *Node::as_class() const
{
	return script->syntax->classes[link_no];
}

Constant *Node::as_const() const
{
	return script->syntax->constants[link_no];
}

void Node::set_instance(Node *p)
{
	set_command(instance, p);
}

void Node::set_num_params(int n)
{
	params.resize(n);
}

void Node::set_param(int index, Node *p)
{
	if ((index < 0) or (index >= params.num)){
		this->script->syntax->ShowNode(this, this->script->cur_func);
		script->DoErrorInternal(format("Command.set_param...  %d %d", index, params.num));
	}
	set_command(params[index], p);
}

Node *SyntaxTree::AddNode(int kind, long long link_no, Class *type)
{
	Node *c = new Node(kind, link_no, script, type);
	nodes.add(c);
	return c;
}

Node *SyntaxTree::AddNode(int kind, long long link_no, Class *type, Script *s)
{
	Node *c = new Node(kind, link_no, s, type);
	nodes.add(c);
	return c;
}


Node *SyntaxTree::add_node_const(int nc)
{
	return AddNode(KIND_CONSTANT, nc, constants[nc]->type);
}

int SyntaxTree::WhichPrimitiveOperator(const string &name)
{
	for (int i=0;i<NUM_PRIMITIVE_OPERATORS;i++)
		if (name == PrimitiveOperators[i].name)
			return i;
	return -1;
}

int SyntaxTree::WhichType(const string &name)
{
	for (int i=0;i<classes.num;i++)
		if (name == classes[i]->name)
			return i;

	return -1;
}

int SyntaxTree::WhichStatement(const string &name)
{
	for (int i=0;i<Statements.num;i++)
		if (name == Statements[i].name)
			return i;
	return -1;
}

Node *exlink_make_var_local(SyntaxTree *ps, Class *t, int var_no)
{
	return new Node(KIND_VAR_LOCAL, var_no, ps->script, t);
}

Node *exlink_make_var_element(SyntaxTree *ps, Function *f, ClassElement &e)
{
	Node *self = ps->add_node_local_var(f->__get_var(IDENTIFIER_SELF), f->_class->get_pointer());
	Node *link = new Node(KIND_DEREF_ADDRESS_SHIFT, e.offset, ps->script, e.type);
	link->set_num_params(1);
	link->params[0] = self;
	return link;
}

Node *exlink_make_func_class(SyntaxTree *ps, Function *f, ClassFunction &cf)
{
	Node *self = ps->add_node_local_var(f->__get_var(IDENTIFIER_SELF), f->_class->get_pointer());
	Node *link;
	if (cf.virtual_index >= 0){
		link = new Node(KIND_VIRTUAL_FUNCTION, cf.virtual_index, cf.script, cf.return_type);
	}else{
		link = new Node(KIND_FUNCTION, cf.nr, cf.script, cf.return_type);
	}
	link->set_num_params(cf.param_types.num);
	link->set_instance(self);
	return link;
}

Array<Node*> SyntaxTree::GetExistenceShared(const string &name)
{
	Array<Node*> links;

	// global variables (=local variables in "RootOfAllEvil")
	foreachi(Variable *v, root_of_all_evil.var, i)
		if (v->name == name)
			return new Node(KIND_VAR_GLOBAL, i, script, v->type);

	// named constants
	foreachi(Constant *c, constants, i)
		if (name == c->name)
			return new Node(KIND_CONSTANT, i, script, c->type);

	// then the (real) functions
	foreachi(Function *f, functions, i)
		if (f->name == name){
			Node *n = new Node(KIND_FUNCTION, i, script, f->literal_return_type);
			n->set_num_params(f->num_params);
			links.add(n);
		}
	if (links.num > 0)
		return links;

	// types
	int w = WhichType(name);
	if (w >= 0)
		return new Node(KIND_TYPE, w, script, TypeClass);

	// ...unknown
	return {};
}

Array<Node*> SyntaxTree::GetExistence(const string &name, Block *block)
{
	Array<Node*> links;

	if (block){
		Function *f = block->function;

		// first test local variables
		int n = block->get_var(name);
		if (n >= 0)
			return exlink_make_var_local(this, f->var[n]->type, n);
		if (f->_class){
			if ((name == IDENTIFIER_SUPER) and (f->_class->parent))
				return exlink_make_var_local(this, f->_class->parent->get_pointer(), f->__get_var(IDENTIFIER_SELF));
			// class elements (within a class function)
			for (ClassElement &e: f->_class->elements)
				if (e.name == name)
					return exlink_make_var_element(this, f, e);
			for (ClassFunction &cf: f->_class->functions)
				if (cf.name == name)
					return exlink_make_func_class(this, f, cf);
		}
	}

	// shared stuff (global variables, functions)
	links = GetExistenceShared(name);
	if (links.num > 0)
		return links;

	// then the statements
	int w = WhichStatement(name);
	if (w >= 0){
		Node *n = new Node(KIND_STATEMENT, w, script, TypeVoid);
		n->set_num_params(Statements[w].num_params);
		return n;
	}

	// operators
	w = WhichPrimitiveOperator(name);
	if (w >= 0)
		return new Node(KIND_PRIMITIVE_OPERATOR, w, script, TypeUnknown);

	// in include files (only global)...
	for (Script *i: includes)
		links.append(i->syntax->GetExistenceShared(name));

	// ...unknown
	return links;
}

// expression naming a type
Class *SyntaxTree::FindType(const string &name)
{
	for (Class *c: classes)
		if (name == c->name)
			return c;
	for (Script *inc: includes)
		for (Class *c: inc->syntax->classes)
			if (name == c->name)
				return c;
	return nullptr;
}

Class *SyntaxTree::CreateNewClass(const string &name, Class::Type type, int size, int array_size, Class *sub)
{
	// check if it already exists
	for (Class *t: classes)
		if (name == t->name)
			return t;
	for (Script *inc: includes)
		for (Class *t: inc->syntax->classes)
			if (name == t->name)
				return t;

	// add new class
	Class *t = new Class(name, size, this);
	t->type = type;
	t->array_length = max(array_size, 0);
	t->name = name;
	t->size = size;
	t->parent = sub;
	classes.add(t);
	if (t->is_super_array() or t->is_dict()){
		Class *parent = t->parent;
		t->derive_from(TypeDynamicArray, false);
		t->parent = parent;
		AddMissingFunctionHeadersForClass(t);
	}else if (t->is_array()){
		AddMissingFunctionHeadersForClass(t);
	}
	return t;
}

Class *SyntaxTree::CreateArrayClass(Class *element_type, int num_elements)
{
	string name_pre = element_type->name;
	if (num_elements < 0){
		return CreateNewClass(name_pre + "[]",
				Class::Type::SUPER_ARRAY, config.super_array_size, num_elements, element_type);
	}else{
		return CreateNewClass(name_pre + format("[%d]", num_elements),
				Class::Type::ARRAY, element_type->size * num_elements, num_elements, element_type);
	}
}

Class *SyntaxTree::CreateDictClass(Class *element_type)
{
	return CreateNewClass(element_type->name + "{}",
			Class::Type::DICT, config.super_array_size, 0, element_type);
}

void SyntaxTree::ConvertInline()
{
	for (auto com: nodes)
		if (com->kind == KIND_FUNCTION){
			// inline function?
			int index = com->script->syntax->functions[com->link_no]->inline_no;
			if (index >= 0){
				msg_write(" >>>>>>>>>>>>>>>>> inline ....");
				com->kind = KIND_INLINE_FUNCTION;

				/*if (com->instance){
					msg_write("   INST");
					// dirty quick move
					com->param.insert(com->instance, 0);
					com->instance = NULL;
				}*/
			}
		}
}

Node *conv_cbr(SyntaxTree *ps, Node *c, int var)
{
	// convert
	if ((c->kind == KIND_VAR_LOCAL) and (c->link_no == var)){
		c->type = c->type->get_pointer();
		return ps->deref_node(c);
	}
	return c;
}

#if 0
void conv_return(SyntaxTree *ps, nodes *c)
{
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->params[i]);
	
	if ((c->kind == KIND_STATEMENT) and (c->link_no == COMMAND_RETURN)){
		msg_write("conv ret");
		ref_command_old(ps, c);
	}
}
#endif


Node *conv_calls(SyntaxTree *ps, Node *c, int tt)
{
	if ((c->kind == KIND_STATEMENT) and (c->link_no == STATEMENT_RETURN))
		if (c->params.num > 0){
			if ((c->params[0]->type->is_array()) /*or (c->Param[j]->Type->IsSuperArray)*/){
				c->set_param(0, ps->ref_node(c->params[0]));
			}
			return c;
		}

	if ((c->kind == KIND_FUNCTION) or (c->kind == KIND_VIRTUAL_FUNCTION) or (c->kind == KIND_ARRAY_BUILDER) or (c->kind == KIND_CONSTRUCTOR_AS_FUNCTION)){

		// parameters: array/class as reference
		for (int j=0;j<c->params.num;j++)
			if (c->params[j]->type->uses_call_by_reference()){
				c->set_param(j, ps->ref_node(c->params[j]));
			}

		// return: array reference (-> dereference)
		if ((c->type->is_array()) /*or (c->Type->IsSuperArray)*/){
			c->type = c->type->get_pointer();
			return ps->deref_node(c);
			//deref_command_old(this, c);
		}
	}

	// special string / list operators
	if (c->kind == KIND_OPERATOR){
		// parameters: super array as reference
		for (int j=0;j<c->params.num;j++)
			if ((c->params[j]->type->is_array()) or (c->params[j]->type->is_super_array())){
				c->set_param(j, ps->ref_node(c->params[j]));
			}
  	}
	return c;
}


// remove &*x
Node *easyfy_ref_deref(SyntaxTree *ps, Node *c, int l)
{
	if (c->kind == KIND_REFERENCE){
		if (c->params[0]->kind == KIND_DEREFERENCE){
			// remove 2 knots...
			return c->params[0]->params[0];
		}
	}
	return c;
}

// remove (*x)[] and (*x).y
Node *easyfy_shift_deref(SyntaxTree *ps, Node *c, int l)
{
	if ((c->kind == KIND_ADDRESS_SHIFT) or (c->kind == KIND_ARRAY)){
		if (c->params[0]->kind == KIND_DEREFERENCE){
			// unify 2 knots (remove 1)
			Node *t = c->params[0]->params[0];
			c->kind = (c->kind == KIND_ADDRESS_SHIFT) ? KIND_DEREF_ADDRESS_SHIFT : KIND_POINTER_AS_ARRAY;
			c->set_param(0, t);
			return c;
		}
	}

	return c;
}

void convert_return_by_memory(SyntaxTree *ps, Block *b, Function *f)
{
	ps->script->cur_func = f;

	foreachib(Node *c, b->nodes, i){
		// recursion...
		if (c->kind == KIND_BLOCK)
			convert_return_by_memory(ps, c->as_block(), f);
		if ((c->kind != KIND_STATEMENT) or (c->link_no != STATEMENT_RETURN))
			continue;

		// convert into   *-return- = param
		Node *p_ret = nullptr;
		foreachi(Variable *v, f->var, i)
			if (v->name == IDENTIFIER_RETURN_VAR){
				p_ret = ps->AddNode(KIND_VAR_LOCAL, i, v->type);
			}
		if (!p_ret)
			ps->DoError("-return- not found...");
		Node *ret = ps->deref_node(p_ret);
		Node *op = ps->LinkOperator(OPERATOR_ASSIGN, ret, c->params[0]);
		if (!op)
			ps->DoError("no = operator for return from function found: " + f->name);
		b->nodes.insert(op, i);

		c->set_num_params(0);

		_foreach_it_.update();
	}
}

// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void SyntaxTree::ConvertCallByReference()
{
	if (config.verbose)
		msg_write("ConvertCallByReference");
	// convert functions
	for (Function *f: functions){
		
		// parameter: array/class as reference
		for (int j=0;j<f->num_params;j++)
			if (f->var[j]->type->uses_call_by_reference()){
				f->var[j]->type = f->var[j]->type->get_pointer();

				// internal usage...
				transform_block(f->block, [&](Node *n){ return conv_cbr(this, n, j); });
			}

		// return: array as reference
#if 0
		if ((f->return_type->is_array) /*or (f->Type->IsSuperArray)*/){
			f->return_type = GetPointerType(f->return_type);
			/*for (int k=0;k<f->Block->Command.num;k++)
				conv_return(this, f->Block->Command[k]);*/
			// no need... return gets converted automatically (all calls...)
		}
#endif
	}

	// convert return...
	for (Function *f: functions)
		if (f->return_type->uses_return_by_memory())
			convert_return_by_memory(this, f->block, f);

	// convert function calls
	transform([&](Node *n){ return conv_calls(this, n, 0); });
}


void SyntaxTree::SimplifyRefDeref()
{
	// remove &*
	transform([&](Node *n){ return easyfy_ref_deref(this, n, 0); });
}

void SyntaxTree::SimplifyShiftDeref()
{
	// remove &*
	transform([&](Node *n){ return easyfy_shift_deref(this, n, 0); });
}

int __get_pointer_add_int()
{
	if (config.abi == Asm::INSTRUCTION_SET_AMD64)
		return INLINE_INT64_ADD_INT;
	return INLINE_INT_ADD;
}

Node *SyntaxTree::BreakDownComplicatedCommand(Node *c)
{
	if (c->kind == KIND_ARRAY){

		Class *el_type = c->type;

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
		Node *c_size = add_node_const(AddConstant(TypeInt));
		c_size->as_const()->as_int() = el_type->size;
		// offset = size * index
		Node *c_offset = add_node_operator_by_inline(c_index, c_size, INLINE_INT_MULTIPLY);
		c_offset->type = TypeInt;//TypePointer;
		// address = &array + offset
		Node *c_address = add_node_operator_by_inline(c_ref_array, c_offset, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	}else if (c->kind == KIND_POINTER_AS_ARRAY){

		Class *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

		Node *c_index = c->params[1];
		Node *c_ref_array = c->params[0];
		// create command for size constant
		Node *c_size = add_node_const(AddConstant(TypeInt));
		c_size->as_const()->as_int() = el_type->size;
		// offset = size * index
		Node *c_offset = add_node_operator_by_inline(c_index, c_size, INLINE_INT_MULTIPLY);
		c_offset->type = TypeInt;
		// address = &array + offset
		Node *c_address = add_node_operator_by_inline(c_ref_array, c_offset, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	}else if (c->kind == KIND_ADDRESS_SHIFT){

		Class *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

		// & struct
		Node *c_ref_struct = ref_node(c->params[0]);
		// create command for shift constant
		Node *c_shift = add_node_const(AddConstant(TypeInt));
		c_shift->as_const()->as_int() = c->link_no;
		// address = &struct + shift
		Node *c_address = add_node_operator_by_inline(c_ref_struct, c_shift, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	}else if (c->kind == KIND_DEREF_ADDRESS_SHIFT){

		Class *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		Node *c_ref_struct = c->params[0];
		// create command for shift constant
		Node *c_shift = add_node_const(AddConstant(TypeInt));
		c_shift->as_const()->as_int() = c->link_no;
		// address = &struct + shift
		Node *c_address = add_node_operator_by_inline(c_ref_struct, c_shift, __get_pointer_add_int());
		c_address->type = el_type->get_pointer();//TypePointer;
		// * address
		return deref_node(c_address);
	}
	return c;
}

Node* SyntaxTree::transform_node(Node *n, std::function<Node*(Node*)> F)
{
	for (int i=0; i<n->params.num; i++)
		n->set_param(i, transform_node(n->params[i], F));

	if (n->kind == KIND_BLOCK)
		transform_block(n->as_block(), F);

	if (n->instance)
		n->set_instance(transform_node(n->instance, F));

	return F(n);
}

static Node* _transform_insert_before_ = nullptr;

void SyntaxTree::transform_block(Block *block, std::function<Node*(Node*)> F)
{
	//foreachi (Node *n, block->nodes, i){
	for (int i=0; i<block->nodes.num; i++){
		block->nodes[i] = transform_node(block->nodes[i], F);
		if (_transform_insert_before_){
			if (config.verbose)
				msg_error("INSERT BEFORE...");
			block->nodes.insert(_transform_insert_before_, i);
			_transform_insert_before_ = nullptr;
			i ++;
		}
	}
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::transform(std::function<Node*(Node*)> F)
{
	_transform_insert_before_ = nullptr;
	for (Function *f: functions){
		cur_func = f;
		transform_block(f->block, F);
	}
}

Node *conv_constr_func(SyntaxTree *ps, Node *n)
{
	if (n->kind == KIND_CONSTRUCTOR_AS_FUNCTION){
		if (config.verbose){
			msg_error("constr func....");
			ps->ShowNode(n, ps->cur_func);
		}
		_transform_insert_before_ = ps->cp_node(n);
		_transform_insert_before_->kind = KIND_FUNCTION;
		_transform_insert_before_->type = TypeVoid;
		if (config.verbose)
			ps->ShowNode(_transform_insert_before_, ps->cur_func);

		// n->instance should be a reference to local... FIXME
		return ps->cp_node(n->instance->params[0]);
	}
	return n;
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::BreakDownComplicatedCommands()
{
	if (config.verbose){
		Show("break:a");
		msg_write("BreakDownComplicatedCommands");
	}
	transform([&](Node* n){ return BreakDownComplicatedCommand(n); });
	transform([&](Node* n){ return conv_constr_func(this, n); });
	if (config.verbose)
		Show("break:b");
}

void MapLVSX86Return(Function *f)
{
	if (f->return_type->uses_return_by_memory()){
		foreachi(Variable *v, f->var, i)
			if (v->name == IDENTIFIER_RETURN_VAR){
				v->_offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void MapLVSX86Self(Function *f)
{
	if (f->_class){
		foreachi(Variable *v, f->var, i)
			if (v->name == IDENTIFIER_SELF){
				v->_offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void SyntaxTree::MapLocalVariablesToStack()
{
	for (Function *f: functions){
		f->_param_size = 2 * config.pointer_size; // space for return value and eBP
		if (config.instruction_set == Asm::INSTRUCTION_SET_X86){
			f->_var_size = 0;

			if (config.abi == ABI_WINDOWS_32){
				// map "self" to the VERY first parameter
				MapLVSX86Self(f);

				// map "-return-" to the first parameter
				MapLVSX86Return(f);
			}else{
				// map "-return-" to the VERY first parameter
				MapLVSX86Return(f);

				// map "self" to the first parameter
				MapLVSX86Self(f);
			}

			foreachi(Variable *v, f->var, i){
				if ((f->_class) and (v->name == IDENTIFIER_SELF))
					continue;
				if (v->name == IDENTIFIER_RETURN_VAR)
					continue;
				int s = mem_align(v->type->size, 4);
				if (i < f->num_params){
					// parameters
					v->_offset = f->_param_size;
					f->_param_size += s;
				}else{
					// "real" local variables
					v->_offset = - f->_var_size - s;
					f->_var_size += s;
				}				
			}
		}else if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64){
			f->_var_size = 0;
			
			foreachi(Variable *v, f->var, i){
				long long s = mem_align(v->type->size, 4);
				v->_offset = - f->_var_size - s;
				f->_var_size += s;
			}
		}else if (config.instruction_set == Asm::INSTRUCTION_SET_ARM){
			f->_var_size = 0;

			foreachi(Variable *v, f->var, i){
				int s = mem_align(v->type->size, 4);
				v->_offset = f->_var_size + s;
				f->_var_size += s;
			}
		}
	}
}


// no included scripts may be deleted before us!!!
SyntaxTree::~SyntaxTree()
{
	// delete all types created by this script
	for (Class *t: classes)
		if (t->owner == this)
			delete(t);

	if (asm_meta_info)
		delete(asm_meta_info);

	for (Node *c: nodes)
		delete(c);
	
	for (Function *f: functions)
		delete(f);

	for (Constant *c: constants)
		delete(c);
}

void SyntaxTree::ShowNode(Node *c, Function *f)
{
	string orig;
	if (c->script->syntax != this)
		orig = " << " + c->script->filename;
	msg_write("[" + Kind2Str(c->kind) + "] " + c->type->name + " " + LinkNr2Str(c->script->syntax, f, c->kind, c->link_no) + orig);
	msg_right();
	if (c->instance)
		ShowNode(c->instance, f);
	//msg_write(c->param.num);
	if (c->params.num > 10)
		return;
	for (Node *p: c->params)
		if (p)
			ShowNode(p, f);
		else
			msg_write("<param nil>");
	msg_left();
}

void SyntaxTree::ShowBlock(Block *b)
{
	msg_write("block");
	msg_right();
	for (Node *c: b->nodes){
		if (c->kind == KIND_BLOCK)
			ShowBlock(c->as_block());
		else
			ShowNode(c, b->function);
	}
	msg_left();
	msg_write("/block");
}

void SyntaxTree::ShowFunction(Function *f, const string &stage)
{
	if (!config.allow_output(f, stage))
		return;
	msg_write("[function] " + f->return_type->name + " " + f->name);
	cur_func = f;
	ShowBlock(f->block);
}

void SyntaxTree::Show(const string &stage)
{
	if (!config.allow_output_stage(stage))
		return;
	msg_write("--------- Syntax of " + script->filename + "  " + stage + " ---------");
	msg_right();
	for (Function *f: functions)
		if (!f->is_extern)
			ShowFunction(f, stage);
	msg_left();
	msg_write("\n\n");
}

};
