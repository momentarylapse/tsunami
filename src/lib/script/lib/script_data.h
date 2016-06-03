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
class Type;


void script_make_super_array(Type *t, SyntaxTree *ps = NULL);


extern const string IDENTIFIER_CLASS;
extern const string IDENTIFIER_FUNC_INIT;
extern const string IDENTIFIER_FUNC_DELETE;
extern const string IDENTIFIER_FUNC_ASSIGN;
extern const string IDENTIFIER_SUPER;
extern const string IDENTIFIER_SELF;
extern const string IDENTIFIER_EXTENDS;
extern const string IDENTIFIER_STATIC;
extern const string IDENTIFIER_NEW;
extern const string IDENTIFIER_DELETE;
extern const string IDENTIFIER_NAMESPACE;
extern const string IDENTIFIER_RETURN_VAR;
extern const string IDENTIFIER_VTABLE_VAR;
extern const string IDENTIFIER_ENUM;
extern const string IDENTIFIER_CONST;
extern const string IDENTIFIER_OVERRIDE;
extern const string IDENTIFIER_VIRTUAL;
extern const string IDENTIFIER_EXTERN;
extern const string IDENTIFIER_USE;
extern const string IDENTIFIER_RETURN;
extern const string IDENTIFIER_IF;
extern const string IDENTIFIER_ELSE;
extern const string IDENTIFIER_WHILE;
extern const string IDENTIFIER_FOR;
extern const string IDENTIFIER_IN;
extern const string IDENTIFIER_BREAK;
extern const string IDENTIFIER_CONTINUE;
extern const string IDENTIFIER_AND;
extern const string IDENTIFIER_OR;
extern const string IDENTIFIER_XOR;
extern const string IDENTIFIER_NOT;
extern const string IDENTIFIER_ASM;


//--------------------------------------------------------------------------------------------------
// operators
enum
{
	OPERATOR_ASSIGN,        //  =
	OPERATOR_ADD,           //  +
	OPERATOR_SUBTRACT,      //  -
	OPERATOR_MULTIPLY,      //  *
	OPERATOR_DIVIDE,        //  /
	OPERATOR_ADDS,          // +=
	OPERATOR_SUBTRACTS,     // -=
	OPERATOR_MULTIPLYS,     // *=
	OPERATOR_DIVIDES,       // /=
	OPERATOR_EQUAL,         // ==
	OPERATOR_NOTEQUAL,      // !=
	OPERATOR_NEGATE,        //  !
	OPERATOR_SMALLER,       //  <
	OPERATOR_GREATER,       //  >
	OPERATOR_SMALLER_EQUAL, // <=
	OPERATOR_GREATER_EQUAL, // >=
	OPERATOR_AND,           // and
	OPERATOR_OR,            // or
	OPERATOR_MODULO,        //  %
	OPERATOR_BIT_AND,       //  &
	OPERATOR_BIT_OR,        //  |
	OPERATOR_SHIFT_LEFT,    // <<
	OPERATOR_SHIFT_RIGHT,   // >>
	OPERATOR_INCREASE,      // ++
	OPERATOR_DECREASE,      // --
	NUM_PRIMITIVE_OPERATORS
};

struct PrimitiveOperator
{
	string name;
	int id;
	bool left_modifiable;
	unsigned char level; // order of operators ("Punkt vor Strich")
	string function_name;
};
extern int NumPrimitiveOperators;
extern PrimitiveOperator PrimitiveOperators[];
struct PreOperator
{
	int primitive_id;
	Type *return_type, *param_type_1, *param_type_2;
	void *func;
	string str() const;
};
extern Array<PreOperator> PreOperators;



enum
{
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
	OperatorInt64Assign,
	OperatorInt64Add,
	OperatorInt64Subtract,
	OperatorInt64Multiply,
	OperatorInt64Divide,
	OperatorInt64AddS,
	OperatorInt64SubtractS,
	OperatorInt64MultiplyS,
	OperatorInt64DivideS,
	OperatorInt64Modulo,
	OperatorInt64Equal,
	OperatorInt64NotEqual,
	OperatorInt64Greater,
	OperatorInt64GreaterEqual,
	OperatorInt64Smaller,
	OperatorInt64SmallerEqual,
	OperatorInt64BitAnd,
	OperatorInt64BitOr,
	OperatorInt64ShiftRight,
	OperatorInt64ShiftLeft,
	OperatorInt64Negate,
	OperatorInt64Increase,
	OperatorInt64Decrease,
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
	OperatorFloat64Assign,
	OperatorFloat64Add,
	OperatorFloat64Subtract,
	OperatorFloat64Multiply,
	OperatorFloat64MultiplyFI,
	OperatorFloat64MultiplyIF,
	OperatorFloat64Divide,
	OperatorFloat64AddS,
	OperatorFloat64SubtractS,
	OperatorFloat64MultiplyS,
	OperatorFloat64DivideS,
	OperatorFloat64Equal,
	OperatorFloat64NotEqual,
	OperatorFloat64Greater,
	OperatorFloat64GreaterEqual,
	OperatorFloat64Smaller,
	OperatorFloat64SmallerEqual,
	OperatorFloat64Negate,
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
	OperatorVectorDivideVF,
	OperatorVectorAddS,
	OperatorVectorSubtractS,
	OperatorVectorMultiplyS,
	OperatorVectorDivideS,
	OperatorVectorNegate,
};




