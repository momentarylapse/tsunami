/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../Helper/PeakMeterDisplay.h"
#include "../Helper/ModulePanel.h"
#include <math.h>

#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Module/Audio/AudioEffect.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../../Module/Midi/MidiEffect.h"
#include "../../Module/ConfigPanel.h"
#include "../../Module/SignalChain.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Stream/AudioOutput.h"
#include "../AudioView.h"
#include "../Graph/AudioViewTrack.h"

class TrackSelectionDialog : public hui::Dialog {
public:
	Song *song;
	Track *selected = nullptr;
	TrackSelectionDialog(hui::Window *parent, Song *_song) : hui::Dialog("track-selector", parent) {
		song = _song;
		for (Track *t: weak(song->tracks))
			add_string("tracks", t->nice_name());
		event("tracks", [=]{ on_select(); });
		event("ok", [=]{ on_select(); });
		event("cancel", [=]{ destroy(); });
	}
	void on_select() {
		int n = get_int("");
		if (n >= 0)
			selected = song->tracks[n].get();
		destroy();
	}
};

class TrackMixer: public hui::Panel {
public:
	TrackMixer(AudioViewTrack *t, MixingConsole *c) {
		set_spacing(0);
		from_resource("track-mixer2");

		track = nullptr;
		vtrack = nullptr;
		editing = false;
		console = c;

		reveal("revealer-fx", false);
		reveal("config-revealer", false);

		id_separator = "mixing-track-separator";
		id_name = "name";
		vol_slider_id = "volume";
		pan_slider_id = "panning";
		mute_id = "mute";
		add_string(pan_slider_id, "-1\\L");
		add_string(pan_slider_id, "0\\");
		add_string(pan_slider_id, "1\\R");
		add_string(vol_slider_id, format("%f\\%+d", db2slider(DB_MAX), (int)DB_MAX));
		add_string(vol_slider_id, format("%f\\%+d", db2slider(5), (int)5));
		add_string(vol_slider_id, format("%f\\%d", db2slider(0), 0));
		add_string(vol_slider_id, format("%f\\%d", db2slider(-5), (int)-5));
		add_string(vol_slider_id, format("%f\\%d", db2slider(-10), (int)-10));
		add_string(vol_slider_id, format("%f\\%d", db2slider(-20), (int)-20));
		add_string(vol_slider_id, format(u8"%f\\-\u221e", db2slider(DB_MIN))); // \u221e

		event(vol_slider_id, [=]{ on_volume(); });
		event(pan_slider_id, [=]{ on_panning(); });
		event(mute_id, [=]{ on_mute(); });
		event("solo", [=]{ on_solo(); });
		event_x("fx", "hui:select", [=]{ on_fx_select(); });
		event_x("fx", "hui:change", [=]{ on_fx_edit(); });
		event_x("fx", "hui:move", [=]{ on_fx_move(); });
		event_x("fx", "hui:right-button-down", [=]{ on_fx_right_click(); });
		event("add-fx", [=]{ on_add_fx(); });
		event("fx-add", [=]{ on_add_fx(); });
		event("fx-delete", [=]{ on_fx_delete(); });
		event("fx-enabled", [=]{ on_fx_enabled(); });
		event("fx-copy-from-track", [=]{ on_fx_copy_from_track(); });
		event_x("midi-fx", "hui:select", [=]{ on_midi_fx_select(); });
		event_x("midi-fx", "hui:change", [=]{ on_midi_fx_edit(); });
		event_x("midi-fx", "hui:move", [=]{ on_midi_fx_move(); });
		event("add-midi-fx", [=]{ on_add_midi_fx(); });
		event_xp("peaks", "hui:draw", [=](Painter* p){ on_peak_draw(p); });
		event("show-fx", [=]{ on_show_fx(is_checked("")); });

		vtrack = t;
		track = t->track;
		auto *song = console->session->song.get();
		song->subscribe(this, [=]{ update(); }, song->MESSAGE_ENABLE_FX);
		vtrack->subscribe(this, [=]{ update(); }, vtrack->MESSAGE_CHANGE);
		vtrack->subscribe(this, [=]{ on_vtrack_delete(); }, vtrack->MESSAGE_DELETE);
		update();
	}
	~TrackMixer() {
		select_module(nullptr);
		console->session->song->unsubscribe(this);
		clear_track();
	}

