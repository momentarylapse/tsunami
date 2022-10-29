/*
 * PauseEditDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "PauseEditDialog.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/Song.h"

extern bool bar_dialog_move_data;

PauseEditDialog::PauseEditDialog(hui::Window *parent, Song *_song, int _index):
	hui::Dialog("pause_edit_dialog", parent)
{
	song = _song;
	index = _index;

	Bar *b = song->bars[index].get();
	set_float("duration", (float)b->length / (float)song->sample_rate);
	check("shift-data", bar_dialog_move_data);

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
}

void PauseEditDialog::on_ok() {
	bar_dialog_move_data = is_checked("shift-data");
	float duration = get_float("duration");
	BarPattern b = *song->bars[index].get();
	b.length = (float)song->sample_rate * duration;
	song->edit_bar(index, b, bar_dialog_move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);

	request_destroy();
}
