#include "../kaba.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <type_traits>

namespace kaba {

typedef void op_func(Value&, Value&, Value&);


void db_out(const string &s) {
//	msg_write(s);
}

#define CALL_DEBUG_X		0

// call-by-reference dummy
class CBR {};

class vec2 { float a; float b; };
class vec3 { float a; float b; float c; };
class vec4 { float a; float b; float c; float d; };

void call0_void(void *ff, void *ret, const Array<void*> &param) {
	((void(*)())ff)();
}

template<class R>
void call0(void *ff, void *ret, const Array<void*> &param) {
	if (std::is_same<CBR,R>::value) {
		//msg_write("CBR return (1p)!!!");
		((void(*)(void*))ff)(ret);
	} else {
		*(R*)ret = ((R(*)())ff)();
	}
}

template<class A>
void call1_void_x(void *ff, void *ret, const Array<void*> &param) {
	if (std::is_same<CBR,A>::value) {
		db_out("CBR -> void");
		((void(*)(void*))ff)(param[0]);
	} else {
		db_out("x -> void");
		((void(*)(A))ff)(*(A*)param[0]);
	}
}

template<class R, class A>
void call1(void *ff, void *ret, const Array<void*> &param) {
	if (std::is_same<CBR,R>::value) {
		if (std::is_same<CBR,A>::value) {
			db_out("CBR -> CBR");
			((void(*)(void*, void*))ff)(ret, param[0]);
		} else {
			db_out("x -> CBR");
			((void(*)(void*, A))ff)(ret, *(A*)param[0]);
		}
	} else {
		if (std::is_same<CBR,A>::value) {
			db_out("CBR -> x");
			*(R*)ret = ((R(*)(void*))ff)(param[0]);
		} else {
			db_out("x -> x");
			*(R*)ret = ((R(*)(A))ff)(*(A*)param[0]);
		}
	}
}

template<class R, class A, class B>
void call2(void *ff, void *ret, const Array<void*> &param) {
	if (std::is_same<CBR,R>::value) {
		if (std::is_same<CBR,A>::value and std::is_same<CBR,B>::value) {
			db_out("CBR CBR -> CBR");
			((void(*)(void*, void*, void*))ff)(ret, param[0], param[1]);
		} else if (std::is_same<CBR,A>::value) {
			db_out("CBR x -> CBR");
			((void(*)(void*, void*, B))ff)(ret, param[0], *(B*)param[1]);
		} else {
			db_out("x x -> CBR");
			((void(*)(void*, A, B))ff)(ret, *(A*)param[0], *(B*)param[1]);
		}

	} else {
		if (std::is_same<CBR,A>::value) {
			db_out("CBR x -> x");
			*(R*)ret = ((R(*)(void*, B))ff)(param[0], *(B*)param[1]);
		} else {
			db_out("x x -> x");
			*(R*)ret = ((R(*)(A, B))ff)(*(A*)param[0], *(B*)param[1]);
		}
	}
}

template<class R, class A, class B, class C>
void call3(void *ff, void *ret, const Array<void*> &param) {
	if (std::is_same<CBR,R>::value) {
		((void(*)(void*, A, B, C))ff)(ret, *(A*)param[0], *(B*)param[1], *(C*)param[2]);
	} else {
		*(R*)ret = ((R(*)(A, B, C))ff)(*(A*)param[0], *(B*)param[1], *(C*)param[2]);
	}
}

template<class R, class A, class B, class C, class D>
void call4(void *ff, void *ret, const Array<void*> &param) {
	if (std::is_same<CBR,R>::value) {
		((void(*)(void*, A, B, C, D))ff)(ret, *(A*)param[0], *(B*)param[1], *(C*)param[2], *(D*)param[3]);
	} else {
		*(R*)ret = ((R(*)(A, B, C, D))ff)(*(A*)param[0], *(B*)param[1], *(C*)param[2], *(D*)param[3]);
	}
}

// void*,int,int64,float,float64,char,bool,string,vector,complex

// BEFORE call-by-ref transformation!!!
bool call_function(Function *f, void *ff, void *ret, const Array<void*> &param) {
	db_out("eval: " + f->signature());
	Array<const Class*> ptype = f->literal_param_type;
	if (!f->is_static())
		ptype.insert(f->name_space, 0);


	if (ptype.num == 0) {
		if (f->return_type == TypeVoid) {
			call0_void(ff, ret, param);
			return true;
		} else if (f->return_type == TypeInt) {
			call0<int>(ff, ret, param);
			return true;
		} else if (f->return_type == TypeFloat32) {
			call0<float>(ff, ret, param);
			return true;
		} else if (f->return_type->uses_return_by_memory()) {
			call0<CBR>(ff, ret, param);
			return true;
		}
	} else if (ptype.num == 1) {
		if (f->return_type == TypeVoid) {
			if (ptype[0] == TypeInt) {
				call1_void_x<int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1_void_x<float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
#if CALL_DEBUG_X
				void *ppp, *qqq;
				asm volatile ("movq %%rsp, %%rax;"
				              "movq %%rax, %0;"
							"movq %%rbp, %%rax;"
		  "movq %%rax, %1;"
				                  :  "=r" (ppp), "=r"(qqq) : : );
				printf("stack before  sp=%p  bp=%p\n", ppp, qqq);
#endif
				call1_void_x<CBR>(ff, ret, param);
#if CALL_DEBUG_X
				asm volatile ("movq %%rsp, %%rax;"
				              "movq %%rax, %0;"
							"movq %%rbp, %%rax;"
		  "movq %%rax, %1;"
				                  :  "=r" (ppp), "=r"(qqq) : : );
				printf("stack after  sp=%p  bp=%p\n", ppp, qqq);
#endif
				return true;
			}
		} else if (f->return_type == TypeInt) {
			if (ptype[0] == TypeInt) {
				call1<int,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeChar) {
				call1<int,char>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<int,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<int,CBR>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeBool or f->return_type == TypeChar) {
			if (ptype[0] == TypeInt) {
				call1<char,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<char,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<char,CBR>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeFloat32) {
			if (ptype[0] == TypeInt) {
				call1<float,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<float,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<float,CBR>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeQuaternion) {
			if (ptype[0]->uses_call_by_reference()) {
				call1<vec4,CBR>(ff, ret, param);
				return true;
			}
		} else if (f->return_type->uses_return_by_memory()) {
			if (ptype[0] == TypeInt) {
				call1<CBR,int>(ff, ret, param);
				return true;
			} else if (ptype[0] == TypeFloat32) {
				call1<CBR,float>(ff, ret, param);
				return true;
			} else if (ptype[0]->is_pointer()) {
				call1<CBR,void*>(ff, ret, param);
				return true;
			} else if (ptype[0]->uses_call_by_reference()) {
				call1<CBR,CBR>(ff, ret, param);
				return true;
			}
		}
	} else if (ptype.num == 2) {
		if (f->return_type == TypeInt) {
			if ((ptype[0] == TypeInt) and(ptype[1] == TypeInt)) {
				call2<int,int,int>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeFloat32) {
			if ((ptype[0] == TypeFloat32) and(ptype[1] == TypeFloat32)) {
				call2<float,float,float>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeInt64) {
			if ((ptype[0] == TypeInt64) and(ptype[1] == TypeInt64)) {
				call2<int64,int64,int64>(ff, ret, param);
				return true;
			} else if ((ptype[0] == TypeInt64) and(ptype[1] == TypeInt)) {
				call2<int64,int64,int>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeComplex) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32)) {
				call2<vec2,float,float>(ff, ret, param);
				return true;
			}
		} else if (f->return_type == TypeQuaternion) {
			if ((ptype[0]->uses_call_by_reference()) and(ptype[1] == TypeFloat32)) {
				call2<vec4,CBR,float>(ff, ret, param);
				return true;
			}
		} else if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == TypeInt) and(ptype[1] == TypeInt)) {
				call2<CBR,int,int>(ff, ret, param);
				return true;
			} else if ((ptype[0] == TypeFloat32) and(ptype[1] == TypeFloat32)) {
				call2<CBR,float,float>(ff, ret, param);
				return true;
			} else if ((ptype[0]->uses_call_by_reference()) and (ptype[1]->uses_call_by_reference())) {
				call2<CBR,CBR,CBR>(ff, ret, param);
				return true;
			}
		}
	} else if (ptype.num == 3) {
		if (f->return_type == TypeVector) {
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32)) {
				call3<vec3,float,float,float>(ff, ret, param);
				return true;
			}
		}
		/*if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == TypeFloat32) and(ptype[1] == TypeFloat32) and(ptype[2] == TypeFloat32)) {
				((void(*)(void*, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2]);
				return true;
			}
		}*/
	} else if (ptype.num == 4) {
		if (f->return_type->_amd64_allow_pass_in_xmm() and (f->return_type->size == 16)) { // rect, color, plane, quaternion
			if ((ptype[0] == TypeFloat32) and (ptype[1] == TypeFloat32) and (ptype[2] == TypeFloat32) and (ptype[3] == TypeFloat32)) {
				call4<vec4,float,float,float,float>(ff, ret, param);
				return true;
			}
		}
		/*if (f->return_type->uses_return_by_memory()) {
			if ((ptype[0] == TypeFloat32) and(ptype[1] == TypeFloat32) and(ptype[2] == TypeFloat32) and(ptype[3] == TypeFloat32)) {
				((void(*)(void*, float, float, float, float))ff)(ret, *(float*)param[0], *(float*)param[1], *(float*)param[2], *(float*)param[3]);
				return true;
			}
		}*/
	}
	db_out(".... NOPE");
	return false;
}