	void on_volume() {
		editing = true;
		if (track) {
			if (parent->is_checked("link-volumes"))
				track->song->change_all_track_volumes(track, slider2vol(get_float("")));
			else
				vtrack->set_volume(slider2vol(get_float("")));
		}
		editing = false;
	}

	// allow update() for mute/solo!
	void on_mute() {
		if (vtrack)
			vtrack->set_muted(is_checked(""));
	}
	void on_solo() {
		if (vtrack)
			vtrack->set_solo(is_checked(""));
	}

	void on_panning() {
		editing = true;
		if (vtrack)
			vtrack->set_panning(get_float(""));
		editing = false;
	}
	void clear_track() {
		if (vtrack)
			vtrack->unsubscribe(this);
		vtrack = nullptr;
		track = nullptr;
		select_module(nullptr);
	}
	void on_vtrack_delete() {
		clear_track();
	}
	void on_show_fx(bool show) {
		if (show)
			console->show_fx(track);
		else
			console->show_fx(nullptr);
	}
	void show_fx(bool show) {
		reveal("revealer-fx", show);
		check("show-fx", show);
		select_module(nullptr);
	}

	Module *selected_module = nullptr;
	owned<ModulePanel> config_panel;

	void select_module(Module *m) {
		if (selected_module)
			selected_module->unsubscribe(this);

		config_panel = nullptr;

		selected_module = m;

		string config_grid_id = "grid-config";

		if (selected_module) {
			config_panel = new ModulePanel(m);

			if (m->module_type == ModuleType::AUDIO_EFFECT) {
				AudioEffect *fx = (AudioEffect*)m;
				config_panel->set_func_enable([=](bool enabled){ track->enable_effect(fx, enabled); });
				config_panel->set_func_delete([=]{ track->delete_effect(fx); });
			} else { // MIDI_EFFECT
				MidiEffect *fx = (MidiEffect*)m;
				config_panel->set_func_enable([=](bool enabled){ track->enable_midi_effect(fx, enabled); });
				config_panel->set_func_delete([=]{ track->delete_midi_effect(fx); });
			}
			embed(config_panel.get(), config_grid_id, 0, 0);

			m->subscribe(this, [=]{ select_module(nullptr); }, m->MESSAGE_DELETE);
		}

		update_fx_list_selection();
		reveal("config-revealer", m);
	}

