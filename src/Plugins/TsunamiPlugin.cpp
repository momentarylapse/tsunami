/*
 * TsunamiPlugin.cpp
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#include "TsunamiPlugin.h"
#include "../Session.h"
#include "PluginManager.h"
#include "Plugin.h"

const string TsunamiPlugin::MESSAGE_STOP_REQUEST = "StopRequest";

TsunamiPlugin::TsunamiPlugin()
{
	session = NULL;
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



TsunamiPlugin *CreateTsunamiPlugin(Session *session, const string &name)
{
	Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::TYPE_TSUNAMI_PLUGIN, name);
	TsunamiPlugin *t = NULL;

	if (p->usable)
		t = (TsunamiPlugin*)p->createInstance(session, "TsunamiPlugin");

	// dummy?
	if (!t)
		t = new TsunamiPlugin;

	t->name = name;
	t->session = session;
	t->song = session->song;
	/*t->plugin = p;
	t->usable = p->usable;
	t->resetConfig();*/
	return t;
}
