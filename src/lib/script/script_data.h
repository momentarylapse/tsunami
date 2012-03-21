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
#define SCRIPT_MAX_NAME					42		// variables' name length (+1)
#define SCRIPT_MAX_PARAMS				16		// number of possible parameters per function/command
#define SCRIPT_MAX_OPCODE				(2*65536)	// max. amount of opcode
#define SCRIPT_MAX_THREAD_OPCODE		1024
#define SCRIPT_DEFAULT_STACK_SIZE		32768	// max. amount of stack memory
extern int ScriptStackSize;

#define PointerSize (sizeof(char*))
#define SuperArraySize	(PointerSize + 3 * sizeof(int))


//#define mem_align(x)	((x) + (4 - (x) % 4) % 4)
#define mem_align(x)	((((x) + 3) / 4 ) * 4)

extern string ScriptDataVersion;

class CPreScript;
struct sType;


//--------------------------------------------------------------------------------------------------
// types



struct sClassElement{
	char Name[SCRIPT_MAX_NAME];
	sType *Type;
	int Offset;
};
struct sClassFunction{
	char Name[SCRIPT_MAX_NAME];
	int Kind, Nr; // PreCommand / Own Function?,  index
	// _func_(x)  ->  p.func(x)
};

struct sType{
	char Name[SCRIPT_MAX_NAME];
	int Size; // complete size of type
	int ArrayLength;
	bool IsArray, IsSuperArray; // mutially exclusive!
	bool IsPointer, IsSilent; // pointer silent (&)
	Array<sClassElement> Element;
	Array<sClassFunction> Function;
	sType *SubType;
	CPreScript *Owner; // to share and be able to delete...
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

struct sPrimitiveOperator{
	char Name[4];
	int ID;
	bool LeftModifiable;
	unsigned char Level; // order of operators ("Punkt vor Strich")
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
	OperatorStringAddS,
	OperatorStringAdd,
	OperatorStringAssignCString,
	OperatorStringAddCString,
	OperatorStringEqual,
	OperatorStringNotEqual,

// -- the rest is used by script_make_super_array()
	OperatorSuperArrayAssign,
	OperatorIntListAddS,
	OperatorIntListSubtractS,
	OperatorIntListMultiplyS,
	OperatorIntListDivideS,
	/*OperatorIntListAdd,
	OperatorIntListSubtract,
	OperatorIntListMultiply,
	OperatorIntListDivide,*/
	OperatorIntListAssignInt,
	OperatorIntListAddSInt,
	OperatorIntListSubtractSInt,
	OperatorIntListMultiplySInt,
	OperatorIntListDivideSInt,
	/*OperatorIntListAddInt,
	OperatorIntListSubtractInt,
	OperatorIntListMultiplyInt,
	OperatorIntListDivideInt,*/
	OperatorFloatListAddS,
	OperatorFloatListSubtractS,
	OperatorFloatListMultiplyS,
	OperatorFloatListDivideS,
	OperatorFloatListAssignFloat,
	OperatorFloatListAddSFloat,
	OperatorFloatListSubtractSFloat,
	OperatorFloatListMultiplySFloat,
	OperatorFloatListDivideSFloat,
	OperatorComplexListAddS,
	OperatorComplexListSubtractS,
	OperatorComplexListMultiplyS,
	OperatorComplexListDivideS,
	OperatorComplexListAssignComplex,
	OperatorComplexListAddSComplex,
	OperatorComplexListSubtractSComplex,
	OperatorComplexListMultiplySComplex,
	OperatorComplexListDivideSComplex,
	OperatorComplexListMultiplySFloat,
};




//--------------------------------------------------------------------------------------------------
// constants

struct sPreConstant{
	const char *Name; // reference (not new'ed)
	sType *Type;
	void *Value;
	int package;
};
extern Array<sPreConstant> PreConstant;




//--------------------------------------------------------------------------------------------------
// external variables (in the surrounding program...)

struct sPreExternalVar{
	const char *Name; // reference (not new'ed)   (unless semiexternal)
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
	const char *Name; // reference (not new'ed)   (unless semiexternal)
	//char Name[SCRIPT_MAX_NAME];
	sType *Type;
};
struct sPreCommand{
	const char *Name; // reference (not new'ed)   (unless semiexternal)
	//char Name[SCRIPT_MAX_NAME];
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
	sType *Source,*Dest;
	int Command;
	t_cast_func *Func;
};
extern Array<sTypeCast> TypeCast;


typedef void t_func();

extern void ScriptInit();
extern void ScriptEnd();
extern void ScriptResetSemiExternalData();
extern void ScriptLinkSemiExternalVar(const char *name, void *pointer);
extern void ScriptLinkSemiExternalFunc(const char *name, void *pointer);
extern void ScriptAddPreGlobalVar(const char *name, sType *type);
extern sType *ScriptGetPreType(const char *name);




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
	// special functions
	void int_sort();
	void int_unique();
	void float_sort();
	int string_cfind(char *a, int start);
};

