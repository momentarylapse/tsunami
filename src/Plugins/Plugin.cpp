/*
 * Plugin.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
#include "PluginManager.h"
#include "Effect.h"
#include "../Stuff/Log.h"


Plugin::Plugin(const string &_filename)
{
	s = NULL;
	filename = _filename;
	index = -1;
	usable = false;

	// load + compile
	try{
		s = Script::Load(filename);

		usable = true;

		type = TYPE_OTHER;//f_process_track ? TYPE_EFFECT : TYPE_OTHER;
	}catch(Script::Exception &e){
		error_message = e.message;
	}
}

string Plugin::getError()
{
	return format(_("Error in script file: \"%s\"\n%s"), filename.c_str(), error_message.c_str());
}

void *Plugin::createInstance(const string &root_type)
{
	if (!usable)
		return NULL;

	for (auto *t : s->syntax->types){
		if (t->IsDerivedFrom(root_type))
			return t->CreateInstance();
	}
	tsunami->log->error(format(_("Script file \"%s\" does not define a class derived from %s"), filename.c_str(), root_type.c_str()));
	return NULL;
}
