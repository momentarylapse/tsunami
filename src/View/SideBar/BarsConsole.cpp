/*
 * BarsConsole.cpp
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#include "../../Data/Song.h"
#include "../AudioView.h"
#include "../Mode/ViewModeBars.h"
#include "../Mode/ViewModeDefault.h"
#include "../Dialog/BarAddDialog.h"
#include "../Dialog/BarEditDialog.h"
#include "BarsConsole.h"

BarsConsole::BarsConsole(Song *_song, AudioView *_view) :
	SideBarConsole(_("Takte")),
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

	setTooltip(id_add, _("neue Takte einf&ugen"));
	setTooltip(id_add_pause, _("Pause einf&ugen"));
	setTooltip(id_delete, _("markierte Takte l&oschen"));
	setTooltip(id_edit, _("markierte Takte editieren"));
	setTooltip(id_scale, _("markierte Takte skalieren (durch Ziehen der Markierung links)"));

	view->mode_bars->modify_midi = true;
	check(id_link, view->mode_bars->modify_midi);
	enable(id, true);
	enable(id_add, true);
	enable(id_add_pause, true);

	fillList();
	event(id, this, &BarsConsole::onList);
	eventX(id, "hui:select", this, &BarsConsole::onListSelect);
	eventX(id, "hui:change", this, &BarsConsole::onListEdit);
	event(id_add, this, &BarsConsole::onAdd);
	event(id_add_pause, this, &BarsConsole::onAddPause);
	event(id_delete, this, &BarsConsole::onDelete);
	event(id_edit, this, &BarsConsole::onEdit);
	event(id_scale, this, &BarsConsole::onScale);
	event(id_link, this, &BarsConsole::onModifyMidi);

	subscribe(view, view->MESSAGE_SELECTION_CHANGE);
	subscribe(song, song->MESSAGE_EDIT_BARS);
	subscribe(song, song->MESSAGE_NEW);
	subscribe(song, song->MESSAGE_ADD_TRACK);
	subscribe(song, song->MESSAGE_DELETE_TRACK);

	updateMessage();

	event("create_time_track", this, &BarsConsole::onCreateTimeTrack);

	event("edit_song", this, &BarsConsole::onEditSong);
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
	song->addTrack(Track::TYPE_TIME);
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
	foreach(BarPattern &b, song->bars){
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
	addNewBar();
}


void BarsConsole::onAddPause()
{
	int s = getInt(id);

	song->addPause(s, 2.0f, isChecked(id_link));
}


void BarsConsole::onDelete()
{
	Array<int> s = getSelection(id);
	song->action_manager->beginActionGroup();

	foreachb(int i, s){

		int pos = 0;
		for (int j=0; j<i; j++)
			pos += song->bars[j].length;

		BarPattern b = song->bars[i];
		int l0 = b.length;
		if (isChecked(id_link)){
			foreach(Track *t, song->tracks){
				if (t->type != t->TYPE_MIDI)
					continue;
				Set<int> del;
				Array<MidiNote> add;
				foreachi(MidiNote &n, t->midi, j){
					if (n.range.end() <= pos){
					}else if (n.range.offset >= pos + l0){
						MidiNote n2 = n;
						n2.range.offset -= l0;
						add.add(n2);
						del.add(j);
					}else{
						del.add(j);
					}
				}
				foreachb(int j, del)
					t->deleteMidiNote(j);
				foreach(MidiNote &n, add)
					t->addMidiNote(n);
			}
		}
		song->deleteBar(i, isChecked(id_link));
	}
	song->action_manager->endActionGroup();
}

void BarsConsole::onEdit()
{
	Array<int> s = getSelection(id);

	HuiDialog *dlg = new BarEditDialog(win, song, s, isChecked(id_link));
	dlg->show();
}

void BarsConsole::onScale()
{
	view->mode_bars->startScaling(getSelection(id));
}

void BarsConsole::onModifyMidi()
{
	view->mode_bars->modify_midi = isChecked(id_link);
}

void BarsConsole::addNewBar()
{
	int s = getInt(id);


	HuiDialog *dlg = new BarAddDialog(win, song, s, isChecked(id_link));
	dlg->show();
}

void BarsConsole::selectFromView()
{
	Array<int> s;
	int pos = 0;
	foreachi(BarPattern &b, song->bars, i){
		Range r = Range(pos, b.length - 1);
		if (r.overlaps(view->sel_range))
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
	view->setMode(view->mode_bars);
}

void BarsConsole::onLeave()
{
	view->setMode(view->mode_default);
}

void BarsConsole::onUpdate(Observable *o, const string &message)
{
	if (o == view){
		if (!view->mode_bars->scaling)
			selectFromView();
	}else if (o == song){
		fillList();
	}
	updateMessage();
}
