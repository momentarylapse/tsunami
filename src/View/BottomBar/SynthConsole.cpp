/*
 * SynthConsole.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "SynthConsole.h"
#include "../AudioView.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../Data/Track.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"



class SynthPanel : public HuiPanel, public Observer
{
public:
	SynthPanel(Track *t) :
		Observer("SynthPanel")
	{
		track = t;
		synth = t->synth;
		addGrid("!noexpandx,expandy", 0, 0, 1, 2, "grid");
		setTarget("grid", 0);
		addGrid("", 0, 0, 5, 1, "header");
		setTarget("header", 0);
		addButton("!flat", 0, 0, 0, 0, "load_favorite");
		setImage("load_favorite", "hui:open");
		setTooltip("load_favorite", _("Parameter laden"));
		addButton("!flat", 1, 0, 0, 0, "save_favorite");
		setImage("save_favorite", "hui:save");
		setTooltip("save_favorite", _("Parameter speichern"));
		addLabel("!bold,center,expandx\\" + synth->name, 2, 0, 0, 0, "");
		p = synth->createPanel();
		if (p){
			embed(p, "grid", 0, 1);
			p->update();
		}else{
			setTarget("grid", 0);
			addLabel(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			hideControl("load_favorite", true);
			hideControl("save_favorite", true);
		}

		event("load_favorite", this, &SynthPanel::onLoad);
		event("save_favorite", this, &SynthPanel::onSave);

		old_param = synth->configToString();
		subscribe(synth, synth->MESSAGE_CHANGE);
		subscribe(synth, synth->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SynthPanel()
	{
		unsubscribe(synth);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, (Configurable*)synth, false);
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
	virtual void onUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			track->editSynthesizer(old_param);
		}
		p->update();
		old_param = synth->configToString();
	}
	Track *track;
	Synthesizer *synth;
	ConfigPanel *p;
	string old_param;
};

SynthConsole::SynthConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Synthesizer")),
	Observer("SynthConsole")
{
	view = _view;
	audio = _audio;
	id_inner = "grid";

	addGrid("!expandy", 0, 0, 1, 32, id_inner);
	setTarget(id_inner, 0);

	addButton("!expandy,flat", 31, 0, 0, 0, "select");
	setImage("select", "hui:open");
	setTooltip("select", _("Synthesizer w&ahlen"));

	event("select", this, &SynthConsole::onSelect);

	track = NULL;
	panel = NULL;

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
}

SynthConsole::~SynthConsole()
{
	unsubscribe(view);
	if (track){
		unsubscribe(track);
		if (track->synth)
			unsubscribe(track->synth);
	}
}

void SynthConsole::onSelect()
{
	if (!track)
		return;
	Synthesizer *s = ChooseSynthesizer(tsunami->win, track->synth->name);
	if (s)
		track->setSynthesizer(s);
}

void SynthConsole::clear()
{
	if (track){
		unsubscribe(track);
		if (track->synth){
			unsubscribe(track->synth);
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

	subscribe(track, track->MESSAGE_DELETE);
	subscribe(track, track->MESSAGE_CHANGE);

	if (track->synth){
		subscribe(track->synth, track->synth->MESSAGE_DELETE);
		panel = new SynthPanel(track);
		embed(panel, id_inner, 0, 0);
		addSeparator("!vertical", 1, 0, 0, 0, "separator_0");
	}
}

void SynthConsole::onUpdate(Observable* o, const string &message)
{
	if ((o->getName() == "Synthesizer") && (message == o->MESSAGE_DELETE)){
		clear();
	}else if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE))
		setTrack(view->cur_track);
	else
		setTrack(track);
}

