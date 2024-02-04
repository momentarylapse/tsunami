/*
 * Node.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_NODE_H_
#define SRC_LIB_KABA_SYNTAX_NODE_H_


#include "../../base/base.h"
#include "../../base/pointer.h"
//#include "../lib/common.h"
#include "Flags.h"

namespace kaba {

class Class;
class Block;
class SyntaxTree;
class Module;
class Function;
class Variable;
class Constant;
class Operator;
class AbstractOperator;
class Statement;
class SpecialFunction;
enum class StatementID;
enum class SpecialFunctionID;
enum class InlineID;
extern const Class* TypeVoid;


enum class NodeKind {
	NONE = -1,
	UNKNOWN,
	PLACEHOLDER,
	// data
	VAR_LOCAL,
	VAR_GLOBAL,
	FUNCTION,           // = just the name
	CONSTANT,
	// execution
	CALL_FUNCTION,      // = real function call
	CALL_VIRTUAL,       // = virtual function call
	CALL_INLINE,        // = function defined inside the compiler...
	CALL_RAW_POINTER,   // = function call via a raw pointer
	STATEMENT,          // = if/while/break/...
	CALL_SPECIAL_FUNCTION, // = len(), sorted() etc
	SPECIAL_FUNCTION_NAME, // = len, sorted etc
	BLOCK,              // = block of commands {...}
	OPERATOR,
	// data altering
	ADDRESS_SHIFT,      // = . "struct"
	ARRAY,              // = []
	POINTER_AS_ARRAY,   // = []
	DYNAMIC_ARRAY,      // = []
	REFERENCE,          // = &
	DEREFERENCE,        // = *
	DEREF_ADDRESS_SHIFT,// = ->
	DEFINITELY,         // = x!
	CONSTANT_BY_ADDRESS,
	ADDRESS,            // &global (for pre processing address shifts)
	MEMORY,             // global (but LinkNr = address)
	LOCAL_ADDRESS,      // &local (for pre processing address shifts)
	LOCAL_MEMORY,       // local (but LinkNr = address)
	// special
	CLASS,
	ARRAY_BUILDER,		// = [X,Y,...]
	ARRAY_BUILDER_FOR,
	ARRAY_BUILDER_FOR_IF,
	DICT_BUILDER,		// = {"x":y, ...}
	TUPLE,				// = (X,Y,...)
	TUPLE_EXTRACTION,	// (X,Y,...) = ...
	CONSTRUCTOR_AS_FUNCTION,
	// abstract syntax tree
	ABSTRACT_TOKEN,
	ABSTRACT_OPERATOR,
	ABSTRACT_ELEMENT,
	ABSTRACT_CALL,
	ABSTRACT_TYPE_REFERENCE,// X&
	ABSTRACT_TYPE_STAR,    // X*
	ABSTRACT_TYPE_LIST,    // X[]
	ABSTRACT_TYPE_DICT,    // X{}
	ABSTRACT_TYPE_OPTIONAL,// X?
	ABSTRACT_TYPE_CALLABLE,// X->Y
	ABSTRACT_VAR,          // var x ...
	// compilation
	VAR_TEMP,
	DEREF_VAR_TEMP,
	DEREF_LOCAL_MEMORY,
	REGISTER,
	DEREF_REGISTER,
	LABEL,
	DEREF_LABEL,
	IMMEDIATE,
	GLOBAL_LOOKUP,       // ARM
	DEREF_GLOBAL_LOOKUP, // ARM
};

// single operand/command
class Node : public Sharable<base::Empty> {
public:
	NodeKind kind;
	int token_id;
	int64 link_no;
	// parameters
	shared_array<Node> params;
	// linking of class function instances
	// return value
	const Class *type;
	Flags flags;
	bool is_mutable() const;

	Node(NodeKind kind, int64 link_no, const Class *type, Flags flags = Flags::MUTABLE, int token_id = -1);
	/*Node(const Class *c);
	Node(const Block *b);
	Node(const Constant *c);*/
	~Node();
	void set_mutable(bool _mutable);
	bool is_call() const;
	bool is_function() const;
	Block *as_block() const;
	Function *as_func() const;
	const Class *as_class() const;
	Constant *as_const() const;
	Operator *as_op() const;
	AbstractOperator *as_abstract_op() const;
	Statement *as_statement() const;
	SpecialFunction *as_special_function() const;
	void *as_func_p() const;
	void *as_const_p() const;
	void *as_global_p() const;
	Variable *as_global() const;
	Variable *as_local() const;
	string as_token() const;
	void set_num_params(int n);
	void set_param(int index, shared<Node> p);
	void set_instance(shared<Node> p);
	void set_type(const Class *type);
	string signature(const Class *ns = nullptr) const;
	string str(const Class *ns = nullptr) const;
	void show(const Class *ns = nullptr) const;

	shared<Node> shallow_copy() const;
	shared<Node> ref(const Class *t) const;
	shared<Node> ref(SyntaxTree *tree) const;
	shared<Node> deref(const Class *override_type = nullptr) const;
	shared<Node> shift(int64 shift, const Class *type, int token_id = -1) const;
	shared<Node> change_type(const Class *type, int token_id = -1) const;
	shared<Node> deref_shift(int64 shift, const Class *type, int token_id) const;
};

