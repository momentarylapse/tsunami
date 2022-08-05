/*
 * ModulePanel.cpp
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#include "ModulePanel.h"
#include "../../module/Module.h"
#include "../../module/ConfigPanel.h"
#include "../../plugins/PluginManager.h"
#include "../../data/Song.h"
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../../lib/hui/Controls/ControlMenuButton.h"

extern const int CONFIG_PANEL_WIDTH = 400;
extern const int CONFIG_PANEL_HEIGHT = 300;
extern const int CONFIG_PANEL_MIN_HEIGHT = 200;

ModulePanel::ModulePanel(Module *_m, hui::Panel *_parent, Mode mode) {
	set_parent(_parent);
	module = _m;
	session = module->session;
	menu = nullptr;

	outer = _parent;
	bool own_header = (mode & Mode::HEADER);

	from_resource("fx_panel");
	//set_options("grid", format("width=%d,height=%d,expandy,noexpandx", CONFIG_PANEL_WIDTH, CONFIG_PANEL_MIN_HEIGHT));

	if (own_header) {
		outer = this;
		set_string("name", module->module_class);
	} else {
		remove_control("header");
	}

	ConfigPanel::_hidden_parent_ = this;
	p = module->create_panel();
	if (p) {
		embed(p.get(), "content", 0, 0);
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

	outer->event("enabled", [this] { on_enabled(); });
	outer->event("delete", [this] { on_delete(); });
	outer->event("load_favorite", [this] { on_load(); });
	outer->event("save_favorite", [this] { on_save(); });
	outer->event("show_large", [this] { on_large(); });
	outer->event("show_external", [this] { on_external(); });
	outer->event("replace", [this] { on_replace(); });
	outer->event("detune", [this] { on_detune(); });

	outer->hide_control("enabled", true);
	outer->check("enabled", module->enabled);
	
	auto *mb = (hui::ControlMenuButton*)outer->_get_control_("menu");
	if (mb and mb->menu) {
		menu = mb->menu;
		menu->enable("delete", false);
		menu->enable("replace", false);
	}

	old_param = module->config_to_string();
	module->subscribe(this, [this] { on_change(); }, module->MESSAGE_CHANGE);
	module->subscribe(this, [this] {
		module->unsubscribe(this);
		module = nullptr;
		unembed(p.get());
		p = nullptr;
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
	if (menu)
		menu->enable("delete", f != nullptr);
}

void ModulePanel::set_func_close(std::function<void()> f) {
	func_close = f;
}

void ModulePanel::set_func_replace(std::function<void()> f) {
	func_replace = f;
	if (menu)
		menu->enable("replace", f != nullptr);
}

void ModulePanel::set_func_detune(std::function<void()> f) {
	func_detune = f;
	if (f and menu) {
		menu->add_separator();
		menu->add(_("Detune..."), "detune");
	}
}

void ModulePanel::on_load() {
	session->plugin_manager->select_profile_name(win, module, false, [this] (const string &name) {
		if (name.num == 0)
			return;
		session->plugin_manager->apply_profile(module, name, false);
		module->changed();
		//if (func_edit)
		//	func_edit(old_param);
		//old_param = module->config_to_string();
	});
}

void ModulePanel::on_save() {
	session->plugin_manager->select_profile_name(win, module, true, [this] (const string &name) {
		if (name.num == 0)
			return;
		session->plugin_manager->save_profile(module, name);
	});
}

void ModulePanel::on_enabled() {
	if (func_enable)
		func_enable(outer->is_checked("enabled"));
}

void ModulePanel::on_delete() {
	if (func_delete)
		hui::run_later(0, func_delete);
}

void ModulePanel::copy_into(ModulePanel *c) {
	c->set_func_delete(func_delete);
	c->set_func_enable(func_enable);
	c->set_func_close(func_close);
	c->set_func_replace(func_replace);
	c->set_func_detune(func_detune);
}

void ModulePanel::on_large() {
	auto *c = new ModulePanel(module, session->win.get());
	copy_into(c);
	session->win->set_big_panel(c);
}

void ModulePanel::on_external() {
	auto *dlg = new ModuleExternalDialog(module, session->win.get());
	copy_into(dlg->module_panel);
	dlg->show();
	// "self-deleting"
}

void ModulePanel::on_change() {
	outer->check("enabled", module->enabled);
}

void ModulePanel::on_replace() {
	hui::run_later(0.001f, func_replace);
}

void ModulePanel::on_detune() {
	func_detune();
}




ModuleExternalDialog::ModuleExternalDialog(Module *_module, hui::Window *parent) : hui::Dialog("module-external-dialog", parent) {
	module = _module;
	module_panel = new ModulePanel(module, this, ModulePanel::Mode::DEFAULT_S);
	set_title(module->module_class);
	set_size(CONFIG_PANEL_WIDTH, 300);
	//m->set_options("grid", "expandx");
	embed(module_panel, "content", 0, 0);
	module->subscribe(this, [this] {
		module = nullptr;
		request_destroy();
	}, module->MESSAGE_DELETE);
	event("hui:close",[this] {
		msg_write("deleting external dialog...soon");
		hui::run_later(0.01f, [this] {
			msg_error("deleting external dialog...");
			delete this;
		});
	});
}

ModuleExternalDialog::~ModuleExternalDialog() {
	if (module)
		module->unsubscribe(this);
}