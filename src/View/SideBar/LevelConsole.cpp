/*
 * LevelConsole.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */


#include "../../Tsunami.h"
#include "../../Stuff/Observer.h"
#include "../../View/AudioView.h"
#include "LevelConsole.h"
#include "../../Data/Song.h"
#include "../../Stuff/Log.h"


LevelConsole::LevelConsole(Song *s, AudioView *v) :
	SideBarConsole(_("Levels")),
	Observer("LevelConsole")
{
	song = s;
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
	event("merge_level", this, &LevelConsole::onMerge);

	event("edit_song", this, &LevelConsole::onEditSong);

	subscribe(song);
	subscribe(view, view->MESSAGE_CUR_LEVEL_CHANGE);
}

LevelConsole::~LevelConsole()
{
	unsubscribe(song);
	unsubscribe(view);
}

void LevelConsole::loadData()
{
	reset("levels");
	foreachi(string &n, song->level_names, i)
		addString("levels", i2s(i + 1) + "\\" + n);
	if (song->level_names.num > 0)
		setInt("levels", view->cur_level);

	enable("delete_level", song->level_names.num > 1);
	enable("merge_level", view->cur_level > 0);
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
	song->renameLevel(r, getCell("levels", r, 1));
}

void LevelConsole::onAdd()
{
	song->addLevel("");
}

void LevelConsole::onDelete()
{
	int s = getInt("levels");
	if (s >= 0)
		song->deleteLevel(s);
}

void LevelConsole::onMerge()
{
	int s = getInt("levels");
	if (s >= 1)
		song->mergeLevels(s, s - 1);
}

void LevelConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void LevelConsole::onUpdate(Observable *o, const string &message)
{
	loadData();
}
