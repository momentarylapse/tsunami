/*
 * LevelConsole.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */


#include "../../Tsunami.h"
#include "../../Stuff/Observer.h"
#include "../../Data/AudioFile.h"
#include "../../View/AudioView.h"
#include "LevelConsole.h"


LevelConsole::LevelConsole(AudioFile *a, AudioView *v) :
	BottomBarConsole(_("Ebenen")),
	Observer("LevelConsole")
{
	audio = a;
	view = v;

	// dialog
	setBorderWidth(5);
	embedDialog("level_dialog", 0, 0);
	setDecimals(1);

	loadData();

	eventX("levels", "hui:select", this, &LevelConsole::onSelect);
	eventX("levels", "hui:change", this, &LevelConsole::onEdit);
	event("add_level", this, &LevelConsole::onAdd);
	event("delete_level", this, &LevelConsole::onDelete);

	subscribe(audio);
	subscribe(view, view->MESSAGE_CUR_LEVEL_CHANGE);
}

LevelConsole::~LevelConsole()
{
	unsubscribe(audio);
	unsubscribe(view);
}

void LevelConsole::loadData()
{
	reset("levels");
	foreachi(string &n, audio->level_names, i)
		addString("levels", i2s(i + 1) + "\\" + n);
	if (audio->level_names.num > 0)
		setInt("levels", view->cur_level);
}


void LevelConsole::onSelect()
{
	int s = getInt("levels");
	view->setCurLevel(s);
}

void LevelConsole::onEdit()
{
	int r = HuiGetEvent()->row;
	if (r < 0)
		return;
	audio->renameLevel(r, getCell("levels", r, 1));
}

void LevelConsole::onAdd()
{
	audio->addLevel("");
}

void LevelConsole::onDelete()
{
	int s = getInt("levels");
	if (s >= 0)
		audio->deleteLevel(s, false);
}

void LevelConsole::onUpdate(Observable *o, const string &message)
{
	loadData();
}
