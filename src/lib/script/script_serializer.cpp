#include "script.h"
//#include "dasm.h"
#include "../file/file.h"

//#define _insert_asm_

#define allow_simplification
#define allow_registers
#define _insert_asm_

extern void script_db_out(const string &str);
extern void script_db_out(int i);
extern void script_db_right();
extern void script_db_left();

#define so		script_db_out
#define right	script_db_right
#define left	script_db_left

//#ifdef ScriptDebug

bool UseConstAsGlobalVar = false;


extern int GlobalWaitingMode;
extern float GlobalTimeToWait;


struct sRegChannel
{
	int reg;
	int first, last;
};
#define max_reg		8 // >= all RegXXX used...


inline bool is_reg_used_in_interval(SerializerData *d, int reg, int first, int last);

enum{
	inst_marker = 1000,
	inst_asm,
	inst_end,
};

struct sLoopData
{
	int marker_continue, marker_break;
	int level, index;
};

static sSerialCommandParam p_eax, p_eax_int, p_deref_eax, p_ax, p_al, p_ah, p_al_bool, p_al_char, p_st0, p_st1;
static const sSerialCommandParam p_none = {-1, NULL, NULL, 0};

struct sTempVar
{
	sType *type;
	int first, last, count;
	bool referenced;
	int entangled;
};

struct sStuffToAdd
{
	int kind, marker, level, index;
};

enum{
	StuffKindMarker,
	StuffKindJump,
};


struct SerializerData
{
	Array<sSerialCommand> cmd;
	int NumMarkers;
	CScript *cur_script;
	sFunction *cur_func;
	bool call_used;
	int LastCommandSize;
	sCommand *NextCommand;
	bool TempVarRangesDefined;

	Array<int> MapReg;
	Array<sRegChannel> RegChannel;

	bool RegUsed[max_reg];
	Array<sLoopData> LoopData;

	int StackOffset, StackMaxSize;
	Array<sTempVar> TempVar;

	Array<sStuffToAdd> StuffToAdd;
};

inline void add_reg_channel(SerializerData *d, int reg, int first, int last)
{
	sRegChannel c = {reg, first, last};
	d->RegChannel.add(c);
}

void add_cmd_constructor(SerializerData *d, sSerialCommandParam &param, bool is_temp);

inline void add_temp(SerializerData *d, sType *t, sSerialCommandParam &param)
{
	if (t != TypeVoid){
		sTempVar v;
		v.type = t;
		v.referenced = (t->IsSuperArray);
		v.entangled = 0;
		d->TempVar.add(v);
		param.kind = KindVarTemp;
		param.p = (char*)(d->TempVar.num - 1);
		param.type = t;
		param.shift = 0;
		

		if (t->Element.num > 0)
			add_cmd_constructor(d, param, true);
	}else{
		param = p_none;
	}
}

inline sType *get_subtype(sType *t)
{
	if (t->SubType)
		return t->SubType;
	msg_error("subtype wanted of... " + t->Name);
	//msg_write(cur_func->Name);
	return TypeUnknown;
}

inline void deref_temp(sSerialCommandParam &param, sSerialCommandParam &deref)
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

inline sSerialCommandParam param_shift(sSerialCommandParam &param, int shift)
{
	sSerialCommandParam p = param;
	p.shift = shift;
	return p;
}

inline sSerialCommandParam param_global(sType *type, void *v)
{
	sSerialCommandParam p;
	p.type = type;
	p.kind = KindVarGlobal;
	p.p = (char*)v;
	p.shift = 0;
	return p;
}

inline sSerialCommandParam param_local(sType *type, int offset)
{
	sSerialCommandParam p;
	p.type = type;
	p.kind = KindVarLocal;
	p.p = (char*)offset;
	p.shift = 0;
	return p;
}

inline sSerialCommandParam param_const(sType *type, void *c)
{
	sSerialCommandParam p;
	p.type = type;
	p.kind = KindConstant;
	p.p = (char*)c;
	p.shift = 0;
	return p;
}

inline sSerialCommandParam param_marker(int m)
{
	sSerialCommandParam p;
	p.type = TypeInt;
	p.kind = KindMarker;
	p.p = (char*)m;
	p.shift = 0;
	return p;
}

inline sSerialCommandParam param_reg(sType *type, int reg)
{
	sSerialCommandParam p;
	p.kind = KindRegister;
	p.p = (char*)reg;
	p.type = type;
	p.shift = 0;
	return p;
}

inline void param_out(string &str, sSerialCommandParam &p)
{
	//msg_db_r("param_out", 4);
	if (p.kind >= 0){
		str += format("   %s %p (%s)", Kind2Str(p.kind).c_str(), p.p, p.type->Name.c_str());
		if (p.shift > 0)
			str += format(" + shift %d", p.shift);
	}
	//msg_db_l(4);
}

void cmd_out(int n, sSerialCommand &c)
{
	//msg_db_r("cmd_out", 4);
	if (c.inst == inst_marker)
		so(format("%d: -- Marker %d --", n, c.p1.kind));
	else if (c.inst == inst_asm)
		so(format("%d: -- Asm --", n));
	else{
		string t;
		t = "???";
		bool found = false;
		for (int i=0;i<NumInstructionNames;i++)
			if (c.inst == InstructionName[i].inst)
				t = format("%3d:  %s", n, InstructionName[i].name);
		param_out(t, c.p1);
		param_out(t, c.p2);
		so(t);
	}
	//msg_db_l(4);
}

void cmd_list_out()
{
#ifdef ScriptDebug
	msg_db_r("cmd_list_out", 4);
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
	msg_db_l(4);
#endif
}

inline void add_cmd(SerializerData *d, int inst, sSerialCommandParam p1, sSerialCommandParam p2)
{
	sSerialCommand c;
	c.inst = inst;
	c.p1 = p1;
	c.p2 = p2;
	d->cmd.add(c);

	// call violates all used registers...
	if (inst == inst_call)
		for (int i=0;i<d->MapReg.num;i++)
			add_reg_channel(d, d->MapReg[i], d->cmd.num - 1, d->cmd.num - 1);
}

inline void add_cmd(SerializerData *d, int inst, sSerialCommandParam p)
{
	add_cmd(d, inst, p, p_none);
}

inline void add_cmd(SerializerData *d, int inst)
{
	add_cmd(d, inst, p_none, p_none);
}

inline void move_last_cmd(SerializerData *d, int index)
{
	sSerialCommand c = d->cmd.back();
	for (int i=d->cmd.num-1;i>index;i--)
		d->cmd[i] = d->cmd[i - 1];
	d->cmd[index] = c;

	// adjust temp vars
	if (d->TempVarRangesDefined)
		foreach(d->TempVar, v){
			if (v->first >= index)
				v->first ++;
			if (v->last >= index)
				v->last ++;
		}

	// adjust reg channels
	foreach(d->RegChannel, r){
		if (r->first >= index)
			r->first ++;
		if (r->last >= index)
			r->last ++;
	}
}

inline void remove_cmd(SerializerData *d, int index)
{
	d->cmd.erase(index);

	// adjust temp vars
	foreach(d->TempVar, v){
		if (v->first >= index)
			v->first --;
		if (v->last >= index)
			v->last --;
	}

	// adjust reg channels
	foreach(d->RegChannel, r){
		if (r->first >= index)
			r->first --;
		if (r->last >= index)
			r->last --;
	}
}

inline void remove_temp_var(SerializerData *d, int v)
{
	foreach(d->cmd, c){
		if ((c->p1.kind == KindVarTemp) || (c->p1.kind == KindDerefVarTemp))
			if ((long)c->p1.p > v)
				c->p1.p = (char*)((long)c->p1.p - 1);
		if ((c->p2.kind == KindVarTemp) || (c->p2.kind == KindDerefVarTemp))
			if ((long)c->p2.p > v)
				c->p2.p = (char*)((long)c->p2.p - 1);
	}
	d->TempVar.erase(v);
}

inline void move_param(SerializerData *d, sSerialCommandParam &p, int from, int to)
{
	if ((p.kind == KindVarTemp) || (p.kind == KindDerefVarTemp)){
		so("move_param temp");
		long v = (long)p.p;
		if (d->TempVar[v].last < max(from, to))
			d->TempVar[v].last = max(from, to);
		if (d->TempVar[v].first > min(from, to))
			d->TempVar[v].first = min(from, to);
	}else if ((p.kind == KindRegister) || (p.kind == KindDerefRegister)){
		so("move_param reg");
		long r = (long)p.p;
		bool found = false;
		foreach(d->RegChannel, rc)
			if ((r == rc->reg) && (from >= rc->first) && (from >= rc->first)){
				if (rc->last < max(from, to))
					rc->last = max(from, to);
				if (rc->first > min(from, to))
					rc->first = min(from, to);
				found = true;
			}
		if (!found)
			msg_error(format("move_param: kein RegChannel...  reg=%d  from=%d", r, from));
	}
}

inline void add_marker(SerializerData *d, int m = -1)
{
	sSerialCommandParam p = p_none;
	if (m < 0)
		p.kind = d->NumMarkers ++;
	else
		p.kind = m;
	add_cmd(d, inst_marker, p);
}

inline int add_marker_after_command(SerializerData *d, int level, int index)
{
	int n = d->NumMarkers ++;
	sStuffToAdd m = {StuffKindMarker, n, level, index};
	d->StuffToAdd.add(m);
	return n;
}

inline void add_jump_after_command(SerializerData *d, int level, int index, int marker)
{
	sStuffToAdd j = {StuffKindJump, marker, level, index};
	d->StuffToAdd.add(j);
}

inline int reg_smallify(int reg)
{
	if (reg == RegEax)	return RegAl;
	if (reg == RegEcx)	return RegCl;
	if (reg == RegEdx)	return RegDl;
	if (reg == RegEbx)	return RegBl;
	msg_error("reg smallify");
	return -1;
}


static Array<sSerialCommandParam> CompilerFunctionParam;
static sSerialCommandParam CompilerFunctionReturn = {-1, NULL, NULL};
static sSerialCommandParam CompilerFunctionInstance = {-1, NULL, NULL};

void AddFuncParam(sSerialCommandParam &p)
{
	CompilerFunctionParam.add(p);
}

void AddFuncReturn(sSerialCommandParam &r)
{
	CompilerFunctionReturn = r;
}

void AddFuncInstance(sSerialCommandParam &inst)
{
	CompilerFunctionInstance = inst;
}

void AddReference(SerializerData *d, sSerialCommandParam &param, sType *type, sSerialCommandParam &ret);

void AddFunctionCall(SerializerData *d, void *func, int func_no = -1)
{
	msg_db_r("AddFunctionCall", 4);
	d->call_used = true;
	sType *type = CompilerFunctionReturn.type;
	if (!type)
		type = TypeVoid;

	// return data too big... push address
	sSerialCommandParam ret_temp, ret_ref;
	if ((type->Size > 4) && (!type->IsArray)){
		//add_temp(type, ret_temp);
		AddReference(d, /*ret_temp*/ CompilerFunctionReturn, TypePointer, ret_ref);
		//add_ref();
		//add_cmd(inst_lea, KindRegister, (char*)RegEaxCompilerFunctionReturn.kind, CompilerFunctionReturn.param);
	}

	// grow stack (down) for local variables of the calling function
//	add_cmd(, - d->cur_func->_VarSize - LocalOffset - 8);
	int dp = 0;

	// push parameters onto stack
	for (int p=CompilerFunctionParam.num-1;p>=0;p--){
		if (CompilerFunctionParam[p].type){
			int s = mem_align(CompilerFunctionParam[p].type->Size);
			for (int j=0;j<s/4;j++)
				add_cmd(d, inst_push, param_shift(CompilerFunctionParam[p], s - 4 - j * 4));
			dp += s;
		}
	}

#ifdef NIX_IDE_VCS
	// more than 4 byte have to be returned -> give return address as very last parameter!
	if (type->Size > 4)
		add_cmd(inst_push, ret_ref; // nachtraegliche eSP-Korrektur macht die Funktion
#endif
	
	// _cdecl: Klassen-Instanz als ersten Parameter push'en
	if (CompilerFunctionInstance.type){
		add_cmd(d, inst_push, CompilerFunctionInstance);
		dp += 4;
	}
	
#ifndef NIX_IDE_VCS
	// more than 4 byte have to be returned -> give return address as very first parameter!
	if (type->Size > 4)
		add_cmd(d, inst_push, ret_ref); // nachtraegliche eSP-Korrektur macht die Funktion
#endif
	
	add_cmd(d, inst_call, param_const(TypePointer, func)); // the actual call
	// function pointer will be shifted later...

	//dp += 8; // esp, eip
	if (dp > 127)
		add_cmd(d, inst_add, param_reg(TypePointer, RegEsp), param_const(TypeInt, (void*)dp));
	else if (dp > 0)
		add_cmd(d, inst_add_b, param_reg(TypePointer, RegEsp), param_const(TypeInt, (void*)dp));

	// return > 4b already got copied to [ret] by the function!
	if (type != TypeVoid)
		if (type->Size <= 4){
			if (type == TypeFloat)
				add_cmd(d, inst_fstp, CompilerFunctionReturn);
			else if (type->Size == 1){
				add_cmd(d, inst_mov_b, CompilerFunctionReturn, param_reg(type, RegAl));
				add_reg_channel(d, RegEax, d->cmd.num - 2, d->cmd.num - 1);
			}else{
				add_cmd(d, inst_mov, CompilerFunctionReturn, param_reg(type, RegEax));
				add_reg_channel(d, RegEax, d->cmd.num - 2, d->cmd.num - 1);
			}
		}

	// clean up for next call
	CompilerFunctionParam.clear();
	CompilerFunctionReturn.type = NULL;
	CompilerFunctionInstance.type = NULL;
	msg_db_l(4);
}


// creates res...
void AddReference(SerializerData *d, sSerialCommandParam &param, sType *type, sSerialCommandParam &ret)
{
	msg_db_r("AddReference", 3);
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
		add_temp(d, type, ret);
		add_cmd(d, inst_lea, param_reg(type, RegEax), param);
		add_cmd(d, inst_mov, ret, param_reg(type, RegEax));
		add_reg_channel(d, RegEax, d->cmd.num - 2, d->cmd.num - 1);
	}
	msg_db_l(3);
}