shared<Node> eval_function_call(SyntaxTree *tree, shared<Node> c, Function *f) {
	db_out("??? " + f->signature());

	if (!f->is_pure())
		return c;
	db_out("-pure");

	if (!Value::can_init(f->return_type))
		return c;
	db_out("-constr");

	void *ff = f->address_preprocess;
	if (!ff)
		return c;
	db_out("-addr");

	// parameters
	Array<void*> p;
	for (int i=0;i<c->params.num;i++) {
		if (c->params[i]->kind == NodeKind::DEREFERENCE and c->params[i]->params[0]->kind == NodeKind::CONSTANT) {
			db_out("pp: " + c->params[i]->params[0]->str(tree->base_class));
			p.add(*(void**)c->params[i]->params[0]->as_const()->p());
		} else if (c->params[i]->kind == NodeKind::CONSTANT) {
			db_out("p: " + c->params[i]->str(tree->base_class));
			p.add(c->params[i]->as_const()->p());
		} else {
			return c;
		}
	}
	db_out("-param const");

	Value temp;
	temp.init(f->return_type);
	if (!call_function(f, ff, temp.p(), p))
		return c;
	auto r = tree->add_node_const(tree->add_constant(f->return_type));
	r->as_const()->set(temp);
	db_out(">>>  " + r->str(tree->base_class));
	return r;
}

