#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"

namespace Kaba{

typedef void op_func(Value &r, Value &a, Value &b);

//static Function *cur_func;

template<class R>
void call0(void *ff, void *ret, const Array<void*> &param) {
	*(R*)ret = ((R(*)())ff)();
}
template<class R, class A>
void call1(void *ff, void *ret, const Array<void*> &param) {
	*(R*)ret = ((R(*)(A))ff)(*(A*)param[0]);
}
template<class R, class A, class B>
void call2(void *ff, void *ret, const Array<void*> &param) {
	*(R*)ret = ((R(*)(A, B))ff)(*(A*)param[0], *(B*)param[1]);
}
template<class R, class A, class B, class C>
void call3(void *ff, void *ret, const Array<void*> &param) {
	*(R*)ret = ((R(*)(A, B, C))ff)(*(A*)param[0], *(B*)param[1], *(C*)param[2]);
}
template<class R, class A, class B, class C, class D>
void call4(void *ff, void *ret, const Array<void*> &param) {
	*(R*)ret = ((R(*)(A, B, C, D))ff)(*(A*)param[0], *(B*)param[1], *(C*)param[2], *(D*)param[3]);
}

bool call_function(Function *f, void *ff, void *ret, void *__inst, const Array<void*> &param) {
	if (f->num_params == 0) {
		if (f->return_type == TypeInt) {
			call0<int>(ff, ret, param);
			return true;
		} else if (f->return_type == TypeFloat32) {
			call0<float>(ff, ret, param);
			return true;
		} else if (f->return_type->uses_return_by_memory()) {
			//call1<void,void*>(ff, ret, param);
			((void(*)(void*))ff)(ret);
			return true;
		}
	}else if (f->num_params == 1) {
		if (f->return_type == TypeInt) {
			if (f->literal_param_type[0] == TypeInt) {
				call1<int,int>(ff, ret, param);
				return true;
			} else if (f->literal_param_type[0] == TypeFloat32) {
				call1<int,float>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeBool) {
			if (f->literal_param_type[0] == TypeInt) {
				call1<bool,int>(ff, ret, param);
				return true;
			} else if (f->literal_param_type[0] == TypeFloat32) {
				call1<bool,float>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeFloat32) {
			if (f->literal_param_type[0] == TypeFloat32) {
				call1<float,float>(ff, ret, param);
				return true;
			}
		} else if (f->return_type->uses_return_by_memory()) {
			if (f->literal_param_type[0] == TypeInt) {
				((void(*)(void*, int))ff)(ret, *(int*)param[0]);
				return true;
			} else if (f->literal_param_type[0] == TypeFloat32) {
				((void(*)(void*, float))ff)(ret, *(float*)param[0]);
				return true;
			} else if (f->literal_param_type[0]->uses_call_by_reference()) {
				((void(*)(void*, void*))ff)(ret, param[0]);
				return true;
			}
		}
	}else if (f->num_params == 2){
		if (f->return_type == TypeInt){
			if ((f->literal_param_type[0] == TypeInt) and(f->literal_param_type[1] == TypeInt)){
				call2<int,int,int>(ff, ret, param);
				return true;
			}
		}else if (f->return_type == TypeFloat32){
			if ((f->literal_param_type[0] == TypeFloat32) and(f->literal_param_type[1] == TypeFloat32)){
				call2<float,float,float>(ff, ret, param);
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
						int nc = ps->add_constant(o->return_type);
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


Node *SyntaxTree::pre_process_node(Node *c)
{
	if (c->kind == KIND_OPERATOR){
		Operator *o = c->as_op();
		/*if (c->link_nr == OperatorIntAdd){
			if (c->param[1]->kind == KindConstant){
				int v = *(int*)Constants[c->param[1]->link_nr].data;
				if (v == 0){
					msg_error("addr + 0");
					*c = *c->param[0];
				}
			}
		}else*/ if (o->f->address_preprocess){
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
				op_func *f = (op_func*)o->f->address_preprocess;
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
					Node *r = add_node_const(add_constant(o->return_type));
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
	}else if (c->kind == KIND_FUNCTION_CALL){
		Function *f = c->as_func();
		if (!f->is_pure)
			return c;
		if (f->return_type->get_default_constructor()) // TODO
			return c;
		void *ff = f->address_preprocess;
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
		Node *r = add_node_const(add_constant(f->return_type));
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
			Node *c_array = add_node_const(add_constant(c->type));
			int el_size = c->type->parent->size;
			DynamicArray *da = &c_array->as_const()->as_array();
			da->init(el_size);
			da->simple_resize(c->params.num);
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


// may not use AddConstant()!!!
Node *SyntaxTree::pre_process_node_addresses(Node *c)
{
	if (c->kind == KIND_OPERATOR){
		Operator *o = c->as_op();
		if (o->f->address){
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
				op_func *f = (op_func*)o->f->address;
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
					return new Node(is_local ? KIND_LOCAL_ADDRESS : KIND_ADDRESS, *(int_p*)r.p(), c->type);
				}
			}
		}
	}else if (c->kind == KIND_REFERENCE){
		Node *p0 = c->params[0];
		if (p0->kind == KIND_VAR_GLOBAL){
			return new Node(KIND_ADDRESS, (int_p)p0->as_global_p(), c->type);
		}else if (p0->kind == KIND_VAR_LOCAL){
			return new Node(KIND_LOCAL_ADDRESS, (int_p)p0->as_local()->_offset, c->type);
		}else if (p0->kind == KIND_CONSTANT){
			return new Node(KIND_ADDRESS, (int_p)p0->as_const_p(), c->type);
		}
	}else if (c->kind == KIND_DEREFERENCE){
		Node *p0 = c->params[0];
		if (p0->kind == KIND_ADDRESS){
			return new Node(KIND_MEMORY, p0->link_no, c->type);
		}else if (p0->kind == KIND_LOCAL_ADDRESS){
			return new Node(KIND_LOCAL_MEMORY, p0->link_no, c->type);
		}
	}
	return c;
}

void SyntaxTree::pre_processor()
{
	transform([&](Node *n){ return pre_process_node(n); });
	//Show();
}

void SyntaxTree::pre_processor_addresses()
{
	transform([&](Node *n){ return pre_process_node_addresses(n); });
	//Show();
}

};
