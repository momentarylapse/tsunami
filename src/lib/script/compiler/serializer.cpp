#include "../script.h"
#include "serializer.h"
#include "../../file/file.h"

namespace Script{

#define allow_simplification
#define allow_registers

extern void script_db_out(const string &str);
extern void script_db_out(int i);

#define so		script_db_out

//#ifdef ScriptDebug

bool UseConstAsGlobalVar = false;


extern int GlobalWaitingMode;
extern float GlobalTimeToWait;


#define StackMemAlign	8
#define FunctionAlign	8


inline bool is_reg_used_in_interval(Serializer *d, int reg, int first, int last);


static SerialCommandParam p_eax, p_eax_int, p_deref_eax, p_ax, p_al, p_ah, p_al_bool, p_al_char, p_st0, p_st1;
static const SerialCommandParam p_none = {-1, NULL, NULL, 0};


void Serializer::add_reg_channel(int reg, int first, int last)
{
	sRegChannel c = {reg, first, last};
	RegChannel.add(c);
}

void Serializer::add_temp(Type *t, SerialCommandParam &param)
{
	if (t != TypeVoid){
		sTempVar v;
		v.type = t;
		v.referenced = (t->is_super_array);
		v.entangled = 0;
		TempVar.add(v);
		param.kind = KindVarTemp;
		param.p = (char*)(TempVar.num - 1);
		param.type = t;
		param.shift = 0;
		

		if (t->element.num > 0)
			add_cmd_constructor(param, true);
	}else{
		param = p_none;
	}
}

inline Type *get_subtype(Type *t)
{
	if (t->parent)
		return t->parent;
	msg_error("subtype wanted of... " + t->name);
	//msg_write(cur_func->Name);
	return TypeUnknown;
}

inline void deref_temp(SerialCommandParam &param, SerialCommandParam &deref)
{
	so("deref temp");
	so((long)param.p);
	//deref = param;
	deref.kind = KindDerefVarTemp;
	deref.p = param.p;
	deref.type = get_subtype(param.type);
	deref.shift = 0;
	so((long)deref.p);
}

inline SerialCommandParam param_shift(SerialCommandParam &param, int shift)
{
	SerialCommandParam p = param;
	p.shift = shift;
	return p;
}

inline SerialCommandParam param_global(Type *type, void *v)
{
	SerialCommandParam p;
	p.type = type;
	p.kind = KindVarGlobal;
	p.p = (char*)v;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_local(Type *type, int offset)
{
	SerialCommandParam p;
	p.type = type;
	p.kind = KindVarLocal;
	p.p = (char*)offset;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_const(Type *type, void *c)
{
	SerialCommandParam p;
	p.type = type;
	p.kind = KindConstant;
	p.p = (char*)c;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_marker(int m)
{
	SerialCommandParam p;
	p.type = TypeInt;
	p.kind = KindMarker;
	p.p = (char*)m;
	p.shift = 0;
	return p;
}

inline SerialCommandParam param_reg(Type *type, int reg)
{
	SerialCommandParam p;
	p.kind = KindRegister;
	p.p = (char*)reg;
	p.type = type;
	p.shift = 0;
	return p;
}

inline void param_out(string &str, SerialCommandParam &p)
{
	//msg_db_f("param_out", 4);
	if (p.kind >= 0){
		str += format("   %s %p (%s)", Kind2Str(p.kind).c_str(), p.p, p.type->name.c_str());
		if (p.shift > 0)
			str += format(" + shift %d", p.shift);
	}
}

void cmd_out(int n, SerialCommand &c)
{
	//msg_db_f("cmd_out", 4);
	if (c.inst == inst_marker)
		so(format("%d: -- Marker %d --", n, c.p1.kind));
	else if (c.inst == inst_asm)
		so(format("%d: -- Asm --", n));
	else{
		string t = format("%3d:  ", n) + Asm::GetInstructionName(c.inst);
		param_out(t, c.p1);
		param_out(t, c.p2);
		so(t);
	}
}

void cmd_list_out()
{
#ifdef ScriptDebug
	msg_db_f("cmd_list_out", 4);
	so("--------------------------------");
	for (int i=0;i<cmd.num;i++)
		cmd_out(i, cmd[i]);
	so("-----------");
	for (int i=0;i<RegChannel.num;i++)
		so(format("  %d   %d -> %d", RegChannel[i].reg, RegChannel[i].first, RegChannel[i].last));
	so("-----------");
	if (TempVarRangesDefined)
		for (int i=0;i<TempVar.num;i++)
			so(format("  %d   %d -> %d", i, TempVar[i].first, TempVar[i].last));
	so("--------------------------------");
#endif
}

void Serializer::add_cmd(int inst, SerialCommandParam p1, SerialCommandParam p2)
{
	SerialCommand c;
	c.inst = inst;
	c.p1 = p1;
	c.p2 = p2;
	cmd.add(c);

	// call violates all used registers...
	if (inst == Asm::inst_call)
		for (int i=0;i<MapReg.num;i++)
			add_reg_channel(MapReg[i], cmd.num - 1, cmd.num - 1);
}

void Serializer::add_cmd(int inst, SerialCommandParam p)
{
	add_cmd(inst, p, p_none);
}

void Serializer::add_cmd(int inst)
{
	add_cmd(inst, p_none, p_none);
}

void Serializer::move_last_cmd(int index)
{
	SerialCommand c = cmd.back();
	for (int i=cmd.num-1;i>index;i--)
		cmd[i] = cmd[i - 1];
	cmd[index] = c;

	// adjust temp vars
	if (TempVarRangesDefined){
		foreach(sTempVar &v, TempVar){
			if (v.first >= index)
				v.first ++;
			if (v.last >= index)
				v.last ++;
		}
	}

	// adjust reg channels
	foreach(sRegChannel &r, RegChannel){
		if (r.first >= index)
			r.first ++;
		if (r.last >= index)
			r.last ++;
	}
}

void Serializer::remove_cmd(int index)
{
	cmd.erase(index);

	// adjust temp vars
	foreach(sTempVar &v, TempVar){
		if (v.first >= index)
			v.first --;
		if (v.last >= index)
			v.last --;
	}

	// adjust reg channels
	foreach(sRegChannel &r, RegChannel){
		if (r.first >= index)
			r.first --;
		if (r.last >= index)
			r.last --;
	}
}

void Serializer::remove_temp_var(int v)
{
	foreach(SerialCommand &c, cmd){
		if ((c.p1.kind == KindVarTemp) || (c.p1.kind == KindDerefVarTemp))
			if ((long)c.p1.p > v)
				c.p1.p = (char*)((long)c.p1.p - 1);
		if ((c.p2.kind == KindVarTemp) || (c.p2.kind == KindDerefVarTemp))
			if ((long)c.p2.p > v)
				c.p2.p = (char*)((long)c.p2.p - 1);
	}
	TempVar.erase(v);
}

void Serializer::move_param(SerialCommandParam &p, int from, int to)
{
	if ((p.kind == KindVarTemp) || (p.kind == KindDerefVarTemp)){
		so("move_param temp");
		long v = (long)p.p;
		if (TempVar[v].last < max(from, to))
			TempVar[v].last = max(from, to);
		if (TempVar[v].first > min(from, to))
			TempVar[v].first = min(from, to);
	}else if ((p.kind == KindRegister) || (p.kind == KindDerefRegister)){
		so("move_param reg");
		long r = (long)p.p;
		bool found = false;
		foreach(sRegChannel &rc, RegChannel)
			if ((r == rc.reg) && (from >= rc.first) && (from >= rc.first)){
				if (rc.last < max(from, to))
					rc.last = max(from, to);
				if (rc.first > min(from, to))
					rc.first = min(from, to);
				found = true;
			}
		if (!found)
			msg_error(format("move_param: kein RegChannel...  reg=%d  from=%d", r, from));
	}
}

void Serializer::add_marker(int m)
{
	SerialCommandParam p = p_none;
	if (m < 0)
		p.kind = NumMarkers ++;
	else
		p.kind = m;
	add_cmd(inst_marker, p);
}

int Serializer::add_marker_after_command(int level, int index)
{
	int n = NumMarkers ++;
	sStuffToAdd m = {StuffKindMarker, n, level, index};
	StuffToAdd.add(m);
	return n;
}

void Serializer::add_jump_after_command(int level, int index, int marker)
{
	sStuffToAdd j = {StuffKindJump, marker, level, index};
	StuffToAdd.add(j);
}

inline int reg_smallify(int reg)
{
	if (reg == Asm::RegEax)	return Asm::RegAl;
	if (reg == Asm::RegEcx)	return Asm::RegCl;
	if (reg == Asm::RegEdx)	return Asm::RegDl;
	if (reg == Asm::RegEbx)	return Asm::RegBl;
	msg_error("reg smallify");
	return -1;
}


static Array<SerialCommandParam> CompilerFunctionParam;
static SerialCommandParam CompilerFunctionReturn = {-1, NULL, NULL};
static SerialCommandParam CompilerFunctionInstance = {-1, NULL, NULL};

void AddFuncParam(SerialCommandParam &p)
{
	CompilerFunctionParam.add(p);
}

void AddFuncReturn(SerialCommandParam &r)
{
	CompilerFunctionReturn = r;
}

void AddFuncInstance(SerialCommandParam &inst)
{
	CompilerFunctionInstance = inst;
}

void AddReference(Serializer *d, SerialCommandParam &param, Type *type, SerialCommandParam &ret);

void Serializer::AddFunctionCall(void *func, int func_no)
{
	msg_db_f("AddFunctionCall", 4);
	call_used = true;
	Type *type = CompilerFunctionReturn.type;
	if (!type)
		type = TypeVoid;

	// return data too big... push address
	SerialCommandParam ret_temp, ret_ref;
	if ((type->size > 4) && (!type->is_array)){
		//add_temp(type, ret_temp);
		AddReference(/*ret_temp*/ CompilerFunctionReturn, TypePointer, ret_ref);
		//add_ref();
		//add_cmd(Asm::inst_lea, KindRegister, (char*)RegEaxCompilerFunctionReturn.kind, CompilerFunctionReturn.param);
	}

	// grow stack (down) for local variables of the calling function
//	add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	int push_size = 0;

	// push parameters onto stack
	for (int p=CompilerFunctionParam.num-1;p>=0;p--){
		if (CompilerFunctionParam[p].type){
			int s = mem_align(CompilerFunctionParam[p].type->size);
			for (int j=0;j<s/4;j++)
				add_cmd(Asm::inst_push, param_shift(CompilerFunctionParam[p], s - 4 - j * 4));
			push_size += s;
		}
	}

#ifdef NIX_IDE_VCS
	// more than 4 byte have to be returned -> give return address as very last parameter!
	if (type->Size > 4)
		add_cmd(Asm::inst_push, ret_ref; // nachtraegliche eSP-Korrektur macht die Funktion
#endif
	
	// _cdecl: Klassen-Instanz als ersten Parameter push'en
	if (CompilerFunctionInstance.type){
		add_cmd(Asm::inst_push, CompilerFunctionInstance);
		push_size += 4;
	}
	
#ifndef NIX_IDE_VCS
	// more than 4 byte have to be returned -> give return address as very first parameter!
	if (type->size > 4)
		add_cmd(Asm::inst_push, ret_ref); // nachtraegliche eSP-Korrektur macht die Funktion
#endif
	
	add_cmd(Asm::inst_call, param_const(TypePointer, func)); // the actual call
	// function pointer will be shifted later...

	//dp += 8; // esp, eip
	if (push_size > 127)
		add_cmd(Asm::inst_add, param_reg(TypePointer, Asm::RegEsp), param_const(TypeInt, (void*)push_size));
	else if (push_size > 0)
		add_cmd(Asm::inst_add_b, param_reg(TypePointer, Asm::RegEsp), param_const(TypeInt, (void*)push_size));

	// return > 4b already got copied to [ret] by the function!
	if (type != TypeVoid)
		if (type->size <= 4){
			if (type == TypeFloat)
				add_cmd(Asm::inst_fstp, CompilerFunctionReturn);
			else if (type->size == 1){
				add_cmd(Asm::inst_mov_b, CompilerFunctionReturn, param_reg(type, Asm::RegAl));
				add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
			}else{
				add_cmd(Asm::inst_mov, CompilerFunctionReturn, param_reg(type, Asm::RegEax));
				add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
			}
		}

	// clean up for next call
	CompilerFunctionParam.clear();
	CompilerFunctionReturn.type = NULL;
	CompilerFunctionInstance.type = NULL;
}


// creates res...
void Serializer::AddReference(SerialCommandParam &param, Type *type, SerialCommandParam &ret)
{
	msg_db_f("AddReference", 3);
	so(Kind2Str(param.kind));
	ret.type = type;
	ret.shift = 0;
	if (param.kind == KindRefToConst){
		so("  Reference-RefConst");
		ret.kind = KindConstant;
		ret.p = param.p;
	}else if ((param.kind == KindConstant) || (param.kind == KindVarGlobal)){
		so("  Reference-Const");
		ret.kind = KindConstant;
		ret.p = param.p;
	}else if (param.kind == KindDerefVarTemp){
		ret = param;
		param.kind = KindVarTemp;
	}else{
		add_temp(type, ret);
		add_cmd(Asm::inst_lea, param_reg(type, Asm::RegEax), param);
		add_cmd(Asm::inst_mov, ret, param_reg(type, Asm::RegEax));
		add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
	}
}

void Serializer::AddDereference(SerialCommandParam &param, SerialCommandParam &ret, Type *force_type)
{
	msg_db_f("AddDereference", 4);
	/*add_temp(TypePointer, ret);
	SerialCommandParam temp;
	add_temp(TypePointer, temp);
	add_cmd(Asm::inst_mov, temp, param);
	temp.kind = KindDerefVarTemp;
	add_cmd(Asm::inst_mov, ret, temp);*/
	if (param.kind == KindVarTemp){
		deref_temp(param, ret);
	}else if (param.kind == KindRegister){
		ret = param;
		ret.kind = KindDerefRegister;
		ret.type = force_type ? force_type : get_subtype(param.type);
		ret.shift = 0;
	}else{
		//msg_error(string("unhandled deref ", Kind2Str(param.kind)));
		SerialCommandParam temp;
		add_temp(param.type, temp);
		add_cmd(Asm::inst_mov, temp, param);
		deref_temp(temp, ret);
	}
}

// create data for a (function) parameter
//   and compile its command if the parameter is executable itself
void Serializer::SerializeParameter(Command *link, int level, int index, SerialCommandParam &p)
{
	msg_db_f("SerializeParameter", 4);
	p.kind = link->kind;
	p.type = link->type;
	p.p = NULL;
	p.shift = 0;
	//Type *rt=link->;
	//so(Kind2Str(link->Kind));
	if (link->kind == KindVarFunction){
		so(" -var-func");
		if (pre_script->FlagCompileOS)
			p.p = (char*)((long)script->func[link->link_nr] - (long)&script->Opcode[0] + (pre_script->AsmMetaInfo)->CodeOrigin);
		else
			p.p = (char*)script->func[link->link_nr];
		p.kind = KindVarGlobal;
	}else if (link->kind == KindMemory){
		so(" -mem");
		p.p = (char*)link->link_nr;
		p.kind = KindVarGlobal;
	}else if (link->kind == KindAddress){
		so(" -addr");
		p.p = (char*)&link->link_nr;
		p.kind = KindRefToConst;
	}else if (link->kind == KindVarGlobal){
		so(" -global");
		if (link->script)
			p.p = link->script->g_var[link->link_nr];
		else
			p.p = script->g_var[link->link_nr];
	}else if (link->kind == KindVarLocal){
		so(" -local");
		p.p = (char*)(long)cur_func->var[link->link_nr]._offset;
	}else if (link->kind == KindLocalMemory){
		so(" -local mem");
		p.p = (char*)link->link_nr;
		p.kind = KindVarLocal;
	}else if (link->kind == KindLocalAddress){
		so(" -local addr");
		SerialCommandParam param;
		param.p = (char*)link->link_nr;
		param.kind = KindVarLocal;
		param.type = TypePointer;
		param.shift = 0;

		AddReference(param, link->type, p);
	}else if (link->kind == KindVarExternal){
		so(" -external-var");
		p.p = (char*)PreExternalVars[link->link_nr].pointer;
		p.kind = KindVarGlobal;
		if (!p.p)
			script->DoErrorLink(format("external variable is not linkable: %s",PreExternalVars[link->link_nr].name.c_str()));
	}else if (link->kind == KindConstant){
		so(" -const");
		if ((UseConstAsGlobalVar) || (pre_script->FlagCompileOS))
			p.kind = KindVarGlobal;
		else
			p.kind = KindRefToConst;
		p.p = script->cnst[link->link_nr];
	}else if ((link->kind==KindOperator) || (link->kind==KindFunction) || (link->kind==KindCompilerFunction)){
		p = SerializeCommand(link, level, index);
	}else if (link->kind == KindReference){
		//so(Kind2Str(link->Meta->Kind));
		so(" -ref");
		SerialCommandParam param;
		SerializeParameter(link->param[0], level, index, param);
		//printf("%d  -  %s\n",pk,Kind2Str(pk));
		AddReference(param, link->type, p);
	}else if (link->kind == KindDereference){
		so(" -deref...");
		SerialCommandParam param;
		SerializeParameter(link->param[0], level, index, param);
		/*if ((param.kind == KindVarLocal) || (param.kind == KindVarGlobal)){
			p.type = param.type->sub_type;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		AddDereference(param, p);
	}else{
		DoError("unexpected type of parameter: " + Kind2Str(link->kind));
	}
}


void Serializer::SerializeOperator(Command *com, SerialCommandParam *param, SerialCommandParam &ret)
{
	msg_db_f("SerializeOperator", 4);
	switch(com->link_nr){
		case OperatorIntAssign:
		case OperatorFloatAssign:
		case OperatorPointerAssign:
			add_cmd(Asm::inst_mov, param[0], param[1]);
			break;
		case OperatorCharAssign:
		case OperatorBoolAssign:
			add_cmd(Asm::inst_mov_b, param[0], param[1]);
			break;
		case OperatorClassAssign:
			for (int i=0;i<signed(com->param[0]->type->size)/4;i++)
				add_cmd(Asm::inst_mov, param_shift(param[0], i * 4), param_shift(param[1], i * 4));
			for (int i=4*signed(com->param[0]->type->size/4);i<signed(com->param[0]->type->size);i++)
				add_cmd(Asm::inst_mov, param_shift(param[0], i), param_shift(param[1], i));
			break;
// string   TODO: create own code!
		case OperatorCStringAssignAA:
			AddFuncParam(param[0]);
			AddFuncParam(param[1]);
			AddFunctionCall((void*)&strcpy);
			break;
		case OperatorCStringAddAAS:
			AddFuncParam(param[0]);
			AddFuncParam(param[1]);
			AddFunctionCall((void*)&strcat);
			break;
		case OperatorCStringAddAA:{
			SerialCommandParam ret_ref;
			AddReference(ret, TypePointer, ret_ref);
			AddFuncParam(ret_ref);
			AddFuncParam(param[0]);
			AddFunctionCall((void*)&strcpy);
			AddFuncParam(ret_ref);
			AddFuncParam(param[1]);
			AddFunctionCall((void*)&strcat);
			}break;
		case OperatorCStringEqualAA:
		case OperatorCStringNotEqualAA:
			AddFuncParam(param[0]);
			AddFuncParam(param[1]);
			AddFunctionCall((void*)&strcmp); // well... return in eax...

			add_cmd(Asm::inst_cmp, p_eax_int, param_const(TypeInt, (void*)0x0));
			if (com->link_nr == OperatorCStringEqualAA)
				add_cmd(Asm::inst_setz_b, p_al_bool);
			if (com->link_nr==OperatorCStringNotEqualAA)
				add_cmd(Asm::inst_setnz_b, p_al_bool);
			add_cmd(Asm::inst_mov_b, ret, p_al_bool);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
			break;
// int
		case OperatorIntAddS:
			add_cmd(Asm::inst_add, param[0], param[1]);
			break;
		case OperatorIntSubtractS:
			add_cmd(Asm::inst_sub, param[0], param[1]);
			break;
		case OperatorIntMultiplyS:
			add_cmd(Asm::inst_imul, param[0], param[1]);
			break;
		case OperatorIntDivideS:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEdx), p_eax_int);
			add_cmd(Asm::inst_sar, param_reg(TypeInt, Asm::RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(Asm::inst_idiv, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, param[0], p_eax_int);
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 1);
			add_reg_channel(Asm::RegEdx, cmd.num - 2, cmd.num - 2);
			break;
		case OperatorIntAdd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_add, ret, param[1]);
			break;
		case OperatorIntSubtract:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_sub, ret, param[1]);
			break;
		case OperatorIntMultiply:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_imul, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, ret, p_eax_int);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
			break;
		case OperatorIntDivide:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEdx), p_eax_int);
			add_cmd(Asm::inst_sar, param_reg(TypeInt, Asm::RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(Asm::inst_idiv, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, ret, p_eax_int);
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 1);
			add_reg_channel(Asm::RegEdx, cmd.num - 2, cmd.num - 2);
			break;
		case OperatorIntModulo:
			add_cmd(Asm::inst_mov, p_eax_int, param[0]);
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEdx), p_eax_int);
			add_cmd(Asm::inst_sar, param_reg(TypeInt, Asm::RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(Asm::inst_idiv, p_eax_int, param[1]);
			add_cmd(Asm::inst_mov, ret, param_reg(TypeInt, Asm::RegEdx));
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 2);
			add_reg_channel(Asm::RegEdx, cmd.num - 2, cmd.num - 1);
			break;
		case OperatorIntEqual:
		case OperatorIntNotEqual:
		case OperatorIntGreater:
		case OperatorIntGreaterEqual:
		case OperatorIntSmaller:
		case OperatorIntSmallerEqual:
		case OperatorPointerEqual:
		case OperatorPointerNotEqual:
			add_cmd(Asm::inst_cmp, param[0], param[1]);
			if (com->link_nr==OperatorIntEqual)			add_cmd(Asm::inst_setz_b, ret);
			if (com->link_nr==OperatorIntNotEqual)		add_cmd(Asm::inst_setnz_b, ret);
			if (com->link_nr==OperatorIntGreater)		add_cmd(Asm::inst_setnle_b, ret);
			if (com->link_nr==OperatorIntGreaterEqual)	add_cmd(Asm::inst_setnl_b, ret);
			if (com->link_nr==OperatorIntSmaller)		add_cmd(Asm::inst_setl_b, ret);
			if (com->link_nr==OperatorIntSmallerEqual)	add_cmd(Asm::inst_setle_b, ret);
			if (com->link_nr==OperatorPointerEqual)		add_cmd(Asm::inst_setz_b, ret);
			if (com->link_nr==OperatorPointerNotEqual)	add_cmd(Asm::inst_setnz_b, ret);
			break;
		case OperatorIntBitAnd:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_and, ret, param[1]);
			break;
		case OperatorIntBitOr:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_or, ret, param[1]);
			break;
		case OperatorIntShiftRight:
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEcx), param[1]);
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_shr, ret, param_reg(TypeChar, Asm::RegCl));
			add_reg_channel(Asm::RegEcx, cmd.num - 3, cmd.num - 1);
			break;
		case OperatorIntShiftLeft:
			add_cmd(Asm::inst_mov, param_reg(TypeInt, Asm::RegEcx), param[1]);
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_shl, ret, param_reg(TypeChar, Asm::RegCl));
			add_reg_channel(Asm::RegEcx, cmd.num - 3, cmd.num - 1);
			break;
		case OperatorIntNegate:
			add_cmd(Asm::inst_mov, ret, param_const(TypeInt, (void*)0x0));
			add_cmd(Asm::inst_sub, ret, param[0]);
			break;
		case OperatorIntIncrease:
			add_cmd(Asm::inst_add, param[0], param_const(TypeInt, (void*)0x1));
			break;
		case OperatorIntDecrease:
			add_cmd(Asm::inst_sub, param[0], param_const(TypeInt, (void*)0x1));
			break;
// float
		case OperatorFloatAddS:
		case OperatorFloatSubtractS:
		case OperatorFloatMultiplyS:
		case OperatorFloatDivideS:
			add_cmd(Asm::inst_fld, param[0]);
			if (com->link_nr==OperatorFloatAddS)			add_cmd(Asm::inst_fadd, param[1]);
			if (com->link_nr==OperatorFloatSubtractS)	add_cmd(Asm::inst_fsub, param[1]);
			if (com->link_nr==OperatorFloatMultiplyS)	add_cmd(Asm::inst_fmul, param[1]);
			if (com->link_nr==OperatorFloatDivideS)		add_cmd(Asm::inst_fdiv, param[1]);
			add_cmd(Asm::inst_fstp, param[0]);
			break;
		case OperatorFloatAdd:
		case OperatorFloatSubtract:
		case OperatorFloatMultiply:
		case OperatorFloatDivide:
			add_cmd(Asm::inst_fld, param[0]);
			if (com->link_nr==OperatorFloatAdd)			add_cmd(Asm::inst_fadd, param[1]);
			if (com->link_nr==OperatorFloatSubtract)		add_cmd(Asm::inst_fsub, param[1]);
			if (com->link_nr==OperatorFloatMultiply)		add_cmd(Asm::inst_fmul, param[1]);
			if (com->link_nr==OperatorFloatDivide)		add_cmd(Asm::inst_fdiv, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			break;
		case OperatorFloatMultiplyFI:
			add_cmd(Asm::inst_fild, param[1]);
			add_cmd(Asm::inst_fmul, param[0]);
			add_cmd(Asm::inst_fstp, ret);
			break;
		case OperatorFloatMultiplyIF:
			add_cmd(Asm::inst_fild, param[0]);
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			break;
		case OperatorFloatEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 4, cmd.num - 2);
			break;
		case OperatorFloatNotEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setnz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 4, cmd.num - 2);
			break;
		case OperatorFloatGreater:
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_setz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatGreaterEqual:
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test_b, p_ah, param_const(TypeChar, (void*)0x05));
			add_cmd(Asm::inst_setz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatSmaller:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_setz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatSmallerEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_test_b, p_ah, param_const(TypeChar, (void*)0x05));
			add_cmd(Asm::inst_setz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 2);
			break;
		case OperatorFloatNegate:
			add_cmd(Asm::inst_mov, ret, param[0]);
			add_cmd(Asm::inst_xor, ret, param_const(TypeInt, (void*)0x80000000));
			break;
// complex
		case OperatorComplexAddS:
		case OperatorComplexSubtractS:
		//case OperatorComplexMultiplySCF:
		//case OperatorComplexDivideS:
			add_cmd(Asm::inst_fld, param[0]);
			if (com->link_nr == OperatorComplexAddS)			add_cmd(Asm::inst_fadd, param[1]);
			if (com->link_nr == OperatorComplexSubtractS)	add_cmd(Asm::inst_fsub, param[1]);
			add_cmd(Asm::inst_fstp, param[0]);
			add_cmd(Asm::inst_fld, param_shift(param[0], 4));
			if (com->link_nr == OperatorComplexAddS)			add_cmd(Asm::inst_fadd, param_shift(param[1], 4));
			if (com->link_nr == OperatorComplexSubtractS)	add_cmd(Asm::inst_fsub, param_shift(param[1], 4));
			add_cmd(Asm::inst_fstp, param_shift(param[0], 4));
			break;
		case OperatorComplexAdd:
		case OperatorComplexSubtract:
//		case OperatorFloatMultiply:
//		case OperatorFloatDivide:
			add_cmd(Asm::inst_fld, param[0]);
			if (com->link_nr == OperatorComplexAdd)		add_cmd(Asm::inst_fadd, param[1]);
			if (com->link_nr == OperatorComplexSubtract)	add_cmd(Asm::inst_fsub, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			add_cmd(Asm::inst_fld, param_shift(param[0], 4));
			if (com->link_nr == OperatorComplexAdd)		add_cmd(Asm::inst_fadd, param_shift(param[1], 4));
			if (com->link_nr == OperatorComplexSubtract)	add_cmd(Asm::inst_fsub, param_shift(param[1], 4));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexMultiply:
			// r.x = a.y * b.y
			add_cmd(Asm::inst_fld, param_shift(param[0], 4));
			add_cmd(Asm::inst_fmul, param_shift(param[1], 4));
			add_cmd(Asm::inst_fstp, ret);
			// r.x = a.x * b.x - r.x
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fsub, ret);
			add_cmd(Asm::inst_fstp, ret);
			// r.y = a.y * b.x
			add_cmd(Asm::inst_fld, param_shift(param[0], 4));
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, param_shift(ret, 4));
			// r.y += a.x * b.y
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param_shift(param[1], 4));
			add_cmd(Asm::inst_fadd, param_shift(ret, 4));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexMultiplyFC:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param_shift(param[1], 4));
			add_cmd(Asm::inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexMultiplyCF:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, ret);
			add_cmd(Asm::inst_fld, param_shift(param[0], 4));
			add_cmd(Asm::inst_fmul, param[1]);
			add_cmd(Asm::inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexEqual:
			add_cmd(Asm::inst_fld, param[0]);
			add_cmd(Asm::inst_fld, param[1]);
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setz_b, ret);
			add_reg_channel(Asm::RegEax, cmd.num - 4, cmd.num - 2);
			add_cmd(Asm::inst_fld, param_shift(param[0], 4));
			add_cmd(Asm::inst_fld, param_shift(param[1], 4));
			add_cmd(Asm::inst_fucompp, p_st0, p_st1);
			add_cmd(Asm::inst_fnstsw, p_ax);
			add_cmd(Asm::inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(Asm::inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(Asm::inst_setz_b, p_ah);
			add_cmd(Asm::inst_and_b, ret, p_ah);
			add_reg_channel(Asm::RegEax, cmd.num - 5, cmd.num - 1);
			break;
// bool/char
		case OperatorCharEqual:
		case OperatorCharNotEqual:
		case OperatorBoolEqual:
		case OperatorBoolNotEqual:
		case OperatorBoolGreater:
		case OperatorBoolGreaterEqual:
		case OperatorBoolSmaller:
		case OperatorBoolSmallerEqual:
			add_cmd(Asm::inst_cmp_b, param[0], param[1]);
			if ((com->link_nr == OperatorCharEqual) || (com->link_nr == OperatorBoolEqual))
				add_cmd(Asm::inst_setz_b, ret);
			else if ((com->link_nr ==OperatorCharNotEqual) || (com->link_nr == OperatorBoolNotEqual))
				add_cmd(Asm::inst_setnz_b, ret);
			else if (com->link_nr == OperatorBoolGreater)		add_cmd(Asm::inst_setnle_b, ret);
			else if (com->link_nr == OperatorBoolGreaterEqual)	add_cmd(Asm::inst_setnl_b, ret);
			else if (com->link_nr == OperatorBoolSmaller)		add_cmd(Asm::inst_setl_b, ret);
			else if (com->link_nr == OperatorBoolSmallerEqual)	add_cmd(Asm::inst_setle_b, ret);
			break;
		case OperatorBoolAnd:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_and_b, ret, param[1]);
			break;
		case OperatorBoolOr:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_or_b, ret, param[1]);
			break;
		case OperatorCharAddS:
			add_cmd(Asm::inst_add_b, param[0], param[1]);
			break;
		case OperatorCharSubtractS:
			add_cmd(Asm::inst_sub_b, param[0], param[1]);
			break;
		case OperatorCharAdd:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_add_b, ret, param[1]);
			break;
		case OperatorCharSubtract:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_sub_b, ret, param[1]);
			break;
		case OperatorCharBitAnd:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_and_b, ret, param[1]);
			break;
		case OperatorCharBitOr:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_or_b, ret, param[1]);
			break;
		case OperatorBoolNegate:
			add_cmd(Asm::inst_mov_b, ret, param[0]);
			add_cmd(Asm::inst_xor_b, ret, param_const(TypeBool, (char*)0x1));
			break;
		case OperatorCharNegate:
			add_cmd(Asm::inst_mov_b, ret, param_const(TypeChar, (char*)0x0));
			add_cmd(Asm::inst_sub_b, ret, param[0]);
			break;
// vector
		case OperatorVectorAddS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fadd, param_shift(param[1], i * 4));
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorMultiplyS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fmul, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorDivideS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fdiv, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorSubtractS:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fsub, param_shift(param[1], i * 4));
				add_cmd(Asm::inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorAdd:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fadd, param_shift(param[1], i * 4));
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorSubtract:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fsub, param_shift(param[1], i * 4));
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorMultiplyVF:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fmul, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorMultiplyFV:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param[0]);
				add_cmd(Asm::inst_fmul, param_shift(param[1], i * 4));
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorDivideVF:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_fld, param_shift(param[0], i * 4));
				add_cmd(Asm::inst_fdiv, param[1]);
				add_cmd(Asm::inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorNegate:
			for (int i=0;i<3;i++){
				add_cmd(Asm::inst_mov, param_shift(ret, i * 4), param_shift(param[0], i * 4));
				add_cmd(Asm::inst_xor, param_shift(ret, i * 4), param_const(TypeInt, (void*)0x80000000));
			}
			break;
		default:
			DoError("unimplemented operator: " + Operator2Str(pre_script, com->link_nr));
	}
}


SerialCommandParam Serializer::SerializeCommand(Command *com, int level, int index)
{
	msg_db_f("SerializeCommand", 4);
	//so(Kind2Str(com->Kind));
	SerialCommandParam param[SCRIPT_MAX_PARAMS];
	SerialCommandParam ret;// = (char*)( -add_temp_var(s) - cur_func->_VarSize);
	add_temp(com->type, ret);
	//so(d2h((char*)&ret,4,false));
	//so(string2("return: %d/%d/%d", com->type->size, LocalOffset, LocalOffset));

	// compile parameters
	for (int p=0;p<com->num_params;p++)
		SerializeParameter(com->param[p], level, index, param[p]);

	// class function -> compile instance
	bool is_class_function = false;
	if (com->kind == KindCompilerFunction)
		if (PreCommands[com->link_nr].is_class_function)
			is_class_function = true;
	if (com->kind == KindFunction){
		if (com->script){
			if (com->script->pre_script->Functions[com->link_nr]->_class)
				is_class_function = true;
		}else{
			if (pre_script->Functions[com->link_nr]->_class)
				is_class_function = true;
		}
	}
	SerialCommandParam instance = {-1, NULL, NULL};
	if (is_class_function){
		so("member");
		SerializeParameter(com->instance, level, index, instance);
		so(Kind2Str(instance.kind));
		// super_array automatically referenced...
	}

	    
	if (com->kind == KindOperator){
		//so("---operator");
		SerializeOperator(com, param, ret);
		
	}else if ((com->kind == KindCompilerFunction) || (com->kind == KindFunction)){
		//so("---func");
		void *fp = NULL;
		string name;
		if (com->kind == KindFunction){ // own script Function
			so("Funktion!!!");
			if (com->script){
				//so(com->link_nr);
				so("    extern!!!");
				fp = (void*)com->script->func[com->link_nr];
			}else{
				fp = (void*)script->func[com->link_nr];
			}
			so("   -ok");
		}else{ // compiler function
			fp = PreCommands[com->link_nr].func;
			name = PreCommands[com->link_nr].name;
		}
		if (fp){ // a real function

			for (int p=0;p<com->num_params;p++)
				AddFuncParam(param[p]);
			
			AddFuncReturn(ret);
			
			if (is_class_function)
				AddFuncInstance(instance);
			
			AddFunctionCall(fp);
			
		}else if (PreCommands[com->link_nr].is_special){
			switch(com->link_nr){
				/*case CommandSine:
					break;*/
				case CommandIf:{
					// cmp;  jz m;  -block-  m;
					add_cmd(Asm::inst_cmp_b, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_true = add_marker_after_command(level, index + 1);
					add_cmd(Asm::inst_jz, param_marker(m_after_true));
					}break;
				case CommandIfElse:{
					// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
					add_cmd(Asm::inst_cmp_b, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_true = add_marker_after_command(level, index + 1);
					int m_after_false = add_marker_after_command(level, index + 2);
					add_cmd(Asm::inst_jz, param_marker(m_after_true)); // jz_b ...
					add_jump_after_command(level, index + 1, m_after_false); // insert before <m_after_true> is inserted!
					}break;
				case CommandWhile:
				case CommandFor:{
					// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
					// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
					int m_before_while = NumMarkers ++;
					add_marker(m_before_while);
					move_last_cmd(LastCommandSize); // has to be before evaluating the parameter!
					add_cmd(Asm::inst_cmp_b, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_while = add_marker_after_command(level, index + 1);
					add_cmd(Asm::inst_jz, param_marker(m_after_while));
					add_jump_after_command(level, index + 1, m_before_while); // insert before <m_after_while> is inserted!

					int m_continue = m_before_while;
					if (com->link_nr == CommandFor){
						// NextCommand is a block!
						if (NextCommand->kind != KindBlock)
							DoError("command block in \"for\" loop missing");
						m_continue = add_marker_after_command(level + 1, pre_script->Blocks[NextCommand->link_nr]->command.num - 2);
					}
					sLoopData l = {m_continue, m_after_while, level, index};
					LoopData.add(l);
					}break;
				case CommandBreak:
					add_cmd(Asm::inst_jmp, param_marker(LoopData.back().marker_break));
					break;
				case CommandContinue:
					add_cmd(Asm::inst_jmp, param_marker(LoopData.back().marker_continue));
					break;
				case CommandReturn:
					if (com->num_params > 0){
						if (cur_func->return_type->size > 4){ // we already got a return address in [ebp+0x08] (> 4 byte)
							int s = mem_align(cur_func->return_type->size);

							// slow
							/*SerialCommandParam p, p_deref;
							p.kind = KindVarLocal;
							p.type = TypeReg32;
							p.p = (char*) 0x8;
							p.shift = 0;
							for (int j=0;j<s/4;j++){
								AddDereference(p, p_deref);
								add_cmd(Asm::inst_mov, p_deref, param_shift(param[0], j * 4));
								add_cmd(Asm::inst_add, p, param_const(TypeInt, (void*)0x4));
							}*/

							// test
							SerialCommandParam p_edx = param_reg(TypeReg32, Asm::RegEdx), p_deref_edx;
							SerialCommandParam p_ret_addr;
							p_ret_addr.kind = KindVarLocal;
							p_ret_addr.type = TypeReg32;
							p_ret_addr.p = (char*)0x8;
							p_ret_addr.shift = 0;
							int c_0 = cmd.num;
							add_cmd(Asm::inst_mov, p_edx, p_ret_addr);
							AddDereference(p_edx, p_deref_edx, TypeReg32);
							for (int j=0;j<s/4;j++)
								add_cmd(Asm::inst_mov, param_shift(p_deref_edx, j * 4), param_shift(param[0], j * 4));
							add_reg_channel(Asm::RegEdx, c_0, cmd.num - 1);
						}else{ // store return directly in eax / fpu stack (4 byte)
							if (cur_func->return_type == TypeFloat)
								add_cmd(Asm::inst_fld, param[0]);
							else if (cur_func->return_type->size == 1)
								add_cmd(Asm::inst_mov_b, param_reg(cur_func->return_type, Asm::RegAl), param[0]);
							else
								add_cmd(Asm::inst_mov, param_reg(cur_func->return_type, Asm::RegEax), param[0]);
						}
					}
					add_cmd(Asm::inst_leave);
					if (cur_func->return_type->size > 4)
						add_cmd(Asm::inst_ret, param_const(TypeInt, (void*)4));
					else
						add_cmd(Asm::inst_ret);
					break;
				case CommandWaitOneFrame:
				case CommandWait:
				case CommandWaitRT:{
				// set waiting state
					// GlobalWaitingMode = mode
					// GlobalWaitingTime = time
					SerialCommandParam p_mode = param_global(TypeInt, &GlobalWaitingMode);
					SerialCommandParam p_ttw = param_global(TypeFloat, &GlobalTimeToWait);
					if (com->link_nr == CommandWaitOneFrame){
						add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeRT));
						add_cmd(Asm::inst_mov, p_ttw, param_const(TypeFloat, NULL));
					}else if (com->link_nr == CommandWait){
						add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeGT));
						add_cmd(Asm::inst_mov, p_ttw, param[0]);
					}else if (com->link_nr == CommandWaitRT){
						add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeRT));
						add_cmd(Asm::inst_mov, p_ttw, param[0]);
					}
					
				// save script state
					// stack[ -8] = ebp
					// stack[-12] = esp
					// stack[-16] = eip
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[StackSize-8]));
					add_cmd(Asm::inst_mov, p_deref_eax, param_reg(TypeReg32, Asm::RegEbp));
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[StackSize-12]));
					add_cmd(Asm::inst_mov, p_deref_eax, param_reg(TypeReg32, Asm::RegEsp));
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEsp), param_const(TypePointer, &script->Stack[StackSize-12]));
					add_cmd(Asm::inst_call, param_const(TypePointer, NULL)); // push eip
				// load return
					// mov esp, &stack[-4]
					// pop esp
					// mov ebp, esp
					// leave
					// ret
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEsp), param_const(TypePointer, &script->Stack[StackSize-4])); // start of the script stack
					add_cmd(Asm::inst_pop, param_reg(TypeReg32, Asm::RegEsp)); // old stackpointer (real program)
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEbp), param_reg(TypeReg32, Asm::RegEsp));
					add_cmd(Asm::inst_leave);
					add_cmd(Asm::inst_ret);
				// here comes the "waiting"...

				// reload script state (eip already loaded)
					// ebp = &stack[-8]
					// esp = &stack[-12]
					// GlobalWaitingMode = WaitingModeNone
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[StackSize-8]));
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEbp), p_deref_eax);
					add_cmd(Asm::inst_mov, p_eax, param_const(TypePointer, &script->Stack[StackSize-12]));
					add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEsp), p_deref_eax);
					add_cmd(Asm::inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeNone));
					}break;
				case CommandIntToFloat:
					add_cmd(Asm::inst_fild, param[0]);
					add_cmd(Asm::inst_fstp, ret);
					break;
				case CommandFloatToInt:
					// round to nearest...
					//add_cmd(Asm::inst_fld, param[0]);
					//add_cmd(Asm::inst_fistp, ret);

					// round to zero...
					SerialCommandParam t1, t2;
					add_temp(TypeInt, t1);
					add_temp(TypeInt, t2);
					add_cmd(Asm::inst_fld, param[0]);
					add_cmd(Asm::inst_fnstcw, t1);
					add_cmd(Asm::inst_movzx, p_eax, t1);
					add_cmd(Asm::inst_mov_b, p_ah, param_const(TypeChar, (void*)0x0c));
					add_cmd(Asm::inst_mov, t2, p_eax);
					add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
					add_cmd(Asm::inst_fldcw, t2);
					add_cmd(Asm::inst_fistp, ret);
					add_cmd(Asm::inst_fldcw, t1);
					break;
				case CommandIntToChar:
					add_cmd(Asm::inst_mov, p_eax_int, param[0]);
					add_cmd(Asm::inst_mov_b, ret, p_al_char);
					add_reg_channel(Asm::RegEax, cmd.num - 2, cmd.num - 1);
					break;
				case CommandCharToInt:
					add_cmd(Asm::inst_mov, p_eax_int, param_const(TypeInt, (void*)0x0));
					add_cmd(Asm::inst_mov_b, p_al_char, param[0]);
					add_cmd(Asm::inst_mov, ret, p_eax);
					add_reg_channel(Asm::RegEax, cmd.num - 3, cmd.num - 1);
					break;
				case CommandPointerToBool:
					add_cmd(Asm::inst_cmp, param[0], param_const(TypePointer, NULL));
					add_cmd(Asm::inst_setnz_b, ret);
					break;
				case CommandAsm:
					add_cmd(inst_asm);
					break;
				case CommandRectSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 12), param[3]);
				case CommandVectorSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 8), param[2]);
				case CommandComplexSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 4), param[1]);
					add_cmd(Asm::inst_mov, param_shift(ret, 0), param[0]);
					break;
				case CommandColorSet:
					add_cmd(Asm::inst_mov, param_shift(ret, 12), param[0]);
					add_cmd(Asm::inst_mov, param_shift(ret, 0), param[1]);
					add_cmd(Asm::inst_mov, param_shift(ret, 4), param[2]);
					add_cmd(Asm::inst_mov, param_shift(ret, 8), param[3]);
					break;
				default:
					DoError("compiler function unimplemented: " + PreCommands[com->link_nr].name);
			}
		}else{
			if (PreCommands[com->link_nr].is_semi_external)
	 			DoErrorLink("external function not linkable: " + PreCommands[com->link_nr].name);
			else
	 			DoError("compiler function not linkable: " + PreCommands[com->link_nr].name);
		}
	}else if (com->kind == KindBlock){
		//so("---block");
		SerializeBlock(pre_script->Blocks[com->link_nr], level + 1);
	}else{
		//so("---???");
		//DoError(string("type of command is unimplemented (call Michi!): ",Kind2Str(com->Kind)));
	}
	//so(Kind2Str(com->Kind));
	//msg_ok();
	return ret;
}

