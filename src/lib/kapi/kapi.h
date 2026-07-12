#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <functional>

class Path;
class Any;

namespace kaba {

class Module;
class Context;
class Class;
struct Package;
class IExporter;

class IContext {
public:
	virtual ~IContext() = default;
	virtual shared<Module> load_module(const Path& filename, bool just_analyse) = 0;
	virtual shared<Module> create_module_for_source(const string& source, const Path& filename, bool just_analyse) = 0;
	virtual xfer<Context> create_new_context() const = 0;
	virtual void execute_single_command(const string& cmd) = 0;
	virtual void clean_up() = 0;
	virtual const Class* get_dynamic_type(const VirtualBase* o) const = 0;
	virtual Package* get_package(const string& name) const = 0;
	virtual string type_name(const Class* c) const = 0;
	virtual Any dynify(const void* p, const Class* type) const = 0;
	virtual void unwrap_any(const Any &aa, void *var, const Class *type) const = 0;
	virtual void register_package_init(const string& name, const Path& dir, std::function<void(IExporter*)> f) = 0;
	virtual Array<string> list_keywords() const = 0;
	virtual Array<string> list_modifiers() const = 0;
	virtual Array<string> list_special_functions() const = 0;
	virtual Array<string> list_operator_functions() const = 0;
	virtual Path installation_root() const = 0;
	virtual Path packages_root() const = 0;
	virtual void set_installation_root(const Path& dir) = 0;
	virtual void* get_global_symbol(const string& package, const string& name) = 0;

	shared_array<Module> public_modules;
	shared_array<Package> internal_packages;
	owned_array<Package> external_packages;
};

extern IContext* default_context;

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
	const Class* trait_t;


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

	const Class* sharable_trait;
	const Class* noauto_trait;

	const Class* i32_ref;
	const Class* box;
};
extern CommonTypes common_types;
}


