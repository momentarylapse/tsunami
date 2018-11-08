/*
 * PauseEditDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "PauseEditDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"

PauseEditDialog::PauseEditDialog(hui::Window *root, Song *_song, int _index):
	hui::Dialog("", 100, 100, root, false)
{
	from_resource("pause_edit_dialog");
	song = _song;
	index = _index;

	Bar *b = song->bars[index];
	set_float("duration", (float)b->length / (float)song->sample_rate);
	check("shift-data", true);

	event("ok", std::bind(&PauseEditDialog::on_ok, this));
	event("cancel", std::bind(&PauseEditDialog::destroy, this));
	event("hui:close", std::bind(&PauseEditDialog::destroy, this));
}

void PauseEditDialog::on_ok()
{
	bool move_data = is_checked("shift-data");
	float duration = get_float("duration");
	BarPattern b = *song->bars[index];
	b.length = (float)song->sample_rate * duration;
	song->edit_bar(index, b.length, b.num_beats, b.num_sub_beats, move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);

	destroy();
}
