#if !defined(SYNTAX_TREE_H__INCLUDED_)
#define SYNTAX_TREE_H__INCLUDED_


#include "lexical.h"
#include <functional>
#include "Class.h"
#include "Constant.h"
#include "Function.h"
#include "Node.h"


class complex;

namespace Asm{
	struct MetaInfo;
};

namespace Kaba{

class Script;
class SyntaxTree;
class Operator;
class Function;
class Variable;
class Node;
class Constant;
class Block;

#define MAX_STRING_CONST_LENGTH	2048

// macros
struct Define
{
	string source;
	Array<string> dest;
};


struct Operator
{
	int primitive_id;
	const Class *return_type, *param_type_1, *param_type_2;

	SyntaxTree *owner;
	Function *f;

	string sig() const;
};


struct AsmBlock
{
	string block;
	int line;
};



// data structures (uncompiled)
class SyntaxTree
{
public:
	SyntaxTree(Script *_script);
	~SyntaxTree();

	void parse_buffer(const string &buffer, bool just_analyse);
	void AddIncludeData(Script *s);

	void do_error(const string &msg, int override_exp_no = -1, int override_line = -1);
	void expect_no_new_line();
	void expect_new_line();
	void expect_indent();
	
	// syntax parsing
	void parse();
	void parse_top_level();
	void parse_all_class_names();
	void parse_all_function_bodies();
	void parse_import();
	void parse_enum();
	void parse_class();
	Function *parse_function_header(const Class *class_type, bool as_extern);
	void SkipParsingFunctionBody();
	void parse_function_body(Function *f);
	void parse_class_function_header(Class *t, bool as_extern, bool as_virtual, bool override);
	bool ParseFunctionCommand(Function *f, ExpressionBuffer::Line *this_line);
	const Class *parse_type();
	void ParseVariableDef(bool single, Block *block);
	void parse_global_const(const string &name, const Class *type);
	int which_primitive_operator(const string &name);
	int which_statement(const string &name);
	const Class *which_owned_class(const string &name);

	// pre compiler
	void PreCompiler(bool just_analyse);
	void HandleMacro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);
	void AutoImplementAddVirtualTable(Node *self, Function *f, const Class *t);
	void AutoImplementAddChildConstructors(Node *self, Function *f, const Class *t);
	void AutoImplementConstructor(Function *f, const Class *t, bool allow_parent_constructor);
	void AutoImplementDestructor(Function *f, const Class *t);
	void AutoImplementAssign(Function *f, const Class *t);
	void AutoImplementArrayClear(Function *f, const Class *t);
	void AutoImplementArrayResize(Function *f, const Class *t);
	void AutoImplementArrayAdd(Function *f, const Class *t);
	void AutoImplementArrayRemove(Function *f, const Class *t);
	void AutoImplementFunctions(const Class *t);
	void AddMissingFunctionHeadersForClass(Class *t);

