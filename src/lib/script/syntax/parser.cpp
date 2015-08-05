#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{


static int FoundConstantNr;
static Script *FoundConstantScript;


void exlink_make_var_local(SyntaxTree *ps, Type *t, int var_no);
Command *conv_cbr(SyntaxTree *ps, Command *c, int var);

extern bool next_extern;
extern bool next_const;

const int TYPE_CAST_OWN_STRING = 4096;

#define is_variable(kind)	(((kind) == KindVarLocal) or ((kind) == KindVarGlobal))

inline bool type_match(Type *type, bool is_class, Type *wanted);
inline bool direct_type_match(Type *a, Type *b)
{
	return ( (a==b) or ( (a->is_pointer) and (b->is_pointer) ) or (a->IsDerivedFrom(b)) );
}
inline bool type_match_with_cast(Type *type, bool is_class, bool is_modifiable, Type *wanted, int &penalty, int &cast);


long long s2i2(const string &str)
{
	if ((str.num > 1) and (str[0]=='0')and(str[1]=='x')){
		long long r=0;
		for (int i=2;i<str.num;i++){
			r *= 16;
			if ((str[i]>='0')and(str[i]<='9'))
				r+=str[i]-48;
			if ((str[i]>='a')and(str[i]<='f'))
				r+=str[i]-'a'+10;
			if ((str[i]>='A')and(str[i]<='F'))
				r+=str[i]-'A'+10;
		}
		return r;
	}else
		return	str.i64();
}

// find the type of a (potential) constant
//  "1.2" -> float
Type *SyntaxTree::GetConstantType()
{
	msg_db_f("GetConstantType", 4);
	FoundConstantNr = -1;
	FoundConstantScript = NULL;

	// named constants
	foreachi(Constant &c, constants, i)
		if (Exp.cur == c.name){
			FoundConstantNr = i;
			FoundConstantScript = script;
			return c.type;
		}


	// included named constants
	foreach(Script *inc, includes)
		foreachi(Constant &c, inc->syntax->constants, i)
			if (Exp.cur == c.name){
				FoundConstantNr = i;
				FoundConstantScript = inc;
				return c.type;
			}

	// character "..."
	if ((Exp.cur[0] == '\'') and (Exp.cur.back() == '\''))
		return TypeChar;

	// string "..."
	if ((Exp.cur[0] == '"') and (Exp.cur.back() == '"'))
		return flag_string_const_as_cstring ? TypeCString : TypeString;

	// numerical (int/float)
	Type *type = TypeInt;
	bool hex = (Exp.cur.num > 1) and (Exp.cur[0] == '0') and (Exp.cur[1] == 'x');
	char last = 0;
	for (int ic=0;ic<Exp.cur.num;ic++){
		char c = Exp.cur[ic];
		if ((c < '0') or (c > '9')){
			if (hex){
				if ((ic >= 2) and (c < 'a') and (c > 'f'))
					return TypeUnknown;
			}else if (c == '.'){
				type = TypeFloat32;
			}else{
				if ((ic != 0) or (c != '-')){ // allow sign
					if ((c != 'e') and (c != 'E'))
						if (((c != '+') and (c != '-')) or ((last != 'e') and (last != 'E')))
							return TypeUnknown;
				}
			}
		}
		last = c;
	}
	if (type == TypeInt){
		if (hex){
			if ((s2i2(Exp.cur) >= 0x100000000) or (-s2i2(Exp.cur) > 0x00000000))
				type = TypeInt64;
		}else{
			if ((s2i2(Exp.cur) >= 0x80000000) or (-s2i2(Exp.cur) > 0x80000000))
				type = TypeInt64;
		}
	}

	// super array [...]
	if (Exp.cur == "["){
		DoError("super array constant");
	}
	return type;
}

static int _some_int_;
static long long _some_int64_;
static float _some_float_;

string SyntaxTree::GetConstantValue()
{
	Type *type = GetConstantType();
// named constants
	if (FoundConstantNr >= 0)
		return FoundConstantScript->syntax->constants[FoundConstantNr].value;
// literal
	if (type == TypeChar){
		_some_int_ = Exp.cur[1];
		return string((char*)&_some_int_, sizeof(int));
	}
	if ((type == TypeString) or (type == TypeCString)){
		return Exp.cur.substr(1, -2);
	}
	if (type == TypeInt){
		_some_int_ = (int)s2i2(Exp.cur);
		return string((char*)&_some_int_, sizeof(int));
	}
	if (type == TypeInt64){
		_some_int64_ = s2i2(Exp.cur);
		return string((char*)&_some_int64_, sizeof(long long));
	}
	if (type == TypeFloat32){
		_some_float_ = Exp.cur._float();
		return string((char*)&_some_float_, sizeof(float));
	}
	return "";
}


Command *SyntaxTree::DoClassFunction(Command *ob, ClassFunction &cf, Block *block)
{
	msg_db_f("DoClassFunc", 4);

	// the function
	Function *ff = cf.script->syntax->functions[cf.nr];
	/*if (cf.virtual_index >= 0){
		Command *cmd = AddCommand(KindVirtualFunction, cf.virtual_index, ff->literal_return_type);
		cmd->set_num_params(ff->num_params);
		cmd->script = cf.script;
		cmd->set_instance(ob);
		return GetFunctionCall("(virtual)." + cf.name, cmd, f);
	}*/

	Command *cmd = add_command_classfunc(&cf, ob);
	cmd = GetFunctionCall(ff->name, cmd, block);
	return cmd;
}

Command *SyntaxTree::GetOperandExtensionElement(Command *Operand, Block *block)
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

	// super
	if ((type->parent) and (Exp.cur == "super")){
		Exp.next();
		if (deref){
			Operand->type = type->parent->GetPointer();
			return Operand;
		}
		return ref_command(Operand, type->parent->GetPointer());
	}

	// find element
	for (int e=0;e<type->element.num;e++)
		if (Exp.cur == type->element[e].name){
			Exp.next();
			return shift_command(Operand, deref, type->element[e].offset, type->element[e].type);
		}

	// class function?
	foreach(ClassFunction &cf, type->function)
		if (Exp.cur == cf.name){
			if (!deref)
				Operand = ref_command(Operand);
			Exp.next();
			return DoClassFunction(Operand, cf, block);
		}

	DoError("unknown element of " + type->name);
	return NULL;
}

