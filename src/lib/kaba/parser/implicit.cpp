#include "../kaba.h"
#include "implicit.h"
#include "../asm/asm.h"
#include "Parser.h"
#include "../syntax/SyntaxTree.h"
#include <stdio.h>
#include "../../os/msg.h"
#include "../../base/iter.h"

namespace kaba {


const int FULL_CONSTRUCTOR_MAX_PARAMS = 4;

extern const Class *TypeCallableBase;


Array<const Class*> get_callable_param_types(const Class *fp);
const Class *get_callable_return_type(const Class *fp);
Array<const Class*> get_callable_capture_types(const Class *fp);


void db_add_print_node(Parser *p, shared<Block> block, shared<Node> node) {
	auto ff = p->tree->required_func_global("print");
	auto cmd = add_node_call(ff);
	cmd->set_param(0, p->con.add_converter_str(node, false));
	block->add(cmd);
}

void db_add_print_label(Parser *p, shared<Block> block, const string &s) {
	auto c = p->tree->add_constant(TypeString);
	c->as_string() = s;
	db_add_print_node(p, block, add_node_const(c));
}

void db_add_print_label_node(Parser *p, shared<Block> block, const string &s, shared<Node> node) {
	auto c = p->tree->add_constant(TypeString);
	c->as_string() = s;

	auto ff = p->tree->required_func_global("print");
	auto cmd = add_node_call(ff);
	cmd->set_param(0, p->con.link_operator_id(OperatorID::ADD, add_node_const(c), p->con.add_converter_str(node, false)));
	block->add(cmd);
}

AutoImplementer::AutoImplementer(Parser *p, SyntaxTree *t) {
	parser = p;
	tree = t;
}

void AutoImplementer::do_error_implicit(Function *f, const string &str) {
	int token_id = max(f->token_id, f->name_space->token_id);
	tree->do_error(format("[auto generating %s] : %s", f->signature(), str), token_id);
}

void AutoImplementer::auto_implement_add_virtual_table(shared<Node> self, Function *f, const Class *t) {
	if (t->vtable.num > 0) {
		auto *c = tree->add_constant_pointer(TypePointer, t->_vtable_location_target_);
		f->block->add(add_node_operator_by_inline(InlineID::POINTER_ASSIGN,
				self->shift(0, TypePointer),
				add_node_const(c)));
	}
}

void AutoImplementer::auto_implement_add_child_constructors(shared<Node> n_self, Function *f, const Class *t, bool allow_elements_from_parent) {
	int i0 = t->parent ? t->parent->elements.num : 0;
	if (allow_elements_from_parent)
		i0 = 0;
	for (auto&& [i,e]: enumerate(t->elements)) {
		if (i < i0)
			continue;
		Function *ff = e.type->get_default_constructor();
		if (e.type->needs_constructor() and !ff)
			do_error_implicit(f, format("missing default constructor for element %s", e.name));
		if (!ff)
			continue;
		f->block->add(add_node_member_call(ff,
				n_self->shift(e.offset, e.type)));
	}

	// auto initializers
	for (auto &init: t->initializers) {
		auto &e = t->elements[init.element];
		auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN,
				n_self->shift(e.offset, e.type),
				add_node_const(init.value.get()));
		if (!n_assign)
			do_error_implicit(f, format("(auto init) no operator %s = %s found", e.type->long_name(), init.value->type->long_name()));
		f->block->add(n_assign);

	}

	if (flags_has(t->flags, Flags::SHARED)) {
		for (auto &e: t->elements)
			if (e.name == IDENTIFIER_SHARED_COUNT and e.type == TypeInt) {
				f->block->add(add_node_operator_by_inline(InlineID::INT_ASSIGN,
						n_self->shift(e.offset, e.type),
						add_node_const(tree->add_constant_int(0))));
			}
	}
}

void AutoImplementer::auto_implement_regular_constructor(Function *f, const Class *t, bool allow_parent_constructor) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array()) {
	} else if (t->is_dict()) {
	} else if (t->is_array()) {
	} else if (t->is_pointer_shared() or t->is_pointer_owned()) {
	} else if (t->is_callable_fp()) {
	} else if (flags_has(f->flags, Flags::__INIT_FILL_ALL_PARAMS)) {
		// init
		auto_implement_add_child_constructors(self, f, t, true);

		// element[] = param[]
		for (auto&& [i,e]: enumerate(t->elements))
			if (!e.hidden()) {
				auto param = add_node_local(f->__get_var(e.name));
				auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN, self->shift(e.offset, e.type), param);
				if (!n_assign)
					do_error_implicit(f, format("no operator %s = %s found", param->type->long_name(), e.type->long_name()));
				f->block->add(n_assign);
			}
	} else {

		// parent constructor
		if (t->parent and allow_parent_constructor) {
			Function *f_same = t->parent->get_same_func(IDENTIFIER_FUNC_INIT, f);
			Function *f_def = t->parent->get_default_constructor();
			if (f_same) {
				// first, try same signature
				auto n_init_parent = add_node_member_call(f_same, self);
				for (int i=1; i<f_same->num_params; i++)
					n_init_parent->set_param(i, add_node_local(f->var[i].get()));
				f->block->add(n_init_parent);
			} else if (f_def) {
				// then, try default constructor
				f->block->add(add_node_member_call(f_def, self));
			} else if (t->parent->needs_constructor()) {
				do_error_implicit(f, "parent class does not have a default constructor or one with matching signature. Use super.__init__(...)");
			}
		}

		// call child constructors for elements
		auto_implement_add_child_constructors(self, f, t, false);

		// add vtable reference
		// after child constructor (otherwise would get overwritten)
		if (t->vtable.num > 0)
			auto_implement_add_virtual_table(self, f, t);
	}
}


