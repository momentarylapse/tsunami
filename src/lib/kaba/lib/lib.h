/*----------------------------------------------------------------------------*\
| Kaba Lib                                                                     |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include "../CompilerConfiguration.h"
#include "../syntax/Flags.h"
#include "extern.h"
#include "../../base/pointer.h"
#include <cstddef>

namespace kaba {


#define MAX_OPCODE				(2*65536)	// max. amount of opcode
#define MAX_THREAD_OPCODE		1024


//#define mem_align(x)	((x) + (4 - (x) % 4) % 4)
#define mem_align(x, n)		((((x) + (n) - 1) / (n) ) * (n))

class SyntaxTree;
class Script;
class Class;
class Value;
class Function;
class Variable;
class Constant;


void script_make_super_array(Class *t, SyntaxTree *ps = nullptr);
void script_make_dict(Class *t, SyntaxTree *ps = nullptr);





//--------------------------------------------------------------------------------------------------
// type casting

class TypeCast {
public:
	int penalty;
	const Class *source, *dest;
	Function *f;
};
extern Array<TypeCast> TypeCasts;


typedef void t_func();



void init(Abi abi = Abi::NATIVE, bool allow_std_lib = true);
void clean_up();





extern shared_array<Script> packages;


template<class T>
int element_offset(T p) {
	return *(int*)(void*)&p;
}

void add_package(const string &name, Flags = Flags::NONE);
const Class *add_type(const string &name, int size, Flags = Flags::NONE, const Class *parent = nullptr);
const Class *add_type_p(const Class *sub_type, Flags = Flags::NONE, const string &name = "");
const Class *add_type_a(const Class *sub_type, int array_length, const string &name = "");
const Class *add_type_l(const Class *sub_type, const string &name = "");
const Class *add_type_d(const Class *sub_type, const string &name = "");
const Class *add_type_f(const Class *ret_type, const Array<const Class*> &params);


Function *add_func_x(const string &name, const Class *return_type, void *func, Flags flag = Flags::NONE);
// version: regular function
template<class T>
Function *add_func(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
	return add_func_x(name, return_type, (void*)func, flag);
}

void func_set_inline(InlineID index);
void func_add_param(const string &name, const Class *type, Flags flags = Flags::CONST);
Class *add_class(const Class *root_type);
void class_add_element_x(const string &name, const Class *type, int offset, Flags flag = Flags::NONE);
template<class T>
void class_add_element(const string &name, const Class *type, T p, Flags flag = Flags::NONE) {
	// allows &Class::element
	class_add_element_x(name, type, element_offset(p), flag);
}



Function* class_add_func_x(const string &name, const Class *return_type, void *func, Flags = Flags::NONE);
// version: null
Function* class_add_func(const string &name, const Class *return_type, nullptr_t func, Flags flag = Flags::NONE);
// version: regular function
template <typename R, typename ...Args>
Function* class_add_func(const string &name, const Class *return_type, R (*func)(Args...), Flags flag = Flags::NONE) {
	return class_add_func_x(name, return_type, (void*)func, flag);
}
// version: member function
template <typename T, typename R, typename ...Args>
Function* class_add_func(const string &name, const Class *return_type, R (T::*func)(Args...), Flags flag = Flags::NONE) {
	return class_add_func_x(name, return_type, mf(func), flag);
}
// version: const member function
template <typename T, typename R, typename ...Args>
Function* class_add_func(const string &name, const Class *return_type, R (T::*func)(Args...) const, Flags flag = Flags::NONE) {
	return class_add_func_x(name, return_type, mf(func), flag);
}





Function* class_add_func_virtual_x(const string &name, const Class *return_type, void *func, Flags = Flags::NONE);
template<class T>
Function* class_add_func_virtual(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
	return class_add_func_virtual_x(name, return_type, mf(func), flag);
}


void class_link_vtable(void *p);
void class_derive_from(const Class *parent, bool increase_size, bool copy_vtable);
void add_const(const string &name, const Class *type, const void *value);
void class_add_const(const string &name, const Class *type, const void *value);
template<class T>
void add_enum(const string &name, const Class *type, T e) {
	// for enums and nullptr!
	add_const(name, type, (const void*)(int_p)e);
}


void add_ext_var(const string &name, const Class *type, void *var);
void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd);



void add_operator_x(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func = nullptr);
// version: no function
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index);
void add_operator(OperatorID primitive_op, const Class* return_type, const Class* param_type1, const Class* param_type2, InlineID inline_index, nullptr_t func);
// version: regular function
template <typename R, typename ...Args>
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, R (*func)(Args...)) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, (void*)func);
}
// version: member function
template <typename T, typename R, typename ...Args>
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, R (T::*func)(Args...)) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, mf(func));
}
// version: const member function
template <typename T, typename R, typename ...Args>
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, R (T::*func)(Args...) const) {
	add_operator_x(primitive_op, return_type, param_type1, param_type2, inline_index, mf(func));
}


#define class_set_vtable(TYPE) \
	{TYPE my_instance; \
	class_link_vtable(*(void***)&my_instance);}




#define MAKE_OP_FOR(T) \
	T op_##T##_add(T a, T b) { return a + b; } \
	T op_##T##_sub(T a, T b) { return a - b; } \
	T op_##T##_mul(T a, T b) { return a * b; } \
	T op_##T##_div(T a, T b) { return a / b; } \
	T op_##T##_neg(T a) { return - a; } \
	bool op_##T##_eq(T a, T b) { return a == b; } \
	bool op_##T##_neq(T a, T b) { return a != b; } \
	bool op_##T##_l(T a, T b) { return a < b; } \
	bool op_##T##_le(T a, T b) { return a <= b; } \
	bool op_##T##_g(T a, T b) { return a > b; } \
	bool op_##T##_ge(T a, T b) { return a >= b; }


// T[] += T[]
#define IMPLEMENT_IOP(OP, TYPE) \
{ \
	int n = ::min(this->num, b.num); \
	TYPE *pa = (TYPE*)this->data; \
	TYPE *pb = (TYPE*)b.data; \
	for (int i=0;i<n;i++) \
		*(pa ++) OP *(pb ++); \
}

// T[] += x
#define IMPLEMENT_IOP2(OP, TYPE) \
{ \
	TYPE *pa = (TYPE*)this->data; \
	for (int i=0;i<this->num;i++) \
		*(pa ++) OP x; \
}


// R[] = T[] + T[]
#define IMPLEMENT_OP(OP, TYPE, RETURN) \
{ \
	int n = ::min(this->num, b.num); \
	Array<RETURN> r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)this->data; \
	TYPE *pb = (TYPE*)b.data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<n;i++) \
		*(pr ++) = *(pa ++) OP *(pb ++); \
	return r; \
}
// R[] = T[] + x
#define IMPLEMENT_OP2(OP, TYPE, RETURN) \
{ \
	Array<RETURN> r; \
	r.resize(this->num); \
	TYPE *pa = (TYPE*)this->data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<this->num;i++) \
		*(pr ++) = *(pa ++) OP x; \
	return r; \
}
// R[] = F(T[], T[])
#define IMPLEMENT_OPF(F, TYPE, RETURN) \
{ \
	int n = ::min(this->num, b.num); \
	Array<RETURN> r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)this->data; \
	TYPE *pb = (TYPE*)b.data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<n;i++) \
		*(pr ++) = F(*(pa ++), *(pb ++)); \
	return r; \
}

// R[] = F(T[], x)
#define IMPLEMENT_OPF2(F, TYPE, RETURN) \
{ \
	Array<RETURN> r; \
	r.resize(this->num); \
	TYPE *pa = (TYPE*)this->data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<this->num;i++) \
		*(pr ++) = F(*(pa ++), x); \
	return r; \
}





};