void AddDereference(SerializerData *d, sSerialCommandParam &param, sSerialCommandParam &ret, sType *force_type = NULL)
{
	msg_db_r("AddDereference", 4);
	/*add_temp(TypePointer, ret);
	sSerialCommandParam temp;
	add_temp(TypePointer, temp);
	add_cmd(inst_mov, temp, param);
	temp.kind = KindDerefVarTemp;
	add_cmd(inst_mov, ret, temp);*/
	if (param.kind == KindVarTemp){
		deref_temp(param, ret);
	}else if (param.kind == KindRegister){
		ret = param;
		ret.kind = KindDerefRegister;
		ret.type = force_type ? force_type : get_subtype(param.type);
		ret.shift = 0;
	}else{
		//msg_error(string("unhandled deref ", Kind2Str(param.kind)));
		sSerialCommandParam temp;
		add_temp(d, param.type, temp);
		add_cmd(d, inst_mov, temp, param);
		deref_temp(temp, ret);
	}
	msg_db_l(4);
}

// create data for a (function) parameter
//   and compile its command if the parameter is executable itself
void CScript::SerializeParameter(SerializerData *d, sCommand *link, int level, int index, sSerialCommandParam &p)
{
	msg_db_r("SerializeParameter", 4);
	p.kind = link->Kind;
	p.type = link->Type;
	p.p = NULL;
	p.shift = 0;
	//sType *rt=link->;
	//so(Kind2Str(link->Kind));
	if (link->Kind == KindVarFunction){
		so(" -var-func");
		if (pre_script->FlagCompileOS)
			p.p = (char*)((long)func[link->LinkNr] - (long)&Opcode[0] + ((sAsmMetaInfo*)pre_script->AsmMetaInfo)->CodeOrigin);
		else
			p.p = (char*)func[link->LinkNr];
		p.kind = KindVarGlobal;
	}else if (link->Kind == KindMemory){
		so(" -mem");
		p.p = (char*)link->LinkNr;
		p.kind = KindVarGlobal;
	}else if (link->Kind == KindAddress){
		so(" -addr");
		p.p = (char*)&link->LinkNr;
		p.kind = KindRefToConst;
	}else if (link->Kind == KindVarGlobal){
		so(" -global");
		if (link->script)
			p.p = link->script->g_var[link->LinkNr];
		else
			p.p = g_var[link->LinkNr];
	}else if (link->Kind == KindVarLocal){
		so(" -local");
		p.p = (char*)(long)d->cur_func->Var[link->LinkNr]._Offset;
	}else if (link->Kind == KindLocalMemory){
		so(" -local mem");
		p.p = (char*)link->LinkNr;
		p.kind = KindVarLocal;
	}else if (link->Kind == KindLocalAddress){
		so(" -local addr");
		sSerialCommandParam param;
		param.p = (char*)link->LinkNr;
		param.kind = KindVarLocal;
		param.type = TypePointer;
		param.shift = 0;

		AddReference(d, param, link->Type, p);
	}else if (link->Kind == KindVarExternal){
		so(" -external-var");
		p.p = (char*)PreExternalVar[link->LinkNr].Pointer;
		p.kind = KindVarGlobal;
		if (!p.p){
			DoErrorLink(format("external variable is not linkable: %s",PreExternalVar[link->LinkNr].Name.c_str()));
			_return_(4,);
		}
	}else if (link->Kind == KindConstant){
		so(" -const");
		if ((UseConstAsGlobalVar) || (pre_script->FlagCompileOS))
			p.kind = KindVarGlobal;
		else
			p.kind = KindRefToConst;
		p.p = cnst[link->LinkNr];
	}else if ((link->Kind==KindOperator) || (link->Kind==KindFunction) || (link->Kind==KindCompilerFunction)){
		p = SerializeCommand(d, link, level, index);
		if (Error)	_return_(4,);
	}else if (link->Kind == KindReference){
		//so(Kind2Str(link->Meta->Kind));
		so(" -ref");
		sSerialCommandParam param;
		SerializeParameter(d, link->Param[0], level, index, param);
		if (Error)	_return_(4,);
		//printf("%d  -  %s\n",pk,Kind2Str(pk));
		AddReference(d, param, link->Type, p);
		if (Error)	_return_(4,);
	}else if (link->Kind == KindDereference){
		so(" -deref...");
		sSerialCommandParam param;
		SerializeParameter(d, link->Param[0], level, index, param);
		if (Error)	_return_(4,);
		/*if ((param.kind == KindVarLocal) || (param.kind == KindVarGlobal)){
			p.type = param.type->SubType;
			if (param.kind == KindVarLocal)		p.kind = KindRefToLocal;
			if (param.kind == KindVarGlobal)	p.kind = KindRefToGlobal;
			p.p = param.p;
		}*/
		AddDereference(d, param, p);
	}else
		_do_error_int_("unexpected type of parameter: " + Kind2Str(link->Kind), 4,);
	msg_db_l(4);
}


void CScript::SerializeOperator(SerializerData *d, sCommand *com, sSerialCommandParam *param, sSerialCommandParam &ret)
{
	msg_db_r("SerializeOperator", 4);
	switch(com->LinkNr){
		case OperatorIntAssign:
		case OperatorFloatAssign:
		case OperatorPointerAssign:
			add_cmd(d, inst_mov, param[0], param[1]);
			break;
		case OperatorCharAssign:
		case OperatorBoolAssign:
			add_cmd(d, inst_mov_b, param[0], param[1]);
			break;
		case OperatorClassAssign:
			for (int i=0;i<signed(com->Param[0]->Type->Size)/4;i++)
				add_cmd(d, inst_mov, param_shift(param[0], i * 4), param_shift(param[1], i * 4));
			for (int i=4*signed(com->Param[0]->Type->Size/4);i<signed(com->Param[0]->Type->Size);i++)
				add_cmd(d, inst_mov, param_shift(param[0], i), param_shift(param[1], i));
			break;
// string   TODO: create own code!
		case OperatorCStringAssignAA:
			AddFuncParam(param[0]);
			AddFuncParam(param[1]);
			AddFunctionCall(d, (void*)&strcpy);
			break;
		case OperatorCStringAddAAS:
			AddFuncParam(param[0]);
			AddFuncParam(param[1]);
			AddFunctionCall(d, (void*)&strcat);
			break;
		case OperatorCStringAddAA:{
			sSerialCommandParam ret_ref;
			AddReference(d, ret, TypePointer, ret_ref);
			AddFuncParam(ret_ref);
			AddFuncParam(param[0]);
			AddFunctionCall(d, (void*)&strcpy);
			AddFuncParam(ret_ref);
			AddFuncParam(param[1]);
			AddFunctionCall(d, (void*)&strcat);
			}break;
		case OperatorCStringEqualAA:
		case OperatorCStringNotEqualAA:
			AddFuncParam(param[0]);
			AddFuncParam(param[1]);
			AddFunctionCall(d, (void*)&strcmp); // well... return in eax...

			add_cmd(d, inst_cmp, p_eax_int, param_const(TypeInt, (void*)0x0));
			if (com->LinkNr == OperatorCStringEqualAA)
				add_cmd(d, inst_setz_b, p_al_bool);
			if (com->LinkNr==OperatorCStringNotEqualAA)
				add_cmd(d, inst_setnz_b, p_al_bool);
			add_cmd(d, inst_mov_b, ret, p_al_bool);
			add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 1);
			break;
// int
		case OperatorIntAddS:
			add_cmd(d, inst_add, param[0], param[1]);
			break;
		case OperatorIntSubtractS:
			add_cmd(d, inst_sub, param[0], param[1]);
			break;
		case OperatorIntMultiplyS:
			add_cmd(d, inst_imul, param[0], param[1]);
			break;
		case OperatorIntDivideS:
			add_cmd(d, inst_mov, p_eax_int, param[0]);
			add_cmd(d, inst_mov, param_reg(TypeInt, RegEdx), p_eax_int);
			add_cmd(d, inst_sar, param_reg(TypeInt, RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(d, inst_idiv, p_eax_int, param[1]);
			add_cmd(d, inst_mov, param[0], p_eax_int);
			add_reg_channel(d, RegEax, d->cmd.num - 5, d->cmd.num - 1);
			add_reg_channel(d, RegEdx, d->cmd.num - 2, d->cmd.num - 2);
			break;
		case OperatorIntAdd:
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_add, ret, param[1]);
			break;
		case OperatorIntSubtract:
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_sub, ret, param[1]);
			break;
		case OperatorIntMultiply:
			add_cmd(d, inst_mov, p_eax_int, param[0]);
			add_cmd(d, inst_imul, p_eax_int, param[1]);
			add_cmd(d, inst_mov, ret, p_eax_int);
			add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 1);
			break;
		case OperatorIntDivide:
			add_cmd(d, inst_mov, p_eax_int, param[0]);
			add_cmd(d, inst_mov, param_reg(TypeInt, RegEdx), p_eax_int);
			add_cmd(d, inst_sar, param_reg(TypeInt, RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(d, inst_idiv, p_eax_int, param[1]);
			add_cmd(d, inst_mov, ret, p_eax_int);
			add_reg_channel(d, RegEax, d->cmd.num - 5, d->cmd.num - 1);
			add_reg_channel(d, RegEdx, d->cmd.num - 2, d->cmd.num - 2);
			break;
		case OperatorIntModulo:
			add_cmd(d, inst_mov, p_eax_int, param[0]);
			add_cmd(d, inst_mov, param_reg(TypeInt, RegEdx), p_eax_int);
			add_cmd(d, inst_sar, param_reg(TypeInt, RegEdx), param_const(TypeChar, (void*)0x1f));
			add_cmd(d, inst_idiv, p_eax_int, param[1]);
			add_cmd(d, inst_mov, ret, param_reg(TypeInt, RegEdx));
			add_reg_channel(d, RegEax, d->cmd.num - 5, d->cmd.num - 2);
			add_reg_channel(d, RegEdx, d->cmd.num - 2, d->cmd.num - 1);
			break;
		case OperatorIntEqual:
		case OperatorIntNotEqual:
		case OperatorIntGreater:
		case OperatorIntGreaterEqual:
		case OperatorIntSmaller:
		case OperatorIntSmallerEqual:
		case OperatorPointerEqual:
		case OperatorPointerNotEqual:
			add_cmd(d, inst_cmp, param[0], param[1]);
			if (com->LinkNr==OperatorIntEqual)			add_cmd(d, inst_setz_b, ret);
			if (com->LinkNr==OperatorIntNotEqual)		add_cmd(d, inst_setnz_b, ret);
			if (com->LinkNr==OperatorIntGreater)		add_cmd(d, inst_setnle_b, ret);
			if (com->LinkNr==OperatorIntGreaterEqual)	add_cmd(d, inst_setnl_b, ret);
			if (com->LinkNr==OperatorIntSmaller)		add_cmd(d, inst_setl_b, ret);
			if (com->LinkNr==OperatorIntSmallerEqual)	add_cmd(d, inst_setle_b, ret);
			if (com->LinkNr==OperatorPointerEqual)		add_cmd(d, inst_setz_b, ret);
			if (com->LinkNr==OperatorPointerNotEqual)	add_cmd(d, inst_setnz_b, ret);
			break;
		case OperatorIntBitAnd:
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_and, ret, param[1]);
			break;
		case OperatorIntBitOr:
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_or, ret, param[1]);
			break;
		case OperatorIntShiftRight:
			add_cmd(d, inst_mov, param_reg(TypeInt, RegEcx), param[1]);
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_shr, ret, param_reg(TypeChar, RegCl));
			add_reg_channel(d, RegEcx, d->cmd.num - 3, d->cmd.num - 1);
			break;
		case OperatorIntShiftLeft:
			add_cmd(d, inst_mov, param_reg(TypeInt, RegEcx), param[1]);
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_shl, ret, param_reg(TypeChar, RegCl));
			add_reg_channel(d, RegEcx, d->cmd.num - 3, d->cmd.num - 1);
			break;
		case OperatorIntNegate:
			add_cmd(d, inst_mov, ret, param_const(TypeInt, (void*)0x0));
			add_cmd(d, inst_sub, ret, param[0]);
			break;
		case OperatorIntIncrease:
			add_cmd(d, inst_add, param[0], param_const(TypeInt, (void*)0x1));
			break;
		case OperatorIntDecrease:
			add_cmd(d, inst_sub, param[0], param_const(TypeInt, (void*)0x1));
			break;
