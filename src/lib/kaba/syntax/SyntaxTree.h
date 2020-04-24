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

	string sig() const;
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

	void parse_buffer(const string &buffer, bool just_analyse);
	void add_include_data(Script *s);

	void do_error(const string &msg, int override_exp_no = -1, int override_line = -1);
	void do_error_implicit(Function *f, const string &msg);
	void expect_no_new_line();
	void expect_new_line();
	void expect_indent();
	
	// syntax parsing
	void parse();
	void parse_top_level();
	void parse_all_class_names(Class *ns, int indent0);
	void parse_all_function_bodies(const Class *name_space);
	Flags parse_flags(Flags initial);
	void parse_import();
	void parse_enum(Class *_namespace);
	void parse_class(Class *_namespace);
	Function *parse_function_header(Class *name_space, Flags flags);
	void skip_parsing_function_body();
	void parse_function_body(Function *f);
	bool parse_function_command(Function *f, int indent0);
	const Class *parse_type(const Class *ns);
	void parse_variable_def(bool single, Block *block, Flags flags);
	void parse_global_const(const string &name, const Class *type);
	PrimitiveOperator *which_primitive_operator(const string &name, int param_flags = 3);
	Statement *which_statement(const string &name);
	const Class *which_owned_class(const string &name);

	// pre compiler
	void pre_compiler(bool just_analyse);
	void handle_macro(int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);
	void auto_implement_add_virtual_table(Node *self, Function *f, const Class *t);
	void auto_implement_add_child_constructors(Node *self, Function *f, const Class *t);
	void auto_implement_constructor(Function *f, const Class *t, bool allow_parent_constructor);
	void auto_implement_destructor(Function *f, const Class *t);
	void auto_implement_assign(Function *f, const Class *t);
	void auto_implement_array_clear(Function *f, const Class *t);
	void auto_implement_array_resize(Function *f, const Class *t);
	void auto_implement_array_add(Function *f, const Class *t);
	void auto_implement_array_remove(Function *f, const Class *t);
	void auto_implement_functions(const Class *t);
	void add_missing_function_headers_for_class(Class *t);

	// syntax analysis
	const Class *get_constant_type(const string &str);
	void get_constant_value(const string &str, Value &value);
	const Class *find_root_type_by_name(const string &name, const Class *_namespace, bool allow_recursion);
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
	void link_most_important_operator(Array<Node*> &operand, Array<Node*> &_operator, Array<int> &op_exp);
	Node *link_operator(PrimitiveOperator *primop, Node *param1, Node *param2);
	Node *link_operator_id(OperatorID op_no, Node *param1, Node *param2);
	Node *parse_operand_extension(Array<Node*> operands, Block *block);
	Array<Node*> parse_operand_extension_element(Node *operand);
	Node *parse_operand_extension_array(Node *operand, Block *block);
	Node *parse_operand_extension_call(Array<Node*> operands, Block *block);
	Array<Node*> make_class_node_callable(const Class *t, Block *block, Array<Node*> &params);
	void make_func_node_callable(Node *l);
	const Class *parse_type_extension_array(const Class *c);
	const Class *parse_type_extension_dict(const Class *c);
	const Class *parse_type_extension_pointer(const Class *c);
	Node *parse_single_func_param(Block *block);
	void parse_complete_command(Block *block);
	void parse_local_definition(Block *block, const Class *type);
	Node *parse_block(Block *parent, Block *block = nullptr);
	Node *parse_operand(Block *block, bool prefer_class = false);
	Node *parse_operand_greedy(Block *block, bool allow_tuples = false, Node *first_operand = nullptr);
	Node *parse_operand_super_greedy(Block *block);
	Node *parse_set_builder(Block *block);
	Node *link_unary_operator(PrimitiveOperator *op, Node *operand, Block *block);
	Node *parse_primitive_operator(Block *block);
	Array<Node*> parse_call_parameters(Block *block);
	//void FindFunctionSingleParameter(int p, Array<Type*> &wanted_type, Block *block, Node *cmd);
	Array<const Class*> get_wanted_param_types(Node *link);
	Node *check_param_link(Node *link, const Class *type, const string &f_name = "", int param_no = -1);
	Node *try_parse_format_string(Block *block, Value &v);
	Node *apply_format(Node *n, const string &fmt);
	Node *parse_statement(Block *block);
	Node *parse_for_header(Block *block);
	void post_process_for(Node *n);
	Node *parse_statement_for(Block *block);
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
	Node *parse_statement_repr(Block *block);
	Node *parse_statement_len(Block *block);
	Node *parse_statement_let(Block *block);
	Node *parse_statement_map(Block *block);
	Node *parse_statement_lambda(Block *block);
	Node *parse_statement_sorted(Block *block);
	Node *parse_statement_dyn(Block *block);
	Node *parse_statement_call(Block *block);

	Node *apply_type_cast(int tc, Node *param, const Class *wanted);
	Node *apply_params_with_cast(Node *operand, const Array<Node*> &params, const Array<int> &casts, const Array<const Class*> &wanted);
	bool direct_param_match(Node *operand, Array<Node*> &params);
	bool param_match_with_cast(Node *operand, Array<Node*> &params, Array<int> &casts, Array<const Class*> &wanted);
	Node *apply_params_direct(Node *operand, Array<Node*> &params);
	Node *force_concrete_type(Node *node);
	void force_concrete_types(Array<Node*> &nodes);

	Node *link_special_operator_is(Node *param1, Node *param2);
	Node *link_special_operator_in(Node *param1, Node *param2);
	Node *make_dynamical(Node *node);
	Array<const Class*> type_list_from_nodes(const Array<Node*> &nn);

	Node *build_abstract_list(const Array<Node*> &el);
	Node *parse_list(Block *block);
	Node *build_abstract_dict(const Array<Node*> &el);
	Node *parse_dict(Block *block);

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
	Node *add_converter_str(Node *sub, bool repr);

	// pre processor
	Node *conv_eval_const_func(Node *c);
	void eval_const_expressions();
	Node *pre_process_node_addresses(Node *c);
	void pre_processor_addresses();
	void simplify_shift_deref();
	void simplify_ref_deref();

	void show(const string &stage);

// data

	ExpressionBuffer Exp;

	// compiler options
	bool flag_immortal;
	bool flag_string_const_as_cstring;
	bool flag_function_pointer_as_code;

	Class *base_class;
	Array<const Class*> owned_classes;
	Array<Script*> includes;
	Array<Define> defines;
	Asm::MetaInfo *asm_meta_info;
	Array<AsmBlock> asm_blocks;
	Array<Operator*> operators;
	Array<Function*> functions;

	Function *root_of_all_evil;

	Script *script;
	Function *cur_func;
	int for_index_count;

	int parser_loop_depth;
};





};

#endif
