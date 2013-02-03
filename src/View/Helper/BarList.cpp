/*
 * BarList.cpp
 *
 *  Created on: 03.12.2012
 *      Author: michi
 */

#include "BarList.h"
#include "../../Tsunami.h"
#include "../../Action/Track/Bar/ActionTrackAddBar.h"
#include "../../Action/Track/Bar/ActionTrackEditBar.h"
#include "../../Action/Track/Bar/ActionTrackDeleteBar.h"



BarList::BarList(CHuiWindow *_dlg, const string & _id, const string &_id_add, const string &_id_add_pause, const string &_id_delete)
{
	dlg = _dlg;
	id = _id;
	id_add = _id_add;
	id_add_pause = _id_add_pause;
	id_delete = _id_delete;

	track = NULL;

	FillList();
	dlg->EventM(id, this, &BarList::OnList);
	dlg->EventMX(id, "hui:select", this, &BarList::OnListSelect);
	dlg->EventMX(id, "hui:change", this, &BarList::OnListEdit);
	dlg->EventM(id_add, this, &BarList::OnAdd);
	dlg->EventM(id_add_pause, this, &BarList::OnAddPause);
	dlg->EventM(id_delete, this, &BarList::OnDelete);
}



void BarList::FillList()
{
	msg_db_r("FillBarList", 1);
	dlg->Reset(id);
	if (track){
		int sample_rate = track->root->sample_rate;
		int n = 1;
		foreach(Bar &b, track->bar){
			if (b.type == b.TYPE_BAR){
				if (b.count == 1)
					dlg->AddString(id, format("%d\\%d\\%.1f\\%d", n, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), b.count));
				else
					dlg->AddString(id, format("%d-%d\\%d\\%.1f\\%d", n, n + b.count - 1, b.num_beats, sample_rate * 60.0f / (b.length / b.num_beats), b.count));
				n += b.count;
			}else if (b.type == b.TYPE_PAUSE){
				dlg->AddString(id, format(_("(Pause)\\-\\-\\%.3f"), (float)b.length / (float)sample_rate));
			}
		}
		tsunami->ForceRedraw();
	}
	dlg->Enable(id_delete, false);
	msg_db_l(1);
}



void BarList::OnList()
{
	int s = dlg->GetInt(id);
	if (s >= 0){
		ExecuteBarDialog(s);
		FillList();
	}
}


void BarList::OnListSelect()
{
	int s = dlg->GetInt(id);
	dlg->Enable(id_delete, s >= 0);
}


void BarList::OnListEdit()
{
	if (!track)
		return;
	int sample_rate = track->root->sample_rate;
	int index = HuiGetEvent()->row;
	Bar b = track->bar[index];
	string text = dlg->GetCell(id, HuiGetEvent()->row, HuiGetEvent()->column);
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
	track->root->Execute(new ActionTrackEditBar(track, index, b));
	FillList();
}


void BarList::OnAdd()
{
	AddNewBar();
}


void BarList::OnAddPause()
{
	if (!track)
		return;
	int s = dlg->GetInt(id);

	Bar b;
	b.num_beats = 1;
	b.type = b.TYPE_PAUSE;
	b.length = (int)((float)track->root->sample_rate * 2.0f);
	b.count = 1;
	if (s >= 0)
		track->root->Execute(new ActionTrackAddBar(track, s + 1, b));
	else
		track->root->Execute(new ActionTrackAddBar(track, track->bar.num, b));
	FillList();
}


void BarList::OnDelete()
{
	if (!track)
		return;
	int s = dlg->GetInt(id);
	if (s >= 0){
		track->root->Execute(new ActionTrackDeleteBar(track, s));
		FillList();
	}
}

BarList::~BarList()
{
}

void BarList::AddNewBar()
{
	if (!track)
		return;
	msg_db_r("AddNewBar", 1);

	int s = dlg->GetInt(id);

	Bar b;
	b.num_beats = 4;
	b.type = b.TYPE_BAR;
	b.length = (int)((float)b.num_beats * (float)track->root->sample_rate * 60.0f / 90.0f);
	b.count = 1;
	if (s >= 0)
		track->root->Execute(new ActionTrackAddBar(track, s + 1, b));
	else
		track->root->Execute(new ActionTrackAddBar(track, track->bar.num, b));
	FillList();

	msg_db_l(1);
}

void BarList::ExecuteBarDialog(int index)
{
	msg_db_r("ExecuteBarDialog", 1);

	msg_db_l(1);
}

void BarList::SetTrack(Track *t)
{
	track = NULL;
	if (t)
		if (t->type == t->TYPE_TIME)
			track = t;
	FillList();

	dlg->Enable(id, track);
	dlg->Enable(id_add, track);
	dlg->Enable(id_add_pause, track);
}

