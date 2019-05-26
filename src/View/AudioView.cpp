/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"

#include "AudioViewTrack.h"
#include "AudioViewLayer.h"
#include "Mode/ViewModeDefault.h"
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
#include "../Module/Audio/SongRenderer.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Audio/PeakMeter.h"
#include "../Module/SignalChain.h"
#include "../Stuff/PerformanceMonitor.h"
#include "../lib/math/math.h"
#include "../lib/threads/Thread.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Mutex.h"
#include "../Stream/AudioOutput.h"
#include "Helper/ScrollBar.h"
#include "Painter/BufferPainter.h"
#include "Painter/GridPainter.h"
#include "Painter/MidiPainter.h"
#include "SideBar/SideBar.h"

const float AudioView::FONT_SIZE = 10.0f;
const int AudioView::MAX_TRACK_CHANNEL_HEIGHT = 74;
const float AudioView::LINE_WIDTH = 1.0f;
const float AudioView::CORNER_RADIUS = 8.0f;
const int AudioView::SAMPLE_FRAME_HEIGHT = 20;
const int AudioView::TIME_SCALE_HEIGHT = 20;
const int AudioView::TRACK_HANDLE_WIDTH = 120;
const int AudioView::LAYER_HANDLE_WIDTH = 70;
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


class PeakThread : public Thread
{
public:
	AudioView *view;
	Song *song;
	int perf_channel;
	bool allow_running = true;
	PeakThread(AudioView *_view)
	{
		view = _view;
		song = view->song;
		perf_channel = PerformanceMonitor::create_channel("peak");
	}
	~PeakThread()
	{
		PerformanceMonitor::delete_channel(perf_channel);
	}
	void _cdecl on_run() override
	{
		try{
			update_song();
		}catch(...){
		}
	}

	void update_buffer(AudioBuffer &buf)
	{
		song->lock();
		if (!allow_running){
			song->unlock();
			throw "";
		}
		int n = buf._update_peaks_prepare();
		song->unlock();

		Thread::cancelation_point();

		if (!allow_running)
			throw "";

		for (int i=0; i<n; i++){
			if (buf._peaks_chunk_needs_update(i)){
				while (!song->try_lock()){
					Thread::cancelation_point();
					hui::Sleep(0.01f);
					if (!allow_running)
						throw "";
				}
				PerformanceMonitor::start_busy(perf_channel);
				buf._update_peaks_chunk(i);
				PerformanceMonitor::end_busy(perf_channel);
				song->unlock();
				Thread::cancelation_point();
			}
			if (!allow_running)
				throw "";
		}
	}

	void update_track(Track *t)
	{
		for (TrackLayer *l: t->layers)
			for (AudioBuffer &b: l->buffers)
				update_buffer(b);
	}

	void update_song()
	{
		for (Track *t: song->tracks)
			update_track(t);
		for (Sample *s: song->samples)
			update_buffer(s->buf);
	}
};

