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

namespace kaba {

class Module;
class Function;
class Class;
class TypeCast;
class TemplateManager;
class ImplicitClassRegistry;
class ExternalLinkData;

class Exception : public Asm::Exception {
public:
	Exception(const string &message, const string &expression, int line, int column, Module *s);
	Exception(const Asm::Exception &e, Module *s, Function *f);
	string message() const override;
	//Module *module;
	Path filename;
};
/*struct SyntaxException : Exception{};
struct LinkerException : Exception{};
struct LinkerException : Exception{};*/

class Context {
public:
    shared_array<Module> public_modules;
    shared_array<Module> packages;
    Array<TypeCast> type_casts;
    owned<TemplateManager> template_manager;
    owned<ImplicitClassRegistry> implicit_class_registry;
    owned<ExternalLinkData> external;

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