// float
		case OperatorFloatAddS:
		case OperatorFloatSubtractS:
		case OperatorFloatMultiplyS:
		case OperatorFloatDivideS:
			add_cmd(d, inst_fld, param[0]);
			if (com->LinkNr==OperatorFloatAddS)			add_cmd(d, inst_fadd, param[1]);
			if (com->LinkNr==OperatorFloatSubtractS)	add_cmd(d, inst_fsub, param[1]);
			if (com->LinkNr==OperatorFloatMultiplyS)	add_cmd(d, inst_fmul, param[1]);
			if (com->LinkNr==OperatorFloatDivideS)		add_cmd(d, inst_fdiv, param[1]);
			add_cmd(d, inst_fstp, param[0]);
			break;
		case OperatorFloatAdd:
		case OperatorFloatSubtract:
		case OperatorFloatMultiply:
		case OperatorFloatDivide:
			add_cmd(d, inst_fld, param[0]);
			if (com->LinkNr==OperatorFloatAdd)			add_cmd(d, inst_fadd, param[1]);
			if (com->LinkNr==OperatorFloatSubtract)		add_cmd(d, inst_fsub, param[1]);
			if (com->LinkNr==OperatorFloatMultiply)		add_cmd(d, inst_fmul, param[1]);
			if (com->LinkNr==OperatorFloatDivide)		add_cmd(d, inst_fdiv, param[1]);
			add_cmd(d, inst_fstp, ret);
			break;
		case OperatorFloatMultiplyFI:
			add_cmd(d, inst_fild, param[1]);
			add_cmd(d, inst_fmul, param[0]);
			add_cmd(d, inst_fstp, ret);
			break;
		case OperatorFloatMultiplyIF:
			add_cmd(d, inst_fild, param[0]);
			add_cmd(d, inst_fmul, param[1]);
			add_cmd(d, inst_fstp, ret);
			break;
		case OperatorFloatEqual:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(d, inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(d, inst_setz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 4, d->cmd.num - 2);
			break;
		case OperatorFloatNotEqual:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(d, inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(d, inst_setnz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 4, d->cmd.num - 2);
			break;
		case OperatorFloatGreater:
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_test_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(d, inst_setz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 2);
			break;
		case OperatorFloatGreaterEqual:
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_test_b, p_ah, param_const(TypeChar, (void*)0x05));
			add_cmd(d, inst_setz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 2);
			break;
		case OperatorFloatSmaller:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_test_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(d, inst_setz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 2);
			break;
		case OperatorFloatSmallerEqual:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_test_b, p_ah, param_const(TypeChar, (void*)0x05));
			add_cmd(d, inst_setz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 2);
			break;
		case OperatorFloatNegate:
			add_cmd(d, inst_mov, ret, param[0]);
			add_cmd(d, inst_xor, ret, param_const(TypeInt, (void*)0x80000000));
			break;
// complex
		case OperatorComplexAddS:
		case OperatorComplexSubtractS:
		//case OperatorComplexMultiplySCF:
		//case OperatorComplexDivideS:
			add_cmd(d, inst_fld, param[0]);
			if (com->LinkNr == OperatorComplexAddS)			add_cmd(d, inst_fadd, param[1]);
			if (com->LinkNr == OperatorComplexSubtractS)	add_cmd(d, inst_fsub, param[1]);
			add_cmd(d, inst_fstp, param[0]);
			add_cmd(d, inst_fld, param_shift(param[0], 4));
			if (com->LinkNr == OperatorComplexAddS)			add_cmd(d, inst_fadd, param_shift(param[1], 4));
			if (com->LinkNr == OperatorComplexSubtractS)	add_cmd(d, inst_fsub, param_shift(param[1], 4));
			add_cmd(d, inst_fstp, param_shift(param[0], 4));
			break;
		case OperatorComplexAdd:
		case OperatorComplexSubtract:
//		case OperatorFloatMultiply:
//		case OperatorFloatDivide:
			add_cmd(d, inst_fld, param[0]);
			if (com->LinkNr == OperatorComplexAdd)		add_cmd(d, inst_fadd, param[1]);
			if (com->LinkNr == OperatorComplexSubtract)	add_cmd(d, inst_fsub, param[1]);
			add_cmd(d, inst_fstp, ret);
			add_cmd(d, inst_fld, param_shift(param[0], 4));
			if (com->LinkNr == OperatorComplexAdd)		add_cmd(d, inst_fadd, param_shift(param[1], 4));
			if (com->LinkNr == OperatorComplexSubtract)	add_cmd(d, inst_fsub, param_shift(param[1], 4));
			add_cmd(d, inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexMultiply:
			// r.x = a.y * b.y
			add_cmd(d, inst_fld, param_shift(param[0], 4));
			add_cmd(d, inst_fmul, param_shift(param[1], 4));
			add_cmd(d, inst_fstp, ret);
			// r.x = a.x * b.x - r.x
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fmul, param[1]);
			add_cmd(d, inst_fsub, ret);
			add_cmd(d, inst_fstp, ret);
			// r.y = a.y * b.x
			add_cmd(d, inst_fld, param_shift(param[0], 4));
			add_cmd(d, inst_fmul, param[1]);
			add_cmd(d, inst_fstp, param_shift(ret, 4));
			// r.y += a.x * b.y
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fmul, param_shift(param[1], 4));
			add_cmd(d, inst_fadd, param_shift(ret, 4));
			add_cmd(d, inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexMultiplyFC:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fmul, param[1]);
			add_cmd(d, inst_fstp, ret);
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fmul, param_shift(param[1], 4));
			add_cmd(d, inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexMultiplyCF:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fmul, param[1]);
			add_cmd(d, inst_fstp, ret);
			add_cmd(d, inst_fld, param_shift(param[0], 4));
			add_cmd(d, inst_fmul, param[1]);
			add_cmd(d, inst_fstp, param_shift(ret, 4));
			break;
		case OperatorComplexEqual:
			add_cmd(d, inst_fld, param[0]);
			add_cmd(d, inst_fld, param[1]);
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(d, inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(d, inst_setz_b, ret);
			add_reg_channel(d, RegEax, d->cmd.num - 4, d->cmd.num - 2);
			add_cmd(d, inst_fld, param_shift(param[0], 4));
			add_cmd(d, inst_fld, param_shift(param[1], 4));
			add_cmd(d, inst_fucompp, p_st0, p_st1);
			add_cmd(d, inst_fnstsw, p_ax);
			add_cmd(d, inst_and_b, p_ah, param_const(TypeChar, (void*)0x45));
			add_cmd(d, inst_cmp_b, p_ah, param_const(TypeChar, (void*)0x40));
			add_cmd(d, inst_setz_b, p_ah);
			add_cmd(d, inst_and_b, ret, p_ah);
			add_reg_channel(d, RegEax, d->cmd.num - 5, d->cmd.num - 1);
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
			add_cmd(d, inst_cmp_b, param[0], param[1]);
			if ((com->LinkNr == OperatorCharEqual) || (com->LinkNr == OperatorBoolEqual))
				add_cmd(d, inst_setz_b, ret);
			else if ((com->LinkNr ==OperatorCharNotEqual) || (com->LinkNr == OperatorBoolNotEqual))
				add_cmd(d, inst_setnz_b, ret);
			else if (com->LinkNr == OperatorBoolGreater)		add_cmd(d, inst_setnle_b, ret);
			else if (com->LinkNr == OperatorBoolGreaterEqual)	add_cmd(d, inst_setnl_b, ret);
			else if (com->LinkNr == OperatorBoolSmaller)		add_cmd(d, inst_setl_b, ret);
			else if (com->LinkNr == OperatorBoolSmallerEqual)	add_cmd(d, inst_setle_b, ret);
			break;
		case OperatorBoolAnd:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_and_b, ret, param[1]);
			break;
		case OperatorBoolOr:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_or_b, ret, param[1]);
			break;
		case OperatorCharAddS:
			add_cmd(d, inst_add_b, param[0], param[1]);
			break;
		case OperatorCharSubtractS:
			add_cmd(d, inst_sub_b, param[0], param[1]);
			break;
		case OperatorCharAdd:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_add_b, ret, param[1]);
			break;
		case OperatorCharSubtract:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_sub_b, ret, param[1]);
			break;
		case OperatorCharBitAnd:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_and_b, ret, param[1]);
			break;
		case OperatorCharBitOr:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_or_b, ret, param[1]);
			break;
		case OperatorBoolNegate:
			add_cmd(d, inst_mov_b, ret, param[0]);
			add_cmd(d, inst_xor_b, ret, param_const(TypeBool, (char*)0x1));
			break;
		case OperatorCharNegate:
			add_cmd(d, inst_mov_b, ret, param_const(TypeChar, (char*)0x0));
			add_cmd(d, inst_sub_b, ret, param[0]);
			break;
// vector
		case OperatorVectorAddS:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fadd, param_shift(param[1], i * 4));
				add_cmd(d, inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorMultiplyS:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fmul, param[1]);
				add_cmd(d, inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorDivideS:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fdiv, param[1]);
				add_cmd(d, inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorSubtractS:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fsub, param_shift(param[1], i * 4));
				add_cmd(d, inst_fstp, param_shift(param[0], i * 4));
			}
			break;
		case OperatorVectorAdd:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fadd, param_shift(param[1], i * 4));
				add_cmd(d, inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorSubtract:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fsub, param_shift(param[1], i * 4));
				add_cmd(d, inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorMultiplyVF:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fmul, param[1]);
				add_cmd(d, inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorMultiplyFV:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param[0]);
				add_cmd(d, inst_fmul, param_shift(param[1], i * 4));
				add_cmd(d, inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorDivideVF:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_fld, param_shift(param[0], i * 4));
				add_cmd(d, inst_fdiv, param[1]);
				add_cmd(d, inst_fstp, param_shift(ret, i * 4));
			}
			break;
		case OperatorVectorNegate:
			for (int i=0;i<3;i++){
				add_cmd(d, inst_mov, param_shift(ret, i * 4), param_shift(param[0], i * 4));
				add_cmd(d, inst_xor, param_shift(ret, i * 4), param_const(TypeInt, (void*)0x80000000));
			}
			break;
		default:
			DoErrorInternal("unimplemented operator: " + Operator2Str(pre_script, com->LinkNr));
	}

//	if (AsmError)
//		DoErrorInternal("asm error");
	msg_db_l(4);
}


