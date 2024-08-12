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
#include <algorithm>
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
const Class *TypeFutureT;
const Class *TypeFutureCoreT;
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

string kaba_i16_to_str(unsigned short w) {
	return str((int)w);
}

void kaba_i16_from_i32(unsigned short& w, int i) {
	w = i;
}

string kaba_float2str(float f) {
	return f2s(f, 6);
}

string kaba_float_format(float f, const string &fmt) {
	try {
		return _xf_str_<float>(fmt + "f", f);
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




void SIAddXCommands(Context *c) {

	add_func("@sorted", TypeDynamicArray, &array_sort, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("list", TypePointer);
		func_add_param("class", TypeClassRef);
		func_add_param("by", TypeString);
	add_func("@var2str", TypeString, &var2str, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassRef);
	add_func("@var_repr", TypeString, &var_repr, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassRef);
	add_func("@dyn", TypeAny, &dynify, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("var", TypePointer);
		func_add_param("class", TypeClassRef);
}

void SIAddPackageBase(Context *c) {
	add_package(c, "base", Flags::AUTO_IMPORT);

	// internal
	TypeUnknown			= add_type  ("@unknown", 0); // should not appear anywhere....or else we're screwed up!
	TypeReg128			= add_type_simple  ("@reg128", 16, 16, Flags::FORCE_CALL_BY_VALUE);
	TypeReg64			= add_type_simple  ("@reg64", 8, 8, Flags::FORCE_CALL_BY_VALUE);
	TypeReg32			= add_type_simple  ("@reg32", 4, 4, Flags::FORCE_CALL_BY_VALUE);
	TypeReg16			= add_type_simple  ("@reg16", 2, 2, Flags::FORCE_CALL_BY_VALUE);
	TypeReg8			= add_type_simple  ("@reg8", 1, 1, Flags::FORCE_CALL_BY_VALUE);
	TypeObject			= add_type  ("Object", sizeof(VirtualBase)); // base for most virtual classes
	TypeDynamic			= add_type  ("@dynamic", 0);

	// "real"
	TypeVoid			= add_type_simple  ("void", 0, 1, Flags::FORCE_CALL_BY_VALUE);
	TypeBool			= add_type_simple  ("bool", sizeof(bool), 1, Flags::FORCE_CALL_BY_VALUE);
	TypeInt8			= add_type_simple  ("i8", 1, 1, Flags::FORCE_CALL_BY_VALUE);
	TypeUInt8			= add_type_simple  ("u8", 1, 1, Flags::FORCE_CALL_BY_VALUE);
	TypeInt16			= add_type_simple  ("i16", 2, 2, Flags::FORCE_CALL_BY_VALUE);
	TypeInt32			= add_type_simple  ("i32", sizeof(int32), 4, Flags::FORCE_CALL_BY_VALUE);
	TypeInt64			= add_type_simple  ("i64", sizeof(int64), 8, Flags::FORCE_CALL_BY_VALUE);
	TypeFloat32			= add_type_simple  ("f32", sizeof(float), 4, Flags::FORCE_CALL_BY_VALUE);
	TypeFloat64			= add_type_simple  ("f64", sizeof(double), 8, Flags::FORCE_CALL_BY_VALUE);
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
	TypeFutureCoreT = add_class_template("@FutureCore", {"T"}, new TemplateClassInstantiatorFutureCore);
	TypeFutureT = add_class_template("future", {"T"}, new TemplateClassInstantiatorFuture);
	TypeStructT = add_class_template("@Struct", {"T"}, nullptr);
	TypeEnumT = add_class_template("@Enum", {"T"}, new TemplateClassInstantiatorEnum);
	TypeInterfaceT = add_class_template("@Interface", {"T"}, nullptr);
	TypeNamespaceT = add_class_template("@Namespace", {"T"}, nullptr);


	add_class(TypeObject);
		class_add_func(Identifier::Func::INIT, TypeVoid, &_VirtualBase::__init__, Flags::MUTABLE);
		class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &VirtualBase::__delete__, Flags::MUTABLE);
		class_set_vtable(VirtualBase);

	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt32, config.target.pointer_size);
		class_add_func("swap", TypeVoid, &DynamicArray::simple_swap, Flags::MUTABLE);
			func_add_param("i1", TypeInt32);
			func_add_param("i2", TypeInt32);
		class_add_func(Identifier::Func::SUBARRAY, TypeDynamicArray, &DynamicArray::ref_subarray, Flags::REF);
			func_add_param("start", TypeInt32);
			func_add_param("end", TypeInt32);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init, Flags::MUTABLE);
			func_add_param("element_size", TypeInt32);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear, Flags::MUTABLE);
		class_add_func("__mem_forget__", TypeVoid, &DynamicArray::forget, Flags::MUTABLE);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize, Flags::MUTABLE);
			func_add_param("size", TypeInt32);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single, Flags::MUTABLE);
			func_add_param("index", TypeInt32);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt32, config.target.pointer_size);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init, Flags::MUTABLE);
			func_add_param("element_size", TypeInt32);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear, Flags::MUTABLE);
		class_add_func("__mem_forget__", TypeVoid, &DynamicArray::forget, Flags::MUTABLE);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize, Flags::MUTABLE);
			func_add_param("size", TypeInt32);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single, Flags::MUTABLE);
			func_add_param("index", TypeInt32);

	add_class(TypeSharedPointer);
		class_add_func(Identifier::Func::INIT, TypeVoid, nullptr, Flags::MUTABLE);
			func_set_inline(InlineID::SHARED_POINTER_INIT);


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


	lib_create_dict<int>(TypeIntDict);
	lib_create_dict<float>(TypeFloatDict);
	lib_create_dict<string>(TypeStringDict);


	lib_create_optional<int>(TypeIntOptional);


	add_class(TypeCallableBase);
		class_add_element("_fp", TypePointer, &KabaCallableBase::fp);
		class_add_element("_pp", TypePointer, &KabaCallableBase::pp);
		//class_add_func(Identifier::Func::INIT, TypeVoid, &KabaCallableBase::__init__);
		class_add_func(Identifier::Func::ASSIGN, TypeVoid, nullptr, Flags::MUTABLE);
			func_set_inline(InlineID::CHUNK_ASSIGN);
		class_add_func_virtual("call", TypeVoid, &KabaCallableBase::operator());
	


	add_func("p2b", TypeBool, &kaba_cast<void*,bool>, Flags::STATIC | Flags::PURE);
		func_set_inline(InlineID::POINTER_TO_BOOL);
		func_add_param("p", TypePointer);
	

	add_class(TypePointer);
		class_add_func(Identifier::Func::STR, TypeString, &p2s, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypePointer, TypePointer, InlineID::POINTER_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypePointer, TypePointer, InlineID::POINTER_NOT_EQUAL);


	add_class(TypeReference);
		add_operator(OperatorID::REF_ASSIGN, TypeVoid, TypeReference, TypeReference, InlineID::POINTER_ASSIGN);


	add_class(TypeBool);
		class_add_func(Identifier::Func::STR, TypeString, &b2s, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeBool, TypeBool, InlineID::BOOL_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeBool, TypeBool, InlineID::BOOL_NOT_EQUAL);
		add_operator(OperatorID::AND, TypeBool, TypeBool, TypeBool, InlineID::BOOL_AND);
		add_operator(OperatorID::OR, TypeBool, TypeBool, TypeBool, InlineID::BOOL_OR);
		add_operator(OperatorID::NEGATE, TypeBool, nullptr, TypeBool, InlineID::BOOL_NOT);


	add_class(TypeUInt8);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_uint8_to_str, Flags::PURE);
		//class_add_func(Identifier::Func::REPR, TypeString, &kaba_char_repr, Flags::PURE);
		class_add_func("__i32__", TypeInt32, &kaba_cast<uint8,int>, Flags::PURE);
			func_set_inline(InlineID::UINT8_TO_INT32);
		class_add_func("__i8__", TypeInt8, &kaba_cast<uint8,int8>, Flags::PURE);
			func_set_inline(InlineID::PASSTHROUGH);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeUInt8, TypeUInt8, InlineID::INT8_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeUInt8, TypeUInt8, InlineID::INT8_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeUInt8, TypeUInt8, InlineID::INT8_NOT_EQUAL);
		add_operator(OperatorID::GREATER, TypeBool, TypeUInt8, TypeUInt8, InlineID::UINT8_GREATER);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeUInt8, TypeUInt8, InlineID::UINT8_GREATER_EQUAL);
		add_operator(OperatorID::SMALLER, TypeBool, TypeUInt8, TypeUInt8, InlineID::UINT8_SMALLER);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeUInt8, TypeUInt8, InlineID::UINT8_SMALLER_EQUAL);
		add_operator(OperatorID::ADD, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::INT8_ADD);
		add_operator(OperatorID::SUBTRACTS, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::INT8_SUBTRACT_ASSIGN);
		add_operator(OperatorID::ADDS, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::INT8_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACT, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::INT8_SUBTRACT);
		add_operator(OperatorID::BIT_AND, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::INT8_AND);
		add_operator(OperatorID::BIT_OR, TypeUInt8, TypeUInt8, TypeUInt8, InlineID::INT8_OR);
		add_operator(OperatorID::NEGATIVE, TypeUInt8, nullptr, TypeUInt8, InlineID::INT8_NEGATIVE);


	add_class(TypeInt8);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_int8_to_str, Flags::PURE);
		//class_add_func(Identifier::Func::REPR, TypeString, &kaba_char_repr, Flags::PURE);
		class_add_func("__i32__", TypeInt32, &kaba_cast<char,int>, Flags::PURE);
			func_set_inline(InlineID::INT8_TO_INT32);
		class_add_func("__u8__", TypeInt8, &kaba_cast<int8,uint8>, Flags::PURE);
			func_set_inline(InlineID::PASSTHROUGH);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt8, TypeInt8, InlineID::INT8_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt8, TypeInt8, InlineID::INT8_EQUAL);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeInt8, TypeInt8, InlineID::INT8_NOT_EQUAL);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt8, TypeInt8, InlineID::INT8_GREATER);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt8, TypeInt8, InlineID::INT8_GREATER_EQUAL);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt8, TypeInt8, InlineID::INT8_SMALLER);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt8, TypeInt8, InlineID::INT8_SMALLER_EQUAL);
		add_operator(OperatorID::ADD, TypeInt8, TypeInt8, TypeInt8, InlineID::INT8_ADD);
		add_operator(OperatorID::SUBTRACTS, TypeInt8, TypeInt8, TypeInt8, InlineID::INT8_SUBTRACT_ASSIGN);
		add_operator(OperatorID::ADDS, TypeInt8, TypeInt8, TypeInt8, InlineID::INT8_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACT, TypeInt8, TypeInt8, TypeInt8, InlineID::INT8_SUBTRACT);
		add_operator(OperatorID::BIT_AND, TypeInt8, TypeInt8, TypeInt8, InlineID::INT8_AND);
		add_operator(OperatorID::BIT_OR, TypeInt8, TypeInt8, TypeInt8, InlineID::INT8_OR);
		add_operator(OperatorID::NEGATIVE, TypeInt8, nullptr, TypeInt8, InlineID::INT8_NEGATIVE);


	add_class(TypeInt16);
		class_add_element("low", TypeUInt8, 0);
		class_add_element("high", TypeUInt8, 1);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_i16_to_str, Flags::PURE);
		class_add_func("__i32__", TypeInt32, &kaba_cast<unsigned short,int>, Flags::PURE);
		//	func_set_inline(InlineID::INT16_TO_INT32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt16, TypeInt16, InlineID::CHUNK_ASSIGN);
		//add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt16, TypeInt32, InlineID::INT16_ASSIGN_INT32);
		class_add_func("__assign__", TypeVoid, &kaba_i16_from_i32, Flags::MUTABLE);
			func_add_param("o", TypeInt32);


	add_class(TypeInt32);
		class_add_func(Identifier::Func::STR, TypeString, &i2s, Flags::PURE);
		class_add_func(Identifier::Func::FORMAT, TypeString, &kaba_int_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__f32__", TypeFloat32, &kaba_cast<int,float>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_FLOAT32);
		class_add_func("__f64__", TypeFloat64, &kaba_cast<int,double>, Flags::PURE);
		class_add_func("__i8__", TypeInt8, &kaba_cast<int,int8>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_INT8);
		class_add_func("__u8__", TypeUInt8, &kaba_cast<int,uint8>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_UINT8);
		class_add_func("__i64__", TypeInt64, &kaba_cast<int,int64>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_INT64);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt32, TypeInt32, InlineID::INT32_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_ADD, &op_int_add);
		add_operator(OperatorID::SUBTRACT, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_SUBTRACT, &op_int_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_MULTIPLY, &op_int_mul);
		add_operator(OperatorID::DIVIDE, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_DIVIDE, &op_int_div);
		add_operator(OperatorID::EXPONENT, TypeInt32, TypeInt32, TypeInt32, InlineID::NONE, &xop_exp<int>);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt32, TypeInt32, InlineID::INT32_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt32, TypeInt32, InlineID::INT32_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt32, TypeInt32, InlineID::INT32_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt32, TypeInt32, InlineID::INT32_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_MODULO, &op_int32_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt32, TypeInt32, InlineID::INT32_EQUAL, &op_int_eq);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeInt32, TypeInt32, InlineID::INT32_NOT_EQUAL, &op_int_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt32, TypeInt32, InlineID::INT32_GREATER, &op_int_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt32, TypeInt32, InlineID::INT32_GREATER_EQUAL, &op_int_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt32, TypeInt32, InlineID::INT32_SMALLER, &op_int_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt32, TypeInt32, InlineID::INT32_SMALLER_EQUAL, &op_int_le);
		add_operator(OperatorID::BIT_AND, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_AND, &op_int32_and);
		add_operator(OperatorID::BIT_OR, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_OR, &op_int32_or);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_SHIFT_RIGHT, &op_int32_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt32, TypeInt32, TypeInt32, InlineID::INT32_SHIFT_LEFT, &op_int32_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt32, nullptr, TypeInt32, InlineID::INT32_NEGATIVE, &op_int_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt32, nullptr, InlineID::INT32_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt32, nullptr, InlineID::INT32_DECREASE);

	add_class(TypeInt64);
		class_add_func(Identifier::Func::STR, TypeString, &i642s, Flags::PURE);
		class_add_func("__i32__", TypeInt32, &kaba_cast<int64,int>, Flags::PURE);
			func_set_inline(InlineID::INT64_TO_INT32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_ADD, &op_int64_add);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt32, InlineID::INT64_ADD_INT32, &op_int64_add_int); // needed by internal address calculations!
		add_operator(OperatorID::SUBTRACT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT, &op_int64_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY, &op_int64_mul);
		add_operator(OperatorID::DIVIDE, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE, &op_int64_div);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_MODULO, &op_int64_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_EQUAL, &op_int64_eq);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_NOT_EQUAL, &op_int64_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER, &op_int64_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_GREATER_EQUAL, &op_int64_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER, &op_int64_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt64, TypeInt64, InlineID::INT64_SMALLER_EQUAL, &op_int64_le);
		add_operator(OperatorID::BIT_AND, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_AND, &op_int64_and);
		add_operator(OperatorID::BIT_OR, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_OR, &op_int64_or);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_RIGHT, &op_int64_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_LEFT, &op_int64_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt64, nullptr, TypeInt64, InlineID::INT64_NEGATIVE, &op_int64_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_DECREASE);

	add_class(TypeFloat32);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_float2str, Flags::PURE);
		class_add_func("str2", TypeString, &f2s, Flags::PURE);
			func_add_param("decimals", TypeInt32);
		class_add_func(Identifier::Func::FORMAT, TypeString, &kaba_float_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__i32__", TypeInt32, &kaba_cast<float,int>, Flags::PURE);
			func_set_inline(InlineID::FLOAT32_TO_INT32);    // sometimes causes floating point exceptions...
		class_add_func("__f64__", TypeFloat64, &kaba_cast<float,double>, Flags::PURE);
			func_set_inline(InlineID::FLOAT32_TO_FLOAT64);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT32_ASSIGN);
		add_operator(OperatorID::ADD, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT32_ADD, &op_float_add);
		add_operator(OperatorID::SUBTRACT, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT32_SUBTARCT, &op_float_sub);
		add_operator(OperatorID::MULTIPLY, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT32_MULTIPLY, &op_float_mul);
		add_operator(OperatorID::DIVIDE, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::FLOAT32_DIVIDE, &op_float_div);
		add_operator(OperatorID::EXPONENT, TypeFloat32, TypeFloat32, TypeFloat32, InlineID::NONE, &xop_exp<float>);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT32_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT32_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT32_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat32, TypeFloat32, InlineID::FLOAT32_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT32_EQUAL, &op_float_eq);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT32_NOT_EQUAL, &op_float_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT32_GREATER, &op_float_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT32_GREATER_EQUAL, &op_float_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT32_SMALLER, &op_float_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat32, TypeFloat32, InlineID::FLOAT32_SMALLER_EQUAL, &op_float_le);
		add_operator(OperatorID::NEGATIVE, TypeFloat32, nullptr, TypeFloat32, InlineID::FLOAT32_NEGATIVE, &op_float_neg);


	add_class(TypeFloat64);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_float642str, Flags::PURE);
		class_add_func("__f32__", TypeFloat32, &kaba_cast<double,float>, Flags::PURE);
			func_set_inline(InlineID::FLOAT64_TO_FLOAT32);
		class_add_func("__i32__", TypeInt32, &kaba_cast<double,int>, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD, &op_double_add);
		add_operator(OperatorID::SUBTRACT, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT, &op_double_sub);
		add_operator(OperatorID::MULTIPLY, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY, &op_double_mul);
		add_operator(OperatorID::DIVIDE, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE, &op_double_div);
		add_operator(OperatorID::EXPONENT, TypeFloat64, TypeFloat64, TypeFloat64, InlineID::NONE, &xop_exp<double>);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat64, TypeFloat64, InlineID::FLOAT64_DIVIDE_ASSIGN);
		add_operator(OperatorID::EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_EQUAL, &op_double_eq);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_NOT_EQUAL, &op_double_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER, &op_double_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_GREATER_EQUAL, &op_double_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER, &op_double_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeFloat64, TypeFloat64, InlineID::FLOAT64_SMALLER_EQUAL, &op_double_le);
		add_operator(OperatorID::NEGATIVE, TypeFloat32, nullptr, TypeFloat64, InlineID::FLOAT64_NEGATIVE, &op_double_neg);


	add_class(TypeString);
		add_operator(OperatorID::ADDS, TypeVoid, TypeString, TypeString, InlineID::NONE, &string::operator+=);
		add_operator(OperatorID::ADD, TypeString, TypeString, TypeString, InlineID::NONE, &string::operator+);
		add_operator(OperatorID::EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator==);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator!=);
		add_operator(OperatorID::SMALLER, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator<);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator<=);
		add_operator(OperatorID::GREATER, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator>);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeString, TypeString, InlineID::NONE, &string::operator>=);
		class_add_func("head", TypeString, &string::head, Flags::PURE);
			func_add_param("size", TypeInt32);
		class_add_func("tail", TypeString, &string::tail, Flags::PURE);
			func_add_param("size", TypeInt32);
		class_add_func("find", TypeIntOptional, &KabaString::_find, Flags::PURE);
			func_add_param("str", TypeString);
			func_add_param_def("start", TypeInt32, 0);
		class_add_func("compare", TypeInt32, &string::compare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("icompare", TypeInt32, &string::icompare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("replace", TypeString, &string::replace, Flags::PURE);
			func_add_param("sub", TypeString);
			func_add_param("by", TypeString);
		class_add_func("explode", TypeStringList, &string::explode, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("parse_tokens", TypeStringList, &string::parse_tokens, Flags::PURE);
			func_add_param("splitters", TypeString);
		class_add_func("repeat", TypeString, &string::repeat, Flags::PURE);
			func_add_param("n", TypeInt32);
		class_add_func("lower", TypeString, &string::lower, Flags::PURE);
		class_add_func("upper", TypeString, &string::upper, Flags::PURE);
		class_add_func("reverse", TypeString, &string::reverse, Flags::PURE);
		class_add_func("hash", TypeInt32, &string::hash, Flags::PURE);
		class_add_func("hex", TypeString, &string::hex, Flags::PURE);
		class_add_func("unhex", TypeBytes, &string::unhex, Flags::PURE);
		class_add_func("match", TypeBool, &string::match, Flags::PURE);
			func_add_param("glob", TypeString);
		class_add_func("__i32__", TypeInt32, &string::_int, Flags::PURE);
		class_add_func("__i64__", TypeInt64, &string::i64, Flags::PURE);
		class_add_func("__f32__", TypeFloat32, &string::_float, Flags::PURE);
		class_add_func("__f64__", TypeFloat64, &string::f64, Flags::PURE);
		class_add_func("trim", TypeString, &string::trim, Flags::PURE);
		class_add_func("escape", TypeString, &string::escape, Flags::PURE);
		class_add_func("unescape", TypeString, &string::unescape, Flags::PURE);
		class_add_func("utf8_to_utf32", TypeIntList, &string::utf8_to_utf32, Flags::PURE);
		class_add_func("utf8_length", TypeInt32, &string::utf8len, Flags::PURE);
		class_add_func("encode", TypeBytes, &KabaString::encode, Flags::PURE);
		class_add_func("decode", TypeString, &KabaString::decode, Flags::PURE | Flags::STATIC);
			func_add_param("b", TypeBytes);
		class_add_func(Identifier::Func::REPR, TypeString, &string::repr, Flags::PURE);
		class_add_func(Identifier::Func::FORMAT, TypeString, &KabaString::format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func(Identifier::Func::CONTAINS, TypeBool, &KabaString::contains_s, Flags::PURE);
			func_add_param("s", TypeString);


	add_class(TypeBytes);
		add_operator(OperatorID::EQUAL, TypeBool, TypeBytes, TypeBytes, InlineID::NONE, &bytes::operator==);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeBytes, TypeBytes, InlineID::NONE, &bytes::operator!=);
		class_add_func("reverse", TypeBytes, &kaba_bytes_reverse, Flags::PURE);
		class_add_func("hash", TypeInt32, &bytes::hash, Flags::PURE);
		class_add_func("md5", TypeString, &bytes::md5, Flags::PURE);
		class_add_func("hex", TypeString, &bytes::hex, Flags::PURE);
		class_add_func("utf8", TypeString, &KabaBytes::utf8, Flags::PURE);
		class_add_func("find", TypeIntOptional, &KabaBytes::_find, Flags::PURE);
			func_add_param("str", TypeBytes);
			func_add_param_def("start", TypeInt32, 0);
		//class_add_func(Identifier::Func::REPR, TypeString, &bytes::hex, Flags::PURE);
	//	class_add_func(Identifier::Func::FORMAT, TypeString, &KabaString::format, Flags::PURE);
	//		func_add_param("fmt", TypeString);


	add_class(TypeBoolList);
		class_add_func(Identifier::Func::STR, TypeString, &BoolList::str, Flags::PURE);
		class_add_func("all", TypeBool, &BoolList::all, Flags::PURE);
		class_add_func("any", TypeBool, &BoolList::any, Flags::PURE);
		//class_add_func("__bool__", TypeBool, &BoolList::all, Flags::PURE);
		add_operator(OperatorID::AND, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::and_values);
		add_operator(OperatorID::OR, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::or_values);
		// maybe bool[] == bool[] -> bool  ???
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::eq_values);
		add_operator(OperatorID::NOT_EQUAL, TypeBoolList, TypeBoolList, TypeBoolList, InlineID::NONE, &BoolList::ne_values);
		add_operator(OperatorID::AND, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::and_values_scalar);
		add_operator(OperatorID::OR, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::or_values_scalar);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::eq_values_scalar);
		add_operator(OperatorID::NOT_EQUAL, TypeBoolList, TypeBoolList, TypeBool, InlineID::NONE, &BoolList::ne_values_scalar);

	
	
	add_class(TypeIntList);
		class_add_func(Identifier::Func::STR, TypeString, &XList<int>::str, Flags::PURE);
		add_operator(OperatorID::ADDS, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::iadd_values);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::isub_values);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::imul_values);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::idiv_values);
		add_operator(OperatorID::ADD, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::add_values);
		add_operator(OperatorID::SUBTRACT, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::sub_values);
		add_operator(OperatorID::MULTIPLY, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::mul_values);
		add_operator(OperatorID::DIVIDE, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::div_values);
		add_operator(OperatorID::EXPONENT, TypeIntList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::exp_values);
		add_operator(OperatorID::ADD, TypeIntList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::add_values_scalar);
		add_operator(OperatorID::SUBTRACT, TypeIntList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::sub_values_scalar);
		add_operator(OperatorID::MULTIPLY, TypeIntList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::mul_values_scalar);
		add_operator(OperatorID::DIVIDE, TypeIntList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::div_values_scalar);
		add_operator(OperatorID::EXPONENT, TypeIntList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::exp_values_scalar);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::assign_values_scalar);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::lt_values);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::le_values);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::gt_values);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::ge_values);
		// don't we prefer  int[] == int[] -> bool ???
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::eq_values);
		add_operator(OperatorID::NOT_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::ne_values);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::lt_values_scalar);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::le_values_scalar);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::gt_values_scalar);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::ge_values_scalar);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::eq_values_scalar);
		add_operator(OperatorID::NOT_EQUAL, TypeBoolList, TypeIntList, TypeInt32, InlineID::NONE, &XList<int>::ne_values_scalar);
		class_add_func(Identifier::Func::CONTAINS, TypeBool, &XList<int>::__contains__, Flags::PURE);
			func_add_param("i", TypeInt32);

	add_class(TypeFloatList);
		class_add_func(Identifier::Func::STR, TypeString, &XList<float>::str, Flags::PURE);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::iadd_values);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::isub_values);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::imul_values);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::idiv_values);
		add_operator(OperatorID::ADD, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::add_values);
		add_operator(OperatorID::SUBTRACT, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::sub_values);
		add_operator(OperatorID::MULTIPLY, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::mul_values);
		add_operator(OperatorID::DIVIDE, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::div_values);
		add_operator(OperatorID::EXPONENT, TypeFloatList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::exp_values);
		add_operator(OperatorID::ADD, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::add_values_scalar);
		add_operator(OperatorID::SUBTRACT, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::sub_values_scalar);
		add_operator(OperatorID::MULTIPLY, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::mul_values_scalar);
		add_operator(OperatorID::DIVIDE, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::div_values_scalar);
		add_operator(OperatorID::EXPONENT, TypeFloatList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::exp_values_scalar);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::iadd_values_scalar);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::isub_values_scalar);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::imul_values_scalar);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::idiv_values_scalar);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::assign_values_scalar);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::lt_values);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::le_values);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::gt_values);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeFloatList, TypeFloatList, InlineID::NONE, &XList<float>::ge_values);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::lt_values_scalar);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::le_values_scalar);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::gt_values_scalar);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeFloatList, TypeFloat32, InlineID::NONE, &XList<float>::ge_values_scalar);


	add_class(TypeFloat64List);
		class_add_func(Identifier::Func::STR, TypeString, &XList<double>::str, Flags::PURE);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::iadd_values);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::isub_values);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::imul_values);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::idiv_values);
		add_operator(OperatorID::ADD, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::add_values);
		add_operator(OperatorID::SUBTRACT, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::sub_values);
		add_operator(OperatorID::MULTIPLY, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::mul_values);
		add_operator(OperatorID::DIVIDE, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::div_values);
		add_operator(OperatorID::EXPONENT, TypeFloat64List, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::exp_values);
		add_operator(OperatorID::ADD, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::add_values_scalar);
		add_operator(OperatorID::SUBTRACT, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::sub_values_scalar);
		add_operator(OperatorID::MULTIPLY, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::mul_values_scalar);
		add_operator(OperatorID::DIVIDE, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::div_values_scalar);
		add_operator(OperatorID::EXPONENT, TypeFloat64List, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::exp_values_scalar);
		add_operator(OperatorID::ADDS, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::iadd_values_scalar);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::isub_values_scalar);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::imul_values_scalar);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::idiv_values_scalar);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::assign_values_scalar);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::lt_values);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::le_values);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::gt_values);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeFloat64List, TypeFloat64List, InlineID::NONE, &XList<double>::ge_values);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::lt_values_scalar);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::le_values_scalar);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::gt_values_scalar);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeFloat64List, TypeFloat64, InlineID::NONE, &XList<double>::ge_values_scalar);



	add_class(TypeStringList);
		class_add_func(Identifier::Func::INIT, TypeVoid, &StringList::__init__, Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &StringList::clear, Flags::MUTABLE);
		class_add_func("add", TypeVoid, &StringList::add, Flags::MUTABLE);
			func_add_param("x", TypeString);
		class_add_func("clear", TypeVoid, &StringList::clear, Flags::MUTABLE);
		class_add_func("remove", TypeVoid, &StringList::erase, Flags::MUTABLE);
			func_add_param("index", TypeInt32);
		class_add_func("resize", TypeVoid, &StringList::resize, Flags::MUTABLE);
			func_add_param("num", TypeInt32);
		class_add_func("join", TypeString, &StringList::join, Flags::PURE);
			func_add_param("glue", TypeString);
		class_add_func(Identifier::Func::STR, TypeString, &StringList::str, Flags::PURE);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeStringList, TypeStringList, InlineID::NONE, &StringList::assign);
		add_operator(OperatorID::EQUAL, TypeBool, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__eq__);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__neq__);
		add_operator(OperatorID::ADD, TypeStringList, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__add__);
		add_operator(OperatorID::ADDS, TypeVoid, TypeStringList, TypeStringList, InlineID::NONE, &StringList::__adds__);
		class_add_func(Identifier::Func::CONTAINS, TypeBool, &StringList::__contains__, Flags::PURE);
			func_add_param("s", TypeString);



	// constants
	void *kaba_nil = nullptr;
	bool kaba_true = true;
	bool kaba_false = false;
	add_const("nil", TypeNone, &kaba_nil);
	add_const("false", TypeBool, &kaba_false);
	add_const("true",  TypeBool, &kaba_true);


	add_class(TypeException);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaException::__init__, Flags::MUTABLE);
			func_add_param("message", TypeString);
		class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &KabaException::__delete__, Flags::MUTABLE);
		class_add_func_virtual(Identifier::Func::STR, TypeString, &KabaException::message);
		class_add_element("_text", TypeString, config.target.pointer_size);
		class_set_vtable(KabaException);

	add_class(TypeNoValueError);
		class_derive_from(TypeException);
		class_add_func(Identifier::Func::INIT, TypeVoid, &KabaNoValueError::__init__, Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, &KabaNoValueError::__delete__, Flags::OVERRIDE | Flags::MUTABLE);
		class_set_vtable(KabaNoValueError);

	add_func(Identifier::RAISE, TypeVoid, &kaba_raise_exception, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("e", TypeExceptionXfer);
	add_func("@die", TypeVoid, &kaba_die, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("e", TypePointer);
		
		
	// type casting
	add_func("p2s", TypeString, &p2s, Flags::STATIC | Flags::PURE);
		func_add_param("p", TypePointer);
	add_func("char", TypeString, &kaba_char2str, Flags::STATIC | Flags::PURE);
		func_add_param("c", TypeInt32);
	add_func("hex", TypeString, &kaba_int32_hex, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt32);
	add_func("hex", TypeString, &kaba_int64_hex, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt64);
	// debug output
	/*add_func("cprint", TypeVoid, &_cstringout, Flags::STATIC);
		func_add_param("str", TypeCString);*/
	add_func("print", TypeVoid, &os::terminal::print, Flags::STATIC);
		func_add_param("str", TypeStringAutoCast);//, (Flags)((int)Flags::CONST | (int)Flags::AUTO_CAST));
	add_ext_var("_print_postfix", TypeString, &os::terminal::_print_postfix_);
	add_func("as_binary", TypeBytes, &kaba_binary, Flags::STATIC);
		func_add_param("p", TypeReference, Flags::REF);
		func_add_param("length", TypeInt32);
	// memory
	add_func("@malloc", TypePointer, &kaba_malloc, Flags::STATIC);
		func_add_param("size", TypeInt32);
	add_func("@free", TypeVoid, &free, Flags::STATIC);
		func_add_param("p", TypePointer);

	// basic testing
	add_func("_ping", TypeVoid, &kaba_ping, Flags::STATIC);
	add_func("_int_out", TypeVoid, &kaba_int_out, Flags::STATIC);
		func_add_param("i", TypeInt32);
	add_func("_float_out", TypeVoid, &kaba_float_out, Flags::STATIC);
		func_add_param("f", TypeFloat32);
	add_func("_call_float", TypeVoid, &_x_call_float, Flags::STATIC);
	add_func("_float_ret", TypeFloat32, &kaba_float_ret, Flags::STATIC);
	add_func("_int_ret", TypeInt32, &kaba_int_ret, Flags::STATIC);
	add_func("_xxx", TypeVoid, &kaba_xxx, Flags::STATIC);
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
