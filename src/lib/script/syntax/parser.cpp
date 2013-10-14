#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{


static int FoundConstantNr;
static Script *FoundConstantScript;


void ref_command_old(SyntaxTree *ps, Command *c);
void deref_command_old(SyntaxTree *ps, Command *c);
void command_make_ref(SyntaxTree *ps, Command *c, Command *param);
void CommandSetConst(SyntaxTree *ps, Command *c, int nc);
void exlink_make_var_local(SyntaxTree *ps, Type *t, int var_no);
void conv_cbr(SyntaxTree *ps, Command *&c, int var);

extern bool next_extern;
extern bool next_const;


#define is_variable(kind)	(((kind) == KindVarLocal) || ((kind) == KindVarGlobal))

inline bool type_match(Type *type, bool is_class, Type *wanted);
inline bool direct_type_match(Type *a, Type *b)
{
	return ( (a==b) || ( (a->is_pointer) && (b->is_pointer) ) || (a->IsDerivedFrom(b)) );
}
inline bool type_match_with_cast(Type *type, bool is_class, bool is_modifiable, Type *wanted, int &penalty, int &cast);

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


int s2i2(const string &str)
{
	if ((str.num > 1) && (str[0]=='0')&&(str[1]=='x')){
		int r=0;
		for (int i=2;i<str.num;i++){
			r *= 16;
			if ((str[i]>='0')&&(str[i]<='9'))
				r+=str[i]-48;
			if ((str[i]>='a')&&(str[i]<='f'))
				r+=str[i]-'a'+10;
			if ((str[i]>='A')&&(str[i]<='F'))
				r+=str[i]-'A'+10;
		}
		return r;
	}else
		return	str._int();
}

// find the type of a (potential) constant
//  "1.2" -> float
Type *SyntaxTree::GetConstantType()
{
	msg_db_f("GetConstantType", 4);
	FoundConstantNr = -1;
	FoundConstantScript = NULL;

	// named constants
	foreachi(Constant &c, Constants, i)
		if (Exp.cur == c.name){
			FoundConstantNr = i;
			FoundConstantScript = script;
			return c.type;
		}


	// included named constants
	foreach(Script *inc, Includes)
		foreachi(Constant &c, inc->syntax->Constants, i)
			if (Exp.cur == c.name){
				FoundConstantNr = i;
				FoundConstantScript = inc;
				return c.type;
			}

	// character "..."
	if ((Exp.cur[0] == '\'') && (Exp.cur.back() == '\''))
		return TypeChar;

	// string "..."
	if ((Exp.cur[0] == '"') && (Exp.cur.back() == '"'))
		return FlagStringConstAsCString ? TypeCString : TypeString;

	// numerical (int/float)
	Type *type = TypeInt;
	bool hex = (Exp.cur.num > 1) && (Exp.cur[0] == '0') && (Exp.cur[1] == 'x');
	for (int c=0;c<Exp.cur.num;c++)
		if ((Exp.cur[c] < '0') || (Exp.cur[c] > '9')){
			if (hex){
				if ((c >= 2) && (Exp.cur[c] < 'a') && (Exp.cur[c] > 'f'))
					return TypeUnknown;
			}else if (Exp.cur[c] == '.'){
				type = TypeFloat;
			}else{
				if ((c != 0) || (Exp.cur[c] != '-')) // allow sign
					return TypeUnknown;
			}
		}

	// super array [...]
	if (Exp.cur == "["){
		DoError("super array constant");
	}
	return type;
}

static int _some_int_;
static float _some_float_;
static char _some_string_[2048];

void *SyntaxTree::GetConstantValue()
{
	Type *type = GetConstantType();
// named constants
	if (FoundConstantNr >= 0)
		return FoundConstantScript->syntax->Constants[FoundConstantNr].data;
// literal
	if (type == TypeChar){
		_some_int_ = Exp.cur[1];
		return &_some_int_;
	}
	if ((type == TypeString) || (type == TypeCString)){
		for (int i=0;i<Exp.cur.num - 2;i++)
			_some_string_[i] = Exp.cur[i+1];
		_some_string_[Exp.cur.num - 2] = 0;
		return _some_string_;
	}
	if (type == TypeInt){
		_some_int_ = s2i2(Exp.cur);
		return &_some_int_;
	}
	if (type == TypeFloat){
		_some_float_ = Exp.cur._float();
		return &_some_float_;
	}
	return NULL;
}


Command *DoClassFunction(SyntaxTree *ps, Command *ob, ClassFunction &cf, Function *f)
{
	msg_db_f("DoClassFunc", 1);

	// the function
	Function *ff = cf.script->syntax->Functions[cf.nr];
	if (cf.virtual_index >= 0){
		Command *cmd = ps->AddCommand(KindVirtualFunction, cf.virtual_index, ff->literal_return_type);
		cmd->num_params = ff->num_params;
		cmd->script = cf.script;
		cmd->instance = ob;
		ps->GetFunctionCall("?." + cf.name, cmd, f);
		return cmd;
	}

	Command *cmd = ps->AddCommand(KindFunction, cf.nr, ff->literal_return_type);
	cmd->num_params = ff->num_params;
	cmd->script = cf.script;
	ps->GetFunctionCall(ff->name, cmd, f);
	cmd->instance = ob;
	return cmd;
}

Command *SyntaxTree::GetOperandExtensionElement(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtensionElement", 4);
	Exp.next();
	Type *type = Operand->type;

	// pointer -> dereference
	bool deref = false;
	if (type->is_pointer){
		type = type->parent;
		deref = true;
	}

	// find element
	for (int e=0;e<type->element.num;e++)
		if (Exp.cur == type->element[e].name){
			Exp.next();
			return 	shift_command(Operand, deref, type->element[e].offset, type->element[e].type);
		}

	// class function?
	foreach(ClassFunction &cf, type->function)
		if (Exp.cur == cf.name){
			if (!deref)
				ref_command_old(this, Operand);
			Exp.next();
			return DoClassFunction(this, Operand, cf, f);
		}

	DoError("unknown element of " + type->name);
	return NULL;
}

Command *SyntaxTree::GetOperandExtensionArray(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtensionArray", 4);

	// allowed?
	bool allowed = ((Operand->type->is_array) || (Operand->type->is_super_array));
	bool pparray = false;
	if (!allowed)
		if (Operand->type->is_pointer){
			if ((!Operand->type->parent->is_array) && (!Operand->type->parent->is_super_array))
				DoError(format("using pointer type \"%s\" as an array (like in C) is not allowed any more", Operand->type->name.c_str()));
			allowed = true;
			pparray = (Operand->type->parent->is_super_array);
		}
	if (!allowed)
		DoError(format("type \"%s\" is neither an array nor a pointer to an array", Operand->type->name.c_str()));
	Exp.next();

	Command *array;

	// pointer?
	so(Operand->type->name);
	if (pparray){
		DoError("test... anscheinend gibt es [] auf * super array");
		//array = cp_command(this, Operand);
/*		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent;
		deref_command_old(this, Operand);
		array = Operand->param[0];*/
	}else if (Operand->type->is_super_array){
		array = AddCommand(KindPointerAsArray, 0, Operand->type->parent);
		array->param[0] = shift_command(Operand, false, 0, array->type->GetPointer());
	}else if (Operand->type->is_pointer){
		array = AddCommand(KindPointerAsArray, 0, Operand->type->parent->parent);
		array->param[0] = Operand;
	}else{
		array = AddCommand(KindArray, 0, Operand->type->parent);
		array->param[0] = Operand;
	}
	array->num_params = 2;

	// array index...
	Command *index = GetCommand(f);
	array->param[1] = index;
	if (index->type != TypeInt){
		Exp.rewind();
		DoError(format("type of index for an array needs to be (int), not (%s)", index->type->name.c_str()));
	}
	if (Exp.cur != "]")
		DoError("\"]\" expected after array index");
	Exp.next();
	return array;
}

