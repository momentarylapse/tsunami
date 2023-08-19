/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "graph/AudioViewLayer.h"
#include "graph/AudioViewTrack.h"
#include "graph/Background.h"
#include "graph/BottomBarExpandButton.h"
#include "graph/Cursor.h"
#include "graph/TimeScale.h"
#include "graph/LogNotifier.h"
#include "../MouseDelayPlanner.h"
#include "../helper/graph/SceneGraph.h"
#include "../helper/graph/ScrollBar.h"
#include "../helper/Dial.h"
#include "../helper/Drawing.h"
#include "../helper/PeakThread.h"
#include "../helper/CpuDisplay.h"
#include "../helper/PeakMeterDisplay.h"
#include "../helper/PeakDatabase.h"
#include "../mode/ViewModeDefault.h"
#include "../mode/ViewModeEdit.h"
#include "../mode/ViewModeEditDummy.h"
#include "../mode/ViewModeEditAudio.h"
#include "../mode/ViewModeEditBars.h"
#include "../mode/ViewModeEditMidi.h"
#include "../mode/ViewModeCurve.h"
#include "../mode/ViewModeCapture.h"
#include "../mode/ViewModeScaleMarker.h"
#include "../painter/BufferPainter.h"
#include "../painter/GridPainter.h"
#include "../sidebar/SideBar.h"
#include "../bottombar/BottomBar.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/rhythm/BarCollection.h"
#include "../../data/rhythm/Beat.h"
#include "../../data/SampleRef.h"
#include "../../device/stream/AudioOutput.h"
#include "../../module/audio/SongRenderer.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../module/audio/PeakMeter.h"
#include "../../module/SignalChain.h"
#include "../../plugins/TsunamiPlugin.h"
#include "../../lib/math/math.h"
#include "../../lib/threads/Thread.h"
#include "../../lib/hui/hui.h"
#include "../../lib/threads/Mutex.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../stuff/BackupManager.h"
#include "../../Session.h"
#include "../../Playback.h"
#include "../../EditModes.h"
#include "../../Tsunami.h"


const int AudioView::SNAPPING_DIST = 8;


