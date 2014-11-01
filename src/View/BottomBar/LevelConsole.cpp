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

	EventMX("levels", "hui:select", this, &LevelConsole::onLevelsSelect);
	EventMX("levels", "hui:change", this, &LevelConsole::onLevelsEdit);
	EventM("add_level", this, &LevelConsole::onAddLevel);
	EventM("delete_level", this, &LevelConsole::onDeleteLevel);

	subscribe(audio);
	subscribe(view, view->MESSAGE_CUR_LEVEL_CHANGE);
}

LevelConsole::~LevelConsole()
{
	unsubscribe(audio);
	unsubscribe(view);
}

void LevelConsole::onLevelsSelect()
{
	int s = GetInt("levels");
	view->SetCurLevel(s);
}

void LevelConsole::onLevelsEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	audio->RenameLevel(r, GetCell("levels", r, 1));
}

void LevelConsole::onAddLevel()
{
	audio->AddLevel("");
}

void LevelConsole::onDeleteLevel()
{
	int s = GetInt("levels");
	if (s >= 0)
		audio->DeleteLevel(s, false);
}


void LevelConsole::loadData()
{
	Reset("levels");
	foreachi(string &n, audio->level_name, i)
		AddString("levels", i2s(i + 1) + "\\" + n);
	if (audio->level_name.num > 0)
		SetInt("levels", view->cur_level);
}

void LevelConsole::onUpdate(Observable *o, const string &message)
{
	loadData();
}