// find any ".", "->", or "[...]"'s    or operators?
Command *SyntaxTree::GetOperandExtension(Command *Operand, Function *f)
{
	msg_db_f("GetOperandExtension", 4);

	// nothing?
	int op = WhichPrimitiveOperator(Exp.cur);
	if ((Exp.cur != ".") && (Exp.cur != "[") && (Exp.cur != "->") && (op < 0))
		return Operand;

	if (Exp.cur == "->")
		DoError("\"->\" deprecated,  use \".\" instead");

	if (Exp.cur == "."){
		// class element?

		Operand = GetOperandExtensionElement(Operand, f);

	}else if (Exp.cur == "["){
		// array?

		Operand = GetOperandExtensionArray(Operand, f);


	}else if (op >= 0){
		// unary operator? (++,--)

		for (int i=0;i<PreOperators.num;i++)
			if (PreOperators[i].primitive_id == op)
				if ((PreOperators[i].param_type_1 == Operand->type) && (PreOperators[i].param_type_2 == TypeVoid)){
					so("  => unaerer Operator");
					Exp.next();
					return add_command_operator(Operand, NULL, i);
				}
		return Operand;
	}

	// recursion
	return GetOperandExtension(Operand, f);
}

bool SyntaxTree::GetSpecialFunctionCall(const string &f_name, Command *Operand, Function *f)
{
	msg_db_f("GetSpecialFuncCall", 4);

	// sizeof
	if ((Operand->kind == KindCompilerFunction) && (Operand->link_no == CommandSizeof)){

		so("sizeof");
		Exp.next();
		int nc = AddConstant(TypeInt);
		CommandSetConst(this, Operand, nc);


		Type *type = FindType(Exp.cur);
		if (type){
			(*(int*)(Constants[nc].data)) = type->size;
		}else if ((GetExistence(Exp.cur, f)) && ((GetExistenceLink.kind == KindVarGlobal) || (GetExistenceLink.kind == KindVarLocal))){
			(*(int*)(Constants[nc].data)) = GetExistenceLink.type->size;
		}else{
			type = GetConstantType();
			if (type)
				(*(int*)(Constants[nc].data)) = type->size;
			else
				DoError("type-name or variable name expected in sizeof(...)");
		}
		Exp.next();
		if (Exp.cur != ")")
			DoError("\")\" expected after parameter list");
		Exp.next();

		so(*(int*)(Constants[nc].data));
		return true;
	}

	DoError("evil special function");

	return false;
}


// cmd needs to have Param[]'s existing with correct Type!
void SyntaxTree::FindFunctionSingleParameter(int p, Type **WantedType, Function *f, Command *cmd)
{
	msg_db_f("FindFuncSingleParam", 4);
	Command *Param = GetCommand(f);

	WantedType[p] = TypeUnknown;
	if (cmd->kind == KindFunction){
		Function *ff = cmd->script->syntax->Functions[cmd->link_no];
		if (p < ff->num_params)
			WantedType[p] = ff->literal_param_type[p];
	}else if (cmd->kind == KindVirtualFunction){
		ClassFunction *cf = cmd->instance->type->parent->GetVirtualFunction(cmd->link_no);
		if (!cf)
			DoError("FindFunctionSingleParameter: cant find virtual function...?!?");
		if (p < cf->param_type.num)
			WantedType[p] = cf->param_type[p];
	}else if (cmd->kind == KindCompilerFunction){
		if (p < PreCommands[cmd->link_no].param.num)
			WantedType[p] = PreCommands[cmd->link_no].param[p].type;
	}else
		DoError("evil function...");
	// link parameters
	cmd->param[p] = Param;
}

void SyntaxTree::FindFunctionParameters(int &np, Type **WantedType, Function *f, Command *cmd)
{
	if (Exp.cur != "(")
		DoError("\"(\" expected in front of function parameter list");
	msg_db_f("FindFunctionParameters", 4);
	Exp.next();

	// list of parameters
	np = 0;
	for (int p=0;p<SCRIPT_MAX_PARAMS;p++){
		if (Exp.cur == ")")
			break;
		np ++;
		// find parameter

		FindFunctionSingleParameter(p, WantedType, f, cmd);

		if (Exp.cur != ","){
			if (Exp.cur == ")")
				break;
			DoError("\",\" or \")\" expected after parameter for function");
		}
		Exp.next();
	}
	Exp.next(); // ')'
}

void apply_type_cast(SyntaxTree *ps, int tc, Command *param);


// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
void SyntaxTree::CheckParamLink(Command *link, Type *type, const string &f_name, int param_no)
{
	msg_db_f("CheckParamLink", 4);
	// type cast needed and possible?
	Type *pt = link->type;
	Type *wt = type;

	// "silent" pointer (&)?
	if ((wt->is_pointer) && (wt->is_silent)){
		if (direct_type_match(pt, wt->parent)){
			so("<silent Ref &>");

			ref_command_old(this, link);
		}else if ((pt->is_pointer) && (direct_type_match(pt->parent, wt->parent))){
			so("<silent Ref & of *>");

			// no need to do anything...
		}else{
			Exp.rewind();
			DoError(format("(c) parameter %d in command \"%s\" has type (%s), (%s) expected", param_no + 1, f_name.c_str(), pt->name.c_str(), wt->name.c_str()));
		}

	// normal type cast
	}else if (!direct_type_match(pt, wt)){
		int tc = -1;
		for (int i=0;i<TypeCasts.num;i++)
			if ((direct_type_match(TypeCasts[i].source, pt)) && (direct_type_match(TypeCasts[i].dest, wt)))
				tc = i;

		if (tc >= 0){
			so("TypeCast");
			apply_type_cast(this, tc, link);
		}else{
			Exp.rewind();
			DoError(format("parameter %d in command \"%s\" has type (%s), (%s) expected", param_no + 1, f_name.c_str(), pt->name.c_str(), wt->name.c_str()));
		}
	}
}

// creates <Operand> to be the function call
//  on entry <Operand> only contains information from GetExistence (Kind, Nr, Type, NumParams)
void SyntaxTree::GetFunctionCall(const string &f_name, Command *Operand, Function *f)
{
	msg_db_f("GetFunctionCall", 4);

	// function as a variable?
	if (Exp.cur_exp >= 2)
	if ((Exp.get_name(Exp.cur_exp - 2) == "&") && (Exp.cur != "(")){
		if (Operand->kind == KindFunction){
			so("Funktion als Variable!");
			Operand->kind = KindVarFunction;
			Operand->type = TypePointer;
			Operand->num_params = 0;
			return;
		}else{
			Exp.rewind();
			//DoError("\"(\" expected in front of parameter list");
			DoError("only functions can be referenced");
		}
	}


	// "special" functions
    if (Operand->kind == KindCompilerFunction)
	    if (Operand->link_no == CommandSizeof){
			GetSpecialFunctionCall(f_name, Operand, f);
			return;
	    }

	so(Operand->type->name);
	// link operand onto this command
//	so(cmd->NumParams);



	// find (and provisional link) the parameters in the source
	int np;
	Type *WantedType[SCRIPT_MAX_PARAMS];

	bool needs_brackets = ((Operand->type != TypeVoid) || (Operand->num_params != 1));
	if (needs_brackets){
		FindFunctionParameters(np, WantedType, f, Operand);

	}else{
		np = 1;
		FindFunctionSingleParameter(0, WantedType, f, Operand);
	}

	// test compatibility
	if (np != Operand->num_params){
		Exp.rewind();
		DoError(format("function \"%s\" expects %d parameters, %d were found",f_name.c_str(), Operand->num_params, np));
	}
	for (int p=0;p<np;p++){
		CheckParamLink(Operand->param[p], WantedType[p], f_name, p);
	}
}

