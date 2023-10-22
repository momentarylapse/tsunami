#if !defined(SYNTAX_TREE_H__INCLUDED_)
#define SYNTAX_TREE_H__INCLUDED_


#include <functional>
#include "../parser/lexical.h"
#include "Class.h"
#include "Constant.h"
#include "Flags.h"
#include "Function.h"
#include "Node.h"
#include "Block.h"
#include "Operator.h"
#include "Variable.h"
#include "Identifier.h"
#include "Inline.h"
#include "Statement.h"


namespace Asm {
	struct MetaInfo;
};

namespace kaba {

class Module;
class SyntaxTree;
class Parser;



struct AsmBlock {
	int uuid;
	string block;
	int line;
};



// data structures (uncompiled)
class SyntaxTree {
public:
	SyntaxTree(Module *module);
	~SyntaxTree();

	void default_import();
	void import_data(shared<Module> s, bool indirect, const string &as_name);

	void do_error(const string &msg, int override_token_id = -1);
	
	// syntax parsing
	const Class *which_owned_class(const string &name);

	// syntax analysis
	Class *create_new_class(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, Class *ns, int token_id);
	Class *create_new_class_no_check(const string &name, Class::Type type, int size, int array_size, const Class *parent, const Array<const Class*> &params, Class *ns, int token_id);
	const Class *get_pointer(const Class *base, int token_id = -1);
	const Class *request_implicit_class_pointer(const Class *parent, int token_id);
	const Class *request_implicit_class_shared(const Class *parent, int token_id);
	const Class *request_implicit_class_shared_not_null(const Class *parent, int token_id);
	const Class *request_implicit_class_owned(const Class *parent, int token_id);
	const Class *request_implicit_class_owned_not_null(const Class *parent, int token_id);
	const Class *request_implicit_class_xfer(const Class *parent, int token_id);
	const Class *request_implicit_class_alias(const Class *parent, int token_id);
	const Class *request_implicit_class_reference(const Class *base, int token_id);
	const Class *request_implicit_class_list(const Class *element_type, int token_id);
	const Class *request_implicit_class_array(const Class *element_type, int num_elements, int token_id);
	const Class *request_implicit_class_dict(const Class *element_type, int token_id);
	const Class *request_implicit_class_optional(const Class *param, int token_id);
	const Class *request_implicit_class_callable_fp(Function *f, int token_id);
	const Class *request_implicit_class_callable_fp(const Array<const Class*> &params, const Class *ret, int token_id);
	const Class *request_implicit_class_callable_bind(const Array<const Class*> &params, const Class *ret, const Array<const Class*> &captures, const Array<bool> &capture_via_ref, int token_id);
	shared_array<Node> get_existence(const string &name, Block *block, const Class *ns, int token_id);
	shared_array<Node> get_existence_global(const string &name, const Class *ns, int token_id);
	shared_array<Node> get_existence_block(const string &name, Block *block, int token_id);

	shared_array<Node> get_element_of(shared<Node> n, const string &name, int token_id);

	Function *required_func_global(const string &name, int token_id = -1);

	void create_asm_meta_info();

	// neccessary conversions
	void digest();
	void convert_call_by_reference();
	void map_local_variables_to_stack();
	shared<Node> conv_fake_constructors(shared<Node> n);
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

	// node
	shared<Node> make_fake_constructor(const Class *t, const Class *param_type, int token_id = -1);

	// pre processor
	shared<Node> conv_eval_const_func(shared<Node> c);
	shared<Node> conv_eval_const_func_nofunc(shared<Node> c);
	void eval_const_expressions(bool allow_func_eval);
	shared<Node> pre_process_node_addresses(shared<Node> c);
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


	ExpressionBuffer expressions;

	Class *base_class;
	shared<Class> _base_class;
	shared<Class> imported_symbols;
	shared<Class> implicit_symbols;
	Array<const Class*> owned_classes;
	shared_array<Module> includes;
	owned<Asm::MetaInfo> asm_meta_info;
	Array<AsmBlock> asm_blocks;
	Array<Function*> functions;

	shared<Function> root_of_all_evil;

	Module *module;
	owned<Parser> parser;
};





};

#endif
