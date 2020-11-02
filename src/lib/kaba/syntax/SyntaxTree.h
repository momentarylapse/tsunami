#if !defined(SYNTAX_TREE_H__INCLUDED_)
#define SYNTAX_TREE_H__INCLUDED_


#include "lexical.h"
#include <functional>
#include "Class.h"
#include "Constant.h"
#include "Function.h"
#include "Node.h"
#include "../lib/common.h"


class complex;

namespace Asm {
	struct MetaInfo;
};

namespace kaba {

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

	void default_import();
	void add_include_data(shared<Script> s, bool indirect);

	void do_error(const string &msg, int override_exp_no = -1, int override_line = -1);
	
	// syntax parsing
	const Class *which_owned_class(const string &name);

	// syntax analysis
	const Class *add_class(const Class *type);
	Class *create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, const Class *ns);
	const Class *make_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, const Class *ns);
	const Class *make_class_super_array(const Class *element_type);
	const Class *make_class_array(const Class *element_type, int num_elements);
	const Class *make_class_dict(const Class *element_type);
	const Class *make_class_func(Function *f);
	const Class *make_class_func(const Array<const Class*> &param, const Class *ret);
	shared_array<Node> get_existence(const string &name, Block *block, const Class *ns, bool prefer_class);
	shared_array<Node> get_existence_global(const string &name, const Class *ns, bool prefer_class);
	shared_array<Node> get_existence_block(const string &name, Block *block);

	Function *required_func_global(const string &name);

	void create_asm_meta_info();

	// neccessary conversions
	void digest();
	void convert_call_by_reference();
	void map_local_variables_to_stack();
	shared<Node> conv_class_and_func_to_const(shared<Node> n);
	shared<Node> conv_break_down_high_level(shared<Node> n, Block *b);
	shared<Node> conv_break_down_low_level(shared<Node> c);
	shared<Node> conv_cbr(shared<Node> c, Variable *var);
	shared<Node> conv_calls(shared<Node> c);
	shared<Node> conv_easyfy_ref_deref(shared<Node> c, int l);
	shared<Node> conv_easyfy_shift_deref(shared<Node> c, int l);
	shared<Node> conv_return_by_memory(shared<Node> n, Function *f);
	shared<Node> conv_func_inline(shared<Node> n);

	void transform(std::function<shared<Node>(shared<Node>)> F);
	static void transform_block(Block *block, std::function<shared<Node>(shared<Node>)> F);
	static shared<Node> transform_node(shared<Node> n, std::function<shared<Node>(shared<Node>)> F);

	void transformb(std::function<shared<Node>(shared<Node>, Block*)> F);
	static void transformb_block(Block *block, std::function<shared<Node>(shared<Node>, Block*)> F);
	static shared<Node> transformb_node(shared<Node> n, Block *b, std::function<shared<Node>(shared<Node>, Block*)> F);

	// data creation
	Constant *add_constant(const Class *type, Class *name_space = nullptr);
	Constant *add_constant_int(int value);
	Constant *add_constant_pointer(const Class *type, const void *value);
	Function *add_function(const string &name, const Class *type, const Class *name_space, Flags flags);

	// nodes
	shared<Node> add_node_statement(StatementID id);
	shared<Node> add_node_member_call(Function *f, shared<Node> inst, bool force_non_virtual = false);
	shared<Node> add_node_func_name(Function *f);
	shared<Node> add_node_class(const Class *c);
	shared<Node> add_node_call(Function *f);
	shared<Node> add_node_const(Constant *c);
	//shared<Node> add_node_block(Block *b);
	shared<Node> add_node_operator(shared<Node> p1, shared<Node> p2, Operator *op);
	shared<Node> add_node_operator_by_inline(shared<Node> p1, shared<Node> p2, InlineID inline_index);
	shared<Node> add_node_global(Variable *var);
	shared<Node> add_node_local(Variable *var);
	shared<Node> add_node_local(Variable *var, const Class *type);
	shared<Node> make_constructor_static(shared<Node> n, const string &name);
	shared<Node> exlink_add_element(Function *f, ClassElement &e);
	shared<Node> exlink_add_class_func(Function *f, Function *cf);
	shared<Node> add_node_parray(shared<Node> p, shared<Node> index, const Class *type);
	shared<Node> add_node_dyn_array(shared<Node> array, shared<Node> index);
	shared<Node> add_node_array(shared<Node> array, shared<Node> index);
	shared<Node> add_node_constructor(Function *f);
	shared<Node> make_fake_constructor(const Class *t, Block *block, const Class *param_type);
	//shared<Node> add_node_block(Block *b);
	shared<Node> cp_node(shared<Node> c);
	shared<Node> ref_node(shared<Node> sub, const Class *override_type = nullptr);
	shared<Node> deref_node(shared<Node> sub, const Class *override_type = nullptr);
	shared<Node> shift_node(shared<Node> sub, bool deref, int64 shift, const Class *type);

	// pre processor
	shared<Node> conv_eval_const_func(shared<Node> c);
	shared<Node> conv_eval_const_func_nofunc(shared<Node> c);
	void eval_const_expressions(bool allow_func_eval);
	shared<Node> pre_process_node_addresses(shared<Node> c);
	void pre_processor_addresses();
	void simplify_shift_deref();
	void simplify_ref_deref();

	void add_missing_function_headers_for_class(Class *t);
	void add_func_header(Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, Function *cf = nullptr, Flags flags = Flags::NONE);
	const Class *find_root_type_by_name(const string &name, const Class *_namespace, bool allow_recursion);

	void show(const string &stage);

// data

	// compiler options
	bool flag_immortal;
	bool flag_string_const_as_cstring;
	bool flag_function_pointer_as_code;

	Class *base_class;
	shared<Class> _base_class;
	shared<Class> imported_symbols;
	Array<const Class*> owned_classes;
	shared_array<Script> includes;
	Array<Define> defines;
	owned<Asm::MetaInfo> asm_meta_info;
	Array<AsmBlock> asm_blocks;
	Array<Operator*> operators;
	Array<Function*> functions;

	shared<Function> root_of_all_evil;

	Script *script;
	owned<Parser> parser;
};





};

#endif
