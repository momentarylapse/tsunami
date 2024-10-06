/*
 * template.cpp
 *
 *  Created on: 22 May 2022
 *      Author: michi
 */

#include "../kaba.h"
#include "../parser/Parser.h"
#include "../parser/Concretifier.h"
#include "template.h"
#include "../../os/msg.h"
#include "../../base/iter.h"
#include "../../base/algo.h"

namespace kaba {

string type_list_to_str(const Array<const Class*> &params);

TemplateManager::TemplateManager(Context *c) {
	context = c;
}

void TemplateManager::copy_from(TemplateManager *t) {
	function_templates = t->function_templates;
	for (auto mm: weak(t->class_managers)) {
		auto m = new TemplateClassInstanceManager(mm->template_class, mm->param_names, mm->instantiator);
		m->instances = mm->instances;
		class_managers.add(m);
	}
	//class_managers = t->class_managers;
}


void TemplateManager::add_function_template(Function *f_template, const Array<string> &param_names, FunctionCreateF f_create) {
	if (config.verbose)
		msg_write("ADD FUNC TEMPLATE");
	FunctionTemplate t;
	t.func = f_template;
	t.params = param_names;
	t.f_create = f_create;
	function_templates.add(t);
}

Class *TemplateManager::add_class_template(SyntaxTree *tree, const string &name, const Array<string> &param_names, TemplateClassInstantiator* instantiator) {
	if (config.verbose)
		msg_write("ADD CLASS TEMPLATE " + name);
	//msg_write("add class template  " + c->long_name());
	Class *c = new Class(nullptr, name, 0, 1, tree);
	flags_set(c->flags, Flags::Template);
	auto m = new TemplateClassInstanceManager(c, param_names, instantiator);
	class_managers.add(m);
	return c;
}

void TemplateManager::clear_from_module(Module *m) {
}


void show_node_details(shared<Node> n) {
	if (n->kind == NodeKind::Block) {
		msg_write("block " + p2s(n.get()) + "  ->  " + p2s(n->as_block()->function));
		msg_right();
		for (auto v: n->as_block()->vars)
			msg_write(v->name + ": " + v->type->name + "  " + p2s(v));
	}
	for (auto p: weak(n->params))
		show_node_details(p);
	if (n->kind == NodeKind::Block) {
		msg_left();
	}
}

void show_func_details(Function *f) {
	msg_write("DETAILS:    " + f->signature() + "  " + p2s(f));
	show_node_details(f->block.get());
}

Function *TemplateManager::full_copy(SyntaxTree *tree, Function *f0) {
	//msg_error("FULL COPY");
	auto f = f0->create_dummy_clone(f0->name_space);
	f->block = cp_node(f0->block.get())->as_block();
	flags_clear(f->flags, Flags::NeedsOverride);

	auto convert = [f] (shared<Node> n) {
		if (n->kind != NodeKind::Block)
			return n;
		auto b = n->as_block();
		//msg_write("block " + p2s(b));
		for (auto&& [vi,v]: enumerate(b->vars)) {
			int i = weak(b->function->var).find(v);
			b->vars[vi] = f->var[i].get();
		}
		b->function = f;
		return n;
	};

	//convert(f->block.get())->as_block();
	tree->transform_node(f->block.get(), convert);

	//show_func_details(f0);
	//show_func_details(f);

	//parser->do_error("x", -1);

	return f;
}

Function *TemplateManager::request_function_instance(SyntaxTree *tree, Function *f0, const Array<const Class*> &params, int token_id) {
	auto &t = get_function_template(tree, f0, token_id);
	
	// already instantiated?
	for (auto &i: t.instances)
		if (i.params == params)
			return i.f;
	
	// new
	FunctionInstance ii;
	ii.f = instantiate_function(tree, t, params, token_id);
	ii.params = params;
	t.instances.add(ii);
	return ii.f;
}

const Class* TemplateManager::request_class_instance(SyntaxTree *tree, const Class *c0, const Array<const Class*> &params, int array_size, int token_id) {
	auto &t = get_class_manager(tree, c0, token_id);
	return t.request_instance(tree, params, array_size, token_id);
}

const Class* TemplateManager::request_class_instance(SyntaxTree *tree, const Class *c0, const Array<const Class*> &params, int token_id) {
	return request_class_instance(tree, c0, params, 0, token_id);
}

Class* TemplateManager::declare_new_class(SyntaxTree *tree, const Class *c0, const Array<const Class*> &params, int array_size, int token_id) {
	auto &t = get_class_manager(tree, c0, token_id);
	return t.declare_instance(tree, params, array_size, token_id);
}

void TemplateManager::match_parameter_type(shared<Node> p, const Class *t, std::function<void(const string&, const Class*)> f) {
	if (p->kind == NodeKind::AbstractToken) {
		// direct
		string token = p->as_token();
		f(token, t);
	} else if (p->kind == NodeKind::AbstractTypeList) {
		if (t->is_list())
			match_parameter_type(p->params[0], t->get_array_element(), f);
	} else if (p->kind == NodeKind::AbstractTypeStar) {
		if (t->is_pointer_raw())
			match_parameter_type(p->params[0], t->param[0], f);
	} else if (p->kind == NodeKind::AbstractTypeReference) {
		if (t->is_reference())
			match_parameter_type(p->params[0], t->param[0], f);
	} /*else if (p->kind == NodeKind::ABSTRACT_TYPE_SHARED) {
		if (t->is_pointer_shared())
			match_parameter_type(p->params[0], t->param[0], f);
	} else if (p->kind == NodeKind::ABSTRACT_TYPE_SHARED_NOT_NULL) {
		if (t->is_pointer_shared_not_null())
			match_parameter_type(p->params[0], t->param[0], f);
	} else if (p->kind == NodeKind::ARRAY) {
		if (p->params[0]->kind == NodeKind::ABSTRACT_TYPE_POINTER and t->is_pointer_raw())
			match_parameter_type(p->params[1], t->param[0], f);
		else if (p->params[0]->kind == NodeKind::ABSTRACT_TYPE_SHARED and t->is_pointer_shared())
			match_parameter_type(p->params[1], t->param[0], f);
		else if (p->params[0]->kind == NodeKind::ABSTRACT_TYPE_SHARED_NOT_NULL and t->is_pointer_shared_not_null())
			match_parameter_type(p->params[1], t->param[0], f);
	}*/
}

Function *TemplateManager::request_function_instance_matching(SyntaxTree *tree, Function *f0, const shared_array<Node> &params, int token_id) {
	if (config.verbose)
		msg_write("____MATCHING");
	auto &t = get_function_template(tree, f0, token_id);

	Array<const Class*> arg_types;
	arg_types.resize(t.params.num);

	auto set_match_type = [&arg_types, &t, tree, token_id] (const string &token, const Class *type) {
		for (int j=0; j<t.params.num; j++)
			if (token == t.params[j]) {
				if (arg_types[j])
					if (arg_types[j] != type)
						tree->do_error(format("inconsistent template parameter: %s = %s  vs  %s", token, arg_types[j]->long_name(), type->long_name()), token_id);
				arg_types[j] = type;
				if (config.verbose)
					msg_error("FOUND: " + token + " = " + arg_types[j]->name);
			}
	};

	if (params.num != f0->abstract_param_types.num)
		tree->do_error(format("not able to match all template parameters: %d parameters given, %d expected", params.num, f0->abstract_param_types.num), token_id);

	for (auto&& [i,p]: enumerate(weak(params))) {
		if (p->type == TypeUnknown)
			tree->do_error(format("parameter #%d '%s' has undecided type when trying to match template arguments", i+1, f0->var[i]->name), token_id);
		match_parameter_type(f0->abstract_param_types[i], p->type, set_match_type);
	}

	for (auto t: arg_types)
		if (!t)
			tree->do_error("not able to match all template parameters", token_id);


	return request_function_instance(tree, f0, arg_types, token_id);
}

TemplateManager::FunctionTemplate &TemplateManager::get_function_template(SyntaxTree *tree, Function *f0, int token_id) {
	for (auto &t: function_templates)
		if (t.func == f0)
			return t;

	tree->do_error("INTERNAL: can not find template...", token_id);
	return function_templates[0];
}

TemplateClassInstanceManager& TemplateManager::get_class_manager(SyntaxTree *tree, const Class *c0, int token_id) {
	for (auto t: weak(class_managers))
		if (t->template_class == c0)
			return *t;

	tree->do_error("INTERNAL: can not find template...", token_id);
	return *class_managers[0];
}

/*static const Class *concretify_type(shared<Node> n, SyntaxTree *tree, Block *block, const Class *ns) {
	return parser->concretify_as_type(n, block, ns);
}*/

shared<Node> TemplateManager::node_replace(SyntaxTree *tree, shared<Node> n, const Array<string> &names, const Array<const Class*> &params) {
	//return parser->concretify_as_type(n, block, ns);
	return tree->transform_node(n, [&names, &params] (shared<Node> nn) {
		if (nn->kind == NodeKind::AbstractToken) {
			string token = nn->as_token();
			for (int i=0; i<names.num; i++)
				if (token == names[i])
					return add_node_class(params[i], nn->token_id);
		}
		return nn;
	});
}

Function *TemplateManager::instantiate_function(SyntaxTree *tree, FunctionTemplate &t, const Array<const Class*> &params, int token_id) {
	if (config.verbose)
		msg_write("INSTANTIATE TEMPLATE");
	Function *f0 = t.func;

	Function *f = nullptr;
	if (t.f_create) {
		f = t.f_create(tree, params, token_id);
	} else {
		f = full_copy(tree, f0);
		f->name += format("[%s]", type_list_to_str(params));

		// replace in parameters/return type
		for (int i=0; i<f->num_params; i++) {
			f->abstract_param_types[i] = node_replace(tree, f->abstract_param_types[i], t.params, params);
			//f->abstract_param_types[i]->show();
		}
		if (f->abstract_return_type)
			f->abstract_return_type = node_replace(tree, f->abstract_return_type, t.params, params);

		// replace in body
		for (int i=0; i<f->block->params.num; i++)
			f->block->params[i] = node_replace(tree, f->block->params[i], t.params, params);
	}

	// concretify
	try {

		tree->parser->con.concretify_function_header(f);

		f->update_parameters_after_parsing();

		tree->parser->con.concretify_function_body(f);

		if (config.verbose)
			f->block->show();

		auto __ns = const_cast<Class*>(f0->name_space);
		__ns->add_function(tree, f, false, false);

		tree->functions.add(f);

	} catch (kaba::Exception &e) {
		//msg_write(e.message());
		tree->do_error(format("failed to instantiate template %s: %s", f->name, e.message()), token_id);
	}

	return f;
}


extern const Class *TypeRawT;
extern const Class *TypeReferenceT;
extern const Class *TypeXferT;
extern const Class *TypeAliasT;
extern const Class *TypeSharedT;
extern const Class *TypeSharedNotNullT;
extern const Class *TypeOwnedT;
extern const Class *TypeOwnedNotNullT;
extern const Class *TypeArrayT;
extern const Class *TypeListT;
extern const Class *TypeDictT;
extern const Class *TypeOptionalT;
extern const Class *TypeProductT;
extern const Class *TypeFutureT;
extern const Class *TypePromiseT;
extern const Class *TypeFutureCoreT;
extern const Class *TypeCallableFPT;
extern const Class *TypeCallableBindT;

extern const Class *TypeDynamicArray;
extern const Class *TypeDictBase;
extern const Class *TypeCallableBase;


TemplateClassInstantiator::TemplateClassInstantiator() = default;

const Class* TemplateClassInstantiator::create_new_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	auto c = declare_new_instance(tree, params, array_size, token_id);
	add_function_headers(c);
	return c;
}

