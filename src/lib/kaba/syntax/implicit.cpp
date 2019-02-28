#include "../kaba.h"
#include "../asm/asm.h"
#include <stdio.h>
#include "../../file/file.h"

namespace Kaba{

void SyntaxTree::AutoImplementAddVirtualTable(Node *self, Function *f, const Class *t)
{
	if (t->vtable.num > 0){
		Node *p = shift_node(self, true, 0, TypePointer);
		auto *c = add_constant(TypePointer);
		(*(void**)c->value.data) = t->_vtable_location_target_;
		Node *n_0 = add_node_const(c);
		Node *n_assign = add_node_operator_by_inline(p, n_0, INLINE_POINTER_ASSIGN);
		f->block->add(n_assign);
	}
}

void SyntaxTree::AutoImplementAddChildConstructors(Node *n_self, Function *f, const Class *t)
{
	int i0 = t->parent ? t->parent->elements.num : 0;
	foreachi(ClassElement &e, t->elements, i){
		if (i < i0)
			continue;
		ClassFunction *ff = e.type->get_default_constructor();
		if (!ff)
			continue;
		Node *p = shift_node(cp_node(n_self), true, e.offset, e.type);
		Node *c = add_node_member_call(ff, ref_node(p));
		f->block->add(c);
	}
}

void SyntaxTree::AutoImplementConstructor(Function *f, const Class *t, bool allow_parent_constructor)
{
	if (!f)
		return;
	Node *n_self = add_node_local_var(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array()){
		Node *n_el_size = add_node_const(add_constant(TypeInt));
		n_el_size->as_const()->as_int() = t->parent->size;
		Node *n_mem_init = add_node_member_call(t->get_func("__mem_init__", TypeVoid, {TypeInt}), n_self);
		n_mem_init->set_param(0, n_el_size);
		f->block->add(n_mem_init);
	}else if (t->is_dict()){
		Node *n_el_size = add_node_const(add_constant(TypeInt));
		n_el_size->as_const()->as_int() = t->parent->size + TypeString->size;
		Node *n_mem_init = add_node_member_call(t->get_func("__mem_init__", TypeVoid, {TypeInt}), n_self);
		n_mem_init->set_param(0, n_el_size);
		f->block->add(n_mem_init);
	}else if (t->is_array()){
		auto *pc_el_init = t->parent->get_default_constructor();
		if (pc_el_init){
			for (int i=0; i<t->array_length; i++){
				Node *n_el = shift_node(cp_node(n_self), true, t->parent->size * i, t->parent);
				Node *n_init_el = add_node_member_call(pc_el_init, ref_node(n_el));
				f->block->add(n_init_el);
			}
		}
		delete n_self;
	}else{

		// parent constructor
		if (t->parent and allow_parent_constructor){
			ClassFunction *pc_same = t->parent->get_same_func(IDENTIFIER_FUNC_INIT, f);
			ClassFunction *pc_def = t->parent->get_default_constructor();
			if (pc_same){
				// first, try same signature
				Node *n_init_parent = add_node_member_call(pc_same, cp_node(n_self));
				for (int i=0;i<pc_same->param_types.num;i++)
					n_init_parent->set_param(i, add_node_local_var(f->var[i]));
				f->block->add(n_init_parent);
			}else if (pc_def){
				// then, try default constructor
				f->block->add(add_node_member_call(pc_def, cp_node(n_self)));
			}else if (t->parent->needs_constructor()){
				do_error("can't find a constructor in the parent class with matching signature or a default constructor");
			}
		}

		// call child constructors for elements
		AutoImplementAddChildConstructors(n_self, f, t);

		// add vtable reference
		// after child constructor (otherwise would get overwritten)
		if (t->vtable.num > 0)
			AutoImplementAddVirtualTable(cp_node(n_self), f, t);
		delete n_self;
	}
}

void SyntaxTree::AutoImplementDestructor(Function *f, const Class *t)
{
	if (!f)
		return;
	Node *n_self = add_node_local_var(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array() or t->is_dict()){
		ClassFunction *f_clear = t->get_func("clear", TypeVoid, {});
		if (f_clear){
			f->block->add(add_node_member_call(f_clear, n_self));
		}
	}else if (t->is_array()){
		auto *pc_el_init = t->parent->get_destructor();
		if (pc_el_init){
			for (int i=0; i<t->array_length; i++){
				Node *p = shift_node(cp_node(n_self), true, t->parent->size * i, t->parent);
				Node *c = add_node_member_call(pc_el_init, ref_node(p));
				f->block->add(c);
			}
		}
		delete n_self;
	}else{

		// call child destructors
		int i0 = t->parent ? t->parent->elements.num : 0;
		foreachi(ClassElement &e, t->elements, i){
			if (i < i0)
				continue;
			ClassFunction *ff = e.type->get_destructor();
			if (!ff)
				continue;
			Node *p = shift_node(cp_node(n_self), true, e.offset, e.type);
			f->block->add(add_node_member_call(ff, ref_node(p)));
		}

		// parent destructor
		if (t->parent){
			ClassFunction *ff = t->parent->get_destructor();
			if (ff){
				f->block->add(add_node_member_call(ff, cp_node(n_self), true));
			}
		}
		delete n_self;
	}
}

void SyntaxTree::AutoImplementAssign(Function *f, const Class *t)
{
	if (!f)
		return;
	Node *n_other = add_node_local_var(f->__get_var("other"));
	Node *n_self = add_node_local_var(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array()){

		ClassFunction *f_resize = t->get_func("resize", TypeVoid, {TypeInt});
		if (!f_resize)
			do_error(format("%s.__assign__(): no %s.resize(int) found", t->name.c_str(), t->name.c_str()));

		// self.resize(other.num)
		Node *n_other_num = shift_node(n_other, false, config.pointer_size, TypeInt);

		Node *n_resize = add_node_member_call(f_resize, n_self);
		n_resize->set_num_params(1);
		n_resize->set_param(0, n_other_num);
		f->block->add(n_resize);

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		auto *v_i = f->block->add_var("i", TypeInt);
		Node *n_i = add_node_local_var(v_i);

		Node *n_for = add_node_statement(STATEMENT_FOR);
		f->block->add(n_for);

		// for_var = 0
		Node *n_0 = add_node_const(add_constant(TypeInt));
		n_0->as_const()->as_int() = 0;
		Node *n_assign0 = add_node_operator_by_inline(n_i, n_0, INLINE_INT_ASSIGN);
		n_for->set_param(0, n_assign0);

		// while(for_var < self.num)
		Node *n_cmp = add_node_operator_by_inline(cp_node(n_i), cp_node(n_other_num), INLINE_INT_SMALLER);
		n_for->set_param(1, n_cmp);

		Block *b = new Block(f, f->block);
		n_for->set_param(2, b);

		// el := self.data[for_var]
		Node *n_deref_self = deref_node(cp_node(n_self));
		Node *n_self_data = shift_node(n_deref_self, false, 0, t->parent->get_pointer());
		Node *n_self_el = add_node_parray(n_self_data, cp_node(n_i), t->parent);

		// el2 := other.data[for_var]
		Node *n_other_data = shift_node(cp_node(n_other), false, 0, t->parent->get_pointer());
		Node *n_other_el = add_node_parray(n_other_data, cp_node(n_i), t->parent);


		Node *n_assign = link_operator(OPERATOR_ASSIGN, n_self_el, n_other_el);
		if (!n_assign)
			do_error(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
		b->add(n_assign);

		// ...for_var += 1
		Node *n_inc = add_node_operator_by_inline(cp_node(n_i), n_0 /*dummy*/, INLINE_INT_INCREASE);
		n_for->set_param(3, n_inc);
	}else if (t->is_array()){

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		auto *v_i = f->block->add_var("i", TypeInt);
		Node *n_num = add_node_const(add_constant(TypeInt));
		n_num->as_const()->as_int() = t->array_length;

		Node *n_i = add_node_local_var(v_i);


		Node *n_for = add_node_statement(STATEMENT_FOR);
		f->block->add(n_for);


		// for_var = 0
		Node *n_0 = add_node_const(add_constant(TypeInt));
		n_0->as_const()->as_int() = 0;
		Node *n_assign0 = add_node_operator_by_inline(n_i, n_0, INLINE_INT_ASSIGN);
		n_for->set_param(0, n_assign0);

		// while(for_var < self.num)
		Node *n_cmp = add_node_operator_by_inline(cp_node(n_i), n_num, INLINE_INT_SMALLER);
		n_for->set_param(1, n_cmp);

		Block *b = new Block(f, f->block);
		n_for->set_param(2, b);

		// el := self.data[for_var]
		Node *n_self_el = add_node_parray(n_self, cp_node(n_i), t->parent);

		// el2 := other.data[for_var]
		Node *n_other_el = add_node_parray(ref_node(n_other), cp_node(n_i), t->parent);


		Node *n_assign = link_operator(OPERATOR_ASSIGN, n_self_el, n_other_el);
		if (!n_assign)
			do_error(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
		b->add(n_assign);

		// ...for_var += 1
		Node *n_inc = add_node_operator_by_inline(cp_node(n_i), n_0 /*dummy*/, INLINE_INT_INCREASE);
		n_for->set_param(3, n_inc);
	}else{

		// parent assignment
		if (t->parent){
			Node *p = deref_node(cp_node(n_self));
			Node *o = cp_node(n_other);
			p->type = o->type = t->parent;

			Node *cmd_assign = link_operator(OPERATOR_ASSIGN, p, o);
			if (!cmd_assign)
				do_error(format("%s.__assign__(): no parent %s.__assign__", t->name.c_str(), t->parent->name.c_str()));
			f->block->add(cmd_assign);
		}

		// call child assignment
		int i0 = t->parent ? t->parent->elements.num : 0;
		foreachi(ClassElement &e, t->elements, i){
			if (i < i0)
				continue;
			Node *p = shift_node(cp_node(n_self), true, e.offset, e.type);
			Node *o = shift_node(cp_node(n_other), false, e.offset, e.type); // needed for call-by-ref conversion!

			Node *n_assign = link_operator(OPERATOR_ASSIGN, p, o);
			if (!n_assign)
				do_error(format("%s.__assign__(): no %s.__assign__ for element \"%s\"", t->name.c_str(), e.type->name.c_str(), e.name.c_str()));
			f->block->add(n_assign);
		}

		delete n_self;
	}
}


void SyntaxTree::AutoImplementArrayClear(Function *f, const Class *t)
{
	if (!f)
		return;
	auto *var_i = f->block->add_var("for_var", TypeInt);

	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF));

	Node *self_num = shift_node(cp_node(self), true, config.pointer_size, TypeInt);

	Node *for_var = add_node_local_var(var_i);

// delete...
	ClassFunction *f_del = t->parent->get_destructor();
	if (f_del){
		Node *cmd_for = add_node_statement(STATEMENT_FOR);
		f->block->add(cmd_for);

		// for_var = 0
		Node *cmd_0 = add_node_const(add_constant(TypeInt));
		cmd_0->as_const()->as_int() = 0;
		Node *cmd_assign = add_node_operator_by_inline(for_var, cmd_0, INLINE_INT_ASSIGN);
		cmd_for->set_param(0, cmd_assign);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(cp_node(for_var), self_num, INLINE_INT_SMALLER);
		cmd_for->set_param(1, cmd_cmp);

		Block *b = new Block(f, f->block);
		cmd_for->set_param(2, b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->get_pointer());
		Node *cmd_el = add_node_parray(self_data, cp_node(for_var), t->parent);

		// __delete__
		Node *cmd_delete = add_node_member_call(f_del, ref_node(cmd_el));
		b->add(cmd_delete);

		// for_var ++
		Node *cmd_inc = add_node_operator_by_inline(cp_node(for_var), cmd_0 /*dummy*/, INLINE_INT_INCREASE);
		cmd_for->set_param(3, cmd_inc);
	}

	// clear
	Node *cmd_clear = add_node_member_call(t->get_func("__mem_clear__", TypeVoid, {}), self);
	f->block->add(cmd_clear);
}


void SyntaxTree::AutoImplementArrayResize(Function *f, const Class *t)
{
	if (!f)
		return;
	auto *var = f->block->add_var("for_var", TypeInt);
	f->block->add_var("num_old", TypeInt);

	Node *num = add_node_local_var(f->__get_var("num"));

	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF));

	Node *self_num = shift_node(self, true, config.pointer_size, TypeInt);

	Node *num_old = add_node_local_var(f->__get_var("num_old"));

	// num_old = self.num
	f->block->add(add_node_operator_by_inline(num_old, self_num, INLINE_INT_ASSIGN));

