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
	/*add_grid("!expandy", 0, 0, "main-grid");
	set_target("main-grid");
	add_scroller("", 0, 0, "scroller");
	add_label("!angle=90,bold,width=60,expandy\\" + _("no plugins yet"), 1, 0, "no-plugins-label");
	add_button("!expandy,noexpandx", 2, 0, "add");
	set_image("add", "hui:add");
	set_tooltip("add", _("add plugin"));
	set_target("scroller");
	add_grid("", 0, 0, "panel-grid");*/
	next_x = 0;

	event("add", [=]{ on_add_button(); });

	session->subscribe(this, [=]{ on_add_plugin(); }, session->MESSAGE_ADD_PLUGIN);
	session->subscribe(this, [=]{ on_remove_plugin(); }, session->MESSAGE_REMOVE_PLUGIN);
}

PluginConsole::~PluginConsole() {
	session->unsubscribe(this);
	for (auto *p: panels)
		delete p;
}

void PluginConsole::on_add_button() {
	string name = session->plugin_manager->choose_module(this, session, ModuleType::TSUNAMI_PLUGIN, "");
	if (name != "")
		session->execute_tsunami_plugin(name);
}

void PluginConsole::on_add_plugin() {
	auto *plugin = session->last_plugin;
	auto *p = new ModulePanel(plugin);
	p->set_func_delete([=]{ plugin->stop_request(); });
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

