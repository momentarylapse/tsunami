/*
 * BarsConsole.cpp
 *
 *  Created on: 21.10.2015
 *      Author: michi
 */

#include "../../Data/Song.h"
#include "../AudioView.h"
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
	id_link = "link_to_midi";

	song = _song;

	check(id_link, true);
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
	if (s >= 0)
		executeBarDialog(s);
}


void BarsConsole::onListSelect()
{
	Array<int> s = getSelection(id);
	enable(id_delete, s.num > 0);
	enable(id_edit, s.num > 0);

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

class BarEditDialog : public HuiDialog
{
public:
	Song *song;
	Array<int> sel;
	bool apply_to_midi;

	BarEditDialog(HuiWindow *root, Song *_song, Array<int> &_s, bool _apply_to_midi):
		HuiDialog("", 100, 100, root, false)
	{
		fromResource("bar_edit_dialog");
		song = _song;
		sel = _s;
		apply_to_midi = _apply_to_midi;

		BarPattern &b = song->bars[sel[0]];
		setInt("beats", b.num_beats);
		setFloat("bpm", song->sample_rate * 60.0f / (b.length / b.num_beats));
		check("edit_bpm", true);

		event("ok", this, &BarEditDialog::onOk);
		event("cancel", this, &BarEditDialog::onClose);
		event("hui:close", this, &BarEditDialog::onClose);
	}

	void onOk()
	{
		int beats = getInt("beats");
		float bpm = getFloat("bpm");
		bool edit_beats = isChecked("edit_beats");
		bool edit_bpm = isChecked("edit_bpm");
		song->action_manager->beginActionGroup();
		foreachb(int i, sel){
			BarPattern b = song->bars[i];
			if (edit_beats)
				b.num_beats = beats;
			if (edit_bpm)
				b.length = song->sample_rate * 60.0f * b.num_beats / bpm;
			song->editBar(i, b, apply_to_midi);
		}
		song->action_manager->endActionGroup();

		delete(this);
	}

	void onClose()
	{
		delete(this);
	}
};

class BarAddDialog : public HuiDialog
{
public:
	Song *song;
	int index;
	bool apply_to_midi;

	BarAddDialog(HuiWindow *root, Song *_s, int _index, bool _apply_to_midi):
		HuiDialog("", 100, 100, root, false)
	{
		fromResource("bar_add_dialog");
		song = _s;
		index = _index;
		apply_to_midi = _apply_to_midi;

		setInt("count", 1);
		int beats = 4;
		float bpm = 90.0f;
		if (song->bars.num > 0){
			foreachi(BarPattern &b, song->bars, i)
				if ((i <= index) and (b.num_beats > 0)){
					beats = b.num_beats;
					bpm = song->sample_rate * 60.0f / (b.length / b.num_beats);
				}
		}
		setInt("beats", beats);
		setFloat("bpm", bpm);

		event("ok", this, &BarAddDialog::onOk);
		event("cancel", this, &BarAddDialog::onClose);
		event("hui:close", this, &BarAddDialog::onClose);
	}

	void onOk()
	{
		int count = getInt("count");
		int beats = getInt("beats");
		float bpm = getFloat("bpm");
		song->action_manager->beginActionGroup();


		for (int i=0; i<count; i++)
			song->addBar(index, bpm, beats, apply_to_midi);
		song->action_manager->endActionGroup();

		delete(this);
	}

	void onClose()
	{
		delete(this);
	}
};

void BarsConsole::onEdit()
{
	Array<int> s = getSelection(id);

	HuiDialog *dlg = new BarEditDialog(win, song, s, isChecked(id_link));
	dlg->show();

	fillList();
	setSelection(id, s);
	selectToView();
}

void BarsConsole::addNewBar()
{
	int s = getInt(id);


	HuiDialog *dlg = new BarAddDialog(win, song, s, isChecked(id_link));
	dlg->show();
}

void BarsConsole::executeBarDialog(int index)
{
	msg_db_f("executeBarDialog", 1);
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
}



void BarsConsole::onUpdate(Observable *o, const string &message)
{
	if (o == view)
		selectFromView();
	else if (o == song)
		fillList();
	updateMessage();
}
