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
#include "../../Session.h"
#include "../../EditModes.h"

hui::Panel *create_dummy_synth_panel() {
	auto panel = new hui::Panel();
	panel->add_label("!expandx,center,disabled\\<i>" + _("only for midi or time tracks") + "</i>", 0, 0, "");
	return panel;
}

hui::Panel *create_synth_panel(Track *track, Session *session, hui::Panel *parent) {
	auto *p = new ModulePanel(track->synth.get(), parent, ConfigPanelMode::FIXED_HEIGHT | ConfigPanelMode::PROFILES | ConfigPanelMode::REPLACE);
	//p->set_func_edit([track](const string &param){ track->edit_synthesizer(param); });
	p->set_func_replace([parent, track, session] {
		session->plugin_manager->choose_module(parent, session, ModuleCategory::SYNTHESIZER, [track, session] (const base::optional<string> &name) {
			if (name.has_value())
				track->set_synthesizer(CreateSynthesizer(session, *name));
		}, track->synth->module_class);
	});
	p->set_func_detune([parent, track, session] {
		hui::fly(new TemperamentDialog(track, session->view, parent->win));
	});
	return p;
}

TrackConsole::TrackConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Track"), "track-console", session, bar)
{
	track = nullptr;
	panel = nullptr;
	editing = false;
	from_resource("track_dialog");
	set_decimals(1);
	set_mode(Mode::FX);

	instrument_list = base::sorted(Instrument::enumerate(), [] (const Instrument &a, const Instrument &b) { return a.name() <= b.name(); });
	for (auto &i: instrument_list)
		set_string("instrument", i.name());

	event("name", [this] { on_name(); });
	event("volume", [this] { on_volume(); });
	event("panning", [this] { on_panning(); });
	event("instrument", [this] { on_instrument(); });
	event("edit_tuning", [this] { on_edit_strings(); });

	event("edit-song", [session] { session->set_mode(EditMode::DefaultSong); });
	event("edit-track-curves", [session] { session->set_mode(EditMode::Curves); });
	event("edit-track-data", [session] { session->set_mode(EditMode::EditTrack); });
}

void TrackConsole::set_mode(Mode mode) {
	if (mode == Mode::FX) {
		expand("g_fx", true);
		set_int("tc", 0);
	} else if (mode == Mode::MIDI_FX) {
		expand("g_fx", true);
		set_int("tc", 1);
	} else if (mode == Mode::SYNTH) {
		expand("g_synth", true);
	}
}

void TrackConsole::on_enter() {
	set_track(view->cur_track());
	view->subscribe(this, [this] { on_view_cur_track_change(); }, view->MESSAGE_CUR_TRACK_CHANGE);

}
void TrackConsole::on_leave() {
	view->unsubscribe(this);
	set_track(nullptr);
}

bool track_wants_synth(const Track *t) {
	return (t->type == SignalType::MIDI) or (t->type == SignalType::BEATS);
}

void TrackConsole::load_data() {
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	enable("instrument", track);
	hide_control("td_t_edit", !track);
	if (panel)
		unembed(panel);
	panel = nullptr;

	if (track) {
		set_string("name", track->name);
		set_options("name", "placeholder=" + track->nice_name());
		set_float("volume", amplitude2db(track->volume));
		set_float("panning", track->panning * 100.0f);
		//hide_control("edit_midi_fx", track->type != SignalType::MIDI);
		//hide_control("edit_synth", track->type != SignalType::MIDI);
		enable("edit_synth", track->type == SignalType::MIDI or track->type == SignalType::BEATS);
		enable("select_synth", track->type == SignalType::MIDI or track->type == SignalType::BEATS);

		for (auto&& [ii,i]: enumerate(instrument_list)) {
			if (track->instrument.type == i.type)
				set_int("instrument", ii);
		}

		update_strings();


		if (track->synth and track_wants_synth(track)) {
			panel = create_synth_panel(track, session, this);
		} else {
			panel = create_dummy_synth_panel();
		}
		embed(panel, "synth", 0, 0);

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
		track->subscribe(this, [this] { set_track(nullptr); }, track->MESSAGE_DELETE);
		track->subscribe(this, [this] { on_update(); }, track->MESSAGE_ANY);
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
	hui::fly(dlg, [dlg, this] {
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
