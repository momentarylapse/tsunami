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
#include "../../Playback.h"
#include "../../module/Module.h"
#include "../../module/SignalChain.h"
#include "../../module/audio/AudioEffect.h"
#include "../../module/audio/SongRenderer.h"
#include "../audioview/AudioView.h"

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

		if (module->module_category != ModuleCategory::AUDIO_EFFECT and module->module_category != ModuleCategory::AUDIO_SOURCE)
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
		if (progress)
			preview_end();

		progress = new ProgressCancelable(_("Preview"), win);
		progress->out_cancel >> create_sink([this]{ on_progress_cancel(); });

		playback = new Playback(module->session);
		auto c = playback->signal_chain.get();
		if (module->module_category == ModuleCategory::AUDIO_EFFECT) {
			playback->renderer->set_range(module->session->view->sel.range());
			playback->renderer->allow_layers({module->session->view->cur_layer()});

			// TODO remove other effects?
			playback->renderer->preview_effect = (AudioEffect*)module.get();
		} else if (module->module_category == ModuleCategory::AUDIO_SOURCE) {
			c->disconnect_out(playback->renderer.get(), 0);
			c->_add(module);
			c->connect(module.get(), 0, (Module*)playback->peak_meter.get(), 0);
		}
		playback->out_tick >> create_sink([this] { on_update_stream(); });
		playback->out_state_changed >> create_sink([this] {
			if (!playback->is_active())
				hui::run_later(0.01f, [this] { preview_end(); });
		});
		playback->play();
	}

	void on_progress_cancel() {
		preview_end();
	}

	void on_update_stream() {
		if (progress) {
			int pos = playback->get_pos();
			Range r = playback->renderer->range();
			progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
		}
	}

	void preview_end() {
		progress = nullptr;
		playback = nullptr;
	}

	shared<Module> module;
	owned<Progress> progress;
	owned<Playback> playback;
	base::promise<void> _promise;
};

base::future<void> configure_module(hui::Window *win, shared<Module> m) {
	auto *config = m->get_config();
	if (!config) {
		base::promise<void> promise;
		hui::run_later(0.01f, [promise] () mutable {
				promise();
		});
		return promise.get_future();
	}

	auto *dlg = new ConfigurationDialog(m, win);
	hui::fly(dlg);
	return dlg->_promise.get_future();
}


