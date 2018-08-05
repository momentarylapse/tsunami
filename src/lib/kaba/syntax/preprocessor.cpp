#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"

namespace Kaba{

typedef void op_func(Value &r, Value &a, Value &b);

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
		}else if (f->return_type->uses_return_by_memory()){
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
		}else if (f->return_type->uses_return_by_memory()){
			if (f->literal_param_type[0] == TypeInt){
				((void(*)(void*, int))ff)(ret, *(int*)param[0]);
				return true;
			}else if (f->literal_param_type[0] == TypeFloat32){
				((void(*)(void*, float))ff)(ret, *(float*)param[0]);
				return true;
			}else if (f->literal_param_type[0]->uses_call_by_reference()){
				((void(*)(void*, void*))ff)(ret, param[0]);
				return true;
			}
		}
	}else if (f->num_params == 2){
		if (f->return_type == TypeInt){
			if ((f->literal_param_type[0] == TypeInt) and(f->literal_param_type[1] == TypeInt)){
				*(int*)ret = ((int(*)(int, int))ff)(*(int*)param[0], *(int*)param[1]);
				return true;
			}
		}else if (f->return_type == TypeFloat32){
			if ((f->literal_param_type[0] == TypeFloat32) and(f->literal_param_type[1] == TypeFloat32)){
				*(float*)ret = ((float(*)(float, float))ff)(*(float*)param[0], *(float*)param[1]);
				return true;
			}
		}else if (f->return_type->uses_return_by_memory()){
			if ((f->literal_param_type[0] == TypeInt) and(f->literal_param_type[1] == TypeInt)){
				((void(*)(void*, int, int))ff)(ret, *(int*)param[0], *(int*)param[1]);
				return true;
			}else if ((f->literal_param_type[0] == TypeFloat32) and(f->literal_param_type[1] == TypeFloat32)){
				((void(*)(void*, float, float))ff)(ret, *(float*)param[0], *(float*)param[1]);
				return true;
			}else if ((f->literal_param_type[0]->uses_call_by_reference()) and(f->literal_param_type[1]->uses_call_by_reference())){
				((void(*)(void*, void*, void*))ff)(ret, param[0], param[1]);
				return true;
			}
		}
	}else if (f->num_params == 3){
		if (f->return_type->uses_return_by_memory()){
			if ((f->literal_param_type[0] == TypeFloat32) and(f->literal_param_type[1] == TypeFloat32) and(f->literal_param_type[2] == TypeFloat32)){
				((void(*)(void*, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2]);
				return true;
			}
		}
	}else if (f->num_params == 4){
		if (f->return_type->uses_return_by_memory()){
			if ((f->literal_param_type[0] == TypeFloat32) and(f->literal_param_type[1] == TypeFloat32) and(f->literal_param_type[2] == TypeFloat32) and(f->literal_param_type[3] == TypeFloat32)){
				((void(*)(void*, float, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2], *(float*)param[3]);
				return true;
			}
		}
	}
	return false;
}


#if 0
void PreProcessFunction(SyntaxTree *ps, Node *c)
{
	bool all_const = true;
	bool is_address = false;
	bool is_local = false;
	for (int i=0;i<c->num_params;i++)
		if (c->params[i]->kind == KIND_ADDRESS)
			is_address = true;
		else if (c->params[i]->kind == KIND_LOCAL_ADDRESS)
			is_address = is_local = true;
		else if (c->params[i]->kind != KIND_CONSTANT)
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
						string d1 = ps->constants[c->params[0]->link_no].value;
						string d2;
						if (c->num_params > 1)
							d2 = ps->constants[c->params[1]->link_no].value;
						f(ps->constants[nc].value, d1, d2);
						c->script = ps->script;
						c->kind = KIND_CONSTANT;
						c->link_no = nc;
						c->num_params = 0;
					}
}
#endif


