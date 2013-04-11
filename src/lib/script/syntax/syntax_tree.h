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

// macros
struct Define
{
	string Source;
	Array<string> Dest;
};

// for any type of constant used in the script
struct Constant
{
	SyntaxTree *owner;
	string name;
	char *data;
	Type *type;
};

enum
{
	KindUnknown,
	// data
	KindVarLocal,
	KindVarGlobal,
	KindVarFunction,
	KindEnum,				// = single enum entry
	KindConstant,
	// execution
	KindFunction,			// = script defined functions
	KindCompilerFunction,	// = compiler functions
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
	// compilation
	KindVarTemp,
	KindDerefVarTemp,
	KindDerefVarLocal,
	KindRegister,
	KindDerefRegister,
	KindMarker,
	KindAsmBlock,
};

struct Command;

// {...}-block
struct Block
{
	int root;
	int index;
	Array<Command*> command; // ID of command in global command array
};

struct LocalVariable
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
	Array<LocalVariable> var;
	Type *literal_param_type[SCRIPT_MAX_PARAMS];
	Type *_class;
	Type *return_type;
	Type *literal_return_type;
	bool is_extern;
	// for compilation...
	int _var_size, _param_size;
};

// single operand/command
struct Command
{
	int kind, link_nr;
	Script *script;
	// parameters
	int num_params;
	Command *param[SCRIPT_MAX_PARAMS];
	// linking of class function instances
	Command *instance;
	// return value
	Type *type;
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
	void ParseImport();
	void ParseEnum();
	void ParseClass();
	void ParseFunction(Type *class_type, bool as_extern);
	void ParseClassFunction(Type *t, bool as_extern);
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
	void CreateImplicitFunctions(Type *t, bool relocate_last_function);
	void CreateAllImplicitFunctions(bool relocate_last_function);

	// syntax analysis
	Type *GetConstantType();
	void *GetConstantValue();
	Type *GetType(const string &name, bool force);
	void AddType(Type **type);
	Type *CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub);
	Type *GetPointerType(Type *sub);
	void TestArrayDefinition(Type **type, bool is_pointer);
	bool GetExistence(const string &name, Function *f);
	void LinkMostImportantOperator(Array<Command*> &Operand, Array<Command*> &Operator, Array<int> &op_exp);
	Command *LinkOperator(int op_no, Command *param1, Command *param2);
	Command *GetOperandExtension(Command *Operand, Function *f);
	Command *GetOperandExtensionElement(Command *Operand, Function *f);
	Command *GetOperandExtensionArray(Command *Operand, Function *f);
	Command *GetCommand(Function *f);
	void GetCompleteCommand(Block *block, Function *f);
	Command *GetOperand(Function *f);
	Command *GetPrimitiveOperator(Function *f);
	void FindFunctionParameters(int &np, Type **WantedType, Function *f, Command *cmd);
	void FindFunctionSingleParameter(int p, Type **WantedType, Function *f, Command *cmd);
	void GetFunctionCall(const string &f_name, Command *Operand, Function *f);
	bool GetSpecialFunctionCall(const string &f_name, Command *Operand, Function *f);
	void CheckParamLink(Command *link, Type *type, const string &f_name = "", int param_no = -1);
	void GetSpecialCommand(Block *block, Function *f);

	void CreateAsmMetaInfo();

	// neccessary conversions
	void ConvertCallByReference();
	void BreakDownComplicatedCommands();
	void MapLocalVariablesToStack();

	// data creation
	int AddVar(const string &name, Type *type, Function *f);
	int AddConstant(Type *type);
	Block *AddBlock();
	Function *AddFunction(const string &name, Type *type);

	// command
	Command *AddCommand();
	Command *add_command_compilerfunc(int cf);
	Command *add_command_classfunc(Type *class_type, ClassFunction &f, Command *inst);
	Command *add_command_const(int nc);
	Command *add_command_operator(Command *p1, Command *p2, int op);
	Command *cp_command(Command *c);
	Command *cp_command_deep(Command *c);
	Command *ref_command(Command *sub);
	Command *deref_command(Command *sub);
	Command *shift_command(Command *sub, bool deref, int shift, Type *type);

	// pre processor
	void PreProcessCommand(Script *s, Command *c);
	void PreProcessor(Script *s);
	void PreProcessCommandAddresses(Script *s, Command *c);
	void PreProcessorAddresses(Script *s);
	void Simplify();

	// debug displaying
	void ShowCommand(Command *c);
	void ShowFunction(Function *f);
	void ShowBlock(Block *b);
	void Show();

// data

	string Buffer;
	int BufferLength, BufferPos;
	ExpressionBuffer Exp;
	Command GetExistenceLink;

	// compiler options
	bool FlagShowPrae;
	bool FlagShow;
	bool FlagDisassemble;
	bool FlagNoExecution;
	bool FlagImmortal;
	bool FlagCompileOS;
	bool FlagCompileInitialRealMode;
	bool FlagOverwriteVariablesOffset;
	int VariablesOffset;

	int NumOwnTypes;
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
};

string Kind2Str(int kind);
string LinkNr2Str(SyntaxTree *s,int kind,int nr);




};

#endif
