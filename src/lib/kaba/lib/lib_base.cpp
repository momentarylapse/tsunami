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
#include "../../os/msg.h"
#include "../../os/terminal.h"
#include "../../base/callable.h"
#include "../../base/map.h"
#include <algorithm>
#include <math.h>
#include <cstdio>

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
const Class *TypeCallableT;
const Class *TypeOptionalT;
const Class *TypeProductT;
const Class *TypeFutureT;
const Class *TypeFutureCoreT;

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

void* kaba_pointer_definitely(void* p) {
	if (!p)
		kaba::kaba_raise_exception(new KabaNoValueError());
	return p;
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

string kaba_int32_hex(int i) {
	return format("%x", i);
}

/*string kaba_char_repr(char c) {
	return "'" + string(&c, 1).escape() + "'";
}*/

string kaba_int8_to_str(char c) {
	return format("0x%02x", (int)c);//i2s((int)c);
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
	TypeReg128			= add_type  ("@reg128", 16, Flags::FORCE_CALL_BY_VALUE);
	TypeReg64			= add_type  ("@reg64", 8, Flags::FORCE_CALL_BY_VALUE);
	TypeReg32			= add_type  ("@reg32", 4, Flags::FORCE_CALL_BY_VALUE);
	TypeReg16			= add_type  ("@reg16", 2, Flags::FORCE_CALL_BY_VALUE);
	TypeReg8			= add_type  ("@reg8", 1, Flags::FORCE_CALL_BY_VALUE);
	TypeObject			= add_type  ("Object", sizeof(VirtualBase)); // base for most virtual classes
	TypeDynamic			= add_type  ("@dynamic", 0);

	// "real"
	TypeVoid			= add_type  ("void", 0, Flags::FORCE_CALL_BY_VALUE);
	TypeBool			= add_type  ("bool", sizeof(bool), Flags::FORCE_CALL_BY_VALUE);
	TypeInt8			= add_type  ("i8", 1, Flags::FORCE_CALL_BY_VALUE);
	TypeInt16			= add_type  ("i16", 2, Flags::FORCE_CALL_BY_VALUE);
	TypeInt32			= add_type  ("i32", sizeof(int32), Flags::FORCE_CALL_BY_VALUE);
	TypeInt64			= add_type  ("i64", sizeof(int64), Flags::FORCE_CALL_BY_VALUE);
	TypeFloat32			= add_type  ("f32", sizeof(float), Flags::FORCE_CALL_BY_VALUE);
	TypeFloat64			= add_type  ("f64", sizeof(double), Flags::FORCE_CALL_BY_VALUE);
	TypeDynamicArray	= add_type  ("@DynamicArray", config.target.dynamic_array_size);
	TypeDictBase		= add_type  ("@DictBase",   config.target.dynamic_array_size);
	TypeSharedPointer	= add_type  ("@SharedPointer", config.target.pointer_size);
	TypeCallableBase	= add_type  ("@CallableBase", sizeof(Callable<void()>));

	TypeNoValueError    = add_type  ("NoValueError", sizeof(KabaException));


	// select default float type
	TypeFloat = TypeFloat32;
	(const_cast<Class*>(TypeFloat))->name = "float";
	TypeInt = TypeInt32;
	(const_cast<Class*>(TypeInt32))->name = "int";


	add_class(TypeObject);
		class_add_func(Identifier::Func::INIT, TypeVoid, &_VirtualBase::__init__, Flags::MUTABLE);
		class_add_func_virtual(Identifier::Func::DELETE, TypeVoid, &VirtualBase::__delete__, Flags::MUTABLE);
		class_set_vtable(VirtualBase);

	add_class(TypeDynamicArray);
		class_add_element("num", TypeInt, config.target.pointer_size);
		class_add_func("swap", TypeVoid, &DynamicArray::simple_swap, Flags::MUTABLE);
			func_add_param("i1", TypeInt);
			func_add_param("i2", TypeInt);
		class_add_func(Identifier::Func::SUBARRAY, TypeDynamicArray, &DynamicArray::ref_subarray, Flags::REF);
			func_add_param("start", TypeInt);
			func_add_param("end", TypeInt);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init, Flags::MUTABLE);
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear, Flags::MUTABLE);
		class_add_func("__mem_forget__", TypeVoid, &DynamicArray::forget, Flags::MUTABLE);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize, Flags::MUTABLE);
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single, Flags::MUTABLE);
			func_add_param("index", TypeInt);

	add_class(TypeDictBase);
		class_add_element("num", TypeInt, config.target.pointer_size);
		// low level operations
		class_add_func("__mem_init__", TypeVoid, &DynamicArray::init, Flags::MUTABLE);
			func_add_param("element_size", TypeInt);
		class_add_func("__mem_clear__", TypeVoid, &DynamicArray::simple_clear, Flags::MUTABLE);
		class_add_func("__mem_forget__", TypeVoid, &DynamicArray::forget, Flags::MUTABLE);
		class_add_func("__mem_resize__", TypeVoid, &DynamicArray::simple_resize, Flags::MUTABLE);
			func_add_param("size", TypeInt);
		class_add_func("__mem_remove__", TypeVoid, &DynamicArray::delete_single, Flags::MUTABLE);
			func_add_param("index", TypeInt);

	add_class(TypeSharedPointer);
		class_add_func(Identifier::Func::INIT, TypeVoid, nullptr, Flags::MUTABLE);
			func_set_inline(InlineID::SHARED_POINTER_INIT);