void clear_nodes(Array<Node*> &nodes);
void clear_nodes(Array<Node*> &nodes, Node *keep);

shared<Node> cp_node(shared<Node> c, Block *parent_block = nullptr);

Array<const Class*> node_extract_param_types(const shared<Node> n);
bool node_is_member_function_with_instance(shared<Node> n);
bool is_type_tuple(const shared<Node> n);
Array<const Class*> class_tuple_extract_classes(const shared<Node> n);

shared<Node> add_node_statement(StatementID id, int token_id = -1, const Class *type = TypeVoid);//, const shared_array<Node> &params);
shared<Node> add_node_special_function_call(SpecialFunctionID id, int token_id = -1, const Class *type = TypeVoid);
shared<Node> add_node_special_function_name(SpecialFunctionID id, int token_id = -1, const Class *type = TypeVoid);
shared<Node> add_node_member_call(const Function *f, const shared<Node> inst, int token_id = -1, const shared_array<Node> &params = {}, bool force_non_virtual = false);
shared<Node> add_node_func_name(const Function *f, int token_id = -1);
shared<Node> add_node_class(const Class *c, int token_id = -1);
shared<Node> add_node_call(const Function *f, int token_id = -1);
shared<Node> add_node_const(const Constant *c, int token_id = -1);
//shared<Node> add_node_block(Block *b);
shared<Node> add_node_operator(const Operator *op, const shared<Node> p1, const shared<Node> p2, int token_id = -1, const Class *override_type = nullptr);
shared<Node> add_node_operator_by_inline(InlineID inline_index, const shared<Node> p1, const shared<Node> p2, int token_id = -1, const Class *override_type = nullptr);
shared<Node> add_node_global(const Variable *var, int token_id = -1);
shared<Node> add_node_local(const Variable *var, int token_id = -1);
shared<Node> add_node_local(const Variable *var, const Class *type, int token_id = -1);
shared<Node> add_node_parray(shared<Node> p, shared<Node> index, const Class *type);
shared<Node> add_node_dyn_array(shared<Node> array, shared<Node> index);
shared<Node> add_node_array(shared<Node> array, shared<Node> index, const Class *override_type = nullptr);
shared<Node> add_node_constructor(const Function *f, int token_id = -1);
shared<Node> make_constructor_static(shared<Node> n, const string &name);

string kind2str(NodeKind kind);
string node2str(SyntaxTree *s, Node *n);

}


#endif /* SRC_LIB_KABA_SYNTAX_NODE_H_ */
