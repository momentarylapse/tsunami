#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{

void CreateImplicitConstructor(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__init__", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 0;
	self->type = ps->GetPointerType(t);

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "__mem_init__"){
				int nc = ps->AddConstant(TypeInt);
				*(int*)ps->Constants[nc].data = t->parent->size;
				Command *c = ps->add_command_classfunc(t, ff, self);
				Command *p = ps->add_command_const(nc);
				c->param[0] = p;
				c->num_params = 1;
				f->block->command.add(c);
			}
	}else{

		// call child constructors
		foreach(ClassElement &e, t->element){
			ClassFunction *ff = e.type->GetConstructor();
			if (!ff)
				continue;
			Command *p = ps->AddCommand();
			p->kind = KindDerefAddressShift;
			p->link_nr = e.offset;
			p->type = e.type;
			p->num_params = 1;
			p->param[0] = self;
			Command *c = ps->add_command_classfunc(t, *ff, ps->ref_command(p));
			f->block->command.add(c);
		}
	}

	ClassFunction cf;
	cf.nr = fn;
	cf.name = "__init__";
	cf.return_type = TypeVoid;
	cf.script = ps->script;
	t->function.add(cf);
}

void CreateImplicitDestructor(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__delete__", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 0;
	self->type = ps->GetPointerType(t);

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "clear"){
				Command *c = ps->add_command_classfunc(t, ff, self);
				f->block->command.add(c);
			}
	}else{

		// call child destructors
		foreach(ClassElement &e, t->element){
			ClassFunction *ff = e.type->GetDestructor();
			if (!ff)
				continue;
			Command *p = ps->AddCommand();
			p->kind = KindDerefAddressShift;
			p->link_nr = e.offset;
			p->type = e.type;
			p->num_params = 1;
			p->param[0] = self;
			Command *c = ps->add_command_classfunc(t, *ff, ps->ref_command(p));
			f->block->command.add(c);
		}
	}


	ClassFunction cf;
	cf.nr = fn;
	cf.name = "__delete__";
	cf.return_type = TypeVoid;
	cf.script = ps->script;
	t->function.add(cf);
}

