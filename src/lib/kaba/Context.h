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

struct Package {
	string name;
	Path directory;
	owned<ExternalLinkData> external;
};

class Context {
public:
    shared_array<Module> public_modules;
    shared_array<Module> internal_packages; // TODO Package[]
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

    void __delete__();

    void clean_up();


    shared<Module> load_module(const Path &filename, bool just_analyse = false);
    shared<Module> create_module_for_source(const string &buffer, bool just_analyse = false);
    shared<Module> create_empty_module(const Path &filename);
    //void remove_module(Module *s);
    
    void execute_single_command(const string &cmd);

    const Class *get_dynamic_type(const VirtualBase *p) const;

    static xfer<Context> create();
};

extern Context *default_context;

}