// delete...
	ClassFunction *f_del = t->parent->get_destructor();
	if (f_del){

		Node *cmd_for = add_node_statement(STATEMENT_FOR);
		f->block->add(cmd_for);

		// for_var = num
		Node *for_var = add_node_local_var(var);
		Node *cmd_assign = add_node_operator_by_inline(for_var, cp_node(num), INLINE_INT_ASSIGN);
		cmd_for->set_param(0, cmd_assign);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(cp_node(for_var), cp_node(self_num), INLINE_INT_SMALLER);
		cmd_for->set_param(1, cmd_cmp);

		Block *b = new Block(f, f->block);
		cmd_for->set_param(2, b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->get_pointer());
		Node *cmd_el = add_node_parray(self_data, cp_node(for_var), t->parent);

		// __delete__
		Node *cmd_delete = add_node_member_call(f_del, ref_node(cmd_el));
		b->add(cmd_delete);

		// ...for_var += 1
		Node *cmd_inc = add_node_operator_by_inline(cp_node(for_var), num /*dummy*/, INLINE_INT_INCREASE);
		cmd_for->set_param(3, cmd_inc);
	}

	// resize
	Node *c_resize = add_node_member_call(t->get_func("__mem_resize__", TypeVoid, {TypeInt}), cp_node(self));
	c_resize->set_param(0, num);
	f->block->add(c_resize);

	// new...
	ClassFunction *f_init = t->parent->get_default_constructor();
	if (f_init){
		Node *cmd_for = add_node_statement(STATEMENT_FOR);
		f->block->add(cmd_for);

		// for_var = num_old
		Node *for_var = add_node_local_var(var);
		Node *cmd_assign = add_node_operator_by_inline(for_var, cp_node(num_old), INLINE_INT_ASSIGN);
		cmd_for->set_param(0, cmd_assign);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(cp_node(for_var), cp_node(self_num), INLINE_INT_SMALLER);
		cmd_for->set_param(1, cmd_cmp);

		Block *b = new Block(f, f->block);
		cmd_for->set_param(2, b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->get_pointer());
		Node *cmd_el = add_node_parray(self_data, cp_node(for_var), t->parent);

		// __init__
		Node *cmd_init = add_node_member_call(f_init, ref_node(cmd_el));
		b->add(cmd_init);

		// ...for_var += 1
		Node *cmd_inc = add_node_operator_by_inline(cp_node(for_var), num /*dummy*/, INLINE_INT_INCREASE);
		cmd_for->set_param(3, cmd_inc);
	}
}


