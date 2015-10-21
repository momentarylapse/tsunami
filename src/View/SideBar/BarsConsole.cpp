/*
 * BarsConsole.cpp
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#include "../Helper/BarList.h"
#include "../../Data/Song.h"
#include "BarsConsole.h"

BarsConsole::BarsConsole(Song *_song, AudioView *view) :
	SideBarConsole(_("Takte")),
	Observer("BarsConsole")
{
	song = _song;
	setBorderWidth(5);
	fromResource("bars_dialog");
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar", "edit_selected_bars", song, view);

	subscribe(song, song->MESSAGE_ADD_TRACK);
	subscribe(song, song->MESSAGE_DELETE_TRACK);

	updateMessage();

	event("create_time_track", this, &BarsConsole::onCreateTimeTrack);

	event("edit_song", this, &BarsConsole::onEditSong);
}

BarsConsole::~BarsConsole()
{
	unsubscribe(song);
	delete(bar_list);
}

void BarsConsole::updateMessage()
{
	Track *t = song->getTimeTrack();
	hideControl("bbd_g_no_time_track", t);
}

void BarsConsole::onCreateTimeTrack()
{
	song->addTrack(Track::TYPE_TIME);
}

void BarsConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void BarsConsole::onUpdate(Observable *o, const string &message)
{
	updateMessage();
}
