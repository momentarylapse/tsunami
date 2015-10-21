/*
 * BarsConsole.cpp
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#include "../Helper/BarList.h"
#include "BarsConsole.h"

BarsConsole::BarsConsole(Song *song, AudioView *view) :
	SideBarConsole(_("Takte")),
	Observer("BarsConsole")
{
	setBorderWidth(5);
	fromResource("bars_dialog");
	setOptions("ttd_grid_1", "noexpandx,width=300");
	setDecimals(1);
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar", "edit_selected_bars", song, view);


	event("edit_song", this, &BarsConsole::onEditSong);
}

BarsConsole::~BarsConsole()
{
	delete(bar_list);
}

void BarsConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void BarsConsole::onUpdate(Observable *o, const string &message)
{
}