sSerialCommandParam CScript::SerializeCommand(SerializerData *d, sCommand *com, int level, int index)
{
	msg_db_r("SerializeCommand", 4);
	//so(Kind2Str(com->Kind));
	sSerialCommandParam param[SCRIPT_MAX_PARAMS];
	sSerialCommandParam ret;// = (char*)( -add_temp_var(s) - d->cur_func->_VarSize);
	add_temp(d, com->Type, ret);
	//so(d2h((char*)&ret,4,false));
	//so(string2("return: %d/%d/%d", com->Type->Size, LocalOffset, LocalOffset));

	// compile parameters
	for (int p=0;p<com->NumParams;p++){
		SerializeParameter(d, com->Param[p], level, index, param[p]);
		if (Error) _return_(4, ret);
	}

	// class function -> compile instance
	bool is_class_function = false;
	if (com->Kind == KindCompilerFunction)
		if (PreCommand[com->LinkNr].IsClassFunction)
			is_class_function = true;
	if (com->Kind == KindFunction){
		if (com->script){
			if (com->script->pre_script->Function[com->LinkNr]->Class)
				is_class_function = true;
		}else{
			if (pre_script->Function[com->LinkNr]->Class)
				is_class_function = true;
		}
	}
	sSerialCommandParam instance = {-1, NULL, NULL};
	if (is_class_function){
		so("member");
		SerializeParameter(d, com->Instance, level, index, instance);
		if (Error)	_return_(4, ret);
		so(Kind2Str(instance.kind));
		// super_array automatically referenced...
	}

	    
	if (com->Kind == KindOperator){
		//so("---operator");
		SerializeOperator(d, com, param, ret);
		if (Error)
			_return_(4, ret);
		
	}else if ((com->Kind == KindCompilerFunction) || (com->Kind == KindFunction)){
		//so("---func");
		void *fp = NULL;
		string name;
		if (com->Kind == KindFunction){ // own script Function
			so("Funktion!!!");
			if (com->script){
				//so(com->LinkNr);
				so("    extern!!!");
				fp = (void*)com->script->func[com->LinkNr];
			}else{
				fp = (void*)func[com->LinkNr];
			}
			so("   -ok");
		}else{ // compiler function
			fp = PreCommand[com->LinkNr].Func;
			name = PreCommand[com->LinkNr].Name;
		}
		if (fp){ // a real function

			for (int p=0;p<com->NumParams;p++)
				AddFuncParam(param[p]);
			
			AddFuncReturn(ret);
			
			if (is_class_function)
				AddFuncInstance(instance);
			
			AddFunctionCall(d, fp);
			
		}else if (PreCommand[com->LinkNr].IsSpecial){
			switch(com->LinkNr){
				/*case CommandSine:
					break;*/
				case CommandIf:{
					// cmp;  jz m;  -block-  m;
					add_cmd(d, inst_cmp_b, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_true = add_marker_after_command(d, level, index + 1);
					add_cmd(d, inst_jz, param_marker(m_after_true));
					}break;
				case CommandIfElse:{
					// cmp;  jz m1;  -block-  jmp m2;  m1;  -block-  m2;
					add_cmd(d, inst_cmp_b, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_true = add_marker_after_command(d, level, index + 1);
					int m_after_false = add_marker_after_command(d, level, index + 2);
					add_cmd(d, inst_jz, param_marker(m_after_true)); // jz_b ...
					add_jump_after_command(d, level, index + 1, m_after_false); // insert before <m_after_true> is inserted!
					}break;
				case CommandWhile:
				case CommandFor:{
					// m1;  cmp;  jz m2;  -block-             jmp m1;  m2;     (while)
					// m1;  cmp;  jz m2;  -block-  m3;  i++;  jmp m1;  m2;     (for)
					int m_before_while = d->NumMarkers ++;
					add_marker(d, m_before_while);
					move_last_cmd(d, d->LastCommandSize); // has to be before evaluating the parameter!
					add_cmd(d, inst_cmp_b, param[0], param_const(TypeBool, (void*)0x0));
					int m_after_while = add_marker_after_command(d, level, index + 1);
					add_cmd(d, inst_jz, param_marker(m_after_while));
					add_jump_after_command(d, level, index + 1, m_before_while); // insert before <m_after_while> is inserted!

					int m_continue = m_before_while;
					if (com->LinkNr == CommandFor){
						// NextCommand is a block!
						if (d->NextCommand->Kind != KindBlock)
							_do_error_int_("command block in \"for\" loop missing", 4, ret);
						m_continue = add_marker_after_command(d, level + 1, pre_script->Block[d->NextCommand->LinkNr]->Command.num - 2);
					}
					sLoopData l = {m_continue, m_after_while, level, index};
					d->LoopData.add(l);
					}break;
				case CommandBreak:
					add_cmd(d, inst_jmp, param_marker(d->LoopData.back().marker_break));
					break;
				case CommandContinue:
					add_cmd(d, inst_jmp, param_marker(d->LoopData.back().marker_continue));
					break;
				case CommandReturn:
					if (com->NumParams > 0){
						if (d->cur_func->Type->Size > 4){ // we already got a return address in [ebp+0x08] (> 4 byte)
							int s = mem_align(d->cur_func->Type->Size);

							// slow
							/*sSerialCommandParam p, p_deref;
							p.kind = KindVarLocal;
							p.type = TypeReg32;
							p.p = (char*) 0x8;
							p.shift = 0;
							for (int j=0;j<s/4;j++){
								AddDereference(p, p_deref);
								add_cmd(d, inst_mov, p_deref, param_shift(param[0], j * 4));
								add_cmd(d, inst_add, p, param_const(TypeInt, (void*)0x4));
							}*/

							// test
							sSerialCommandParam p_edx = param_reg(TypeReg32, RegEdx), p_deref_edx;
							sSerialCommandParam p_ret_addr;
							p_ret_addr.kind = KindVarLocal;
							p_ret_addr.type = TypeReg32;
							p_ret_addr.p = (char*)0x8;
							p_ret_addr.shift = 0;
							int c_0 = d->cmd.num;
							add_cmd(d, inst_mov, p_edx, p_ret_addr);
							AddDereference(d, p_edx, p_deref_edx, TypeReg32);
							for (int j=0;j<s/4;j++)
								add_cmd(d, inst_mov, param_shift(p_deref_edx, j * 4), param_shift(param[0], j * 4));
							add_reg_channel(d, RegEdx, c_0, d->cmd.num - 1);
						}else{ // store return directly in eax / fpu stack (4 byte)
							if (d->cur_func->Type == TypeFloat)
								add_cmd(d, inst_fld, param[0]);
							else if (d->cur_func->Type->Size == 1)
								add_cmd(d, inst_mov_b, param_reg(d->cur_func->Type, RegAl), param[0]);
							else
								add_cmd(d, inst_mov, param_reg(d->cur_func->Type, RegEax), param[0]);
						}
					}
					add_cmd(d, inst_leave);
					if (d->cur_func->Type->Size > 4)
						add_cmd(d, inst_ret, param_const(TypeInt, (void*)4));
					else
						add_cmd(d, inst_ret);
					break;
				case CommandWaitOneFrame:
				case CommandWait:
				case CommandWaitRT:{
				// set waiting state
					// GlobalWaitingMode = mode
					// GlobalWaitingTime = time
					sSerialCommandParam p_mode = param_global(TypeInt, &GlobalWaitingMode);
					sSerialCommandParam p_ttw = param_global(TypeFloat, &GlobalTimeToWait);
					if (com->LinkNr == CommandWaitOneFrame){
						add_cmd(d, inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeRT));
						add_cmd(d, inst_mov, p_ttw, param_const(TypeFloat, NULL));
					}else if (com->LinkNr == CommandWait){
						add_cmd(d, inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeGT));
						add_cmd(d, inst_mov, p_ttw, param[0]);
					}else if (com->LinkNr == CommandWaitRT){
						add_cmd(d, inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeRT));
						add_cmd(d, inst_mov, p_ttw, param[0]);
					}
					
				// save script state
					// stack[ -8] = ebp
					// stack[-12] = esp
					// stack[-16] = eip
					add_cmd(d, inst_mov, p_eax, param_const(TypePointer, &Stack[ScriptStackSize-8]));
					add_cmd(d, inst_mov, p_deref_eax, param_reg(TypeReg32, RegEbp));
					add_cmd(d, inst_mov, p_eax, param_const(TypePointer, &Stack[ScriptStackSize-12]));
					add_cmd(d, inst_mov, p_deref_eax, param_reg(TypeReg32, RegEsp));
					add_cmd(d, inst_mov, param_reg(TypeReg32, RegEsp), param_const(TypePointer, &Stack[ScriptStackSize-12]));
					add_cmd(d, inst_call, param_const(TypePointer, NULL)); // push eip
				// load return
					// mov esp, &stack[-4]
					// pop esp
					// mov ebp, esp
					// leave
					// ret
					add_cmd(d, inst_mov, param_reg(TypeReg32, RegEsp), param_const(TypePointer, &Stack[ScriptStackSize-4])); // start of the script stack
					add_cmd(d, inst_pop, param_reg(TypeReg32, RegEsp)); // old stackpointer (real program)
					add_cmd(d, inst_mov, param_reg(TypeReg32, RegEbp), param_reg(TypeReg32, RegEsp));
					add_cmd(d, inst_leave);
					add_cmd(d, inst_ret);
				// here comes the "waiting"...

				// reload script state (eip already loaded)
					// ebp = &stack[-8]
					// esp = &stack[-12]
					// GlobalWaitingMode = WaitingModeNone
					add_cmd(d, inst_mov, p_eax, param_const(TypePointer, &Stack[ScriptStackSize-8]));
					add_cmd(d, inst_mov, param_reg(TypeReg32, RegEbp), p_deref_eax);
					add_cmd(d, inst_mov, p_eax, param_const(TypePointer, &Stack[ScriptStackSize-12]));
					add_cmd(d, inst_mov, param_reg(TypeReg32, RegEsp), p_deref_eax);
					add_cmd(d, inst_mov, p_mode, param_const(TypeInt, (void*)WaitingModeNone));
					}break;
				case CommandIntToFloat:
					add_cmd(d, inst_fild, param[0]);
					add_cmd(d, inst_fstp, ret);
					break;
				case CommandFloatToInt:
					// round to nearest...
					//add_cmd(d, inst_fld, param[0]);
					//add_cmd(d, inst_fistp, ret);

					// round to zero...
					sSerialCommandParam t1, t2;
					add_temp(d, TypeInt, t1);
					add_temp(d, TypeInt, t2);
					add_cmd(d, inst_fld, param[0]);
					add_cmd(d, inst_fnstcw, t1);
					add_cmd(d, inst_movzx, p_eax, t1);
					add_cmd(d, inst_mov_b, p_ah, param_const(TypeChar, (void*)0x0c));
					add_cmd(d, inst_mov, t2, p_eax);
					add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 1);
					add_cmd(d, inst_fldcw, t2);
					add_cmd(d, inst_fistp, ret);
					add_cmd(d, inst_fldcw, t1);
					break;
				case CommandIntToChar:
					add_cmd(d, inst_mov, p_eax_int, param[0]);
					add_cmd(d, inst_mov_b, ret, p_al_char);
					add_reg_channel(d, RegEax, d->cmd.num - 2, d->cmd.num - 1);
					break;
				case CommandCharToInt:
					add_cmd(d, inst_mov, p_eax_int, param_const(TypeInt, (void*)0x0));
					add_cmd(d, inst_mov_b, p_al_char, param[0]);
					add_cmd(d, inst_mov, ret, p_eax);
					add_reg_channel(d, RegEax, d->cmd.num - 3, d->cmd.num - 1);
					break;
				case CommandPointerToBool:
					add_cmd(d, inst_cmp, param[0], param_const(TypePointer, NULL));
					add_cmd(d, inst_setnz_b, ret);
					break;
				case CommandAsm:
					add_cmd(d, inst_asm);
					break;
				case CommandRectSet:
					add_cmd(d, inst_mov, param_shift(ret, 12), param[3]);
				case CommandVectorSet:
					add_cmd(d, inst_mov, param_shift(ret, 8), param[2]);
				case CommandComplexSet:
					add_cmd(d, inst_mov, param_shift(ret, 4), param[1]);
					add_cmd(d, inst_mov, param_shift(ret, 0), param[0]);
					break;
				case CommandColorSet:
					add_cmd(d, inst_mov, param_shift(ret, 12), param[0]);
					add_cmd(d, inst_mov, param_shift(ret, 0), param[1]);
					add_cmd(d, inst_mov, param_shift(ret, 4), param[2]);
					add_cmd(d, inst_mov, param_shift(ret, 8), param[3]);
					break;
				default:
 					_do_error_int_("compiler function unimplemented: " + PreCommand[com->LinkNr].Name, 4, ret);
			}
		}else{
			if (PreCommand[com->LinkNr].IsSemiExternal)
	 			DoErrorLink("external function not linkable: " + PreCommand[com->LinkNr].Name);
			else
	 			DoErrorInternal("compiler function not linkable: " + PreCommand[com->LinkNr].Name);
			_return_(4, ret);
		}
	}else if (com->Kind == KindBlock){
		//so("---block");
		SerializeBlock(d, pre_script->Block[com->LinkNr], level + 1);
		if (Error)	_return_(4, ret);
	}else{
		//so("---???");
		//_do_error_(string("type of command is unimplemented (call Michi!): ",Kind2Str(com->Kind)), 4, NULL);
	}
	//so(Kind2Str(com->Kind));
	//msg_ok();
	msg_db_l(4);
	return ret;
}

