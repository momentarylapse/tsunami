/*
 * BarAddDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarAddDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"
#include "../../Data/base.h"
#include "../AudioView.h"

BarAddDialog::BarAddDialog(hui::Window *root, Song *s, int _index):
	hui::Dialog("", 100, 100, root, false)
{
	fromResource("bar_add_dialog");
	song = s;
	index = max(_index, 0);


	setInt("count", 1);
	int beats = 4;
	int sub_beats = 1;
	float bpm = 90.0f;

	// get default data from "selected" reference bar
	if (song->bars.num > 0){
		foreachi(Bar *b, song->bars, i)
			if ((i <= index) and (!b->is_pause())){
				beats = b->num_beats;
				sub_beats = b->num_sub_beats;
				bpm = song->sample_rate * 60.0f / (b->length / b->num_beats);
			}
	}
	setInt("beats", beats);
	setInt("sub_beats", sub_beats);
	setFloat("bpm", bpm);
	check("shift-data", true);

	event("ok", std::bind(&BarAddDialog::onOk, this));
	event("cancel", std::bind(&BarAddDialog::onClose, this));
	event("hui:close", std::bind(&BarAddDialog::onClose, this));
}

void BarAddDialog::onOk()
{
	int count = getInt("count");
	int beats = getInt("beats");
	int sub_beats = getInt("sub_beats");
	float bpm = getFloat("bpm");
	bool move_data = isChecked("shift-data");

	song->begin_action_group();

	if (!song->time_track())
		song->add_track(SignalType::BEATS, 0);

	for (int i=0; i<count; i++)
		song->add_bar(index, bpm, beats, sub_beats, move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);
	song->end_action_group();

	destroy();
}

void BarAddDialog::onClose()
{
	destroy();
}
