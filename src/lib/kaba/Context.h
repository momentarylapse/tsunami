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

class Context {
public:
	shared_array<Module> public_modules;
	shared_array<Package> internal_packages;
	Array<TypeCast> type_casts;
	owned<TemplateManager> template_manager;
	owned<ExternalLinkData> external;

	shared_array<Operator> global_operators;

	owned_array<Package> external_packages;

	struct PackageInit {
		string name;
		Path dir;
		std::function<void(Exporter*)> f;
	};
	Array<PackageInit> package_inits;
	void register_package_init(const string& name, const Path& dir, std::function<void(Exporter*)> f);

	Context();
	~Context();

	void clean_up();


	// careful: can not be used through dll
	shared<Module> load_module(const Path& filename, bool just_analyse = false);
	shared<Module> create_module_for_source(const string& source, bool just_analyse = false);
	shared<Module> create_empty_module(const Path& filename);
	//void remove_module(Module *s);

	void execute_single_command(const string& cmd);

	const Class* get_dynamic_type(const VirtualBase* p) const;

	// dll API (experimental!)
	shared<Module> dll_load_module(const Path& filename, bool just_analyse = false);
	shared<Module> dll_create_module_for_source(const string& source, bool just_analyse = false);
	xfer<Context> dll_create_context() const;
	void dll_execute_single_command(const string& cmd);
	std::function<shared<Module>(Context*, const Path&, bool)> f_load_module;
	std::function<shared<Module>(Context*, const string&, bool)> f_create_module_for_source;
	std::function<void(Context*, const string&)> f_execute_single_command;
	std::function<xfer<Context>()> f_create_new_context;

	// internal or already loaded
	Package* get_package(const string& name) const;

	static xfer<Context> create();
	static Path installation_root();
	static Path packages_root();
};

extern Context *default_context;

}