TemplateClassInstanceManager::TemplateClassInstanceManager(const Class* template_class, const Array<string>& param_names, shared<TemplateClassInstantiator> instantiator) {
	this->template_class = template_class;
	this->param_names = param_names;
	this->instantiator = instantiator;
}

const Class* TemplateClassInstanceManager::request_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	// already instanciated?
	for (auto &i: instances)
		if (i.params == params and i.array_size == array_size)
			return i.c;
	return create_instance(tree, params, array_size, token_id);
}

const Class* TemplateClassInstanceManager::create_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	if (config.verbose)
		msg_write("INSTANTIATE TEMPLATE CLASS  " + template_class->name + " ... " + params[0]->name);
	ClassInstance ii;
	ii.c = instantiator->create_new_instance(tree, params, array_size, token_id);
	ii.array_size = array_size;
	ii.params = params;
	instances.add(ii);
	return ii.c;
}

Class* TemplateClassInstanceManager::declare_instance(SyntaxTree *tree, const Array<const Class*> &params, int array_size, int token_id) {
	if (config.verbose)
		msg_write("INSTANTIATE TEMPLATE CLASS  " + template_class->name + " ... " + params[0]->name);
	auto c = instantiator->declare_new_instance(tree, params, array_size, token_id);
	ClassInstance ii;
	ii.c = c;
	ii.array_size = array_size;
	ii.params = params;
	instances.add(ii);
	return c;
}


