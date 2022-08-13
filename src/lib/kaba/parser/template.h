/*
 * template.h
 *
 *  Created on: 22 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_TEMPLATE_H_
#define SRC_LIB_KABA_PARSER_TEMPLATE_H_

namespace kaba {

class Function;
class Class;
class Node;
class Block;

class TemplateManager {
public:

	static void add_template(Function *f, const Array<string> &param_names);
	static Function *get_instantiated(Parser *parser, Function *f0, const Array<const Class*> &params, Block *block, const Class *ns, int token_id);
	static Function *get_instantiated_matching(Parser *parser, Function *f0, const shared_array<Node> &params, Block *block, const Class *ns, int token_id);

private:
	struct Instance {
		Function *f;
		Array<const Class*> params;
	};
	struct Template {
		Function *func;
		Array<string> params;
		Array<Instance> instances;
	};
	static Array<Template> templates;

	static Template &get_template(Parser *parser, Function *f0, int token_id);

	static Function *full_copy(Parser *parser, Function *f0);
	static shared<Node> node_replace(Parser *parser, shared<Node> n, const Array<string> &names, const Array<const Class*> &params);
	static Function *instantiate(Parser *parser, Template &t, const Array<const Class*> &params, Block *block, const Class *ns, int token_id);

	static void match_parameter_type(shared<Node> p, const Class *t, std::function<void(const string&, const Class*)> f);
};

}

#endif /* SRC_LIB_KABA_PARSER_TEMPLATE_H_ */
