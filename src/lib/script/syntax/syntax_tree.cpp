#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{

//#define ScriptDebug


/*#define PRESCRIPT_DB_LEVEL	2
#define db_f(msg,level)		msg_db_f(msg,level+PRESCRIPT_DB_LEVEL)*/


bool next_extern = false;
bool next_const = false;

void conv_cbr(SyntaxTree *ps, Command *&c, int var);
 

Command *SyntaxTree::cp_command(Command *c)
{
	Command *cmd = AddCommand();
	*cmd = *c;
	return cmd;
}

Command *SyntaxTree::cp_command_deep(Command *c)
{
	Command *cmd = cp_command(c);
	for (int i=0;i<c->num_params;i++)
		cmd->param[i] = cp_command_deep(c->param[i]);
	return cmd;
}

void command_make_ref(SyntaxTree *ps, Command *c, Command *param)
{
	c->kind = KindReference;
	c->num_params = 1;
	c->param[0] = param;
	c->type = ps->GetPointerType(param->type);
}

void ref_command_old(SyntaxTree *ps, Command *c)
{
	Command *t = ps->cp_command(c);
	command_make_ref(ps, c, t);
}

Command *SyntaxTree::ref_command(Command *sub)
{
	Command *c = AddCommand();
	command_make_ref(this, c, sub);
	return c;
}

void command_make_deref(SyntaxTree *ps, Command *c, Command *param)
{
	c->kind = KindDereference;
	c->num_params = 1;
	c->param[0] = param;
	c->type = param->type->parent;
}

void deref_command_old(SyntaxTree *ps, Command *c)
{
	Command *t = ps->cp_command(c);
	command_make_deref(ps, c, t);
}

Command *SyntaxTree::deref_command(Command *sub)
{
	Command *c = AddCommand();
	command_make_deref(this, c, sub);
	return c;
}

Command *SyntaxTree::shift_command(Command *sub, bool deref, int shift, Type *type)
{
	Command *c= AddCommand();
	c->kind = deref ? KindDerefAddressShift : KindAddressShift;
	c->link_nr = shift;
	c->type = type;
	c->num_params = 1;
	c->param[0] = sub;
	return c;
}

Command *SyntaxTree::add_command_compilerfunc(int cf)
{
	Command *c = AddCommand();
	CommandSetCompilerFunction(cf, c);
	return c;
}

// link as NON-VIRTUAL function!
Command *SyntaxTree::add_command_classfunc(Type *class_type, ClassFunction &f, Command *inst)
{
	Command *c = AddCommand();
	c->kind = KindFunction;
	c->link_nr = f.nr;
	c->instance = inst;
	c->script = f.script;
	c->type = f.return_type;
	return c;
}


Command *SyntaxTree::add_command_operator(Command *p1, Command *p2, int op)
{
	Command *cmd = AddCommand();
	cmd->kind = KindOperator;
	cmd->link_nr = op;
	cmd->num_params = ((PreOperators[op].param_type_1 == TypeVoid) || (PreOperators[op].param_type_2 == TypeVoid)) ? 1 : 2; // unary / binary
	cmd->param[0] = p1;
	cmd->param[1] = p2;
	cmd->type = PreOperators[op].return_type;
	return cmd;
}


Command *SyntaxTree::add_command_local_var(int no, Type *type)
{
	Command *cmd = AddCommand();
	cmd->kind = KindVarLocal;
	cmd->link_nr = no;
	cmd->type = type;
	return cmd;
}

Command *SyntaxTree::add_command_parray(Command *p, Command *index, Type *type)
{
	Command *cmd_el = AddCommand();
	cmd_el->kind = KindPointerAsArray;
	cmd_el->type = type;
	cmd_el->param[0] = p;
	cmd_el->param[1] = index;
	cmd_el->num_params = 2;
	return cmd_el;
}

SyntaxTree::SyntaxTree(Script *_script)
{
	FlagShow = false;
	FlagShowPrae = false;
	FlagDisassemble = false;
	FlagCompileOS = false;
	FlagCompileInitialRealMode = false;
	FlagOverwriteVariablesOffset = false;
	FlagImmortal = false;
	FlagNoExecution = false;
	AsmMetaInfo = NULL;
	RootOfAllEvil.name = "RootOfAllEvil";
	RootOfAllEvil.num_params = 0;
	RootOfAllEvil.return_type = TypeVoid;
	cur_func = NULL;
	script = _script;

	// "include" default stuff
	foreach(Package &p, Packages)
		if (p.used_by_default)
			AddIncludeData(p.script);
}


