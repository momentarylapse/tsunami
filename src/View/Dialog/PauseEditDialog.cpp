/*
 * PauseEditDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "PauseEditDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"

extern bool bar_dialog_move_data;

PauseEditDialog::PauseEditDialog(hui::Window *root, Song *_song, int _index):
	hui::Dialog("", 100, 100, root, false)
{
	from_resource("pause_edit_dialog");
	song = _song;
	index = _index;

	Bar *b = song->bars[index];
	set_float("duration", (float)b->length / (float)song->sample_rate);
	check("shift-data", bar_dialog_move_data);

	event("ok", [=]{ on_ok(); });
	event("cancel", [=]{ destroy(); });
	event("hui:close", [=]{ destroy(); });
}

void PauseEditDialog::on_ok()
{
	bar_dialog_move_data = is_checked("shift-data");
	float duration = get_float("duration");
	BarPattern b = *song->bars[index];
	b.length = (float)song->sample_rate * duration;
	song->edit_bar(index, b, bar_dialog_move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);

	destroy();
}
