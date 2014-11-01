/*
 * BarList.cpp
 *
 *  Created on: 03.12.2012
 *      Author: michi
 */

#include "BarList.h"
#include "../../Tsunami.h"
#include "../../Data/AudioFile.h"



BarList::BarList(HuiPanel *_panel, const string & _id, const string &_id_add, const string &_id_add_pause, const string &_id_delete)
{
	panel = _panel;
	id = _id;
	id_add = _id_add;
	id_add_pause = _id_add_pause;
	id_delete = _id_delete;

	track = NULL;

	fillList();
	panel->event(id, this, &BarList::onList);
	panel->eventX(id, "hui:select", this, &BarList::onListSelect);
	panel->eventX(id, "hui:change", this, &BarList::onListEdit);
	panel->event(id_add, this, &BarList::onAdd);
	panel->event(id_add_pause, this, &BarList::onAddPause);
	panel->event(id_delete, this, &BarList::onDelete);
}



void BarList::fillList()
{
	msg_db_f("FillBarList", 1);
	panel->reset(id);
	if (track){
		int sample_rate = track->root->sample_rate;
		int n = 1;
		foreach(BarPattern &b, track->bar){
			if (b.type == b.TYPE_BAR){
				if (b.count == 1)
					panel->addString(id, format("%d\\%d\\%.1f\\%d", n, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), b.count));
				else
					panel->addString(id, format("%d-%d\\%d\\%.1f\\%d", n, n + b.count - 1, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), b.count));
				n += b.count;
			}else if (b.type == b.TYPE_PAUSE){
				panel->addString(id, format(_("(Pause)\\-\\-\\%.3f"), (float)b.length / (float)sample_rate));
			}
		}
	}
	panel->enable(id_delete, false);
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
	int s = panel->getInt(id);
	panel->enable(id_delete, s >= 0);
}


void BarList::onListEdit()
{
	if (!track)
		return;
	int sample_rate = track->root->sample_rate;
	int index = HuiGetEvent()->row;
	BarPattern b = track->bar[index];
	string text = panel->getCell(id, HuiGetEvent()->row, HuiGetEvent()->column);
	if (b.type == b.TYPE_BAR){
		if (HuiGetEvent()->column == 1){
			float l = (float)b.length / (float)b.num_beats;
			b.num_beats = text._int();
			b.length = l * b.num_beats;
		}else if (HuiGetEvent()->column == 2){
			b.length = (int)((float)b.num_beats * (float)sample_rate * 60.0f / text._float());
		}else if (HuiGetEvent()->column == 3){
			b.count = text._int();
		}
	}else if (b.type == b.TYPE_PAUSE){
		if (HuiGetEvent()->column == 3){
			b.length = (int)(text._float() * (float)sample_rate);
		}
	}
	track->EditBar(index, b);
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

	track->AddPause(s, 2.0f);
	fillList();
}


void BarList::onDelete()
{
	if (!track)
		return;
	int s = panel->getInt(id);
	if (s >= 0){
		track->DeleteBar(s);
		fillList();
	}
}

BarList::~BarList()
{
}

void BarList::addNewBar()
{
	if (!track)
		return;
	msg_db_f("AddNewBar", 1);

	int s = panel->getInt(id);

	track->AddBars(s, 90.0f, 4, 10);
	fillList();
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

