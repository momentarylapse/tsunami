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
	cmd->set_num_params(c->num_params);
	for (int i=0;i<c->num_params;i++)
		if (c->param[i])
			cmd->set_param(i, cp_command(c->param[i]));
	if (c->instance)
		cmd->set_instance(cp_command(c->instance));
	return cmd;
}

Command *SyntaxTree::ref_command(Command *sub, Type *overwrite_type)
{
	Type *t = overwrite_type ? overwrite_type : sub->type->GetPointer();
	Command *c = AddCommand(KindReference, 0, t);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Command *SyntaxTree::deref_command(Command *sub, Type *overwrite_type)
{
	Command *c = AddCommand(KindUnknown, 0, TypeVoid);
	c->kind = KindDereference;
	c->set_num_params(1);
	c->set_param(0, sub);
	if (overwrite_type)
		c->type = overwrite_type;
	else
		c->type = sub->type->parent;
	return c;
}

Command *SyntaxTree::shift_command(Command *sub, bool deref, int shift, Type *type)
{
	Command *c= AddCommand(deref ? KindDerefAddressShift : KindAddressShift, shift, type);
	c->set_num_params(1);
	c->set_param(0, sub);
	return c;
}

Command *SyntaxTree::add_command_compilerfunc(int cf)
{
	Command *c = AddCommand(KindUnknown, 0, TypeVoid);

	//if (FlagCompileOS)
	//	DoError(format("external function call (%s) not allowed with #os", PreCommands[CF].name.c_str()));

// a function the compiler knows
	c->kind = KindCompilerFunction;
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
	if ((f->virtual_index >= 0) && (!force_non_virtual))
		c = AddCommand(KindVirtualFunction, f->virtual_index, f->return_type);
	else
		c = AddCommand(KindFunction, f->nr, f->return_type);
	c->script = f->script;
	c->set_instance(inst);
	c->set_num_params(f->param_type.num);
	return c;
}

Command *SyntaxTree::add_command_func(Script *script, int no, Type *return_type)
{
	Command *c = AddCommand(KindFunction, no, return_type);
	c->script = script;
	c->set_num_params(script->syntax->Functions[no]->num_params);
	return c;
}


Command *SyntaxTree::add_command_operator(Command *p1, Command *p2, int op)
{
	Command *cmd = AddCommand(KindOperator, op, PreOperators[op].return_type);
	bool unitary = ((PreOperators[op].param_type_1 == TypeVoid) || (PreOperators[op].param_type_2 == TypeVoid));
	cmd->set_num_params( unitary ? 1 : 2); // unary / binary
	cmd->set_param(0, p1);
	if (!unitary)
		cmd->set_param(1, p2);
	return cmd;
}


Command *SyntaxTree::add_command_local_var(int no, Type *type)
{
	return AddCommand(KindVarLocal, no, type);
}

Command *SyntaxTree::add_command_parray(Command *p, Command *index, Type *type)
{
	Command *cmd_el = AddCommand(KindPointerAsArray, 0, type);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

Command *SyntaxTree::add_command_block(Block *b)
{
	return AddCommand(KindBlock, b->index, TypeVoid);
}

SyntaxTree::SyntaxTree(Script *_script) :
	GetExistenceLink(KindUnknown, 0, NULL, TypeVoid),
	RootOfAllEvil(this, "RootOfAllEvil", TypeVoid)
{
	FlagShow = false;
	FlagShowPrae = false;
	FlagDisassemble = false;
	FlagCompileOS = false;
	FlagStringConstAsCString = false;
	FlagNoFunctionFrame = false;
	FlagAddEntryPoint = false;
	FlagOverwriteVariablesOffset = false;
	FlagImmortal = false;
	FlagNoExecution = false;
	cur_func = NULL;
	script = _script;
	AsmMetaInfo = new Asm::MetaInfo;
	ForIndexCount = 0;
	Exp.cur_line = NULL;

	// "include" default stuff
	foreach(Package &p, Packages)
		if (p.used_by_default)
			AddIncludeData(p.script);
}


void SyntaxTree::ParseBuffer(const string &buffer, bool just_analyse)
{
	msg_db_f("LoadAndParseFile",4);

	Exp.Analyse(this, buffer + string("\0", 1)); // compatibility... expected by lexical
	
	PreCompiler(just_analyse);

	Parser();
	
	if (FlagShowPrae)
		Show();

	ConvertCallByReference();

	/*if (FlagShow)
		Show();*/

	Simplify();
	
	PreProcessor();

	if (FlagShow)
		Show();

	Exp.clear();
}

string Kind2Str(int kind)
{
	if (kind == KindVarLocal)			return "local variable";
	if (kind == KindVarGlobal)			return "global variable";
	if (kind == KindVarFunction)		return "function as variable";
	if (kind == KindConstant)			return "constant";
	if (kind == KindRefToConst)			return "reference to const";
	if (kind == KindFunction)			return "function";
	if (kind == KindVirtualFunction)	return "virtual function";
	if (kind == KindCompilerFunction)	return "compiler function";
	if (kind == KindOperator)			return "operator";
	if (kind == KindPrimitiveOperator)	return "PRIMITIVE operator";
	if (kind == KindBlock)				return "command block";
	if (kind == KindAddressShift)		return "address shift";
	if (kind == KindArray)				return "array element";
	if (kind == KindPointerAsArray)		return "pointer as array element";
	if (kind == KindReference)			return "address operator";
	if (kind == KindDereference)		return "dereferencing";
	if (kind == KindDerefAddressShift)	return "deref address shift";
	if (kind == KindType)				return "type";
	if (kind == KindArrayBuilder)		return "array builder";
	if (kind == KindVarTemp)			return "temp";
	if (kind == KindDerefVarTemp)		return "deref temp";
	if (kind == KindRegister)			return "register";
	if (kind == KindAddress)			return "address";
	if (kind == KindMemory)				return "memory";
	if (kind == KindLocalAddress)		return "local address";
	if (kind == KindLocalMemory)		return "local memory";
	if (kind == KindDerefRegister)		return "deref register";
	if (kind == KindMarker)				return "marker";
	if (kind == KindRefToLocal)			return "ref to local";
	if (kind == KindRefToGlobal)		return "ref to global";
	if (kind == KindRefToConst)			return "ref to const";
	if (kind == KindDerefVarLocal)		return "deref local";
	return format("UNKNOWN KIND: %d", kind);
}

string LinkNr2Str(SyntaxTree *s,int kind,int nr)
{
	if (kind == KindVarLocal)			return i2s(nr);//s->cur_func->var[nr].name;
	if (kind == KindVarGlobal)			return s->RootOfAllEvil.var[nr].name;
	if (kind == KindVarFunction)		return s->Functions[nr]->name;
	if (kind == KindConstant)			return i2s(nr);//s->Constants[nr].type->var2str(s->Constants[nr].data);
	if (kind == KindFunction)			return s->Functions[nr]->name;
	if (kind == KindVirtualFunction)	return i2s(nr);//s->Functions[nr]->name;
	if (kind == KindCompilerFunction)	return PreCommands[nr].name;
	if (kind == KindOperator)			return PreOperators[nr].str();
	if (kind == KindPrimitiveOperator)	return PrimitiveOperators[nr].name;
	if (kind == KindBlock)				return i2s(nr);
	if (kind == KindAddressShift)		return i2s(nr);
	if (kind == KindArray)				return "(no LinkNr)";
	if (kind == KindPointerAsArray)		return "(no LinkNr)";
	if (kind == KindReference)			return "(no LinkNr)";
	if (kind == KindDereference)		return "(no LinkNr)";
	if (kind == KindDerefAddressShift)	return i2s(nr);
	if (kind == KindType)				return s->Types[nr]->name;
	if (kind == KindRegister)			return Asm::GetRegName(nr);
	if (kind == KindAddress)			return d2h(&nr, config.PointerSize);
	if (kind == KindMemory)				return d2h(&nr, config.PointerSize);
	if (kind == KindLocalAddress)		return d2h(&nr, config.PointerSize);
	if (kind == KindLocalMemory)		return d2h(&nr, config.PointerSize);
	return i2s(nr);
}

void SyntaxTree::DoError(const string &str, int overwrite_line)
{
	// what data do we have?
	int line = -1;
	int pos = 0;
	string expr;
	if (Exp.cur_line){
		line = Exp.cur_line->physical_line;
		if (Exp.cur_exp >= 0){
			expr = Exp.cur;
			pos = Exp.cur_line->exp[Exp.cur_exp].pos;
		}
	}
	if (overwrite_line >= 0){
		line = overwrite_line;
		pos = 0;
	}

	throw Exception(str, expr, line, pos, script);
}

void SyntaxTree::CreateAsmMetaInfo()
{
	msg_db_f("CreateAsmMetaInfo",5);
	AsmMetaInfo->global_var.clear();
	for (int i=0;i<RootOfAllEvil.var.num;i++){
		Asm::GlobalVar v;
		v.Name = RootOfAllEvil.var[i].name;
		v.Size = RootOfAllEvil.var[i].type->size;
		v.Pos = script->g_var[i];
		AsmMetaInfo->global_var.add(v);
	}
}


int Function::AddVar(const string &name, Type *type)
{
	if (get_var(name) >= 0)
		tree->DoError(format("variable '%s' already declared in this context", name.c_str()));
	Variable v;
	v.name = name;
	v.type = type;
	v.is_extern = next_extern;
	var.add(v);
	return var.num - 1;
}

// constants

int SyntaxTree::AddConstant(Type *type)
{
	Constant c;
	c.name = "-none-";
	c.type = type;
	c.value.resize(max(type->size, config.PointerSize));
	Constants.add(c);
	return Constants.num - 1;
}

Block *SyntaxTree::AddBlock()
{
	Block *b = new Block;
	b->index = Blocks.num;
	Blocks.add(b);
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
	command.add(c);
	c->ref_count ++;
}

void Block::set(int index, Command *c)
{
	set_command(command[index], c);
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
	inline_no = -1;
}

int Function::get_var(const string &name)
{
	foreachi(Variable &v, var, i)
		if (v.name == name)
			return i;
	return -1;
}

Function *SyntaxTree::AddFunction(const string &name, Type *type)
{
	Function *f = new Function(this, name, type);
	Functions.add(f);
	f->block = AddBlock();
	return f;
}

Command::Command(int _kind, int _link_no, Script *_script, Type *_type)
{
	type = _type;
	kind = _kind;
	link_no = _link_no;
	num_params = 0;
	instance = NULL;
	script = _script;
	ref_count = 0;
}

Block *Command::block() const
{
	return script->syntax->Blocks[link_no];
}

void Command::set_instance(Command *p)
{
	set_command(instance, p);
}

void Command::set_num_params(int n)
{
	for (int i=num_params; i<n; i++)
		param[i] = NULL;
	num_params = n;
}

void Command::set_param(int index, Command *p)
{
	set_command(param[index], p);
}

Command *SyntaxTree::AddCommand(int kind, int link_no, Type *type)
{
	Command *c = new Command(kind, link_no, script, type);
	Commands.add(c);
	return c;
}

Command *SyntaxTree::AddCommand(int kind, int link_no, Type *type, Script *s)
{
	Command *c = new Command(kind, link_no, s, type);
	Commands.add(c);
	return c;
}


Command *SyntaxTree::add_command_const(int nc)
{
	return AddCommand(KindConstant, nc, Constants[nc].type);
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
	for (int i=0;i<Types.num;i++)
		if (name == Types[i]->name)
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

void exlink_make_var_local(SyntaxTree *ps, Type *t, int var_no)
{
	ps->GetExistenceLink.type = t;
	ps->GetExistenceLink.link_no = var_no;
	ps->GetExistenceLink.kind = KindVarLocal;
	ps->GetExistenceLink.set_num_params(0);
	ps->GetExistenceLink.script = ps->script;
	ps->GetExistenceLink.instance = NULL;
}

void exlink_make_var_element(SyntaxTree *ps, Function *f, ClassElement &e)
{
	Command *self = ps->add_command_local_var(f->get_var("self"), f->_class->GetPointer());
	ps->GetExistenceLink.type = e.type;
	ps->GetExistenceLink.link_no = e.offset;
	ps->GetExistenceLink.kind = KindDerefAddressShift;
	ps->GetExistenceLink.set_num_params(1);
	ps->GetExistenceLink.param[0] = self;
	ps->GetExistenceLink.script = ps->script;
	ps->GetExistenceLink.instance = NULL;
}

void exlink_make_func_class(SyntaxTree *ps, Function *f, ClassFunction &cf)
{
	Command *self = ps->add_command_local_var(f->get_var("self"), f->_class->GetPointer());
	if (cf.virtual_index >= 0){
		ps->GetExistenceLink.kind = KindVirtualFunction;
		ps->GetExistenceLink.link_no = cf.virtual_index;
	}else{
		ps->GetExistenceLink.kind = KindFunction;
		ps->GetExistenceLink.link_no = cf.nr;
	}
	ps->GetExistenceLink.script = cf.script;
	ps->GetExistenceLink.type = cf.return_type;
	ps->GetExistenceLink.set_num_params(cf.param_type.num);
	ps->GetExistenceLink.instance = self;
}

bool SyntaxTree::GetExistenceShared(const string &name)
{
	msg_db_f("GetExistenceShared", 3);
	MultipleFunctionList.clear();
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.num_params = 0;
	GetExistenceLink.script = script;
	GetExistenceLink.instance = NULL;

	// global variables (=local variables in "RootOfAllEvil")
	foreachi(Variable &v, RootOfAllEvil.var, i)
		if (v.name == name){
			GetExistenceLink.type = v.type;
			GetExistenceLink.link_no = i;
			GetExistenceLink.kind = KindVarGlobal;
			return true;
		}

	// then the (real) functions
	foreachi(Function *f, Functions, i)
		if (f->name == name){
			GetExistenceLink.kind = KindFunction;
			GetExistenceLink.link_no = i;
			GetExistenceLink.type = f->literal_return_type;
			GetExistenceLink.set_num_params(f->num_params);
			return true;
		}

	// types
	int w = WhichType(name);
	if (w >= 0){
		GetExistenceLink.kind = KindType;
		GetExistenceLink.link_no = w;
		return true;
	}

	// ...unknown
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.kind = KindUnknown;
	GetExistenceLink.link_no = 0;
	return false;
}

bool SyntaxTree::GetExistence(const string &name, Function *func)
{
	msg_db_f("GetExistence", 3);
	MultipleFunctionList.clear();
	GetExistenceLink.ref_count = 0;
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.num_params = 0;
	GetExistenceLink.script = script;
	GetExistenceLink.instance = NULL;

	if (func){
		// first test local variables
		foreachi(Variable &v, func->var, i){
			if (v.name == name){
				exlink_make_var_local(this, v.type, i);
				return true;
			}
		}
		if (func->_class){
			if ((name == "super") && (func->_class->parent)){
				exlink_make_var_local(this, func->_class->parent->GetPointer(), func->get_var("self"));
				return true;
			}
			// class elements (within a class function)
			foreach(ClassElement &e, func->_class->element)
				if (e.name == name){
					exlink_make_var_element(this, func, e);
					return true;
				}
			foreach(ClassFunction &cf, func->_class->function)
				if (cf.name == name){
					exlink_make_func_class(this, func, cf);
					return true;
				}
		}
	}

	// shared stuff (global variables, functions)
	if (GetExistenceShared(name))
		return true;

	// then the compiler functions
	int w = WhichCompilerFunction(name);
	if (w >= 0){
		GetExistenceLink.kind = KindCompilerFunction;
		GetExistenceLink.link_no = w;
		GetExistenceLink.type = PreCommands[w].return_type;
		GetExistenceLink.set_num_params(PreCommands[w].param.num);
		return true;
	}

	// operators
	w = WhichPrimitiveOperator(name);
	if (w >= 0){
		GetExistenceLink.kind = KindPrimitiveOperator;
		GetExistenceLink.link_no = w;
		return true;
	}

	// in include files (only global)...
	foreach(Script *i, Includes)
		if (i->syntax->GetExistenceShared(name)){
			memcpy(&GetExistenceLink, &(i->syntax->GetExistenceLink), sizeof(Command));
			GetExistenceLink.script = i;
			//msg_error(string2("\"%s\" in Include gefunden!  %s", name, GetExistenceLink.Type->Name));
			return true;
		}

	// ...unknown
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.kind = KindUnknown;
	GetExistenceLink.link_no = 0;
	return false;
}

// expression naming a type
Type *SyntaxTree::FindType(const string &name)
{
	for (int i=0;i<Types.num;i++)
		if (name == Types[i]->name)
			return Types[i];
	foreach(Script *inc, Includes)
		for (int i=0;i<inc->syntax->Types.num;i++)
			if (name == inc->syntax->Types[i]->name)
				return inc->syntax->Types[i];
	return NULL;
}

// create a new type?
Type *SyntaxTree::AddType(Type *type)
{
	foreach(Type *t, Types)
		if (type->name == t->name)
			return t;
	foreach(Script *inc, Includes)
		foreach(Type *t, inc->syntax->Types)
			if (type->name == t->name)
				return t;
	Type *t = new Type;
	*t = *type;
	t->owner = this;
	t->name = type->name;
	Types.add(t);


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
	nt.is_array = is_array && (array_size >= 0);
	nt.is_super_array = is_array && (array_size < 0);
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
			config.SuperArraySize, false, false, true, num_elements, element_type);
	}else{
		return CreateNewType(name_pre + format("[%d]", num_elements) + suffix,
			element_type->size * num_elements, false, false, true, num_elements, element_type);
	}
}



#define TRANSFORM_COMMANDS_RECURSION(FUNC, PREPARAMS, POSTPARAMS, CMD) \
	for (int i=0;i<(CMD)->num_params;i++) \
		(CMD)->set_param(i, FUNC(PREPARAMS, (CMD)->param[i], POSTPARAMS)); \
	if ((CMD)->kind == KindBlock){ \
		foreachi(Command *cc, (CMD)->block()->command, i) \
			(CMD)->block()->set(i, FUNC(PREPARAMS, cc, POSTPARAMS)); \
	} \
	if ((CMD)->instance) \
		(CMD)->set_instance(FUNC(PREPARAMS, (CMD)->instance, POSTPARAMS));

Command *conv_cbr(SyntaxTree *ps, Command *c, int var)
{
	msg_db_f("conv_cbr", 4);
	
	// recursion...
	TRANSFORM_COMMANDS_RECURSION(conv_cbr, ps, var, c);

	// convert
	if ((c->kind == KindVarLocal) && (c->link_no == var)){
		c->type = c->type->GetPointer();
		return ps->deref_command(c);
	}
	return c;
}

#if 0
void conv_return(SyntaxTree *ps, command *c)
{
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->param[i]);
	
	if ((c->kind == KindCompilerFunction) && (c->link_no == CommandReturn)){
		msg_write("conv ret");
		ref_command_old(ps, c);
	}
}
#endif


