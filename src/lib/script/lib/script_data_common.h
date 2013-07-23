namespace Script{

enum TypeFlag
{
	FLAG_NONE = 0,
	FLAG_CALL_BY_VALUE = 1,
	FLAG_SILENT = 2,
};

void add_package(const string &name, bool used_by_default);
Type *add_type(const string &name, int size, TypeFlag = FLAG_NONE);
Type *add_type_p(const string &name, Type *sub_type, TypeFlag = FLAG_NONE);
Type *add_type_a(const string &name, Type *sub_type, int array_length);
int add_func(const string &name, Type *return_type, void *func, bool is_class = false);
int add_compiler_func(const string &name, Type *return_type, int index);
void func_add_param(const string &name, Type *type);
void add_class(Type *root_type);
void class_add_element(const string &name, Type *type, int offset);
void class_add_func(const string &name, Type *return_type, void *func);
void class_add_func_virtual(const string &name, Type *return_type, void *func);
void class_link_vtable(void *p);
void add_const(const string &name, Type *type, void *value);
void add_ext_var(const string &name, Type *type, void *var);
void add_type_cast(int penalty, Type *source, Type *dest, const string &cmd, void *func);

#define class_set_vtable(type) \
	{type type##Instance; \
	class_link_vtable(*(void***)&type##Instance);}


};
