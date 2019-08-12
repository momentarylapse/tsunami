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


Plugin::Plugin(const string &_filename, ModuleType _type) {
	s = nullptr;
	type = _type;
	filename = _filename;
	index = -1;

	file_date = -1;
}

string Plugin::get_error() {
	return format(_("Error in script file: \"%s\"\n%s"), filename.c_str(), error_message.c_str());
}

bool Plugin::file_changed() {
	int new_date = -1;
	try {
		File *f = FileOpen(filename);
		new_date = f->GetDateModification().time;
		if (new_date != file_date) {
			file_date = new_date;
			return true;
		}
	} catch(...) {}
	return false;

}

void Plugin::recompile(Session *session) {
	session->i(_("compiling script: ") + filename);

	if (s) {
		Kaba::Remove(s);
		s = nullptr;
	}

	// load + compile
	try {
		s = Kaba::Load(filename);

	} catch(Kaba::Exception &e) {
		error_message = e.message();
		session->e(get_error());
	}
}

bool Plugin::usable(Session *session) {
	if (file_changed())
		recompile(session);
	return s;
}

void *Plugin::create_instance(Session *session, const string &root_type) {
	if (!usable(session))
		return nullptr;

	for (auto *t : s->syntax->base_class->classes) {
		if (t->is_derived_from_s(root_type)) {
			return t->create_instance();
		}
	}
	session->e(format(_("Script file \"%s\" does not define a class derived from %s"), filename.c_str(), root_type.c_str()));
	return nullptr;
}
