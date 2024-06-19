/*
 * NewDialog.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "NewSongDialog.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"
#include "../../action/ActionManager.h"
#include "../../stuff/SessionManager.h"
#include "../../Tsunami.h"
#include "../../Session.h"

namespace tsunami {

void set_bar_pattern(BarPattern &b, const string &pat);

NewSongDialog::NewSongDialog(hui::Window *_parent):
	hui::Dialog("new-song-dialog", _parent)
{
	for (int rate: POSSIBLE_SAMPLE_RATES)
		add_string("sample-rate", i2s(rate));
	set_int("sample-rate", POSSIBLE_SAMPLE_RATES.find(DEFAULT_SAMPLE_RATE));
	expand("metro-revealer", false);

	on_type(SignalType::Audio);

	check("channels:mono", true);

	new_bar = {1000, 4, 1};
	set_int("num-bars", 32);
	set_int("beats", new_bar.beats.num);
	set_string("pattern", new_bar.pat_str());
	set_int("divisor", 0);

	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
	event("ok", [this] { on_ok(); });
	event("metronome", [this] { on_metronome(); });
	event("type-audio", [this] { on_type(SignalType::Audio); });
	//event("type-audio-stereo", [this] { on_type(SignalType::Audio_STEREO); });
	event("type-midi", [this] { on_type(SignalType::Midi); });
	event("type-preset", [this] { on_type((SignalType)-1); });
	event("beats", [this] { on_beats(); });
	event("divisor", [this] { on_divisor(); });
	event("pattern", [this] { on_pattern(); });
	event("complex", [this] { on_complex(); });
}

void NewSongDialog::on_ok() {
	int sample_rate = get_string("sample-rate")._int();
	Session *session = Tsunami::instance->session_manager->spawn_new_session();
	Song *song = session->song.get();
	song->sample_rate = sample_rate;
	song->action_manager->enable(false);
	if (is_checked("metronome")) {
		song->add_track(SignalType::Beats, 0);
		int count = get_int("num-bars");
		float bpm = get_float("beats-per-minute");
		new_bar.set_bpm(bpm, song->sample_rate);
		for (int i=0; i<count; i++)
			song->add_bar(-1, new_bar, BarEditMode::Ignore);
	}
	song->add_track(type);

	song->add_tag("title", _("New Audio File"));
	song->add_tag("album", AppName);
	song->add_tag("artist", hui::config.get_str("DefaultArtist", AppName));

	song->action_manager->enable(true);
	song->out_new.notify();
	song->out_finished_loading.notify();
	request_destroy();
	session->win->show();
	session->win->activate("");
}

void NewSongDialog::on_beats() {
	new_bar = Bar(100, get_int(""), new_bar.divisor);
	set_string("pattern", new_bar.pat_str());
}

void NewSongDialog::on_divisor() {
	new_bar.divisor = 1 << get_int("");
}

void NewSongDialog::on_pattern() {
	set_bar_pattern(new_bar, get_string("pattern"));
	set_int("beats", new_bar.beats.num);
}

void NewSongDialog::on_complex() {
	bool complex = is_checked("complex");
	hide_control("beats", complex);
	hide_control("pattern", !complex);
}

void NewSongDialog::on_type(SignalType t) {
	type = t;
	check("type-audio", t == SignalType::Audio);
	//check("type-audio-stereo", t == SignalType::Audio_STEREO);
	check("type-midi", t == SignalType::Midi);
	check("type-preset", t == (SignalType)-1);

	expand("revealer-channels", type == SignalType::Audio);
	expand("revealer-presets", type == (SignalType)-1);
	enable("ok", type != (SignalType)-1);

	if (!manually_changed_metronome_flag) {
		check("metronome", t == SignalType::Midi);
		expand("metro-revealer", t == SignalType::Midi);
	}
}

void NewSongDialog::on_metronome() {
	expand("metro-revealer", is_checked(""));
	manually_changed_metronome_flag = true;
}

}