void Serializer::SerializeBlock(Block *block, int level)
{
	msg_db_f("SerializeBlock", 4);
	for (int i=0;i<block->command.num;i++){
		//so(string2("%d - %d",i,block->NumCommands));
		StackOffset = cur_func->_var_size;
		LastCommandSize = cmd.num;
		if (block->command.num > i + 1)
			NextCommand = block->command[i + 1];

		// serialize
		SerializeCommand(block->command[i], level, i);
		
		FillInDestructors(true);

		// any markers / jumps to add?
		for (int j=StuffToAdd.num-1;j>=0;j--)
			if ((level == StuffToAdd[j].level) && (i == StuffToAdd[j].index)){
				if (StuffToAdd[j].kind == StuffKindMarker)
					add_marker(StuffToAdd[j].marker);
				else if (StuffToAdd[j].kind == StuffKindJump)
					add_cmd(Asm::inst_jmp, param_marker(StuffToAdd[j].marker));
				StuffToAdd.erase(j);
			}

		// end of loop?
		if (LoopData.num > 0)
			if ((LoopData.back().level == level) && (LoopData.back().index == i - 1))
				LoopData.pop();
	}
}

Array<SerialCommandParam> InsertedConstructorFunc;
Array<SerialCommandParam> InsertedConstructorTemp;

