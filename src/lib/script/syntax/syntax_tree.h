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
	KindUnknown,
	// data
	KindVarLocal,
	KindVarGlobal,
	KindVarFunction,
	KindConstant,
	// execution
	KindFunction,			// = real function call
	KindVirtualFunction,	// = virtual function call
	KindCompilerFunction,	// = special internal functions
	KindBlock,				// = block of commands {...}
	KindOperator,
	KindPrimitiveOperator,	// provisorical...
	// data altering
	KindAddressShift,		// = . "struct"
	KindArray,				// = []
	KindPointerAsArray,		// = []
	KindReference,			// = &
	KindDereference,		// = *
	KindDerefAddressShift,	// = ->
	KindRefToLocal,
	KindRefToGlobal,
	KindRefToConst,
	KindAddress,			// &global (for pre processing address shifts)
	KindMemory,				// global (but LinkNr = address)
	KindLocalAddress,		// &local (for pre processing address shifts)
	KindLocalMemory,		// local (but LinkNr = address)
	// special
	KindType,
	KindArrayBuilder,
	// compilation
	KindVarTemp,
	KindDerefVarTemp,
	KindDerefVarLocal,
	KindRegister,
	KindDerefRegister,
	KindMarker,
};

struct Command;

// {...}-block
struct Block
{
	int index;
	Array<Command*> command;
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
	// for compilation...
	int _var_size, _param_size;
	int _logical_line_no;
	Function(const string &name, Type *return_type);
	int get_var(const string &name);
	int AddVar(const string &name, Type *type);
	void Update(Type *class_type);
};

// single operand/command
struct Command
{
	int kind, link_no;
	Script *script;
	// parameters
	int num_params;
	Command *param[SCRIPT_MAX_PARAMS];
	// linking of class function instances
	Command *instance;
	// return value
	Type *type;
	Command(int kind, int link_no, Script *script, Type *type);
	Block *block() const;
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

	void LoadAndParseFile(const string &filename, bool just_analyse);
	void LoadToBuffer(const string &filename, bool just_analyse);
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
	Type *ParseVariableDefSingle(Type *type, Function *f, bool as_param = false);
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
	Type *GetType(const string &name, bool force);
	void AddType(Type **type);
	Type *CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub);
	Type *CreateArrayType(Type *element_type, int num_elements, const string &name_pre = "", const string &suffix = "");
	void TestArrayDefinition(Type **type, bool is_pointer);
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
	void GetFunctionCall(const string &f_name, Command *Operand, Function *f);
	Command *DoClassFunction(Command *ob, ClassFunction &cf, Function *f);
	bool GetSpecialFunctionCall(const string &f_name, Command *Operand, Function *f);
	void CheckParamLink(Command *link, Type *type, const string &f_name = "", int param_no = -1);
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
	void BreakDownComplicatedCommand(Command *c);
	void MapLocalVariablesToStack();

	// data creation
	int AddConstant(Type *type);
	Block *AddBlock();
	Function *AddFunction(const string &name, Type *type);

	// command
	Command *AddCommand(int kind, int link_no, Type *type);
	Command *add_command_compilerfunc(int cf);
	Command *add_command_classfunc(Type *class_type, ClassFunction *f, Command *inst);
	Command *add_command_const(int nc);
	Command *add_command_operator(Command *p1, Command *p2, int op);
	Command *add_command_local_var(int no, Type *type);
	Command *add_command_parray(Command *p, Command *index, Type *type);
	Command *cp_command(Command *c);
	Command *cp_command_deep(Command *c);
	Command *ref_command(Command *sub, Type *overwrite_type = NULL);
	Command *deref_command(Command *sub);
	Command *shift_command(Command *sub, bool deref, int shift, Type *type);

	// pre processor
	void PreProcessCommand(Command *c);
	void PreProcessor();
	void PreProcessCommandAddresses(Command *c);
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
	bool FlagShowPrae;
	bool FlagShow;
	bool FlagDisassemble;
	bool FlagNoExecution;
	bool FlagImmortal;
	bool FlagCompileOS;
	bool FlagStringConstAsCString;
	bool FlagNoFunctionFrame;
	bool FlagAddEntryPoint;
	bool FlagOverwriteVariablesOffset;
	int VariablesOffset;

	Array<Type*> Types;
	Array<Script*> Includes;
	Array<Define> Defines;
	Asm::MetaInfo *AsmMetaInfo;
	Array<AsmBlock> AsmBlocks;
	Array<Constant> Constants;
	Array<Block*> Blocks;
	Array<Function*> Functions;
	Array<Command*> Commands;

	Function RootOfAllEvil;

	Script *script;
	Function *cur_func;
	int ForIndexCount;
};

string Kind2Str(int kind);
string LinkNr2Str(SyntaxTree *s,int kind,int nr);




};

#endif
