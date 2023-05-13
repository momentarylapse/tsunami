//
// Created by michi on 13.05.23.
//

#include "NewTrackDialog.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/rhythm/Bar.h"
#include "../../action/ActionManager.h"
#include "../../stuff/SessionManager.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../lib/base/sort.h"
#include "../../data/midi/MidiData.h"
#include "TuningDialog.h"

void set_bar_pattern(BarPattern &b, const string &pat);

NewTrackDialog::NewTrackDialog(hui::Window *_parent, Session *s):
		hui::Dialog("new-track-dialog", _parent)
{
	session = s;
	on_type(SignalType::AUDIO);
	check("channels:mono", true);
	instrument = Instrument(Instrument::Type::PIANO);

	instrument_list = base::sorted(Instrument::enumerate(), [] (const Instrument &a, const Instrument &b) { return a.name() <= b.name(); });
	for (auto &i: instrument_list) {
		set_string("instrument", i.name());
	}
	set_int("instrument", instrument_list.find(instrument));
	update_strings();

	/*new_bar = {1000, 4, 1};
	set_int("num_bars", 32);
	set_int("beats", new_bar.beats.num);
	set_string("pattern", new_bar.pat_str());
	set_int("divisor", 0);*/

	event("cancel", [this] { request_destroy(); });
	event("hui:close", [this] { request_destroy(); });
	event("ok", [this] { on_ok(); });
	event("instrument", [this] { on_instrument(); });
	event("edit_tuning", [this] { on_edit_tuning(); });
	event("metronome", [this] { on_metronome(); });
	event("type-audio", [this] { on_type(SignalType::AUDIO); });
	event("type-midi", [this] { on_type(SignalType::MIDI); });
	event("type-metronome", [this] { on_type(SignalType::BEATS); });
	event("type-master", [this] { on_type(SignalType::GROUP); });
	event("beats", [this] { on_beats(); });
	event("divisor", [this] { on_divisor(); });
	event("pattern", [this] { on_pattern(); });
	event("complex", [this] { on_complex(); });
}

void NewTrackDialog::on_instrument() {
	int n = get_int("");
	instrument = instrument_list[n];
	update_strings();
}

void NewTrackDialog::on_edit_tuning() {
	auto dlg = new TuningDialog(win, instrument.string_pitch);
	hui::fly(dlg, [this, dlg] {
		if (dlg->ok) {
			instrument.string_pitch = dlg->strings;
			update_strings();
		}
	});
}

void NewTrackDialog::update_strings() {
	string tuning = format(_("%d strings: <b>"), instrument.string_pitch.num);
	for (int i=0; i<instrument.string_pitch.num; i++) {
		if (i > 0)
			tuning += ", ";
		tuning += pitch_name(instrument.string_pitch[i]);
	}
	tuning += "</b>";
	if (instrument.string_pitch.num == 0)
		tuning = "<i>" + _(" - no strings -") + "</i>";
	set_string("tuning", tuning);
}

void NewTrackDialog::on_ok() {
	auto song = session->song;
	auto effective_type = type;
	if (type == SignalType::AUDIO and is_checked("channels:stereo"))
		effective_type = SignalType::AUDIO_STEREO;
	song->begin_action_group("add-track");
	auto t = song->add_track(effective_type);
	if (type == SignalType::MIDI) {
		t->set_instrument(instrument);
	}
	song->end_action_group();
	/*int sample_rate = get_string("sample_rate")._int();
	Session *session = tsunami->session_manager->spawn_new_session();
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
	song->out_new.notify();
	song->out_finished_loading.notify();*/
	request_destroy();
}

void NewTrackDialog::on_beats() {
	/*new_bar = Bar(100, get_int(""), new_bar.divisor);
	set_string("pattern", new_bar.pat_str());*/
}

void NewTrackDialog::on_divisor() {
	//new_bar.divisor = 1 << get_int("");
}

void NewTrackDialog::on_pattern() {
	//set_bar_pattern(new_bar, get_string("pattern"));
	//set_int("beats", new_bar.beats.num);
}

void NewTrackDialog::on_complex() {
	//bool complex = is_checked("complex");
	//hide_control("beats", complex);
	//hide_control("pattern", !complex);
}

void NewTrackDialog::on_type(SignalType t) {
	type = t;
	check("type-audio", t == SignalType::AUDIO);
	check("type-midi", t == SignalType::MIDI);
	check("type-metronome", t == SignalType::BEATS);
	check("type-master", t == SignalType::GROUP);

	hide_control("g-channels", t != SignalType::AUDIO);
	hide_control("g-instrument", t != SignalType::MIDI);
	hide_control("g-synth", t != SignalType::MIDI and t != SignalType::BEATS);
	hide_control("g-fx-midi", true);//t != SignalType::MIDI);
	hide_control("g-metronome", t != SignalType::BEATS);
	hide_control("l-master", t != SignalType::GROUP);
}

void NewTrackDialog::on_metronome() {
	//expand("metro-revealer", is_checked(""));
}
