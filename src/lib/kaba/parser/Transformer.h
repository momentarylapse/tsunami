/*
 * Transformer.h
 *
 *  Created on: 27 Feb 2023
 *      Author: michi
 */

#pragma once

#include <lib/base/pointer.h>
#include <functional>

namespace kaba {

struct SyntaxTree;
struct Block;
struct Node;
struct Variable;
struct Function;

class Transformer {
public:
	explicit Transformer(SyntaxTree* tree);
	~Transformer();

	static void transform(SyntaxTree* tree, std::function<shared<Node>(shared<Node>)> F);
	static void transform_block(Node *block, std::function<shared<Node>(shared<Node>)> F);
	static shared<Node> transform_node(shared<Node> n, std::function<shared<Node>(shared<Node>)> F);

	static void transformb(SyntaxTree* tree, std::function<shared<Node>(shared<Node>, Block*)> F);
	static void transformb_block(Node *block, std::function<shared<Node>(shared<Node>, Block*)> F);
	static shared<Node> transformb_node(shared<Node> n, Block *b, std::function<shared<Node>(shared<Node>, Block*)> F);

	void fully_transform();
	shared<Node> conv_break_down_high_level(shared<Node> n, Block *b);
	shared<Node> conv_break_down_med_level(shared<Node> c);
	void convert_call_by_reference();
	void map_local_variables_to_stack();
	shared<Node> conv_fake_constructors(shared<Node> n);
	shared<Node> conv_class_and_func_to_const(shared<Node> n);
	shared<Node> conv_break_down_low_level(shared<Node> c);
	shared<Node> conv_cbr(shared<Node> c, Variable *var);
	shared<Node> conv_calls(shared<Node> c);
	shared<Node> conv_easyfy_ref_deref(shared<Node> c, int l);
	shared<Node> conv_easyfy_shift_deref(shared<Node> c, int l);
	shared<Node> conv_return_by_memory(shared<Node> n, Function *f);
	shared<Node> conv_func_inline(shared<Node> n);

	// pre processor
	shared<Node> conv_eval_const_func(shared<Node> c);
	shared<Node> conv_eval_const_func_nofunc(shared<Node> c);
	void eval_const_expressions(bool allow_func_eval);
	shared<Node> pre_process_node_addresses(shared<Node> c);
	void pre_processor_addresses();
	void simplify_shift_deref();
	void simplify_ref_deref();

	SyntaxTree *tree = nullptr;
};

}
