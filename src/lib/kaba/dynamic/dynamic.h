
#pragma once

struct DynamicArray;
struct string;
class Any;

namespace kaba {
	
class Class;
class Function;

void var_assign(void *pa, const void *pb, const Class *type);
void var_init(void *p, const Class *type);
void array_clear(void *p, const Class *type);
void array_resize(void *p, const Class *type, int num);
void array_add(DynamicArray &array, void *p, const Class *type);
string var_repr(const void *p, const Class *type);
string var2str(const void *p, const Class *type);
Any dynify(const void *var, const Class *type);

string find_enum_label(const Class *type, int value);

// deprecated
DynamicArray array_map(void *fff, DynamicArray *a, const Class *t1, const Class *t2);

string class_repr(const Class *c);
string func_repr(const Function *f);

}
