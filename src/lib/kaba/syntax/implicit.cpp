#include "../kaba.h"
#include "../asm/asm.h"
#include <stdio.h>
#include "../../file/file.h"

namespace Kaba{

void SyntaxTree::AutoImplementAddVirtualTable(Node *self, Function *f, Class *t)
{
	if (t->vtable.num > 0){
		Node *p = shift_node(self, true, 0, TypePointer);
		int nc = AddConstant(TypePointer);
		(*(void**)constants[nc]->value.data) = t->_vtable_location_target_;
		Node *cmd_0 = add_node_const(nc);
		Node *c = add_node_operator_by_inline(p, cmd_0, INLINE_POINTER_ASSIGN);
		f->block->nodes.add(c);
	}
}

void SyntaxTree::AutoImplementAddChildConstructors(Node *self, Function *f, Class *t)
{
	int i0 = t->parent ? t->parent->elements.num : 0;
	foreachi(ClassElement &e, t->elements, i){
		if (i < i0)
			continue;
		ClassFunction *ff = e.type->GetDefaultConstructor();
		if (!ff)
			continue;
		Node *p = shift_node(self, true, e.offset, e.type);
		Node *c = add_node_classfunc(ff, ref_node(p));
		f->block->nodes.add(c);
	}
}

void SyntaxTree::AutoImplementDefaultConstructor(Function *f, Class *t, bool allow_parent_constructor)
{
	if (!f)
		return;
	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	if (t->is_super_array){
		int nc = AddConstant(TypeInt);
		constants[nc]->as_int() = t->parent->size;
		Node *c = add_node_classfunc(t->GetFunc("__mem_init__", TypeVoid, 1, TypeInt), self);
		c->set_param(0, add_node_const(nc));
		f->block->nodes.add(c);
	}else{

		// parent constructor
		if (t->parent and allow_parent_constructor){
			ClassFunction *ff = t->parent->GetDefaultConstructor();
			if (ff){
				Node *c = add_node_classfunc(ff, cp_node(self));
				f->block->nodes.add(c);
			}
		}

		// add vtable reference
		if (t->vtable.num > 0)
			AutoImplementAddVirtualTable(self, f, t);

		// call child constructors
		AutoImplementAddChildConstructors(self, f, t);
	}
}

void SyntaxTree::AutoImplementComplexConstructor(Function *f, Class *t)
{
	if (!f)
		return;
	if (!f->auto_implement)
		return;
	ClassFunction *pcc = t->parent->GetComplexConstructor();

	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	// parent constructor
	Node *c = add_node_classfunc(pcc, cp_node(self));
	for (int i=0;i<pcc->param_types.num;i++)
		c->set_param(i, add_node_local_var(i, pcc->param_types[i]));
	f->block->nodes.add(c);

	// add vtable reference
	if (t->vtable.num > 0)
		AutoImplementAddVirtualTable(self, f, t);

	// call child constructors
	AutoImplementAddChildConstructors(self, f, t);
}


void SyntaxTree::AutoImplementDestructor(Function *f, Class *t)
{
	if (!f)
		return;
	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	if (t->is_super_array){
		ClassFunction *f_clear = t->GetFunc("clear", TypeVoid, 0);
		if (f_clear){
			Node *c = add_node_classfunc(f_clear, self);
			f->block->nodes.add(c);
		}
	}else{

		// call child destructors
		int i0 = t->parent ? t->parent->elements.num : 0;
		foreachi(ClassElement &e, t->elements, i){
			if (i < i0)
				continue;
			ClassFunction *ff = e.type->GetDestructor();
			if (!ff)
				continue;
			Node *p = shift_node(self, true, e.offset, e.type);
			Node *c = add_node_classfunc(ff, ref_node(p));
			f->block->nodes.add(c);
		}

		// parent destructor
		if (t->parent){
			ClassFunction *ff = t->parent->GetDestructor();
			if (ff){
				Node *c = add_node_classfunc(ff, cp_node(self), true);
				f->block->nodes.add(c);
			}
		}
	}
}

void SyntaxTree::AutoImplementAssign(Function *f, Class *t)
{
	if (!f)
		return;
	Node *other = add_node_local_var(f->__get_var("other"), t);
	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	if (t->is_super_array){

		ClassFunction *f_resize = t->GetFunc("resize", TypeVoid, 1, TypeInt);
		if (!f_resize)
			DoError(format("%s.__assign__(): no %s.resize(int) found", t->name.c_str(), t->name.c_str()));

		// self.resize(other.num)
		Node *other_num = shift_node(other, false, config.pointer_size, TypeInt);

		Node *cmd_resize = add_node_classfunc(f_resize, cp_node(self));
		cmd_resize->set_num_params(1);
		cmd_resize->set_param(0, other_num);
		f->block->nodes.add(cmd_resize);

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		f->block->add_var("i", TypeInt);

		Node *for_var = add_node_local_var(f->__get_var("i"), TypeInt);


		// for_var = 0
		int nc = AddConstant(TypeInt);
		constants[nc]->as_int() = 0;
		Node *cmd_0 = add_node_const(nc);
		Node *cmd_assign0 = add_node_operator_by_inline(for_var, cmd_0, INLINE_INT_ASSIGN);
		f->block->nodes.add(cmd_assign0);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(for_var, cp_node(other_num), INLINE_INT_SMALLER);

		Node *cmd_while = add_node_statement(STATEMENT_FOR);
		cmd_while->set_param(0, cmd_cmp);
		f->block->nodes.add(cmd_while);

		Block *b = AddBlock(f, f->block);
		Node *cb = add_node_block(b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->GetPointer());
		Node *cmd_el = add_node_parray(self_data, for_var, t->parent);

		// el2 := other.data[for_var]
		Node *other_data = shift_node(other, false, 0, t->parent->GetPointer());
		Node *cmd_el2 = add_node_parray(other_data, for_var, t->parent);


		Node *cmd_assign = LinkOperator(OPERATOR_ASSIGN, cmd_el, cmd_el2);
		if (!cmd_assign)
			DoError(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
		b->nodes.add(cmd_assign);

		// ...for_var += 1
		Node *cmd_inc = add_node_operator_by_inline(for_var, cmd_0 /*dummy*/, INLINE_INT_INCREASE);
		b->nodes.add(cmd_inc);
		f->block->nodes.add(cb);
	}else if (t->is_array){

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		f->block->add_var("i", TypeInt);
		int nc_num = AddConstant(TypeInt);
		constants[nc_num]->as_int() = t->array_length;

		Node *for_var = add_node_local_var(f->__get_var("i"), TypeInt);
		Node *c_num = add_node_const(nc_num);


		// for_var = 0
		int nc_0 = AddConstant(TypeInt);
		constants[nc_0]->as_int() = 0;
		Node *cmd_0 = add_node_const(nc_0);
		Node *cmd_assign0 = add_node_operator_by_inline(for_var, cmd_0, INLINE_INT_ASSIGN);
		f->block->nodes.add(cmd_assign0);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(for_var, c_num, INLINE_INT_SMALLER);

		Node *cmd_while = add_node_statement(STATEMENT_FOR);
		cmd_while->set_param(0, cmd_cmp);
		f->block->nodes.add(cmd_while);

		Block *b = AddBlock(f, f->block);
		Node *cb = add_node_block(b);

		// el := self.data[for_var]
		Node *cmd_el = add_node_parray(self, for_var, t->parent);

		// el2 := other.data[for_var]
		Node *cmd_el2 = add_node_parray(ref_node(other), for_var, t->parent);


		Node *cmd_assign = LinkOperator(OPERATOR_ASSIGN, cmd_el, cmd_el2);
		if (!cmd_assign)
			DoError(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
		b->nodes.add(cmd_assign);

		// ...for_var += 1
		Node *cmd_inc = add_node_operator_by_inline(for_var, cmd_0 /*dummy*/, INLINE_INT_INCREASE);
		b->nodes.add(cmd_inc);
		f->block->nodes.add(cb);
	}else{

		// parent assignment
		if (t->parent){
			Node *p = deref_node(cp_node(self));
			Node *o = cp_node(other);
			p->type = o->type = t->parent;

			Node *cmd_assign = LinkOperator(OPERATOR_ASSIGN, p, o);
			if (!cmd_assign)
				DoError(format("%s.__assign__(): no parent %s.__assign__", t->name.c_str(), t->parent->name.c_str()));
			f->block->nodes.add(cmd_assign);
		}

		// call child assignment
		int i0 = t->parent ? t->parent->elements.num : 0;
		foreachi(ClassElement &e, t->elements, i){
			if (i < i0)
				continue;
			Node *p = shift_node(self, true, e.offset, e.type);
			Node *o = shift_node(cp_node(other), false, e.offset, e.type); // needed for call-by-ref conversion!

			Node *cmd_assign = LinkOperator(OPERATOR_ASSIGN, p, o);
			if (!cmd_assign)
				DoError(format("%s.__assign__(): no %s.__assign__ for element \"%s\"", t->name.c_str(), e.type->name.c_str(), e.name.c_str()));
			f->block->nodes.add(cmd_assign);
		}
	}
}


void SyntaxTree::AutoImplementArrayClear(Function *f, Class *t)
{
	if (!f)
		return;
	f->block->add_var("for_var", TypeInt);

	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	Node *self_num = shift_node(cp_node(self), true, config.pointer_size, TypeInt);

	Node *for_var = add_node_local_var(f->__get_var("for_var"), TypeInt);

// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){
		// for_var = 0
		int nc = AddConstant(TypeInt);
		constants[nc]->as_int() = 0;
		Node *cmd_0 = add_node_const(nc);
		Node *cmd_assign = add_node_operator_by_inline(for_var, cmd_0, INLINE_INT_ASSIGN);
		f->block->nodes.add(cmd_assign);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(for_var, self_num, INLINE_INT_SMALLER);

		Node *cmd_while = add_node_statement(STATEMENT_FOR);
		cmd_while->set_param(0, cmd_cmp);
		f->block->nodes.add(cmd_while);

		Block *b = AddBlock(f, f->block);
		Node *cb = add_node_block(b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->GetPointer());
		Node *cmd_el = add_node_parray(self_data, for_var, t->parent);

		// __delete__
		Node *cmd_delete = add_node_classfunc(f_del, ref_node(cmd_el));
		b->nodes.add(cmd_delete);

		// for_var ++
		Node *cmd_inc = add_node_operator_by_inline(for_var, cmd_0 /*dummy*/, INLINE_INT_INCREASE);
		b->nodes.add(cmd_inc);
		f->block->nodes.add(cb);
	}

	// clear
	Node *cmd_clear = add_node_classfunc(t->GetFunc("__mem_clear__", TypeVoid, 0), self);
	f->block->nodes.add(cmd_clear);
}


void SyntaxTree::AutoImplementArrayResize(Function *f, Class *t)
{
	if (!f)
		return;
	f->block->add_var("for_var", TypeInt);
	f->block->add_var("num_old", TypeInt);

	Node *num = add_node_local_var(f->__get_var("num"), TypeInt);

	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	Node *self_num = shift_node(cp_node(self), true, config.pointer_size, TypeInt);

	Node *for_var = add_node_local_var(f->__get_var("for_var"), TypeInt);

	Node *num_old = add_node_local_var(f->__get_var("num_old"), TypeInt);

	// num_old = self.num
	Node *cmd_copy_num = add_node_operator_by_inline(num_old, self_num, INLINE_INT_ASSIGN);
	f->block->nodes.add(cmd_copy_num);

// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){
		// for_var = num
		Node *cmd_assign = add_node_operator_by_inline(for_var, num, INLINE_INT_ASSIGN);
		f->block->nodes.add(cmd_assign);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(for_var, self_num, INLINE_INT_SMALLER);

		Node *cmd_while = add_node_statement(STATEMENT_FOR);
		cmd_while->set_param(0, cmd_cmp);
		f->block->nodes.add(cmd_while);

		Block *b = AddBlock(f, f->block);
		Node *cb = add_node_block(b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->GetPointer());
		Node *cmd_el = add_node_parray(self_data, for_var, t->parent);

		// __delete__
		Node *cmd_delete = add_node_classfunc(f_del, ref_node(cmd_el));
		b->nodes.add(cmd_delete);

		// ...for_var += 1
		Node *cmd_inc = add_node_operator_by_inline(for_var, num /*dummy*/, INLINE_INT_INCREASE);
		b->nodes.add(cmd_inc);
		f->block->nodes.add(cb);
	}

	// resize
	Node *c_resize = add_node_classfunc(t->GetFunc("__mem_resize__", TypeVoid, 1, TypeInt), self);
	c_resize->set_param(0, num);
	f->block->nodes.add(c_resize);

	// new...
	ClassFunction *f_init = t->parent->GetDefaultConstructor();
	if (f_init){
		// for_var = num_old
		Node *cmd_assign = add_node_operator_by_inline(for_var, num_old, INLINE_INT_ASSIGN);
		f->block->nodes.add(cmd_assign);

		// while(for_var < self.num)
		Node *cmd_cmp = add_node_operator_by_inline(for_var, self_num, INLINE_INT_SMALLER);

		Node *cmd_while = add_node_statement(STATEMENT_FOR);
		cmd_while->set_param(0, cmd_cmp);
		f->block->nodes.add(cmd_while);

		Block *b = AddBlock(f, f->block);
		Node *cb = add_node_block(b);

		// el := self.data[for_var]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->GetPointer());
		Node *cmd_el = add_node_parray(self_data, for_var, t->parent);

		// __init__
		Node *cmd_init = add_node_classfunc(f_init, ref_node(cmd_el));
		b->nodes.add(cmd_init);

		// ...for_var += 1
		Node *cmd_inc = add_node_operator_by_inline(for_var, num /*dummy*/, INLINE_INT_INCREASE);
		b->nodes.add(cmd_inc);
		f->block->nodes.add(cb);
	}
}


void SyntaxTree::AutoImplementArrayRemove(Function *f, Class *t)
{
	if (!f)
		return;

	Node *index = add_node_local_var(f->__get_var("index"), TypeInt);
	Node *self = add_node_local_var(f->__get_var(IDENTIFIER_SELF), t->GetPointer());

	// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){

		// el := self.data[index]
		Node *deref_self = deref_node(cp_node(self));
		Node *self_data = shift_node(deref_self, false, 0, t->parent->GetPointer());
		Node *cmd_el = add_node_parray(self_data, index, t->parent);

		// __delete__
		Node *cmd_delete = add_node_classfunc(f_del, ref_node(cmd_el));
		f->block->nodes.add(cmd_delete);
	}

