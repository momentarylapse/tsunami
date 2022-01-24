/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"
#include "../Helper/ModulePanel.h"
#include "SignalEditor/SignalEditorTab.h"

#include "../../Module/Port/Port.h"
#include "../../Module/Module.h"
#include "../../Module/ConfigPanel.h"
#include "../../Module/SignalChain.h"
#include "../../Session.h"
#include "../../Storage/Storage.h"
#include "../../Data/base.h"
#include "../../TsunamiWindow.h"



SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	grid_id = "main-grid";
	config_grid_id = "config-panel-grid";
	add_grid("", 0, 0, grid_id);
	set_target(grid_id);
	add_tab_control("!left\\aaa", 0, 0, "selector");
	add_revealer("!slide=left", 1, 0, "revealer");
	set_target("revealer");
	add_grid("!noexpandx", 1, 0, config_grid_id);
	set_target(config_grid_id);
	//add_label("!bold,center,big,expandx", 0, 0, "config-label");
	//add_label("!bold,center,expandx", 0, 1, "message");

	menu_chain = hui::create_resource_menu("popup_signal_chain_menu", this);
	menu_module = hui::create_resource_menu("popup_signal_module_menu", this);

	config_module = nullptr;
	config_panel = nullptr;

	event("selector", [this]{ on_chain_switch(); });

	for (auto *c: weak(session->all_signal_chains))
		add_chain(c);
	show_config(nullptr);

	session->subscribe(this, [this, session] {
		add_chain(session->all_signal_chains.back().get());
	}, session->MESSAGE_ADD_SIGNAL_CHAIN);
}

SignalEditor::~SignalEditor() {
	session->unsubscribe(this);
	show_config(nullptr);
	delete menu_chain;
	delete menu_module;
}

void SignalEditor::add_chain(SignalChain *c) {
	auto *tab = new SignalEditorTab(this, c);
	int index = tabs.num;
	string grid_id = "grid-" + i2s(index);
	if (index > 0)
		add_string("selector", c->name);
	else
		change_string("selector", index, c->name);
	set_target("selector");
	add_grid("", index, 0, grid_id);
	embed(tab, grid_id, 0, 0);
	tabs.add(tab);

	set_int("selector", index);
}

void SignalEditor::on_new() {
	session->create_signal_chain("new");
}

void SignalEditor::on_load() {
	hui::file_dialog_open(win, session->storage->current_chain_directory, {"title=" + _("Load a signal chain"), "filter=*.chain", "showfilter=*.chain"}, [this] (const Path &filename) {
		if (!filename.is_empty()) {
			session->storage->current_chain_directory = filename.parent();
			session->load_signal_chain(filename);
		}
	});
}

void SignalEditor::on_chain_switch() {
	//msg_write("switch");
}

void SignalEditor::show_config(Module *m) {
	if (m and (m == config_module))
		return;
	if (config_panel)
		delete config_panel;
	config_panel = nullptr;
	config_module = m;
	if (m) {
		config_panel = new ModulePanel(config_module, this);
		config_panel->set_func_delete([this, m] {
			for (auto *chain: weak(session->all_signal_chains))
				for (auto *_m: weak(chain->modules))
					if (m == _m)
						chain->delete_module(m);
		});
		embed(config_panel, config_grid_id, 0, 0);
	} else {
		/*set_string("config-label", "");
		set_string("message", _("no module selected"));
		hide_control("message", false);*/
	}
	reveal("revealer", m);
}

void SignalEditor::remove_tab(SignalEditorTab *t) {
	foreachi(auto *tt, tabs, i)
		if (tt == t) {
			delete t;
			tabs.erase(i);
			remove_string("selector", i);
			set_int("selector", 0);
		}
}
