/*
 * SongPlugin.cpp
 *
 *  Created on: 24.05.2016
 *      Author: michi
 */

#include "SongPlugin.h"

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
