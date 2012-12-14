/*
 * BarList.cpp
 *
 *  Created on: 03.12.2012
 *      Author: michi
 */

#include "BarList.h"
#include "../../Tsunami.h"



BarList::BarList(CHuiWindow *_dlg, const string & _id, const string &_id_add, const string &_id_add_pause, const string &_id_delete)
{
	dlg = _dlg;
	id = _id;
	id_add = _id_add;
	id_add_pause = _id_add_pause;
	id_delete = _id_delete;

	bar = NULL;
	sample_rate = 1;

	FillList();
	dlg->EventM(id, this, (void(HuiEventHandler::*)())&BarList::OnList);
	dlg->EventMX(id, "hui:select", this, (void(HuiEventHandler::*)())&BarList::OnListSelect);
	dlg->EventMX(id, "hui:change", this, (void(HuiEventHandler::*)())&BarList::OnListEdit);
	dlg->EventM(id_add, this, (void(HuiEventHandler::*)())&BarList::OnAdd);
	dlg->EventM(id_add_pause, this, (void(HuiEventHandler::*)())&BarList::OnAddPause);
	dlg->EventM(id_delete, this, (void(HuiEventHandler::*)())&BarList::OnDelete);
}



void BarList::FillList()
{
	msg_db_r("FillBarList", 1);
	dlg->Reset(id);
	if (bar){
		int n = 1;
		foreach(Bar &b, *bar){
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
	if (!bar)
		return;
	Bar &b = (*bar)[HuiGetEvent()->row];
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
	FillList();
}


void BarList::OnAdd()
{
	AddNewBar();
}


void BarList::OnAddPause()
{
	if (!bar)
		return;
	int s = dlg->GetInt(id);

	Bar b;
	b.num_beats = 1;
	b.type = b.TYPE_PAUSE;
	b.length = (int)((float)sample_rate * 2.0f);
	b.count = 1;
	if (s >= 0)
		bar->insert(b, s + 1);
	else
		bar->add(b);
	FillList();
}


void BarList::OnDelete()
{
	if (!bar)
		return;
	int s = dlg->GetInt(id);
	if (s >= 0){
		bar->erase(s);
		FillList();
	}
}

BarList::~BarList()
{
}

void BarList::AddNewBar()
{
	if (!bar)
		return;
	msg_db_r("AddNewBar", 1);

	int s = dlg->GetInt(id);

	Bar b;
	b.num_beats = 4;
	b.type = b.TYPE_BAR;
	b.length = (int)((float)b.num_beats * (float)sample_rate * 60.0f / 90.0f);
	b.count = 1;
	if (s >= 0)
		bar->insert(b, s + 1);
	else
		bar->add(b);
	FillList();

	msg_db_l(1);
}

void BarList::ExecuteBarDialog(int index)
{
	msg_db_r("ExecuteBarDialog", 1);

	//UpdateEffectParams(fx[index]);

	msg_db_l(1);
}

void BarList::SetBar(Array<Bar>* _bar, int _sample_rate)
{
	bar = _bar;
	sample_rate = _sample_rate;
	FillList();

	dlg->Enable(id, bar);
	dlg->Enable(id_add, bar);
	dlg->Enable(id_add_pause, bar);
}

