/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../Helper/PeakMeterDisplay.h"
#include "../Helper/ModulePanel.h"
#include "../Helper/FxListEditor.h"
#include "../Helper/Drawing.h"
#include "../AudioView.h"
#include "../Graph/AudioViewTrack.h"
#include "../Graph/TrackHeader.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Module/Audio/AudioEffect.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../../Module/Midi/MidiEffect.h"
#include "../../Module/ConfigPanel.h"
#include "../../Module/SignalChain.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../Plugins/PluginManager.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Stream/AudioOutput.h"
#include <math.h>


Array<int> track_group_colors(Track *t);

class TrackMixer: public hui::Panel {
public:
	TrackMixer(AudioViewTrack *t, MixingConsole *c) {
		set_spacing(0);
		from_resource("track-mixer2");

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

		event_xp(id_name, "hui:draw", [=] (Painter* p) { on_name_draw(p); });
		event_x(id_name, "hui:left-button-down", [=] { on_name_left_click(); });
		event_x(id_name, "hui:right-button-down", [=] { on_name_right_click(); });
		event_x(id_name, "hui:left-double-click", [=] { on_name_double_click(); });
		event(vol_slider_id, [=]{ on_volume(); });
		event(pan_slider_id, [=]{ on_panning(); });
		event(mute_id, [=]{ on_mute(); });
		event("solo", [=]{ on_solo(); });
		event_xp("peaks", "hui:draw", [=] (Painter* p) { on_peak_draw(p); });
		event("show-fx", [=]{ on_show_fx(is_checked("")); });

		vtrack = t;
		vtrack->subscribe(this, [=]{ update(); }, vtrack->MESSAGE_CHANGE);
		vtrack->subscribe(this, [=]{ on_vtrack_delete(); }, vtrack->MESSAGE_DELETE);
		vtrack->view->subscribe(this, [=] { redraw(id_name); }, AudioView::MESSAGE_SELECTION_CHANGE);
		fx_editor = new FxListEditor(track(), this, "fx", "midi-fx", false);
		update();
	}
	~TrackMixer() {
		vtrack->view->unsubscribe(this);
		fx_editor->select_module(nullptr);
		clear_track();
	}

	void on_name_draw(Painter *p) {
		auto t = track();
		if (!t)
			return;

		auto view = console->view;
		bool is_playable = view->get_playable_tracks().contains(track());

		p->set_color(vtrack->header->color_bg());
		p->set_roundness(theme.CORNER_RADIUS);
		p->draw_rect(rect(0, p->width, -theme.CORNER_RADIUS, p->height));
		p->set_roundness(0);

		auto group_colors = track_group_colors(track());
		foreachi (int g, group_colors, i) {
			p->set_color(theme.pitch_soft1[g]);
			float y = 5*(group_colors.num - i - 1);
			p->draw_rect(rect(0, p->width, y, y+5));
		}

		string tt = nice_title();
		p->set_color(vtrack->header->color_text());
		if (is_playable) {
			p->set_font("", theme.FONT_SIZE, true, false);
		} else {
			p->set_font("", theme.FONT_SIZE, false, true);
			//set_string(id_name, "<s>" + nice_title() + "</s>");
			//float w = p->get_str_width(tt);
			//p->draw_str((p->width - w) / 2, 8, tt);
		}
		draw_str_constrained(p, p->width/2, 8, p->width, tt, TextAlign::CENTER);
	}

	void set_current() {
		auto view = console->view;
		HoverData h;
		h.vtrack = vtrack;
		h.vlayer = vtrack->first_layer();
		view->set_current(h);
		view->select_xor = win->get_key(hui::KEY_CONTROL);
	}

	void on_name_left_click() {
		set_current();
		auto view = console->view;
		if (view->select_xor) {
			view->toggle_select_track_with_content_in_cursor(vtrack);
		} else {
			if (view->sel.has(vtrack->track)) {
				view->set_selection(view->sel.restrict_to_track(vtrack->track));
			} else {
				view->exclusively_select_track(vtrack);
				view->select_under_cursor();
			}
		}
	}
	void on_name_right_click() {
		set_current();
		auto view = console->view;
		if (!view->sel.has(vtrack->track)) {
			view->exclusively_select_layer(vtrack->first_layer());
			view->select_under_cursor();
		}
		view->open_popup(view->menu_track.get());
	}

	void on_name_double_click() {
		set_current();
		console->session->set_mode(EditMode::DefaultTrack);
	}