	// resize
	Node *c_remove = add_node_classfunc(t->GetFunc("__mem_remove__", TypeVoid, 1, TypeInt), self);
	c_remove->set_param(0, index);
	f->block->nodes.add(c_remove);
}

void SyntaxTree::AutoImplementArrayAdd(Function *f, Class *t)
{
	if (!f)
		return;
	Block *b = f->block;
	Node *item = add_node_local_var(b->get_var("x"), t->parent);

	Node *self = add_node_local_var(b->get_var(IDENTIFIER_SELF), t->GetPointer());

	Node *self_num = shift_node(cp_node(self), true, config.pointer_size, TypeInt);


	// resize(self.num + 1)
	int nc = AddConstant(TypeInt);
	constants[nc]->as_int() = 1;
	Node *cmd_1 = add_node_const(nc);
	Node *cmd_add = add_node_operator_by_inline(self_num, cmd_1, INLINE_INT_ADD);
	Node *cmd_resize = add_node_classfunc(t->GetFunc("resize", TypeVoid, 1, TypeInt), self);
	cmd_resize->set_param(0, cmd_add);
	b->nodes.add(cmd_resize);



	// el := self.data[self.num - 1]
	Node *cmd_sub = add_node_operator_by_inline(cp_node(self_num), cmd_1, INLINE_INT_SUBTRACT);
	Node *deref_self = deref_node(cp_node(self));
	Node *self_data = shift_node(deref_self, false, 0, t->parent->GetPointer());
	Node *cmd_el = add_node_parray(self_data, cmd_sub, t->parent);

	Node *cmd_assign = LinkOperator(OPERATOR_ASSIGN, cmd_el, item);
	if (!cmd_assign)
		DoError(format("%s.add(): no %s.%s for elements", t->name.c_str(), t->parent->name.c_str(), IDENTIFIER_FUNC_ASSIGN.c_str()));
	b->nodes.add(cmd_assign);
}

