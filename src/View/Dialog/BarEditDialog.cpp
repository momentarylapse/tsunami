/*
 * BarEditDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarEditDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"

void set_bar_pattern(BarPattern &b, const string &pat);

BarEditDialog::BarEditDialog(hui::Window *root, Song *_song, const Range &_bars):
	hui::Dialog("", 100, 100, root, false)
{
	from_resource("bar_edit_dialog");
	song = _song;
	duration = 0;
	for (int i=_bars.start(); i<_bars.end(); i++){
		sel.add(i);
		duration += song->bars[i]->length;
	}

	Bar *b = song->bars[sel[0]];
	new_bar = new Bar(*b);

	set_int("number", sel.num);
	set_int("beats", new_bar->beats.num);
	set_int("divisor", 0);
	if (new_bar->divisor == 2)
		set_int("divisor", 1);
	if (new_bar->divisor == 4)
		set_int("divisor", 2);
	set_string("pattern", new_bar->pat_str());
	check("complex", !new_bar->is_uniform());
	hide_control("beats", !new_bar->is_uniform());
	hide_control("pattern", new_bar->is_uniform());
	set_float("bpm", b->bpm(song->sample_rate));
	check("shift-data", true);
	check("scale-audio", false);

	update_result_bpm();

	event("ok", [&]{ on_ok(); });
	event("cancel", [&]{ on_close(); });
	event("hui:close", [&]{ on_close(); });
	event("beats", [&]{ on_beats(); });
	event("divisor", [&]{ on_divisor(); });
	event("pattern", [&]{ on_pattern(); });
	event("bpm", [&]{ on_bpm(); });
	event("number", [&]{ on_number(); });
	event("complex", [&]{ on_complex(); });
	event("mode", [&]{ on_mode(); });
	event("shift-data", [&]{ on_shift_data(); });
}

void BarEditDialog::on_ok()
{
	song->begin_action_group();
	int mode = get_int("mode");
	if (mode == 0){
		float bpm = get_float("bpm");
		bool move_data = is_checked("shift-data");
		bool scale_audio = is_checked("scale-audio");

		int bmode = Bar::EditMode::IGNORE;
		if (move_data){
			bmode = Bar::EditMode::STRETCH;
			if (scale_audio)
				bmode = Bar::EditMode::STRETCH_AND_SCALE_AUDIO;
		}

		foreachb(int i, sel){
			BarPattern b = *song->bars[i];
			b.set_bpm(bpm, song->sample_rate);
			song->edit_bar(i, b, bmode);
		}

	}else{
		int number = get_int("number");

		//if (number != sel.num){
			foreachb(int i, sel)
				song->delete_bar(i, false);
			new_bar->length = duration / number;
			for (int i=0; i<number; i++)
				song->add_bar(sel[0], *new_bar, Bar::EditMode::IGNORE);
		/*}else{
			foreachb(int i, sel){
				BarPattern b = *song->bars[i];
				b.num_beats = new_bar->num_beats;
				b.num_sub_beats = new_bar->num_sub_beats;
				//set_bar_pattern(b, get_string("pattern"));
				b.set_pattern(new_bar->pattern);
				b.length = duration / number;
				song->edit_bar(i, b, Bar::EditMode::IGNORE);
			}
		}*/
	}
	song->end_action_group();

	destroy();
}

void BarEditDialog::on_close()
{
	destroy();
}

void BarEditDialog::on_beats()
{
	*new_bar = Bar(1000, get_int(""), new_bar->divisor);
	set_string("pattern", new_bar->pat_str());
	update_result_bpm();
}

void BarEditDialog::on_divisor()
{
	new_bar->divisor = 1 << get_int("");
	update_result_bpm();
}

void BarEditDialog::on_pattern()
{
	set_bar_pattern(*new_bar, get_string("pattern"));
	set_int("beats", new_bar->beats.num);
	update_result_bpm();
}

void BarEditDialog::on_complex()
{
	bool complex = is_checked("complex");
	hide_control("beats", complex);
	hide_control("pattern", !complex);
}

void BarEditDialog::on_bpm()
{
	//check("edit_bpm", true);
}

void BarEditDialog::on_mode()
{
	//check("edit_bpm", true);
}

void BarEditDialog::on_number()
{
	update_result_bpm();
}

void BarEditDialog::update_result_bpm()
{
	int number = get_int("number");
	new_bar->length = duration / number;
	set_float("result_bpm", new_bar->bpm(song->sample_rate));
}

void BarEditDialog::on_shift_data()
{
	enable("scale-audio", is_checked(""));
}
