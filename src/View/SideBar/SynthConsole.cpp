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
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"



class SynthPanel : public hui::Panel
{
public:
	SynthPanel(Track *t)
	{
		track = t;
		synth = t->synth;
		fromResource("synth_panel");
		setString("name", synth->name);
		p = synth->createPanel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			setTarget("grid", 0);
			addLabel(_("not configurable"), 0, 1, 0, 0, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
		}

		event("load_favorite", std::bind(&SynthPanel::onLoad, this));
		event("save_favorite", std::bind(&SynthPanel::onSave, this));

		old_param = synth->configToString();
		synth->subscribe_old2(this, SynthPanel, synth->MESSAGE_CHANGE);
		synth->subscribe_old2(this, SynthPanel, synth->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SynthPanel()
	{
		synth->unsubscribe(this);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, synth, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(synth, name);
		track->editSynthesizer(old_param);
		old_param = synth->configToString();
	}
	void onSave()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, synth, true);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->SaveFavorite(synth, name);
	}
	virtual void onUpdate(Observable *o)
	{
		if (o->cur_message() == o->MESSAGE_CHANGE){
			track->editSynthesizer(old_param);
		}
		if (p)
			p->update();
		old_param = synth->configToString();
	}
	Track *track;
	Synthesizer *synth;
	ConfigPanel *p;
	string old_param;
};

SynthConsole::SynthConsole(AudioView *_view) :
	SideBarConsole(_("Synthesizer"))
{
	view = _view;
	id_inner = "grid";

	fromResource("synth_console");

	event("select", std::bind(&SynthConsole::onSelect, this));
	event("detune", std::bind(&SynthConsole::onDetune, this));

	event("edit_song", std::bind(&SynthConsole::onEditSong, this));
	event("edit_track", std::bind(&SynthConsole::onEditTrack, this));

	track = NULL;
	panel = NULL;

	view->subscribe_old2(this, SynthConsole, view->MESSAGE_CUR_TRACK_CHANGE);
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
	Synthesizer *s = tsunami->plugin_manager->ChooseSynthesizer(win, track->song, track->synth->name);
	if (s)
		track->setSynthesizer(s);
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
		if (track->synth){
			track->synth->unsubscribe(this);
			delete(panel);
			panel = NULL;
			removeControl("separator_0");
		}
	}
	track = NULL;
}

void SynthConsole::setTrack(Track *t)
{
	clear();
	track = t;
	if (!track)
		return;

	track->subscribe_old2(this, SynthConsole, track->MESSAGE_DELETE);
	track->subscribe_old2(this, SynthConsole, track->MESSAGE_CHANGE);

	if (track->synth){
		track->synth->subscribe_old2(this, SynthConsole, track->synth->MESSAGE_DELETE);
		panel = new SynthPanel(track);
		embed(panel, id_inner, 0, 0);
		addSeparator("!horizontal", 0, 1, 0, 0, "separator_0");
	}
}

void SynthConsole::onUpdate(Observable* o)
{
	if ((o->getName() == "Synthesizer") and (o->cur_message() == o->MESSAGE_DELETE)){
		clear();
	}else if ((o == track) and (o->cur_message() == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) and (o->cur_message() == view->MESSAGE_CUR_TRACK_CHANGE))
		setTrack(view->cur_track);
	else
		setTrack(track);
}

