/*
 * FxPanel.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxPanel.h"
#include "../../Data/Track.h"
#include "../../Plugins/Effect.h"

class SingleFxPanel : public HuiPanel
{
public:
	SingleFxPanel(Effect *_fx, int _index)
	{
		fx = _fx;
		index = _index;

		AddGroup("!noexpandx\\" + fx->name, 0, 0, 0, 0, "group");
		SetTarget("group", 0);
		AddControlTable("", 0, 0, 1, 2, "grid");

		Embed(fx->CreatePanel(), "grid", 0, 1);
	}
	Effect *fx;
	int index;
};

FxPanel::FxPanel()
{
	id_inner = "mixing_inner_table";

	AddControlTable("!height=250,noexpandy", 0, 0, 2, 1, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 1, 3, "button_grid");
	AddControlTable("", 1, 0, 1, 20, id_inner);
	SetTarget("button_grid", 0);
	AddButton("!noexpandy", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddButton("!noexpandy", 0, 1, 0, 0, "add");
	SetImage("add", "hui:add");
	AddText("!big,bold,angle=90\\Effekte", 0, 2, 0, 0, "");
	SetTarget("group", 0);

	track = NULL;
	Enable("add", false);

	EventM("close", (HuiPanel*)this, (void(HuiPanel::*)())&FxPanel::OnClose);
	EventM("add", (HuiPanel*)this, (void(HuiPanel::*)())&FxPanel::OnAdd);
}

FxPanel::~FxPanel()
{
	Clear();
}

void FxPanel::OnClose()
{
	Hide();
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

void FxPanel::Clear()
{
	foreach(HuiPanel *p, panels)
		delete(p);
	panels.clear();
	track = NULL;
	Enable("add", false);
}

void FxPanel::SetTrack(Track *t)
{
	Clear();
	track = t;
	Enable("add", t);

	foreachi(Effect *fx, track->fx, i){
		panels.add(new SingleFxPanel(fx, i));
		Embed(panels.back(), id_inner, panels.num - 1, 0);
	}
}

