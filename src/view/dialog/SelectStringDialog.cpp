/*
 * SelectStringDialog.cpp
 *
 *  Created on: 3 May 2023
 *      Author: michi
 */

#include "SelectStringDialog.h"
#include "../../data/midi/MidiData.h"

SelectStringDialog::SelectStringDialog(hui::Window *parent, const Array<int> &strings) :
	hui::Dialog("select-string-dialog", parent)
{
	set_target("grid-strings");
	foreachi (int s, strings, i) {
		string id = format("string-%d", i);
		add_button("!expandx,width=200\\" + pitch_name(s), 0, strings.num - i - 1, id);
		event(id, [this] { on_string(); });
	}

	event("hui:close", [this] { on_cancel(); });
	event("cancel", [this] { on_cancel(); });
}

void SelectStringDialog::on_string() {
	if (hui::get_event()->id.head(7) == "string-")
		result = hui::get_event()->id.sub_ref(7)._int();
	request_destroy();
}

void SelectStringDialog::on_cancel() {
	request_destroy();
}

