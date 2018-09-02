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
		fromResource("synth_panel");
		setString("name", synth->module_subtype);
		p = synth->create_panel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			setTarget("grid");
			addLabel(_("not configurable"), 0, 1, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
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
		string name = session->plugin_manager->SelectFavoriteName(win, synth, false);
		if (name.num == 0)
			return;
		session->plugin_manager->ApplyFavorite(synth, name);
		track->editSynthesizer(old_param);
		old_param = synth->config_to_string();
	}
	void onSave()
	{
		string name = session->plugin_manager->SelectFavoriteName(win, synth, true);
		if (name.num == 0)
			return;
		session->plugin_manager->SaveFavorite(synth, name);
	}
	void onSynthChange()
	{
		track->editSynthesizer(old_param);
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

	fromResource("synth_console");

	event("select", std::bind(&SynthConsole::onSelect, this));
	event("detune", std::bind(&SynthConsole::onDetune, this));

	event("edit_song", std::bind(&SynthConsole::onEditSong, this));
	event("edit_track", std::bind(&SynthConsole::onEditTrack, this));

	track = nullptr;
	panel = nullptr;

	view->subscribe(this, std::bind(&SynthConsole::onViewCurTrackChange, this), view->MESSAGE_CUR_TRACK_CHANGE);
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

void SynthConsole::onSelect()
{
	if (!track)
		return;
	string name = session->plugin_manager->ChooseModule(win, session, ModuleType::SYNTHESIZER, track->synth->module_subtype);
	if (name != "")
		track->setSynthesizer(CreateSynthesizer(session, name));
}

void SynthConsole::onDetune()
{
	hui::Dialog *dlg = new DetuneSynthesizerDialog(track->synth, track, view, win);
	dlg->show();
}

void SynthConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void SynthConsole::onEditTrack()
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
			removeControl("separator_0");
		}
	}
	track = nullptr;
}

void SynthConsole::setTrack(Track *t)
{
	clear();
	track = t;
	if (!track)
		return;

	track->subscribe(this, std::bind(&SynthConsole::onTrackDelete, this), track->MESSAGE_DELETE);
	track->subscribe(this, std::bind(&SynthConsole::onTrackChange, this), track->MESSAGE_REPLACE_SYNTHESIZER);

	if (track->synth){
		track->synth->subscribe(this, std::bind(&SynthConsole::onSynthDelete, this), track->synth->MESSAGE_DELETE);
		panel = new SynthPanel(session, track);
		embed(panel, id_inner, 0, 0);
		addSeparator("!horizontal", 0, 1, "separator_0");
	}
}

void SynthConsole::onTrackDelete()
{
	setTrack(nullptr);
}

void SynthConsole::onTrackChange()
{
	setTrack(track);
}

void SynthConsole::onSynthDelete()
{
	if (track){
		if (track->synth and panel){
			track->synth->unsubscribe(this);
			delete(panel);
			panel = nullptr;
			removeControl("separator_0");
		}
	}
}

void SynthConsole::onViewCurTrackChange()
{
	setTrack(view->cur_track());
}

