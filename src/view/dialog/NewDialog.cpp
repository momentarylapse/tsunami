/*
 * NewDialog.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "NewDialog.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"
#include "../../action/ActionManager.h"
#include "../../stuff/SessionManager.h"
#include "../../Tsunami.h"
#include "../../Session.h"

void set_bar_pattern(BarPattern &b, const string &pat);

NewDialog::NewDialog(hui::Window *_parent):
	hui::Dialog("new_dialog", _parent)
{
	add_string("sample_rate", "22050");
	add_string("sample_rate", i2s(DEFAULT_SAMPLE_RATE));
	add_string("sample_rate", "48000");
	add_string("sample_rate", "96000");
	set_int("sample_rate", 1);
	expand("metro-revealer", false);

	type = SignalType::AUDIO_MONO;
	check("type-audio-mono", true);

	new_bar = {1000, 4, 1};
	set_int("num_bars", 32);
	set_int("beats", new_bar.beats.num);
	set_string("pattern", new_bar.pat_str());
	set_int("divisor", 0);

	event("cancel", [=]{ request_destroy(); });
	event("hui:close", [=]{ request_destroy(); });
	event("ok", [=]{ on_ok(); });
	event("metronome", [=]{ on_metronome(); });
	event("type-audio-mono", [=]{ on_type(SignalType::AUDIO_MONO); });
	event("type-audio-stereo", [=]{ on_type(SignalType::AUDIO_STEREO); });
	event("type-midi", [=]{ on_type(SignalType::MIDI); });
	event("beats", [=]{ on_beats(); });
	event("divisor", [=]{ on_divisor(); });
	event("pattern", [=]{ on_pattern(); });
	event("complex", [=]{ on_complex(); });
}

void NewDialog::on_ok() {
	int sample_rate = get_string("sample_rate")._int();
	Session *session = tsunami->session_manager->create_session();
	Song *song = session->song.get();
	song->sample_rate = sample_rate;
	song->action_manager->enable(false);
	if (is_checked("metronome")) {
		song->add_track(SignalType::BEATS, 0);
		int count = get_int("num_bars");
		float bpm = get_float("beats_per_minute");
		new_bar.set_bpm(bpm, song->sample_rate);
		for (int i=0; i<count; i++)
			song->add_bar(-1, new_bar, false);
	}
	song->add_track(type);

	song->add_tag("title", _("New Audio File"));
	song->add_tag("album", AppName);
	song->add_tag("artist", hui::config.get_str("DefaultArtist", AppName));

	song->action_manager->enable(true);
	song->notify(song->MESSAGE_NEW);
	song->notify(song->MESSAGE_FINISHED_LOADING);
	request_destroy();
	session->win->show();
	session->win->activate("");
}

void NewDialog::on_beats() {
	new_bar = Bar(100, get_int(""), new_bar.divisor);
	set_string("pattern", new_bar.pat_str());
}

void NewDialog::on_divisor() {
	new_bar.divisor = 1 << get_int("");
}

void NewDialog::on_pattern() {
	set_bar_pattern(new_bar, get_string("pattern"));
	set_int("beats", new_bar.beats.num);
}

void NewDialog::on_complex() {
	bool complex = is_checked("complex");
	hide_control("beats", complex);
	hide_control("pattern", !complex);
}

void NewDialog::on_type(SignalType t) {
	type = t;
	check("type-audio-mono", t == SignalType::AUDIO_MONO);
	check("type-audio-stereo", t == SignalType::AUDIO_STEREO);
	check("type-midi", t == SignalType::MIDI);

	if (t == SignalType::MIDI) {
		check("metronome", true);
		expand("metro-revealer", true);
	}
}

void NewDialog::on_metronome() {
	expand("metro-revealer", is_checked(""));
}