	void on_fx_select() {
		int n = get_int("");
		if (n >= 0)
			select_module(track->fx[n].get());
		else
			select_module(nullptr);
	}
	void on_fx_edit() {
		int n = hui::GetEvent()->row;
		if (n >= 0)
			track->enable_effect(track->fx[n].get(), get_cell("", n, hui::GetEvent()->column)._bool());
	}
	void on_fx_move() {
		int s = hui::GetEvent()->row;
		int t = hui::GetEvent()->row_target;
		track->move_effect(s, t);
	}
	void on_fx_right_click() {
		int n = hui::GetEvent()->row;
		console->menu_fx->enable("fx-delete", n >= 0);
		console->menu_fx->enable("fx-enabled", n >= 0);
		if (n >= 0)
			console->menu_fx->check("fx-enabled", track->fx[n]->enabled);
		console->menu_fx->open_popup(this);
	}
	void on_fx_delete() {
		int n = get_int("fx");
		if (n >= 0)
			track->delete_effect(track->fx[n].get());
	}
	void on_fx_enabled() {
		int n = get_int("fx");
		if (n >= 0)
			track->enable_effect(track->fx[n].get(), !track->fx[n]->enabled);
	}
	void on_fx_copy_from_track() {
		auto dlg = ownify(new TrackSelectionDialog(win, track->song));
		dlg->run();
		if (dlg->selected and dlg->selected != track) {
			track->song->begin_action_group();
			foreachb (auto *fx, weak(track->fx))
				track->delete_effect(fx);
			for (auto *fx: weak(dlg->selected->fx))
				track->add_effect((AudioEffect*)fx->copy());
			track->song->end_action_group();
		}
	}
	void on_add_fx() {
		string name = console->session->plugin_manager->choose_module(win, console->session, ModuleType::AUDIO_EFFECT);
		if (name == "")
			return;
		auto *effect = CreateAudioEffect(console->session, name);
		track->add_effect(effect);
	}
	void on_midi_fx_select() {
		int n = get_int("");
		if (n >= 0)
			select_module(track->midi_fx[n].get());
		else
			select_module(nullptr);
	}
	void on_midi_fx_edit() {
		int n = hui::GetEvent()->row;
		if (n >= 0)
			track->enable_midi_effect(track->midi_fx[n].get(), get_cell("", n, hui::GetEvent()->column)._bool());
	}
	void on_midi_fx_move() {
		int s = hui::GetEvent()->row;
		int t = hui::GetEvent()->row_target;
		track->move_midi_effect(s, t);
	}
	void on_add_midi_fx() {
		string name = console->session->plugin_manager->choose_module(win, console->session, ModuleType::MIDI_EFFECT);
		if (name == "")
			return;
		auto *effect = CreateMidiEffect(console->session, name);
		track->add_midi_effect(effect);
	}
	void on_peak_draw(Painter* p) {
		int w = p->width;
		int h = p->height;
		p->set_color(AudioView::colors.background);
		p->draw_rect(0, 0, w, h);
		p->set_color(AudioView::colors.text);
		float f = sqrt(console->view->renderer->get_peak(track));
		p->draw_rect(0, h * (1 - f), w, h * f);
	}
	string nice_title() {
		string s = track->nice_name();
		if (vtrack->solo)
			s = u8"\u00bb " + s + u8" \u00ab";
		if (s.num > 16)
			return s.head(8) + ".." + s.tail(8);
		return s;
	}
	void update() {
		if (!vtrack)
			return;
		if (editing)
			return;
		set_float(vol_slider_id, vol2slider(track->volume));
		set_float(pan_slider_id, track->panning);
		check(mute_id, track->muted);
		check("solo", vtrack->solo);
		bool is_playable = vtrack->view->get_playable_tracks().contains(track);
		enable(id_name, is_playable);
		if (is_playable)
			set_string(id_name, nice_title());
		else
			set_string(id_name, "<s>" + nice_title() + "</s>");

		// fx list
		reset("fx");
		for (auto *fx: weak(track->fx))
			add_string("fx", b2s(fx->enabled) + "\\" + fx->module_subtype);

		// midi fx list
		reset("midi-fx");
		for (auto *fx: weak(track->midi_fx))
			add_string("midi-fx", b2s(fx->enabled) + "\\" + fx->module_subtype);
		
		update_fx_list_selection();
	}
	void update_fx_list_selection() {
		if (track) {
			set_int("fx", weak(track->fx).find((AudioEffect*)selected_module));
			set_int("midi-fx", weak(track->midi_fx).find((MidiEffect*)selected_module));
		}
	}


	static constexpr float DB_MIN = -1000000;
	static constexpr float DB_MAX = 10;
	static constexpr float TAN_SCALE = 10.0f;

	static float db2slider(float db) {
		return (atan(db / TAN_SCALE) - atan(DB_MIN / TAN_SCALE)) / (atan(DB_MAX / TAN_SCALE) - atan(DB_MIN / TAN_SCALE));
	}
	static float slider2db(float val) {
		return tan(atan(DB_MIN / TAN_SCALE) + val * (atan(DB_MAX / TAN_SCALE)- atan(DB_MIN / TAN_SCALE))) * TAN_SCALE;
	}
	static float vol2slider(float vol) {
		return db2slider(amplitude2db(vol));
	}
	static float slider2vol(float val) {
		return db2amplitude(slider2db(val));
	}


