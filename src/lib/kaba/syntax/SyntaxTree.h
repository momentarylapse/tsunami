#if !defined(SYNTAX_TREE_H__INCLUDED_)
#define SYNTAX_TREE_H__INCLUDED_


#include "lexical.h"
#include <functional>
#include "Class.h"
#include "Constant.h"
#include "Function.h"
#include "Node.h"


class complex;

namespace Asm {
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
class Parser;
enum class StatementID;
enum class InlineID;
enum class OperatorID;


// macros
struct Define {
	string source;
	Array<string> dest;
};


class Operator {
public:
	PrimitiveOperator *primitive;
	const Class *return_type, *param_type_1, *param_type_2;

	SyntaxTree *owner;
	Function *f;

	string sig(const Class *ns) const;
};


struct AsmBlock {
	string block;
	int line;
};



// data structures (uncompiled)
class SyntaxTree {
public:
	SyntaxTree(Script *_script);
	~SyntaxTree();

	void add_include_data(Script *s, bool indirect);

	void do_error(const string &msg, int override_exp_no = -1, int override_line = -1);
	
	// syntax parsing
	const Class *which_owned_class(const string &name);

	// syntax analysis
	const Class *add_class(const Class *type);
	Class *create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Class *param, const Class *ns);
	const Class *make_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Class *param, const Class *ns);
	const Class *make_class_super_array(const Class *element_type);
	const Class *make_class_array(const Class *element_type, int num_elements);
	const Class *make_class_dict(const Class *element_type);
	const Class *make_class_func(Function *f);
	Array<Node*> get_existence(const string &name, Block *block, const Class *ns, bool prefer_class);
	Array<Node*> get_existence_global(const string &name, const Class *ns, bool prefer_class);
	Node* get_existence_block(const string &name, Block *block);

	void create_asm_meta_info();

	// neccessary conversions
	void digest();
	void convert_call_by_reference();
	void map_local_variables_to_stack();
	Node *conv_class_and_func_to_const(Node *n);
	Node *conv_break_down_high_level(Node *n, Block *b);
	Node *conv_break_down_low_level(Node *c);
	Node *conv_cbr(Node *c, Variable *var);
	Node *conv_calls(Node *c);
	Node *conv_easyfy_ref_deref(Node *c, int l);
	Node *conv_easyfy_shift_deref(Node *c, int l);
	Node *conv_return_by_memory(Node *n, Function *f);
	Node* conv_func_inline(Node *n);

	void transform(std::function<Node*(Node*)> F);
	static void transform_block(Block *block, std::function<Node*(Node*)> F);
	static Node* transform_node(Node *n, std::function<Node*(Node*)> F);

	void transformb(std::function<Node*(Node*, Block*)> F);
	static void transformb_block(Block *block, std::function<Node*(Node*, Block*)> F);
	static Node* transformb_node(Node *n, Block *b, std::function<Node*(Node*, Block*)> F);

	// data creation
	Constant *add_constant(const Class *type, Class *name_space = nullptr);
	Constant *add_constant_int(int value);
	Constant *add_constant_pointer(const Class *type, const void *value);
	Function *add_function(const string &name, const Class *type, const Class *name_space, Flags flags);

	// nodes
	Node *add_node_statement(StatementID id);
	Node *add_node_member_call(Function *f, Node *inst, bool force_non_virtual = false);
	Node *add_node_func_name(Function *f);
	Node *add_node_class(const Class *c);
	Node *add_node_call(Function *f);
	Node *add_node_const(Constant *c);
	//Node *add_node_block(Block *b);
	Node *add_node_operator(Node *p1, Node *p2, Operator *op);
	Node *add_node_operator_by_inline(Node *p1, Node *p2, InlineID inline_index);
	Node *add_node_global(Variable *var);
	Node *add_node_local(Variable *var);
	Node *add_node_local(Variable *var, const Class *type);
	Node *make_constructor_static(Node *n, const string &name);
	Node *exlink_add_element(Function *f, ClassElement &e);
	Node *exlink_add_class_func(Function *f, Function *cf);
	Node *add_node_parray(Node *p, Node *index, const Class *type);
	Node *add_node_dyn_array(Node *array, Node *index);
	Node *add_node_array(Node *array, Node *index);
	Node *add_node_constructor(Function *f);
	Node *make_fake_constructor(const Class *t, Block *block, const Class *param_type);
	//Node *add_node_block(Block *b);
	Node *cp_node(Node *c);
	Node *ref_node(Node *sub, const Class *override_type = nullptr);
	Node *deref_node(Node *sub, const Class *override_type = nullptr);
	Node *shift_node(Node *sub, bool deref, int shift, const Class *type);

	// pre processor
	Node *conv_eval_const_func(Node *c);
	Node *conv_eval_const_func_nofunc(Node *c);
	void eval_const_expressions(bool allow_func_eval);
	Node *pre_process_node_addresses(Node *c);
	void pre_processor_addresses();
	void simplify_shift_deref();
	void simplify_ref_deref();

	void add_missing_function_headers_for_class(Class *t);
	const Class *find_root_type_by_name(const string &name, const Class *_namespace, bool allow_recursion);

	void show(const string &stage);

// data

	// compiler options
	bool flag_immortal;
	bool flag_string_const_as_cstring;
	bool flag_function_pointer_as_code;

	Class *base_class;
	Class *imported_symbols;
	Array<const Class*> owned_classes;
	Array<Script*> includes;
	Array<Define> defines;
	Asm::MetaInfo *asm_meta_info;
	Array<AsmBlock> asm_blocks;
	Array<Operator*> operators;
	Array<Function*> functions;

	Function *root_of_all_evil;

	Script *script;
	Parser *parser;
};





};

#endif