Class* TemplateClassInstantiator::create_raw_class(SyntaxTree* tree, const string& name, const Class* from_template, int size, int alignment, int array_size, const Class* parent, const Array<const Class*>& params, int token_id) {
	/*msg_write("CREATE " + name);
	msg_write(p2s(tree));
	msg_write(p2s(tree->implicit_symbols.get()));*/

	auto ns = tree->implicit_symbols.get();

	Class *t = new Class(from_template, name, size, alignment, tree, parent, params);
	t->token_id = token_id;
	t->array_length = array_size;
	tree->owned_classes.add(t);

	// link namespace
	ns->classes.add(t);
	t->name_space = ns;
	return t;
}

// skip the "self" parameter!
Function* TemplateClassInstantiator::add_func_header(Class *t, const string &name, const Class *return_type, const Array<const Class*> &param_types, const Array<string> &param_names, Function *cf, Flags flags, const shared_array<Node> &def_params) {
	Function *f = t->owner->add_function(name, return_type, t, flags); // always member-function??? no...?
	f->auto_declared = true;
	f->token_id = t->token_id;
	for (auto&& [i,p]: enumerate(param_types)) {
		f->literal_param_type.add(p);
		f->block->add_var(param_names[i], p, Flags::None);
		f->num_params ++;
	}
	f->default_parameters = def_params;
	f->update_parameters_after_parsing();
	if (config.verbose)
		msg_write("ADD HEADER " + f->signature(TypeVoid));

	bool override = cf;
	t->add_function(t->owner, f, false, override);
	return f;
}


