/*
 * BarReplaceDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarReplaceDialog.h"
#include "common.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/Song.h"


void set_bar_pattern(BarPattern &b, const string &pat);

BarReplaceDialog::BarReplaceDialog(hui::Window *parent, Song *_song, const Array<int> &_bars):
	hui::Dialog("bar-replace-dialog", parent)
{
	song = _song;
	duration = 0;
	sel = _bars;
	for (int i: sel)
		duration += song->bars[i]->length;

	new_bar = song->bars[sel[0]]->pattern();

	set_int("number", sel.num);
	set_int("beats", new_bar.beats.num);
	set_int("divisor", 0);
	if (new_bar.divisor == 2)
		set_int("divisor", 1);
	if (new_bar.divisor == 4)
		set_int("divisor", 2);
	set_string("pattern", new_bar.pat_str());
	check("complex", !new_bar.is_uniform());
	hide_control("beats", !new_bar.is_uniform());
	hide_control("pattern", new_bar.is_uniform());
	check("shift-data", bar_dialog_move_data);
	check("scale-audio", bar_dialog_scale_audio);
	enable("scale-audio", bar_dialog_move_data);

	update_result_bpm();

	event("ok", [this] { on_ok(); });
	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
	event("beats", [this] { on_beats(); });
	event("divisor", [this] { on_divisor(); });
	event("pattern", [this] { on_pattern(); });
	event("number", [this] { on_number(); });
	event("complex", [this] { on_complex(); });
	event("shift-data", [this] { on_shift_data(); });
}

void BarReplaceDialog::on_ok() {
	song->begin_action_group("replace bars");

	int number = get_int("number");

	//if (number != sel.num){
		foreachb(int i, sel)
			song->delete_bar(i, false);
		new_bar.length = duration / number;
		for (int i=0; i<number; i++)
			song->add_bar(sel[0], new_bar, Bar::EditMode::IGNORE);
	/*}else{
		foreachb(int i, sel){
			BarPattern b = *song->bars[i];
			b.num_beats = new_bar.num_beats;
			b.num_sub_beats = new_bar.num_sub_beats;
			//set_bar_pattern(b, get_string("pattern"));
			b.set_pattern(new_bar.pattern);
			b.length = duration / number;
			song->edit_bar(i, b, Bar::EditMode::IGNORE);
		}
	}*/
	song->end_action_group();

	request_destroy();
}

void BarReplaceDialog::on_close() {
	request_destroy();
}

void BarReplaceDialog::on_beats() {
	new_bar = {1000, get_int(""), new_bar.divisor};
	set_string("pattern", new_bar.pat_str());
	update_result_bpm();
}

void BarReplaceDialog::on_divisor() {
	new_bar.divisor = 1 << get_int("");
	update_result_bpm();
}

void BarReplaceDialog::on_pattern() {
	set_bar_pattern(new_bar, get_string("pattern"));
	set_int("beats", new_bar.beats.num);
	update_result_bpm();
}

void BarReplaceDialog::on_complex() {
	bool complex = is_checked("complex");
	hide_control("beats", complex);
	hide_control("pattern", !complex);
}

void BarReplaceDialog::on_number() {
	update_result_bpm();
}

void BarReplaceDialog::update_result_bpm() {
	int number = get_int("number");
	new_bar.length = duration / number;
	set_float("result_bpm", new_bar.bpm(song->sample_rate));
}

void BarReplaceDialog::on_shift_data() {
	enable("scale-audio", is_checked(""));
}