void Serializer::add_cmd_constructor(SerialCommandParam &param, bool is_temp)
{
	foreach(ClassFunction &f, param.type->function){
		if (f.name == "__init__"){ // TODO test signature "void __init__()"
			SerialCommandParam inst;
			AddReference(param, TypePointer, inst);
			AddFuncInstance(inst);
			void *fp;
			if (f.kind == KindCompilerFunction)
				fp = PreCommands[f.nr].func;
			else if (f.kind == KindFunction){
				fp = (void*)param.type->owner->script->func[f.nr];
				if (!fp)
					DoErrorLink(param.type->name + ".__init__() unlinkable compiler function!");
			}

			AddFunctionCall(fp);
			if (is_temp)
				InsertedConstructorTemp.add(param);
			else
				InsertedConstructorFunc.add(param);
			return;
		}
	}
}

void Serializer::add_cmd_destructor(SerialCommandParam &param)
{
	foreach(ClassFunction &f, param.type->function)
		if (f.name == "__delete__"){ // TODO test signature "void __delete__()"
			SerialCommandParam inst;
			AddReference(param, TypePointer, inst);
			AddFuncInstance(inst);
			void *fp;
			if (f.kind == KindCompilerFunction)
				fp = PreCommands[f.nr].func;
			else if (f.kind == KindFunction){
				fp = (void*)param.type->owner->script->func[f.nr];
				if (!fp)
					DoErrorLink(param.type->name + ".__delete__() unlinkable compiler function!");
			}
			AddFunctionCall(fp);
		}
}

