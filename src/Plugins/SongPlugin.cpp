/*
 * SongPlugin.cpp
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#include "SongPlugin.h"
#include "../Session.h"
#include "PluginManager.h"
#include "Plugin.h"
#include "../Module/Module.h"

SongPlugin::SongPlugin()
{
	session = nullptr;
	song = nullptr;
}

SongPlugin::~SongPlugin()
{
}

void SongPlugin::__init__()
{
	new(this) SongPlugin;
}

void SongPlugin::__delete__()
{
	this->SongPlugin::~SongPlugin();
}



SongPlugin *CreateSongPlugin(Session *session, const string &name)
{
	Plugin *p = session->plugin_manager->get_plugin(session, ModuleType::SONG_PLUGIN, name);
	SongPlugin *sp = (SongPlugin*)p->create_instance(session, "SongPlugin");

	// dummy?
	if (!sp)
		sp = new SongPlugin;

	sp->session = session;
	sp->song = session->song;
	/*sp->name = name;
	sp->plugin = p;
	sp->usable = p->usable;
	sp->song = song;
	sp->resetConfig();*/
	return sp;
}
