namespace Kaba{

enum ScriptFlag
{
	FLAG_NONE = 0,
	FLAG_CALL_BY_VALUE = 1,
	FLAG_SILENT = 2,
	FLAG_CLASS = 4,
	FLAG_PURE = 8,
	FLAG_HIDDEN = 16,
	FLAG_OVERRIDE = 32
};

void add_package(const string &name, bool used_by_default);
Class *add_type(const string &name, int size, ScriptFlag = FLAG_NONE);
Class *add_type_p(const string &name, Class *sub_type, ScriptFlag = FLAG_NONE);
Class *add_type_a(const string &name, Class *sub_type, int array_length);
int add_func(const string &name, Class *return_type, void *func, ScriptFlag = FLAG_NONE);
void func_set_inline(int index);
void func_add_param(const string &name, Class *type);
void add_class(Class *root_type);
void class_add_element(const string &name, Class *type, int offset, ScriptFlag = FLAG_NONE);
void class_add_func(const string &name, Class *return_type, void *func, ScriptFlag = FLAG_NONE);
void class_add_func_virtual(const string &name, Class *return_type, void *func, ScriptFlag = FLAG_NONE);
void class_link_vtable(void *p);
void add_const(const string &name, Class *type, void *value);
void add_ext_var(const string &name, Class *type, void *var);
void add_type_cast(int penalty, Class *source, Class *dest, const string &cmd, void *func);

#define class_set_vtable(TYPE) \
	{TYPE my_instance; \
	class_link_vtable(*(void***)&my_instance);}


};