	Track *track;
	AudioViewTrack *vtrack;
	//Slider *volume_slider;
	//Slider *panning_slider;
	string id_name;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id;
	string id_separator;
	AudioView *view;
	bool editing;
	MixingConsole *console;
};





MixingConsole::MixingConsole(Session *session) :
	BottomBar::Console(_("Mixer"), session)
{
	device_manager = session->device_manager;
	id_inner = "inner-grid";

	set_spacing(2);
	from_resource("mixing-console");

	peak_meter = new PeakMeterDisplay(this, "output-peaks", view->peak_meter, PeakMeterDisplay::Mode::PEAKS);
	spectrum_meter = new PeakMeterDisplay(this, "output-spectrum", view->peak_meter, PeakMeterDisplay::Mode::SPECTRUM);
	set_float("output-volume", device_manager->get_output_volume());
	
	peak_meter->enable(false);
	spectrum_meter->enable(false);
	
	menu_fx = hui::CreateResourceMenu("popup-menu-fx-list");

	event("output-volume", [=]{ on_output_volume(); });

	view->subscribe(this, [=]{ on_tracks_change(); }, session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, [=]{ update_all(); }, session->view->MESSAGE_SOLO_CHANGE);
	song->subscribe(this, [=]{ update_all(); }, song->MESSAGE_FINISHED_LOADING);

	device_manager->subscribe(this, [=]{ on_update_device_manager(); });
	load_data();


	peak_runner_id = -1;
	view->signal_chain->subscribe(this, [=]{ on_chain_state_change(); }, SignalChain::MESSAGE_STATE_CHANGE);
}

MixingConsole::~MixingConsole() {
	view->signal_chain->unsubscribe(this);
	if (peak_runner_id >= 0)
		hui::CancelRunner(peak_runner_id);
	//song->unsubscribe(this);
	view->unsubscribe(this);
	device_manager->unsubscribe(this);
}

void MixingConsole::on_chain_state_change() {
	if (peak_runner_id and !view->signal_chain->is_playback_active()) {
		hui::CancelRunner(peak_runner_id);
		peak_runner_id = -1;
		// clear
		view->renderer->clear_peaks();
		for (auto &m: mixer)
			m->redraw("peaks");
	} else if (peak_runner_id == -1 and view->signal_chain->is_playback_active()) {
		peak_runner_id = hui::RunRepeated(0.1f, [=]{ for (auto *m: mixer) m->redraw("peaks"); });
	}
}

void MixingConsole::on_output_volume() {
	device_manager->set_output_volume(get_float(""));
}

void MixingConsole::show_fx(Track *t) {
	for (auto &m: mixer)
		m->show_fx(m->track == t);
}

void MixingConsole::load_data() {
	// how many TrackMixers still match?
	int n_ok = 0;
	foreachi (auto &m, mixer, i) {
		if (!m->vtrack)
			break;
		if (i >= view->vtrack.num)
			break;
		if (m->vtrack != view->vtrack[i])
			break;
		n_ok ++;
	}

	// delete non-matching
	for (int i=n_ok; i<mixer.num; i++)
		remove_control("separator-" + i2s(i));
	mixer.resize(n_ok);

	// add new
	foreachi(auto *t, view->vtrack, i) {
		if (i >= n_ok) {
			TrackMixer *m = new TrackMixer(t, this);
			mixer.add(m);
			embed(m, id_inner, i*2, 0);
			add_separator("!vertical", i*2 + 1, 0, "separator-" + i2s(i));
		}
	}

	hide_control("link-volumes", mixer.num <= 1);
}

void MixingConsole::on_update_device_manager() {
	set_float("output-volume", device_manager->get_output_volume());
}

void MixingConsole::on_tracks_change() {
	load_data();
}

void MixingConsole::update_all() {
	for (auto &m: mixer)
		m->update();
}

void MixingConsole::on_show() {
	peak_meter->enable(true);
	spectrum_meter->enable(true);
}

void MixingConsole::on_hide() {
	peak_meter->enable(false);
	spectrum_meter->enable(false);
}
