/*
 * Node.h
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_SYNTAX_NODE_H_
#define SRC_LIB_KABA_SYNTAX_NODE_H_


#include "../../base/base.h"

namespace Kaba{

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


enum
{
	KIND_UNKNOWN,
	// data
	KIND_VAR_LOCAL,
	KIND_VAR_GLOBAL,
	KIND_FUNCTION_NAME,
	KIND_FUNCTION_POINTER,
	KIND_CONSTANT,
	// execution
	KIND_FUNCTION_CALL,      // = real function call
	KIND_VIRTUAL_CALL,       // = virtual function call
	KIND_INLINE_CALL,        // = function defined inside the compiler...
	KIND_POINTER_CALL,       // = function call via pointer
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
	KIND_CONSTANT_BY_ADDRESS,
	KIND_ADDRESS,            // &global (for pre processing address shifts)
	KIND_MEMORY,             // global (but LinkNr = address)
	KIND_LOCAL_ADDRESS,      // &local (for pre processing address shifts)
	KIND_LOCAL_MEMORY,       // local (but LinkNr = address)
	// special
	KIND_CLASS,
	KIND_ARRAY_BUILDER,
	KIND_CONSTRUCTOR_AS_FUNCTION,
	// compilation
	KIND_VAR_TEMP,
	KIND_DEREF_VAR_TEMP,
	KIND_DEREF_LOCAL_MEMORY,
	KIND_REGISTER,
	KIND_DEREF_REGISTER,
	KIND_MARKER,
	KIND_DEREF_MARKER,
	KIND_IMMEDIATE,
	KIND_GLOBAL_LOOKUP,       // ARM
	KIND_DEREF_GLOBAL_LOOKUP, // ARM
};

class Node;

// single operand/command
class Node
{
public:
	int kind;
	int64 link_no;
	// parameters
	Array<Node*> params;
	// linking of class function instances
	Node *instance;
	// return value
	const Class *type;
	Node(int kind, int64 link_no, const Class *type);
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
	void set_num_params(int n);
	void set_param(int index, Node *p);
	void set_instance(Node *p);
	string sig() const;
	string str() const;
	void show() const;
};

void clear_nodes(Array<Node*> &nodes);
void clear_nodes(Array<Node*> &nodes, Node *keep);

string kind2str(int kind);
string node2str(SyntaxTree *s, Node *n);

// {...}-block
class Block : public Node
{
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

	Variable *get_var(const string &name);
	Variable *add_var(const string &name, const Class *type);
};

}


#endif /* SRC_LIB_KABA_SYNTAX_NODE_H_ */