Command *SyntaxTree::GetOperandExtensionArray(Command *Operand, Block *block)
{
	msg_db_f("GetOperandExtensionArray", 4);

	// array index...
	Exp.next();
	Command *index = GetCommand(block);
	if (Exp.cur != "]")
		DoError("\"]\" expected after array index");
	Exp.next();

	// __get__() ?
	ClassFunction *cf = Operand->type->GetGet(index->type);
	if (cf){
		Command *f = add_command_classfunc(cf, ref_command(Operand));
		f->set_param(0, index);
		return f;
	}

	// allowed?
	bool allowed = ((Operand->type->is_array) or (Operand->type->usable_as_super_array()));
	bool pparray = false;
	if (!allowed)
		if (Operand->type->is_pointer){
			if ((!Operand->type->parent->is_array) and (!Operand->type->parent->usable_as_super_array()))
				DoError(format("using pointer type \"%s\" as an array (like in C) is not allowed any more", Operand->type->name.c_str()));
			allowed = true;
			pparray = (Operand->type->parent->usable_as_super_array());
		}
	if (!allowed)
		DoError(format("type \"%s\" is neither an array nor a pointer to an array nor does it have a function __get__(%s)", Operand->type->name.c_str(), index->type->name.c_str()));

	Command *array;

	// pointer?
	if (pparray){
		DoError("test... anscheinend gibt es [] auf * super array");
		//array = cp_command(this, Operand);
/*		Operand->kind = KindPointerAsArray;
		Operand->type = t->type->parent;
		deref_command_old(this, Operand);
		array = Operand->param[0];*/
	}else if (Operand->type->usable_as_super_array()){
		array = add_command_parray(shift_command(Operand, false, 0, Operand->type->GetPointer()),
		                           index, Operand->type->GetArrayElement());
	}else if (Operand->type->is_pointer){
		array = add_command_parray(Operand, index, Operand->type->parent->parent);
	}else{
		array = AddCommand(KIND_ARRAY, 0, Operand->type->parent);
		array->set_num_params(2);
		array->set_param(0, Operand);
		array->set_param(1, index);
	}

	if (index->type != TypeInt){
		Exp.rewind();
		DoError(format("type of index for an array needs to be (int), not (%s)", index->type->name.c_str()));
	}
	return array;
}

// find any ".", "->", or "[...]"'s    or operators?
Command *SyntaxTree::GetOperandExtension(Command *Operand, Block *block)
{
	msg_db_f("GetOperandExtension", 4);

	// nothing?
	int op = WhichPrimitiveOperator(Exp.cur);
	if ((Exp.cur != ".") and (Exp.cur != "[") and (Exp.cur != "->") and (op < 0))
		return Operand;

	if (Exp.cur == "->")
		DoError("\"->\" deprecated,  use \".\" instead");

	if (Exp.cur == "."){
		// class element?

		Operand = GetOperandExtensionElement(Operand, block);

	}else if (Exp.cur == "["){
		// array?

		Operand = GetOperandExtensionArray(Operand, block);


	}else if (op >= 0){
		// unary operator? (++,--)

		for (int i=0;i<PreOperators.num;i++)
			if (PreOperators[i].primitive_id == op)
				if ((PreOperators[i].param_type_1 == Operand->type) and (PreOperators[i].param_type_2 == TypeVoid)){
					Exp.next();
					return add_command_operator(Operand, NULL, i);
				}
		return Operand;
	}

	// recursion
	return GetOperandExtension(Operand, block);
}

Command *SyntaxTree::GetSpecialFunctionCall(const string &f_name, Command *Operand, Block *block)
{
	msg_db_f("GetSpecialFuncCall", 4);

	// sizeof
	if ((Operand->kind != KIND_COMPILER_FUNCTION) or (Operand->link_no != COMMAND_SIZEOF))
		DoError("evil special function");

	Exp.next();
	int nc = AddConstant(TypeInt);
	Command *c = add_command_const(nc);


	Type *type = FindType(Exp.cur);
	if (type){
		constants[nc].setInt(type->size);
	}else if ((GetExistence(Exp.cur, block)) and ((GetExistenceLink.kind == KIND_VAR_GLOBAL) or (GetExistenceLink.kind == KIND_VAR_LOCAL))){
		constants[nc].setInt(GetExistenceLink.type->size);
	}else{
		type = GetConstantType();
		if (type)
			constants[nc].setInt(type->size);
		else
			DoError("type-name or variable name expected in sizeof(...)");
	}
	Exp.next();
	if (Exp.cur != ")")
		DoError("\")\" expected after parameter list");
	Exp.next();

	return c;
}


// cmd needs to have Param[]'s existing with correct Type!
void SyntaxTree::FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, Command *cmd)
{
	msg_db_f("FindFuncSingleParam", 4);
	Command *param = GetCommand(block);

	wanted_type[p] = TypeUnknown;
	if (cmd->kind == KIND_FUNCTION){
		Function *ff = cmd->script->syntax->functions[cmd->link_no];
		if (p < ff->num_params)
			wanted_type[p] = ff->literal_param_type[p];
	}else if (cmd->kind == KIND_VIRTUAL_FUNCTION){
		ClassFunction *cf = cmd->instance->type->parent->GetVirtualFunction(cmd->link_no);
		if (!cf)
			DoError("FindFunctionSingleParameter: can't find virtual function...?!?");
		if (p < cf->param_type.num)
			wanted_type[p] = cf->param_type[p];
	}else if (cmd->kind == KIND_COMPILER_FUNCTION){
		if (p < PreCommands[cmd->link_no].param.num)
			wanted_type[p] = PreCommands[cmd->link_no].param[p].type;
	}else
		DoError("evil function...");
	// link parameters
	cmd->set_param(p, param);
}

void SyntaxTree::FindFunctionParameters(Array<Type*> &wanted_type, Block *block, Command *cmd)
{
	if (Exp.cur != "(")
		DoError("\"(\" expected in front of function parameter list");
	msg_db_f("FindFunctionParameters", 4);
	Exp.next();

	// list of parameters
	for (int p=0;;p++){
		if (Exp.cur == ")")
			break;
		wanted_type.add(TypeUnknown);
		// find parameter

		FindFunctionSingleParameter(p, wanted_type, block, cmd);

		if (Exp.cur != ","){
			if (Exp.cur == ")")
				break;
			DoError("\",\" or \")\" expected after parameter for function");
		}
		Exp.next();
	}
	Exp.next(); // ')'
}

