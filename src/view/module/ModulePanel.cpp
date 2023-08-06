/*
 * ModulePanel.cpp
 *
 *  Created on: Jun 21, 2019
 *      Author: michi
 */

#include "ModulePanel.h"
#include "ConfigPanel.h"
#include "../dialog/QuestionDialog.h"
#include "../TsunamiWindow.h"
#include "../../module/Module.h"
#include "../../plugins/PluginManager.h"
#include "../../data/Song.h"
#include "../../Session.h"
#include "../../lib/hui/Controls/ControlMenuButton.h"


#include "../../module/audio/AudioEffect.h"

extern const int CONFIG_PANEL_WIDTH = 400;
extern const int CONFIG_PANEL_HEIGHT = 300;
extern const int CONFIG_PANEL_MIN_HEIGHT = 200;
extern const int CONFIG_PANEL_DIALOG_WIDTH = 520;
extern const int CONFIG_PANEL_DIALOG_HEIGHT = 380;



float module_wetness(Module *m) {
	if (m->module_category == ModuleCategory::AUDIO_EFFECT) {
		auto fx = reinterpret_cast<AudioEffect*>(m);
		return fx->wetness;
	}
	return 0;
}


ConfigPanelSocket::ConfigPanelSocket(Module *_m, ConfigPanelMode _mode) {
	module = _m;
	session = module->session;
	menu = nullptr;
	mode = _mode;

	panel = nullptr;
}

ConfigPanelSocket::~ConfigPanelSocket() {
}

void ConfigPanelSocket::integrate(hui::Panel *_panel) {
	panel = _panel;

	ConfigPanel::_hidden_parent_ = panel;
	config_panel = module->create_panel();

	if (config_panel) {
		panel->embed(config_panel.get(), "content", 0, 0);
		config_panel->update();
	} else {
		panel->set_target("content");
		panel->add_label("!center,expandx,disabled\\<i>" + _("not configurable") + "</i>", 0, 1, "");
		panel->hide_control("load_favorite", true);
		panel->hide_control("save_favorite", true);
	}

	panel->hide_control("enabled", (int)(mode & ConfigPanelMode::ENABLE) == 0);
	panel->hide_control("replace", (int)(mode & ConfigPanelMode::REPLACE) == 0);
	panel->hide_control("wetness", (int)(mode & ConfigPanelMode::WETNESS) == 0);

	panel->event("enabled", [this] { on_enabled(); });
	panel->event("wetness", [this] { on_wetness(); });
	panel->event("delete", [this] { on_delete(); });
	panel->event("load_favorite", [this] { on_load(); });
	panel->event("save_favorite", [this] { on_save(); });
	panel->event("show_large", [this] { on_large(); });
	panel->event("show_external", [this] { on_external(); });
	panel->event("replace", [this] { on_replace(); });
	panel->event("detune", [this] { on_detune(); });

	on_change();

	auto *mb = (hui::ControlMenuButton*)panel->_get_control_("menu");
	if (mb and mb->menu) {
		menu = mb->menu;
		menu->enable("delete", (int)(mode & ConfigPanelMode::DELETE));
		menu->enable("replace", (int)(mode & ConfigPanelMode::REPLACE));
	}

	module->out_changed >> create_sink([this] { on_change(); });
}

void ConfigPanelSocket::on_load() {
	session->plugin_manager->select_module_preset_name(panel->win, module, false, [this](const string &name) {
		if (name.num == 0)
			return;
		session->plugin_manager->apply_module_preset(module, name, false);
		module->changed();
		//if (func_edit)
		//	func_edit(old_param);
		//old_param = module->config_to_string();
	});
}

void ConfigPanelSocket::on_save() {
	session->plugin_manager->select_module_preset_name(panel->win, module, true, [this](const string &name) {
		if (name.num == 0)
			return;
		session->plugin_manager->save_module_preset(module, name);
	});
}

