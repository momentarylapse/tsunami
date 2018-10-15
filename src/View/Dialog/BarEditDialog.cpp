/*
 * BarEditDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarEditDialog.h"

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Song.h"

BarEditDialog::BarEditDialog(hui::Window *root, Song *_song, const Range &_bars):
	hui::Dialog("", 100, 100, root, false)
{
	fromResource("bar_edit_dialog");
	song = _song;
	duration = 0;
	for (int i=_bars.start(); i<_bars.end(); i++){
		sel.add(i);
		duration += song->bars[i]->length;
	}

	Bar *b = song->bars[sel[0]];
	setInt("number", sel.num);
	setInt("beats", b->num_beats);
	setInt("sub_beats", b->num_sub_beats);
	setFloat("bpm", song->sample_rate * 60.0f / (b->length / b->num_beats));
	check("shift-data", true);
	check("scale-audio", false);

	update_result_bpm();

	event("ok", std::bind(&BarEditDialog::on_ok, this));
	event("cancel", std::bind(&BarEditDialog::on_close, this));
	event("hui:close", std::bind(&BarEditDialog::on_close, this));
	event("beats", std::bind(&BarEditDialog::on_beats, this));
	event("sub_beats", std::bind(&BarEditDialog::on_sub_beats, this));
	event("bpm", std::bind(&BarEditDialog::on_bpm, this));
	event("number", std::bind(&BarEditDialog::on_number, this));
	event("mode", std::bind(&BarEditDialog::on_mode, this));
	event("shift-data", std::bind(&BarEditDialog::on_shift_data, this));
}

void BarEditDialog::on_ok()
{
	song->beginActionGroup();
	int mode = getInt("mode");
	if (mode == 0){
		float bpm = getFloat("bpm");
		bool move_data = isChecked("shift-data");
		bool scale_audio = isChecked("scale-audio");

		int bmode = Bar::EditMode::IGNORE;
		if (move_data){
			bmode = Bar::EditMode::STRETCH;
			if (scale_audio)
				bmode = Bar::EditMode::STRETCH_AND_SCALE_AUDIO;
		}

		foreachb(int i, sel){
			BarPattern b = *song->bars[i];
			b.length = song->sample_rate * 60.0f * b.num_beats / bpm;
			song->editBar(i, b.length, b.num_beats, b.num_sub_beats, bmode);
		}

	}else{
		int number = getInt("number");
		int beats = getInt("beats");
		int sub_beats = getInt("sub_beats");
		bool edit_number = isChecked("edit_number");
		bool edit_beats = isChecked("edit_beats");
		bool edit_sub_beats = isChecked("edit_sub_beats");

		if (edit_number){
			foreachb(int i, sel)
				song->deleteBar(i, false);
			float t = (float)duration / (float)song->sample_rate;
			float bpm = 60.0f * (float)(number * beats) / t;
			for (int i=0; i<number; i++){
				song->addBar(sel[0], bpm, beats, sub_beats, Bar::EditMode::IGNORE);
			}
		}else{
			foreachb(int i, sel){
				BarPattern b = *song->bars[i];
				if (edit_beats)
					b.num_beats = beats;
				if (edit_sub_beats)
					b.num_sub_beats = sub_beats;
				b.length = duration / number;
				song->editBar(i, b.length, b.num_beats, b.num_sub_beats, Bar::EditMode::IGNORE);
			}
		}
	}
	song->endActionGroup();

	destroy();
}

void BarEditDialog::on_close()
{
	destroy();
}

void BarEditDialog::on_beats()
{
	check("edit_beats", true);
	update_result_bpm();
}

void BarEditDialog::on_sub_beats()
{
	check("edit_sub_beats", true);
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
	check("edit_number", true);
	update_result_bpm();
}

void BarEditDialog::update_result_bpm()
{
	float t = (float)duration / (float)song->sample_rate;
	int number = getInt("number");
	int beats = getInt("beats");
	setFloat("result_bpm", 60.0f * (float)(number * beats) / t);
}

void BarEditDialog::on_shift_data()
{
	enable("scale-audio", isChecked(""));
}