bool all_params_are_const(shared<Node> n) {
	for (int i=0; i<n->params.num; i++)
		if (n->params[i]->kind != NodeKind::CONSTANT)
			return false;
	return true;
}

void rec_resize(DynamicArray *ar, int num, const Class *type);

void *ar_el(DynamicArray *ar, int i) {
	return (char*)ar->data + i * ar->element_size;
}

int host_size(const Class *_type);

void rec_init(void *p, const Class *type) {
	if (type->is_super_array()) {
		((DynamicArray*)p)->init(host_size(type->get_array_element()));
	} else {
		for (auto &el: type->elements)
			rec_init((char*)p + el.offset, el.type);
	}
}

void rec_delete(void *p, const Class *type) {
	if (type->is_super_array()) {
		auto ar = (DynamicArray*)p;
		rec_resize(ar, 0, type->param[0]);
		ar->simple_clear();
	} else {
		for (auto &el: type->elements)
			rec_delete((char*)p + el.offset, el.type);
	}
}

void rec_resize(DynamicArray *ar, int num, const Class *type) {
	auto *t_el = type->param[0];
	int num_old = ar->num;

	for (int i=num; i<num_old; i++)
		rec_delete(ar_el(ar, i), t_el);

	ar->simple_resize(num);

	for (int i=num_old; i<num; i++)
		rec_init(ar_el(ar, i), t_el);
}

void rec_assign(void *a, void *b, const Class *type) {
	if (type->is_super_array()) {
		auto aa = (DynamicArray*)a;
		auto bb = (DynamicArray*)b;
		rec_resize(aa, bb->num, type);
		for (int i=0; i<bb->num; i++)
			rec_assign(ar_el(aa, i), ar_el(bb, i), type->param[0]);

	} else if (type->is_simple_class()){
		memcpy(a, b, type->size);
	} else {
		for (auto &el: type->elements)
			rec_assign((char*)a + el.offset, (char*)b + el.offset, el.type);
	}
}

