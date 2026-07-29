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
#include "../../os/app.h"


const int MAX_IMPORT_DIRECTORY_PARENTS = 5;

namespace kaba {

extern Array<shared<Module>> loading_module_stack;
void SetImmortal(SyntaxTree *ps);

/*string canonical_import_name(const string &s) {
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
}*/

Path check_package_dir(const Path& dir, const string& package_name, const string& name) {
	// msg_write(format("CHECK %s  %s", dir, name));
	auto xx = name.explode("/");
	string name2 = name;
	if (xx.num >= 2 and xx[0] == package_name)
		name2 = implode(xx.sub_ref(1), "/");

	const auto fn = dir | (name2 + ".kaba");
	if (os::fs::exists(fn))
		return fn;
	return Path::EMPTY;
}

Path find_installed_package_import(Context* ctx, string& package_name, const string &name) {

	// packages provided by host program
	for (auto& i: ctx->package_inits)
		if (i.name == package_name)
			if (auto fn = check_package_dir(i.dir, package_name, name))
				return fn;

	// system wide install
	if (auto fn = check_package_dir(ctx->packages_root() | package_name, package_name, name))
		return fn;

	return Path::EMPTY;
}

Path find_import_module_file(Module *s, const string &_name) {
	string name = _name.replace(".kaba", "");
	name = name.replace(".", "/");

	if (name.head(1) == "/") {
		// relative....

		// count leading /s
		int n = 0;
		while (n < name.num and name[n] == '/')
			n++;

		Path filename = (s->filename.parent() | string("../").repeat(n-1)).canonical() | (name.sub(n) + ".kaba");
		if (os::fs::exists(filename))
			return filename;
	} else if (name.num > 0) {
		// "absolute"...

		string wanted_package_name = name.explode("/")[0];

		// already loaded package
		for (auto p: s->context->external_packages)
			if (p->name == wanted_package_name)
				if (auto fn = check_package_dir(p->directory, p->name, name))
					return fn;

		// installed?
		if (auto fn = find_installed_package_import(s->context, wanted_package_name, name))
			return fn;

		// fallback: local/relative path
		//  useful for editors finding not-installed packages
		for (int i=0; i<MAX_IMPORT_DIRECTORY_PARENTS; i++) {
			const auto dir = (s->filename.parent() | string("../").repeat(i)).canonical() | wanted_package_name;
			if (os::fs::exists(dir))
				if (auto fn = check_package_dir(dir, wanted_package_name, name))
					return fn;
		}
	}
	return Path::EMPTY;
}

shared<Module> get_import_module(Parser* parser, const string& name, int token_id) {

	// internal packages?
	for (auto p: weak(parser->context->internal_packages))
		if (p->main_module->filename.str() == name)
			return p->main_module;

	Path filename = find_import_module_file(parser->tree->module, name);
	if (!filename)
		return nullptr;
		//parser->do_error(format("can not find import '%s'", name), token_id);

	for (auto ss: weak(loading_module_stack))
		if (ss->filename == filename)
			parser->do_error("recursive import", token_id);

	msg_right();
	shared<Module> include;
	try {
		include = parser->context->_load_module_throw(filename, parser->tree->module->just_analyse or config.fully_linear_output);
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
		for (const auto&& [_name, type]: source._class->type_aliases)
			if (_name == name) {
				r._class = type;
				return r;
			}
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
			source.is_scope = true;
			i_module = i;
			break;
		}
	}
	if (!source.module)
		parser->do_error(format("can not find import '%s'", implode(name, ".")), token);

	// symbol in module...
	for (int i=i_module+1; i<name.num; i++) {
		if (source.is_scope) {
			source._class = source.module->base_class();
			source.is_scope = false;
		}
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
	for (const auto&& [_name, type]: c->type_aliases)
		if (_name == name)
			return true;
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
	auto check = [tree, source] (bool ok, const string &name) {
		if (!ok)
			tree->do_error(format("can not import class '%s' since symbol '%s' is already in scope", source->long_name(), name));
	};
	auto filter = [] (const string& name) {
		if (/*name.head(1) == "_" or*/ name.head(1) == "-") // FIXME  _int_out() _print_postfix_ etc... :P
			return false;
		if (name == "EXPORT_IMPORTS") // :P
			return false;
		return true;
	};
	for (const auto&& [name, type]: source->type_aliases)
		if (filter(name))
			check(dest.add_class(name, type), name);
	for (auto c: weak(source->classes))
		if (filter(c->name))
			check(dest.add_class(c->name, c), c->name);
	for (auto f: weak(source->functions))
		if (filter(f->name))
			check(dest.add_function(f->name, f), f->name);
	for (auto v: weak(source->static_variables))
		if (filter(v->name))
			check(dest.add_variable(v->name, v), v->name);
	for (auto c: weak(source->constants))
		if (filter(c->name))
			check(dest.add_const(c->name, c), c->name);
}

void import_flags(SyntaxTree *me, SyntaxTree *source) {
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

void SyntaxTree::import_data_all(const ImportSource& source, int token_id, bool also_export) {
	import_flags(this, source.module->tree.get());
	auto source_class = source._class;
	if (source.is_scope) {
		source_class = source.module->tree->base_class;
		global_scope.entries.append(source.module->tree->import_export_scope.entries);
		if (also_export)
			import_export_scope.entries.append(source.module->tree->import_export_scope.entries);
	}
	namespace_import_contents(this, global_scope, source_class, token_id);
	if (also_export)
		namespace_import_contents(this, import_export_scope, source_class, token_id);
}

void SyntaxTree::import_data_single_item(const ImportSource& source, const string &as_name, int token_id, bool also_export) {
	if (source.is_scope) {
		import_flags(this, source.module->tree.get());
		if (also_export)
			import_export_scope.add_module(as_name, source.module.get());
		if (global_scope.add_module(as_name, source.module.get()))
			return;
	} else if (source._class) {
		import_flags(this, source._class->owner);
		if (also_export)
			import_export_scope.add_class(as_name, source._class);
		if (global_scope.add_class(as_name, source._class))
			return;
	} else if (source.func) {
		import_flags(this, source.func->owner());
		if (also_export)
			import_export_scope.add_function(as_name, source.func);
		if (global_scope.add_function(as_name, source.func))
			return;
	} else if (source.var) {
		//general_import(this, v->);
		if (also_export)
			import_export_scope.add_variable(as_name, source.var);
		if (global_scope.add_variable(as_name, source.var))
			return;
	} else if (source._const) {
		import_flags(this, source._const->owner);
		if (also_export)
			import_export_scope.add_const(as_name, source._const);
		if (global_scope.add_const(as_name, source._const))
			return;
	}
	do_error(format("symbol '%s' already in scope", as_name), token_id);
}


}
