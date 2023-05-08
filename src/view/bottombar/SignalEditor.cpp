/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"
#include "signaleditor/SignalEditorTab.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../TsunamiWindow.h"
#include "../../module/port/Port.h"
#include "../../module/Module.h"
#include "../../module/SignalChain.h"
#include "../../storage/Storage.h"
#include "../../data/base.h"
#include "../../Session.h"



SignalEditor::SignalEditor(Session *session, BottomBar *bar) :
	BottomBar::Console(_("Signal Chain"), "signal-editor", session, bar)
{
	grid_id = "main-grid";
	config_grid_id = "config-panel-grid";
	add_grid("", 0, 0, grid_id);
	set_target(grid_id);
	add_tab_control("!left,noframe\\aaa", 0, 0, "selector");
	add_expander("!slide=left", 1, 0, "revealer");
	set_target("revealer");
	add_grid("!noexpandx", 1, 0, config_grid_id);
	set_target(config_grid_id);
	//add_label("!bold,center,big,expandx", 0, 0, "config-label");
	//add_label("!bold,center,expandx", 0, 1, "message");

	config_module = nullptr;
	config_panel = nullptr;

	event("selector", [this]{ on_chain_switch(); });

	for (auto *c: weak(session->all_signal_chains))
		add_chain(c);
	show_config(nullptr);

	session->out_add_signal_chain >> create_sink([this, session] {
		add_chain(session->all_signal_chains.back().get());
	});
}

SignalEditor::~SignalEditor() {
	session->unsubscribe(this);
	show_config(nullptr);
}

void SignalEditor::add_chain(SignalChain *c) {
	auto *tab = new SignalEditorTab(this, c);
	int index = tabs.num;
	string grid_id = format("grid-%d", index);
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
	hui::file_dialog_open(win, _("Load a signal chain"), session->storage->current_chain_directory, {"filter=*.chain", "showfilter=*.chain"}, [this] (const Path &filename) {
		if (filename) {
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
		unembed(config_panel.get());
	config_panel = nullptr;
	config_module = m;
	if (m) {
		config_panel = new ModulePanel(config_module, this, ConfigPanelMode::FIXED_WIDTH | ConfigPanelMode::PROFILES | ConfigPanelMode::DELETE);
		config_panel->set_func_delete([this, m] {
			for (auto *chain: weak(session->all_signal_chains))
				for (auto *_m: weak(chain->modules))
					if (m == _m)
						chain->delete_module(m);
		});
		embed(config_panel.get(), config_grid_id, 0, 0);
	} else {
		/*set_string("config-label", "");
		set_string("message", _("no module selected"));
		hide_control("message", false);*/
	}
	expand("revealer", m);
}

void SignalEditor::remove_tab(SignalEditorTab *t) {
	foreachi(auto *tt, weak(tabs), i)
		if (tt == t) {
			unembed(t);
			tabs.erase(i);
			remove_string("selector", i);
			set_int("selector", 0);
		}
}
