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
#include "../Module/Module.h"

const string TsunamiPlugin::MESSAGE_STOP_REQUEST = "StopRequest";

TsunamiPlugin::TsunamiPlugin()
{
	session = nullptr;
	song = nullptr;
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
	Plugin *p = session->plugin_manager->GetPlugin(session, ModuleType::TSUNAMI_PLUGIN, name);
	TsunamiPlugin *t = nullptr;

	if (p->usable)
		t = (TsunamiPlugin*)p->create_instance(session, "TsunamiPlugin");

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