Command *SyntaxTree::GetOperand(Function *f)
{
	msg_db_f("GetOperand", 4);
	Command *Operand = NULL;
	so(Exp.cur);

	// ( -> one level down and combine commands
	if (Exp.cur == "("){
		Exp.next();
		Operand = GetCommand(f);
		if (Exp.cur != ")")
			DoError("\")\" expected");
		Exp.next();
	}else if (Exp.cur == "&"){ // & -> address operator
		so("<Adress-Operator &>");
		Exp.next();
		Operand = GetOperand(f);
		ref_command_old(this, Operand);
	}else if (Exp.cur == "*"){ // * -> dereference
		so("<Dereferenzierung *>");
		Exp.next();
		Operand = GetOperand(f);
		if (!Operand->type->is_pointer){
			Exp.rewind();
			DoError("only pointers can be dereferenced using \"*\"");
		}
		deref_command_old(this, Operand);
	}else if (Exp.cur == "new"){ // new operator
		Exp.next();
		Type *t = GetType(Exp.cur, true);
		Operand = add_command_compilerfunc(CommandNew);
		Operand->type = t->GetPointer();
		if (Exp.cur == "("){
			ClassFunction *cf = t->GetComplexConstructor();
			if (!cf)
				DoError(format("class \"%s\" does not have a constructor with parameters", t->name.c_str()));
			Operand->param[0] = DoClassFunction(this, NULL, *cf, f);
			Operand->num_params = 1;
		}
	}else if (Exp.cur == "delete"){ // delete operator
		Exp.next();
		Operand = add_command_compilerfunc(CommandDelete);
		Operand->param[0] = GetOperand(f);
		if (!Operand->param[0]->type->is_pointer)
			DoError("pointer expected after delete");
	}else{
		// direct operand
		if (GetExistence(Exp.cur, f)){
			Operand = cp_command(&GetExistenceLink);
			string f_name =  Exp.cur;
			so("=> " + Kind2Str(Operand->kind));
			Exp.next();
			// variables get linked directly...

			// operand is executable
			if ((Operand->kind == KindFunction) || (Operand->kind == KindVirtualFunction) || (Operand->kind == KindCompilerFunction)){
				GetFunctionCall(f_name, Operand, f);

			}else if (Operand->kind == KindPrimitiveOperator){
				// unary operator
				int _ie=Exp.cur_exp-1;
				so("  => unaerer Operator");
				int po = Operand->link_no, o=-1;
				Command *sub_command = GetOperand(f);
				Type *r = TypeVoid;
				Type *p2 = sub_command->type;

				// exact match?
				bool ok=false;
				for (int i=0;i<PreOperators.num;i++)
					if (po == PreOperators[i].primitive_id)
						if ((PreOperators[i].param_type_1 == TypeVoid) && (type_match(p2, false, PreOperators[i].param_type_2))){
							o = i;
							r = PreOperators[i].return_type;
							ok = true;
							break;
						}


				// needs type casting?
				if (!ok){
					int pen2;
					int c2, c2_best;
					int pen_min = 100;
					for (int i=0;i<PreOperators.num;i++)
						if (po == PreOperators[i].primitive_id)
							if ((PreOperators[i].param_type_1 == TypeVoid) && (type_match_with_cast(p2, false, false, PreOperators[i].param_type_2, pen2, c2))){
								ok = true;
								if (pen2 < pen_min){
									r = PreOperators[i].return_type;
									o = i;
									pen_min = pen2;
									c2_best = c2;
								}
						}
					// cast
					if (ok){
						apply_type_cast(this, c2_best, sub_command);
					}
				}


				if (!ok){
					Exp.cur_exp = _ie;
					DoError("unknown unitary operator  " + p2->name);
				}
				return add_command_operator(sub_command, NULL, o);
			}
		}else{
			Type *t = GetConstantType();
			if (t != TypeUnknown){
				so("=> Konstante");
				Operand = AddCommand(KindConstant, AddConstant(t), t);
				// constant for parameter (via variable)
				int size = t->size;
				if (t == TypeString)
					size = 256;
				memcpy(Constants[Operand->link_no].data, GetConstantValue(), size);
				Exp.next();
			}else{
				//Operand.Kind=0;
				DoError("unknown operand");
			}
		}

	}

	// Arrays, Strukturen aufloessen...
	Operand = GetOperandExtension(Operand,f);

	so("Operand endet mit " + Exp.get_name(Exp.cur_exp - 1));
	return Operand;
}

// only "primitive" operator -> no type information
Command *SyntaxTree::GetPrimitiveOperator(Function *f)
{
	msg_db_f("GetOperator",4);
	so(Exp.cur);
	int op = WhichPrimitiveOperator(Exp.cur);
	if (op < 0)
		return NULL;

	// command from operator
	Command *cmd = AddCommand(KindPrimitiveOperator, op, TypeUnknown);
	// only provisional (only operator sign, parameters and their types by GetCommand!!!)

	Exp.next();
	return cmd;
}

/*inline int find_operator(int primitive_id, Type *param_type1, Type *param_type2)
{
	for (int i=0;i<PreOperator.num;i++)
		if (PreOperator[i].PrimitiveID == primitive_id)
			if ((PreOperator[i].ParamType1 == param_type1) && (PreOperator[i].ParamType2 == param_type2))
				return i;
	//_do_error_("");
	return 0;
}*/

// both operand types have to match the operator's types
//   (operator wants a pointer -> all pointers are allowed!!!)
//   (same for classes of same type...)
inline bool type_match(Type *type, bool is_class, Type *wanted)
{
	if (type == wanted)
		return true;
	if ((type->is_pointer) && (wanted == TypePointer))
		return true;
	if ((is_class) && (wanted == TypeClass))
		return true;
	if (type->IsDerivedFrom(wanted))
		return true;
	return false;
}

inline bool type_match_with_cast(Type *type, bool is_class, bool is_modifiable, Type *wanted, int &penalty, int &cast)
{
	penalty = 0;
	cast = -1;
	if (type_match(type, is_class, wanted))
	    return true;
	if (is_modifiable) // is a variable getting assigned.... better not cast
		return false;
	for (int i=0;i<TypeCasts.num;i++)
		if ((direct_type_match(TypeCasts[i].source, type)) && (direct_type_match(TypeCasts[i].dest, wanted))){ // type_match()?
			penalty = TypeCasts[i].penalty;
			cast = i;
			return true;
		}
	return false;
}

