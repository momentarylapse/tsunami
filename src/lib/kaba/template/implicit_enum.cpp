/*
 * implicit_enum.cpp
 *
 *  Created on: 12 Feb 2023
 *      Author: michi
 */


#include "../kaba.h"
#include "implicit.h"
#include "../parser/Parser.h"

namespace kaba {

extern Module *cur_package_module;

int kaba_int_passthrough(int i);
int op_int_add(int a, int b);
bool op_int_eq(int a, int b);
bool op_int_neq(int a, int b);
int enum_parse(const string&, const Class*);
Array<int> enum_all(const Class*);


void AutoImplementer::_implement_functions_for_enum(const Class *t) {
}


Class* TemplateClassInstantiatorEnum::declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	auto c = const_cast<Class*>(params[0]);
	c->from_template = common_types.enum_t;
	flags_set(c->flags, Flags::ForceCallByValue);
	kaba::add_class(c);
	return c;
}
void TemplateClassInstantiatorEnum::add_function_headers(Class* t) {
	cur_package_module = t->owner->module;

	class_add_func("from_int", t, &kaba_int_passthrough, Flags::Static | Flags::Pure);
		func_set_inline(InlineID::Passthrough);
		func_add_param("i", common_types.i32);
	//class_add_func(Identifier::Func::STR, common_types.string, &i2s, Flags::PURE);
	class_add_func("__i32__", common_types.i32, &kaba_int_passthrough, Flags::Pure);
		func_set_inline(InlineID::Passthrough);
	if (!flags_has(t->flags, Flags::Noauto)) {
		class_add_func("parse", t, &enum_parse, Flags::Static | Flags::Pure);
			func_add_param("label", common_types.string);
			func_add_param("type", common_types.class_ref);
		class_add_func("all", common_types.dynamic_array, &enum_all, Flags::Static | Flags::Pure);
			func_add_param("type", common_types.class_ref);
	}
	add_operator(OperatorID::Assign, common_types._void, t, t, InlineID::Int32Assign);
	add_operator(OperatorID::Add, t, t, t, InlineID::Int32Add, &op_int_add);
	add_operator(OperatorID::AddAssign, common_types._void, t, t, InlineID::Int32AddAssign);
	add_operator(OperatorID::Equal, common_types._bool, t, t, InlineID::Int32Equal, &op_int_eq);
	add_operator(OperatorID::NotEqual, common_types._bool, t, t, InlineID::Int32NotEqual, &op_int_neq);
	add_operator(OperatorID::BitAnd, t, t, t, InlineID::Int32BitAnd);
	add_operator(OperatorID::BitOr, t, t, t, InlineID::Int32BitOr);

	for (auto f: weak(t->functions)) {
		if (f->name == "parse") {
			f->abstract_node->params[2]->set_num_params(2*3);
			auto c = t->owner->add_constant(common_types.class_ref, t);
			c->as_int64() = (int_p)t;
			f->mandatory_params = 1;
			f->abstract_node->params[2]->set_param(1*3+2, add_node_const(c, t->token_id));
		} else if (f->name == "all") {
			f->literal_return_type = t->owner->request_implicit_class_list(t, t->token_id);
			f->abstract_node->params[2]->set_num_params(1*3);
			auto c = t->owner->add_constant(common_types.class_ref, t);
			c->as_int64() = (int_p)t;
			f->mandatory_params = 0;
			f->abstract_node->params[2]->set_param(0*3+2, add_node_const(c, t->token_id));
		}
	}
}

}