void Serializer::FillInConstructorsFunc()
{
	msg_db_f("FillInConstructorsFunc", 4);
	for (int i=0;i<cur_func->var.num;i++)
		/*if (cur_func->Var[i].Type->Element.num > 0)*/{
			SerialCommandParam param = param_local(cur_func->var[i].type, cur_func->var[i]._offset);
			add_cmd_constructor(param, false);
		}
}

void Serializer::FillInDestructors(bool from_temp)
{
	msg_db_f("FillInDestructors", 4);
	if (from_temp){
		for (int i=0;i<InsertedConstructorTemp.num;i++)
			add_cmd_destructor(InsertedConstructorTemp[i]);
		InsertedConstructorTemp.clear();
	}else{
		for (int i=0;i<InsertedConstructorFunc.num;i++)
			add_cmd_destructor(InsertedConstructorFunc[i]);
		InsertedConstructorFunc.clear();
	}
}

int Serializer::temp_in_cmd(int c, int v)
{
	if (cmd[c].inst >= inst_marker)
		return 0;
	int r = 0;
	if ((cmd[c].p1.kind == KindVarTemp) || (cmd[c].p1.kind == KindDerefVarTemp))
		if ((long)cmd[c].p1.p == v)
			r = 1 + ((cmd[c].p1.kind == KindDerefVarTemp) ? 4 : 0);
	if ((cmd[c].p2.kind == KindVarTemp) || (cmd[c].p2.kind == KindDerefVarTemp))
		if ((long)cmd[c].p2.p == v)
			r += 2 + ((cmd[c].p2.kind == KindDerefVarTemp) ? 8 : 0);
	return r;
}