Command *conv_calls(SyntaxTree *ps, Command *c, int tt)
{
	// recursion...
	TRANSFORM_COMMANDS_RECURSION(conv_calls, ps, tt, c)

	if ((c->kind == KindCompilerFunction) && (c->link_no == CommandReturn))
		if (c->num_params > 0){
			if ((c->param[0]->type->is_array) /*|| (c->Param[j]->Type->IsSuperArray)*/){
				c->set_param(0, ps->ref_command(c->param[0]));
			}
			return c;
		}

	if ((c->kind == KindFunction) || (c->kind == KindVirtualFunction) || (c->kind == KindCompilerFunction) || (c->kind == KindArrayBuilder)){
		// parameters: array/class as reference
		for (int j=0;j<c->num_params;j++)
			if (c->param[j]->type->UsesCallByReference()){
				c->set_param(j, ps->ref_command(c->param[j]));
			}

		// return: array reference (-> dereference)
		if ((c->type->is_array) /*|| (c->Type->IsSuperArray)*/){
			c->type = c->type->GetPointer();
			return ps->deref_command(c);
			//deref_command_old(this, c);
		}
	}

	// special string / list operators
	if (c->kind == KindOperator){
		// parameters: super array as reference
		for (int j=0;j<c->num_params;j++)
			if ((c->param[j]->type->is_array) || (c->param[j]->type->is_super_array)){
				c->set_param(j, ps->ref_command(c->param[j]));
			}
  	}
	return c;
}