Command *apply_type_cast(SyntaxTree *ps, int tc, Command *param);


// check, if the command <link> links to really has type <type>
//   ...and try to cast, if not
Command *SyntaxTree::CheckParamLink(Command *link, Type *type, const string &f_name, int param_no)
{
	msg_db_f("CheckParamLink", 4);
	// type cast needed and possible?
	Type *pt = link->type;
	Type *wt = type;

	// "silent" pointer (&)?
	if ((wt->is_pointer) and (wt->is_silent)){
		if (direct_type_match(pt, wt->parent)){

			return ref_command(link);
		}else if ((pt->is_pointer) and (direct_type_match(pt->parent, wt->parent))){
			// silent Ref & of *

			// no need to do anything...
		}else{
			Exp.rewind();
			DoError(format("(c) parameter %d in command \"%s\" has type (%s), (%s) expected", param_no + 1, f_name.c_str(), pt->name.c_str(), wt->name.c_str()));
		}

	// normal type cast
	}else if (!direct_type_match(pt, wt)){
		int pen, tc;
		/*int tc = -1;
		for (int i=0;i<TypeCasts.num;i++)
			if ((direct_type_match(TypeCasts[i].source, pt)) and (direct_type_match(TypeCasts[i].dest, wt)))
				tc = i;*/

		if (type_match_with_cast(pt, false, false, wt, pen, tc))
			return apply_type_cast(this, tc, link);
		Exp.rewind();
		DoError(format("parameter %d in command \"%s\" has type (%s), (%s) expected", param_no + 1, f_name.c_str(), pt->name.c_str(), wt->name.c_str()));
	}
	return link;
}

// creates <Operand> to be the function call
//  on entry <Operand> only contains information from GetExistence (Kind, Nr, Type, NumParams)
Command *SyntaxTree::GetFunctionCall(const string &f_name, Command *Operand, Block *block)
{
	msg_db_f("GetFunctionCall", 4);

	// function as a variable?
	if (Exp.cur_exp >= 2)
	if ((Exp.get_name(Exp.cur_exp - 2) == "&") and (Exp.cur != "(")){
		if (Operand->kind == KIND_FUNCTION){
			Command *c = AddCommand(KIND_VAR_FUNCTION, Operand->link_no, TypePointer);
			c->script = Operand->script;
			return c;
		}else{
			Exp.rewind();
			//DoError("\"(\" expected in front of parameter list");
			DoError("only functions can be referenced");
		}
	}


	// "special" functions
    if (Operand->kind == KIND_COMPILER_FUNCTION)
	    if (Operand->link_no == COMMAND_SIZEOF){
			return GetSpecialFunctionCall(f_name, Operand, block);
	    }

	// link operand onto this command


	// find (and provisional link) the parameters in the source
	Array<Type*> wanted_type;

	bool needs_brackets = ((Operand->type != TypeVoid) or (Operand->param.num != 1));
	if (needs_brackets){
		FindFunctionParameters(wanted_type, block, Operand);

	}else{
		wanted_type.add(TypeUnknown);
		FindFunctionSingleParameter(0, wanted_type, block, Operand);
	}

	// test compatibility
	if (wanted_type.num != Operand->param.num){
		Exp.rewind();
		DoError(format("function \"%s\" expects %d parameters, %d were found",f_name.c_str(), Operand->param.num, wanted_type.num));
	}
	for (int p=0;p<wanted_type.num;p++){
		Operand->set_param(p, CheckParamLink(Operand->param[p], wanted_type[p], f_name, p));
	}
	return Operand;
}

Command *build_list(SyntaxTree *ps, Array<Command*> &el)
{
	if (el.num == 0)
		ps->DoError("empty arrays not supported yet");
//	if (el.num > SCRIPT_MAX_PARAMS)
//		ps->DoError(format("only %d elements in auto arrays supported yet", SCRIPT_MAX_PARAMS));
	Type *t = ps->CreateArrayType(el[0]->type, -1);
	Command *c = ps->AddCommand(KIND_ARRAY_BUILDER, 0, t);
	c->set_num_params(el.num);
	for (int i=0; i<el.num; i++){
		if (el[i]->type != el[0]->type)
			ps->DoError(format("inhomogenous array types %s/%s", el[i]->type->name.c_str(), el[0]->type->name.c_str()));
		c->set_param(i, el[i]);
	}
	return c;
}

