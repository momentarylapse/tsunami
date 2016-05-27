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

void TsunamiPlugin::wish_to_stop()
{
	notify(MESSAGE_END);
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
