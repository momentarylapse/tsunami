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

#if 0
class PluginPanel : public hui::Panel {
public:
	PluginPanel(TsunamiPlugin *p, PluginConsole *_console) {
		add_grid("!width=380,noexpandx", 0, 0, "grid");
		set_target("grid");
		add_grid("", 0, 0, "header-grid");
		set_target("header-grid");
		add_button("", 0, 0, "close");
		set_image("close", "hui:close");
		set_tooltip("close", _("stop plugin"));
		add_button("", 1, 0, "big");
		set_image("big", "hui:up");
		set_tooltip("big", _("big!!!"));
		add_button("", 2, 0, "external");
		set_image("external", "hui:zoom-out");
		set_tooltip("external", _("external!!!"));
		add_label("!expandx,center,bold,big\\" + p->module_subtype, 3, 0, "label");
		plugin = p;
		console = _console;
		config_panel = p->create_panel();
		if (config_panel)
			embed(config_panel, "grid", 0, 1);
		hide_control("big", !config_panel);

		event("close", [=]{ plugin->stop_request(); });
		event("external", [=]{ start_external(); });
		event("big", [=]{ if (is_big) stop_big(); else start_big(); });
	}

	~PluginPanel() {
		if (dlg)
			delete dlg;
		if (is_big)
			console->set_big(nullptr);
	}

	void start_big() {
		if (!config_panel or is_big)
			return;
		delete config_panel;
		config_panel = nullptr;
		console->set_big(this);
		is_big = true;
	}

	void stop_big() {
		if (!is_big)
			return;
		console->set_big(nullptr);
		restart_normal();
		is_big = false;
	}

	void restart_normal() {
		if (config_panel)
			return;
		config_panel = plugin->create_panel();
		if (config_panel)
			embed(config_panel, "grid", 0, 1);
		is_big = false;
	}

	void start_external() {
		if (dlg or !config_panel)
			return;
		delete config_panel;
		config_panel = nullptr;
		auto *c = plugin->create_panel();
		if (!c)
			return;
		dlg = new hui::Dialog(plugin->module_subtype, 500, 400, console->win, true);
		dlg->add_grid("", 0, 0, "root");
		dlg->embed(plugin->create_panel(), "root", 0, 0);
		dlg->show();

		dlg->event("hui:close", [=]{ hui::RunLater(0.001f, [=]{ stop_external(); }); });
	}

	void stop_external() {
		if (!dlg)
			return;
		delete dlg;
		dlg = nullptr;
		config_panel = plugin->create_panel();
		if (config_panel)
			embed(config_panel, "grid", 0, 1);
	}

	bool is_big = false;
	ConfigPanel *config_panel;
	hui::Dialog *dlg = nullptr;
	TsunamiPlugin *plugin;
	PluginConsole *console;
};

#endif

PluginConsole::PluginConsole(Session *s) :
	BottomBar::Console(_("Plugins"), s)
{
	add_grid("", 0, 0, "main-grid");
	set_target("main-grid");
	add_scroller("", 0, 0, "scroller");
	add_label("!angle=90,bold,width=60\\" + _("no plugins yet"), 1, 0, "no-plugins-label");
	add_button("!expandy,noexpandx", 2, 0, "add");
	set_image("add", "hui:add");
	set_tooltip("add", _("add plugin"));
	set_target("scroller");
	add_grid("", 0, 0, "panel-grid");
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

/*void PluginConsole::set_big(PluginPanel *p) {
	if (big)
		big->restart_normal();
	if (big_panel) {
		delete big_panel;
		big_panel = nullptr;
		win->remove_control("plugin-grid");
	}
	big = p;
	if (big) {
		big_panel = big->plugin->create_panel();
		win->set_target("root-grid");
		//win->add_paned("!expandx", 0, 0, "plugin-grid");
		win->add_grid("!expandx,width=500", 0, 0, "plugin-grid");
		win->embed(big_panel, "plugin-grid", 0, 0);
	}
}*/

