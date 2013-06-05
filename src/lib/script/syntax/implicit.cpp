#include "../script.h"
#include "../asm/asm.h"
#include "../../file/file.h"
#include <stdio.h>

namespace Script{

void SyntaxTree::ImplementImplicitConstructor(Function *f, Type *t)
{
	Command *self = add_command_local_var(0, GetPointerType(t));

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "__mem_init__"){
				int nc = AddConstant(TypeInt);
				*(int*)Constants[nc].data = t->parent->size;
				Command *c = add_command_classfunc(t, ff, self);
				c->param[0] = add_command_const(nc);
				c->num_params = 1;
				f->block->command.add(c);
			}
	}else{

		if (t->vtable){
			Command *p = shift_command(self, true, 0, TypePointer);
			int nc = AddConstant(TypePointer);
			(*(void**)Constants[nc].data) = t->vtable;
			Command *cmd_0 = add_command_const(nc);
			Command *c = add_command_operator(p, cmd_0, OperatorAssign);
			f->block->command.add(c);
		}

		// call child constructors
		foreach(ClassElement &e, t->element){
			ClassFunction *ff = e.type->GetConstructor();
			if (!ff)
				continue;
			Command *p = shift_command(self, true, e.offset, e.type);
			Command *c = add_command_classfunc(t, *ff, ref_command(p));
			f->block->command.add(c);
		}
	}
}

void CreateImplicitConstructor(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__init__", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	ps->ImplementImplicitConstructor(f, t);

	t->function.add(ClassFunction("__init__", TypeVoid, ps->script, fn));
}


void SyntaxTree::ImplementImplicitDestructor(Function *f, Type *t)
{
	Command *self = add_command_local_var(0, GetPointerType(t));

	if (t->is_super_array){
		foreach(ClassFunction &ff, t->function)
			if (ff.name == "clear"){
				Command *c = add_command_classfunc(t, ff, self);
				f->block->command.add(c);
			}
	}else{

		// call child destructors
		foreach(ClassElement &e, t->element){
			ClassFunction *ff = e.type->GetDestructor();
			if (!ff)
				continue;
			Command *p = shift_command(self, true, e.offset, e.type);
			Command *c = add_command_classfunc(t, *ff, ref_command(p));
			f->block->command.add(c);
		}
	}
}

void CreateImplicitDestructor(SyntaxTree *ps, Type *t)
{
	// create function
	Function *f = ps->AddFunction(t->name + ".__delete__", TypeVoid);
	int fn = ps->Functions.num - 1;
	f->_class = t;
	ps->AddVar("self", ps->GetPointerType(t), f);

	ps->ImplementImplicitDestructor(f, t);

	t->function.add(ClassFunction("__delete__", TypeVoid, ps->script, fn));
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

	Command *other = ps->add_command_local_var(0, t);

	Command *self = ps->add_command_local_var(1, ps->GetPointerType(t));

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

		Command *for_var = ps->add_command_local_var(2, TypeInt);


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
		Command *cmd_el = ps->add_command_parray(self_data, for_var, t->parent);

		// el2 := other.data[for_var]
		Command *other_data = ps->shift_command(other, false, 0, ps->GetPointerType(t->parent));
		Command *cmd_el2 = ps->add_command_parray(other_data, for_var, t->parent);


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

	ClassFunction cf = ClassFunction("__assign__", TypeVoid, ps->script, fn);
	cf.param_type.add(t);
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

	Command *self = ps->add_command_local_var(0, ps->GetPointerType(t));

	Command *self_num = ps->shift_command(ps->cp_command(self), true, config.PointerSize, TypeInt);

	Command *for_var = ps->add_command_local_var(1, TypeInt);

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
		Command *cmd_el = ps->add_command_parray(self_data, for_var, t->parent);

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


	t->function.add(ClassFunction("clear", TypeVoid, ps->script, fn));
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

	Command *num = ps->add_command_local_var(0, TypeInt);

	Command *self = ps->add_command_local_var(1, ps->GetPointerType(t));

	Command *self_num = ps->shift_command(ps->cp_command(self), true, config.PointerSize, TypeInt);

	Command *for_var = ps->add_command_local_var(2, TypeInt);

	Command *num_old = ps->add_command_local_var(3, TypeInt);

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
		Command *cmd_el = ps->add_command_parray(self_data, for_var, t->parent);

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
		Command *cmd_el = ps->add_command_parray(self_data, for_var, t->parent);

		// __init__
		Command *cmd_init = ps->add_command_classfunc(t, *f_init, ps->ref_command(cmd_el));
		b->command.add(cmd_init);

		// ...for_var += 1
		Command *cmd_inc = ps->add_command_operator(for_var, num /*dummy*/, OperatorIntIncrease);
		b->command.add(cmd_inc);
		f->block->command.add(cb);
	}


	ClassFunction cf = ClassFunction("resize", TypeVoid, ps->script, fn);
	cf.param_type.add(TypeInt);
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

	Command *item = ps->add_command_local_var(0, t->parent);

	Command *self = ps->add_command_local_var(1, ps->GetPointerType(t));

	Command *self_num = ps->shift_command(ps->cp_command(self), true, config.PointerSize, TypeInt);


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
	Command *cmd_el = ps->add_command_parray(self_data, cmd_sub, t->parent);

	Command *cmd_assign = ps->LinkOperator(OperatorAssign, cmd_el, item);
	if (!cmd_assign)
		ps->DoError(format("%s.add(): no %s.__assign__ for elements", t->name.c_str(), t->parent->name.c_str()));
	f->block->command.add(cmd_assign);

	ClassFunction cf = ClassFunction("add", TypeVoid, ps->script, fn);
	cf.param_type.add(t->parent);
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


}
