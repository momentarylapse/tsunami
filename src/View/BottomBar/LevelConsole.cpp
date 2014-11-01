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

	fromResource("level_console");

	eventX("levels", "hui:select", this, &LevelConsole::onLevelsSelect);
	eventX("levels", "hui:change", this, &LevelConsole::onLevelsEdit);
	event("add_level", this, &LevelConsole::onAddLevel);
	event("delete_level", this, &LevelConsole::onDeleteLevel);

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
	int s = getInt("levels");
	view->setCurLevel(s);
}

void LevelConsole::onLevelsEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	audio->RenameLevel(r, getCell("levels", r, 1));
}

void LevelConsole::onAddLevel()
{
	audio->AddLevel("");
}

void LevelConsole::onDeleteLevel()
{
	int s = getInt("levels");
	if (s >= 0)
		audio->DeleteLevel(s, false);
}


void LevelConsole::loadData()
{
	reset("levels");
	foreachi(string &n, audio->level_name, i)
		addString("levels", i2s(i + 1) + "\\" + n);
	if (audio->level_name.num > 0)
		setInt("levels", view->cur_level);
}

void LevelConsole::onUpdate(Observable *o, const string &message)
{
	loadData();
}

