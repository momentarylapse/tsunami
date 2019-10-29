namespace Kaba{

enum class InlineID;
enum class OperatorID;

enum ScriptFlag {
	FLAG_NONE = 0,
	FLAG_CALL_BY_VALUE = 1,
	FLAG_SILENT = 2,
	//FLAG_CLASS = 4,
	FLAG_PURE = 8,
	//FLAG_HIDDEN = 16,
	FLAG_OVERRIDE = 32,
	FLAG_RAISES_EXCEPTIONS = 64,
	FLAG_STATIC = 128
};

class Function;

void add_package(const string &name, bool used_by_default);
const Class *add_type(const string &name, int size, ScriptFlag = FLAG_NONE);
const Class *add_type_p(const string &name, const Class *sub_type, ScriptFlag = FLAG_NONE);
const Class *add_type_a(const string &name, const Class *sub_type, int array_length);
const Class *add_type_d(const string &name, const Class *sub_type);
Function *add_func(const string &name, const Class *return_type, void *func, ScriptFlag flag = FLAG_NONE);
template<class T>
Function *add_funcx(const string &name, const Class *return_type, T func, ScriptFlag flag = FLAG_NONE) {
	return add_func(name, return_type, (void*)func, flag);
}
void func_set_inline(InlineID index);
void func_add_param(const string &name, const Class *type);
Class *add_class(const Class *root_type);
void class_add_element(const string &name, const Class *type, int offset, ScriptFlag flag = FLAG_NONE);
template<class T>
void class_add_elementx(const string &name, const Class *type, T p, ScriptFlag flag = FLAG_NONE) {
	class_add_element(name, type, *(int*)(void*)&p, flag);
}
Function* class_add_func(const string &name, const Class *return_type, void *func, ScriptFlag = FLAG_NONE);
template<class T>
Function* class_add_funcx(const string &name, const Class *return_type, T func, ScriptFlag flag = FLAG_NONE) {
	return class_add_func(name, return_type, mf(func), flag);
}
Function* class_add_func_virtual(const string &name, const Class *return_type, void *func, ScriptFlag = FLAG_NONE);
template<class T>
Function* class_add_func_virtualx(const string &name, const Class *return_type, T func, ScriptFlag flag = FLAG_NONE) {
	return class_add_func_virtual(name, return_type, mf(func), flag);
}
void class_link_vtable(void *p);
void class_derive_from(const Class *parent, bool increase_size, bool copy_vtable);
void add_const(const string &name, const Class *type, const void *value);
void class_add_const(const string &name, const Class *type, const void *value);
void add_ext_var(const string &name, const Class *type, void *var);
void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd);
void add_operator(OperatorID primitive_op, const Class *return_type, const Class *param_type1, const Class *param_type2, InlineID inline_index, void *func = nullptr);

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
	int n = min(num, b.num); \
	TYPE *pa = (TYPE*)data; \
	TYPE *pb = (TYPE*)b.data; \
	for (int i=0;i<n;i++) \
		*(pa ++) OP *(pb ++); \
}

// T[] += x
#define IMPLEMENT_IOP2(OP, TYPE) \
{ \
	TYPE *pa = (TYPE*)data; \
	for (int i=0;i<num;i++) \
		*(pa ++) OP x; \
}


// R[] = T[] + T[]
#define IMPLEMENT_OP(OP, TYPE, RETURN) \
{ \
	int n = min(num, b.num); \
	Array<RETURN> r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)data; \
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
	r.resize(num); \
	TYPE *pa = (TYPE*)data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<num;i++) \
		*(pr ++) = *(pa ++) OP x; \
	return r; \
}
// R[] = F(T[], T[])
#define IMPLEMENT_OPF(F, TYPE, RETURN) \
{ \
	int n = min(num, b.num); \
	Array<RETURN> r; \
	r.resize(n); \
	TYPE *pa = (TYPE*)data; \
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
	r.resize(num); \
	TYPE *pa = (TYPE*)data; \
	RETURN *pr = (RETURN*)r.data; \
	for (int i=0;i<num;i++) \
		*(pr ++) = F(*(pa ++), x); \
	return r; \
}




};
