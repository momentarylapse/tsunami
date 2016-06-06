/*
 * SongPlugin.cpp
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#include "SongPlugin.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "PluginManager.h"
#include "Plugin.h"

SongPlugin::SongPlugin()
{
	win = NULL;
	view = NULL;
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



SongPlugin *CreateSongPlugin(const string &name, TsunamiWindow *win)
{
	Plugin *p = tsunami->plugin_manager->GetPlugin(Plugin::TYPE_SONG_PLUGIN, name);
	SongPlugin *sp = NULL;
	if (p->usable)
		sp = (SongPlugin*)p->createInstance("SongPlugin");

	// dummy?
	if (!sp)
		sp = new SongPlugin;

	sp->win = win;
	sp->view = win->view;
	/*sp->name = name;
	sp->plugin = p;
	sp->usable = p->usable;
	sp->song = song;
	sp->resetConfig();*/
	return sp;
}
