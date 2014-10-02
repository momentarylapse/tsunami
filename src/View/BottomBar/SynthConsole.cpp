/*
 * SynthConsole.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "SynthConsole.h"
#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"



class SynthPanel : public HuiPanel, public Observer
{
public:
	SynthPanel(Track *t) :
		Observer("SynthPanel")
	{
		track = t;
		synth = t->synth;
		AddControlTable("!noexpandx,expandy", 0, 0, 1, 2, "grid");
		SetTarget("grid", 0);
		AddControlTable("", 0, 0, 5, 1, "header");
		SetTarget("header", 0);
		AddButton("!flat", 0, 0, 0, 0, "load_favorite");
		SetImage("load_favorite", "hui:open");
		SetTooltip("load_favorite", _("Parameter laden"));
		AddButton("!flat", 1, 0, 0, 0, "save_favorite");
		SetImage("save_favorite", "hui:save");
		SetTooltip("save_favorite", _("Parameter speichern"));
		AddText("!bold,center,expandx\\" + synth->name, 2, 0, 0, 0, "");
		p = synth->CreatePanel();
		if (p){
			Embed(p, "grid", 0, 1);
			p->update();
		}else{
			SetTarget("grid", 0);
			AddText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			HideControl("load_favorite", true);
			HideControl("save_favorite", true);
		}

		EventM("load_favorite", this, &SynthPanel::onLoad);
		EventM("save_favorite", this, &SynthPanel::onSave);

		old_param = synth->ConfigToString();
		Subscribe(synth, synth->MESSAGE_CHANGE);
		Subscribe(synth, synth->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SynthPanel()
	{
		Unsubscribe(synth);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, (Configurable*)synth, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(synth, name);
		track->EditSynthesizer(old_param);
		old_param = synth->ConfigToString();
	}
	void onSave()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, synth, true);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->SaveFavorite(synth, name);
	}
	virtual void OnUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			track->EditSynthesizer(old_param);
		}
		p->update();
		old_param = synth->ConfigToString();
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

	AddControlTable("!expandy", 0, 0, 1, 32, id_inner);
	SetTarget(id_inner, 0);
	AddText("!angle=90\\...", 0, 0, 0, 0, "track_name");
	AddSeparator("!vertical", 1, 0, 0, 0, "");

	track = NULL;
	panel = NULL;
	Enable("track_name", false);

	Subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
}

SynthConsole::~SynthConsole()
{
	Unsubscribe(view);
	if (track){
		Unsubscribe(track);
		if (track->synth)
			Unsubscribe(track->synth);
	}
}

void SynthConsole::Clear()
{
	if (track){
		Unsubscribe(track);
		if (track->synth){
			Unsubscribe(track->synth);
			delete(panel);
			panel = NULL;
			RemoveControl("separator_0");
		}
	}
	track = NULL;
}

void SynthConsole::SetTrack(Track *t)
{
	Clear();
	track = t;
	if (track){
		Subscribe(track, track->MESSAGE_DELETE);
		Subscribe(track, track->MESSAGE_CHANGE);
		SetString("track_name", format(_("!angle=90\\f&ur die Spur '%s'"), track->GetNiceName().c_str()));

		if (track->synth){
			Subscribe(track->synth, track->synth->MESSAGE_DELETE);
			panel = new SynthPanel(track);
			Embed(panel, id_inner, 2, 0);
			AddSeparator("!vertical", 3, 0, 0, 0, "separator_0");
		}
	}else
		SetString("track_name", _("!angle=90\\keine Spur gew&ahlt"));
}

void SynthConsole::OnUpdate(Observable* o, const string &message)
{
	if ((o->GetName() == "Synthesizer") && (message == o->MESSAGE_DELETE)){
		Clear();
	}else if ((o == track) && (message == track->MESSAGE_DELETE)){
		SetTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE))
		SetTrack(view->cur_track);
	else
		SetTrack(track);
}

