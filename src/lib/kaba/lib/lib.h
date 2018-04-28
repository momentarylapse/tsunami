/*----------------------------------------------------------------------------*\
| Kaba Lib                                                                     |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(LIB_H__INCLUDED_)
#define LIB_H__INCLUDED_

namespace Kaba{


#define MAX_OPCODE				(2*65536)	// max. amount of opcode
#define MAX_THREAD_OPCODE		1024
#define DEFAULT_STACK_SIZE		32768	// max. amount of stack memory


//#define mem_align(x)	((x) + (4 - (x) % 4) % 4)
#define mem_align(x, n)		((((x) + (n) - 1) / (n) ) * (n))

extern string LibVersion;

class SyntaxTree;
class Class;
struct Value;


void script_make_super_array(Class *t, SyntaxTree *ps = NULL);


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
extern const string IDENTIFIER_RAISE;
extern const string IDENTIFIER_TRY;
extern const string IDENTIFIER_EXCEPT;
extern const string IDENTIFIER_IF;
extern const string IDENTIFIER_ELSE;
extern const string IDENTIFIER_WHILE;
extern const string IDENTIFIER_FOR;
extern const string IDENTIFIER_IN;
extern const string IDENTIFIER_BREAK;
extern const string IDENTIFIER_CONTINUE;
extern const string IDENTIFIER_PASS;
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
extern PrimitiveOperator PrimitiveOperators[];





//--------------------------------------------------------------------------------------------------
// commands

struct Statement
{
	string name;
	int num_params;
};
extern Array<Statement> Statements;


// statements
enum
{
	STATEMENT_RETURN,
	STATEMENT_IF,
	STATEMENT_IF_ELSE,
	STATEMENT_WHILE,
	STATEMENT_FOR,
	STATEMENT_BREAK,
	STATEMENT_CONTINUE,
	STATEMENT_NEW,
	STATEMENT_DELETE,
	STATEMENT_SIZEOF,
	STATEMENT_TYPE,
	STATEMENT_ASM,
	STATEMENT_RAISE,
	STATEMENT_TRY,
	STATEMENT_EXCEPT,
	STATEMENT_PASS,
	NUM_STATEMENTS
};



// inline commands
enum
{
	INLINE_FLOAT_TO_INT,
	INLINE_FLOAT_TO_FLOAT64,
	INLINE_FLOAT64_TO_FLOAT,
	INLINE_INT_TO_FLOAT,
	INLINE_INT_TO_INT64,
	INLINE_INT64_TO_INT,
	INLINE_INT_TO_CHAR,
	INLINE_CHAR_TO_INT,
	INLINE_POINTER_TO_BOOL,
	INLINE_COMPLEX_SET,
	INLINE_VECTOR_SET,
	INLINE_RECT_SET,
	INLINE_COLOR_SET,

	INLINE_POINTER_ASSIGN,
	INLINE_POINTER_EQUAL,
	INLINE_POINTER_NOT_EQUAL,

	INLINE_CHAR_ASSIGN,
	INLINE_CHAR_EQUAL,
	INLINE_CHAR_NOT_EQUAL,
	INLINE_CHAR_GREATER,
	INLINE_CHAR_GREATER_EQUAL,
	INLINE_CHAR_SMALLER,
	INLINE_CHAR_SMALLER_EQUAL,
	INLINE_CHAR_ADD,
	INLINE_CHAR_ADD_ASSIGN,
	INLINE_CHAR_SUBTRACT,
	INLINE_CHAR_SUBTRACT_ASSIGN,
	INLINE_CHAR_AND,
	INLINE_CHAR_OR,
	INLINE_CHAR_NEGATE,

	INLINE_BOOL_ASSIGN,
	INLINE_BOOL_EQUAL,
	INLINE_BOOL_NOT_EQUAL,
	INLINE_BOOL_AND,
	INLINE_BOOL_OR,
	INLINE_BOOL_NEGATE,

	INLINE_INT_ASSIGN,
	INLINE_INT_ADD,
	INLINE_INT_ADD_ASSIGN,
	INLINE_INT_SUBTRACT,
	INLINE_INT_SUBTRACT_ASSIGN,
	INLINE_INT_MULTIPLY,
	INLINE_INT_MULTIPLY_ASSIGN,
	INLINE_INT_DIVIDE,
	INLINE_INT_DIVIDE_ASSIGN,
	INLINE_INT_MODULO,
	INLINE_INT_EQUAL,
	INLINE_INT_NOT_EQUAL,
	INLINE_INT_GREATER,
	INLINE_INT_GREATER_EQUAL,
	INLINE_INT_SMALLER,
	INLINE_INT_SMALLER_EQUAL,
	INLINE_INT_AND,
	INLINE_INT_OR,
	INLINE_INT_SHIFT_RIGHT,
	INLINE_INT_SHIFT_LEFT,
	INLINE_INT_NEGATE,
	INLINE_INT_INCREASE,
	INLINE_INT_DECREASE,

	INLINE_INT64_ASSIGN,
	INLINE_INT64_ADD,
	INLINE_INT64_ADD_INT,
	INLINE_INT64_ADD_ASSIGN,
	INLINE_INT64_SUBTRACT,
	INLINE_INT64_SUBTRACT_ASSIGN,
	INLINE_INT64_MULTIPLY,
	INLINE_INT64_MULTIPLY_ASSIGN,
	INLINE_INT64_DIVIDE,
	INLINE_INT64_DIVIDE_ASSIGN,
	INLINE_INT64_MODULO,
	INLINE_INT64_EQUAL,
	INLINE_INT64_NOT_EQUAL,
	INLINE_INT64_GREATER,
	INLINE_INT64_GREATER_EQUAL,
	INLINE_INT64_SMALLER,
	INLINE_INT64_SMALLER_EQUAL,
	INLINE_INT64_AND,
	INLINE_INT64_OR,
	INLINE_INT64_SHIFT_RIGHT,
	INLINE_INT64_SHIFT_LEFT,
	INLINE_INT64_NEGATE,
	INLINE_INT64_INCREASE,
	INLINE_INT64_DECREASE,

	INLINE_FLOAT_ASSIGN,
	INLINE_FLOAT_ADD,
	INLINE_FLOAT_ADD_ASSIGN,
	INLINE_FLOAT_SUBTARCT,
	INLINE_FLOAT_SUBTRACT_ASSIGN,
	INLINE_FLOAT_MULTIPLY,
	INLINE_FLOAT_MULTIPLY_FI,
	INLINE_FLOAT_MULTIPLY_IF,
	INLINE_FLOAT_MULTIPLY_ASSIGN,
	INLINE_FLOAT_DIVIDE,
	INLINE_FLOAT_DIVIDE_ASSIGN,
	INLINE_FLOAT_EQUAL,
	INLINE_FLOAT_NOT_EQUAL,
	INLINE_FLOAT_GREATER,
	INLINE_FLOAT_GREATER_EQUAL,
	INLINE_FLOAT_SMALLER,
	INLINE_FLOAT_SMALLER_EQUAL,
	INLINE_FLOAT_NEGATE,

	INLINE_FLOAT64_ASSIGN,
	INLINE_FLOAT64_ADD,
	INLINE_FLOAT64_ADD_ASSIGN,
	INLINE_FLOAT64_SUBTRACT,
	INLINE_FLOAT64_SUBTRACT_ASSIGN,
	INLINE_FLOAT64_MULTIPLY,
	INLINE_FLOAT64_MULTIPLY_FI,
	INLINE_FLOAT64_MULTIPLY_IF,
	INLINE_FLOAT64_MULTIPLY_ASSIGN,
	INLINE_FLOAT64_DIVIDE,
	INLINE_FLOAT64_DIVIDE_ASSIGN,
	INLINE_FLOAT64_EQUAL,
	INLINE_FLOAT64_NOT_EQUAL,
	INLINE_FLOAT64_GREATER,
	INLINE_FLOAT64_GREATER_EQUAL,
	INLINE_FLOAT64_SMALLER,
	INLINE_FLOAT64_SMALLER_EQUAL,
	INLINE_FLOAT64_NEGATE,

//	INLINE_COMPLEX_ASSIGN,
	INLINE_COMPLEX_ADD,
	INLINE_COMPLEX_ADD_ASSIGN,
	INLINE_COMPLEX_SUBTRACT,
	INLINE_COMPLEX_SUBTARCT_ASSIGN,
	INLINE_COMPLEX_MULTIPLY,
	INLINE_COMPLEX_MULTIPLY_FC,
	INLINE_COMPLEX_MULTIPLY_CF,
	INLINE_COMPLEX_MULTIPLY_ASSIGN,
	INLINE_COMPLEX_DIVIDE,
	INLINE_COMPLEX_DIVIDE_ASSIGN,
	INLINE_COMPLEX_EQUAL,
	INLINE_COMPLEX_NEGATE,

	INLINE_CHUNK_ASSIGN,
	INLINE_CHUNK_EQUAL,
	INLINE_CHUNK_NOT_EQUAL,

	INLINE_VECTOR_ADD,
	INLINE_VECTOR_ADD_ASSIGN,
	INLINE_VECTOR_SUBTRACT,
	INLINE_VECTOR_SUBTARCT_ASSIGN,
	INLINE_VECTOR_MULTIPLY_VV,
	INLINE_VECTOR_MULTIPLY_VF,
	INLINE_VECTOR_MULTIPLY_FV,
	INLINE_VECTOR_MULTIPLY_ASSIGN,
	INLINE_VECTOR_DIVIDE_VF,
	INLINE_VECTOR_DIVIDE_ASSIGN,
	INLINE_VECTOR_NEGATE,
};





//--------------------------------------------------------------------------------------------------
// type casting

typedef void t_cast_func(Value&, Value&);
struct TypeCast
{
	int penalty;
	Class *source, *dest;
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

	long long stack_size;
	long long pointer_size;
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