void ConfigPanelSocket::on_enabled() {
	if (func_enable)
		func_enable(panel->is_checked("enabled"));
}

void ConfigPanelSocket::on_wetness() {
	if (func_set_wetness)
		QuestionDialogFloat::ask(panel->win, "Effect wetness", [this] (float wetness) {
			if (!QuestionDialogFloat::aborted)
				func_set_wetness(clamp(wetness / 100, 0.0f, 1.0f));
		}, module_wetness(module) * 100, 0, 100);

}

void ConfigPanelSocket::on_delete() {
	if (func_delete)
		hui::run_later(0, func_delete);
}

void ConfigPanelSocket::on_large() {
	auto *c = new ModulePanel(module, session->win.get(), mode);
	copy_into(&c->socket);
	session->win->set_big_panel(c);
}

void ConfigPanelSocket::on_external() {
	auto *dlg = new ModuleExternalDialog(module, session->win.get(), mode);
	copy_into(&dlg->socket);
	hui::fly(dlg);
	// "self-deleting"
}

void ConfigPanelSocket::on_change() {
	panel->check("enabled", module->enabled);

	if (module->module_category == ModuleCategory::AUDIO_EFFECT)
		panel->set_string("wetness", format("%.0f%%", module_wetness(module) * 100));
}

void ConfigPanelSocket::on_replace() {
	if (func_replace)
		hui::run_later(0.001f, func_replace);
}

void ConfigPanelSocket::on_detune() {
	if (func_detune)
		func_detune();
}

void ConfigPanelSocket::set_func_enable(std::function<void(bool)> f) {
	func_enable = f;
}

void ConfigPanelSocket::set_func_set_wetness(std::function<void(float)> f) {
	func_set_wetness = f;
}

void ConfigPanelSocket::set_func_delete(std::function<void()> f) {
	func_delete = f;
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
	c->set_func_set_wetness(func_set_wetness);
	c->set_func_close(func_close);
	c->set_func_replace(func_replace);
	c->set_func_detune(func_detune);
}


ModulePanel::ModulePanel(Module *module, hui::Panel *_parent, ConfigPanelMode mode) :
	socket(module, mode)
{
	set_parent(_parent);
	//bool own_header = int(mode & ConfigPanelMode::HEADER);

	from_resource("module-panel");
	//set_options("grid", format("width=%d,height=%d,expandy,noexpandx", CONFIG_PANEL_WIDTH, CONFIG_PANEL_MIN_HEIGHT));

	set_string("name", socket.module->module_class);

	socket.integrate(this);


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

	

	socket.old_param = module->config_to_string();
	module->out_death >> create_sink([this, module] {
		module->unsubscribe(this);
		socket.module = nullptr;
		if (socket.config_panel)
			unembed(socket.config_panel.get());
		socket.config_panel = nullptr;
	});
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




ModuleExternalDialog::ModuleExternalDialog(Module *module, hui::Window *parent, ConfigPanelMode mode) :
		obs::Node<hui::Dialog>("module-external-dialog", parent),
		socket(module, mode)
{
	set_title(module->module_class);
	set_size(CONFIG_PANEL_DIALOG_WIDTH, CONFIG_PANEL_DIALOG_HEIGHT);
	//m->set_options("grid", "expandx");

	socket.integrate(this);


	//set_options("grid", "noexpandx");
	//set_options("content", format("width=%d", CONFIG_PANEL_WIDTH));
	//set_options("content", format("height=%d,expandx,expandy", CONFIG_PANEL_HEIGHT));
	//set_options("content", format("minheight=%d,expandx,expandy", CONFIG_PANEL_HEIGHT));
	set_options("content", "expandx,expandy");
	//set_options("grid", "noexpandy");

	module->out_death >> create_sink([this] {
		request_destroy();
	});
	event("hui:close",[this] {
		request_destroy();
	});
}

ModuleExternalDialog::~ModuleExternalDialog() {
	if (socket.module)
		socket.module->unsubscribe(this);
}
