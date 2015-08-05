#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"

namespace Script{

typedef void op_func(string &r, string &a, string &b);

//static Function *cur_func;

bool call_function(Function *f, void *ff, void *ret, void *inst, Array<void*> param)
{
	if (f->num_params == 0){
		if (f->return_type == TypeInt){
			*(int*)ret = ((int(*)())ff)();
			return true;
		}else if (f->return_type == TypeFloat32){
			*(float*)ret = ((float(*)())ff)();
			return true;
		}else if (f->return_type->UsesReturnByMemory()){
			((void(*)(void*))ff)(ret);
			return true;
		}
	}else if (f->num_params == 1){
		if (f->return_type == TypeInt){
			if (f->literal_param_type[0] == TypeInt){
				*(int*)ret = ((int(*)(int))ff)(*(int*)param[0]);
				return true;
			}
		}else if (f->return_type == TypeFloat32){
			if (f->literal_param_type[0] == TypeFloat32){
				*(float*)ret = ((float(*)(float))ff)(*(float*)param[0]);
				return true;
			}
		}else if (f->return_type->UsesReturnByMemory()){
			if (f->literal_param_type[0] == TypeInt){
				((void(*)(void*, int))ff)(ret, *(int*)param[0]);
				return true;
			}else if (f->literal_param_type[0] == TypeFloat32){
				((void(*)(void*, float))ff)(ret, *(float*)param[0]);
				return true;
			}else if (f->literal_param_type[0]->UsesCallByReference()){
				((void(*)(void*, void*))ff)(ret, param[0]);
				return true;
			}
		}
	}else if (f->num_params == 2){
		if (f->return_type == TypeInt){
			if ((f->literal_param_type[0] == TypeInt) && (f->literal_param_type[1] == TypeInt)){
				*(int*)ret = ((int(*)(int, int))ff)(*(int*)param[0], *(int*)param[1]);
				return true;
			}
		}else if (f->return_type == TypeFloat32){
			if ((f->literal_param_type[0] == TypeFloat32) && (f->literal_param_type[1] == TypeFloat32)){
				*(float*)ret = ((float(*)(float, float))ff)(*(float*)param[0], *(float*)param[1]);
				return true;
			}
		}else if (f->return_type->UsesReturnByMemory()){
			if ((f->literal_param_type[0] == TypeInt) && (f->literal_param_type[1] == TypeInt)){
				((void(*)(void*, int, int))ff)(ret, *(int*)param[0], *(int*)param[1]);
				return true;
			}else if ((f->literal_param_type[0] == TypeFloat32) && (f->literal_param_type[1] == TypeFloat32)){
				((void(*)(void*, float, float))ff)(ret, *(float*)param[0], *(float*)param[1]);
				return true;
			}else if ((f->literal_param_type[0]->UsesCallByReference()) && (f->literal_param_type[1]->UsesCallByReference())){
				((void(*)(void*, void*, void*))ff)(ret, param[0], param[1]);
				return true;
			}
		}
	}else if (f->num_params == 3){
		if (f->return_type->UsesReturnByMemory()){
			if ((f->literal_param_type[0] == TypeFloat32) && (f->literal_param_type[1] == TypeFloat32) && (f->literal_param_type[2] == TypeFloat32)){
				((void(*)(void*, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2]);
				return true;
			}
		}
	}else if (f->num_params == 4){
		if (f->return_type->UsesReturnByMemory()){
			if ((f->literal_param_type[0] == TypeFloat32) && (f->literal_param_type[1] == TypeFloat32) && (f->literal_param_type[2] == TypeFloat32) && (f->literal_param_type[3] == TypeFloat32)){
				((void(*)(void*, float, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2], *(float*)param[3]);
				return true;
			}
		}
	}
	return false;
}


#if 0
void PreProcessFunction(SyntaxTree *ps, Command *c)
{
	bool all_const = true;
	bool is_address = false;
	bool is_local = false;
	for (int i=0;i<c->num_params;i++)
		if (c->param[i]->kind == KIND_ADDRESS)
			is_address = true;
		else if (c->param[i]->kind == KIND_LOCAL_ADDRESS)
			is_address = is_local = true;
		else if (c->param[i]->kind != KIND_CONSTANT)
			all_const = false;
	if (!all_const)
		return;
					op_func *f = (op_func*)o->func;
					if (is_address){
						// pre process address
						/*void *d1 = (void*)&c->Param[0]->LinkNr;
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
						// pre process operator
						int nc = ps->AddConstant(o->return_type);
						string d1 = ps->constants[c->param[0]->link_no].value;
						string d2;
						if (c->num_params > 1)
							d2 = ps->constants[c->param[1]->link_no].value;
						f(ps->constants[nc].value, d1, d2);
						c->script = ps->script;
						c->kind = KIND_CONSTANT;
						c->link_no = nc;
						c->num_params = 0;
					}
}
#endif


Command *SyntaxTree::PreProcessCommand(Command *c)
{
	msg_db_f("PreProcessCommand", 4);

	// recursion
	if (c->kind == KIND_BLOCK){
		for (int i=0;i<c->as_block()->command.num;i++)
			c->as_block()->command[i] = PreProcessCommand(c->as_block()->command[i]);
	}
	for (int i=0;i<c->param.num;i++)
		c->set_param(i, PreProcessCommand(c->param[i]));
	if (c->instance)
		c->set_instance(PreProcessCommand(c->instance));
	

	// process...
	if (c->kind == KIND_OPERATOR){
		PreOperator *o = &PreOperators[c->link_no];
		/*if (c->link_nr == OperatorIntAdd){
			if (c->param[1]->kind == KindConstant){
				int v = *(int*)Constants[c->param[1]->link_nr].data;
				if (v == 0){
					msg_error("addr + 0");
					*c = *c->param[0];
				}
			}
		}else*/ if (o->func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->param.num;i++)
				if (c->param[i]->kind == KIND_ADDRESS)
					is_address = true;
				else if (c->param[i]->kind == KIND_LOCAL_ADDRESS)
					is_address = is_local = true;
				else if (c->param[i]->kind != KIND_CONSTANT)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->func;
				if (is_address){
					// pre process address
					/*void *d1 = (void*)&c->Param[0]->LinkNr;
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
					// pre process operator
					int nc = AddConstant(o->return_type);
					string d1 = constants[c->param[0]->link_no].value;
					string d2;
					if (c->param.num > 1)
						d2 = constants[c->param[1]->link_no].value;
					f(constants[nc].value, d1, d2);
					return add_command_const(nc);
				}
			}
		}
#if 1
	}else if (c->kind == KIND_FUNCTION){
		Function *f = c->script->syntax->functions[c->link_no];
		if (!f->is_pure)
			return c;
		void *ff = (void*)c->script->func[c->link_no];
		if (!ff)
			return c;
		bool all_const = true;
		bool is_address = false;
		bool is_local = false;
		for (int i=0;i<c->param.num;i++){
			if (c->param[i]->kind == KIND_ADDRESS)
				is_address = true;
			else if (c->param[i]->kind == KIND_LOCAL_ADDRESS)
				is_address = is_local = true;
			else if (c->param[i]->kind != KIND_CONSTANT)
				all_const = false;
		}
		void *inst = NULL;
		if (c->instance){
			return c;
			if (c->instance->kind != KIND_CONSTANT)
				all_const = false;
			inst = constants[c->instance->link_no].value.data;
		}
		if (!all_const)
			return c;
		if (is_address)
			return c;
		string temp;
		temp.resize(f->return_type->size);
		Array<void*> p;
		for (int i=0; i<c->param.num; i++)
			p.add(constants[c->param[i]->link_no].value.data);
		if (!call_function(f, ff, temp.data, inst, p))
			return c;
		int nc = AddConstant(f->return_type);
		constants[nc].value = temp;
		return add_command_const(nc);
#endif
	}else if (c->kind == KIND_ARRAY_BUILDER){
		bool all_consts = true;
		for (int i=0; i<c->param.num; i++)
			if (c->param[i]->kind != KIND_CONSTANT)
				all_consts = false;
		if (all_consts){
			int nc = AddConstant(c->type);
			int el_size = c->type->parent->size;
			DynamicArray *da = (DynamicArray*)constants[nc].value.data;
			da->init(el_size);
			da->resize(c->param.num);
			for (int i=0; i<c->param.num; i++)
				memcpy((char*)da->data + el_size * i, constants[c->param[i]->link_no].value.data, el_size);
			return add_command_const(nc);
		}
	}/*else if (c->kind == KindReference){
	// no... we don't know the addresses of globals/constants yet...
		if (s){
			if ((c->Param[0]->Kind == KindVarGlobal) || (c->Param[0]->Kind == KindVarLocal) || (c->Param[0]->Kind == KindVarExternal) || (c->Param[0]->Kind == KindConstant)){
				// pre process ref var
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
	}else if (c->kind == KindDereference){
		if (c->Param[0]->Kind == KindAddress){
			// pre process deref address
			c->Kind = KindMemory;
			c->LinkNr = c->Param[0]->LinkNr;
			c->NumParams = 0;
		}else if (c->Param[0]->Kind == KindLocalAddress){
			// pre process deref local address
			c->Kind = KindLocalMemory;
			c->LinkNr = c->Param[0]->LinkNr;
			c->NumParams = 0;
		}
	}*/
	return c;
}

string LinkNr2Str(SyntaxTree *s,int kind,int nr);

// may not use AddConstant()!!!
Command *SyntaxTree::PreProcessCommandAddresses(Command *c)
{
	msg_db_f("PreProcessCommandAddr", 4);
	/*msg_write(Kind2Str(c->Kind));
	if (c->script)
		msg_write(LinkNr2Str(c->script->pre_script, c->Kind, c->LinkNr));
	else if (s)
		msg_write(LinkNr2Str(s->pre_script, c->Kind, c->LinkNr));*/

	// recursion
	if (c->kind == KIND_BLOCK){
		for (int i=0;i<c->as_block()->command.num;i++)
			c->as_block()->set(i, PreProcessCommandAddresses(c->as_block()->command[i]));
	}
	for (int i=0;i<c->param.num;i++)
		c->set_param(i, PreProcessCommandAddresses(c->param[i]));
	if (c->instance)
		c->set_instance(PreProcessCommandAddresses(c->instance));
	

	// process...
	if (c->kind == KIND_OPERATOR){
		PreOperator *o = &PreOperators[c->link_no];
		if (o->func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->param.num;i++)
				if (c->param[i]->kind == KIND_ADDRESS)
					is_address = true;
				else if (c->param[i]->kind == KIND_LOCAL_ADDRESS)
					is_address = is_local = true;
				else if (c->param[i]->kind != KIND_CONSTANT)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->func;
				if (is_address){
					// pre process address
					string d1 = string((char*)&c->param[0]->link_no, 4);
					string d2 = string((char*)&c->param[1]->link_no, 4);
					if (c->param[0]->kind == KIND_CONSTANT)
					    d1 = constants[c->param[0]->link_no].value;
					if (c->param[1]->kind == KIND_CONSTANT)
					    d2 = constants[c->param[1]->link_no].value;
					string r = "--------";
					f(r, d1, d2);
					return AddCommand(is_local ? KIND_LOCAL_ADDRESS : KIND_ADDRESS, *(int*)r.data, c->type);
				}
			}
		}
	}else if (c->kind == KIND_REFERENCE){
		if (c->script){
			if ((c->param[0]->kind == KIND_VAR_GLOBAL) || (c->param[0]->kind == KIND_VAR_LOCAL) || (c->param[0]->kind == KIND_CONSTANT)){
				// pre process ref var
				if (c->param[0]->kind == KIND_VAR_GLOBAL){
					return AddCommand(KIND_ADDRESS, (long)c->param[0]->script->g_var[c->param[0]->link_no], c->type, c->param[0]->script);
				}else if (c->param[0]->kind == KIND_VAR_LOCAL){
					return AddCommand(KIND_LOCAL_ADDRESS, (long)cur_func->var[c->param[0]->link_no]._offset, c->type);
				}else /*if (c->param[0]->kind == KindConstant)*/{
					return AddCommand(KIND_ADDRESS, (long)c->param[0]->script->cnst[c->param[0]->link_no], c->type, c->param[0]->script);
				}
			}
		}
	}else if (c->kind == KIND_DEREFERENCE){
		if (c->param[0]->kind == KIND_ADDRESS){
			// pre process deref address
			return AddCommand(KIND_MEMORY, c->param[0]->link_no, c->type);
		}else if (c->param[0]->kind == KIND_LOCAL_ADDRESS){
			// pre process deref local address
			return AddCommand(KIND_LOCAL_MEMORY, c->param[0]->link_no, c->type);
		}
	}
	return c;
}

void SyntaxTree::PreProcessor()
{
	msg_db_f("PreProcessor", 4);
	foreach(Function *f, functions){
		cur_func = f;
		foreachi(Command *c, f->block->command, i)
			f->block->command[i] = PreProcessCommand(c);
	}
	//Show();
}

void SyntaxTree::PreProcessorAddresses()
{
	msg_db_f("PreProcessorAddr", 4);
	foreach(Function *f, functions){
		cur_func = f;
		foreachi(Command *c, f->block->command, i)
			f->block->command[i] = PreProcessCommandAddresses(c);
	}
	//Show();
}

};
