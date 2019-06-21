/*
 * ModulePanel.cpp
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#include "ModulePanel.h"
#include "../../Module/Module.h"
#include "../../Module/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"

extern const int CONFIG_PANEL_WIDTH = 400;

ModulePanel::ModulePanel(Module *_m, std::function<void(bool)> _func_enable, std::function<void()> _func_delete, std::function<void(const string&)> _func_edit) {
	module = _m;
	session = module->session;

	func_enable = _func_enable;
	func_delete = _func_delete;
	func_edit = _func_edit;

	from_resource("fx_panel");
	set_options("grid", format("width=%d", CONFIG_PANEL_WIDTH));

	set_string("name", module->module_subtype);

	p = module->create_panel();
	if (p) {
		embed(p, "content", 0, 0);
		p->update();
	} else {
		set_target("content");
		add_label("!center,expandx\\<i>" + _("not configurable") + "</i>", 0, 1, "");
		hide_control("load_favorite", true);
		hide_control("save_favorite", true);
	}
	set_options("content", format("width=%d", CONFIG_PANEL_WIDTH));

	event("enabled", [=]{ on_enabled(); });
	event("delete", [=]{ on_delete(); });
	event("load_favorite", [=]{ on_load(); });
	event("save_favorite", [=]{ on_save(); });
	event("show_large", [=]{ on_large(); });

	check("enabled", module->enabled);

	old_param = module->config_to_string();
	module->subscribe(this, [=]{ on_change(); }, module->MESSAGE_CHANGE);
	module->subscribe(this, [=]{ on_change_by_action(); }, module->MESSAGE_CHANGE_BY_ACTION);
	module->subscribe(this, [=]{
		module = nullptr;
		delete p;
	}, module->MESSAGE_DELETE);
}

ModulePanel::~ModulePanel() {
	if (module)
		module->unsubscribe(this);
}

void ModulePanel::on_load() {
	string name = session->plugin_manager->select_favorite_name(win, module, false);
	if (name.num == 0)
		return;
	session->plugin_manager->apply_favorite(module, name);
	func_edit(old_param);
	old_param = module->config_to_string();
}

void ModulePanel::on_save() {
	string name = session->plugin_manager->select_favorite_name(win, module, true);
	if (name.num == 0)
		return;
	session->plugin_manager->save_favorite(module, name);
}

void ModulePanel::on_enabled() {
	func_enable(is_checked(""));
}

void ModulePanel::on_delete() {
	hui::RunLater(0, func_delete);
}

void ModulePanel::on_large() {
	//console->set_exclusive(this);
	//p->set_large(true);

}

void ModulePanel::on_change() {
	func_edit(old_param);
	check("enabled", module->enabled);
	if (p)
		p->update();
	old_param = module->config_to_string();

}

void ModulePanel::on_change_by_action() {
	check("enabled", module->enabled);
	if (p)
		p->update();
	old_param = module->config_to_string();
}


