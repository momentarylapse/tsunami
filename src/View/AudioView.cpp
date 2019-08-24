/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "MouseDelayPlanner.h"
#include "Node/AudioViewTrack.h"
#include "Node/AudioViewLayer.h"
#include "Node/TimeScale.h"
#include "Node/SceneGraph.h"
#include "Node/ScrollBar.h"
#include "Node/Cursor.h"
#include "Node/Background.h"
#include "Mode/ViewModeDefault.h"
#include "Mode/ViewModeEditAudio.h"
#include "Mode/ViewModeMidi.h"
#include "Mode/ViewModeCurve.h"
#include "Mode/ViewModeCapture.h"
#include "Mode/ViewModeScaleBars.h"
#include "Mode/ViewModeScaleMarker.h"
#include "../Session.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Data/base.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/TrackMarker.h"
#include "../Data/Song.h"
#include "../Data/Sample.h"
#include "../Data/Rhythm/Bar.h"
#include "../Data/Rhythm/BarCollection.h"
#include "../Data/Rhythm/Beat.h"
#include "../Data/SampleRef.h"
#include "../Device/Stream/AudioOutput.h"
#include "../Module/Audio/SongRenderer.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Audio/PeakMeter.h"
#include "../Module/SignalChain.h"
#include "../Stuff/PerformanceMonitor.h"
#include "../lib/math/math.h"
#include "../lib/threads/Thread.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Mutex.h"
#include "Painter/BufferPainter.h"
#include "Painter/GridPainter.h"
#include "Painter/MidiPainter.h"
#include "Helper/PeakThread.h"
#include "SideBar/SideBar.h"


const float AudioView::FONT_SIZE = 10.0f;
const int AudioView::MAX_TRACK_CHANNEL_HEIGHT = 74;
const float AudioView::LINE_WIDTH = 1.0f;
const float AudioView::CORNER_RADIUS = 8.0f;
const int AudioView::SAMPLE_FRAME_HEIGHT = 20;
const int AudioView::TIME_SCALE_HEIGHT = 20;
const int AudioView::TRACK_HANDLE_WIDTH = 120;
const int AudioView::LAYER_HANDLE_WIDTH = 60;
const int AudioView::TRACK_HANDLE_HEIGHT = AudioView::TIME_SCALE_HEIGHT * 2;
const int AudioView::TRACK_HANDLE_HEIGHT_SMALL = AudioView::TIME_SCALE_HEIGHT;
const int AudioView::SCROLLBAR_WIDTH = 20;
const int AudioView::SNAPPING_DIST = 8;
ColorSchemeBasic AudioView::basic_colors;
ColorScheme AudioView::colors;


const string AudioView::MESSAGE_CUR_TRACK_CHANGE = "CurTrackChange";
const string AudioView::MESSAGE_CUR_SAMPLE_CHANGE = "CurSampleChange";
const string AudioView::MESSAGE_CUR_LAYER_CHANGE = "CurLayerChange";
const string AudioView::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string AudioView::MESSAGE_SETTINGS_CHANGE = "SettingsChange";
const string AudioView::MESSAGE_VIEW_CHANGE = "ViewChange";
const string AudioView::MESSAGE_VTRACK_CHANGE = "VTrackChange";
const string AudioView::MESSAGE_SOLO_CHANGE = "SoloChange";



// make shadows thicker
Image *ExpandImageMask(Image *im, float d) {
	Image *r = new Image(im->width, im->height, Black);
	for (int x=0; x<r->width; x++)
		for (int y=0; y<r->height; y++) {
			float a = 0;
			for (int i=0; i<r->width; i++)
				for (int j=0; j<r->height; j++) {
					float dd = sqrt(pow(i-x, 2) + pow(j-y, 2));
					if (dd > d+0.5f)
						continue;
					float aa = im->get_pixel(i, j).a;
					if (dd > d-0.5f)
						aa *= (d + 0.5f - dd);
					if (aa > a)
						a = aa;
				}
			r->set_pixel(x, y, color(a, 0, 0, 0));
		}
	return r;
}



void ___draw_str_with_shadow(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_shadow) {
	c->set_fill(false);
	c->set_line_width(3);
	c->set_color(col_shadow);
	c->draw_str(x, y, str);

	c->set_fill(true);
	c->set_line_width(1);
	c->set_color(col_text);
	c->draw_str(x, y, str);
}