void AutoImplementer::auto_implement_super_array_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto te = t->get_array_element();
	auto ff = t->get_member_func("__mem_init__", TypeVoid, {TypeInt});
	f->block->add(add_node_member_call(ff,
			self, -1,
			{add_node_const(tree->add_constant_int(te->size))}));
}



void AutoImplementer::auto_implement_array_constructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto te = t->get_array_element();
	auto *f_el_init = te->get_default_constructor();
	if (te->needs_constructor() and !f_el_init)
		do_error_implicit(f, format("missing default constructor for %s", te->long_name()));
	if (f_el_init) {
		for (int i=0; i<t->array_length; i++) {
			// self[i].__init__()
			f->block->add(add_node_member_call(f_el_init,
					self->shift(te->size * i, te)));
		}
	}
}


void AutoImplementer::auto_implement_dict_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto te = t->get_array_element();
	auto ff = t->get_member_func("__mem_init__", TypeVoid, {TypeInt});
	f->block->add(add_node_member_call(ff,
			self, -1,
			{add_node_const(tree->add_constant_int(te->size + TypeString->size))}));
}


void AutoImplementer::auto_implement_shared_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto te = t->param[0];
	// self.p = nil
	f->block->add(add_node_operator_by_inline(InlineID::POINTER_ASSIGN,
			self->shift(0, TypePointer),
			add_node_const(tree->add_constant_pointer(tree->get_pointer(te), nullptr))));
}

void AutoImplementer::auto_implement_regular_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	if (t->is_super_array() or t->is_dict()) {
		Function *f_clear = t->get_member_func("clear", TypeVoid, {});
		if (!f_clear)
			do_error_implicit(f, "clear() missing");
		f->block->add(add_node_member_call(f_clear, self));
	} else if (t->is_array()) {
		auto *f_el_del = te->get_destructor();
		if (f_el_del) {
			for (int i=0; i<t->array_length; i++) {
				// self[i].__delete__()
				f->block->add(add_node_member_call(f_el_del,
						self->shift(te->size * i, te)));
			}
		} else if (te->needs_destructor()) {
			do_error_implicit(f, "element destructor missing");
		}
	} else if (t->is_pointer_shared() or t->is_pointer_owned()) {
		// call clear()
		auto f_clear = t->get_member_func(IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {});
		if (!f_clear)
			do_error_implicit(f, IDENTIFIER_FUNC_SHARED_CLEAR + "() missing");
		f->block->add(add_node_member_call(f_clear,
				self));
	} else {

		// call child destructors
		int i0 = t->parent ? t->parent->elements.num : 0;
		for (auto&& [i,e]: enumerate(t->elements)) {
			if (i < i0)
				continue;
			Function *ff = e.type->get_destructor();
			if (!ff and e.type->needs_destructor())
				do_error_implicit(f, format("missing destructor for element %s", e.name));
			if (!ff)
				continue;
			// self.el.__delete__()
			f->block->add(add_node_member_call(ff,
					self->shift(e.offset, e.type)));
		}

		// parent destructor
		if (t->parent) {
			Function *ff = t->parent->get_destructor();
			if (ff)
				f->block->add(add_node_member_call(ff, self, -1, {}, true));
			else if (t->parent->needs_destructor())
				do_error_implicit(f, "parent destructor missing");
		}
	}
}

void AutoImplementer::auto_implement_array_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto *f_el_del = te->get_destructor();
	if (f_el_del) {
		for (int i=0; i<t->array_length; i++) {
			// self[i].__delete__()
			f->block->add(add_node_member_call(f_el_del,
					self->shift(te->size * i, te)));
		}
	} else if (te->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}
}


void AutoImplementer::auto_implement_shared_destructor(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	// call clear()
	auto f_clear = t->get_member_func(IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, IDENTIFIER_FUNC_SHARED_CLEAR + "() missing");
	f->block->add(add_node_member_call(f_clear, self));
}