	void on_volume() {
		editing = true;
		if (track()) {
			if (parent->is_checked("link-volumes"))
				track()->song->change_all_track_volumes(track(), slider2vol(get_float("")));
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
		fx_editor->select_module(nullptr);
	}
	void on_vtrack_delete() {
		clear_track();
	}
	void on_show_fx(bool show) {
		if (show)
			console->show_fx(track());
		else
			console->show_fx(nullptr);
	}
	void show_fx(bool show) {
		reveal("revealer-fx", show);
		check("show-fx", show);
		fx_editor->select_module(nullptr);
	}

	owned<FxListEditor> fx_editor;
	void on_peak_draw(Painter* p) {
		int w = p->width;
		int h = p->height;
		p->set_color(theme.background);
		p->draw_rect(0, 0, w, h);
		p->set_color(theme.text);
		float peak[2];
		console->view->renderer->get_peak(track(), peak);
		peak[0] = sqrt(peak[0]);
		peak[1] = sqrt(peak[1]);
		p->draw_rect(0,   h * (1 - peak[0]), w/2, h * peak[0]);
		p->draw_rect(w/2, h * (1 - peak[1]), w/2, h * peak[1]);
	}
	string nice_title() {
		string s = track()->nice_name();
		if (vtrack->solo)
			s = u8"\u00bb " + s + u8" \u00ab";
		return s;
	}
	void update() {
		if (!vtrack)
			return;
		if (editing)
			return;
		set_float(vol_slider_id, vol2slider(track()->volume));
		set_float(pan_slider_id, track()->panning);
		check(mute_id, track()->muted);
		check("solo", vtrack->solo);
		redraw(id_name);
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


	Track *track() const {
		if (!vtrack)
			return nullptr;
		return vtrack->track;
	}

	AudioViewTrack *vtrack;
	//Slider *volume_slider;
	//Slider *panning_slider;
	string id_name;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id;
	string id_separator;
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
	//set_float("output-volume", device_manager->get_output_volume());
	set_float("output-volume", view->output_stream->get_volume());
	
	peak_meter->enable(false);
	spectrum_meter->enable(false);

	event("output-volume", [=]{ on_output_volume(); });

	view->subscribe(this, [=]{ on_tracks_change(); }, session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, [=]{ update_all(); }, session->view->MESSAGE_SOLO_CHANGE);
	song->subscribe(this, [=]{ update_all(); }, song->MESSAGE_FINISHED_LOADING);

	//device_manager->subscribe(this, [=]{ on_update_device_manager(); });
	view->output_stream->subscribe(this, [=]{
		set_float("output-volume", view->output_stream->get_volume());

	});
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
	view->output_stream->unsubscribe(this);
	//device_manager->unsubscribe(this);
}

void MixingConsole::on_chain_state_change() {
	if (peak_runner_id and !view->is_playback_active()) {
		hui::CancelRunner(peak_runner_id);
		peak_runner_id = -1;
		// clear
		view->renderer->clear_peaks();
		for (auto &m: mixer)
			m->redraw("peaks");
	} else if (peak_runner_id == -1 and view->is_playback_active()) {
		peak_runner_id = hui::RunRepeated(0.1f, [=]{ for (auto *m: mixer) m->redraw("peaks"); });
	}
}

void MixingConsole::on_output_volume() {
	view->output_stream->set_volume(get_float(""));
	//device_manager->set_output_volume(get_float(""));
}

void MixingConsole::show_fx(Track *t) {
	for (auto &m: mixer)
		m->show_fx(m->track() == t);
}

void MixingConsole::load_data() {
	// how many TrackMixers still match?
	int n_ok = 0;
	foreachi (auto &m, mixer, i) {
		if (!m->vtrack)
			break;
		if (i >= view->vtracks.num)
			break;
		if (m->vtrack != view->vtracks[i])
			break;
		n_ok ++;
	}

	// delete non-matching
	for (int i=n_ok; i<mixer.num; i++)
		remove_control("separator-" + i2s(i));
	mixer.resize(n_ok);

	// add new
	foreachi(auto *t, view->vtracks, i) {
		if (i >= n_ok) {
			auto m = new TrackMixer(t, this);
			mixer.add(m);
			embed(m, id_inner, i*2, 0);
			add_separator("!vertical", i*2 + 1, 0, "separator-" + i2s(i));
		}
	}

	hide_control("link-volumes", mixer.num <= 1);
}

void MixingConsole::on_update_device_manager() {
	//set_float("output-volume", device_manager->get_output_volume());
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
