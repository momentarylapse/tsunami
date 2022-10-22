/*
 * BarEditSpeedDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarEditSpeedDialog.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/Song.h"

extern bool bar_dialog_move_data;
static bool bar_dialog_scale_audio = false;

void set_bar_pattern(BarPattern &b, const string &pat);

BarEditSpeedDialog::BarEditSpeedDialog(hui::Window *parent, Song *_song, const Array<int> &_bars):
	hui::Dialog("bar-edit-speed-dialog", parent)
{
	song = _song;
	duration = 0;
	sel = _bars;
	for (int i: sel)
		duration += song->bars[i]->length;

	new_bar = song->bars[sel[0]]->pattern();

	set_float("bpm", new_bar.bpm(song->sample_rate));
	check("shift-data", bar_dialog_move_data);
	check("scale-audio", bar_dialog_scale_audio);
	enable("scale-audio", bar_dialog_move_data);

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
	event("bpm", [this] { on_bpm(); });
	event("shift-data", [this] { on_shift_data(); });
}

void BarEditSpeedDialog::on_ok() {
	song->begin_action_group("edit bars");

	float bpm = get_float("bpm");
	bar_dialog_move_data = is_checked("shift-data");
	bar_dialog_scale_audio = is_checked("scale-audio");

	int bmode = Bar::EditMode::IGNORE;
	if (bar_dialog_move_data) {
		bmode = Bar::EditMode::STRETCH;
		if (bar_dialog_scale_audio)
			bmode = Bar::EditMode::STRETCH_AND_SCALE_AUDIO;
	}

	foreachb(int i, sel) {
		auto b = song->bars[i]->pattern();
		b.set_bpm(bpm, song->sample_rate);
		song->edit_bar(i, b, bmode);
	}
	song->end_action_group();

	request_destroy();
}

void BarEditSpeedDialog::on_close() {
	request_destroy();
}

void BarEditSpeedDialog::on_bpm() {
	//check("edit_bpm", true);
}

void BarEditSpeedDialog::on_shift_data() {
	enable("scale-audio", is_checked(""));
}
