/*
 * PauseAddDialog.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "PauseAddDialog.h"
#include "common.h"
#include "../audioview/AudioView.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/Song.h"
#include "../../data/base.h"


PauseAddDialog::PauseAddDialog(hui::Window *parent, Song *s, int _index):
	hui::Dialog("pause_add_dialog", parent)
{
	song = s;
	index = _index;

	set_float("duration", 1.0f);
	check("shift-data", bar_dialog_move_data);

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
}

void PauseAddDialog::on_ok() {
	bar_dialog_move_data = is_checked("shift-data");
	float duration = get_float("duration");
	song->begin_action_group("add pause");

	if (!song->time_track())
		song->add_track(SignalType::Beats, 0);

	int length = (int)(duration * (float)song->sample_rate);
	song->add_pause(index, length, bar_dialog_move_data ? BarEditMode::Stretch : BarEditMode::Ignore);
	song->end_action_group();

	request_destroy();
}

void PauseAddDialog::on_close() {
	request_destroy();
}
