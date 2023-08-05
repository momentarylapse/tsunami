#include "../kaba.h"
#include "../asm/asm.h"
#include "../dynamic/call.h"
#include "../../os/msg.h"

namespace kaba {

#if 0
void db_out(const string &s) {
	msg_write(s);
}
#else
#define db_out(x)
#endif


Array<void*> prepare_const_func_params(shared<Node> c, bool allow_placeholder) {
	Array<void*> p;
	for (int i=0;i<c->params.num;i++) {
		if (c->params[i]->kind == NodeKind::DEREFERENCE and c->params[i]->params[0]->kind == NodeKind::CONSTANT) {
			db_out("pp: " + c->params[i]->params[0]->str(tree->base_class));
			p.add(*(void**)c->params[i]->params[0]->as_const()->p());
		} else if (c->params[i]->kind == NodeKind::CONSTANT) {
			db_out("p: " + c->params[i]->str(tree->base_class));
			p.add(c->params[i]->as_const()->p());
		} else if (c->params[i]->kind == NodeKind::PLACEHOLDER and allow_placeholder) {
			p.add(nullptr);
		} else {
			return {};
		}
	}
	return p;
}

shared<Node> eval_function_call(SyntaxTree *tree, shared<Node> c, Function *f) {
	db_out("??? " + f->signature());

	if (!f->is_pure())
		return c;
	db_out("-pure");

	if (!Value::can_init(f->literal_return_type))
		return c;
	db_out("-constr");

	void *ff = f->address_preprocess;
	if (!ff)
		return c;
	db_out("-addr");

	// parameters
	auto p = prepare_const_func_params(c, false);
	if (p.num < c->params.num)
		return c;
	db_out("-param const");

	Value temp;
	temp.init(f->literal_return_type);
	if (!call_function(f, temp.p(), p))
		return c;
	auto r = add_node_const(tree->add_constant(f->literal_return_type));
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
	if (type->is_list()) {
		((DynamicArray*)p)->init(host_size(type->get_array_element()));
	} else {
		for (auto &el: type->elements)
			rec_init((char*)p + el.offset, el.type);
	}
}

void rec_delete(void *p, const Class *type) {
	if (type->is_list()) {
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
	if (type->is_list()) {
		auto aa = (DynamicArray*)a;
		auto bb = (DynamicArray*)b;
		rec_resize(aa, bb->num, type);
		for (int i=0; i<bb->num; i++)
			rec_assign(ar_el(aa, i), ar_el(bb, i), type->param[0]);

	} else if (type->can_memcpy()){
		memcpy(a, b, type->size);
	} else {
		for (auto &el: type->elements)
			rec_assign((char*)a + el.offset, (char*)b + el.offset, el.type);
	}
}

shared<Node> eval_constructor_function(SyntaxTree *tree, shared<Node> c, Function *f) {
	db_out("EVAL CONSTRUCTOR");
	auto t = f->name_space;
	if (!Value::can_init(t))
		return c;

	void *ff = f->address_preprocess;
	if (!ff)
		return c;
	db_out("-addr");

	// parameters
	auto p = prepare_const_func_params(c, true);
	if (p.num < c->params.num)
		return c;
	db_out("-param const");

	Value temp;
	temp.init(t);
	p[0] = temp.p();
	if (!call_function(f, nullptr, p))
		return c;
	auto r = add_node_const(tree->add_constant(t));
	r->as_const()->set(temp);
	db_out(">>>  " + r->str(tree->base_class));
	return r;
}

// BEFORE transforming to call-by-reference!
shared<Node> SyntaxTree::conv_eval_const_func(shared<Node> c) {
	if (c->kind == NodeKind::OPERATOR) {
		return eval_function_call(this, c, c->as_op()->f);
	} else if (c->kind == NodeKind::CONSTRUCTOR_AS_FUNCTION) {
		return eval_constructor_function(this, c, c->as_func());
	} else if (c->kind == NodeKind::CALL_FUNCTION) {
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
	if (c->kind == NodeKind::CALL_INLINE) {
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
		if (!call_function(f, &new_addr, p))
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
		transform([this] (shared<Node> n) {
			return conv_eval_const_func(n);
		});
	} else {
		transform([this] (shared<Node> n) {
			return conv_eval_const_func_nofunc(n);
		});
	}
}

void SyntaxTree::pre_processor_addresses() {
	transform([this] (shared<Node> n) {
		return pre_process_node_addresses(n);
	});
}

};