AudioView::AudioView(Session *_session, const string &_id) :
		in_solo_changed{this, [this] {
			update_playback_layers();
			out_solo_changed.notify();
		}},
		in_redraw(this, [this] {
			force_redraw();
		})
{
	id = _id;
	session = _session;
	win = session->win.get();
	song = session->song.get();
	_optimize_view_requested = false;

	perf_channel = PerformanceMonitor::create_channel("view", this);

	color_schemes.add(ColorSchemeBright());
	color_schemes.add(ColorSchemeDark());
	color_schemes.add(ColorSchemeSystem(win, id));

	set_color_scheme(hui::config.get_str("View.ColorScheme", "system"));

	midi_view_mode = (MidiMode)hui::config.get_int("View.MidiMode", (int)MidiMode::CLASSICAL);

	playback_range_locked = false;

	selection_mode = SelectionMode::NONE;
	selection_snap_mode = SelectionSnapMode::NONE;
	hide_selection = false;

	// modes
	mode = nullptr;
	mode_default = new ViewModeDefault(this);
	mode_edit_audio = new ViewModeEditAudio(this);
	mode_edit_midi = new ViewModeEditMidi(this);
	mode_edit_bars = new ViewModeEditBars(this);
	mode_edit_dummy = new ViewModeEditDummy(this);
	mode_edit = new ViewModeEdit(this);
	mode_scale_marker = new ViewModeScaleMarker(this);
	mode_curve = new ViewModeCurve(this);
	mode_capture = new ViewModeCapture(this);
	all_modes = {mode_default, mode_edit_audio, mode_edit_midi, mode_edit_bars, mode_edit_dummy, mode_edit, mode_scale_marker, mode_curve, mode_capture};
	set_mode(mode_default);

	scene_graph = new scenegraph::SceneGraph();
	scene_graph->set_perf_name("view");
	/*scene_graph->set_callback_set_current([this] {
		if (!selecting_or())
			set_current(scene_graph->cur_selection);
	});*/
	scene_graph->out_redraw >> in_redraw;
	PerformanceMonitor::set_parent(scene_graph->perf_channel, perf_channel);
	area = rect(0, 1024, 0, 768);
	enabled = false;
	time_scale = new TimeScale(this);

	auto *vbox = new scenegraph::VBox();
	auto *hbox = new scenegraph::HBox();

	background = new Background(this);
	cursor_start = new Cursor(this, false);
	cursor_end = new Cursor(this, true);
	selection_marker = new SelectionMarker(this);
	scroll_bar_y = new ScrollBar(); // pixels
	scroll_bar_y->auto_hide = true;
	scroll_bar_y->out_offset >> create_data_sink<float>([this] (float offset) {
		thm.update_immediately(this, song, song_area());
	});
	scroll_bar_time = new ScrollBarHorizontal(); // samples
	scroll_bar_time->out_offset >> create_data_sink<float>([this] (float offset) {
		cam.dirty_jump(offset);
	});
	scroll_bar_time->constrained = false;

	metronome_overlay_vlayer = new AudioViewLayer(this, nullptr);
	metronome_overlay_vlayer->align.dz = 80;
	dummy_vtrack = new AudioViewTrack(this, nullptr);
	dummy_vlayer = new AudioViewLayer(this, nullptr);


	scene_graph->add_child(vbox);

	vbox->add_child(time_scale);
	vbox->add_child(hbox);
	vbox->add_child(scroll_bar_time);

	hbox->add_child(background);
	hbox->add_child(scroll_bar_y);

	scene_graph->add_child(cursor_start);
	scene_graph->add_child(cursor_end);
	scene_graph->add_child(selection_marker);
	scene_graph->add_child(metronome_overlay_vlayer);


	cpu_display = new CpuDisplay(session);
	scene_graph->add_child(cpu_display);

	peak_database = new PeakDatabase(song);

	buffer_painter = new BufferPainter(this);
	grid_painter = new GridPainter(song, &cam, &sel, &hover(), theme);

	preview_sleep_time = hui::config.get_int("PreviewSleepTime", 10);
	ScrollSpeed = 20;
	ScrollSpeedFast = 200;
	ZoomSpeed = hui::config.get_float("View.ZoomSpeed", 0.1f);
	set_mouse_wheel_speed(hui::config.get_float("View.MouseWheelSpeed", 1.0f));
	set_antialiasing(hui::config.get_bool("View.Antialiasing", true));
	set_high_details(hui::config.get_bool("View.HighDetails", true));
	hui::config.set_float("View.ZoomSpeed", ZoomSpeed);

	images.speaker = Image::load(tsunami->directory_static | "icons/volume.png");
	images.solo = Image::load(tsunami->directory_static | "icons/solo.png");
	images.config = Image::load(tsunami->directory_static | "icons/wrench.png");
	images.x = Image::load(tsunami->directory_static | "icons/x.png");
	images.track_audio = Image::load(tsunami->directory_static | "icons/track-audio.png");
	images.track_time = Image::load(tsunami->directory_static | "icons/track-time.png");
	images.track_midi = Image::load(tsunami->directory_static | "icons/track-midi.png");
	images.track_group = Image::load(tsunami->directory_static | "icons/track-group.png");

	draw_runner_id = -1;


	session->playback->out_tick >> create_sink([this] {
		on_stream_tick();
	});
	session->playback->out_state_changed >> create_sink([this] {
		on_stream_state_change();
	});
	cam.out_changed >> create_sink([this] {
		cam_changed();
	});


	// TODO move "OnScreenDisplay"?
	peak_meter_display = new PeakMeterDisplay(session->playback->peak_meter.get(), PeakMeterDisplay::Mode::BOTH);
	peak_meter_display->align.dx = 90;
	peak_meter_display->align.dy = -20;
	peak_meter_display->align.horizontal = scenegraph::Node::AlignData::Mode::LEFT;
	peak_meter_display->align.vertical = scenegraph::Node::AlignData::Mode::BOTTOM;
	peak_meter_display->align.dz = 100;
	peak_meter_display->hidden = true;
	scene_graph->add_child(peak_meter_display);

	output_volume_dial = new Dial(_("output"), 0, 100);
	output_volume_dial->align.horizontal = scenegraph::Node::AlignData::Mode::LEFT;
	output_volume_dial->align.vertical = scenegraph::Node::AlignData::Mode::BOTTOM;
	output_volume_dial->align.dx = 230;
	output_volume_dial->align.dy = 0;
	output_volume_dial->align.dz = 100;
	//output_volume_dial->reference_value = 50;
	output_volume_dial->unit = "%";
	output_volume_dial->set_value(session->playback->output_stream->get_volume() * 100);
	output_volume_dial->out_value >> create_data_sink<float>([this] (float v) {
		session->playback->output_stream->set_volume(v / 100.0f);
	});
	session->playback->output_stream->out_changed >> create_sink([this] {
		output_volume_dial->set_value(session->playback->output_stream->get_volume() * 100);
	});
	output_volume_dial->hidden = true;
	scene_graph->add_child(output_volume_dial);

	bottom_bar_expand_button = new BottomBarExpandButton(this);
	scene_graph->add_child(bottom_bar_expand_button);

	log_notifier = new LogNotifier(this);
	scene_graph->add_child(log_notifier);

	onscreen_display = nullptr; //new scenegraph::NodeFree();

	m = {0,0};


	hui::run_later(0.5f, [this] {
		if (BackupManager::should_notify_found_backups()) {
			session->q("Old recording backup found\nto load or delete, open the session manager in the bottom bar", {"show-session-console:yes"});
			BackupManager::notify_found_backups_done();
		}
	});


	// events
	win->event_xp(id, "hui:draw", [this](Painter *p) { on_draw(p); });
	win->event_x(id, "hui:mouse-move", [this]{ on_mouse_move(); });
	win->event_x(id, "hui:left-button-down", [this]{ on_left_button_down(); });
	win->event_x(id, "hui:left-double-click", [this]{ on_left_double_click(); });
	win->event_x(id, "hui:left-button-up", [this]{ on_left_button_up(); });
	win->event_x(id, "hui:middle-button-down", [this]{ on_middle_button_down(); });
	win->event_x(id, "hui:middle-button-up", [this]{ on_middle_button_up(); });
	win->event_x(id, "hui:right-button-down", [this]{ on_right_button_down(); });
	win->event_x(id, "hui:right-button-up", [this]{ on_right_button_up(); });
	win->event_x(id, "hui:mouse-leave", [this]{ on_mouse_leave(); });
	win->event_x(id, "hui:key-down", [this]{ on_key_down(); });
	win->event_x(id, "hui:key-up", [this]{ on_key_up(); });
	win->event_x(id, "hui:mouse-wheel", [this]{ on_mouse_wheel(); });
	win->event_x(id, "hui:gesture-zoom", [this]{ mode->on_command("hui:gesture-zoom"); });
	win->event_x(id, "hui:gesture-zoom-begin", [this]{ mode->on_command("hui:gesture-zoom-begin"); });
	win->event_x(id, "hui:gesture-zoom-end", [this]{ mode->on_command("hui:gesture-zoom-end"); });

	win->activate(id);


	menu_song = hui::create_resource_menu("popup-menu-song", win);
	menu_track = hui::create_resource_menu("popup-menu-track", win);
	menu_layer = hui::create_resource_menu("popup-menu-layer", win);
	menu_playback_range = hui::create_resource_menu("popup-menu-playback-range", win);
	menu_sample = hui::create_resource_menu("popup-menu-sample", win);
	menu_marker = hui::create_resource_menu("popup-menu-marker", win);
	menu_bar = hui::create_resource_menu("popup-menu-bar", win);
	menu_bar_gap = hui::create_resource_menu("popup-menu-bar-gap", win);
	menu_buffer = hui::create_resource_menu("popup-menu-buffer", win);

	song->out_track_list_changed >> create_sink([this]{ on_song_tracks_change(); });
	song->out_layer_list_changed >> create_sink([this]{ on_song_tracks_change(); });
	song->out_channels_changed >> create_sink([this]{ on_song_tracks_change(); });
	song->out_new >> create_sink([this] {
		on_song_new();
	});
	//song->subscribe(this, [this]{ on_song_finished_loading(); }, song->MESSAGE_FINISHED_LOADING);
	auto apply_bar_scale = [this](int i) {
		auto b = song->bars[song->x_message_data.i[0]].get();
		if (i <= b->offset)
			return i;
		if (i >= b->offset + song->x_message_data.i[1])
			return i - song->x_message_data.i[1] + song->x_message_data.i[2];
		return b->offset + (int)((float)(i - b->offset) * (float)song->x_message_data.i[2] / (float)song->x_message_data.i[1]);
	};
	song->out_scale_bars >> create_sink([this, apply_bar_scale] {
		sel.range_raw.set_start(apply_bar_scale(sel.range_raw.start()));
		sel.range_raw.set_end(apply_bar_scale(sel.range_raw.end()));
		update_selection();
	});
	song->out_finished_loading >> create_sink([this] {
		on_song_new();
		on_song_finished_loading();
		enable(true);
	});
	song->out_start_loading >> create_sink([this] {
		enable(false);
	});
	song->out_before_change >> create_sink([this] {
		peak_database->stop_update();
	});
	song->out_after_change >> create_sink([this] {
		on_song_change();
	});
	/*song->subscribe(this, [this] {
		force_redraw();
		update_menu();
	}, song->MESSAGE_ANY);*/
	enable(true);

	//ForceRedraw();
	update_menu();
}