Command *SyntaxTree::GetOperand(Block *block)
{
	msg_db_f("GetOperand", 4);
	Command *Operand = NULL;

	// ( -> one level down and combine commands
	if (Exp.cur == "("){
		Exp.next();
		Operand = GetCommand(block);
		if (Exp.cur != ")")
			DoError("\")\" expected");
		Exp.next();
	}else if (Exp.cur == "&"){ // & -> address operator
		Exp.next();
		Operand = ref_command(GetOperand(block));
	}else if (Exp.cur == "*"){ // * -> dereference
		Exp.next();
		Operand = GetOperand(block);
		if (!Operand->type->is_pointer){
			Exp.rewind();
			DoError("only pointers can be dereferenced using \"*\"");
		}
		Operand = deref_command(Operand);
	}else if (Exp.cur == "["){
		Exp.next();
		Array<Command*> el;
		while(true){
			el.add(GetCommand(block));
			if ((Exp.cur != ",") and (Exp.cur != "]"))
				DoError("\",\" or \"]\" expected");
			if (Exp.cur == "]")
				break;
			Exp.next();
		}
		Operand = build_list(this, el);
		Exp.next();
	}else if (Exp.cur == "new"){ // new operator
		Exp.next();
		Type *t = ParseType();
		Operand = add_command_compilerfunc(COMMAND_NEW);
		Operand->type = t->GetPointer();
		if (Exp.cur == "("){
			ClassFunction *cf = t->GetComplexConstructor();
			if (!cf)
				DoError(format("class \"%s\" does not have a constructor with parameters", t->name.c_str()));
			Operand->set_num_params(1);
			Operand->set_param(0, DoClassFunction(NULL, *cf, block));
		}
	}else if (Exp.cur == "delete"){ // delete operator
		Exp.next();
		Operand = add_command_compilerfunc(COMMAND_DELETE);
		Operand->set_param(0, GetOperand(block));
		if (!Operand->param[0]->type->is_pointer)
			DoError("pointer expected after delete");
	}else{
		// direct operand
		if (GetExistence(Exp.cur, block)){
			Operand = cp_command(&GetExistenceLink);
			string f_name =  Exp.cur;
			Exp.next();
			// variables get linked directly...

			// operand is executable
			if ((Operand->kind == KIND_FUNCTION) or (Operand->kind == KIND_VIRTUAL_FUNCTION) or (Operand->kind == KIND_COMPILER_FUNCTION)){
				Operand = GetFunctionCall(f_name, Operand, block);

			}else if (Operand->kind == KIND_PRIMITIVE_OPERATOR){
				// unary operator
				int _ie=Exp.cur_exp-1;
				int po = Operand->link_no, o=-1;
				Command *sub_command = GetOperand(block);
				Type *r = TypeVoid;
				Type *p2 = sub_command->type;

				// exact match?
				bool ok=false;
				for (int i=0;i<PreOperators.num;i++)
					if (po == PreOperators[i].primitive_id)
						if ((PreOperators[i].param_type_1 == TypeVoid) and (type_match(p2, false, PreOperators[i].param_type_2))){
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
							if ((PreOperators[i].param_type_1 == TypeVoid) and (type_match_with_cast(p2, false, false, PreOperators[i].param_type_2, pen2, c2))){
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
						sub_command = apply_type_cast(this, c2_best, sub_command);
					}
				}


				if (!ok){
					Exp.cur_exp = _ie;
					DoError("unknown unitary operator " + PrimitiveOperators[po].name + " " + p2->name);
				}
				return add_command_operator(sub_command, NULL, o);
			}
		}else{
			Type *t = GetConstantType();
			if (t != TypeUnknown){
				Operand = AddCommand(KIND_CONSTANT, AddConstant(t), t);
				// constant for parameter (via variable)
				constants[Operand->link_no].value = GetConstantValue();
				Exp.next();
			}else{
				//Operand.Kind=0;
				DoError("unknown operand");
			}
		}

	}

	// resolve arrays, and structures...
	Operand = GetOperandExtension(Operand,block);

	return Operand;
}

// only "primitive" operator -> no type information
Command *SyntaxTree::GetPrimitiveOperator(Block *block)
{
	msg_db_f("GetOperator",4);
	int op = WhichPrimitiveOperator(Exp.cur);
	if (op < 0)
		return NULL;

	// command from operator
	Command *cmd = AddCommand(KIND_PRIMITIVE_OPERATOR, op, TypeUnknown);
	// only provisional (only operator sign, parameters and their types by GetCommand!!!)

	Exp.next();
	return cmd;
}

/*inline int find_operator(int primitive_id, Type *param_type1, Type *param_type2)
{
	for (int i=0;i<PreOperator.num;i++)
		if (PreOperator[i].PrimitiveID == primitive_id)
			if ((PreOperator[i].ParamType1 == param_type1) and (PreOperator[i].ParamType2 == param_type2))
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
	if ((type->is_pointer) and (wanted == TypePointer))
		return true;
	if ((is_class) and (wanted == TypeClass))
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
	if (wanted == TypeString){
		ClassFunction *cf = type->GetFunc("str", TypeString, 0);
		if (cf){
			penalty = 50;
			cast = TYPE_CAST_OWN_STRING;
			return true;
		}
	}
	for (int i=0;i<TypeCasts.num;i++)
		if ((direct_type_match(TypeCasts[i].source, type)) and (direct_type_match(TypeCasts[i].dest, wanted))){ // type_match()?
			penalty = TypeCasts[i].penalty;
			cast = i;
			return true;
		}
	return false;
}

Command *apply_type_cast(SyntaxTree *ps, int tc, Command *param)
{
	if (tc < 0)
		return param;
	if (tc == TYPE_CAST_OWN_STRING){
		ClassFunction *cf = param->type->GetFunc("str", TypeString, 0);
		if (cf)
			return ps->add_command_classfunc(cf, ps->ref_command(param));
		ps->DoError("automatic .str() not implemented yet");
		return param;
	}
	if (param->kind == KIND_CONSTANT){
		string data_old = ps->constants[param->link_no].value;
		string data_new = TypeCasts[tc].func(data_old);
		/*if ((TypeCasts[tc].dest->is_array) or (TypeCasts[tc].dest->is_super_array)){
			// arrays as return value -> reference!
			int size = TypeCasts[tc].dest->size;
			if (TypeCasts[tc].dest == TypeString)
				size = SCRIPT_MAX_STRING_CONST_LENGTH;
			delete[] data_old;
			ps->Constants[param->link_no].data = new char[size];
			data_new = *(char**)data_new;
			memcpy(ps->Constants[param->link_no].data, data_new, size);
		}else*/
		ps->constants[param->link_no].value = data_new;
		ps->constants[param->link_no].type = TypeCasts[tc].dest;
		param->type = TypeCasts[tc].dest;
		return param;
	}else{
		if (TypeCasts[tc].kind == KIND_FUNCTION){
			Command *c = ps->add_command_func(TypeCasts[tc].script, TypeCasts[tc].func_no, TypeCasts[tc].dest);
			c->set_param(0, param);
			return c;
		}else if (TypeCasts[tc].kind == KIND_COMPILER_FUNCTION){
			Command *c = ps->add_command_compilerfunc(TypeCasts[tc].func_no);
			c->set_param(0, param);
			return c;
		}
	}
	return param;
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

	Type *pp1 = p1;
	if (pp1->is_pointer)
		pp1 = p1->parent;

	// exact match as class function?
	foreach(ClassFunction &f, pp1->function)
		if (f.name == op_func_name){
			// exact match as class function but missing a "&"?
			if (f.param_type[0]->is_pointer and f.param_type[0]->is_silent){
				if (direct_type_match(p2, f.param_type[0]->parent)){
					Command *inst = ref_command(param1);
					if (p1 == pp1)
						op = add_command_classfunc(&f, inst);
					else
						op = add_command_classfunc(&f, deref_command(inst));
					op->set_num_params(1);
					op->set_param(0, ref_command(param2));
					return op;
				}
			}else if (type_match(p2, equal_classes, f.param_type[0])){
				Command *inst = ref_command(param1);
				if (p1 == pp1)
					op = add_command_classfunc(&f, inst);
				else
					op = add_command_classfunc(&f, deref_command(inst));
				op->set_num_params(1);
				op->set_param(0, param2);
				return op;
			}
		}

	// exact (operator) match?
	for (int i=0;i<PreOperators.num;i++)
		if (op_no == PreOperators[i].primitive_id)
			if (type_match(p1, equal_classes, PreOperators[i].param_type_1) and type_match(p2, equal_classes, PreOperators[i].param_type_2)){
				return add_command_operator(param1, param2, i);
			}


	// needs type casting?
	int pen1, pen2;
	int c1, c2, c1_best, c2_best;
	int pen_min = 2000;
	int op_found = -1;
	bool op_is_class_func = false;
	for (int i=0;i<PreOperators.num;i++)
		if (op_no == PreOperators[i].primitive_id)
			if (type_match_with_cast(p1, equal_classes, left_modifiable, PreOperators[i].param_type_1, pen1, c1) and type_match_with_cast(p2, equal_classes, false, PreOperators[i].param_type_2, pen2, c2))
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
		param1 = apply_type_cast(this, c1_best, param1);
		param2 = apply_type_cast(this, c2_best, param2);
		if (op_is_class_func){
			Command *inst = ref_command(param1);
			op = add_command_classfunc(&p1->function[op_found], inst);
			op->set_num_params(1);
			op->set_param(0, param2);
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
		if (PrimitiveOperators[Operator[i]->link_no].level > PrimitiveOperators[Operator[mio]->link_no].level)
			mio = i;
	}

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

