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
	SongPlugin *p = tsunami->plugin_manager->LoadSongPlugin(name);
	if (!p)
		return NULL;

	p->win = win;
	p->view = win->view;
	return p;
}
