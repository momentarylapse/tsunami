namespace Script{

enum TypeFlag
{
	FLAG_NONE = 0,
	FLAG_CALL_BY_VALUE = 1,
	FLAG_SILENT = 2,
};

void set_cur_package(const string &name);
Type *add_type(const string &name, int size, TypeFlag = FLAG_NONE);
Type *add_type_p(const string &name, Type *sub_type, TypeFlag = FLAG_NONE);
Type *add_type_a(const string &name, Type *sub_type, int array_length);
int add_func(const string &name, Type *return_type, void *func, bool is_class = false);
int add_func_special(const string &name, Type *return_type, int index);
void func_add_param(const string &name, Type *type);
void add_class(Type *root_type);
void class_add_element(const string &name, Type *type, int offset);
void class_add_func(const string &name, Type *return_type, void *func);
void add_const(const string &name, Type *type, void *value);
void add_ext_var(const string &name, Type *type, void *var);

class MFDummyClass
{};

typedef void (MFDummyClass::*tmf)();
typedef char *tcpa[4];
static void *mf(tmf vmf)
{
	tcpa *cpa=(tcpa*)&vmf;
	return (*cpa)[0];
}

};