void FillInDestructors(SerializerData *d, bool from_temp);

void CScript::SerializeBlock(SerializerData *d, sBlock *block, int level)
{
	msg_db_r("SerializeBlock", 4);
	for (int i=0;i<block->Command.num;i++){
		//so(string2("%d - %d",i,block->NumCommands));
		d->StackOffset = d->cur_func->_VarSize;
		d->LastCommandSize = d->cmd.num;
		if (block->Command.num > i + 1)
			d->NextCommand = block->Command[i + 1];

		// serialize
		SerializeCommand(d, block->Command[i], level, i);
		if (Error)	_return_(4,);

		
		FillInDestructors(d, true);
		if (Error)	_return_(4,);

		// any markers / jumps to add?
		for (int j=d->StuffToAdd.num-1;j>=0;j--)
			if ((level == d->StuffToAdd[j].level) && (i == d->StuffToAdd[j].index)){
				if (d->StuffToAdd[j].kind == StuffKindMarker)
					add_marker(d, d->StuffToAdd[j].marker);
				else if (d->StuffToAdd[j].kind == StuffKindJump)
					add_cmd(d, inst_jmp, param_marker(d->StuffToAdd[j].marker));
				d->StuffToAdd.erase(j);
			}

		// end of loop?
		if (d->LoopData.num > 0)
			if ((d->LoopData.back().level == level) && (d->LoopData.back().index == i - 1))
				d->LoopData.pop();
	}
	msg_db_l(4);
}

void _fake_array_init_(DynamicArray *a, int item_size)
{
//	msg_write("f init");
	a->init(item_size);
	/*a->num = 0;
	a->item_size = item_size;
	a->data = NULL;*/
}

void _fake_array_clear_(DynamicArray *a)
{
//	msg_write("f clear");
	a->clear();
}

Array<sSerialCommandParam> InsertedConstructorFunc;
Array<sSerialCommandParam> InsertedConstructorTemp;

void add_cmd_constructor(SerializerData *d, sSerialCommandParam &param, bool is_temp)
{
	foreach(param.type->Function, f){
		if (f->Name == "__init__"){ // TODO test signature "void __init__()"
			sSerialCommandParam inst;
			AddReference(d, param, TypePointer, inst);
			AddFuncInstance(inst);
			void *fp;
			if (f->Kind == KindCompilerFunction)
				fp = PreCommand[f->Nr].Func;
			else if (f->Kind == KindFunction){
				fp = (void*)param.type->Owner->script->func[f->Nr];
				if (!fp)
					msg_error(param.type->Name + ".__init__() unlinkable compiler function!");
			}

			AddFunctionCall(d, fp);
			if (is_temp)
				InsertedConstructorTemp.add(param);
			else
				InsertedConstructorFunc.add(param);
			return;
		}
	}
}

void add_cmd_destructor(SerializerData *d, sSerialCommandParam &param)
{
	foreach(param.type->Function, f)
		if (f->Name == "__delete__"){ // TODO test signature "void __delete__()"
			sSerialCommandParam inst;
			AddReference(d, param, TypePointer, inst);
			AddFuncInstance(inst);
			void *fp;
			if (f->Kind == KindCompilerFunction)
				fp = PreCommand[f->Nr].Func;
			else if (f->Kind == KindFunction){
				fp = (void*)param.type->Owner->script->func[f->Nr];
				if (!fp)
					msg_error(param.type->Name + ".__delete__() unlinkable compiler function!");
			}
			AddFunctionCall(d, fp);
		}
}

void FillInConstructorsFunc(SerializerData *d)
{
	msg_db_r("FillInConstructorsFunc", 4);
	for (int i=0;i<d->cur_func->Var.num;i++)
		/*if (cur_func->Var[i].Type->Element.num > 0)*/{
			sSerialCommandParam param = param_local(d->cur_func->Var[i].Type, d->cur_func->Var[i]._Offset);
			add_cmd_constructor(d, param, false);
		}
	msg_db_l(4);
}

void FillInDestructors(SerializerData *d, bool from_temp)
{
	msg_db_r("FillInDestructors", 4);
	if (from_temp){
		for (int i=0;i<InsertedConstructorTemp.num;i++)
			add_cmd_destructor(d, InsertedConstructorTemp[i]);
		InsertedConstructorTemp.clear();
	}else{
		for (int i=0;i<InsertedConstructorFunc.num;i++)
			add_cmd_destructor(d, InsertedConstructorFunc[i]);
		InsertedConstructorFunc.clear();
	}
	msg_db_l(4);
}

inline int temp_in_cmd(SerializerData *d, int c, int v)
{
	if (d->cmd[c].inst >= inst_marker)
		return 0;
	int r = 0;
	if ((d->cmd[c].p1.kind == KindVarTemp) || (d->cmd[c].p1.kind == KindDerefVarTemp))
		if ((long)d->cmd[c].p1.p == v)
			r = 1 + ((d->cmd[c].p1.kind == KindDerefVarTemp) ? 4 : 0);
	if ((d->cmd[c].p2.kind == KindVarTemp) || (d->cmd[c].p2.kind == KindDerefVarTemp))
		if ((long)d->cmd[c].p2.p == v)
			r += 2 + ((d->cmd[c].p2.kind == KindDerefVarTemp) ? 8 : 0);
	return r;
}

void ScanTempVarUsage(SerializerData *d)
{
	msg_db_r("ScanTempVarUsage", 4);
	foreachi(d->TempVar, v, i){
		v->first = -1;
		v->last = -1;
		v->count = 0;
		for (int c=0;c<d->cmd.num;c++){
			if (temp_in_cmd(d, c, i) > 0){
				v->count ++;
				if (v->first < 0)
					v->first = c;
				v->last = c;
			}
		}
		//so(string2("var %d:   %d - %d", i, v.first, v.last));
	}
	d->TempVarRangesDefined = true;
	msg_db_l(4);
}

inline bool param_is_simple(sSerialCommandParam &p)
{
	return ((p.kind == KindRegister) || (p.kind == KindVarTemp) || (p.kind < 0));
}

