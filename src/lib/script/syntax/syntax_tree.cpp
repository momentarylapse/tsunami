#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{

//#define ScriptDebug


/*#define PRESCRIPT_DB_LEVEL	2
#define db_f(msg,level)		msg_db_f(msg,level+PRESCRIPT_DB_LEVEL)*/

extern Type *TypeDynamicArray;


bool next_extern = false;
bool next_const = false;

Command *conv_cbr(SyntaxTree *ps, Command *c, int var);
 
void Constant::setInt(int i)
{
	(*(int*)(value.data)) = i;
}

int Constant::getInt()
{
	return (*(int*)(value.data));
}

Command *SyntaxTree::cp_command(Command *c)
{
	Command *cmd = AddCommand(c->kind, c->link_no, c->type, c->script);
	cmd->set_num_params(c->param.num);
	for (int i=0;i<c->param.num;i++)
		if (c->param[i])
			cmd->set_param(i, cp_command(c->param[i]));
	if (c->instance)
		cmd->set_instance(cp_command(c->instance));
	return cmd;
}

Command *SyntaxTree::ref_command(Command *sub, Type *override_type)
{
	Type *t = override_type ? override_type : sub->type->GetPointer();
	Command *c = AddCommand(KIND_REFERENCE, 0, t);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Command *SyntaxTree::deref_command(Command *sub, Type *override_type)
{
	Command *c = AddCommand(KIND_UNKNOWN, 0, TypeVoid);
	c->kind = KIND_DEREFERENCE;
	c->set_num_params(1);
	c->set_param(0, sub);
	if (override_type)
		c->type = override_type;
	else
		c->type = sub->type->parent;
	return c;
}

Command *SyntaxTree::shift_command(Command *sub, bool deref, int shift, Type *type)
{
	Command *c= AddCommand(deref ? KIND_DEREF_ADDRESS_SHIFT : KIND_ADDRESS_SHIFT, shift, type);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Command *SyntaxTree::add_command_compilerfunc(int cf)
{
	Command *c = AddCommand(KIND_UNKNOWN, 0, TypeVoid);

	//if (FlagCompileOS)
	//	DoError(format("external function call (%s) not allowed with #os", PreCommands[CF].name.c_str()));

// a function the compiler knows
	c->kind = KIND_COMPILER_FUNCTION;
	c->link_no = cf;
	c->script = Packages[0].script;
	c->instance = NULL;

	c->set_num_params(PreCommands[cf].param.num);
	c->type = PreCommands[cf].return_type;
	return c;
}

Command *SyntaxTree::add_command_classfunc(ClassFunction *f, Command *inst, bool force_non_virtual)
{
	Command *c;
	if ((f->virtual_index >= 0) and (!force_non_virtual))
		c = AddCommand(KIND_VIRTUAL_FUNCTION, f->virtual_index, f->return_type);
	else
		c = AddCommand(KIND_FUNCTION, f->nr, f->return_type);
	c->script = f->script;
	c->set_instance(inst);
	c->set_num_params(f->param_type.num);
	return c;
}

Command *SyntaxTree::add_command_func(Script *script, int no, Type *return_type)
{
	Command *c = AddCommand(KIND_FUNCTION, no, return_type);
	c->script = script;
	c->set_num_params(script->syntax->functions[no]->num_params);
	return c;
}


Command *SyntaxTree::add_command_operator(Command *p1, Command *p2, int op)
{
	Command *cmd = AddCommand(KIND_OPERATOR, op, PreOperators[op].return_type);
	bool unitary = ((PreOperators[op].param_type_1 == TypeVoid) or (PreOperators[op].param_type_2 == TypeVoid));
	cmd->set_num_params( unitary ? 1 : 2); // unary / binary
	cmd->set_param(0, p1);
	if (!unitary)
		cmd->set_param(1, p2);
	return cmd;
}


Command *SyntaxTree::add_command_local_var(int no, Type *type)
{
	if (no < 0)
		script->DoErrorInternal("negative local variable index");
	return AddCommand(KIND_VAR_LOCAL, no, type);
}

Command *SyntaxTree::add_command_parray(Command *p, Command *index, Type *type)
{
	Command *cmd_el = AddCommand(KIND_POINTER_AS_ARRAY, 0, type);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

Command *SyntaxTree::add_command_block(Block *b)
{
	return AddCommand(KIND_BLOCK, b->index, TypeVoid);
}

SyntaxTree::SyntaxTree(Script *_script) :
	root_of_all_evil(this, "RootOfAllEvil", TypeVoid)
{
	root_of_all_evil.block = AddBlock(&root_of_all_evil, NULL);

	flag_string_const_as_cstring = false;
	flag_immortal = false;
	cur_func = NULL;
	script = _script;
	asm_meta_info = new Asm::MetaInfo;
	for_index_count = 0;
	Exp.cur_line = NULL;

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
	
	if (config.verbose)
		Show();

	ConvertCallByReference();

	/*if (FlagShow)
		Show();*/

	Simplify();
	
	PreProcessor();

	if (config.verbose)
		Show();

	Exp.clear();
}

string Kind2Str(int kind)
{
	if (kind == KIND_VAR_LOCAL)			return "local variable";
	if (kind == KIND_VAR_GLOBAL)			return "global variable";
	if (kind == KIND_VAR_FUNCTION)		return "function as variable";
	if (kind == KIND_CONSTANT)			return "constant";
	if (kind == KIND_REF_TO_CONST)			return "reference to const";
	if (kind == KIND_FUNCTION)			return "function";
	if (kind == KIND_VIRTUAL_FUNCTION)	return "virtual function";
	if (kind == KIND_COMPILER_FUNCTION)	return "compiler function";
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

string LinkNr2Str(SyntaxTree *s, int kind, long long nr)
{
	if (kind == KIND_VAR_LOCAL)			return i2s(nr);//s->cur_func->var[nr].name;
	if (kind == KIND_VAR_GLOBAL)			return s->root_of_all_evil.var[nr].name;
	if (kind == KIND_VAR_FUNCTION)		return s->functions[nr]->name;
	if (kind == KIND_CONSTANT)			return i2s(nr);//s->Constants[nr].type->var2str(s->Constants[nr].data);
	if (kind == KIND_FUNCTION)			return s->functions[nr]->name;
	if (kind == KIND_VIRTUAL_FUNCTION)	return i2s(nr);//s->Functions[nr]->name;
	if (kind == KIND_COMPILER_FUNCTION)	return PreCommands[nr].name;
	if (kind == KIND_OPERATOR)			return PreOperators[nr].str();
	if (kind == KIND_PRIMITIVE_OPERATOR)	return PrimitiveOperators[nr].name;
	if (kind == KIND_BLOCK)				return i2s(nr);
	if (kind == KIND_ADDRESS_SHIFT)		return i2s(nr);
	if (kind == KIND_ARRAY)				return "(no LinkNr)";
	if (kind == KIND_POINTER_AS_ARRAY)		return "(no LinkNr)";
	if (kind == KIND_REFERENCE)			return "(no LinkNr)";
	if (kind == KIND_DEREFERENCE)		return "(no LinkNr)";
	if (kind == KIND_DEREF_ADDRESS_SHIFT)	return i2s(nr);
	if (kind == KIND_TYPE)				return s->types[nr]->name;
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
		v.name = root_of_all_evil.var[i].name;
		v.size = root_of_all_evil.var[i].type->size;
		v.pos = script->g_var[i];
		asm_meta_info->global_var.add(v);
	}
}



// constants

int SyntaxTree::AddConstant(Type *type)
{
	Constant c;
	c.name = "-none-";
	c.type = type;
	c.value.resize(max(type->size, config.pointer_size));
	constants.add(c);
	return constants.num - 1;
}

Block *SyntaxTree::AddBlock(Function *f, Block *parent)
{
	Block *b = new Block;
	b->level = 0;
	b->index = blocks.num;
	b->function = f;
	b->parent = parent;
	if (parent)
		b->level = parent->level + 1;
	blocks.add(b);
	return b;
}



inline void set_command(Command *&a, Command *b)
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

void Block::add(Command *c)
{
	commands.add(c);
	c->ref_count ++;
}

void Block::set(int index, Command *c)
{
	set_command(commands[index], c);
}

int Block::add_var(const string &name, Type *type)
{
	if (get_var(name) >= 0)
		function->tree->DoError(format("variable '%s' already declared in this context", name.c_str()));
	Variable v;
	v.name = name;
	v.type = type;
	v._offset = 0;
	v.is_extern = next_extern;
	function->var.add(v);
	int n = function->var.num - 1;
	vars.add(n);
	return n;
}

int Block::get_var(const string &name)
{
	for (int i: vars)
		if (function->var[i].name == name)
			return i;
	if (parent)
		return parent->get_var(name);
	return -1;
}

// functions


Function::Function(SyntaxTree *_tree, const string &_name, Type *_return_type)
{
	tree = _tree;
	name = _name;
	block = NULL;
	num_params = 0;
	return_type = _return_type;
	literal_return_type = _return_type;
	_class = NULL;
	is_extern = false;
	auto_implement = false;
	is_pure = false;
	_param_size = 0;
	_var_size = 0;
	_logical_line_no = -1;
	_exp_no = -1;
	inline_no = -1;
}

int Function::__get_var(const string &name)
{
	return block->get_var(name);
}

Function *SyntaxTree::AddFunction(const string &name, Type *type)
{
	Function *f = new Function(this, name, type);
	functions.add(f);
	f->block = AddBlock(f, NULL);
	return f;
}

Command::Command()
{
}

Command::Command(int _kind, long long _link_no, Script *_script, Type *_type)
{
	type = _type;
	kind = _kind;
	link_no = _link_no;
	instance = NULL;
	script = _script;
	ref_count = 0;
}

Block *Command::as_block() const
{
	return script->syntax->blocks[link_no];
}

void Command::set_instance(Command *p)
{
	set_command(instance, p);
}

void Command::set_num_params(int n)
{
	param.resize(n);
}

void Command::set_param(int index, Command *p)
{
	if ((index < 0) or (index >= param.num)){
		this->script->syntax->ShowCommand(this);
		script->DoErrorInternal(format("Command.set_param...  %d %d", index, param.num));
	}
	set_command(param[index], p);
}

Command *SyntaxTree::AddCommand(int kind, long long link_no, Type *type)
{
	Command *c = new Command(kind, link_no, script, type);
	commands.add(c);
	return c;
}

Command *SyntaxTree::AddCommand(int kind, long long link_no, Type *type, Script *s)
{
	Command *c = new Command(kind, link_no, s, type);
	commands.add(c);
	return c;
}


Command *SyntaxTree::add_command_const(int nc)
{
	return AddCommand(KIND_CONSTANT, nc, constants[nc].type);
}

int SyntaxTree::WhichPrimitiveOperator(const string &name)
{
	for (int i=0;i<NumPrimitiveOperators;i++)
		if (name == PrimitiveOperators[i].name)
			return i;
	return -1;
}

int SyntaxTree::WhichType(const string &name)
{
	for (int i=0;i<types.num;i++)
		if (name == types[i]->name)
			return i;

	return -1;
}

Array<int> MultipleFunctionList;

int SyntaxTree::WhichCompilerFunction(const string &name)
{
	MultipleFunctionList.clear();
	for (int i=0;i<PreCommands.num;i++)
		if (name == PreCommands[i].name)
			MultipleFunctionList.add(i);
			//return i;
	if (MultipleFunctionList.num > 0)
		return MultipleFunctionList[0];
	return -1;
}

Command exlink_make_var_local(SyntaxTree *ps, Type *t, int var_no)
{
	Command link;
	link.type = t;
	link.link_no = var_no;
	link.kind = KIND_VAR_LOCAL;
	link.set_num_params(0);
	link.script = ps->script;
	link.instance = NULL;
	return link;
}

Command exlink_make_var_element(SyntaxTree *ps, Function *f, ClassElement &e)
{
	Command link;
	Command *self = ps->add_command_local_var(f->__get_var(IDENTIFIER_SELF), f->_class->GetPointer());
	link.type = e.type;
	link.link_no = e.offset;
	link.kind = KIND_DEREF_ADDRESS_SHIFT;
	link.set_num_params(1);
	link.param[0] = self;
	link.script = ps->script;
	link.instance = NULL;
	return link;
}

Command exlink_make_func_class(SyntaxTree *ps, Function *f, ClassFunction &cf)
{
	Command link;
	Command *self = ps->add_command_local_var(f->__get_var(IDENTIFIER_SELF), f->_class->GetPointer());
	if (cf.virtual_index >= 0){
		link.kind = KIND_VIRTUAL_FUNCTION;
		link.link_no = cf.virtual_index;
	}else{
		link.kind = KIND_FUNCTION;
		link.link_no = cf.nr;
	}
	link.script = cf.script;
	link.type = cf.return_type;
	link.set_num_params(cf.param_type.num);
	link.instance = self;
	return link;
}

Array<Command> SyntaxTree::GetExistenceShared(const string &name)
{
	Array<Command> links;
	Command link;
	link.type = TypeUnknown;
	link.param.clear();
	link.script = script;
	link.instance = NULL;

	// global variables (=local variables in "RootOfAllEvil")
	foreachi(Variable &v, root_of_all_evil.var, i)
		if (v.name == name){
			link.type = v.type;
			link.link_no = i;
			link.kind = KIND_VAR_GLOBAL;
			links.add(link);
			return links;
		}

	// then the (real) functions
	foreachi(Function *f, functions, i)
		if (f->name == name){
			link.kind = KIND_FUNCTION;
			link.link_no = i;
			link.type = f->literal_return_type;
			link.set_num_params(f->num_params);
			links.add(link);
		}
	if (links.num > 0)
		return links;

	// types
	int w = WhichType(name);
	if (w >= 0){
		link.kind = KIND_TYPE;
		link.link_no = w;
		links.add(link);
		return links;
	}

	// ...unknown
	return links;
}

Array<Command> SyntaxTree::GetExistence(const string &name, Block *block)
{
	Array<Command> links;
	Command link;
	link.type = TypeUnknown;
	link.param.clear();
	link.script = script;
	link.instance = NULL;

	if (block){
		Function *f = block->function;

		// first test local variables
		int n = block->get_var(name);
		if (n >= 0){
			links.add(exlink_make_var_local(this, f->var[n].type, n));
			return links;
		}
		if (f->_class){
			if ((name == IDENTIFIER_SUPER) and (f->_class->parent)){
				links.add(exlink_make_var_local(this, f->_class->parent->GetPointer(), f->__get_var(IDENTIFIER_SELF)));
				return links;
			}
			// class elements (within a class function)
			for (ClassElement &e: f->_class->element)
				if (e.name == name){
					links.add(exlink_make_var_element(this, f, e));
					return links;
				}
			for (ClassFunction &cf: f->_class->function)
				if (cf.name == name){
					links.add(exlink_make_func_class(this, f, cf));
					return links;
				}
		}
	}

	// shared stuff (global variables, functions)
	links = GetExistenceShared(name);
	if (links.num > 0)
		return links;

	// then the compiler functions
	int w = WhichCompilerFunction(name);
	if (w >= 0){
		link.kind = KIND_COMPILER_FUNCTION;
		link.link_no = w;
		link.type = PreCommands[w].return_type;
		link.set_num_params(PreCommands[w].param.num);
		links.add(link);
		return links;
	}

	// operators
	w = WhichPrimitiveOperator(name);
	if (w >= 0){
		link.kind = KIND_PRIMITIVE_OPERATOR;
		link.link_no = w;
		return link;
	}

	// in include files (only global)...
	for (Script *i: includes)
		links.append(i->syntax->GetExistenceShared(name));

	// ...unknown
	return links;
}

// expression naming a type
Type *SyntaxTree::FindType(const string &name)
{
	for (int i=0;i<types.num;i++)
		if (name == types[i]->name)
			return types[i];
	for (Script *inc: includes)
		for (int i=0;i<inc->syntax->types.num;i++)
			if (name == inc->syntax->types[i]->name)
				return inc->syntax->types[i];
	return NULL;
}

// create a new type?
Type *SyntaxTree::AddType(Type *type)
{
	for (Type *t: types)
		if (type->name == t->name)
			return t;
	for (Script *inc: includes)
		for (Type *t: inc->syntax->types)
			if (type->name == t->name)
				return t;
	Type *t = new Type;
	*t = *type;
	t->owner = this;
	t->name = type->name;
	types.add(t);


	if (t->is_super_array){
		Type *parent = t->parent;
		t->DeriveFrom(TypeDynamicArray, false);
		t->parent = parent;
		AddFunctionHeadersForClass(t);
	}else if (t->is_array){
		AddFunctionHeadersForClass(t);
	}
	return t;
}

Type *SyntaxTree::CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub)
{
	Type nt;
	nt.is_array = is_array and (array_size >= 0);
	nt.is_super_array = is_array and (array_size < 0);
	nt.array_length = max(array_size, 0);
	nt.is_pointer = is_pointer;
	nt.is_silent = is_silent;
	nt.name = name;
	nt.size = size;
	nt.parent = sub;
	return AddType(&nt);
}

Type *SyntaxTree::CreateArrayType(Type *element_type, int num_elements, const string &_name_pre, const string &suffix)
{
	string name_pre = _name_pre;
	if (name_pre.num == 0)
		name_pre = element_type->name;
	if (num_elements < 0){
		return CreateNewType(name_pre + "[]" +  suffix,
			config.super_array_size, false, false, true, num_elements, element_type);
	}else{
		return CreateNewType(name_pre + format("[%d]", num_elements) + suffix,
			element_type->size * num_elements, false, false, true, num_elements, element_type);
	}
}



#define TRANSFORM_COMMANDS_RECURSION(FUNC, PREPARAMS, POSTPARAMS, CMD) \
	for (int i=0;i<(CMD)->param.num;i++) \
		(CMD)->set_param(i, FUNC(PREPARAMS, (CMD)->param[i], POSTPARAMS)); \
	if ((CMD)->kind == KIND_BLOCK){ \
		foreachi(Command *cc, (CMD)->as_block()->commands, i) \
			(CMD)->as_block()->set(i, FUNC(PREPARAMS, cc, POSTPARAMS)); \
	} \
	if ((CMD)->instance) \
		(CMD)->set_instance(FUNC(PREPARAMS, (CMD)->instance, POSTPARAMS));

Command *conv_cbr(SyntaxTree *ps, Command *c, int var)
{
	// recursion...
	TRANSFORM_COMMANDS_RECURSION(conv_cbr, ps, var, c);

	// convert
	if ((c->kind == KIND_VAR_LOCAL) and (c->link_no == var)){
		c->type = c->type->GetPointer();
		return ps->deref_command(c);
	}
	return c;
}

#if 0
void conv_return(SyntaxTree *ps, commands *c)
{
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->param[i]);
	
	if ((c->kind == KIND_COMPILER_FUNCTION) and (c->link_no == COMMAND_RETURN)){
		msg_write("conv ret");
		ref_command_old(ps, c);
	}
}
#endif


Command *conv_calls(SyntaxTree *ps, Command *c, int tt)
{
	// recursion...
	TRANSFORM_COMMANDS_RECURSION(conv_calls, ps, tt, c)

	if ((c->kind == KIND_COMPILER_FUNCTION) and (c->link_no == COMMAND_RETURN))
		if (c->param.num > 0){
			if ((c->param[0]->type->is_array) /*or (c->Param[j]->Type->IsSuperArray)*/){
				c->set_param(0, ps->ref_command(c->param[0]));
			}
			return c;
		}

	if ((c->kind == KIND_FUNCTION) or (c->kind == KIND_VIRTUAL_FUNCTION) or (c->kind == KIND_COMPILER_FUNCTION) or (c->kind == KIND_ARRAY_BUILDER)){
		// parameters: array/class as reference
		for (int j=0;j<c->param.num;j++)
			if (c->param[j]->type->UsesCallByReference()){
				c->set_param(j, ps->ref_command(c->param[j]));
			}

		// return: array reference (-> dereference)
		if ((c->type->is_array) /*or (c->Type->IsSuperArray)*/){
			c->type = c->type->GetPointer();
			return ps->deref_command(c);
			//deref_command_old(this, c);
		}
	}

	// special string / list operators
	if (c->kind == KIND_OPERATOR){
		// parameters: super array as reference
		for (int j=0;j<c->param.num;j++)
			if ((c->param[j]->type->is_array) or (c->param[j]->type->is_super_array)){
				c->set_param(j, ps->ref_command(c->param[j]));
			}
  	}
	return c;
}


// remove &*x and (*x)[] and (*x).y
Command *easyfy(SyntaxTree *ps, Command *c, int l)
{
	//msg_write(l);
	//msg_write("a");
	
	// recursion...
	for (int i=0;i<c->param.num;i++)
		c->set_param(i, easyfy(ps, c->param[i], l+1));
	if (c->kind == KIND_BLOCK)
		for (int i=0;i<c->as_block()->commands.num;i++)
			c->as_block()->set(i, easyfy(ps, c->as_block()->commands[i], l+1));
	if (c->instance)
		c->set_instance(easyfy(ps, c->instance, l+1));
	
	//msg_write("b");


	// convert
	if (c->kind == KIND_REFERENCE){
		if (c->param[0]->kind == KIND_DEREFERENCE){
			// remove 2 knots...
			return c->param[0]->param[0];
		}
	}else if ((c->kind == KIND_ADDRESS_SHIFT) or (c->kind == KIND_ARRAY)){
		if (c->param[0]->kind == KIND_DEREFERENCE){
			// unify 2 knots (remove 1)
			Command *t = c->param[0]->param[0];
			c->kind = (c->kind == KIND_ADDRESS_SHIFT) ? KIND_DEREF_ADDRESS_SHIFT : KIND_POINTER_AS_ARRAY;
			c->set_param(0, t);
			return c;
		}
	}
	//msg_write("ok");
	return c;
}

void convert_return_by_memory(SyntaxTree *ps, Block *b, Function *f)
{
	ps->script->cur_func = f;

	foreachib(Command *c, b->commands, i){
		// recursion...
		if (c->kind == KIND_BLOCK)
			convert_return_by_memory(ps, c->as_block(), f);
		if ((c->kind != KIND_COMPILER_FUNCTION) or (c->link_no != COMMAND_RETURN))
			continue;

		// convert into   *-return- = param
		Command *p_ret = NULL;
		foreachi(Variable &v, f->var, i)
			if (v.name == IDENTIFIER_RETURN_VAR){
				p_ret = ps->AddCommand(KIND_VAR_LOCAL, i, v.type);
			}
		if (!p_ret)
			ps->DoError("-return- not found...");
		Command *ret = ps->deref_command(p_ret);
		Command *op = ps->LinkOperator(OPERATOR_ASSIGN, ret, c->param[0]);
		if (!op)
			ps->DoError("no = operator for return from function found: " + f->name);
		b->commands.insert(op, i);

		c->set_num_params(0);

		_foreach_it_.update();
	}
}

// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void SyntaxTree::ConvertCallByReference()
{
	// convert functions
	for (Function *f: functions){
		
		// parameter: array/class as reference
		for (int j=0;j<f->num_params;j++)
			if (f->var[j].type->UsesCallByReference()){
				f->var[j].type = f->var[j].type->GetPointer();

				// internal usage...
				foreachi(Command *c, f->block->commands, i)
					f->block->commands[i] = conv_cbr(this, c, j);
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
		if (f->return_type->UsesReturnByMemory())
			convert_return_by_memory(this, f->block, f);

	// convert function calls
	for (Function *f: functions)
		foreachi(Command *c, f->block->commands, i)
			f->block->commands[i] = conv_calls(this, c, 0);
}


void SyntaxTree::Simplify()
{
	// remove &*
	for (Function *f: functions)
		foreachi(Command *c, f->block->commands, i)
			f->block->commands[i] = easyfy(this, c, 0);
}

int __get_pointer_add_int()
{
	if (config.abi == Asm::INSTRUCTION_SET_AMD64)
		return OperatorInt64AddInt;
	return OperatorIntAdd;
}

Command *SyntaxTree::BreakDownComplicatedCommand(Command *c)
{
	// recursion...
	for (int i=0;i<c->param.num;i++)
		c->set_param(i, BreakDownComplicatedCommand(c->param[i]));
	if (c->kind == KIND_BLOCK){
		for (int i=0;i<c->as_block()->commands.num;i++)
			c->as_block()->set(i, BreakDownComplicatedCommand(c->as_block()->commands[i]));
	}
	if (c->instance)
		c->set_instance(BreakDownComplicatedCommand(c->instance));

	if (c->kind == KIND_ARRAY){

		Type *el_type = c->type;

// array el -> array
//          -> index
//
// * -> + -> & array
//        -> * -> size
//             -> index

		Command *c_index = c->param[1];
		// & array
		Command *c_ref_array = ref_command(c->param[0]);
		// create command for size constant
		int nc = AddConstant(TypeInt);
		constants[nc].setInt(el_type->size);
		Command *c_size = add_command_const(nc);
		// offset = size * index
		Command *c_offset = add_command_operator(c_index, c_size, OperatorIntMultiply);
		c_offset->type = TypeInt;//TypePointer;
		// address = &array + offset
		Command *c_address = add_command_operator(c_ref_array, c_offset, __get_pointer_add_int());
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}else if (c->kind == KIND_POINTER_AS_ARRAY){

		Type *el_type = c->type;

// array el -> array_pointer
//          -> index
//
// * -> + -> array_pointer
//        -> * -> size
//             -> index

		Command *c_index = c->param[1];
		Command *c_ref_array = c->param[0];
		// create command for size constant
		int nc = AddConstant(TypeInt);
		constants[nc].setInt(el_type->size);
		Command *c_size = add_command_const(nc);
		// offset = size * index
		Command *c_offset = add_command_operator(c_index, c_size, OperatorIntMultiply);
		c_offset->type = TypeInt;
		// address = &array + offset
		Command *c_address = add_command_operator(c_ref_array, c_offset, __get_pointer_add_int());
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}else if (c->kind == KIND_ADDRESS_SHIFT){

		Type *el_type = c->type;

// struct el -> struct
//           -> shift (LinkNr)
//
// * -> + -> & struct
//        -> shift

		// & struct
		Command *c_ref_struct = ref_command(c->param[0]);
		// create command for shift constant
		int nc = AddConstant(TypeInt);
		constants[nc].setInt(c->link_no);
		Command *c_shift = add_command_const(nc);
		// address = &struct + shift
		Command *c_address = add_command_operator(c_ref_struct, c_shift, __get_pointer_add_int());
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}else if (c->kind == KIND_DEREF_ADDRESS_SHIFT){

		Type *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		Command *c_ref_struct = c->param[0];
		// create command for shift constant
		int nc = AddConstant(TypeInt);
		constants[nc].setInt(c->link_no);
		Command *c_shift = add_command_const(nc);
		// address = &struct + shift
		Command *c_address = add_command_operator(c_ref_struct, c_shift, __get_pointer_add_int());
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}
	return c;
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::BreakDownComplicatedCommands()
{
	for (Function *f: functions){
		foreachi(Command *c, f->block->commands, i)
			f->block->commands[i] = BreakDownComplicatedCommand(c);
	}
}

void MapLVSX86Return(Function *f)
{
	if (f->return_type->UsesReturnByMemory()){
		foreachi(Variable &v, f->var, i)
			if (v.name == IDENTIFIER_RETURN_VAR){
				v._offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void MapLVSX86Self(Function *f)
{
	if (f->_class){
		foreachi(Variable &v, f->var, i)
			if (v.name == IDENTIFIER_SELF){
				v._offset = f->_param_size;
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

			foreachi(Variable &v, f->var, i){
				if ((f->_class) and (v.name == IDENTIFIER_SELF))
					continue;
				if (v.name == IDENTIFIER_RETURN_VAR)
					continue;
				int s = mem_align(v.type->size, 4);
				if (i < f->num_params){
					// parameters
					v._offset = f->_param_size;
					f->_param_size += s;
				}else{
					// "real" local variables
					v._offset = - f->_var_size - s;
					f->_var_size += s;
				}				
			}
		}else if (config.instruction_set == Asm::INSTRUCTION_SET_AMD64){
			f->_var_size = 0;
			
			foreachi(Variable &v, f->var, i){
				int s = mem_align(v.type->size, 4);
				v._offset = - f->_var_size - s;
				f->_var_size += s;
			}
		}else if (config.instruction_set == Asm::INSTRUCTION_SET_ARM){
			f->_var_size = 0;

			foreachi(Variable &v, f->var, i){
				int s = mem_align(v.type->size, 4);
				v._offset = f->_var_size + s;
				f->_var_size += s;
			}
		}
	}
}


// no included scripts may be deleted before us!!!
SyntaxTree::~SyntaxTree()
{
	// delete all types created by this script
	for (Type *t: types)
		if (t->owner == this)
			delete(t);

	if (asm_meta_info)
		delete(asm_meta_info);

	for (Command *c: commands)
		delete(c);

	for (Block *b: blocks)
		delete(b);
	
	for (Function *f: functions)
		delete(f);
}

void SyntaxTree::ShowCommand(Command *c)
{
	string orig;
	if (c->script->syntax != this)
		orig = " << " + c->script->filename;
	msg_write("[" + Kind2Str(c->kind) + "] " + c->type->name + " " + LinkNr2Str(c->script->syntax,c->kind,c->link_no) + orig);
	msg_right();
	if (c->instance)
		ShowCommand(c->instance);
	for (Command *p: c->param)
		if (p)
			ShowCommand(p);
		else
			msg_write("<param nil>");
	msg_left();
}

void SyntaxTree::ShowBlock(Block *b)
{
	msg_write("block");
	msg_right();
	for (Command *c: b->commands){
		if (c->kind == KIND_BLOCK)
			ShowBlock(c->as_block());
		else
			ShowCommand(c);
	}
	msg_left();
	msg_write("/block");
}

void SyntaxTree::ShowFunction(Function *f)
{
	msg_write("[function] " + f->return_type->name + " " + f->name);
	cur_func = f;
	ShowBlock(f->block);
}

void SyntaxTree::Show()
{
	msg_write("--------- Syntax of " + script->filename + " ---------");
	msg_right();
	for (Function *f: functions)
		if (!f->is_extern)
			ShowFunction(f);
	msg_left();
	msg_write("\n\n");
}

};
