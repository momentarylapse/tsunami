#include "script.h"
#include "../file/file.h"

typedef void op_func(void *r, void *a, void *b);

//static sFunction *cur_func;



extern void script_db_out(const string &str);
extern void script_db_out(int i);
extern void script_db_right();
extern void script_db_left();

#define so		script_db_out
#define right	script_db_right
#define left	script_db_left


void CPreScript::PreProcessCommand(CScript *s, sCommand *c)
{
	msg_db_r("PreProcessCommand", 4);

	// recursion
	if (c->Kind == KindBlock){
		for (int i=0;i<Block[c->LinkNr]->Command.num;i++)
			PreProcessCommand(s, Block[c->LinkNr]->Command[i]);
	}
	for (int i=0;i<c->NumParams;i++)
		PreProcessCommand(s, c->Param[i]);
	if (c->Instance)
		PreProcessCommand(s, c->Instance);
	

	// process...
	if (c->Kind == KindOperator){
		sPreOperator *o = &PreOperator[c->LinkNr];
		if (o->Func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->NumParams;i++)
				if (c->Param[i]->Kind == KindAddress)
					is_address = true;
				else if (c->Param[i]->Kind == KindLocalAddress)
					is_address = is_local = true;
				else if (c->Param[i]->Kind != KindConstant)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->Func;
				if (is_address){
					/*so("pre process address");
					void *d1 = (void*)&c->Param[0]->LinkNr;
					void *d2 = (void*)&c->Param[1]->LinkNr;
					if (c->Param[0]->Kind == KindConstant)
					    d1 = Constant[c->Param[0]->LinkNr].data;
					if (c->Param[1]->Kind == KindConstant)
					    d2 = Constant[c->Param[1]->LinkNr].data;
					void *r = (void*)&c->LinkNr;
					f(r, d1, d2);
					c->Kind = is_local ? KindLocalAddress : KindAddress;
					c->NumParams = 0;*/
				}else{
					so("pre process operator");
					int nc = AddConstant(o->ReturnType);
					void *d1 = Constant[c->Param[0]->LinkNr].data;
					void *d2 = NULL;
					if (c->NumParams > 1)
						d2 = Constant[c->Param[1]->LinkNr].data;
					f(Constant[nc].data, d1, d2);
					c->Kind = KindConstant;
					c->LinkNr = nc;
					c->NumParams = 0;
				}
			}
		}
	}/*else if (c->Kind == KindReference){
		if (s){
			if ((c->Param[0]->Kind == KindVarGlobal) || (c->Param[0]->Kind == KindVarLocal) || (c->Param[0]->Kind == KindVarExternal) || (c->Param[0]->Kind == KindConstant)){
				so("pre process ref var");
				c->Kind = KindAddress;
				c->NumParams = 0;
				if (c->Param[0]->Kind == KindVarGlobal){
					if (c->Param[0]->script)
						c->LinkNr = (long)c->Param[0]->script->g_var[c->Param[0]->LinkNr];
					else
						c->LinkNr = (long)s->g_var[c->Param[0]->LinkNr];
				}else if (c->Param[0]->Kind == KindVarExternal){
					c->LinkNr = (long)PreExternalVar[c->Param[0]->LinkNr].Pointer;
				}else if (c->Param[0]->Kind == KindVarLocal){
					c->LinkNr = (long)cur_func->Var[c->Param[0]->LinkNr]._Offset;
					c->Kind = KindLocalAddress;
				}else if (c->Param[0]->Kind == KindConstant)
					c->LinkNr = (long)s->cnst[c->Param[0]->LinkNr];
			}
		}
	}else if (c->Kind == KindDereference){
		if (c->Param[0]->Kind == KindAddress){
			so("pre process deref address");
			c->Kind = KindMemory;
			c->LinkNr = c->Param[0]->LinkNr;
			c->NumParams = 0;
		}else if (c->Param[0]->Kind == KindLocalAddress){
			so("pre process deref local address");
			c->Kind = KindLocalMemory;
			c->LinkNr = c->Param[0]->LinkNr;
			c->NumParams = 0;
		}
	}*/
	msg_db_l(4);
}

string LinkNr2Str(CPreScript *s,int kind,int nr);

