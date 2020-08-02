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

class SyntaxTree;
class Class;
class Value;
class Function;
class Variable;
class Constant;


void script_make_super_array(Class *t, SyntaxTree *ps = nullptr);
void script_make_dict(Class *t, SyntaxTree *ps = nullptr);


extern const string IDENTIFIER_CLASS;
extern const string IDENTIFIER_FUNC_INIT;
extern const string IDENTIFIER_FUNC_DELETE;
extern const string IDENTIFIER_FUNC_ASSIGN;
extern const string IDENTIFIER_FUNC_GET;
extern const string IDENTIFIER_FUNC_SET;
extern const string IDENTIFIER_FUNC_LENGTH;
extern const string IDENTIFIER_FUNC_STR;
extern const string IDENTIFIER_FUNC_REPR;
extern const string IDENTIFIER_FUNC_SUBARRAY;
extern const string IDENTIFIER_SUPER;
extern const string IDENTIFIER_SELF;
extern const string IDENTIFIER_EXTENDS;
extern const string IDENTIFIER_STATIC;
extern const string IDENTIFIER_NEW;
extern const string IDENTIFIER_DELETE;
extern const string IDENTIFIER_SIZEOF;
extern const string IDENTIFIER_TYPE;
extern const string IDENTIFIER_STR;
extern const string IDENTIFIER_REPR;
extern const string IDENTIFIER_LEN;
extern const string IDENTIFIER_LET;
extern const string IDENTIFIER_NAMESPACE;
extern const string IDENTIFIER_RETURN_VAR;
extern const string IDENTIFIER_VTABLE_VAR;
extern const string IDENTIFIER_ENUM;
extern const string IDENTIFIER_CONST;
extern const string IDENTIFIER_OUT;
extern const string IDENTIFIER_OVERRIDE;
extern const string IDENTIFIER_VIRTUAL;
extern const string IDENTIFIER_EXTERN;
extern const string IDENTIFIER_SELFREF;
extern const string IDENTIFIER_USE;
extern const string IDENTIFIER_IMPORT;
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
extern const string IDENTIFIER_IS;
extern const string IDENTIFIER_ASM;
extern const string IDENTIFIER_MAP;
extern const string IDENTIFIER_LAMBDA;
extern const string IDENTIFIER_SORTED;
extern const string IDENTIFIER_DYN;
extern const string IDENTIFIER_CALL;


//--------------------------------------------------------------------------------------------------
// operators
enum class OperatorID {
	NONE = -1,
	ASSIGN,        //  =
	ADD,           //  +
	SUBTRACT,      //  -
	MULTIPLY,      //  *
	DIVIDE,        //  /
	NEGATIVE,      //  -
	ADDS,          // +=
	SUBTRACTS,     // -=
	MULTIPLYS,     // *=
	DIVIDES,       // /=
	EQUAL,         // ==
	NOTEQUAL,      // !=
	NEGATE,        //  not
	SMALLER,       //  <
	GREATER,       //  >
	SMALLER_EQUAL, // <=
	GREATER_EQUAL, // >=
	AND,           // and
	OR,            // or
	MODULO,        //  %
	BIT_AND,       //  &
	BIT_OR,        //  |
	SHIFT_LEFT,    // <<
	SHIFT_RIGHT,   // >>
	INCREASE,      // ++
	DECREASE,      // --
	IS,            // is
	IN,            // in
	EXTENDS,       // extends
	EXPONENT,      // ^
	COMMA,         // ,
	DEREFERENCE,   // *
	REFERENCE,     // &
	ARRAY,         // [...]
	_COUNT_
};

class PrimitiveOperator {
public:
	string name;
	OperatorID id;
	bool left_modifiable;
	unsigned char level; // order of operators ("Punkt vor Strich")
	string function_name;
	int param_flags; // 1 = only left, 2 = only right, 3 = both
	bool order_inverted; // (param, instance) instead of (instance, param)
};
extern PrimitiveOperator PrimitiveOperators[];





//--------------------------------------------------------------------------------------------------
// commands


// statements
enum class StatementID {
	RETURN,
	IF,
	IF_ELSE,
	WHILE,
	FOR_ARRAY,
	FOR_RANGE,
	FOR_DIGEST,
	BREAK,
	CONTINUE,
	NEW,
	DELETE,
	SIZEOF,
	TYPE,
	ASM,
	//RAISE,
	TRY,
	EXCEPT,
	PASS,
	STR,
	REPR,
	LEN,
	LET,
	MAP,
	LAMBDA,
	SORTED,
	DYN,
	CALL
};

