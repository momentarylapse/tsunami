//
// Created by michi on 13.05.23.
//

#include "NewTrackDialog.h"
#include "EditStringsDialog.h"
#include "PresetSelectionDialog.h"
#include "QuestionDialog.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/rhythm/Bar.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../action/ActionManager.h"
#include "../../stuff/SessionManager.h"
#include "../../plugins/PluginManager.h"
#include "../../plugins/PresetManager.h"
#include "../../Tsunami.h"
#include "../../Session.h"
#include "../../lib/base/sort.h"
#include "../../lib/hui/language.h"

namespace tsunami {

static const int FAKE_TYPE_PRESET = -1;

void set_bar_pattern(BarPattern &b, const string &pat);

hui::Panel *create_synth_panel(Synthesizer *synth, Session *session, NewTrackDialog *parent) {
	auto *p = new ModulePanel(synth, parent, ConfigPanelMode::FixedHeight | ConfigPanelMode::Profiles | ConfigPanelMode::Replace);
	//p->set_func_edit([track](const string &param){ track->edit_synthesizer(param); });
	p->set_func_replace([parent, session, synth] {
		ModuleSelectorDialog::choose(parent, session, ModuleCategory::Synthesizer, synth->module_class).then([parent, session] (const string &name) {
			parent->set_synthesizer(CreateSynthesizer(session, name));
		});
	});
	/*p->set_func_detune([parent, track, session] {
		hui::fly(new TemperamentDialog(track, session->view, parent->win));
	});*/
	return p;
}

NewTrackDialog::NewTrackDialog(hui::Window *_parent, Session *s, SignalType initial_type):
		hui::Dialog("new-track-dialog", _parent)
{
	session = s;
	on_type(initial_type);
	check("channels:mono", true);
	instrument = Instrument(Instrument::Type::Piano);

	instrument_list = base::sorted(Instrument::enumerate(), [] (const Instrument &a, const Instrument &b) { return a.name() <= b.name(); });
	for (auto &i: instrument_list) {
		set_string("instrument", i.name());
	}
	set_int("instrument", instrument_list.find(instrument));
	update_strings();

	new_bar = {1000, 4, 1};
	set_int("num-bars", 32);
	set_int("beats", new_bar.beats.num);
	set_string("pattern", new_bar.pat_str());
	set_int("divisor", 0);

	set_synthesizer(CreateSynthesizer(session, "Dummy"));

	presets = session->plugin_manager->presets->enumerate_track_presets(session);
	for (auto& n: presets)
		add_string("presets", n);

	auto disable_beats = [this] (const string& message) {
		hide_control("l-no-metronome", false);
		set_string("l-no-metronome", message);
		enable("beats-per-minute", false);
		enable("num-bars", false);
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
	event("save-preset", [this] { on_save_preset(); });
	event_x("presets", "hui:select", [this] { on_preset_select(); });
	event_x("presets", "hui:activate", [this] { on_preset(); });
	event("instrument", [this] { on_instrument(); });
	event("edit_tuning", [this] { on_edit_tuning(); });
	event("metronome", [this] { on_metronome(); });
	event("type-audio", [this] { on_type(SignalType::Audio); });
	event("type-midi", [this] { on_type(SignalType::Midi); });
	event("type-metronome", [this] { on_type(SignalType::Beats); });
	event("type-master", [this] { on_type(SignalType::Group); });
	event("type-preset", [this] { on_type((SignalType)FAKE_TYPE_PRESET); });
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
	hui::fly(dlg).then([this, dlg] {
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

	song->begin_action_group("add-track");
	if (type == SignalType::Audio) {
		auto t = song->add_track(type);
		if (is_checked("channels:stereo"))
			t->set_channels(2);
	} else if (type == SignalType::Midi) {
		auto t = song->add_track(type);
		t->set_instrument(instrument);
		t->set_synthesizer(synth);
	} else if (type == SignalType::Beats) {
		auto t = song->add_track(type);
		t->set_synthesizer(synth);

		if (song->bars.num == 0) {
			int count = get_int("num-bars");
			float bpm = get_float("beats-per-minute");
			new_bar.set_bpm(bpm, song->sample_rate);
			for (int i = 0; i < count; i++)
				song->add_bar(-1, new_bar, BarEditMode::Ignore);
		}
	} else if (type == SignalType::Group) {
		[[maybe_unused]] auto t = song->add_track(type);
	} else if (type == (SignalType)FAKE_TYPE_PRESET) {
		int r = get_int("presets");
		const auto& p = session->plugin_manager->presets->get_track_preset(session, presets[r]);
		auto t = song->add_track(p.type);
		t->set_channels(p.channels);
		if (p.type == SignalType::Midi)
			t->set_instrument(p.instrument);
		if (p.type == SignalType::Midi or p.type == SignalType::Beats) {
			synth = CreateSynthesizer(session, p.synth_class);
			synth->config_from_string(p.synth_version, p.synth_options);
			t->set_synthesizer(synth);
		}
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
	check("type-audio", t == SignalType::Audio);
	check("type-midi", t == SignalType::Midi);
	check("type-metronome", t == SignalType::Beats);
	check("type-master", t == SignalType::Group);
	check("type-preset", t == (SignalType)FAKE_TYPE_PRESET);

	bool allow_ok = true;
	if (type == (SignalType)FAKE_TYPE_PRESET) {
		int r = get_int("presets");
		allow_ok = (r >= 0);
	} else if (type == SignalType::Beats and session->song->time_track()) {
		allow_ok = false;
	}
	enable("ok", allow_ok);

	expand("revealer-channels", t == SignalType::Audio);
	expand("revealer-instrument", t == SignalType::Midi);
	expand("revealer-synth", t == SignalType::Midi or t == SignalType::Beats);
	//expand("g-fx-midi", true);//t == SignalType::Midi);
	expand("revealer-metronome", t == SignalType::Beats);
	expand("revealer-master", t == SignalType::Group);
	expand("revealer-presets", t == (SignalType)FAKE_TYPE_PRESET);
}

void NewTrackDialog::on_metronome() {
	//expand("metro-revealer", is_checked(""));
}

void NewTrackDialog::on_save_preset() {
	QuestionDialogString::ask(this, _("Name of the track preset?")).then([this] (const string& name) {
		PresetManager::TrackPreset p;
		p.name = name;
		p.type = type;
		p.instrument = instrument;
		p.synth_class = synth->module_class;
		p.synth_version = synth->version();
		p.synth_options = synth->config_to_string();
		session->plugin_manager->presets->save_track_preset(session, p);
	});
}

void NewTrackDialog::on_preset_select() {
	int r = get_int("presets");
	enable("ok", r >= 0);
}

void NewTrackDialog::on_preset() {
	on_ok();
}

}