const Class *TemplateManager::request_pointer(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeRawT, {base}, token_id);
}

const Class *TemplateManager::request_shared(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeSharedT, {base}, token_id);
}

const Class *TemplateManager::request_shared_not_null(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeSharedNotNullT, {base}, token_id);
}

const Class *TemplateManager::request_owned(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeOwnedT, {base}, token_id);
}

const Class *TemplateManager::request_owned_not_null(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeOwnedNotNullT, {base}, token_id);
}

const Class *TemplateManager::request_xfer(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeXferT, {base}, token_id);
}

const Class *TemplateManager::request_alias(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeAliasT, {base}, token_id);
}

const Class *TemplateManager::request_reference(SyntaxTree *tree, const Class *base, int token_id) {
	return request_class_instance(tree, TypeReferenceT, {base}, token_id);
}

const Class *TemplateManager::request_list(SyntaxTree *tree, const Class *element_type, int token_id) {
	return request_class_instance(tree, TypeListT, {element_type}, token_id);
}

const Class *TemplateManager::request_array(SyntaxTree *tree, const Class *element_type, int num_elements, int token_id) {
	return request_class_instance(tree, TypeArrayT, {element_type}, num_elements, token_id);
}

const Class *TemplateManager::request_dict(SyntaxTree *tree, const Class *element_type, int token_id) {
	return request_class_instance(tree, TypeDictT, {element_type}, token_id);
}