// may not use AddConstant()!!!
void CPreScript::PreProcessCommandAddresses(CScript *s, sCommand *c)
{
	msg_db_r("PreProcessCommandAddr", 4);
	/*msg_write(Kind2Str(c->Kind));
	if (c->script)
		msg_write(LinkNr2Str(c->script->pre_script, c->Kind, c->LinkNr));
	else if (s)
		msg_write(LinkNr2Str(s->pre_script, c->Kind, c->LinkNr));*/

	// recursion
	if (c->Kind == KindBlock){
		for (int i=0;i<Block[c->LinkNr]->Command.num;i++)
			PreProcessCommandAddresses(s, Block[c->LinkNr]->Command[i]);
	}
	for (int i=0;i<c->NumParams;i++)
		PreProcessCommandAddresses(s, c->Param[i]);
	if (c->Instance)
		PreProcessCommandAddresses(s, c->Instance);
	

	// process...
	if (c->Kind == KindOperator){
		sPreOperator *o = &PreOperator[c->LinkNr];
		if (o->Func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->NumParams;i++)
				if (c->Param[i]->Kind == KindAddress)
					is_address = true;
				else if (c->Param[i]->Kind == KindLocalAddress)
					is_address = is_local = true;
				else if (c->Param[i]->Kind != KindConstant)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->Func;
				if (is_address){
					so("pre process address");
					void *d1 = (void*)&c->Param[0]->LinkNr;
					void *d2 = (void*)&c->Param[1]->LinkNr;
					if (c->Param[0]->Kind == KindConstant)
					    d1 = Constant[c->Param[0]->LinkNr].data;
					if (c->Param[1]->Kind == KindConstant)
					    d2 = Constant[c->Param[1]->LinkNr].data;
					void *r = (void*)&c->LinkNr;
					f(r, d1, d2);
					c->Kind = is_local ? KindLocalAddress : KindAddress;
					c->NumParams = 0;
				}
			}
		}
	}else if (c->Kind == KindReference){
		if (s){
			if ((c->Param[0]->Kind == KindVarGlobal) || (c->Param[0]->Kind == KindVarLocal) || (c->Param[0]->Kind == KindVarExternal) || (c->Param[0]->Kind == KindConstant)){
				so("pre process ref var");
				c->Kind = KindAddress;
				c->NumParams = 0;
				if (c->Param[0]->Kind == KindVarGlobal){
					if (c->Param[0]->script)
						c->LinkNr = (long)c->Param[0]->script->g_var[c->Param[0]->LinkNr];
					else
						c->LinkNr = (long)s->g_var[c->Param[0]->LinkNr];
				}else if (c->Param[0]->Kind == KindVarExternal){
					c->LinkNr = (long)PreExternalVar[c->Param[0]->LinkNr].Pointer;
				}else if (c->Param[0]->Kind == KindVarLocal){
					c->LinkNr = (long)cur_func->Var[c->Param[0]->LinkNr]._Offset;
					c->Kind = KindLocalAddress;
				}else if (c->Param[0]->Kind == KindConstant)
					c->LinkNr = (long)s->cnst[c->Param[0]->LinkNr];
			}
		}
	}else if (c->Kind == KindDereference){
		if (c->Param[0]->Kind == KindAddress){
			so("pre process deref address");
			c->Kind = KindMemory;
			c->LinkNr = c->Param[0]->LinkNr;
			c->NumParams = 0;
		}else if (c->Param[0]->Kind == KindLocalAddress){
			so("pre process deref local address");
			c->Kind = KindLocalMemory;
			c->LinkNr = c->Param[0]->LinkNr;
			c->NumParams = 0;
		}
	}
	msg_db_l(4);
}

void CPreScript::PreProcessor(CScript *s)
{
	msg_db_r("PreProcessor", 4);
	foreach(Function, f){
		cur_func = *f;
		foreach((*f)->Block->Command, c)
			PreProcessCommand(s, *c);
	}
	//Show();
	msg_db_l(4);
}

void CPreScript::PreProcessorAddresses(CScript *s)
{
	msg_db_r("PreProcessorAddr", 4);
	foreach(Function, f){
		cur_func = *f;
		foreach((*f)->Block->Command, c)
			PreProcessCommandAddresses(s, *c);
	}
	//Show();
	msg_db_l(4);
}
