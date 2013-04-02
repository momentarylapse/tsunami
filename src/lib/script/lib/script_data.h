/*----------------------------------------------------------------------------*\
| Script Data                                                                  |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(SCRIPT_DATA_H__INCLUDED_)
#define SCRIPT_DATA_H__INCLUDED_

namespace Script{


#define SCRIPT_MAX_DEFINE_RECURSIONS	128
#define SCRIPT_MAX_PARAMS				16		// number of possible parameters per function/command
#define SCRIPT_MAX_OPCODE				(2*65536)	// max. amount of opcode
#define SCRIPT_MAX_THREAD_OPCODE		1024
#define SCRIPT_DEFAULT_STACK_SIZE		32768	// max. amount of stack memory
extern int StackSize;

#define PointerSize (sizeof(char*))
#define SuperArraySize	(sizeof(DynamicArray))


//#define mem_align(x)	((x) + (4 - (x) % 4) % 4)
#define mem_align(x)	((((x) + 3) / 4 ) * 4)

extern string DataVersion;

class PreScript;
struct Type;


//--------------------------------------------------------------------------------------------------
// types



struct ClassElement{
	string name;
	Type *type;
	int offset;
};
struct ClassFunction{
	string name;
	int kind, nr; // PreCommand / Own Function?,  index
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
	PreScript *owner; // to share and be able to delete...

	bool force_call_by_value;
	bool UsesCallByReference()
	{	return (!force_call_by_value) && (!is_pointer) && ((is_array) || (is_super_array) || (element.num > 0));	}
	int GetFunc(const string &name)
	{
		foreachi(ClassFunction &f, function, i)
			if (f.name == name)
				return i;
		return -1;
	}
};
extern Array<Type*> PreTypes;
extern Type *TypeUnknown;
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

void script_make_super_array(Type *t, PreScript *ps = NULL);




//--------------------------------------------------------------------------------------------------
// operators
enum{
	OperatorAssign,			//  =
	OperatorAdd,			//  +
	OperatorSubtract,		//  -
	OperatorMultiply,		//  *
	OperatorDivide,			//  /
	OperatorAddS,			// +=
	OperatorSubtractS,		// -=
	OperatorMultiplyS,		// *=
	OperatorDivideS,		// /=
	OperatorEqual,			// ==
	OperatorNotEqual,		// !=
	OperatorNegate,			//  !
	OperatorSmaller,		//  <
	OperatorGreater,		//  >
	OperatorSmallerEqual,	// <=
	OperatorGreaterEqual,	// >=
	OperatorAnd,			// and
	OperatorOr,				// or
	OperatorModulo,			//  %
	OperatorBitAnd,			//  &
	OperatorBitOr,			//  |
	OperatorShiftLeft,		// <<
	OperatorShiftRight,		// >>
	OperatorIncrease,		// ++
	OperatorDecrease,		// --
	NUM_PRIMITIVE_OPERATORS
};

struct PrimitiveOperator{
	string name;
	int id;
	bool left_modifiable;
	unsigned char level; // order of operators ("Punkt vor Strich")
	string function_name;
};
extern int NumPrimitiveOperators;
extern PrimitiveOperator PrimitiveOperators[];
struct PreOperator{
	int primitive_id;
	Type *return_type, *param_type_1, *param_type_2;
	void *func;
};
extern Array<PreOperator> PreOperators;



enum{
	OperatorPointerAssign,
	OperatorPointerEqual,
	OperatorPointerNotEqual,
	OperatorCharAssign,
	OperatorCharEqual,
	OperatorCharNotEqual,
	OperatorCharAdd,
	OperatorCharSubtract,
	OperatorCharAddS,
	OperatorCharSubtractS,
	OperatorCharBitAnd,
	OperatorCharBitOr,
	OperatorCharNegate,
	OperatorBoolAssign,
	OperatorBoolEqual,
	OperatorBoolNotEqual,
	OperatorBoolGreater,
	OperatorBoolGreaterEqual,
	OperatorBoolSmaller,
	OperatorBoolSmallerEqual,
	OperatorBoolAnd,
	OperatorBoolOr,
	OperatorBoolNegate,
	OperatorIntAssign,
	OperatorIntAdd,
	OperatorIntSubtract,
	OperatorIntMultiply,
	OperatorIntDivide,
	OperatorIntAddS,
	OperatorIntSubtractS,
	OperatorIntMultiplyS,
	OperatorIntDivideS,
	OperatorIntModulo,
	OperatorIntEqual,
	OperatorIntNotEqual,
	OperatorIntGreater,
	OperatorIntGreaterEqual,
	OperatorIntSmaller,
	OperatorIntSmallerEqual,
	OperatorIntBitAnd,
	OperatorIntBitOr,
	OperatorIntShiftRight,
	OperatorIntShiftLeft,
	OperatorIntNegate,
	OperatorIntIncrease,
	OperatorIntDecrease,
	OperatorFloatAssign,
	OperatorFloatAdd,
	OperatorFloatSubtract,
	OperatorFloatMultiply,
	OperatorFloatMultiplyFI,
	OperatorFloatMultiplyIF,
	OperatorFloatDivide,
	OperatorFloatAddS,
	OperatorFloatSubtractS,
	OperatorFloatMultiplyS,
	OperatorFloatDivideS,
	OperatorFloatEqual,
	OperatorFloatNotEqual,
	OperatorFloatGreater,
	OperatorFloatGreaterEqual,
	OperatorFloatSmaller,
	OperatorFloatSmallerEqual,
	OperatorFloatNegate,
//	OperatorComplexAssign,
	OperatorComplexAdd,
	OperatorComplexSubtract,
	OperatorComplexMultiply,
	OperatorComplexMultiplyFC,
	OperatorComplexMultiplyCF,
	OperatorComplexDivide,
	OperatorComplexAddS,
	OperatorComplexSubtractS,
	OperatorComplexMultiplyS,
	OperatorComplexDivideS,
	OperatorComplexEqual,
	OperatorComplexNegate,
	OperatorCStringAssignAA,
	OperatorCStringAddAA,
	OperatorCStringAddAAS,
	OperatorCStringEqualAA,
	OperatorCStringNotEqualAA,
	OperatorClassAssign,
	OperatorClassEqual,
	OperatorClassNotEqual,
	OperatorVectorAdd,
	OperatorVectorSubtract,
	OperatorVectorMultiplyVV,
	OperatorVectorMultiplyVF,
	OperatorVectorMultiplyFV,
	OperatorVectorDivide,
	OperatorVectorDivideVF,
	OperatorVectorAddS,
	OperatorVectorSubtractS,
	OperatorVectorMultiplyS,
	OperatorVectorDivideS,
	OperatorVectorNegate,
};




//--------------------------------------------------------------------------------------------------
// constants

struct PreConstant{
	string name;
	Type *type;
	void *value;
	int package;
};
extern Array<PreConstant> PreConstants;




//--------------------------------------------------------------------------------------------------
// external variables (in the surrounding program...)

struct PreExternalVar{
	string name;
	Type *type;
	void *pointer;
	bool is_semi_external;
	int package;
};
extern Array<PreExternalVar> PreExternalVars;

//--------------------------------------------------------------------------------------------------
// semi external variables (in the surrounding program...but has to be defined "extern")



//--------------------------------------------------------------------------------------------------
// commands

struct PreCommandParam{
	string name;
	Type *type;
};
struct PreCommand{
	string name;
	void *func;
	bool is_class_function, is_special;
	Type *return_type;
	Array<PreCommandParam> param;
	bool is_semi_external;
	int package;
};
extern Array<PreCommand> PreCommands;


enum{
	// structural commands
	CommandReturn,
	CommandIf,
	CommandIfElse,
	CommandWhile,
	CommandFor,
	CommandBreak,
	CommandContinue,
	CommandSizeof,
	CommandWait,
	CommandWaitRT,
	CommandWaitOneFrame,
	CommandFloatToInt,
	CommandIntToFloat,
	CommandIntToChar,
	CommandCharToInt,
	CommandPointerToBool,
	CommandComplexSet,
	CommandVectorSet,
	CommandRectSet,
	CommandColorSet,
	CommandAsm,
	NUM_INTERN_PRE_COMMANDS
};





//--------------------------------------------------------------------------------------------------
// type casting

typedef void *t_cast_func(void*);
struct TypeCast{
	int penalty;
	Type *source, *dest;
	int command;
	t_cast_func *func;
};
extern Array<TypeCast> TypeCasts;


typedef void t_func();
class DummyClass
{
public:
	void func(){}
};

extern void Init();
extern void End();
extern void ResetSemiExternalData();
extern void LinkSemiExternalVar(const string &name, void *pointer);
extern void LinkSemiExternalFunc(const string &name, void *pointer);
extern void _LinkSemiExternalClassFunc(const string &name, void (DummyClass::*function)());
template<typename T>
void LinkSemiExternalClassFunc(const string &name, T pointer)
{
	_LinkSemiExternalClassFunc(name, (void(DummyClass::*)())pointer);
}
extern void AddPreGlobalVar(const string &name, Type *type);
extern Type *GetPreType(const string &name);




//--------------------------------------------------------------------------------------------------
// packages

struct Package
{
	string name;
	Array<Type*> type;
	Array<PreCommand> command;
	Array<PreExternalVar> external_var;
	Array<PreOperator> _operator;
	Array<PreConstant> constant;
};
extern Array<Package> Packages;


};

#endif
