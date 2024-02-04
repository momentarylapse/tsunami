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

extern const Class *TypeDynamicArray;
extern Module *cur_package;

int kaba_int_passthrough(int i);
int op_int_add(int a, int b);
bool op_int_eq(int a, int b);
bool op_int_neq(int a, int b);
int enum_parse(const string&, const Class*);
Array<int> enum_all(const Class*);

void AutoImplementer::_add_missing_function_headers_for_enum(Class *t) {
	cur_package = t->owner->module;

	class_add_func("from_int", t, &kaba_int_passthrough, Flags::STATIC | Flags::PURE);
		func_set_inline(InlineID::PASSTHROUGH);
		func_add_param("i", TypeInt);
	//class_add_func(Identifier::Func::STR, TypeString, &i2s, Flags::PURE);
	class_add_func("__int__", TypeInt, &kaba_int_passthrough, Flags::PURE);
		func_set_inline(InlineID::PASSTHROUGH);
    if (!flags_has(t->flags, Flags::NOAUTO)) {
        class_add_func("parse", t, &enum_parse, Flags::STATIC | Flags::PURE);
            func_add_param("label", TypeString);
            func_add_param("type", TypeClassRef);
        class_add_func("all", TypeDynamicArray, &enum_all, Flags::STATIC | Flags::PURE);
            func_add_param("type", TypeClassRef);
    }
	add_operator(OperatorID::ASSIGN, TypeVoid, t, t, InlineID::INT32_ASSIGN);
	add_operator(OperatorID::ADD, t, t, t, InlineID::INT32_ADD, &op_int_add);
	add_operator(OperatorID::ADDS, TypeVoid, t, t, InlineID::INT32_ADD_ASSIGN);
	add_operator(OperatorID::EQUAL, TypeBool, t, t, InlineID::INT32_EQUAL, &op_int_eq);
	add_operator(OperatorID::NOT_EQUAL, TypeBool, t, t, InlineID::INT32_NOT_EQUAL, &op_int_neq);
	add_operator(OperatorID::BIT_AND, t, t, t, InlineID::INT32_AND);
	add_operator(OperatorID::BIT_OR, t, t, t, InlineID::INT32_OR);

	for (auto f: weak(t->functions)) {
		if (f->name == "parse") {
			f->default_parameters.resize(2);
			auto c = tree->add_constant(TypeClassRef, t);
			c->as_int64() = (int_p)t;
			f->mandatory_params = 1;
			f->default_parameters[1] = add_node_const(c, t->token_id);
		} else if (f->name == "all") {
			f->literal_return_type = tree->request_implicit_class_list(t, t->token_id);
			f->default_parameters.resize(1);
			auto c = tree->add_constant(TypeClassRef, t);
			c->as_int64() = (int_p)t;
			f->mandatory_params = 0;
			f->default_parameters[0] = add_node_const(c, t->token_id);
		}
	}
}

void AutoImplementer::_implement_functions_for_enum(const Class *t) {
}

}