AudioView::AudioView(Session *_session, const string &_id) :
	cam(this)
{
	id = _id;
	session = _session;
	win = session->win;
	song = session->song;

	perf_channel = PerformanceMonitor::create_channel("view", this);

	ColorSchemeBasic bright;
	bright.background = White;
	bright.text = color(1, 0.3f, 0.3f, 0.3f);
	bright.selection = color(1, 0.2f, 0.2f, 0.7f);
	bright.hover = White;
	bright.gamma = 1.0f;
	bright.name = "bright";
	basic_schemes.add(bright);

	ColorSchemeBasic dark;
	dark.background = color(1, 0.15f, 0.15f, 0.15f);
	dark.text = color(1, 0.95f, 0.95f, 0.95f);
	dark.selection = color(1, 0.3f, 0.3f, 0.9f);
	dark.hover = White;
	dark.gamma = 0.3f;
	dark.name = "dark";
	basic_schemes.add(dark);

	set_color_scheme(hui::Config.get_str("View.ColorScheme", "bright"));

	midi_view_mode = (MidiMode)hui::Config.get_int("View.MidiMode", (int)MidiMode::CLASSICAL);

	playback_range_locked = false;
	playback_loop = false;

	selection_mode = SelectionMode::NONE;
	selection_snap_mode = SelectionSnapMode::NONE;
	hide_selection = false;

	// modes
	mode = nullptr;
	mode_default = new ViewModeDefault(this);
	mode_edit_audio = new ViewModeEditAudio(this);
	mode_midi = new ViewModeMidi(this);
	mode_scale_bars = new ViewModeScaleBars(this);
	mode_scale_marker = new ViewModeScaleMarker(this);
	mode_curve = new ViewModeCurve(this);
	mode_capture = new ViewModeCapture(this);
	set_mode(mode_default);

	scene_graph = new SceneGraph([=]{ set_current(scene_graph->cur_selection); });
	area = rect(0, 1024, 0, 768);
	enabled = false;
	time_scale = new TimeScale(this);

	auto *vbox = new NodeVBox();
	auto *hbox = new NodeHBox();

	background = new Background(this);
	cursor_start = new Cursor(this, false);
	cursor_end = new Cursor(this, true);
	selection_marker = new SelectionMarker(this);
	scroll_bar_y = new ScrollBar(this);
	scroll_bar_y->auto_hide = true;
	scroll_bar_y->set_callback([=]{ thm.update_immediately(this, song, song_area()); });
	scroll_bar_time = new ScrollBarHorizontal(this);
	scroll_bar_time->set_callback([=]{ cam.dirty_jump(song->range_with_time().start() + scroll_bar_time->offset); });
	scroll_bar_time->constrained = false;

	metronome_overlay_vlayer = new AudioViewLayer(this, nullptr);
	metronome_overlay_vlayer->align.dz = 20;
	dummy_vtrack = new AudioViewTrack(this, nullptr);
	dummy_vlayer = new AudioViewLayer(this, nullptr);


	scene_graph->add_child(vbox);

	vbox->add_child(time_scale);
	vbox->add_child(hbox);
	vbox->add_child(scroll_bar_time);

	hbox->add_child(scroll_bar_y);
	hbox->add_child(background);

	scene_graph->add_child(cursor_start);
	scene_graph->add_child(cursor_end);
	scene_graph->add_child(selection_marker);
	scene_graph->add_child(metronome_overlay_vlayer);

	buffer_painter = new BufferPainter(this);
	grid_painter = new GridPainter(this);
	midi_painter = new MidiPainter(this);

	preview_sleep_time = hui::Config.get_int("PreviewSleepTime", 10);
	ScrollSpeed = 20;
	ScrollSpeedFast = 200;
	ZoomSpeed = hui::Config.get_float("View.ZoomSpeed", 0.1f);
	set_mouse_wheel_speed(hui::Config.get_float("View.MouseWheelSpeed", 1.0f));
	set_antialiasing(hui::Config.get_bool("View.Antialiasing", true));
	set_high_details(hui::Config.get_bool("View.HighDetails", true));
	hui::Config.set_float("View.ZoomSpeed", ZoomSpeed);

	images.speaker = LoadImage(tsunami->directory_static + "icons/volume.png");
	images.solo = LoadImage(tsunami->directory_static + "icons/solo.png");
	images.config = LoadImage(tsunami->directory_static + "icons/wrench.png");
	images.x = LoadImage(tsunami->directory_static + "icons/x.png");
	images.track_audio = LoadImage(tsunami->directory_static + "icons/track-audio.png");
	images.track_time = LoadImage(tsunami->directory_static + "icons/track-time.png");
	images.track_midi = LoadImage(tsunami->directory_static + "icons/track-midi.png");
	images.track_group = LoadImage(tsunami->directory_static + "icons/track-group.png");

	peak_thread = nullptr;
	draw_runner_id = -1;


	signal_chain = session->add_signal_chain_system("playback");
	renderer = (SongRenderer*)signal_chain->add(ModuleType::AUDIO_SOURCE, "SongRenderer");
	peak_meter = (PeakMeter*)signal_chain->add(ModuleType::AUDIO_VISUALIZER, "PeakMeter");
	output_stream = (AudioOutput*)signal_chain->add(ModuleType::STREAM, "AudioOutput");
	signal_chain->connect(renderer, 0, peak_meter, 0);
	signal_chain->connect(peak_meter, 0, output_stream, 0);
	signal_chain->mark_all_modules_as_system();

	signal_chain->subscribe(this, [=]{ on_stream_tick(); }, Module::MESSAGE_TICK);
	signal_chain->subscribe(this, [=]{ on_stream_state_change(); }, Module::MESSAGE_STATE_CHANGE);

	mx = my = 0;

	message.ttl = -1;


	// events
	win->event_xp(id, "hui:draw", [=](Painter *p){ on_draw(p); });
	win->event_x(id, "hui:mouse-move", [=]{ on_mouse_move(); });
	win->event_x(id, "hui:left-button-down", [=]{ on_left_button_down(); });
	win->event_x(id, "hui:left-double-click", [=]{ on_left_double_click(); });
	win->event_x(id, "hui:left-button-up", [=]{ on_left_button_up(); });
	win->event_x(id, "hui:middle-button-down", [=]{ on_middle_button_down(); });
	win->event_x(id, "hui:middle-button-up", [=]{ on_middle_button_up(); });
	win->event_x(id, "hui:right-button-down", [=]{ on_right_button_down(); });
	win->event_x(id, "hui:right-button-up", [=]{ on_right_button_up(); });
	win->event_x(id, "hui:mouse-leave", [=]{ on_mouse_leave(); });
	win->event_x(id, "hui:key-down", [=]{ on_key_down(); });
	win->event_x(id, "hui:key-up", [=]{ on_key_up(); });
	win->event_x(id, "hui:mouse-wheel", [=]{ on_mouse_wheel(); });

	win->activate(id);


	menu_song = hui::CreateResourceMenu("popup-menu-song");
	menu_track = hui::CreateResourceMenu("popup-menu-track");
	menu_layer = hui::CreateResourceMenu("popup-menu-layer");
	menu_playback_range = hui::CreateResourceMenu("popup-menu-playback-range");
	menu_sample = hui::CreateResourceMenu("popup-menu-sample");
	menu_marker = hui::CreateResourceMenu("popup-menu-marker");
	menu_bar = hui::CreateResourceMenu("popup-menu-bar");
	menu_bar_gap = hui::CreateResourceMenu("popup-menu-bar-gap");
	menu_buffer = hui::CreateResourceMenu("popup-menu-buffer");

	enable(true);

	//ForceRedraw();
	update_menu();
}

