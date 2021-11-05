/*
 * TimeTrackAddDialog.cpp
 *
 *  Created on: Nov 5, 2021
 *      Author: michi
 */

#include "TimeTrackAddDialog.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Action/ActionManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"


void set_bar_pattern(BarPattern &b, const string &pat);


TimeTrackAddDialog::TimeTrackAddDialog(Song *_song, hui::Window *_parent):
	hui::Dialog("time-track-add-dialog", _parent)
{
	song = _song;

	check("add-bars", true);
	reveal("metro-revealer", true);

	new_bar = {1000, 4, 1};
	set_int("num_bars", 32);
	set_int("beats", new_bar.beats.num);
	set_string("pattern", new_bar.pat_str());
	set_int("divisor", 0);

	event("cancel", [=]{ request_destroy(); });
	event("hui:close", [=]{ request_destroy(); });
	event("ok", [=]{ on_ok(); });
	event("beats", [=]{ on_beats(); });
	event("divisor", [=]{ on_divisor(); });
	event("pattern", [=]{ on_pattern(); });
	event("complex", [=]{ on_complex(); });
	event("add-bars", [=]{ on_add_bars(); });
}



void TimeTrackAddDialog::on_ok() {
	song->begin_action_group(_("add time track"));
	try {
		song->add_track(SignalType::BEATS, 0);

		// some default data
		if (is_checked("add-bars")) {
			int count = get_int("num_bars");
			float bpm = get_float("beats_per_minute");
			new_bar.set_bpm(bpm, song->sample_rate);
			for (int i=0; i<count; i++)
				song->add_bar(-1, new_bar, false);
		}
	} catch (Exception &e) {
		song->session->e(e.message());
	}
	song->end_action_group();

	request_destroy();
}

void TimeTrackAddDialog::on_beats() {
	new_bar = Bar(100, get_int(""), new_bar.divisor);
	set_string("pattern", new_bar.pat_str());
}

void TimeTrackAddDialog::on_divisor() {
	new_bar.divisor = 1 << get_int("");
}

void TimeTrackAddDialog::on_pattern() {
	set_bar_pattern(new_bar, get_string("pattern"));
	set_int("beats", new_bar.beats.num);
}

void TimeTrackAddDialog::on_complex() {
	bool complex = is_checked("complex");
	hide_control("beats", complex);
	hide_control("pattern", !complex);
}

void TimeTrackAddDialog::on_add_bars() {
	reveal("metro-revealer", is_checked(""));
}