Command *SyntaxTree::GetCommand(Block *block)
{
	msg_db_f("GetCommand", 4);
	Array<Command*> Operand;
	Array<Command*> Operator;
	Array<int> op_exp;

	// find the first operand
	Operand.add(GetOperand(block));

	// find pairs of operators and operands
	for (int i=0;true;i++){
		op_exp.add(Exp.cur_exp);
		Command *op = GetPrimitiveOperator(block);
		if (!op)
			break;
		Operator.add(op);
		if (Exp.end_of_line()){
			//Exp.rewind();
			DoError("unexpected end of line after operator");
		}
		Operand.add(GetOperand(block));
	}


	// in each step remove/link the most important operator
	while(Operator.num > 0)
		LinkMostImportantOperator(Operand, Operator, op_exp);

	// complete command is now collected in Operand[0]
	return Operand[0];
}

void SyntaxTree::ParseSpecialCommandFor(Block *block)
{
	msg_db_f("ParseSpecialCommandFor", 4);

	// variable name
	Exp.next();
	string var_name = Exp.cur;
	Exp.next();

	if (Exp.cur != "in")
		DoError("\"in\" expected after variable in for");
	Exp.next();

	// first value
	Command *val0 = GetCommand(block);


	// last value
	if (Exp.cur != ":")
		DoError("\":\" expected after first value in for");
	Exp.next();
	Command *val1 = GetCommand(block);

	Command *val_step = NULL;
	if (Exp.cur == ":"){
		Exp.next();
		val_step = GetCommand(block);
	}

	// type?
	Type *t = val0->type;
	if (val1->type == TypeFloat32)
		t = val1->type;
	if (val_step)
		if (val_step->type == TypeFloat32)
			t = val_step->type;
	val0 = CheckParamLink(val0, t, "for", 1);
	val1 = CheckParamLink(val1, t, "for", 1);
	if (val_step)
		val_step = CheckParamLink(val_step, t, "for", 1);

	// variable
	Command *for_var;
	int var_no = block->add_var(var_name, t);
	exlink_make_var_local(this, t, var_no);
	for_var = cp_command(&GetExistenceLink);

	// implement
	// for_var = val0
	Command *cmd_assign = add_command_operator(for_var, val0, OperatorIntAssign);
	block->commands.add(cmd_assign);

	// while(for_var < val1)
	Command *cmd_cmp = add_command_operator(for_var, val1, OperatorIntSmaller);

	Command *cmd_while = add_command_compilerfunc(COMMAND_FOR);
	cmd_while->set_param(0, cmd_cmp);
	block->commands.add(cmd_while);
	ExpectNewline();
	// ...block
	Exp.next_line();
	ExpectIndent();
	int loop_block_no = blocks.num; // should get created...soon
	ParseCompleteCommand(block);

	// ...for_var += 1
	Command *cmd_inc;
	if (for_var->type == TypeInt){
		if (val_step)
			cmd_inc = add_command_operator(for_var, val_step, OperatorIntAddS);
		else
			cmd_inc = add_command_operator(for_var, val1 /*dummy*/, OperatorIntIncrease);
	}else{
		if (!val_step){
			int nc = AddConstant(TypeFloat32);
			*(float*)constants[nc].value.data = 1.0;
			val_step = add_command_const(nc);
		}
		cmd_inc = add_command_operator(for_var, val_step, OperatorFloatAddS);
	}
	Block *loop_block = blocks[loop_block_no];
	loop_block->commands.add(cmd_inc); // add to loop-block

	// <for_var> declared internally?
	// -> force it out of scope...
	block->function->var[for_var->link_no].name = "-out-of-scope-";
	// TODO  FIXME
}