void SyntaxTree::LoadAndParseFile(const string &filename, bool just_analyse)
{
	msg_db_f("LoadAndParseFile",4);

	LoadToBuffer(config.Directory + filename, just_analyse);

	
	PreCompiler(just_analyse);

	Parser();
	
	if (FlagShowPrae)
		Show();

	ConvertCallByReference();

	/*if (FlagShow)
		Show();*/

	Simplify();
	
	PreProcessor(NULL);

	if (FlagShow)
		Show();

	Exp.clear();
}

static void so(const char *str)
{
#ifdef ScriptDebug
	/*if (strlen(str)>256)
		str[256]=0;*/
	msg_write(str);
#endif
}

static void so(const string &str)
{
#ifdef ScriptDebug
	/*if (strlen(str)>256)
		str[256]=0;*/
	msg_write(str);
#endif
}

static void so(int i)
{
#ifdef ScriptDebug
	msg_write(i);
#endif
}

static void right()
{
#ifdef ScriptDebug
	msg_right();
#endif
}

static void left()
{
#ifdef ScriptDebug
	msg_left();
#endif
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
	if (kind == KindVarTemp)			return "temp";
	if (kind == KindDerefVarTemp)		return "deref temp";
	if (kind == KindRegister)			return "register";
	if (kind == KindAddress)			return "address";
	if (kind == KindMemory)				return "memory";
	if (kind == KindLocalAddress)		return "local address";
	if (kind == KindLocalMemory)		return "local memory";
	if (kind == KindDerefRegister)		return "deref register";
	if (kind == KindMarker)				return "marker";
	if (kind == KindAsmBlock)			return "assembler block";
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
	//msg_error("zu coden: CreateAsmMetaInfo");
	if (!AsmMetaInfo){
		AsmMetaInfo = new Asm::MetaInfo;
		AsmMetaInfo->Mode16 = FlagCompileInitialRealMode;
		AsmMetaInfo->CodeOrigin = 0; // FIXME:  &Opcode[0] ????
	}
	AsmMetaInfo->Opcode = script->Opcode;
	AsmMetaInfo->global_var.clear();
	for (int i=0;i<RootOfAllEvil.var.num;i++){
		Asm::GlobalVar v;
		v.Name = RootOfAllEvil.var[i].name;
		v.Size = RootOfAllEvil.var[i].type->size;
		v.Pos = script->g_var[i];
		AsmMetaInfo->global_var.add(v);
	}
}


int SyntaxTree::AddVar(const string &name, Type *type, Function *f)
{
	Variable v;
	v.name = name;
	v.type = type;
	v.is_extern = next_extern;
	f->var.add(v);
	return f->var.num - 1;
}

// constants

int SyntaxTree::AddConstant(Type *type)
{
	Constant c;
	c.name = "-none-";
	c.type = type;
	int s = max(type->size, config.PointerSize);
	if (type == TypeString)
		s = 256;
	c.data = new char[s];
	Constants.add(c);
	return Constants.num - 1;
}

Block *SyntaxTree::AddBlock()
{
	Block *b = new Block;
	b->index = Blocks.num;
	b->root = -1;
	Blocks.add(b);
	return b;
}

// functions

Function *SyntaxTree::AddFunction(const string &name, Type *type)
{
	Function *f = new Function();
	Functions.add(f);
	f->name = name;
	f->block = AddBlock();
	f->num_params = 0;
	f->var.clear();
	f->return_type = type;
	f->literal_return_type = type;
	f->_class = NULL;
	f->is_extern = next_extern;
	return f;
}
Command *SyntaxTree::AddCommand()
{
	Command *c = new Command;
	Commands.add(c);
	c->type = TypeVoid;
	c->kind = KindUnknown;
	c->num_params = 0;
	c->instance = NULL;
	c->script = script;
	return c;
}


void CommandSetConst(SyntaxTree *ps, Command *c, int nc)
{
	c->kind = KindConstant;
	c->link_nr = nc;
	c->type = ps->Constants[nc].type;
	c->num_params = 0;
}