inline bool param_combi_allowed(int inst, sSerialCommandParam &p1, sSerialCommandParam &p2)
{
//	if (inst >= inst_marker)
//		return true;
	if ((!param_is_simple(p1)) && (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	GetInstructionParamFlags(inst, r1, w1, r2, w2);
	if ((w1) && (p1.kind == KindConstant))
		return false;
	if ((w2) && (p2.kind == KindConstant))
		return false;
	if ((p1.kind == KindConstant) || (p2.kind == KindConstant))
		if (!GetInstructionAllowConst(inst))
			return false;
	return true;
}

// mov [0x..] [0x...]  ->  mov temp, [0x..]   mov [0x..] temp
/*void CorrectUnallowedParamCombis()
{
	msg_db_r("CorrectCombis", 3);
	for (int i=cmd.num-1;i>=0;i--)
		if (!param_combi_allowed(cmd[i].inst, cmd[i].p1, cmd[i].p2)){
			msg_write(string2("correcting param combi  cmd=%d", i));
			bool mov_first_param = (cmd[i].p2.kind < 0) || (cmd[i].p1.kind == KindRefToConst) || (cmd[i].p1.kind == KindConstant);
			sSerialCommandParam *pp = mov_first_param ? &cmd[i].p1 : &cmd[i].p2;
			sSerialCommandParam temp, p = *pp;
			add_temp(p.type, temp);

			*pp = temp;
			if (p.type->Size == 1)
				add_cmd(d, inst_mov_b, temp, p);
			else
				add_cmd(d, inst_mov, temp, p);
			move_last_cmd(i);
			//so("...ok");
		}
	ScanTempVarUsage();
	msg_db_l(3);
}*/

// mov [0x..] [0x...]  ->  mov eax, [0x..]   mov [0x..] eax    (etc)
void CorrectUnallowedParamCombis(SerializerData *d)
{
	msg_db_r("CorrectCombis", 3);
	for (int i=d->cmd.num-1;i>=0;i--){
		if (d->cmd[i].inst >= inst_marker)
			continue;

		// bad?
		if (param_combi_allowed(d->cmd[i].inst, d->cmd[i].p1, d->cmd[i].p2))
			continue;

		// correct
		so(format("correcting param combi  cmd=%d", i));
		bool mov_first_param = (d->cmd[i].p2.kind < 0) || (d->cmd[i].p1.kind == KindRefToConst) || (d->cmd[i].p1.kind == KindConstant);
		sSerialCommandParam *pp = mov_first_param ? &d->cmd[i].p1 : &d->cmd[i].p2;
		sSerialCommandParam p = *pp;

		if (p.type->Size == 1){
			*pp = param_reg(p.type, RegAl);
			add_cmd(d, inst_mov_b, *pp, p);
		}else{
			*pp = param_reg(p.type, RegEax);
			add_cmd(d, inst_mov, *pp, p);
		}
		move_last_cmd(d, i);
		//so("...ok");
	}
	so("scan");
	ScanTempVarUsage(d);
	msg_db_l(3);
}

inline int find_unused_reg(SerializerData *d, int first, int last, bool allow_eax)
{
	for (int r=0;r<d->MapReg.num;r++)
		if (!is_reg_used_in_interval(d, d->MapReg[r], first, last))
			return d->MapReg[r];
	if (allow_eax)
		if (!is_reg_used_in_interval(d, RegEax, first, last))
			return RegEax;
	return -1;
}

// inst ... [local] ...
// ->      mov reg, local     inst ...[reg]...
inline void solve_deref_temp_local(SerializerData *d, int c, int np, bool is_local)
{
	sSerialCommandParam *pp = (np == 0) ? &d->cmd[c].p1 : &d->cmd[c].p2;
	sSerialCommandParam p = *pp;
	int shift = p.shift;

	sType *type_pointer = is_local ? TypePointer : d->TempVar[(long)p.p].type;
	sType *type_data = p.type;
	
	p.kind = is_local ? KindVarLocal : KindVarTemp;
	p.shift = 0;
	p.type = type_pointer;

	int reg = find_unused_reg(d, c, c, true);
	//so(reg);
	if (reg < 0){
		cur_script->DoErrorInternal("solve_deref_temp_local... no registers available");
		return;
	}
	sSerialCommandParam p_reg = param_reg(type_pointer, reg);
	sSerialCommandParam p_deref_reg;
	p_deref_reg.kind = KindDerefRegister;
	p_deref_reg.p = (char*)reg;
	p_deref_reg.type = type_data;
	p_deref_reg.shift = 0;
	
	*pp = p_deref_reg;
		
	add_cmd(d, inst_mov, p_reg, p);
	move_last_cmd(d, c);
	if (shift > 0){
		add_cmd(d, inst_add, p_reg, param_const(TypeInt, (void*)shift));
		move_last_cmd(d, c + 1);
		add_reg_channel(d, reg, c, c + 2);
	}else
		add_reg_channel(d, reg, c, c + 1);
}

void add_stack_var(sType *type, int first, int last, sSerialCommandParam &p);

#if 0
void ResolveDerefLocal()
{
	msg_db_r("ResolveDerefLocal", 3);
	for (int i=cmd.num-1;i>=0;i--){
		if (cmd[i].inst >= inst_marker)
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
			if (reg < 0){
				cur_script->DoErrorInternal("deref local... both sides... .no registers available");
				msg_db_l(3);
				return;
			}
			sSerialCommandParam p_reg = param_reg(TypeReg32, reg);
			add_reg_channel(reg, i, i); // temp
			
			int reg2 = find_unused_reg(i, i, true);
			so(reg2);
			if (reg2 < 0){
				cur_script->DoErrorInternal("deref local... both sides... .no registers available");
				msg_db_l(3);
				return;
			}
			sSerialCommandParam p_reg2 = param_reg(TypeReg32, reg2);
			sSerialCommandParam p_deref_reg2;
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
			sSerialCommandParam p1 = cmd[i].p1;
			sSerialCommandParam p2 = cmd[i].p2;
			if (p1.shift + p2.shift > 0){
				cur_script->DoErrorInternal("deref local... both sides... shift");
				msg_db_l(3);
				return;
			}
			p1.kind = p2.kind = KindVarLocal;
			cmd[i].p1 = p_deref_reg2;
			cmd[i].p2 = p_reg;
	
			add_cmd(d, inst_mov, p_reg2, p2);
			move_last_cmd(i);
	
			add_cmd(d, inst_mov, p_reg, p_deref_reg2);
			move_last_cmd(i + 1);
	
			add_cmd(d, inst_mov, p_reg2, p1);
			move_last_cmd(i + 2);

			add_reg_channel(reg, i + 1, i + 3);
			add_reg_channel(reg2, i, i + 3);
				
			i += 3;
		}
	}
	msg_db_l(3);
}
#endif


void ResolveDerefTempAndLocal(SerializerData *d)
{
	msg_db_r("ResolveDerefTempAndLocal", 3);
	for (int i=d->cmd.num-1;i>=0;i--){
		if (d->cmd[i].inst >= inst_marker)
			continue;
		bool dl1 = ((d->cmd[i].p1.kind == KindDerefVarLocal) || (d->cmd[i].p1.kind == KindDerefVarTemp));
		bool dl2 = ((d->cmd[i].p2.kind == KindDerefVarLocal) || (d->cmd[i].p2.kind == KindDerefVarTemp));
		if (!(dl1 || dl2))
			continue;

		bool is_local1 = (d->cmd[i].p1.kind == KindDerefVarLocal);
		bool is_local2 = (d->cmd[i].p2.kind == KindDerefVarLocal);
		
		so(format("deref temp/local... cmd=%d", i));
		if (!dl2){
			solve_deref_temp_local(d, i, 0, is_local1);
			i ++;
		}else if (!dl1){
			solve_deref_temp_local(d, i, 1, is_local2);
			i ++;
		}else{
			// hopefully... p2 is read-only

			sType *type_pointer = TypePointer;
			sType *type_data = d->cmd[i].p1.type;

			int reg = find_unused_reg(d, i, i, true);
			so(reg);
			if (reg < 0){
				cur_script->DoErrorInternal("deref local... both sides... .no registers available");
				msg_db_l(3);
				return;
			}
			
			sSerialCommandParam p_reg = param_reg(type_data, reg);
			if (type_data->Size == 1)
				p_reg = param_reg(type_data, reg_smallify(reg));
			add_reg_channel(d, reg, i, i); // temp
			
			int reg2 = find_unused_reg(d, i, i, true);
			so(reg2);
			if (reg2 < 0){
				cur_script->DoErrorInternal("deref temp/local... both sides... .no registers available");
				msg_db_l(3);
				return;
			}
			sSerialCommandParam p_reg2 = param_reg(type_pointer, reg2);
			sSerialCommandParam p_deref_reg2;
			p_deref_reg2.kind = KindDerefRegister;
			p_deref_reg2.p = (char*)reg2;
			p_deref_reg2.type = type_data;
			p_deref_reg2.shift = 0;
			d->RegChannel.pop(); // remove temp reg channel...

			// inst [l1] [l2]
			// ->
			// mov reg2, l2
			//   (add reg2, shift2)
			// mov reg, [reg2]
			// mov reg2, l1
			//   (add reg2, shift1)
			// inst [reg2], reg
			sSerialCommandParam p1 = d->cmd[i].p1;
			sSerialCommandParam p2 = d->cmd[i].p2;
			int shift1 = p1.shift;
			int shift2 = p2.shift;
			p1.shift = p2.shift = 0;
			
			p1.kind = is_local1 ? KindVarLocal : KindVarTemp;
			p2.kind = is_local2 ? KindVarLocal : KindVarTemp;
			p1.type = type_pointer;
			p2.type = type_pointer;
			d->cmd[i].p1 = p_deref_reg2;
			d->cmd[i].p2 = p_reg;
			int cmd_pos = i;

			int r2_first = cmd_pos;
			add_cmd(d, inst_mov, p_reg2, p2);
			move_last_cmd(d, cmd_pos ++);

			if (shift2 > 0){
				add_cmd(d, inst_add, p_reg2, param_const(TypeInt, (void*)shift2));
				move_last_cmd(d, cmd_pos ++);
			}

			int r1_first = cmd_pos;
			if (type_data->Size == 1)
				add_cmd(d, inst_mov_b, p_reg, p_deref_reg2);
			else
				add_cmd(d, inst_mov, p_reg, p_deref_reg2);
			move_last_cmd(d, cmd_pos ++);
	
			add_cmd(d, inst_mov, p_reg2, p1);
			move_last_cmd(d, cmd_pos ++);

			if (shift1 > 0){
				add_cmd(d, inst_add, p_reg2, param_const(TypeInt, (void*)shift1));
				move_last_cmd(d, cmd_pos ++);
			}

			add_reg_channel(d, reg, r1_first, cmd_pos);
			add_reg_channel(d, reg2, r2_first, cmd_pos);
				
			i = cmd_pos;
		}
	}
	msg_db_l(3);
}

bool ParamUntouchedInInterval(SerializerData *d, sSerialCommandParam &p, int first, int last)
{
	// direct usage?
	for (int i=first;i<=last;i++)
		if ((d->cmd[i].p1 == p) || (d->cmd[i].p2 == p))
			return false;
	
	// registers may be more subtle..
	if ((p.kind == KindRegister) || (p.kind == KindDerefRegister)){
		for (int i=first;i<=last;i++){
			
			// call violates all!
			if (d->cmd[i].inst == inst_call)
				return false;

			// div violates eax and edx
			if (d->cmd[i].inst == inst_div)
				if (((long)p.p == RegEdx) || ((long)p.p == RegEax))
					return false;

			// registers used? (may be part of the same meta-register)
			if ((d->cmd[i].p1.kind == KindRegister) || (d->cmd[i].p1.kind == KindDerefRegister))
				if (RegRoot[(long)d->cmd[i].p1.p] == RegRoot[(long)p.p])
					return false;
			if ((d->cmd[i].p2.kind == KindRegister) || (d->cmd[i].p2.kind == KindDerefRegister))
				if (RegRoot[(long)d->cmd[i].p2.p] == RegRoot[(long)p.p])
					return false;
		}
	}
	return true;
}

void SimplifyFPUStack(SerializerData *d)
{
	msg_db_r("SimplifyFPUStack", 3);

// fstp temp
// fld temp
	for (int v=d->TempVar.num-1;v>=0;v--){
		// may only appear two times
		if (d->TempVar[v].count > 2)
			continue;

		// stored then loaded...?
		if ((d->cmd[d->TempVar[v].first].inst != inst_fstp) || (d->cmd[d->TempVar[v].last].inst != inst_fld))
			continue;

		// value still on the stack?
		int d_stack = 0, min_d_stack = 0, max_d_stack = 0;
		for (int i=d->TempVar[v].first + 1;i<d->TempVar[v].last;i++){
			if (d->cmd[i].inst == inst_fld)
				d_stack ++;
			else if (d->cmd[i].inst == inst_fstp)
				d_stack --;
			min_d_stack = min(min_d_stack, d_stack);
			max_d_stack = max(max_d_stack, d_stack);
		}
		if ((d_stack != 0) || (min_d_stack < 0) || (max_d_stack > 5))
			continue;

		// reuse value on the stack
		so(format("fpu (a)  var=%d first=%d last=%d stack: d=%d min=%d max=%d", v, d->TempVar[v].first, d->TempVar[v].last, d_stack, min_d_stack, max_d_stack));
		remove_cmd(d, d->TempVar[v].last);
		remove_cmd(d, d->TempVar[v].first);
		remove_temp_var(d, v);
	}

// fstp temp
// mov xxx, temp
	for (int v=d->TempVar.num-1;v>=0;v--){
		// may only appear two times
		if (d->TempVar[v].count > 2)
			continue;

		// stored then moved...?
		if ((d->cmd[d->TempVar[v].first].inst != inst_fstp) || (d->cmd[d->TempVar[v].last].inst != inst_mov))
			continue;
		if (temp_in_cmd(d, d->TempVar[v].last, v) != 2)
			continue;
		// moved into fstore'able?
		int kind = d->cmd[d->TempVar[v].last].p1.kind;
		if ((kind != KindVarLocal) && (kind != KindVarGlobal) && (kind != KindVarTemp) && (kind != KindDerefVarTemp) && (kind != KindDerefRegister))
		    continue;

		// check, if mov target is used in between
		sSerialCommandParam target = d->cmd[d->TempVar[v].last].p1;
		if (!ParamUntouchedInInterval(d, target, d->TempVar[v].first + 1 ,d->TempVar[v].last - 1))
			continue;
		// ...we are lazy...
		//if (TempVar[v].last - TempVar[v].first != 1)
		//	continue;

		// store directly into target
		so(format("fpu (b)  var=%d first=%d last=%d", v, d->TempVar[v].first, d->TempVar[v].last));
		d->cmd[d->TempVar[v].first].p1 = target;
		move_param(d, target, d->TempVar[v].last, d->TempVar[v].first);
		remove_cmd(d, d->TempVar[v].last);
		remove_temp_var(d, v);
	}
	msg_db_l(3);
}

// remove  mov x,...    mov ...,x   (and similar)
//    (does not affect reg channels)
void SimplifyMovs(SerializerData *d)
{
	// TODO: count > 2 .... first == input && all_other == output?  (only if first == mov (!=eax?)... else count == 2)
	// should take care of fpu simplification (b)...
	
	msg_db_r("SimplifyMovs", 3);
	for (int vi=d->TempVar.num-1;vi>=0;vi--){
		sTempVar &v = d->TempVar[vi];
		
		// may only appear two times
		if (v.count > 2)
			continue;

		// both times in a mov command (or fld as second)
		if (d->cmd[v.first].inst != inst_mov)
			continue;
		int n = d->cmd[v.last].inst;
		bool fld = (n == inst_fld) || (n == inst_fadd) || (n == inst_fadd) || (n == inst_fsub) || (n == inst_fmul) || (n == inst_fdiv);
		if ((d->cmd[v.last].inst != inst_mov) && (!fld))
			continue;
		
		// used as source/target?   no deref?
		if ((temp_in_cmd(d, v.first, vi) != 1) || (temp_in_cmd(d, v.last, vi) != (fld ? 1 : 2)))
			continue;

		// new construction allowed?
		sSerialCommandParam target = d->cmd[v.last].p1;
		sSerialCommandParam source = d->cmd[v.first].p2;
		if (fld){
			if (!param_combi_allowed(d->cmd[v.last].inst, source, d->cmd[v.last].p2))
				continue;
		}else{
			if (!param_combi_allowed(d->cmd[v.last].inst, d->cmd[v.last].p1, source))
				continue;
		}

		// check, if mov source or target are used in between
		if (!ParamUntouchedInInterval(d, target, v.first + 1 ,v.last - 1))
			continue;
		if (!ParamUntouchedInInterval(d, source, v.first + 1 ,v.last - 1))
			continue;
		
		so(format("mov simplification allowed  v=%d first=%d last=%d", vi, v.first, v.last));
		if (fld)
			d->cmd[v.last].p1 = source;
		else
			d->cmd[v.last].p2 = source;
		move_param(d, source, v.first, v.last);
		remove_cmd(d, v.first);
		remove_temp_var(d, vi);
	}

	// TODO: should happen automatically...
	//ScanTempVarUsage();
	//cmd_list_out();


	msg_db_l(3);
}

/*inline void test_reg_usage(int c)
{
	// call -> violates all...
	if (cmd[c].inst == inst_call){
		for (int i=0;i<max_reg;i++)
			RegUsed[i] = true;
		return;
	}
	if ((cmd[c].p1.kind == KindRegister) || (cmd[c].p1.kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p1.p);
	if ((cmd[c].p2.kind == KindRegister) || (cmd[c].p2.kind == KindDerefRegister))
		set_reg_used((long)cmd[c].p2.p);
}*/

void MapTempVarToReg(SerializerData *d, int vi, int reg)
{
	msg_db_r("reg", 4);
	sTempVar &v = d->TempVar[vi];
	so(format("temp=reg:  %d - %d:   tv %d := reg %d", v.first, v.last, vi, reg));

	int reg32 = reg;
	
	// only small register?
	if (v.type->Size == 1){
		if (reg == RegEax)		reg = RegAl;
		else if (reg == RegEdx)	reg = RegDl;
		else if (reg == RegEcx)	reg = RegCl;
		else if (reg == RegEbx)	reg = RegBl;
		else msg_error(format("wrong 8b register: %d", reg));
	}
	
	sSerialCommandParam p = param_reg(v.type, reg);
	
	// map register
	for (int i=v.first;i<=v.last;i++){
		int r = temp_in_cmd(d, i, vi);
		if (r & 1){
			p.shift = d->cmd[i].p1.shift;
			d->cmd[i].p1 = p;
			if (r & 4)
				d->cmd[i].p1.kind = KindDerefRegister;
		}
		if (r & 2){
			p.shift = d->cmd[i].p2.shift;
			d->cmd[i].p2 = p;
			if (r & 8)
				d->cmd[i].p2.kind = KindDerefRegister;
		}
	}
	add_reg_channel(d, reg32, v.first, v.last);
	msg_db_l(4);
}

void add_stack_var(SerializerData *d, sType *type, int first, int last, sSerialCommandParam &p)
{
	so("add_stack_var");
	so(type->Size);
	// find free stack space for the life span of the variable....
	// don't mind... use old algorithm: ALWAYS grow stack... remove ALL on each command in a block

	int s = mem_align(type->Size);
	d->StackOffset += s;
	int offset = - d->StackOffset;
	if (d->StackOffset > d->StackMaxSize)
		d->StackMaxSize = d->StackOffset;

	p.kind = KindVarLocal;
	p.p = (char*)offset;
	p.type = type;
	p.shift = 0;
}

void MapTempVarToStack(SerializerData *d, int vi)
{
	msg_db_r("stack", 4);
	sTempVar &v = d->TempVar[vi];
	so(format("temp=stack: %d   (%d - %d)", vi, v.first, v.last));

	sSerialCommandParam p;
	add_stack_var(d, v.type, v.first, v.last, p);
	
	// map
	for (int i=v.first;i<=v.last;i++){
		int r = temp_in_cmd(d, i, vi);
		if (r == 0)
			continue;

		if ((r & 3) == 3){
			cur_script->DoErrorInternal("asm error: (MapTempVar) temp var on both sides of command");
			_return_(4,);
		}
		
		sSerialCommandParam *p_own;
		if ((r & 1) > 0){
			p_own = &d->cmd[i].p1;
		}else{
			p_own = &d->cmd[i].p2;
		}
		bool deref = (r > 3);

		p_own->kind = deref ? KindDerefVarLocal : KindVarLocal;
		p_own->p = p.p;
		
#if 0
		sSerialCommandParam *p_own, *p_other;
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
				cur_script->DoErrorInternal("map_stack_var (read, write, deref)");
			*p_own = p_eax;
			add_cmd(d, inst_mov, p_eax, p);
			move_last_cmd(i);
			add_cmd(d, inst_mov, p, p_eax);
			move_last_cmd(i+2);
			add_reg_channel(RegEax, i, i + 2);
			i += 2;
			
		}else if (var_write){ // write only
			if (deref){
				//cur_script->DoErrorInternal("map_stack_var (write, deref)");
				int shift = p_own->shift;
				*p_own = p_deref_eax;
				add_cmd(d, inst_mov, p_eax, p);
				move_last_cmd(i);
				if (shift > 0){
					add_cmd(d, inst_add, p_eax, param_const(TypeInt, (void*)shift));
					move_last_cmd(i + 1);
					add_reg_channel(RegEax, i, i + 2);
				}else
					add_reg_channel(RegEax, i, i + 1);
			}else{
				*p_own = p_eax;
				add_cmd(d, inst_mov, p, p_eax);
				move_last_cmd(i+1);
				add_reg_channel(RegEax, i, i + 1);
			}
			i ++;
		}else{ // read only
			int shift = p_own->shift;
			*p_own = deref ? p_deref_eax : p_eax;
			add_cmd(d, inst_mov, p_eax, p);
			move_last_cmd(i);
			if ((deref) && (shift > 0)){
				add_cmd(d, inst_add, p_eax, param_const(TypeInt, (void*)shift));
				move_last_cmd(i + 1);
				add_reg_channel(RegEax, i, i + 2);
			}else
				add_reg_channel(RegEax, i, i + 1);
			i ++;
		}
#endif
	}
	msg_db_l(4);
}

