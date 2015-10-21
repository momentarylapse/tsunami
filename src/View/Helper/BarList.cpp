/*
 * BarList.cpp
 *
 *  Created on: 03.12.2012
 *      Author: michi
 */

#include "BarList.h"

#include "../../Data/Song.h"
#include "../../Tsunami.h"
#include "../AudioView.h"



BarList::BarList(HuiPanel *_panel, const string & _id, const string &_id_add, const string &_id_add_pause, const string &_id_delete, const string &_id_edit, Song *_song, AudioView *_view) :
	Observer("BarList")
{
	panel = _panel;
	id = _id;
	id_add = _id_add;
	id_add_pause = _id_add_pause;
	id_delete = _id_delete;
	id_edit = _id_edit;
	id_link = "link_to_midi";
	view = _view;

	song = _song;

	panel->check(id_link, true);
	panel->enable(id, true);
	panel->enable(id_add, true);
	panel->enable(id_add_pause, true);

	fillList();
	panel->event(id, this, &BarList::onList);
	panel->eventX(id, "hui:select", this, &BarList::onListSelect);
	panel->eventX(id, "hui:change", this, &BarList::onListEdit);
	panel->event(id_add, this, &BarList::onAdd);
	panel->event(id_add_pause, this, &BarList::onAddPause);
	panel->event(id_delete, this, &BarList::onDelete);
	panel->event(id_edit, this, &BarList::onEdit);

	if (view)
		subscribe(view, view->MESSAGE_SELECTION_CHANGE);
	subscribe(song, song->MESSAGE_EDIT_BARS);
	subscribe(song, song->MESSAGE_NEW);
}



void BarList::fillList()
{
	panel->reset(id);
	int sample_rate = song->sample_rate;
	int n = 1;
	foreach(BarPattern &b, song->bars){
		float duration = (float)b.length / (float)sample_rate;
		if (b.type == b.TYPE_BAR){
			panel->addString(id, format("%d\\%d\\%.1f\\%.3f", n, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), duration));
			n ++;
		}else if (b.type == b.TYPE_PAUSE){
			panel->addString(id, format(_("(Pause)\\-\\-\\%.3f"), duration));
		}
	}
	//panel->enable(id_delete, false);
	onListSelect();
}



void BarList::onList()
{
	int s = panel->getInt(id);
	if (s >= 0)
		executeBarDialog(s);
}


void BarList::onListSelect()
{
	Array<int> s = panel->getSelection(id);
	panel->enable(id_delete, s.num > 0);
	panel->enable(id_edit, s.num > 0);

	selectToView();
}

void BarList::selectToView()
{
	if (!view)
		return;
	Array<int> s = panel->getSelection(id);

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


void BarList::onListEdit()
{
	int sample_rate = song->sample_rate;
	int index = HuiGetEvent()->row;
	BarPattern b = song->bars[index];
	string text = panel->getCell(id, HuiGetEvent()->row, HuiGetEvent()->column);
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
	song->editBar(index, b, panel->isChecked(id_link));
}


void BarList::onAdd()
{
	addNewBar();
}


void BarList::onAddPause()
{
	int s = panel->getInt(id);

	song->addPause(s, 2.0f, panel->isChecked(id_link));
}


void BarList::onDelete()
{
	Array<int> s = panel->getSelection(id);
	song->action_manager->beginActionGroup();

	foreachb(int i, s){

		int pos = 0;
		for (int j=0; j<i; j++)
			pos += song->bars[j].length;

		BarPattern b = song->bars[i];
		int l0 = b.length;
		if (panel->isChecked(id_link)){
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
		song->deleteBar(i, panel->isChecked(id_link));
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
		setInt("beats", 4);
		setFloat("bpm", 90.0f);
		if (song->bars.num > 0){
			BarPattern &b = song->bars.back();
			if (index >= 0)
				b = song->bars[index];
			setInt("beats", b.num_beats);
			setFloat("bpm", song->sample_rate * 60.0f / (b.length / b.num_beats));
		}

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

void BarList::onEdit()
{
	Array<int> s = panel->getSelection(id);

	HuiDialog *dlg = new BarEditDialog(panel->win, song, s, panel->isChecked(id_link));
	dlg->show();

	fillList();
	panel->setSelection(id, s);
	selectToView();
}

BarList::~BarList()
{
	unsubscribe(song);
	if (view)
		unsubscribe(view);
}

void BarList::addNewBar()
{
	int s = panel->getInt(id);


	HuiDialog *dlg = new BarAddDialog(panel->win, song, s, panel->isChecked(id_link));
	dlg->show();
}

void BarList::executeBarDialog(int index)
{
	msg_db_f("executeBarDialog", 1);
}

void BarList::onUpdate(Observable *o, const string &message)
{
	if (o == view)
		selectFromView();
	else if (o == song)
		fillList();
}

void BarList::selectFromView()
{
	Array<int> s;
	int pos = 0;
	foreachi(BarPattern &b, song->bars, i){
		Range r = Range(pos, b.length - 1);
		if (r.overlaps(view->sel_range))
			s.add(i);
		pos += b.length;
	}
	panel->setSelection(id, s);
	panel->enable(id_delete, s.num > 0);
	panel->enable(id_edit, s.num > 0);
}

