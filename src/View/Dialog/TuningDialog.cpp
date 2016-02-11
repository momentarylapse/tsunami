/*
 * TuningDialog.cpp
 *
 *  Created on: 11.02.2016
 *      Author: michi
 */

#include "TuningDialog.h"
#include "../../Data/Track.h"

TuningDialog::TuningDialog(HuiWindow *_parent, Track *t) :
	HuiWindow("tuning_dialog", _parent, false)
{
	track = t;
	tuning = track->instrument.tuning;

	gui_num_strings = 0;

	update();

	event("ok", this, &TuningDialog::onOk);
	event("hui:close", this, &TuningDialog::onClose);
	event("add_first", this, &TuningDialog::onAddFirst);
}

TuningDialog::~TuningDialog()
{
}

void TuningDialog::update()
{
	for (int i=tuning.num; i<gui_num_strings; i++){
		string id = format("string%d", i);
		removeControl(id);
		removeControl(id + "_label");
		removeControl("delete_" + id);
		removeControl("add_" + id);
	}
	removeControl("add_first");

	setTarget("td_g_tuning", 0);
	foreachi(int t, tuning, i){
		string id = format("string%d", i);
		if (i >= gui_num_strings){
			addLabel(i2s(i+1), 0, 100 - i, 0, 0, id + "_label");
			addComboBox("", 1, 100 - i, 0, 0, id);
			addButton("", 2, 100 - i, 0, 0, "delete_" + id);
			setImage("delete_" + id, "hui:delete");
			addButton("", 3, 100 - i, 0, 0, "add_" + id);
			setImage("add_" + id, "hui:add");
			event(id, this, &TuningDialog::onEdit);
			event("delete_" + id, this, &TuningDialog::onDelete);
			event("add_" + id, this, &TuningDialog::onAdd);
			for (int p=0; p<MAX_PITCH; p++)
				setString(id, pitch_name(p));
		}
		setInt(id, t);
	}
	if (tuning.num == 0){
		addButton("", 0, 0, 0, 0, "add_first");
		setImage("add_first", "hui:add");
	}

	gui_num_strings = tuning.num;
}

void TuningDialog::onOk()
{
	Instrument i = track->instrument;
	i.tuning = tuning;
	track->setInstrument(i);
	delete(this);
}

void TuningDialog::onClose()
{
	delete(this);
}

void TuningDialog::onEdit()
{
	string id = HuiGetEvent()->id;
	int n = id.substr(6, -1)._int();
	int p = getInt(id);
	tuning[n] = p;
}

void TuningDialog::onDelete()
{
	string id = HuiGetEvent()->id;
	int n = id.substr(7+6, -1)._int();
	tuning.erase(n);
	update();
}

void TuningDialog::onAdd()
{
	string id = HuiGetEvent()->id;
	int n = id.substr(4+6, -1)._int();
	tuning.insert(tuning[n], n);
	update();
}

void TuningDialog::onAddFirst()
{
	tuning.add(69);
	update();
}

