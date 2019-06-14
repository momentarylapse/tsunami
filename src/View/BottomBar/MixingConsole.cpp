/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../Helper/PeakMeterDisplay.h"
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
#include "../../Stream/AudioOutput.h"
#include "../AudioView.h"
#include "../Node/AudioViewTrack.h"

//string module_header(Module *m);

extern const int CONFIG_PANEL_WIDTH;

class TrackMixer: public hui::Panel {
public:
	TrackMixer(AudioViewTrack *t, MixingConsole *c) {
		set_border_width(0);
		from_resource("track-mixer2");

		track = nullptr;
		vtrack = nullptr;
		editing = false;
		console = c;

		reveal("revealer-volume", true);
		hide_control("grid-fx", true);
		hide_control("grid-midi-fx", true);

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
		event("add-fx", [=]{ on_add_fx(); });
		event_x("midi-fx", "hui:select", [=]{ on_midi_fx_select(); });
		event_x("midi-fx", "hui:change", [=]{ on_midi_fx_edit(); });
		event_x("midi-fx", "hui:move", [=]{ on_midi_fx_move(); });
		event("add-midi-fx", [=]{ on_add_midi_fx(); });
		event_xp("peaks", "hui:draw", [=](Painter* p){ on_peak_draw(p); });

		vtrack = t;
		track = t->track;
		vtrack->subscribe(this, [=]{ update(); }, vtrack->MESSAGE_CHANGE);
		vtrack->subscribe(this, [=]{ on_vtrack_delete(); }, vtrack->MESSAGE_DELETE);
		update();
	}
	~TrackMixer() {
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
	}
	void on_vtrack_delete() {
		clear_track();
	}
	void set_mode(MixerMode mode) {
		hide_control("grid-volume", mode != MixerMode::VOLUME);
		hide_control("grid-fx", mode != MixerMode::EFFECTS);
		hide_control("grid-midi-fx", mode != MixerMode::MIDI_EFFECTS);
		reveal("revealer-volume", mode == MixerMode::VOLUME);
		reveal("revealer-fx", mode == MixerMode::EFFECTS);
		reveal("revealer-midi-fx", mode == MixerMode::MIDI_EFFECTS);
	}
	void on_fx_select() {
		int n = get_int("");
		if (n >= 0)
			console->select_module(track->fx[n]);
		else
			console->select_module(nullptr);
	}
	void on_fx_edit() {
		int n = hui::GetEvent()->row;
		if (n >= 0)
			track->enable_effect(track->fx[n], get_cell("", n, hui::GetEvent()->column)._bool());
	}
	void on_fx_move() {
		int s = hui::GetEvent()->row;
		int t = hui::GetEvent()->row_target;
		track->move_effect(s, t);
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
			console->select_module(track->midi_fx[n]);
		else
			console->select_module(nullptr);
	}
	void on_midi_fx_edit() {
		int n = hui::GetEvent()->row;
		if (n >= 0)
			track->enable_midi_effect(track->midi_fx[n], get_cell("", n, hui::GetEvent()->column)._bool());
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
			set_string(id_name, track->nice_name());
		else
			set_string(id_name, "<s>" + track->nice_name() + "</s>");

		// fx list
		reset("fx");
		for (auto *fx: vtrack->track->fx)
			add_string("fx", b2s(fx->enabled) + "\\" + fx->module_subtype);
		set_int("fx", vtrack->track->fx.find((AudioEffect*)console->selected_module));

		// midi fx list
		reset("midi-fx");
		for (auto *fx: vtrack->track->midi_fx)
			add_string("midi-fx", b2s(fx->enabled) + "\\" + fx->module_subtype);
		set_int("midi-fx", vtrack->track->midi_fx.find((MidiEffect*)console->selected_module));
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




class ModulePanel : public hui::Panel
{
public:
	ModulePanel(Session *_session, MixingConsole *_console, Module *_m, std::function<void(bool)> _func_enable, std::function<void()> _func_delete, std::function<void(const string&)> _func_edit) {
		session = _session;
		console = _console;
		module = _m;

		func_enable = _func_enable;
		func_delete = _func_delete;
		func_edit = _func_edit;

		from_resource("fx_panel");

		set_string("name", module->module_subtype);

		p = module->create_panel();
		if (p) {
			embed(p, "content", 0, 0);
			p->update();
		} else {
			set_target("content");
			add_label(_("not configurable"), 0, 1, "");
			hide_control("load_favorite", true);
			hide_control("save_favorite", true);
		}
		set_options("content", format("width=%d", CONFIG_PANEL_WIDTH));

		event("enabled", [=]{ on_enabled(); });
		event("delete", [=]{ on_delete(); });
		event("load_favorite", [=]{ on_load(); });
		event("save_favorite", [=]{ on_save(); });
		event("show_large", [=]{ on_large(); });

		check("enabled", module->enabled);

		old_param = module->config_to_string();
		module->subscribe(this, [=]{ on_fx_change(); }, module->MESSAGE_CHANGE);
		module->subscribe(this, [=]{ on_fx_change_by_action(); }, module->MESSAGE_CHANGE_BY_ACTION);
	}
	virtual ~ModulePanel() {
		module->unsubscribe(this);
	}
	void on_load() {
		string name = session->plugin_manager->select_favorite_name(win, module, false);
		if (name.num == 0)
			return;
		session->plugin_manager->apply_favorite(module, name);
		func_edit(old_param);
		old_param = module->config_to_string();
	}
	void on_save() {
		string name = session->plugin_manager->select_favorite_name(win, module, true);
		if (name.num == 0)
			return;
		session->plugin_manager->save_favorite(module, name);
	}
	void on_enabled() {
		func_enable(is_checked(""));
	}
	void on_delete() {
		hui::RunLater(0, func_delete);
	}
	void on_large() {
		//console->set_exclusive(this);
		p->set_large(true);

	}
	void on_fx_change() {
		func_edit(old_param);
		check("enabled", module->enabled);
		if (p)
			p->update();
		old_param = module->config_to_string();

	}
	void on_fx_change_by_action() {
		check("enabled", module->enabled);
		if (p)
			p->update();
		old_param = module->config_to_string();
	}
	std::function<void(bool)> func_enable;
	std::function<void(const string&)> func_edit;
	std::function<void()> func_delete;
	Session *session;
	Module *module;
	string old_param;
	ConfigPanel *p;
	MixingConsole *console;
};



MixingConsole::MixingConsole(Session *session) :
	BottomBar::Console(_("Mixer"), session)
{
	device_manager = session->device_manager;
	id_inner = "inner-grid";

	set_border_width(2);
	from_resource("mixing-console");

	config_panel = nullptr;
	selected_module = nullptr;
	select_module(nullptr);

	peak_meter = new PeakMeterDisplay(this, "output-peaks", view->peak_meter, PeakMeterDisplay::Mode::PEAKS);
	spectrum_meter = new PeakMeterDisplay(this, "output-spectrum", view->peak_meter, PeakMeterDisplay::Mode::SPECTRUM);
	set_float("output-volume", device_manager->get_output_volume());

	event("output-volume", [=]{ on_output_volume(); });
	event("show-vol", [=]{ set_mode(MixerMode::VOLUME); });
	event("show-fx", [=]{ set_mode(MixerMode::EFFECTS); });
	event("show-midi-fx", [=]{ set_mode(MixerMode::MIDI_EFFECTS); });

	view->subscribe(this, [=]{ on_tracks_change(); }, session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, [=]{ update_all(); }, session->view->MESSAGE_SOLO_CHANGE);
	song->subscribe(this, [=]{ update_all(); }, song->MESSAGE_FINISHED_LOADING);

	set_mode(MixerMode::VOLUME);

	device_manager->subscribe(this, [=]{ on_update_device_manager(); });
	load_data();


	peak_runner_id = -1;
	view->signal_chain->subscribe(this, [=]{ on_chain_state_change(); }, SignalChain::MESSAGE_STATE_CHANGE);
}

MixingConsole::~MixingConsole()
{
	view->signal_chain->unsubscribe(this);
	if (peak_runner_id >= 0)
		hui::CancelRunner(peak_runner_id);
	//song->unsubscribe(this);
	view->unsubscribe(this);
	device_manager->unsubscribe(this);
	select_module(nullptr);
	for (TrackMixer *m: mixer)
		delete m;
	delete peak_meter;
	delete spectrum_meter;
}

void MixingConsole::on_chain_state_change() {
	if (peak_runner_id and !view->signal_chain->is_playback_active()) {
		hui::CancelRunner(peak_runner_id);
		peak_runner_id = -1;
		// clear
		view->renderer->clear_peaks();
		for (auto *m: mixer)
			m->redraw("peaks");
	} else if (peak_runner_id == -1 and view->signal_chain->is_playback_active()) {
		peak_runner_id = hui::RunRepeated(0.1f, [=]{ for (auto *m: mixer) m->redraw("peaks"); });
	}
}

void MixingConsole::on_output_volume() {
	device_manager->set_output_volume(get_float(""));
}

void MixingConsole::set_mode(MixerMode _mode) {
	mode = _mode;
	for (auto *m: mixer)
		m->set_mode(mode);
	check("show-vol", mode == MixerMode::VOLUME);
	check("show-fx", mode == MixerMode::EFFECTS);
	check("show-midi-fx", mode == MixerMode::MIDI_EFFECTS);
}

void MixingConsole::load_data() {
	// how many TrackMixers still match?
	int n_ok = 0;
	foreachi (auto *m, mixer, i) {
		if (!m->vtrack)
			break;
		if (i >= view->vtrack.num)
			break;
		if (m->vtrack != view->vtrack[i])
			break;
		n_ok ++;
	}

	// delete non-matching
	for (int i=n_ok; i<mixer.num; i++) {
		delete mixer[i];
		remove_control("separator-" + i2s(i));
	}
	mixer.resize(n_ok);

	// add new
	foreachi(auto *t, view->vtrack, i) {
		if (i >= n_ok) {
			TrackMixer *m = new TrackMixer(t, this);
			mixer.add(m);
			embed(m, id_inner, i*2, 0);
			add_separator("!vertical", i*2 + 1, 0, "separator-" + i2s(i));
			m->set_mode(mode);
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

void MixingConsole::select_module(Module *m) {
	if (selected_module)
		selected_module->unsubscribe(this);

	if (config_panel)
		delete config_panel;
	config_panel = nullptr;

	selected_module = m;

	string config_grid_id = "config-panel-grid";

	Track *track = nullptr;

	if (selected_module) {

		if (m->module_type == ModuleType::AUDIO_EFFECT) {
			AudioEffect *fx = (AudioEffect*)m;
			for (auto *mm: mixer)
				if (mm->track->fx.find(fx) >= 0)
					track = mm->track;
			config_panel = new ModulePanel(session, this, m,
					[=](bool enabled){ track->enable_effect(fx, enabled); },
					[=]{ track->delete_effect(fx); },
					[=](const string &old_param){ track->edit_effect(fx, old_param); });
		} else { // MIDI_EFFECT
			MidiEffect *fx = (MidiEffect*)m;
			for (auto *mm: mixer)
				if (mm->track->midi_fx.find(fx) >= 0)
					track = mm->track;
			config_panel = new ModulePanel(session, this, m,
					[=](bool enabled){ track->enable_midi_effect(fx, enabled); },
					[=]{ track->delete_midi_effect(fx); },
					[=](const string &old_param){ track->edit_midi_effect(fx, old_param); });
		}
		embed(config_panel, config_grid_id, 0, 0);

		m->subscribe(this, [=]{ select_module(nullptr); }, m->MESSAGE_DELETE);
	}

	reveal("config-revealer", m);


	// make sure only 1 item is selected in lists
	for (auto *m: mixer)
		if (m->track != track)
			m->set_int("fx", -1);
}

void MixingConsole::update_all() {
	for (auto *m: mixer)
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