AudioView::~AudioView() {
	if (draw_runner_id >= 0)
		hui::CancelRunner(draw_runner_id);

	signal_chain->unsubscribe(this);
	song->unsubscribe(this);

	delete scroll_bar_y;
	delete scroll_bar_time;

	delete buffer_painter;
	delete grid_painter;
	delete midi_painter;

	delete mode_curve;
	delete mode_scale_bars;
	delete mode_scale_marker;
	delete mode_edit_audio;
	delete mode_midi;
	delete mode_capture;
	delete mode_default;

	if (peak_thread)
		delete peak_thread;

	delete dummy_vtrack;
	delete dummy_vlayer;
	delete metronome_overlay_vlayer;

	delete images.speaker;
	delete images.x;
	delete images.solo;
	delete images.config;
	delete images.track_audio;
	delete images.track_midi;
	delete images.track_time;

	delete signal_chain;

	PerformanceMonitor::delete_channel(perf_channel);
}

void AudioView::set_antialiasing(bool set) {
	antialiasing = set;
	hui::Config.set_bool("View.Antialiasing", antialiasing);
	force_redraw();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void AudioView::set_high_details(bool set) {
	high_details = set;
	detail_steps = high_details ? 1 : 3;
	hui::Config.set_bool("View.HighDetails", high_details);
	force_redraw();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void AudioView::set_mouse_wheel_speed(float speed) {
	mouse_wheel_speed = speed;
	hui::Config.set_float("View.MouseWheelSpeed", mouse_wheel_speed);
	notify(MESSAGE_SETTINGS_CHANGE);
}

void AudioView::set_color_scheme(const string &name) {
	hui::Config.set_str("View.ColorScheme", name);
	basic_colors = basic_schemes[0];
	for (auto &b: basic_schemes)
		if (b.name == name)
			basic_colors = b;

	colors = basic_colors.create(true);
	force_redraw();
}

string mode_name(ViewMode *m, AudioView *v) {
	if (m == v->mode_default)
		return "default";
	if (m == v->mode_curve)
		return "curves";
	if (m == v->mode_midi)
		return "midi";
	if (m == v->mode_capture)
		return "capture";
	return "??";
}

void AudioView::set_mode(ViewMode *m) {
	if (m == mode)
		return;
	if (mode) {
		//msg_write("end mode " + mode_name(mode, this));
		mode->on_end();
	}
	mode = m;
	if (mode) {
		//msg_write("start mode " + mode_name(mode, this));
		mode->on_start();
	}
	thm.dirty = true;
	force_redraw();
}

void AudioView::set_mouse() {
	mx = hui::GetEvent()->mx;
	my = hui::GetEvent()->my;
	select_xor = win->get_key(hui::KEY_CONTROL);
}

int AudioView::mouse_over_sample(SampleRef *s) {
	if ((mx >= s->area.x1) and (mx < s->area.x2)) {
		int offset = cam.screen2sample(mx) - s->pos;
		if ((my >= s->area.y1) and (my < s->area.y1 + SAMPLE_FRAME_HEIGHT))
			return offset;
		if ((my >= s->area.y2 - SAMPLE_FRAME_HEIGHT) and (my < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::selection_update_pos(HoverData &s) {
	s.pos = cam.screen2sample(mx);
}

void AudioView::update_selection() {
	//sel.range = sel.range.canonical();

	if (!playback_range_locked) {
		playback_wish_range = sel.range();
	}


	renderer->set_range(get_playback_selection(false));

	// TODO ...check....
	if (is_playback_active()) {
		if (renderer->range().is_inside(playback_pos()))
			renderer->set_range(get_playback_selection(false));
		else{
			stop();
		}
	}
	force_redraw();

	notify(MESSAGE_SELECTION_CHANGE);
}


void AudioView::set_selection(const SongSelection &s) {
	sel = s;
	update_selection();
}


bool AudioView::mouse_over_time(int pos) {
	int ssx = cam.sample2screen(pos);
	return ((mx >= ssx - 5) and (mx <= ssx + 5));
}


Range AudioView::get_playback_selection(bool for_recording) {
	if (playback_wish_range.length > 0)
		return playback_wish_range;
	if (sel.range().empty()) {
		if (for_recording)
			return Range(sel.range().start(), 0x70000000);
		int num = song->range_with_time().end() - sel.range().start();
		if (num <= 0)
			num = song->sample_rate; // 1 second
		return Range(sel.range().start(), num);
	}
	return sel.range();
}

void AudioView::set_selection_snap_mode(SelectionSnapMode mode) {
	selection_snap_mode = mode;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void _update_find_min(int &new_pos, bool &found, float &dmin, int pos, int trial_pos) {
	float dist = fabs(trial_pos - pos);
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
		if (mode == mode_midi)
			sub_beats = mode_midi->sub_beat_partition;

		// time bar...
		auto beats = song->bars.get_beats(cam.range(), true, sub_beats>0, sub_beats);
		for (Beat &b: beats)
			_update_find_min(new_pos, found, dmin, pos, b.range.offset);

	}else if (selection_snap_mode == SelectionSnapMode::BAR) {
		float dmin = cam.dscreen2sample(SNAPPING_DIST) * 1000;
		auto bars = song->bars.get_bars(cam.range());
		for (Bar *b: bars) {
			_update_find_min(new_pos, found, dmin, pos, b->range().start());
			_update_find_min(new_pos, found, dmin, pos, b->range().end());
		}
	}else if (selection_snap_mode == SelectionSnapMode::PART) {
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

	scene_graph->on_mouse_move();

	force_redraw();
}

void AudioView::on_left_button_down() {
	set_mouse();
	scene_graph->on_left_button_down();

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
	scene_graph->on_left_button_up();

	force_redraw();
	update_menu();
}



void AudioView::on_middle_button_down() {
}



void AudioView::on_middle_button_up() {
}



void AudioView::on_right_button_down() {
	scene_graph->on_right_button_down();
}

void AudioView::on_right_button_up() {
	mode->on_right_button_up();
}

void AudioView::on_mouse_leave() {
	// TODO check if necessary
	if (!hui::GetEvent()->lbut and !hui::GetEvent()->rbut) {
		hover().clear();
		force_redraw();
	}
}



void AudioView::on_left_double_click() {
	scene_graph->on_left_double_click();
	mode->on_left_double_click();
	force_redraw();
	update_menu();
}


AudioViewLayer *next_layer(AudioView *view, AudioViewLayer *vlayer) {
	bool found = false;
	for (auto *l: view->vlayer) {
		if (found and !l->hidden)
			return l;
		if (l == vlayer)
			found = true;
	}
	return vlayer;
}
AudioViewLayer *prev_layer(AudioView *view, AudioViewLayer *vlayer) {
	AudioViewLayer *prev = nullptr;
	for (auto *l: view->vlayer) {
		if (l == vlayer and prev)
			return prev;
		if (!l->hidden)
			prev = l;
	}
	return vlayer;
}

void move_to_layer(AudioView *view, int delta) {
	auto *vlayer = view->cur_vlayer();
	if (delta > 0)
		vlayer = next_layer(view, vlayer);
	else
		vlayer = prev_layer(view, vlayer);

	view->set_current(vlayer->get_hover_data(0,0));
	view->exclusively_select_layer(vlayer);
	view->select_under_cursor();
}


void zoom_y(AudioView *view, float zoom) {
	view->cam.scale_y = clampf(zoom, 0.5f, 2.0f);
	view->thm.dirty = true;
	view->set_message(format(_("vertical zoom %.0f%%"), view->cam.scale_y * 100.0f));
	view->force_redraw();
}


void AudioView::on_command(const string &id) {
	if (id == "track-muted") {
		bool muted = cur_track()->muted;
		song->begin_action_group();
		for (auto *t: song->tracks)
			if (sel.has(t))
				t->set_muted(!muted);
		song->end_action_group();
	}
	if (id == "track-solo") {
		if (cur_vtrack()->solo) {
			for (auto *vt: vtrack)
				vt->set_solo(false);
		} else {
			for (auto *vt: vtrack)
				vt->set_solo(sel.has(vt->track));
		}
	}
	if (id == "layer-muted")
		cur_layer()->set_muted(!cur_layer()->muted);
	if (id == "layer-solo")
		cur_vlayer()->set_solo(!cur_vlayer()->solo);

	
	if (id == "track-explode") {
		if (cur_track()->layers.num > 0) {
			if (cur_vtrack()->imploded)
				explode_track(cur_track());
			else
				implode_track(cur_track());
		}
	}

	if (mode == mode_default) {
		if (id == "layer-up")
			move_to_layer(this, -1);
		if (id == "layer-down")
			move_to_layer(this, 1);
	}

	float dt = 0.05f;
	if (id == "cam-move-right")
		cam.move(ScrollSpeedFast * dt / cam.scale);
	if (id == "cam-move-left")
		cam.move(- ScrollSpeedFast * dt / cam.scale);
	if (id == "cursor-jump-start")
		set_cursor_pos(song->range_with_time().start());
	if (id == "cursor-jump-end")
		set_cursor_pos(song->range_with_time().end());

	// zoom
	if (id == "zoom-in")
		cam.zoom(exp(  ZoomSpeed), mx);
	if (id == "zoom-out")
		cam.zoom(exp(- ZoomSpeed), mx);

	// vertical zoom
	if (id == "vertical-zoom-in")
		zoom_y(this, cam.scale_y * 1.2f);
	if (id == "vertical-zoom-out")
		zoom_y(this, cam.scale_y / 1.2f);
	if (id == "vertical-zoom-default")
		zoom_y(this, 1.0f);

	mode->on_command(id);
}



void AudioView::on_key_down() {
	int k = hui::GetEvent()->key_code;
	if (k == hui::KEY_ESCAPE) {
		if (mdp()->acting()) {
			mdp()->cancel();
		} else {
			if (win->side_bar->allow_close())
				session->set_mode("default");
		}
	}
	mode->on_key_down(k);
}

void AudioView::on_key_up() {
	int k = hui::GetEvent()->key_code;
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
	sel.samples.clear();
}

void AudioView::update_buffer_zoom() {
	prefered_buffer_layer = -1;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (cam.scale < 0.2f)
		for (int i=24-1;i>=0;i--) {
			double _f = (double)(1 << (i + AudioBuffer::PEAK_OFFSET_EXP));
			if (_f > 2.0 / cam.scale) {
				prefered_buffer_layer = i;
				buffer_zoom_factor = _f;
			}
		}
}

void _try_set_good_cur_layer(AudioView *v) {
	for (auto *l: v->vlayer)
		if (l->layer->track == v->_prev_selection.track()){
			//msg_write("  -> set by track");
			v->__set_cur_layer(l);
			return;
		}
	if (v->vlayer.num > 0) {
		//msg_write("  -> set first layer");
		v->__set_cur_layer(v->vlayer[0]);
	} else {
		//msg_error("....no vlayers");
	}
}

void AudioView::check_consistency()
{
	// cur_vlayer = null
	if (!cur_vlayer() and (vlayer.num > 0)){
		//msg_error("cur_vlayer = nil");
		//msg_write(msg_get_trace());
		//msg_write("  -> setting first");
		__set_cur_layer(vlayer[0]);
	}

	// cur_vlayer illegal?
	if (cur_vlayer() and (vlayer.find(cur_vlayer()) < 0)){
		//msg_error("cur_vlayer illegal...");
		//msg_write(msg_get_trace());
		_try_set_good_cur_layer(this);
	}

	// illegal midi mode?
	for (auto *t: vtrack)
		if ((t->midi_mode() == MidiMode::TAB) and (t->track->instrument.string_pitch.num == 0))
			t->set_midi_mode(MidiMode::CLASSICAL);
}

void AudioView::implode_track(Track *t) {
	for (auto *l: vlayer)
		if (l->layer->track == t) {
			l->represents_imploded = l->layer->is_main();
			l->hidden = !l->layer->is_main();
		}
	get_track(t)->imploded = true;
	thm.dirty = true;
	force_redraw();
}

void AudioView::explode_track(Track *t) {
	for (auto *l: vlayer)
		if (l->layer->track == t) {
			l->represents_imploded = false;
			l->hidden = false;
		}
	get_track(t)->imploded = false;
	thm.dirty = true;
	force_redraw();
}

void AudioView::on_song_new() {
	__set_cur_layer(nullptr);
	update_tracks();
	sel.range_raw = Range::EMPTY;
	sel.clear();
	for (Track *t: song->tracks)
		for (TrackLayer *l: t->layers)
			sel.add(l);
	check_consistency();
	optimize_view();
}

void AudioView::on_song_finished_loading() {
	for (auto *t: vtrack)
		if (t->track->layers.num > 1)
			implode_track(t->track);
	optimize_view();
	hui::RunLater(0.5f, [=]{ optimize_view(); });
}

void AudioView::on_song_tracks_change() {
	update_tracks();
	force_redraw();
	update_menu();
}

void AudioView::on_song_change() {
	if (song->history_enabled())
		update_peaks();
}

void AudioView::on_stream_tick() {
	cam.make_sample_visible(playback_pos(), session->sample_rate() * 2);
	force_redraw();
}

void AudioView::on_stream_state_change() {
	force_redraw();
}

void AudioView::on_update() {
	check_consistency();
	force_redraw();
}

void AudioView::update_peaks_now(AudioBuffer &buf) {
	int n = buf._update_peaks_prepare();

	for (int i=0; i<n; i++)
		if (buf._peaks_chunk_needs_update(i))
			buf._update_peaks_chunk(i);
}

AudioViewTrack *AudioView::get_track(Track *track) {
	for (auto t: vtrack) {
		if (t->track == track)
			return t;
	}
	return dummy_vtrack;
}

AudioViewLayer *AudioView::get_layer(TrackLayer *layer) {
	for (auto l: vlayer) {
		if (l->layer == layer)
			return l;
	}

	msg_write("get_layer() failed for " + p2s(layer));
	msg_write(msg_get_trace());

	return dummy_vlayer;
}

void AudioView::update_tracks()
{
	Array<AudioViewTrack*> vtrack2;
	Array<AudioViewLayer*> vlayer2;
	vtrack2.resize(song->tracks.num);
	vlayer2.resize(song->layers().num);

	foreachi(Track *t, song->tracks, ti){
		bool found = false;

		// find existing
		foreachi(auto *v, vtrack, vi)
			if (v){
				if (v->track == t){
					vtrack2[ti] = v;
					vtrack[vi] = nullptr;
					found = true;
					break;
				}
			}

		// new track
		if (!found){
			vtrack2[ti] = new AudioViewTrack(this, t);
			background->add_child(vtrack2[ti]);
		}
	}

	int li = 0;
	for (TrackLayer *l: song->layers()){

		bool found = false;

		// find existing
		foreachi(auto *v, vlayer, vi)
			if (v){
				if (v->layer == l){
					vlayer2[li] = v;
					vlayer[vi] = nullptr;
					found = true;
					break;
				}
			}

		// new layer
		if (!found){
			vlayer2[li] = new AudioViewLayer(this, l);
			background->add_child(vlayer2[li]);
			sel.add(l);
		}

		li ++;
	}

	// delete deleted
	for (auto *v: vtrack)
		if (v)
			background->delete_child(v);
	for (auto *v: vlayer)
		if (v)
			background->delete_child(v);
	vtrack = vtrack2;
	vlayer = vlayer2;
	thm.dirty = true;

	// guess where to create new tracks
	foreachi(auto *v, vtrack, i){
		if (v->area.height() == 0){
			if (i > 0){
				v->area = vtrack[i-1]->area;
				v->area.y1 = v->area.y2;
			}else if (vtrack.num > 1){
				v->area = vtrack[i+1]->area;
				v->area.y2 = v->area.y1;
			}
		}
	}

	// guess where to create new tracks
	foreachi(auto *v, vlayer, i){
		if (v->area.height() == 0){
			if (i > 0){
				v->area = vlayer[i-1]->area;
				v->area.y1 = v->area.y2;
			}else if (vtrack.num > 1){
				v->area = vlayer[i+1]->area;
				v->area.y2 = v->area.y1;
			}
		}
	}

	rebuild_scene_graph();
	hover().node = nullptr;

	// TODO: detect order change
	check_consistency();
	notify(MESSAGE_VTRACK_CHANGE);
}

void AudioView::rebuild_scene_graph() {
	/*scene_graph->children.clear();
	scene_graph->children.add(background);
	scene_graph->children.add(scroll_bar_h);
	scene_graph->children.add(scroll_bar_w);

	for (auto *v: vlayer)
		scene_graph->children.add(v);
	for (auto *v: vtrack)
		scene_graph->children.add(v);

	scene_graph->children.add(metronome_overlay_vlayer);
	scene_graph->children.add(time_scale);
	scene_graph->children.add(cursor_start);
	scene_graph->children.add(cursor_end);
	scene_graph->children.add(selection_marker);*/
}

bool need_metro_overlay(Song *song, AudioView *view) {
	Track *tt = song->time_track();
	if (!tt)
		return false;
	auto *vv = view->get_layer(tt->layers[0]);
	if (!vv)
		return false;
	return !vv->on_screen();
}

bool AudioView::update_scene_graph() {

	bool scroll_bar_w_needed = !cam.range().covers(song->range_with_time());
	bool animating = thm.update(this, song, song_area());

	scroll_bar_time->hidden = !scroll_bar_w_needed;
	scroll_bar_time->update(cam.range().length, song->range_with_time().length);

	for (auto *v: vlayer) {
		v->update_header();
	}

	metronome_overlay_vlayer->hidden = !need_metro_overlay(song, this);
	if (!metronome_overlay_vlayer->hidden) {
		metronome_overlay_vlayer->layer = song->time_track()->layers[0];
		metronome_overlay_vlayer->area = rect(song_area().x1, song_area().x2, song_area().y1, song_area().y1 + this->TIME_SCALE_HEIGHT*2);
	}
	return animating;
}

rect AudioView::song_area() {
	if (!background)
		return rect::EMPTY;
	return background->area;
}


rect AudioView::get_boxed_str_rect(Painter *c, float x, float y, const string &str) {
	float w = c->get_str_width(str);
	return rect(x-CORNER_RADIUS, x + w + CORNER_RADIUS, y-CORNER_RADIUS, y + c->font_size + CORNER_RADIUS);
}

void AudioView::draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, int align) {
	rect r = get_boxed_str_rect(c, x, y, str);
	float dx =r.width() / 2 - CORNER_RADIUS;
	dx *= (align - 1);
	r.x1 += dx;
	r.x2 += dx;
	c->set_color(col_bg);
	c->set_roundness(CORNER_RADIUS);
	c->draw_rect(r);
	c->set_roundness(0);
	c->set_color(col_text);
	c->draw_str(x + dx, y, str);
}

void AudioView::draw_framed_box(Painter *p, const rect &r, const color &bg, const color &frame, float frame_width) {
	p->set_color(bg);
	p->set_roundness(CORNER_RADIUS);
	p->draw_rect(r);
	
	p->set_fill(false);
	p->set_line_width(frame_width);
	p->set_color(frame);
	p->draw_rect(r);
	p->set_line_width(1);
	p->set_fill(true);
	p->set_roundness(0);
}

float AudioView::draw_str_constrained(Painter *p, float x, float y, float w_max, const string &_str, int align) {
	string str = _str;
	float w = p->get_str_width(str);
	//float fs0 = p->font_size;
	if (w > w_max) {
		for (int i=str.num/2; i>=1; i--) {
			str = _str.head(i) + ".." + _str.tail(i);
			w = p->get_str_width(str);
			if (w < w_max)
				break;
		}
	}
	float dx = w/2 * (align-1);
	p->draw_str(x + dx, y, str);
	//p->set_font_size(fs0);
	return w;
}


void AudioView::draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area) {
	c->set_font("", -1, true, false);
	float w = c->get_str_width(msg);
	float x = min(max(mx - 20.0f, area.x1 + 2.0f), area.x2 - w);
	float y = min(max(my + 30, area.y1 + 2.0f), area.y2 - FONT_SIZE - 5);
	draw_boxed_str(c, x, y, msg, colors.background, colors.text_soft1);
	c->set_font("", -1, false, false);
}

void AudioView::draw_cursor_hover(Painter *c, const string &msg) {
	draw_cursor_hover(c, msg, mx, my, song_area());
}

void AudioView::draw_message(Painter *c, Message &m) {
	float xm = area.mx();
	float ym = area.my();
	float a = min(m.ttl*8, 1.0f);
	a = pow(a, 0.4f);
	color c1 = colors.high_contrast_a;
	color c2 = colors.high_contrast_b;
	c1.a = c2.a = a;
	c->set_font_size(FONT_SIZE * m.size * a);
	draw_boxed_str(c, xm, ym, m.text, c1, c2, 0);
	c->set_font_size(FONT_SIZE);
}

void AudioView::draw_time_line(Painter *c, int pos, const color &col, bool hover, bool show_time, bool show_circle) {
	float x = cam.sample2screen(pos);
	if ((x >= song_area().x1) and (x <= song_area().x2)) {
		color cc = col;
		if (hover)
			cc = colors.selection_boundary_hover;
		c->set_color(cc);
		c->set_line_width(2.0f);
		c->draw_line(x, area.y1, x, area.y2);
		if (show_time)
			draw_boxed_str(c,  x, song_area().my(), song->get_time_str_long(pos), cc, colors.background);
		c->set_line_width(1.0f);
		if (show_circle)
			c->draw_circle(x, area.y2, 8);
	}
}

void AudioView::draw_background(Painter *c) {
}

void AudioView::draw_selection(Painter *c) {
}

void AudioView::draw_song(Painter *c) {
	bool slow_repeat = false;
	bool animating = update_scene_graph();

	c->set_antialiasing(false);

	cam.update(0.1f);

	update_buffer_zoom();

	scene_graph->update_geometry_recursive(area);
	scene_graph->draw(c);

	// playing/capturing position
	if (is_playback_active())
		draw_time_line(c, playback_pos(), colors.preview_marker, false, true);

	mode->draw_post(c);

	// tool tip?
	string tip;
	if (hover().node)
		tip = hover().node->get_tip();
	if (tip.num > 0)
		draw_cursor_hover(c, tip);

	tip = mode->get_tip();
	if (tip.num > 0)
		draw_boxed_str(c, song_area().mx(), area.y2 - 30, tip, colors.text_soft1, colors.background_track_selected, 0);


	if (message.ttl > 0) {
		draw_message(c, message);
		message.ttl -= 0.03f;
		animating = true;
	}

	if (peak_thread and !peak_thread->is_done())
		slow_repeat = true;

	if (cam.needs_update())
		animating = true;

	if (animating or slow_repeat)
		draw_runner_id = hui::RunLater(animating ? 0.03f : 0.2f, [=]{ force_redraw(); });
}

int frame = 0;

void AudioView::on_draw(Painter *c) {
	PerformanceMonitor::start_busy(perf_channel);

	colors = basic_colors.create(win->is_active(id));

	area = c->area();
	clip = c->clip();

	c->set_font_size(FONT_SIZE);
	c->set_line_width(LINE_WIDTH);

	if (enabled)
		draw_song(c);

	//c->draw_str(100, 100, i2s(frame++));

	colors = basic_colors.create(true);

	PerformanceMonitor::end_busy(perf_channel);
}

void AudioView::optimize_view() {
	if (area.x2 <= 0)
		area.x2 = 1024;

	Range r = song->range_with_time();

	if (r.length == 0)
		r.length = 10 * song->sample_rate;

	cam.show(r);
}

void AudioView::update_menu() {
	MidiMode common_midi_mode = midi_view_mode;
	for (auto *t: vtrack)
		if (t->track)
			if ((t->track->type == SignalType::MIDI) and (t->midi_mode_wanted != midi_view_mode))
				common_midi_mode = MidiMode::DONT_CARE;
	// view
	win->check("view-midi-linear", common_midi_mode == MidiMode::LINEAR);
	win->check("view-midi-tab", common_midi_mode == MidiMode::TAB);
	win->check("view-midi-classical", common_midi_mode == MidiMode::CLASSICAL);
}

void AudioView::update_peaks() {
	//msg_write("-------------------- view update peaks");
	if (peak_thread) {
		//msg_error("   already updating peaks...");
		peak_thread->allow_running = false;
		peak_thread->join();
		delete peak_thread;
	}

	peak_thread = new PeakThread(this);
	peak_thread->run();

	/*for (int i=0; i<5; i++){
		if (peak_thread->is_done()){
			forceRedraw();
			break;
		}else
			hui::Sleep(0.001f);
	}*/
}

void AudioView::set_midi_view_mode(MidiMode mode) {
	midi_view_mode = mode;
	hui::Config.set_int("View.MidiMode", (int)midi_view_mode);
	for (auto *t: vtrack)
		t->set_midi_mode(mode);
	//forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	update_menu();
}

void AudioView::zoom_in() {
	cam.zoom(2.0f, mx);
}

void AudioView::zoom_out() {
	cam.zoom(0.5f, mx);
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
		for (Track *t: song->tracks) {
			if (!sel.has(t))
				continue;

			if (t->type == SignalType::BEATS)
				for (Bar* b: song->bars)
					test_range(b->range(), r, update);

			// midi
			for (TrackLayer *l: t->layers)
				if (sel.has(l))
					for (MidiNote *n: l->midi)
						test_range(n->range, r, update);

			for (TrackLayer *l: t->layers) {
				// buffers
				for (AudioBuffer &b: l->buffers)
					test_range(b.range(), r, update);

				// samples
				for (SampleRef *s: l->samples)
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

void AudioView::set_current(const HoverData &h) {
	_prev_selection = cur_selection;

	cur_selection = h;

	if (!cur_vlayer()) {
		//msg_write("   ...setting cur_vlayer = nil");
		//msg_write(msg_get_trace());
		cur_selection.vlayer = _prev_selection.vlayer;
	}
	if (cur_sample() != _prev_selection.sample) {
		notify(MESSAGE_CUR_SAMPLE_CHANGE);
	}
	if (cur_vlayer() != _prev_selection.vlayer) {
		mode->on_cur_layer_change();
		force_redraw();
		notify(MESSAGE_CUR_LAYER_CHANGE);
	}
	if (cur_track() != _prev_selection.track()) {
		notify(MESSAGE_CUR_TRACK_CHANGE);
	}
}

Track *AudioView::cur_track() {
	if (!cur_vlayer())
		return nullptr;
	if (!cur_vlayer()->layer)
		return nullptr;
	return cur_vlayer()->layer->track;
}

AudioViewTrack *AudioView::cur_vtrack() {
	for (auto *t: vtrack)
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

void AudioView::__set_cur_sample(SampleRef *s) {
	auto h = cur_selection;
	h.sample = s;
	set_current(h);
}



// unused?!?
void AudioView::enable(bool _enabled) {
	if (enabled and !_enabled) {
		song->unsubscribe(this);
	} else if (!enabled and _enabled) {
		song->subscribe(this, [=]{ on_song_tracks_change(); }, song->MESSAGE_ADD_TRACK);
		song->subscribe(this, [=]{ on_song_tracks_change(); }, song->MESSAGE_DELETE_TRACK);
		song->subscribe(this, [=]{ on_song_tracks_change(); }, song->MESSAGE_ADD_LAYER);
		song->subscribe(this, [=]{ on_song_tracks_change(); }, song->MESSAGE_DELETE_LAYER);
		song->subscribe(this, [=]{ on_song_tracks_change(); }, song->MESSAGE_CHANGE_CHANNELS);
		song->subscribe(this, [=]{ on_song_new(); }, song->MESSAGE_NEW);
		song->subscribe(this, [=]{ on_song_finished_loading(); }, song->MESSAGE_FINISHED_LOADING);
		song->subscribe(this, [=]{ on_song_change(); }, song->MESSAGE_CHANGE);
		song->subscribe(this, [=]{
			force_redraw();
			update_menu();
		}, song->MESSAGE_ANY);
	}
	enabled = _enabled;
}


void AudioView::play() {
	if (signal_chain->is_paused()) {
		signal_chain->start();
		return;
	}

	prepare_playback(get_playback_selection(false), true);
	signal_chain->start();
}

void AudioView::prepare_playback(const Range &range, bool allow_loop) {
	if (signal_chain->is_playback_active())
		stop();

	renderer->prepare(range, allow_loop);
	renderer->allow_tracks(get_playable_tracks());
	renderer->allow_layers(get_playable_layers());
	_playback_stream_offset = range.offset - output_stream->samples_played();

	signal_chain->command(ModuleCommand::PREPARE_START, 0);
}

void AudioView::set_playback_loop(bool loop) {
	playback_loop = loop;
	renderer->loop_if_allowed = playback_loop;
	force_redraw();
	notify(MESSAGE_SELECTION_CHANGE);
}

void AudioView::stop() {
	signal_chain->stop_hard();
}

void AudioView::pause(bool _pause) {
	if (_pause)
		signal_chain->stop();
	else
		signal_chain->start();
}

bool AudioView::is_playback_active() {
	return signal_chain->is_playback_active();
}

bool AudioView::is_paused() {
	return signal_chain->is_paused();
}

int loop_in_range(int pos, const Range &r) {
	return loopi(pos, r.start(), r.end());
}

int AudioView::playback_pos() {
	// crappy syncing....
	_playback_sync_counter ++;
	if (_playback_sync_counter > 200)
		_sync_playback_pos();
	
	int pos = output_stream->samples_played() + _playback_stream_offset;
	if (playback_loop and renderer->allow_loop)
		return loop_in_range(pos, renderer->range());
	return pos;
}

// crappy syncing....
void AudioView::_sync_playback_pos() {
	int spos = output_stream->samples_played();
	int xpos = renderer->get_pos() - output_stream->get_available() - output_stream->get_latency();
	_playback_stream_offset = xpos - spos;
	_playback_sync_counter = 0;
}

void AudioView::set_playback_pos(int pos) {
	if (mode == mode_capture)
		return;
	renderer->set_pos(pos);
	_playback_stream_offset = pos - output_stream->samples_played();
}

void AudioView::playback_click() {
	if (mode == mode_capture)
		return;

	if (is_playback_active()) {
		if (renderer->range().is_inside(hover().pos)) {
			set_playback_pos(hover().pos);
			hover().type = HoverData::Type::PLAYBACK_CURSOR;
			force_redraw();
		} else {
			stop();
		}
	}
}

bool AudioView::has_any_solo_track() {
	for (auto *t: vtrack)
		if (t->solo)
			return true;
	return false;
}

Set<const Track*> AudioView::get_playable_tracks() {
	Set<const Track*> tracks;
	auto prevented = mode->prevent_playback();
	bool any_solo = has_any_solo_track();
	for (auto *t: vtrack)
		if (!t->track->muted and (t->solo or !any_solo) and !prevented.contains(t->track))
			tracks.add(t->track);
	return tracks;
}

bool AudioView::has_any_solo_layer(Track *t) {
	for (auto *v: vlayer)
		if (v->solo and (v->layer->track == t))
			return true;
	return false;
}

Set<const TrackLayer*> AudioView::get_playable_layers() {
	auto tracks = get_playable_tracks();

	Set<const TrackLayer*> layers;
	for (Track* t: song->tracks) {
		if (!tracks.contains(t))
			continue;
		bool any_solo = has_any_solo_layer(t);
		for (auto *l: vlayer)
			if ((l->layer->track == t) and (l->solo or !any_solo) and !l->layer->muted)
				layers.add(l->layer);
	}
	return layers;
}

void AudioView::set_message(const string& text, float size) {
	message.text = text;
	message.ttl = 0.8f;
	message.size = size;
	force_redraw();
}

void AudioView::set_playback_range_locked(bool locked) {
	playback_range_locked = locked;
	if (!locked)
		playback_wish_range = sel.range();
	force_redraw();
	//notify();
}





void AudioView::set_cursor_pos(int pos) {
	sel.range_raw = Range(pos, 0);
	select_under_cursor();
	//update_selection();
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
}

bool AudioView::exclusively_select_layer(AudioViewLayer *l) {
	bool had_sel = sel.has(l->layer);
	sel.layers.clear();
	sel.add(l->layer);
	return had_sel;
}

void AudioView::toggle_select_layer(AudioViewLayer *l) {
	sel.toggle(l->layer);
}

void AudioView::exclusively_select_object() {
	sel.clear_data();
	select_object();
}

int AudioView::get_mouse_pos() {
	return cam.screen2sample(mx);
}

int AudioView::get_mouse_pos_snap() {
	int pos = get_mouse_pos();
	snap_to_grid(pos);
	return pos;
}

HoverData AudioView::hover_time(float mx, float my) {
	HoverData s;
	s.pos = get_mouse_pos();
	s.pos_snap = get_mouse_pos_snap();
	s.range = Range(s.pos, 0);
	s.y0 = s.y1 = my;
	s.type = HoverData::Type::TIME;
	return s;
}

void AudioView::cam_changed() {
	notify(MESSAGE_VIEW_CHANGE);
	scroll_bar_time->update(cam.range().length, song->range_with_time().length);
	scroll_bar_time->offset = cam.pos - song->range_with_time().start();
	force_redraw();
}

// hmmm, should we also unselect contents of this layer that is not in the cursor range?!?!?
void AudioView::toggle_select_layer_with_content_in_cursor(AudioViewLayer *l) {
	if (sel.has(l->layer))
		sel = sel.minus(SongSelection::all(song).filter({l->layer}));
	else
		sel = sel || SongSelection::from_range(song, sel.range()).filter({l->layer});
	//toggle_select_layer();
}

void AudioView::prepare_menu(hui::Menu *menu) {
	auto *vl = hover().vlayer;
	auto *vt = hover().vtrack;
	auto *l = hover().layer();
	auto *t = hover().track();
	// midi mode
	if (t) {
		menu->enable("menu-midi-mode", t->type == SignalType::MIDI);
		menu->enable("track-midi-mode-tab", t->instrument.string_pitch.num > 0);
	}
	if (vt) {
		menu->check("track-midi-mode-linear", vt->midi_mode() == MidiMode::LINEAR);
		menu->check("track-midi-mode-classical", vt->midi_mode() == MidiMode::CLASSICAL);
		menu->check("track-midi-mode-tab", vt->midi_mode() == MidiMode::TAB);
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
		menu->enable("track-convert-stereo", t->channels == 1);
		menu->enable("track-convert-mono", t->channels == 2);
	}
	if (l) {
		menu->enable("layer-merge", t->layers.num > 1);
		menu->enable("layer-mark-dominant", t->layers.num > 1);// and sel.layers.num == 1);
		menu->enable("layer-add-dominant", t->layers.num > 1);// and sel.layers.num == 1);
		menu->enable("layer-make-track", t->layers.num > 1);
		//menu->enable("layer-delete", !l->is_main());
	}

	menu->check("play-loop", playback_loop);
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
	return scene_graph->mdp;
}

void AudioView::mdp_prepare(MouseDelayAction *a) {
	scene_graph->mdp_prepare(a);
}

void AudioView::mdp_run(MouseDelayAction *a) {
	scene_graph->mdp_run(a);
}

void AudioView::mdp_prepare(hui::Callback update) {
	scene_graph->mdp_prepare(update);
}
