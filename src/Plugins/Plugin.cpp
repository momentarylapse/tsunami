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

string Plugin::GetError()
{
	return format(_("Fehler in  Script-Datei: \"%s\"\n%s"), filename.c_str(), error_message.c_str());
}