void AutoImplementer::auto_implement_regular_assign(Function *f, const Class *t) {
	if (!f)
		return;
	auto te = t->get_array_element();
	auto n_other = add_node_local(f->__get_var("other"));
	auto n_self = add_node_local(f->__get_var(IDENTIFIER_SELF));


	// parent assignment
	if (t->parent) {
		auto p = n_self->shift(0, t->parent);
		auto o = n_other->shift(0, t->parent);

		auto cmd_assign = parser->con.link_operator_id(OperatorID::ASSIGN, p, o);
		if (!cmd_assign)
			do_error_implicit(f, "missing parent default constructor");
		f->block->add(cmd_assign);
	}

	// call child assignment
	int i0 = t->parent ? t->parent->elements.num : 0;
	for (auto&& [i,e]: enumerate(t->elements)) {
		if (i < i0)
			continue;
		auto p = n_self->shift(e.offset, e.type);
		auto o = n_other->shift(e.offset, e.type); // needed for call-by-ref conversion!

		auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN, p, o);
		if (!n_assign)
			do_error_implicit(f, format("no operator %s = %s for element \"%s\"", e.type->long_name(), e.type->long_name(), e.name));
		f->block->add(n_assign);
	}
}



void AutoImplementer::auto_implement_super_array_destructor(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	Function *f_clear = t->get_member_func("clear", TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, "clear() missing");
	f->block->add(add_node_member_call(f_clear, self));
}


void AutoImplementer::auto_implement_array_assign(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto n_other = add_node_local(f->__get_var("other"));
	auto n_self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	// for el,i in *self
	//    el = other[i]

	auto *v_el = f->block->add_var("el", tree->get_pointer(t->get_array_element()));
	auto *v_i = f->block->add_var("i", TypeInt);

	Block *b = new Block(f, f->block.get());

	// other[i]
	shared<Node> n_other_el = add_node_array(n_other, add_node_local(v_i));

	auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN, add_node_local(v_el)->deref(), n_other_el);
	if (!n_assign)
		do_error_implicit(f, format("no operator %s = %s found", te->long_name(), te->long_name()));
	b->add(n_assign);

	auto n_for = add_node_statement(StatementID::FOR_ARRAY);
	// [VAR, INDEX, ARRAY, BLOCK]
	n_for->set_param(0, add_node_local(v_el));
	n_for->set_param(1, add_node_local(v_i));
	n_for->set_param(2, n_self);
	n_for->set_param(3, b);
	f->block->add(n_for);

}


void AutoImplementer::auto_implement_super_array_assign(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto n_other = add_node_local(f->__get_var("other"));
	auto n_self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	Function *f_resize = t->get_member_func("resize", TypeVoid, {TypeInt});
	if (!f_resize)
		do_error_implicit(f, format("no %s.resize(int) found", t->long_name()));

	// self.resize(other.num)
	auto n_other_num = n_other->shift(config.pointer_size, TypeInt);

	auto n_resize = add_node_member_call(f_resize, n_self);
	n_resize->set_num_params(2);
	n_resize->set_param(1, n_other_num);
	f->block->add(n_resize);

	// for el,i in *self
	//    el = other[i]

	auto *v_el = f->block->add_var("el", tree->get_pointer(t->get_array_element()));
	auto *v_i = f->block->add_var("i", TypeInt);

	Block *b = new Block(f, f->block.get());

	// other[i]
	shared<Node> n_other_el = add_node_dyn_array(n_other, add_node_local(v_i));

	auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN, add_node_local(v_el)->deref(), n_other_el);
	if (!n_assign)
		do_error_implicit(f, format("no operator %s = %s found", te->long_name(), te->long_name()));
	b->add(n_assign);

	auto n_for = add_node_statement(StatementID::FOR_ARRAY);
	// [VAR, INDEX, ARRAY, BLOCK]
	n_for->set_param(0, add_node_local(v_el));
	n_for->set_param(1, add_node_local(v_i));
	n_for->set_param(2, n_self);
	n_for->set_param(3, b);
	f->block->add(n_for);

}

