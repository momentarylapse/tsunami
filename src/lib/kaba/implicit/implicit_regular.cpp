/*
 * implicit_regular.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */

#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"
#include "../../base/iter.h"

namespace kaba {

void AutoImplementer::_add_missing_function_headers_for_regular(Class *t) {
	if (t->is_struct()) {
		// force to have:
		if (!flags_has(t->flags, Flags::NOAUTO)) {
			if (t->parent) {
				if (has_user_constructors(t)) {
					// don't inherit constructors!
					remove_inherited_constructors(t);
				} else {
					// only auto-implement matching constructors
					redefine_inherited_constructors(t);
				}
			}
			if (t->get_constructors().num == 0) {
				if (t->needs_constructor())
					add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, t->get_default_constructor());
				if (class_can_fully_construct(t))
					add_full_constructor(t);
			}
			if (needs_new(t->get_destructor()))
				add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {}, t->get_destructor());
			if (needs_new(t->get_assign()))
				add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());

		}
		if (t->get_assign() and t->can_memcpy()) {
			t->get_assign()->inline_no = InlineID::CHUNK_ASSIGN;
		}

	} else {
		// class X

		// TODO rethink
		if (t->can_memcpy()) {
			if (has_user_constructors(t)) {
			} else {
				if (t->needs_constructor())
					add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, t->get_default_constructor());
				/*if (!flags_has(t->flags, Flags::NOAUTO))
					if (can_fully_construct(t))
						add_full_constructor(t);*/
			}
		} else {
			if (t->parent) {
				if (has_user_constructors(t)) {
					// don't inherit constructors!
					remove_inherited_constructors(t);
				} else {
					// only auto-implement matching constructors
					redefine_inherited_constructors(t);
				}
			}
			if (t->get_constructors().num == 0) {
				if (t->needs_constructor())
					add_func_header(t, Identifier::Func::INIT, TypeVoid, {}, {}, t->get_default_constructor());
				/*if (!flags_has(t->flags, Flags::NOAUTO))
					if (can_fully_construct(t))
						add_full_constructor(t);*/
			}
			if (needs_new(t->get_destructor()))
				add_func_header(t, Identifier::Func::DELETE, TypeVoid, {}, {}, t->get_destructor());
		}

		/*if (!flags_has(t->flags, Flags::NOAUTO) and needs_new(t->get_assign())) {
			if (t->parent) {
				// implement only if parent has also done so
				if (class_can_assign(t->parent))
					add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			} else {
				if (class_can_elements_assign(t))
					add_func_header(t, Identifier::Func::ASSIGN, TypeVoid, {t}, {"other"}, t->get_assign());
			}
		}
		if (t->get_assign() and t->can_memcpy()) {
			t->get_assign()->inline_no = InlineID::CHUNK_ASSIGN;
		}*/
	}
}

void AutoImplementer::implement_add_virtual_table(shared<Node> self, Function *f, const Class *t) {
	if (t->vtable.num > 0) {
		auto *c = tree->add_constant_pointer(TypePointer, t->_vtable_location_target_);
		f->block->add(add_node_operator_by_inline(InlineID::POINTER_ASSIGN,
				self->change_type(TypePointer),
				add_node_const(c)));
	}
}

void AutoImplementer::implement_add_child_constructors(shared<Node> n_self, Function *f, const Class *t, bool allow_elements_from_parent) {
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
			if (e.name == Identifier::SHARED_COUNT and e.type == TypeInt) {
				f->block->add(add_node_operator_by_inline(InlineID::INT_ASSIGN,
						n_self->shift(e.offset, e.type),
						add_node_const(tree->add_constant_int(0))));
			}
	}
}

void AutoImplementer::implement_regular_constructor(Function *f, const Class *t, bool allow_parent_constructor) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	if (flags_has(f->flags, Flags::__INIT_FILL_ALL_PARAMS)) {
		// init
		implement_add_child_constructors(self, f, t, true);

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
			Function *f_same = t->parent->get_same_func(Identifier::Func::INIT, f);
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
		implement_add_child_constructors(self, f, t, false);

		// add vtable reference
		// after child constructor (otherwise would get overwritten)
		if (t->vtable.num > 0)
			implement_add_virtual_table(self, f, t);
	}
}

void AutoImplementer::implement_regular_destructor(Function *f, const Class *t) {
	if (!f)
		return;
	auto self = add_node_local(f->__get_var(Identifier::SELF));

	// call child destructors
	int i0 = t->parent ? t->parent->elements.num : 0;
	for (auto&& [i,e]: enumerate(t->elements)) {
		if (i < i0)
			continue;

		// self.el.__delete__()
		if (auto f_des = e.type->get_destructor()) {
			f->block->add(add_node_member_call(f_des,
					self->shift(e.offset, e.type)));
		} else if (e.type->needs_destructor()) {
			do_error_implicit(f, format("missing destructor for element %s", e.name));
		}
	}

	// parent destructor
	if (t->parent) {
		if (auto f_des = t->parent->get_destructor())
			f->block->add(add_node_member_call(f_des, self, -1, {}, true));
		else if (t->parent->needs_destructor())
			do_error_implicit(f, "parent destructor missing");
	}
}


void AutoImplementer::implement_regular_assign(Function *f, const Class *t) {
	if (!f)
		return;

	auto n_other = add_node_local(f->__get_var("other"));
	auto n_self = add_node_local(f->__get_var(Identifier::SELF));


	// parent assignment
	if (t->parent) {
		auto p = n_self->change_type(t->parent);
		auto o = n_other->change_type(t->parent);

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
			do_error_implicit(f, format("no operator '%s = %s' for element '%s'", e.type->long_name(), e.type->long_name(), e.name));
		f->block->add(n_assign);
	}
}

void AutoImplementer::_implement_functions_for_regular(const Class *t) {
	for (auto *cf: t->get_constructors())
		implement_regular_constructor(prepare_auto_impl(t, cf), t, true);
	implement_regular_destructor(prepare_auto_impl(t, t->get_destructor()), t); // if exists...
	implement_regular_assign(prepare_auto_impl(t, t->get_assign()), t); // if exists...
}


}


