/*
 * ModuleConsole.cpp
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#include "ModuleConsole.h"

#include "../../Module/SignalChain.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/Configurable.h"

ModuleConsole::ModuleConsole(Session* session) :
	SideBarConsole(_("Module"), session)
{
	id_inner = "grid";

	fromResource("module_console");

	module = NULL;
	module_panel = NULL;

	event("edit_song", std::bind(&ModuleConsole::onEditSong, this));
}

ModuleConsole::~ModuleConsole()
{
	clear();
}

void ModuleConsole::clear()
{
	if (module_panel)
		delete module_panel;
	module_panel = NULL;
	if (module)
		module->unsubscribe(this);
	module = NULL;
	reset("category");
	reset("sub_category");
}

void ModuleConsole::setModule(Configurable* m)
{
	clear();

	module = m;

	if (module){
		module->subscribe(this, std::bind(&ModuleConsole::onModuleDelete, this), Configurable::MESSAGE_DELETE);
		setString("category", Configurable::type_to_name(module->configurable_type));
		//setString("sub_category", m->sub_type());
		module_panel = module->create_panel();
		if (module_panel)
			embed(module_panel, "grid", 0, 0);
		hideControl("no_config", module_panel);
	}
}

void ModuleConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void ModuleConsole::onModuleDelete()
{
	clear();
}

void ModuleConsole::onModuleSelectionChange()
{
}
