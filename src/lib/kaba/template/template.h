/*
 * template.h
 *
 *  Created on: 22 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_KABA_PARSER_TEMPLATE_H_
#define SRC_LIB_KABA_PARSER_TEMPLATE_H_

#include <functional>
#include "../syntax/Class.h"

namespace kaba {

class Function;
class Class;
class Node;
class Block;
class Parser;
class SyntaxTree;
class Context;
class TemplateManager;



class TemplateClassInstantiator {
public:
	TemplateClassInstantiator();
	virtual ~TemplateClassInstantiator() = default;
	virtual Class* declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) = 0;
	virtual void add_function_headers(Class* c) = 0;
	const Class* create_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id);

	// helpers
	static Class* create_raw_class(SyntaxTree* tree, const string& name, Class::Type type, int size, int alignment, int array_size, const Class* parent, const Array<const Class*>& params, int token_id);
};

class TemplateClassInstanceManager : public Sharable<base::Empty> {
public:
	TemplateClassInstanceManager(const Class *template_class, const Array<string>& params_names, TemplateClassInstantiator* instantiator);

	const Class* request_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id);
	const Class* create_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id);
	Class* declare_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id);

	struct ClassInstance {
		const Class* c;
		Array<const Class*> params;
		int array_size;
	};

	const Class *template_class;
	Array<string> param_names;
	Array<ClassInstance> instances;
	owned<TemplateClassInstantiator> instantiator;
};

class TemplateManager {
public:
	explicit TemplateManager(Context* c);
	void copy_from(TemplateManager* m);
	void clear_from_module(Module* m);

// functions

	using FunctionCreateF = std::function<Function*(SyntaxTree*, const Array<const Class*>&, int)>;
	
	void add_function_template(Function* f_template, const Array<string>& param_names, FunctionCreateF f_create = nullptr);
	Function* request_function_instance(SyntaxTree *tree, Function *f0, const Array<const Class*> &params, Block *block, const Class *ns, int token_id);
	Function* request_function_instance_matching(SyntaxTree *tree, Function *f0, const shared_array<Node> &params, Block *block, const Class *ns, int token_id);

// classes
	Class *add_class_template(SyntaxTree* tree, const string& name, const Array<string>& param_names, TemplateClassInstantiator* instantiator);
	const Class *request_class_instance(SyntaxTree *tree, const Class *c0, const Array<const Class*> &params, int token_id);
	const Class *request_class_instance(SyntaxTree *tree, const Class *c0, const Array<const Class*> &params, int array_size, int token_id);
	Class *declare_new_class(SyntaxTree *tree, const Class *c0, const Array<const Class*> &params, int array_size, int token_id);

	void add_explicit_class_instance(SyntaxTree *tree, const Class* t_instance, const Class* t_template, const Array<const Class*> &params, int array_size = 0);


// convenience

	const Class *request_pointer(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_shared(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_shared_not_null(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_owned(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_owned_not_null(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_xfer(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_alias(SyntaxTree *tree, const Class *parent, int token_id);
	const Class *request_reference(SyntaxTree *tree, const Class *base, int token_id);
	const Class *request_list(SyntaxTree *tree, const Class *element_type, int token_id);
	const Class *request_array(SyntaxTree *tree, const Class *element_type, int num_elements, int token_id);
	const Class *request_dict(SyntaxTree *tree, const Class *element_type, int token_id);
	const Class *request_optional(SyntaxTree *tree, const Class *param, int token_id);
	const Class *request_callable_fp(SyntaxTree *tree, Function *f, int token_id);
	const Class *request_callable_fp(SyntaxTree *tree, const Array<const Class*> &params, const Class *ret, int token_id);
	const Class *request_callable_bind(SyntaxTree *tree, const Array<const Class*> &params, const Class *ret, const Array<const Class*> &captures, const Array<bool> &capture_via_ref, int token_id);
	const Class *request_product(SyntaxTree *tree, const Array<const Class*> &classes, int token_id);
	const Class *request_future(SyntaxTree *tree, const Class *base, int token_id);
	const Class *request_futurecore(SyntaxTree *tree, const Class *base, int token_id);

private:
	Context *context;

	struct FunctionInstance {
		Function *f;
		Array<const Class*> params;
	};
	struct FunctionTemplate {
		Function *func;
		Array<string> params;
		FunctionCreateF f_create;
		Array<FunctionInstance> instances;
	};
	Array<FunctionTemplate> function_templates;

	shared_array<TemplateClassInstanceManager> class_managers;

	FunctionTemplate &get_function_template(SyntaxTree *tree, Function *f0, int token_id);
	TemplateClassInstanceManager &get_class_manager(SyntaxTree *tree, const Class *c0, int token_id);

	Function *full_copy(SyntaxTree *tree, Function *f0);
	shared<Node> node_replace(SyntaxTree *tree, shared<Node> n, const Array<string> &names, const Array<const Class*> &params);
	Function *instantiate_function(SyntaxTree *tree, FunctionTemplate &t, const Array<const Class*> &params, Block *block, const Class *ns, int token_id);
	//const Class *instantiate_class(SyntaxTree *tree, ClassTemplate &t, const Array<const Class*> &params, int array_size, int token_id);

	void match_parameter_type(shared<Node> p, const Class *t, std::function<void(const string&, const Class*)> f);
};





class TemplateClassInstantiatorList : public TemplateClassInstantiator {
	Class* declare_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) override;
	void add_function_headers(Class* c) override;
};

}

#endif /* SRC_LIB_KABA_PARSER_TEMPLATE_H_ */
