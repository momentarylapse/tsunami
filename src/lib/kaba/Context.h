/*
 * Context.h
 *
 *  Created on: Nov 15, 2022
 *      Author: michi
 */

#pragma once

#include "../base/base.h"
#include "../base/pointer.h"
#include "../os/path.h"
#include "asm/asm.h"
#include "../kapi/kapi.h"
#include <functional>

namespace kaba {

class Module;
class Function;
class Class;
class TypeCast;
class Operator;
class TemplateManager;
class ExternalLinkData;
class Exporter;
class IExporter;

class Exception : public Asm::Exception {
public:
	Exception(const string &message, const string &expression, int line, int column, Module *s);
	Exception(const Asm::Exception &e, Module *s, Function *f);
	Exception(const Exception& e);
	string message() const override;
	//Module *module;
	Path filename;

	owned<Exception> parent;
};
/*struct SyntaxException : Exception{};
struct LinkerException : Exception{};
struct LinkerException : Exception{};*/

struct Package : Sharable<base::Empty> {
	Package(const string& name, const string& version, const Path& directory);
	string name;
	string version;
	Path directory;
	Path directory_dynamic;
	bool is_installed;
	bool is_internal;
	owned<ExternalLinkData> external;
	shared<Module> main_module;
	bool auto_import = false;
	Path default_directory() const;
};

class Context : public IContext {
public:
	Array<TypeCast> type_casts;
	owned<TemplateManager> template_manager;
	owned<ExternalLinkData> external;

	shared_array<Operator> global_operators;

	struct PackageInit {
		string name;
		Path dir;
		std::function<void(IExporter*)> f;
	};
	Array<PackageInit> package_inits;
	void register_package_init(const string& name, const Path& dir, std::function<void(IExporter*)> f) override;

	Context();
	~Context() override;

	void clean_up() override;


	// careful: can not be used through dll
	shared<Module> load_module(const Path& filename, bool just_analyse) override;
	shared<Module> create_module_for_source(const string& source, const Path& filename, bool just_analyse) override;
	shared<Module> create_empty_module(const Path& filename);
	//void remove_module(Module *s);

	void execute_single_command(const string& cmd) override;
	xfer<Context> create_new_context() const override;

	const Class* get_dynamic_type(const VirtualBase* p) const override;

	// internal or already loaded
	Package* get_package(const string& name) const override;

	string type_name(const Class* c) const override;
	Any dynify(const void* p, const Class* type) const override;
	void unwrap_any(const Any &aa, void *var, const Class *type) const override;

	Array<string> list_keywords() const override;
	Array<string> list_modifiers() const override;
	Array<string> list_special_functions() const override;
	Array<string> list_operator_functions() const override;

	static xfer<Context> create();
	static Path installation_root();
	static Path packages_root();
};

void make_context_public(IExporter* e);

}