void AutoImplementer::auto_implement_super_array_clear(Function *f, const Class *t) {
	auto te = t->get_array_element();

	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

// delete...
	Function *f_del = te->get_destructor();
	if (f_del) {

		auto *var_i = f->block->add_var("i", TypeInt);
		auto *var_el = f->block->add_var("el", tree->get_pointer(t->get_array_element()));

		Block *b = new Block(f, f->block.get());

		// __delete__
		auto cmd_delete = add_node_member_call(f_del, add_node_local(var_el)->deref());
		b->add(cmd_delete);

		auto cmd_for = add_node_statement(StatementID::FOR_ARRAY);
		cmd_for->set_param(0, add_node_local(var_el));
		cmd_for->set_param(1, add_node_local(var_i));
		cmd_for->set_param(2, self);
		cmd_for->set_param(3, b);

		f->block->add(cmd_for);
	} else if (te->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	// clear
	auto cmd_clear = add_node_member_call(t->get_member_func("__mem_clear__", TypeVoid, {}), self);
	f->block->add(cmd_clear);
}


void AutoImplementer::auto_implement_super_array_resize(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto *var = f->block->add_var("i", TypeInt);
	f->block->add_var("num_old", TypeInt);

	auto num = add_node_local(f->__get_var("num"));

	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto self_num = self->shift(config.pointer_size, TypeInt);

	auto num_old = add_node_local(f->__get_var("num_old"));

	// num_old = self.num
	f->block->add(add_node_operator_by_inline(InlineID::INT_ASSIGN, num_old, self_num));

// delete...
	Function *f_del = te->get_destructor();
	if (f_del) {

		Block *b = new Block(f, f->block.get());

		// el := self[i]
		auto el = add_node_dyn_array(self, add_node_local(var));

		// __delete__
		auto cmd_delete = add_node_member_call(f_del, el);
		b->add(cmd_delete);

		//  [VAR, START, STOP, STEP, BLOCK]
		auto cmd_for = add_node_statement(StatementID::FOR_RANGE);
		cmd_for->set_param(0, add_node_local(var));
		cmd_for->set_param(1, num);
		cmd_for->set_param(2, self_num);
		cmd_for->set_param(3, add_node_const(tree->add_constant_int(1)));
		cmd_for->set_param(4, b);
		f->block->add(cmd_for);

	} else if (te->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	// resize
	auto c_resize = add_node_member_call(t->get_member_func("__mem_resize__", TypeVoid, {TypeInt}), self);
	c_resize->set_param(1, num);
	f->block->add(c_resize);

	// new...
	Function *f_init = te->get_default_constructor();
	if (f_init) {

		Block *b = new Block(f, f->block.get());

		// el := self[i]
		auto el = add_node_dyn_array(self, add_node_local(var));

		// __init__
		auto cmd_init = add_node_member_call(f_init, el);
		b->add(cmd_init);

		//  [VAR, START, STOP, STEP, BLOCK]
		auto cmd_for = add_node_statement(StatementID::FOR_RANGE);
		cmd_for->set_param(0, add_node_local(var));
		cmd_for->set_param(1, num_old);
		cmd_for->set_param(2, self_num);
		cmd_for->set_param(3, add_node_const(tree->add_constant_int(1)));
		cmd_for->set_param(4, b);
		f->block->add(cmd_for);

	} else if (te->needs_constructor()) {
		do_error_implicit(f, "element default constructor missing");
	}
}


void AutoImplementer::auto_implement_super_array_remove(Function *f, const Class *t) {
	auto te = t->get_array_element();
	auto index = add_node_local(f->__get_var("index"));
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	// delete...
	Function *f_del = te->get_destructor();
	if (f_del) {

		// el := self[index]
		auto cmd_el = add_node_dyn_array(self, index);

		// __delete__
		auto cmd_delete = add_node_member_call(f_del, cmd_el);
		f->block->params.add(cmd_delete);
	} else if (te->needs_destructor()) {
		do_error_implicit(f, "element destructor missing");
	}

	// resize
	auto c_remove = add_node_member_call(t->get_member_func("__mem_remove__", TypeVoid, {TypeInt}), self);
	c_remove->set_param(1, index);
	f->block->params.add(c_remove);
}

void AutoImplementer::auto_implement_super_array_add(Function *f, const Class *t) {
	auto te = t->get_array_element();
	Block *b = f->block.get();
	auto item = add_node_local(b->get_var("x"));

	auto self = add_node_local(b->get_var(IDENTIFIER_SELF));

	auto self_num = self->shift(config.pointer_size, TypeInt);


	// resize(self.num + 1)
	auto cmd_1 = add_node_const(tree->add_constant_int(1));
	auto cmd_add = add_node_operator_by_inline(InlineID::INT_ADD, self_num, cmd_1);
	auto cmd_resize = add_node_member_call(t->get_member_func("resize", TypeVoid, {TypeInt}), self);
	cmd_resize->set_param(1, cmd_add);
	b->add(cmd_resize);


	// el := self.data[self.num - 1]
	auto cmd_sub = add_node_operator_by_inline(InlineID::INT_SUBTRACT, self_num, cmd_1);
	auto cmd_el = add_node_dyn_array(self, cmd_sub);

	auto cmd_assign = parser->con.link_operator_id(OperatorID::ASSIGN, cmd_el, item);
	if (!cmd_assign)
		do_error_implicit(f, format("no operator %s = %s for elements found", te->long_name(), te->long_name()));
	b->add(cmd_assign);
}



void AutoImplementer::auto_implement_shared_assign(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	// call clear()
	auto f_clear = t->get_member_func(IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, IDENTIFIER_FUNC_SHARED_CLEAR + "() missing");
	auto call_clear = add_node_member_call(f_clear, self);
	f->block->add(call_clear);


	auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, p);
	f->block->add(op);


	// if p
	//     p.count ++
	auto cmd_if = add_node_statement(StatementID::IF);

	// if p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, p);
	cmd_if->set_param(0, cmd_cmp);

	auto b = new Block(f, f->block.get());
	cmd_if->set_param(1, b);


	f->block->add(cmd_if);

	auto tt = self->type->param[0];
	bool found = false;
	for (auto &e: tt->elements)
		if (e.name == IDENTIFIER_SHARED_COUNT and e.type == TypeInt) {
			// count ++
			auto count = self_p->deref()->shift(e.offset, e.type);
			auto inc = add_node_operator_by_inline(InlineID::INT_INCREASE, count, nullptr);
			b->add(inc);
			found = true;
		}
	if (!found)
		do_error_implicit(f, format("class '%s' is not a shared class (declare with '%s class' or add an element 'int %s')", tt->long_name(), IDENTIFIER_SHARED, IDENTIFIER_SHARED_COUNT));
}

void AutoImplementer::auto_implement_shared_clear(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));
	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	auto tt = t->param[0];

	// if self.p
	//     self.p.count --
	//     if self.p.count == 0
	//         del self.p
	//     self.p = nil

	auto cmd_if = add_node_statement(StatementID::IF);

	// if self.p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, self_p);
	cmd_if->set_param(0, cmd_cmp);

	auto b = new Block(f, f->block.get());


	shared<Node> count;
	for (auto &e: tt->elements)
		if (e.name == IDENTIFIER_SHARED_COUNT and e.type == TypeInt)
			count = self_p->deref_shift(e.offset, e.type, -1);
	if (!count)
		do_error_implicit(f, format("class '%s' is not a shared class (declare with '%s class' or add an element 'int %s')", tt->long_name(), IDENTIFIER_SHARED, IDENTIFIER_SHARED_COUNT));

	// count --
	auto dec = add_node_operator_by_inline(InlineID::INT_DECREASE, count, nullptr);
	b->add(dec);


	auto cmd_if_del = add_node_statement(StatementID::IF);

	// if count == 0
	auto zero = add_node_const(tree->add_constant_int(0));
	auto cmp = add_node_operator_by_inline(InlineID::INT_EQUAL, count, zero);
	cmd_if_del->set_param(0, cmp);

	auto b2 = new Block(f, b);


	// del self
	auto cmd_del = add_node_statement(StatementID::DELETE);
	cmd_del->set_param(0, self_p);
	b2->add(cmd_del);
	cmd_if_del->set_param(1, b2);
	b->add(cmd_if_del);


	// self = nil
	auto n_null = add_node_const(tree->add_constant_pointer(tree->get_pointer(t->param[0]), nullptr));
	auto n_op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, n_null);
	b->add(n_op);

	cmd_if->set_param(1, b);
	f->block->add(cmd_if);
}


