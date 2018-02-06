/*
 * TuningDialog.cpp
 *
 *  Created on: 11.02.2016
 *      Author: michi
 */

#include "TuningDialog.h"
#include "../../Data/Track.h"

TuningDialog::TuningDialog(hui::Window *_parent, Track *t) :
	hui::ResourceWindow("tuning_dialog", _parent)
{
	track = t;
	tuning = track->instrument.string_pitch;

	gui_num_strings = 0;

	update();

	event("ok", std::bind(&TuningDialog::onOk, this));
	event("cancel", std::bind(&TuningDialog::destroy, this));
	event("hui:close", std::bind(&TuningDialog::destroy, this));
	event("add_first", std::bind(&TuningDialog::onAddFirst, this));
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

	setTarget("td_g_tuning");
	foreachi(int t, tuning, i){
		string id = format("string%d", i);
		if (i >= gui_num_strings){
			addLabel(i2s(i+1), 0, 100 - i, id + "_label");
			addComboBox("", 1, 100 - i, id);
			addButton("", 2, 100 - i, "delete_" + id);
			setImage("delete_" + id, "hui:delete");
			addButton("", 3, 100 - i, "add_" + id);
			setImage("add_" + id, "hui:add");
			event(id, std::bind(&TuningDialog::onEdit, this));
			event("delete_" + id, std::bind(&TuningDialog::onDelete, this));
			event("add_" + id, std::bind(&TuningDialog::onAdd, this));

			// reverse order list... nicer gui
			for (int p=MAX_PITCH-1; p>=0; p--)
				setString(id, pitch_name(p));
		}
		setInt(id, MAX_PITCH - 1 - t);
	}
	if (tuning.num == 0){
		addButton("", 0, 0, "add_first");
		setImage("add_first", "hui:add");
	}

	gui_num_strings = tuning.num;
}

void TuningDialog::onOk()
{
	Instrument i = track->instrument;
	i.string_pitch = tuning;
	track->setInstrument(i);
	destroy();
}

void TuningDialog::onEdit()
{
	string id = hui::GetEvent()->id;
	int n = id.substr(6, -1)._int();
	int p = MAX_PITCH - 1 - getInt(id);
	tuning[n] = p;
}

void TuningDialog::onDelete()
{
	string id = hui::GetEvent()->id;
	int n = id.substr(7+6, -1)._int();
	tuning.erase(n);
	update();
}

void TuningDialog::onAdd()
{
	string id = hui::GetEvent()->id;
	int n = id.substr(4+6, -1)._int();
	tuning.insert(tuning[n], n);
	update();
}

void TuningDialog::onAddFirst()
{
	tuning.add(69);
	update();
}

