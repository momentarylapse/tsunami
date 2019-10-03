/*
 * Node.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_NODE_H_
#define SRC_LIB_KABA_SYNTAX_NODE_H_


#include "../../base/base.h"

namespace Kaba {

class Class;
class Block;
class SyntaxTree;
class Script;
class Function;
class Variable;
class Constant;
class Operator;
class PrimitiveOperator;
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
	FUNCTION_CALL,      // = real function call
	VIRTUAL_CALL,       // = virtual function call
	INLINE_CALL,        // = function defined inside the compiler...
	POINTER_CALL,       // = function call via pointer
	STATEMENT,          // = if/while/break/...
	BLOCK,              // = block of commands {...}
	OPERATOR,
	PRIMITIVE_OPERATOR, // tentative...
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
	ARRAY_BUILDER,
	ARRAY_BUILDER_FOR,
	ARRAY_BUILDER_FOR_IF,
	CONSTRUCTOR_AS_FUNCTION,
	// compilation
	VAR_TEMP,
	DEREF_VAR_TEMP,
	DEREF_LOCAL_MEMORY,
	REGISTER,
	DEREF_REGISTER,
	MARKER,
	DEREF_MARKER,
	IMMEDIATE,
	GLOBAL_LOOKUP,       // ARM
	DEREF_GLOBAL_LOOKUP, // ARM
};

class Node;

// single operand/command
class Node {
public:
	NodeKind kind;
	int64 link_no;
	// parameters
	Array<Node*> uparams;
	// linking of class function instances
	//Node *instance;
	// return value
	const Class *type;
	Node(NodeKind kind, int64 link_no, const Class *type);
	/*Node(const Class *c);
	Node(const Block *b);
	Node(const Constant *c);*/
	virtual ~Node();
	Block *as_block() const;
	Function *as_func() const;
	const Class *as_class() const;
	Constant *as_const() const;
	Operator *as_op() const;
	PrimitiveOperator *as_prim_op() const;
	Statement *as_statement() const;
	void *as_func_p() const;
	void *as_const_p() const;
	void *as_global_p() const;
	Variable *as_global() const;
	Variable *as_local() const;
	void set_num_uparams(int n);
	void set_uparam(int index, Node *p);
	void set_instance(Node *p);
	//Array<Node*> uparams() const;
	string sig() const;
	string str() const;
	void show() const;
};

void clear_nodes(Array<Node*> &nodes);
void clear_nodes(Array<Node*> &nodes, Node *keep);

string kind2str(NodeKind kind);
string node2str(SyntaxTree *s, Node *n);

// {...}-block
class Block : public Node {
public:
	Block(Function *f, Block *parent);
	virtual ~Block();
	Array<Variable*> vars;
	Function *function;
	Block *parent;
	void *_start, *_end; // opcode range
	int _label_start, _label_end;
	int level;
	void add(Node *c);
	void set(int index, Node *c);
	
	const Class *name_space() const;

	Variable *get_var(const string &name);
	Variable *add_var(const string &name, const Class *type);
};

}


#endif /* SRC_LIB_KABA_SYNTAX_NODE_H_ */
