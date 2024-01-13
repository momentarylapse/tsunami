/*
 * import.cpp
 *
 *  Created on: 29 Mar 2022
 *      Author: michi
 */

#include "../kaba.h"
#include "Parser.h"
#include "import.h"
#include "../../os/filesystem.h"
#include "../../os/msg.h"


#include "../../config.h"

#ifdef _X_USE_HUI_
#include "../../hui/Application.h"
#elif defined(_X_USE_HUI_MINIMAL_)
#include "../../hui_minimal/Application.h"
#endif


const int MAX_IMPORT_DIRECTORY_PARENTS = 5;

namespace kaba {

extern Array<shared<Module>> loading_module_stack;
void SetImmortal(SyntaxTree *ps);

string canonical_import_name(const string &s) {
	return s.lower().replace(" ", "").replace("_", "").replace("-", "");
}

string dir_has(const Path &dir, const string &name) {
	auto list = os::fs::search(dir, "*", "fd");
	for (auto &e: list)
		if (canonical_import_name(e.str()) == name)
			return e.str();
	return "";
}

Path import_dir_match(const Path &dir0, const string &name) {
	auto xx = name.replace(".kaba", "").explode("/");
	Path filename = dir0;

	// parents matching?
	for (int i=0; i<xx.num-1; i++) {
		string e = dir_has(filename, canonical_import_name(xx[i]));
		if (e == "")
			return Path::EMPTY;
		filename |= e;
	}
	{
		// direct file  zzz.kaba?
		string e = dir_has(filename, canonical_import_name(xx.back() + ".kaba"));
		if (e != "") {
			filename |= e;
			return filename;
		}

		// package  zzz/zzz.kaba  or  zzz/main.kaba?
		e = dir_has(filename, canonical_import_name(xx.back()));
		if (e == "")
			return Path::EMPTY;
		filename |= e;
		if (os::fs::exists(filename | (xx.back() + ".kaba")))
			return filename | (xx.back() + ".kaba");
		if (os::fs::exists(filename | "main.kaba"))
			return filename | "main.kaba";
		return Path::EMPTY;
	}
	return filename;

	if (os::fs::exists(dir0 | name))
		return dir0 | name;
	return Path::EMPTY;
}

Path find_installed_lib_import(const string &name) {
	Path kaba_dir = hui::Application::directory.parent() | "kaba";
	if (hui::Application::directory.basename()[0] == '.')
		kaba_dir = hui::Application::directory.parent() | ".kaba";
	Path kaba_dir_static = hui::Application::directory_static.parent() | "kaba";
	for (auto &dir: Array<Path>({kaba_dir, kaba_dir_static})) {
		auto path = (dir | "lib" | name).canonical();
		if (os::fs::exists(path))
			return path;
	}
	return Path::EMPTY;
}

Path find_import(Module *s, const string &_name) {
	string name = _name.replace(".kaba", "");
	name = name.replace(".", "/") + ".kaba";

	// deprecated...
	if (name.head(2) == "@/")
		return find_installed_lib_import(name.sub(2));

	for (int i=0; i<MAX_IMPORT_DIRECTORY_PARENTS; i++) {
		Path filename = import_dir_match((s->filename.parent() | string("../").repeat(i)).canonical(), name);
		if (filename)
			return filename;
	}

	// installed?
	return find_installed_lib_import(name);

	return Path::EMPTY;
}

shared<Module> get_import_module(Parser *parser, const string &name, int token_id) {

	// internal packages?
	for (auto p: parser->context->packages)
		if (p->filename.str() == name)
			return p;

	Path filename = find_import(parser->tree->module, name);
	if (!filename)
		return nullptr;
		//parser->do_error(format("can not find import '%s'", name), token_id);

	for (auto ss: weak(loading_module_stack))
		if (ss->filename == filename)
			parser->do_error("recursive import", token_id);

	msg_right();
	shared<Module> include;
	try {
		include = parser->context->load_module(filename, parser->tree->module->just_analyse or config.fully_linear_output);
		// os-includes will be appended to syntax_tree... so don't compile yet
	} catch (kaba::Exception &e) {
		msg_left();

		auto p = new Exception(e);
		e.line = parser->Exp.token_physical_line_no(token_id);
		e.column = parser->Exp.token_line_offset(token_id);
		e.filename = parser->tree->module->filename;
		e.parent = p;
		throw e;
	}

	msg_left();
	return include;
}


ImportSource resolve_import_sub(ImportSource source, const string &name) {

	ImportSource r = source;
	if (source._class) {
		for (auto c: weak(source._class->classes))
			if (c->name == name) {
				r._class = c;
				return r;
			}
		for (auto f: weak(source._class->functions))
			if (name == f->name) {
				r.func = f;
				r._class = nullptr;
				return r;
			}
		for (auto v: weak(source._class->static_variables))
			if (name == v->name) {
				r.var = v;
				r._class = nullptr;
				return r;
			}
		for (auto c: weak(source._class->constants))
			if (name == c->name) {
				r._const = c;
				r._class = nullptr;
				return r;
			}
	}
	return {};
}

ImportSource resolve_import_source(Parser *parser, const Array<string> &name, int token) {
	ImportSource source;

	// find (longest possible) module path
	int i_module = -1;
	for (int i=name.num-1; i>=0; i--) {
		if (auto m = get_import_module(parser, implode(name.sub_ref(0, i+1), "."), token)) {
			source.module = m;
			source._class = m->base_class();
			i_module = i;
			break;
		}
	}
	if (!source.module)
		parser->do_error(format("can not find import '%s'", implode(name, ".")), token);

	for (int i=i_module+1; i<name.num; i++) {
		if (source._class) {
			source = resolve_import_sub(source, name[i]);
			if (!source.module)
				parser->do_error(format("can not use '%s' from module '%s'",
						implode(name.sub_ref(i_module, i), "."),
						implode(name.sub_ref(0, i_module), ".")), token);
		} else {
			parser->do_error(format("can not use '%s' from non-class '%s'", name[i],
					implode(name.sub_ref(0, i), ".")), token);
		}
	}
	return source;
}

[[maybe_unused]] static bool _class_contains(const Class *c, const string &name) {
	for (auto *cc: weak(c->classes))
		if (cc->name == name)
			return true;
	for (auto *f: weak(c->functions))
		if (f->name == name)
			return true;
	for (auto *cc: weak(c->constants))
		if (cc->name == name)
			return true;
	return false;
}

void namespace_import_contents(SyntaxTree *tree, Scope &dest, const Class *source, int token_id) {
	auto check = [tree, token_id, source] (bool ok, const string &name) {
		if (!ok)
			tree->do_error(format("can not import class '%s' since symbol '%s' is already in scope", source->long_name(), name));
	};
	for (auto *c: weak(source->classes))
		check(dest.add_class(c->name, c), c->name);
	for (auto *f: weak(source->functions))
		check(dest.add_function(f->name, f), f->name);
	for (auto *v: weak(source->static_variables))
		check(dest.add_variable(v->name, v), v->name);
	for (auto *c: weak(source->constants))
		if (c->name.head(1) != "-")
			check(dest.add_const(c->name, c), c->name);
}

void general_import(SyntaxTree *me, SyntaxTree *source) {
	for (auto i: weak(me->includes))
		if (i->tree == source)
			return;

	// propagate immortality TO the (dependent) source!
	//  (might be unnecessary due to shared pointers)
	if (me->flag_immortal)
		SetImmortal(source);

	me->flag_string_const_as_cstring |= source->flag_string_const_as_cstring;


	me->includes.add(source->module);
}

void SyntaxTree::import_data_all(const Class *source, int token_id) {
	general_import(this, source->owner);
	namespace_import_contents(this, global_scope, source, token_id);


	// hack: package auto import
	for (auto c: weak(source->constants))
		if (c->name == "EXPORT_IMPORTS") {
			for (auto i: weak(source->owner->includes))
				if (!i->is_system_module())
					import_data_all(i->base_class(), token_id);
		}
}

void SyntaxTree::import_data_selective(const Class *cl, const Function *f, const Variable *v, const Constant *cn, const string &as_name, int token_id) {
	if (cl) {
		general_import(this, cl->owner);
		if (global_scope.add_class(as_name, cl))
			return;
	} else if (f) {
		general_import(this, f->owner());
		if (global_scope.add_function(as_name, f))
			return;
	} else if (v) {
		//general_import(this, v->);
		if (global_scope.add_variable(as_name, v))
			return;
	} else if (cn) {
		general_import(this, cn->owner);
		if (global_scope.add_const(as_name, cn))
			return;
	}
	do_error(format("symbol '%s' already in scope", as_name), token_id);
}

#if 0
// import data from an included module file
void SyntaxTree::import_data(shared<Module> source, const Class *source, bool directly_import_contents, const string &as_name) {
	for (auto i: weak(includes))
		if (i == source)
			return;

	SyntaxTree *ps = source->tree.get();

	// propagate immortality TO the (dependent) source!
	//  (might be unnecessary due to shared pointers)
	if (flag_immortal)
		SetImmortal(ps);

	flag_string_const_as_cstring |= ps->flag_string_const_as_cstring;


	/*if (FlagCompileOS) {
		import_deep(this, ps);
	} else {*/
	if (!directly_import_contents) {
		// "use aaa.bbb"
		namespace_import_contents(imported_symbols.get(), ps->base_class);
	} else {
		// "use aaa.bbb.*"
		namespace_import_contents(imported_symbols.get(), ps->base_class);
		if (source->is_system_module())
			if (!_class_contains(imported_symbols.get(), ps->base_class->name)) {
				imported_symbols->classes.add(ps->base_class);
			}

		// hack: package auto import
		for (auto c: weak(ps->base_class->constants))
			if (c->name == "EXPORT_IMPORTS") {
				for (auto i: weak(ps->includes))
					if (!i->is_system_module())
						import_data(i, directly_import_contents, "");
			}
	}
	includes.add(source);
	//}
}
#endif


}
