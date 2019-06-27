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
#include "../../TsunamiWindow.h"

extern const int CONFIG_PANEL_WIDTH = 400;

ModulePanel::ModulePanel(Module *_m, Mode mode) {
	module = _m;
	session = module->session;

	from_resource("fx_panel");
	set_options("grid", format("width=%d,expandy,noexpandx", CONFIG_PANEL_WIDTH));

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
	event("show_external", [=]{ on_external(); });

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

void ModulePanel::set_width(int width) {
	set_options("grid", format("width=%d", width));
}

void ModulePanel::set_func_enabled(std::function<void(bool)> _func_enable) {
	func_enable = _func_enable;
}

void ModulePanel::set_func_delete(std::function<void()> _func_delete) {
	func_delete = _func_delete;
}

void ModulePanel::set_func_edit(std::function<void(const string&)> _func_edit) {
	func_edit = _func_edit;
}

void ModulePanel::on_load() {
	string name = session->plugin_manager->select_favorite_name(win, module, false);
	if (name.num == 0)
		return;
	session->plugin_manager->apply_favorite(module, name);
	if (func_edit)
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
	if (func_enable)
		func_enable(is_checked(""));
}

void ModulePanel::on_delete() {
	if (func_delete)
		hui::RunLater(0, func_delete);
}

ModulePanel *ModulePanel::copy() {
	auto *c = new ModulePanel(module);
	c->set_func_delete(func_delete);
	c->set_func_edit(func_edit);
	c->set_func_enabled(func_enable);
	return c;
}

void ModulePanel::on_large() {
	session->win->set_big_panel(copy());
}

class ModuleExternalDialog : public hui::Dialog {
public:
	Module *module;
	ModuleExternalDialog(ModulePanel *m, hui::Window *parent) : hui::Dialog(m->module->module_subtype, CONFIG_PANEL_WIDTH, 300, parent, true) {
		module = m->module;
		m->set_options("grid", "expandx");
		add_grid("", 0, 0, "grid");
		embed(m, "grid", 0, 0);
		module->subscribe(this, [=]{
			module = nullptr;
			destroy();
		}, module->MESSAGE_DELETE);
	}
	void on_destroy() override {
		if (module)
			module->unsubscribe(this);
	}
};

void ModulePanel::on_external() {
	auto *dlg = new ModuleExternalDialog(copy(), session->win);
	dlg->show();
}

void ModulePanel::on_change() {
	if (func_edit)
		func_edit(old_param);
	check("enabled", module->enabled);
	old_param = module->config_to_string();

}

void ModulePanel::on_change_by_action() {
	check("enabled", module->enabled);
	old_param = module->config_to_string();
}


