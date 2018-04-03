/*
 * ModuleConsole.cpp
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#include "ModuleConsole.h"

ModuleConsole::ModuleConsole(Session* session) :
	SideBarConsole(_("Module"), session)
{
	id_inner = "grid";

	fromResource("module_console");

	event("edit_song", std::bind(&ModuleConsole::onEditSong, this));
}

ModuleConsole::~ModuleConsole()
{
}

void ModuleConsole::clear()
{
}

void ModuleConsole::setModule(void* m)
{
}

void ModuleConsole::onEditSong()
{
}

void ModuleConsole::onEditTrack()
{
}

void ModuleConsole::onModuleDelete()
{
}

void ModuleConsole::onModuleSelectionChange()
{
}