void AutoImplementer::auto_implement_shared_create(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto r = add_node_local(f->block->add_var("r", t));


	// r = p
	auto f_assign = t->get_member_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, {p->type});
	if (!f_assign)
		do_error_implicit(f, "= missing...");
	auto call_assign = add_node_member_call(f_assign, r);
	call_assign->set_param(1, p);
	f->block->add(call_assign);

	// return r
	auto ret = add_node_statement(StatementID::RETURN);
	ret->set_num_params(1);
	ret->set_param(0, r);
	f->block->add(ret);
}



void AutoImplementer::auto_implement_owned_assign(Function *f, const Class *t) {
	auto p = add_node_local(f->__get_var("p"));
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	// call clear()
	auto f_clear = t->get_member_func(IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {});
	if (!f_clear)
		do_error_implicit(f, IDENTIFIER_FUNC_SHARED_CLEAR + "() missing");
	auto call_clear = add_node_member_call(f_clear, self);
	f->block->add(call_clear);


	auto op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, p);
	f->block->add(op);
}

void AutoImplementer::auto_implement_owned_clear(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));
	auto self_p = self->shift(0, tree->get_pointer(t->param[0]));

	auto tt = t->param[0];

	// if self.p
	//     del self.p
	//     self.p = nil

	auto cmd_if = add_node_statement(StatementID::IF);

	// if self.p
	auto ff = tree->required_func_global("p2b");
	auto cmd_cmp = add_node_call(ff);
	cmd_cmp->set_param(0, self_p);
	cmd_if->set_param(0, cmd_cmp);

	auto b = new Block(f, f->block.get());


	// del self
	auto cmd_del = add_node_statement(StatementID::DELETE);
	cmd_del->set_param(0, self_p);
	b->add(cmd_del);


	// self = nil
	auto n_null = add_node_const(tree->add_constant_pointer(tree->get_pointer(t->param[0]), nullptr));
	auto n_op = add_node_operator_by_inline(InlineID::POINTER_ASSIGN, self_p, n_null);
	b->add(n_op);

	cmd_if->set_param(1, b);
	f->block->add(cmd_if);
}



shared<Node> get_callable_fp(const Class *t, shared<Node> self) {
	for (auto &e: t->elements)
		if (e.name == "_fp")
			return self->shift(e.offset, e.type);
	return nullptr;
}

static const Array<string> DUMMY_PARAMS = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};