void Serializer::ScanTempVarUsage()
{
	msg_db_f("ScanTempVarUsage", 4);
	foreachi(sTempVar &v, TempVar, i){
		v.first = -1;
		v.last = -1;
		v.count = 0;
		for (int c=0;c<cmd.num;c++){
			if (temp_in_cmd(c, i) > 0){
				v.count ++;
				if (v.first < 0)
					v.first = c;
				v.last = c;
			}
		}
		//so(string2("var %d:   %d - %d", i, v.first, v.last));
	}
	TempVarRangesDefined = true;
}

inline bool param_is_simple(SerialCommandParam &p)
{
	return ((p.kind == KindRegister) || (p.kind == KindVarTemp) || (p.kind < 0));
}

inline bool param_combi_allowed(int inst, SerialCommandParam &p1, SerialCommandParam &p2)
{
//	if (inst >= Asm::inst_marker)
//		return true;
	if ((!param_is_simple(p1)) && (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	Asm::GetInstructionParamFlags(inst, r1, w1, r2, w2);
	if ((w1) && (p1.kind == KindConstant))
		return false;
	if ((w2) && (p2.kind == KindConstant))
		return false;
	if ((p1.kind == KindConstant) || (p2.kind == KindConstant))
		if (!Asm::GetInstructionAllowConst(inst))
			return false;
	return true;
}

// mov [0x..] [0x...]  ->  mov temp, [0x..]   mov [0x..] temp
/*void CorrectUnallowedParamCombis()
{
	msg_db_f("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--)
		if (!param_combi_allowed(cmd[i].inst, cmd[i].p1, cmd[i].p2)){
			msg_write(string2("correcting param combi  cmd=%d", i));
			bool mov_first_param = (cmd[i].p2.kind < 0) || (cmd[i].p1.kind == KindRefToConst) || (cmd[i].p1.kind == KindConstant);
			SerialCommandParam *pp = mov_first_param ? &cmd[i].p1 : &cmd[i].p2;
			SerialCommandParam temp, p = *pp;
			add_temp(p.type, temp);

			*pp = temp;
			if (p.type->Size == 1)
				add_cmd(Asm::inst_mov_b, temp, p);
			else
				add_cmd(Asm::inst_mov, temp, p);
			move_last_cmd(i);
			//so("...ok");
		}
	ScanTempVarUsage();
}*/

// mov [0x..] [0x...]  ->  mov eax, [0x..]   mov [0x..] eax    (etc)
void Serializer::CorrectUnallowedParamCombis()
{
	msg_db_f("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= inst_marker)
			continue;

		// bad?
		if (param_combi_allowed(cmd[i].inst, cmd[i].p1, cmd[i].p2))
			continue;

		// correct
		so(format("correcting param combi  cmd=%d", i));
		bool mov_first_param = (cmd[i].p2.kind < 0) || (cmd[i].p1.kind == KindRefToConst) || (cmd[i].p1.kind == KindConstant);
		SerialCommandParam *pp = mov_first_param ? &cmd[i].p1 : &cmd[i].p2;
		SerialCommandParam p = *pp;

		if (p.type->size == 1){
			*pp = param_reg(p.type, Asm::RegAl);
			add_cmd(Asm::inst_mov_b, *pp, p);
		}else{
			*pp = param_reg(p.type, Asm::RegEax);
			add_cmd(Asm::inst_mov, *pp, p);
		}
		move_last_cmd(i);
		//so("...ok");
	}
	so("scan");
	ScanTempVarUsage();
}

int Serializer::find_unused_reg(int first, int last, bool allow_eax)
{
	for (int r=0;r<MapReg.num;r++)
		if (!is_reg_used_in_interval(MapReg[r], first, last))
			return MapReg[r];
	if (allow_eax)
		if (!is_reg_used_in_interval(Asm::RegEax, first, last))
			return Asm::RegEax;
	return -1;
}

// inst ... [local] ...
// ->      mov reg, local     inst ...[reg]...
void Serializer::solve_deref_temp_local(int c, int np, bool is_local)
{
	SerialCommandParam *pp = (np == 0) ? &cmd[c].p1 : &cmd[c].p2;
	SerialCommandParam p = *pp;
	int shift = p.shift;

	Type *type_pointer = is_local ? TypePointer : TempVar[(long)p.p].type;
	Type *type_data = p.type;
	
	p.kind = is_local ? KindVarLocal : KindVarTemp;
	p.shift = 0;
	p.type = type_pointer;

	int reg = find_unused_reg(c, c, true);
	//so(reg);
	if (reg < 0)
		script->DoErrorInternal("solve_deref_temp_local... no registers available");
	SerialCommandParam p_reg = param_reg(type_pointer, reg);
	SerialCommandParam p_deref_reg;
	p_deref_reg.kind = KindDerefRegister;
	p_deref_reg.p = (char*)reg;
	p_deref_reg.type = type_data;
	p_deref_reg.shift = 0;
	
	*pp = p_deref_reg;
		
	add_cmd(Asm::inst_mov, p_reg, p);
	move_last_cmd(c);
	if (shift > 0){
		add_cmd(Asm::inst_add, p_reg, param_const(TypeInt, (void*)shift));
		move_last_cmd(c + 1);
		add_reg_channel(reg, c, c + 2);
	}else
		add_reg_channel(reg, c, c + 1);
}

#if 0
void ResolveDerefLocal()
{
	msg_db_f("ResolveDerefLocal", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= Asm::inst_marker)
			continue;
		bool dl1 = (cmd[i].p1.kind == KindDerefVarLocal);
		bool dl2 = (cmd[i].p2.kind == KindDerefVarLocal);
		if (!(dl1 || dl2))
			continue;
		
		so(string2("deref local... cmd=%d", i));
		if (!dl2){
			solve_deref_temp(i, 0);
			i ++;
		}else if (!dl1){
			solve_deref_temp(i, 1);
			i ++;
		}else{
			// hopefully... p2 is read-only

			int reg = find_unused_reg(i, i, true);
			so(reg);
			if (reg < 0)
				script->DoErrorInternal("deref local... both sides... .no registers available");
			SerialCommandParam p_reg = param_reg(TypeReg32, reg);
			add_reg_channel(reg, i, i); // temp
			
			int reg2 = find_unused_reg(i, i, true);
			so(reg2);
			if (reg2 < 0)
				script->DoErrorInternal("deref local... both sides... .no registers available");
			SerialCommandParam p_reg2 = param_reg(TypeReg32, reg2);
			SerialCommandParam p_deref_reg2;
			p_deref_reg2.kind = KindDerefRegister;
			p_deref_reg2.p = (char*)reg2;
			p_deref_reg2.type = TypePointer;
			p_deref_reg2.shift = 0;
			RegChannel.resize(RegChannel.num - 1); // remove temp reg channel...

			// inst [l1] [l2]
			// ->
			// mov reg2, l1
			// mov reg, [reg2]
			// mov reg2, l2
			// inst [reg2], reg
			SerialCommandParam p1 = cmd[i].p1;
			SerialCommandParam p2 = cmd[i].p2;
			if (p1.shift + p2.shift > 0)
				script->DoErrorInternal("deref local... both sides... shift");
			p1.kind = p2.kind = KindVarLocal;
			cmd[i].p1 = p_deref_reg2;
			cmd[i].p2 = p_reg;
	
			add_cmd(Asm::inst_mov, p_reg2, p2);
			move_last_cmd(i);
	
			add_cmd(Asm::inst_mov, p_reg, p_deref_reg2);
			move_last_cmd(i + 1);
	
			add_cmd(Asm::inst_mov, p_reg2, p1);
			move_last_cmd(i + 2);

			add_reg_channel(reg, i + 1, i + 3);
			add_reg_channel(reg2, i, i + 3);
				
			i += 3;
		}
	}
}
#endif


void Serializer::ResolveDerefTempAndLocal()
{
	msg_db_f("ResolveDerefTempAndLocal", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= inst_marker)
			continue;
		bool dl1 = ((cmd[i].p1.kind == KindDerefVarLocal) || (cmd[i].p1.kind == KindDerefVarTemp));
		bool dl2 = ((cmd[i].p2.kind == KindDerefVarLocal) || (cmd[i].p2.kind == KindDerefVarTemp));
		if (!(dl1 || dl2))
			continue;

		bool is_local1 = (cmd[i].p1.kind == KindDerefVarLocal);
		bool is_local2 = (cmd[i].p2.kind == KindDerefVarLocal);
		
		so(format("deref temp/local... cmd=%d", i));
		if (!dl2){
			solve_deref_temp_local(i, 0, is_local1);
			i ++;
		}else if (!dl1){
			solve_deref_temp_local(i, 1, is_local2);
			i ++;
		}else{
			// hopefully... p2 is read-only

			Type *type_pointer = TypePointer;
			Type *type_data = cmd[i].p1.type;

			int reg = find_unused_reg(i, i, true);
			so(reg);
			if (reg < 0)
				DoError("deref local... both sides... .no registers available");
			
			SerialCommandParam p_reg = param_reg(type_data, reg);
			if (type_data->size == 1)
				p_reg = param_reg(type_data, reg_smallify(reg));
			add_reg_channel(reg, i, i); // temp
			
			int reg2 = find_unused_reg(i, i, true);
			so(reg2);
			if (reg2 < 0)
				DoError("deref temp/local... both sides... .no registers available");
			SerialCommandParam p_reg2 = param_reg(type_pointer, reg2);
			SerialCommandParam p_deref_reg2;
			p_deref_reg2.kind = KindDerefRegister;
			p_deref_reg2.p = (char*)reg2;
			p_deref_reg2.type = type_data;
			p_deref_reg2.shift = 0;
			RegChannel.pop(); // remove temp reg channel...

			// inst [l1] [l2]
			// ->
			// mov reg2, l2
			//   (add reg2, shift2)
			// mov reg, [reg2]
			// mov reg2, l1
			//   (add reg2, shift1)
			// inst [reg2], reg
			SerialCommandParam p1 = cmd[i].p1;
			SerialCommandParam p2 = cmd[i].p2;
			int shift1 = p1.shift;
			int shift2 = p2.shift;
			p1.shift = p2.shift = 0;
			
			p1.kind = is_local1 ? KindVarLocal : KindVarTemp;
			p2.kind = is_local2 ? KindVarLocal : KindVarTemp;
			p1.type = type_pointer;
			p2.type = type_pointer;
			cmd[i].p1 = p_deref_reg2;
			cmd[i].p2 = p_reg;
			int cmd_pos = i;

			int r2_first = cmd_pos;
			add_cmd(Asm::inst_mov, p_reg2, p2);
			move_last_cmd(cmd_pos ++);

			if (shift2 > 0){
				add_cmd(Asm::inst_add, p_reg2, param_const(TypeInt, (void*)shift2));
				move_last_cmd(cmd_pos ++);
			}

			int r1_first = cmd_pos;
			if (type_data->size == 1)
				add_cmd(Asm::inst_mov_b, p_reg, p_deref_reg2);
			else
				add_cmd(Asm::inst_mov, p_reg, p_deref_reg2);
			move_last_cmd(cmd_pos ++);
	
			add_cmd(Asm::inst_mov, p_reg2, p1);
			move_last_cmd(cmd_pos ++);

			if (shift1 > 0){
				add_cmd(Asm::inst_add, p_reg2, param_const(TypeInt, (void*)shift1));
				move_last_cmd(cmd_pos ++);
			}

			add_reg_channel(reg, r1_first, cmd_pos);
			add_reg_channel(reg2, r2_first, cmd_pos);
				
			i = cmd_pos;
		}
	}
}

