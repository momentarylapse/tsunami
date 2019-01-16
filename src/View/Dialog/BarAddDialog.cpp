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
	from_resource("bar_add_dialog");
	song = s;
	index = max(_index, 0);


	set_int("count", 1);
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
	set_int("beats", beats);
	set_int("sub_beats", sub_beats);
	set_float("bpm", bpm);
	check("shift-data", true);

	event("ok", std::bind(&BarAddDialog::on_ok, this));
	event("cancel", std::bind(&BarAddDialog::on_close, this));
	event("hui:close", std::bind(&BarAddDialog::on_close, this));
}

void set_bar_pattern(BarPattern &b, const string &pat)
{
	b.pattern.resize(b.num_beats);
	auto xx = pat.replace(",", " ").replace(":", " ").explode(" ");
	for (int i=0; i<b.num_beats; i++){
		b.pattern[i] = b.num_sub_beats;
		if (i < xx.num)
			b.pattern[i] = xx[i]._int();
	}
	b.update_total();
}

void BarAddDialog::on_ok()
{
	BarPattern b;
	int count = get_int("count");
	b.num_beats = get_int("beats");
	b.num_sub_beats = get_int("sub_beats");
	float bpm = get_float("bpm");
	bool move_data = is_checked("shift-data");
	set_bar_pattern(b, get_string("pattern"));
	b.length = (int)((float)b.num_beats * (float)song->sample_rate * 60.0f / bpm);

	song->begin_action_group();

	if (!song->time_track())
		song->add_track(SignalType::BEATS, 0);

	for (int i=0; i<count; i++)
		song->add_bar(index, b, move_data ? Bar::EditMode::STRETCH : Bar::EditMode::IGNORE);
	song->end_action_group();

	destroy();
}

void BarAddDialog::on_close()
{
	destroy();
}
