#include "../kaba.h"
#include "../asm/asm.h"
#include <stdio.h>
#include "../../file/file.h"

namespace Kaba{

void SyntaxTree::auto_implement_add_virtual_table(Node *self, Function *f, const Class *t) {
	if (t->vtable.num > 0) {
		Node *p = shift_node(self, false, 0, TypePointer);
		auto *c = add_constant_pointer(TypePointer, t->_vtable_location_target_);
		Node *n_0 = add_node_const(c);
		Node *n_assign = add_node_operator_by_inline(p, n_0, InlineID::POINTER_ASSIGN);
		f->block->add(n_assign);
	}
}

void SyntaxTree::auto_implement_add_child_constructors(Node *n_self, Function *f, const Class *t) {
	int i0 = t->parent ? t->parent->elements.num : 0;
	foreachi(ClassElement &e, t->elements, i) {
		if (i < i0)
			continue;
		Function *ff = e.type->get_default_constructor();
		if (e.type->needs_constructor() and !ff)
			do_error_implicit(f, format("missing default constructor for element %s", e.name.c_str()));
		if (!ff)
			continue;
		Node *p = shift_node(cp_node(n_self), false, e.offset, e.type);
		Node *c = add_node_member_call(ff, p);
		f->block->add(c);
	}
}

void SyntaxTree::auto_implement_constructor(Function *f, const Class *t, bool allow_parent_constructor) {
	if (!f)
		return;
	Node *n_self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array()) {
		Node *n_el_size = add_node_const(add_constant_int(t->param->size));
		Node *n_mem_init = add_node_member_call(t->get_func("__mem_init__", TypeVoid, {TypeInt}), n_self);
		n_mem_init->set_uparam(1, n_el_size);
		f->block->add(n_mem_init);
	}else if (t->is_dict()) {
		Node *n_el_size = add_node_const(add_constant_int(t->param->size + TypeString->size));
		Node *n_mem_init = add_node_member_call(t->get_func("__mem_init__", TypeVoid, {TypeInt}), n_self);
		n_mem_init->set_uparam(1, n_el_size);
		f->block->add(n_mem_init);
	}else if (t->is_array()) {
		auto *pc_el_init = t->param->get_default_constructor();
		if (t->param->needs_constructor() and !pc_el_init)
			do_error_implicit(f, format("missing default constructor for %s", t->param->long_name().c_str()));
		if (pc_el_init) {
			for (int i=0; i<t->array_length; i++) {
				Node *n_el = shift_node(cp_node(n_self), false, t->param->size * i, t->param);
				Node *n_init_el = add_node_member_call(pc_el_init, n_el);
				f->block->add(n_init_el);
			}
		}
		delete n_self;
	} else {

		// parent constructor
		if (t->parent and allow_parent_constructor) {
			Function *pc_same = t->parent->get_same_func(IDENTIFIER_FUNC_INIT, f);
			Function *pc_def = t->parent->get_default_constructor();
			if (pc_same) {
				// first, try same signature
				Node *n_init_parent = add_node_member_call(pc_same, cp_node(n_self));
				for (int i=0; i<pc_same->num_params; i++)
					n_init_parent->set_uparam(i+1, add_node_local(f->var[i]));
				f->block->add(n_init_parent);
			} else if (pc_def) {
				// then, try default constructor
				f->block->add(add_node_member_call(pc_def, cp_node(n_self)));
			} else if (t->parent->needs_constructor()) {
				do_error_implicit(f, "parent class does not have a default constructor or one with matching signature. Use super.__init__(...)");
			}
		}

		// call child constructors for elements
		auto_implement_add_child_constructors(n_self, f, t);

		// add vtable reference
		// after child constructor (otherwise would get overwritten)
		if (t->vtable.num > 0)
			auto_implement_add_virtual_table(cp_node(n_self), f, t);
		delete n_self;
	}
}