void AutoImplementer::auto_implement_callable_constructor(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	auto_implement_add_virtual_table(self, f, t);

	// self.fp = p
	{
		auto n_p = add_node_local(f->__get_var("p"));

		auto fp = get_callable_fp(t, self);
		auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN, fp, n_p);
		if (!n_assign)
			do_error_implicit(f, format("no operator %s = %s for element \"%s\"", fp->type->long_name(), fp->type->long_name(), "_fp"));
		f->block->add(n_assign);
	}

	int i_capture = 0;
	for (auto &e: t->elements)
		if (e.name.head(7) == "capture") {
			auto n_p = add_node_local(f->__get_var(DUMMY_PARAMS[i_capture ++]));

			auto fp = self->shift(e.offset, e.type);
			auto n_assign = parser->con.link_operator_id(OperatorID::ASSIGN, fp, n_p);
			if (!n_assign)
				do_error_implicit(f, format("no operator %s = %s for element \"%s\"", fp->type->long_name(), fp->type->long_name(), "_fp"));
			f->block->add(n_assign);
		}
}


void AutoImplementer::auto_implement_callable_fp_call(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	//db_add_print_label(this, f->block, "== callable.call ==");

	// contains a Function* pointer, extract its raw pointer
	auto raw = add_node_statement(StatementID::RAW_FUNCTION_POINTER);
	raw->type = TypeFunctionCodeP;
	raw->set_param(0, get_callable_fp(t, self));

	// call its raw pointer
	auto call = new Node(NodeKind::CALL_RAW_POINTER, 0, f->literal_return_type);
	call->set_num_params(1 + get_callable_param_types(t).num);
	call->set_param(0, raw);
	for (int i=1; i<f->num_params; i++) // skip "self"
		call->set_param(i, add_node_local(f->var[i].get()));

	if (f->literal_return_type == TypeVoid) {
		f->block->add(call);
	} else {
		auto ret = add_node_statement(StatementID::RETURN);
		ret->set_num_params(1);
		ret->set_param(0, call);
		f->block->add(ret);
	}
}



void AutoImplementer::auto_implement_callable_bind_call(Function *f, const Class *t) {
	auto self = add_node_local(f->__get_var(IDENTIFIER_SELF));

	//db_add_print_label(this, f->block, "== bind.call ==");

	auto fp = get_callable_fp(t, self);
	auto call = add_node_member_call(fp->type->param[0]->get_call(), fp);
	int index = 1;
	//for (int i=0; i<f->num_params; i++)
	//	call->set_param(i+1, add_node_local(f->var[i].get()));

	/*int
	for (int index=0; index<nn; index++) {

	}*/

	shared_array<Node> params;

	for (auto *v: weak(f->var)) {
		//msg_write("V " + v->name);
		if (v->name.num == 1) {
			//db_add_print_label_node(this, f->block, "  param " + v->name + ": ", add_node_local(v));
			params.add(add_node_local(v));
			//call->set_param(index ++, add_node_local(v));
		}
	}
	for (auto &e: t->elements) {
		//msg_write("E " + e.name);
		if (e.name.head(7) == "capture") {
			int n = e.name.replace("capture", "").replace("_ref", "")._int();
			params.insert(self->shift(e.offset, e.type), n);
			//db_add_print_label_node(this, f->block, "  capture " + e.name + ": ", self->shift(e.offset, e.type));
			//call->set_param(index ++, self->shift(e.offset, e.type));
		}
	}

	for (auto&& [i,p]: enumerate(params))
		call->set_param(i+1, p);

	if (f->literal_return_type == TypeVoid) {
		f->block->add(call);
	} else {
		auto ret = add_node_statement(StatementID::RETURN);
		ret->set_num_params(1);
		ret->set_param(0, call);
		f->block->add(ret);
	}
}


// skip the "self" parameter!
Function *SyntaxTree::add_func_header(Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, Function *cf, Flags flags, const shared_array<Node> &def_params) {
	Function *f = add_function(name, return_type, t, flags); // always member-function??? no...?
	f->auto_declared = true;
	f->token_id = t->token_id;
	for (auto&& [i,p]: enumerate(param_types)) {
		f->literal_param_type.add(p);
		auto v = f->block->add_var(param_names[i], p);
		flags_set(v->flags, Flags::CONST);
		f->num_params ++;
	}
	f->default_parameters = def_params;
	f->update_parameters_after_parsing();
	if (config.verbose)
		msg_write("ADD HEADER " + f->signature(TypeVoid));
	bool override = cf;
	t->add_function(this, f, false, override);
	return f;
}

bool needs_new(Function *f) {
	if (!f)
		return true;
	return f->needs_overriding();
}

Array<string> class_func_param_names(Function *cf) {
	Array<string> names;
	auto *f = cf;
	for (int i=0; i<f->num_params; i++)
		names.add(f->var[i]->name);
	return names;
}

bool has_user_constructors(const Class *t) {
	for (auto *cc: t->get_constructors())
		if (!cc->needs_overriding())
			return true;
	return false;
}

void remove_inherited_constructors(Class *t) {
	for (int i=t->functions.num-1; i>=0; i--)
		if (t->functions[i]->name == IDENTIFIER_FUNC_INIT and t->functions[i]->needs_overriding())
			t->functions.erase(i);
}

void redefine_inherited_constructors(Class *t, SyntaxTree *tree) {
	for (auto *pcc: t->parent->get_constructors()) {
		auto c = t->get_same_func(IDENTIFIER_FUNC_INIT, pcc);
		if (needs_new(c)) {
			auto ff = tree->add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, pcc->literal_param_type, class_func_param_names(pcc), c, Flags::NONE, pcc->default_parameters);
		}
	}
}