void apply_type_cast(SyntaxTree *ps, int tc, Command *param)
{
	if (tc < 0)
		return;
	so(format("Benoetige automatischen TypeCast: %s -> %s", TypeCasts[tc].source->name.c_str(), TypeCasts[tc].dest->name.c_str()));
	if (param->kind == KindConstant){
		char *data_old = ps->Constants[param->link_no].data;
		char *data_new = (char*)TypeCasts[tc].func(data_old);
		if ((TypeCasts[tc].dest->is_array) || (TypeCasts[tc].dest->is_super_array)){
			// arrays as return value -> reference!
			int size = TypeCasts[tc].dest->size;
			if (TypeCasts[tc].dest == TypeString)
				size = 256;
			delete[] data_old;
			ps->Constants[param->link_no].data = new char[size];
			data_new = *(char**)data_new;
			memcpy(ps->Constants[param->link_no].data, data_new, size);
		}else
			memcpy(ps->Constants[param->link_no].data, data_new, TypeCasts[tc].dest->size);
		ps->Constants[param->link_no].type = TypeCasts[tc].dest;
		param->type = TypeCasts[tc].dest;
		so("  ...Konstante wurde direkt gewandelt!");
	}else{
		Command *sub_cmd = ps->cp_command(param);
		if (TypeCasts[tc].kind == KindFunction){
			param->kind = KindFunction;
			param->link_no = TypeCasts[tc].func_no;
			param->script = TypeCasts[tc].script;
			param->num_params = 1;
			param->param[0] = sub_cmd;
			param->instance = NULL;
			param->type = TypeCasts[tc].dest;
		}else if (TypeCasts[tc].kind == KindCompilerFunction){
			ps->CommandSetCompilerFunction(TypeCasts[tc].func_no, param);
			param->param[0] = sub_cmd;
		}
		so("  ...keine Konstante: Wandel-Befehl wurde hinzugefuegt!");
	}
}

Command *SyntaxTree::LinkOperator(int op_no, Command *param1, Command *param2)
{
	msg_db_f("LinkOp",4);
	bool left_modifiable = PrimitiveOperators[op_no].left_modifiable;
	string op_func_name = PrimitiveOperators[op_no].function_name;
	Command *op = NULL;

	Type *p1 = param1->type;
	Type *p2 = param2->type;
	bool equal_classes = false;
	if (p1 == p2)
		if (!p1->is_super_array)
			if (p1->element.num > 0)
				equal_classes = true;


	// exact match as class function?
	foreach(ClassFunction &f, p1->function)
		if (f.name == op_func_name){
			if (type_match(p2, equal_classes, f.param_type[0])){
				Command *inst = param1;
				ref_command_old(this, inst);
				op = add_command_classfunc(p1, &f, inst);
				op->num_params = 1;
				op->param[0] = param2;
				return op;
			}
		}

	// exact match?
	for (int i=0;i<PreOperators.num;i++)
		if (op_no == PreOperators[i].primitive_id)
			if (type_match(p1, equal_classes, PreOperators[i].param_type_1) && type_match(p2, equal_classes, PreOperators[i].param_type_2)){
				return add_command_operator(param1, param2, i);
			}

	// exact match as class function but missing a "&"?
	foreach(ClassFunction &f, p1->function)
		if (f.name == op_func_name){
			if (f.param_type[0]->is_pointer && f.param_type[0]->is_silent)
				if (direct_type_match(p2, f.param_type[0]->parent)){
					Command *inst = param1;
					ref_command_old(this, inst);
					op = add_command_classfunc(p1, &f, inst);
					op->num_params = 1;
					op->param[0] = param2;
					ref_command_old(this, op->param[0]);
					return op;
				}
		}


	// needs type casting?
	int pen1, pen2;
	int c1, c2, c1_best, c2_best;
	int pen_min = 2000;
	int op_found = -1;
	bool op_is_class_func = false;
	for (int i=0;i<PreOperators.num;i++)
		if (op_no == PreOperators[i].primitive_id)
			if (type_match_with_cast(p1, equal_classes, left_modifiable, PreOperators[i].param_type_1, pen1, c1) && type_match_with_cast(p2, equal_classes, false, PreOperators[i].param_type_2, pen2, c2))
				if (pen1 + pen2 < pen_min){
					op_found = i;
					pen_min = pen1 + pen2;
					c1_best = c1;
					c2_best = c2;
				}
	foreachi(ClassFunction &f, p1->function, i)
		if (f.name == op_func_name)
			if (type_match_with_cast(p2, equal_classes, false, f.param_type[0], pen2, c2))
				if (pen2 < pen_min){
					op_found = i;
					pen_min = pen2;
					c1_best = -1;
					c2_best = c2;
					op_is_class_func = true;
				}
	// cast
	if (op_found >= 0){
		apply_type_cast(this, c1_best, param1);
		apply_type_cast(this, c2_best, param2);
		if (op_is_class_func){
			Command *inst = param1;
			ref_command_old(this, inst);
			op = add_command_classfunc(p1, &p1->function[op_found], inst);
			op->num_params = 1;
			op->param[0] = param2;
		}else{
			return add_command_operator(param1, param2, op_found);
		}
		return op;
	}

	return NULL;
}

void SyntaxTree::LinkMostImportantOperator(Array<Command*> &Operand, Array<Command*> &Operator, Array<int> &op_exp)
{
	msg_db_f("LinkMostImpOp",4);
// find the most important operator (mio)
	int mio = 0;
	for (int i=0;i<Operator.num;i++){
		so(format("%d %d", Operator[i]->link_no, Operator[i]->link_no));
		if (PrimitiveOperators[Operator[i]->link_no].level > PrimitiveOperators[Operator[mio]->link_no].level)
			mio = i;
	}
	so(mio);

// link it
	Command *param1 = Operand[mio];
	Command *param2 = Operand[mio + 1];
	int op_no = Operator[mio]->link_no;
	Operator[mio] = LinkOperator(op_no, param1, param2);
	if (!Operator[mio]){
		Exp.cur_exp = op_exp[mio];
		DoError(format("no operator found: (%s) %s (%s)", param1->type->name.c_str(), PrimitiveOperators[op_no].name.c_str(), param2->type->name.c_str()));
	}

// remove from list
	Operand[mio] = Operator[mio];
	Operator.erase(mio);
	op_exp.erase(mio);
	Operand.erase(mio + 1);
}

Command *SyntaxTree::GetCommand(Function *f)
{
	msg_db_f("GetCommand", 4);
	Array<Command*> Operand;
	Array<Command*> Operator;
	Array<int> op_exp;

	// find the first operand
	Operand.add(GetOperand(f));

	// find pairs of operators and operands
	for (int i=0;true;i++){
		op_exp.add(Exp.cur_exp);
		Command *op = GetPrimitiveOperator(f);
		if (!op)
			break;
		Operator.add(op);
		if (Exp.end_of_line()){
			//Exp.rewind();
			DoError("unexpected end of line after operator");
		}
		Operand.add(GetOperand(f));
	}


	// in each step remove/link the most important operator
	while(Operator.num > 0)
		LinkMostImportantOperator(Operand, Operator, op_exp);

	// complete command is now collected in Operand[0]
	return Operand[0];
}

