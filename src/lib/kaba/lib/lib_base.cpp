#include "../kaba.h"
#include "lib.h"
#include "list.h"
#include "dict.h"
#include "optional.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"
#include "../dynamic/sorting.h"
#include "../template/template.h"
#include "../template/implicit.h"
#include "../template/implicit_future.h"
#include "../../os/msg.h"
#include "../../os/terminal.h"
#include "../../base/callable.h"
#include "../../base/map.h"
#include <math.h>
#include <cstdio>

#include "future.h"
#include "../../base/sort.h"

namespace kaba {

string class_name_might_need_parantheses(const Class *t);


void _cdecl kaba_cstringout(char *str) {
	os::terminal::print(str);
}

bytes _cdecl kaba_binary(char *p, int length) {
	//return bytes(p, length);
	bytes b;
	b.num = length;
	b.data = p;
	return b;
}

#if 0
void* kaba_malloc(int size) {
	msg_error("MALLOC " + i2s(size));
	return malloc(size);
}
#else
#define kaba_malloc malloc
#endif

template<class S, class T>
T _cdecl kaba_cast(S s) {
	return (T) s;
}
template<>
bool _cdecl kaba_cast(void* s) {
	return (s != nullptr);
}

int kaba_int_passthrough(int i) { return i; }


static void kaba_ping() {
	msg_write("<ping>");
}

static void kaba_int_out(int a) {
	msg_write("out: " + i2s(a));
}

static void kaba_float_out(float a) {
	msg_write("float out..." + d2h(&a, 4));
	msg_write("out: " + f2s(a, 6));
}

static float kaba_float_ret() {
	return 13.0f;
}

static void _x_call_float() {
	kaba_float_out(13);
}

static int kaba_int_ret() {
	return 2001;
}

static void kaba_xxx(int a, int b, int c, int d, int e, int f, int g, int h) {
	msg_write(format("xxx  %d %d %d %d %d %d %d %d", a, b, c, d, e, f, g, h));
}

static int extern_variable1 = 13;


MAKE_OP_FOR(int)
MAKE_OP_FOR(float)
MAKE_OP_FOR(int64)
MAKE_OP_FOR(double)

static int op_int32_mod(int a, int b) { return a % b; }
static int op_int32_shr(int a, int b) { return a >> b; }
static int op_int32_shl(int a, int b) { return a << b; }
	//static int op_int32_passthrough(int i) { return i; }
static int op_int32_and(int a, int b) { return a & b; }
static int op_int32_or(int a, int b) { return a | b; }
static int64 op_int64_and(int64 a, int64 b) { return a & b; }
static int64 op_int64_or(int64 a, int64 b) { return a | b; }
static int64 op_int64_mod(int64 a, int64 b) { return a % b; }
static int64 op_int64_shr(int64 a, int64 b) { return a >> b; }
static int64 op_int64_shl(int64 a, int64 b) { return a << b; }
static int64 op_int64_add_int(int64 a, int b) { return a + b; }



class StringList : public XList<string> {
public:
	string _cdecl join(const string &glue) {
		return implode(*this, glue);
	}
	bool __contains__(const string &s) {
		return this->find(s) >= 0;
	}
	bool __eq__(const StringList &s) {
		return *this == s;
	}
	bool __neq__(const StringList &s) {
		return *this != s;
	}
	Array<string> __add__(const Array<string> &o) {
		return *this + o;
	}
	void __adds__(const Array<string> &o) {
		append(o);
	}
};

string kaba_int_format(int i, const string &fmt) {
	try {
		if (fmt.tail(1) == "x")
			return _xf_str_<int>(fmt, i);
		return _xf_str_<int>(fmt + "d", i);
	} catch(::Exception &e) {
		return "{ERROR: " + e.message() + "}";
	}
}

string kaba_int64_format(int64 i, const string &fmt) {
	try {
		if (fmt.tail(1) == "x")
			return _xf_str_<int64>(fmt, i);
		return _xf_str_<int64>(fmt + "d", i);
	} catch(::Exception &e) {
		return "{ERROR: " + e.message() + "}";
	}
}

string kaba_i16_to_str(short w) {
	return str((int)w);
}

void kaba_i16_from_i32(short& w, int i) {
	w = i;
}

string kaba_u16_to_str(unsigned short w) {
	return str((int)w);
}

void kaba_u16_from_i32(unsigned short& w, int i) {
	w = i;
}

string kaba_float2str(float f) {
	return f2s(f, 6);
}

string kaba_f32_format(float f, const string &fmt) {
	try {
		return _xf_str_<float>(fmt + "f", f);
	} catch(::Exception &e) {
		return "{ERROR: " + e.message() + "}";
	}
}

string kaba_f64_format(double f, const string &fmt) {
	try {
		return _xf_str_<double>(fmt + "f", f);
	} catch(::Exception &e) {
		return "{ERROR: " + e.message() + "}";
	}
}

string kaba_float642str(double f) {
	return f642s(f, 6);
}

// char()
string kaba_char2str(int c) {
	return utf32_to_utf8({c});
	//return string(&c, 1);
}

string kaba_int32_hex(unsigned int i) {
	return format("0x%08x", i);
}

string kaba_int64_hex(int64 i) {
	return format("0x%016x", i);
}

auto kaba_bytes_reverse(const bytes& b) {
	return base::reverse(b);
}

/*string kaba_char_repr(char c) {
	return "'" + string(&c, 1).escape() + "'";
}*/

string kaba_uint8_to_str(uint8 c) {
	return format("0x%02x", (unsigned int)c);
}

string kaba_int8_to_str(int8 c) {
	return i2s(c);
}

/*string kaba_char_repr(char c) {
	return "'" + string(&c, 1).escape() + "'";
}*/


class KabaString : public string {
public:
	string format(const string &fmt) const {
		try {
			return _xf_str_<const string &>(fmt + "s", *this);
		} catch (::Exception &e) {
			return "{ERROR: " + e.message() + "}";
		}
	}

	bool contains_s(const string &s) const {
		return find(s, 0) >= 0;
	}

	base::optional<int> _find(const string &s, int pos0) const {
		int r = find(s, pos0);
		if (r < 0)
			return base::None;
		return r;
	}

	bytes encode() const {
		return *(bytes*)this;
	}
	static string decode(const bytes& b) {
		return string(b);
	}
};

class KabaBytes : public bytes {
public:
	string utf8() const {
		return *(string*)this;
	}