void add_full_constructor(Class *t, SyntaxTree *tree) {
	Array<string> names;
	Array<const Class*> types;
	for (auto &e: t->elements) {
		if (!e.hidden()) {
			names.add(e.name);
			types.add(e.type);
		}
	}
	auto f = tree->add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, types, names);
	flags_set(f->flags, Flags::__INIT_FILL_ALL_PARAMS);
}

bool can_fully_construct(const Class *t) {
	if (t->vtable.num > 0)
		return false;
	if (t->elements.num > FULL_CONSTRUCTOR_MAX_PARAMS)
		return false;
	int num_el = 0;
	for (auto &e: t->elements) {
		if (e.hidden())
			continue;
		if (!e.type->get_assign() and e.type->uses_call_by_reference()) {
			msg_write(format("class %s auto constructor prevented by element %s %s", t->name, e.name, e.type->name));
			return false;
		}
		num_el ++;
	}
	return num_el > 0;
}

bool class_should_assign(const Class *t) {
	if (t->parent)
		if (!class_should_assign(t->parent))
			return false;
	for (auto &e: t->elements) {
		if (!class_should_assign(e.type))
			return false;
		if (e.type->is_pointer_owned())
			return false;
	}
	return true;
}

void SyntaxTree::add_missing_function_headers_for_class(Class *t) {
	if (t->owner != this)
		return;
	if (t->is_pointer())
		return;

	if (t->is_super_array()) {
		add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(t, "clear", TypeVoid, {}, {});
		add_func_header(t, "resize", TypeVoid, {TypeInt}, {"num"});
		add_func_header(t, "add", TypeVoid, {t->param[0]}, {"x"});
		add_func_header(t, "remove", TypeVoid, {TypeInt}, {"index"});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	} else if (t->is_array()) {
		if (t->needs_constructor())
			add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		if (t->needs_destructor())
			add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	} else if (t->is_dict()) {
		add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(t, "clear", TypeVoid, {}, {});
		add_func_header(t, "add", TypeVoid, {TypeString, t->param[0]}, {"key", "x"});
		add_func_header(t, IDENTIFIER_FUNC_GET, t->param[0], {TypeString}, {"key"});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
	} else if (t->is_pointer_shared()) {
		add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {get_pointer(t->param[0])}, {"p"});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"p"});
		add_func_header(t, IDENTIFIER_FUNC_SHARED_CREATE, t, {get_pointer(t->param[0])}, {"p"}, nullptr, Flags::STATIC);
	} else if (t->is_pointer_owned()) {
		add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {get_pointer(t->param[0])}, {"p"});
		//add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"p"});
		//add_func_header(t, IDENTIFIER_FUNC_SHARED_CREATE, t, {t->param[0]->get_pointer()}, {"p"}, nullptr, Flags::STATIC);
	} else if (t->is_product()) {
		if (t->needs_constructor())
			add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {});
		add_full_constructor(t, this);
		if (!t->can_memcpy()) // needs destructor...
			add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {});
		add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"});
		if (t->get_assign() and t->can_memcpy())
			t->get_assign()->inline_no = InlineID::CHUNK_ASSIGN;
	} else if (t->is_callable_fp()) {
		add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {TypePointer}, {"p"});
		add_func_header(t, IDENTIFIER_FUNC_CALL, get_callable_return_type(t), get_callable_param_types(t), {"a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::CONST)->virtual_index = TypeCallableBase->get_call()->virtual_index;
	} else if (t->is_callable_bind()) {
		auto types = get_callable_capture_types(t);
		types.insert(TypePointer, 0);
		add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, types, {"p", "a", "b", "c", "d", "e", "f", "g", "h"});
		add_func_header(t, IDENTIFIER_FUNC_CALL, get_callable_return_type(t), get_callable_param_types(t), {"a", "b", "c", "d", "e", "f", "g", "h"}, nullptr, Flags::CONST)->virtual_index = TypeCallableBase->get_call()->virtual_index;
	} else { // regular classes
		if (t->can_memcpy()) {
			if (has_user_constructors(t)) {
			} else {
				if (t->needs_constructor())
					add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {}, t->get_default_constructor());
				if (can_fully_construct(t))
					add_full_constructor(t, this);
			}
		} else {
			if (t->parent) {
				if (has_user_constructors(t)) {
					// don't inherit constructors!
					remove_inherited_constructors(t);
				} else {
					// only auto-implement matching constructors
					redefine_inherited_constructors(t, this);
				}
			}
			if (t->get_constructors().num == 0) {
				if (t->needs_constructor())
					add_func_header(t, IDENTIFIER_FUNC_INIT, TypeVoid, {}, {}, t->get_default_constructor());
				if (can_fully_construct(t))
					add_full_constructor(t, this);
			}
			if (needs_new(t->get_destructor()))
				add_func_header(t, IDENTIFIER_FUNC_DELETE, TypeVoid, {}, {}, t->get_destructor());
		}
		if (class_should_assign(t) and needs_new(t->get_assign())) {
			//add_func_header(t, NAME_FUNC_ASSIGN, TypeVoid, t, "other");
			// implement only if parent has also done so
			if (t->parent) {
				if (t->parent->get_assign())
					add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			} else {
				add_func_header(t, IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			}
		}
		if (t->get_assign() and t->can_memcpy()) {
			t->get_assign()->inline_no = InlineID::CHUNK_ASSIGN;
		}
	}
}

