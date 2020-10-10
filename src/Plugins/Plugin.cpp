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


Plugin::Plugin(const Path &_filename, ModuleType _type) {
	type = _type;
	filename = _filename;
	index = -1;

	file_date = -1;
}

string Plugin::get_error() {
	return format(_("Error in script file: \"%s\"\n%s"), filename, error_message);
}

bool Plugin::file_changed() {
	int new_date = -1;
	try {
		new_date = file_mtime(filename).time;
		if (new_date != file_date) {
			file_date = new_date;
			return true;
		}
	} catch(...) {}
	return false;

}

void Plugin::recompile(Session *session) {
	session->i(_("compiling script: ") + filename.str());

	if (s) {
		Kaba::remove_script(s.get());
		s = nullptr;
	}

	// load + compile
	try {
		s = Kaba::load(filename);

	} catch(Kaba::Exception &e) {
		error_message = e.message();
		session->e(get_error());
	}
}

bool Plugin::usable(Session *session) {
	if (file_changed())
		recompile(session);
	return s.get();
}

void *Plugin::create_instance(Session *session, const string &root_type) {
	if (!usable(session))
		return nullptr;

	for (auto *t: weak(s->syntax->base_class->classes)) {
		if (t->is_derived_from_s(root_type)) {
			return t->create_instance();
		}
	}
	session->e(format(_("Script file \"%s\" does not define a class derived from %s"), filename, root_type));
	return nullptr;
}
