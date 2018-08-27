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
#include "../../Module/ConfigPanel.h"

class PluginPanel : public hui::Panel
{
public:
	PluginPanel(TsunamiPlugin *p, PluginConsole *_console)
	{
		addGrid("!width=380,noexpandx", 0, 0, "grid");
		setTarget("grid");
		addGrid("", 0, 0, "header-grid");
		setTarget("header-grid");
		addButton("", 0, 0, "close");
		setImage("close", "hui:close");
		setTooltip("close", _("stop plugin"));
		addButton("", 1, 0, "big");
		setImage("big", "hui:up");
		setTooltip("big", _("big!!!"));
		addLabel("!expandx,center,bold,big\\" + p->module_subtype, 2, 0, "label");
		plugin = p;
		console = _console;
		config_panel = p->create_panel();
		if (config_panel)
			embed(config_panel, "grid", 0, 1);
		hideControl("big", !config_panel);

		event("close", [&]{ plugin->stop_request(); });
		event("big", [&]{ on_big(); });
	}

	~PluginPanel()
	{
		if (dlg)
			delete dlg;
	}

	void on_big()
	{
		if (dlg or !config_panel)
			return;
		delete config_panel;
		config_panel = nullptr;
		auto *c = plugin->create_panel();
		if (!c)
			return;
		dlg = new hui::Dialog(plugin->module_subtype, 500, 400, console->win, true);
		dlg->addGrid("", 0, 0, "root");
		dlg->embed(plugin->create_panel(), "root", 0, 0);
		dlg->show();

		dlg->event("hui:close", [this]{ hui::RunLater(0.001f, [this]{ close_dialog(); }); });
	}

	void close_dialog()
	{
		delete dlg;
		dlg = nullptr;
		config_panel = plugin->create_panel();
		if (config_panel)
			embed(config_panel, "grid", 0, 1);
	}

	ConfigPanel *config_panel;
	hui::Dialog *dlg = nullptr;
	TsunamiPlugin *plugin;
	PluginConsole *console;

};

PluginConsole::PluginConsole(Session *s) :
	BottomBar::Console(_("Plugins"), s)
{
	addGrid("", 0, 0, "main-grid");
	setTarget("main-grid");
	addScroller("", 0, 0, "scroller");
	addLabel("!angle=90,bold,width=60\\" + _("no plugins yet"), 1, 0, "no-plugins-label");
	addButton("!expandy,noexpandx", 2, 0, "add");
	setImage("add", "hui:add");
	setTooltip("add", _("add plugin"));
	setTarget("scroller");
	addGrid("", 0, 0, "panel-grid");
	next_x = 0;

	event("add", [&]{ on_add_button(); });

	session->subscribe(this, [&]{ on_add_plugin(); }, session->MESSAGE_ADD_PLUGIN);
	session->subscribe(this, [&]{ on_remove_plugin(); }, session->MESSAGE_REMOVE_PLUGIN);
}

PluginConsole::~PluginConsole()
{
	session->unsubscribe(this);
	for (auto *p: panels)
		delete p;
}

void PluginConsole::on_add_button()
{
	string name = session->plugin_manager->ChooseModule(this, session, ModuleType::TSUNAMI_PLUGIN, "");
	if (name != "")
		session->executeTsunamiPlugin(name);
}

void PluginConsole::on_add_plugin()
{
	auto *p = new PluginPanel(session->last_plugin, this);
	embed(p, "panel-grid", next_x ++, 0);
	panels.add(p);
	hideControl("no-plugins-label", true);
	blink();
}

void PluginConsole::on_remove_plugin()
{
	foreachi (auto *p, panels, i)
		if (p->plugin == session->last_plugin){
			delete p;
			panels.erase(i);
			break;
		}
	hideControl("no-plugins-label", panels.num > 0);
}