void SyntaxTree::ParseSpecialCommandForall(Block *block)
{
	msg_db_f("ParseSpecialCommandForall", 4);

	// variable
	Exp.next();
	string var_name = Exp.cur;
	Exp.next();

	// index
	string index_name = format("-for_index_%d-", for_index_count ++);
	if (Exp.cur == ","){
		Exp.next();
		index_name = Exp.cur;
		Exp.next();
	}

	// for index
	int var_no_index = block->add_var(index_name, TypeInt);
	exlink_make_var_local(this, TypeInt, var_no_index);
		Command *for_index = cp_command(&GetExistenceLink);

	// super array
	if (Exp.cur != "in")
		DoError("\"in\" expected after variable in \"for . in .\"");
	Exp.next();
	Command *for_array = GetOperand(block);
	if ((!for_array->type->usable_as_super_array()) and (!for_array->type->is_array))
		DoError("array or list expected as second parameter in \"for . in .\"");
	//Exp.next();

	// variable...
	Type *var_type = for_array->type->GetArrayElement();
	int var_no = block->add_var(var_name, var_type);
	exlink_make_var_local(this, var_type, var_no);
	Command *for_var = cp_command(&GetExistenceLink);

	// 0
	int nc = AddConstant(TypeInt);
	constants[nc].setInt(0);
	Command *val0 = add_command_const(nc);

	// implement
	// for_index = 0
	Command *cmd_assign = add_command_operator(for_index, val0, OperatorIntAssign);
	block->commands.add(cmd_assign);

	Command *val1;
	if (for_array->type->usable_as_super_array()){
		// array.num
		val1 = AddCommand(KIND_ADDRESS_SHIFT, config.pointer_size, TypeInt);
		val1->set_num_params(1);
		val1->set_param(0, for_array);
	}else{
		// array.size
		int nc = AddConstant(TypeInt);
		constants[nc].setInt(for_array->type->array_length);
		val1 = add_command_const(nc);
	}

	// while(for_index < val1)
	Command *cmd_cmp = add_command_operator(for_index, val1, OperatorIntSmaller);

	Command *cmd_while = add_command_compilerfunc(COMMAND_FOR);
	cmd_while->set_param(0, cmd_cmp);
	block->commands.add(cmd_while);
	ExpectNewline();
	// ...block
	Exp.next_line();
	ExpectIndent();
	int loop_block_no = blocks.num; // should get created...soon
	ParseCompleteCommand(block);

	// ...for_index += 1
	Command *cmd_inc = add_command_operator(for_index, val1 /*dummy*/, OperatorIntIncrease);
	Block *loop_block = blocks[loop_block_no];
	loop_block->commands.add(cmd_inc); // add to loop-block

	// &for_var
	Command *for_var_ref = ref_command(for_var);

	Command *array_el;
	if (for_array->type->usable_as_super_array()){
		// &array.data[for_index]
		array_el = add_command_parray(shift_command(cp_command(for_array), false, 0, var_type->GetPointer()),
	                                       	   for_index, var_type);
	}else{
		// &array[for_index]
		array_el = add_command_parray(ref_command(for_array),
	                                       	   for_index, var_type);
	}
	Command *array_el_ref = ref_command(array_el);

	// &for_var = &array[for_index]
	Command *cmd_var_assign = add_command_operator(for_var_ref, array_el_ref, OperatorPointerAssign);
	loop_block->commands.insert(cmd_var_assign, 0);

	// ref...
	block->function->var[var_no].type = var_type->GetPointer();
	foreachi(Command *c, loop_block->commands, i)
		loop_block->commands[i] = conv_cbr(this, c, var_no);

	// force for_var out of scope...
	block->function->var[for_var->link_no].name = "-out-of-scope-";
	block->function->var[for_index->link_no].name = "-out-of-scope-";
}

void SyntaxTree::ParseSpecialCommandWhile(Block *block)
{
	msg_db_f("ParseSpecialCommandWhile", 4);
	Exp.next();
	Command *cmd_cmp = CheckParamLink(GetCommand(block), TypeBool, "while", 0);
	ExpectNewline();

	Command *cmd_while = add_command_compilerfunc(COMMAND_WHILE);
	cmd_while->set_param(0, cmd_cmp);
	block->commands.add(cmd_while);
	// ...block
	Exp.next_line();
	ExpectIndent();
	ParseCompleteCommand(block);
}

void SyntaxTree::ParseSpecialCommandBreak(Block *block)
{
	msg_db_f("ParseSpecialCommandBreak", 4);
	Exp.next();
	Command *cmd = add_command_compilerfunc(COMMAND_BREAK);
	block->commands.add(cmd);
}

void SyntaxTree::ParseSpecialCommandContinue(Block *block)
{
	msg_db_f("ParseSpecialCommandContinue", 4);
	Exp.next();
	Command *cmd = add_command_compilerfunc(COMMAND_CONTINUE);
	block->commands.add(cmd);
}

void SyntaxTree::ParseSpecialCommandReturn(Block *block)
{
	msg_db_f("ParseSpecialCommandReturn", 4);
	Exp.next();
	Command *cmd = add_command_compilerfunc(COMMAND_RETURN);
	block->commands.add(cmd);
	if (block->function->return_type == TypeVoid){
		cmd->set_num_params(0);
	}else{
		Command *cmd_value = CheckParamLink(GetCommand(block), block->function->return_type, "return", 0);
		cmd->set_num_params(1);
		cmd->set_param(0, cmd_value);
	}
	ExpectNewline();
}

void SyntaxTree::ParseSpecialCommandIf(Block *block)
{
	msg_db_f("ParseSpecialCommandIf", 4);
	int ind = Exp.cur_line->indent;
	Exp.next();
	Command *cmd_cmp = CheckParamLink(GetCommand(block), TypeBool, "if", 0);
	ExpectNewline();

	Command *cmd_if = add_command_compilerfunc(COMMAND_IF);
	cmd_if->set_param(0, cmd_cmp);
	block->commands.add(cmd_if);
	// ...block
	Exp.next_line();
	ExpectIndent();
	ParseCompleteCommand(block);
	Exp.next_line();

	// else?
	if ((!Exp.end_of_file()) and (Exp.cur == "else") and (Exp.cur_line->indent >= ind)){
		cmd_if->link_no = COMMAND_IF_ELSE;
		Exp.next();
		// iterative if
		if (Exp.cur == "if"){
			// sub-if's in a new block
			Block *new_block = AddBlock(block->function, block);
			// parse the next if
			ParseCompleteCommand(new_block);
			// command for the found block
			Command *cmd_block = add_command_block(new_block);
			// ...
			block->commands.add(cmd_block);
			return;
		}
		ExpectNewline();
		// ...block
		Exp.next_line();
		ExpectIndent();
		ParseCompleteCommand(block);
		//Exp.next_line();
	}else{
		Exp.cur_line --;
		Exp.cur_exp = Exp.cur_line->exp.num - 1;
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
	}
}

void SyntaxTree::ParseSpecialCommand(Block *block)
{
	bool has_colon = false;
	foreach(ExpressionBuffer::Expression &e, Exp.cur_line->exp)
		if (e.name == ":")
			has_colon = true;

	// special commands...
	if (Exp.cur == "for" and !has_colon){
		ParseSpecialCommandForall(block);
	}else if (Exp.cur == "for"){
		ParseSpecialCommandFor(block);
	}else if (Exp.cur == "while"){
		ParseSpecialCommandWhile(block);
 	}else if (Exp.cur == "break"){
		ParseSpecialCommandBreak(block);
	}else if (Exp.cur == "continue"){
		ParseSpecialCommandContinue(block);
	}else if (Exp.cur == "return"){
		ParseSpecialCommandReturn(block);
	}else if (Exp.cur == "if"){
		ParseSpecialCommandIf(block);
	}
}

