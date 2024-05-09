//
// Created by michi on 5/9/24.
//

#include "SignalChainPanel.h"
#include "../signaleditor/SignalEditorTab.h"
#include "../mainview/MainView.h"
#include "../module/ModulePanel.h"
#include "../TsunamiWindow.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../plugins/PluginManager.h"
#include "../../module/Module.h"
#include "../../module/SignalChain.h"
#include "../../storage/Storage.h"
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
	event("delete-signal-chain", [this] { on_delete(); });
	event("save-signal-chain", [this] { on_save_as(); });
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
	auto cat_prev = ModuleCategory::TSUNAMI_PLUGIN; // invalid
	for (auto& m: modules) {
		string cat;
		if (m.category != cat_prev) {
			cat = format("<small><span alpha=\"50%%\">%s</span></small>", Module::category_to_str(m.category));
			cat_prev = m.category;
		}
		add_string(id_list, format("%s\\%s", cat, m.name));
	}
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

void SignalChainPanel::on_delete() {
	if (auto c = chain()) {
		if (c->belongs_to_system) {
			session->e(_("not allowed to delete the main signal chain"));
			return;
		}
		hui::run_later(0.001f, [c] { c->unregister(); });
	}
}

void SignalChainPanel::on_save_as() {
	if (auto c = chain()) {
		hui::file_dialog_save(session->win.get(), _("Save the signal chain"), session->storage->current_chain_directory,
		                      {"filter=*.chain", "showfilter=*.chain"}).then([this, c] (const Path &filename) {
			session->storage->current_chain_directory = filename.parent();
			c->save(filename);
		});
	}
}
