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


#define SCRIPT_MAX_OPCODE				(2*65536)	// max. amount of opcode
#define SCRIPT_MAX_THREAD_OPCODE		1024
#define SCRIPT_DEFAULT_STACK_SIZE		32768	// max. amount of stack memory


//#define mem_align(x)	((x) + (4 - (x) % 4) % 4)
#define mem_align(x, n)		((((x) + (n) - 1) / (n) ) * (n))

extern string DataVersion;

class SyntaxTree;
struct Type;


void script_make_super_array(Type *t, SyntaxTree *ps = NULL);




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
	string str() const;
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
// commands

struct PreCommandParam{
	string name;
	Type *type;
};
struct PreCommand{
	string name;
	Type *return_type;
	Array<PreCommandParam> param;
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
	CommandNew,
	CommandDelete,
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
	int kind;
	int func_no;
	Script *script;
	t_cast_func *func;
};
extern Array<TypeCast> TypeCasts;


typedef void t_func();
class DummyClass
{
public:
	void func(){}
};

enum
{
	AbiGnu32,
	AbiGnu64,
	AbiWindows32,
	AbiWindows64,
};

struct CompilerConfiguration
{
	int instruction_set;
	int abi;

	int StackSize;
	int PointerSize;
	int SuperArraySize;

	bool allow_simplification;
	bool allow_registers;

	string Directory;
	bool CompileSilently;
	bool ShowCompilerStats;
	bool UseConstAsGlobalVar;

	int StackMemAlign;
	int FunctionAlign;
	int StackFrameAlign;

};

extern CompilerConfiguration config;

void Init(int instruction_set = -1, int abi = -1);
void End();



void ResetExternalData();
void LinkExternal(const string &name, void *pointer);
void _LinkExternalClassFunc(const string &name, void (DummyClass::*function)());
template<typename T>
void LinkExternalClassFunc(const string &name, T pointer)
{
	_LinkExternalClassFunc(name, (void(DummyClass::*)())pointer);
}
void DeclareClassSize(const string &class_name, int offset);
void DeclareClassOffset(const string &class_name, const string &element, int offset);

void *GetExternalLink(const string &name);
int ProcessClassOffset(const string &class_name, const string &element, int offset);
int ProcessClassSize(const string &class_name, int size);




//--------------------------------------------------------------------------------------------------
// packages

struct Package
{
	string name;
	Script *script;
	bool used_by_default;
};
extern Array<Package> Packages;


};

#endif
