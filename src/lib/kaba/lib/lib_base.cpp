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

extern const Class *TypeDynamicArray;
extern const Class *TypeSharedPointer;
extern const Class *TypeStringAutoCast;
extern const Class *TypeDictBase;
extern const Class *TypeCallableBase;
extern const Class *TypeNone;
extern const Class *TypePointerList;
extern const Class *TypeObject;
extern const Class *TypeObjectP;
extern const Class *TypeBoolList;
extern const Class *TypeIntP;
extern const Class *TypeIntList;
extern const Class *TypeFloatP;
extern const Class *TypeFloatList;
extern const Class *TypeFloat64List;
extern const Class *TypeStringList;
extern const Class *TypeIntDict;
extern const Class *TypeFloatDict;
extern const Class *TypeStringDict;
extern const Class *TypeAny;
extern const Class *TypeNoValueError;

const Class *TypeIntOptional;
const Class *TypeFloatOptional;

const Class *TypeRawT;
const Class *TypeXferT;
const Class *TypeSharedT;
const Class *TypeSharedNotNullT;
const Class *TypeOwnedT;
const Class *TypeOwnedNotNullT;
const Class *TypeAliasT;
const Class *TypeReferenceT;
const Class *TypeArrayT;
const Class *TypeListT;
const Class *TypeDictT;
const Class *TypeCallableFPT;
const Class *TypeCallableBindT;
const Class *TypeOptionalT;
const Class *TypeProductT;
const Class *TypeEnumT;
const Class *TypeStructT;
const Class *TypeInterfaceT;
const Class *TypeNamespaceT;

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
	void _cdecl assign(StringList &s) {
		*this = s;
	}
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

	add_func("@sorted", TypeDynamicArray, &array_sort, Flags::Static | Flags::RaisesExceptions);
		func_add_param("list", TypePointer);
		func_add_param("class", TypeClassRef);
		func_add_param("by", TypeString);
	add_func("@var2str", TypeString, &var2str, Flags::Static | Flags::RaisesExceptions);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassRef);
	add_func("@var_repr", TypeString, &var_repr, Flags::Static | Flags::RaisesExceptions);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassRef);
	add_func("@dyn", TypeAny, &dynify, Flags::Static | Flags::RaisesExceptions);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassRef);
}

