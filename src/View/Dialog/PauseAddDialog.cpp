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

PauseAddDialog::PauseAddDialog(hui::Window *root, Song *s, int _index):
	hui::Dialog("", 100, 100, root, false)
{
	fromResource("pause_add_dialog");
	song = s;
	index = _index;

	setFloat("duration", 1.0f);
	check("shift-data", true);

	event("ok", std::bind(&PauseAddDialog::onOk, this));
	event("cancel", std::bind(&PauseAddDialog::onClose, this));
	event("hui:close", std::bind(&PauseAddDialog::onClose, this));
}

void PauseAddDialog::onOk()
{
	bool move_data = isChecked("shift-data");
	float duration = getFloat("duration");
	song->begin_action_group();

	if (!song->time_track())
		song->add_track(SignalType::BEATS, 0);

	int length = (int)(duration * (float)song->sample_rate);
	song->add_pause(index, length, move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);
	song->end_action_group();

	destroy();
}

void PauseAddDialog::onClose()
{
	destroy();
}