#if 0
	auto create_class = [] (SyntaxTree *tree, const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, int token_id) {
		/*msg_write("CREATE " + name);
		msg_write(p2s(tree));
		msg_write(p2s(tree->implicit_symbols.get()));*/

		auto ns = tree->implicit_symbols.get();

		Class *t = new Class(type, name, size, tree, parent, params);
		t->token_id = token_id;
		tree->owned_classes.add(t);

		// link namespace
		ns->classes.add(t);
		t->name_space = ns;
		return t;
	};

	auto create_auto_class = [create_class] (SyntaxTree *tree, const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, int token_id) {
		auto t = create_class(tree, name, type, size, array_size, parent, params, token_id);
		AutoImplementer ai(nullptr, tree);
		ai.complete_type(t, 0, token_id);
		return t;
	};

	TypeRawT = add_class_template("ptr", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, class_name_might_need_parantheses(params[0]) + "*", Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, params, token_id);
//		return create_auto_class(format("%s[%s]", Identifier::RAW_POINTER, params[0]->name), Class::Type::POINTER_RAW, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeXferT = add_class_template("xfer", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, format("%s[%s]", Identifier::XFER, params[0]->name), Class::Type::POINTER_XFER_NOT_NULL, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeSharedT = add_class_template("shared", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, format("%s[%s]", Identifier::SHARED, params[0]->name), Class::Type::POINTER_SHARED, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeSharedNotNullT = add_class_template("shared!", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, format("%s![%s]", Identifier::SHARED, params[0]->name), Class::Type::POINTER_SHARED_NOT_NULL, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeOwnedT = add_class_template("owned", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, format("%s[%s]", Identifier::OWNED, params[0]->name), Class::Type::POINTER_OWNED, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeOwnedNotNullT = add_class_template("owned!", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, format("%s![%s]", Identifier::OWNED, params[0]->name), Class::Type::POINTER_OWNED_NOT_NULL, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeAliasT = add_class_template("@alias", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, format("%s[%s]", Identifier::ALIAS, params[0]->name), Class::Type::POINTER_ALIAS, config.target.pointer_size, 0, nullptr, params, token_id);
	});
	TypeReferenceT = add_class_template("ref", {"T"}, [create_auto_class] (SyntaxTree *tree, const Array<const Class*>& params, int token_id) {
		return create_auto_class(tree, class_name_might_need_parantheses(params[0]) + "&", Class::Type::REFERENCE, config.target.pointer_size, 0, nullptr, params, token_id);
	});

#else
	TypeRawT = add_class_template("ptr", {"T"}, nullptr);
	TypeXferT = add_class_template("xfer", {"T"}, nullptr);
	TypeSharedT = add_class_template("shared", {"T"}, nullptr);
	TypeSharedNotNullT = add_class_template("shared!", {"T"}, nullptr);
	TypeOwnedT = add_class_template("owned", {"T"}, nullptr);
	TypeOwnedNotNullT = add_class_template("owned!", {"T"}, nullptr);
	TypeAliasT = add_class_template("@alias", {"T"}, nullptr);
	TypeReferenceT = add_class_template("ref", {"T"}, nullptr);
	TypeArrayT = add_class_template("@Array", {"T"}, nullptr);
	TypeListT = add_class_template("@List", {"T"}, nullptr);
	TypeDictT = add_class_template("@Dict", {"T"}, nullptr);
	TypeCallableT = add_class_template("@Callable", {"T..."}, nullptr);
	TypeOptionalT = add_class_template("@Optional", {"T"}, nullptr);
	TypeProductT = add_class_template("@Product", {"T"}, nullptr);
	TypeFutureCoreT = add_class_template("@FutureCore", {"T"}, nullptr);
	TypeFutureT = add_class_template("future", {"T"}, nullptr);
