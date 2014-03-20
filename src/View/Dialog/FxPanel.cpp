/*
 * FxPanel.cpp
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#include "FxPanel.h"
#include "../../Data/Track.h"
#include "../../Plugins/Effect.h"

FxPanel::FxPanel()
{
	id_inner = "mixing_inner_table";

	AddControlTable("!height=250,noexpandy", 0, 0, 2, 1, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 1, 2, "button_grid");
	AddGroup(_("Effekte"), 1, 0, 0, 0, "group");
	SetTarget("button_grid", 0);
	AddButton("!noexpandy", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddButton("!noexpandy", 0, 1, 0, 0, "add");
	SetImage("add", "hui:add");
	SetTarget("group", 0);
	AddControlTable("", 0, 0, 1, 20, id_inner);

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
}

void FxPanel::Clear()
{
	foreach(HuiPanel *p, panels)
		delete(p);
	panels.clear();
	track = NULL;
}

void FxPanel::SetTrack(Track *t)
{
	Clear();
	track = t;

	foreach(Effect *fx, track->fx){
		HuiPanel *p = new HuiPanel;
		p->AddGroup("!noexpandx\\" + fx->name, 0, 0, 0, 0, "group");
		p->SetTarget("group", 0);
		p->AddControlTable("", 0, 0, 1, 2, "grid");

		p->Embed(fx->CreatePanel(), "grid", 0, 1);
		panels.add(p);
		Embed(panels.back(), id_inner, panels.num - 1, 0);
	}
}

