/*
 * TsunamiPlugin.cpp
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#include "TsunamiPlugin.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "PluginManager.h"

const string TsunamiPlugin::MESSAGE_STOP_REQUEST = "StopRequest";

TsunamiPlugin::TsunamiPlugin() :
	Observable("TsunamiPlugin")
{
	win = NULL;
	view = NULL;
	song = NULL;
	active = false;
}

TsunamiPlugin::~TsunamiPlugin()
{
}

void TsunamiPlugin::__init__()
{
	new(this) TsunamiPlugin;
}

void TsunamiPlugin::__delete__()
{
	this->TsunamiPlugin::~TsunamiPlugin();
}

void TsunamiPlugin::stop_request()
{
	notify(MESSAGE_STOP_REQUEST);
}

void TsunamiPlugin::start()
{
	if (active)
		return;
	onStart();
	active = true;
}

void TsunamiPlugin::stop()
{
	if (!active)
		return;
	onStop();
	active = false;
}



TsunamiPlugin *CreateTsunamiPlugin(const string &name, TsunamiWindow *win)
{
	TsunamiPlugin *p = tsunami->plugin_manager->LoadTsunamiPlugin(name);
	p->name = name;
	p->win = win;
	p->song = win->song;
	p->view = win->view;
	return p;
}
