/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../../Data/base.h"
#include "../Helper/Slider.h"
#include "../Helper/ModulePanel.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ConfigPanel.h"
#include "../Dialog/DetuneSynthesizerDialog.h"
#include "../Dialog/TuningDialog.h"
#include "../Dialog/ModuleSelectorDialog.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../AudioView.h"
#include "TrackConsole.h"

hui::Panel *create_dummy_synth_panel() {
	auto panel = new hui::Panel();
	panel->add_label("!expandx,center,disabled\\<i>" + _("only for midi or time tracks") + "</i>", 0, 0, "");
	return panel;
}

hui::Panel *create_synth_panel(Track *track, Session *session, hui::Window *win) {
	auto *p = new ModulePanel(track->synth.get(), nullptr, ModulePanel::Mode::DEFAULT_H);
	//p->set_func_edit([=](const string &param){ track->edit_synthesizer(param); });
	p->set_func_replace([=]{
		string name = session->plugin_manager->choose_module(win, session, ModuleCategory::SYNTHESIZER, track->synth->module_class);
		if (name != "")
			track->set_synthesizer(CreateSynthesizer(session, name));
	});
	p->set_func_detune([=]{
		auto dlg = ownify(new DetuneSynthesizerDialog(track->synth.get(), track, session->view, win));
		dlg->run();
	});
	return p;
}

TrackConsole::TrackConsole(Session *session) :
	SideBarConsole(_("Track"), session)
{
	track = nullptr;
	panel = nullptr;
	editing = false;
	from_resource("track_dialog");
	set_decimals(1);

	auto instruments = Instrument::enumerate();
	for (auto &i: instruments)
		set_string("instrument", i.name());

	event("name", [=]{ on_name(); });
	event("volume", [=]{ on_volume(); });
	event("panning", [=]{ on_panning(); });
	event("instrument", [=]{ on_instrument(); });
	event("edit_tuning", [=]{ on_edit_tuning(); });

	event("edit_song", [=]{ session->set_mode(EditMode::DefaultSong); });
	event("edit_fx", [=]{ session->set_mode(EditMode::DefaultFx); });
	event("edit_curves", [=]{ session->set_mode(EditMode::Curves); });
	event("edit_midi", [=]{ session->set_mode(EditMode::DefaultTrack); });
	event("edit_midi_fx", [=]{ session->set_mode(EditMode::DefaultMidiFx); });
}

void TrackConsole::on_enter() {
	set_track(view->cur_track());
	view->subscribe(this, [=]{ on_view_cur_track_change(); }, view->MESSAGE_CUR_TRACK_CHANGE);

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
		delete panel;
	panel = nullptr;

	if (track) {
		set_string("name", track->name);
		set_options("name", "placeholder=" + track->nice_name());
		set_float("volume", amplitude2db(track->volume));
		set_float("panning", track->panning * 100.0f);
		hide_control("edit_midi", track->type != SignalType::MIDI);
		hide_control("edit_midi_fx", track->type != SignalType::MIDI);
		enable("_edit_synth", track->type == SignalType::MIDI or track->type == SignalType::BEATS);
		enable("select_synth", track->type == SignalType::MIDI or track->type == SignalType::BEATS);

		auto instruments = Instrument::enumerate();
		foreachi(auto &ii, instruments, i) {
			if (track->instrument == ii)
				set_int("instrument", i);
		}

		update_strings();


		if (track->synth and track_wants_synth(track)) {
			panel = create_synth_panel(track, session, win);
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
	if (track)
		track->unsubscribe(this);
	track = t;
	load_data();
	if (track)
		track->subscribe(this, [=]{ on_update(); });
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
	auto instruments = Instrument::enumerate();
	track->set_instrument(instruments[n]);
	update_strings();
	editing = false;
}

void TrackConsole::on_edit_tuning() {
	auto dlg = ownify(new TuningDialog(win, track));
	dlg->run();
}

void TrackConsole::on_view_cur_track_change() {
	set_track(view->cur_track());
}

void TrackConsole::on_update() {
	if (track->cur_message() == track->MESSAGE_DELETE) {
		set_track(nullptr);
	} else {
		if (!editing)
			load_data();
	}
}