void SyntaxTree::auto_implement_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	Node *n_self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array() or t->is_dict()) {
		Function *f_clear = t->get_func("clear", TypeVoid, {});
		if (!f_clear)
			do_error_implicit(f, "clear() missing");
		f->block->add(add_node_member_call(f_clear, n_self));
	} else if (t->is_array()) {
		auto *pc_el_init = t->param->get_destructor();
		if (pc_el_init) {
			for (int i=0; i<t->array_length; i++){
				Node *p = shift_node(cp_node(n_self), false, t->param->size * i, t->param);
				Node *c = add_node_member_call(pc_el_init, p);
				f->block->add(c);
			}
		} else if (t->param->needs_destructor()) {
			do_error_implicit(f, "element desctructor missing");
		}
		delete n_self;
	} else {

		// call child destructors
		int i0 = t->parent ? t->parent->elements.num : 0;
		foreachi(ClassElement &e, t->elements, i) {
			if (i < i0)
				continue;
			Function *ff = e.type->get_destructor();
			if (!ff and e.type->needs_destructor())
				do_error_implicit(f, format("missing destructor for element %s", e.name.c_str()));
			if (!ff)
				continue;
			Node *p = shift_node(cp_node(n_self), false, e.offset, e.type);
			f->block->add(add_node_member_call(ff, p));
		}

		// parent destructor
		if (t->parent) {
			Function *ff = t->parent->get_destructor();
			if (ff)
				f->block->add(add_node_member_call(ff, cp_node(n_self), true));
			else if (t->parent->needs_destructor())
				do_error_implicit(f, "parent desctructor missing");
		}
		delete n_self;
	}
}

void SyntaxTree::auto_implement_assign(Function *f, const Class *t) {
	if (!f)
		return;
	Node *n_other = add_node_local(f->__get_var("other"));
	Node *n_self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array() or t->is_array()){

		if (t->is_super_array()) {
			Function *f_resize = t->get_func("resize", TypeVoid, {TypeInt});
			if (!f_resize)
				do_error_implicit(f, format("no %s.resize(int) found", t->long_name().c_str()));

			// self.resize(other.num)
			Node *n_other_num = shift_node(n_other, false, config.pointer_size, TypeInt);

			Node *n_resize = add_node_member_call(f_resize, n_self);
			n_resize->set_num_uparams(2);
			n_resize->set_uparam(1, n_other_num);
			f->block->add(n_resize);
		}

		// for el,i in *self
		//    el = other[i]

		auto *v_el = f->block->add_var("el", t->get_array_element()->get_pointer());
		auto *v_i = f->block->add_var("i", TypeInt);

		Block *b = new Block(f, f->block);

		// other[i]
		Node *n_other_el;
		if (t->is_array())
			n_other_el = add_node_array(cp_node(n_other), add_node_local(v_i));
		else
			n_other_el = add_node_dyn_array(cp_node(n_other), add_node_local(v_i));

		Node *n_assign = link_operator_id(OperatorID::ASSIGN, deref_node(add_node_local(v_el)), n_other_el);
		if (!n_assign)
			do_error_implicit(f, format("no %s.__assign__() found", t->param->long_name().c_str()));
		b->add(n_assign);

		Node *n_for = add_node_statement(StatementID::FOR_ARRAY);
		// [VAR, INDEX, ARRAY, BLOCK]
		n_for->set_uparam(0, add_node_local(v_el));
		n_for->set_uparam(1, add_node_local(v_i));
		n_for->set_uparam(2, cp_node(n_self));
		n_for->set_uparam(3, b);
		f->block->add(n_for);

	} else {

		// parent assignment
		if (t->parent) {
			Node *p = cp_node(n_self);
			Node *o = cp_node(n_other);
			p->type = o->type = t->parent;

			Node *cmd_assign = link_operator_id(OperatorID::ASSIGN, p, o);
			if (!cmd_assign)
				do_error_implicit(f, "missing parent default constructor");
			f->block->add(cmd_assign);
		}

		// call child assignment
		int i0 = t->parent ? t->parent->elements.num : 0;
		foreachi(ClassElement &e, t->elements, i) {
			if (i < i0)
				continue;
			Node *p = shift_node(cp_node(n_self), false, e.offset, e.type);
			Node *o = shift_node(cp_node(n_other), false, e.offset, e.type); // needed for call-by-ref conversion!

			Node *n_assign = link_operator_id(OperatorID::ASSIGN, p, o);
			if (!n_assign)
				do_error_implicit(f, format("no %s.__assign__ for element \"%s\"", e.type->long_name().c_str(), e.name.c_str()));
			f->block->add(n_assign);
		}

		delete n_self;
	}
}


