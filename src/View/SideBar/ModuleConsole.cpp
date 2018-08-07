/*
 * ModuleConsole.cpp
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#include "ModuleConsole.h"

#include "../../Module/SignalChain.h"
#include "../../Module/ConfigPanel.h"
#include "../../Module/Module.h"

ModuleConsole::ModuleConsole(Session* session) :
	SideBarConsole(_("Module"), session)
{
	id_inner = "grid";

	fromResource("module_console");

	module = nullptr;
	module_panel = nullptr;

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
	module_panel = nullptr;
	if (module)
		module->unsubscribe(this);
	module = nullptr;
	reset("category");
	reset("sub_category");
}

void ModuleConsole::setModule(Module* m)
{
	clear();

	module = m;

	if (module){
		module->subscribe(this, std::bind(&ModuleConsole::onModuleDelete, this), Module::MESSAGE_DELETE);
		setString("category", Module::type_to_name(module->module_type));
		setString("sub_category", m->module_subtype);
		module_panel = module->create_panel();
		if (module_panel){
			module_panel->update();
			embed(module_panel, "grid", 0, 0);
		}
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