bool Serializer::ParamUntouchedInInterval(SerialCommandParam &p, int first, int last)
{
	// direct usage?
	for (int i=first;i<=last;i++)
		if ((cmd[i].p1 == p) || (cmd[i].p2 == p))
			return false;
	
	// registers may be more subtle..
	if ((p.kind == KindRegister) || (p.kind == KindDerefRegister)){
		for (int i=first;i<=last;i++){
			
			// call violates all!
			if (cmd[i].inst == Asm::inst_call)
				return false;

			// div violates eax and edx
			if (cmd[i].inst == Asm::inst_div)
				if (((long)p.p == Asm::RegEdx) || ((long)p.p == Asm::RegEax))
					return false;

			// registers used? (may be part of the same meta-register)
			if ((cmd[i].p1.kind == KindRegister) || (cmd[i].p1.kind == KindDerefRegister))
				if (Asm::RegRoot[(long)cmd[i].p1.p] == Asm::RegRoot[(long)p.p])
					return false;
			if ((cmd[i].p2.kind == KindRegister) || (cmd[i].p2.kind == KindDerefRegister))
				if (Asm::RegRoot[(long)cmd[i].p2.p] == Asm::RegRoot[(long)p.p])
					return false;
		}
	}
	return true;
}

void Serializer::SimplifyFPUStack()
{
	msg_db_f("SimplifyFPUStack", 3);

// fstp temp
// fld temp
	for (int v=TempVar.num-1;v>=0;v--){
		// may only appear two times
		if (TempVar[v].count > 2)
			continue;

		// stored then loaded...?
		if ((cmd[TempVar[v].first].inst != Asm::inst_fstp) || (cmd[TempVar[v].last].inst != Asm::inst_fld))
			continue;

		// value still on the stack?
		int d_stack = 0, min_d_stack = 0, max_d_stack = 0;
		for (int i=TempVar[v].first + 1;i<TempVar[v].last;i++){
			if (cmd[i].inst == Asm::inst_fld)
				d_stack ++;
			else if (cmd[i].inst == Asm::inst_fstp)
				d_stack --;
			min_d_stack = min(min_d_stack, d_stack);
			max_d_stack = max(max_d_stack, d_stack);
		}
		if ((d_stack != 0) || (min_d_stack < 0) || (max_d_stack > 5))
			continue;

		// reuse value on the stack
		so(format("fpu (a)  var=%d first=%d last=%d stack: d=%d min=%d max=%d", v, TempVar[v].first, TempVar[v].last, d_stack, min_d_stack, max_d_stack));
		remove_cmd(TempVar[v].last);
		remove_cmd(TempVar[v].first);
		remove_temp_var(v);
	}

// fstp temp
// mov xxx, temp
	for (int v=TempVar.num-1;v>=0;v--){
		// may only appear two times
		if (TempVar[v].count > 2)
			continue;

		// stored then moved...?
		if ((cmd[TempVar[v].first].inst != Asm::inst_fstp) || (cmd[TempVar[v].last].inst != Asm::inst_mov))
			continue;
		if (temp_in_cmd(TempVar[v].last, v) != 2)
			continue;
		// moved into fstore'able?
		int kind = cmd[TempVar[v].last].p1.kind;
		if ((kind != KindVarLocal) && (kind != KindVarGlobal) && (kind != KindVarTemp) && (kind != KindDerefVarTemp) && (kind != KindDerefRegister))
		    continue;

		// check, if mov target is used in between
		SerialCommandParam target = cmd[TempVar[v].last].p1;
		if (!ParamUntouchedInInterval(target, TempVar[v].first + 1 ,TempVar[v].last - 1))
			continue;
		// ...we are lazy...
		//if (TempVar[v].last - TempVar[v].first != 1)
		//	continue;

		// store directly into target
		so(format("fpu (b)  var=%d first=%d last=%d", v, TempVar[v].first, TempVar[v].last));
		cmd[TempVar[v].first].p1 = target;
		move_param(target, TempVar[v].last, TempVar[v].first);
		remove_cmd(TempVar[v].last);
		remove_temp_var(v);
	}
}

// remove  mov x,...    mov ...,x   (and similar)
//    (does not affect reg channels)
void Serializer::SimplifyMovs()
{
	// TODO: count > 2 .... first == input && all_other == output?  (only if first == mov (!=eax?)... else count == 2)
	// should take care of fpu simplification (b)...
	
	msg_db_f("SimplifyMovs", 3);
	for (int vi=TempVar.num-1;vi>=0;vi--){
		sTempVar &v = TempVar[vi];
		
		// may only appear two times
		if (v.count > 2)
			continue;

		// both times in a mov command (or fld as second)
		if (cmd[v.first].inst != Asm::inst_mov)
			continue;
		int n = cmd[v.last].inst;
		bool fld = (n == Asm::inst_fld) || (n == Asm::inst_fadd) || (n == Asm::inst_fadd) || (n == Asm::inst_fsub) || (n == Asm::inst_fmul) || (n == Asm::inst_fdiv);
		if ((cmd[v.last].inst != Asm::inst_mov) && (!fld))
			continue;
		
		// used as source/target?   no deref?
		if ((temp_in_cmd(v.first, vi) != 1) || (temp_in_cmd(v.last, vi) != (fld ? 1 : 2)))
			continue;

		// new construction allowed?
		SerialCommandParam target = cmd[v.last].p1;
		SerialCommandParam source = cmd[v.first].p2;
		if (fld){
			if (!param_combi_allowed(cmd[v.last].inst, source, cmd[v.last].p2))
				continue;
		}else{
			if (!param_combi_allowed(cmd[v.last].inst, cmd[v.last].p1, source))
				continue;
		}

		// check, if mov source or target are used in between
		if (!ParamUntouchedInInterval(target, v.first + 1 ,v.last - 1))
			continue;
		if (!ParamUntouchedInInterval(source, v.first + 1 ,v.last - 1))
			continue;
		
		so(format("mov simplification allowed  v=%d first=%d last=%d", vi, v.first, v.last));
		if (fld)
			cmd[v.last].p1 = source;
		else
			cmd[v.last].p2 = source;
		move_param(source, v.first, v.last);
		remove_cmd(v.first);
		remove_temp_var(vi);
	}

	// TODO: should happen automatically...
	//ScanTempVarUsage();
	//cmd_list_out();
}