const Class *TemplateManager::request_optional(SyntaxTree *tree, const Class *param, int token_id) {
	return request_class_instance(tree, TypeOptionalT, {param}, token_id);
}

const Class *TemplateManager::request_future(SyntaxTree *tree, const Class *param, int token_id) {
	return request_class_instance(tree, TypeFutureT, {param}, token_id);
}

const Class *TemplateManager::request_promise(SyntaxTree *tree, const Class *param, int token_id) {
	return request_class_instance(tree, TypePromiseT, {param}, token_id);
}

const Class *TemplateManager::request_futurecore(SyntaxTree *tree, const Class *param, int token_id) {
	return request_class_instance(tree, TypeFutureCoreT, {param}, token_id);
}



string make_callable_signature(const Array<const Class*> &params, const Class *ret) {
	// maybe some day...
	string signature;// = param->name;
	for (int i=0; i<params.num; i++) {
		if (i > 0)
			signature += ",";
		signature += class_name_might_need_parantheses(params[i]);
	}
	if (params.num == 0)
		signature = "void";
	if (params.num > 1)
		signature = "(" + signature + ")";
	if (params.num == 0 or (params.num == 1 and params[0] == TypeVoid)) {
		signature = "void";
	}
	return signature + "->" + class_name_might_need_parantheses(ret);
}


// input {}->R  OR  void->void   BOTH create  void->R
const Class *TemplateManager::request_callable_fp(SyntaxTree *tree, const Array<const Class*> &param, const Class *ret, int token_id) {

	string name = make_callable_signature(param, ret);

	auto params_ret = param;
	if ((param.num == 1) and (param[0] == TypeVoid))
		params_ret = {};
	params_ret.add(ret);

	auto ff = request_class_instance(tree, TypeCallableFPT, params_ret, token_id);

	auto c = request_pointer(tree, ff, token_id);
	const_cast<Class*>(c)->name = name;
	return c;
}

// inner callable: params [A,B,C,D,E]
// captures: [-,x0,-,-,x1]
// class CallableBind
//     func __init__(f, c0, c1)
//     func call(a,b,c)
//         f(a,x0,b,c,c1)
// (A,C,D) -> R
const Class *TemplateManager::request_callable_bind(SyntaxTree *tree, const Array<const Class*> &params, const Class *ret, const Array<const Class*> &captures, const Array<bool> &capture_via_ref, int token_id) {
	int magic = 0;
	for (auto [i,b]: enumerate(captures)) {
		if (!b)
			continue;
		magic += (1 << i);
		if (capture_via_ref[i])
			magic += (1 << i) << 16;
	}

	Array<const Class*> inner_params_ret = params;
	inner_params_ret.add(ret);

	return request_class_instance(tree, TypeCallableBindT, inner_params_ret, magic, token_id);
}

const Class *TemplateManager::request_product(SyntaxTree *tree, const Array<const Class*> &classes, int token_id) {
	return request_class_instance(tree, TypeProductT, classes, token_id);
}

void TemplateManager::add_explicit_class_instance(SyntaxTree *tree, const Class* c_instance, const Class* c_template, const Array<const Class*> &params, int array_size) {
	auto &t = get_class_manager(tree, c_template, -1);

	TemplateClassInstanceManager::ClassInstance ii;
	ii.c = c_instance;
	ii.params = params;
	ii.array_size = array_size;
	t.instances.add(ii);
}



}