// remove &*x and (*x)[] and (*x).y
Command *easyfy(SyntaxTree *ps, Command *c, int l)
{
	msg_db_f("easyfy", 4);
	//msg_write(l);
	//msg_write("a");
	
	// recursion...
	for (int i=0;i<c->num_params;i++)
		c->set_param(i, easyfy(ps, c->param[i], l+1));
	if (c->kind == KindBlock)
		for (int i=0;i<c->block()->command.num;i++)
			c->block()->set(i, easyfy(ps, c->block()->command[i], l+1));
	if (c->instance)
		c->set_instance(easyfy(ps, c->instance, l+1));
	
	//msg_write("b");


	// convert
	if (c->kind == KindReference){
		if (c->param[0]->kind == KindDereference){
			// remove 2 knots...
			return c->param[0]->param[0];
		}
	}else if ((c->kind == KindAddressShift) || (c->kind == KindArray)){
		if (c->param[0]->kind == KindDereference){
			// unify 2 knots (remove 1)
			Command *t = c->param[0]->param[0];
			c->kind = (c->kind == KindAddressShift) ? KindDerefAddressShift : KindPointerAsArray;
			c->set_param(0, t);
			return c;
		}
	}
	//msg_write("ok");
	return c;
}

void convert_return_by_memory(SyntaxTree *ps, Block *b, Function *f)
{
	msg_db_f("convert_return_by_memory", 2);
	ps->script->cur_func = f;

	foreachib(Command *c, b->command, i){
		// recursion...
		if (c->kind == KindBlock)
			convert_return_by_memory(ps, c->block(), f);
		if ((c->kind != KindCompilerFunction) || (c->link_no != CommandReturn))
			continue;

		// convert into   *-return- = param
		Command *p_ret = NULL;
		foreachi(Variable &v, f->var, i)
			if (v.name == "-return-"){
				p_ret = ps->AddCommand(KindVarLocal, i, v.type);
			}
		if (!p_ret)
			ps->DoError("-return- not found...");
		Command *ret = ps->deref_command(p_ret);
		Command *op = ps->LinkOperator(OperatorAssign, ret, c->param[0]);
		if (!op)
			ps->DoError("no = operator for return from function found: " + f->name);
		b->command.insert(op, i);

		c->set_num_params(0);

		_foreach_it_.update();
	}
}