/*inline void test_reg_usage(int c)
{
	// call -> violates all...
	if (cmd[c].inst == Asm::inst_call){
		for (int i=0;i<max_reg;i++)
			RegUsed[i] = true;
		return;
	}
	if ((cmd[c].p1.kind == KindRegister) || (cmd[c].p1.kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p1.p);
	if ((cmd[c].p2.kind == KindRegister) || (cmd[c].p2.kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p2.p);
}*/

void Serializer::MapTempVarToReg(int vi, int reg)
{
	msg_db_f("reg", 4);
	sTempVar &v = TempVar[vi];
	so(format("temp=reg:  %d - %d:   tv %d := reg %d", v.first, v.last, vi, reg));

	int reg32 = reg;
	
	// only small register?
	if (v.type->size == 1){
		if (reg == Asm::RegEax)		reg = Asm::RegAl;
		else if (reg == Asm::RegEdx)	reg = Asm::RegDl;
		else if (reg == Asm::RegEcx)	reg = Asm::RegCl;
		else if (reg == Asm::RegEbx)	reg = Asm::RegBl;
		else DoError(format("wrong 8b register: %d", reg));
	}
	
	SerialCommandParam p = param_reg(v.type, reg);
	
	// map register
	for (int i=v.first;i<=v.last;i++){
		int r = temp_in_cmd(i, vi);
		if (r & 1){
			p.shift = cmd[i].p1.shift;
			cmd[i].p1 = p;
			if (r & 4)
				cmd[i].p1.kind = KindDerefRegister;
		}
		if (r & 2){
			p.shift = cmd[i].p2.shift;
			cmd[i].p2 = p;
			if (r & 8)
				cmd[i].p2.kind = KindDerefRegister;
		}
	}
	add_reg_channel(reg32, v.first, v.last);
}

void Serializer::add_stack_var(Type *type, int first, int last, SerialCommandParam &p)
{
	so("add_stack_var");
	so(type->size);
	// find free stack space for the life span of the variable....
	// don't mind... use old algorithm: ALWAYS grow stack... remove ALL on each command in a block

	int s = mem_align(type->size);
	StackOffset += s;
	int offset = - StackOffset;
	if (StackOffset > StackMaxSize)
		StackMaxSize = StackOffset;

	p.kind = KindVarLocal;
	p.p = (char*)offset;
	p.type = type;
	p.shift = 0;
}

void Serializer::MapTempVarToStack(int vi)
{
	msg_db_f("stack", 4);
	sTempVar &v = TempVar[vi];
	so(format("temp=stack: %d   (%d - %d)", vi, v.first, v.last));

	SerialCommandParam p;
	add_stack_var(v.type, v.first, v.last, p);
	
	// map
	for (int i=v.first;i<=v.last;i++){
		int r = temp_in_cmd(i, vi);
		if (r == 0)
			continue;

		if ((r & 3) == 3)
			script->DoErrorInternal("asm error: (MapTempVar) temp var on both sides of command");
		
		SerialCommandParam *p_own;
		if ((r & 1) > 0){
			p_own = &cmd[i].p1;
		}else{
			p_own = &cmd[i].p2;
		}
		bool deref = (r > 3);

		p_own->kind = deref ? KindDerefVarLocal : KindVarLocal;
		p_own->p = p.p;
		
#if 0
		SerialCommandParam *p_own, *p_other;
		if ((r & 1) > 0){
			p_own = &cmd[i].p1;
			p_other = &cmd[i].p2;
		}else{
			p_own = &cmd[i].p2;
			p_other = &cmd[i].p1;
		}
		bool deref = (r > 3);

		// allowed directly?
		if (!deref){
			if (param_is_simple(*p_other)){
				*p_own = p;
				continue;
			}
		}

		// TODO: map literally... solve unallowed combis later...?

		// is our variable used for writing... or reading?
		bool var_read = false;
		bool var_write = false;
		int c = cmd[i].inst;
		bool dummy1, dummy2;
		if ((r & 1) > 0)
			GetInstructionParamFlags(cmd[i].inst, var_read, var_write, dummy1, dummy2);
		else if ((r & 2) > 0)
			GetInstructionParamFlags(cmd[i].inst, dummy1, dummy2, var_read, var_write);

		if ((var_write) && (var_read)){ // rw
			if (deref)
				script->DoErrorInternal("map_stack_var (read, write, deref)");
			*p_own = p_eax;
			add_cmd(Asm::inst_mov, p_eax, p);
			move_last_cmd(i);
			add_cmd(Asm::inst_mov, p, p_eax);
			move_last_cmd(i+2);
			add_reg_channel(RegEax, i, i + 2);
			i += 2;
			
		}else if (var_write){ // write only
			if (deref){
				//script->DoErrorInternal("map_stack_var (write, deref)");
				int shift = p_own->shift;
				*p_own = p_deref_eax;
				add_cmd(Asm::inst_mov, p_eax, p);
				move_last_cmd(i);
				if (shift > 0){
					add_cmd(Asm::inst_add, p_eax, param_const(TypeInt, (void*)shift));
					move_last_cmd(i + 1);
					add_reg_channel(RegEax, i, i + 2);
				}else
					add_reg_channel(RegEax, i, i + 1);
			}else{
				*p_own = p_eax;
				add_cmd(Asm::inst_mov, p, p_eax);
				move_last_cmd(i+1);
				add_reg_channel(RegEax, i, i + 1);
			}
			i ++;
		}else{ // read only
			int shift = p_own->shift;
			*p_own = deref ? p_deref_eax : p_eax;
			add_cmd(Asm::inst_mov, p_eax, p);
			move_last_cmd(i);
			if ((deref) && (shift > 0)){
				add_cmd(Asm::inst_add, p_eax, param_const(TypeInt, (void*)shift));
				move_last_cmd(i + 1);
				add_reg_channel(RegEax, i, i + 2);
			}else
				add_reg_channel(RegEax, i, i + 1);
			i ++;
		}
#endif
	}
}

bool Serializer::is_reg_used_in_interval(int reg, int first, int last)
{
	//so(string2("used?   %d: %d - %d", reg, first, last));
	for (int i=0;i<RegChannel.num;i++)
		if (RegChannel[i].reg == reg){
			//so(string2("rc   %d: %d - %d", RegChannel[i].reg, RegChannel[i].first, RegChannel[i].last));
			if ((RegChannel[i].first <= last) && (RegChannel[i].last >= first)){
				so(i);
				return true;
			}
		}
	//so("false");
	return false;
}

void Serializer::MapTempVar(int vi)
{
	msg_db_f("MapTempVar", 4);
	sTempVar &v = TempVar[vi];
	int first = v.first;
	int last = v.last;

	bool reg_allowed = true;
	for (int i=first;i<=last;i++)
		if (temp_in_cmd(i, vi))
			if (!Asm::GetInstructionAllowGenReg(cmd[i].inst)){
				reg_allowed = false;
				break;
			}

	int reg = -1;
	if (reg_allowed){

		// any register not used in this interval?
		for (int i=0;i<max_reg;i++)
			RegUsed[i] = false;
		for (int i=0;i<RegChannel.num;i++)
			if ((RegChannel[i].first <= last) && (RegChannel[i].last >= first))
				RegUsed[RegChannel[i].reg] = true;
		for (int i=0;i<MapReg.num;i++)
			if (!RegUsed[MapReg[i]]){
				reg = MapReg[i];
				break;
			}
	}

	if (reg >= 0)
		MapTempVarToReg(vi, reg);
	else
		MapTempVarToStack(vi);
}

void Serializer::MapTempVars()
{
	msg_db_f("MapTempVars", 3);

	for (int i=0;i<TempVar.num;i++)
		MapTempVar(i);
	
	//cmd_list_out();
}

inline void try_map_param_to_stack(SerialCommandParam &p, int v, SerialCommandParam &stackvar)
{
	if ((p.kind == KindVarTemp) && ((long)p.p == v)){
		int shift = p.shift;
		p = stackvar;
		p.shift = shift;
	}
}

void Serializer::MapReferencedTempVars()
{
	msg_db_f("MapReferencedTempVars", 3);
	for (int i=0;i<cmd.num;i++)
		if (cmd[i].inst == Asm::inst_lea)
			if (cmd[i].p2.kind == KindVarTemp){
				TempVar[(long)cmd[i].p2.p].referenced = true;
			}

	for (int i=TempVar.num-1;i>=0;i--)
		if (TempVar[i].referenced){
			SerialCommandParam stackvar;
			add_stack_var(TempVar[i].type, TempVar[i].first, TempVar[i].last, stackvar);
			for (int j=0;j<cmd.num;j++){
				try_map_param_to_stack(cmd[j].p1, i, stackvar);
				try_map_param_to_stack(cmd[j].p2, i, stackvar);
			}
			remove_temp_var(i);
		}
}

void Serializer::DisentangleShiftedTempVars()
{
	msg_db_f("DisentangleShiftedTempVars", 3);
	for (int i=0;i<cmd.num;i++){
		if ((cmd[i].p1.kind == KindVarTemp) && (cmd[i].p1.shift > 0)){
			TempVar[(long)cmd[i].p1.p].entangled = max(TempVar[(long)cmd[i].p1.p].entangled, cmd[i].p1.shift);
		}
		if ((cmd[i].p2.kind == KindVarTemp) && (cmd[i].p2.shift > 0)){
			TempVar[(long)cmd[i].p2.p].entangled = max(TempVar[(long)cmd[i].p2.p].entangled, cmd[i].p2.shift);
		}
	}
	for (int i=TempVar.num-1;i>=0;i--)
		if (TempVar[i].entangled > 0){
			int n = TempVar[i].entangled / 4 + 1;
			Type *t = TempVar[i].type;
			so("entangled");
			so(n);
			SerialCommandParam *p = new SerialCommandParam[n];

			// create small temp vars
			for (int j=0;j<n;j++){
				Type *tt = TypeReg32;
				// corresponding to element in a class?
				for (int k=0;k<t->element.num;k++)
					if (t->element[k].offset == j * 4)
						if (t->element[k].type->size == 4)
							tt = t->element[k].type;
				add_temp(tt, p[j]);
			}
			
			for (int j=0;j<cmd.num;j++){
				if ((cmd[j].p1.kind == KindVarTemp) && ((long)cmd[j].p1.p == i))
					cmd[j].p1 = p[cmd[j].p1.shift / 4];
				if ((cmd[j].p2.kind == KindVarTemp) && ((long)cmd[j].p2.p == i))
					cmd[j].p2 = p[cmd[j].p2.shift / 4];
			}
			delete[]p;
			remove_temp_var(i);
		}

	ScanTempVarUsage();
}

