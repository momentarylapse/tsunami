/*
 * PluginConsole.cpp
 *
 *  Created on: 23.08.2018
 *      Author: michi
 */

#include "PluginConsole.h"
#include "../../Session.h"
#include "../../Plugins/TsunamiPlugin.h"
#include "../../Plugins/PluginManager.h"
#include "../Helper/ModulePanel.h"


PluginConsole::PluginConsole(Session *s) :
	BottomBar::Console(_("Plugins"), s)
{
	from_resource("plugin-console");
	next_x = 0;

	event("add", [this] { on_add_button(); });

	session->subscribe(this, [this] { on_add_plugin(); }, session->MESSAGE_ADD_PLUGIN);
	session->subscribe(this, [this] { on_remove_plugin(); }, session->MESSAGE_REMOVE_PLUGIN);
}

PluginConsole::~PluginConsole() {
	session->unsubscribe(this);
	for (auto *p: panels)
		delete p;
}

void PluginConsole::on_add_button() {
	session->plugin_manager->choose_module(this, session, ModuleCategory::TSUNAMI_PLUGIN, [this] (const string &name) {
		if (name != "")
			session->execute_tsunami_plugin(name);
	});
}

void PluginConsole::on_add_plugin() {
	auto *plugin = session->last_plugin;
	auto *p = new ModulePanel(plugin, this);
	p->set_func_delete([this, plugin] { plugin->stop_request(); });
	embed(p, "panel-grid", next_x ++, 0);
	panels.add(p);
	hide_control("no-plugins-label", true);
	blink();
}

void PluginConsole::on_remove_plugin() {
	foreachi (auto *p, panels, i)
		if (p->module == session->last_plugin) {
			delete p;
			panels.erase(i);
			break;
		}
	hide_control("no-plugins-label", panels.num > 0);
}