class Statement {
public:
	string name;
	int num_params;
	StatementID id;
};
extern Array<Statement*> Statements;
Statement *statement_from_id(StatementID id);


// inline commands
enum class InlineID {
	NONE = -1,
	FLOAT_TO_INT,
	FLOAT_TO_FLOAT64,
	FLOAT64_TO_FLOAT,
	INT_TO_FLOAT,
	INT_TO_INT64,
	INT64_TO_INT,
	INT_TO_CHAR,
	CHAR_TO_INT,
	POINTER_TO_BOOL,
	COMPLEX_SET,
	VECTOR_SET,
	RECT_SET,
	COLOR_SET,

	POINTER_ASSIGN,
	POINTER_EQUAL,
	POINTER_NOT_EQUAL,

	CHAR_ASSIGN,
	CHAR_EQUAL,
	CHAR_NOT_EQUAL,
	CHAR_GREATER,
	CHAR_GREATER_EQUAL,
	CHAR_SMALLER,
	CHAR_SMALLER_EQUAL,
	CHAR_ADD,
	CHAR_ADD_ASSIGN,
	CHAR_SUBTRACT,
	CHAR_SUBTRACT_ASSIGN,
	CHAR_AND,
	CHAR_OR,
	CHAR_NEGATE,

	BOOL_ASSIGN,
	BOOL_EQUAL,
	BOOL_NOT_EQUAL,
	BOOL_AND,
	BOOL_OR,
	BOOL_NEGATE,

	INT_ASSIGN,
	INT_ADD,
	INT_ADD_ASSIGN,
	INT_SUBTRACT,
	INT_SUBTRACT_ASSIGN,
	INT_MULTIPLY,
	INT_MULTIPLY_ASSIGN,
	INT_DIVIDE,
	INT_DIVIDE_ASSIGN,
	INT_MODULO,
	INT_EQUAL,
	INT_NOT_EQUAL,
	INT_GREATER,
	INT_GREATER_EQUAL,
	INT_SMALLER,
	INT_SMALLER_EQUAL,
	INT_AND,
	INT_OR,
	INT_SHIFT_RIGHT,
	INT_SHIFT_LEFT,
	INT_NEGATE,
	INT_INCREASE,
	INT_DECREASE,

	INT64_ASSIGN,
	INT64_ADD,
	INT64_ADD_INT,
	INT64_ADD_ASSIGN,
	INT64_SUBTRACT,
	INT64_SUBTRACT_ASSIGN,
	INT64_MULTIPLY,
	INT64_MULTIPLY_ASSIGN,
	INT64_DIVIDE,
	INT64_DIVIDE_ASSIGN,
	INT64_MODULO,
	INT64_EQUAL,
	INT64_NOT_EQUAL,
	INT64_GREATER,
	INT64_GREATER_EQUAL,
	INT64_SMALLER,
	INT64_SMALLER_EQUAL,
	INT64_AND,
	INT64_OR,
	INT64_SHIFT_RIGHT,
	INT64_SHIFT_LEFT,
	INT64_NEGATE,
	INT64_INCREASE,
	INT64_DECREASE,

	FLOAT_ASSIGN,
	FLOAT_ADD,
	FLOAT_ADD_ASSIGN,
	FLOAT_SUBTARCT,
	FLOAT_SUBTRACT_ASSIGN,
	FLOAT_MULTIPLY,
	FLOAT_MULTIPLY_ASSIGN,
	FLOAT_DIVIDE,
	FLOAT_DIVIDE_ASSIGN,
	FLOAT_EQUAL,
	FLOAT_NOT_EQUAL,
	FLOAT_GREATER,
	FLOAT_GREATER_EQUAL,
	FLOAT_SMALLER,
	FLOAT_SMALLER_EQUAL,
	FLOAT_NEGATE,