// TODO....
void Serializer::ResolveDerefRegShift()
{
	msg_db_f("ResolveDerefRegShift", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if ((cmd[i].p1.kind == KindDerefRegister) && (cmd[i].p1.shift > 0)){
			int s = cmd[i].p1.shift;
			cmd[i].p1.shift = 0;
			add_cmd(Asm::inst_add, param_reg(TypeReg32, Asm::RegRoot[(long)cmd[i].p1.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(i);
			add_cmd(Asm::inst_sub, param_reg(TypeReg32, Asm::RegRoot[(long)cmd[i].p1.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(i + 2);
			continue;
		}
		if ((cmd[i].p2.kind == KindDerefRegister) && (cmd[i].p2.shift > 0)){
			int s = cmd[i].p2.shift;
			cmd[i].p2.shift = 0;
			add_cmd(Asm::inst_add, param_reg(TypeReg32, Asm::RegRoot[(long)cmd[i].p2.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(i);
			add_cmd(Asm::inst_sub, param_reg(TypeReg32, Asm::RegRoot[(long)cmd[i].p2.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(i + 2);
			continue;
		}
	}
}

void init_serializing()
{
	p_eax = param_reg(TypeReg32, Asm::RegEax);
	p_eax_int = param_reg(TypeInt, Asm::RegEax);

	p_deref_eax.kind = KindDerefRegister;
	p_deref_eax.p = (char*)Asm::RegEax;
	p_deref_eax.type = TypePointer;
	p_deref_eax.shift = 0;
	
	p_ax = param_reg(TypeReg16, Asm::RegAx);
	p_al = param_reg(TypeReg8, Asm::RegAl);
	p_al_bool = param_reg(TypeBool, Asm::RegAl);
	p_al_char = param_reg(TypeChar, Asm::RegAl);
	p_ah = param_reg(TypeReg8, Asm::RegAh);
	p_st0 = param_reg(TypeFloat, Asm::RegSt0);
	p_st1 = param_reg(TypeFloat, Asm::RegSt1);
}

void Serializer::SerializeFunction(Function *f)
{
	msg_db_f("SerializeFunction", 2);

	init_serializing();

	CreateAsmMetaInfo(pre_script);
	pre_script->AsmMetaInfo->CurrentOpcodePos = script->OpcodeSize;
	pre_script->AsmMetaInfo->PreInsertionLength = script->OpcodeSize;
	pre_script->AsmMetaInfo->LineOffset = 0;
	Asm::CurrentMetaInfo = pre_script->AsmMetaInfo;

	cur_func = f;
	NumMarkers = 0;
	call_used = false;
	StackOffset = f->_var_size;
	StackMaxSize = f->_var_size;
	TempVarRangesDefined = false;

	InsertedConstructorTemp.clear();
	InsertedConstructorFunc.clear();
	
#ifdef allow_registers
	//MapReg.add(Asm::RegEax);
	MapReg.add(Asm::RegEcx);
	MapReg.add(Asm::RegEdx);
	/*MapReg.add(Asm::RegEbx);
	MapReg.add(Asm::RegEsi);
	MapReg.add(Asm::RegEdi);*/
#endif

// serialize

	// intro
	add_cmd(Asm::inst_push, param_reg(TypeReg32, Asm::RegEbp));
	add_cmd(Asm::inst_mov, param_reg(TypeReg32, Asm::RegEbp), param_reg(TypeReg32, Asm::RegEsp));
	

	FillInConstructorsFunc();

	// function
	SerializeBlock(f->block, 0);
	
	FillInDestructors(false);


	if (StuffToAdd.num > 0){
		msg_write(f->name);
		msg_write(StuffToAdd.num);
		for (int i=0;i<StuffToAdd.num;i++){
			msg_write(StuffToAdd[i].kind);
			msg_write(StuffToAdd[i].marker);
			msg_write(StuffToAdd[i].index);
			msg_write(StuffToAdd[i].level);
		}
		DoError("StuffToAdd");
	}

	// outro
	bool need_outro = true;
	if (f->block->command.num > 0)
		if ((f->block->command.back()->kind == KindCompilerFunction) && (f->block->command.back()->link_nr == CommandReturn))
			need_outro = false;
	if (need_outro){
		add_cmd(Asm::inst_leave);
		if (f->return_type->size > 4)
			add_cmd(Asm::inst_ret, param_const(TypeInt, (void*)4));
		else
			add_cmd(Asm::inst_ret);
	}
	
	cmd_list_out();

// do all the translations...

	MapReferencedTempVars();

	//HandleDerefTemp();

	DisentangleShiftedTempVars();
	cmd_list_out();

	ResolveDerefTempAndLocal();
	cmd_list_out();

#ifdef allow_simplification
	SimplifyMovs();
	cmd_list_out();

	SimplifyFPUStack();
	cmd_list_out();
#endif

	MapTempVars();
	cmd_list_out();

	ResolveDerefRegShift();
	cmd_list_out();

	//ResolveDerefLocal();
	//cmd_list_out();

	CorrectUnallowedParamCombis();

	// allocate stack memory
	//if (call_used){
	StackMaxSize = ((StackMaxSize + StackMemAlign - 1) / StackMemAlign) * StackMemAlign;
		if (StackMaxSize > 127){
			add_cmd(Asm::inst_sub, param_reg(TypeReg32, Asm::RegEsp), param_const(TypeInt, (void*)StackMaxSize));
			move_last_cmd(2);
		}else if (StackMaxSize > 0){
			add_cmd(Asm::inst_sub_b, param_reg(TypeReg32, Asm::RegEsp), param_const(TypeInt, (void*)StackMaxSize));
			move_last_cmd(2);
		}
	//}
	cmd_list_out();
}

inline void get_param(int inst, SerialCommandParam &p, int &param_type, void *&param, Asm::InstructionWithParamsList *list, Script *s)
{
	if (p.kind < 0){
		param_type = Asm::PKNone;
		param = NULL;
	}else if (p.kind == KindMarker){
		//msg_error("marker");
		param_type = Asm::PKLabel;
		param = (void*)list->add_label("_kaba_" + i2s((int)(long)p.p), false);
	}else if (p.kind == KindRegister){
		param_type = Asm::PKRegister;
		param = p.p;
		if (p.shift > 0)
			s->DoErrorInternal("get_param: reg + shift");
	}else if (p.kind == KindDerefRegister){
		param_type = Asm::PKDerefRegister;
		param = p.p;
		if (p.shift > 0){
			if ((long)p.p == Asm::RegEdx){
				param_type = Asm::PKEdxRel;
				param = (void*)p.shift;
			}else
				s->DoErrorInternal("get_param: [reg] + shift");
		}
	}else if (p.kind == KindVarGlobal){
		param_type = Asm::PKDerefConstant;
		param = p.p + p.shift;
	}else if (p.kind == KindVarLocal){
		param_type = Asm::PKLocal;
		param = p.p + p.shift;
	}else if (p.kind == KindRefToConst){
		bool imm_allowed = Asm::GetInstructionAllowConst(inst);
		if ((p.type->size <= 4) && (imm_allowed)){
			param_type = (p.type->size == 1) ? Asm::PKConstant8 : Asm::PKConstant32;
			param = (char*)*(int*)(p.p + p.shift);
		}else{
			param_type = Asm::PKDerefConstant;
			param = p.p + p.shift;
		}
	}else if (p.kind == KindConstant){
		param_type = Asm::PKConstant32;
		if (p.type->size == 1)
			param_type = Asm::PKConstant8;
		param = p.p;
		if (p.shift > 0)
			s->DoErrorInternal("get_param: const + shift");
	}else
		s->DoErrorInternal("get_param: unexpected param..." + Kind2Str(p.kind));
}


void assemble_cmd(Asm::InstructionWithParamsList *list, SerialCommand &c, Script *s)
{
	// translate parameters
	int param1_type, param2_type;
	void *param1, *param2;
	get_param(c.inst, c.p1, param1_type, param1, list, s);
	get_param(c.inst, c.p2, param2_type, param2, list, s);

	// assemble instruction
	//list->current_line = c.
	list->add_easy(c.inst, param1_type, param1, param2_type, param2);
	//printf("%s", Opcode2Asm(oc, ocs));
}

void AddAsmBlock(Asm::InstructionWithParamsList *list, Script *s)
{
	msg_db_f("AddAsmBlock", 4);
	//msg_write(".------------------------------- asm");
	PreScript *ps = s->pre_script;
	ps->AsmMetaInfo->LineOffset = ps->AsmBlocks[0].line;
	list->AppendFromSource(ps->AsmBlocks[0].block);
	delete[](ps->AsmBlocks[0].block);
	ps->AsmBlocks.erase(0);
}

void Serializer::Assemble(char *Opcode, int &OpcodeSize)
{
	msg_db_f("Serializer.void Serializer::ResolveDerefRegShift()", 2);

	for (int i=0;i<cmd.num;i++){

		if (cmd[i].inst == inst_marker){
			//msg_write("marker _kaba_" + i2s(cmd[i].p1.kind));
			list->add_label("_kaba_" + i2s(cmd[i].p1.kind), true);
		}else if (cmd[i].inst == inst_asm){
			AddAsmBlock(list, script);
		}else{

			// push 8 bit -> push 32 bit
			if (cmd[i].inst == Asm::inst_push)
				if (cmd[i].p1.kind == KindRegister)
					cmd[i].p1.p = (char*) Asm::RegRoot[(long)cmd[i].p1.p];
			

			assemble_cmd(list, cmd[i], script);
		}
	}

	//msg_write(Opcode2Asm(Opcode, OpcodeSize));

	list->Optimize(Opcode, OpcodeSize);
	list->Compile(Opcode, OpcodeSize);
}

void Serializer::DoError(const string &msg)
{
	script->DoErrorInternal(msg);
}

void Serializer::DoErrorLink(const string &msg)
{
	script->DoErrorLink(msg);
}

void do_func_align(char *Opcode, int &OpcodeSize)
{
	int ocs_new = ((OpcodeSize + FunctionAlign - 1) / FunctionAlign) * FunctionAlign;
	for (int i=OpcodeSize;i<ocs_new;i++)
		Opcode[i] = 0x90;
	OpcodeSize = ocs_new;
}

Serializer::Serializer(Script *s)
{
	script = s;
	pre_script = s->pre_script;
	list = new Asm::InstructionWithParamsList(0);
}

Serializer::~Serializer()
{
	delete(list);
}

void Script::CompileFunction(Function *f, char *Opcode, int &OpcodeSize)
{
	msg_db_f("Compile Function", 2);

	do_func_align(Opcode, OpcodeSize);

	cur_func = f;
	Serializer d = Serializer(this);

	try{
		d.SerializeFunction(f);
		d.Assemble(Opcode, OpcodeSize);
	}catch(Exception &e){
		throw e;
	}catch(Asm::Exception &e){
		throw Exception(e, this);
	}
}

};
