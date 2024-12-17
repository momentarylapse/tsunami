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
#include "graph/Cursor.h"
#include "graph/TimeScale.h"
#include "../MouseDelayPlanner.h"
#include "../helper/graph/SceneGraph.h"
#include "../helper/graph/ScrollBar.h"
#include "../helper/Drawing.h"
#include "../helper/PeakThread.h"
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
#include "../sidebar/TrackConsole.h"
#include "../mainview/MainView.h"
#include "../TsunamiWindow.h"
#include "../ColorScheme.h"
#include "../../data/base.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../data/rhythm/Beat.h"
#include "../../data/SampleRef.h"
#include "../../lib/image/Painter.h"
#include "../../lib/hui/config.h"
#include "../../lib/hui/Menu.h"
#include "../../lib/hui/Resource.h"
#include "../../lib/hui/language.h"
#include "../../lib/os/msg.h"
#include "../../module/audio/SongRenderer.h"
#include "../../plugins/TsunamiPlugin.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../Session.h"
#include "../../Playback.h"
#include "../../EditModes.h"
#include "../../Tsunami.h"

namespace tsunami {

const int AudioView::SNAPPING_DIST = 8;

extern Synthesizer* CreateSynthesizer(Session *session, const string &name);

class AddTrackButton : public ::scenegraph::Node {
	AudioView *view;
public:
	explicit AddTrackButton(AudioView *_view) : scenegraph::Node(theme.TRACK_HANDLE_HEIGHT, theme.TRACK_HANDLE_HEIGHT) {
		align.dz = 70;
		// size overwritten by AudioView.update_scene_graph()!
		/*align.horizontal = AlignData::Mode::LEFT;
		align.vertical = AlignData::Mode::TOP;*/
		view = _view;
	}
	void on_draw(Painter *p) override {
		color bg = theme.background_overlay;
		color fg = theme.text_soft3;
		float radius = area.width() * 0.48f;
		if (is_cur_hover()) {
			bg = theme.hoverify(bg);
			fg = theme.text;
			radius = area.width() * 0.60f;
		}
		p->set_color(bg);
		p->draw_circle(area.center(), radius);
		p->set_color(fg);
		p->set_font("", theme.FONT_SIZE_BIG, true, false);
		draw_str_centered(p, area.center(), "+");
	}
	bool on_left_button_down(const vec2 &m) override {
		view->win->on_add_new_track(SignalType::Audio);
		return true;
	}
	string get_tip() const override {
		return _("add new track");
	}
};

class SongOverlay : public scenegraph::Node {
	AudioView *view;
public:
	SongOverlay(AudioView *v) {
		view = v;
	}
	void on_draw(Painter *p) override {
		p->set_font_size(theme.FONT_SIZE);
		p->set_line_width(theme.LINE_WIDTH);

		// mode/plugin overlays
		view->mode->draw_post(p);
		for (auto *plugin: weak(view->session->plugins))
			plugin->on_draw_post(p);

		// hints from current mode (full line at bottom)
		string tip = view->mode->get_tip();
		if (tip.num > 0)
			draw_boxed_str(p, {view->song_area().center().x, area.y2 - 50}, tip, theme.text_soft1, theme.background_track_selected, TextAlign::Center);

		//static int frame = 0;
		//c->draw_str(100, 100, i2s(frame++));
	}
};

AudioView::AudioView(Session *_session) :
		in_solo_changed{this, [this] {
			update_playback_layers();
			out_solo_changed();
		}},
		in_redraw(this, [this] {
			force_redraw();
		})
{
	session = _session;
	win = session->win.get();
	song = session->song.get();
	_optimize_view_requested = false;

	set_perf_name("audioview");

	midi_view_mode = (MidiMode)hui::config.get_int("View.MidiMode", (int)MidiMode::Classical);

	playback_range_locked = false;

	selection_mode = SelectionMode::None;
	selection_snap_mode = SelectionSnapMode::None;
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

	/*scene_graph->set_callback_set_current([this] {
		if (!selecting_or())
			set_current(scene_graph->cur_selection);
	});*/
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

	dummy_track = new Track(song, SignalType::Audio, CreateSynthesizer(session, ""));
	dummy_layer = new TrackLayer(dummy_track.get());
	dummy_vtrack = new AudioViewTrack(this, dummy_track.get());
	dummy_vlayer = new AudioViewLayer(this, dummy_layer.get());

	metronome_overlay_vlayer = new AudioViewLayer(this, nullptr);
	metronome_overlay_vlayer->layer = dummy_layer.get(); // don't subscribe...
	metronome_overlay_vlayer->align.dz = 80;
	metronome_overlay_vlayer->set_hidden(true);


	add_child(vbox);

	vbox->add_child(time_scale);
	vbox->add_child(hbox);
	vbox->add_child(scroll_bar_time);

	hbox->add_child(background);
	hbox->add_child(scroll_bar_y);

	add_child(cursor_start);
	add_child(cursor_end);
	add_child(selection_marker);
	add_child(metronome_overlay_vlayer);
	add_child(new SongOverlay(this));


	peak_database = new PeakDatabase();
	peak_database->out_changed >> in_redraw;

	buffer_painter = new BufferPainter(this);
	grid_painter = new GridPainter(song, &cam, &sel, theme);
	grid_painter->get_hover_bar = [this] { return hover().bar; };

	preview_sleep_time = hui::config.get_int("PreviewSleepTime", 10);
	zoom_speed = hui::config.get_float("View.ZoomSpeed", 0.15f);
	//if (zoom_speed == 0.1f)
		zoom_speed = 0.15f; // fix for legacy config
	scroll_speed = hui::config.get_float("View.ScrollSpeed", 100.0f);
	scroll_speed_fast = hui::config.get_float("View.ScrollSpeedFast", 1000.0f);
	set_antialiasing(hui::config.get_bool("View.Antialiasing", true));
	set_high_details(hui::config.get_bool("View.HighDetails", true));
	hui::config.set_float("View.ZoomSpeed", zoom_speed);

	images.speaker = Image::load(Tsunami::directory_static | "icons/volume.png");
	images.solo = Image::load(Tsunami::directory_static | "icons/solo.png");
	images.config = Image::load(Tsunami::directory_static | "icons/wrench.png");
	images.x = Image::load(Tsunami::directory_static | "icons/x.png");
	images.track_audio = Image::load(Tsunami::directory_static | "icons/track-audio.png");
	images.track_time = Image::load(Tsunami::directory_static | "icons/track-time.png");
	images.track_midi = Image::load(Tsunami::directory_static | "icons/track-midi.png");
	images.track_group = Image::load(Tsunami::directory_static | "icons/track-group.png");

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

	add_track_button = new AddTrackButton(this);
	add_child(add_track_button);

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
}

AudioView::~AudioView() {
	enabled = false;
	if (draw_runner_id >= 0)
		hui::cancel_runner(draw_runner_id);

	session->playback->unsubscribe(this);
	song->unsubscribe(this);

	// workaround... since we didn't subsribe
	metronome_overlay_vlayer->layer = nullptr;
}

void* AudioView::mvn_data() const {
	return song;
}

string AudioView::mvn_description() const {
	return "song: " + song->filename.basename();
}

void AudioView::mvn_on_enter() {
	// restore side bar state
	set_mode(current_mode_name);
}

void AudioView::play() {
	if (mode == mode_capture)
		return;
	session->playback->play();
}

void AudioView::stop() {
	if (mode == mode_capture) {
		set_mode(EditMode::Default);
	} else {
		session->playback->stop();
	}
}

void AudioView::pause() {
	if (mode == mode_capture)
		return;
	session->playback->pause(true);
}

bool AudioView::is_playback_active() {
	return session->playback->is_active();
}

bool AudioView::is_paused() {
	return session->playback->is_paused();
}

bool AudioView::mvn_can_play() {
	return mode != mode_capture;
}

bool AudioView::mvn_can_stop() {
	return is_playback_active() or (mode == mode_capture);
}

bool AudioView::mvn_can_pause() {
	return is_playback_active() and !is_paused();
}

bool AudioView::mvn_can_record() {
	return (mode != mode_capture);
}
bool AudioView::mvn_can_edit() {
	return true;
}
bool AudioView::is_editing() {
	return mode == mode_edit;
}
bool AudioView::mvn_show_undo() {
	return true;
};

bool AudioView::is_recording() {
	return mode == mode_capture;
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

void AudioView::_set_mode(ViewMode *m) {
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

void AudioView::set_mode(const string& m) {
	current_mode_name = m;

	if (m == EditMode::Default) {
		_set_mode(mode_default);
		win->side_bar->_hide();
	} else if (m == EditMode::Capture) {
		_set_mode(mode_capture);
		mode->set_side_bar(SideBar::Index::CaptureConsole);
	} else if (m == EditMode::EditTrack) {
		_set_mode(mode_edit);
		//mode->set_side_bar(SideBar::Index::T);
	} else if (m == EditMode::ScaleBars) {
		_set_mode(mode_edit_bars);
		mode_edit_bars->set_edit_mode(ViewModeEditBars::EditMode::Rubber);
	} else if (m == EditMode::ScaleMarker) {
		_set_mode(mode_scale_marker);
	} else if (m == EditMode::Curves) {
		_set_mode(mode_curve);
		mode->set_side_bar(SideBar::Index::CurveConsole);
	} else if (m == EditMode::DefaultTrack) {
		_set_mode(mode_default);
		win->side_bar->open(SideBar::Index::TrackConsole);
	} else if (m == EditMode::DefaultTrackFx) {
		_set_mode(mode_default);
		win->side_bar->open(SideBar::Index::EffectsConsole);
		//win->side_bar->track_console->set_mode(TrackConsole::Mode::FX);
	} else if (m == EditMode::DefaultTrackMidiFx) {
		win->side_bar->open(SideBar::Index::EffectsConsole);
		//win->side_bar->open(SideBar::TRACK_CONSOLE);
		//win->side_bar->track_console->set_mode(TrackConsole::Mode::MIDI_FX);
	} else if (m == EditMode::DefaultTrackSynth) {
		_set_mode(mode_default);
		win->side_bar->open(SideBar::Index::TrackConsole);
		win->side_bar->track_console->set_mode(TrackConsole::Mode::Synth);
	} else if (m == EditMode::DefaultSong) {
		_set_mode(mode_default);
		win->side_bar->open(SideBar::Index::SongConsole);
	} else if (m == EditMode::DefaultSamples) {
		_set_mode(mode_default);
		win->side_bar->open(SideBar::Index::SampleConsole);
	} else if (m == EditMode::DefaultFx) {
		_set_mode(mode_default);
		//win->bottom_bar->open(BottomBar::Index::MixingConsole);
		//win->bottom_bar->mixing_console->show_fx(cur_track());
	} else if (m == EditMode::DefaultMidiFx) {
		_set_mode(mode_default);
		//win->bottom_bar->open(BottomBar::Index::MixingConsole);
		//win->bottom_bar->mixing_console->show_fx(cur_track());
	} else if (m == EditMode::DefaultSampleRef) {
		_set_mode(mode_default);
		win->side_bar->open(SideBar::Index::SamplerefConsole);
	}
}

bool AudioView::in_mode(const string& m) const {
	return current_mode_name == m;
}

int AudioView::mouse_over_sample(SampleRef *s, const vec2& m) {
	if ((m.x >= s->area.x1) and (m.x < s->area.x2)) {
		int offset = cam.screen2sample(m.x) - s->pos;
		if ((m.y >= s->area.y1) and (m.y < s->area.y1 + theme.SAMPLE_FRAME_HEIGHT))
			return offset;
		if ((m.y >= s->area.y2 - theme.SAMPLE_FRAME_HEIGHT) and (m.y < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::selection_update_pos(HoverData &s, const vec2& m) {
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


bool AudioView::mouse_over_time(int pos, const vec2& m) {
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

	if (selection_snap_mode == SelectionSnapMode::None) {
		float dmin = cam.dscreen2sample(SNAPPING_DIST);

		base::optional<int> sub_beats;
		if (mode == mode_edit and mode_edit->mode == mode_edit_midi)
			sub_beats = mode_edit_midi->sub_beat_partition;

		// time bar...
		auto beats = song->bars.get_beats(cam.range(), true, sub_beats);
		for (Beat &b: beats)
			_update_find_min(new_pos, found, dmin, pos, b.range.offset);

	} else if (selection_snap_mode == SelectionSnapMode::Bar) {
		float dmin = cam.dscreen2sample(SNAPPING_DIST) * 1000;
		auto bars = song->bars.get_bars(cam.range());
		for (Bar *b: bars) {
			_update_find_min(new_pos, found, dmin, pos, b->range().start());
			_update_find_min(new_pos, found, dmin, pos, b->range().end());
		}
	} else if (selection_snap_mode == SelectionSnapMode::Part) {
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

bool AudioView::on_mouse_move(const vec2 &m) {
	mode->on_mouse_move(m);

	force_redraw();
	return true;
}

bool AudioView::on_left_button_down(const vec2 &m) {
	force_redraw();
	update_menu();
	return true;
}

// extend to beats
void align_to_beats(Song *s, Range &r, int beat_partition) {
	auto beats = s->bars.get_beats(Range::ALL, true, beat_partition);//audio->getRange());
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


bool AudioView::on_left_button_up(const vec2 &m) {
	force_redraw();
	update_menu();
	return true;
}



bool AudioView::on_right_button_up(const vec2 &m) {
	mode->on_right_button_up(m);
	return true;
}

void AudioView::on_mouse_leave() {
	// TODO check if necessary
	if (!hui::get_event()->lbut and !hui::get_event()->rbut) {
		hover().clear();
		force_redraw();
	}
}



bool AudioView::on_left_double_click(const vec2 &m) {
	mode->on_left_double_click(m);
	force_redraw();
	update_menu();
	return true;
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
		cam.move(scroll_speed_fast * dt / (float)cam.pixels_per_sample);
	if (id == "cam-move-left")
		cam.move(- scroll_speed_fast * dt / (float)cam.pixels_per_sample);

	// vertical zoom
	if (id == "vertical-zoom-in")
		zoom_y(cam.scale_y * 1.2f);
	if (id == "vertical-zoom-out")
		zoom_y(cam.scale_y / 1.2f);
	if (id == "vertical-zoom-default")
		zoom_y(1.0f);

	mode->on_command(id);
}



bool AudioView::on_key_down(int k) {
	if (k == hui::KEY_ESCAPE) {
		if (mdp()->acting()) {
			mdp()->cancel();
		} else {
			win->side_bar->test_allow_close().then([this] (bool allow) {
				if (allow)
					session->set_mode(EditMode::Default);
			});
		}
	}
	if ((k == hui::KEY_LSHIFT) or (k == hui::KEY_RSHIFT))
		force_redraw();

	mode->on_key_down(k);
	return true;
}

bool AudioView::on_key_up(int k) {
	if ((k == hui::KEY_LSHIFT) or (k == hui::KEY_RSHIFT))
		force_redraw();

	mode->on_key_up(k);
	return true;
}

bool AudioView::on_mouse_wheel(const vec2 &d) {
	mode->on_mouse_wheel(d);
	return true;
}

bool AudioView::on_gesture(const string &id, const vec2 &m, const vec2 &param) {
	mode->on_gesture(id, m, param);
	return true;
}

void AudioView::force_redraw() {
	request_redraw();
	//win->redraw(id);
	//win->redraw_rect(id, rect(200, 300, 0, area.y2));
}

void AudioView::force_redraw_part(const rect &r) {
	request_redraw();
	//win->redraw_rect(id, r);
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
		if ((t->midi_mode() == MidiMode::Tab) and (t->track->instrument.string_pitch.num == 0))
			t->set_midi_mode(MidiMode::Classical);
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


void AudioView::on_stream_state_change() {
	// TODO send event instead
	win->main_view->update_onscreen_displays();
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

	if (vtracks.num > 0) {
		const auto a = vtracks.back()->area;
		add_track_button->area = {a.x1, a.x1 + theme.TRACK_HANDLE_HEIGHT, a.y2, a.y2 + theme.TRACK_HANDLE_HEIGHT};
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
	::draw_cursor_hover(c, msg, cursor(), song_area());
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

void AudioView::prepare_draw_song() {
	bool animating = update_scene_graph();

	cam.area = song_area();
	if (_optimize_view_requested)
		perform_optimize_view();
	cam.update(0.1f);

	update_buffer_zoom();

	peak_database->iterate(session->sample_rate());

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
		prepare_draw_song();

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
			if ((t->track->type == SignalType::Midi) and (t->midi_mode_wanted != midi_view_mode))
				common_midi_mode = MidiMode::DontCare;
	// view
	win->check("view-midi-linear", common_midi_mode == MidiMode::Linear);
	win->check("view-midi-tab", common_midi_mode == MidiMode::Tab);
	win->check("view-midi-classical", common_midi_mode == MidiMode::Classical);
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
	cam.zoom(exp( zoom_speed * 0.8f), cursor().x);
}

void AudioView::zoom_out() {
	cam.zoom(exp(-zoom_speed * 0.8f), cursor().x);
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

			if (t->type == SignalType::Beats)
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
	return cur_vlayer()->layer->track;
}

AudioViewTrack *AudioView::cur_vtrack() {
	for (auto *t: vtracks)
		if (t->track == cur_track())
			return t;
	return dummy_vtrack.get();
}

TrackLayer *AudioView::cur_layer() {
	return cur_vlayer()->layer;
}

AudioViewLayer *AudioView::cur_vlayer() {
	if (!cur_selection.vlayer)
		return dummy_vlayer.get();
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
			hover().type = HoverData::Type::PlaybackCursor;
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

int AudioView::get_mouse_pos(const vec2 &m) {
	return cam.screen2sample(m.x);
}

int AudioView::get_mouse_pos_snap(const vec2 &m) {
	int pos = get_mouse_pos(m);
	snap_to_grid(pos);
	return pos;
}

HoverData AudioView::hover_time(const vec2 &m) {
	HoverData s;
	s.pos = get_mouse_pos(m);
	s.pos_snap = get_mouse_pos_snap(m);
	s.range = Range(s.pos, 0);
	s.y0 = s.y1 = m.y;
	s.type = HoverData::Type::Time;
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

	auto l = vl->layer;
	auto t = vt->track;

	// midi mode
	menu->enable("menu-midi-mode", t->type == SignalType::Midi);
	menu->enable("track-midi-mode-tab", t->instrument.string_pitch.num > 0);
	menu->enable("menu-audio-mode", t->type == SignalType::Audio);
	menu->check("track-midi-mode-linear", vt->midi_mode() == MidiMode::Linear);
	menu->check("track-midi-mode-classical", vt->midi_mode() == MidiMode::Classical);
	menu->check("track-midi-mode-tab", vt->midi_mode() == MidiMode::Tab);
	menu->check("track-audio-mode-peaks", vt->audio_mode == AudioViewMode::Peaks);
	menu->check("track-audio-mode-spectrum", vt->audio_mode == AudioViewMode::Spectrum);
	
	// mute/solo
	menu->check("track-muted", t->muted);
	menu->check("track-solo", vt->solo);
	menu->check("layer-muted", l->muted);
	menu->check("layer-solo", vl->solo);
	menu->check("track-explode", !vt->imploded);
	menu->enable("track-explode", t->layers.num > 1);

	menu->enable("track-edit-midi", t->type == SignalType::Midi);
	menu->enable("track_add_marker", true);//hover->type == Selection::Type::LAYER);

	// convert
	menu->enable("menu-convert", t->type == SignalType::Audio);
	//menu->enable("track-convert-mono", t->type == SignalType::Audio);
	//menu->enable("track-convert-stereo", t->type == SignalType::Audio);
	menu->enable("track-convert-stereo", t->channels == 1 and t->type == SignalType::Audio);
	menu->enable("track-convert-mono", t->channels == 2 and t->type == SignalType::Audio);
	// view
	menu->enable("menu-audio-mode", t->type == SignalType::Audio);
	menu->enable("track-audio-mode-peaks", t->type == SignalType::Audio);
	menu->enable("track-audio-mode-spectrum", t->type == SignalType::Audio);
	menu->enable("menu-midi-mode", t->type == SignalType::Midi);
	menu->enable("track-midi-mode-linear", t->type == SignalType::Midi);
	menu->enable("track-midi-mode-tab", t->type == SignalType::Midi);
	menu->enable("track-midi-mode-classical", t->type == SignalType::Midi);
	menu->enable("layer-merge", t->layers.num > 1);
	menu->enable("layer-mark-dominant", t->layers.num > 1);// and sel.layers.num == 1);
	menu->enable("layer-add-dominant", t->layers.num > 1);// and sel.layers.num == 1);
	menu->enable("layer-make-track", t->layers.num > 1);
	//menu->enable("layer-delete", !l->is_main());

	// group
	bool any_in_group = false;
	for (auto t: sel.tracks())
		if (t->send_target)
			any_in_group = true;
	menu->enable("track-ungroup", any_in_group);

	menu->check("play-loop", looping());
	menu->check("playback-range-lock", playback_range_locked);
}

void AudioView::open_popup(hui::Menu* menu) {
	prepare_menu(menu);
	hover_before_leave = hover();
	menu->open_popup(win);
}

HoverData &AudioView::hover() {
	return graph()->hover;
}

scenegraph::MouseDelayPlanner *AudioView::mdp() {
	return graph()->mdp.get();
}

void AudioView::mdp_prepare(scenegraph::MouseDelayAction *a, const vec2 &m) {
	graph()->_mdp_prepare(a, m);
}

void AudioView::mdp_run(scenegraph::MouseDelayAction *a, const vec2 &m) {
	graph()->_mdp_run(a, m);
}

void AudioView::mdp_prepare(std::function<void(const vec2&)> update, const vec2 &m) {
	graph()->_mdp_prepare(update, m);
}

bool view_has_focus(AudioView *view) {
	return view->win->is_active("area"); //view->id);
}

}
