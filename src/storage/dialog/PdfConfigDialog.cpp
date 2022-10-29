/*
 * PdfConfigDialog.cpp
 *
 *  Created on: 25.08.2018
 *      Author: michi
 */

#include "PdfConfigDialog.h"
#include "../StorageOperationData.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../lib/base/iter.h"

PdfConfigDialog::PdfConfigDialog(StorageOperationData *_od, hui::Window *parent) :
	hui::Dialog("pdf export config", 200, 100, parent, false)
{
	od = _od;
	song = od->song;
	ok = false;

	add_grid("", 0, 0, "root");
	set_target("root");
	add_grid("", 0, 0, "stuff");
	add_grid("", 0, 1, "tracks");
	add_grid("!button-bar", 0, 2, "buttons");
	set_target("buttons");
	add_button(_("Cancel"), 0, 0, "cancel");
	add_button(_("Ok"), 1, 0, "ok");
	set_target("stuff");
	add_label("scale", 0, 0, "");
	add_spin_button("!expandx", 1, 0, "scale");
	add_label("%", 2, 0, "");
	set_target("tracks");
	for (auto&& [i,t]: enumerate(weak(song->tracks))){
		if (t->type != SignalType::MIDI)
			continue;
		add_label(t->nice_name(), 0, i, "");
		add_check_box("Classical", 1, i, format("classical-%d", i));
		add_check_box("TAB", 2, i, format("tab-%d", i));
		check(format("classical-%d", i), true);
		check(format("tab-%d", i), t->instrument.string_pitch.allocated > 0);
		enable(format("tab-%d", i), t->instrument.string_pitch.allocated > 0);
		// TODO read from parameters
	}
	set_float("scale", od->parameters["horizontal-scale"]._float() * 100);

	event("hui:close", [this] { on_close(); });
	event("cancel", [this] { on_close(); });
	event("ok", [this] { on_ok(); });
}

void PdfConfigDialog::on_close() {
	request_destroy();
}

void PdfConfigDialog::on_ok() {
	ok = true;
	Any ats;
	for (auto&& [i,t]: enumerate(weak(song->tracks))){
		if (t->type != SignalType::MIDI)
			continue;
		Any at;
		at.map_set("index", i);
		if (is_checked(format("classical-%d", i)))
			at.map_set("classical", true);
		if (is_checked(format("tab-%d", i)))
			at.map_set("tab", true);
		ats.add(at);
	}
	od->parameters.map_set("tracks", ats);
	od->parameters.map_set("horizontal-scale", get_float("scale") / 100);

	request_destroy();
}

