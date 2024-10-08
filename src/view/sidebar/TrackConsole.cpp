/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "TrackConsole.h"
#include "../audioview/AudioView.h"
#include "../helper/Slider.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../helper/FxListEditor.h"
#include "../dialog/TemperamentDialog.h"
#include "../dialog/EditStringsDialog.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../../data/Track.h"
#include "../../data/base.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../plugins/PluginManager.h"
#include "../../lib/base/sort.h"
#include "../../lib/base/iter.h"
#include "../../lib/hui/language.h"
#include "../../Session.h"

namespace tsunami {

hui::Panel *create_dummy_synth_panel() {
	auto panel = new hui::Panel();
	panel->add_label("", 0, 0, "");
	return panel;
}

hui::Panel *create_synth_panel(Track *track, Session *session, hui::Panel *parent) {
	auto *p = new ModulePanel(track->synth.get(), parent, ConfigPanelMode::FixedHeight | ConfigPanelMode::Profiles | ConfigPanelMode::Replace);
	//p->set_func_edit([track](const string &param){ track->edit_synthesizer(param); });
	p->set_func_replace([parent, track, session] {
		ModuleSelectorDialog::choose(parent, session, ModuleCategory::Synthesizer, track->synth->module_class).then([track, session] (const string &name) {
			track->set_synthesizer(CreateSynthesizer(session, name));
		});
	});
	p->set_func_detune([parent, track, session] {
		hui::fly(new TemperamentDialog(track, session->view, parent->win));
	});
	return p;
}

TrackConsole::TrackConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Track"), "track-console", session, bar),
	in_track_update(this, [this] { on_update(); })
{
	track = nullptr;
	synth_panel = nullptr;
	editing = false;
	from_resource("track_dialog");
	set_decimals(1);
	set_mode(Mode::Fx);

	instrument_list = base::sorted(Instrument::enumerate(), [] (const Instrument &a, const Instrument &b) { return a.name() <= b.name(); });
	for (auto &i: instrument_list)
		set_string("instrument", i.name());

	event("name", [this] { on_name(); });
	event("volume", [this] { on_volume(); });
	event("panning", [this] { on_panning(); });
	event("instrument", [this] { on_instrument(); });
	event("edit-tuning", [this] { on_edit_strings(); });

	//event("g-fx", )
}

void TrackConsole::set_mode(Mode mode) {
	/*if (mode == Mode::FX) {
		expand("g-fx", true);
		set_int("tc", 0);
	} else if (mode == Mode::MIDI_FX) {
		expand("g-fx", true);
		set_int("tc", 1);
	} else if (mode == Mode::SYNTH) {
		expand("g-synth", true);
	}*/
	expand("g-fx", mode == Mode::Fx);
	expand("g-midi-fx", mode == Mode::MidiFx);
	expand("g-synth", mode == Mode::Synth);
}

void TrackConsole::on_enter() {
	set_track(view->cur_track());
	view->out_cur_track_changed >> create_sink([this] { on_view_cur_track_change(); });

}
void TrackConsole::on_leave() {
	view->unsubscribe(this);
	set_track(nullptr);
}

bool track_wants_synth(const Track *t) {
	return (t->type == SignalType::Midi) or (t->type == SignalType::Beats);
}

void TrackConsole::load_data() {
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	enable("instrument", track);
	hide_control("td_t_edit", !track);
	if (synth_panel)
		unembed(synth_panel);
	synth_panel = nullptr;

	if (track) {
		set_string("name", track->name);
		set_options("name", "placeholder=" + track->nice_name());
		set_float("volume", amplitude2db(track->volume));
		set_float("panning", track->panning * 100.0f);
		//hide_control("edit_midi_fx", track->type != SignalType::Midi);
		//hide_control("edit_synth", track->type != SignalType::Midi);
		enable("edit_synth", track->type == SignalType::Midi or track->type == SignalType::Beats);
		enable("select_synth", track->type == SignalType::Midi or track->type == SignalType::Beats);

		for (auto&& [ii,i]: enumerate(instrument_list)) {
			if (track->instrument.type == i.type)
				set_int("instrument", ii);
		}

		update_strings();


		bool show_synth = track->synth and track_wants_synth(track);
		if (show_synth)
			synth_panel = create_synth_panel(track, session, this);
		else
			synth_panel = create_dummy_synth_panel();
		hide_control("l-no-synth", show_synth);
		hide_control("synth", !show_synth);
		embed(synth_panel, "synth", 0, 0);

		if (track->fx.num > 0)
			set_string("link-to-fx", format(_("%d effects active"), track->fx.num));
		else
			set_string("link-to-fx", _("no effects yet"));

	} else {
		hide_control("td_t_bars", true);
		set_string("tuning", "");
	}
}

void TrackConsole::update_strings() {
	string tuning = format(_("%d strings: <b>"), track->instrument.string_pitch.num);
	for (int i=0; i<track->instrument.string_pitch.num; i++) {
		if (i > 0)
			tuning += ", ";
		tuning += pitch_name(track->instrument.string_pitch[i]);
	}
	tuning += "</b>";
	if (track->instrument.string_pitch.num == 0)
		tuning = "<i>" + _(" - no strings -") + "</i>";
	set_string("tuning", tuning);
}

void TrackConsole::set_track(Track *t) {
	if (t == track)
		return;
	if (track)
		track->unsubscribe(this);
	fx_editor = nullptr;
	track = t;
	load_data();
	if (track) {
		fx_editor = new FxListEditor(track, this, "fx", true);
		track->out_death >> create_sink([this] { set_track(nullptr); });
		track->out_changed >> in_track_update;
		track->out_replace_synthesizer >> in_track_update;
		track->out_effect_list_changed >> in_track_update;
	}
}

void TrackConsole::on_name() {
	editing = true;
	track->set_name(get_string(""));
	editing = false;
}

void TrackConsole::on_volume() {
	editing = true;
	track->set_volume(db2amplitude(get_float("volume")));
	editing = false;
}

void TrackConsole::on_panning() {
	editing = true;
	track->set_panning(get_float("panning") / 100.0f);
	editing = false;
}

void TrackConsole::on_instrument() {
	editing = true;
	int n = get_int("");
	track->set_instrument(instrument_list[n]);
	update_strings();
	editing = false;
}

void TrackConsole::on_edit_strings() {
	auto dlg = new EditStringsDialog(win, track->instrument.string_pitch);
	hui::fly(dlg).then([dlg, this] {
		if (dlg->ok) {
			Instrument i = track->instrument;
			i.string_pitch = dlg->strings;
			track->set_instrument(i);
		}
	});
}

void TrackConsole::on_view_cur_track_change() {
	set_track(view->cur_track());
}

void TrackConsole::on_update() {
	if (!editing)
		load_data();
}

}