	base::optional<int> _find(const bytes &s, int pos0) const {
		int r = reinterpret_cast<const string*>(this)->find(s, pos0);
		if (r < 0)
			return base::None;
		return r;
	}
};


class _VirtualBase : public VirtualBase {
public:
	void __init__() {
		new(this) _VirtualBase();
	}
};



class KabaCallableBase : public Callable<void()> {
public:
	void __init__() {
		new(this) KabaCallableBase();
	}
};

Array<int> enum_all(const Class *e) {
	Array<int> r;
	for (auto c: weak(e->constants))
		if (c->type == e)
			r.add(c->as_int());
	return r;
}

bool pointer_equal(const void* a, const void* b) {
	return a == b;
}

bool pointer_not_equal(const void* a, const void* b) {
	return a != b;
}



void SIAddXCommands(Context *c) {

	add_func("@sorted", common_types.dynamic_array, &array_sort, Flags::Static | Flags::RaisesExceptions);
		func_add_param("list", common_types.pointer);
		func_add_param("class", common_types.class_ref);
		func_add_param("by", common_types.string);
	add_func("@var2str", common_types.string, &var2str, Flags::Static | Flags::RaisesExceptions);
		func_add_param("var", common_types.pointer);
		func_add_param("class", common_types.class_ref);
	add_func("@var_repr", common_types.string, &var_repr, Flags::Static | Flags::RaisesExceptions);
		func_add_param("var", common_types.pointer);
		func_add_param("class", common_types.class_ref);
	add_func("@dyn", common_types.any, &dynify, Flags::Static | Flags::RaisesExceptions);
		func_add_param("var", common_types.pointer);
		func_add_param("class", common_types.class_ref);
}

void SIAddPackageBase(Context *c) {
	add_internal_package(c, "base", "1", Flags::AutoImport);

	// internal
	common_types.unknown		= add_type  ("@unknown", 0); // should not appear anywhere....or else we're screwed up!
	common_types.reg128			= add_type_simple  ("@reg128", 16, 16, Flags::ForceCallByValue);
	common_types.reg64			= add_type_simple  ("@reg64", 8, 8, Flags::ForceCallByValue);
	common_types.reg32			= add_type_simple  ("@reg32", 4, 4, Flags::ForceCallByValue);
	common_types.reg16			= add_type_simple  ("@reg16", 2, 2, Flags::ForceCallByValue);
	common_types.reg8			= add_type_simple  ("@reg8", 1, 1, Flags::ForceCallByValue);
	common_types.object			= add_type  ("Object", sizeof(VirtualBase)); // base for most virtual classes
	common_types.dynamic		= add_type  ("@dynamic", 0);

	// "real"
	common_types._void			= add_type_simple  ("void", 0, 1, Flags::ForceCallByValue);
	common_types._bool			= add_type_simple  ("bool", sizeof(bool), 1, Flags::ForceCallByValue);
	common_types.i8			= add_type_simple  ("i8", 1, 1, Flags::ForceCallByValue);
	common_types.u8			= add_type_simple  ("u8", 1, 1, Flags::ForceCallByValue);
	common_types.i16			= add_type_simple  ("i16", 2, 2, Flags::ForceCallByValue);
	common_types.u16			= add_type_simple  ("u16", 2, 2, Flags::ForceCallByValue);
	common_types.i32			= add_type_simple  ("i32", sizeof(int32), 4, Flags::ForceCallByValue);
	common_types.i64			= add_type_simple  ("i64", sizeof(int64), 8, Flags::ForceCallByValue);
	common_types.f32			= add_type_simple  ("f32", sizeof(float), 4, Flags::ForceCallByValue);
	common_types.f64			= add_type_simple  ("f64", sizeof(double), 8, Flags::ForceCallByValue);
	common_types.dynamic_array	= add_type  ("@DynamicArray", config.target.dynamic_array_size);
	common_types.dict_base		= add_type  ("@DictBase",   config.target.dynamic_array_size);
	common_types.shared_pointer	= add_type  ("@SharedPointer", config.target.pointer_size);
	common_types.callable_base	= add_type  ("@CallableBase", sizeof(Callable<void()>));

	common_types.no_value_error    = add_type  ("NoValueError", sizeof(KabaException));

	// type aliases
	cur_package_module->tree->base_class->type_aliases.add({"int", common_types.i32});
	cur_package_module->tree->base_class->type_aliases.add({"float", common_types.f32});



	common_types.raw_t = add_class_template("ptr", {"T"}, new TemplateClassInstantiatorPointerRaw);
	common_types.xfer_t = add_class_template("xfer", {"T"}, new TemplateClassInstantiatorPointerXfer);
	common_types.shared_t = add_class_template("shared", {"T"}, new TemplateClassInstantiatorPointerShared);
	common_types.shared_not_null_t = add_class_template("shared!", {"T"}, new TemplateClassInstantiatorPointerSharedNotNull);
	common_types.owned_t = add_class_template("owned", {"T"}, new TemplateClassInstantiatorPointerOwned);
	common_types.owned_not_null_t = add_class_template("owned!", {"T"}, new TemplateClassInstantiatorPointerOwnedNotNull);
	common_types.alias_t = add_class_template("@alias", {"T"}, new TemplateClassInstantiatorPointerAlias);
	common_types.reference_t = add_class_template("ref", {"T"}, new TemplateClassInstantiatorReference);
	common_types.array_t = add_class_template("@Array", {"T"}, new TemplateClassInstantiatorArray);
	common_types.list_t = add_class_template("@List", {"T"}, new TemplateClassInstantiatorList);
	common_types.dict_t = add_class_template("@Dict", {"T"}, new TemplateClassInstantiatorDict);
	common_types.callable_fp_t = add_class_template("@CallableFP", {"T..."}, new TemplateClassInstantiatorCallableFP);
	common_types.callable_bind_t = add_class_template("@Bind", {"T..."}, new TemplateClassInstantiatorCallableBind);
	common_types.optional_t = add_class_template("@Optional", {"T"}, new TemplateClassInstantiatorOptional);
	common_types.product_t = add_class_template("@Product", {"T"}, new TemplateClassInstantiatorProduct);
	common_types.struct_t = add_class_template("@Struct", {"T"}, nullptr);
	common_types.enum_t = add_class_template("@Enum", {"T"}, new TemplateClassInstantiatorEnum);
	common_types.interface_t = add_class_template("@Interface", {"T"}, nullptr);
	common_types.namespace_t = add_class_template("@Namespace", {"T"}, nullptr);


	add_class(common_types.object);
		class_add_func(Identifier::func::Init, common_types._void, &_VirtualBase::__init__, Flags::Mutable);
		class_add_func_virtual(Identifier::func::Delete, common_types._void, &VirtualBase::__delete__, Flags::Mutable);
		class_set_vtable(VirtualBase);

	add_class(common_types.dynamic_array);
		class_add_element("num", common_types.i32, config.target.pointer_size);
		class_add_func("swap", common_types._void, &DynamicArray::simple_swap, Flags::Mutable);
			func_add_param("i1", common_types.i32);
			func_add_param("i2", common_types.i32);
		class_add_func(Identifier::func::Subarray, common_types.dynamic_array, &DynamicArray::ref_subarray, Flags::Ref);
			func_add_param("start", common_types.i32);
			func_add_param("end", common_types.i32);
		// low level operations
		class_add_func("__mem_init__", common_types._void, &DynamicArray::init, Flags::Mutable);
			func_add_param("element_size", common_types.i32);
		class_add_func("__mem_clear__", common_types._void, &DynamicArray::simple_clear, Flags::Mutable);
		class_add_func("__mem_forget__", common_types._void, &DynamicArray::forget, Flags::Mutable);
		class_add_func("__mem_resize__", common_types._void, &DynamicArray::simple_resize, Flags::Mutable);
			func_add_param("size", common_types.i32);
		class_add_func("__mem_remove__", common_types._void, &DynamicArray::delete_single, Flags::Mutable);
			func_add_param("index", common_types.i32);

	add_class(common_types.dict_base);
		class_add_element("num", common_types.i32, config.target.pointer_size);
		// low level operations
		class_add_func("__mem_init__", common_types._void, &DynamicArray::init, Flags::Mutable);
			func_add_param("element_size", common_types.i32);
		class_add_func("__mem_clear__", common_types._void, &DynamicArray::simple_clear, Flags::Mutable);
		class_add_func("__mem_forget__", common_types._void, &DynamicArray::forget, Flags::Mutable);
		class_add_func("__mem_resize__", common_types._void, &DynamicArray::simple_resize, Flags::Mutable);
			func_add_param("size", common_types.i32);
		class_add_func("__mem_remove__", common_types._void, &DynamicArray::delete_single, Flags::Mutable);
			func_add_param("index", common_types.i32);

	add_class(common_types.shared_pointer);
		class_add_func(Identifier::func::Init, common_types._void, nullptr, Flags::Mutable);
			func_set_inline(InlineID::SharedPointerInit);


	common_types.object_p			= add_type_p_raw(common_types.object);


	// derived   (must be defined after the primitive types and the bases!)
	common_types.pointer     = add_type_p_raw(common_types._void); // substitute for all raw pointer types
	common_types.reference   = add_type_ref(common_types._void); // substitute for all reference types
	common_types.none        = add_type_p_raw(common_types._void); // type of <nil>
	const_cast<Class*>(common_types.none)->name = "None";
	common_types.pointer_list = add_type_list(common_types.pointer);
	common_types.bool_list    = add_type_list(common_types._bool);
	common_types.i32_p      = add_type_p_raw(common_types.i32);
	common_types.i32_optional = add_type_optional(common_types.i32);
	common_types.f32_optional = add_type_optional(common_types.f32);
	common_types.i32_list   = add_type_list(common_types.i32);
	common_types.f32_p    = add_type_p_raw(common_types.f32);
	common_types.f32_list = add_type_list(common_types.f32);
	common_types.f64_list = add_type_list(common_types.f64);
	common_types.bytes      = add_type_list(common_types.u8);
	common_types.cstring     = add_type_array(common_types.u8, 256);
	capture_implicit_type(common_types.cstring, "cstring"); // cstring := u8[256]
	common_types.string      = add_type_list(common_types.u8);
	capture_implicit_type(common_types.string, "string"); // string := u8[]
	common_types.string_auto_cast = add_type("<string-auto-cast>", config.target.dynamic_array_size);	// string := i8[]
	common_types.string_list  = add_type_list(common_types.string);
	capture_implicit_type(common_types.bytes, "bytes"); // bytes := u8[]

	common_types.i32_dict   = add_type_dict(common_types.i32);
	common_types.f32_dict = add_type_dict(common_types.f32);
	common_types.string_dict  = add_type_dict(common_types.string);

	common_types.exception		= add_type  ("Exception", sizeof(KabaException));
	common_types.exception_xfer	= add_type_p_xfer(common_types.exception);

	lib_create_list<void*>(common_types.pointer_list);
	lib_create_list<bool>(common_types.bool_list);
	lib_create_list<int>(common_types.i32_list);
	lib_create_list<float>(common_types.f32_list);
	lib_create_list<double>(common_types.f64_list);
	lib_create_list<char>(common_types.string);
	lib_create_list<uint8>(common_types.bytes);
	lib_create_list<string>(common_types.string_list);


	lib_create_optional<int>(common_types.i32_optional);
	lib_create_optional<float>(common_types.f32_optional);


	auto TypeStringP = add_type_p_raw(common_types.string);

	lib_create_dict<int>(common_types.i32_dict, common_types.i32_p);
	lib_create_dict<float>(common_types.f32_dict, common_types.f32_p);
	lib_create_dict<string>(common_types.string_dict, TypeStringP);



	add_class(common_types.callable_base);
		class_add_element("_fp", common_types.pointer, &KabaCallableBase::fp);
		class_add_element("_pp", common_types.pointer, &KabaCallableBase::pp);
		//class_add_func(Identifier::Func::INIT, common_types._void, &KabaCallableBase::__init__);
		class_add_func(Identifier::func::Assign, common_types._void, nullptr, Flags::Mutable);
			func_set_inline(InlineID::ChunkAssign);
		class_add_func_virtual("call", common_types._void, &KabaCallableBase::operator());
	


	add_func("p2b", common_types._bool, &kaba_cast<void*,bool>, Flags::Static | Flags::Pure);
		func_set_inline(InlineID::PointerToBool);
		func_add_param("p", common_types.pointer);


	add_class(common_types.pointer);
		class_add_func(Identifier::func::Str, common_types.string, &p2s, Flags::Pure);
		add_operator(OperatorID::Assign, common_types._void, common_types.pointer, common_types.pointer, InlineID::PointerAssign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.pointer, common_types.pointer, InlineID::PointerEqual, &pointer_equal);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.pointer, common_types.pointer, InlineID::PointerNotEqual, &pointer_not_equal);


