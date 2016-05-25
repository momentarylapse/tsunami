/*
 * TsunamiPlugin.cpp
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#include "TsunamiPlugin.h"

const string TsunamiPlugin::MESSAGE_END = "End";

TsunamiPlugin::TsunamiPlugin() :
	Observable("TsunamiPlugin")
{
	win = NULL;
	view = NULL;
	song = NULL;
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

void TsunamiPlugin::end()
{
	notify(MESSAGE_END);
}
