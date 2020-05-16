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
#include "../../Data/Song.h"
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../../lib/hui/Controls/ControlMenuButton.h"

extern const int CONFIG_PANEL_WIDTH = 400;
extern const int CONFIG_PANEL_HEIGHT = 300;
extern const int CONFIG_PANEL_MIN_HEIGHT = 200;

ModulePanel::ModulePanel(Module *_m, hui::Panel *_outer, Mode mode) {
	module = _m;
	session = module->session;

	outer = _outer;

	from_resource("fx_panel");
	//set_options("grid", format("width=%d,height=%d,expandy,noexpandx", CONFIG_PANEL_WIDTH, CONFIG_PANEL_MIN_HEIGHT));

	if (outer) {
		remove_control("header");
	} else {
		outer = this;
		set_string("name", module->module_subtype);
	}

	p = module->create_panel();
	if (p) {
		embed(p, "content", 0, 0);
		p->update();
	} else {
		set_target("content");
		add_label("!center,expandx,disabled\\<i>" + _("not configurable") + "</i>", 0, 1, "");
		outer->hide_control("load_favorite", true);
		outer->hide_control("save_favorite", true);
	}
	if (mode & Mode::FIXED_WIDTH) {
		set_options("grid", "noexpandx");
		set_options("content", format("width=%d", CONFIG_PANEL_WIDTH));
	} else {
		set_options("grid", "expandx");
		//set_options("content", format("expandx", CONFIG_PANEL_WIDTH));
	}
	if (mode & Mode::FIXED_HEIGHT) {
		set_options("content", format("height=%d", CONFIG_PANEL_HEIGHT));
		set_options("grid", "noexpandy");
	} else {
		set_options("grid", "expandy");
		//set_options("grid", format("height=%d,expandy", CONFIG_PANEL_MIN_HEIGHT));
	}

	outer->event("enabled", [=]{ on_enabled(); });
	outer->event("delete", [=]{ on_delete(); });
	outer->event("load_favorite", [=]{ on_load(); });
	outer->event("save_favorite", [=]{ on_save(); });
	outer->event("show_large", [=]{ on_large(); });
	outer->event("show_external", [=]{ on_external(); });
	outer->event("replace", [=]{ on_replace(); });
	outer->event("detune", [=]{ on_detune(); });

	outer->hide_control("enabled", true);
	outer->check("enabled", module->enabled);
	
	auto *mb = (hui::ControlMenuButton*)outer->_get_control_("menu");
	menu = mb->menu;
	menu->enable("delete", false);
	menu->enable("replace", false);

	old_param = module->config_to_string();
	module->subscribe(this, [=]{ on_change(); }, module->MESSAGE_CHANGE);
	module->subscribe(this, [=]{
		module->unsubscribe(this);
		module = nullptr;
		delete p;
	}, module->MESSAGE_DELETE);
}

ModulePanel::~ModulePanel() {
	if (module)
		module->unsubscribe(this);
	if (func_close)
		func_close();
}

void ModulePanel::set_width(int width) {
	set_options("grid", format("width=%d", width));
}

void ModulePanel::set_func_enable(std::function<void(bool)> f) {
	func_enable = f;
	outer->hide_control("enabled", f == nullptr);
}

void ModulePanel::set_func_delete(std::function<void()> f) {
	func_delete = f;
	menu->enable("delete", f != nullptr);
}

void ModulePanel::set_func_close(std::function<void()> f) {
	func_close = f;
}

void ModulePanel::set_func_replace(std::function<void()> f) {
	func_replace = f;
	menu->enable("replace", f != nullptr);
}

void ModulePanel::set_func_detune(std::function<void()> f) {
	func_detune = f;
	if (f) {
		menu->add_separator();
		menu->add(_("Detune..."), "detune");
	}
}

void ModulePanel::on_load() {
	string name = session->plugin_manager->select_favorite_name(win, module, false);
	if (name.num == 0)
		return;
	session->plugin_manager->apply_favorite(module, name, false);
	module->changed();
	//if (func_edit)
	//	func_edit(old_param);
	//old_param = module->config_to_string();
}

void ModulePanel::on_save() {
	string name = session->plugin_manager->select_favorite_name(win, module, true);
	if (name.num == 0)
		return;
	session->plugin_manager->save_favorite(module, name);
}

void ModulePanel::on_enabled() {
	if (func_enable)
		func_enable(outer->is_checked("enabled"));
}

void ModulePanel::on_delete() {
	if (func_delete)
		hui::RunLater(0, func_delete);
}

void ModulePanel::copy_into(ModulePanel *c) {
	c->set_func_delete(func_delete);
	c->set_func_enable(func_enable);
	c->set_func_close(func_close);
	c->set_func_replace(func_replace);
	c->set_func_detune(func_detune);
}

void ModulePanel::on_large() {
	auto *c = new ModulePanel(module, nullptr);
	copy_into(c);
	session->win->set_big_panel(c);
}

class ModuleExternalDialog : public hui::Dialog {
public:
	Module *module;
	ModuleExternalDialog(ModulePanel *m_orig, hui::Window *parent) : hui::Dialog("module-external-dialog", parent) {
		auto m = new ModulePanel(m_orig->module, this, ModulePanel::Mode::DEFAULT_S);
		m_orig->copy_into(m);
		set_title(m->module->module_subtype);
		set_size(CONFIG_PANEL_WIDTH, 300);
		module = m->module;
		//m->set_options("grid", "expandx");
		embed(m, "content", 0, 0);
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
	auto *dlg = new ModuleExternalDialog(this, session->win);
	dlg->show();
}

void ModulePanel::on_change() {
	outer->check("enabled", module->enabled);
}

void ModulePanel::on_replace() {
	hui::RunLater(0.001f, func_replace);
}

void ModulePanel::on_detune() {
	func_detune();
}