void SyntaxTree::AutoImplementArrayRemove(Function *f, const Class *t)
{
	if (!f)
		return;

	Node *index = add_node_local_var(f->__get_var("index"));
	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF));

	// delete...
	ClassFunction *f_del = t->parent->get_destructor();
	if (f_del){

		// el := self.data[index]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->get_pointer());
		Node *cmd_el = add_node_parray(self_data, cp_node(index), t->parent);

		// __delete__
		Node *cmd_delete = add_node_member_call(f_del, ref_node(cmd_el));
		f->block->params.add(cmd_delete);
	}

	// resize
	Node *c_remove = add_node_member_call(t->get_func("__mem_remove__", TypeVoid, {TypeInt}), self);
	c_remove->set_param(0, index);
	f->block->params.add(c_remove);
}

void SyntaxTree::AutoImplementArrayAdd(Function *f, const Class *t)
{
	if (!f)
		return;
	Block *b = f->block;
	Node *item = add_node_local_var(b->get_var("x"));

	Node *self = add_node_local_var(b->get_var(IDENTIFIER_SELF));

	Node *self_num = shift_node(cp_node(self), true, config.pointer_size, TypeInt);


	// resize(self.num + 1)
	Node *cmd_1 = add_node_const(add_constant(TypeInt));
	cmd_1->as_const()->as_int() = 1;
	Node *cmd_add = add_node_operator_by_inline(self_num, cmd_1, INLINE_INT_ADD);
	Node *cmd_resize = add_node_member_call(t->get_func("resize", TypeVoid, {TypeInt}), self);
	cmd_resize->set_param(0, cmd_add);
	b->add(cmd_resize);



	// el := self.data[self.num - 1]
	Node *cmd_sub = add_node_operator_by_inline(cp_node(self_num), cp_node(cmd_1), INLINE_INT_SUBTRACT);
	Node *deref_self = deref_node(cp_node(self));
	Node *self_data = shift_node(deref_self, false, 0, t->parent->get_pointer());
	Node *cmd_el = add_node_parray(self_data, cmd_sub, t->parent);

	Node *cmd_assign = link_operator(OPERATOR_ASSIGN, cmd_el, item);
	if (!cmd_assign)
		do_error(format("%s.add(): no %s.%s for elements", t->name.c_str(), t->parent->name.c_str(), IDENTIFIER_FUNC_ASSIGN.c_str()));
	b->add(cmd_assign);
}

