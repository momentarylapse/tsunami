//
// Created by michi on 5/9/24.
//

#include "SignalChainPanel.h"
#include "../signaleditor/SignalEditorTab.h"
#include "../mainview/MainView.h"
#include "../module/ModulePanel.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../plugins/PluginManager.h"
#include "../../module/Module.h"
#include "../../module/SignalChain.h"
#include "../../lib/base/sort.h"

SignalChainPanel::SignalChainPanel(Session *session, SideBar *bar) :
		SideBarConsole(_("Signal Chain"), "signal-chain-panel", session, bar),
		in_editor_module_selected(this, [this] (Module *m) {
			set_module(m);
		})
{

	from_resource("signal-chain-panel");

	id_list = "module-list";

	session->main_view->out_view_changed >> create_data_sink<MainViewNode*>([this] (MainViewNode* node) {
		if (editor) {
			editor->out_module_selected.unsubscribe(this);
			editor = nullptr;
		}
		if (auto v = dynamic_cast<SignalEditorTab*>(node)) {
			editor = v;
			set_module(nullptr);
			v->out_module_selected >> in_editor_module_selected;
		}
	});

	fill_module_list("");

	event("search", [this]{ on_search(); });
	event(id_list, [this]{ on_list(); });
}

SignalChainPanel::~SignalChainPanel() = default;

void SignalChainPanel::fill_module_list(const string& search) {
	modules.clear();
	for (int c=0; c<(int)ModuleCategory::SIGNAL_CHAIN; c++) {
		auto names = session->plugin_manager->find_module_sub_types((ModuleCategory) c);
		for (auto &n: names)
			if (n.lower().find(search) >= 0) //.match("*" + search + "*"))
				modules.add({(ModuleCategory)c, n});
	}
//	modules = base::sorted(modules, [] (const string& a, const string& b) { return a <= b; });

	reset(id_list);
	for (auto& m: modules)
		add_string(id_list, m.name);
}

void SignalChainPanel::on_list() {
	int n = get_int("");
	if (n >= 0) {
		if (auto c = chain())
			c->add(modules[n].category, modules[n].name);
	}

}

void SignalChainPanel::on_search() {
	fill_module_list(get_string(""));
}

SignalChain* SignalChainPanel::chain() {
	if (editor)
		return editor->chain;
	return nullptr;
}

void SignalChainPanel::set_module(Module *m) {
	if (module_panel) {
		unembed(module_panel);
		//delete module_panel;
		module_panel = nullptr;
	}

	hide_control("grid-chain", m);
	hide_control("grid-module", !m);

	if (m) {
		module_panel = new ModulePanel(m, parent, ConfigPanelMode::FIXED_HEIGHT | ConfigPanelMode::PROFILES |
												  ConfigPanelMode::REPLACE);
		embed(module_panel, "grid-module", 0, 0);
	} else {
		session->set_mode(EditMode::SignalChain);
	}
}