void SyntaxTree::ParseSpecialCommandFor(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandFor", 4);
	// variable
	Exp.next();
	Command *for_var;
	// internally declared?
	bool internally = false;
	if ((Exp.cur == "int") || (Exp.cur == "float")){
		Type *t = (Exp.cur == "int") ? TypeInt : TypeFloat;
		internally = true;
		Exp.next();
		int var_no = f->AddVar(Exp.cur, t);
		exlink_make_var_local(this, t, var_no);
			for_var = cp_command(&GetExistenceLink);
	}else{
		GetExistence(Exp.cur, f);
			for_var = cp_command(&GetExistenceLink);
		if ((!is_variable(for_var->kind)) || ((for_var->type != TypeInt) && (for_var->type != TypeFloat)))
			DoError("int or float variable expected after \"for\"");
	}
	Exp.next();

	// first value
	if (Exp.cur != ",")
		DoError("\",\" expected after variable in for");
	Exp.next();
	Command *val0 = GetCommand(f);
	CheckParamLink(val0, for_var->type, "for", 1);

	// last value
	if (Exp.cur != ",")
		DoError("\",\" expected after first value in for");
	Exp.next();
	Command *val1 = GetCommand(f);
	CheckParamLink(val1, for_var->type, "for", 2);

	Command *val_step = NULL;
	if (Exp.cur == ","){
		Exp.next();
		val_step = GetCommand(f);
		CheckParamLink(val_step, for_var->type, "for", 2);
	}

	// implement
	// for_var = val0
	Command *cmd_assign = add_command_operator(for_var, val0, OperatorIntAssign);
	block->command.add(cmd_assign);

	// while(for_var < val1)
	Command *cmd_cmp = add_command_operator(for_var, val1, OperatorIntSmaller);

	Command *cmd_while = add_command_compilerfunc(CommandFor);
	cmd_while->param[0] = cmd_cmp;
	block->command.add(cmd_while);
	ExpectNewline();
	// ...block
	Exp.next_line();
	ExpectIndent();
	int loop_block_no = Blocks.num; // should get created...soon
	ParseCompleteCommand(block, f);

	// ...for_var += 1
	Command *cmd_inc;
	if (for_var->type == TypeInt){
		if (val_step)
			cmd_inc = add_command_operator(for_var, val_step, OperatorIntAddS);
		else
			cmd_inc = add_command_operator(for_var, val1 /*dummy*/, OperatorIntIncrease);
	}else{
		if (!val_step){
			int nc = AddConstant(TypeFloat);
			*(float*)Constants[nc].data = 1.0;
			val_step = add_command_const(nc);
		}
		cmd_inc = add_command_operator(for_var, val_step, OperatorFloatAddS);
	}
	Block *loop_block = Blocks[loop_block_no];
	loop_block->command.add(cmd_inc); // add to loop-block

	// <for_var> declared internally?
	// -> force it out of scope...
	if (internally)
		f->var[for_var->link_no].name = "-out-of-scope-";
}

void SyntaxTree::ParseSpecialCommandForall(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandForall", 4);
	// for index
	int var_no_index = f->AddVar("-for_index-", TypeInt);
	exlink_make_var_local(this, TypeInt, var_no_index);
		Command *for_index = cp_command(&GetExistenceLink);

	// variable
	Exp.next();
	string var_name = Exp.cur;
	Exp.next();

	// super array
	if (Exp.cur != "in")
		DoError("\"in\" expected after variable in forall");
	Exp.next();
	Command *for_array = GetOperand(f);
	if (!for_array->type->is_super_array)
		DoError("list expected as second parameter in \"forall\"");
	//Exp.next();

	// variable...
	Type *var_type = for_array->type->parent;
	int var_no = f->AddVar(var_name, var_type);
	exlink_make_var_local(this, var_type, var_no);
		Command *for_var = cp_command(&GetExistenceLink);

	// 0
	int nc = AddConstant(TypeInt);
	*(int*)Constants[nc].data = 0;
	Command *val0 = add_command_const(nc);

	// implement
	// for_index = 0
	Command *cmd_assign = add_command_operator(for_index, val0, OperatorIntAssign);
	block->command.add(cmd_assign);

	// array.num
	Command *val1 = AddCommand(KindAddressShift, config.PointerSize, TypeInt);
	val1->num_params = 1;
	val1->param[0] = for_array;

	// while(for_index < val1)
	Command *cmd_cmp = add_command_operator(for_index, val1, OperatorIntSmaller);

	Command *cmd_while = add_command_compilerfunc(CommandFor);
	cmd_while->param[0] = cmd_cmp;
	block->command.add(cmd_while);
	ExpectNewline();
	// ...block
	Exp.next_line();
	ExpectIndent();
	int loop_block_no = Blocks.num; // should get created...soon
	ParseCompleteCommand(block, f);

	// ...for_index += 1
	Command *cmd_inc = add_command_operator(for_index, val1 /*dummy*/, OperatorIntIncrease);
	Block *loop_block = Blocks[loop_block_no];
	loop_block->command.add(cmd_inc); // add to loop-block

	// &for_var
	Command *for_var_ref = AddCommand(KindUnknown, 0, TypeVoid); // TODO
	command_make_ref(this, for_var_ref, for_var);

	// &array.data[for_index]
	Command *array_el = AddCommand(KindPointerAsArray, 0, var_type);
	array_el->num_params = 2;
	array_el->param[0] = shift_command(for_array, false, 0, var_type->GetPointer());
	array_el->param[1] = for_index;
	Command *array_el_ref = AddCommand(KindUnknown, 0, TypeVoid); // TODO
	command_make_ref(this, array_el_ref, array_el);

	// &for_var = &array[for_index]
	Command *cmd_var_assign = add_command_operator(for_var_ref, array_el_ref, OperatorPointerAssign);
	loop_block->command.insert(cmd_var_assign, 0);

	// ref...
	f->var[var_no].type = var_type->GetPointer();
	foreach(Command *c, loop_block->command)
		conv_cbr(this, c, var_no);

	// force for_var out of scope...
	f->var[for_var->link_no].name = "-out-of-scope-";
	f->var[for_index->link_no].name = "-out-of-scope-";
}

void SyntaxTree::ParseSpecialCommandWhile(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandWhile", 4);
	Exp.next();
	Command *cmd_cmp = GetCommand(f);
	CheckParamLink(cmd_cmp, TypeBool, "while", 0);
	ExpectNewline();

	Command *cmd_while = add_command_compilerfunc(CommandWhile);
	cmd_while->param[0] = cmd_cmp;
	block->command.add(cmd_while);
	// ...block
	Exp.next_line();
	ExpectIndent();
	ParseCompleteCommand(block, f);
}

void SyntaxTree::ParseSpecialCommandBreak(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandBreak", 4);
	Exp.next();
	Command *cmd = add_command_compilerfunc(CommandBreak);
	block->command.add(cmd);
}

void SyntaxTree::ParseSpecialCommandContinue(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandContinue", 4);
	Exp.next();
	Command *cmd = add_command_compilerfunc(CommandContinue);
	block->command.add(cmd);
}

void SyntaxTree::ParseSpecialCommandReturn(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandReturn", 4);
	Exp.next();
	Command *cmd = add_command_compilerfunc(CommandReturn);
	block->command.add(cmd);
	if (f->return_type == TypeVoid){
		cmd->num_params = 0;
	}else{
		Command *cmd_value = GetCommand(f);
		CheckParamLink(cmd_value, f->return_type, "return", 0);
		cmd->num_params = 1;
		cmd->param[0] = cmd_value;
	}
	ExpectNewline();
}

