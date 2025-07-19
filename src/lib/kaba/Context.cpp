#include "Context.h"
#include "kaba.h"
#include "Interpreter.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "parser/Parser.h"
#include "parser/Concretifier.h"
#include "template/template.h"
#include "compiler/Compiler.h"
#include "../os/msg.h"
#if HAS_LIB_DL
#include <dlfcn.h>
#endif

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

Exception::Exception(const Exception& e) :
	Asm::Exception(e.text, e.expression, e.line, e.column)
{
	filename = e.filename;
	if (e.parent.get())
		parent = new Exception(*e.parent.get());
}

string Exception::message() const {
	string m;
	if (expression != "")
		m += format("\"%s\": ", expression);
	m += text;

	auto location = [] (const Exception &e) {
		if (e.line >= 0)
			return format("%s, line %d", e.filename, e.line + 1);
		return str(e.filename);
	};

	Array<string> locations;
	locations.add(location(*this));

	auto ee = parent.get();
	while (ee) {
		locations.add(location(*ee));
		ee = ee->parent.get();
	}
	locations.reverse();
	return m + "\nat: " + implode(locations, "\nimported at: ");
}




Path absolute_module_path(const Path &filename) {
	if (filename.is_relative())
		return (config.directory | filename).absolute().canonical();
	else
		return filename.absolute().canonical();
}

Context::Context() {
	template_manager = new TemplateManager(this);
	external = new ExternalLinkData(this);
}

Context::~Context() {
    clean_up();
}

void Context::__delete__() {
	this->Context::~Context();
}

void try_import_dynamic_library_for_module(const Path& filename, Context* ctx, shared<Module> module) {
#if HAS_LIB_DL
	const auto dir = filename.parent();
#ifdef OS_MAC
	auto files = os::fs::search(dir, "lib*.dylib", "f");
#elif defined(OS_LINUX)
	auto files = os::fs::search(dir, "lib*.so", "f");
#else
	auto files = os::fs::search(dir, "lib*.dll", "f");
#endif

	if (files.num == 1) {
		Exporter e(ctx, module.get());
		auto handle = dlopen((dir | files[0]).c_str(), RTLD_NOW|RTLD_LOCAL);
		typedef void t_f(Exporter*);
		if (auto f = (t_f*)dlsym(handle, "export_symbols")) {
			(*f)(&e);
		} else {
			msg_error(format("found dynamic library %s, but no 'export_symbols()'", dir | files[0]));
		//	s->do_error_link("can't load symbol '" + name + "' from library " + libname);
		}
#endif
	}
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
	if (!just_analyse)
		try_import_dynamic_library_for_module(filename, this, s);
	s->load(filename, just_analyse);

	// store module in database
	public_modules.add(s);
	return s;
}

shared<Module> Context::create_module_for_source(const string &buffer, bool just_analyse) {
    auto s = create_empty_module("<from-source>");
	s->just_analyse = just_analyse;
	s->filename = config.default_filename;
	s->tree->parser = new Parser(s->tree.get());
	s->tree->default_import();
	s->tree->parser->parse_buffer(buffer, just_analyse);

	if (!just_analyse)
		Compiler::compile(s.get());

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
	auto tree = s->tree.get();
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
			tree->import_data_selective(p->base_class(), nullptr, nullptr, nullptr, str(p->filename), -1);

// analyse syntax

	// create a main() function
	Function *func = tree->add_function("--command-func--", TypeVoid, tree->base_class, Flags::Static);
	func->_var_size = 0; // set to -1...

	parser->Exp.reset_walker();

	// parse
	func->block->type = TypeUnknown;
	parser->parse_abstract_complete_command_into_block(func->block.get());
	if (config.verbose) {
		msg_write("ABSTRACT SINGLE:");
		func->block->show();
	}
	parser->con.concretify_node(func->block.get(), func->block.get(), func->name_space);

	if (func->block->params.num == 0)
		return;

	auto node = func->block->params[0];
	
	// implicit print(...)?
	if (node->type != TypeVoid) {
		auto n_str = parser->con.add_converter_str(node, true);
		auto f_print = tree->required_func_global("print");

		auto cmd = add_node_call(f_print);
		cmd->set_param(0, n_str);
		func->block->params[0] = cmd;
	}
	//for (auto *c: tree->owned_classes)
	for (int i=0; i<tree->owned_classes.num; i++) // array might change...
		parser->auto_implementer.implement_functions(tree->owned_classes[i]);
	//ps->show("aaaa");


	if (config.verbose)
		tree->show("parse:a");

// compile
	Compiler::compile(s.get());


	if (config.target.interpreted) {
		s->interpreter->run("--command-func--");
		return;
	}

// execute
	if (config.target.is_native) {
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
		if (auto t = _dyn_type_in_namespace(pp, s->tree->base_class))
			return t;
	}
	return nullptr;
}



void Context::clean_up() {
	global_operators.clear();
	public_modules.clear();
	packages.clear();
	external->reset();
}

extern Context *_secret_lib_context_;

xfer<Context> Context::create() {
	auto c = new Context;
	c->packages = _secret_lib_context_->packages;
	c->type_casts = _secret_lib_context_->type_casts;
	//c->external = _secret_lib_context_->external;
	c->template_manager->copy_from(_secret_lib_context_->template_manager.get());
	c->global_operators = _secret_lib_context_->global_operators;
	return c;
}


}
