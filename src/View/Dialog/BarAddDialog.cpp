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
	Array<int> beats = {1,1,1,1};
	int divisor = 1;
	float bpm = 90.0f;

	// get default data from "selected" reference bar
	if (song->bars.num > 0){
		foreachi(Bar *b, song->bars, i)
			if ((i <= index) and (!b->is_pause())){
				beats = b->beats;
				divisor = b->divisor;
				bpm = b->bpm(song->sample_rate);
			}
	}
	set_int("beats", beats.num);
	set_int("sub_beats", divisor);
	set_string("pattern", "...");
	set_float("bpm", bpm);
	check("shift-data", true);

	event("ok", std::bind(&BarAddDialog::on_ok, this));
	event("cancel", std::bind(&BarAddDialog::on_close, this));
	event("hui:close", std::bind(&BarAddDialog::on_close, this));
}

void set_bar_pattern(BarPattern &b, const string &pat)
{
	auto xx = pat.replace(",", " ").replace(":", " ").explode(" ");
	b.beats.clear();
	for (string &x: xx)
		b.beats.add(x._int());
	b.update_total();
}

void BarAddDialog::on_ok()
{
	BarPattern b = BarPattern(0, get_int("beats"), get_int("sub_beats"));
	int count = get_int("count");
	float bpm = get_float("bpm");
	bool move_data = is_checked("shift-data");
	//set_bar_pattern(b, get_string("pattern"));
	b.update_total();
	b.length = (int)((float)b.total_sub_beats / b.divisor * (float)song->sample_rate * 60.0f / bpm);

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
