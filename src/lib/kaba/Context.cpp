#include "Context.h"
#include "kaba.h"
#include "Interpreter.h"
#include "parser/Parser.h"
#include "parser/Concretifier.h"
#include "parser/template.h"
#include "../os/msg.h"

namespace kaba {

VirtualTable* get_vtable(const VirtualBase *p);

Context *default_context = nullptr;


Exception::Exception(const string &_message, const string &_expression, int _line, int _column, Module *s) :
	Asm::Exception(_message, _expression, _line, _column)
{
	filename = s->filename;
}

Exception::Exception(const Asm::Exception &e, Module *s, Function *f) :
	Asm::Exception(e)
{
	filename = s->filename;
	text = format("assembler: %s, %s", message(), f->long_name());
}

string Exception::message() const {
	return format("%s, %s", Asm::Exception::message(), filename);
}




Path absolute_module_path(const Path &filename) {
	if (filename.is_relative())
		return (config.directory << filename).absolute().canonical();
	else
		return filename.absolute().canonical();
}

Context::Context() {
	template_manager = new TemplateManager(this);
	implicit_class_registry = new ImplicitClassRegistry(this);
	external = new ExternalLinkData(this);
}

Context::~Context() {
    clean_up();
}

void Context::__delete__() {
	this->Context::~Context();
}



shared<Module> Context::load_module(const Path &filename, bool just_analyse) {
	//msg_write("loading " + filename.str());

	auto _filename = absolute_module_path(filename);

	// already loaded?
	for (auto ps: public_modules)
		if (ps->filename == _filename)
			return ps;
	
	// load
    auto s = create_empty_module(filename);
	s->load(filename, just_analyse);

	// store module in database
	public_modules.add(s);
	return s;
}

shared<Module> Context::create_module_for_source(const string &buffer, bool just_analyse) {
    auto s = create_empty_module("<from-source>");
	s->just_analyse = just_analyse;
	s->filename = config.default_filename;
	s->syntax->parser = new Parser(s->syntax);
	s->syntax->default_import();
	s->syntax->parser->parse_buffer(buffer, just_analyse);

	if (!just_analyse)
		s->compile();
	return s;
}

shared<Module> Context::create_empty_module(const Path &filename) {
	shared<Module> s = new Module(this, filename);
    return s;
}

/*void Context::remove_module(Module *s) {

	// remove from normal list
	for (int i=0;i<public_modules.num;i++)
		if (public_modules[i] == s)
			public_modules.erase(i);
}*/



// bad:  should clean up in case of errors!
void Context::execute_single_command(const string &cmd) {
	if (cmd.num < 1)
		return;
	//msg_write("command: " + cmd);

    auto s = create_empty_module("<command-line>");
	auto tree = s->syntax;
	tree->default_import();
	auto parser = new Parser(tree);
	tree->parser = parser;

// find expressions
	parser->Exp.analyse(tree, cmd);
	if (parser->Exp.empty()) {
		//clear_exp_buffer(&ps->Exp);
		return;
	}
	
	for (auto p: packages)
		if (!p->used_by_default)
			tree->import_data(p, true, "");

// analyse syntax

	// create a main() function
	Function *func = tree->add_function("--command-func--", TypeVoid, tree->base_class, Flags::STATIC);
	func->_var_size = 0; // set to -1...

	parser->Exp.reset_walker();

	// parse
	func->block->type = TypeUnknown;
	parser->parse_abstract_complete_command(func->block.get());
	if (config.verbose) {
		msg_write("ABSTRACT SINGLE:");
		func->block->show();
	}
	parser->con.concretify_node(func->block.get(), func->block.get(), func->name_space);
	
	// implicit print(...)?
	if (func->block->params.num > 0 and func->block->params[0]->type != TypeVoid) {
		auto n = parser->con.add_converter_str(func->block->params[0], true);
		
		auto f = tree->required_func_global("print");

		auto cmd = add_node_call(f);
		cmd->set_param(0, n);
		func->block->params[0] = cmd;
	}
	for (auto *c: tree->owned_classes)
		parser->auto_implementer.auto_implement_functions(c);
	//ps->show("aaaa");


	if (config.verbose)
		tree->show("parse:a");

// compile
	s->compile();


	if (config.interpreted) {
		s->interpreter->run("--command-func--");
		return;
	}

// execute
	if (config.abi == config.native_abi) {
		typedef void void_func();
		void_func *f = (void_func*)func->address;
		if (f)
			f();
	}
}



const Class *_dyn_type_in_namespace(const VirtualTable *p, const Class *ns) {
	for (auto *c: weak(ns->classes)) {
		if (c->_vtable_location_target_ == p)
			return c;
		auto t = _dyn_type_in_namespace(p, c);
		if (t)
			return t;
	}
	return nullptr;
}

// TODO...namespace
const Class *Context::get_dynamic_type(const VirtualBase *p) const {
	auto *pp = get_vtable(p);
	for (auto s: public_modules) {
		auto t = _dyn_type_in_namespace(pp, s->syntax->base_class);
		if (t)
			return t;
	}
	return nullptr;
}



void Context::clean_up() {
	//delete_all_modules(true, true);

	packages.clear();

	external->reset();
}

extern Context *_secret_lib_context_;

Context *Context::create() {
	auto c = new Context;
	c->packages = _secret_lib_context_->packages;
	c->type_casts = _secret_lib_context_->type_casts;
	//c->external = _secret_lib_context_->external;
	c->template_manager->copy_from(_secret_lib_context_->template_manager.get());
	c->implicit_class_registry->copy_from(_secret_lib_context_->implicit_class_registry.get());
	return c;
}


}