void SyntaxTree::ParseSpecialCommandIf(Block *block, Function *f)
{
	msg_db_f("ParseSpecialCommandIf", 4);
	int ind = Exp.cur_line->indent;
	Exp.next();
	Command *cmd_cmp = GetCommand(f);
	CheckParamLink(cmd_cmp, TypeBool, "if", 0);
	ExpectNewline();

	Command *cmd_if = add_command_compilerfunc(CommandIf);
	cmd_if->param[0] = cmd_cmp;
	block->command.add(cmd_if);
	// ...block
	Exp.next_line();
	ExpectIndent();
	ParseCompleteCommand(block, f);
	Exp.next_line();

	// else?
	if ((!Exp.end_of_file()) && (Exp.cur == "else") && (Exp.cur_line->indent >= ind)){
		cmd_if->link_no = CommandIfElse;
		Exp.next();
		// iterative if
		if (Exp.cur == "if"){
			// sub-if's in a new block
			Block *new_block = AddBlock();
			// parse the next if
			ParseCompleteCommand(new_block, f);
			// command for the found block
			Command *cmd_block = AddCommand(KindBlock, new_block->index, TypeVoid);
			// ...
			block->command.add(cmd_block);
			return;
		}
		ExpectNewline();
		// ...block
		Exp.next_line();
		ExpectIndent();
		ParseCompleteCommand(block, f);
		//Exp.next_line();
	}else{
		Exp.cur_line --;
		Exp.cur_exp = Exp.cur_line->exp.num - 1;
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
	}
}

void SyntaxTree::ParseSpecialCommand(Block *block, Function *f)
{
	// special commands...
	if (Exp.cur == "for"){
		ParseSpecialCommandFor(block, f);
	}else if (Exp.cur == "forall"){
		ParseSpecialCommandForall(block, f);
	}else if (Exp.cur == "while"){
		ParseSpecialCommandWhile(block, f);
 	}else if (Exp.cur == "break"){
		ParseSpecialCommandBreak(block, f);
	}else if (Exp.cur == "continue"){
		ParseSpecialCommandContinue(block, f);
	}else if (Exp.cur == "return"){
		ParseSpecialCommandReturn(block, f);
	}else if (Exp.cur == "if"){
		ParseSpecialCommandIf(block, f);
	}
}

/*void ParseBlock(sBlock *block, sFunction *f)
{
}*/

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void SyntaxTree::ParseCompleteCommand(Block *block, Function *f)
{
	msg_db_f("GetCompleteCommand", 4);
	// cur_exp = 0!

	Type *tType = GetType(Exp.cur, false);
	int last_indent = Exp.indent_0;

	// block?  <- indent
	if (Exp.indented){
		Exp.indented = false;
		Exp.cur_exp = 0; // bad hack...
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
		msg_db_f("Block", 4);
		Block *new_block = AddBlock();
		new_block->root = block->index;

		Command *c = AddCommand(KindBlock, new_block->index, TypeVoid);
		block->command.add(c);

		for (int i=0;true;i++){
			if (((i > 0) && (Exp.cur_line->indent < last_indent)) || (Exp.end_of_file()))
				break;

			ParseCompleteCommand(new_block, f);
			Exp.next_line();
		}
		Exp.cur_line --;
		Exp.indent_0 = Exp.cur_line->indent;
		Exp.indented = false;
		Exp.cur_exp = Exp.cur_line->exp.num - 1;
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;

	// assembler block
	}else if (Exp.cur == "-asm-"){
		Exp.next();
		so("<Asm-Block>");
		Command *c = add_command_compilerfunc(CommandAsm);
		block->command.add(c);

	// local (variable) definitions...
	// type of variable
	}else if (tType){
		for (int l=0;!Exp.end_of_line();l++){
			ParseVariableDefSingle(tType, f);

			// assignment?
			if (Exp.cur == "="){
				//Exp.rewind();
				// insert variable name because declaration might end with "[]"
				Exp.insert(f->var.back().name.c_str(), 0, Exp.cur_exp);
				Exp.cur = f->var.back().name;
				// parse assignment
				Command *c = GetCommand(f);
				block->command.add(c);
			}
			if (Exp.end_of_line())
				break;
			if ((Exp.cur != ",") && (!Exp.end_of_line()))
				DoError("\",\", \"=\" or newline expected after declaration of local variable");
			Exp.next();
		}
		return;
	}else{


	// commands (the actual code!)
		if ((Exp.cur == "for") || (Exp.cur == "forall") || (Exp.cur == "while") || (Exp.cur == "break") || (Exp.cur == "continue") || (Exp.cur == "return") || (Exp.cur == "if")){
			ParseSpecialCommand(block, f);

		}else{

			// normal commands
			Command *c = GetCommand(f);

			// link
			block->command.add(c);
		}
	}

	ExpectNewline();
}

// look for array definitions and correct pointers
void SyntaxTree::TestArrayDefinition(Type **type, bool is_pointer)
{
	msg_db_f("TestArrayDef", 4);
	if (is_pointer){
		(*type) = (*type)->GetPointer();
	}
	if (Exp.cur == "["){
		int array_size;
		string or_name = (*type)->name;
		int or_name_length = or_name.num;
		so("-Array-");
		Exp.next();

		// no index -> super array
		if (Exp.cur == "]"){
			array_size = -1;

		}else{

			// find array index
			Command *c = GetCommand(&RootOfAllEvil);
			PreProcessCommand(NULL, c);

			if ((c->kind != KindConstant) || (c->type != TypeInt))
				DoError("only constants of type \"int\" allowed for size of arrays");
			array_size = *(int*)Constants[c->link_no].data;
			//Exp.next();
			if (Exp.cur != "]")
				DoError("\"]\" expected after array size");
		}
		Exp.next();
		// recursion
		TestArrayDefinition(type, false); // is_pointer=false, since pointers have been handled

		// create array       (complicated name necessary to get correct ordering   int a[2][4] = (int[4])[2])
		if (array_size < 0){
			(*type) = CreateNewType(or_name + "[]" +  (*type)->name.substr(or_name_length, -1),
			                        config.SuperArraySize, false, false, true, array_size, (*type));
		}else{
			(*type) = CreateNewType(or_name + format("[%d]", array_size) + (*type)->name.substr(or_name_length, -1),
			                        (*type)->size * array_size, false, false, true, array_size, (*type));
		}
		if (Exp.cur == "*"){
			so("nachtraeglich Pointer");
			Exp.next();
			TestArrayDefinition(type, true);
		}
	}
}


void SyntaxTree::ParseImport()
{
	msg_db_f("ParseImport", 4);
	Exp.next(); // 'use' / 'import'

	string name = Exp.cur;
	if (name.find(".kaba") >= 0){

		string filename = script->Filename.dirname() + name.substr(1, name.num - 2); // remove "
		filename = filename.no_recursion();

		msg_right();
		Script *include;
		try{
			include = Load(filename, script->JustAnalyse);
		}catch(Exception &e){
			string msg = "in imported file:\n\"" + e.message + "\"";
			DoError(msg);
		}

		msg_left();
		AddIncludeData(include);
	}else{
		foreach(Package &p, Packages)
			if (p.name == name){
				AddIncludeData(p.script);
				return;
			}
		DoError("unknown package: " + name);
	}
}