	// TODO define in template by exact X&:=X&
	// then allow right side casting by standard rules
	add_class(common_types.reference);
		add_operator(OperatorID::RefAssign, common_types._void, common_types.reference, common_types.reference, InlineID::PointerAssign);


	add_class(common_types._bool);
		class_add_func(Identifier::func::Str, common_types.string, &b2s, Flags::Pure);
		add_operator(OperatorID::Assign, common_types._void, common_types._bool, common_types._bool, InlineID::BoolAssign);
		add_operator(OperatorID::Equal, common_types._bool, common_types._bool, common_types._bool, InlineID::BoolEqual);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types._bool, common_types._bool, InlineID::BoolNotEqual);
		add_operator(OperatorID::And, common_types._bool, common_types._bool, common_types._bool, InlineID::BoolAnd);
		add_operator(OperatorID::Or, common_types._bool, common_types._bool, common_types._bool, InlineID::BoolOr);
		add_operator(OperatorID::Negate, common_types._bool, nullptr, common_types._bool, InlineID::BoolNot);


	add_class(common_types.u8);
		class_add_func(Identifier::func::Str, common_types.string, &kaba_uint8_to_str, Flags::Pure);
		//class_add_func(Identifier::Func::REPR, common_types.string, &kaba_char_repr, Flags::PURE);
		class_add_func("__i32__", common_types.i32, &kaba_cast<uint8,int>, Flags::Pure);
			func_set_inline(InlineID::Uint8ToInt32);
		class_add_func("__i8__", common_types.i8, &kaba_cast<uint8,int8>, Flags::Pure);
			func_set_inline(InlineID::Passthrough);
		add_operator(OperatorID::Assign, common_types._void, common_types.u8, common_types.u8, InlineID::Int8Assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.u8, common_types.u8, InlineID::Int8Equal);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.u8, common_types.u8, InlineID::Int8NotEqual);
		add_operator(OperatorID::Greater, common_types._bool, common_types.u8, common_types.u8, InlineID::Uint8Greater);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.u8, common_types.u8, InlineID::Uint8GreaterEqual);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.u8, common_types.u8, InlineID::Uint8Smaller);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.u8, common_types.u8, InlineID::Uint8SmallerEqual);
		add_operator(OperatorID::Add, common_types.u8, common_types.u8, common_types.u8, InlineID::Int8Add);
		add_operator(OperatorID::SubtractAssign, common_types.u8, common_types.u8, common_types.u8, InlineID::Int8SubtractAssign);
		add_operator(OperatorID::AddAssign, common_types.u8, common_types.u8, common_types.u8, InlineID::Int8AddAssign);
		add_operator(OperatorID::Subtract, common_types.u8, common_types.u8, common_types.u8, InlineID::Int8Subtract);
		add_operator(OperatorID::BitAnd, common_types.u8, common_types.u8, common_types.u8, InlineID::Int8BitAnd);
		add_operator(OperatorID::BitOr, common_types.u8, common_types.u8, common_types.u8, InlineID::Int8BitOr);
		add_operator(OperatorID::Negative, common_types.u8, nullptr, common_types.u8, InlineID::Int8Negative);


	add_class(common_types.i8);
		class_add_func(Identifier::func::Str, common_types.string, &kaba_int8_to_str, Flags::Pure);
		//class_add_func(Identifier::Func::REPR, common_types.string, &kaba_char_repr, Flags::PURE);
		class_add_func("__i32__", common_types.i32, &kaba_cast<char,int>, Flags::Pure);
			func_set_inline(InlineID::Int8ToInt32);
		class_add_func("__u8__", common_types.i8, &kaba_cast<int8,uint8>, Flags::Pure);
			func_set_inline(InlineID::Passthrough);
		add_operator(OperatorID::Assign, common_types._void, common_types.i8, common_types.i8, InlineID::Int8Assign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.i8, common_types.i8, InlineID::Int8Equal);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.i8, common_types.i8, InlineID::Int8NotEqual);
		add_operator(OperatorID::Greater, common_types._bool, common_types.i8, common_types.i8, InlineID::Int8Greater);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.i8, common_types.i8, InlineID::Int8GreaterEqual);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.i8, common_types.i8, InlineID::Int8Smaller);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.i8, common_types.i8, InlineID::Int8SmallerEqual);
		add_operator(OperatorID::Add, common_types.i8, common_types.i8, common_types.i8, InlineID::Int8Add);
		add_operator(OperatorID::SubtractAssign, common_types.i8, common_types.i8, common_types.i8, InlineID::Int8SubtractAssign);
		add_operator(OperatorID::AddAssign, common_types.i8, common_types.i8, common_types.i8, InlineID::Int8AddAssign);
		add_operator(OperatorID::Subtract, common_types.i8, common_types.i8, common_types.i8, InlineID::Int8Subtract);
		add_operator(OperatorID::BitAnd, common_types.i8, common_types.i8, common_types.i8, InlineID::Int8BitAnd);
		add_operator(OperatorID::BitOr, common_types.i8, common_types.i8, common_types.i8, InlineID::Int8BitOr);
		add_operator(OperatorID::Negative, common_types.i8, nullptr, common_types.i8, InlineID::Int8Negative);


	add_class(common_types.i16);
		class_add_element("low", common_types.u8, 0);
		class_add_element("high", common_types.u8, 1);
		class_add_func(Identifier::func::Str, common_types.string, &kaba_i16_to_str, Flags::Pure);
		class_add_func("__i32__", common_types.i32, &kaba_cast<short,int>, Flags::Pure);
		//	func_set_inline(InlineID::INT16_TO_INT32);
		add_operator(OperatorID::Assign, common_types._void, common_types.i16, common_types.i16, InlineID::ChunkAssign);
		//add_operator(OperatorID::ASSIGN, common_types._void, common_types.i16, common_types.i32, InlineID::INT16_ASSIGN_INT32);
		class_add_func("__assign__", common_types._void, &kaba_i16_from_i32, Flags::Mutable);
			func_add_param("o", common_types.i32);


	add_class(common_types.u16);
		class_add_element("low", common_types.u8, 0);
		class_add_element("high", common_types.u8, 1);
		class_add_func(Identifier::func::Str, common_types.string, &kaba_u16_to_str, Flags::Pure);
		class_add_func("__i32__", common_types.i32, &kaba_cast<unsigned short,int>, Flags::Pure);
		//	func_set_inline(InlineID::INT16_TO_INT32);
		add_operator(OperatorID::Assign, common_types._void, common_types.u16, common_types.u16, InlineID::ChunkAssign);
		//add_operator(OperatorID::ASSIGN, common_types._void, common_types.i16, common_types.i32, InlineID::INT16_ASSIGN_INT32);
		class_add_func("__assign__", common_types._void, &kaba_u16_from_i32, Flags::Mutable);
			func_add_param("o", common_types.i32);

	add_class(common_types.i32);
		class_add_func(Identifier::func::Str, common_types.string, &i2s, Flags::Pure);
		class_add_func(Identifier::func::Format, common_types.string, &kaba_int_format, Flags::Pure);
			func_add_param("fmt", common_types.string);
		class_add_func("__f32__", common_types.f32, &kaba_cast<int,float>, Flags::Pure);
			func_set_inline(InlineID::Int32ToFloat32);
		class_add_func("__f64__", common_types.f64, &kaba_cast<int,double>, Flags::Pure);
		class_add_func("__i8__", common_types.i8, &kaba_cast<int,int8>, Flags::Pure);
			func_set_inline(InlineID::Int32ToInt8);
		class_add_func("__u8__", common_types.u8, &kaba_cast<int,uint8>, Flags::Pure);
			func_set_inline(InlineID::Int32ToUint8);
		class_add_func("__i64__", common_types.i64, &kaba_cast<int,int64>, Flags::Pure);
			func_set_inline(InlineID::Int32ToInt64);
		add_operator(OperatorID::Assign, common_types._void, common_types.i32, common_types.i32, InlineID::Int32Assign);
		add_operator(OperatorID::Add, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32Add, &op_int_add);
		add_operator(OperatorID::Subtract, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32Subtract, &op_int_sub);
		add_operator(OperatorID::Multiply, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32Multiply, &op_int_mul);
		add_operator(OperatorID::Divide, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32Divide, &op_int_div);
		add_operator(OperatorID::Exponent, common_types.i32, common_types.i32, common_types.i32, InlineID::None, &xop_exp<int>);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.i32, common_types.i32, InlineID::Int32AddAssign);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.i32, common_types.i32, InlineID::Int32SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.i32, common_types.i32, InlineID::Int32MultiplyAssign);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.i32, common_types.i32, InlineID::Int32DivideAssign);
		add_operator(OperatorID::Modulo, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32Modulo, &op_int32_mod);
		add_operator(OperatorID::Equal, common_types._bool, common_types.i32, common_types.i32, InlineID::Int32Equal, &op_int_eq);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.i32, common_types.i32, InlineID::Int32NotEqual, &op_int_neq);
		add_operator(OperatorID::Greater, common_types._bool, common_types.i32, common_types.i32, InlineID::Int32Greater, &op_int_g);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.i32, common_types.i32, InlineID::Int32GreaterEqual, &op_int_ge);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.i32, common_types.i32, InlineID::Int32Smaller, &op_int_l);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.i32, common_types.i32, InlineID::Int32SmallerEqual, &op_int_le);
		add_operator(OperatorID::BitAnd, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32BitAnd, &op_int32_and);
		add_operator(OperatorID::BitOr, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32BitOr, &op_int32_or);
		add_operator(OperatorID::ShiftRight, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32ShiftRight, &op_int32_shr);
		add_operator(OperatorID::ShiftLeft, common_types.i32, common_types.i32, common_types.i32, InlineID::Int32ShiftLeft, &op_int32_shl);
		add_operator(OperatorID::Negative, common_types.i32, nullptr, common_types.i32, InlineID::Int32Negative, &op_int_neg);
		add_operator(OperatorID::Increase, common_types._void, common_types.i32, nullptr, InlineID::Int32Increase);
		add_operator(OperatorID::Decrease, common_types._void, common_types.i32, nullptr, InlineID::Int32Decrease);

	add_class(common_types.i64);
		class_add_func(Identifier::func::Str, common_types.string, &i642s, Flags::Pure);
		class_add_func(Identifier::func::Format, common_types.string, &kaba_int64_format, Flags::Pure);
			func_add_param("fmt", common_types.string);
		class_add_func("__i32__", common_types.i32, &kaba_cast<int64,int>, Flags::Pure);
			func_set_inline(InlineID::Int64ToInt32);
		add_operator(OperatorID::Assign, common_types._void, common_types.i64, common_types.i64, InlineID::Int64Assign);
		add_operator(OperatorID::Add, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64Add, &op_int64_add);
		add_operator(OperatorID::Add, common_types.i64, common_types.i64, common_types.i32, InlineID::Int64AddInt32, &op_int64_add_int); // needed by internal address calculations!
		add_operator(OperatorID::Subtract, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64Subtract, &op_int64_sub);
		add_operator(OperatorID::Multiply, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64Multiply, &op_int64_mul);
		add_operator(OperatorID::Divide, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64Divide, &op_int64_div);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.i64, common_types.i64, InlineID::Int64AddAssign);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.i64, common_types.i64, InlineID::Int64SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.i64, common_types.i64, InlineID::Int64MultiplyAssign);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.i64, common_types.i64, InlineID::Int64DivideAssign);
		add_operator(OperatorID::Modulo, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64Modulo, &op_int64_mod);
		add_operator(OperatorID::Equal, common_types._bool, common_types.i64, common_types.i64, InlineID::Int64Equal, &op_int64_eq);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.i64, common_types.i64, InlineID::Int64NotEqual, &op_int64_neq);
		add_operator(OperatorID::Greater, common_types._bool, common_types.i64, common_types.i64, InlineID::Int64Greater, &op_int64_g);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.i64, common_types.i64, InlineID::Int64GreaterEqual, &op_int64_ge);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.i64, common_types.i64, InlineID::Int64Smaller, &op_int64_l);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.i64, common_types.i64, InlineID::Int64SmallerEqual, &op_int64_le);
		add_operator(OperatorID::BitAnd, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64BitAnd, &op_int64_and);
		add_operator(OperatorID::BitOr, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64BitOr, &op_int64_or);
		add_operator(OperatorID::ShiftRight, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64ShiftRight, &op_int64_shr);
		add_operator(OperatorID::ShiftLeft, common_types.i64, common_types.i64, common_types.i64, InlineID::Int64ShiftLeft, &op_int64_shl);
		add_operator(OperatorID::Negative, common_types.i64, nullptr, common_types.i64, InlineID::Int64Negative, &op_int64_neg);
		add_operator(OperatorID::Increase, common_types._void, common_types.i64, nullptr, InlineID::Int64Increase);
		add_operator(OperatorID::Decrease, common_types._void, common_types.i64, nullptr, InlineID::Int64Decrease);

	add_class(common_types.f32);
		class_add_func(Identifier::func::Str, common_types.string, &kaba_float2str, Flags::Pure);
		class_add_func("str2", common_types.string, &f2s, Flags::Pure);
			func_add_param("decimals", common_types.i32);
		class_add_func(Identifier::func::Format, common_types.string, &kaba_f32_format, Flags::Pure);
			func_add_param("fmt", common_types.string);
		class_add_func("__i32__", common_types.i32, &kaba_cast<float,int>, Flags::Pure);
			func_set_inline(InlineID::FloatToInt32);    // sometimes causes floating point exceptions...
		class_add_func("__f64__", common_types.f64, &kaba_cast<float,double>, Flags::Pure);
			func_set_inline(InlineID::Float32ToFloat64);
		add_operator(OperatorID::Assign, common_types._void, common_types.f32, common_types.f32, InlineID::Float32Assign);
		add_operator(OperatorID::Add, common_types.f32, common_types.f32, common_types.f32, InlineID::Float32Add, &op_float_add);
		add_operator(OperatorID::Subtract, common_types.f32, common_types.f32, common_types.f32, InlineID::Float32Subtarct, &op_float_sub);
		add_operator(OperatorID::Multiply, common_types.f32, common_types.f32, common_types.f32, InlineID::Float32Multiply, &op_float_mul);
		add_operator(OperatorID::Divide, common_types.f32, common_types.f32, common_types.f32, InlineID::Float32Divide, &op_float_div);
		add_operator(OperatorID::Exponent, common_types.f32, common_types.f32, common_types.f32, InlineID::None, &xop_exp<float>);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.f32, common_types.f32, InlineID::Float32AddAssign);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.f32, common_types.f32, InlineID::Float32SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.f32, common_types.f32, InlineID::Float32MultiplyAssign);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.f32, common_types.f32, InlineID::Float32DivideAssign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.f32, common_types.f32, InlineID::Float32Equal, &op_float_eq);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.f32, common_types.f32, InlineID::Float32NotEqual, &op_float_neq);
		add_operator(OperatorID::Greater, common_types._bool, common_types.f32, common_types.f32, InlineID::Float32Greater, &op_float_g);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.f32, common_types.f32, InlineID::Float32GreaterEqual, &op_float_ge);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.f32, common_types.f32, InlineID::Float32Smaller, &op_float_l);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.f32, common_types.f32, InlineID::Float32SmallerEqual, &op_float_le);
		add_operator(OperatorID::Negative, common_types.f32, nullptr, common_types.f32, InlineID::Float32Negative, &op_float_neg);


	add_class(common_types.f64);
		class_add_func(Identifier::func::Str, common_types.string, &kaba_float642str, Flags::Pure);
		class_add_func(Identifier::func::Format, common_types.string, &kaba_f64_format, Flags::Pure);
			func_add_param("fmt", common_types.string);
		class_add_func("__f32__", common_types.f32, &kaba_cast<double,float>, Flags::Pure);
			func_set_inline(InlineID::Float64ToFloat32);
		class_add_func("__i32__", common_types.i32, &kaba_cast<double,int>, Flags::Pure);
		add_operator(OperatorID::Assign, common_types._void, common_types.f64, common_types.f64, InlineID::Float64Assign);
		add_operator(OperatorID::Add, common_types.f64, common_types.f64, common_types.f64, InlineID::Float64Add, &op_double_add);
		add_operator(OperatorID::Subtract, common_types.f64, common_types.f64, common_types.f64, InlineID::Float64Subtract, &op_double_sub);
		add_operator(OperatorID::Multiply, common_types.f64, common_types.f64, common_types.f64, InlineID::Float64Multiply, &op_double_mul);
		add_operator(OperatorID::Divide, common_types.f64, common_types.f64, common_types.f64, InlineID::Float64Divide, &op_double_div);
		add_operator(OperatorID::Exponent, common_types.f64, common_types.f64, common_types.f64, InlineID::None, &xop_exp<double>);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.f64, common_types.f64, InlineID::Float64AddAssign);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.f64, common_types.f64, InlineID::Float64SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.f64, common_types.f64, InlineID::Float64MultiplyAssign);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.f64, common_types.f64, InlineID::Float64DivideAssign);
		add_operator(OperatorID::Equal, common_types._bool, common_types.f64, common_types.f64, InlineID::Float64Equal, &op_double_eq);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.f64, common_types.f64, InlineID::Float64NotEqual, &op_double_neq);
		add_operator(OperatorID::Greater, common_types._bool, common_types.f64, common_types.f64, InlineID::Float64Greater, &op_double_g);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.f64, common_types.f64, InlineID::Float64GreaterEqual, &op_double_ge);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.f64, common_types.f64, InlineID::Float64Smaller, &op_double_l);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.f64, common_types.f64, InlineID::Float64SmallerEqual, &op_double_le);
