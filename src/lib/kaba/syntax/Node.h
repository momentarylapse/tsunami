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
	None = -1,
	Unknown,
	Placeholder,
	// data
	VarLocal,
	VarGlobal,
	Function,            // = just the name
	Constant,
	// execution
	CallFunction,        // = real function call
	CallVirtual,         // = virtual function call
	CallInline,          // = function defined inside the compiler...
	CallRawPointer,      // = function call via a raw pointer
	Statement,           // = if/while/break/...
	CallSpecialFunction, // = len(), sorted() etc
	SpecialFunctionName, // = len, sorted etc
	Block,               // = block of commands {...}
	Operator,
	NamedParameter,
	// data altering
	AddressShift,        // = struct.element
	Array,               // = []
	PointerAsArray,      // = []
	DynamicArray,        // = []
	Reference,           // = &
	Dereference,         // = *
	DereferenceAddressShift,// = ->
	Definitely,          // = x!
	ConstantByAddress,
	Address,             // &global (for pre processing address shifts)
	Memory,              // global (but LinkNr = address)
	LocalAddress,        // &local (for pre processing address shifts)
	LocalMemory,         // local (but LinkNr = address)
	// special
	Class,
	ArrayBuilder,        // = [X,Y,...]
	ArrayBuilderFor,
	ArrayBuilderForIf,
	DictBuilder,         // = {"x":y, ...}
	Tuple,               // = (X,Y,...)
	TupleExtraction,     // (X,Y,...) = ...
	ConstructorAsFunction,
	Slice,               // = A:B or A:B:C
	// abstract syntax tree
	AbstractToken,
	AbstractOperator,
	AbstractElement,
	AbstractCall,
	AbstractTypeReference,// X&
	AbstractTypeStar,    // X*
	AbstractTypeList,    // X[]
	AbstractTypeDict,    // X{}
	AbstractTypeOptional,// X?
	AbstractTypeCallable,// X->Y
	AbstractVar,         // var x ...
	// compilation
	VarTemp,
	DereferenceVarTemp,
	DerefereceLocalMemory,
	Register,
	DereferenceRegister,
	Label,
	DereferenceLabel,
	Immediate,
	GlobalLookup,        // ARM
	DereferenceGlobalLookup, // ARM
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

	Node(NodeKind kind, int64 link_no, const Class *type, Flags flags = Flags::Mutable, int token_id = -1);
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
shared<Node> add_node_slice(shared<Node> start, shared<Node> end);
shared<Node> add_node_constructor(const Function *f, int token_id = -1);
shared<Node> make_constructor_static(shared<Node> n, const string &name);
shared<Node> add_node_named_parameter(SyntaxTree* tree, int name_token_id, shared<Node> param);
shared<Node> add_node_token(SyntaxTree* tree, int token_id);

string kind2str(NodeKind kind);
string node2str(SyntaxTree *s, Node *n);

}


#endif /* SRC_LIB_KABA_SYNTAX_NODE_H_ */
