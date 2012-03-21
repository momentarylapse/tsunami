
void set_cur_package(const char *name);
sType *add_type(const char *name, int size);
sType *add_type_p(const char *name, sType *sub_type, bool is_silent = false);
sType *add_type_a(const char *name, sType *sub_type, int array_length);
int add_func(const char *name, sType *return_type, void *func, bool is_class = false);
int add_func_special(const char *name, sType *return_type, int index);
void func_add_param(const char *name, sType *type);
void add_class(sType *root_type);
void class_add_element(const char *name, sType *type, int offset);
void class_add_func(const char *name, sType *return_type, void *func);
void add_const(const char *name, sType *type, void *value);
void add_ext_var(const char *name, sType *type, void *var);

typedef void (CFile::*tmf)();
typedef char *tcpa[4];
static void *mf(tmf vmf)
{
	tcpa *cpa=(tcpa*)&vmf;
	return (*cpa)[0];
}