Command *SyntaxTree::add_command_const(int nc)
{
	Command *c = AddCommand();
	c->kind = KindConstant;
	c->link_nr = nc;
	c->type = Constants[nc].type;
	c->num_params = 0;
	return c;
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
	ps->GetExistenceLink.link_nr = var_no;
	ps->GetExistenceLink.kind = KindVarLocal;
	ps->GetExistenceLink.num_params = 0;
	ps->GetExistenceLink.script = ps->script;
	ps->GetExistenceLink.instance = NULL;
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
			GetExistenceLink.link_nr = i;
			GetExistenceLink.kind = KindVarGlobal;
			return true;
		}

	// then the (self-coded) functions
	foreachi(Function *f, Functions, i)
		if (f->name == name){
			GetExistenceLink.kind = KindFunction;
			GetExistenceLink.link_nr = i;
			GetExistenceLink.type = f->literal_return_type;
			GetExistenceLink.num_params = f->num_params;
			return true;
		}

	// types
	int w = WhichType(name);
	if (w >= 0){
		GetExistenceLink.kind = KindType;
		GetExistenceLink.link_nr = w;
		return true;
	}

	// ...unknown
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.kind = KindUnknown;
	GetExistenceLink.link_nr = 0;
	return false;
}

bool SyntaxTree::GetExistence(const string &name, Function *func)
{
	msg_db_f("GetExistence", 3);
	MultipleFunctionList.clear();
	GetExistenceLink.type = TypeUnknown;
	GetExistenceLink.num_params = 0;
	GetExistenceLink.script = script;
	GetExistenceLink.instance = NULL;

	// first test local variables
	if (func){
		foreachi(Variable &v, func->var, i){
			if (v.name == name){
				exlink_make_var_local(this, v.type, i);
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
		GetExistenceLink.link_nr = w;
		GetExistenceLink.type = PreCommands[w].return_type;
		GetExistenceLink.num_params = PreCommands[w].param.num;
		return true;
	}

	// operators
	w = WhichPrimitiveOperator(name);
	if (w >= 0){
		GetExistenceLink.kind = KindPrimitiveOperator;
		GetExistenceLink.link_nr = w;
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
	GetExistenceLink.link_nr = 0;
	return false;
}

void SyntaxTree::CommandSetCompilerFunction(int CF, Command *Com)
{
	msg_db_f("CommandSetCompilerFunction", 4);
	//if (FlagCompileOS)
	//	DoError(format("external function call (%s) not allowed with #os", PreCommands[CF].name.c_str()));
	
// a function the compiler knows
	Com->kind = KindCompilerFunction;
	Com->link_nr = CF;
	Com->script = Packages[0].script;
	Com->instance = NULL;

	Com->num_params = PreCommands[CF].param.num;
	for (int p=0;p<Com->num_params;p++){
		Com->param[p] = AddCommand(); // temporary...
		Com->param[p]->type = PreCommands[CF].param[p].type;
	}
	Com->type = PreCommands[CF].return_type;
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

// expression naming a type
Type *SyntaxTree::GetType(const string &name, bool force)
{
	Type *type = FindType(name);
	if ((force) && (!type))
		DoError("unknown type");
	if (type)
		Exp.next();
	return type;
}

// create a new type?
void SyntaxTree::AddType(Type **type)
{
	for (int i=0;i<Types.num;i++)
		if ((*type)->name == Types[i]->name){
			(*type)=Types[i];
			return;
		}
	foreach(Script *inc, Includes)
		for (int i=0;i<inc->syntax->Types.num;i++)
			if ((*type)->name == inc->syntax->Types[i]->name){
				(*type) = inc->syntax->Types[i];
				return;
			}
	Type *t = new Type;
	(*t) = (**type);
	t->owner = this;
	t->name = (*type)->name;
	so("AddType: " + t->name);
	(*type) = t;
	Types.add(t);

	if (t->is_super_array)
		script_make_super_array(t, this);
}

Type *SyntaxTree::CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub)
{
	Type nt, *pt = &nt;
	nt.is_array = is_array && (array_size >= 0);
	nt.is_super_array = is_array && (array_size < 0);
	nt.array_length = max(array_size, 0);
	nt.is_pointer = is_pointer;
	nt.is_silent = is_silent;
	nt.name = name;
	nt.size = size;
	nt.parent = sub;
	AddType(&pt);
	return pt;
}

Type *SyntaxTree::GetPointerType(Type *sub)
{
	return CreateNewType(sub->name + "*", config.PointerSize, true, false, false, 0, sub);
}



// read the file and do a lexical analysis
void SyntaxTree::LoadToBuffer(const string &filename,bool just_analyse)
{
	msg_db_f("LoadToBuffer",4);

// read file
	CFile *f = OpenFile(filename);
	if (!f){
		Exp.cur_line = NULL;
		DoError("script file not loadable");
	}
	string Buffer = f->ReadComplete();
	Buffer.add(0); // compatibility... expected by lexical
	FileClose(f);

	Exp.Analyse(this, Buffer);
}

void conv_cbr(SyntaxTree *ps, Command *&c, int var)
{
	msg_db_f("conv_cbr", 4);
	//so(Kind2Str(c->Kind));
	
	// recursion...
	so(c->num_params);
	for (int i=0;i<c->num_params;i++)
		conv_cbr(ps, c->param[i], var);
	if (c->kind == KindBlock){
		foreach(Command *cc, ps->Blocks[c->link_nr]->command)
			conv_cbr(ps, cc, var);
	}
	if (c->instance)
		conv_cbr(ps, c->instance, var);
	so("a");

	// convert
	if ((c->kind == KindVarLocal) && (c->link_nr == var)){
		so("conv");
		c = ps->cp_command(c);
		c->type = ps->GetPointerType(c->type);
		deref_command_old(ps, c);
	}
}

#if 0
void conv_return(SyntaxTree *ps, command *c)
{
	// recursion...
	for (int i=0;i<c->num_params;i++)
		conv_return(ps, c->param[i]);
	
	if ((c->kind == KindCompilerFunction) && (c->link_nr == CommandReturn)){
		msg_write("conv ret");
		ref_command_old(ps, c);
	}
}
#endif


// remove &*x and (*x)[] and (*x).y
void easyfy(SyntaxTree *ps, Command *c, int l)
{
	msg_db_f("easyfy", 4);
	//msg_write(l);
	//msg_write("a");
	
	// recursion...
	for (int i=0;i<c->num_params;i++)
		easyfy(ps, c->param[i], l+1);
	if (c->kind == KindBlock)
		for (int i=0;i<ps->Blocks[c->link_nr]->command.num;i++)
			easyfy(ps, ps->Blocks[c->link_nr]->command[i], l+1);
	if (c->instance)
		easyfy(ps, c->instance, l+1);
	
	//msg_write("b");


	// convert
	if (c->kind == KindReference){
		if (c->param[0]->kind == KindDereference){
			so("rem 2");
			// remove 2 knots...
			Command *t = c->param[0]->param[0];
			*c = *t;
		}
	}else if ((c->kind == KindAddressShift) || (c->kind == KindArray)){
		if (c->param[0]->kind == KindDereference){
			so("rem 1 (unify)");
			// unify 2 knots (remove 1)
			Command *t = c->param[0]->param[0];
			c->kind = (c->kind == KindAddressShift) ? KindDerefAddressShift : KindPointerAsArray;
			c->param[0] = t;
		}
	}
	//msg_write("ok");
}

void convert_return_by_memory(SyntaxTree *ps, Block *b, Function *f)
{
	msg_db_f("convert_return_by_memory", 2);
	ps->script->cur_func = f;

	foreachib(Command *c, b->command, i){
		// recursion...
		if (c->kind == KindBlock)
			convert_return_by_memory(ps, ps->Blocks[c->link_nr], f);
		if ((c->kind != KindCompilerFunction) || (c->link_nr != CommandReturn))
			continue;
		so("convert return by mem");

		// convert into   *-return- = param
		Command *p_ret = NULL;
		foreachi(Variable &v, f->var, i)
			if (v.name == "-return-"){
				p_ret = ps->AddCommand();
				p_ret->type = v.type;
				p_ret->link_nr = i;
				p_ret->kind = KindVarLocal;
			}
		if (!p_ret)
			ps->DoError("-return- not found...");
		Command *ret = ps->deref_command(p_ret);
		Command *op = ps->LinkOperator(OperatorAssign, ret, c->param[0]);
		if (!op)
			ps->DoError("no = operator for return from function found: " + f->name);
		b->command.insert(op, i);

		c->num_params = 0;

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
				f->var[j].type = GetPointerType(f->var[j].type);

				// internal usage...
				foreach(Command *c, f->block->command)
					conv_cbr(this, c, j);
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
	foreach(Command *ccc, Commands){
		// Command array might be reallocated in the loop!
		Command *c = ccc;

		if (c->kind == KindCompilerFunction)
			if (c->link_nr == CommandReturn){
				if ((c->param[0]->type->is_array) /*|| (c->Param[j]->Type->IsSuperArray)*/){
					so("conv param (return)");
					so(c->param[0]->type->name);
					ref_command_old(this, c->param[0]);
				}
				_foreach_it_.update(); // TODO badness10000!!!!!!!!
				continue;
			}
		
		if ((c->kind == KindFunction)|| (c->kind == KindCompilerFunction)){
			// parameters: array/class as reference
			for (int j=0;j<c->num_params;j++)
				if (c->param[j]->type->UsesCallByReference()){
					so("conv param");
					so(c->param[j]->type->name);
					ref_command_old(this, c->param[j]);
				}

			// return: array reference (-> dereference)
			if ((c->type->is_array) /*|| (c->Type->IsSuperArray)*/){
				so("conv ret");
				so(c->type->name);
				c->type = GetPointerType(c->type);
				deref_command_old(this, c);
			}	
		}

		// special string / list operators
		if (c->kind == KindOperator){
			// parameters: super array as reference
			for (int j=0;j<c->num_params;j++)
				if ((c->param[j]->type->is_array) || (c->param[j]->type->is_super_array)){
					so("conv param (op)");
					so(c->param[j]->type->name);
					ref_command_old(this, c->param[j]);
				}
		}
		_foreach_it_.update(); // TODO !!!!!
	}
}


void SyntaxTree::Simplify()
{
	msg_db_f("Simplify", 2);
	
	// remove &*
	foreach(Function *f, Functions)
		foreach(Command *c, f->block->command)
			easyfy(this, c, 0);
}

// split arrays and address shifts into simpler commands...
void SyntaxTree::BreakDownComplicatedCommands()
{
	msg_db_f("BreakDownComplicatedCommands", 4);
	
	for (int i=0;i<Commands.num;i++){
		Command *c = Commands[i];
		if (c->kind == KindArray){
			so("array");

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
			*(int*)Constants[nc].data = el_type->size;
			Command *c_size = add_command_const(nc);
			// offset = size * index
			Command *c_offset = add_command_operator(c_index, c_size, OperatorIntMultiply);
			c_offset->type = TypeInt;//TypePointer;
			// address = &array + offset
			Command *c_address = add_command_operator(c_ref_array, c_offset, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
		}else if (c->kind == KindPointerAsArray){
			so("array");

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
			*(int*)Constants[nc].data = el_type->size;
			Command *c_size = add_command_const(nc);
			// offset = size * index
			Command *c_offset = add_command_operator(c_index, c_size, OperatorIntMultiply);
			c_offset->type = TypeInt;
			// address = &array + offset
			Command *c_address = add_command_operator(c_ref_array, c_offset, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
		}else if (c->kind == KindAddressShift){
			so("address shift");

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
			*(int*)Constants[nc].data = c->link_nr;
			Command *c_shift = add_command_const(nc);
			// address = &struct + shift
			Command *c_address = add_command_operator(c_ref_struct, c_shift, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
		}else if (c->kind == KindDerefAddressShift){
			so("deref address shift");

			Type *el_type = c->type;

// struct el -> struct_pointer
//           -> shift (LinkNr)
//
// * -> + -> struct_pointer
//        -> shift

			Command *c_ref_struct = c->param[0];
			// create command for shift constant
			int nc = AddConstant(TypeInt);
			*(int*)Constants[nc].data = c->link_nr;
			Command *c_shift = add_command_const(nc);
			// address = &struct + shift
			Command *c_address = add_command_operator(c_ref_struct, c_shift, OperatorIntAdd);
			c_address->type = GetPointerType(el_type);//TypePointer;
			// c = * address
			command_make_deref(this, c, c_address);
			c->type = el_type;
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

			// map "-return-" to the VERY first parameter
			if (f->return_type->UsesReturnByMemory()){
				foreachi(Variable &v, f->var, i)
					if (v.name == "-return-"){
						v._offset = f->_param_size;
						f->_param_size += 4;
					}
			}

			// map "self" to the first parameter
			if (f->_class){
				foreachi(Variable &v, f->var, i)
					if (v.name == "self"){
						v._offset = f->_param_size;
						f->_param_size += 4;
					}
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
	
	foreach(Constant &c, Constants)
		delete[](c.data);

	foreach(Command *c, Commands)
		delete(c);

	foreach(Block *b, Blocks)
		delete(b);
	
	foreach(Function *f, Functions)
		delete(f);
}

void SyntaxTree::ShowCommand(Command *c)
{
	msg_write("[" + Kind2Str(c->kind) + "] " + c->type->name + " " + LinkNr2Str(c->script->syntax,c->kind,c->link_nr));
	msg_right();
	if (c->instance)
		ShowCommand(c->instance);
	for (int p=0;p<c->num_params;p++)
		ShowCommand(c->param[p]);
	msg_left();
}

void SyntaxTree::ShowBlock(Block *b)
{
	msg_write("block");
	msg_right();
	foreach(Command *c, b->command){
		if (c->kind == KindBlock)
			ShowBlock(Blocks[c->link_nr]);
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
		ShowFunction(f);
	msg_left();
	msg_write("\n\n");
}

};
