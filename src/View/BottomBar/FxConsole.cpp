/*
 * FxConsole.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxConsole.h"
#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../../Plugins/Effect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"

class SingleFxPanel : public HuiPanel, public Observer
{
public:
	SingleFxPanel(AudioFile *a, Track *t, Effect *_fx, int _index)
	{
		audio = a;
		track = t;
		fx = _fx;
		index = _index;
		AddControlTable("!noexpandx,expandy", 0, 0, 1, 2, "grid");
		SetTarget("grid", 0);
		AddControlTable("", 0, 0, 4, 1, "header");
		SetTarget("header", 0);
		AddText("!bold,center,expandx\\" + fx->name, 0, 0, 0, 0, "");
		AddButton("!flat", 1, 0, 0, 0, "load_favorite");
		SetImage("load_favorite", "hui:open");
		SetTooltip("load_favorite", _("Parameter laden"));
		AddButton("!flat", 2, 0, 0, 0, "save_favorite");
		SetImage("save_favorite", "hui:save");
		SetTooltip("save_favorite", _("Parameter speichern"));
		AddButton("!flat", 3, 0, 0, 0, "delete");
		SetImage("delete", "hui:delete");
		SetTooltip("delete", _("Effekt l&oschen"));
		HuiPanel *p = fx->CreatePanel();
		if (p){
			Embed(p, "grid", 0, 1);
		}else{
			SetTarget("grid", 0);
			AddText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			HideControl("load_favorite", true);
			HideControl("save_favorite", true);
		}

		EventM("delete", this, &SingleFxPanel::onDelete);
		EventM("load_favorite", this, &SingleFxPanel::onLoad);
		EventM("save_favorite", this, &SingleFxPanel::onSave);

		old_param = fx->ConfigToString();
		Subscribe(fx);
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
	void onDelete()
	{
		if (track)
			track->DeleteEffect(index);
		else
			audio->DeleteEffect(index);
	}
	virtual void OnUpdate(Observable *o, const string &message)
	{
		//msg_write("SingleFxPanel: " + message);
		if (message == "Change"){
			if (track)
				track->EditEffect(index, old_param);
			else
				audio->EditEffect(index, old_param);
		}
		fx->UpdateDialog();
		old_param = fx->ConfigToString();
	}
	AudioFile *audio;
	Track *track;
	Effect *fx;
	string old_param;
	int index;
};

FxConsole::FxConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Effekte"))
{
	view = _view;
	audio = _audio;
	id_inner = "mixing_inner_table";

	AddControlTable("!expandy", 0, 0, 1, 30, id_inner);
	SetTarget(id_inner, 0);
	AddButton("!expandy,flat", 29, 0, 0, 0, "add");
	SetImage("add", "hui:add");
	SetTooltip("add", _("neuen Effekt hinzuf&ugen"));

	track = NULL;
	//Enable("add", false);

	EventM("add", (HuiPanel*)this, (void(HuiPanel::*)())&FxConsole::OnAdd);

	Subscribe(view, "CurTrackChange");
	Subscribe(audio, "AddEffect");
	Subscribe(audio, "DeleteEffect");
}

FxConsole::~FxConsole()
{
	Clear();
	Unsubscribe(view);
	Unsubscribe(audio);
}

void FxConsole::OnAdd()
{
	if (!HuiFileDialogOpen(win, _("einen Effekt w&ahlen"), HuiAppDirectoryStatic + "Plugins/Buffer/", "*.kaba", "*.kaba"))
		return;

	string name = HuiFilename.basename(); // remove directory
	name = name.substr(0, name.num - 5); //      and remove ".kaba"
	Effect *effect = CreateEffect(name);
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
		Subscribe(track, "Delete");
		Subscribe(track, "AddEffect");
		Subscribe(track, "DeleteEffect");
	}


	Array<Effect*> fx;
	if (track)
		fx = track->fx;
	else
		fx = audio->fx;
	foreachi(Effect *e, fx, i){
		panels.add(new SingleFxPanel(audio, track, e, i));
		Embed(panels.back(), id_inner, i*2, 0);
		AddSeparator("!vertical", i*2 + 1, 0, 0, 0, "separator_" + i2s(i));
	}
	//Enable("add", track);
}

void FxConsole::OnUpdate(Observable* o, const string &message)
{
	//msg_write("FxPanel: " + message);
	if ((o == track) && (message == "Delete"))
		SetTrack(NULL);
	else if ((o == view) && (message == "CurTrackChange"))
		SetTrack(view->cur_track);
	else
		SetTrack(track);
}

