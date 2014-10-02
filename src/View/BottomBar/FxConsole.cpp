/*
 * FxConsole.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxConsole.h"
#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"

class SingleFxPanel : public HuiPanel, public Observer
{
public:
	SingleFxPanel(AudioFile *a, Track *t, Effect *_fx, int _index) :
		Observer("SingleFxPanel")
	{
		audio = a;
		track = t;
		fx = _fx;
		index = _index;
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
		AddText("!bold,center,expandx\\" + fx->name, 2, 0, 0, 0, "");
		AddCheckBox("", 3, 0, 0, 0, "enabled");
		SetTooltip("enabled", _("aktiv?"));
		AddButton("!flat", 4, 0, 0, 0, "delete");
		SetImage("delete", "hui:delete");
		SetTooltip("delete", _("Effekt l&oschen"));
		p = fx->CreatePanel();
		if (p){
			Embed(p, "grid", 0, 1);
			p->update();
		}else{
			SetTarget("grid", 0);
			AddText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			HideControl("load_favorite", true);
			HideControl("save_favorite", true);
		}

		EventM("enabled", this, &SingleFxPanel::onEnabled);
		EventM("delete", this, &SingleFxPanel::onDelete);
		EventM("load_favorite", this, &SingleFxPanel::onLoad);
		EventM("save_favorite", this, &SingleFxPanel::onSave);

		Check("enabled", fx->enabled);

		old_param = fx->ConfigToString();
		Subscribe(fx, fx->MESSAGE_CHANGE);
		Subscribe(fx, fx->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~SingleFxPanel()
	{
		Unsubscribe(fx);
	}
	void onLoad()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, (Configurable*)fx, false);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->ApplyFavorite(fx, name);
		if (track)
			track->EditEffect(index, old_param);
		else
			audio->EditEffect(index, old_param);
		old_param = fx->ConfigToString();
	}
	void onSave()
	{
		string name = tsunami->plugin_manager->SelectFavoriteName(win, fx, true);
		if (name.num == 0)
			return;
		tsunami->plugin_manager->SaveFavorite(fx, name);
	}
	void onEnabled()
	{
		if (track)
			track->EnableEffect(index, IsChecked(""));
		else
			audio->EnableEffect(index, IsChecked(""));
	}
	void onDelete()
	{
		if (track)
			track->DeleteEffect(index);
		else
			audio->DeleteEffect(index);
	}
	virtual void OnUpdate(Observable *o, const string &message)
	{
		if (message == o->MESSAGE_CHANGE){
			if (track)
				track->EditEffect(index, old_param);
			else
				audio->EditEffect(index, old_param);
		}
		Check("enabled", fx->enabled);
		p->update();
		old_param = fx->ConfigToString();
	}
	AudioFile *audio;
	Track *track;
	Effect *fx;
	string old_param;
	ConfigPanel *p;
	int index;
};

FxConsole::FxConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Effekte")),
	Observer("FxConsole")
{
	view = _view;
	audio = _audio;
	id_inner = "fx_inner_table";

	AddControlTable("!expandy", 0, 0, 1, 32, id_inner);
	SetTarget(id_inner, 0);
	AddText("!angle=90\\...", 0, 0, 0, 0, "track_name");
	AddSeparator("!vertical", 1, 0, 0, 0, "");
	AddText(_("- hier sind (noch) keine Effekte aktiv -"), 30, 0, 0, 0, "comment_no_fx");
	AddButton("!expandy,flat", 31, 0, 0, 0, "add");
	SetImage("add", "hui:add");
	SetTooltip("add", _("neuen Effekt hinzuf&ugen"));

	track = NULL;
	//Enable("add", false);
	Enable("track_name", false);

	EventM("add", this, &FxConsole::OnAdd);

	Subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	Subscribe(audio, audio->MESSAGE_ADD_EFFECT);
	Subscribe(audio, audio->MESSAGE_DELETE_EFFECT);
}

FxConsole::~FxConsole()
{
	Clear();
	Unsubscribe(view);
	Unsubscribe(audio);
}

void FxConsole::OnAdd()
{
	Effect *effect = tsunami->plugin_manager->ChooseEffect(this);
	if (!effect)
		return;
	if (track)
		track->AddEffect(effect);
	else
		audio->AddEffect(effect);
}

void FxConsole::Clear()
{
	if (track)
		Unsubscribe(track);
	foreachi(HuiPanel *p, panels, i){
		delete(p);
		RemoveControl("separator_" + i2s(i));
	}
	panels.clear();
	track = NULL;
	//Enable("add", false);
}

void FxConsole::SetTrack(Track *t)
{
	Clear();
	track = t;
	if (track){
		Subscribe(track, track->MESSAGE_DELETE);
		Subscribe(track, track->MESSAGE_ADD_EFFECT);
		Subscribe(track, track->MESSAGE_DELETE_EFFECT);
		SetString("track_name", format(_("!angle=90\\wirken auf die Spur '%s'"), track->GetNiceName().c_str()));
	}else
		SetString("track_name", _("!angle=90\\wirken auf die komplette Datei"));


	Array<Effect*> fx;
	if (track)
		fx = track->fx;
	else
		fx = audio->fx;
	foreachi(Effect *e, fx, i){
		panels.add(new SingleFxPanel(audio, track, e, i));
		Embed(panels.back(), id_inner, i*2 + 2, 0);
		AddSeparator("!vertical", i*2 + 3, 0, 0, 0, "separator_" + i2s(i));
	}
	HideControl("comment_no_fx", fx.num > 0);
	//Enable("add", track);
}

void FxConsole::OnUpdate(Observable* o, const string &message)
{
	if ((o == track) && (message == track->MESSAGE_DELETE)){
		SetTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE))
		SetTrack(view->cur_track);
	else
		SetTrack(track);
}

