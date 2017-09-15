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
#include "Plugin.h"

const string TsunamiPlugin::MESSAGE_STOP_REQUEST = "StopRequest";

TsunamiPlugin::TsunamiPlugin()
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
	Plugin *p = tsunami->plugin_manager->GetPlugin(Plugin::TYPE_TSUNAMI_PLUGIN, name);
	TsunamiPlugin *t = NULL;

	if (p->usable)
		t = (TsunamiPlugin*)p->createInstance("TsunamiPlugin");

	// dummy?
	if (!t)
		t = new TsunamiPlugin;

	t->name = name;
	t->win = win;
	t->song = NULL;
	t->view = NULL;
	if (win){
		t->song = win->song;
		t->view = win->view;
	}
	/*t->plugin = p;
	t->usable = p->usable;
	t->resetConfig();*/
	return t;
}