Node *SyntaxTree::PreProcessNode(Node *c)
{
	// recursion
	if (c->kind == KIND_BLOCK){
		for (int i=0;i<c->as_block()->nodes.num;i++)
			c->as_block()->nodes[i] = PreProcessNode(c->as_block()->nodes[i]);
	}
	for (int i=0;i<c->params.num;i++)
		c->set_param(i, PreProcessNode(c->params[i]));
	if (c->instance)
		c->set_instance(PreProcessNode(c->instance));
	

	// process...
	if (c->kind == KIND_OPERATOR){
		Operator *o = &operators[c->link_no];
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
			for (int i=0;i<c->params.num;i++)
				if (c->params[i]->kind == KIND_ADDRESS)
					is_address = true;
				else if (c->params[i]->kind == KIND_LOCAL_ADDRESS)
					is_address = is_local = true;
				else if (c->params[i]->kind != KIND_CONSTANT)
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
					Node *r = add_node_const(AddConstant(o->return_type));
					if (c->params.num > 1){
						f(*r->as_const(), *c->params[0]->as_const(), *c->params[1]->as_const());
					}else{
						Value dummy;
						f(*r->as_const(), *c->params[0]->as_const(), dummy);
					}
					return r;
				}
			}
		}
#if 1
	}else if (c->kind == KIND_FUNCTION){
		Function *f = c->as_func();
		if (!f->is_pure)
			return c;
		if (f->return_type->get_default_constructor()) // TODO
			return c;
		void *ff = (void*)c->script->func[c->link_no];
		if (!ff)
			return c;
		bool all_const = true;
		bool is_address = false;
		bool is_local = false;
		for (int i=0;i<c->params.num;i++){
			if (c->params[i]->kind == KIND_ADDRESS)
				is_address = true;
			else if (c->params[i]->kind == KIND_LOCAL_ADDRESS)
				is_address = is_local = true;
			else if (c->params[i]->kind != KIND_CONSTANT)
				all_const = false;
		}
		void *inst = nullptr;
		if (c->instance){
			return c;
			if (c->instance->kind != KIND_CONSTANT)
				all_const = false;
			inst = c->instance->as_const()->p();
		}
		if (!all_const)
			return c;
		if (is_address)
			return c;
		Value temp;
		temp.init(f->return_type);
		Array<void*> p;
		for (int i=0; i<c->params.num; i++)
			p.add(c->params[i]->as_const()->p());
		if (!call_function(f, ff, temp.p(), inst, p))
			return c;
		Node *r = add_node_const(AddConstant(f->return_type));
		r->as_const()->set(temp);
		//DoError("...pure function evaluation?!?....TODO");
		return r;
#endif
	}else if (c->kind == KIND_ARRAY_BUILDER){
		bool all_consts = true;
		for (int i=0; i<c->params.num; i++)
			if (c->params[i]->kind != KIND_CONSTANT)
				all_consts = false;
		if (all_consts){
			Node *c_array = add_node_const(AddConstant(c->type));
			int el_size = c->type->parent->size;
			DynamicArray *da = &c_array->as_const()->as_array();
			da->init(el_size);
			da->resize(c->params.num);
			for (int i=0; i<c->params.num; i++)
				memcpy((char*)da->data + el_size * i, c->params[i]->as_const()->p(), el_size);
			return c_array;
		}
	}/*else if (c->kind == KindReference){
	// no... we don't know the addresses of globals/constants yet...
		if (s){
			if ((c->Param[0]->Kind == KindVarGlobal) or (c->Param[0]->Kind == KindVarLocal) or (c->Param[0]->Kind == KindVarExternal) or (c->Param[0]->Kind == KindConstant)){
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

string LinkNr2Str(SyntaxTree *s, int kind, int64 nr);

// may not use AddConstant()!!!
Node *SyntaxTree::PreProcessNodeAddresses(Node *c)
{
	/*msg_write(Kind2Str(c->Kind));
	if (c->script)
		msg_write(LinkNr2Str(c->script->pre_script, c->Kind, c->LinkNr));
	else if (s)
		msg_write(LinkNr2Str(s->pre_script, c->Kind, c->LinkNr));*/

	// recursion
	if (c->kind == KIND_BLOCK){
		for (int i=0;i<c->as_block()->nodes.num;i++)
			c->as_block()->set(i, PreProcessNodeAddresses(c->as_block()->nodes[i]));
	}
	for (int i=0;i<c->params.num;i++)
		c->set_param(i, PreProcessNodeAddresses(c->params[i]));
	if (c->instance)
		c->set_instance(PreProcessNodeAddresses(c->instance));
	

	// process...
	if (c->kind == KIND_OPERATOR){
		Operator *o = &operators[c->link_no];
		if (o->func){
			bool all_const = true;
			bool is_address = false;
			bool is_local = false;
			for (int i=0;i<c->params.num;i++)
				if (c->params[i]->kind == KIND_ADDRESS)
					is_address = true;
				else if (c->params[i]->kind == KIND_LOCAL_ADDRESS)
					is_address = is_local = true;
				else if (c->params[i]->kind != KIND_CONSTANT)
					all_const = false;
			if (all_const){
				op_func *f = (op_func*)o->func;
				if (is_address){
					// pre process address
					Value d1, d2;
					d1.init(c->params[0]->type);
					d2.init(c->params[1]->type);
					*(void**)d1.p() = (void*)c->params[0]->link_no;
					*(void**)d2.p() = (void*)c->params[1]->link_no;
					if (c->params[0]->kind == KIND_CONSTANT)
					    d1.set(*c->params[0]->as_const());
					if (c->params[1]->kind == KIND_CONSTANT)
					    d2.set(*c->params[1]->as_const());
					Value r;
					r.init(c->type);
					f(r, d1, d2);
					return AddNode(is_local ? KIND_LOCAL_ADDRESS : KIND_ADDRESS, *(int_p*)r.p(), c->type);
				}
			}
		}
	}else if (c->kind == KIND_REFERENCE){
		if (c->script){
			if ((c->params[0]->kind == KIND_VAR_GLOBAL) or (c->params[0]->kind == KIND_VAR_LOCAL) or (c->params[0]->kind == KIND_CONSTANT)){
				// pre process ref var
				if (c->params[0]->kind == KIND_VAR_GLOBAL){
					return AddNode(KIND_ADDRESS, (int_p)c->params[0]->script->g_var[c->params[0]->link_no], c->type, c->params[0]->script);
				}else if (c->params[0]->kind == KIND_VAR_LOCAL){
					return AddNode(KIND_LOCAL_ADDRESS, (int_p)cur_func->var[c->params[0]->link_no]._offset, c->type);
				}else /*if (c->param[0]->kind == KindConstant)*/{
					return AddNode(KIND_ADDRESS, (int_p)c->params[0]->script->cnst[c->params[0]->link_no], c->type, c->params[0]->script);
				}
			}
		}
	}else if (c->kind == KIND_DEREFERENCE){
		if (c->params[0]->kind == KIND_ADDRESS){
			// pre process deref address
			return AddNode(KIND_MEMORY, c->params[0]->link_no, c->type);
		}else if (c->params[0]->kind == KIND_LOCAL_ADDRESS){
			// pre process deref local address
			return AddNode(KIND_LOCAL_MEMORY, c->params[0]->link_no, c->type);
		}
	}
	return c;
}

void SyntaxTree::PreProcessor()
{
	for (Function *f: functions){
		cur_func = f;
		foreachi(Node *c, f->block->nodes, i)
			f->block->nodes[i] = PreProcessNode(c);
	}
	//Show();
}

void SyntaxTree::PreProcessorAddresses()
{
	for (Function *f: functions){
		cur_func = f;
		foreachi(Node *c, f->block->nodes, i)
			f->block->nodes[i] = PreProcessNodeAddresses(c);
	}
	//Show();
}

};
