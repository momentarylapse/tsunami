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
class Parser;
class Context;

class TemplateManager {
public:

	TemplateManager(Context *c);
	void copy_from(TemplateManager *m);
	
	void add_template(Function *f, const Array<string> &param_names);
	Function *get_instantiated(Parser *parser, Function *f0, const Array<const Class*> &params, Block *block, const Class *ns, int token_id);
	Function *get_instantiated_matching(Parser *parser, Function *f0, const shared_array<Node> &params, Block *block, const Class *ns, int token_id);

	void clear_from_module(Module *m);

private:
	Context *context;

	struct Instance {
		Function *f;
		Array<const Class*> params;
	};
	struct Template {
		Function *func;
		Array<string> params;
		Array<Instance> instances;
	};
	Array<Template> templates;

	Template &get_template(Parser *parser, Function *f0, int token_id);

	Function *full_copy(Parser *parser, Function *f0);
	shared<Node> node_replace(Parser *parser, shared<Node> n, const Array<string> &names, const Array<const Class*> &params);
	Function *instantiate(Parser *parser, Template &t, const Array<const Class*> &params, Block *block, const Class *ns, int token_id);

	void match_parameter_type(shared<Node> p, const Class *t, std::function<void(const string&, const Class*)> f);
};


class ImplicitClassRegistry {
public:
	ImplicitClassRegistry(Context *c);
	void copy_from(ImplicitClassRegistry *i);
	void init();
	const Class *find(const string &name, Class::Type type, int array_size, const Array<const Class*> &params);
	void add(const Class* t);
	void clear_from_module(Module *m);

	Context *context;
	Module *module = nullptr;
	Array<const Class*> classes;
};


}

#endif /* SRC_LIB_KABA_PARSER_TEMPLATE_H_ */