void add_func_header(SyntaxTree *s, Class *t, const string &name, Class *return_type, Class *param_type, const string &param_name, ClassFunction *cf = NULL)
{
	Function *f = s->AddFunction(name, return_type);
	f->auto_implement = true;
	if (param_type != TypeVoid){
		f->block->add_var(param_name, param_type);
		f->num_params ++;
	}
	f->Update(t);
	t->AddFunction(s, s->functions.num - 1, false, cf);
}

bool needs_new(ClassFunction *f)
{
	if (!f)
		return true;
	return f->needs_overriding;
}

void SyntaxTree::AddFunctionHeadersForClass(Class *t)
{
	if (t->owner != this)
		return;
	if (t->is_pointer)
		return;

	if (t->is_super_array){
		add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, TypeVoid, "");
		add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, TypeVoid, "");
		add_func_header(this, t, "clear", TypeVoid, TypeVoid, "");
		add_func_header(this, t, "resize", TypeVoid, TypeInt, "num");
		add_func_header(this, t, "add", TypeVoid, t->parent, "x");
		add_func_header(this, t, "remove", TypeVoid, TypeInt, "index");
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, t, "other");
	}else if (t->is_array){
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, t, "other");
	}else if (!t->is_simple_class()){//needs_init){
		if (needs_new(t->GetDefaultConstructor()))
			add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, TypeVoid, "", t->GetDefaultConstructor());
		if (needs_new(t->GetDestructor()))
			add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, TypeVoid, "", t->GetDestructor());
		if (needs_new(t->GetAssign())){
			//add_func_header(this, t, NAME_FUNC_ASSIGN, TypeVoid, t, "other");
			// implement only if parent has also done so
			if (t->parent){
				if (t->parent->GetAssign())
					add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, t, "other", t->GetAssign());
			}else{
				add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, t, "other", t->GetAssign());
			}
		}
		if (needs_new(t->GetComplexConstructor()))
			if (t->parent)
				if (t->parent->GetComplexConstructor()){
					Function *f = AddFunction(IDENTIFIER_FUNC_INIT, TypeVoid);
					f->auto_implement = true;

					Function *b = t->parent->GetComplexConstructor()->GetFunc();
					f->num_params = b->num_params;
					f->var = b->var;
					f->Update(t);
					t->AddFunction(this, functions.num - 1, false, t->GetComplexConstructor());

				}
	}
}

