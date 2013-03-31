#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"

namespace Script{

typedef void op_func(void *r, void *a, void *b);

//static Function *cur_func;



extern void script_db_out(const string &str);
extern void script_db_out(int i);
extern void script_db_right();
extern void script_db_left();

#define so		script_db_out
#define right	script_db_right
#define left	script_db_left


void PreScript::PreProcessCommand(Script *s, Command *c)
{
	msg_db_r("PreProcessCommand", 4);

	// recursion
	if (c->kind == KindBlock){
		for (int i=0;i<Blocks[c->link_nr]->command.num;i++)
			PreProcessCommand(s, Blocks[c->link_nr]->command[i]);
	}
	for (int i=0;i<c->num_params;i++)
		PreProcessCommand(s, c->param[i]);
	if (c->instance)
		PreProcessCommand(s, c->instance);
	

	// process...
	if (c->kind == KindOperator){
		PreOperator *o = &PreOperators[c->link_nr];
		if (o->func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->num_params;i++)
				if (c->param[i]->kind == KindAddress)
					is_address = true;
				else if (c->param[i]->kind == KindLocalAddress)
					is_address = is_local = true;
				else if (c->param[i]->kind != KindConstant)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->func;
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
					int nc = AddConstant(o->return_type);
					void *d1 = Constants[c->param[0]->link_nr].data;
					void *d2 = NULL;
					if (c->num_params > 1)
						d2 = Constants[c->param[1]->link_nr].data;
					f(Constants[nc].data, d1, d2);
					c->kind = KindConstant;
					c->link_nr = nc;
					c->num_params = 0;
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

string LinkNr2Str(PreScript *s,int kind,int nr);

// may not use AddConstant()!!!
void PreScript::PreProcessCommandAddresses(Script *s, Command *c)
{
	msg_db_r("PreProcessCommandAddr", 4);
	/*msg_write(Kind2Str(c->Kind));
	if (c->script)
		msg_write(LinkNr2Str(c->script->pre_script, c->Kind, c->LinkNr));
	else if (s)
		msg_write(LinkNr2Str(s->pre_script, c->Kind, c->LinkNr));*/

	// recursion
	if (c->kind == KindBlock){
		for (int i=0;i<Blocks[c->link_nr]->command.num;i++)
			PreProcessCommandAddresses(s, Blocks[c->link_nr]->command[i]);
	}
	for (int i=0;i<c->num_params;i++)
		PreProcessCommandAddresses(s, c->param[i]);
	if (c->instance)
		PreProcessCommandAddresses(s, c->instance);
	

	// process...
	if (c->kind == KindOperator){
		PreOperator *o = &PreOperators[c->link_nr];
		if (o->func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->num_params;i++)
				if (c->param[i]->kind == KindAddress)
					is_address = true;
				else if (c->param[i]->kind == KindLocalAddress)
					is_address = is_local = true;
				else if (c->param[i]->kind != KindConstant)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->func;
				if (is_address){
					so("pre process address");
					void *d1 = (void*)&c->param[0]->link_nr;
					void *d2 = (void*)&c->param[1]->link_nr;
					if (c->param[0]->kind == KindConstant)
					    d1 = Constants[c->param[0]->link_nr].data;
					if (c->param[1]->kind == KindConstant)
					    d2 = Constants[c->param[1]->link_nr].data;
					void *r = (void*)&c->link_nr;
					f(r, d1, d2);
					c->kind = is_local ? KindLocalAddress : KindAddress;
					c->num_params = 0;
				}
			}
		}
	}else if (c->kind == KindReference){
		if (s){
			if ((c->param[0]->kind == KindVarGlobal) || (c->param[0]->kind == KindVarLocal) || (c->param[0]->kind == KindVarExternal) || (c->param[0]->kind == KindConstant)){
				so("pre process ref var");
				c->kind = KindAddress;
				c->num_params = 0;
				if (c->param[0]->kind == KindVarGlobal){
					if (c->param[0]->script)
						c->link_nr = (long)c->param[0]->script->g_var[c->param[0]->link_nr];
					else
						c->link_nr = (long)s->g_var[c->param[0]->link_nr];
				}else if (c->param[0]->kind == KindVarExternal){
					c->link_nr = (long)PreExternalVars[c->param[0]->link_nr].pointer;
				}else if (c->param[0]->kind == KindVarLocal){
					c->link_nr = (long)cur_func->var[c->param[0]->link_nr]._offset;
					c->kind = KindLocalAddress;
				}else if (c->param[0]->kind == KindConstant)
					c->link_nr = (long)s->cnst[c->param[0]->link_nr];
			}
		}
	}else if (c->kind == KindDereference){
		if (c->param[0]->kind == KindAddress){
			so("pre process deref address");
			c->kind = KindMemory;
			c->link_nr = c->param[0]->link_nr;
			c->num_params = 0;
		}else if (c->param[0]->kind == KindLocalAddress){
			so("pre process deref local address");
			c->kind = KindLocalMemory;
			c->link_nr = c->param[0]->link_nr;
			c->num_params = 0;
		}
	}
	msg_db_l(4);
}

void PreScript::PreProcessor(Script *s)
{
	msg_db_r("PreProcessor", 4);
	foreach(Function *f, Functions){
		cur_func = f;
		foreach(Command *c, f->block->command)
			PreProcessCommand(s, c);
	}
	//Show();
	msg_db_l(4);
}

void PreScript::PreProcessorAddresses(Script *s)
{
	msg_db_r("PreProcessorAddr", 4);
	foreach(Function *f, Functions){
		cur_func = f;
		foreach(Command *c, f->block->command)
			PreProcessCommandAddresses(s, c);
	}
	//Show();
	msg_db_l(4);
}

};
