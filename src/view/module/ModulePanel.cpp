/*
 * ModulePanel.cpp
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#include "ModulePanel.h"
#include "ConfigPanel.h"
#include "../../module/Module.h"
#include "../../plugins/PluginManager.h"
#include "../../data/Song.h"
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../../lib/hui/Controls/ControlMenuButton.h"

extern const int CONFIG_PANEL_WIDTH = 400;
extern const int CONFIG_PANEL_HEIGHT = 300;
extern const int CONFIG_PANEL_MIN_HEIGHT = 200;


ConfigPanelSocket::ConfigPanelSocket(Module *_m, hui::Panel *_parent, ConfigPanelMode mode) {
	module = _m;
	session = module->session;
	menu = nullptr;

	outer = _parent;
}

ConfigPanelSocket::~ConfigPanelSocket() {
}

void ConfigPanelSocket::on_load() {
	session->plugin_manager->select_profile_name(outer->win, module, false, [this] (const string &name) {
		if (name.num == 0)
			return;
		session->plugin_manager->apply_profile(module, name, false);
		module->changed();
		//if (func_edit)
		//	func_edit(old_param);
		//old_param = module->config_to_string();
	});
}

void ConfigPanelSocket::on_save() {
	session->plugin_manager->select_profile_name(outer->win, module, true, [this] (const string &name) {
		if (name.num == 0)
			return;
		session->plugin_manager->save_profile(module, name);
	});
}

void ConfigPanelSocket::on_enabled() {
	if (func_enable)
		func_enable(outer->is_checked("enabled"));
}

void ConfigPanelSocket::on_delete() {
	if (func_delete)
		hui::run_later(0, func_delete);
}

void ConfigPanelSocket::on_large() {
	auto *c = new ModulePanel(module, session->win.get());
	copy_into(&c->socket);
	session->win->set_big_panel(c);
}

void ConfigPanelSocket::on_external() {
	auto *dlg = new ModuleExternalDialog(module, session->win.get());
	copy_into(&dlg->module_panel->socket);
	dlg->show();
	// "self-deleting"
}

void ConfigPanelSocket::on_change() {
	outer->check("enabled", module->enabled);
}

void ConfigPanelSocket::on_replace() {
	hui::run_later(0.001f, func_replace);
}

void ConfigPanelSocket::on_detune() {
	func_detune();
}

void ConfigPanelSocket::set_func_enable(std::function<void(bool)> f) {
	func_enable = f;
	outer->hide_control("enabled", f == nullptr);
}

void ConfigPanelSocket::set_func_delete(std::function<void()> f) {
	func_delete = f;
	if (menu)
		menu->enable("delete", f != nullptr);
}

void ConfigPanelSocket::set_func_close(std::function<void()> f) {
	func_close = f;
}

void ConfigPanelSocket::set_func_replace(std::function<void()> f) {
	func_replace = f;
	if (menu)
		menu->enable("replace", f != nullptr);
}

void ConfigPanelSocket::set_func_detune(std::function<void()> f) {
	func_detune = f;
	if (f and menu) {
		menu->add_separator();
		menu->add(_("Detune..."), "detune");
	}
}

void ConfigPanelSocket::copy_into(ConfigPanelSocket *c) {
	c->set_func_delete(func_delete);
	c->set_func_enable(func_enable);
	c->set_func_close(func_close);
	c->set_func_replace(func_replace);
	c->set_func_detune(func_detune);
}


ModulePanel::ModulePanel(Module *_m, hui::Panel *_parent, ConfigPanelMode mode) :
	socket(_m, _parent, mode)
{
	set_parent(_parent);
	bool own_header = int(mode & ConfigPanelMode::HEADER);

	from_resource("module-panel");
	//set_options("grid", format("width=%d,height=%d,expandy,noexpandx", CONFIG_PANEL_WIDTH, CONFIG_PANEL_MIN_HEIGHT));

	if (own_header) {
		socket.outer = this;
		set_string("name", socket.module->module_class);
	} else {
		remove_control("header");
	}

	ConfigPanel::_hidden_parent_ = this;
	socket.p = socket.module->create_panel();
	if (socket.p) {
		embed(socket.p.get(), "content", 0, 0);
		socket.p->update();
	} else {
		set_target("content");
		add_label("!center,expandx,disabled\\<i>" + _("not configurable") + "</i>", 0, 1, "");
		socket.outer->hide_control("load_favorite", true);
		socket.outer->hide_control("save_favorite", true);
	}
	if (int(mode & ConfigPanelMode::FIXED_WIDTH)) {
		set_options("grid", "noexpandx");
		set_options("content", format("width=%d", CONFIG_PANEL_WIDTH));
	} else {
		set_options("grid", "expandx");
		//set_options("content", format("expandx", CONFIG_PANEL_WIDTH));
	}
	if (int(mode & ConfigPanelMode::FIXED_HEIGHT)) {
		set_options("content", format("height=%d", CONFIG_PANEL_HEIGHT));
		set_options("grid", "noexpandy");
	} else {
		set_options("grid", "expandy");
		//set_options("grid", format("height=%d,expandy", CONFIG_PANEL_MIN_HEIGHT));
	}

	socket.outer->event("enabled", [this] { socket.on_enabled(); });
	socket.outer->event("delete", [this] { socket.on_delete(); });
	socket.outer->event("load_favorite", [this] { socket.on_load(); });
	socket.outer->event("save_favorite", [this] { socket.on_save(); });
	socket.outer->event("show_large", [this] { socket.on_large(); });
	socket.outer->event("show_external", [this] { socket.on_external(); });
	socket.outer->event("replace", [this] { socket.on_replace(); });
	socket.outer->event("detune", [this] { socket.on_detune(); });

	socket.outer->hide_control("enabled", true);
	socket.outer->check("enabled", socket.module->enabled);
	
	auto *mb = (hui::ControlMenuButton*)socket.outer->_get_control_("menu");
	if (mb and mb->menu) {
		socket.menu = mb->menu;
		socket.menu->enable("delete", false);
		socket.menu->enable("replace", false);
	}

	socket.old_param = socket.module->config_to_string();
	socket.module->subscribe(this, [this] {
		socket.on_change();
	}, Module::MESSAGE_CHANGE);
	socket.module->subscribe(this, [this] {
		socket.module->unsubscribe(this);
		socket.module = nullptr;
		unembed(socket.p.get());
		socket.p = nullptr;
	}, Module::MESSAGE_DELETE);
}

ModulePanel::~ModulePanel() {
	if (socket.module)
		socket.module->unsubscribe(this);
	if (socket.func_close)
		socket.func_close();
}

void ModulePanel::set_width(int width) {
	set_options("grid", format("width=%d", width));
}




ModuleExternalDialog::ModuleExternalDialog(Module *_module, hui::Window *parent) : hui::Dialog("module-external-dialog", parent) {
	module = _module;
	module_panel = new ModulePanel(module, this, ConfigPanelMode::DEFAULT_S);
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
