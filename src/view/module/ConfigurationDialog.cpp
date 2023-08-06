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
#include "../../module/audio/AudioEffect.h"

extern const int CONFIG_PANEL_DIALOG_WIDTH;
extern const int CONFIG_PANEL_DIALOG_HEIGHT;



class ConfigurationDialog : public obs::Node<hui::Dialog> {
public:
	ConfigPanelSocket socket;

	static ConfigPanelMode config_panel_mode(shared<Module> m) {
		if (m->module_category == ModuleCategory::AUDIO_EFFECT)
			return ConfigPanelMode::PROFILES | ConfigPanelMode::WETNESS;
		return ConfigPanelMode::PROFILES;
	}

	ConfigurationDialog(shared<Module> m, hui::Window *parent) :
		obs::Node<hui::Dialog>("configurable_dialog", parent),
		socket(m.get(), config_panel_mode(m))
	{
		module = m;
		set_title(module->module_class);
		set_size(CONFIG_PANEL_DIALOG_WIDTH, CONFIG_PANEL_DIALOG_HEIGHT);
		socket.integrate(this);
		module->out_death >> create_sink([this] {
			module = nullptr;
			request_destroy();
		});

		if (module->module_category == ModuleCategory::AUDIO_EFFECT) {
			socket.set_func_set_wetness([this] (float w) {
				reinterpret_cast<AudioEffect*>(module.get())->wetness = w;
				module->out_changed();
			});
		}

		if (module->module_category != ModuleCategory::AUDIO_EFFECT)
			hide_control("preview", true);

		event("ok", [this]{ on_ok(); });
		event("cancel", [this]{ on_cancel(); });
		event("preview", [this]{ on_preview(); });
	}
	~ConfigurationDialog() {
		if (module)
			module->unsubscribe(this);
	}
	void on_ok() {
		_promise();
		request_destroy();
	}
	void on_cancel() {
		_promise.fail();
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

	shared<Module> module;
	owned<Progress> progress;
	hui::promise<void> _promise;
};

hui::future<void> configure_module(hui::Window *win, shared<Module> m) {
	auto *config = m->get_config();
	if (!config) {
		static hui::promise<void> static_promise;
		static_promise.cb_success = nullptr;
		static_promise.cb_fail = nullptr;
		hui::run_later(0.01f, [] {
				static_promise();
		});
		return static_promise.get_future();
	}

	auto *dlg = new ConfigurationDialog(m, win);
	hui::fly(dlg);
	return dlg->_promise.get_future();
}


