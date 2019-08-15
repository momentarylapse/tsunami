namespace Kaba{

enum ScriptFlag {
	FLAG_NONE = 0,
	FLAG_CALL_BY_VALUE = 1,
	FLAG_SILENT = 2,
	//FLAG_CLASS = 4,
	FLAG_PURE = 8,
	FLAG_HIDDEN = 16,
	FLAG_OVERRIDE = 32,
	FLAG_RAISES_EXCEPTIONS = 64,
	FLAG_STATIC = 128
};

class Function;

void add_package(const string &name, bool used_by_default);
const Class *add_type(const string &name, int size, ScriptFlag = FLAG_NONE);
const Class *add_type_p(const string &name, const Class *sub_type, ScriptFlag = FLAG_NONE);
const Class *add_type_a(const string &name, const Class *sub_type, int array_length);
Function *add_func(const string &name, const Class *return_type, void *func, ScriptFlag flag = FLAG_NONE);
template<class T>
Function *add_funcx(const string &name, const Class *return_type, T func, ScriptFlag flag = FLAG_NONE) {
	return add_func(name, return_type, (void*)func, flag);
}
void func_set_inline(int index);
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
void add_type_cast(int penalty, const Class *source, const Class *dest, const string &cmd, void *func);

#define class_set_vtable(TYPE) \
	{TYPE my_instance; \
	class_link_vtable(*(void***)&my_instance);}


};
