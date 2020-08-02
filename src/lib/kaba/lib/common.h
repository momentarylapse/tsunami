namespace Kaba{

enum class InlineID;
enum class OperatorID;

enum class Flags {
	NONE = 0,
	CALL_BY_VALUE = 1,
	SILENT = 2,
	//CLASS = 4,
	CONST = 8,
	PURE_XXX = 16,
	PURE = CONST | PURE_XXX,
	OVERRIDE = 32,
	RAISES_EXCEPTIONS = 64,
	STATIC = 128,
	EXTERN = 256,
	OUT = 512,
	VIRTUAL = 1024,
	SELFREF = 2048,

	AUTO_IMPORT = 1<<24,

	_CONST__PURE = CONST | PURE,
	_STATIC__RAISES_EXCEPTIONS = STATIC | RAISES_EXCEPTIONS,
	_STATIC__PURE = STATIC | PURE,
	_SELFREF__RAISES_EXCEPTIONS = SELFREF | RAISES_EXCEPTIONS
};
bool flags_has(Flags flags, Flags t);
Flags flags_mix(const Array<Flags> &f);

class Function;

void add_package(const string &name, Flags = Flags::NONE);
const Class *add_type(const string &name, int size, Flags = Flags::NONE, const Class *parent = nullptr);
const Class *add_type_p(const Class *sub_type, Flags = Flags::NONE, const string &name = "");
const Class *add_type_a(const Class *sub_type, int array_length, const string &name = "");
const Class *add_type_l(const Class *sub_type, const string &name = "");
const Class *add_type_d(const Class *sub_type, const string &name = "");
Function *add_func(const string &name, const Class *return_type, void *func, Flags flag = Flags::NONE);
template<class T>
Function *add_funcx(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
	return add_func(name, return_type, (void*)func, flag);
}
void func_set_inline(InlineID index);
void func_add_param(const string &name, const Class *type);
Class *add_class(const Class *root_type);
void class_add_element(const string &name, const Class *type, int offset, Flags flag = Flags::NONE);
template<class T>
void class_add_elementx(const string &name, const Class *type, T p, Flags flag = Flags::NONE) {
	class_add_element(name, type, *(int*)(void*)&p, flag);
}
Function* class_add_func(const string &name, const Class *return_type, void *func, Flags = Flags::NONE);
template<class T>
Function* class_add_funcx(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
	return class_add_func(name, return_type, mf(func), flag);
}
Function* class_add_func_virtual(const string &name, const Class *return_type, void *func, Flags = Flags::NONE);
template<class T>
Function* class_add_func_virtualx(const string &name, const Class *return_type, T func, Flags flag = Flags::NONE) {
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
