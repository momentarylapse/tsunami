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


Plugin::Plugin(const string &_filename, int _type)
{
	s = NULL;
	type = _type;
	filename = _filename;
	index = -1;
	usable = false;

	// load + compile
	try{
		s = Kaba::Load(filename);

		usable = true;
	}catch(Kaba::Exception &e){
		error_message = e.message;
	}
}

string Plugin::get_error()
{
	return format(_("Error in script file: \"%s\"\n%s"), filename.c_str(), error_message.c_str());
}

void *Plugin::create_instance(Session *session, const string &root_type)
{
	if (!usable)
		return NULL;

	for (auto *t : s->syntax->classes){
		if (t->is_derived_from(root_type))
			return t->create_instance();
	}
	session->e(format(_("Script file \"%s\" does not define a class derived from %s"), filename.c_str(), root_type.c_str()));
	return NULL;
}
