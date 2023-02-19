/*
 * MixingConsole.cpp
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#include "MixingConsole.h"
#include "../helper/PeakMeterDisplay.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../helper/FxListEditor.h"
#include "../helper/Drawing.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../audioview/graph/TrackHeader.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../module/audio/AudioEffect.h"
#include "../../module/audio/SongRenderer.h"
#include "../../module/SignalChain.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../plugins/PluginManager.h"
#include "../../device/DeviceManager.h"
#include "../../device/stream/AudioOutput.h"
#include <math.h>


Array<int> track_group_colors(Track *t);
Array<Track*> track_group_members(Track *group, bool with_self);

const int TRACK_MIXER_WIDTH = 100;
const int TRACK_MIXER_WIDTH_SHRUNK = 20;

class TrackMixer: public hui::Panel {
public:
	int shrink_button_x = 0;

	TrackMixer(AudioViewTrack *t, MixingConsole *c) {
		set_spacing(0);
		from_resource("track-mixer2");

		console = c;

		expand("revealer-fx", false);
		expand("config-revealer", false);
		set_options("grid-volume", format("width=%d", TRACK_MIXER_WIDTH));

		id_separator = "mixing-track-separator";
		id_name = "name";
		vol_slider_id = "volume";
		pan_slider_id = "panning";
		mute_id = "mute";
		solo_id = "solo";
		fx_id = "show-fx";
		id_fx_header = "mixer-fx-header";
		add_string(pan_slider_id, "-1\\<span size='x-small'>L</span>");
		add_string(pan_slider_id, "0\\");
		add_string(pan_slider_id, "1\\<span size='x-small'>R</span>");
		add_string(vol_slider_id, format("%f\\<span size='x-small'>%+d</span>", db2slider(DB_MAX), (int)DB_MAX));
		add_string(vol_slider_id, format("%f\\", db2slider(6), 6));
		add_string(vol_slider_id, format("%f\\<span size='x-small'>%d</span>", db2slider(0), 0));
		add_string(vol_slider_id, format("%f\\", db2slider(-6), -6));
		add_string(vol_slider_id, format("%f\\<span size='x-small'>%d</span>", db2slider(-12), -12));
		add_string(vol_slider_id, format("%f\\<span size='x-small'>%d</span>", db2slider(-24), -24));
		add_string(vol_slider_id, format(u8"%f\\<span size='x-small'>-\u221e</span>", db2slider(DB_MIN))); // \u221e

		event_xp(id_name, "hui:draw", [this] (Painter* p) { on_name_draw(p); });
		event_x(id_name, "hui:left-button-down", [this] { on_name_left_click(); });
		event_x(id_name, "hui:right-button-down", [this] { on_name_right_click(); });
		event_x(id_name, "hui:left-double-click", [this] { on_name_double_click(); });
		event_xp(id_fx_header, "hui:draw", [this] (Painter* p) { on_fx_header_draw(p); });
		event_x(id_fx_header, "hui:left-button-down", [this] { on_name_left_click(); });
		event_x(id_fx_header, "hui:right-button-down", [this] { on_name_right_click(); });
		event_x(id_fx_header, "hui:left-double-click", [this] { on_name_double_click(); });
		event(vol_slider_id, [this] { on_volume(); });
		event(pan_slider_id, [this] { on_panning(); });
		event(mute_id, [this] { on_mute(); });
		event(solo_id, [this] { on_solo(); });
		event_xp("peaks", "hui:draw", [this] (Painter* p) { on_peak_draw(p); });
		event(fx_id, [this] { on_show_fx(is_checked("")); });

		vtrack = t;
		vtrack->subscribe(this, [this] { update(); }, vtrack->MESSAGE_CHANGE);
		vtrack->subscribe(this, [this] { on_vtrack_delete(); }, vtrack->MESSAGE_DELETE);
		vtrack->view->subscribe(this, [this] {
			redraw(id_name);
			redraw(id_fx_header);
		}, AudioView::MESSAGE_SELECTION_CHANGE);
		fx_editor = new FxListEditor(track(), this, "fx", false);
		update();
	}
	~TrackMixer() {
		if (vtrack)
			vtrack->view->unsubscribe(this);
		fx_editor->select_module(nullptr);
		clear_track();
	}

	void set_shrunk(bool _shrink) {
		shrunk = _shrink;
		hide_control(vol_slider_id, shrunk);
		hide_control(pan_slider_id, shrunk);
		hide_control(mute_id, shrunk);
		hide_control(solo_id, shrunk);
		hide_control(fx_id, shrunk);
		set_options("grid-volume", format("width=%d", shrunk ? TRACK_MIXER_WIDTH_SHRUNK : TRACK_MIXER_WIDTH));
	}

	void on_name_draw(Painter *p) {
		auto t = track();
		if (!t)
			return;

		auto view = console->view;
		bool is_playable = view->get_playable_tracks().contains(track());

		p->set_color(vtrack->header->color_bg(false));
		p->set_roundness(theme.CORNER_RADIUS);
		p->draw_rect(rect(0, p->width, -theme.CORNER_RADIUS, p->height));
		p->set_roundness(0);

		auto group_colors = track_group_colors(track());
		foreachi (int g, group_colors, i) {
			p->set_color(theme.pitch_soft1[g]);
			float y = 5*(group_colors.num - i - 1);
			p->draw_rect(rect(0, p->width, y, y+5));
		}

		if (!shrunk) {
			string tt = vtrack->header->nice_title();
			p->set_color(vtrack->header->color_text());
			if (is_playable) {
				p->set_font("", theme.FONT_SIZE, true, false);
			} else {
				p->set_font("", theme.FONT_SIZE, false, true);
				//set_string(id_name, "<s>" + nice_title() + "</s>");
				//float w = p->get_str_width(tt);
				//p->draw_str((p->width - w) / 2, 8, tt);
			}
			draw_str_constrained(p, {p->width/2.0f, 8}, p->width, tt, TextAlign::CENTER);
		}

		if (vtrack->track->type == SignalType::GROUP) {
			bool any_shrunk = false;
			for (auto gm: track_group_members(vtrack->track, false))
				if (console->mixer[gm->get_index()]->shrunk)
					any_shrunk = true;
			shrink_button_x = p->width - 10;
			p->draw_str(vec2(shrink_button_x, 8), any_shrunk ? ">" : "<");
		}
	}
	void on_fx_header_draw(Painter *p) {
		auto t = track();
		if (!t)
			return;

		//auto view = console->view;
		//bool is_playable = view->get_playable_tracks().contains(track());

		p->set_color(vtrack->header->color_bg(false));
		//p->draw_polygon({{0,0}, {p->width,0}, {0,p->height}});
		p->draw_rect(p->area());
	}

	void set_current() {
		auto view = console->view;
		HoverData h;
		h.vlayer = vtrack->first_layer();
		view->set_current(h);
	}

	void on_name_left_click() {
		auto view = console->view;

		if (view->selecting_xor()) {
			view->toggle_select_track_with_content_in_cursor(vtrack);
			return;
		}

		set_current();

		if (vtrack->track->type == SignalType::GROUP and (hui::get_event()->m.x > shrink_button_x)) {
			bool any_shrunk = false;
			for (auto gm: track_group_members(vtrack->track, false))
				if (console->mixer[gm->get_index()]->shrunk)
					any_shrunk = true;
			for (auto gm: track_group_members(vtrack->track, false))
				console->mixer[gm->get_index()]->set_shrunk(!any_shrunk);
			redraw(id_name);
		}

		if (view->sel.has(vtrack->track)) {
			view->set_selection(view->sel.restrict_to_track(vtrack->track));
		} else {
			view->exclusively_select_track(vtrack);
			view->select_under_cursor();
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
				track()->song->change_track_volumes(track(), vtrack->view->sel.tracks(), slider2vol(get_float("")));
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
		expand("revealer-fx", show);
		check("show-fx", show);
		fx_editor->select_module(nullptr);
	}

	owned<FxListEditor> fx_editor;
	void on_peak_draw(Painter* p) {
		int w = p->width;
		int h = p->height;
		p->set_color(theme.background);
		p->draw_rect(rect(0, w, 0, h));
		p->set_color(theme.text);
		float peak[2];
		console->view->renderer->get_peak(track(), peak);
		peak[0] = sqrt(peak[0]);
		peak[1] = sqrt(peak[1]);
		p->draw_rect(rect(0, w/2,  h * (1 - peak[0]), h));
		p->draw_rect(rect(w/2, w, h * (1 - peak[1]), h));
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
	static constexpr float DB_MAX = 12;
	static constexpr float TAN_SCALE = 13.0f;

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

	AudioViewTrack *vtrack = nullptr;
	string id_name;
	string id_fx_header;
	string vol_slider_id;
	string pan_slider_id;
	string mute_id, solo_id, fx_id;
	string id_separator;
	bool editing = false;
	bool shrunk = false;
	MixingConsole *console;
};





MixingConsole::MixingConsole(Session *session, BottomBar *bar) :
	BottomBar::Console(_("Mixer"), "mixing-console", session, bar)
{
	device_manager = session->device_manager;
	id_inner = "inner-grid";

	set_spacing(2);
	from_resource("mixing-console");

	peak_meter = new PeakMeterDisplay(this, "output-peaks", view->peak_meter.get(), PeakMeterDisplay::Mode::PEAKS);
	spectrum_meter = new PeakMeterDisplay(this, "output-spectrum", view->peak_meter.get(), PeakMeterDisplay::Mode::SPECTRUM);
	//set_float("output-volume", device_manager->get_output_volume());
	set_float("output-volume", view->output_stream->get_volume());
	
	peak_meter->enable(false);
	spectrum_meter->enable(false);

	event("output-volume", [this] { on_output_volume(); });

	view->subscribe(this, [this] { on_tracks_change(); }, session->view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, [this] { update_all(); }, session->view->MESSAGE_SOLO_CHANGE);
	song->subscribe(this, [this] { update_all(); }, song->MESSAGE_FINISHED_LOADING);

	//device_manager->subscribe(this, [this]{ on_update_device_manager(); });
	view->output_stream->subscribe(this, [this] {
		set_float("output-volume", view->output_stream->get_volume());

	}, view->output_stream->MESSAGE_ANY);
	load_data();


	peak_runner_id = -1;
	view->signal_chain->subscribe(this, [this] {
		on_chain_state_change();
	}, SignalChain::MESSAGE_STATE_CHANGE);
}

MixingConsole::~MixingConsole() {
	view->signal_chain->unsubscribe(this);
	if (peak_runner_id >= 0)
		hui::cancel_runner(peak_runner_id);
	//song->unsubscribe(this);
	view->unsubscribe(this);
	view->output_stream->unsubscribe(this);
	//device_manager->unsubscribe(this);
}

void MixingConsole::on_chain_state_change() {
	if (peak_runner_id and !view->is_playback_active()) {
		hui::cancel_runner(peak_runner_id);
		peak_runner_id = -1;
		// clear
		view->renderer->clear_peaks();
		for (auto m: weak(mixer))
			m->redraw("peaks");
	} else if (peak_runner_id == -1 and view->is_playback_active()) {
		peak_runner_id = hui::run_repeated(0.1f, [this] {
			for (auto *m: weak(mixer))
				m->redraw("peaks");
		});
	}
}

void MixingConsole::on_output_volume() {
	view->output_stream->set_volume(get_float(""));
	//device_manager->set_output_volume(get_float(""));
}

void MixingConsole::show_fx(Track *t) {
	for (auto m: weak(mixer))
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
	for (int i=n_ok; i<mixer.num; i++) {
		remove_control("separator-" + i2s(i));
		unembed(mixer[i].get());
	}
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
	for (auto m: weak(mixer))
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
