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

struct Value
{
	string value;
	Class *type;

	Value();
	~Value();

	void init(Class *type);
	void clear();
	void set(const Value &v);

	void* p() const;
	int& as_int() const;
	long long& as_int64() const;
	float& as_float() const;
	double& as_float64() const;
	string& as_string() const;
	DynamicArray& as_array() const;

	int mapping_size() const;
	void map_into(char *mem, char *addr) const;
	string str() const;
};

// for any type of constant used in the script
struct Constant : Value
{
	Constant(Class *type);
	string name;
	string str() const;
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

struct Node;

// {...}-block
struct Block
{
	int index;
	Array<Node*> nodes;
	Array<int> vars;
	Function *function;
	Block *parent;
	void *_start, *_end; // opcode range
	int level;
	void add(Node *c);
	void set(int index, Node *c);

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
	bool throws_exceptions; // for external
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
struct Node
{
	int kind;
	long long link_no;
	Script *script;
	int ref_count;
	// parameters
	Array<Node*> params;
	// linking of class function instances
	Node *instance;
	// return value
	Class *type;
	Node();
	Node(int kind, long long link_no, Script *script, Class *type);
	Block *as_block() const;
	Function *as_func() const;
	Constant *as_const() const;
	void set_num_params(int n);
	void set_param(int index, Node *p);
	void set_instance(Node *p);
};


struct Operator
{
	int primitive_id;
	Class *return_type, *param_type_1, *param_type_2;

	SyntaxTree *owner;
	int func_index;
	int class_func_index;
	int inline_index;

	void *func; // temporary...!

	string str() const;
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
	void AutoImplementAddVirtualTable(Node *self, Function *f, Class *t);
	void AutoImplementAddChildConstructors(Node *self, Function *f, Class *t);
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
	void GetConstantValue(const string &str, Value &value);
	Class *FindType(const string &name);
	Class *AddClass(Class *type);
	Class *CreateNewClass(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, Class *sub);
	Class *CreateArrayClass(Class *element_type, int num_elements, const string &name_pre = "", const string &suffix = "");
	Array<Node> GetExistence(const string &name, Block *block);
	Array<Node> GetExistenceShared(const string &name);
	void LinkMostImportantOperator(Array<Node*> &operand, Array<Node*> &_operator, Array<int> &op_exp);
	Node *LinkOperator(int op_no, Node *param1, Node *param2);
	Node *GetOperandExtension(Node *operand, Block *block);
	Node *GetOperandExtensionElement(Node *operand, Block *block);
	Node *GetOperandExtensionArray(Node *operand, Block *block);
	Node *GetCommand(Block *block);
	void ParseCompleteCommand(Block *block);
	Node *GetOperand(Block *block);
	Node *GetPrimitiveOperator(Block *block);
	Array<Node*> FindFunctionParameters(Block *block);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, Node *cmd);
	Array<Class*> GetFunctionWantedParams(Node &link);
	Node *GetFunctionCall(const string &f_name, Array<Node> &links, Block *block);
	Node *DoClassFunction(Node *ob, Array<ClassFunction> &cfs, Block *block);
	Node *GetSpecialFunctionCall(const string &f_name, Node &link, Block *block);
	Node *CheckParamLink(Node *link, Class *type, const string &f_name = "", int param_no = -1);
	void ParseStatement(Block *block);
	void ParseStatementFor(Block *block);
	void ParseStatementForall(Block *block);
	void ParseStatementWhile(Block *block);
	void ParseStatementBreak(Block *block);
	void ParseStatementContinue(Block *block);
	void ParseStatementReturn(Block *block);
	void ParseStatementRaise(Block *block);
	void ParseStatementTry(Block *block);
	void ParseStatementIf(Block *block);
	void ParseStatementPass(Block *block);

	void CreateAsmMetaInfo();

	// neccessary conversions
	void ConvertCallByReference();
	void ConvertInline();
	void BreakDownComplicatedCommands();
	Node *BreakDownComplicatedCommand(Node *c);
	void MapLocalVariablesToStack();

	// data creation
	int AddConstant(Class *type);
	Block *AddBlock(Function *f, Block *parent);
	Function *AddFunction(const string &name, Class *type);

	// nodes
	Node *AddNode(int kind, long long link_no, Class *type);
	Node *AddNode(int kind, long long link_no, Class *type, Script *s);
	Node *add_node_statement(int index);
	Node *add_node_classfunc(ClassFunction *f, Node *inst, bool force_non_virtual = false);
	Node *add_node_func(Script *script, int no, Class *return_type);
	Node *add_node_const(int nc);
	Node *add_node_operator_by_index(Node *p1, Node *p2, int op);
	Node *add_node_operator_by_inline(Node *p1, Node *p2, int inline_index);
	Node *add_node_local_var(int no, Class *type);
	Node *add_node_parray(Node *p, Node *index, Class *type);
	Node *add_node_block(Block *b);
	Node *cp_node(Node *c);
	Node *ref_node(Node *sub, Class *override_type = NULL);
	Node *deref_node(Node *sub, Class *override_type = NULL);
	Node *shift_node(Node *sub, bool deref, int shift, Class *type);

	// pre processor
	Node *PreProcessNode(Node *c);
	void PreProcessor();
	Node *PreProcessNodeAddresses(Node *c);
	void PreProcessorAddresses();
	void SimplifyRefDeref();
	void SimplifyShiftDeref();

	// debug displaying
	void ShowNode(Node *c, Function *f);
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
	Array<Constant*> constants;
	Array<Operator> operators;
	Array<Block*> blocks;
	Array<Function*> functions;
	Array<Node*> nodes;

	Function root_of_all_evil;

	Script *script;
	Function *cur_func;
	int for_index_count;

	int parser_loop_depth;
};

string Kind2Str(int kind);
string LinkNr2Str(SyntaxTree *s, int kind, long long nr);




};

#endif