Function* class_get_member_func(const Class *t, const string &name, const Class *return_type, const Array<const Class*> &params) {
	Function *cf = t->get_member_func(name, return_type, params);
	if (cf) {
		Function *f = cf;
		flags_clear(f->flags, Flags::NEEDS_OVERRIDE); // we're about to implement....
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
		flags_clear(f->flags, Flags::NEEDS_OVERRIDE); // we're about to implement....
		return f;
	}
	return nullptr;
	t->owner->module->do_error_internal("prepare class func..." + f->signature());
	return f;
}

// completely create and implement
void AutoImplementer::auto_implement_functions(const Class *t) {
	if (t->owner != tree)
		return;
	if (t->is_pointer())
		return;

	auto sub_classes = t->classes; // might change

	if (t->is_super_array()) {
		auto_implement_super_array_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
		auto_implement_super_array_destructor(prepare_auto_impl(t, t->get_destructor()), t);
		auto_implement_super_array_clear(prepare_auto_impl(t, t->get_member_func("clear", TypeVoid, {})), t);
		auto_implement_super_array_resize(prepare_auto_impl(t, t->get_member_func("resize", TypeVoid, {TypeInt})), t);
		auto_implement_super_array_remove(prepare_auto_impl(t, t->get_member_func("remove", TypeVoid, {TypeInt})), t);
		auto_implement_super_array_add(class_get_member_func(t, "add", TypeVoid, {nullptr}), t);
		auto_implement_super_array_assign(prepare_auto_impl(t, t->get_assign()), t);
	} else if (t->is_array()) {
		auto_implement_array_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
		auto_implement_array_destructor(prepare_auto_impl(t, t->get_destructor()), t);
		auto_implement_array_assign(prepare_auto_impl(t, t->get_assign()), t);
	} else if (t->is_dict()) {
		auto_implement_super_array_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
	} else if (t->is_pointer_shared()) {
		auto_implement_shared_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
		auto_implement_shared_destructor(prepare_auto_impl(t, t->get_destructor()), t);
		auto_implement_shared_clear(prepare_auto_impl(t, t->get_member_func(IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {})), t);
		auto_implement_shared_assign(prepare_auto_impl(t, t->get_member_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, {tree->get_pointer(t->param[0])})), t);
		auto_implement_shared_assign(prepare_auto_impl(t, t->get_member_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, {t})), t);
		auto_implement_shared_create(prepare_auto_impl(t, t->get_func(IDENTIFIER_FUNC_SHARED_CREATE, t, {tree->get_pointer(t->param[0])})), t);
	} else if (t->is_pointer_owned()) {
		auto_implement_shared_constructor(prepare_auto_impl(t, t->get_default_constructor()), t);
		auto_implement_shared_destructor(prepare_auto_impl(t, t->get_destructor()), t);
		auto_implement_owned_clear(prepare_auto_impl(t, t->get_member_func(IDENTIFIER_FUNC_SHARED_CLEAR, TypeVoid, {})), t);
		auto_implement_owned_assign(prepare_auto_impl(t, t->get_member_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, {tree->get_pointer(t->param[0])})), t);
		//auto_implement_shared_assign(prepare_auto_impl(t, t->get_func(IDENTIFIER_FUNC_ASSIGN, TypeVoid, {nullptr, t})), t);
		//auto_implement_shared_create(prepare_auto_impl(t, t->get_func(IDENTIFIER_FUNC_SHARED_CREATE, t, {nullptr, tree->get_pointer(t->param[0])})), t);
	} else if (t->is_callable_fp()) {
		for (auto *cf: t->get_constructors())
			auto_implement_callable_constructor(prepare_auto_impl(t, cf), t);
		auto_implement_callable_fp_call(prepare_auto_impl(t, t->get_call()), t);
	} else if (t->is_callable_bind()) {
		for (auto *cf: t->get_constructors())
			auto_implement_callable_constructor(prepare_auto_impl(t, cf), t);
		auto_implement_callable_bind_call(prepare_auto_impl(t, t->get_call()), t);
	} else {
		for (auto *cf: t->get_constructors())
			auto_implement_regular_constructor(prepare_auto_impl(t, cf), t, true);
		auto_implement_regular_destructor(prepare_auto_impl(t, t->get_destructor()), t); // if exists...
		auto_implement_regular_assign(prepare_auto_impl(t, t->get_assign()), t); // if exists...

	}

	// recursion
	//for (auto *c: sub_classes)
	//	auto_implement_functions(c);
}


}