AudioView::~AudioView() {
	enabled = false;
	if (draw_runner_id >= 0)
		hui::cancel_runner(draw_runner_id);

	session->playback->unsubscribe(this);
	song->unsubscribe(this);

	PerformanceMonitor::delete_channel(perf_channel);
}

void AudioView::set_antialiasing(bool set) {
	antialiasing = set;
	hui::config.set_bool("View.Antialiasing", antialiasing);
	force_redraw();
	out_settings_changed.notify();
}

void AudioView::set_high_details(bool set) {
	high_details = set;
	detail_steps = high_details ? 1 : 3;
	hui::config.set_bool("View.HighDetails", high_details);
	force_redraw();
	out_settings_changed.notify();
}

void AudioView::set_mouse_wheel_speed(float speed) {
	mouse_wheel_speed = speed;
	hui::config.set_float("View.MouseWheelSpeed", mouse_wheel_speed);
	out_settings_changed.notify();
}

void AudioView::set_color_scheme(const string &name) {
	hui::config.set_str("View.ColorScheme", name);
	theme = color_schemes[0];
	for (auto &b: color_schemes)
		if (b.name == name)
			theme = b;

	force_redraw();
}

void AudioView::set_mode(ViewMode *m) {
	if (m == mode)
		return;
	if (mode) {
		session->debug("view", "end mode " + mode->mode_name);
		mode->on_end();
	}
	mode = m;
	if (mode) {
		session->debug("view", "start mode " + mode->mode_name);
		mode->on_start();
	}
	thm.set_dirty();
	force_redraw();
}

void AudioView::set_mouse() {
	m = hui::get_event()->m;
}

