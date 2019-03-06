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
#include "../Module/ModuleFactory.h"

const string TsunamiPlugin::MESSAGE_STOP_REQUEST = "StopRequest";

TsunamiPlugin::TsunamiPlugin() :
	Module(ModuleType::TSUNAMI_PLUGIN, "")
{
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

TsunamiPlugin *CreateTsunamiPlugin(Session *session, const string &name)
{
	return (TsunamiPlugin*)ModuleFactory::create(session, ModuleType::TSUNAMI_PLUGIN, name);
}