// convert "source code"...
//    call by ref params:  array, super array, class
//    return by ref:       array
void SyntaxTree::ConvertCallByReference()
{
	msg_db_f("ConvertCallByReference", 2);


	// convert functions
	foreach(Function *f, Functions){
		
		// parameter: array/class as reference
		for (int j=0;j<f->num_params;j++)
			if (f->var[j].type->UsesCallByReference()){
				f->var[j].type = f->var[j].type->GetPointer();

				// internal usage...
				foreachi(Command *c, f->block->command, i)
					f->block->command[i] = conv_cbr(this, c, j);
			}

		// return: array as reference
#if 0
		if ((f->return_type->is_array) /*|| (f->Type->IsSuperArray)*/){
			f->return_type = GetPointerType(f->return_type);
			/*for (int k=0;k<f->Block->Command.num;k++)
				conv_return(this, f->Block->Command[k]);*/
			// no need... return gets converted automatically (all calls...)
		}
#endif
	}

	msg_db_m("a", 3);


	// convert return...
	foreach(Function *f, Functions)
		if (f->return_type->UsesReturnByMemory())
			convert_return_by_memory(this, f->block, f);

	// convert function calls
	foreach(Function *f, Functions)
		foreachi(Command *c, f->block->command, i)
			f->block->command[i] = conv_calls(this, c, 0);
}


