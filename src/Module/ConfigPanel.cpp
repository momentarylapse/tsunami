/*
 * ConfigPanel.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ConfigPanel.h"
#include "Module.h"
#include "../View/Helper/Progress.h"
#include "../Plugins/PluginManager.h"
#include "../Session.h"

ConfigPanel::ConfigPanel(Module *_c)
{
	c = _c;
}

ConfigPanel::ConfigPanel()
{
	c = nullptr;
}

ConfigPanel::~ConfigPanel()
{
}


void ConfigPanel::__init__(Module *_c)
{
	new(this) ConfigPanel(_c);
}

void ConfigPanel::__delete__()
{
	this->ConfigPanel::~ConfigPanel();
}

void ConfigPanel::changed()
{
	c->changed();
}





class ConfigurationDialog : public hui::Window
{
public:
	ConfigurationDialog(Module *c, ModuleConfiguration *pd, ConfigPanel *p, hui::Window *parent) :
		hui::Window("configurable_dialog", parent)
	{
		config = c;
		panel = p;
		progress = nullptr;
		ok = false;

		set_title(config->module_subtype);
		embed(panel, "grid", 0, 1);

		if (c->module_type != ModuleType::AUDIO_EFFECT)
			hide_control("preview", true);

		event("load_favorite", [=]{ on_load(); });
		event("save_favorite", [=]{ on_save(); });
		event("ok", [=]{ on_ok(); });
		event("preview", [=]{ on_preview(); });
		event("cancel", [=]{ on_close(); });
		event("hui:close", [=]{ on_close(); });

		config->subscribe(this, [=]{ on_config_change(); }, config->MESSAGE_CHANGE);
	}
	~ConfigurationDialog()
	{
		config->unsubscribe(this);
	}
	void on_ok()
	{
		ok = true;
		destroy();
	}
	void on_close()
	{
		destroy();
	}
	void on_preview()
	{
		preview_start();
	}
	void on_load()
	{
		string name = config->session->plugin_manager->select_favorite_name(this, config, false);
		if (name.num == 0)
			return;
		config->session->plugin_manager->apply_favorite(config, name);
		panel->update();
	}
	void on_save()
	{
		string name = config->session->plugin_manager->select_favorite_name(this, config, true);
		if (name.num == 0)
			return;
		config->session->plugin_manager->save_favorite(config, name);
	}

	void on_config_change()
	{
		panel->update();
	}

	void preview_start()
	{
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

	void on_progress_cancel()
	{
		preview_end();
	}

	void on_update_stream()
	{
		/*if (progress){
			int pos = tsunami->win->view->stream->getPos(0); // TODO
			Range r = tsunami->win->view->sel.range;
			progress->set(_("Preview"), (float)(pos - r.offset) / r.length);
			if (!tsunami->win->view->stream->isPlaying())
				previewEnd();
		}*/
	}

	void preview_end()
	{
		/*if (!progress)
			return;
		tsunami->win->view->stream->unsubscribe(this);
		progress->unsubscribe(this);
		tsunami->win->view->stream->stop();
		delete(progress);
		progress = NULL;


		tsunami->win->view->renderer->preview_effect = NULL;*/
	}

	Module *config;
	ConfigPanel *panel;
	Progress *progress;
	bool ok;
};



bool configure_module(hui::Window *win, Module *m)
{
	ModuleConfiguration *config = m->get_config();
	if (!config)
		return true;

	//_auto_panel_ = NULL;
	ConfigPanel *panel = m->create_panel();
	if (!panel)
		return true;
	panel->set_large(true);
	panel->update();
	ConfigurationDialog *dlg = new ConfigurationDialog(m, config, panel, win);
	dlg->run();
	bool ok = dlg->ok;
	delete(dlg);
	return ok;
}