//		add_operator(OperatorID::Negative, common_types.f64, nullptr, common_types.f64, InlineID::Float64Negative, &op_double_neg);
		add_operator(OperatorID::Negative, common_types.f64, nullptr, common_types.f64, InlineID::None, &op_double_neg);


	add_class(common_types.string);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.string, common_types.string, InlineID::None, &string::operator+=);
		add_operator(OperatorID::Add, common_types.string, common_types.string, common_types.string, InlineID::None, &string::operator+);
		add_operator(OperatorID::Equal, common_types._bool, common_types.string, common_types.string, InlineID::None, &string::operator==);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.string, common_types.string, InlineID::None, &string::operator!=);
		add_operator(OperatorID::Smaller, common_types._bool, common_types.string, common_types.string, InlineID::None, &string::operator<);
		add_operator(OperatorID::SmallerEqual, common_types._bool, common_types.string, common_types.string, InlineID::None, &string::operator<=);
		add_operator(OperatorID::Greater, common_types._bool, common_types.string, common_types.string, InlineID::None, &string::operator>);
		add_operator(OperatorID::GreaterEqual, common_types._bool, common_types.string, common_types.string, InlineID::None, &string::operator>=);
		class_add_func("head", common_types.string, &string::head, Flags::Pure);
			func_add_param("size", common_types.i32);
		class_add_func("tail", common_types.string, &string::tail, Flags::Pure);
			func_add_param("size", common_types.i32);
		class_add_func("find", common_types.i32_optional, &KabaString::_find, Flags::Pure);
			func_add_param("str", common_types.string);
			func_add_param_def("start", common_types.i32, 0);
		class_add_func("compare", common_types.i32, &string::compare, Flags::Pure);
			func_add_param("str", common_types.string);
		class_add_func("icompare", common_types.i32, &string::icompare, Flags::Pure);
			func_add_param("str", common_types.string);
		class_add_func("replace", common_types.string, &string::replace, Flags::Pure);
			func_add_param("sub", common_types.string);
			func_add_param("by", common_types.string);
		class_add_func("explode", common_types.string_list, &string::explode, Flags::Pure);
			func_add_param("str", common_types.string);
		class_add_func("parse_tokens", common_types.string_list, &string::parse_tokens, Flags::Pure);
			func_add_param("splitters", common_types.string);
		class_add_func("repeat", common_types.string, &string::repeat, Flags::Pure);
			func_add_param("n", common_types.i32);
		class_add_func("lower", common_types.string, &string::lower, Flags::Pure);
		class_add_func("upper", common_types.string, &string::upper, Flags::Pure);
		class_add_func("reverse", common_types.string, &string::reverse, Flags::Pure);
		class_add_func("hash", common_types.i32, &string::hash, Flags::Pure);
		class_add_func("hex", common_types.string, &string::hex, Flags::Pure);
		class_add_func("unhex", common_types.bytes, &string::unhex, Flags::Pure);
		class_add_func("match", common_types._bool, &string::match, Flags::Pure);
			func_add_param("glob", common_types.string);
		class_add_func("__i32__", common_types.i32, &string::_int, Flags::Pure);
		class_add_func("__i64__", common_types.i64, &string::i64, Flags::Pure);
		class_add_func("__f32__", common_types.f32, &string::_float, Flags::Pure);
		class_add_func("__f64__", common_types.f64, &string::f64, Flags::Pure);
		class_add_func("trim", common_types.string, &string::trim, Flags::Pure);
		class_add_func("escape", common_types.string, &string::escape, Flags::Pure);
		class_add_func("unescape", common_types.string, &string::unescape, Flags::Pure);
		class_add_func("utf8_to_utf32", common_types.i32_list, &string::utf8_to_utf32, Flags::Pure);
		class_add_func("utf8_length", common_types.i32, &string::utf8len, Flags::Pure);
		class_add_func("encode", common_types.bytes, &KabaString::encode, Flags::Pure);
		class_add_func("decode", common_types.string, &KabaString::decode, Flags::Pure | Flags::Static);
			func_add_param("b", common_types.bytes);
		class_add_func(Identifier::func::Repr, common_types.string, &string::repr, Flags::Pure);
		class_add_func(Identifier::func::Format, common_types.string, &KabaString::format, Flags::Pure);
			func_add_param("fmt", common_types.string);
		class_add_func(Identifier::func::Contains, common_types._bool, &KabaString::contains_s, Flags::Pure);
			func_add_param("s", common_types.string);


	add_class(common_types.bytes);
		add_operator(OperatorID::Equal, common_types._bool, common_types.bytes, common_types.bytes, InlineID::None, &bytes::operator==);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.bytes, common_types.bytes, InlineID::None, &bytes::operator!=);
		class_add_func("reverse", common_types.bytes, &kaba_bytes_reverse, Flags::Pure);
		class_add_func("hash", common_types.i32, &bytes::hash, Flags::Pure);
		class_add_func("md5", common_types.string, &bytes::md5, Flags::Pure);
		class_add_func("hex", common_types.string, &bytes::hex, Flags::Pure);
		class_add_func("utf8", common_types.string, &KabaBytes::utf8, Flags::Pure);
		class_add_func("find", common_types.i32_optional, &KabaBytes::_find, Flags::Pure);
			func_add_param("str", common_types.bytes);
			func_add_param_def("start", common_types.i32, 0);
		//class_add_func(Identifier::Func::REPR, common_types.string, &bytes::hex, Flags::PURE);
	//	class_add_func(Identifier::Func::FORMAT, common_types.string, &KabaString::format, Flags::PURE);
	//		func_add_param("fmt", common_types.string);


	add_class(common_types.bool_list);
		class_add_func(Identifier::func::Str, common_types.string, &BoolList::str, Flags::Pure);
		class_add_func("all", common_types._bool, &BoolList::all, Flags::Pure);
		class_add_func("any", common_types._bool, &BoolList::any, Flags::Pure);
		//class_add_func("__bool__", common_types._bool, &BoolList::all, Flags::PURE);
		add_operator(OperatorID::And, common_types.bool_list, common_types.bool_list, common_types.bool_list, InlineID::None, &BoolList::and_values);
		add_operator(OperatorID::Or, common_types.bool_list, common_types.bool_list, common_types.bool_list, InlineID::None, &BoolList::or_values);
		// maybe bool[] == bool[] -> bool  ???
		add_operator(OperatorID::Equal, common_types.bool_list, common_types.bool_list, common_types.bool_list, InlineID::None, &BoolList::eq_values);
		add_operator(OperatorID::NotEqual, common_types.bool_list, common_types.bool_list, common_types.bool_list, InlineID::None, &BoolList::ne_values);
		add_operator(OperatorID::And, common_types.bool_list, common_types.bool_list, common_types._bool, InlineID::None, &BoolList::and_values_scalar);
		add_operator(OperatorID::Or, common_types.bool_list, common_types.bool_list, common_types._bool, InlineID::None, &BoolList::or_values_scalar);
		add_operator(OperatorID::Equal, common_types.bool_list, common_types.bool_list, common_types._bool, InlineID::None, &BoolList::eq_values_scalar);
		add_operator(OperatorID::NotEqual, common_types.bool_list, common_types.bool_list, common_types._bool, InlineID::None, &BoolList::ne_values_scalar);



	add_class(common_types.i32_list);
		class_add_func(Identifier::func::Str, common_types.string, &XList<int>::str, Flags::Pure);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::iadd_values);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::isub_values);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::imul_values);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::idiv_values);
		add_operator(OperatorID::Add, common_types.i32_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::add_values);
		add_operator(OperatorID::Subtract, common_types.i32_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::sub_values);
		add_operator(OperatorID::Multiply, common_types.i32_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::mul_values);
		add_operator(OperatorID::Divide, common_types.i32_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::div_values);
		add_operator(OperatorID::Exponent, common_types.i32_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::exp_values);
		add_operator(OperatorID::Add, common_types.i32_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::add_values_scalar);
		add_operator(OperatorID::Subtract, common_types.i32_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::sub_values_scalar);
		add_operator(OperatorID::Multiply, common_types.i32_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::mul_values_scalar);
		add_operator(OperatorID::Divide, common_types.i32_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::div_values_scalar);
		add_operator(OperatorID::Exponent, common_types.i32_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::exp_values_scalar);
		add_operator(OperatorID::Assign, common_types._void, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::assign_values_scalar);
		add_operator(OperatorID::Smaller, common_types.bool_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::lt_values);
		add_operator(OperatorID::SmallerEqual, common_types.bool_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::le_values);
		add_operator(OperatorID::Greater, common_types.bool_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::gt_values);
		add_operator(OperatorID::GreaterEqual, common_types.bool_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::ge_values);
		// don't we prefer  int[] == int[] -> bool ???
		add_operator(OperatorID::Equal, common_types.bool_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::eq_values);
		add_operator(OperatorID::NotEqual, common_types.bool_list, common_types.i32_list, common_types.i32_list, InlineID::None, &XList<int>::ne_values);
		add_operator(OperatorID::Smaller, common_types.bool_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::lt_values_scalar);
		add_operator(OperatorID::SmallerEqual, common_types.bool_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::le_values_scalar);
		add_operator(OperatorID::Greater, common_types.bool_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::gt_values_scalar);
		add_operator(OperatorID::GreaterEqual, common_types.bool_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::ge_values_scalar);
		add_operator(OperatorID::Equal, common_types.bool_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::eq_values_scalar);
		add_operator(OperatorID::NotEqual, common_types.bool_list, common_types.i32_list, common_types.i32, InlineID::None, &XList<int>::ne_values_scalar);

	add_class(common_types.f32_list);
		class_add_func(Identifier::func::Str, common_types.string, &XList<float>::str, Flags::Pure);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::iadd_values);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::isub_values);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::imul_values);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::idiv_values);
		add_operator(OperatorID::Add, common_types.f32_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::add_values);
		add_operator(OperatorID::Subtract, common_types.f32_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::sub_values);
		add_operator(OperatorID::Multiply, common_types.f32_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::mul_values);
		add_operator(OperatorID::Divide, common_types.f32_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::div_values);
		add_operator(OperatorID::Exponent, common_types.f32_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::exp_values);
		add_operator(OperatorID::Add, common_types.f32_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::add_values_scalar);
		add_operator(OperatorID::Subtract, common_types.f32_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::sub_values_scalar);
		add_operator(OperatorID::Multiply, common_types.f32_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::mul_values_scalar);
		add_operator(OperatorID::Divide, common_types.f32_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::div_values_scalar);
		add_operator(OperatorID::Exponent, common_types.f32_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::exp_values_scalar);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::iadd_values_scalar);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::isub_values_scalar);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::imul_values_scalar);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::idiv_values_scalar);
		add_operator(OperatorID::Assign, common_types._void, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::assign_values_scalar);
		add_operator(OperatorID::Smaller, common_types.bool_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::lt_values);
		add_operator(OperatorID::SmallerEqual, common_types.bool_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::le_values);
		add_operator(OperatorID::Greater, common_types.bool_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::gt_values);
		add_operator(OperatorID::GreaterEqual, common_types.bool_list, common_types.f32_list, common_types.f32_list, InlineID::None, &XList<float>::ge_values);
		add_operator(OperatorID::Smaller, common_types.bool_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::lt_values_scalar);
		add_operator(OperatorID::SmallerEqual, common_types.bool_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::le_values_scalar);
		add_operator(OperatorID::Greater, common_types.bool_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::gt_values_scalar);
		add_operator(OperatorID::GreaterEqual, common_types.bool_list, common_types.f32_list, common_types.f32, InlineID::None, &XList<float>::ge_values_scalar);


	add_class(common_types.f64_list);
		class_add_func(Identifier::func::Str, common_types.string, &XList<double>::str, Flags::Pure);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::iadd_values);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::isub_values);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::imul_values);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::idiv_values);
		add_operator(OperatorID::Add, common_types.f64_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::add_values);
		add_operator(OperatorID::Subtract, common_types.f64_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::sub_values);
		add_operator(OperatorID::Multiply, common_types.f64_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::mul_values);
		add_operator(OperatorID::Divide, common_types.f64_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::div_values);
		add_operator(OperatorID::Exponent, common_types.f64_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::exp_values);
		add_operator(OperatorID::Add, common_types.f64_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::add_values_scalar);
		add_operator(OperatorID::Subtract, common_types.f64_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::sub_values_scalar);
		add_operator(OperatorID::Multiply, common_types.f64_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::mul_values_scalar);
		add_operator(OperatorID::Divide, common_types.f64_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::div_values_scalar);
		add_operator(OperatorID::Exponent, common_types.f64_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::exp_values_scalar);
		add_operator(OperatorID::AddAssign, common_types._void, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::iadd_values_scalar);
		add_operator(OperatorID::SubtractAssign, common_types._void, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::isub_values_scalar);
		add_operator(OperatorID::MultiplyAssign, common_types._void, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::imul_values_scalar);
		add_operator(OperatorID::DivideAssign, common_types._void, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::idiv_values_scalar);
		add_operator(OperatorID::Assign, common_types._void, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::assign_values_scalar);
		add_operator(OperatorID::Smaller, common_types.bool_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::lt_values);
		add_operator(OperatorID::SmallerEqual, common_types.bool_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::le_values);
		add_operator(OperatorID::Greater, common_types.bool_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::gt_values);
		add_operator(OperatorID::GreaterEqual, common_types.bool_list, common_types.f64_list, common_types.f64_list, InlineID::None, &XList<double>::ge_values);
		add_operator(OperatorID::Smaller, common_types.bool_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::lt_values_scalar);
		add_operator(OperatorID::SmallerEqual, common_types.bool_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::le_values_scalar);
		add_operator(OperatorID::Greater, common_types.bool_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::gt_values_scalar);
		add_operator(OperatorID::GreaterEqual, common_types.bool_list, common_types.f64_list, common_types.f64, InlineID::None, &XList<double>::ge_values_scalar);



	add_class(common_types.string_list);
		class_add_func("join", common_types.string, &StringList::join, Flags::Pure);
			func_add_param("glue", common_types.string);
		add_operator(OperatorID::Equal, common_types._bool, common_types.string_list, common_types.string_list, InlineID::None, &StringList::__eq__);
		add_operator(OperatorID::NotEqual, common_types._bool, common_types.string_list, common_types.string_list, InlineID::None, &StringList::__neq__);
		add_operator(OperatorID::Add, common_types.string_list, common_types.string_list, common_types.string_list, InlineID::None, &StringList::__add__); // legacy... |
		add_operator(OperatorID::AddAssign, common_types._void, common_types.string_list, common_types.string_list, InlineID::None, &StringList::__adds__); // legacy... |=



	// constants
	void *kaba_nil = nullptr;
	bool kaba_true = true;
	bool kaba_false = false;
	add_const("nil", common_types.none, &kaba_nil);
	add_const("false", common_types._bool, &kaba_false);
	add_const("true",  common_types._bool, &kaba_true);


	add_class(common_types.exception);
		class_add_func(Identifier::func::Init, common_types._void, &KabaException::__init__, Flags::Mutable);
			func_add_param("message", common_types.string);
		class_add_func_virtual(Identifier::func::Delete, common_types._void, &KabaException::__delete__, Flags::Mutable);
		class_add_func_virtual(Identifier::func::Str, common_types.string, &KabaException::message);
		class_add_element("_text", common_types.string, config.target.pointer_size);
		class_set_vtable(KabaException);

	add_class(common_types.no_value_error);
		class_derive_from(common_types.exception);
		class_add_func(Identifier::func::Init, common_types._void, &KabaNoValueError::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, common_types._void, &KabaNoValueError::__delete__, Flags::Override | Flags::Mutable);
		class_set_vtable(KabaNoValueError);

	add_func(Identifier::Raise, common_types._void, &kaba_raise_exception, Flags::Static | Flags::RaisesExceptions);
		func_add_param("e", common_types.exception_xfer);
	add_func("@die", common_types._void, &kaba_die, Flags::Static | Flags::RaisesExceptions);
		func_add_param("e", common_types.pointer);
	add_func(Identifier::Assert, common_types._void, &kaba_assert, Flags::Static | Flags::RaisesExceptions);
		func_add_param("b", common_types._bool);


	// type casting
	add_func("p2s", common_types.string, &p2s, Flags::Static | Flags::Pure);
		func_add_param("p", common_types.pointer);
	add_func("char", common_types.string, &kaba_char2str, Flags::Static | Flags::Pure);
		func_add_param("c", common_types.i32);
	add_func("hex", common_types.string, &kaba_int32_hex, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i32);
	add_func("hex", common_types.string, &kaba_int64_hex, Flags::Static | Flags::Pure);
		func_add_param("i", common_types.i64);
	// debug output
	/*add_func("cprint", common_types._void, &_cstringout, Flags::STATIC);
		func_add_param("str", common_types.cstring);*/
	add_func("print", common_types._void, &os::terminal::print, Flags::Static);
		func_add_param("str", common_types.string_auto_cast);//, (Flags)((int)Flags::CONST | (int)Flags::AUTO_CAST));
	add_ext_var("_print_postfix", common_types.string, &os::terminal::_print_postfix_);
	add_func("as_binary", common_types.bytes, &kaba_binary, Flags::Static);
		func_add_param("p", common_types.reference, Flags::Ref);
		func_add_param("length", common_types.i32);
	// memory
	add_func("@malloc", common_types.pointer, &kaba_malloc, Flags::Static);
		func_add_param("size", common_types.i32);
	add_func("@free", common_types._void, &free, Flags::Static);
		func_add_param("p", common_types.pointer);

	// basic testing
	add_func("_ping", common_types._void, &kaba_ping, Flags::Static);
	add_func("_int_out", common_types._void, &kaba_int_out, Flags::Static);
		func_add_param("i", common_types.i32);
	add_func("_float_out", common_types._void, &kaba_float_out, Flags::Static);
		func_add_param("f", common_types.f32);
	add_func("_call_float", common_types._void, &_x_call_float, Flags::Static);
	add_func("_float_ret", common_types.f32, &kaba_float_ret, Flags::Static);
	add_func("_int_ret", common_types.i32, &kaba_int_ret, Flags::Static);
	add_func("_xxx", common_types._void, &kaba_xxx, Flags::Static);
		func_add_param("a", common_types.i32);
		func_add_param("b", common_types.i32);
		func_add_param("c", common_types.i32);
		func_add_param("d", common_types.i32);
		func_add_param("e", common_types.i32);
		func_add_param("f", common_types.i32);
		func_add_param("g", common_types.i32);
		func_add_param("h", common_types.i32);
	add_ext_var("_extern_variable", common_types.i32, &extern_variable1);


	add_type_cast(10, common_types.i32, common_types.f32, "i32.__f32__");
	add_type_cast(10, common_types.i32, common_types.f64, "i32.__f64__");
	add_type_cast(10, common_types.i32, common_types.i64, "i32.__i64__");
	add_type_cast(200, common_types.i64, common_types.i32, "i64.__i32__");
	add_type_cast(10, common_types.f32, common_types.f64,"f32.__f64__");
	add_type_cast(200, common_types.f32, common_types.i32, "f32.__i32__");
	add_type_cast(200, common_types.i32, common_types.i8, "i32.__i8__");
	add_type_cast(20, common_types.i8, common_types.i32, "i8.__i32__");
	add_type_cast(200, common_types.i32, common_types.u8, "i32.__u8__");
	add_type_cast(20, common_types.u8, common_types.i32, "u8.__i32__");
	//add_type_cast(30, common_types.bool_list, common_types._bool, "bool[].__bool__");
	add_type_cast(50, common_types.pointer, common_types._bool, "p2b");
	//add_type_cast(50, common_types.pointer, common_types.string, "p2s");
}



}