void add_func_header(SyntaxTree *s, Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, ClassFunction *cf = nullptr)
{
	Function *f = s->add_function(name, return_type);
	f->auto_declared = true;
	foreachi (auto &p, param_types, i){
		f->literal_param_type.add(p);
		f->block->add_var(param_names[i], p);
		f->num_params ++;
	}
	f->update(t);
	bool override = cf;
	t->add_function(s, f, false, override);
}

bool needs_new(ClassFunction *f)
{
	if (!f)
		return true;
	return f->needs_overriding;
}

Array<string> class_func_param_names(ClassFunction *cf)
{
	Array<string> names;
	auto *f = cf->func;
	for (int i=0; i<f->num_params; i++)
		names.add(f->var[i]->name);
	return names;
}

void SyntaxTree::AddMissingFunctionHeadersForClass(Class *t)
{
	if (t->owner != this)
		return;
	if (t->is_pointer())
		return;

	if (t->is_super_array()){
		add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(this, t, "clear", TypeVoid, {}, {});
		add_func_header(this, t, "resize", TypeVoid, {TypeInt}, {"num"});
		add_func_header(this, t, "add", TypeVoid, {t->parent}, {"x"});
		add_func_header(this, t, "remove", TypeVoid, {TypeInt}, {"index"});
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	}else if (t->is_array()){
		if (t->needs_constructor())
			add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		if (t->needs_destructor())
			add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	}else if (t->is_dict()){
		add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(this, t, "clear", TypeVoid, {}, {});
		add_func_header(this, t, "add", TypeVoid, {TypeString, t->parent}, {"key", "x"});
		add_func_header(this, t, "__get__", t->parent, {TypeString}, {"key"});
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	}else if (!t->is_simple_class()){//needs_init){
		if (t->parent){
			// only auto-implement matching constructors
			for (auto *pcc: t->parent->get_constructors()){
				auto c = t->get_same_func(IDENTIFIER_FUNC_INIT, pcc->func);
				if (needs_new(c))
					add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, pcc->param_types, class_func_param_names(pcc), c);
			}
		}else{
			if (t->needs_constructor() and needs_new(t->get_default_constructor()))
				add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {}, t->get_default_constructor());
		}
		if (needs_new(t->get_destructor()))
			add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {}, t->get_destructor());
		if (needs_new(t->get_assign())){
			//add_func_header(this, t, NAME_FUNC_ASSIGN, TypeVoid, t, "other");
			// implement only if parent has also done so
			if (t->parent){
				if (t->parent->get_assign())
					add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			}else{
				add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			}
		}
	}
}