void SyntaxTree::Simplify()
{
	msg_db_f("Simplify", 2);
	
	// remove &*
	foreach(Function *f, Functions)
		foreachi(Command *c, f->block->command, i)
			f->block->command[i] = easyfy(this, c, 0);
}

Command *SyntaxTree::BreakDownComplicatedCommand(Command *c)
{
	// recursion...
	for (int i=0;i<c->num_params;i++)
		c->set_param(i, BreakDownComplicatedCommand(c->param[i]));
	if (c->kind == KindBlock){
		for (int i=0;i<c->block()->command.num;i++)
			c->block()->set(i, BreakDownComplicatedCommand(c->block()->command[i]));
	}
	if (c->instance)
		c->set_instance(BreakDownComplicatedCommand(c->instance));

	if (c->kind == KindArray){

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
		Constants[nc].setInt(el_type->size);
		Command *c_size = add_command_const(nc);
		// offset = size * index
		Command *c_offset = add_command_operator(c_index, c_size, OperatorIntMultiply);
		c_offset->type = TypeInt;//TypePointer;
		// address = &array + offset
		Command *c_address = add_command_operator(c_ref_array, c_offset, OperatorIntAdd);
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}else if (c->kind == KindPointerAsArray){

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
		Constants[nc].setInt(el_type->size);
		Command *c_size = add_command_const(nc);
		// offset = size * index
		Command *c_offset = add_command_operator(c_index, c_size, OperatorIntMultiply);
		c_offset->type = TypeInt;
		// address = &array + offset
		Command *c_address = add_command_operator(c_ref_array, c_offset, OperatorIntAdd);
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}else if (c->kind == KindAddressShift){

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
		Constants[nc].setInt(c->link_no);
		Command *c_shift = add_command_const(nc);
		// address = &struct + shift
		Command *c_address = add_command_operator(c_ref_struct, c_shift, OperatorIntAdd);
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}else if (c->kind == KindDerefAddressShift){

		Type *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

		Command *c_ref_struct = c->param[0];
		// create command for shift constant
		int nc = AddConstant(TypeInt);
		Constants[nc].setInt(c->link_no);
		Command *c_shift = add_command_const(nc);
		// address = &struct + shift
		Command *c_address = add_command_operator(c_ref_struct, c_shift, OperatorIntAdd);
		c_address->type = el_type->GetPointer();//TypePointer;
		// * address
		return deref_command(c_address);
	}
	return c;
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::BreakDownComplicatedCommands()
{
	msg_db_f("BreakDownComplicatedCommands", 4);

	foreach(Function *f, Functions){
		foreachi(Command *c, f->block->command, i)
			f->block->command[i] = BreakDownComplicatedCommand(c);
	}
}

void MapLVSX86Return(Function *f)
{
	if (f->return_type->UsesReturnByMemory()){
		foreachi(Variable &v, f->var, i)
			if (v.name == "-return-"){
				v._offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void MapLVSX86Self(Function *f)
{
	if (f->_class){
		foreachi(Variable &v, f->var, i)
			if (v.name == "self"){
				v._offset = f->_param_size;
				f->_param_size += 4;
			}
	}
}

void SyntaxTree::MapLocalVariablesToStack()
{
	msg_db_f("MapLocalVariablesToStack", 1);
	foreach(Function *f, Functions){
		f->_param_size = 2 * config.PointerSize; // space for return value and eBP
		if (config.instruction_set == Asm::InstructionSetX86){
			f->_var_size = 0;

			if (config.abi == AbiWindows32){
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
				if ((f->_class) && (v.name == "self"))
					continue;
				if (v.name == "-return-")
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
		}else if (config.instruction_set == Asm::InstructionSetAMD64){
			f->_var_size = 0;
			
			foreachi(Variable &v, f->var, i){
				int s = mem_align(v.type->size, 4);
				v._offset = - f->_var_size - s;
				f->_var_size += s;
			}
		}
	}
}


// no included scripts may be deleted before us!!!
SyntaxTree::~SyntaxTree()
{
	msg_db_f("~SyntaxTree", 4);
	
	Exp.clear();

	// delete all types created by this script
	foreach(Type *t, Types)
		delete(t);

	if (AsmMetaInfo)
		delete(AsmMetaInfo);

	foreach(Command *c, Commands)
		delete(c);

	foreach(Block *b, Blocks)
		delete(b);
	
	foreach(Function *f, Functions)
		delete(f);
}

void SyntaxTree::ShowCommand(Command *c)
{
	msg_write("[" + Kind2Str(c->kind) + "] " + c->type->name + " " + LinkNr2Str(c->script->syntax,c->kind,c->link_no) + " << " + c->script->Filename);
	msg_right();
	if (c->instance)
		ShowCommand(c->instance);
	for (int p=0;p<c->num_params;p++)
		if (c->param[p])
			ShowCommand(c->param[p]);
		else
			msg_write("<param nil>");
	msg_left();
}

void SyntaxTree::ShowBlock(Block *b)
{
	msg_write("block");
	msg_right();
	foreach(Command *c, b->command){
		if (c->kind == KindBlock)
			ShowBlock(c->block());
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
	msg_write("--------- Syntax of " + script->Filename + " ---------");
	msg_right();
	foreach(Function *f, Functions)
		if (!f->is_extern)
			ShowFunction(f);
	msg_left();
	msg_write("\n\n");
}

};
