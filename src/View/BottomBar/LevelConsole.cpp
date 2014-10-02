/*
 * LevelConsole.cpp
 *
 *  Created on: 03.10.2014
 *      Author: michi
 */

#include "LevelConsole.h"
#include "../../Data/AudioFile.h"
#include "../AudioView.h"

LevelConsole::LevelConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Ebenen")),
	Observer("LevelConsole")
{
	view = _view;
	audio = _audio;

	FromResource("level_console");

	EventMX("levels", "hui:select", this, &LevelConsole::OnLevelsSelect);
	EventMX("levels", "hui:change", this, &LevelConsole::OnLevelsEdit);
	EventM("add_level", this, &LevelConsole::OnAddLevel);
	EventM("delete_level", this, &LevelConsole::OnDeleteLevel);

	Subscribe(audio);
	Subscribe(view, view->MESSAGE_CUR_LEVEL_CHANGE);
}

LevelConsole::~LevelConsole()
{
	Unsubscribe(audio);
	Unsubscribe(view);
}

void LevelConsole::OnLevelsSelect()
{
	int s = GetInt("levels");
	view->SetCurLevel(s);
}

void LevelConsole::OnLevelsEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	audio->RenameLevel(r, GetCell("levels", r, 1));
}

void LevelConsole::OnAddLevel()
{
	audio->AddLevel("");
}

void LevelConsole::OnDeleteLevel()
{
	int s = GetInt("levels");
	if (s >= 0)
		audio->DeleteLevel(s, false);
}


void LevelConsole::LoadData()
{
	Reset("levels");
	foreachi(string &n, audio->level_name, i)
		AddString("levels", i2s(i + 1) + "\\" + n);
	if (audio->level_name.num > 0)
		SetInt("levels", view->cur_level);
}

void LevelConsole::OnUpdate(Observable *o, const string &message)
{
	LoadData();
}