	FLOAT64_ASSIGN,
	FLOAT64_ADD,
	FLOAT64_ADD_ASSIGN,
	FLOAT64_SUBTRACT,
	FLOAT64_SUBTRACT_ASSIGN,
	FLOAT64_MULTIPLY,
	FLOAT64_MULTIPLY_ASSIGN,
	FLOAT64_DIVIDE,
	FLOAT64_DIVIDE_ASSIGN,
	FLOAT64_EQUAL,
	FLOAT64_NOT_EQUAL,
	FLOAT64_GREATER,
	FLOAT64_GREATER_EQUAL,
	FLOAT64_SMALLER,
	FLOAT64_SMALLER_EQUAL,
	FLOAT64_NEGATE,

//	COMPLEX_ASSIGN,
	COMPLEX_ADD,
	COMPLEX_ADD_ASSIGN,
	COMPLEX_SUBTRACT,
	COMPLEX_SUBTARCT_ASSIGN,
	COMPLEX_MULTIPLY,
	COMPLEX_MULTIPLY_FC,
	COMPLEX_MULTIPLY_CF,
	COMPLEX_MULTIPLY_ASSIGN,
	COMPLEX_DIVIDE,
	COMPLEX_DIVIDE_ASSIGN,
	COMPLEX_NEGATE,

	CHUNK_ASSIGN,
	CHUNK_EQUAL,
	CHUNK_NOT_EQUAL,

	VECTOR_ADD,
	VECTOR_ADD_ASSIGN,
	VECTOR_SUBTRACT,
	VECTOR_SUBTARCT_ASSIGN,
	VECTOR_MULTIPLY_VV,
	VECTOR_MULTIPLY_VF,
	VECTOR_MULTIPLY_FV,
	VECTOR_MULTIPLY_ASSIGN,
	VECTOR_DIVIDE_VF,
	VECTOR_DIVIDE_ASSIGN,
	VECTOR_NEGATE,
};





//--------------------------------------------------------------------------------------------------
// type casting

class TypeCast {
public:
	int penalty;
	const Class *source, *dest;
	Function *f;
};
extern Array<TypeCast> TypeCasts;


typedef void t_func();

enum class Abi {
	NATIVE = -1,
	GNU_32,
	GNU_64,
	WINDOWS_32,
	WINDOWS_64,
	GNU_ARM_32,
	GNU_ARM_64,
};

class CompilerConfiguration {
public:
	Asm::InstructionSet instruction_set;
	Abi abi;
	bool allow_std_lib;

	int64 stack_size;
	int64 pointer_size;
	int super_array_size;

	bool allow_simplification;
	bool allow_registers;

	Path directory;
	bool verbose;
	string verbose_func_filter;
	string verbose_stage_filter;
	bool allow_output(const Function *f, const string &stage);
	bool allow_output_func(const Function *f);
	bool allow_output_stage(const string &stage);
	bool compile_silently;
	bool show_compiler_stats;

	bool compile_os;
	bool no_function_frame;
	bool add_entry_point;
	bool override_variables_offset;
	int64 variables_offset;
	bool override_code_origin;
	int64 code_origin;

	int stack_mem_align;
	int function_align;
	int stack_frame_align;

};

extern CompilerConfiguration config;




template<typename T>
void* mf(T tmf) {
	union {
		T f;
		struct {
			int_p a;
			int_p b;
		};
	} pp;
	pp.a = 0;
	pp.b = 0;
	pp.f = tmf;

	// on ARM the "virtual bit" is in <b>, on x86 it is in <a>
	return (void*)(pp.a | (pp.b & 1));
}


void init(Asm::InstructionSet instruction_set = Asm::InstructionSet::NATIVE, Abi abi = Abi::NATIVE, bool allow_std_lib = true);
void clean_up();



void reset_external_data();
void link_external(const string &name, void *pointer);
template<typename T>
void link_external_class_func(const string &name, T pointer) {
	link_external(name, mf(pointer));
}
void declare_class_size(const string &class_name, int offset);
void _declare_class_element(const string &name, int offset);
template<class T>
void declare_class_element(const string &name, T pointer) {
	_declare_class_element(name, *(int*)(void*)&pointer);
}
void _link_external_virtual(const string &name, void *p, void *instance);
template<class T>
void link_external_virtual(const string &name, T pointer, void *instance) {
	_link_external_virtual(name, mf(pointer), instance);
}

void *get_external_link(const string &name);
int process_class_offset(const string &class_name, const string &element, int offset);
int process_class_size(const string &class_name, int size);
int process_class_num_virtuals(const string &class_name, int num_virtual);




//--------------------------------------------------------------------------------------------------
// packages

extern Array<Script*> packages;


};

#endif