void SyntaxTree::auto_implement_array_clear(Function *f, const Class *t) {
	if (!f)
		return;

	Node *self = add_node_local(f->__get_var(IDENTIFIER_SELF));

// delete...
	Function *f_del = t->param->get_destructor();
	if (f_del) {

		auto *var_i = f->block->add_var("i", TypeInt);
		auto *var_el = f->block->add_var("el", t->get_array_element()->get_pointer());

		Block *b = new Block(f, f->block);

		// __delete__
		Node *cmd_delete = add_node_member_call(f_del, deref_node(add_node_local(var_el)));
		b->add(cmd_delete);

		Node *cmd_for = add_node_statement(StatementID::FOR_ARRAY);
		cmd_for->set_uparam(0, add_node_local(var_el));
		cmd_for->set_uparam(1, add_node_local(var_i));
		cmd_for->set_uparam(2, cp_node(self));
		cmd_for->set_uparam(3, b);

		f->block->add(cmd_for);
	} else if (t->param->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	// clear
	Node *cmd_clear = add_node_member_call(t->get_func("__mem_clear__", TypeVoid, {}), self);
	f->block->add(cmd_clear);
}


void SyntaxTree::auto_implement_array_resize(Function *f, const Class *t) {
	if (!f)
		return;
	auto *var = f->block->add_var("i", TypeInt);
	f->block->add_var("num_old", TypeInt);

	Node *num = add_node_local(f->__get_var("num"));

	Node *self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	Node *self_num = shift_node(self, false, config.pointer_size, TypeInt);

	Node *num_old = add_node_local(f->__get_var("num_old"));

	// num_old = self.num
	f->block->add(add_node_operator_by_inline(num_old, self_num, InlineID::INT_ASSIGN));

// delete...
	Function *f_del = t->param->get_destructor();
	if (f_del) {

		Block *b = new Block(f, f->block);

		// el := self[i]
		Node *el = add_node_dyn_array(cp_node(self), add_node_local(var));

		// __delete__
		Node *cmd_delete = add_node_member_call(f_del, el);
		b->add(cmd_delete);

		//  [VAR, START, STOP, STEP, BLOCK]
		Node *cmd_for = add_node_statement(StatementID::FOR_RANGE);
		cmd_for->set_uparam(0, add_node_local(var));
		cmd_for->set_uparam(1, cp_node(num));
		cmd_for->set_uparam(2, cp_node(self_num));
		cmd_for->set_uparam(3, add_node_const(add_constant_int(1)));
		cmd_for->set_uparam(4, b);
		f->block->add(cmd_for);

	} else if (t->param->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	// resize
	Node *c_resize = add_node_member_call(t->get_func("__mem_resize__", TypeVoid, {TypeInt}), cp_node(self));
	c_resize->set_uparam(1, num);
	f->block->add(c_resize);

	// new...
	Function *f_init = t->param->get_default_constructor();
	if (f_init) {

		Block *b = new Block(f, f->block);

		// el := self[i]
		Node *el = add_node_dyn_array(cp_node(self), add_node_local(var));

		// __init__
		Node *cmd_init = add_node_member_call(f_init, el);
		b->add(cmd_init);

		//  [VAR, START, STOP, STEP, BLOCK]
		Node *cmd_for = add_node_statement(StatementID::FOR_RANGE);
		cmd_for->set_uparam(0, add_node_local(var));
		cmd_for->set_uparam(1, cp_node(num_old));
		cmd_for->set_uparam(2, cp_node(self_num));
		cmd_for->set_uparam(3, add_node_const(add_constant_int(1)));
		cmd_for->set_uparam(4, b);
		f->block->add(cmd_for);

	} else if (t->param->needs_constructor()) {
		do_error_implicit(f, "element default constructor missing");
	}
}


void SyntaxTree::auto_implement_array_remove(Function *f, const Class *t) {
	if (!f)
		return;

	Node *index = add_node_local(f->__get_var("index"));
	Node *self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	// delete...
	Function *f_del = t->param->get_destructor();
	if (f_del) {

		// el := self[index]
		Node *cmd_el = add_node_dyn_array(cp_node(self), cp_node(index));

		// __delete__
		Node *cmd_delete = add_node_member_call(f_del, cmd_el);
		f->block->uparams.add(cmd_delete);
	} else if (t->param->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	// resize
	Node *c_remove = add_node_member_call(t->get_func("__mem_remove__", TypeVoid, {TypeInt}), self);
	c_remove->set_uparam(1, index);
	f->block->uparams.add(c_remove);
}

void SyntaxTree::auto_implement_array_add(Function *f, const Class *t) {
	if (!f)
		return;
	Block *b = f->block;
	Node *item = add_node_local(b->get_var("x"));

	Node *self = add_node_local(b->get_var(IDENTIFIER_SELF));

	Node *self_num = shift_node(cp_node(self), false, config.pointer_size, TypeInt);


	// resize(self.num + 1)
	Node *cmd_1 = add_node_const(add_constant_int(1));
	Node *cmd_add = add_node_operator_by_inline(self_num, cmd_1, InlineID::INT_ADD);
	Node *cmd_resize = add_node_member_call(t->get_func("resize", TypeVoid, {TypeInt}), self);
	cmd_resize->set_uparam(1, cmd_add);
	b->add(cmd_resize);


	// el := self.data[self.num - 1]
	Node *cmd_sub = add_node_operator_by_inline(cp_node(self_num), cp_node(cmd_1), InlineID::INT_SUBTRACT);
	Node *cmd_el = add_node_dyn_array(cp_node(self), cmd_sub);

	Node *cmd_assign = link_operator_id(OperatorID::ASSIGN, cmd_el, item);
	if (!cmd_assign)
		do_error_implicit(f, format("no %s.%s for elements", t->param->long_name().c_str(), IDENTIFIER_FUNC_ASSIGN.c_str()));
	b->add(cmd_assign);
}

void add_func_header(SyntaxTree *s, Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, Function *cf = nullptr) {
	Function *f = s->add_function(name, return_type, t, false); // always member-function!
	f->auto_declared = true;
	foreachi (auto &p, param_types, i) {
		f->literal_param_type.add(p);
		f->block->add_var(param_names[i], p);
		f->num_params ++;
	}
	f->update_parameters_after_parsing();
	bool override = cf;
	t->add_function(s, f, false, override);
}

bool needs_new(Function *f) {
	if (!f)
		return true;
	return f->needs_overriding;
}

Array<string> class_func_param_names(Function *cf) {
	Array<string> names;
	auto *f = cf;
	for (int i=0; i<f->num_params; i++)
		names.add(f->var[i]->name);
	return names;
}

void SyntaxTree::add_missing_function_headers_for_class(Class *t) {
	if (t->owner != this)
		return;
	if (t->is_pointer())
		return;

	if (t->is_super_array()) {
		add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(this, t, "clear", TypeVoid, {}, {});
		add_func_header(this, t, "resize", TypeVoid, {TypeInt}, {"num"});
		add_func_header(this, t, "add", TypeVoid, {t->param}, {"x"});
		add_func_header(this, t, "remove", TypeVoid, {TypeInt}, {"index"});
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	} else if (t->is_array()) {
		if (t->needs_constructor())
			add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		if (t->needs_destructor())
			add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	} else if (t->is_dict()) {
		add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(this, t, "clear", TypeVoid, {}, {});
		add_func_header(this, t, "add", TypeVoid, {TypeString, t->param}, {"key", "x"});
		add_func_header(this, t, "__get__", t->param, {TypeString}, {"key"});
		add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	} else { // regular classes
		if (!t->is_simple_class()) {
			if (t->parent) {
				bool has_own_constructors = false;
				for (auto *cc: t->get_constructors())
					if (!cc->needs_overriding)
						has_own_constructors = true;

				if (has_own_constructors) {
					// don't inherit constructors!
					for (int i=t->functions.num-1; i>=0; i--)
						if (t->functions[i]->name == IDENTIFIER_FUNC_INIT and t->functions[i]->needs_overriding)
							t->functions.erase(i);
				} else {
					// only auto-implement matching constructors
					for (auto *pcc: t->parent->get_constructors()) {
						auto c = t->get_same_func(IDENTIFIER_FUNC_INIT, pcc);
						if (needs_new(c))
							add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, pcc->literal_param_type, class_func_param_names(pcc), c);
					}
				}
			} else {
				if (t->needs_constructor() and needs_new(t->get_default_constructor()))
					if (t->get_constructors().num == 0)
						add_func_header(this, t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {}, t->get_default_constructor());
			}
			if (needs_new(t->get_destructor()))
				add_func_header(this, t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {}, t->get_destructor());
		}
		if (needs_new(t->get_assign())) {
			//add_func_header(this, t, NAME_FUNC_ASSIGN, TypeVoid, t, "other");
			// implement only if parent has also done so
			if (t->parent) {
				if (t->parent->get_assign())
					add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			} else {
				add_func_header(this, t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			}
		}
		if (t->get_assign() and t->is_simple_class()) {
			t->get_assign()->inline_no = InlineID::CHUNK_ASSIGN;
		}
	}
}

Function* class_get_func(const Class *t, const string &name, const Class *return_type, const Array<const Class*> &params) {
	Function *cf = t->get_func(name, return_type, params);
	if (cf) {
		Function *f = cf;
		f->needs_overriding = false; // we're about to implement....
		if (f->auto_declared) {
			return f;
		}
		return nullptr;
	}
	//t->owner->DoError("class_get_func... " + t->name + "." + name);
	return nullptr;
}

Function* prepare_auto_impl(const Class *t, Function *f) {
	if (!f)
		return nullptr;
	if (f->auto_declared) {
		f->needs_overriding = false; // we're about to implement....
		return f;
	}
	return nullptr;
	t->owner->script->do_error_internal("prepare class func..." + f->signature());
	return f;
}

// completely create and implement
void SyntaxTree::auto_implement_functions(const Class *t) {
	if (t->owner != this)
		return;
	if (t->is_pointer())
		return;

	auto sub_classes = t->classes; // might change

	if (t->is_super_array()) {
		auto_implement_constructor(prepare_auto_impl(t, t->get_default_constructor()), t, true);
		auto_implement_destructor(prepare_auto_impl(t, t->get_destructor()), t);
		auto_implement_array_clear(prepare_auto_impl(t, t->get_func("clear", TypeVoid, {})), t);
		auto_implement_array_resize(prepare_auto_impl(t, t->get_func("resize", TypeVoid, {TypeInt})), t);
		auto_implement_array_remove(prepare_auto_impl(t, t->get_func("remove", TypeVoid, {TypeInt})), t);
		auto_implement_array_add(class_get_func(t, "add", TypeVoid, {nullptr}), t);
		auto_implement_assign(prepare_auto_impl(t, t->get_assign()), t);
	} else if (t->is_array()) {
		auto_implement_constructor(prepare_auto_impl(t, t->get_default_constructor()), t, true);
		auto_implement_destructor(prepare_auto_impl(t, t->get_destructor()), t);
		auto_implement_assign(prepare_auto_impl(t, t->get_assign()), t);
	} else if (t->is_dict()) {
		auto_implement_constructor(prepare_auto_impl(t, t->get_default_constructor()), t, true);
	} else {
		for (auto *cf: t->get_constructors())
			auto_implement_constructor(prepare_auto_impl(t, cf), t, true);
		auto_implement_destructor(prepare_auto_impl(t, t->get_destructor()), t); // if exists...
		auto_implement_assign(prepare_auto_impl(t, t->get_assign()), t); // if exists...
	}

	// recursion
	//for (auto *c: sub_classes)
	//	auto_implement_functions(c);
}


}
