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



#define SCRIPT_MAX_DEFINE_RECURSIONS	128
#define SCRIPT_MAX_PARAMS				16		// number of possible parameters per function/command
#define SCRIPT_MAX_OPCODE				(2*65536)	// max. amount of opcode
#define SCRIPT_MAX_THREAD_OPCODE		1024
#define SCRIPT_DEFAULT_STACK_SIZE		32768	// max. amount of stack memory
extern int ScriptStackSize;

#define PointerSize (sizeof(char*))
#define SuperArraySize	(sizeof(DynamicArray))


//#define mem_align(x)	((x) + (4 - (x) % 4) % 4)
#define mem_align(x)	((((x) + 3) / 4 ) * 4)

extern string ScriptDataVersion;

class CPreScript;
struct sType;


//--------------------------------------------------------------------------------------------------
// types



struct sClassElement{
	string Name;
	sType *Type;
	int Offset;
};
struct sClassFunction{
	string Name;
	int Kind, Nr; // PreCommand / Own Function?,  index
	// _func_(x)  ->  p.func(x)
	Array<sType*> ParamType;
	sType *ReturnType;
};

struct sType{
	sType(){
		Owner = NULL;
		Size = 0;
		IsArray = false;
		IsSuperArray = false;
		ArrayLength = 0;
		IsPointer = false;
		IsSilent = false;
		SubType = NULL;
		ForceCallByValue = false;
	};
	string Name;
	int Size; // complete size of type
	int ArrayLength;
	bool IsArray, IsSuperArray; // mutially exclusive!
	bool IsPointer, IsSilent; // pointer silent (&)
	Array<sClassElement> Element;
	Array<sClassFunction> Function;
	sType *SubType;
	CPreScript *Owner; // to share and be able to delete...

	bool ForceCallByValue;
	bool UsesCallByReference()
	{	return (!ForceCallByValue) && (!IsPointer) && ((IsArray) || (IsSuperArray) || (Element.num > 0));	}
	int GetFunc(const string &name)
	{
		foreachi(Function, f, i)
			if (f->Name == name)
				return i;
		return -1;
	}
};
extern Array<sType*> PreType;
extern sType *TypeUnknown;
extern sType *TypeReg32; // dummy for compilation
extern sType *TypeReg16; // dummy for compilation
extern sType *TypeReg8; // dummy for compilation
extern sType *TypeVoid;
extern sType *TypePointer;
extern sType *TypeClass;
extern sType *TypeBool;
extern sType *TypeInt;
extern sType *TypeFloat;
extern sType *TypeChar;
extern sType *TypeCString;
extern sType *TypeString;
extern sType *TypeSuperArray;

extern sType *TypeComplex;
extern sType *TypeVector;
extern sType *TypeRect;
extern sType *TypeColor;
extern sType *TypeQuaternion;

void script_make_super_array(sType *t, CPreScript *ps = NULL);




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

struct sPrimitiveOperator{
	string Name;
	int ID;
	bool LeftModifiable;
	unsigned char Level; // order of operators ("Punkt vor Strich")
	string FunctionName;
};
extern int NumPrimitiveOperators;
extern sPrimitiveOperator PrimitiveOperator[];
struct sPreOperator{
	int PrimitiveID;
	sType *ReturnType, *ParamType1, *ParamType2;
	void *Func;
};
extern Array<sPreOperator> PreOperator;



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

struct sPreConstant{
	string Name;
	sType *Type;
	void *Value;
	int package;
};
extern Array<sPreConstant> PreConstant;




//--------------------------------------------------------------------------------------------------
// external variables (in the surrounding program...)

struct sPreExternalVar{
	string Name;
	sType *Type;
	void *Pointer;
	bool IsSemiExternal;
	int package;
};
extern Array<sPreExternalVar> PreExternalVar;

//--------------------------------------------------------------------------------------------------
// semi external variables (in the surrounding program...but has to be defined "extern")



//--------------------------------------------------------------------------------------------------
// commands

struct sPreCommandParam{
	string Name;
	sType *Type;
};
struct sPreCommand{
	string Name;
	void *Func;
	bool IsClassFunction, IsSpecial;
	sType *ReturnType;
	Array<sPreCommandParam> Param;
	bool IsSemiExternal;
	int package;
};
extern Array<sPreCommand> PreCommand;


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
struct sTypeCast{
	int Penalty;
	sType *Source, *Dest;
	int Command;
	t_cast_func *Func;
};
extern Array<sTypeCast> TypeCast;


typedef void t_func();

extern void ScriptInit();
extern void ScriptEnd();
extern void ScriptResetSemiExternalData();
extern void ScriptLinkSemiExternalVar(const string &name, void *pointer);
extern void ScriptLinkSemiExternalFunc(const string &name, void *pointer);
extern void ScriptAddPreGlobalVar(const string &name, sType *type);
extern sType *ScriptGetPreType(const string &name);




//--------------------------------------------------------------------------------------------------
// packages

struct sPackage
{
	string name;
	Array<sType*> type;
	Array<sPreCommand> command;
	Array<sPreExternalVar> external_var;
	Array<sPreOperator> _operator;
	Array<sPreConstant> constant;
};
extern Array<sPackage> Package;


//--------------------------------------------------------------------------------------------------
// other stuff


class CSuperArray : public DynamicArray
{
	public:
	void init_by_type(sType *t);
	int string_cfind(char *a, int start);
};


#endif