	// syntax analysis
	const Class *get_constant_type(const string &str);
	void get_constant_value(const string &str, Value &value);
	const Class *find_type_by_name(const string &name);
	const Class *AddClass(const Class *type);
	Class *create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *sub);
	const Class *make_class(const string &name, Class::Type type, int size, int array_size, const Class *sub);
	const Class *make_class_super_array(const Class *element_type);
	const Class *make_class_array(const Class *element_type, int num_elements);
	const Class *make_class_dict(const Class *element_type);
	const Class *make_class_func(Function *f);
	Array<Node*> get_existence(const string &name, Block *block);
	Array<Node*> get_existence_shared(const string &name);
	void link_most_important_operator(Array<Node*> &operand, Array<Node*> &_operator, Array<int> &op_exp);
	Node *link_operator(int op_no, Node *param1, Node *param2);
	Node *parse_operand_extension(Array<Node*> operands, Block *block);
	Array<Node*> parse_operand_extension_element(Node *operand, Block *block);
	Node *parse_operand_extension_array(Node *operand, Block *block);
	Node *parse_operand_extension_call(Array<Node*> operands, Block *block);
	Node *parse_command(Block *block);
	Node *parse_single_func_param(Block *block);
	void parse_complete_command(Block *block);
	void parse_local_definition(Block *block);
	Node *parse_block(Block *parent, Block *block = nullptr);
	Node *parse_operand(Block *block);
	Node *link_unary_operator(int op_no, Node *operand, Block *block);
	Node *parse_primitive_operator(Block *block);
	Array<Node*> parse_call_parameters(Block *block);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, Node *cmd);
	Array<const Class*> get_wanted_param_types(Node *link);
	Node *check_param_link(Node *link, const Class *type, const string &f_name = "", int param_no = -1);
	Node *parse_statement(Block *block);
	Node *parse_statement_for(Block *block);
	Node *parse_statement_for_array(Block *block);
	Node *parse_statement_while(Block *block);
	Node *parse_statement_break(Block *block);
	Node *parse_statement_continue(Block *block);
	Node *parse_statement_return(Block *block);
	Node *parse_statement_raise(Block *block);
	Node *parse_statement_try(Block *block);
	Node *parse_statement_if(Block *block);
	Node *parse_statement_pass(Block *block);
	Node *parse_statement_new(Block *block);
	Node *parse_statement_delete(Block *block);
	Node *parse_statement_sizeof(Block *block);
	Node *parse_statement_type(Block *block);
	Node *parse_statement_str(Block *block);
	Node *parse_statement_len(Block *block);

	void CreateAsmMetaInfo();

	// neccessary conversions
	void ConvertCallByReference();
	void BreakDownComplicatedCommands();
	Node *BreakDownComplicatedCommand(Node *c);
	void MakeFunctionsInline();
	void MapLocalVariablesToStack();

	void transform(std::function<Node*(Node*)> F);
	static void transform_block(Block *block, std::function<Node*(Node*)> F);
	static Node* transform_node(Node *n, std::function<Node*(Node*)> F);

	// data creation
	Constant *add_constant(const Class *type);
	Function *add_function(const string &name, const Class *type);

	// nodes
	Node *add_node_statement(int index);
	Node *add_node_member_call(ClassFunction *f, Node *inst, bool force_non_virtual = false);
	Node *add_node_func_name(Function *f);
	Node *add_node_call(Function *f);
	Node *add_node_const(Constant *c);
	Node *add_node_operator(Node *p1, Node *p2, Operator *op);
	Node *add_node_operator_by_inline(Node *p1, Node *p2, int inline_index);
	Node *add_node_local_var(Variable *var);
	Node *add_node_parray(Node *p, Node *index, const Class *type);
	//Node *add_node_block(Block *b);
	Node *cp_node(Node *c);
	Node *ref_node(Node *sub, const Class *override_type = nullptr);
	Node *deref_node(Node *sub, const Class *override_type = nullptr);
	Node *shift_node(Node *sub, bool deref, int shift, const Class *type);

	// pre processor
	Node *PreProcessNode(Node *c);
	void PreProcessor();
	Node *PreProcessNodeAddresses(Node *c);
	void PreProcessorAddresses();
	void SimplifyRefDeref();
	void SimplifyShiftDeref();

	void Show(const string &stage);

// data

	ExpressionBuffer Exp;

	// compiler options
	bool flag_immortal;
	bool flag_string_const_as_cstring;

	Array<const Class*> classes;
	Array<Script*> includes;
	Array<Define> defines;
	Asm::MetaInfo *asm_meta_info;
	Array<AsmBlock> asm_blocks;
	Array<Constant*> constants;
	Array<Operator*> operators;
	Array<Function*> functions;

	Function root_of_all_evil;

	Script *script;
	Function *cur_func;
	int for_index_count;

	int parser_loop_depth;
};





};

#endif