Function* class_get_func(const Class *t, const string &name, const Class *return_type, const Array<const Class*> &params)
{
	ClassFunction *cf = t->get_func(name, return_type, params);
	if (cf){
		Function *f = cf->func;
		if (f->auto_declared){
			cf->needs_overriding = false; // we're about to implement....
			return f;
		}
		return nullptr;
	}
	//t->owner->DoError("class_get_func... " + t->name + "." + name);
	return nullptr;
}

Function* prepare_auto_impl(const Class *t, ClassFunction *cf)
{
	if (!cf)
		return nullptr;
	Function *f = cf->func;
	if (f->auto_declared){
		cf->needs_overriding = false; // we're about to implement....
		return f;
	}
	return nullptr;
	t->owner->script->do_error_internal("prepare class func..." + cf->signature(true));
	return f;
}

// completely create and implement
void SyntaxTree::AutoImplementFunctions(const Class *t)
{
	if (t->owner != this)
		return;
	if (t->is_pointer())
		return;

	// TODO: really check here?
	// ...or just implement any function that's declared but not implemented?
	if (t->is_super_array()){
		AutoImplementConstructor(prepare_auto_impl(t, t->get_default_constructor()), t, true);
		AutoImplementDestructor(prepare_auto_impl(t, t->get_destructor()), t);
		AutoImplementArrayClear(prepare_auto_impl(t, t->get_func("clear", TypeVoid, {})), t);
		AutoImplementArrayResize(prepare_auto_impl(t, t->get_func("resize", TypeVoid, {TypeInt})), t);
		AutoImplementArrayRemove(prepare_auto_impl(t, t->get_func("remove", TypeVoid, {TypeInt})), t);
		AutoImplementArrayAdd(class_get_func(t, "add", TypeVoid, {nullptr}), t);
		AutoImplementAssign(prepare_auto_impl(t, t->get_assign()), t);
	}else if (t->is_array()){
		if (t->needs_constructor())
			AutoImplementConstructor(prepare_auto_impl(t, t->get_default_constructor()), t, true);
		if (t->needs_destructor())
			AutoImplementDestructor(prepare_auto_impl(t, t->get_destructor()), t);
		AutoImplementAssign(prepare_auto_impl(t, t->get_assign()), t);
	}else if (t->is_dict()){
		AutoImplementConstructor(prepare_auto_impl(t, t->get_default_constructor()), t, true);
	}else if (!t->is_simple_class()){
		for (auto *cf: t->get_constructors())
			AutoImplementConstructor(prepare_auto_impl(t, cf), t, true);
		if (t->needs_destructor())
			AutoImplementDestructor(prepare_auto_impl(t, t->get_destructor()), t);
		AutoImplementAssign(prepare_auto_impl(t, t->get_assign()), t);
	}
}


}
