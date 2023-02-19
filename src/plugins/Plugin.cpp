/*
 * Plugin.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Plugin.h"
#include "../Session.h"
#include "../lib/kaba/kaba.h"
#include "../lib/hui/hui.h"
#include "../lib/os/filesystem.h"
#include "../lib/os/date.h"


Plugin::Plugin(const Path &_filename, ModuleCategory _type) {
	type = _type;
	filename = _filename;
	index = -1;

	file_date = -1;
}

string Plugin::get_error() {
	return format(_("Error in file: \"%s\"\n%s"), filename, error_message);
}

bool Plugin::file_changed() {
	int new_date = -1;
	try {
		new_date = os::fs::mtime(filename).time;
		if (new_date != file_date) {
			file_date = new_date;
			return true;
		}
	} catch(...) {}
	return false;

}

void Plugin::recompile(Session *session) {
	session->i(_("compiling module: ") + filename.str());

	if (module) {
		session->w(_("recompiling modules currently not supported"));
		return;
		#if 0
		kaba::default_context->remove_module(module.get());
		module = nullptr;
		#endif
	}

	// load + compile
	try {
		module = kaba::default_context->load_module(filename);
	} catch(kaba::Exception &e) {
		error_message = e.message();
		session->e(get_error());
	}
}

bool Plugin::usable(Session *session) {
	if (file_changed())
		recompile(session);
	return module.get();
}

void *Plugin::create_instance(Session *session, const string &root_type) {
	if (!usable(session))
		return nullptr;

	for (auto *t: weak(module->tree->base_class->classes)) {
		if (t->is_derived_from_s(root_type)) {
			return t->create_instance();
		}
	}
	session->e(format(_("Plugin file \"%s\" does not define a class derived from %s"), filename, root_type));
	return nullptr;
}
