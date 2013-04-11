
#ifndef TYPE_H_
#define TYPE_H_


namespace Script{

class Script;
class SyntaxTree;
class Type;

struct ClassElement{
	string name;
	Type *type;
	int offset;
};
struct ClassFunction{
	string name;
	Script *script;
	int nr; // index in Functions[]
	// _func_(x)  ->  p.func(x)
	Array<Type*> param_type;
	Type *return_type;
};

struct Type{
	Type(){
		owner = NULL;
		size = 0;
		is_array = false;
		is_super_array = false;
		array_length = 0;
		is_pointer = false;
		is_silent = false;
		parent = NULL;
		force_call_by_value = false;
	};
	string name;
	int size; // complete size of type
	int array_length;
	bool is_array, is_super_array; // mutially exclusive!
	bool is_pointer, is_silent; // pointer silent (&)
	Array<ClassElement> element;
	Array<ClassFunction> function;
	Type *parent;
	SyntaxTree *owner; // to share and be able to delete...

	bool force_call_by_value;
	bool UsesCallByReference();
	bool UsesReturnByMemory();
	bool is_simple_class();
	int GetFunc(const string &name);
	ClassFunction *GetConstructor();
	ClassFunction *GetDestructor();
	string var2str(void *p);
};
extern Type *TypeUnknown;
extern Type *TypeReg128; // dummy for compilation
extern Type *TypeReg64; // dummy for compilation
extern Type *TypeReg32; // dummy for compilation
extern Type *TypeReg16; // dummy for compilation
extern Type *TypeReg8; // dummy for compilation
extern Type *TypeVoid;
extern Type *TypePointer;
extern Type *TypeClass;
extern Type *TypeBool;
extern Type *TypeInt;
extern Type *TypeFloat;
extern Type *TypeChar;
extern Type *TypeCString;
extern Type *TypeString;
extern Type *TypeSuperArray;

extern Type *TypeComplex;
extern Type *TypeVector;
extern Type *TypeRect;
extern Type *TypeColor;
extern Type *TypeQuaternion;

};

#endif /* TYPE_H_ */
