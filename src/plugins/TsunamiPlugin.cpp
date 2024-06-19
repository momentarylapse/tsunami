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
#include "../module/Module.h"
#include "../module/ModuleFactory.h"

namespace tsunami {

TsunamiPlugin::TsunamiPlugin() :
	Module(ModuleCategory::TSUNAMI_PLUGIN, "")
{
}

TsunamiPlugin::~TsunamiPlugin() {
}

void TsunamiPlugin::__init__() {
	new(this) TsunamiPlugin;
}

void TsunamiPlugin::__delete__() {
	this->TsunamiPlugin::~TsunamiPlugin();
}

void TsunamiPlugin::stop_request() {
	out_stop_request.notify();
}

TsunamiPlugin *CreateTsunamiPlugin(Session *session, const string &name) {
	return (TsunamiPlugin*)ModuleFactory::create(session, ModuleCategory::TSUNAMI_PLUGIN, name);
}

}