#endif

	TypeObjectP			= add_type_p_raw(TypeObject);


	// derived   (must be defined after the primitive types and the bases!)
	TypePointer     = add_type_p_raw(TypeVoid); // substitute for all raw pointer types
	TypeReference   = add_type_ref(TypeVoid); // substitute for all reference types
	TypeNone        = add_type_p_raw(TypeVoid); // type of <nil>
	const_cast<Class*>(TypeNone)->name = "None";
	TypePointerList = add_type_list(TypePointer);
	TypeBoolList    = add_type_list(TypeBool);
	TypeIntP        = add_type_p_raw(TypeInt);
	TypeIntOptional = add_type_optional(TypeInt);
	TypeIntList     = add_type_list(TypeInt);
	TypeFloatP      = add_type_p_raw(TypeFloat);
	TypeFloatList   = add_type_list(TypeFloat);
	TypeFloat64List = add_type_list(TypeFloat64);
	TypeBytes      = add_type_list(TypeInt8);
	TypeCString     = add_type_array(TypeInt8, 256);
	capture_implicit_type(TypeCString, "cstring"); // cstring := i8[256]
	TypeString      = add_type_list(TypeInt8);
	capture_implicit_type(TypeString, "string"); // string := i8[]
	TypeStringAutoCast = add_type("<string-auto-cast>", config.target.dynamic_array_size);	// string := i8[]
	TypeStringList  = add_type_list(TypeString);
	capture_implicit_type(TypeBytes, "bytes"); // bytes := i8[]

	TypeIntDict     = add_type_dict(TypeInt);
	TypeFloatDict   = add_type_dict(TypeFloat);
	TypeStringDict  = add_type_dict(TypeString);

	TypeException		= add_type  ("Exception", sizeof(KabaException));
	TypeExceptionXfer	= add_type_p_xfer(TypeException);

	lib_create_list<void*>(TypePointerList);
	lib_create_list<bool>(TypeBoolList);
	lib_create_list<int>(TypeIntList);
	lib_create_list<float>(TypeFloatList);
	lib_create_list<double>(TypeFloat64List);
	lib_create_list<char>(TypeString);
	lib_create_list<uint8_t>(TypeBytes);
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


	add_class(TypeInt8);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_int8_to_str, Flags::PURE);
		//class_add_func(Identifier::Func::REPR, TypeString, &kaba_char_repr, Flags::PURE);
		class_add_func("__int__", TypeInt, &kaba_cast<char,int>, Flags::PURE);
			func_set_inline(InlineID::INT8_TO_INT32);
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
		class_add_element("low", TypeInt8, 0);
		class_add_element("high", TypeInt8, 1);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_i16_to_str, Flags::PURE);
		class_add_func("__int__", TypeInt, &kaba_cast<unsigned short,int>, Flags::PURE);
		//	func_set_inline(InlineID::INT16_TO_INT32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt16, TypeInt16, InlineID::CHUNK_ASSIGN);
		//add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt16, TypeInt32, InlineID::INT16_ASSIGN_INT32);
		class_add_func("__assign__", TypeVoid, &kaba_i16_from_i32, Flags::MUTABLE);
			func_add_param("o", TypeInt);


	add_class(TypeInt32);
		class_add_func(Identifier::Func::STR, TypeString, &i2s, Flags::PURE);
		class_add_func(Identifier::Func::FORMAT, TypeString, &kaba_int_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__float__", TypeFloat32, &kaba_cast<int,float>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_FLOAT32);
		class_add_func("__f64__", TypeFloat64, &kaba_cast<int,double>, Flags::PURE);
		class_add_func("__i8__", TypeInt8, &kaba_cast<int,char>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_INT8);
		class_add_func("__i64__", TypeInt64, &kaba_cast<int,int64>, Flags::PURE);
			func_set_inline(InlineID::INT32_TO_INT64);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt, TypeInt, InlineID::INT32_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt, TypeInt, TypeInt, InlineID::INT32_ADD, &op_int_add);
		add_operator(OperatorID::SUBTRACT, TypeInt, TypeInt, TypeInt, InlineID::INT32_SUBTRACT, &op_int_sub);
		add_operator(OperatorID::MULTIPLY, TypeInt, TypeInt, TypeInt, InlineID::INT32_MULTIPLY, &op_int_mul);
		add_operator(OperatorID::DIVIDE, TypeInt, TypeInt, TypeInt, InlineID::INT32_DIVIDE, &op_int_div);
		add_operator(OperatorID::EXPONENT, TypeInt, TypeInt, TypeInt, InlineID::NONE, &xop_exp<int>);
		add_operator(OperatorID::ADDS, TypeVoid, TypeInt, TypeInt, InlineID::INT32_ADD_ASSIGN);
		add_operator(OperatorID::SUBTRACTS, TypeVoid, TypeInt, TypeInt, InlineID::INT32_SUBTRACT_ASSIGN);
		add_operator(OperatorID::MULTIPLYS, TypeVoid, TypeInt, TypeInt, InlineID::INT32_MULTIPLY_ASSIGN);
		add_operator(OperatorID::DIVIDES, TypeVoid, TypeInt, TypeInt, InlineID::INT32_DIVIDE_ASSIGN);
		add_operator(OperatorID::MODULO, TypeInt, TypeInt, TypeInt, InlineID::INT32_MODULO, &op_int32_mod);
		add_operator(OperatorID::EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT32_EQUAL, &op_int_eq);
		add_operator(OperatorID::NOT_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT32_NOT_EQUAL, &op_int_neq);
		add_operator(OperatorID::GREATER, TypeBool, TypeInt, TypeInt, InlineID::INT32_GREATER, &op_int_g);
		add_operator(OperatorID::GREATER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT32_GREATER_EQUAL, &op_int_ge);
		add_operator(OperatorID::SMALLER, TypeBool, TypeInt, TypeInt, InlineID::INT32_SMALLER, &op_int_l);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBool, TypeInt, TypeInt, InlineID::INT32_SMALLER_EQUAL, &op_int_le);
		add_operator(OperatorID::BIT_AND, TypeInt, TypeInt, TypeInt, InlineID::INT32_AND);
		add_operator(OperatorID::BIT_OR, TypeInt, TypeInt, TypeInt, InlineID::INT32_OR);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt, TypeInt, TypeInt, InlineID::INT32_SHIFT_RIGHT, &op_int32_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt, TypeInt, TypeInt, InlineID::INT32_SHIFT_LEFT, &op_int32_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt, nullptr, TypeInt, InlineID::INT32_NEGATIVE, &op_int_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt, nullptr, InlineID::INT32_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt, nullptr, InlineID::INT32_DECREASE);

	add_class(TypeInt64);
		class_add_func(Identifier::Func::STR, TypeString, &i642s, Flags::PURE);
		class_add_func("__int__", TypeInt, &kaba_cast<int64,int>, Flags::PURE);
			func_set_inline(InlineID::INT64_TO_INT32);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeInt64, TypeInt64, InlineID::INT64_ASSIGN);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_ADD, &op_int64_add);
		add_operator(OperatorID::ADD, TypeInt64, TypeInt64, TypeInt, InlineID::INT64_ADD_INT32, &op_int64_add_int); // needed by internal address calculations!
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
		add_operator(OperatorID::BIT_AND, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_AND);
		add_operator(OperatorID::BIT_OR, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_OR);
		add_operator(OperatorID::SHIFT_RIGHT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_RIGHT, &op_int64_shr);
		add_operator(OperatorID::SHIFT_LEFT, TypeInt64, TypeInt64, TypeInt64, InlineID::INT64_SHIFT_LEFT, &op_int64_shl);
		add_operator(OperatorID::NEGATIVE, TypeInt64, nullptr, TypeInt64, InlineID::INT64_NEGATIVE, &op_int64_neg);
		add_operator(OperatorID::INCREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_INCREASE);
		add_operator(OperatorID::DECREASE, TypeVoid, TypeInt64, nullptr, InlineID::INT64_DECREASE);

	add_class(TypeFloat32);
		class_add_func(Identifier::Func::STR, TypeString, &kaba_float2str, Flags::PURE);
		class_add_func("str2", TypeString, &f2s, Flags::PURE);
			func_add_param("decimals", TypeInt);
		class_add_func(Identifier::Func::FORMAT, TypeString, &kaba_float_format, Flags::PURE);
			func_add_param("fmt", TypeString);
		class_add_func("__int__", TypeInt, &kaba_cast<float,int>, Flags::PURE);
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
		class_add_func("__float__", TypeFloat32, &kaba_cast<double,float>, Flags::PURE);
			func_set_inline(InlineID::FLOAT64_TO_FLOAT32);
		class_add_func("__int__", TypeInt, &kaba_cast<double,int>, Flags::PURE);
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
			func_add_param("size", TypeInt);
		class_add_func("tail", TypeString, &string::tail, Flags::PURE);
			func_add_param("size", TypeInt);
		class_add_func("find", TypeIntOptional, &KabaString::_find, Flags::PURE);
			func_add_param("str", TypeString);
			func_add_param_def("start", TypeInt, 0);
		class_add_func("compare", TypeInt, &string::compare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("icompare", TypeInt, &string::icompare, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("replace", TypeString, &string::replace, Flags::PURE);
			func_add_param("sub", TypeString);
			func_add_param("by", TypeString);
		class_add_func("explode", TypeStringList, &string::explode, Flags::PURE);
			func_add_param("str", TypeString);
		class_add_func("parse_tokens", TypeStringList, &string::parse_tokens, Flags::PURE);
			func_add_param("splitters", TypeString);
		class_add_func("repeat", TypeString, &string::repeat, Flags::PURE);
			func_add_param("n", TypeInt);
		class_add_func("lower", TypeString, &string::lower, Flags::PURE);
		class_add_func("upper", TypeString, &string::upper, Flags::PURE);
		class_add_func("reverse", TypeString, &string::reverse, Flags::PURE);
		class_add_func("hash", TypeInt, &string::hash, Flags::PURE);
		class_add_func("hex", TypeString, &string::hex, Flags::PURE);
		class_add_func("unhex", TypeBytes, &string::unhex, Flags::PURE);
		class_add_func("match", TypeBool, &string::match, Flags::PURE);
			func_add_param("glob", TypeString);
		class_add_func("__int__", TypeInt, &string::_int, Flags::PURE);
		class_add_func("__i64__", TypeInt64, &string::i64, Flags::PURE);
		class_add_func("__float__", TypeFloat32, &string::_float, Flags::PURE);
		class_add_func("__f64__", TypeFloat64, &string::f64, Flags::PURE);
		class_add_func("trim", TypeString, &string::trim, Flags::PURE);
		class_add_func("escape", TypeString, &string::escape, Flags::PURE);
		class_add_func("unescape", TypeString, &string::unescape, Flags::PURE);
		class_add_func("utf8_to_utf32", TypeIntList, &string::utf8_to_utf32, Flags::PURE);
		class_add_func("utf8_length", TypeInt, &string::utf8len, Flags::PURE);
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
		class_add_func("reverse", TypeString, &bytes::reverse, Flags::PURE);
		class_add_func("hash", TypeInt, &bytes::hash, Flags::PURE);
		class_add_func("md5", TypeString, &bytes::md5, Flags::PURE);
		class_add_func("hex", TypeString, &bytes::hex, Flags::PURE);
		class_add_func("utf8", TypeString, &KabaBytes::utf8, Flags::PURE);
		class_add_func("find", TypeIntOptional, &KabaBytes::_find, Flags::PURE);
			func_add_param("str", TypeBytes);
			func_add_param_def("start", TypeInt, 0);
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
		add_operator(OperatorID::ADD, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::add_values_scalar);
		add_operator(OperatorID::SUBTRACT, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::sub_values_scalar);
		add_operator(OperatorID::MULTIPLY, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::mul_values_scalar);
		add_operator(OperatorID::DIVIDE, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::div_values_scalar);
		add_operator(OperatorID::EXPONENT, TypeIntList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::exp_values_scalar);
		add_operator(OperatorID::ASSIGN, TypeVoid, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::assign_values_scalar);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::lt_values);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::le_values);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::gt_values);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::ge_values);
		// don't we prefer  int[] == int[] -> bool ???
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::eq_values);
		add_operator(OperatorID::NOT_EQUAL, TypeBoolList, TypeIntList, TypeIntList, InlineID::NONE, &XList<int>::ne_values);
		add_operator(OperatorID::SMALLER, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::lt_values_scalar);
		add_operator(OperatorID::SMALLER_EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::le_values_scalar);
		add_operator(OperatorID::GREATER, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::gt_values_scalar);
		add_operator(OperatorID::GREATER_EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::ge_values_scalar);
		add_operator(OperatorID::EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::eq_values_scalar);
		add_operator(OperatorID::NOT_EQUAL, TypeBoolList, TypeIntList, TypeInt, InlineID::NONE, &XList<int>::ne_values_scalar);
		class_add_func(Identifier::Func::CONTAINS, TypeBool, &XList<int>::__contains__, Flags::PURE);
			func_add_param("i", TypeInt);

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
			func_add_param("index", TypeInt);
		class_add_func("resize", TypeVoid, &StringList::resize, Flags::MUTABLE);
			func_add_param("num", TypeInt);
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
		
		
	// type casting
	add_func("p2s", TypeString, &p2s, Flags::STATIC | Flags::PURE);
		func_add_param("p", TypePointer);
	add_func("@pointer_definitely", TypeReference, &kaba_pointer_definitely, Flags::STATIC | Flags::RAISES_EXCEPTIONS);
		func_add_param("p", TypePointer);
	add_func("char", TypeString, &kaba_char2str, Flags::STATIC | Flags::PURE);
		func_add_param("c", TypeInt);
	add_func("hex", TypeString, &kaba_int32_hex, Flags::STATIC | Flags::PURE);
		func_add_param("i", TypeInt);
	// debug output
	/*add_func("cprint", TypeVoid, &_cstringout, Flags::STATIC);
		func_add_param("str", TypeCString);*/
	add_func("print", TypeVoid, &os::terminal::print, Flags::STATIC);
		func_add_param("str", TypeStringAutoCast);//, (Flags)((int)Flags::CONST | (int)Flags::AUTO_CAST));
	add_ext_var("_print_postfix", TypeString, &os::terminal::_print_postfix_);
	add_func("as_binary", TypeBytes, &kaba_binary, Flags::STATIC);
		func_add_param("p", TypeReference, Flags::REF);
		func_add_param("length", TypeInt);
	// memory
	add_func("@malloc", TypePointer, &kaba_malloc, Flags::STATIC);
		func_add_param("size", TypeInt);
	add_func("@free", TypeVoid, &free, Flags::STATIC);
		func_add_param("p", TypePointer);

	// basic testing
	add_func("_ping", TypeVoid, &kaba_ping, Flags::STATIC);
	add_func("_int_out", TypeVoid, &kaba_int_out, Flags::STATIC);
		func_add_param("i", TypeInt);
	add_func("_float_out", TypeVoid, &kaba_float_out, Flags::STATIC);
		func_add_param("f", TypeFloat32);
	add_func("_call_float", TypeVoid, &_x_call_float, Flags::STATIC);
	add_func("_float_ret", TypeFloat32, &kaba_float_ret, Flags::STATIC);
	add_func("_int_ret", TypeInt, &kaba_int_ret, Flags::STATIC);
	add_func("_xxx", TypeVoid, &kaba_xxx, Flags::STATIC);
		func_add_param("a", TypeInt);
		func_add_param("b", TypeInt);
		func_add_param("c", TypeInt);
		func_add_param("d", TypeInt);
		func_add_param("e", TypeInt);
		func_add_param("f", TypeInt);
		func_add_param("g", TypeInt);
		func_add_param("h", TypeInt);
	add_ext_var("_extern_variable", TypeInt, &extern_variable1);


	add_type_cast(10, TypeInt, TypeFloat32, "int.__float__");
	add_type_cast(10, TypeInt, TypeFloat64, "int.__f64__");
	add_type_cast(10, TypeInt, TypeInt64, "int.__i64__");
	add_type_cast(200, TypeInt64, TypeInt, "i64.__int__");
	add_type_cast(10, TypeFloat32, TypeFloat64,"float.__f64__");
	add_type_cast(200, TypeFloat32, TypeInt, "float.__int__");
	add_type_cast(200, TypeInt, TypeInt8, "int.__i8__");
	add_type_cast(20, TypeInt8, TypeInt, "i8.__int__");
	//add_type_cast(30, TypeBoolList, TypeBool, "bool[].__bool__");
	add_type_cast(50, TypePointer, TypeBool, "p2b");
	//add_type_cast(50, TypePointer, TypeString, "p2s");
}



}
