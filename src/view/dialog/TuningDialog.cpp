/*
 * TuningDialog.cpp
 *
 *  Created on: 11.02.2016
 *      Author: michi
 */

#include "TuningDialog.h"
#include "../../data/Track.h"

TuningDialog::TuningDialog(hui::Window *_parent, Track *t) :
	hui::Dialog("tuning_dialog", _parent)
{
	track = t;
	tuning = track->instrument.string_pitch;

	gui_num_strings = 0;

	update();

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
	event("add_first", [this] { on_add_first(); });
}

void TuningDialog::update() {
	for (int i=tuning.num; i<gui_num_strings; i++) {
		string id = format("string%d", i);
		remove_control(id);
		remove_control(id + "_label");
		remove_control("delete_" + id);
		remove_control("add_" + id);
	}
	remove_control("add_first");

	set_target("td_g_tuning");
	foreachi(int t, tuning, i) {
		string id = format("string%d", i);
		if (i >= gui_num_strings) {
			add_label(i2s(i+1), 0, 100 - i, id + "_label");
			add_combo_box("", 1, 100 - i, id);
			add_button("", 2, 100 - i, "delete_" + id);
			set_image("delete_" + id, "hui:delete");
			add_button("", 3, 100 - i, "add_" + id);
			set_image("add_" + id, "hui:add");
			event(id, [this] { on_edit(); });
			event("delete_" + id, [this] { on_delete(); });
			event("add_" + id, [this] { on_add(); });

			// reverse order list... nicer gui
			for (int p=MAX_PITCH-1; p>=0; p--)
				set_string(id, pitch_name(p));
		}
		set_int(id, MAX_PITCH - 1 - t);
	}
	if (tuning.num == 0) {
		add_button("", 0, 0, "add_first");
		set_image("add_first", "hui:add");
	}

	gui_num_strings = tuning.num;
}

void TuningDialog::on_ok() {
	Instrument i = track->instrument;
	i.string_pitch = tuning;
	track->set_instrument(i);
	request_destroy();
}

void TuningDialog::on_edit() {
	string id = hui::get_event()->id;
	int n = id.sub(6)._int();
	int p = MAX_PITCH - 1 - get_int(id);
	tuning[n] = p;
}

void TuningDialog::on_delete() {
	string id = hui::get_event()->id;
	int n = id.sub(7+6)._int();
	tuning.erase(n);

	hui::run_later(0.001f, [this] { update(); });
}

void TuningDialog::on_add() {
	string id = hui::get_event()->id;
	int n = id.sub(4+6)._int();
	tuning.insert(tuning[n], n);

	hui::run_later(0.001f, [this] { update(); });
}

void TuningDialog::on_add_first() {
	tuning.add(69);

	hui::run_later(0.001f, [this] { update(); });
}

