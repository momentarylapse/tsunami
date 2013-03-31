#if !defined(PRESCRIPT_H__INCLUDED_)
#define PRESCRIPT_H__INCLUDED_

namespace Asm{
	struct MetaInfo;
};

namespace Script{

// character buffer and expressions (syntax analysis)

struct ps_exp_t
{
	char* name; // points into Exp.buffer
	int pos;
};

struct ps_line_t
{
	int physical_line, length, indent;
	Array<ps_exp_t> exp;
};

struct ps_exp_buffer_t
{
	char *buffer; // holds ALL expressions of the file (0 separated)
	char *buf_cur; // pointer to the latest one
	Array<ps_line_t> line;
	ps_line_t temp_line;
	ps_line_t *cur_line;
	int cur_exp;
	int comment_level;
	string _cur_;
};


#define cur_name		Exp._cur_
#define get_name(n)		string(Exp.cur_line->exp[n].name)
#define next_exp()		{Exp.cur_exp ++; Exp._cur_ = Exp.cur_line->exp[Exp.cur_exp].name;}//;ExpectNoNewline()
#define rewind_exp()	{Exp.cur_exp --; Exp._cur_ = Exp.cur_line->exp[Exp.cur_exp].name;}
#define end_of_line()	(Exp.cur_exp >= Exp.cur_line->exp.num - 1) // the last entry is "-eol-"
#define past_end_of_line()	(Exp.cur_exp >= Exp.cur_line->exp.num)
#define next_line()		{Exp.cur_line ++; Exp.cur_exp = 0; test_indent(Exp.cur_line->indent);  Exp._cur_ = Exp.cur_line->exp[Exp.cur_exp].name;}
#define end_of_file()	((long)Exp.cur_line >= (long)&Exp.line[Exp.line.num - 1]) // last line = "-eol-"


// macros
struct Define
{
	string Source;
	Array<string> Dest;
};

// single enum entries
/*struct sEnum
{
	string Name;
	int Value;
};*/

// for any type of constant used in the script
struct Constant
{
	PreScript *owner;
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
	KindVarExternal,		// = variable from surrounding program
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

// type of expression (syntax)
enum
{
	ExpKindNumber,
	ExpKindLetter,
	ExpKindSpacing,
	ExpKindSign
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
	char *block;
	int line;
};

class Script;


// data structures (uncompiled)
class PreScript
{
public:
	PreScript(Script *_script);
	~PreScript();

	void LoadAndParseFile(const string &filename, bool just_analyse);
	bool LoadToBuffer(const string &filename, bool just_analyse);
	void AddIncludeData(Script *s);

	bool Error, IncludeLinkerError;
	string ErrorMsg, ErrorMsgExt[2];
	int ErrorLine,ErrorColumn;
	void DoError(const string &msg, int overwrite_line = -1);
	bool ExpectNoNewline();
	bool ExpectNewline();
	bool ExpectIndent();

	// lexical analysis
	int GetKind(char c);
	void Analyse(const char *buffer, bool just_analyse);
	bool AnalyseExpression(const char *buffer, int &pos, ps_line_t *l, int &line_no, bool just_analyse);
	bool AnalyseLine(const char *buffer, ps_line_t *l, int &line_no, bool just_analyse);
	void AnalyseLogicalLine(const char *buffer, ps_line_t *l, int &line_no, bool just_analyse);
	
	// syntax analysis
	void Parser();
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
	int WhichExternalVariable(const string &name);
	int WhichType(const string &name);
	void SetExternalVariable(int gv, Command *c);
	void AddType();

	// pre compiler
	void PreCompiler(bool just_analyse);
	void HandleMacro(ps_line_t *l, int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);
	void CreateImplicitFunctions(Type *t, bool relocate_last_function);
	void CreateAllImplicitFunctions(bool relocate_last_function);

