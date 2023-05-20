//
// Created by michi on 13.05.23.
//

#include "NewTrackDialog.h"
#include "EditStringsDialog.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/midi/MidiData.h"
#include "../../data/rhythm/Bar.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../action/ActionManager.h"
#include "../../stuff/SessionManager.h"
#include "../../plugins/PluginManager.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../lib/base/sort.h"

void set_bar_pattern(BarPattern &b, const string &pat);

hui::Panel *create_synth_panel(Synthesizer *synth, Session *session, NewTrackDialog *parent) {
	auto *p = new ModulePanel(synth, parent, ConfigPanelMode::FIXED_HEIGHT | ConfigPanelMode::PROFILES | ConfigPanelMode::REPLACE);
	//p->set_func_edit([track](const string &param){ track->edit_synthesizer(param); });
	p->set_func_replace([parent, session, synth] {
		session->plugin_manager->choose_module(parent, session, ModuleCategory::SYNTHESIZER, [parent, session] (const base::optional<string> &name) {
			if (name.has_value())
				parent->set_synthesizer(CreateSynthesizer(session, *name));
		}, synth->module_class);
	});
	/*p->set_func_detune([parent, track, session] {
		hui::fly(new TemperamentDialog(track, session->view, parent->win));
	});*/
	return p;
}

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

	new_bar = {1000, 4, 1};
	set_int("num_bars", 32);
	set_int("beats", new_bar.beats.num);
	set_string("pattern", new_bar.pat_str());
	set_int("divisor", 0);

	set_synthesizer(CreateSynthesizer(session, "Dummy"));

	auto disable_beats = [this] (const string& message) {
		hide_control("l-no-metronome", false);
		set_string("l-no-metronome", message);
		enable("beats_per_minute", false);
		enable("num_bars", false);
		enable("beats", false);
		enable("pattern", false);
		enable("complex", false);
		enable("divisor", false);
	};
	if (session->song->time_track()) {
		disable_beats("There can only be one metronome track.");
	} else if (session->song->bars.num > 0) {
		disable_beats("There already is a bar structure. A new metronome track will reuse the existing one.");
	}

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
	event("type-preset", [this] { on_type((SignalType)-1); });
	event("beats", [this] { on_beats(); });
	event("divisor", [this] { on_divisor(); });
	event("pattern", [this] { on_pattern(); });
	event("complex", [this] { on_complex(); });
}

void NewTrackDialog::set_synthesizer(Synthesizer *s) {
	synth = s;
	synth_panel = create_synth_panel(synth.get(), session, this);
	embed(synth_panel.get(), "g-synth", 0, 0);
}

void NewTrackDialog::on_instrument() {
	int n = get_int("");
	instrument = instrument_list[n];
	update_strings();
}

void NewTrackDialog::on_edit_tuning() {
	auto dlg = new EditStringsDialog(win, instrument.string_pitch);
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
	if (type == SignalType::AUDIO) {
	} else if (type == SignalType::MIDI) {
		t->set_instrument(instrument);
		t->set_synthesizer(synth);
	} else if (type == SignalType::BEATS) {
		t->set_synthesizer(synth);

		//if (is_checked("metronome")) {
			int count = get_int("num_bars");
			float bpm = get_float("beats_per_minute");
			new_bar.set_bpm(bpm, song->sample_rate);
			for (int i = 0; i < count; i++)
				song->add_bar(-1, new_bar, false);
		//}
	}
	song->end_action_group();
	request_destroy();
}

void NewTrackDialog::on_beats() {
	new_bar = Bar(100, get_int(""), new_bar.divisor);
	set_string("pattern", new_bar.pat_str());
}

void NewTrackDialog::on_divisor() {
	new_bar.divisor = 1 << get_int("");
}

void NewTrackDialog::on_pattern() {
	set_bar_pattern(new_bar, get_string("pattern"));
	set_int("beats", new_bar.beats.num);
}

void NewTrackDialog::on_complex() {
	bool complex = is_checked("complex");
	hide_control("beats", complex);
	hide_control("pattern", !complex);
}

void NewTrackDialog::on_type(SignalType t) {
	type = t;
	check("type-audio", t == SignalType::AUDIO);
	check("type-midi", t == SignalType::MIDI);
	check("type-metronome", t == SignalType::BEATS);
	check("type-master", t == SignalType::GROUP);
	check("type-preset", t == (SignalType)-1);

	bool allow_ok = true;
	if (type == (SignalType)-1)
		allow_ok = false;
	if (type == SignalType::BEATS and session->song->time_track())
		allow_ok = false;
	enable("ok", allow_ok);

	expand("revealer-channels", t == SignalType::AUDIO);
	expand("revealer-instrument", t == SignalType::MIDI);
	expand("revealer-synth", t == SignalType::MIDI or t == SignalType::BEATS);
	//expand("g-fx-midi", true);//t == SignalType::MIDI);
	expand("revealer-metronome", t == SignalType::BEATS);
	expand("revealer-master", t == SignalType::GROUP);
	expand("revealer-presets", t == (SignalType)-1);
}

void NewTrackDialog::on_metronome() {
	//expand("metro-revealer", is_checked(""));
}