inline bool is_reg_used_in_interval(SerializerData *d, int reg, int first, int last)
{
	//so(string2("used?   %d: %d - %d", reg, first, last));
	for (int i=0;i<d->RegChannel.num;i++)
		if (d->RegChannel[i].reg == reg){
			//so(string2("rc   %d: %d - %d", RegChannel[i].reg, RegChannel[i].first, RegChannel[i].last));
			if ((d->RegChannel[i].first <= last) && (d->RegChannel[i].last >= first)){
				so(i);
				return true;
			}
		}
	//so("false");
	return false;
}

void MapTempVar(SerializerData *d, int vi)
{
	msg_db_r("MapTempVar", 4);
	sTempVar &v = d->TempVar[vi];
	int first = v.first;
	int last = v.last;

	bool reg_allowed = true;
	for (int i=first;i<=last;i++)
		if (temp_in_cmd(d, i, vi))
			if (!GetInstructionAllowGenReg(d->cmd[i].inst)){
				reg_allowed = false;
				break;
			}

	int reg = -1;
	if (reg_allowed){

		// any register not used in this interval?
		for (int i=0;i<max_reg;i++)
			d->RegUsed[i] = false;
		for (int i=0;i<d->RegChannel.num;i++)
			if ((d->RegChannel[i].first <= last) && (d->RegChannel[i].last >= first))
				d->RegUsed[d->RegChannel[i].reg] = true;
		for (int i=0;i<d->MapReg.num;i++)
			if (!d->RegUsed[d->MapReg[i]]){
				reg = d->MapReg[i];
				break;
			}
	}

	if (reg >= 0)
		MapTempVarToReg(d, vi, reg);
	else
		MapTempVarToStack(d, vi);
	msg_db_l(4);
}

void MapTempVars(SerializerData *d)
{
	msg_db_r("MapTempVars", 3);

	for (int i=0;i<d->TempVar.num;i++)
		MapTempVar(d, i);
	
	//cmd_list_out();
	msg_db_l(3);
}

inline void try_map_param_to_stack(sSerialCommandParam &p, int v, sSerialCommandParam &stackvar)
{
	if ((p.kind == KindVarTemp) && ((long)p.p == v)){
		int shift = p.shift;
		p = stackvar;
		p.shift = shift;
	}
}

void MapReferencedTempVars(SerializerData *d)
{
	msg_db_r("MapReferencedTempVars", 3);
	for (int i=0;i<d->cmd.num;i++)
		if (d->cmd[i].inst == inst_lea)
			if (d->cmd[i].p2.kind == KindVarTemp){
				d->TempVar[(long)d->cmd[i].p2.p].referenced = true;
			}

	for (int i=d->TempVar.num-1;i>=0;i--)
		if (d->TempVar[i].referenced){
			sSerialCommandParam stackvar;
			add_stack_var(d, d->TempVar[i].type, d->TempVar[i].first, d->TempVar[i].last, stackvar);
			for (int j=0;j<d->cmd.num;j++){
				try_map_param_to_stack(d->cmd[j].p1, i, stackvar);
				try_map_param_to_stack(d->cmd[j].p2, i, stackvar);
			}
			remove_temp_var(d, i);
		}
	msg_db_l(3);
}

void DisentangleShiftedTempVars(SerializerData *d)
{
	msg_db_r("DisentangleShiftedTempVars", 3);
	for (int i=0;i<d->cmd.num;i++){
		if ((d->cmd[i].p1.kind == KindVarTemp) && (d->cmd[i].p1.shift > 0)){
			d->TempVar[(long)d->cmd[i].p1.p].entangled = max(d->TempVar[(long)d->cmd[i].p1.p].entangled, d->cmd[i].p1.shift);
		}
		if ((d->cmd[i].p2.kind == KindVarTemp) && (d->cmd[i].p2.shift > 0)){
			d->TempVar[(long)d->cmd[i].p2.p].entangled = max(d->TempVar[(long)d->cmd[i].p2.p].entangled, d->cmd[i].p2.shift);
		}
	}
	for (int i=d->TempVar.num-1;i>=0;i--)
		if (d->TempVar[i].entangled > 0){
			int n = d->TempVar[i].entangled / 4 + 1;
			sType *t = d->TempVar[i].type;
			so("entangled");
			so(n);
			sSerialCommandParam *p = new sSerialCommandParam[n];

			// create small temp vars
			for (int j=0;j<n;j++){
				sType *tt = TypeReg32;
				// corresponding to element in a class?
				for (int k=0;k<t->Element.num;k++)
					if (t->Element[k].Offset == j * 4)
						if (t->Element[k].Type->Size == 4)
							tt = t->Element[k].Type;
				add_temp(d, tt, p[j]);
			}
			
			for (int j=0;j<d->cmd.num;j++){
				if ((d->cmd[j].p1.kind == KindVarTemp) && ((long)d->cmd[j].p1.p == i))
					d->cmd[j].p1 = p[d->cmd[j].p1.shift / 4];
				if ((d->cmd[j].p2.kind == KindVarTemp) && ((long)d->cmd[j].p2.p == i))
					d->cmd[j].p2 = p[d->cmd[j].p2.shift / 4];
			}
			delete[]p;
			remove_temp_var(d, i);
		}

	ScanTempVarUsage(d);
	msg_db_l(3);
}

