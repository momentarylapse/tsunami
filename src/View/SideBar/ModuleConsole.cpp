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

	from_resource("module_console");

	module = nullptr;
	module_panel = nullptr;

	event("edit_song", std::bind(&ModuleConsole::on_edit_song, this));
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

void ModuleConsole::set_module(Module* m)
{
	clear();

	module = m;

	if (module){
		module->subscribe(this, std::bind(&ModuleConsole::on_module_delete, this), Module::MESSAGE_DELETE);
		set_string("category", Module::type_to_name(module->module_type));
		set_string("sub_category", m->module_subtype);
		module_panel = module->create_panel();
		if (module_panel){
			module_panel->update();
			embed(module_panel, "grid", 0, 0);
		}
		hide_control("no_config", module_panel);
	}
}

void ModuleConsole::on_edit_song()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void ModuleConsole::on_module_delete()
{
	clear();
}
