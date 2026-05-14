
#pragma once

#include "../../base/pointer.h"
#include "../../base/map.h"
#include "../../kapi/kapi.h"

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

};
