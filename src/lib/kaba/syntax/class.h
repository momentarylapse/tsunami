
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
	Class();
	//Class(const string &name, int size, SyntaxTree *owner);
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

	bool force_call_by_value;
	bool UsesCallByReference() const;
	bool UsesReturnByMemory() const;
	bool is_simple_class() const;
	bool is_size_known() const;
	Class *GetArrayElement() const;
	bool usable_as_super_array() const;
	bool needs_constructor() const;
	bool needs_destructor() const;
	bool IsDerivedFrom(const Class *root) const;
	bool IsDerivedFrom(const string &root) const;
	bool DeriveFrom(const Class *root, bool increase_size);
	Class *GetPointer() const;
	Class *GetRoot() const;
	void AddFunction(SyntaxTree *s, int func_no, bool as_virtual = false, bool override = false);
	ClassFunction *GetFunc(const string &name, const Class *return_type, int num_params, const Class *param0 = NULL) const;
	ClassFunction *GetDefaultConstructor() const;
	ClassFunction *GetComplexConstructor() const;
	ClassFunction *GetDestructor() const;
	ClassFunction *GetAssign() const;
	ClassFunction *GetVirtualFunction(int virtual_index) const;
	ClassFunction *GetGet(const Class *index) const;
	void LinkVirtualTable();
	void LinkExternalVirtualTable(void *p);
	void *CreateInstance() const;
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
extern Class *TypeClass;
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

};

#endif /* CLASS_H_ */