void SyntaxTree::ParseEnum()
{
	msg_db_f("ParseEnum", 4);
	Exp.next(); // 'enum'
	ExpectNewline();
	int value = 0;
	Exp.next_line();
	ExpectIndent();
	for (int i=0;!Exp.end_of_file();i++){
		for (int j=0;!Exp.end_of_line();j++){
			int nc = AddConstant(TypeInt);
			Constant *c = &Constants[nc];
			c->name = Exp.cur;
			Exp.next();

			// explicit value
			if (Exp.cur == "="){
				Exp.next();
				ExpectNoNewline();
				Type *type = GetConstantType();
				if (type == TypeInt)
					value = *(int*)GetConstantValue();
				else
					DoError("integer constant expected after \"=\" for explicit value of enum");
				Exp.next();
			}
			*(int*)c->data = value ++;

			if (Exp.end_of_line())
				break;
			if ((Exp.cur != ","))
				DoError("\",\" or newline expected after enum definition");
			Exp.next();
			ExpectNoNewline();
		}
		Exp.next_line();
		if (Exp.unindented)
			break;
	}
	Exp.cur_line --;
}

void SyntaxTree::ParseClassFunctionHeader(Type *t, bool as_extern, int virtual_index, bool overwrite)
{
	Function *f = ParseFunctionHeader(t, as_extern);
	int n = -1;
	foreachi(Function *g, Functions, i)
		if (f == g)
			n = i;

	t->AddFunction(this, n, virtual_index, overwrite);
}

inline bool type_needs_alignment(Type *t)
{
	if (t->is_array)
		return type_needs_alignment(t->parent);
	return (t->size >= 4);
}

int class_count_virtual_functions(SyntaxTree *ps)
{
	ExpressionBuffer::Line *l = ps->Exp.cur_line;
	int count = 0;
	l ++;
	while(l != &ps->Exp.line[ps->Exp.line.num - 1]){
		if (l->indent == 0)
			break;
		if ((l->indent == 1) && (l->exp[0].name == "virtual"))
			count ++;
		else if ((l->indent == 1) && (l->exp[0].name == "extern") && (l->exp[1].name == "virtual"))
			count ++;
		l ++;
	}
	return count;
}

void SyntaxTree::ParseClass()
{
	msg_db_f("ParseClass", 4);

	int indent0 = Exp.cur_line->indent;
	int _offset = 0;
	Exp.next(); // 'class'
	string name = Exp.cur;
	Exp.next();

	// create class and type
	Type *_class = CreateNewType(name, 0, false, false, false, 0, NULL);

	// parent class
	if (Exp.cur == ":"){
		so("vererbung der struktur");
		Exp.next();
		Type *parent = GetType(Exp.cur, true);
		if (!_class->DeriveFrom(parent, true))
			DoError(format("parental type in class definition after \":\" has to be a class, but (%s) is not", parent->name.c_str()));
		_offset = parent->size;
	}
	ExpectNewline();

	// virtual functions?     (derived -> _class->num_virtual)
	int cur_virtual_index = _class->num_virtual;

	// elements
	for (int num=0;!Exp.end_of_file();num++){
		Exp.next_line();
		if (Exp.cur_line->indent <= indent0) //(unindented)
			break;
		if (Exp.end_of_file())
			break;

		// extern?
		next_extern = false;
		if (Exp.cur == "extern"){
			next_extern = true;
			Exp.next();
		}

		// virtual?
		bool next_virtual = false;
		bool overwrite = false;
		if (Exp.cur == "virtual"){
			next_virtual = true;
			Exp.next();
		}else if (Exp.cur == "overwrite"){
			overwrite = true;
			Exp.next();
		}
		int ie = Exp.cur_exp;

		Type *tType = GetType(Exp.cur, true);
		for (int j=0;!Exp.end_of_line();j++){
			//int indent = Exp.cur_line->indent;

			ClassElement el;
			bool is_pointer = false;
			Type *type = tType;
			if (Exp.cur == "*"){
				Exp.next();
				is_pointer = true;
			}
			el.name = Exp.cur;
			Exp.next();
			TestArrayDefinition(&type, is_pointer);
			el.type = type;

			// is a function?
			bool is_function = false;
			if (Exp.cur == "(")
			    is_function = true;
			if (is_function){
				Exp.cur_exp = ie;
				Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
				ParseClassFunctionHeader(_class, next_extern, next_virtual ? (cur_virtual_index ++) : -1, overwrite);

				break;
			}

			// overwrite?
			ClassElement *orig = NULL;
			foreachi(ClassElement &e, _class->element, i)
				if (e.name == el.name) //&& e.type->is_pointer && el.type->is_pointer)
						orig = &e;
			if (overwrite and ! orig)
				DoError(format("can not overwrite element '%s', not previous definition", el.name.c_str()));
			if (!overwrite and orig)
				DoError(format("element '%s' is already defined, use 'overwrite' to overwrite", el.name.c_str()));
			if (overwrite){
				if (orig->type->is_pointer and el.type->is_pointer)
					orig->type = el.type;
				else
					DoError("can only overwrite pointer elements with other pointer type");
				continue;
			}


			// add element
			if (type_needs_alignment(type))
				_offset = mem_align(_offset, 4);
			_offset = ProcessClassOffset(_class->name, el.name, _offset);
			so(format("Class-Element: %s %s  Offset: %d", _class->name.c_str(), el.name.c_str(), _offset));
			if ((Exp.cur != ",") && (!Exp.end_of_line()))
				DoError("\",\" or newline expected after class element");
			el.offset = _offset;
			_offset += type->size;
			_class->element.add(el);
			if (Exp.end_of_line())
				break;
			Exp.next();
		}
	}



	// virtual functions?     (derived -> _class->num_virtual)
	_class->num_virtual = cur_virtual_index;
	//foreach(ClassFunction &cf, _class->function)
	//	_class->num_virtual = max(_class->num_virtual, cf.virtual_index);
	if (_class->num_virtual > 0){
		if (_class->parent){
			if (_class->parent->num_virtual == 0)
				DoError("no virtual functions allowed when inheriting from class without virtual functions");
			// element "-vtable-" being derived
		}else{
			foreach(ClassElement &e, _class->element)
				e.offset += config.PointerSize;

			ClassElement el;
			el.name = "-vtable-";
			el.type = TypePointer;
			el.offset = 0;
			_class->element.insert(el, 0);
			_offset += config.PointerSize;
		}
		_class->vtable = new VirtualTable[_class->num_virtual + 2];
	}

	foreach(ClassElement &e, _class->element)
		if (type_needs_alignment(e.type))
			_offset = mem_align(_offset, 4);
	_class->size = ProcessClassSize(_class->name, _offset);

	AddFunctionHeadersForClass(_class);

	Exp.cur_line --;
}

void SyntaxTree::ExpectNoNewline()
{
	if (Exp.end_of_line())
		DoError("unexpected newline");
}

void SyntaxTree::ExpectNewline()
{
	if (!Exp.end_of_line())
		DoError("newline expected");
}

void SyntaxTree::ExpectIndent()
{
	if (!Exp.indented)
		DoError("additional indent expected");
}

void SyntaxTree::ParseGlobalConst(const string &name, Type *type)
{
	msg_db_f("ParseGlobalConst", 6);
	if (Exp.cur != "=")
		DoError("\"=\" expected after const name");
	Exp.next();

	// find const value
	Command *cv = GetCommand(&RootOfAllEvil);
	PreProcessCommand(NULL, cv);

	if ((cv->kind != KindConstant) || (cv->type != type))
		DoError(format("only constants of type \"%s\" allowed as value for this constant", type->name.c_str()));

	// give our const the name
	Constant *c = &Constants[cv->link_no];
	c->name = name;
}

