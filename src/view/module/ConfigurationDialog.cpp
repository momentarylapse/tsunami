/*
 * ConfigurationDialog.cpp
 *
 *  Created on: 6 Aug 2022
 *      Author: michi
 */

#include "ConfigurationDialog.h"
#include "ConfigPanel.h"
#include "ModulePanel.h"
#include "../helper/Progress.h"
#include "../../plugins/PluginManager.h"
#include "../../Session.h"
#include "../../module/Module.h"

extern const int CONFIG_PANEL_WIDTH;
extern const int CONFIG_PANEL_HEIGHT;

void configure_module_x(hui::Window *win, Module *m, hui::Callback cb, hui::Callback cb_cancel, bool autodel);


class ConfigurationDialog : public hui::Dialog {
public:
	ConfigurationDialog(Module *m, hui::Window *parent) :
		hui::Dialog("configurable_dialog", parent)
	{
		module = m;
		module_panel = new ModulePanel(module, this, ConfigPanelMode::CONFIG_PANEL);
		set_title(module->module_class);
		set_size(CONFIG_PANEL_WIDTH, 300);
		embed(module_panel, "content", 0, 0);
		module->subscribe(this, [this] {
			module = nullptr;
			request_destroy();
		}, module->MESSAGE_DELETE);

		ok = false;


		if (module->module_category != ModuleCategory::AUDIO_EFFECT)
			hide_control("preview", true);

		event("ok", [this]{ on_ok(); });
		event("cancel", [this]{ request_destroy(); });
		event("preview", [this]{ on_preview(); });
	}
	~ConfigurationDialog() {
		if (module)
			module->unsubscribe(this);
	}
	void on_ok() {
		ok = true;
		request_destroy();
	}
	void on_preview() {
		preview_start();
	}

	void preview_start() {
		module->session->e("TODO: preview");
		/*if (progress)
			previewEnd();
		config->configToString();
		tsunami->win->view->renderer->preview_effect = (Effect*)config;


		progress = new ProgressCancelable(_("Preview"), win);
		progress->subscribe(this, [=]{ onProgressCancel(); }, progress->MESSAGE_CANCEL);

		tsunami->win->view->stream->subscribe(this, [=]{ onUpdateStream(); });
		tsunami->win->view->renderer->prepare(tsunami->win->view->sel.range, false);
		tsunami->win->view->stream->play();*/
	}

	void on_progress_cancel() {
		preview_end();
	}

	void on_update_stream() {
		/*if (progress){
			int pos = tsunami->win->view->stream->getPos(0); // TODO
			Range r = tsunami->win->view->sel.range;
			progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
			if (!tsunami->win->view->stream->isPlaying())
				previewEnd();
		}*/
	}

	void preview_end() {
		/*if (!progress)
			return;
		tsunami->win->view->stream->unsubscribe(this);
		progress->unsubscribe(this);
		tsunami->win->view->stream->stop();
		progress = NULL;


		tsunami->win->view->renderer->preview_effect = NULL;*/
	}

	Module *module;
	ModulePanel *module_panel;
	owned<Progress> progress;
	bool ok;
};

void configure_module(hui::Window *win, Module *m, hui::Callback cb, hui::Callback cb_cancel) {
	configure_module_x(win, m, cb, cb_cancel, false);
}

void configure_module_autodel(hui::Window *win, Module *m, hui::Callback cb, hui::Callback cb_cancel) {
	configure_module_x(win, m, cb, cb_cancel, true);
}

void configure_module_x(hui::Window *win, Module *m, hui::Callback cb, hui::Callback cb_cancel, bool autodel) {
	auto *config = m->get_config();
	if (!config) {
		cb();
		if (autodel)
			hui::run_later(2.1f, [m] { delete m; });
		return;
	}

	auto *dlg = new ConfigurationDialog(m, win);

	hui::fly(dlg, [dlg, cb, cb_cancel, autodel, m] {
		if (dlg->ok)
			cb();
		else if (cb_cancel)
			cb_cancel();
		if (autodel)
			hui::run_later(2.1f, [m] { delete m; });
		// should happen after the config dialog was destroyed.... (same thread, should not interrupt the module)
	});
}