// BEFORE transforming to call-by-reference!
shared<Node> SyntaxTree::conv_eval_const_func(shared<Node> c) {
	if (c->kind == NodeKind::OPERATOR) {
		return eval_function_call(this, c, c->as_op()->f);
	} else if (c->kind == NodeKind::FUNCTION_CALL) {
		return eval_function_call(this, c, c->as_func());
	}
	return conv_eval_const_func_nofunc(c);
}

shared<Node> SyntaxTree::conv_eval_const_func_nofunc(shared<Node> c) {
	if (c->kind == NodeKind::DYNAMIC_ARRAY) {
		if (all_params_are_const(c)) {
			auto cr = add_node_const(add_constant(c->type));
			DynamicArray *da = &c->params[0]->as_const()->as_array();
			int index = c->params[1]->as_const()->as_int();
			rec_assign(cr->as_const()->p(), (char*)da->data + index * da->element_size, c->type);
			return cr;
		}
	} else if (c->kind == NodeKind::ARRAY) {
		// hmmm, not existing, I guess....
		if (all_params_are_const(c)) {
			auto cr = add_node_const(add_constant(c->type));
			int index = c->params[1]->as_const()->as_int();
			rec_assign(cr->as_const()->p(), (char*)c->params[0]->as_const()->p() + index * c->type->size, c->type);
			return cr;
		}
	} else if (c->kind == NodeKind::ARRAY_BUILDER) {
		if (all_params_are_const(c)) {
			auto c_array = add_node_const(add_constant(c->type));
			DynamicArray &da = c_array->as_const()->as_array();
			rec_resize(&da, c->params.num, c->type);
			for (int i=0; i<c->params.num; i++)
				rec_assign(ar_el(&da, i), c->params[i]->as_const()->p(), c->type->param[0]);
			return c_array;
		}
	}

	//else if (c->kind == KindReference) {
	// no... we don't know the addresses of globals/constants yet...
	return c;
}


// may not use AddConstant()!!!
shared<Node> SyntaxTree::pre_process_node_addresses(shared<Node> c) {
	if (c->kind == NodeKind::INLINE_CALL) {
		auto *f = c->as_func();
		//if (!f->is_pure or !f->address_preprocess)
		//	return c;
		if ((f->inline_no != InlineID::INT64_ADD_INT) and (f->inline_no != InlineID::INT_ADD))
			return c;
		if (c->params[1]->kind != NodeKind::CONSTANT)
			return c;
		Array<void*> p;
		if ((c->params[0]->kind == NodeKind::ADDRESS) or (c->params[0]->kind == NodeKind::LOCAL_ADDRESS)) {
			p.add((void*)&c->params[0]->link_no);
		} else {
			return c;
		}
		p.add(c->params[1]->as_const()->p());

		int64 new_addr;
		if (!call_function(f, f->address_preprocess, &new_addr, p))
			return c;

		c->params[0]->link_no = new_addr;
		return c->params[0].get();

	} else if (c->kind == NodeKind::REFERENCE) {
		auto p0 = c->params[0];
		if (p0->kind == NodeKind::VAR_GLOBAL) {
			return new Node(NodeKind::ADDRESS, (int_p)p0->as_global_p(), c->type);
		} else if (p0->kind == NodeKind::VAR_LOCAL) {
			return new Node(NodeKind::LOCAL_ADDRESS, (int_p)p0->as_local()->_offset, c->type);
		} else if (p0->kind == NodeKind::CONSTANT) {
			return new Node(NodeKind::ADDRESS, (int_p)p0->as_const_p(), c->type);
		}
	} else if (c->kind == NodeKind::DEREFERENCE) {
		auto p0 = c->params[0];
		if (p0->kind == NodeKind::ADDRESS) {
			return new Node(NodeKind::MEMORY, p0->link_no, c->type);
		} else if (p0->kind == NodeKind::LOCAL_ADDRESS) {
			return new Node(NodeKind::LOCAL_MEMORY, p0->link_no, c->type);
		}
	}
	return c;
}

void SyntaxTree::eval_const_expressions(bool allow_func_eval) {
	if (allow_func_eval) {
		transform([&](shared<Node> n){ return conv_eval_const_func(n); });
	} else {
		transform([&](shared<Node> n){ return conv_eval_const_func_nofunc(n); });
	}
}

void SyntaxTree::pre_processor_addresses() {
	transform([&](shared<Node> n){ return pre_process_node_addresses(n); });
}

};
