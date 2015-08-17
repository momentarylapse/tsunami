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



BarList::BarList(HuiPanel *_panel, const string & _id, const string &_id_add, const string &_id_add_pause, const string &_id_delete, const string &_id_edit, AudioView *_view) :
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

	track = NULL;

	panel->check(id_link, true);

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
}



void BarList::fillList()
{
	msg_db_f("FillBarList", 1);
	panel->reset(id);
	if (track){
		int sample_rate = track->song->sample_rate;
		int n = 1;
		foreach(BarPattern &b, track->bars){
			float duration = (float)b.length / (float)sample_rate;
			if (b.type == b.TYPE_BAR){
				panel->addString(id, format("%d\\%d\\%.1f\\%.3f", n, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), duration));
				n ++;
			}else if (b.type == b.TYPE_PAUSE){
				panel->addString(id, format(_("(Pause)\\-\\-\\%.3f"), duration));
			}
		}
	}
	//panel->enable(id_delete, false);
	onListSelect();
}



void BarList::onList()
{
	int s = panel->getInt(id);
	if (s >= 0){
		executeBarDialog(s);
		fillList();
	}
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
	if (!track)
		return;
	Array<int> s = panel->getSelection(id);

	if (s.num > 0){
		int pos = 0;
		int p0 = 0, p1 = 0;
		foreachi(BarPattern &b, track->bars, i){
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
	if (!track)
		return;
	int sample_rate = track->song->sample_rate;
	int index = HuiGetEvent()->row;
	BarPattern b = track->bars[index];
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
	track->editBar(index, b, panel->isChecked(id_link));
	fillList();
}


void BarList::onAdd()
{
	addNewBar();
}


void BarList::onAddPause()
{
	if (!track)
		return;
	int s = panel->getInt(id);

	track->addPause(s, 2.0f, panel->isChecked(id_link));
	fillList();
}


void BarList::onDelete()
{
	if (!track)
		return;
	Array<int> s = panel->getSelection(id);
	track->song->action_manager->beginActionGroup();

	foreachb(int i, s){

		int pos = 0;
		for (int j=0; j<i; j++)
			pos += track->bars[j].length;

		BarPattern b = track->bars[i];
		int l0 = b.length;
		if (panel->isChecked(id_link)){
			foreach(Track *t, track->song->tracks){
				if (t->type != t->TYPE_MIDI)
					continue;
				Set<int> del;
				Array<MidiEvent> add;
				foreachi(MidiEvent &e, t->midi, j){
					if (e.pos <= pos){
					}else if (e.pos >= pos + l0){
						MidiEvent e2 = e;
						e2.pos -= l0;
						add.add(e2);
						del.add(j);
					}else{
						del.add(j);
					}
				}
				foreachb(int j, del)
					t->deleteMidiEvent(j);
				foreach(MidiEvent &e, add)
					t->addMidiEvent(e);
			}
		}
		track->deleteBar(i, panel->isChecked(id_link));
	}
	track->song->action_manager->endActionGroup();
	fillList();
}

class BarEditDialog : public HuiDialog
{
public:
	Track *track;
	Array<int> sel;
	bool apply_to_midi;

	BarEditDialog(HuiWindow *root, Track *_t, Array<int> &_s, bool _apply_to_midi):
		HuiDialog("", 100, 100, root, false)
	{
		fromResource("bar_edit_dialog");
		track = _t;
		sel = _s;
		apply_to_midi = _apply_to_midi;

		BarPattern &b = track->bars[sel[0]];
		setInt("beats", b.num_beats);
		setFloat("bpm", track->song->sample_rate * 60.0f / (b.length / b.num_beats));
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
		track->song->action_manager->beginActionGroup();
		foreachb(int i, sel){
			BarPattern b = track->bars[i];
			if (edit_beats)
				b.num_beats = beats;
			if (edit_bpm)
				b.length = track->song->sample_rate * 60.0f * b.num_beats / bpm;
			track->editBar(i, b, apply_to_midi);
		}
		track->song->action_manager->endActionGroup();

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
	Track *track;
	int index;
	bool apply_to_midi;

	BarAddDialog(HuiWindow *root, Track *_t, int _index, bool _apply_to_midi):
		HuiDialog("", 100, 100, root, false)
	{
		fromResource("bar_add_dialog");
		track = _t;
		index = _index;
		apply_to_midi = _apply_to_midi;

		setInt("count", 1);
		setInt("beats", 4);
		setFloat("bpm", 90.0f);
		if (track->bars.num > 0){
			BarPattern &b = track->bars.back();
			if (index >= 0)
				b = track->bars[index];
			setInt("beats", b.num_beats);
			setFloat("bpm", track->song->sample_rate * 60.0f / (b.length / b.num_beats));
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
		track->song->action_manager->beginActionGroup();


		for (int i=0; i<count; i++)
			track->addBar(index, bpm, beats, apply_to_midi);
		track->song->action_manager->endActionGroup();

		delete(this);
	}

	void onClose()
	{
		delete(this);
	}
};

void BarList::onEdit()
{
	if (!track)
		return;
	Array<int> s = panel->getSelection(id);

	HuiDialog *dlg = new BarEditDialog(panel->win, track, s, panel->isChecked(id_link));
	dlg->show();

	fillList();
	panel->setSelection(id, s);
	selectToView();
}

BarList::~BarList()
{
	if (view)
		unsubscribe(view);
}

void BarList::addNewBar()
{
	if (!track)
		return;
	msg_db_f("AddNewBar", 1);

	int s = panel->getInt(id);


	HuiDialog *dlg = new BarAddDialog(panel->win, track, s, panel->isChecked(id_link));
	dlg->show();
}

void BarList::executeBarDialog(int index)
{
	msg_db_f("executeBarDialog", 1);
}

void BarList::setTrack(Track *t)
{
	track = NULL;
	if (t)
		if (t->type == t->TYPE_TIME)
			track = t;
	fillList();

	panel->enable(id, track);
	panel->enable(id_add, track);
	panel->enable(id_add_pause, track);
}

void BarList::onUpdate(Observable *o, const string &message)
{
	if (!track)
		return;

	selectFromView();
}

void BarList::selectFromView()
{
	Array<int> s;
	int pos = 0;
	foreachi(BarPattern &b, track->bars, i){
		Range r = Range(pos, b.length - 1);
		if (r.overlaps(view->sel_range))
			s.add(i);
		pos += b.length;
	}
	panel->setSelection(id, s);
	panel->enable(id_delete, s.num > 0);
	panel->enable(id_edit, s.num > 0);
}