	// syntax analysis
	Type *GetConstantType();
	void *GetConstantValue();
	Type *GetType(const string &name, bool force);
	void AddType(Type **type);
	Type *CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Type *sub);
	void TestArrayDefinition(Type **type, bool is_pointer);
	bool GetExistence(const string &name, Function *f);
	void LinkMostImportantOperator(int &NumOperators, Command **Operand, Command **Operator, int *op_exp);
	bool LinkOperator(int op_no, Command *param1, Command *param2, Command **cmd);
	void GetOperandExtension(Command *Operand, Function *f);
	Command *GetCommand(Function *f);
	void GetCompleteCommand(Block *block, Function *f);
	Command *GetOperand(Function *f);
	Command *GetOperator(Function *f);
	void FindFunctionParameters(int &np, Type **WantedType, Function *f, Command *cmd);
	void FindFunctionSingleParameter(int p, Type **WantedType, Function *f, Command *cmd);
	void GetFunctionCall(const string &f_name, Command *Operand, Function *f);
	bool GetSpecialFunctionCall(const string &f_name, Command *Operand, Function *f);
	void CheckParamLink(Command *link, Type *type, const string &f_name = "", int param_no = -1);
	void GetSpecialCommand(Block *block, Function *f);

	// neccessary conversions
	void ConvertCallByReference();
	void BreakDownComplicatedCommands();
	void MapLocalVariablesToStack();

	// data creation
	int AddVar(const string &name, Type *type, Function *f);
	int AddConstant(Type *type);
	Block *AddBlock();
	Function *AddFunction(const string &name, Type *type);
	Command *AddCommand();

	// pre processor
	void PreProcessCommand(Script *s, Command *c);
	void PreProcessor(Script *s);
	void PreProcessCommandAddresses(Script *s, Command *c);
	void PreProcessorAddresses(Script *s);
	void Simplify();

	// debug displaying
	void ShowCommand(Command *c);
	void ShowFunction(int f);
	void ShowBlock(Block *b);
	void Show();

// data

	string Filename;
	string Buffer;
	int BufferLength, BufferPos;
	ps_exp_buffer_t Exp;
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
	//Array<sEnum> Enum;
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

#define _do_error_(str,n,r)	{DoError(str);msg_db_l(n);return r;}
#define _do_error_int_(str,n,r)	{DoErrorInternal(str);msg_db_l(n);return r;}
#define _return_(n,r)		{msg_db_l(n);return r;}

string Kind2Str(int kind);
string Operator2Str(PreScript *s,int cmd);
void clear_exp_buffer(ps_exp_buffer_t *e);
void CreateAsmMetaInfo(PreScript* ps);
extern Script *cur_script;



inline bool isNumber(char c)
{
	if ((c>=48)&&(c<=57))
		return true;
	return false;
}

inline bool isLetter(char c)
{
	if ((c>='a')&&(c<='z'))
		return true;
	if ((c>='A')&&(c<='Z'))
		return true;
	if ((c=='_'))
		return true;
	// Umlaute
#ifdef OS_WINDOWS
	// Windows-Zeichensatz
	if ((c==-28)||(c==-10)||(c==-4)||(c==-33)||(c==-60)||(c==-42)||(c==-36))
		return true;
#endif
#ifdef OS_LINUX
	// Linux-Zeichensatz??? testen!!!!
#endif
	return false;
}

inline bool isSpacing(char c)
{
	if ((c==' ')||(c=='\t')||(c=='\n'))
		return true;
	return false;
}

inline bool isSign(char c)
{
	if ((c=='.')||(c==':')||(c==',')||(c==';')||(c=='+')||(c=='-')||(c=='*')||(c=='%')||(c=='/')||(c=='=')||(c=='<')||(c=='>')||(c=='\''))
		return true;
	if ((c=='(')||(c==')')||(c=='{')||(c=='}')||(c=='&')||(c=='|')||(c=='!')||(c=='[')||(c==']')||(c=='\"')||(c=='\\')||(c=='#')||(c=='?')||(c=='$'))
		return true;
	return false;
}

};

#endif
