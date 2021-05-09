
#pragma once

class DynamicArray;
class string;
class Any;

namespace kaba {
	
class Class;
class Function;

void kaba_var_assign(void *pa, const void *pb, const Class *type);
void kaba_var_init(void *p, const Class *type);
void kaba_array_clear(void *p, const Class *type);
void kaba_array_resize(void *p, const Class *type, int num);
void kaba_array_add(DynamicArray &array, void *p, const Class *type);
DynamicArray kaba_array_sort(DynamicArray &array, const Class *type, const string &by);
string var_repr(const void *p, const Class *type);
string var2str(const void *p, const Class *type);
Any kaba_dyn(const void *var, const Class *type);
DynamicArray kaba_map(Function *func, DynamicArray *a);
DynamicArray kaba_xmap(Function *func, DynamicArray *a, const Class *type);
void kaba_call0(Function *func);
void kaba_call1(Function *func, void *p1);
void kaba_call2(Function *func, void *p1, void *p2);
void kaba_call3(Function *func, void *p1, void *p2, void *p3);

string class_repr(const Class *c);
string func_repr(const Function *f);

}
