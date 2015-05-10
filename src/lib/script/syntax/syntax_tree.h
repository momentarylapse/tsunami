#if !defined(SYNTAX_TREE_H__INCLUDED_)
#define SYNTAX_TREE_H__INCLUDED_


#include "lexical.h"
#include "type.h"

namespace Asm{
	struct MetaInfo;
};

namespace Script{

class Script;
class SyntaxTree;

#define SCRIPT_MAX_PARAMS				16		// number of possible parameters per function/command
#define SCRIPT_MAX_STRING_CONST_LENGTH	2048

// macros
struct Define
{
	string Source;
	Array<string> Dest;
};

// for any type of constant used in the script
struct Constant
{
	string name;
	string value;
	Type *type;
	void setInt(int i);
	int getInt();
};

enum
{
	KIND_UNKNOWN,
	// data
	KIND_VAR_LOCAL,
	KIND_VAR_GLOBAL,
	KIND_VAR_FUNCTION,
	KIND_CONSTANT,
	// execution
	KIND_FUNCTION,           // = real function call
	KIND_VIRTUAL_FUNCTION,   // = virtual function call
	KIND_COMPILER_FUNCTION,  // = special internal functions
	KIND_BLOCK,              // = block of commands {...}
	KIND_OPERATOR,
	KIND_PRIMITIVE_OPERATOR, // tentative...
	// data altering
	KIND_ADDRESS_SHIFT,      // = . "struct"
	KIND_ARRAY,              // = []
	KIND_POINTER_AS_ARRAY,   // = []
	KIND_REFERENCE,          // = &
	KIND_DEREFERENCE,        // = *
	KIND_DEREF_ADDRESS_SHIFT,// = ->
	KIND_REF_TO_LOCAL,
	KIND_REF_TO_GLOBAL,
	KIND_REF_TO_CONST,
	KIND_ADDRESS,            // &global (for pre processing address shifts)
	KIND_MEMORY,             // global (but LinkNr = address)
	KIND_LOCAL_ADDRESS,      // &local (for pre processing address shifts)
	KIND_LOCAL_MEMORY,       // local (but LinkNr = address)
	// special
	KIND_TYPE,
	KIND_ARRAY_BUILDER,
	// compilation
	KIND_VAR_TEMP,
	KIND_DEREF_VAR_TEMP,
	KIND_DEREF_VAR_LOCAL,
	KIND_REGISTER,
	KIND_DEREF_REGISTER,
	KIND_MARKER,
	KIND_DEREF_MARKER,
	KIND_IMMEDIATE,
	KIND_GLOBAL_LOOKUP,       // ARM
	KIND_DEREF_GLOBAL_LOOKUP, // ARM
};

struct Command;

// {...}-block
struct Block
{
	int index;
	Array<Command*> command;
	void add(Command *c);
	void set(int index, Command *c);
};

struct Variable
{
	Type *type; // for creating instances
	string name;
	int _offset; // for compilation
	bool is_extern;
};

// user defined functions
struct Function
{
	SyntaxTree *tree;

	string name;
	// parameters (linked to intern variables)
	int num_params;
	// block of code
	Block *block;
	// local variables
	Array<Variable> var;
	Type *literal_param_type[SCRIPT_MAX_PARAMS];
	Type *_class;
	Type *return_type;
	Type *literal_return_type;
	bool is_extern, auto_implement;
	bool is_pure;
	int inline_no;
	// for compilation...
	int _var_size, _param_size;
	int _logical_line_no;
	Function(SyntaxTree *tree, const string &name, Type *return_type);
	int get_var(const string &name);
	int AddVar(const string &name, Type *type);
	void Update(Type *class_type);
};

// single operand/command
struct Command
{
	int kind;
	long long link_no;
	Script *script;
	int ref_count;
	// parameters
	int num_params;
	Command *param[SCRIPT_MAX_PARAMS];
	// linking of class function instances
	Command *instance;
	// return value
	Type *type;
	Command(int kind, long long link_no, Script *script, Type *type);
	Block *block() const;
	void set_num_params(int n);
	void set_param(int index, Command *p);
	void set_instance(Command *p);
};

struct AsmBlock
{
	string block;
	int line;
};

class Script;


// data structures (uncompiled)
class SyntaxTree
{
public:
	SyntaxTree(Script *_script);
	~SyntaxTree();

	void ParseBuffer(const string &buffer, bool just_analyse);
	void AddIncludeData(Script *s);

	void DoError(const string &msg, int overwrite_line = -1);
	void ExpectNoNewline();
	void ExpectNewline();
	void ExpectIndent();
	
	// syntax parsing
	void Parser();
	void ParseAllClassNames();
	void ParseAllFunctionBodies();
	void ParseImport();
	void ParseEnum();
	void ParseClass();
	Function *ParseFunctionHeader(Type *class_type, bool as_extern);
	void ParseFunctionBody(Function *f);
	void ParseClassFunctionHeader(Type *t, bool as_extern, bool as_virtual, bool overwrite);
	bool ParseFunctionCommand(Function *f, ExpressionBuffer::Line *this_line);
	Type *ParseType();
	void ParseVariableDef(bool single, Function *f);
	void ParseGlobalConst(const string &name, Type *type);
	int WhichPrimitiveOperator(const string &name);
	int WhichCompilerFunction(const string &name);
	void CommandSetCompilerFunction(int CF,Command *Com);
	int WhichType(const string &name);
	void AddType();