void CreateImplicitAssign(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__assign__", TypeVoid);
	int fn = ps->Functions.num - 1;
	ps->AddVar("other", t, f);
	f->num_params = 1;
	f->literal_param_type[0] = t;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *other = ps->AddCommand();
	other->kind = KindVarLocal;
	other->link_nr = 0;
	other->type = t;

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 1;
	self->type = ps->GetPointerType(t);

	if (t->is_super_array){

		int nf = t->GetFunc("resize");
		if (nf < 0){
			ps->DoError(format("%s.__assign__(): no %s.resize() found", t->name.c_str(), t->name.c_str()));
			return;
		}

		// self.resize(other.num)
		Command *other_num = ps->shift_command(other, false, config.PointerSize, TypeInt);

		Command *cmd_resize = ps->add_command_classfunc(t, t->function[nf], ps->cp_command(self));
		cmd_resize->num_params = 1;
		cmd_resize->param[0] = other_num;
		f->block->command.add(cmd_resize);

		// for int i, 0, other.num
		//    self[i].__assign__(other[i])

		ps->AddVar("i", TypeInt, f);

		Command *for_var = ps->AddCommand();
		for_var->kind = KindVarLocal;
		for_var->link_nr = 2;
		for_var->type = TypeInt;


		// for_var = 0
		int nc = ps->AddConstant(TypeInt);
		(*(int*)ps->Constants[nc].data) = 0;
		Command *cmd_0 = ps->add_command_const(nc);
		Command *cmd_assign0 = ps->add_command_operator(for_var, cmd_0, OperatorIntAssign);
		f->block->command.add(cmd_assign0);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->add_command_operator(for_var, ps->cp_command_deep(other_num), OperatorIntSmaller);

		Command *cmd_while = ps->add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self.data[for_var]
		Command *deref_self = ps->deref_command(ps->cp_command(self));
		Command *self_data = ps->shift_command(deref_self, false, 0, ps->GetPointerType(t->parent));
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = self_data;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;

		// el2 := other.data[for_var]
		Command *other_data = ps->shift_command(other, false, 0, ps->GetPointerType(t->parent));
		Command *cmd_el2 = ps->AddCommand();
		cmd_el2->kind = KindPointerAsArray;
		cmd_el2->type = t->parent;
		cmd_el2->param[0] = other_data;
		cmd_el2->param[1] = for_var;
		cmd_el2->num_params = 2;


		Command *cmd_assign = ps->LinkOperator(OperatorAssign, cmd_el, cmd_el2);
		if (!cmd_assign)
			ps->DoError(format("%s.__assign__(): no %s.__assign__() found", t->name.c_str(), t->parent->name.c_str()));
		b->command.add(cmd_assign);

		// ...for_var += 1
		Command *cmd_inc = ps->add_command_operator(for_var, cmd_0 /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}else{

		// call child assignment
		foreach(ClassElement &e, t->element){
			Command *p = ps->shift_command(self, true, e.offset, e.type);
			Command *o = ps->shift_command(ps->cp_command(other), false, e.offset, e.type); // needed for call-by-ref conversion!

			Command *cmd_assign = ps->LinkOperator(OperatorAssign, p, o);
			if (!cmd_assign)
				ps->DoError(format("%s.__assign__(): no %s.__assign__ for element \"%s\"", t->name.c_str(), e.type->name.c_str(), e.name.c_str()));
			f->block->command.add(cmd_assign);
		}
	}

	ClassFunction cf;
	cf.nr = fn;
	cf.name = "__assign__";
	cf.return_type = TypeVoid;
	cf.param_type.add(t);
	cf.script = ps->script;
	t->function.add(cf);
}


void CreateImplicitArrayClear(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".clear", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);
	ps->AddVar("for_var", TypeInt, f);

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 0;
	self->type = ps->GetPointerType(t);

	Command *self_num = ps->AddCommand();
	self_num->kind = KindDerefAddressShift;
	self_num->link_nr = config.PointerSize;
	self_num->type = TypeInt;
	self_num->num_params = 1;
	self_num->param[0] = ps->cp_command(self);

	Command *for_var = ps->AddCommand();
	for_var->kind = KindVarLocal;
	for_var->link_nr = 1;
	for_var->type = TypeInt;

// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){
		// for_var = 0
		int nc = ps->AddConstant(TypeInt);
		(*(int*)ps->Constants[nc].data) = 0;
		Command *cmd_0 = ps->add_command_const(nc);
		Command *cmd_assign = ps->add_command_operator(for_var, cmd_0, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->add_command_operator(for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = ps->add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self.data[for_var]
		Command *deref_self = ps->deref_command(ps->cp_command(self));
		Command *self_data = ps->shift_command(deref_self, false, 0, ps->GetPointerType(t->parent));
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = self_data;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;

		// __delete__
		Command *cmd_delete = ps->add_command_classfunc(t, *f_del, ps->ref_command(cmd_el));
		b->command.add(cmd_delete);

		// ...for_var += 1
		Command *cmd_inc = ps->add_command_operator(for_var, cmd_0 /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}

	// clear
	Command *cmd_clear = ps->add_command_classfunc(t, t->function[t->GetFunc("__mem_clear__")], self);
	f->block->command.add(cmd_clear);


	ClassFunction cf;
	cf.nr = fn;
	cf.name = "clear";
	cf.return_type = TypeVoid;
	cf.script = ps->script;
	t->function.add(cf);
}


void CreateImplicitArrayResize(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".resize", TypeVoid);
	int fn = ps->Functions.num - 1;
	ps->AddVar("num", TypeInt, f);
	f->num_params = 1;
	f->literal_param_type[0] = TypeInt;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);
	ps->AddVar("for_var", TypeInt, f);
	ps->AddVar("num_old", TypeInt, f);

	Command *num = ps->AddCommand();
	num->kind = KindVarLocal;
	num->link_nr = 0;
	num->type = TypeInt;

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 1;
	self->type = ps->GetPointerType(t);

	Command *self_num = ps->AddCommand();
	self_num->kind = KindDerefAddressShift;
	self_num->link_nr = config.PointerSize;
	self_num->type = TypeInt;
	self_num->num_params = 1;
	self_num->param[0] = ps->cp_command(self);

	Command *for_var = ps->AddCommand();
	for_var->kind = KindVarLocal;
	for_var->link_nr = 2;
	for_var->type = TypeInt;

	Command *num_old = ps->AddCommand();
	num_old->kind = KindVarLocal;
	num_old->link_nr = 3;
	num_old->type = TypeInt;

	// num_old = self.num
	Command *cmd_copy_num = ps->add_command_operator(num_old, self_num, OperatorIntAssign);
	f->block->command.add(cmd_copy_num);

// delete...
	ClassFunction *f_del = t->parent->GetDestructor();
	if (f_del){
		// for_var = num
		Command *cmd_assign = ps->add_command_operator(for_var, num, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->add_command_operator(for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = ps->add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self.data[for_var]
		Command *deref_self = ps->deref_command(ps->cp_command(self));
		Command *self_data = ps->shift_command(deref_self, false, 0, ps->GetPointerType(t->parent));
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = self_data;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;

		// __delete__
		Command *cmd_delete = ps->add_command_classfunc(t, *f_del, ps->ref_command(cmd_el));
		b->command.add(cmd_delete);

		// ...for_var += 1
		Command *cmd_inc = ps->add_command_operator(for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}

	// resize
	Command *c_resize = ps->add_command_classfunc(t, t->function[t->GetFunc("__mem_resize__")], self);
	c_resize->num_params = 1;
	c_resize->param[0] = num;
	f->block->command.add(c_resize);

	// new...
	ClassFunction *f_init = t->parent->GetConstructor();
	if (f_init){
		// for_var = num_old
		Command *cmd_assign = ps->add_command_operator(for_var, num_old, OperatorIntAssign);
		f->block->command.add(cmd_assign);

		// while(for_var < self.num)
		Command *cmd_cmp = ps->add_command_operator(for_var, self_num, OperatorIntSmaller);

		Command *cmd_while = ps->add_command_compilerfunc(CommandFor);
		cmd_while->param[0] = cmd_cmp;
		f->block->command.add(cmd_while);

		Command *cb = ps->AddCommand();
		Block *b = ps->AddBlock();
		cb->kind = KindBlock;
		cb->link_nr = b->index;

		// el := self.data[for_var]
		Command *deref_self = ps->deref_command(ps->cp_command(self));
		Command *self_data = ps->shift_command(deref_self, false, 0, ps->GetPointerType(t->parent));
		Command *cmd_el = ps->AddCommand();
		cmd_el->kind = KindPointerAsArray;
		cmd_el->type = t->parent;
		cmd_el->param[0] = self_data;
		cmd_el->param[1] = for_var;
		cmd_el->num_params = 2;

		// __init__
		Command *cmd_init = ps->add_command_classfunc(t, *f_init, ps->ref_command(cmd_el));
		b->command.add(cmd_init);

		// ...for_var += 1
		Command *cmd_inc = ps->add_command_operator(for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}


	ClassFunction cf;
	cf.nr = fn;
	cf.name = "resize";
	cf.return_type = TypeVoid;
	cf.param_type.add(TypeInt);
	cf.script = ps->script;
	t->function.add(cf);
}

void CreateImplicitArrayAdd(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".add", TypeVoid);
	int fn = ps->Functions.num - 1;
	ps->AddVar("x", t->parent, f);
	f->num_params = 1;
	f->literal_param_type[0] = t->parent;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	Command *item = ps->AddCommand();
	item->kind = KindVarLocal;
	item->link_nr = 0;
	item->type = t->parent;

	Command *self = ps->AddCommand();
	self->kind = KindVarLocal;
	self->link_nr = 1;
	self->type = ps->GetPointerType(t);

	Command *self_num = ps->AddCommand();
	self_num->kind = KindDerefAddressShift;
	self_num->link_nr = config.PointerSize;
	self_num->type = TypeInt;
	self_num->num_params = 1;
	self_num->param[0] = ps->cp_command(self);


	// resize(self.num + 1)
	int nc = ps->AddConstant(TypeInt);
	(*(int*)ps->Constants[nc].data) = 1;
	Command *cmd_1 = ps->add_command_const(nc);
	Command *cmd_add = ps->add_command_operator(self_num, cmd_1, OperatorIntAdd);
	Command *cmd_resize = ps->add_command_classfunc(t, t->function[t->GetFunc("resize")], self);
	cmd_resize->num_params = 1;
	cmd_resize->param[0] = cmd_add;
	f->block->command.add(cmd_resize);



	// el := self.data[self.num - 1]
	Command *cmd_sub = ps->add_command_operator(ps->cp_command(self_num), cmd_1, OperatorIntSubtract);
	Command *deref_self = ps->deref_command(ps->cp_command(self));
	Command *self_data = ps->shift_command(deref_self, false, 0, ps->GetPointerType(t->parent));
	Command *cmd_el = ps->AddCommand();
	cmd_el->kind = KindPointerAsArray;
	cmd_el->type = t->parent;
	cmd_el->param[0] = self_data;
	cmd_el->param[1] = cmd_sub;
	cmd_el->num_params = 2;

	Command *cmd_assign = ps->LinkOperator(OperatorAssign, cmd_el, item);
	if (!cmd_assign)
		ps->DoError(format("%s.add(): no %s.__assign__ for elements", t->name.c_str(), t->parent->name.c_str()));
	f->block->command.add(cmd_assign);

	ClassFunction cf;
	cf.nr = fn;
	cf.name = "add";
	cf.return_type = TypeVoid;
	cf.param_type.add(t->parent);
	cf.script = ps->script;
	t->function.add(cf);
}



void SyntaxTree::CreateImplicitFunctions(Type *t, bool relocate_last_function)
{
	int num_funcs = Functions.num;

	if (t->owner != this)
		return;
	if (t->is_pointer)
		return;

	// needs complex functions?
	/*bool needs_init = false;
	foreach(t->Element, e)
		foreach(e->Type->Function, f)
			if (strcmp(f->Name, "__init__") == 0)
				needs_init = true;
	if (t->IsSuperArray)
		needs_init = true;*/

	if (t->is_super_array){
		if (t->GetFunc("clear") < 0)
			CreateImplicitArrayClear(this, t);
		if (t->GetFunc("resize") < 0)
			CreateImplicitArrayResize(this, t);
		if (t->GetFunc("add") < 0)
			CreateImplicitArrayAdd(this, t);
	}
	if (!t->is_simple_class()){//needs_init){
		if (!t->GetConstructor())
			CreateImplicitConstructor(this, t);
		if (!t->GetDestructor())
			CreateImplicitDestructor(this, t);
		if (t->GetFunc("__assign__") < 0)
			CreateImplicitAssign(this, t);
	}

	if (relocate_last_function && (num_funcs != Functions.num)){
		//msg_error("relocate implicit function");
		// resort Function[]
		Function *f = Functions[num_funcs - 1];
		Functions.erase(num_funcs - 1);
		Functions.add(f);

		// relink commands
		foreach(Command *c, Commands)
			if (c->kind == KindFunction){
				if (c->script != script)
					continue;
				if (c->link_nr == num_funcs - 1)
					c->link_nr = Functions.num - 1;
				else if (c->link_nr > num_funcs - 1)
					c->link_nr --;
			}

		// relink class functions
		foreach(Type *t, Types)
			foreach(ClassFunction &f, t->function){
				if (f.script != script)
					continue;
				if (f.nr == num_funcs - 1)
					f.nr = Functions.num - 1;
				else if (f.nr > num_funcs - 1)
					f.nr --;
			}
	}
}

void SyntaxTree::CreateAllImplicitFunctions(bool relocate_last_function)
{
	foreach(Type *t, Types)
		CreateImplicitFunctions(t, relocate_last_function);
}


}
