/*
 * BarsConsole.cpp
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#include "../../Data/Song.h"
#include "../AudioView.h"
#include "../Mode/ViewModeDefault.h"
#include "../Dialog/BarAddDialog.h"
#include "../Dialog/BarEditDialog.h"
#include "BarsConsole.h"
#include "../Mode/ViewModeScaleBars.h"

BarsConsole::BarsConsole(Song *_song, AudioView *_view) :
	SideBarConsole(_("Bars")),
	Observer("BarsConsole")
{
	song = _song;
	view = _view;
	setBorderWidth(5);
	fromResource("bars_dialog");

	id = "bar_list";
	id_add = "add_bar";
	id_add_pause = "add_bar_pause";
	id_delete = "delete_bar";
	id_edit = "edit_selected_bars";
	id_scale = "scale_selected_bars";
	id_link = "link_to_midi";

	enable(id, true);
	enable(id_add, true);
	enable(id_add_pause, true);

	fillList();
	event(id, std::bind(&BarsConsole::onList, this));
	eventX(id, "hui:select", std::bind(&BarsConsole::onListSelect, this));
	eventX(id, "hui:change", std::bind(&BarsConsole::onListEdit, this));
	event(id_add, std::bind(&BarsConsole::onAdd, this));
	event(id_add_pause, std::bind(&BarsConsole::onAddPause, this));
	event(id_delete, std::bind(&BarsConsole::onDelete, this));
	event(id_edit, std::bind(&BarsConsole::onEdit, this));
	event(id_scale, std::bind(&BarsConsole::onScale, this));
	event(id_link, std::bind(&BarsConsole::onModifyMidi, this));

	subscribe(view, view->MESSAGE_SELECTION_CHANGE);
	subscribe(song, song->MESSAGE_EDIT_BARS);
	subscribe(song, song->MESSAGE_NEW);
	subscribe(song, song->MESSAGE_ADD_TRACK);
	subscribe(song, song->MESSAGE_DELETE_TRACK);

	updateMessage();

	event("create_time_track", std::bind(&BarsConsole::onCreateTimeTrack, this));

	event("edit_song", std::bind(&BarsConsole::onEditSong, this));
}

BarsConsole::~BarsConsole()
{
	unsubscribe(song);
	unsubscribe(view);
}

void BarsConsole::updateMessage()
{
	Track *t = song->getTimeTrack();
	hideControl("bbd_g_no_time_track", t);
}

void BarsConsole::onCreateTimeTrack()
{
	song->addTrack(Track::TYPE_TIME, 0);
}

void BarsConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void BarsConsole::fillList()
{
	reset(id);
	int sample_rate = song->sample_rate;
	int n = 1;
	for (BarPattern &b: song->bars){
		float duration = (float)b.length / (float)sample_rate;
		if (b.type == b.TYPE_BAR){
			addString(id, format("%d\\%d\\%.1f\\%.3f", n, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), duration));
			n ++;
		}else if (b.type == b.TYPE_PAUSE){
			addString(id, format(_("(Pause)\\-\\-\\%.3f"), duration));
		}
	}
	//enable(id_delete, false);
	onListSelect();
}



void BarsConsole::onList()
{
	int s = getInt(id);
	if (s >= 0){}
}


void BarsConsole::onListSelect()
{
	Array<int> s = getSelection(id);
	enable(id_delete, s.num > 0);
	enable(id_edit, s.num > 0);
	enable(id_scale, s.num > 0);

	selectToView();
}

void BarsConsole::selectToView()
{
	if (!view)
		return;
	Array<int> s = getSelection(id);

	if (s.num > 0){
		int pos = 0;
		int p0 = 0, p1 = 0;
		foreachi(BarPattern &b, song->bars, i){
			if (i == s[0])
				p0 = pos;
			pos += b.length;
			if (i == s.back())
				p1 = pos - 1;
		}
		view->sel_raw = Range(p0, p1 - p0);
	}else{
		view->sel_raw = Range::EMPTY;
	}
	allowNotification(false);
	view->updateSelection();
	allowNotification(true);
}


void BarsConsole::onListEdit()
{
	int sample_rate = song->sample_rate;
	int index = HuiGetEvent()->row;
	BarPattern b = song->bars[index];
	string text = getCell(id, HuiGetEvent()->row, HuiGetEvent()->column);
	if (b.num_beats > 0){
		if (HuiGetEvent()->column == 1){
			float l = (float)b.length / (float)b.num_beats;
			b.num_beats = text._int();
			b.length = l * b.num_beats;
		}else if (HuiGetEvent()->column == 2){
			b.length = (int)((float)b.num_beats * (float)sample_rate * 60.0f / text._float());
		}else if (HuiGetEvent()->column == 3){
			b.length = (int)(text._float() * (float)sample_rate);
		}
	}else{
		if (HuiGetEvent()->column == 3){
			b.length = (int)(text._float() * (float)sample_rate);
		}
	}
	song->editBar(index, b, isChecked(id_link));
}


void BarsConsole::onAdd()
{
}


void BarsConsole::onAddPause()
{
}


void BarsConsole::onDelete()
{
}

void BarsConsole::onEdit()
{
}

void BarsConsole::onScale()
{
}

void BarsConsole::onModifyMidi()
{
}

void BarsConsole::addNewBar()
{
}

void BarsConsole::selectFromView()
{
	Array<int> s;
	int pos = 0;
	foreachi(BarPattern &b, song->bars, i){
		Range r = Range(pos, b.length - 1);
		if (r.overlaps(view->sel.range))
			s.add(i);
		pos += b.length;
	}
	setSelection(id, s);
	enable(id_delete, s.num > 0);
	enable(id_edit, s.num > 0);
	enable(id_scale, s.num > 0);
}

void BarsConsole::onEnter()
{
}

void BarsConsole::onLeave()
{
}

void BarsConsole::onUpdate(Observable *o, const string &message)
{
}
