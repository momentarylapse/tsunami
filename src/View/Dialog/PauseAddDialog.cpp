/*
 * PauseAddDialog.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "PauseAddDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"
#include "../../Data/base.h"
#include "../AudioView.h"

extern bool bar_dialog_move_data;

PauseAddDialog::PauseAddDialog(hui::Window *parent, Song *s, int _index):
	hui::Dialog("pause_add_dialog", parent)
{
	song = s;
	index = _index;

	set_float("duration", 1.0f);
	check("shift-data", bar_dialog_move_data);

	event("ok", [=]{ on_ok(); });
	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
}

void PauseAddDialog::on_ok() {
	bar_dialog_move_data = is_checked("shift-data");
	float duration = get_float("duration");
	song->begin_action_group();

	if (!song->time_track())
		song->add_track(SignalType::BEATS, 0);

	int length = (int)(duration * (float)song->sample_rate);
	song->add_pause(index, length, bar_dialog_move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);
	song->end_action_group();

	request_destroy();
}

void PauseAddDialog::on_close() {
	request_destroy();
}