	// pre compiler
	void PreCompiler(bool just_analyse);
	void HandleMacro(ExpressionBuffer::Line *l, int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);
	void AutoImplementAddVirtualTable(Command *self, Function *f, Type *t);
	void AutoImplementAddChildConstructors(Command *self, Function *f, Type *t);
	void AutoImplementDefaultConstructor(Function *f, Type *t, bool allow_parent_constructor);
	void AutoImplementComplexConstructor(Function *f, Type *t);
	void AutoImplementDestructor(Function *f, Type *t);
	void AutoImplementAssign(Function *f, Type *t);
	void AutoImplementArrayClear(Function *f, Type *t);
	void AutoImplementArrayResize(Function *f, Type *t);
	void AutoImplementArrayAdd(Function *f, Type *t);
	void AutoImplementArrayRemove(Function *f, Type *t);
	void AutoImplementFunctions(Type *t);
	void AddFunctionHeadersForClass(Type *t);

	// syntax analysis
	Type *GetConstantType();
	string GetConstantValue();
	Type *FindType(const string &name);
	Type *AddType(Type *type);
	Type *CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub);
	Type *CreateArrayType(Type *element_type, int num_elements, const string &name_pre = "", const string &suffix = "");
	bool GetExistence(const string &name, Function *f);
	bool GetExistenceShared(const string &name);
	void LinkMostImportantOperator(Array<Command*> &Operand, Array<Command*> &Operator, Array<int> &op_exp);
	Command *LinkOperator(int op_no, Command *param1, Command *param2);
	Command *GetOperandExtension(Command *Operand, Function *f);
	Command *GetOperandExtensionElement(Command *Operand, Function *f);
	Command *GetOperandExtensionArray(Command *Operand, Function *f);
	Command *GetCommand(Function *f);
	void ParseCompleteCommand(Block *block, Function *f);
	Command *GetOperand(Function *f);
	Command *GetPrimitiveOperator(Function *f);
	void FindFunctionParameters(int &np, Type **WantedType, Function *f, Command *cmd);
	void FindFunctionSingleParameter(int p, Type **WantedType, Function *f, Command *cmd);
	Command *GetFunctionCall(const string &f_name, Command *Operand, Function *f);
	Command *DoClassFunction(Command *ob, ClassFunction &cf, Function *f);
	Command *GetSpecialFunctionCall(const string &f_name, Command *Operand, Function *f);
	Command *CheckParamLink(Command *link, Type *type, const string &f_name = "", int param_no = -1);
	void ParseSpecialCommand(Block *block, Function *f);
	void ParseSpecialCommandFor(Block *block, Function *f);
	void ParseSpecialCommandForall(Block *block, Function *f);
	void ParseSpecialCommandWhile(Block *block, Function *f);
	void ParseSpecialCommandBreak(Block *block, Function *f);
	void ParseSpecialCommandContinue(Block *block, Function *f);
	void ParseSpecialCommandReturn(Block *block, Function *f);
	void ParseSpecialCommandIf(Block *block, Function *f);

	void CreateAsmMetaInfo();

	// neccessary conversions
	void ConvertCallByReference();
	void BreakDownComplicatedCommands();
	Command *BreakDownComplicatedCommand(Command *c);
	void MapLocalVariablesToStack();

	// data creation
	int AddConstant(Type *type);
	Block *AddBlock();
	Function *AddFunction(const string &name, Type *type);

	// command
	Command *AddCommand(int kind, long long link_no, Type *type);
	Command *AddCommand(int kind, long long link_no, Type *type, Script *s);
	Command *add_command_compilerfunc(int cf);
	Command *add_command_classfunc(ClassFunction *f, Command *inst, bool force_non_virtual = false);
	Command *add_command_func(Script *script, int no, Type *return_type);
	Command *add_command_const(int nc);
	Command *add_command_operator(Command *p1, Command *p2, int op);
	Command *add_command_local_var(int no, Type *type);
	Command *add_command_parray(Command *p, Command *index, Type *type);
	Command *add_command_block(Block *b);
	Command *cp_command(Command *c);
	Command *ref_command(Command *sub, Type *overwrite_type = NULL);
	Command *deref_command(Command *sub, Type *overwrite_type = NULL);
	Command *shift_command(Command *sub, bool deref, int shift, Type *type);

	// pre processor
	Command *PreProcessCommand(Command *c);
	void PreProcessor();
	Command *PreProcessCommandAddresses(Command *c);
	void PreProcessorAddresses();
	void Simplify();

	// debug displaying
	void ShowCommand(Command *c);
	void ShowFunction(Function *f);
	void ShowBlock(Block *b);
	void Show();

// data

	ExpressionBuffer Exp;
	Command GetExistenceLink;

	// compiler options
	bool flag_immortal;
	bool flag_string_const_as_cstring;

	Array<Type*> types;
	Array<Script*> includes;
	Array<Define> defines;
	Asm::MetaInfo *asm_meta_info;
	Array<AsmBlock> asm_blocks;
	Array<Constant> constants;
	Array<Block*> blocks;
	Array<Function*> functions;
	Array<Command*> commands;

	Function root_of_all_evil;

	Script *script;
	Function *cur_func;
	int for_index_count;
};

string Kind2Str(int kind);
string LinkNr2Str(SyntaxTree *s,int kind,int nr);




};

#endif
