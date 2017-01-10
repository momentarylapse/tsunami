#if !defined(SYNTAX_TREE_H__INCLUDED_)
#define SYNTAX_TREE_H__INCLUDED_


#include "class.h"
#include "lexical.h"

namespace Asm{
	struct MetaInfo;
};

namespace Kaba{

class Script;
class SyntaxTree;

#define MAX_STRING_CONST_LENGTH	2048

// macros
struct Define
{
	string source;
	Array<string> dest;
};

// for any type of constant used in the script
struct Constant
{
	string name;
	string value;
	Class *type;
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
	KIND_INLINE_FUNCTION,    // = function defined inside the compiler...
	KIND_STATEMENT,          // = if/while/break/...
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
	Array<Command*> commands;
	Array<int> vars;
	Function *function;
	Block *parent;
	int level;
	void add(Command *c);
	void set(int index, Command *c);

	int get_var(const string &name);
	int add_var(const string &name, Class *type);
};

struct Variable
{
	Class *type; // for creating instances
	string name;
	long long _offset; // for compilation
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
	Array<Class*> literal_param_type;
	Class *_class;
	Class *return_type;
	Class *literal_return_type;
	bool is_extern, auto_implement;
	bool is_pure;
	int inline_no;
	// for compilation...
	long long _var_size, _param_size;
	int _logical_line_no;
	int _exp_no;
	Function(SyntaxTree *tree, const string &name, Class *return_type);
	int __get_var(const string &name);
	void Update(Class *class_type);
};

// single operand/command
struct Command
{
	int kind;
	long long link_no;
	Script *script;
	int ref_count;
	// parameters
	Array<Command*> param;
	// linking of class function instances
	Command *instance;
	// return value
	Class *type;
	Command();
	Command(int kind, long long link_no, Script *script, Class *type);
	Block *as_block() const;
	Function *as_func() const;
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

	void DoError(const string &msg, int override_exp_no = -1, int override_line = -1);
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
	Function *ParseFunctionHeader(Class *class_type, bool as_extern);
	void SkipParsingFunctionBody();
	void ParseFunctionBody(Function *f);
	void ParseClassFunctionHeader(Class *t, bool as_extern, bool as_virtual, bool override);
	bool ParseFunctionCommand(Function *f, ExpressionBuffer::Line *this_line);
	Class *ParseType();
	void ParseVariableDef(bool single, Block *block);
	void ParseGlobalConst(const string &name, Class *type);
	int WhichPrimitiveOperator(const string &name);
	int WhichStatement(const string &name);
	int WhichType(const string &name);
	void AddType();

	// pre compiler
	void PreCompiler(bool just_analyse);
	void HandleMacro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);
	void AutoImplementAddVirtualTable(Command *self, Function *f, Class *t);
	void AutoImplementAddChildConstructors(Command *self, Function *f, Class *t);
	void AutoImplementDefaultConstructor(Function *f, Class *t, bool allow_parent_constructor);
	void AutoImplementComplexConstructor(Function *f, Class *t);
	void AutoImplementDestructor(Function *f, Class *t);
	void AutoImplementAssign(Function *f, Class *t);
	void AutoImplementArrayClear(Function *f, Class *t);
	void AutoImplementArrayResize(Function *f, Class *t);
	void AutoImplementArrayAdd(Function *f, Class *t);
	void AutoImplementArrayRemove(Function *f, Class *t);
	void AutoImplementFunctions(Class *t);
	void AddFunctionHeadersForClass(Class *t);

	// syntax analysis
	Class *GetConstantType(const string &str);
	string GetConstantValue(const string &str);
	Class *FindType(const string &name);
	Class *AddType(Class *type);
	Class *CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Class *sub);
	Class *CreateArrayType(Class *element_type, int num_elements, const string &name_pre = "", const string &suffix = "");
	Array<Command> GetExistence(const string &name, Block *block);
	Array<Command> GetExistenceShared(const string &name);
	void LinkMostImportantOperator(Array<Command*> &operand, Array<Command*> &_operator, Array<int> &op_exp);
	Command *LinkOperator(int op_no, Command *param1, Command *param2);
	Command *GetOperandExtension(Command *operand, Block *block);
	Command *GetOperandExtensionElement(Command *operand, Block *block);
	Command *GetOperandExtensionArray(Command *operand, Block *block);
	Command *GetCommand(Block *block);
	void ParseCompleteCommand(Block *block);
	Command *GetOperand(Block *block);
	Command *GetPrimitiveOperator(Block *block);
	Array<Command*> FindFunctionParameters(Block *block);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, Command *cmd);
	Array<Class*> GetFunctionWantedParams(Command &link);
	Command *GetFunctionCall(const string &f_name, Array<Command> &links, Block *block);
	Command *DoClassFunction(Command *ob, Array<ClassFunction> &cfs, Block *block);
	Command *GetSpecialFunctionCall(const string &f_name, Command &link, Block *block);
	Command *CheckParamLink(Command *link, Class *type, const string &f_name = "", int param_no = -1);
	void ParseStatement(Block *block);
	void ParseStatementFor(Block *block);
	void ParseStatementForall(Block *block);
	void ParseStatementWhile(Block *block);
	void ParseStatementBreak(Block *block);
	void ParseStatementContinue(Block *block);
	void ParseStatementReturn(Block *block);
	void ParseStatementIf(Block *block);

	void CreateAsmMetaInfo();

	// neccessary conversions
	void ConvertCallByReference();
	void BreakDownComplicatedCommands();
	Command *BreakDownComplicatedCommand(Command *c);
	void MapLocalVariablesToStack();

	// data creation
	int AddConstant(Class *type);
	Block *AddBlock(Function *f, Block *parent);
	Function *AddFunction(const string &name, Class *type);

	// command
	Command *AddCommand(int kind, long long link_no, Class *type);
	Command *AddCommand(int kind, long long link_no, Class *type, Script *s);
	Command *add_command_statement(int index);
	Command *add_command_classfunc(ClassFunction *f, Command *inst, bool force_non_virtual = false);
	Command *add_command_func(Script *script, int no, Class *return_type);
	Command *add_command_const(int nc);
	Command *add_command_operator(Command *p1, Command *p2, int op);
	Command *add_command_local_var(int no, Class *type);
	Command *add_command_parray(Command *p, Command *index, Class *type);
	Command *add_command_block(Block *b);
	Command *cp_command(Command *c);
	Command *ref_command(Command *sub, Class *override_type = NULL);
	Command *deref_command(Command *sub, Class *override_type = NULL);
	Command *shift_command(Command *sub, bool deref, int shift, Class *type);

	// pre processor
	Command *PreProcessCommand(Command *c);
	void PreProcessor();
	Command *PreProcessCommandAddresses(Command *c);
	void PreProcessorAddresses();
	void Simplify();

	// debug displaying
	void ShowCommand(Command *c, Function *f);
	void ShowFunction(Function *f);
	void ShowBlock(Block *b);
	void Show();

// data

	ExpressionBuffer Exp;

	// compiler options
	bool flag_immortal;
	bool flag_string_const_as_cstring;

	Array<Class*> classes;
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
string LinkNr2Str(SyntaxTree *s, int kind, long long nr);




};

#endif