Type *SyntaxTree::ParseVariableDefSingle(Type *type, Function *f, bool as_param)
{
	msg_db_f("ParseVariableDefSingle", 6);

	bool is_pointer = false;
	string name;

	// pointer?
	if (Exp.cur == "*"){
		Exp.next();
		is_pointer = true;
	}

	// name
	name = Exp.cur;
	Exp.next();
	so("Variable: " + name);

	// array?
	TestArrayDefinition(&type, is_pointer);

	// add
	if (next_const){
		ParseGlobalConst(name, type);
	}else
		f->AddVar(name, type);
	return type;
}

void SyntaxTree::ParseVariableDef(bool single, Function *f)
{
	msg_db_f("ParseVariableDef", 4);
	Type *type = GetType(Exp.cur, true);

	for (int j=0;true;j++){
		ExpectNoNewline();

		ParseVariableDefSingle(type, f);

		if ((Exp.cur != ",") && (!Exp.end_of_line()))
			DoError("\",\" or newline expected after definition of a global variable");

		// last one?
		if (Exp.end_of_line())
			break;

		Exp.next(); // ','
	}
}

bool peak_commands_super(ExpressionBuffer &Exp)
{
	ExpressionBuffer::Line *l = Exp.cur_line + 1;
	if (l->exp.num < 3)
		return false;
	if ((l->exp[0].name == "super") && (l->exp[1].name == ".") && (l->exp[2].name == "__init__"))
		return true;
	return false;
}

bool SyntaxTree::ParseFunctionCommand(Function *f, ExpressionBuffer::Line *this_line)
{
	Exp.next_line();
	Exp.indented = false;

	// end of file
	if (Exp.end_of_file())
		return false;

	// end of function
	if (Exp.cur_line->indent <= this_line->indent)
		return false;

	// command or local definition
	ParseCompleteCommand(f->block, f);
	return true;
}

void Function::Update(Type *class_type)
{
	// save "original" param types (Var[].Type gets altered for call by reference)
	for (int i=0;i<num_params;i++)
		literal_param_type[i] = var[i].type;

	// return by memory
	if (return_type->UsesReturnByMemory())
		AddVar("-return-", return_type->GetPointer());

	// class function
	_class = class_type;
	if (class_type){
		AddVar("self", class_type->GetPointer());
		if (class_type->parent)
			AddVar("super", class_type->parent->GetPointer());

		// convert name to Class.Function
		name = class_type->name + "." +  name;
	}
}

Function *SyntaxTree::ParseFunctionHeader(Type *class_type, bool as_extern)
{
	msg_db_f("ParseFunctionHeader", 4);

// return type
	Type *return_type = GetType(Exp.cur, true);

	// pointer?
	if (Exp.cur == "*"){
		Exp.next();
		return_type = return_type->GetPointer();
	}

	so(Exp.cur);
	Function *f = AddFunction(Exp.cur, return_type);
	cur_func = f;
	next_extern = false;

	Exp.next();
	Exp.next(); // '('

// parameter list

	if (Exp.cur != ")")
		for (int k=0;k<SCRIPT_MAX_PARAMS;k++){
			// like variable definitions

			f->num_params ++;

			// type of parameter variable
			Type *param_type = GetType(Exp.cur, true);
			Type *pt = ParseVariableDefSingle(param_type, f, true);
			f->var.back().type = pt;

			if (Exp.cur == ")")
				break;

			if (Exp.cur != ",")
				DoError("\",\" or \")\" expected after parameter");
			Exp.next(); // ','
		}
	Exp.next(); // ')'

	if (!Exp.end_of_line())
		DoError("newline expected after parameter list");

	f->Update(class_type);

	f->is_extern = as_extern;
	f->_logical_line_no = Exp.get_line_no();
	cur_func = NULL;

	// skip function body
	int indent0 = Exp.cur_line->indent;
	while (!Exp.end_of_file()){
		if (Exp.cur_line[1].indent <= indent0)
			break;
		Exp.next_line();
	}
	return f;
}

void SyntaxTree::ParseFunctionBody(Function *f)
{
	Exp.cur_line = &Exp.line[f->_logical_line_no];

	ExpressionBuffer::Line *this_line = Exp.cur_line;
	bool more_to_parse = true;

	// auto implement constructor?
	if (f->name.tail(9) == ".__init__"){
		if (peak_commands_super(Exp)){
			more_to_parse = ParseFunctionCommand(f, this_line);

			AutoImplementDefaultConstructor(f, f->_class, false);
		}else
			AutoImplementDefaultConstructor(f, f->_class, true);
	}


// instructions
	while(more_to_parse){
		more_to_parse = ParseFunctionCommand(f, this_line);
	}

	// auto implement destructor?
	if (f->name.tail(11) == ".__delete__")
		AutoImplementDestructor(f, f->_class);
	cur_func = NULL;

	Exp.cur_line --;
}

void SyntaxTree::ParseAllClassNames()
{
	msg_db_f("ParseAllClassNames", 4);

	Exp.reset_parser();
	while (!Exp.end_of_file()){
		if ((Exp.cur_line->indent == 0) && (Exp.cur_line->exp.num >= 2)){
			if (Exp.cur == "class"){
				Exp.next();
				int nt0 = Types.num;
				CreateNewType(Exp.cur, 0, false, false, false, 0, NULL);
				if (nt0 == Types.num)
					DoError("class already exists");
			}
		}
		Exp.next_line();
	}
}

void SyntaxTree::ParseAllFunctionBodies()
{
	for (int i=0;i<Functions.num;i++){
		Function *f = Functions[i];
		if ((!f->is_extern) && (f->_logical_line_no >= 0))
			ParseFunctionBody(f);
	}
}

// convert text into script data
void SyntaxTree::Parser()
{
	msg_db_f("Parser", 4);

	RootOfAllEvil.name = "RootOfAllEvil";
	cur_func = NULL;

	// syntax analysis

	ParseAllClassNames();

	Exp.reset_parser();

	// global definitions (enum, class, variables and functions)
	while (!Exp.end_of_file()){
		next_extern = false;
		next_const = false;

		// extern?
		if (Exp.cur == "extern"){
			next_extern = true;
			Exp.next();
		}

		// const?
		if (Exp.cur == "const"){
			next_const = true;
			Exp.next();
		}


		/*if ((Exp.cur == "import") || (Exp.cur == "use")){
			ParseImport();

		// enum
		}else*/ if (Exp.cur == "enum"){
			ParseEnum();

		// class
		}else if (Exp.cur == "class"){
			ParseClass();

		}else{

			// type of definition
			GetType(Exp.cur, true);
			Exp.rewind();
			bool is_function = false;
			for (int j=1;j<Exp.cur_line->exp.num-1;j++)
				if (Exp.cur_line->exp[j].name == "(")
				    is_function = true;

			// function?
			if (is_function){
				ParseFunctionHeader(NULL, next_extern);

			// global variables
			}else{
				ParseVariableDef(false, &RootOfAllEvil);
			}
		}
		if (!Exp.end_of_file())
			Exp.next_line();
	}

	ParseAllFunctionBodies();

	for (int i=0;i<Types.num;i++)
		AutoImplementFunctions(Types[i]);
}

}