// make shadows thicker
Image *ExpandImageMask(Image *im, float d)
{
	Image *r = new Image(im->width, im->height, Black);
	for (int x=0; x<r->width; x++)
		for (int y=0; y<r->height; y++){
			float a = 0;
			for (int i=0; i<r->width; i++)
				for (int j=0; j<r->height; j++){
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



void ___draw_str_with_shadow(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_shadow)
{
	c->set_fill(false);
	c->set_line_width(3);
	c->set_color(col_shadow);
	c->draw_str(x, y, str);

	c->set_fill(true);
	c->set_line_width(1);
	c->set_color(col_text);
	c->draw_str(x, y, str);
}




void AudioView::MouseSelectionPlanner::start(int pos, int y)
{
	dist = 0;
	start_pos = pos;
	start_y = y;
}

bool AudioView::MouseSelectionPlanner::step()
{
	if (dist < 0)
		return false;
	auto e = hui::GetEvent();
	dist += fabs(e->dx) + fabs(e->dy);
	return selecting();
}

bool AudioView::MouseSelectionPlanner::selecting()
{
	return dist > min_move_to_select;
}

void AudioView::MouseSelectionPlanner::stop()
{
	dist = -1;
}

AudioView::AudioView(Session *_session, const string &_id) :
	cam(this)
{
	id = _id;
	session = _session;
	win = session->win;
	song = session->song;

	perf_channel = PerformanceMonitor::create_channel("view");

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

	cur_sample = nullptr;
	cur_vlayer = nullptr;
	_prev_cur_track = nullptr;

	playback_range_locked = false;

	metronome_overlay_vlayer = new AudioViewLayer(this, nullptr);
	dummy_vtrack = new AudioViewTrack(this, nullptr);
	dummy_vlayer = new AudioViewLayer(this, nullptr);

	selection_mode = SelectionMode::NONE;
	selection_snap_mode = SelectionSnapMode::NONE;
	hide_selection = false;
	song->subscribe(this, [=]{ on_song_update(); });

	// modes
	mode = nullptr;
	mode_default = new ViewModeDefault(this);
	mode_midi = new ViewModeMidi(this);
	mode_scale_bars = new ViewModeScaleBars(this);
	mode_scale_marker = new ViewModeScaleMarker(this);
	mode_curve = new ViewModeCurve(this);
	mode_capture = new ViewModeCapture(this);
	set_mode(mode_default);

	area = rect(0, 1024, 0, 768);
	song_area = area;
	enabled = true;
	scroll = new ScrollBar;

	buffer_painter = new BufferPainter(this);
	grid_painter = new GridPainter(this);
	midi_painter = new MidiPainter(this);

	msp.min_move_to_select = hui::Config.get_int("View.MouseMinMoveToSelect", 5);
	preview_sleep_time = hui::Config.get_int("PreviewSleepTime", 10);
	ScrollSpeed = 600;//hui::Config.getInt("View.ScrollSpeed", 600);
	ScrollSpeedFast = 6000;//hui::Config.getInt("View.ScrollSpeedFast", 6000);
	ZoomSpeed = hui::Config.get_float("View.ZoomSpeed", 0.1f);
	set_mouse_wheel_speed(hui::Config.get_float("View.MouseWheelSpeed", 1.0f));
	set_antialiasing(hui::Config.get_bool("View.Antialiasing", true));
	set_high_details(hui::Config.get_bool("View.HighDetails", true));
	hui::Config.set_int("View.MouseMinMoveToSelect", msp.min_move_to_select);
	hui::Config.set_int("View.ScrollSpeed", ScrollSpeed);
	hui::Config.set_int("View.ScrollSpeedFast", ScrollSpeedFast);
	hui::Config.set_float("View.ZoomSpeed", ZoomSpeed);

	images.speaker = LoadImage(tsunami->directory_static + "volume.tga");
	images.speaker_bg = ExpandImageMask(images.speaker, 1.5f);
	images.x = LoadImage(tsunami->directory_static + "x.tga");
	images.x_bg = ExpandImageMask(images.x, 1.5f);
	images.solo = LoadImage(tsunami->directory_static + "solo.tga");
	images.solo_bg = ExpandImageMask(images.solo, 1.5f);
	images.track_audio = LoadImage(tsunami->directory_static + "track-audio.tga");
	images.track_audio_bg = ExpandImageMask(images.track_audio, 1.5f);
	images.track_time = LoadImage(tsunami->directory_static + "track-time.tga");
	images.track_time_bg = ExpandImageMask(images.track_time, 1.5f);
	images.track_midi = LoadImage(tsunami->directory_static + "track-midi.tga");
	images.track_midi_bg = ExpandImageMask(images.track_midi, 1.5f);

	peak_thread = nullptr;
	draw_runner_id = -1;

	signal_chain = session->signal_chain;
	renderer = session->song_renderer;
	peak_meter = session->peak_meter;
	signal_chain->subscribe(this, [=]{ on_stream_tick(); }, Module::MESSAGE_TICK);
	signal_chain->subscribe(this, [=]{ on_stream_state_change(); }, Module::MESSAGE_STATE_CHANGE);

	mx = my = 0;
	msp.stop();

	hover_before_leave = hover = Selection();

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


	menu_song = hui::CreateResourceMenu("popup_song_menu");
	menu_track = hui::CreateResourceMenu("popup_track_menu");
	menu_layer = hui::CreateResourceMenu("popup_layer_menu");
	menu_time_track = hui::CreateResourceMenu("popup_time_track_menu");
	menu_sample = hui::CreateResourceMenu("popup_sample_menu");
	menu_marker = hui::CreateResourceMenu("popup_marker_menu");
	menu_bar = hui::CreateResourceMenu("popup_bar_menu");
	menu_buffer = hui::CreateResourceMenu("popup_buffer_menu");

	//ForceRedraw();
	update_menu();
}

AudioView::~AudioView()
{
	if (draw_runner_id >= 0)
		hui::CancelRunner(draw_runner_id);

	signal_chain->unsubscribe(this);
	song->unsubscribe(this);

	delete scroll;

	delete buffer_painter;
	delete grid_painter;
	delete midi_painter;

	delete mode_curve;
	delete mode_scale_bars;
	delete mode_scale_marker;
	delete mode_midi;
	delete mode_capture;
	delete mode_default;

	if (peak_thread)
		delete peak_thread;

	delete dummy_vtrack;
	delete dummy_vlayer;
	delete metronome_overlay_vlayer;

	delete images.speaker;
	delete images.speaker_bg;
	delete images.x;
	delete images.x_bg;
	delete images.solo;
	delete images.solo_bg;
	delete images.track_audio;
	delete images.track_audio_bg;
	delete images.track_midi;
	delete images.track_midi_bg;
	delete images.track_time;
	delete images.track_time_bg;

	PerformanceMonitor::delete_channel(perf_channel);
}

void AudioView::set_antialiasing(bool set)
{
	antialiasing = set;
	hui::Config.set_bool("View.Antialiasing", antialiasing);
	force_redraw();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void AudioView::set_high_details(bool set)
{
	high_details = set;
	detail_steps = high_details ? 1 : 3;
	hui::Config.set_bool("View.HighDetails", high_details);
	force_redraw();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void AudioView::set_mouse_wheel_speed(float speed)
{
	mouse_wheel_speed = speed;
	hui::Config.set_float("View.MouseWheelSpeed", mouse_wheel_speed);
	notify(MESSAGE_SETTINGS_CHANGE);
}

void AudioView::set_color_scheme(const string &name)
{
	hui::Config.set_str("View.ColorScheme", name);
	basic_colors = basic_schemes[0];
	for (auto &b: basic_schemes)
		if (b.name == name)
			basic_colors = b;

	colors = basic_colors.create(true);
	force_redraw();
}

string mode_name(ViewMode *m, AudioView *v)
{
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

void AudioView::set_mode(ViewMode *m)
{
	if (m == mode)
		return;
	if (mode){
		//msg_write("end mode " + mode_name(mode, this));
		mode->on_end();
	}
	mode = m;
	if (mode){
		//msg_write("start mode " + mode_name(mode, this));
		mode->on_start();
	}
	thm.dirty = true;
	force_redraw();
}

void AudioView::set_mouse()
{
	mx = hui::GetEvent()->mx;
	my = hui::GetEvent()->my;
}

int AudioView::mouse_over_sample(SampleRef *s)
{
	if ((mx >= s->area.x1) and (mx < s->area.x2)){
		int offset = cam.screen2sample(mx) - s->pos;
		if ((my >= s->area.y1) and (my < s->area.y1 + SAMPLE_FRAME_HEIGHT))
			return offset;
		if ((my >= s->area.y2 - SAMPLE_FRAME_HEIGHT) and (my < s->area.y2))
			return offset;
	}
	return -1;
}

void AudioView::selection_update_pos(Selection &s)
{
	s.pos = cam.screen2sample(mx);
}

void AudioView::update_selection()
{
	sel.range = sel.range.canonical();


	renderer->set_range(get_playback_selection(false));
	if (is_playback_active()){
		if (renderer->range().is_inside(playback_pos()))
			renderer->set_range(get_playback_selection(false));
		else{
			stop();
		}
	}
	force_redraw();

	notify(MESSAGE_SELECTION_CHANGE);
}


void AudioView::set_selection(const SongSelection &s)
{
	sel = s;
	update_selection();
}


bool AudioView::mouse_over_time(int pos)
{
	int ssx = cam.sample2screen(pos);
	return ((mx >= ssx - 5) and (mx <= ssx + 5));
}


Range AudioView::get_playback_selection(bool for_recording)
{
	if (playback_range_locked)
		return playback_lock_range;
	if (sel.range.empty()){
		if (for_recording)
			return Range(sel.range.start(), 0x70000000);
		int num = song->range_with_time().end() - sel.range.start();
		if (num <= 0)
			num = song->sample_rate; // 1 second
		return Range(sel.range.start(), num);
	}
	return sel.range;
}

void AudioView::set_selection_snap_mode(SelectionSnapMode mode)
{
	selection_snap_mode = mode;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void _update_find_min(int &new_pos, bool &found, float &dmin, int pos, int trial_pos)
{
	float dist = fabs(trial_pos - pos);
	if (dist < dmin){
		//msg_write(format("barrier:  %d  ->  %d", pos, b));
		new_pos = trial_pos;
		found = true;
		dmin = dist;
	}

}

void AudioView::snap_to_grid(int &pos)
{
	bool found = false;
	int new_pos;

	if (selection_snap_mode == SelectionSnapMode::NONE){
		float dmin = cam.dscreen2sample(SNAPPING_DIST);

		int sub_beats = 0;
		if (mode == mode_midi)
			sub_beats = mode_midi->sub_beat_partition;

		// time bar...
		auto beats = song->bars.get_beats(cam.range(), true, sub_beats>0, sub_beats);
		for (Beat &b: beats)
			_update_find_min(new_pos, found, dmin, pos, b.range.offset);

	}else if (selection_snap_mode == SelectionSnapMode::BAR){
		float dmin = cam.dscreen2sample(SNAPPING_DIST) * 1000;
		auto bars = song->bars.get_bars(cam.range());
		for (Bar *b: bars){
			_update_find_min(new_pos, found, dmin, pos, b->range().start());
			_update_find_min(new_pos, found, dmin, pos, b->range().end());
		}
	}else if (selection_snap_mode == SelectionSnapMode::PART){
		float dmin = cam.dscreen2sample(SNAPPING_DIST) * 1000;
		auto parts = song->get_parts();
		for (auto *p: parts){
			_update_find_min(new_pos, found, dmin, pos, p->range.start());
			_update_find_min(new_pos, found, dmin, pos, p->range.end());
		}
	}

	if (found)
		pos = new_pos;
}

void AudioView::on_mouse_move()
{
	set_mouse();
	mode->on_mouse_move();


	if (selection_mode != SelectionMode::NONE){

		snap_to_grid(hover.pos);
		hover.range.set_end(hover.pos);
		hover.y1 = my;
		if (win->get_key(hui::KEY_CONTROL))
			set_selection(sel_temp or mode->get_selection(hover.range));
		else
			set_selection(mode->get_selection(hover.range));
	}else{

		// selection:
		if (msp.step()){
			mode->start_selection();
			msp.stop();
		}
	}
	force_redraw();
}

void AudioView::on_left_button_down()
{
	set_mouse();
	mode->on_left_button_down();

	force_redraw();
	update_menu();
}

// extend to beats
void align_to_beats(Song *s, Range &r, int beat_partition)
{
	Array<Beat> beats = s->bars.get_beats(Range::ALL, true, beat_partition > 0, beat_partition);//audio->getRange());
	for (Beat &b: beats){
		/*for (int i=0; i<beat_partition; i++){
			Range sr = b.sub(i, beat_partition);
			if (sr.overlaps(sr))
				r = r or sr;
		}*/
		if (b.range.is_inside(r.start())){
			r.set_start(b.range.start());
		}
		if (b.range.is_inside(r.end())){
			r.set_end(b.range.end());
			break;
		}
	}
}


void AudioView::on_left_button_up()
{
	mode->on_left_button_up();

	force_redraw();
	update_menu();
}



void AudioView::on_middle_button_down()
{
}



void AudioView::on_middle_button_up()
{
}



void AudioView::on_right_button_down()
{
	mode->on_right_button_down();
}

void AudioView::on_right_button_up()
{
	mode->on_right_button_up();
}

void AudioView::on_mouse_leave()
{
	if (!hui::GetEvent()->lbut and !hui::GetEvent()->rbut){
		hover.clear();
		force_redraw();
	}
}



void AudioView::on_left_double_click()
{
	mode->on_left_double_click();
	force_redraw();
	update_menu();
}



void AudioView::on_command(const string & id)
{
}



void AudioView::on_key_down()
{
	int k = hui::GetEvent()->key_code;
	mode->on_key_down(k);
}

void AudioView::on_key_up()
{
	int k = hui::GetEvent()->key_code;
	mode->on_key_up(k);
}

void AudioView::on_mouse_wheel()
{
	mode->on_mouse_wheel();
}


void AudioView::force_redraw()
{
	win->redraw(id);
	//win->redraw_rect(id, rect(200, 300, 0, area.y2));
}

void AudioView::force_redraw_part(const rect &r)
{
	win->redraw_rect(id, r);
}

void AudioView::unselect_all_samples()
{
	sel.samples.clear();
}

void AudioView::update_buffer_zoom()
{
	prefered_buffer_layer = -1;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (cam.scale < 0.2f)
		for (int i=24-1;i>=0;i--){
			double _f = (double)(1 << (i + AudioBuffer::PEAK_OFFSET_EXP));
			if (_f > 2.0 / cam.scale){
				prefered_buffer_layer = i;
				buffer_zoom_factor = _f;
			}
		}
}

void _try_set_good_cur_layer(AudioView *v)
{
	for (auto *l: v->vlayer)
		if (l->layer->track == v->_prev_cur_track){
			//msg_write("  -> set by track");
			v->set_cur_layer(l);
			return;
		}
	if (v->vlayer.num > 0){
		//msg_write("  -> set first layer");
		v->set_cur_layer(v->vlayer[0]);
	}else{
		//msg_error("....no vlayers");
	}
}

void AudioView::check_consistency()
{
	// cur_vlayer = null
	if (!cur_vlayer and (vlayer.num > 0)){
		//msg_error("cur_vlayer = nil");
		//msg_write(msg_get_trace());
		//msg_write("  -> setting first");
		set_cur_layer(vlayer[0]);
	}

	// cur_vlayer illegal?
	if (cur_vlayer and (vlayer.find(cur_vlayer) < 0)){
		//msg_error("cur_vlayer illegal...");
		//msg_write(msg_get_trace());
		_try_set_good_cur_layer(this);
	}

	// illegal midi mode?
	for (auto *l: vlayer)
		if ((l->midi_mode == MidiMode::TAB) and (l->layer->track->instrument.string_pitch.num == 0))
			l->set_midi_mode(MidiMode::CLASSICAL);
}

void AudioView::implode_track(Track *t)
{
	for (auto *l: vlayer)
		if (l->layer->track == t){
			l->represents_imploded = l->layer->is_main();
			l->hidden = !l->layer->is_main();
		}
	get_track(t)->imploded = true;
	thm.dirty = true;
}

void AudioView::explode_track(Track *t)
{
	for (auto *l: vlayer)
		if (l->layer->track == t){
			l->represents_imploded = false;
			l->hidden = false;
		}
	get_track(t)->imploded = false;
	thm.dirty = true;
}

void AudioView::on_song_update()
{
	if (song->cur_message() == song->MESSAGE_NEW){
		set_cur_layer(nullptr);
		update_tracks();
		sel.range = Range(0, 0);
		sel.clear();
		for (Track *t: song->tracks){
			sel.add(t);
			for (TrackLayer *l: t->layers)
				sel.add(l);
		}
		check_consistency();
		optimize_view();
	}else if (song->cur_message() == song->MESSAGE_FINISHED_LOADING){
		for (auto *t: vtrack)
			if (t->track->layers.num > 2)
				implode_track(t->track);
		optimize_view();
		hui::RunLater(0.5f, [=]{ optimize_view(); });
	}else{
		if ((song->cur_message() == song->MESSAGE_ADD_TRACK) or (song->cur_message() == song->MESSAGE_DELETE_TRACK))
			update_tracks();
		if ((song->cur_message() == song->MESSAGE_ADD_LAYER) or (song->cur_message() == song->MESSAGE_DELETE_LAYER))
			update_tracks();
		if (song->cur_message() == song->MESSAGE_CHANGE_CHANNELS)
			update_tracks();
		force_redraw();
		update_menu();
	}

	if (song->cur_message() == MESSAGE_CHANGE)
		if (song->history_enabled())
			update_peaks();
}

void AudioView::on_stream_tick()
{
	cam.make_sample_visible(playback_pos(), session->sample_rate() * 2);
	force_redraw();
}

void AudioView::on_stream_state_change()
{
	force_redraw();
}

void AudioView::on_update()
{
	check_consistency();

	force_redraw();
}

void AudioView::update_peaks_now(AudioBuffer &buf)
{
	int n = buf._update_peaks_prepare();

	for (int i=0; i<n; i++)
		if (buf._peaks_chunk_needs_update(i))
			buf._update_peaks_chunk(i);
}

AudioViewTrack *AudioView::get_track(Track *track)
{
	for (auto t: vtrack){
		if (t->track == track)
			return t;
	}
	return dummy_vtrack;
}

AudioViewLayer *AudioView::get_layer(TrackLayer *layer)
{
	for (auto l: vlayer){
		if (l->layer == layer)
			return l;
	}

	msg_write("get_layer() failed for " + p2s(layer));
	msg_write(msg_get_trace());

	return dummy_vlayer;
}

void AudioView::update_tracks()
{
	bool changed = false;

	Array<AudioViewTrack*> vtrack2;
	Array<AudioViewLayer*> vlayer2;
	vtrack2.resize(song->tracks.num);
	vlayer2.resize(song->layers().num);

	foreachi(Track *t, song->tracks, ti){
		bool found = false;

		// find existing
		foreachi(AudioViewTrack *v, vtrack, vi)
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
			changed = true;
			sel.add(t);
		}
	}

	int li = 0;
	for (TrackLayer *l: song->layers()){

		bool found = false;

		// find existing
		foreachi(AudioViewLayer *v, vlayer, vi)
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
			changed = true;
			sel.add(l);
		}

		li ++;
	}

	// delete deleted
	for (AudioViewTrack *v: vtrack)
		if (v){
			delete v;
			changed = true;
		}
	for (AudioViewLayer *v: vlayer)
		if (v){
			delete v;
			changed = true;
		}
	vtrack = vtrack2;
	vlayer = vlayer2;
	thm.dirty = true;

	// guess where to create new tracks
	foreachi(AudioViewTrack *v, vtrack, i){
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
	foreachi(AudioViewLayer *v, vlayer, i){
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

	// TODO: detect order change

	/*if (changed)*/{
		check_consistency();
		notify(MESSAGE_VTRACK_CHANGE);
	}
}


rect AudioView::get_boxed_str_rect(Painter *c, float x, float y, const string &str)
{
	float w = c->get_str_width(str);
	return rect(x-CORNER_RADIUS, x + w + CORNER_RADIUS, y-CORNER_RADIUS, y + c->font_size + CORNER_RADIUS);
}

void AudioView::draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, int align)
{
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


void AudioView::draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area)
{
	c->set_font("", -1, true, false);
	float w = c->get_str_width(msg);
	float x = min(max(mx - 20.0f, area.x1 + 2.0f), area.x2 - w);
	float y = min(max(my + 20, area.y1 + 2.0f), area.y2 - FONT_SIZE - 5);
	draw_boxed_str(c, x, y, msg, colors.background, colors.text_soft1);
	c->set_font("", -1, false, false);
}

void AudioView::draw_cursor_hover(Painter *c, const string &msg)
{
	draw_cursor_hover(c, msg, mx, my, song_area);
}

void draw_message(Painter *c, AudioView *view, AudioView::Message &m)
{
	float xm = view->area.mx();
	float ym = view->area.my();
	float a = min(m.ttl*8, 1.0f);
	a = pow(a, 0.4f);
	color c1 = AudioView::colors.high_contrast_a;
	color c2 = AudioView::colors.high_contrast_b;
	c1.a = c2.a = a;
	c->set_font_size(AudioView::FONT_SIZE * m.size * a);
	view->draw_boxed_str(c, xm, ym, m.text, c1, c2, 0);
	c->set_font_size(AudioView::FONT_SIZE);
}

void AudioView::draw_time_line(Painter *c, int pos, int type, const color &col, bool show_time)
{
	float x = cam.sample2screen(pos);
	if ((x >= song_area.x1) and (x <= song_area.x2)){
		color cc = (type == (int)hover.type) ? colors.selection_boundary_hover : col;
		c->set_color(cc);
		c->set_line_width(2.0f);
		c->draw_line(x, area.y1, x, area.y2);
		if (show_time)
			draw_boxed_str(c,  x, (song_area.y1 + song_area.y2) / 2, song->get_time_str_long(pos), cc, colors.background);
		c->set_line_width(1.0f);
		if ((type == (int)Selection::Type::SELECTION_START) or (type == (int)Selection::Type::SELECTION_END)){
			c->draw_circle(x, area.y2, 8);
		}
	}
}

void draw_layer_separator(Painter *c, AudioViewLayer *l1, AudioViewLayer *l2, AudioView *view)
{
	float y = 0;
	bool sel_any = false;
	bool same_track = false;
	if (l1){
		y = l1->area.y2;
		sel_any |= view->sel.has(l1->layer);
	}
	if (l2){
		y = l2->area.y1;
		sel_any |= view->sel.has(l2->layer);
	}
	if (l1 and l2){
		same_track = (l1->layer->track == l2->layer->track);
	}
	if (same_track)
		c->set_line_dash({3,10}, 0);

	c->set_color(AudioView::colors.grid);
	c->draw_line(view->song_area.x1, y, view->song_area.x2, y);
	c->set_line_dash({}, 0);

}

void AudioView::draw_background(Painter *c)
{
	int yy = song_area.y1;
	if (vlayer.num > 0)
		yy = vlayer.back()->area.y2;

	// tracks
	for (auto *l: vlayer)
		if (l->on_screen())
			mode->draw_layer_background(c, l);

	// free space below tracks
	if (yy < song_area.y2){
		c->set_color(colors.background);
		rect rr = rect(song_area.x1, song_area.x2, yy, song_area.y2);
		GridColors g;
		g.bg = g.bg_sel = colors.background;
		g.fg = g.fg_sel = colors.grid;
		c->draw_rect(rr);
		grid_painter->set_context(rr, g);
		if (song->bars.num > 0)
			grid_painter->draw_bars(c, 0);
		else
			grid_painter->draw_time(c);
	}

	// lines between tracks
	AudioViewLayer *prev = nullptr;
	for (auto *l: vlayer){
		draw_layer_separator(c, prev, l, this);
		prev = l;
	}
	draw_layer_separator(c, prev, nullptr, this);
}

void AudioView::draw_time_scale(Painter *c)
{
	rect r = rect(clip.x1, clip.x2, area.y1, area.y1 + TIME_SCALE_HEIGHT);
	GridColors g;
	g.bg = colors.background_track;
	g.bg_sel = colors.background_track_selection;
	g.fg = g.fg_sel = colors.grid;

	grid_painter->set_context(r, g);
	grid_painter->draw_empty_background(c);
	grid_painter->draw_time(c);

	if (is_playback_active()){
		c->set_color(AudioView::colors.preview_marker_internal);
		float x0, x1;
		cam.range2screen(renderer->range(), x0, x1);
		c->draw_rect(x0, area.y1, x1 - x0, area.y1 + AudioView::TIME_SCALE_HEIGHT);
	}
	grid_painter->draw_time_numbers(c);

	// playback lock range
	if (playback_range_locked){
		c->set_color(AudioView::colors.preview_marker);
		float x0, x1;
		cam.range2screen_clip(playback_lock_range, area, x0, x1);
		c->draw_rect(x0, area.y1, x1-x0, 5);
	}

	// playback lock symbol
	playback_lock_button = rect(0,0,0,0);
	if (playback_range_locked or !sel.range.empty()){
		float x = cam.sample2screen(get_playback_selection(false).end());
		playback_lock_button = rect(x, x + 20, area.y1, area.y1 + TIME_SCALE_HEIGHT);

		c->set_color((hover.type == Selection::Type::PLAYBACK_LOCK) ? AudioView::colors.hover : AudioView::colors.preview_marker);
		c->set_font_size(FONT_SIZE * 0.7f);
		c->draw_str(playback_lock_button.x1 + 5, playback_lock_button.y1 + 3, playback_range_locked ? "🔒" : "🔓");
		c->set_font_size(FONT_SIZE);
	}
}

void AudioView::draw_selection(Painter *c)
{
	// time selection
	float x1, x2;
	cam.range2screen_clip(sel.range, song_area, x1, x2);
	//c->set_color(colors.selection_internal);
	//c->draw_rect(rect(x1, x2, area.y1, area.y1 + TIME_SCALE_HEIGHT));
	draw_time_line(c, sel.range.start(), (int)Selection::Type::SELECTION_START, colors.selection_boundary);
	draw_time_line(c, sel.range.end(), (int)Selection::Type::SELECTION_END, colors.selection_boundary);

	if (!hide_selection){
		if ((selection_mode == SelectionMode::TIME) or (selection_mode == SelectionMode::TRACK_RECT)){
			// drawn as background...

			/*c->setColor(colors.selection_internal);
			for (AudioViewLayer *l: vlayer)
				if (sel.has(l->layer))
					c->draw_rect(rect(sxx1, sxx2, l->area.y1, l->area.y2));*/
		}else if (selection_mode == SelectionMode::RECT){
			float x1, x2;
			cam.range2screen_clip(hover.range, clip, x1, x2);
			c->set_color(colors.selection_internal);
			c->set_fill(false);
			c->draw_rect(rect(x1, x2, hover.y0, hover.y1));
			c->set_fill(true);
			c->draw_rect(rect(x1, x2, hover.y0, hover.y1));
		}
	}

	// bar selection
	if (sel.bar_gap >= 0){
		x1 = cam.sample2screen(song->bar_offset(sel.bar_gap));
		x2 = x1;
		c->set_color(colors.text_soft1);
		c->set_line_width(2.5f);
		for (AudioViewLayer *t: vlayer)
			if (t->layer->type == SignalType::BEATS){
				float dy = t->area.height();
				c->draw_line(x2 + 5, t->area.y1, x2 + 2, t->area.y1 + dy*0.3f);
				c->draw_line(x2 + 2, t->area.y1 + dy*0.3f, x2 + 2, t->area.y2-dy*0.3f);
				c->draw_line(x2 + 2, t->area.y2-dy*0.3f, x2 + 5, t->area.y2);
				c->draw_line(x1 - 5, t->area.y1, x1 - 2, t->area.y1 + dy*0.3f);
				c->draw_line(x1 - 2, t->area.y1 + dy*0.3f, x1 - 2, t->area.y2-dy*0.3f);
				c->draw_line(x1 - 2, t->area.y2-dy*0.3f, x1 - 5, t->area.y2);
		}
		c->set_line_width(1.0f);
	}
	if (hover.type == Selection::Type::BAR_GAP){
		x1 = cam.sample2screen(song->bar_offset(hover.index));
		x2 = x1;
		c->set_color(colors.hover);
		c->set_line_width(2.5f);
		for (AudioViewLayer *t: vlayer)
			if (t->layer->type == SignalType::BEATS){
				float dy = t->area.height();
				c->draw_line(x2 + 5, t->area.y1, x2 + 2, t->area.y1 + dy*0.3f);
				c->draw_line(x2 + 2, t->area.y1 + dy*0.3f, x2 + 2, t->area.y2-dy*0.3f);
				c->draw_line(x2 + 2, t->area.y2-dy*0.3f, x2 + 5, t->area.y2);
				c->draw_line(x1 - 5, t->area.y1, x1 - 2, t->area.y1 + dy*0.3f);
				c->draw_line(x1 - 2, t->area.y1 + dy*0.3f, x1 - 2, t->area.y2-dy*0.3f);
				c->draw_line(x1 - 2, t->area.y2-dy*0.3f, x1 - 5, t->area.y2);
		}
		c->set_line_width(1.0f);
	}
}

bool need_metro_overlay(Song *song, AudioView *view)
{
	Track *tt = song->time_track();
	if (!tt)
		return false;
	auto *vv = view->get_layer(tt->layers[0]);
	if (!vv)
		return false;
	return !vv->on_screen();
}

void AudioView::draw_song(Painter *c)
{
	bool scroll_bar_needed = scroll->page_size < scroll->content_size;
	bool slow_repeat = false;
	song_area = area;
	song_area.y1 += TIME_SCALE_HEIGHT;
	if (scroll_bar_needed)
		song_area.x1 += SCROLLBAR_WIDTH;
	bool animating = thm.update(this, song, song_area);

	c->set_antialiasing(false);

	cam.update(0.1f);

	update_buffer_zoom();

	draw_time_scale(c);

	c->set_clip(song_area);

	draw_background(c);

	// tracks
	for (AudioViewLayer *t: vlayer)
		if (t->on_screen())
			t->draw(c);
	for (AudioViewTrack *t: vtrack)
		//if (t->on_screen())
			t->draw(c);

	if (need_metro_overlay(song, this)){
		metronome_overlay_vlayer->layer = song->time_track()->layers[0];
		metronome_overlay_vlayer->area = rect(song_area.x1, song_area.x2, song_area.y1, song_area.y1 + this->TIME_SCALE_HEIGHT*2);
		metronome_overlay_vlayer->draw(c);
	}

	c->set_clip(clip);


	// selection
	draw_selection(c);

	if (scroll_bar_needed){
		scroll->set_area(rect(area.x1, area.x1 + SCROLLBAR_WIDTH, song_area.y1, song_area.y2));
		scroll->draw(c, hover.type == hover.Type::SCROLLBAR_GLOBAL);
	}else{
		scroll->set_area(rect(-1,0,-1,0));
	}


	// playing/capturing position
	if (is_playback_active())
		draw_time_line(c, playback_pos(), (int)Selection::Type::PLAYBACK, colors.preview_marker, true);

	mode->draw_post(c);

	if (message.ttl > 0){
		draw_message(c, this, message);
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

void AudioView::on_draw(Painter *c)
{
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

void AudioView::optimize_view()
{
	if (area.x2 <= 0)
		area.x2 = 1024;

	Range r = song->range_with_time();

	if (r.length == 0)
		r.length = 10 * song->sample_rate;

	cam.show(r);
}

void AudioView::update_menu()
{
	// view
	win->check("view_midi_default", midi_view_mode == MidiMode::LINEAR);
	win->check("view_midi_tab", midi_view_mode == MidiMode::TAB);
	win->check("view_midi_score", midi_view_mode == MidiMode::CLASSICAL);
	win->enable("view_samples", false);
}

void AudioView::update_peaks()
{
	//msg_write("-------------------- view update peaks");
	if (peak_thread){
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

void AudioView::set_midi_view_mode(MidiMode mode)
{
	midi_view_mode = mode;
	hui::Config.set_int("View.MidiMode", (int)midi_view_mode);
	for (auto *l: vlayer)
		l->set_midi_mode(mode);
	//forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	update_menu();
}

void AudioView::zoom_in()
{
	cam.zoom(2.0f, mx);
}

void AudioView::zoom_out()
{
	cam.zoom(0.5f, mx);
}

void AudioView::select_all()
{
	set_selection(mode->get_selection_for_range(song->range()));
}

void AudioView::select_none()
{
	// select all/none
	set_selection(SongSelection());
	set_cur_sample(nullptr);
}

inline void test_range(const Range &r, Range &sel, bool &update)
{
	if (r.is_more_inside(sel.start())){
		sel.set_start(r.start());
		update = true;
	}
	if (r.is_more_inside(sel.end())){
		sel.set_end(r.end());
		update = true;
	}
}

void AudioView::select_expand()
{
	Range r = sel.range;
	bool update = true;
	while(update){
		update = false;
		for (Track *t: song->tracks){
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

			for (TrackLayer *l: t->layers){
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



void AudioView::select_sample(SampleRef *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		sel.set(s, !sel.has(s));
	}else{
		if (!sel.has(s))
			unselect_all_samples();

		// select this sub
		sel.add(s);
	}
}

void AudioView::set_cur_sample(SampleRef *s)
{
	if (cur_sample == s)
		return;
	cur_sample = s;
	force_redraw();
	notify(MESSAGE_CUR_SAMPLE_CHANGE);
}

Track *AudioView::cur_track()
{
	if (!cur_vlayer)
		return nullptr;
	if (!cur_vlayer->layer)
		return nullptr;
	//if (song->layers().find(cur_vlayer->layer) < 0)
	//	return nullptr;
	return cur_vlayer->layer->track;
}

TrackLayer *AudioView::cur_layer()
{
	if (!cur_vlayer)
		return nullptr;
	//if (song->layers().find(cur_vlayer->layer) < 0)
	//	return nullptr;
	return cur_vlayer->layer;
}

// l must be from vlayer[] or null
void AudioView::set_cur_layer(AudioViewLayer *l)
{
	auto *prev_track = _prev_cur_track;
	auto *prev_vlayer = cur_vlayer;
	cur_vlayer = l;
	_prev_cur_track = cur_track();

	if (!cur_vlayer){
		//msg_write("   ...setting cur_vlayer = nil");
		//msg_write(msg_get_trace());
	}

	if (cur_vlayer != prev_vlayer){
		mode->on_cur_layer_change();
		force_redraw();
		notify(MESSAGE_CUR_LAYER_CHANGE);
	}
	if (cur_track() != prev_track)
		notify(MESSAGE_CUR_TRACK_CHANGE);
}



// unused?!?
void AudioView::enable(bool _enabled)
{
	if (enabled and !_enabled)
		song->unsubscribe(this);
	else if (!enabled and _enabled)
		song->subscribe(this, [=]{ on_song_update(); });
	enabled = _enabled;
}


void AudioView::play()
{
	if (signal_chain->is_paused()){
		signal_chain->start();
		return;
	}

	prepare_playback(get_playback_selection(false), true);
	signal_chain->start();
}

void AudioView::prepare_playback(const Range &range, bool allow_loop)
{
	if (signal_chain->is_playback_active())
		stop();

	renderer->prepare(range, allow_loop);
	renderer->allow_tracks(get_playable_tracks());
	renderer->allow_layers(get_playable_layers());

	signal_chain->command(ModuleCommand::PREPARE_START, 0);
}

void AudioView::stop()
{
	signal_chain->stop_hard();
}

void AudioView::pause(bool _pause)
{
	if (_pause)
		signal_chain->stop();
	else
		signal_chain->start();
}

bool AudioView::is_playback_active()
{
	return signal_chain->is_playback_active();
}

bool AudioView::is_paused()
{
	return signal_chain->is_paused();
}

int AudioView::playback_pos()
{
	return signal_chain->get_pos();
}

bool AudioView::has_any_solo_track()
{
	for (auto *t: vtrack)
		if (t->solo)
			return true;
	return false;
}

Set<const Track*> AudioView::get_playable_tracks()
{
	Set<const Track*> tracks;
	Set<Track*> prevented = mode->prevent_playback();
	bool any_solo = has_any_solo_track();
	for (auto *t: vtrack)
		if (!t->track->muted and (t->solo or !any_solo) and !prevented.contains(t->track))
			tracks.add(t->track);
	return tracks;
}

bool AudioView::has_any_solo_layer(Track *t)
{
	for (auto *v: vlayer)
		if (v->solo and (v->layer->track == t))
			return true;
	return false;
}

Set<const TrackLayer*> AudioView::get_playable_layers()
{
	auto tracks = get_playable_tracks();

	Set<const TrackLayer*> layers;
	for (Track* t: song->tracks){
		if (!tracks.contains(t))
			continue;
		bool any_solo = has_any_solo_layer(t);
		for (auto *l: vlayer)
			if ((l->layer->track == t) and (l->solo or !any_solo))
				layers.add(l->layer);
	}
	return layers;
}

void AudioView::set_message(const string& text, float size)
{
	message.text = text;
	message.ttl = 0.8f;
	message.size = size;
	force_redraw();
}
