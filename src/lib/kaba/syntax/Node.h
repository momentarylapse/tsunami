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

namespace kaba {

class Class;
class Block;
class SyntaxTree;
class Script;
class Function;
class Variable;
class Constant;
class Operator;
class AbstractOperator;
class Statement;


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
	CONSTRUCTOR_AS_FUNCTION,
	// abstract syntax tree
	ABSTRACT_TOKEN,
	ABSTRACT_OPERATOR,
	ABSTRACT_ELEMENT,
	ABSTRACT_CALL,
	ABSTRACT_TYPE_SHARED,  // shared X
	ABSTRACT_TYPE_OWNED,   // owned X
	ABSTRACT_TYPE_POINTER, // X*
	ABSTRACT_TYPE_LIST,    // X[]
	ABSTRACT_TYPE_DICT,    // X{}
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
class Node : public Sharable<Empty> {
public:
	NodeKind kind;
	int token_id = -1;
	int64 link_no;
	// parameters
	shared_array<Node> params;
	// linking of class function instances
	// return value
	const Class *type;
	bool is_const;

	Node(NodeKind kind, int64 link_no, const Class *type, bool is_const = false);
	/*Node(const Class *c);
	Node(const Block *b);
	Node(const Constant *c);*/
	~Node();
	Node *modifiable();
	Node *make_const();
	bool is_call() const;
	bool is_function() const;
	Block *as_block() const;
	Function *as_func() const;
	const Class *as_class() const;
	Constant *as_const() const;
	Operator *as_op() const;
	AbstractOperator *as_abstract_op() const;
	Statement *as_statement() const;
	void *as_func_p() const;
	void *as_const_p() const;
	void *as_global_p() const;
	Variable *as_global() const;
	Variable *as_local() const;
	void set_num_params(int n);
	void set_param(int index, shared<Node> p);
	void set_instance(shared<Node> p);
	void set_type(const Class *type);
	string signature(const Class *ns = nullptr) const;
	string str(const Class *ns = nullptr) const;
	void show(const Class *ns = nullptr) const;

	shared<Node> shallow_copy() const;
	shared<Node> ref(const Class *override_type = nullptr) const;
	shared<Node> deref(const Class *override_type = nullptr) const;
	shared<Node> shift(int64 shift, const Class *type) const;
	shared<Node> deref_shift(int64 shift, const Class *type) const;
};

void clear_nodes(Array<Node*> &nodes);
void clear_nodes(Array<Node*> &nodes, Node *keep);

string kind2str(NodeKind kind);
string node2str(SyntaxTree *s, Node *n);

}


#endif /* SRC_LIB_KABA_SYNTAX_NODE_H_ */
