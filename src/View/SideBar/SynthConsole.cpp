/*
 * SynthConsole.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "SynthConsole.h"
#include "../AudioView.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../Dialog/DetuneSynthesizerDialog.h"
#include "../../Data/Track.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"



class SynthPanel : public hui::Panel
{
public:
	SynthPanel(Session *_session, Track *t)
	{
		session = _session;
		track = t;
		synth = t->synth;
		from_resource("synth_panel");
		set_string("name", synth->module_subtype);
		p = synth->create_panel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			set_target("grid");
			add_label(_("not configurable"), 0, 1, "");
			hide_control("load_favorite", true);
			hide_control("save_favorite", true);
		}

		event("load_favorite", std::bind(&SynthPanel::onLoad, this));
		event("save_favorite", std::bind(&SynthPanel::onSave, this));

		old_param = synth->config_to_string();
		synth->subscribe(this, std::bind(&SynthPanel::onSynthChange, this), synth->MESSAGE_CHANGE);
		synth->subscribe(this, std::bind(&SynthPanel::onSynthChangeByAction, this), synth->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SynthPanel()
	{
		synth->unsubscribe(this);
	}
	void onLoad()
	{
		string name = session->plugin_manager->select_favorite_name(win, synth, false);
		if (name.num == 0)
			return;
		session->plugin_manager->apply_favorite(synth, name);
		track->edit_synthesizer(old_param);
		old_param = synth->config_to_string();
	}
	void onSave()
	{
		string name = session->plugin_manager->select_favorite_name(win, synth, true);
		if (name.num == 0)
			return;
		session->plugin_manager->save_favorite(synth, name);
	}
	void onSynthChange()
	{
		track->edit_synthesizer(old_param);
		if (p)
			p->update();
		old_param = synth->config_to_string();
	}
	void onSynthChangeByAction()
	{
		if (p)
			p->update();
		old_param = synth->config_to_string();
	}
	Session *session;
	Track *track;
	Synthesizer *synth;
	ConfigPanel *p;
	string old_param;
};

SynthConsole::SynthConsole(Session *session) :
	SideBarConsole(_("Synthesizer"), session)
{
	id_inner = "grid";

	from_resource("synth_console");

	event("select", std::bind(&SynthConsole::on_select, this));
	event("detune", std::bind(&SynthConsole::on_detune, this));

	event("edit_song", std::bind(&SynthConsole::on_edit_song, this));
	event("edit_track", std::bind(&SynthConsole::on_edit_track, this));

	track = nullptr;
	panel = nullptr;

	view->subscribe(this, std::bind(&SynthConsole::on_view_cur_track_change, this), view->MESSAGE_CUR_TRACK_CHANGE);
}

SynthConsole::~SynthConsole()
{
	view->unsubscribe(this);
	if (track){
		track->unsubscribe(this);
		if (track->synth)
			track->synth->unsubscribe(this);
	}
}

void SynthConsole::on_select()
{
	if (!track)
		return;
	string name = session->plugin_manager->choose_module(win, session, ModuleType::SYNTHESIZER, track->synth->module_subtype);
	if (name != "")
		track->set_synthesizer(CreateSynthesizer(session, name));
}

void SynthConsole::on_detune()
{
	hui::Dialog *dlg = new DetuneSynthesizerDialog(track->synth, track, view, win);
	dlg->show();
}

void SynthConsole::on_edit_song()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void SynthConsole::on_edit_track()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void SynthConsole::clear()
{
	if (track){
		track->unsubscribe(this);
		if (track->synth and panel){
			track->synth->unsubscribe(this);
			delete(panel);
			panel = nullptr;
			remove_control("separator_0");
		}
	}
	track = nullptr;
}

void SynthConsole::set_track(Track *t)
{
	clear();
	track = t;
	if (!track)
		return;

	track->subscribe(this, std::bind(&SynthConsole::on_track_delete, this), track->MESSAGE_DELETE);
	track->subscribe(this, std::bind(&SynthConsole::on_track_change, this), track->MESSAGE_REPLACE_SYNTHESIZER);

	if (track->synth){
		track->synth->subscribe(this, std::bind(&SynthConsole::on_synth_delete, this), track->synth->MESSAGE_DELETE);
		panel = new SynthPanel(session, track);
		embed(panel, id_inner, 0, 0);
		add_separator("!horizontal", 0, 1, "separator_0");
	}
}

void SynthConsole::on_track_delete()
{
	set_track(nullptr);
}

void SynthConsole::on_track_change()
{
	set_track(track);
}

void SynthConsole::on_synth_delete()
{
	if (track){
		if (track->synth and panel){
			track->synth->unsubscribe(this);
			delete(panel);
			panel = nullptr;
			remove_control("separator_0");
		}
	}
}

void SynthConsole::on_view_cur_track_change()
{
	set_track(view->cur_track());
}

