/*
 * EditStringsDialog.cpp
 *
 *  Created on: 11.02.2016
 *      Author: michi
 */

#include "EditStringsDialog.h"
#include "../../data/Track.h"

namespace tsunami {

EditStringsDialog::EditStringsDialog(hui::Window *_parent, const Array<int> &_strings) :
	hui::Dialog("edit-strings-dialog", _parent)
{
	strings = _strings;

	gui_num_strings = 0;

	update();

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
	event("add-first", [this] { on_add_first(); });
}

void EditStringsDialog::update() {
	for (int i=strings.num; i<gui_num_strings; i++) {
		string id = format("string%d", i);
		remove_control(id);
		remove_control(id + "_label");
		remove_control("delete_" + id);
		remove_control("add_" + id);
	}
	hide_control("add-first", strings.num > 0);

	set_target("g-strings");
	foreachi(int t, strings, i) {
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
			for (int p=MaxPitch-1; p>=0; p--)
				set_string(id, pitch_name(p));
		}
		set_int(id, MaxPitch - 1 - t);
	}

	gui_num_strings = strings.num;
}

void EditStringsDialog::on_ok() {
	ok = true;
	request_destroy();
}

void EditStringsDialog::on_edit() {
	string id = hui::get_event()->id;
	int n = id.sub(6)._int();
	int p = MaxPitch - 1 - get_int(id);
	strings[n] = p;
}

void EditStringsDialog::on_delete() {
	string id = hui::get_event()->id;
	int n = id.sub(7+6)._int();
	strings.erase(n);

	hui::run_later(0.001f, [this] { update(); });
}

void EditStringsDialog::on_add() {
	string id = hui::get_event()->id;
	int n = id.sub(4+6)._int();
	int pitch = strings[n];
	strings.insert(pitch, n);

	hui::run_later(0.001f, [this] { update(); });
}

void EditStringsDialog::on_add_first() {
	strings.add(69);

	hui::run_later(0.001f, [this] { update(); });
}

}

