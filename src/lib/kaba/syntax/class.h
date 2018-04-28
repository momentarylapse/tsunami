
#ifndef CLASS_H_
#define CLASS_H_


namespace Kaba{

class Script;
class SyntaxTree;
class Class;
struct Function;


struct ClassElement{
	string name;
	Class *type;
	long long offset;
	bool hidden;
	ClassElement();
};

// TODO: use Function instead!
struct ClassFunction{
	string name;
	Script *script;
	int nr; // index in functions[]
	// _func_(x)  ->  p.func(x)
	Array<Class*> param_types;
	Class *return_type;
	int virtual_index;
	bool needs_overriding;
	ClassFunction();
	ClassFunction(const string &name, Class *return_type, Script *s, int no);
	Function *GetFunc();
};

typedef void *VirtualTable;

class Class{
public:
	//Class();
	Class(const string &name, int size, SyntaxTree *owner, Class *parent = NULL);
	~Class();
	string name;
	long long size; // complete size of type
	int array_length;
	bool is_array, is_super_array; // mutially exclusive!
	bool is_pointer, is_silent; // pointer silent (&)
	bool fully_parsed;
	Array<ClassElement> elements;
	Array<ClassFunction> functions;
	Class *parent;
	SyntaxTree *owner; // to share and be able to delete...
	Array<void*> vtable;
	void *_vtable_location_compiler_; // may point to const/opcode
	void *_vtable_location_target_; // (opcode offset adjusted)
	void *_vtable_location_external_; // from linked classes (just for reference)

	bool force_call_by_value;
	bool uses_call_by_reference() const;
	bool uses_return_by_memory() const;
	bool is_simple_class() const;
	bool is_size_known() const;
	Class *get_array_element() const;
	bool usable_as_super_array() const;
	bool needs_constructor() const;
	bool needs_destructor() const;
	bool is_derived_from(const Class *root) const;
	bool is_derived_from(const string &root) const;
	bool derive_from(const Class *root, bool increase_size);
	Class *get_pointer() const;
	Class *get_root() const;
	void add_function(SyntaxTree *s, int func_no, bool as_virtual = false, bool override = false);
	ClassFunction *get_func(const string &name, const Class *return_type, int num_params, const Class *param0 = NULL) const;
	ClassFunction *get_default_constructor() const;
	ClassFunction *get_complex_constructor() const;
	ClassFunction *get_destructor() const;
	ClassFunction *get_assign() const;
	ClassFunction *get_virtual_function(int virtual_index) const;
	ClassFunction *get_get(const Class *index) const;
	void link_virtual_table();
	void link_external_virtual_table(void *p);
	void *create_instance() const;
	string var2str(void *p) const;
};
extern Class *TypeUnknown;
extern Class *TypeReg128; // dummy for compilation
extern Class *TypeReg64; // dummy for compilation
extern Class *TypeReg32; // dummy for compilation
extern Class *TypeReg16; // dummy for compilation
extern Class *TypeReg8; // dummy for compilation
extern Class *TypeVoid;
extern Class *TypePointer;
extern Class *TypeChunk;
extern Class *TypeBool;
extern Class *TypeInt;
extern Class *TypeInt64;
extern Class *TypeFloat32;
extern Class *TypeFloat64;
extern Class *TypeChar;
extern Class *TypeCString;
extern Class *TypeString;

extern Class *TypeComplex;
extern Class *TypeVector;
extern Class *TypeRect;
extern Class *TypeColor;
extern Class *TypeQuaternion;

extern Class *TypeException;
extern Class *TypeExceptionP;

extern Class *TypeClassP;

};

#endif /* CLASS_H_ */
