/*
 * ConfigPanel.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ConfigPanel.h"
#include "Module.h"
#include "../View/Helper/Progress.h"
#include "../View/Helper/ModulePanel.h"
#include "../Plugins/PluginManager.h"
#include "../Session.h"

extern const int CONFIG_PANEL_WIDTH;
extern const int CONFIG_PANEL_HEIGHT;


ConfigPanel::ConfigPanel(Module *_c) {
	ignore_change = false;
	c = _c;
	if (c)
		c->subscribe(this, [=]{ if (!ignore_change) update(); }, c->MESSAGE_CHANGE);
}

ConfigPanel::~ConfigPanel() {
	if (c)
		c->unsubscribe(this);
}


void ConfigPanel::__init__(Module *_c) {
	new(this) ConfigPanel(_c);
}

void ConfigPanel::__delete__() {
	this->ConfigPanel::~ConfigPanel();
}

void ConfigPanel::changed() {
	ignore_change = true;
	c->changed();
	ignore_change = false;
}




class ConfigurationDialog : public hui::Dialog {
public:
	ConfigurationDialog(Module *m, hui::Window *parent) :
		hui::Dialog("configurable_dialog", parent)
	{
		module = m;
		module_panel = new ModulePanel(module, this, ModulePanel::Mode::DEFAULT_S);
		set_title(module->module_subtype);
		set_size(CONFIG_PANEL_WIDTH, 300);
		embed(module_panel, "content", 0, 0);
		module->subscribe(this, [=]{
			module = nullptr;
			destroy();
		}, module->MESSAGE_DELETE);

		ok = false;


		if (module->module_type != ModuleType::AUDIO_EFFECT)
			hide_control("preview", true);

		event("ok", [=]{ on_ok(); });
		event("cancel", [=]{ destroy(); });
		event("preview", [=]{ on_preview(); });
	}
	~ConfigurationDialog() {
		if (module)
			module->unsubscribe(this);
	}
	void on_ok() {
		ok = true;
		destroy();
	}
	void on_preview() {
		preview_start();
	}

	void preview_start() {
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



bool configure_module(hui::Window *win, Module *m) {
	auto *config = m->get_config();
	if (!config)
		return true;

	auto *dlg = new ConfigurationDialog(m, win);
	dlg->run();
	bool ok = dlg->ok;
	delete dlg;
	return ok;
}