// TODO....
void ResolveDerefRegShift(SerializerData *d)
{
	msg_db_r("ResolveDerefRegShift", 3);
	for (int i=d->cmd.num-1;i>=0;i--){
		if ((d->cmd[i].p1.kind == KindDerefRegister) && (d->cmd[i].p1.shift > 0)){
			int s = d->cmd[i].p1.shift;
			d->cmd[i].p1.shift = 0;
			add_cmd(d, inst_add, param_reg(TypeReg32, RegRoot[(long)d->cmd[i].p1.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(d, i);
			add_cmd(d, inst_sub, param_reg(TypeReg32, RegRoot[(long)d->cmd[i].p1.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(d, i + 2);
			continue;
		}
		if ((d->cmd[i].p2.kind == KindDerefRegister) && (d->cmd[i].p2.shift > 0)){
			int s = d->cmd[i].p2.shift;
			d->cmd[i].p2.shift = 0;
			add_cmd(d, inst_add, param_reg(TypeReg32, RegRoot[(long)d->cmd[i].p2.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(d, i);
			add_cmd(d, inst_sub, param_reg(TypeReg32, RegRoot[(long)d->cmd[i].p2.p]), param_const(TypeInt, (void*)s));
			move_last_cmd(d, i + 2);
			continue;
		}
	}
	msg_db_l(3);
}

void CScript::SerializeFunction(SerializerData *d, sFunction *f)
{
	msg_db_r("SerializeFunction", 2);

	p_eax = param_reg(TypeReg32, RegEax);
	p_eax_int = param_reg(TypeInt, RegEax);

	p_deref_eax.kind = KindDerefRegister;
	p_deref_eax.p = (char*)RegEax;
	p_deref_eax.type = TypePointer;
	p_deref_eax.shift = 0;
	
	p_ax = param_reg(TypeReg16, RegAx);
	p_al = param_reg(TypeReg8, RegAl);
	p_al_bool = param_reg(TypeBool, RegAl);
	p_al_char = param_reg(TypeChar, RegAl);
	p_ah = param_reg(TypeReg8, RegAh);
	p_st0 = param_reg(TypeFloat, RegSt0);
	p_st1 = param_reg(TypeFloat, RegSt1);

	cur_script = this;

	d->cur_func = f;
	d->NumMarkers = 0;
	d->call_used = false;
	d->StackOffset = f->_VarSize;
	d->StackMaxSize = f->_VarSize;
	d->TempVarRangesDefined = false;
	AsmError = false;

	InsertedConstructorTemp.clear();
	InsertedConstructorFunc.clear();
	
#ifdef allow_registers
	//d->MapReg.add(RegEax);
	d->MapReg.add(RegEcx);
	d->MapReg.add(RegEdx);
	/*d->MapReg.add(RegEbx);
	d->MapReg.add(RegEsi);
	d->MapReg.add(RegEdi);*/
#endif

// serialize

	// intro
	add_cmd(d, inst_push, param_reg(TypeReg32, RegEbp));
	add_cmd(d, inst_mov, param_reg(TypeReg32, RegEbp), param_reg(TypeReg32, RegEsp));
	

	FillInConstructorsFunc(d);
	if (Error)	_return_(2,);

	// function
	SerializeBlock(d, f->Block, 0);
	if (Error)	_return_(2,);
	
	FillInDestructors(d, false);
	if (Error)	_return_(2,);


	if (d->StuffToAdd.num > 0){
		DoErrorInternal("StuffToAdd");
		msg_write(f->Name);
		msg_write(d->StuffToAdd.num);
		for (int i=0;i<d->StuffToAdd.num;i++){
			msg_write(d->StuffToAdd[i].kind);
			msg_write(d->StuffToAdd[i].marker);
			msg_write(d->StuffToAdd[i].index);
			msg_write(d->StuffToAdd[i].level);
		}
		_return_(2,);
	}

	// outro
	bool need_outro = true;
	if (f->Block->Command.num > 0)
		if ((f->Block->Command.back()->Kind == KindCompilerFunction) && (f->Block->Command.back()->LinkNr == CommandReturn))
			need_outro = false;
	if (need_outro){
		add_cmd(d, inst_leave);
		if (f->Type->Size > 4)
			add_cmd(d, inst_ret, param_const(TypeInt, (void*)4));
		else
			add_cmd(d, inst_ret);
	}
	
	cmd_list_out();

// do all the translations...

	MapReferencedTempVars(d);
	if (Error)	_return_(2,);

	//HandleDerefTemp();

	DisentangleShiftedTempVars(d);
	if (Error)	_return_(2,);
	cmd_list_out();

	ResolveDerefTempAndLocal(d);
	if (Error)	_return_(2,);
	cmd_list_out();

#ifdef allow_simplification
	SimplifyMovs(d);
	if (Error)	_return_(2,);
	cmd_list_out();

	SimplifyFPUStack(d);
	if (Error)	_return_(2,);
	cmd_list_out();
#endif

	MapTempVars(d);
	if (Error)	_return_(2,);
	cmd_list_out();

	ResolveDerefRegShift(d);
	if (Error)	_return_(2,);
	cmd_list_out();

	//ResolveDerefLocal();
	//if (Error)	_return_(2,);
	//cmd_list_out();

	CorrectUnallowedParamCombis(d);
	if (Error)	_return_(2,);

	//if (call_used){
		if (d->StackMaxSize > 127){
			add_cmd(d, inst_sub, param_reg(TypeReg32, RegEsp), param_const(TypeInt, (void*)d->StackMaxSize));
			move_last_cmd(d, 2);
		}else if (d->StackMaxSize > 0){
			add_cmd(d, inst_sub_b, param_reg(TypeReg32, RegEsp), param_const(TypeInt, (void*)d->StackMaxSize));
			move_last_cmd(d, 2);
		}
	//}
	cmd_list_out();
	
	msg_db_l(2);
}

inline void get_param(int inst, sSerialCommandParam &p, int &param_type, void *&param)
{
	if (p.kind < 0){
		param_type = PKNone;
		param = NULL;
	}else if (p.kind == KindMarker){
		//msg_error("marker");
		param_type = PKConstant32;
		param = NULL; // real position will be inserted at the end
	}else if (p.kind == KindRegister){
		param_type = PKRegister;
		param = p.p;
		if (p.shift > 0)
			cur_script->DoErrorInternal("get_param: reg + shift");
	}else if (p.kind == KindDerefRegister){
		param_type = PKDerefRegister;
		param = p.p;
		if (p.shift > 0){
			if ((long)p.p == RegEdx){
				param_type = PKEdxRel;
				param = (void*)p.shift;
			}else
				cur_script->DoErrorInternal("get_param: [reg] + shift");
		}
	}else if (p.kind == KindVarGlobal){
		param_type = PKDerefConstant;
		param = p.p + p.shift;
	}else if (p.kind == KindVarLocal){
		param_type = PKLocal;
		param = p.p + p.shift;
	}else if (p.kind == KindRefToConst){
		bool imm_allowed = GetInstructionAllowConst(inst);
		if ((p.type->Size <= 4) && (imm_allowed)){
			param_type = (p.type->Size == 1) ? PKConstant8 : PKConstant32;
			param = (char*)*(int*)(p.p + p.shift);
		}else{
			param_type = PKDerefConstant;
			param = p.p + p.shift;
		}
	}else if (p.kind == KindConstant){
		param_type = PKConstant32;
		if (p.type->Size == 1)
			param_type = PKConstant8;
		param = p.p;
		if (p.shift > 0)
			cur_script->DoErrorInternal("get_param: const + shift");
	}else
		cur_script->DoErrorInternal("get_param: unexpected param..." + Kind2Str(p.kind));
}

Array<int> InstructionPos;

void ProcessJumpTargets(SerializerData *d, char *Opcode, int &OpcodeSize)
{
	msg_db_r("ProcessJumpTargets", 3);
	foreachi(d->cmd, c, i){
		if (c->inst < inst_marker)
			if (c->p1.kind == KindMarker){
				so("adjust jump");
				int marker = (long)c->p1.p;
				so(marker);
				int pos_after_jump_inst = InstructionPos[i + 1];
				int target_pos = -1;
				for (int j=0;j<d->cmd.num;j++)
					if (d->cmd[j].inst == inst_marker)
						if (d->cmd[j].p1.kind == marker){
							target_pos = InstructionPos[j];
							break;
						}
				if (target_pos >= 0){
					*(int*)&Opcode[pos_after_jump_inst - 4] = (target_pos - pos_after_jump_inst);
					c->p1.p = (char*)(target_pos - pos_after_jump_inst);
					c->p1.kind = KindConstant;
					c->p1.type = TypePointer;
				}else{
					cur_script->DoErrorInternal("asm error: jump marker not found");
					_return_(3,);
				}
			}
	}
	msg_db_l(3);
}

inline void assemble_cmd(char *Opcode, int &OpcodeSize, sSerialCommand &c)
{
	// translate parameters
	int param1_type, param2_type;
	void *param1, *param2;
	get_param(c.inst, c.p1, param1_type, param1);
	if (cur_script->Error)	return;
	get_param(c.inst, c.p2, param2_type, param2);
	if (cur_script->Error)	return;
	
	// assemble instruction
	AsmAddInstruction(Opcode, OpcodeSize, c.inst, param1_type, param1, param2_type, param2);
	if (AsmError)
		cur_script->DoErrorInternal("asm error");
	//printf("%s", Opcode2Asm(oc, ocs));
}

void ShrinkOpcode(SerializerData *d, char *oc, int &ocs, int c, int diff)
{
	msg_db_r("ShrinkOpcode", 3);

	// shift opcode
	for (int i=InstructionPos[c+1];i<ocs;i++)
		oc[i - diff] = oc[i];
	ocs -= diff;

	// InstructionPos after c
	for (int i=c+1;i<d->cmd.num+1;i++)
		InstructionPos[i] -= diff;

	// calls after c
	for (int i=c+1;i<d->cmd.num;i++)
		if (d->cmd[i].inst == inst_call)
			if (d->cmd[i].p1.p){ // we need to keep 0x0000...
				so("call");
				*(int*)&d->cmd[i].p1.p -= diff;
				*(int*)&oc[InstructionPos[i + 1] - 4] += diff;
			}

	// jumps over c
	for (int i=0;i<d->cmd.num;i++)
			if ((d->cmd[i].inst == inst_jmp) || (d->cmd[i].inst == inst_jz) || (d->cmd[i].inst == inst_jnz) || (d->cmd[i].inst == inst_jmp_b) || (d->cmd[i].inst == inst_jz_b) || (d->cmd[i].inst == inst_jnz_b)){
				so("jump");
				int s = d->cmd[i].p1.type->Size;
				so(*(int*)&d->cmd[i].p1.p);
				if (i < c)
					if ((long)d->cmd[i].p1.p > InstructionPos[c] - InstructionPos[i + 1]){
						so("jump over forward");
						*(int*)&d->cmd[i].p1.p -= diff;
						if (s == 1)
							*(char*)&oc[InstructionPos[i + 1] - 1] -= diff;
						else
							*(int*)&oc[InstructionPos[i + 1] - 4] -= diff;
					}
				if (i >= c)
					if (-(long)d->cmd[i].p1.p > InstructionPos[i + 1] - InstructionPos[c]){
						so("jump over backward");
						*(int*)&d->cmd[i].p1.p += diff;
						if (s == 1)
							*(char*)&oc[InstructionPos[i + 1] - 1] += diff;
						else
							*(int*)&oc[InstructionPos[i + 1] - 4] += diff;
					}
				so(*(int*)&d->cmd[i].p1.p);
			}
	
	msg_db_l(3);
}

void ShrinkJumps(SerializerData *d, char *Opcode, int &OpcodeSize)
{
	msg_db_r("ShrinkJumps", 3);
	for (int i=0;i<d->cmd.num;i++){
		if ((d->cmd[i].inst == inst_jmp) || (d->cmd[i].inst == inst_jz) || (d->cmd[i].inst == inst_jnz))
			if ((d->cmd[i].p1.kind == KindConstant) && ((long)d->cmd[i].p1.p < 127) && ((long)d->cmd[i].p1.p > -127)){
				so("jump shrinkable");

				// shrink the command
				if (d->cmd[i].inst == inst_jmp)
					d->cmd[i].inst = inst_jmp_b;
				else if (d->cmd[i].inst == inst_jz)
					d->cmd[i].inst = inst_jz_b;
				else if (d->cmd[i].inst == inst_jnz)
					d->cmd[i].inst = inst_jnz_b;
				d->cmd[i].p1.type = TypeChar;

				// assemble the smaller command
				char oc[16];
				int ocs = 0;
				assemble_cmd(oc, ocs, d->cmd[i]);
				if (cur_script->Error)
					_return_(3,);
				//msg_write(Opcode2Asm(oc, ocs));

				int diff = InstructionPos[i + 1] - InstructionPos[i] - ocs;
				so(diff);

				// really smaller?    (should be unneccessary...)
				if (diff < 0)
					continue;

				// insert
				for (int j=0;j<ocs;j++)
					Opcode[InstructionPos[i] + j] = oc[j];

				// correct all other commands...
				ShrinkOpcode(d, Opcode, OpcodeSize, i, diff);
				//break;
			}
	}
	msg_db_l(3);
}

void CompileAsmBlock(char *Opcode, int &OpcodeSize)
{
	msg_db_r("CompileAsmBlock", 4);
	//msg_write(".------------------------------- asm");
	CPreScript *ps = cur_script->pre_script;
	CreateAsmMetaInfo(ps);
	((sAsmMetaInfo*)ps->AsmMetaInfo)->CurrentOpcodePos = OpcodeSize;
	((sAsmMetaInfo*)ps->AsmMetaInfo)->PreInsertionLength = OpcodeSize;
	((sAsmMetaInfo*)ps->AsmMetaInfo)->LineOffset = ps->AsmBlock[0].Line;
	CurrentAsmMetaInfo = (sAsmMetaInfo*)ps->AsmMetaInfo;
	const char *pac = Asm2Opcode(ps->AsmBlock[0].block);
	if (!AsmError){
		//msg_write(AsmCodeLength);
		//msg_write(d2h(pac, AsmCodeLength, false));
		//msg_write(Opcode2Asm(pac, AsmCodeLength));
#ifdef _insert_asm_
		for (int i=0;i<AsmCodeLength;i++)
			Opcode[OpcodeSize + i] = pac[i];
		OpcodeSize += AsmCodeLength;
#endif
	}else{
		AsmErrorLine--; // (T_T)
		cur_script->DoError("error in assembler code...", AsmErrorLine);
		_return_(4,);
	}
	delete[](ps->AsmBlock[0].block);
	ps->AsmBlock.erase(0);
	msg_db_l(4);
}

void CompileSerialized(SerializerData *d, char *Opcode, int &OpcodeSize)
{
	msg_db_r("CompileSerialized", 2);
	InstructionPos.resize(d->cmd.num + 1);
	for (int i=0;i<d->cmd.num;i++){
		InstructionPos[i] = OpcodeSize;

		if (d->cmd[i].inst == inst_marker)
			continue;

		if (d->cmd[i].inst == inst_asm){
			CompileAsmBlock(Opcode, OpcodeSize);
			if (cur_script->Error)
				_return_(2,);
			continue;
		}

		// make calls relative...
		if (d->cmd[i].inst == inst_call)
			if (d->cmd[i].p1.p) // we need to keep 0x0000...
				d->cmd[i].p1.p = (char*)(  (long)d->cmd[i].p1.p - (long)&Opcode[OpcodeSize] - 5 );

		// push 8 bit -> push 32 bit
		if (d->cmd[i].inst == inst_push)
			if (d->cmd[i].p1.kind == KindRegister)
				d->cmd[i].p1.p = (char*) RegRoot[(long)d->cmd[i].p1.p];
			

		assemble_cmd(Opcode, OpcodeSize, d->cmd[i]);
		if (cur_script->Error)
			_return_(2,);
	}
	InstructionPos[d->cmd.num] = OpcodeSize;

	ProcessJumpTargets(d, Opcode, OpcodeSize);

	//msg_write(Opcode2Asm(Opcode, OpcodeSize));

#ifdef allow_simplification
	ShrinkJumps(d, Opcode, OpcodeSize);
#endif
	msg_db_l(2);
}

void CScript::CompileFunction(sFunction *f, char *Opcode, int &OpcodeSize)
{
	cur_func = f;
	SerializerData d;
	SerializeFunction(&d, f);
	CompileSerialized(&d, Opcode, OpcodeSize);
}