int AudioView::mouse_over_sample(SampleRef *s) {
	if ((m.x >= s->area.x1) and (m.x < s->area.x2)) {
		int offset = cam.screen2sample(m.x) - s->pos;
		if ((m.y >= s->area.y1) and (m.y < s->area.y1 + theme.SAMPLE_FRAME_HEIGHT))
			return offset;
		if ((m.y >= s->area.y2 - theme.SAMPLE_FRAME_HEIGHT) and (m.y < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::selection_update_pos(HoverData &s) {
	s.pos = cam.screen2sample(m.x);
}

void AudioView::update_selection() {
	//sel.range = sel.range.canonical();

	if (!playback_range_locked) {
		playback_wish_range = sel.range();
	}

	session->playback->update_range(get_playback_selection(false));
	force_redraw();

	out_selection_changed.notify();
}


void AudioView::set_selection(const SongSelection &s) {
	sel = s;
	update_selection();
}


bool AudioView::mouse_over_time(int pos) {
	int ssx = cam.sample2screen(pos);
	return ((m.x >= ssx - 5) and (m.x <= ssx + 5));
}


Range AudioView::get_playback_selection(bool for_recording) {
	if (playback_wish_range.length > 0)
		return playback_wish_range;
	if (sel.range().is_empty()) {
		if (for_recording)
			return Range(sel.range().start(), 0x70000000);
		int num = song->range().end() - sel.range().start();
		if (num <= 0)
			num = song->sample_rate; // 1 second
		return Range(sel.range().start(), num);
	}
	return sel.range();
}

void AudioView::set_selection_snap_mode(SelectionSnapMode mode) {
	selection_snap_mode = mode;
	out_settings_changed.notify();
}

void _update_find_min(int &new_pos, bool &found, float &dmin, int pos, int trial_pos) {
	float dist = fabs((float)trial_pos - (float)pos);
	if (dist < dmin) {
		//msg_write(format("barrier:  %d  ->  %d", pos, b));
		new_pos = trial_pos;
		found = true;
		dmin = dist;
	}

}

void AudioView::snap_to_grid(int &pos) {
	bool found = false;
	int new_pos = pos;

	if (selection_snap_mode == SelectionSnapMode::NONE) {
		float dmin = cam.dscreen2sample(SNAPPING_DIST);

		int sub_beats = 0;
		if (mode == mode_edit and mode_edit->mode == mode_edit_midi)
			sub_beats = mode_edit_midi->sub_beat_partition;

		// time bar...
		auto beats = song->bars.get_beats(cam.range(), true, sub_beats>0, sub_beats);
		for (Beat &b: beats)
			_update_find_min(new_pos, found, dmin, pos, b.range.offset);

	} else if (selection_snap_mode == SelectionSnapMode::BAR) {
		float dmin = cam.dscreen2sample(SNAPPING_DIST) * 1000;
		auto bars = song->bars.get_bars(cam.range());
		for (Bar *b: bars) {
			_update_find_min(new_pos, found, dmin, pos, b->range().start());
			_update_find_min(new_pos, found, dmin, pos, b->range().end());
		}
	} else if (selection_snap_mode == SelectionSnapMode::PART) {
		float dmin = cam.dscreen2sample(SNAPPING_DIST) * 1000;
		auto parts = song->get_parts();
		for (auto *p: parts) {
			_update_find_min(new_pos, found, dmin, pos, p->range.start());
			_update_find_min(new_pos, found, dmin, pos, p->range.end());
		}
	}

	if (found)
		pos = new_pos;
}

void AudioView::on_mouse_move() {
	set_mouse();

	scene_graph->on_mouse_move(m);

	mode->on_mouse_move();

	force_redraw();
}

void AudioView::on_left_button_down() {
	set_mouse();
	scene_graph->on_left_button_down(m);

	force_redraw();
	update_menu();
}

// extend to beats
void align_to_beats(Song *s, Range &r, int beat_partition) {
	auto beats = s->bars.get_beats(Range::ALL, true, beat_partition > 0, beat_partition);//audio->getRange());
	for (Beat &b: beats) {
		/*for (int i=0; i<beat_partition; i++) {
			Range sr = b.sub(i, beat_partition);
			if (sr.overlaps(sr))
				r = r or sr;
		}*/
		if (b.range.is_inside(r.start())) {
			r.set_start(b.range.start());
		}
		if (b.range.is_inside(r.end())) {
			r.set_end(b.range.end());
			break;
		}
	}
}


void AudioView::on_left_button_up() {
	scene_graph->on_left_button_up(m);

	force_redraw();
	update_menu();
}



void AudioView::on_middle_button_down() {
}



void AudioView::on_middle_button_up() {
}



void AudioView::on_right_button_down() {
	scene_graph->on_right_button_down(m);
}

void AudioView::on_right_button_up() {
	mode->on_right_button_up();
}

void AudioView::on_mouse_leave() {
	// TODO check if necessary
	if (!hui::get_event()->lbut and !hui::get_event()->rbut) {
		hover().clear();
		force_redraw();
	}
}



void AudioView::on_left_double_click() {
	scene_graph->on_left_double_click(m);
	mode->on_left_double_click();
	force_redraw();
	update_menu();
}


AudioViewLayer *AudioView::next_layer(AudioViewLayer *ll) {
	bool found = false;
	for (auto *l: vlayers) {
		if (found and !l->hidden)
			return l;
		if (l == ll)
			found = true;
	}
	return ll;
}
AudioViewLayer *AudioView::prev_layer(AudioViewLayer *ll) {
	AudioViewLayer *prev = nullptr;
	for (auto *l: vlayers) {
		if (l == ll and prev)
			return prev;
		if (!l->hidden)
			prev = l;
	}
	return ll;
}

void AudioView::move_to_layer(int delta) {
	auto *vlayer = cur_vlayer();
	if (delta > 0)
		vlayer = next_layer(vlayer);
	else
		vlayer = prev_layer(vlayer);

	set_current(vlayer->get_hover_data({0,0}));
	exclusively_select_layer(vlayer);
	select_under_cursor();
}


void AudioView::zoom_y(float zoom) {
	cam.scale_y = clamp(zoom, 0.5f, 2.0f);
	thm.set_dirty();
	session->status(format(_("vertical zoom %.0f%%"), cam.scale_y * 100.0f));
	force_redraw();
}

void AudioView::toggle_track_mute() {
	bool any_unmuted = false;
	for (auto *t: weak(song->tracks))
		if (sel.has(t) and !t->muted)
			any_unmuted = true;

	song->begin_action_group("track mute toggle");
	for (auto *t: weak(song->tracks))
		if (sel.has(t))
			t->set_muted(any_unmuted);
	song->end_action_group();
}

void AudioView::toggle_track_solo() {
	if (cur_vtrack()->solo) {
		for (auto *vt: vtracks)
			vt->set_solo(false);
	} else {
		for (auto *vt: vtracks)
			vt->set_solo(sel.has(vt->track));
	}
}

void AudioView::toggle_layer_mute() {
	bool any_unmuted = false;
	for (auto *l: vlayers)
		if (sel.has(l->layer) and !l->layer->muted)
			any_unmuted = true;

	song->begin_action_group("layer mute toggle");
	for (auto *l: vlayers)
		if (sel.has(l->layer))
			l->layer->set_muted(any_unmuted);
	song->end_action_group();
}

void AudioView::toggle_layer_solo() {
	bool any_solo = false;
	for (auto *l: vlayers)
		if (sel.has(l->layer) and l->solo)
			any_solo = true;
	for (auto *t: vtracks)
		if (sel.has(t->track)) {
			if (any_solo) {
				for (auto *l: vlayers)
					if (l->track() == t->track)
						l->set_solo(false);
			} else {
				for (auto *l: vlayers)
					if (l->track() == t->track)
						l->set_solo(sel.has(l->layer));
			}
		}
}

void AudioView::toggle_track_exploded() {
	bool any_imploded = false;
	for (auto *t: vtracks)
		if (sel.has(t->track) and t->imploded and t->track->layers.num > 0)
			any_imploded = true;

	for (auto *t: vtracks)
		if (sel.has(t->track) and t->track->layers.num > 0) {
			if (any_imploded)
				explode_track(t->track);
			else
				implode_track(t->track);
		}
}

void AudioView::on_command(const string &id) {
	if (id == "track-muted")
		toggle_track_mute();
	if (id == "track-solo")
		toggle_track_solo();
	if (id == "layer-muted")
		toggle_layer_mute();
	if (id == "layer-solo")
		toggle_layer_solo();

	
	if (id == "track-explode")
		toggle_track_exploded();

	float dt = 0.05f;
	if (id == "cam-move-right")
		cam.move(ScrollSpeedFast * dt / cam.pixels_per_sample);
	if (id == "cam-move-left")
		cam.move(- ScrollSpeedFast * dt / cam.pixels_per_sample);

	// vertical zoom
	if (id == "vertical-zoom-in")
		zoom_y(cam.scale_y * 1.2f);
	if (id == "vertical-zoom-out")
		zoom_y(cam.scale_y / 1.2f);
	if (id == "vertical-zoom-default")
		zoom_y(1.0f);

	mode->on_command(id);
}



void AudioView::on_key_down() {
	int k = hui::get_event()->key_code;
	if (k == hui::KEY_ESCAPE) {
		if (mdp()->acting()) {
			mdp()->cancel();
		} else {
			win->side_bar->test_allow_close([this] {
				session->set_mode(EditMode::Default);
			}, [] {});
		}
	}
	if ((k == hui::KEY_LSHIFT) or (k == hui::KEY_RSHIFT))
		force_redraw();

	mode->on_key_down(k);
}

void AudioView::on_key_up() {
	int k = hui::get_event()->key_code;
	if ((k == hui::KEY_LSHIFT) or (k == hui::KEY_RSHIFT))
		force_redraw();

	mode->on_key_up(k);
}

void AudioView::on_mouse_wheel() {
	mode->on_mouse_wheel();
}


void AudioView::force_redraw() {
	win->redraw(id);
	//win->redraw_rect(id, rect(200, 300, 0, area.y2));
}

void AudioView::force_redraw_part(const rect &r) {
	win->redraw_rect(id, r);
}

void AudioView::unselect_all_samples() {
	sel._samples.clear();
}

void AudioView::update_buffer_zoom() {
	prefered_buffer_layer = -1;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (cam.pixels_per_sample < 0.2f)
		for (int i=24-1;i>=0;i--) {
			double _f = (double)(1 << (i + PeakData::PEAK_OFFSET_EXP));
			if (_f > 2.0 / cam.pixels_per_sample) {
				prefered_buffer_layer = i;
				buffer_zoom_factor = _f;
			}
		}
}

AudioViewLayer* suggest_good_cur_layer(AudioView *v) {
	for (auto *l: v->vlayers)
		if (v->get_track(l->layer->track) == v->_prev_track) {
			v->session->debug("view", "  -> set by track");
			return l;
		}
	if (v->vlayers.num > 0) {
		v->session->debug("view", "  -> set first layer");
		return v->vlayers[0];
	} else {
		v->session->debug("view", "....no vlayers");
		return nullptr;
	}
}

bool cur_layer_acceptable(AudioView *view, AudioViewLayer *l) {
	if (!l)
		return false;
	return view->vlayers.find(l) >= 0;
}

void AudioView::check_consistency() {

	// cur layer?
	if (!cur_layer_acceptable(this, cur_vlayer())) {
		__set_cur_layer(suggest_good_cur_layer(this));
	}

	if (sel.layers().num == 0) {
		sel.set(cur_layer(), true);
		update_selection();
	}

	// illegal midi mode?
	for (auto *t: vtracks)
		if ((t->midi_mode() == MidiMode::TAB) and (t->track->instrument.string_pitch.num == 0))
			t->set_midi_mode(MidiMode::CLASSICAL);
}

void AudioView::implode_track(Track *t) {
	for (auto *l: vlayers)
		if (l->layer->track == t) {
			l->represents_imploded = l->layer->is_main();
			l->hidden = !l->layer->is_main();
		}
	get_track(t)->imploded = true;
	thm.set_dirty();
	force_redraw();
}

void AudioView::explode_track(Track *t) {
	for (auto *l: vlayers)
		if (l->layer->track == t) {
			l->represents_imploded = false;
			l->hidden = false;
		}
	get_track(t)->imploded = false;
	thm.set_dirty();
	force_redraw();
}

void AudioView::on_song_new() {
	__set_cur_layer(nullptr);
	update_tracks();
	sel.range_raw = Range::to(0, 0);
	sel.clear();
	for (Track *t: weak(song->tracks))
		for (TrackLayer *l: weak(t->layers))
			sel.add(l);
	check_consistency();
	request_optimize_view();
}

void AudioView::on_song_finished_loading() {
	session->debug("view", "------finish loading");
	if ((vlayers.num >= 12) and (vtracks.num >= 2)) {
		for (auto *t: vtracks)
			if (t->track->layers.num > 1)
				implode_track(t->track);
	}
	request_optimize_view();
}

void AudioView::on_song_tracks_change() {
	update_tracks();
	force_redraw();
	update_menu();
}

void AudioView::on_song_change() {
	session->debug("view", "song.after-change");
	if (song->history_enabled())
		session->debug("view", "+++");
}

void AudioView::on_stream_tick() {
	cam.make_sample_visible(session->playback->get_pos(), session->sample_rate() * 2);
	force_redraw();
}


void AudioView::update_onscreen_displays() {
	bottom_bar_expand_button->hidden = false;
	peak_meter_display->hidden = true;
	output_volume_dial->hidden = true;

	if (session->win->bottom_bar)
		if (!session->win->bottom_bar->is_active(BottomBar::MIXING_CONSOLE)) {
			peak_meter_display->hidden = !session->playback->is_active();
			output_volume_dial->hidden = !session->playback->is_active();
		}
	force_redraw();
}

void AudioView::on_stream_state_change() {
	update_onscreen_displays();
}

void AudioView::on_update_sidebar() {
	check_consistency();
	force_redraw();
}

AudioViewTrack *AudioView::get_track(Track *track) {
	for (auto t: vtracks) {
		if (t->track == track)
			return t;
	}
	return dummy_vtrack.get();
}

AudioViewLayer *AudioView::get_layer(TrackLayer *layer) {
	for (auto l: vlayers) {
		if (l->layer == layer)
			return l;
	}

	session->e("get_layer() failed for " + p2s(layer));
	session->i(msg_get_trace());

	return dummy_vlayer.get();
}

AudioViewTrack *get_vtrack_x(AudioView *v, Track *t) {
	for (auto vt: v->vtracks)
		if (vt->track == t)
			return vt;
	return nullptr;
}

void AudioView::update_tracks() {
	int cur_layer_index = vlayers.find(cur_vlayer());

	Array<AudioViewTrack*> vtrack2;
	Array<AudioViewLayer*> vlayer2;
	vtrack2.resize(song->tracks.num);
	vlayer2.resize(song->layers().num);

	foreachi(Track *t, weak(song->tracks), ti) {
		bool found = false;

		// find existing
		foreachi(auto *v, vtracks, vi) {
			if (v)
				if (v->track == t) {
					vtrack2[ti] = v;
					vtracks[vi] = nullptr;
					found = true;
					break;
				}
		}

		// new track
		if (!found) {
			vtrack2[ti] = new AudioViewTrack(this, t);
			background->add_child(vtrack2[ti]);
			vtrack2[ti]->out_solo_changed >> in_solo_changed;
		}
	}
	// delete deleted
	auto vtrack_del = vtracks;
	vtracks = vtrack2;


	int li = 0;
	for (auto l: song->layers()) {

		bool found = false;

		// find existing
		foreachi(auto *v, vlayers, vi)
			if (v) {
				if (v->layer == l) {
					vlayer2[li] = v;
					vlayers[vi] = nullptr;
					found = true;
					break;
				}
			}

		// new layer
		if (!found) {
			vlayer2[li] = new AudioViewLayer(this, l);
			get_track(l->track)->add_child(vlayer2[li]);
			sel.add(l);
			vlayer2[li]->out_solo_changed >> in_solo_changed;
		}

		li ++;
	}

	// delete deleted
	auto vlayer_del = vlayers;
	vlayers = vlayer2;
	thm.set_dirty();

	// guess where to create new tracks
	foreachi(auto *v, vtracks, i) {
		if (v->area.height() == 0) {
			if (i > 0) {
				v->area = vtracks[i-1]->area;
				v->area.y1 = v->area.y2;
			} else if (vtracks.num > 1) {
				v->area = vtracks[i+1]->area;
				v->area.y2 = v->area.y1;
			}
		}
	}

	// guess where to create new tracks
	foreachi(auto *v, vlayers, i) {
		if (v->area.height() == 0) {
			if (i > 0) {
				v->area = vlayers[i-1]->area;
				v->area.y1 = v->area.y2;
			} else if (vtracks.num > 1) {
				v->area = vlayers[i+1]->area;
				v->area.y2 = v->area.y1;
			}
		}
	}

	hover().node = nullptr;
	_prev_selection.clear();

	if (vlayers.find(cur_vlayer()) < 0 and cur_layer_index >= 0) {
		__set_cur_layer(vlayers[max(cur_layer_index - 1, 0)]);
	}

	// TODO: detect order change
	check_consistency();
	out_vtrack_changed.notify();


	for (auto *v: vlayer_del)
		if (v)
			v->parent->delete_child(v);
	for (auto *v: vtrack_del)
		if (v)
			v->parent->delete_child(v);
}

bool need_metro_overlay(Song *song, AudioView *view) {
	Track *tt = song->time_track();
	if (!tt)
		return false;
	auto *vv = view->get_layer(tt->layers[0].get());
	if (!vv)
		return false;
	return !vv->on_screen();
}

bool AudioView::update_scene_graph() {

	bool scroll_bar_w_needed = !cam.range().covers(song->range());
	bool animating = thm.update(this, song, song_area());

	scroll_bar_time->hidden = !scroll_bar_w_needed;
	scroll_bar_time->set_view_size(cam.range().length);
	scroll_bar_time->set_content(song->range());

	for (auto *v: vlayers) {
		v->update_header();
	}

	metronome_overlay_vlayer->hidden = !need_metro_overlay(song, this);
	if (!metronome_overlay_vlayer->hidden) {
		metronome_overlay_vlayer->layer = song->time_track()->layers[0].get();
		metronome_overlay_vlayer->area = rect(song_area().x1, song_area().x2, song_area().y1, song_area().y1 + theme.TIME_SCALE_HEIGHT*2);
	}
	return animating;
}

rect AudioView::song_area() {
	if (!background)
		return rect::EMPTY;
	return background->area;
}

void AudioView::draw_cursor_hover(Painter *c, const string &msg) {
	::draw_cursor_hover(c, msg, m, song_area());
}

void AudioView::draw_time_line(Painter *c, int pos, const color &col, bool hover, bool show_time) {
	float x = cam.sample2screen(pos);
	if ((x >= song_area().x1) and (x <= song_area().x2)) {
		color cc = col;
		if (hover)
			cc = theme.selection_boundary_hover;
		c->set_color(cc);
		c->set_line_width(2.0f);
		c->draw_line({x, area.y1}, {x, area.y2});
		if (show_time)
			draw_boxed_str(c,  {x, song_area().center().y}, song->get_time_str_long(pos), cc, theme.background);
		c->set_line_width(1.0f);
	}
}

bool AudioView::selecting_or() const {
	return session->win->get_key(hui::KEY_SHIFT);
}

bool AudioView::selecting_xor() const {
	return session->win->get_key(hui::KEY_CONTROL);
}

void AudioView::draw_song(Painter *c) {
	bool animating = update_scene_graph();

	c->set_antialiasing(false);

	scene_graph->update_geometry_recursive(area);

	cam.area = song_area();
	if (_optimize_view_requested)
		perform_optimize_view();
	cam.update(0.1f);

	update_buffer_zoom();
	//scene_graph->on_draw(c);
	scene_graph->draw(c);


	// playing/capturing position
	if (session->playback->is_active())
		draw_time_line(c, session->playback->get_pos(), theme.preview_marker, false, true);

	mode->draw_post(c);
	for (auto *plugin: weak(session->plugins))
		plugin->on_draw_post(c);

	// tool tip?
	string tip;
	if (hover().node)
		tip = hover().node->get_tip();
	if (tip.num > 0)
		draw_cursor_hover(c, tip);

	tip = mode->get_tip();
	if (tip.num > 0)
		draw_boxed_str(c, {song_area().center().x, area.y2 - 50}, tip, theme.text_soft1, theme.background_track_selected, TextAlign::CENTER);

	peak_database->iterate();

	if (log_notifier->progress(0.03f))
		animating = true;

	if (cam.needs_update())
		animating = true;

	if (animating)
		draw_runner_id = hui::run_later(animating ? 0.03f : 0.2f, [this]{ force_redraw(); });
}

void AudioView::on_draw(Painter *c) {
	PerformanceMonitor::start_busy(perf_channel);

	area = c->area();
	clip = c->clip();

	c->set_font_size(theme.FONT_SIZE);
	c->set_line_width(theme.LINE_WIDTH);

	if (enabled)
		draw_song(c);

	//static int frame = 0;
	//c->draw_str(100, 100, i2s(frame++));

	PerformanceMonitor::end_busy(perf_channel);
}

void AudioView::perform_optimize_view() {
	if (area.x2 <= 0)
		area.x2 = 1024;

	Range r = song->range();

	if (r.length == 0)
		r.length = 10 * song->sample_rate;

	_optimize_view_requested = false;
	cam.show(r);
}

void AudioView::request_optimize_view() {
	_optimize_view_requested = true;
	force_redraw();
}

void AudioView::update_menu() {
	MidiMode common_midi_mode = midi_view_mode;
	for (auto *t: vtracks)
		if (t->track)
			if ((t->track->type == SignalType::MIDI) and (t->midi_mode_wanted != midi_view_mode))
				common_midi_mode = MidiMode::DONT_CARE;
	// view
	win->check("view-midi-linear", common_midi_mode == MidiMode::LINEAR);
	win->check("view-midi-tab", common_midi_mode == MidiMode::TAB);
	win->check("view-midi-classical", common_midi_mode == MidiMode::CLASSICAL);
}

void AudioView::set_midi_view_mode(MidiMode mode) {
	midi_view_mode = mode;
	hui::config.set_int("View.MidiMode", (int)midi_view_mode);
	for (auto *t: vtracks)
		t->set_midi_mode(mode);
	//forceRedraw();
	out_settings_changed.notify();
	update_menu();
}

void AudioView::zoom_in() {
	cam.zoom(exp( ZoomSpeed), m.x);
}

void AudioView::zoom_out() {
	cam.zoom(exp(-ZoomSpeed), m.x);
}

void AudioView::select_all() {
	set_selection(mode->get_selection_for_range(song->range()));
}

void AudioView::select_none() {
	// select all/none
	set_selection(SongSelection());
}

inline void test_range(const Range &r, Range &sel, bool &update) {
	if (r.is_more_inside(sel.start())) {
		sel.set_start(r.start());
		update = true;
	}
	if (r.is_more_inside(sel.end())) {
		sel.set_end(r.end());
		update = true;
	}
}

void AudioView::select_expand() {
	Range r = sel.range();
	bool update = true;
	while (update) {
		update = false;
		for (Track *t: weak(song->tracks)) {
			if (!sel.has(t))
				continue;

			if (t->type == SignalType::BEATS)
				for (Bar* b: weak(song->bars))
					test_range(b->range(), r, update);

			// midi
			for (TrackLayer *l: weak(t->layers))
				if (sel.has(l))
					for (MidiNote *n: weak(l->midi))
						test_range(n->range, r, update);

			for (TrackLayer *l: weak(t->layers)) {
				// buffers
				for (AudioBuffer &b: l->buffers)
					test_range(b.range(), r, update);

				// samples
				for (SampleRef *s: weak(l->samples))
					test_range(s->range(), r, update);
			}
		}
	}

	set_selection(mode->get_selection_for_range(r));
}



void AudioView::select_sample(SampleRef *s, bool diff) {
	if (!s)
		return;
	if (diff) {
		sel.set(s, !sel.has(s));
	} else {
		if (!sel.has(s))
			unselect_all_samples();

		// select this sub
		sel.add(s);
	}
}

void ensure_layer_on_screen(AudioView *view, AudioViewLayer *l) {
	if (l->area.y1 < view->song_area().y1)
		view->scroll_bar_y->set_view_offset(view->scroll_bar_y->view_offset - (view->song_area().y1 - l->area.y1));
	if (l->area.y2 > view->song_area().y2)
		view->scroll_bar_y->set_view_offset(view->scroll_bar_y->view_offset - (view->song_area().y2 - l->area.y2));
}

void AudioView::set_current(const HoverData &h) {
	cur_selection = h;

	if (!cur_vlayer()) {
		session->debug("view", "   ...setting cur_vlayer = nil");
		//msg_write(msg_get_trace());
		cur_selection.vlayer = _prev_selection.vlayer;
	}

	// notifications
	if (cur_sample() != _prev_selection.sample) {
		out_cur_sample_changed.notify();
	}
	if (cur_vlayer() != _prev_selection.vlayer) {
		mode->on_cur_layer_change();
		ensure_layer_on_screen(this, cur_vlayer());
		force_redraw();
		out_cur_layer_changed.notify();
	}
	if (cur_vtrack() != _prev_track) {
		out_cur_track_changed.notify();
	}

	_prev_selection = cur_selection;
	_prev_track = cur_vtrack();
}

Track *AudioView::cur_track() {
	if (!cur_vlayer())
		return nullptr;
	if (!cur_vlayer()->layer)
		return nullptr;
	return cur_vlayer()->layer->track;
}

AudioViewTrack *AudioView::cur_vtrack() {
	for (auto *t: vtracks)
		if (t->track == cur_track())
			return t;
	return nullptr;
}

TrackLayer *AudioView::cur_layer() {
	if (!cur_vlayer())
		return nullptr;
	//if (song->layers().find(cur_vlayer->layer) < 0)
	//	return nullptr;
	return cur_vlayer()->layer;
}

AudioViewLayer *AudioView::cur_vlayer() {
	return cur_selection.vlayer;
}

SampleRef *AudioView::cur_sample() {
	return cur_selection.sample;
}

// l must be from vlayer[] or null
void AudioView::__set_cur_layer(AudioViewLayer *l) {
	auto h = cur_selection;
	h.vlayer = l;
	set_current(h);
}

// unused?!?
void AudioView::__set_cur_sample(SampleRef *s) {
	auto h = cur_selection;
	h.sample = s;
	set_current(h);
}



// unused?!?
void AudioView::enable(bool _enabled) {
	if (enabled and !_enabled) {
		session->debug("view", "DISABLE");
	} else if (!enabled and _enabled) {
		session->debug("view", "ENABLE");
	}
	enabled = _enabled;
	force_redraw();
}

void AudioView::set_playback_loop(bool loop) {
	session->playback->set_loop(loop);
}
void AudioView::play() {
	session->playback->play();
}
void AudioView::stop() {
	session->playback->stop();
}
void AudioView::pause(bool pause) {
	session->playback->pause(pause);
}
void AudioView::prepare_playback(const Range &range, bool allow_loop) {
	session->playback->prepare(range, allow_loop);
}
bool AudioView::is_playback_active() const {
	return session->playback->is_active();
}
bool AudioView::is_paused() const {
	return session->playback->is_paused();
}
int AudioView::playback_pos() const {
	return session->playback->get_pos();
}
bool AudioView::looping() const {
	return session->playback->looping();
}
void AudioView::set_playback_pos(int pos) {
	if (mode == mode_capture)
		return;
	session->playback->set_pos(pos);
}
Range AudioView::playback_range() const {
	return session->playback->renderer->range();
}

void AudioView::playback_click() {
	if (mode == mode_capture)
		return;

	if (is_playback_active()) {
		if (playback_range().is_inside(hover().pos)) {
			set_playback_pos(hover().pos);
			hover().type = HoverData::Type::PLAYBACK_CURSOR;
			force_redraw();
		} else {
			stop();
		}
	}
}
void AudioView::update_playback_layers() {
	session->playback->renderer->allow_layers(get_playable_layers());
	force_redraw();
}

bool AudioView::has_any_solo_track() {
	for (auto *t: vtracks)
		if (t->solo)
			return true;
	return false;
}

bool allow_track_playback(AudioViewTrack *t, const base::set<Track*> &prevented, bool any_solo) {
	if (prevented.contains(t->track))
		return false;
	if (t->solo)
		return true;
	return !t->track->muted and !any_solo;
}

base::set<const Track*> AudioView::get_playable_tracks() {
	base::set<const Track*> tracks;
	auto prevented = mode->prevent_playback(); // capturing?
	bool any_solo = has_any_solo_track();
	for (auto *t: vtracks)
		if (allow_track_playback(t, prevented, any_solo))
			tracks.add(t->track);
	return tracks;
}

bool AudioView::has_any_solo_layer(Track *t) {
	for (auto *v: vlayers)
		if (v->solo and (v->layer->track == t))
			return true;
	return false;
}

bool allow_layer_playback(AudioViewLayer *l, bool any_solo) {
	if (l->solo)
		return true;
	return !l->layer->muted and !any_solo;
}

base::set<const TrackLayer*> AudioView::get_playable_layers() {
	auto tracks = get_playable_tracks();

	base::set<const TrackLayer*> layers;
	for (Track* t: weak(song->tracks)) {
		if (!tracks.contains(t))
			continue;
		bool any_solo = has_any_solo_layer(t);
		for (auto *l: vlayers)
			if ((l->layer->track == t) and allow_layer_playback(l, any_solo))
				layers.add(l->layer);
	}
	return layers;
}

void AudioView::set_playback_range_locked(bool locked) {
	playback_range_locked = locked;
	if (!locked)
		playback_wish_range = sel.range();
	force_redraw();
	//out_changed.notify();
}





void AudioView::set_cursor_pos(int pos) {
	sel.range_raw = Range(pos, 0);
	select_under_cursor();
	cam.make_sample_visible(pos, 0);
}

int AudioView::cursor_pos() {
	return sel.range_raw.offset;
}

Range AudioView::cursor_range() {
	return sel.range();
}

void AudioView::select_under_cursor() {
	set_selection(mode->get_selection_for_range(sel.range_raw));
}

bool AudioView::hover_any_object() {
	if (hover().bar)
		return true;
	if (hover().marker)
		return true;
	if (hover().sample)
		return true;
	if (hover().note)
		return true;
	return false;
}

bool AudioView::hover_selected_object() {
	if (hover().bar)
		return sel.has(hover().bar);
	if (hover().marker)
		return sel.has(hover().marker);
	if (hover().sample)
		return sel.has(hover().sample);
	if (hover().note)
		return sel.has(hover().note);
	return false;
}

void AudioView::select_object() {
	if (hover().bar) {
		sel.add(hover().bar);
	} else if (hover().marker) {
		sel.add(hover().marker);
	} else if (hover().sample) {
		sel.add(hover().sample);
	} else if (hover().note) {
		sel.add(hover().note);
	}
	update_selection();
}

void AudioView::toggle_object() {
	if (hover().bar) {
		sel.toggle(hover().bar);
	} else if (hover().marker) {
		sel.toggle(hover().marker);
	} else if (hover().sample) {
		sel.toggle(hover().sample);
	} else if (hover().note) {
		sel.toggle(hover().note);
	}
	update_selection();
}

bool AudioView::exclusively_select_layer(AudioViewLayer *l) {
	bool had_sel = sel.has(l->layer);
	sel._layers.clear();
	sel.add(l->layer);
	return had_sel;
}

bool AudioView::exclusively_select_track(AudioViewTrack *t) {
	bool had_sel = sel.has(t->track);
	sel._layers.clear();
	for (auto *l: weak(t->track->layers))
		sel.add(l);
	update_selection();
	return had_sel;
}

void AudioView::toggle_select_layer(AudioViewLayer *l) {
	sel.toggle(l->layer);
	update_selection();
}

void AudioView::exclusively_select_object() {
	sel.clear_data();
	select_object();
}

int AudioView::get_mouse_pos() {
	return cam.screen2sample(m.x);
}

int AudioView::get_mouse_pos_snap() {
	int pos = get_mouse_pos();
	snap_to_grid(pos);
	return pos;
}

HoverData AudioView::hover_time(const vec2 &m) {
	HoverData s;
	s.pos = get_mouse_pos();
	s.pos_snap = get_mouse_pos_snap();
	s.range = Range(s.pos, 0);
	s.y0 = s.y1 = m.y;
	s.type = HoverData::Type::TIME;
	return s;
}

void AudioView::cam_changed() {
	out_view_changed.notify();
	scroll_bar_time->set_view(cam.range());
	scroll_bar_time->set_content(song->range());
	force_redraw();
}

// hmmm, should we also unselect contents of this layer that is not in the cursor range?!?!?
void AudioView::toggle_select_layer_with_content_in_cursor(AudioViewLayer *l) {
	if (sel.has(l->layer))
		sel = sel.minus(SongSelection::all(song).filter({l->layer}));
	else
		sel = sel || SongSelection::from_range(song, sel.range()).filter({l->layer});
	//toggle_select_layer();
	update_selection();
}

// hmmm, should we also unselect contents of this layer that is not in the cursor range?!?!?
void AudioView::toggle_select_track_with_content_in_cursor(AudioViewTrack *t) {
	if (sel.has(t->track)) {
		for (auto *l: weak(t->track->layers))
			sel = sel.minus(SongSelection::all(song).filter({l}));
	} else {
		for (auto *l: weak(t->track->layers))
			sel = sel || SongSelection::from_range(song, sel.range()).filter({l});
	}
	update_selection();
	//toggle_select_layer();
}


Array<Track*> selected_tracks_sorted(AudioView *view) {
	Array<Track*> tracks;
	for (auto t: weak(view->song->tracks))
		if (view->sel.has(t))
			tracks.add(t);
	return tracks;
}

void AudioView::prepare_menu(hui::Menu *menu) {
	auto vl = cur_vlayer();//hover().vlayer;
	auto vt = cur_vtrack();//hover().vtrack;

	auto l = vl ? vl->layer : nullptr;
	auto t = vt ? vt->track : nullptr;

	// midi mode
	if (t) {
		menu->enable("menu-midi-mode", t->type == SignalType::MIDI);
		menu->enable("track-midi-mode-tab", t->instrument.string_pitch.num > 0);
		menu->enable("menu-audio-mode", t->type == SignalType::AUDIO);
	}
	if (vt) {
		menu->check("track-midi-mode-linear", vt->midi_mode() == MidiMode::LINEAR);
		menu->check("track-midi-mode-classical", vt->midi_mode() == MidiMode::CLASSICAL);
		menu->check("track-midi-mode-tab", vt->midi_mode() == MidiMode::TAB);
		menu->check("track-audio-mode-peaks", vt->audio_mode == AudioViewMode::PEAKS);
		menu->check("track-audio-mode-spectrum", vt->audio_mode == AudioViewMode::SPECTRUM);
	}
	
	// mute/solo
	if (t)
		menu->check("track-muted", t->muted);
	if (vt)
		menu->check("track-solo", vt->solo);
	if (l)
		menu->check("layer-muted", l->muted);
	if (vl)
		menu->check("layer-solo", vl->solo);
	if (vt and t) {
		menu->check("track-explode", !vt->imploded);
		menu->enable("track-explode", t->layers.num > 1);
	}

	if (t) {
		menu->enable("track-edit-midi", t->type == SignalType::MIDI);
		menu->enable("track_add_marker", true);//hover->type == Selection::Type::LAYER);
	}

	// convert
	if (t) {
		menu->enable("menu-convert", t->type == SignalType::AUDIO);
		//menu->enable("track-convert-mono", t->type == SignalType::AUDIO);
		//menu->enable("track-convert-stereo", t->type == SignalType::AUDIO);
		menu->enable("track-convert-stereo", t->channels == 1 and t->type == SignalType::AUDIO);
		menu->enable("track-convert-mono", t->channels == 2 and t->type == SignalType::AUDIO);
	}
	// view
	if (t) {
		menu->enable("menu-audio-mode", t->type == SignalType::AUDIO);
		menu->enable("track-audio-mode-peaks", t->type == SignalType::AUDIO);
		menu->enable("track-audio-mode-spectrum", t->type == SignalType::AUDIO);
		menu->enable("menu-midi-mode", t->type == SignalType::MIDI);
		menu->enable("track-midi-mode-linear", t->type == SignalType::MIDI);
		menu->enable("track-midi-mode-tab", t->type == SignalType::MIDI);
		menu->enable("track-midi-mode-classical", t->type == SignalType::MIDI);
	}
	if (l) {
		menu->enable("layer-merge", t->layers.num > 1);
		menu->enable("layer-mark-dominant", t->layers.num > 1);// and sel.layers.num == 1);
		menu->enable("layer-add-dominant", t->layers.num > 1);// and sel.layers.num == 1);
		menu->enable("layer-make-track", t->layers.num > 1);
		//menu->enable("layer-delete", !l->is_main());
	}

	// group
	if (t) {
		bool any_in_group = false;
		for (auto t: sel.tracks())
			if (t->send_target)
				any_in_group = true;
		menu->enable("track-ungroup", any_in_group);
	}

	menu->check("play-loop", looping());
	menu->check("playback-range-lock", playback_range_locked);
}

void AudioView::open_popup(hui::Menu* menu) {
	prepare_menu(menu);
	hover_before_leave = hover();
	menu->open_popup(win);
}

HoverData &AudioView::hover() {
	return scene_graph->hover;
}

MouseDelayPlanner *AudioView::mdp() {
	return scene_graph->mdp.get();
}

void AudioView::mdp_prepare(MouseDelayAction *a) {
	scene_graph->mdp_prepare(a);
}

void AudioView::mdp_run(MouseDelayAction *a) {
	scene_graph->mdp_run(a, m);
}

void AudioView::mdp_prepare(hui::Callback update) {
	scene_graph->mdp_prepare(update);
}

bool view_has_focus(AudioView *view) {
	return view->win->is_active(view->id);
}