/*void ParseBlock(sBlock *block, sFunction *f)
{
}*/

// we already are in the line to analyse ...indentation for a new block should compare to the last line
void SyntaxTree::ParseCompleteCommand(Block *block)
{
	msg_db_f("GetCompleteCommand", 4);
	// cur_exp = 0!

	bool is_type = FindType(Exp.cur);
	int last_indent = Exp.indent_0;

	// block?  <- indent
	if (Exp.indented){
		Exp.indented = false;
		Exp.cur_exp = 0; // bad hack...
		Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
		msg_db_f("Block", 4);
		Block *new_block = AddBlock(block->function, block);

		Command *c = add_command_block(new_block);
		block->commands.add(c);

		for (int i=0;true;i++){
			if (((i > 0) and (Exp.cur_line->indent < last_indent)) or (Exp.end_of_file()))
				break;

			ParseCompleteCommand(new_block);
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
		Command *c = add_command_compilerfunc(COMMAND_ASM);
		block->commands.add(c);

	// local (variable) definitions...
	// type of variable
	}else if (is_type){
		Type *type = ParseType();
		for (int l=0;!Exp.end_of_line();l++){
			// name
			block->add_var(Exp.cur, type);
			Exp.next();

			// assignment?
			if (Exp.cur == "="){
				Exp.rewind();
				// parse assignment
				Command *c = GetCommand(block);
				block->commands.add(c);
			}
			if (Exp.end_of_line())
				break;
			if ((Exp.cur != ",") and (!Exp.end_of_line()))
				DoError("\",\", \"=\" or newline expected after declaration of local variable");
			Exp.next();
		}
		return;
	}else{


	// commands (the actual code!)
		if ((Exp.cur == "for") or (Exp.cur == "while") or (Exp.cur == "break") or (Exp.cur == "continue") or (Exp.cur == "return") or (Exp.cur == "if")){
			ParseSpecialCommand(block);

		}else{

			// normal commands
			Command *c = GetCommand(block);

			// link
			block->commands.add(c);
		}
	}

	ExpectNewline();
}

extern Array<Script*> loading_script_stack;

void SyntaxTree::ParseImport()
{
	msg_db_f("ParseImport", 4);
	Exp.next(); // 'use' / 'import'

	string name = Exp.cur;
	if (name.find(".kaba") >= 0){

		string filename = script->filename.dirname() + name.substr(1, name.num - 2); // remove "
		filename = filename.no_recursion();



		foreach(Script *ss, loading_script_stack)
			if (ss->filename == filename.sys_filename())
				DoError("recursive include");

		msg_right();
		Script *include;
		try{
			include = Load(filename, script->just_analyse or config.compile_os);
			// os-includes will be appended to syntax_tree... so don't compile yet
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
			Constant *c = &constants[nc];
			c->name = Exp.cur;
			Exp.next();

			// explicit value
			if (Exp.cur == "="){
				Exp.next();
				ExpectNoNewline();
				Type *type = GetConstantType();
				if (type == TypeInt)
					value = *(int*)GetConstantValue().data;
				else
					DoError("integer constant expected after \"=\" for explicit value of enum");
				Exp.next();
			}
			c->setInt(value ++);

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

void SyntaxTree::ParseClassFunctionHeader(Type *t, bool as_extern, bool as_virtual, bool overwrite)
{
	Function *f = ParseFunctionHeader(t, as_extern);
	int n = -1;
	foreachi(Function *g, functions, i)
		if (f == g)
			n = i;

	t->AddFunction(this, n, as_virtual, overwrite);
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
		if ((l->indent == 1) and (l->exp[0].name == "virtual"))
			count ++;
		else if ((l->indent == 1) and (l->exp[0].name == "extern") and (l->exp[1].name == "virtual"))
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
		Exp.next();
		Type *parent = ParseType(); // force
		if (!_class->DeriveFrom(parent, true))
			DoError(format("parental type in class definition after \":\" has to be a class, but (%s) is not", parent->name.c_str()));
		_offset = parent->size;
	}
	ExpectNewline();

	// elements
	for (int num=0; !Exp.end_of_file(); num++){
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

		Type *type = ParseType(); // force
		for (int j=0;!Exp.end_of_line();j++){
			//int indent = Exp.cur_line->indent;

			ClassElement el;
			el.type = type;
			el.name = Exp.cur;
			Exp.next();

			// is a function?
			bool is_function = false;
			if (Exp.cur == "(")
			    is_function = true;
			if (is_function){
				Exp.cur_exp = ie;
				Exp.cur = Exp.cur_line->exp[Exp.cur_exp].name;
				ParseClassFunctionHeader(_class, next_extern, next_virtual, overwrite);

				break;
			}

			// overwrite?
			ClassElement *orig = NULL;
			foreachi(ClassElement &e, _class->element, i)
				if (e.name == el.name) //and e.type->is_pointer and el.type->is_pointer)
						orig = &e;
			if (overwrite and ! orig)
				DoError(format("can not overwrite element '%s', no previous definition", el.name.c_str()));
			if (!overwrite and orig)
				DoError(format("element '%s' is already defined, use 'overwrite' to overwrite", el.name.c_str()));
			if (overwrite){
				if (orig->type->is_pointer and el.type->is_pointer)
					orig->type = el.type;
				else
					DoError("can only overwrite pointer elements with other pointer type");
				continue;
			}

			// check parsing dependencies
			if (!type->is_size_known())
				DoError("size of type " + type->name + " is not known at this point");


			// add element
			if (type_needs_alignment(type))
				_offset = mem_align(_offset, 4);
			_offset = ProcessClassOffset(_class->name, el.name, _offset);
			if ((Exp.cur != ",") and (!Exp.end_of_line()))
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
//	_class->vtable = cur_virtual_index;
	//foreach(ClassFunction &cf, _class->function)
	//	_class->num_virtual = max(_class->num_virtual, cf.virtual_index);
	if (_class->vtable.num > 0){
		if (_class->parent){
			if (_class->parent->vtable.num == 0)
				DoError("no virtual functions allowed when inheriting from class without virtual functions");
			// element "-vtable-" being derived
		}else{
			foreach(ClassElement &e, _class->element)
				e.offset = ProcessClassOffset(_class->name, e.name, e.offset + config.pointer_size);

			ClassElement el;
			el.name = "-vtable-";
			el.type = TypePointer;
			el.offset = 0;
			el.hidden = true;
			_class->element.insert(el, 0);
			_offset += config.pointer_size;
		}
	}

	foreach(ClassElement &e, _class->element)
		if (type_needs_alignment(e.type))
			_offset = mem_align(_offset, 4);
	_class->size = ProcessClassSize(_class->name, _offset);

	AddFunctionHeadersForClass(_class);

	_class->fully_parsed = true;

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
	Command *cv = PreProcessCommand(GetCommand(root_of_all_evil.block));

	if ((cv->kind != KIND_CONSTANT) or (cv->type != type))
		DoError(format("only constants of type \"%s\" allowed as value for this constant", type->name.c_str()));

	// give our const the name
	Constant *c = &constants[cv->link_no];
	c->name = name;
}

void SyntaxTree::ParseVariableDef(bool single, Block *block)
{
	msg_db_f("ParseVariableDef", 4);
	Type *type = ParseType(); // force

	for (int j=0;true;j++){
		ExpectNoNewline();

		// name
		string name = Exp.cur;
		Exp.next();

		if (next_const){
			ParseGlobalConst(name, type);
		}else
			block->add_var(name, type);

		if ((Exp.cur != ",") and (!Exp.end_of_line()))
			DoError("\",\" or newline expected after definition of a global variable");

		// last one?
		if (Exp.end_of_line())
			break;

		Exp.next(); // ','
	}
}

bool peek_commands_super(ExpressionBuffer &Exp)
{
	ExpressionBuffer::Line *l = Exp.cur_line + 1;
	if (l->exp.num < 3)
		return false;
	if ((l->exp[0].name == "super") and (l->exp[1].name == ".") and (l->exp[2].name == "__init__"))
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
	ParseCompleteCommand(f->block);
	return true;
}

void Function::Update(Type *class_type)
{
	// save "original" param types (Var[].Type gets altered for call by reference)
	literal_param_type.resize(num_params);
	for (int i=0;i<num_params;i++)
		literal_param_type[i] = var[i].type;

	// return by memory
	if (return_type->UsesReturnByMemory())
		block->add_var("-return-", return_type->GetPointer());

	// class function
	_class = class_type;
	if (class_type){
		if (__get_var("self") < 0)
			block->add_var("self", class_type->GetPointer());

		// convert name to Class.Function
		name = class_type->name + "." +  name;
	}
}

Type *_make_array_(SyntaxTree *s, Type *t, Array<int> dim)
{
	string orig_name = t->name;
	foreachb(int d, dim){
		// create array       (complicated name necessary to get correct ordering   int a[2][4] = (int[4])[2])
		t = s->CreateArrayType(t, d, orig_name, t->name.substr(orig_name.num, -1));
	}
	return t;
}

Type *SyntaxTree::ParseType()
{
	// base type
	Type *t = FindType(Exp.cur);
	if (!t)
		DoError("unknown type");
	Exp.next();

	Array<int> array_dim;

	while (true){

		// pointer?
		if (Exp.cur == "*"){
			t = _make_array_(this, t, array_dim);
			Exp.next();
			t = t->GetPointer();
			array_dim.clear();
			continue;
		}

		if (Exp.cur == "["){
			int array_size;
			Exp.next();

			// no index -> super array
			if (Exp.cur == "]"){
				array_size = -1;

			}else{

				// find array index
				Command *c = PreProcessCommand(GetCommand(root_of_all_evil.block));

				if ((c->kind != KIND_CONSTANT) or (c->type != TypeInt))
					DoError("only constants of type \"int\" allowed for size of arrays");
				array_size = constants[c->link_no].getInt();
				//Exp.next();
				if (Exp.cur != "]")
					DoError("\"]\" expected after array size");
			}

			Exp.next();

			array_dim.add(array_size);
			continue;
		}
		break;
	}

	return _make_array_(this, t, array_dim);
}

Function *SyntaxTree::ParseFunctionHeader(Type *class_type, bool as_extern)
{
	msg_db_f("ParseFunctionHeader", 4);

// return type
	Type *return_type = ParseType(); // force...

	Function *f = AddFunction(Exp.cur, return_type);
	cur_func = f;
	next_extern = false;

	Exp.next();
	Exp.next(); // '('

// parameter list

	if (Exp.cur != ")")
		for (int k=0;;k++){
			// like variable definitions

			// type of parameter variable
			Type *param_type = ParseType(); // force
			f->block->add_var(Exp.cur, param_type);
			Exp.next();
			f->num_params ++;

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
		if (peek_commands_super(Exp)){
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
		if ((Exp.cur_line->indent == 0) and (Exp.cur_line->exp.num >= 2)){
			if (Exp.cur == "class"){
				Exp.next();
				int nt0 = types.num;
				Type *t = CreateNewType(Exp.cur, 0, false, false, false, 0, NULL);
				if (nt0 == types.num)
					DoError("class already exists");
				t->fully_parsed = false;
			}
		}
		Exp.next_line();
	}
}

void SyntaxTree::ParseAllFunctionBodies()
{
	for (int i=0;i<functions.num;i++){
		Function *f = functions[i];
		if ((!f->is_extern) and (f->_logical_line_no >= 0))
			ParseFunctionBody(f);
	}
}

// convert text into script data
void SyntaxTree::Parser()
{
	msg_db_f("Parser", 4);

	root_of_all_evil.name = "RootOfAllEvil";
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


		/*if ((Exp.cur == "import") or (Exp.cur == "use")){
			ParseImport();

		// enum
		}else*/ if (Exp.cur == "enum"){
			ParseEnum();

		// class
		}else if (Exp.cur == "class"){
			ParseClass();

		}else{

			// type of definition
			bool is_function = false;
			for (int j=1;j<Exp.cur_line->exp.num-1;j++)
				if (Exp.cur_line->exp[j].name == "(")
				    is_function = true;

			// function?
			if (is_function){
				ParseFunctionHeader(NULL, next_extern);

			// global variables
			}else{
				ParseVariableDef(false, root_of_all_evil.block);
			}
		}
		if (!Exp.end_of_file())
			Exp.next_line();
	}

	ParseAllFunctionBodies();

	for (int i=0; i<types.num; i++)
		AutoImplementFunctions(types[i]);
}

}