//--------------------------------------------------------------------------------------------------
// commands

struct PreCommandParam
{
	string name;
	Type *type;
};
struct PreCommand
{
	string name;
	Type *return_type;
	Array<PreCommandParam> param;
	int package;
	bool hide_docu;
};
extern Array<PreCommand> PreCommands;


enum
{
	// structural commands
	COMMAND_RETURN,
	COMMAND_IF,
	COMMAND_IF_ELSE,
	COMMAND_WHILE,
	COMMAND_FOR,
	COMMAND_BREAK,
	COMMAND_CONTINUE,
	COMMAND_NEW,
	COMMAND_DELETE,
	COMMAND_SIZEOF,
	COMMAND_WAIT,
	COMMAND_WAIT_RT,
	COMMAND_WAIT_ONE_FRAME,
	COMMAND_ASM,
	NUM_INTERN_PRE_COMMANDS,
	COMMAND_INLINE_FLOAT_TO_INT,
	COMMAND_INLINE_FLOAT_TO_FLOAT64,
	COMMAND_INLINE_FLOAT64_TO_FLOAT,
	COMMAND_INLINE_INT_TO_FLOAT,
	COMMAND_INLINE_INT_TO_INT64,
	COMMAND_INLINE_INT64_TO_INT,
	COMMAND_INLINE_INT_TO_CHAR,
	COMMAND_INLINE_CHAR_TO_INT,
	COMMAND_INLINE_POINTER_TO_BOOL,
	COMMAND_INLINE_COMPLEX_SET,
	COMMAND_INLINE_VECTOR_SET,
	COMMAND_INLINE_RECT_SET,
	COMMAND_INLINE_COLOR_SET,
};





//--------------------------------------------------------------------------------------------------
// type casting

typedef string t_cast_func(string);
struct TypeCast
{
	int penalty;
	Type *source, *dest;
	int kind;
	int func_no;
	Script *script;
	t_cast_func *func;
};
extern Array<TypeCast> TypeCasts;


typedef void t_func();

enum
{
	ABI_GNU_32,
	ABI_GNU_64,
	ABI_WINDOWS_32,
	ABI_WINDOWS_64,
	ABI_GNU_ARM_32,
	ABI_GNU_ARM_64,
};

struct CompilerConfiguration
{
	int instruction_set;
	int abi;
	bool allow_std_lib;

	int stack_size;
	int pointer_size;
	int super_array_size;

	bool allow_simplification;
	bool allow_registers;

	string directory;
	bool verbose;
	bool compile_silently;
	bool show_compiler_stats;
	bool use_const_as_global_var;

	bool compile_os;
	bool no_function_frame;
	bool add_entry_point;
	bool override_variables_offset;
	long long variables_offset;
	bool override_code_origin;
	long long code_origin;

	int stack_mem_align;
	int function_align;
	int stack_frame_align;

};

extern CompilerConfiguration config;




template<typename T>
void* mf(T tmf)
{
	union{
		T f;
		struct{
			long a;
			long b;
		};
	}pp;
	pp.a = 0;
	pp.b = 0;
	pp.f = tmf;

	// on ARM the "virtual bit" is in <b>, on x86 it is in <a>
	return (void*)(pp.a | (pp.b & 1));
}


void Init(int instruction_set = -1, int abi = -1, bool allow_std_lib = true);
void End();



void ResetExternalData();
void LinkExternal(const string &name, void *pointer);
template<typename T>
void LinkExternalClassFunc(const string &name, T pointer)
{
	LinkExternal(name, mf(pointer));
}
void DeclareClassSize(const string &class_name, int offset);
void DeclareClassOffset(const string &class_name, const string &element, int offset);
void DeclareClassVirtualIndex(const string &class_name, const string &func, void *p, void *instance);

void *GetExternalLink(const string &name);
int ProcessClassOffset(const string &class_name, const string &element, int offset);
int ProcessClassSize(const string &class_name, int size);
int ProcessClassNumVirtuals(const string &class_name, int num_virtual);




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
