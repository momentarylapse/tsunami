
#ifndef CLASS_H_
#define CLASS_H_

#include "../../base/pointer.h"
#include "../../base/map.h"

namespace kaba {

class Module;
class SyntaxTree;
class Class;
class Function;
class Constant;
class Variable;
class Node;
enum class Flags;


class ClassElement {
public:
	string name;
	const Class *type;
	int64 offset;
	bool allow_indirect_use;
	ClassElement();
	ClassElement(const string &name, const Class *type, int64 offset);
	string signature(bool include_class) const;
	bool hidden() const;
	string str() const;
};

class ClassInitializers {
public:
	int element;
	//shared<Constant> value;
	shared<Node> value;
};

typedef void *VirtualTable;


enum class DeriveFlags {
	NONE = 0,
	SET_SIZE = 1,
	COPY_VTABLE = 2,
	KEEP_CONSTRUCTORS = 4,
};
DeriveFlags operator|(DeriveFlags a, DeriveFlags b);

class Class : public Sharable<base::Empty> {
public:
	
	//Class();
	Class(const Class* from_template, const string &name, int64 size, int alignment, SyntaxTree *owner, const Class *parent = nullptr, const Array<const Class*> &param = {});
	~Class();
	string name;
	string long_name() const;
	string cname(const Class *ns = nullptr) const;
	int64 size; // complete size of type
	int alignment;
	int array_length;
	const Class* from_template;
	Flags flags;

	bool is_regular() const;
	bool is_struct() const;
	bool is_array() const;
	bool is_list() const;
	bool is_dict() const;
	bool is_some_pointer() const;
	bool is_some_pointer_not_null() const;
	bool is_pointer_raw() const;
	bool is_pointer_shared() const;
	bool is_pointer_shared_not_null() const;
	bool is_pointer_owned() const;
	bool is_pointer_owned_not_null() const;
	bool is_pointer_xfer_not_null() const;
	bool is_pointer_alias() const;
	bool is_reference() const;
	bool is_enum() const;
	bool is_namespace() const;
	bool is_interface() const;
	bool is_product() const;
	bool is_optional() const;
	bool is_callable() const;
	bool is_callable_fp() const;
	bool is_callable_bind() const;
	bool is_template() const;
	bool fully_parsed() const;

	Array<ClassElement> elements;
	Array<ClassInitializers> initializers;
	shared_array<Function> functions;
	shared_array<Variable> static_variables;
	shared_array<Constant> constants;
	shared_array<const Class> classes;
	base::map<string, const Class*> type_aliases;

	const Class *parent; // derived from
	Array<const Class*> param; // for pointers/arrays etc
	const Class *name_space;
	SyntaxTree *owner; // to share and be able to delete...
	int token_id;
	bool _return_in_float_registers() const;
	Array<void*> vtable;
	void *_vtable_location_compiler_; // may point to const/opcode
	void *_vtable_location_target_; // (opcode offset adjusted)
	void *_vtable_location_external_; // from linked classes (just for reference)

	bool force_call_by_value() const;
	bool uses_call_by_reference() const;
	bool uses_return_by_memory() const;
	//bool is_simple_class() const;
	bool can_memcpy() const;
	bool is_size_known() const;
	const Class *get_array_element() const;
	bool usable_as_list() const;
	bool needs_constructor() const;
	bool needs_destructor() const;
	bool is_derived_from(const Class *root) const;
	bool is_derived_from_s(const string &root) const;
	void derive_from(const Class *root, DeriveFlags flags = DeriveFlags::NONE);
	const Class *get_root() const;
	void add_function(SyntaxTree *s, Function *f, bool as_virtual = false, bool override = false);
	void add_template_function(SyntaxTree *s, Function *f, bool as_virtual = false, bool override = false);
	Function *get_func(const string &name, const Class *return_type, const Array<const Class*> &params) const;
	Function *get_member_func(const string &name, const Class *return_type, const Array<const Class*> &params) const;
	Function *get_same_func(const string &name, Function *f) const;
	Function *get_default_constructor() const;
	Array<Function*> get_constructors() const;
	Function *get_destructor() const;
	Function *get_assign() const;
	Function *get_virtual_function(int virtual_index) const;
	Function *get_get(const Class *index) const;
	Function *get_call() const;
	void link_virtual_table();
	void link_external_virtual_table(void *p);
	void *create_instance() const;
};

struct CommonTypes {
	const Class* unknown;
	const Class* none; // nil
	const Class* reg128; // dummy for compilation
	const Class* reg64; // dummy for compilation
	const Class* reg32; // dummy for compilation
	const Class* reg16; // dummy for compilation
	const Class* reg8; // dummy for compilation
	const Class* dynamic;
	const Class* _void;
	const Class* pointer;
	const Class* reference;
	const Class* _bool;
	const Class* i8;
	const Class* u8;
	const Class* i16;
	const Class* u16;
	const Class* i32;
	const Class* i64;
	const Class* f32;
	const Class* f64;
	const Class* cstring;
	const Class* string;
	const Class* string_auto_cast;
	const Class* bytes;
	const Class* any;
	const Class* path;
	const Class* os_configuration;
	const Class* date;
	const Class* timer;

	const Class* complex;
	const Class* vec2;
	const Class* vec3;
	const Class* rect;
	const Class* plane;
	const Class* color;
	const Class* quaternion;
	const Class* mat3;
	const Class* mat4;
	const Class* special_function_ref;

	const Class* i32_p;
	const Class* f32_p;

	const Class* i32_optional;
	const Class* f32_optional;

	const Class* i32_list;
	const Class* f32_list;
	const Class* f64_list;
	const Class* bool_list;
	const Class* pointer_list;
	const Class* string_list;
	const Class* complex_list;
	const Class* vec2_list;
	const Class* vec3_list;
	const Class* plane_list;
	const Class* color_list;
	const Class* path_list;
	const Class* any_list;

	const Class* i32_dict;
	const Class* f32_dict;
	const Class* string_dict;
	const Class* any_dict;

	const Class* exception;
	const Class* exception_xfer;
	const Class* no_value_error;

	const Class* _class;
	const Class* class_ref;
	const Class* function;
	const Class* function_ref;
	const Class* function_code;
	const Class* function_code_ref;
	const Class* special_function;

	const Class* raw_t;
	const Class* xfer_t;
	const Class* shared_t;
	const Class* shared_not_null_t;
	const Class* owned_t;
	const Class* owned_not_null_t;
	const Class* alias_t;
	const Class* reference_t;
	const Class* array_t;
	const Class* list_t;
	const Class* dict_t;
	const Class* callable_fp_t;
	const Class* callable_bind_t;
	const Class* optional_t;
	const Class* product_t;
	const Class* future_t;
	const Class* future_core_t;
	const Class* promise_t;
	const Class* enum_t;
	const Class* struct_t;
	const Class* interface_t;
	const Class* namespace_t;


	const Class* image;
	const Class* object;
	const Class* object_p;
	const Class* base_painter;
	const Class* base_painter_p;
	const Class* base_painter_xfer;


	const Class* dynamic_array;
	const Class* dict_base;
	const Class* callable_base;
	const Class* shared_pointer;

	const Class* callback;
	const Class* callback_string;

	const Class* void_future;
	const Class* void_promise;
	const Class* string_future;
	const Class* string_promise;
	const Class* path_future;
	const Class* bool_future;
};
extern CommonTypes common_types;

};

#endif /* CLASS_H_ */
