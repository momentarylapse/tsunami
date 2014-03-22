/*
 * FxPanel.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxPanel.h"
#include "../../Data/Track.h"
#include "../../Plugins/Effect.h"

class SingleFxPanel : public HuiPanel, public Observer
{
public:
	SingleFxPanel(Track *t, Effect *_fx, int _index)
	{
		track = t;
		fx = _fx;
		index = _index;
		AddControlTable("!noexpandx", 0, 0, 1, 2, "grid");
		SetTarget("grid", 0);
		AddControlTable("", 0, 0, 5, 1, "header");
		SetTarget("header", 0);
		AddText("!bold,center,expandx\\" + fx->name, 0, 0, 0, 0, "");
		AddButton("!flat", 1, 0, 0, 0, "clear");
		SetImage("clear", "hui:clear");
		SetTooltip("clear", _("auf Standard Parameter zur&ucksetzen"));
		AddButton("!flat", 2, 0, 0, 0, "load");
		SetImage("load", "hui:open");
		SetTooltip("load", _("Parameter aus Favoriten laden"));
		AddButton("!flat", 3, 0, 0, 0, "save");
		SetImage("save", "hui:save");
		SetTooltip("save", _("Parameter in Favoriten speichern"));
		AddButton("!flat", 4, 0, 0, 0, "delete");
		SetImage("delete", "hui:delete");
		SetTooltip("delete", _("Effekt l&oschen"));
		HuiPanel *p = fx->CreatePanel();
		if (p){
			Embed(p, "grid", 0, 1);
		}else{
			SetTarget("grid", 0);
			AddText(_("nicht konfigurierbar"), 0, 1, 0, 0, "");
			HideControl("clear", true);
			HideControl("load", true);
			HideControl("save", true);
		}

		EventM("delete", this, &SingleFxPanel::onDelete);
		EventM("clear", this, &SingleFxPanel::onClear);

		old_param = fx->ConfigToString();
		Subscribe(fx);
	}
	virtual ~SingleFxPanel()
	{
		Unsubscribe(fx);
	}
	void onClear()
	{
		fx->ResetConfig();
		track->EditEffect(index, old_param);
		old_param = fx->ConfigToString();
	}
	void onDelete()
	{
		track->DeleteEffect(index);
	}
	virtual void OnUpdate(Observable *o, const string &message)
	{
		//msg_write("SingleFxPanel: " + message);
		if (message == "Change")
			track->EditEffect(index, old_param);
		fx->UpdateDialog();
		old_param = fx->ConfigToString();
	}
	Track *track;
	Effect *fx;
	string old_param;
	int index;
};

FxPanel::FxPanel(AudioFile *_audio) :
	Observable("FxConsole")
{
	audio = _audio;
	id_inner = "mixing_inner_table";

	AddControlTable("!height=250,noexpandy", 0, 0, 2, 1, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 1, 3, "button_grid");
	AddControlTable("", 1, 0, 1, 20, id_inner);
	SetTarget("button_grid", 0);
	AddButton("!noexpandy,flat", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddButton("!noexpandy,flat", 0, 1, 0, 0, "add");
	SetImage("add", "hui:add");
	AddText("!big,bold,angle=90\\Effekte", 0, 2, 0, 0, "");
	SetTarget("group", 0);

	track = NULL;
	Enable("add", false);

	EventM("close", (HuiPanel*)this, (void(HuiPanel::*)())&FxPanel::OnClose);
	EventM("add", (HuiPanel*)this, (void(HuiPanel::*)())&FxPanel::OnAdd);

	enabled = true;
}

FxPanel::~FxPanel()
{
	Clear();
}

void FxPanel::OnClose()
{
	Show(false);
}

void FxPanel::OnAdd()
{
	if (!HuiFileDialogOpen(win, _("einen Effekt w&ahlen"), HuiAppDirectoryStatic + "Plugins/Buffer/", "*.kaba", "*.kaba"))
		return;

	string name = HuiFilename.basename(); // remove directory
	name = name.substr(0, name.num - 5); //      and remove ".kaba"
	Effect *effect = CreateEffect(name);
	if (track)
		track->AddEffect(effect);
	/*else
		audio->AddEffect(effect);*/
}

void FxPanel::Show(bool show)
{
	enabled = show;
	if (show)
		HuiPanel::Show();
	else
		HuiPanel::Hide();
	Notify("Show");
}

void FxPanel::Clear()
{
	if (track)
		Unsubscribe(track);
	foreachi(HuiPanel *p, panels, i){
		delete(p);
		RemoveControl("separator_" + i2s(i));
	}
	panels.clear();
	track = NULL;
	Enable("add", false);
}

void FxPanel::SetTrack(Track *t)
{
	Clear();
	track = t;
	if (track){
		Subscribe(track, "Delete");
		Subscribe(track, "AddEffect");
		Subscribe(track, "DeleteEffect");

		foreachi(Effect *fx, track->fx, i){
			panels.add(new SingleFxPanel(track, fx, i));
			Embed(panels.back(), id_inner, i*2, 0);
			AddSeparator("!vertical", i*2 + 1, 0, 0, 0, "separator_" + i2s(i));
		}
	}
	Enable("add", track);
}

void FxPanel::OnUpdate(Observable* o, const string &message)
{
	//msg_write("FxPanel: " + message);
	if ((o == track) && (message == "Delete"))
		SetTrack(NULL);
	else
		SetTrack(track);
}

