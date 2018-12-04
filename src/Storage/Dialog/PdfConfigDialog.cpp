/*
 * PdfConfigDialog.cpp
 *
 *  Created on: 25.08.2018
 *      Author: michi
 */

#include "PdfConfigDialog.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"

PdfConfigDialog::PdfConfigDialog(PdfConfigData *_data, Song *_song, hui::Window *parent) :
	hui::Dialog("pdf export config", 200, 100, parent, false)
{
	data = _data;
	song = _song;
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
	foreachi(Track *t, song->tracks, i){
		if (t->type != SignalType::MIDI)
			continue;
		add_label(t->nice_name(), 0, i, "");
		add_check_box("Classical", 1, i, format("classical-%d", i));
		add_check_box("TAB", 2, i, format("tab-%d", i));
		check(format("classical-%d", i), true);
		check(format("tab-%d", i), t->instrument.string_pitch.allocated > 0);
		enable(format("tab-%d", i), t->instrument.string_pitch.allocated > 0);
	}
	set_float("scale", 100.0f);

	event("hui:close", [&]{ on_close(); });
	event("cancel", [&]{ on_close(); });
	event("ok", [&]{ on_ok(); });
}

PdfConfigDialog::~PdfConfigDialog()
{
}

void PdfConfigDialog::on_close()
{
	destroy();
}

void PdfConfigDialog::on_ok()
{
	ok = true;
	data->track_mode.resize(song->tracks.num);
	foreachi(Track *t, song->tracks, i){
		if (t->type != SignalType::MIDI)
			continue;
		bool classical = is_checked(format("classical-%d", i));
		bool tab = is_checked(format("tab-%d", i));
		data->track_mode[i] = (classical ? 1 : 0) + (tab ? 2 : 0);
	}
	data->horizontal_scale = get_float("scale") / 100;

	destroy();
}