Function* class_get_func(Class *t, const string &name, Class *return_type, int num_params)
{
	ClassFunction *cf = t->GetFunc(name, return_type, num_params);
	if (cf){
		Function *f = cf->GetFunc();
		if (f->auto_implement){
			cf->needs_overriding = false; // we're about to implement....
			return f;
		}
		return NULL;
	}
	//t->owner->DoError("class_get_func... " + t->name + "." + name);
	return NULL;
}

void SyntaxTree::AutoImplementFunctions(Class *t)
{
	if (t->owner != this)
		return;
	if (t->is_pointer)
		return;

	if (t->is_super_array){
		AutoImplementDefaultConstructor(class_get_func(t, IDENTIFIER_FUNC_INIT, TypeVoid, 0), t, true);
		AutoImplementDestructor(class_get_func(t, IDENTIFIER_FUNC_DELETE, TypeVoid, 0), t);
		AutoImplementArrayClear(class_get_func(t, "clear", TypeVoid, 0), t);
		AutoImplementArrayResize(class_get_func(t, "resize", TypeVoid, 1), t);
		AutoImplementArrayRemove(class_get_func(t, "remove", TypeVoid, 1), t);
		AutoImplementArrayAdd(class_get_func(t, "add", TypeVoid, 1), t);
		AutoImplementAssign(class_get_func(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, 1), t);
	}else if (t->is_array){
		AutoImplementAssign(class_get_func(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, 1), t);
	}else if (!t->is_simple_class()){//needs_init){
		if (t->needs_constructor())
			AutoImplementDefaultConstructor(class_get_func(t, IDENTIFIER_FUNC_INIT, TypeVoid, 0), t, true);
		if (t->needs_destructor())
			AutoImplementDestructor(class_get_func(t, IDENTIFIER_FUNC_DELETE, TypeVoid, 0), t);
		AutoImplementAssign(class_get_func(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, 1), t);
		ClassFunction *cf = t->GetComplexConstructor();
		if (cf)
			AutoImplementComplexConstructor(cf->GetFunc(), t);
	}
}


}