void SIAddPackageBase(Context *c) {
	add_package(c, "base", Flags::AutoImport);

	// internal
	TypeUnknown			= add_type  ("@unknown", 0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type_simple  ("@reg128", 16, 16, Flags::ForceCallByValue);
	TypeReg64			= add_type_simple  ("@reg64", 8, 8, Flags::ForceCallByValue);
	TypeReg32			= add_type_simple  ("@reg32", 4, 4, Flags::ForceCallByValue);
	TypeReg16			= add_type_simple  ("@reg16", 2, 2, Flags::ForceCallByValue);
	TypeReg8			= add_type_simple  ("@reg8", 1, 1, Flags::ForceCallByValue);
	TypeObject			= add_type  ("Object", sizeof(VirtualBase)); // base for most virtual classes
	TypeDynamic			= add_type  ("@dynamic", 0);

	// "real"
	TypeVoid			= add_type_simple  ("void", 0, 1, Flags::ForceCallByValue);
	TypeBool			= add_type_simple  ("bool", sizeof(bool), 1, Flags::ForceCallByValue);
	TypeInt8			= add_type_simple  ("i8", 1, 1, Flags::ForceCallByValue);
	TypeUInt8			= add_type_simple  ("u8", 1, 1, Flags::ForceCallByValue);
	TypeInt16			= add_type_simple  ("i16", 2, 2, Flags::ForceCallByValue);
	TypeUInt16			= add_type_simple  ("u16", 2, 2, Flags::ForceCallByValue);
	TypeInt32			= add_type_simple  ("i32", sizeof(int32), 4, Flags::ForceCallByValue);
	TypeInt64			= add_type_simple  ("i64", sizeof(int64), 8, Flags::ForceCallByValue);
	TypeFloat32			= add_type_simple  ("f32", sizeof(float), 4, Flags::ForceCallByValue);
	TypeFloat64			= add_type_simple  ("f64", sizeof(double), 8, Flags::ForceCallByValue);
	TypeDynamicArray	= add_type  ("@DynamicArray", config.target.dynamic_array_size);
	TypeDictBase		= add_type  ("@DictBase",   config.target.dynamic_array_size);
	TypeSharedPointer	= add_type  ("@SharedPointer", config.target.pointer_size);
	TypeCallableBase	= add_type  ("@CallableBase", sizeof(Callable<void()>));

	TypeNoValueError    = add_type  ("NoValueError", sizeof(KabaException));

	// type aliases
	cur_package->tree->base_class->type_aliases.add({"int", TypeInt32});
	cur_package->tree->base_class->type_aliases.add({"float", TypeFloat32});



	TypeRawT = add_class_template("ptr", {"T"}, new TemplateClassInstantiatorPointerRaw);
	TypeXferT = add_class_template("xfer", {"T"}, new TemplateClassInstantiatorPointerXfer);
	TypeSharedT = add_class_template("shared", {"T"}, new TemplateClassInstantiatorPointerShared);
	TypeSharedNotNullT = add_class_template("shared!", {"T"}, new TemplateClassInstantiatorPointerSharedNotNull);
	TypeOwnedT = add_class_template("owned", {"T"}, new TemplateClassInstantiatorPointerOwned);
	TypeOwnedNotNullT = add_class_template("owned!", {"T"}, new TemplateClassInstantiatorPointerOwnedNotNull);
	TypeAliasT = add_class_template("@alias", {"T"}, new TemplateClassInstantiatorPointerAlias);
	TypeReferenceT = add_class_template("ref", {"T"}, new TemplateClassInstantiatorReference);
	TypeArrayT = add_class_template("@Array", {"T"}, new TemplateClassInstantiatorArray);
	TypeListT = add_class_template("@List", {"T"}, new TemplateClassInstantiatorList);
	TypeDictT = add_class_template("@Dict", {"T"}, new TemplateClassInstantiatorDict);
	TypeCallableFPT = add_class_template("@CallableFP", {"T..."}, new TemplateClassInstantiatorCallableFP);
	TypeCallableBindT = add_class_template("@Bind", {"T..."}, new TemplateClassInstantiatorCallableBind);
	TypeOptionalT = add_class_template("@Optional", {"T"}, new TemplateClassInstantiatorOptional);
	TypeProductT = add_class_template("@Product", {"T"}, new TemplateClassInstantiatorProduct);
	TypeStructT = add_class_template("@Struct", {"T"}, nullptr);
	TypeEnumT = add_class_template("@Enum", {"T"}, new TemplateClassInstantiatorEnum);
	TypeInterfaceT = add_class_template("@Interface", {"T"}, nullptr);
	TypeNamespaceT = add_class_template("@Namespace", {"T"}, nullptr);


	add_class(TypeObject);
		class_add_func(Identifier::func::Init, TypeVoid, &_VirtualBase::__init__, Flags::Mutable);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, &VirtualBase::__delete__, Flags::Mutable);
		class_set_vtable(VirtualBase);

	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt32, config.target.pointer_size);
		class_add_func("swap", TypeVoid, &DynamicArray::simple_swap, Flags::Mutable);
			func_add_param("i1", TypeInt32);
			func_add_param("i2", TypeInt32);
		class_add_func(Identifier::func::Subarray, TypeDynamicArray, &DynamicArray::ref_subarray, Flags::Ref);
			func_add_param("start", TypeInt32);
			func_add_param("end", TypeInt32);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init, Flags::Mutable);
			func_add_param("element_size", TypeInt32);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear, Flags::Mutable);
		class_add_func("__mem_forget__", TypeVoid, &DynamicArray::forget, Flags::Mutable);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize, Flags::Mutable);
			func_add_param("size", TypeInt32);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single, Flags::Mutable);
			func_add_param("index", TypeInt32);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt32, config.target.pointer_size);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init, Flags::Mutable);
			func_add_param("element_size", TypeInt32);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear, Flags::Mutable);
		class_add_func("__mem_forget__", TypeVoid, &DynamicArray::forget, Flags::Mutable);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize, Flags::Mutable);
			func_add_param("size", TypeInt32);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single, Flags::Mutable);
			func_add_param("index", TypeInt32);

	add_class(TypeSharedPointer);
		class_add_func(Identifier::func::Init, TypeVoid, nullptr, Flags::Mutable);
			func_set_inline(InlineID::SharedPointerInit);


	TypeObjectP			= add_type_p_raw(TypeObject);


	// derived   (must be defined after the primitive types and the bases!)
	TypePointer     = add_type_p_raw(TypeVoid); // substitute for all raw pointer types
	TypeReference   = add_type_ref(TypeVoid); // substitute for all reference types
	TypeNone        = add_type_p_raw(TypeVoid); // type of <nil>
	const_cast<Class*>(TypeNone)->name = "None";
	TypePointerList = add_type_list(TypePointer);
	TypeBoolList    = add_type_list(TypeBool);
	TypeIntP        = add_type_p_raw(TypeInt32);
	TypeIntOptional = add_type_optional(TypeInt32);
	TypeFloatOptional = add_type_optional(TypeFloat32);
	TypeIntList     = add_type_list(TypeInt32);
	TypeFloatP      = add_type_p_raw(TypeFloat32);
	TypeFloatList   = add_type_list(TypeFloat32);
	TypeFloat64List = add_type_list(TypeFloat64);
	TypeBytes      = add_type_list(TypeUInt8);
	TypeCString     = add_type_array(TypeUInt8, 256);
	capture_implicit_type(TypeCString, "cstring"); // cstring := u8[256]
	TypeString      = add_type_list(TypeUInt8);
	capture_implicit_type(TypeString, "string"); // string := u8[]
	TypeStringAutoCast = add_type("<string-auto-cast>", config.target.dynamic_array_size);	// string := i8[]
	TypeStringList  = add_type_list(TypeString);
	capture_implicit_type(TypeBytes, "bytes"); // bytes := u8[]

	TypeIntDict     = add_type_dict(TypeInt32);
	TypeFloatDict   = add_type_dict(TypeFloat32);
	TypeStringDict  = add_type_dict(TypeString);

	TypeException		= add_type  ("Exception", sizeof(KabaException));
	TypeExceptionXfer	= add_type_p_xfer(TypeException);

	lib_create_list<void*>(TypePointerList);
	lib_create_list<bool>(TypeBoolList);
	lib_create_list<int>(TypeIntList);
	lib_create_list<float>(TypeFloatList);
	lib_create_list<double>(TypeFloat64List);
	lib_create_list<char>(TypeString);
	lib_create_list<uint8>(TypeBytes);
	lib_create_list<string>(TypeStringList);


	lib_create_optional<int>(TypeIntOptional);
	lib_create_optional<float>(TypeFloatOptional);


	auto TypeInt32Ref = add_type_ref(TypeInt32);
	auto TypeInt32RefOptional = add_type_optional(TypeInt32Ref);
	auto TypeFloat32Ref = add_type_ref(TypeFloat32);
	auto TypeFloat32RefOptional = add_type_optional(TypeFloat32Ref);
	auto TypeStringRef = add_type_ref(TypeString);
	auto TypeStringRefOptional = add_type_optional(TypeStringRef);

	lib_create_optional<void*>(TypeInt32RefOptional);
	lib_create_optional<void*>(TypeFloat32RefOptional);
	lib_create_optional<void*>(TypeStringRefOptional);

	lib_create_dict<int>(TypeIntDict, TypeInt32RefOptional);
	lib_create_dict<float>(TypeFloatDict, TypeFloat32RefOptional);
	lib_create_dict<string>(TypeStringDict, TypeStringRefOptional);



	add_class(TypeCallableBase);
		class_add_element("_fp", TypePointer, &KabaCallableBase::fp);
		class_add_element("_pp", TypePointer, &KabaCallableBase::pp);
		//class_add_func(Identifier::Func::INIT, TypeVoid, &KabaCallableBase::__init__);
		class_add_func(Identifier::func::Assign, TypeVoid, nullptr, Flags::Mutable);
			func_set_inline(InlineID::ChunkAssign);
		class_add_func_virtual("call", TypeVoid, &KabaCallableBase::operator());
	


	add_func("p2b", TypeBool, &kaba_cast<void*,bool>, Flags::Static | Flags::Pure);
		func_set_inline(InlineID::PointerToBool);
		func_add_param("p", TypePointer);


	add_class(TypePointer);
		class_add_func(Identifier::func::Str, TypeString, &p2s, Flags::Pure);
		add_operator(OperatorID::Assign, TypeVoid, TypePointer, TypePointer, InlineID::PointerAssign);
		add_operator(OperatorID::Equal, TypeBool, TypePointer, TypePointer, InlineID::PointerEqual, &pointer_equal);
		add_operator(OperatorID::NotEqual, TypeBool, TypePointer, TypePointer, InlineID::PointerNotEqual, &pointer_not_equal);


	// TODO define in template by exact X&:=X&
	// then allow right side casting by standard rules
	add_class(TypeReference);
		add_operator(OperatorID::RefAssign, TypeVoid, TypeReference, TypeReference, InlineID::PointerAssign);


	add_class(TypeBool);
		class_add_func(Identifier::func::Str, TypeString, &b2s, Flags::Pure);
		add_operator(OperatorID::Assign, TypeVoid, TypeBool, TypeBool, InlineID::BoolAssign);
		add_operator(OperatorID::Equal, TypeBool, TypeBool, TypeBool, InlineID::BoolEqual);
		add_operator(OperatorID::NotEqual, TypeBool, TypeBool, TypeBool, InlineID::BoolNotEqual);
		add_operator(OperatorID::And, TypeBool, TypeBool, TypeBool, InlineID::BoolAnd);
		add_operator(OperatorID::Or, TypeBool, TypeBool, TypeBool, InlineID::BoolOr);
		add_operator(OperatorID::Negate, TypeBool, nullptr, TypeBool, InlineID::BoolNot);


	add_class(TypeUInt8);
		class_add_func(Identifier::func::Str, TypeString, &kaba_uint8_to_str, Flags::Pure);
		//class_add_func(Identifier::Func::REPR, TypeString, &kaba_char_repr, Flags::PURE);
		class_add_func("__i32__", TypeInt32, &kaba_cast<uint8,int>, Flags::Pure);
			func_set_inline(InlineID::Uint8ToInt32);
		class_add_func("__i8__", TypeInt8, &kaba_cast<uint8,int8>, Flags::Pure);
			func_set_inline(InlineID::Passthrough);
		add_operator(OperatorID::Assign, TypeVoid, TypeUInt8, TypeUInt8, InlineID::Int8Assign);
		add_operator(OperatorID::Equal, TypeBool, TypeUInt8, TypeUInt8, InlineID::Int8Equal);
		add_operator(OperatorID::NotEqual, TypeBool, TypeUInt8, TypeUInt8, InlineID::Int8NotEqual);
		add_operator(OperatorID::Greater, TypeBool, TypeUInt8, TypeUInt8, InlineID::Uint8Greater);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeUInt8, TypeUInt8, InlineID::Uint8GreaterEqual);
		add_operator(OperatorID::Smaller, TypeBool, TypeUInt8, TypeUInt8, InlineID::Uint8Smaller);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeUInt8, TypeUInt8, InlineID::Uint8SmallerEqual);
		add_operator(OperatorID::Add, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::Int8Add);
		add_operator(OperatorID::SubtractAssign, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::Int8SubtractAssign);
		add_operator(OperatorID::AddAssign, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::Int8AddAssign);
		add_operator(OperatorID::Subtract, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::Int8Subtract);
		add_operator(OperatorID::BitAnd, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::Int8BitAnd);
		add_operator(OperatorID::BitOr, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::Int8BitOr);
		add_operator(OperatorID::Negative, TypeUInt8, nullptr, TypeUInt8, InlineID::Int8Negative);


	add_class(TypeInt8);
		class_add_func(Identifier::func::Str, TypeString, &kaba_int8_to_str, Flags::Pure);
		//class_add_func(Identifier::Func::REPR, TypeString, &kaba_char_repr, Flags::PURE);
		class_add_func("__i32__", TypeInt32, &kaba_cast<char,int>, Flags::Pure);
			func_set_inline(InlineID::Int8ToInt32);
		class_add_func("__u8__", TypeInt8, &kaba_cast<int8,uint8>, Flags::Pure);
			func_set_inline(InlineID::Passthrough);
		add_operator(OperatorID::Assign, TypeVoid, TypeInt8, TypeInt8, InlineID::Int8Assign);
		add_operator(OperatorID::Equal, TypeBool, TypeInt8, TypeInt8, InlineID::Int8Equal);
		add_operator(OperatorID::NotEqual, TypeBool, TypeInt8, TypeInt8, InlineID::Int8NotEqual);
		add_operator(OperatorID::Greater, TypeBool, TypeInt8, TypeInt8, InlineID::Int8Greater);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeInt8, TypeInt8, InlineID::Int8GreaterEqual);
		add_operator(OperatorID::Smaller, TypeBool, TypeInt8, TypeInt8, InlineID::Int8Smaller);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeInt8, TypeInt8, InlineID::Int8SmallerEqual);
		add_operator(OperatorID::Add, TypeInt8, TypeInt8, TypeInt8, InlineID::Int8Add);
		add_operator(OperatorID::SubtractAssign, TypeInt8, TypeInt8, TypeInt8, InlineID::Int8SubtractAssign);
		add_operator(OperatorID::AddAssign, TypeInt8, TypeInt8, TypeInt8, InlineID::Int8AddAssign);
		add_operator(OperatorID::Subtract, TypeInt8, TypeInt8, TypeInt8, InlineID::Int8Subtract);
		add_operator(OperatorID::BitAnd, TypeInt8, TypeInt8, TypeInt8, InlineID::Int8BitAnd);
		add_operator(OperatorID::BitOr, TypeInt8, TypeInt8, TypeInt8, InlineID::Int8BitOr);
		add_operator(OperatorID::Negative, TypeInt8, nullptr, TypeInt8, InlineID::Int8Negative);


	add_class(TypeInt16);
		class_add_element("low", TypeUInt8, 0);
		class_add_element("high", TypeUInt8, 1);
		class_add_func(Identifier::func::Str, TypeString, &kaba_i16_to_str, Flags::Pure);
		class_add_func("__i32__", TypeInt32, &kaba_cast<short,int>, Flags::Pure);
		//	func_set_inline(InlineID::INT16_TO_INT32);
		add_operator(OperatorID::Assign, TypeVoid, TypeInt16, TypeInt16, InlineID::ChunkAssign);
		//add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt16, TypeInt32, InlineID::INT16_ASSIGN_INT32);
		class_add_func("__assign__", TypeVoid, &kaba_i16_from_i32, Flags::Mutable);
			func_add_param("o", TypeInt32);


	add_class(TypeUInt16);
		class_add_element("low", TypeUInt8, 0);
		class_add_element("high", TypeUInt8, 1);
		class_add_func(Identifier::func::Str, TypeString, &kaba_u16_to_str, Flags::Pure);
		class_add_func("__i32__", TypeInt32, &kaba_cast<unsigned short,int>, Flags::Pure);
		//	func_set_inline(InlineID::INT16_TO_INT32);
		add_operator(OperatorID::Assign, TypeVoid, TypeUInt16, TypeUInt16, InlineID::ChunkAssign);
		//add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt16, TypeInt32, InlineID::INT16_ASSIGN_INT32);
		class_add_func("__assign__", TypeVoid, &kaba_u16_from_i32, Flags::Mutable);
			func_add_param("o", TypeInt32);

	add_class(TypeInt32);
		class_add_func(Identifier::func::Str, TypeString, &i2s, Flags::Pure);
		class_add_func(Identifier::func::Format, TypeString, &kaba_int_format, Flags::Pure);
			func_add_param("fmt", TypeString);
		class_add_func("__f32__", TypeFloat32, &kaba_cast<int,float>, Flags::Pure);
			func_set_inline(InlineID::Int32ToFloat32);
		class_add_func("__f64__", TypeFloat64, &kaba_cast<int,double>, Flags::Pure);
		class_add_func("__i8__", TypeInt8, &kaba_cast<int,int8>, Flags::Pure);
			func_set_inline(InlineID::Int32ToInt8);
		class_add_func("__u8__", TypeUInt8, &kaba_cast<int,uint8>, Flags::Pure);
			func_set_inline(InlineID::Int32ToUint8);
		class_add_func("__i64__", TypeInt64, &kaba_cast<int,int64>, Flags::Pure);
			func_set_inline(InlineID::Int32ToInt64);
		add_operator(OperatorID::Assign, TypeVoid, TypeInt32, TypeInt32, InlineID::Int32Assign);
		add_operator(OperatorID::Add, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32Add, &op_int_add);
		add_operator(OperatorID::Subtract, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32Subtract, &op_int_sub);
		add_operator(OperatorID::Multiply, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32Multiply, &op_int_mul);
		add_operator(OperatorID::Divide, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32Divide, &op_int_div);
		add_operator(OperatorID::Exponent, TypeInt32, TypeInt32, TypeInt32, InlineID::None, &xop_exp<int>);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeInt32, TypeInt32, InlineID::Int32AddAssign);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeInt32, TypeInt32, InlineID::Int32SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeInt32, TypeInt32, InlineID::Int32MultiplyAssign);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeInt32, TypeInt32, InlineID::Int32DivideAssign);
		add_operator(OperatorID::Modulo, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32Modulo, &op_int32_mod);
		add_operator(OperatorID::Equal, TypeBool, TypeInt32, TypeInt32, InlineID::Int32Equal, &op_int_eq);
		add_operator(OperatorID::NotEqual, TypeBool, TypeInt32, TypeInt32, InlineID::Int32NotEqual, &op_int_neq);
		add_operator(OperatorID::Greater, TypeBool, TypeInt32, TypeInt32, InlineID::Int32Greater, &op_int_g);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeInt32, TypeInt32, InlineID::Int32GreaterEqual, &op_int_ge);
		add_operator(OperatorID::Smaller, TypeBool, TypeInt32, TypeInt32, InlineID::Int32Smaller, &op_int_l);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeInt32, TypeInt32, InlineID::Int32SmallerEqual, &op_int_le);
		add_operator(OperatorID::BitAnd, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32BitAnd, &op_int32_and);
		add_operator(OperatorID::BitOr, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32BitOr, &op_int32_or);
		add_operator(OperatorID::ShiftRight, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32ShiftRight, &op_int32_shr);
		add_operator(OperatorID::ShiftLeft, TypeInt32, TypeInt32, TypeInt32, InlineID::Int32ShiftLeft, &op_int32_shl);
		add_operator(OperatorID::Negative, TypeInt32, nullptr, TypeInt32, InlineID::Int32Negative, &op_int_neg);
		add_operator(OperatorID::Increase, TypeVoid, TypeInt32, nullptr, InlineID::Int32Increase);
		add_operator(OperatorID::Decrease, TypeVoid, TypeInt32, nullptr, InlineID::Int32Decrease);

	add_class(TypeInt64);
		class_add_func(Identifier::func::Str, TypeString, &i642s, Flags::Pure);
		class_add_func(Identifier::func::Format, TypeString, &kaba_int64_format, Flags::Pure);
			func_add_param("fmt", TypeString);
		class_add_func("__i32__", TypeInt32, &kaba_cast<int64,int>, Flags::Pure);
			func_set_inline(InlineID::Int64ToInt32);
		add_operator(OperatorID::Assign, TypeVoid, TypeInt64, TypeInt64, InlineID::Int64Assign);
		add_operator(OperatorID::Add, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64Add, &op_int64_add);
		add_operator(OperatorID::Add, TypeInt64, TypeInt64, TypeInt32, InlineID::Int64AddInt32, &op_int64_add_int); // needed by internal address calculations!
		add_operator(OperatorID::Subtract, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64Subtract, &op_int64_sub);
		add_operator(OperatorID::Multiply, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64Multiply, &op_int64_mul);
		add_operator(OperatorID::Divide, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64Divide, &op_int64_div);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeInt64, TypeInt64, InlineID::Int64AddAssign);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeInt64, TypeInt64, InlineID::Int64SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeInt64, TypeInt64, InlineID::Int64MultiplyAssign);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeInt64, TypeInt64, InlineID::Int64DivideAssign);
		add_operator(OperatorID::Modulo, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64Modulo, &op_int64_mod);
		add_operator(OperatorID::Equal, TypeBool, TypeInt64, TypeInt64, InlineID::Int64Equal, &op_int64_eq);
		add_operator(OperatorID::NotEqual, TypeBool, TypeInt64, TypeInt64, InlineID::Int64NotEqual, &op_int64_neq);
		add_operator(OperatorID::Greater, TypeBool, TypeInt64, TypeInt64, InlineID::Int64Greater, &op_int64_g);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeInt64, TypeInt64, InlineID::Int64GreaterEqual, &op_int64_ge);
		add_operator(OperatorID::Smaller, TypeBool, TypeInt64, TypeInt64, InlineID::Int64Smaller, &op_int64_l);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeInt64, TypeInt64, InlineID::Int64SmallerEqual, &op_int64_le);
		add_operator(OperatorID::BitAnd, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64BitAnd, &op_int64_and);
		add_operator(OperatorID::BitOr, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64BitOr, &op_int64_or);
		add_operator(OperatorID::ShiftRight, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64ShiftRight, &op_int64_shr);
		add_operator(OperatorID::ShiftLeft, TypeInt64, TypeInt64, TypeInt64, InlineID::Int64ShiftLeft, &op_int64_shl);
		add_operator(OperatorID::Negative, TypeInt64, nullptr, TypeInt64, InlineID::Int64Negative, &op_int64_neg);
		add_operator(OperatorID::Increase, TypeVoid, TypeInt64, nullptr, InlineID::Int64Increase);
		add_operator(OperatorID::Decrease, TypeVoid, TypeInt64, nullptr, InlineID::Int64Decrease);

	add_class(TypeFloat32);
		class_add_func(Identifier::func::Str, TypeString, &kaba_float2str, Flags::Pure);
		class_add_func("str2", TypeString, &f2s, Flags::Pure);
			func_add_param("decimals", TypeInt32);
		class_add_func(Identifier::func::Format, TypeString, &kaba_f32_format, Flags::Pure);
			func_add_param("fmt", TypeString);
		class_add_func("__i32__", TypeInt32, &kaba_cast<float,int>, Flags::Pure);
			func_set_inline(InlineID::FloatToInt32);    // sometimes causes floating point exceptions...
		class_add_func("__f64__", TypeFloat64, &kaba_cast<float,double>, Flags::Pure);
			func_set_inline(InlineID::Float32ToFloat64);
		add_operator(OperatorID::Assign, TypeVoid, TypeFloat32, TypeFloat32, InlineID::Float32Assign);
		add_operator(OperatorID::Add, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::Float32Add, &op_float_add);
		add_operator(OperatorID::Subtract, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::Float32Subtarct, &op_float_sub);
		add_operator(OperatorID::Multiply, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::Float32Multiply, &op_float_mul);
		add_operator(OperatorID::Divide, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::Float32Divide, &op_float_div);
		add_operator(OperatorID::Exponent, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::None, &xop_exp<float>);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeFloat32, TypeFloat32, InlineID::Float32AddAssign);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeFloat32, TypeFloat32, InlineID::Float32SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeFloat32, TypeFloat32, InlineID::Float32MultiplyAssign);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeFloat32, TypeFloat32, InlineID::Float32DivideAssign);
		add_operator(OperatorID::Equal, TypeBool, TypeFloat32, TypeFloat32, InlineID::Float32Equal, &op_float_eq);
		add_operator(OperatorID::NotEqual, TypeBool, TypeFloat32, TypeFloat32, InlineID::Float32NotEqual, &op_float_neq);
		add_operator(OperatorID::Greater, TypeBool, TypeFloat32, TypeFloat32, InlineID::Float32Greater, &op_float_g);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeFloat32, TypeFloat32, InlineID::Float32GreaterEqual, &op_float_ge);
		add_operator(OperatorID::Smaller, TypeBool, TypeFloat32, TypeFloat32, InlineID::Float32Smaller, &op_float_l);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeFloat32, TypeFloat32, InlineID::Float32SmallerEqual, &op_float_le);
		add_operator(OperatorID::Negative, TypeFloat32, nullptr, TypeFloat32, InlineID::Float32Negative, &op_float_neg);


	add_class(TypeFloat64);
		class_add_func(Identifier::func::Str, TypeString, &kaba_float642str, Flags::Pure);
		class_add_func(Identifier::func::Format, TypeString, &kaba_f64_format, Flags::Pure);
			func_add_param("fmt", TypeString);
		class_add_func("__f32__", TypeFloat32, &kaba_cast<double,float>, Flags::Pure);
			func_set_inline(InlineID::Float64ToFloat32);
		class_add_func("__i32__", TypeInt32, &kaba_cast<double,int>, Flags::Pure);
		add_operator(OperatorID::Assign, TypeVoid, TypeFloat64, TypeFloat64, InlineID::Float64Assign);
		add_operator(OperatorID::Add, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::Float64Add, &op_double_add);
		add_operator(OperatorID::Subtract, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::Float64Subtract, &op_double_sub);
		add_operator(OperatorID::Multiply, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::Float64Multiply, &op_double_mul);
		add_operator(OperatorID::Divide, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::Float64Divide, &op_double_div);
		add_operator(OperatorID::Exponent, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::None, &xop_exp<double>);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeFloat64, TypeFloat64, InlineID::Float64AddAssign);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeFloat64, TypeFloat64, InlineID::Float64SubtractAssign);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeFloat64, TypeFloat64, InlineID::Float64MultiplyAssign);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeFloat64, TypeFloat64, InlineID::Float64DivideAssign);
		add_operator(OperatorID::Equal, TypeBool, TypeFloat64, TypeFloat64, InlineID::Float64Equal, &op_double_eq);
		add_operator(OperatorID::NotEqual, TypeBool, TypeFloat64, TypeFloat64, InlineID::Float64NotEqual, &op_double_neq);
		add_operator(OperatorID::Greater, TypeBool, TypeFloat64, TypeFloat64, InlineID::Float64Greater, &op_double_g);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeFloat64, TypeFloat64, InlineID::Float64GreaterEqual, &op_double_ge);
		add_operator(OperatorID::Smaller, TypeBool, TypeFloat64, TypeFloat64, InlineID::Float64Smaller, &op_double_l);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeFloat64, TypeFloat64, InlineID::Float64SmallerEqual, &op_double_le);
		add_operator(OperatorID::Negative, TypeFloat32, nullptr, TypeFloat64, InlineID::Float64Negative, &op_double_neg);


	add_class(TypeString);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeString, TypeString, InlineID::None, &string::operator+=);
		add_operator(OperatorID::Add, TypeString, TypeString, TypeString, InlineID::None, &string::operator+);
		add_operator(OperatorID::Equal, TypeBool, TypeString, TypeString, InlineID::None, &string::operator==);
		add_operator(OperatorID::NotEqual, TypeBool, TypeString, TypeString, InlineID::None, &string::operator!=);
		add_operator(OperatorID::Smaller, TypeBool, TypeString, TypeString, InlineID::None, &string::operator<);
		add_operator(OperatorID::SmallerEqual, TypeBool, TypeString, TypeString, InlineID::None, &string::operator<=);
		add_operator(OperatorID::Greater, TypeBool, TypeString, TypeString, InlineID::None, &string::operator>);
		add_operator(OperatorID::GreaterEqual, TypeBool, TypeString, TypeString, InlineID::None, &string::operator>=);
		class_add_func("head", TypeString, &string::head, Flags::Pure);
			func_add_param("size", TypeInt32);
		class_add_func("tail", TypeString, &string::tail, Flags::Pure);
			func_add_param("size", TypeInt32);
		class_add_func("find", TypeIntOptional, &KabaString::_find, Flags::Pure);
			func_add_param("str", TypeString);
			func_add_param_def("start", TypeInt32, 0);
		class_add_func("compare", TypeInt32, &string::compare, Flags::Pure);
			func_add_param("str", TypeString);
		class_add_func("icompare", TypeInt32, &string::icompare, Flags::Pure);
			func_add_param("str", TypeString);
		class_add_func("replace", TypeString, &string::replace, Flags::Pure);
			func_add_param("sub", TypeString);
			func_add_param("by", TypeString);
		class_add_func("explode", TypeStringList, &string::explode, Flags::Pure);
			func_add_param("str", TypeString);
		class_add_func("parse_tokens", TypeStringList, &string::parse_tokens, Flags::Pure);
			func_add_param("splitters", TypeString);
		class_add_func("repeat", TypeString, &string::repeat, Flags::Pure);
			func_add_param("n", TypeInt32);
		class_add_func("lower", TypeString, &string::lower, Flags::Pure);
		class_add_func("upper", TypeString, &string::upper, Flags::Pure);
		class_add_func("reverse", TypeString, &string::reverse, Flags::Pure);
		class_add_func("hash", TypeInt32, &string::hash, Flags::Pure);
		class_add_func("hex", TypeString, &string::hex, Flags::Pure);
		class_add_func("unhex", TypeBytes, &string::unhex, Flags::Pure);
		class_add_func("match", TypeBool, &string::match, Flags::Pure);
			func_add_param("glob", TypeString);
		class_add_func("__i32__", TypeInt32, &string::_int, Flags::Pure);
		class_add_func("__i64__", TypeInt64, &string::i64, Flags::Pure);
		class_add_func("__f32__", TypeFloat32, &string::_float, Flags::Pure);
		class_add_func("__f64__", TypeFloat64, &string::f64, Flags::Pure);
		class_add_func("trim", TypeString, &string::trim, Flags::Pure);
		class_add_func("escape", TypeString, &string::escape, Flags::Pure);
		class_add_func("unescape", TypeString, &string::unescape, Flags::Pure);
		class_add_func("utf8_to_utf32", TypeIntList, &string::utf8_to_utf32, Flags::Pure);
		class_add_func("utf8_length", TypeInt32, &string::utf8len, Flags::Pure);
		class_add_func("encode", TypeBytes, &KabaString::encode, Flags::Pure);
		class_add_func("decode", TypeString, &KabaString::decode, Flags::Pure | Flags::Static);
			func_add_param("b", TypeBytes);
		class_add_func(Identifier::func::Repr, TypeString, &string::repr, Flags::Pure);
		class_add_func(Identifier::func::Format, TypeString, &KabaString::format, Flags::Pure);
			func_add_param("fmt", TypeString);
		class_add_func(Identifier::func::Contains, TypeBool, &KabaString::contains_s, Flags::Pure);
			func_add_param("s", TypeString);


	add_class(TypeBytes);
		add_operator(OperatorID::Equal, TypeBool, TypeBytes, TypeBytes, InlineID::None, &bytes::operator==);
		add_operator(OperatorID::NotEqual, TypeBool, TypeBytes, TypeBytes, InlineID::None, &bytes::operator!=);
		class_add_func("reverse", TypeBytes, &kaba_bytes_reverse, Flags::Pure);
		class_add_func("hash", TypeInt32, &bytes::hash, Flags::Pure);
		class_add_func("md5", TypeString, &bytes::md5, Flags::Pure);
		class_add_func("hex", TypeString, &bytes::hex, Flags::Pure);
		class_add_func("utf8", TypeString, &KabaBytes::utf8, Flags::Pure);
		class_add_func("find", TypeIntOptional, &KabaBytes::_find, Flags::Pure);
			func_add_param("str", TypeBytes);
			func_add_param_def("start", TypeInt32, 0);
		//class_add_func(Identifier::Func::REPR, TypeString, &bytes::hex, Flags::PURE);
	//	class_add_func(Identifier::Func::FORMAT, TypeString, &KabaString::format, Flags::PURE);
	//		func_add_param("fmt", TypeString);


	add_class(TypeBoolList);
		class_add_func(Identifier::func::Str, TypeString, &BoolList::str, Flags::Pure);
		class_add_func("all", TypeBool, &BoolList::all, Flags::Pure);
		class_add_func("any", TypeBool, &BoolList::any, Flags::Pure);
		//class_add_func("__bool__", TypeBool, &BoolList::all, Flags::PURE);
		add_operator(OperatorID::And, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::None, &BoolList::and_values);
		add_operator(OperatorID::Or, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::None, &BoolList::or_values);
		// maybe bool[] == bool[] -> bool  ???
		add_operator(OperatorID::Equal, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::None, &BoolList::eq_values);
		add_operator(OperatorID::NotEqual, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::None, &BoolList::ne_values);
		add_operator(OperatorID::And, TypeBoolList, TypeBoolList, TypeBool, InlineID::None, &BoolList::and_values_scalar);
		add_operator(OperatorID::Or, TypeBoolList, TypeBoolList, TypeBool, InlineID::None, &BoolList::or_values_scalar);
		add_operator(OperatorID::Equal, TypeBoolList, TypeBoolList, TypeBool, InlineID::None, &BoolList::eq_values_scalar);
		add_operator(OperatorID::NotEqual, TypeBoolList, TypeBoolList, TypeBool, InlineID::None, &BoolList::ne_values_scalar);



	add_class(TypeIntList);
		class_add_func(Identifier::func::Str, TypeString, &XList<int>::str, Flags::Pure);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeIntList, TypeIntList, InlineID::None, &XList<int>::iadd_values);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeIntList, TypeIntList, InlineID::None, &XList<int>::isub_values);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeIntList, TypeIntList, InlineID::None, &XList<int>::imul_values);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeIntList, TypeIntList, InlineID::None, &XList<int>::idiv_values);
		add_operator(OperatorID::Add, TypeIntList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::add_values);
		add_operator(OperatorID::Subtract, TypeIntList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::sub_values);
		add_operator(OperatorID::Multiply, TypeIntList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::mul_values);
		add_operator(OperatorID::Divide, TypeIntList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::div_values);
		add_operator(OperatorID::Exponent, TypeIntList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::exp_values);
		add_operator(OperatorID::Add, TypeIntList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::add_values_scalar);
		add_operator(OperatorID::Subtract, TypeIntList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::sub_values_scalar);
		add_operator(OperatorID::Multiply, TypeIntList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::mul_values_scalar);
		add_operator(OperatorID::Divide, TypeIntList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::div_values_scalar);
		add_operator(OperatorID::Exponent, TypeIntList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::exp_values_scalar);
		add_operator(OperatorID::Assign, TypeVoid, TypeIntList, TypeInt32, InlineID::None, &XList<int>::assign_values_scalar);
		add_operator(OperatorID::Smaller, TypeBoolList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::lt_values);
		add_operator(OperatorID::SmallerEqual, TypeBoolList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::le_values);
		add_operator(OperatorID::Greater, TypeBoolList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::gt_values);
		add_operator(OperatorID::GreaterEqual, TypeBoolList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::ge_values);
		// don't we prefer  int[] == int[] -> bool ???
		add_operator(OperatorID::Equal, TypeBoolList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::eq_values);
		add_operator(OperatorID::NotEqual, TypeBoolList, TypeIntList, TypeIntList, InlineID::None, &XList<int>::ne_values);
		add_operator(OperatorID::Smaller, TypeBoolList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::lt_values_scalar);
		add_operator(OperatorID::SmallerEqual, TypeBoolList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::le_values_scalar);
		add_operator(OperatorID::Greater, TypeBoolList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::gt_values_scalar);
		add_operator(OperatorID::GreaterEqual, TypeBoolList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::ge_values_scalar);
		add_operator(OperatorID::Equal, TypeBoolList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::eq_values_scalar);
		add_operator(OperatorID::NotEqual, TypeBoolList, TypeIntList, TypeInt32, InlineID::None, &XList<int>::ne_values_scalar);
		class_add_func(Identifier::func::Contains, TypeBool, &XList<int>::__contains__, Flags::Pure);
			func_add_param("i", TypeInt32);

	add_class(TypeFloatList);
		class_add_func(Identifier::func::Str, TypeString, &XList<float>::str, Flags::Pure);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::iadd_values);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::isub_values);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::imul_values);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::idiv_values);
		add_operator(OperatorID::Add, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::add_values);
		add_operator(OperatorID::Subtract, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::sub_values);
		add_operator(OperatorID::Multiply, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::mul_values);
		add_operator(OperatorID::Divide, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::div_values);
		add_operator(OperatorID::Exponent, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::exp_values);
		add_operator(OperatorID::Add, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::add_values_scalar);
		add_operator(OperatorID::Subtract, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::sub_values_scalar);
		add_operator(OperatorID::Multiply, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::mul_values_scalar);
		add_operator(OperatorID::Divide, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::div_values_scalar);
		add_operator(OperatorID::Exponent, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::exp_values_scalar);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::iadd_values_scalar);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::isub_values_scalar);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::imul_values_scalar);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::idiv_values_scalar);
		add_operator(OperatorID::Assign, TypeVoid, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::assign_values_scalar);
		add_operator(OperatorID::Smaller, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::lt_values);
		add_operator(OperatorID::SmallerEqual, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::le_values);
		add_operator(OperatorID::Greater, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::gt_values);
		add_operator(OperatorID::GreaterEqual, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::None, &XList<float>::ge_values);
		add_operator(OperatorID::Smaller, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::lt_values_scalar);
		add_operator(OperatorID::SmallerEqual, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::le_values_scalar);
		add_operator(OperatorID::Greater, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::gt_values_scalar);
		add_operator(OperatorID::GreaterEqual, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::None, &XList<float>::ge_values_scalar);


	add_class(TypeFloat64List);
		class_add_func(Identifier::func::Str, TypeString, &XList<double>::str, Flags::Pure);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::iadd_values);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::isub_values);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::imul_values);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::idiv_values);
		add_operator(OperatorID::Add, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::add_values);
		add_operator(OperatorID::Subtract, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::sub_values);
		add_operator(OperatorID::Multiply, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::mul_values);
		add_operator(OperatorID::Divide, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::div_values);
		add_operator(OperatorID::Exponent, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::exp_values);
		add_operator(OperatorID::Add, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::add_values_scalar);
		add_operator(OperatorID::Subtract, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::sub_values_scalar);
		add_operator(OperatorID::Multiply, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::mul_values_scalar);
		add_operator(OperatorID::Divide, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::div_values_scalar);
		add_operator(OperatorID::Exponent, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::exp_values_scalar);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::iadd_values_scalar);
		add_operator(OperatorID::SubtractAssign, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::isub_values_scalar);
		add_operator(OperatorID::MultiplyAssign, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::imul_values_scalar);
		add_operator(OperatorID::DivideAssign, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::idiv_values_scalar);
		add_operator(OperatorID::Assign, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::assign_values_scalar);
		add_operator(OperatorID::Smaller, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::lt_values);
		add_operator(OperatorID::SmallerEqual, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::le_values);
		add_operator(OperatorID::Greater, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::gt_values);
		add_operator(OperatorID::GreaterEqual, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::None, &XList<double>::ge_values);
		add_operator(OperatorID::Smaller, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::lt_values_scalar);
		add_operator(OperatorID::SmallerEqual, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::le_values_scalar);
		add_operator(OperatorID::Greater, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::gt_values_scalar);
		add_operator(OperatorID::GreaterEqual, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::None, &XList<double>::ge_values_scalar);



	add_class(TypeStringList);
		class_add_func(Identifier::func::Init, TypeVoid, &StringList::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &StringList::clear, Flags::Mutable);
		class_add_func("add", TypeVoid, &StringList::add, Flags::Mutable);
			func_add_param("x", TypeString);
		class_add_func("clear", TypeVoid, &StringList::clear, Flags::Mutable);
		class_add_func("remove", TypeVoid, &StringList::erase, Flags::Mutable);
			func_add_param("index", TypeInt32);
		class_add_func("resize", TypeVoid, &StringList::resize, Flags::Mutable);
			func_add_param("num", TypeInt32);
		class_add_func("join", TypeString, &StringList::join, Flags::Pure);
			func_add_param("glue", TypeString);
		class_add_func(Identifier::func::Str, TypeString, &StringList::str, Flags::Pure);
		add_operator(OperatorID::Assign, TypeVoid, TypeStringList, TypeStringList, InlineID::None, &StringList::assign);
		add_operator(OperatorID::Equal, TypeBool, TypeStringList, TypeStringList, InlineID::None, &StringList::__eq__);
		add_operator(OperatorID::NotEqual, TypeBool, TypeStringList, TypeStringList, InlineID::None, &StringList::__neq__);
		add_operator(OperatorID::Add, TypeStringList, TypeStringList, TypeStringList, InlineID::None, &StringList::__add__);
		add_operator(OperatorID::AddAssign, TypeVoid, TypeStringList, TypeStringList, InlineID::None, &StringList::__adds__);
		class_add_func(Identifier::func::Contains, TypeBool, &StringList::__contains__, Flags::Pure);
			func_add_param("s", TypeString);



	// constants
	void *kaba_nil = nullptr;
	bool kaba_true = true;
	bool kaba_false = false;
	add_const("nil", TypeNone, &kaba_nil);
	add_const("false", TypeBool, &kaba_false);
	add_const("true",  TypeBool, &kaba_true);


	add_class(TypeException);
		class_add_func(Identifier::func::Init, TypeVoid, &KabaException::__init__, Flags::Mutable);
			func_add_param("message", TypeString);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, &KabaException::__delete__, Flags::Mutable);
		class_add_func_virtual(Identifier::func::Str, TypeString, &KabaException::message);
		class_add_element("_text", TypeString, config.target.pointer_size);
		class_set_vtable(KabaException);

	add_class(TypeNoValueError);
		class_derive_from(TypeException);
		class_add_func(Identifier::func::Init, TypeVoid, &KabaNoValueError::__init__, Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, &KabaNoValueError::__delete__, Flags::Override | Flags::Mutable);
		class_set_vtable(KabaNoValueError);

	add_func(Identifier::Raise, TypeVoid, &kaba_raise_exception, Flags::Static | Flags::RaisesExceptions);
		func_add_param("e", TypeExceptionXfer);
	add_func("@die", TypeVoid, &kaba_die, Flags::Static | Flags::RaisesExceptions);
		func_add_param("e", TypePointer);
	add_func(Identifier::Assert, TypeVoid, &kaba_assert, Flags::Static | Flags::RaisesExceptions);
		func_add_param("b", TypeBool);


	// type casting
	add_func("p2s", TypeString, &p2s, Flags::Static | Flags::Pure);
		func_add_param("p", TypePointer);
	add_func("char", TypeString, &kaba_char2str, Flags::Static | Flags::Pure);
		func_add_param("c", TypeInt32);
	add_func("hex", TypeString, &kaba_int32_hex, Flags::Static | Flags::Pure);
		func_add_param("i", TypeInt32);
	add_func("hex", TypeString, &kaba_int64_hex, Flags::Static | Flags::Pure);
		func_add_param("i", TypeInt64);
	// debug output
	/*add_func("cprint", TypeVoid, &_cstringout, Flags::STATIC);
		func_add_param("str", TypeCString);*/
	add_func("print", TypeVoid, &os::terminal::print, Flags::Static);
		func_add_param("str", TypeStringAutoCast);//, (Flags)((int)Flags::CONST | (int)Flags::AUTO_CAST));
	add_ext_var("_print_postfix", TypeString, &os::terminal::_print_postfix_);
	add_func("as_binary", TypeBytes, &kaba_binary, Flags::Static);
		func_add_param("p", TypeReference, Flags::Ref);
		func_add_param("length", TypeInt32);
	// memory
	add_func("@malloc", TypePointer, &kaba_malloc, Flags::Static);
		func_add_param("size", TypeInt32);
	add_func("@free", TypeVoid, &free, Flags::Static);
		func_add_param("p", TypePointer);

	// basic testing
	add_func("_ping", TypeVoid, &kaba_ping, Flags::Static);
	add_func("_int_out", TypeVoid, &kaba_int_out, Flags::Static);
		func_add_param("i", TypeInt32);
	add_func("_float_out", TypeVoid, &kaba_float_out, Flags::Static);
		func_add_param("f", TypeFloat32);
	add_func("_call_float", TypeVoid, &_x_call_float, Flags::Static);
	add_func("_float_ret", TypeFloat32, &kaba_float_ret, Flags::Static);
	add_func("_int_ret", TypeInt32, &kaba_int_ret, Flags::Static);
	add_func("_xxx", TypeVoid, &kaba_xxx, Flags::Static);
		func_add_param("a", TypeInt32);
		func_add_param("b", TypeInt32);
		func_add_param("c", TypeInt32);
		func_add_param("d", TypeInt32);
		func_add_param("e", TypeInt32);
		func_add_param("f", TypeInt32);
		func_add_param("g", TypeInt32);
		func_add_param("h", TypeInt32);
	add_ext_var("_extern_variable", TypeInt32, &extern_variable1);


	add_type_cast(10, TypeInt32, TypeFloat32, "i32.__f32__");
	add_type_cast(10, TypeInt32, TypeFloat64, "i32.__f64__");
	add_type_cast(10, TypeInt32, TypeInt64, "i32.__i64__");
	add_type_cast(200, TypeInt64, TypeInt32, "i64.__i32__");
	add_type_cast(10, TypeFloat32, TypeFloat64,"f32.__f64__");
	add_type_cast(200, TypeFloat32, TypeInt32, "f32.__i32__");
	add_type_cast(200, TypeInt32, TypeInt8, "i32.__i8__");
	add_type_cast(20, TypeInt8, TypeInt32, "i8.__i32__");
	add_type_cast(200, TypeInt32, TypeUInt8, "i32.__u8__");
	add_type_cast(20, TypeUInt8, TypeInt32, "u8.__i32__");
	//add_type_cast(30, TypeBoolList, TypeBool, "bool[].__bool__");
	add_type_cast(50, TypePointer, TypeBool, "p2b");
	//add_type_cast(50, TypePointer, TypeString, "p2s");
}



}