void super_array_assign(CSuperArray *a, CSuperArray *b);
void super_array_assign_single(CSuperArray *a, void *d);
void super_array_assign_8_single(CSuperArray *a, complex x);
void super_array_assign_4_single(CSuperArray *a, int x);
void super_array_assign_1_single(CSuperArray *a, char x);
void super_array_add_s_int(CSuperArray *a, CSuperArray *b);
void super_array_sub_s_int(CSuperArray *a, CSuperArray *b);
void super_array_mul_s_int(CSuperArray *a, CSuperArray *b);
void super_array_div_s_int(CSuperArray *a, CSuperArray *b);
void super_array_add_s_int_int(CSuperArray *a, int x);
void super_array_sub_s_int_int(CSuperArray *a, int x);
void super_array_mul_s_int_int(CSuperArray *a, int x);
void super_array_div_s_int_int(CSuperArray *a, int x);
void super_array_add_int(CSuperArray *a, CSuperArray *b, CSuperArray *c);
void super_array_sub_int(CSuperArray *a, CSuperArray *b, CSuperArray *c);
void super_array_mul_int(CSuperArray *a, CSuperArray *b, CSuperArray *c);
void super_array_div_int(CSuperArray *a, CSuperArray *b, CSuperArray *c);
void super_array_add_s_float(CSuperArray *a, CSuperArray *b);
void super_array_sub_s_float(CSuperArray *a, CSuperArray *b);
void super_array_mul_s_float(CSuperArray *a, CSuperArray *b);
void super_array_div_s_float(CSuperArray *a, CSuperArray *b);
void super_array_add_s_float_float(CSuperArray *a, float x);
void super_array_sub_s_float_float(CSuperArray *a, float x);
void super_array_mul_s_float_float(CSuperArray *a, float x);
void super_array_div_s_float_float(CSuperArray *a, float x);
void super_array_add_s_com(CSuperArray *a, CSuperArray *b);
void super_array_sub_s_com(CSuperArray *a, CSuperArray *b);
void super_array_mul_s_com(CSuperArray *a, CSuperArray *b);
void super_array_div_s_com(CSuperArray *a, CSuperArray *b);
void super_array_add_s_com_com(CSuperArray *a, complex x);
void super_array_sub_s_com_com(CSuperArray *a, complex x);
void super_array_mul_s_com_com(CSuperArray *a, complex x);
void super_array_div_s_com_com(CSuperArray *a, complex x);
void super_array_mul_s_com_float(CSuperArray *a, float x);
void super_array_add_s_str(string *a, string *b);
string super_array_add_str(string *a, string *b);
void super_array_assign_str_cstr(string *a, char *b);
void super_array_add_str_cstr(string *a, char *b);
bool super_array_equal_str(string *a, string *b);
bool super_array_notequal_str(string *a, string *b);



struct sScriptLocation{
	char Name[SCRIPT_MAX_NAME];
	int Location;
};

extern Array<sScriptLocation> ScriptLocation;


#endif
