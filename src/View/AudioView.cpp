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
#include "../Session.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Data/base.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/Song.h"
#include "../Data/Sample.h"
#include "../Data/Rhythm/Bar.h"
#include "../Data/Rhythm/BarCollection.h"
#include "../Data/Rhythm/Beat.h"
#include "../Data/SampleRef.h"
#include "../Device/OutputStream.h"
#include "../Module/Audio/SongRenderer.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Audio/PeakMeter.h"
#include "../Module/SignalChain.h"
#include "../Stuff/PerformanceMonitor.h"
#include "../lib/math/math.h"
#include "../lib/threads/Thread.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Mutex.h"
#include "Helper/ScrollBar.h"

string i2s_small(int i); // -> MidiData.cpp
color col_inter(const color a, const color &b, float t); // -> ColorScheme.cpp

const int AudioView::FONT_SIZE = 10;
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
const string AudioView::MESSAGE_INPUT_CHANGE = "InputChange";
const string AudioView::MESSAGE_OUTPUT_STATE_CHANGE = "OutputStateChange";
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
					float aa = im->getPixel(i, j).a;
					if (dd > d-0.5f)
						aa *= (d + 0.5f - dd);
					if (aa > a)
						a = aa;
				}
			r->setPixel(x, y, color(a, 0, 0, 0));
		}
	return r;
}



void draw_str_with_shadow(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_shadow)
{
	c->setFill(false);
	c->setLineWidth(3);
	c->setColor(col_shadow);
	c->drawStr(x, y, str);

	c->setFill(true);
	c->setLineWidth(1);
	c->setColor(col_text);
	c->drawStr(x, y, str);
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
	midi_scale(Scale::Type::MAJOR, 0),
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

	set_color_scheme(hui::Config.getStr("View.ColorScheme", "bright"));

	midi_view_mode = (MidiMode)hui::Config.getInt("View.MidiMode", (int)MidiMode::CLASSICAL);

	cur_sample = nullptr;
	cur_vlayer = nullptr;
	_prev_cur_track = nullptr;

	dummy_vtrack = new AudioViewTrack(this, nullptr);
	dummy_vlayer = new AudioViewLayer(this, nullptr);

	selection_mode = SelectionMode::NONE;
	hide_selection = false;
	song->subscribe(this, std::bind(&AudioView::on_song_update, this));

	// modes
	mode = nullptr;
	mode_default = new ViewModeDefault(this);
	mode_midi = new ViewModeMidi(this);
	mode_scale_bars = new ViewModeScaleBars(this);
	mode_curve = new ViewModeCurve(this);
	mode_capture = new ViewModeCapture(this);
	set_mode(mode_default);

	area = rect(0, 1024, 0, 768);
	song_area = area;
	enabled = true;
	scroll = new ScrollBar;

	detail_steps = hui::Config.getInt("View.DetailSteps", 1);
	msp.min_move_to_select = hui::Config.getInt("View.MouseMinMoveToSelect", 5);
	preview_sleep_time = hui::Config.getInt("PreviewSleepTime", 10);
	ScrollSpeed = 600;//hui::Config.getInt("View.ScrollSpeed", 600);
	ScrollSpeedFast = 6000;//hui::Config.getInt("View.ScrollSpeedFast", 6000);
	ZoomSpeed = hui::Config.getFloat("View.ZoomSpeed", 0.1f);
	mouse_wheel_speed = hui::Config.getFloat("View.MouseWheelSpeed", 1.0f);
	antialiasing = hui::Config.getBool("View.Antialiasing", false);

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

	renderer = session->song_renderer;
	peak_meter = session->peak_meter;
	playback_active = false;
	stream = session->output_stream;
	stream->subscribe(this, std::bind(&AudioView::on_stream_update, this), stream->MESSAGE_UPDATE);
	stream->subscribe(this, std::bind(&AudioView::on_stream_state_change, this), stream->MESSAGE_STATE_CHANGE);
	stream->subscribe(this, std::bind(&AudioView::on_stream_end_of_stream, this), stream->MESSAGE_PLAY_END_OF_STREAM);

	mx = my = 0;
	msp.stop();

	hover_before_leave = hover = Selection();



	// events
	win->eventXP(id, "hui:draw", std::bind(&AudioView::on_draw, this, std::placeholders::_1));
	win->eventX(id, "hui:mouse-move", std::bind(&AudioView::on_mouse_move, this));
	win->eventX(id, "hui:left-button-down", std::bind(&AudioView::on_left_button_down, this));
	win->eventX(id, "hui:left-double-click", std::bind(&AudioView::on_left_double_click, this));
	win->eventX(id, "hui:left-button-up", std::bind(&AudioView::on_left_button_up, this));
	win->eventX(id, "hui:middle-button-down", std::bind(&AudioView::on_middle_button_down, this));
	win->eventX(id, "hui:middle-button-up", std::bind(&AudioView::on_middle_button_up, this));
	win->eventX(id, "hui:right-button-down", std::bind(&AudioView::on_right_button_down, this));
	win->eventX(id, "hui:right-button-up", std::bind(&AudioView::on_right_button_up, this));
	win->eventX(id, "hui:mouse-leave", std::bind(&AudioView::on_mouse_leave, this));
	win->eventX(id, "hui:key-down", std::bind(&AudioView::on_key_down, this));
	win->eventX(id, "hui:key-up", std::bind(&AudioView::on_key_up, this));
	win->eventX(id, "hui:mouse-wheel", std::bind(&AudioView::on_mouse_wheel, this));

	win->activate(id);


	menu_song = hui::CreateResourceMenu("popup_song_menu");
	menu_track = hui::CreateResourceMenu("popup_track_menu");
	menu_layer = hui::CreateResourceMenu("popup_layer_menu");
	menu_time_track = hui::CreateResourceMenu("popup_time_track_menu");
	menu_sample = hui::CreateResourceMenu("popup_sample_menu");
	menu_marker = hui::CreateResourceMenu("popup_marker_menu");
	menu_bar = hui::CreateResourceMenu("popup_bar_menu");

	//ForceRedraw();
	update_menu();
}

AudioView::~AudioView()
{
	stream->unsubscribe(this);

	song->unsubscribe(this);

	delete(scroll);

	delete(mode_curve);
	delete(mode_scale_bars);
	delete(mode_midi);
	delete(mode_capture);
	delete(mode_default);

	if (peak_thread)
		delete(peak_thread);

	delete(dummy_vtrack);
	delete(dummy_vlayer);

	delete(images.speaker);
	delete(images.speaker_bg);
	delete(images.x);
	delete(images.x_bg);
	delete(images.solo);
	delete(images.solo_bg);
	delete(images.track_audio);
	delete(images.track_audio_bg);
	delete(images.track_midi);
	delete(images.track_midi_bg);
	delete(images.track_time);
	delete(images.track_time_bg);

	hui::Config.setInt("View.DetailSteps", detail_steps);
	hui::Config.setInt("View.MouseMinMoveToSelect", msp.min_move_to_select);
	hui::Config.setInt("View.ScrollSpeed", ScrollSpeed);
	hui::Config.setInt("View.ScrollSpeedFast", ScrollSpeedFast);
	hui::Config.setFloat("View.ZoomSpeed", ZoomSpeed);
	hui::Config.setBool("View.Antialiasing", antialiasing);
	hui::Config.setInt("View.MidiMode", (int)midi_view_mode);

	PerformanceMonitor::delete_channel(perf_channel);
}

void AudioView::set_color_scheme(const string &name)
{
	hui::Config.setStr("View.ColorScheme", name);
	basic_colors = basic_schemes[0];
	for (ColorSchemeBasic &b: basic_schemes)
		if (b.name == name)
			basic_colors = b;

	colors = basic_colors.create(true);
	force_redraw();
}

void AudioView::set_mode(ViewMode *m)
{
	mode = m;
	thm.dirty = true;
	force_redraw();
}

void AudioView::set_scale(const Scale &s)
{
	midi_scale = s;
	notify(MESSAGE_SETTINGS_CHANGE);
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
	if (sel.range.length < 0)
		sel.range.invert();


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

void AudioView::snap_to_grid(int &pos)
{
	int dmin = SNAPPING_DIST;
	bool found = false;
	int new_pos;

	int sub_beats = 1;
	if (mode == mode_midi)
		sub_beats = mode_midi->beat_partition;

	// time bar...
	Array<Beat> beats = song->bars.get_beats(cam.range(), true, true, sub_beats);
	for (Beat &b: beats){
		int dist = fabs(cam.sample2screen(b.range.offset) - cam.sample2screen(pos));
		if (dist < dmin){
			//msg_write(format("barrier:  %d  ->  %d", pos, b));
			new_pos = b.range.offset;
			found = true;
			dmin = dist;
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
		if (win->getKey(hui::KEY_CONTROL))
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

void align_to_beats(Song *s, Range &r, int beat_partition)
{
	Array<Beat> beats = s->bars.get_beats(Range::ALL);//audio->getRange());
	for (Beat &b : beats){
		/*for (int i=0; i<beat_partition; i++){
			Range sr = b.sub(i, beat_partition);
			if (sr.overlaps(sr))
				r = r or sr;
		}*/
		if (b.range.is_inside(r.start())){
			int dl = b.range.length / beat_partition;
			r.set_start(b.range.offset + dl * ((r.start() - b.range.offset) / dl));
		}
		if (b.range.is_inside(r.end())){
			int dl = b.range.length / beat_partition;
			r.set_end(b.range.offset + dl * ((r.end() - b.range.offset) / dl + 1));
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
	//win->redrawRect(id, rect(200, 300, 0, area.y2));
}

void AudioView::force_redraw_part(const rect &r)
{
	win->redrawRect(id, r);
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

void AudioView::draw_grid_time(Painter *c, const rect &r, const color &fg, const color &fg_sel, const color &bg, const color &bg_sel, bool show_time)
{
	double dl = AudioViewTrack::MIN_GRID_DIST / cam.scale; // >= 10 pixel
	double dt = dl / song->sample_rate;
	double ldt = log10(dt);
	double factor = 1;
	if (ldt > 1.5)
		factor = 1.0/0.6/0.60000001;
	else if (ldt > 0)
		factor = 1.0/0.600000001;
	ldt += log10(factor);
	double exp_s = ceil(ldt);
	double exp_s_mod = exp_s - ldt;
	dt = pow(10, exp_s) / factor;
	dl = dt * song->sample_rate;
//	double dw = dl * a->view_zoom;
	int nx0 = ceil(cam.screen2sample(r.x1) / dl);
	int nx1 = ceil(cam.screen2sample(r.x2) / dl);
	color c1 = col_inter(bg, fg, exp_s_mod * 0.8f);
	color c2 = col_inter(bg, fg, 0.8f);
	color c1s = col_inter(bg_sel, fg, exp_s_mod * 0.8f);
	color c2s = col_inter(bg_sel, fg_sel, 0.8f);

	for (int n=nx0; n<nx1; n++){
		double sample = n * dl;
		if (sel.range.is_inside(sample))
			c->setColor(((n % 10) == 0) ? c2s : c1s);
		else
			c->setColor(((n % 10) == 0) ? c2 : c1);
		int xx = cam.sample2screen(sample);
		c->drawLine(xx, r.y1, xx, r.y2);
	}

	if (show_time){
		if (is_playback_active()){
			color cc = colors.preview_marker;
			cc.a = 0.25f;
			c->setColor(cc);
			float x0 = cam.sample2screen(renderer->range().start());
			float x1 = cam.sample2screen(renderer->range().end());
			c->drawRect(x0, r.y1, x1 - x0, r.y1 + TIME_SCALE_HEIGHT);
		}
		c->setColor(colors.grid);
		for (int n=nx0; n<nx1; n++){
			if ((cam.sample2screen(dl) - cam.sample2screen(0)) > 25){
				if (n % 5 == 0)
					c->drawStr(cam.sample2screen(n * dl) + 2, r.y1, song->get_time_str_fuzzy((double)n * dl, dt * 5));
			}else{
				if ((n % 10) == 0)
					c->drawStr(cam.sample2screen(n * dl) + 2, r.y1, song->get_time_str_fuzzy((double)n * dl, dt * 10));
			}
		}
	}
}


void AudioView::draw_grid_bars(Painter *c, const rect &area, const color &fg, const color &fg_sel, const color &bg, const color &bg_sel, bool show_time, int beat_partition)
{
	if (song->bars.num == 0)
		return;
	int prev_num_beats = 0;
	float prev_bpm = 0;
	int s0 = cam.screen2sample(area.x1 - 1);
	int s1 = cam.screen2sample(area.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash = {5,4}, no_dash;


	//color c1 = ColorInterpolate(bg, colors.grid, exp_s_mod);
	//color c2 = colors.grid;

	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar*> bars = song->bars.get_bars(Range(s0, s1 - s0));
	for (Bar *b: bars){
		if (b->is_pause())
			continue;
		int xx = cam.sample2screen(b->range().offset);

		float dx_bar = cam.dsample2screen(b->range().length);
		float dx_beat = dx_bar / b->num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b->index_text % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			if (sel.range.is_inside(b->range().offset))
				c->setColor(col_inter(bg_sel, fg_sel, f1));
			else
				c->setColor(col_inter(bg, fg, f1));
//			c->setLineDash(no_dash, area.y1);
			c->drawLine(xx, area.y1, xx, area.y2);
		}

		if (f2 >= 0.1f){
			color c1 = col_inter(bg, fg, f2*0.5f);
			color c1s = col_inter(bg_sel, fg_sel, f2*0.5f);
			float beat_length = (float)b->range().length / (float)b->num_beats;
//			c->setLineDash(dash, area.y1);
			for (int i=0; i<b->num_beats; i++){
				float beat_offset = b->range().offset + (float)i * beat_length;
				color c2 = col_inter(bg, c1, 0.6f);
				color c2s = col_inter(bg_sel, c1s, 0.6f);
				//c->setColor(c2);
				for (int j=1; j<beat_partition; j++){
					double sample = beat_offset + beat_length * j / beat_partition;
					int x = cam.sample2screen(sample);
					c->setColor(sel.range.is_inside(sample) ? c2s : c2);
					c->drawLine(x, area.y1, x, area.y2);
				}
				if (i == 0)
					continue;
				c->setColor(sel.range.is_inside(beat_offset) ? c1s : c1);
				int x = cam.sample2screen(beat_offset);
				c->drawLine(x, area.y1, x, area.y2);
			}
		}
	}

	c->setFont("", FONT_SIZE, true, false);
	for (Bar *b: bars){
		if (b->is_pause())
			continue;
		int xx = cam.sample2screen(b->range().offset);

		float dx_bar = cam.dsample2screen(b->range().length);
		float dx_beat = dx_bar / b->num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b->index_text % 5) == 0)
			f1 = 1;
		if (show_time){
			if (f1 > 0.9f){
				c->setColor(colors.text_soft1);
				c->drawStr(xx + 4, area.y1, i2s(b->index_text + 1));
			}
			float bpm = b->bpm(song->sample_rate);
			string s;
			if (prev_num_beats != b->num_beats)
				s = i2s(b->num_beats) + "/" + i2s_small(4);
			if (fabs(prev_bpm - bpm) > 0.5f)
				s += format(" \u2669=%.0f", bpm);
			if (s.num > 0){
				c->setColor(colors.text_soft1);
				c->drawStr(max(xx + 4, 20), area.y2 - 16, s);
			}
			prev_num_beats = b->num_beats;
			prev_bpm = bpm;
		}
	}
	c->setFont("", FONT_SIZE, false, false);
	//c->setLineDash(no_dash, 0);
	c->setLineWidth(LINE_WIDTH);
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
		optimize_view();
		hui::RunLater(0.5f, std::bind(&AudioView::optimize_view, this));
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

void AudioView::on_stream_update()
{
	cam.make_sample_visible(playback_pos(), session->sample_rate() * 2);
	force_redraw();
}

void AudioView::on_stream_state_change()
{
	notify(MESSAGE_OUTPUT_STATE_CHANGE);
}

void AudioView::on_stream_end_of_stream()
{
	stream->stop();
	// stop... but wait for other handlers of this message before deleting stream
	hui::RunLater(0.01f,  std::bind(&AudioView::stop, this));
	//stop();
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
			delete(v);
			changed = true;
		}
	for (AudioViewLayer *v: vlayer)
		if (v){
			delete(v);
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
	float w = c->getStrWidth(str);
	return rect(x-CORNER_RADIUS, x + w + CORNER_RADIUS, y-CORNER_RADIUS, y + FONT_SIZE + CORNER_RADIUS);
}

void AudioView::draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, int align)
{
	rect r = get_boxed_str_rect(c, x, y, str);
	float dx =r.width() / 2 - CORNER_RADIUS;
	dx *= (align - 1);
	r.x1 += dx;
	r.x2 += dx;
	c->setColor(col_bg);
	c->setRoundness(CORNER_RADIUS);
	c->drawRect(r);
	c->setRoundness(0);
	c->setColor(col_text);
	c->drawStr(x + dx, y - FONT_SIZE/3, str);
}


void AudioView::draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area)
{
	c->setFont("", -1, true, false);
	float w = c->getStrWidth(msg);
	float x = min(max(mx - 20.0f, area.x1 + 2.0f), area.x2 - w);
	float y = min(max(my + 20, area.y1 + 2.0f), area.y2 - FONT_SIZE - 5);
	draw_boxed_str(c, x, y, msg, colors.background, colors.text_soft1);
	c->setFont("", -1, false, false);
}

void AudioView::draw_cursor_hover(Painter *c, const string &msg)
{
	draw_cursor_hover(c, msg, mx, my, song_area);
}

void AudioView::draw_time_line(Painter *c, int pos, int type, const color &col, bool show_time)
{
	int p = cam.sample2screen(pos);
	if ((p >= song_area.x1) and (p <= song_area.x2)){
		color cc = (type == (int)hover.type) ? colors.selection_boundary_hover : col;
		c->setColor(cc);
		c->setLineWidth(2.0f);
		c->drawLine(p, area.y1, p, area.y2);
		if (show_time)
			draw_boxed_str(c,  p, (song_area.y1 + song_area.y2) / 2, song->get_time_str_long(pos), cc, colors.background);
		c->setLineWidth(1.0f);
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
		c->setLineDash({3,10}, 0);

	c->setColor(AudioView::colors.grid);
	c->drawLine(view->song_area.x1, y, view->song_area.x2, y);
	c->setLineDash({}, 0);

}

void AudioView::draw_background(Painter *c)
{
	int yy = song_area.y1;
	if (vlayer.num > 0)
		yy = vlayer.back()->area.y2;

	// tracks
	for (AudioViewLayer *l: vlayer)
		mode->draw_layer_background(c, l);

	// free space below tracks
	if (yy < song_area.y2){
		c->setColor(colors.background);
		rect rr = rect(song_area.x1, song_area.x2, yy, song_area.y2);
		c->drawRect(rr);
		if (song->bars.num > 0)
			draw_grid_bars(c, rr, colors.grid, colors.grid, colors.background, colors.background, false, 0);
		else
			draw_grid_time(c, rr, colors.grid, colors.grid, colors.background, colors.background, false);
	}

	// lines between tracks
	AudioViewLayer *prev = nullptr;
	for (AudioViewLayer *l: vlayer){
		draw_layer_separator(c, prev, l, this);
		prev = l;
	}
	draw_layer_separator(c, prev, nullptr, this);
}

void AudioView::draw_time_scale(Painter *c)
{
	c->setColor(colors.background_track);
	c->drawRect(clip.x1, clip.y1, clip.width(), TIME_SCALE_HEIGHT);
	draw_grid_time(c, rect(clip.x1, clip.x2, area.y1, area.y1 + TIME_SCALE_HEIGHT), colors.grid, colors.grid, colors.background_track, colors.background_track, true);
}

void AudioView::draw_selection(Painter *c)
{
	// time selection
	int sx1 = cam.sample2screen(sel.range.start());
	int sx2 = cam.sample2screen(sel.range.end());
	int sxx1 = clampi(sx1, song_area.x1, song_area.x2);
	int sxx2 = clampi(sx2, song_area.x1, song_area.x2);
	c->setColor(colors.selection_internal);
	c->drawRect(rect(sxx1, sxx2, area.y1, area.y1 + TIME_SCALE_HEIGHT));
	draw_time_line(c, sel.range.start(), (int)Selection::Type::SELECTION_START, colors.selection_boundary);
	draw_time_line(c, sel.range.end(), (int)Selection::Type::SELECTION_END, colors.selection_boundary);

	if (!hide_selection){
		if ((selection_mode == SelectionMode::TIME) or (selection_mode == SelectionMode::TRACK_RECT)){
			/*c->setColor(colors.selection_internal);
			for (AudioViewLayer *l: vlayer)
				if (sel.has(l->layer))
					c->drawRect(rect(sxx1, sxx2, l->area.y1, l->area.y2));*/
		}else if (selection_mode == SelectionMode::RECT){
			int sx1 = cam.sample2screen(hover.range.start());
			int sx2 = cam.sample2screen(hover.range.end());
			int sxx1 = clampi(sx1, clip.x1, clip.x2);
			int sxx2 = clampi(sx2, clip.x1, clip.x2);
			c->setColor(colors.selection_internal);
			c->setFill(false);
			c->drawRect(rect(sxx1, sxx2, hover.y0, hover.y1));
			c->setFill(true);
			c->drawRect(rect(sxx1, sxx2, hover.y0, hover.y1));
		}
	}

	// bar selection
	if (sel.bar_gap >= 0){
		sx1 = cam.sample2screen(song->barOffset(sel.bar_gap));
		sx2 = sx1;
		c->setAntialiasing(true);
		c->setColor(colors.text_soft1);
		c->setLineWidth(2.5f);
		for (AudioViewLayer *t: vlayer)
			if (t->layer->type == SignalType::BEATS){
				float dy = t->area.height();
				c->drawLine(sx2 + 5, t->area.y1, sx2 + 2, t->area.y1 + dy*0.3f);
				c->drawLine(sx2 + 2, t->area.y1 + dy*0.3f, sx2 + 2, t->area.y2-dy*0.3f);
				c->drawLine(sx2 + 2, t->area.y2-dy*0.3f, sx2 + 5, t->area.y2);
				c->drawLine(sx1 - 5, t->area.y1, sx1 - 2, t->area.y1 + dy*0.3f);
				c->drawLine(sx1 - 2, t->area.y1 + dy*0.3f, sx1 - 2, t->area.y2-dy*0.3f);
				c->drawLine(sx1 - 2, t->area.y2-dy*0.3f, sx1 - 5, t->area.y2);
		}
		c->setLineWidth(1.0f);
		c->setAntialiasing(false);
	}
	if (hover.type == Selection::Type::BAR_GAP){
		sx1 = cam.sample2screen(song->barOffset(hover.index));
		sx2 = sx1;
		c->setAntialiasing(true);
		c->setColor(colors.hover);
		c->setLineWidth(2.5f);
		for (AudioViewLayer *t: vlayer)
			if (t->layer->type == SignalType::BEATS){
				float dy = t->area.height();
				c->drawLine(sx2 + 5, t->area.y1, sx2 + 2, t->area.y1 + dy*0.3f);
				c->drawLine(sx2 + 2, t->area.y1 + dy*0.3f, sx2 + 2, t->area.y2-dy*0.3f);
				c->drawLine(sx2 + 2, t->area.y2-dy*0.3f, sx2 + 5, t->area.y2);
				c->drawLine(sx1 - 5, t->area.y1, sx1 - 2, t->area.y1 + dy*0.3f);
				c->drawLine(sx1 - 2, t->area.y1 + dy*0.3f, sx1 - 2, t->area.y2-dy*0.3f);
				c->drawLine(sx1 - 2, t->area.y2-dy*0.3f, sx1 - 5, t->area.y2);
		}
		c->setLineWidth(1.0f);
		c->setAntialiasing(false);
	}
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

	cam.update(0.1f);

	update_buffer_zoom();

	draw_time_scale(c);

	c->clip(song_area);

	draw_background(c);

	// tracks
	for (AudioViewLayer *t: vlayer)
		t->draw(c);
	for (AudioViewTrack *t: vtrack)
		t->draw(c);

	c->clip(clip);


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

	if (peak_thread and !peak_thread->is_done())
		slow_repeat = true;

	if (cam.needs_update())
		animating = true;

	if (animating or slow_repeat)
		hui::RunLater(animating ? 0.03f : 0.2f, std::bind(&AudioView::force_redraw, this));
}

int frame = 0;

void AudioView::on_draw(Painter *c)
{
	PerformanceMonitor::start_busy(perf_channel);

	colors = basic_colors.create(win->isActive(id));

	area = c->area();
	clip = c->getClip();

	c->setFontSize(FONT_SIZE);
	c->setLineWidth(LINE_WIDTH);
	c->setAntialiasing(antialiasing);
	//c->setColor(ColorWaveCur);

	if (enabled)
		draw_song(c);

	//c->DrawStr(100, 100, i2s(frame++));

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
		delete(peak_thread);
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
	for (auto *l: vlayer)
		l->set_midi_mode(mode);
	//forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	update_menu();
}

void AudioView::zoom_in()
{
	cam.zoom(2.0f);
}

void AudioView::zoom_out()
{
	cam.zoom(0.5f);
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
		song->subscribe(this, std::bind(&AudioView::on_song_update, this));
	enabled = _enabled;
}


void AudioView::play(const Range &range, bool allow_loop)
{
	if (playback_active)
		stop();

	renderer->prepare(range, allow_loop);
	renderer->allow_tracks(get_playable_tracks());
	renderer->allow_layers(get_playable_layers());
	playback_active = true;
	notify(MESSAGE_OUTPUT_STATE_CHANGE);
	//stream->play();
	session->signal_chain->start();
	force_redraw();
}

void AudioView::stop()
{
	if (!playback_active)
		return;
	session->signal_chain->stop();
	//stream->stop();
	playback_active = false;
	notify(MESSAGE_OUTPUT_STATE_CHANGE);
	force_redraw();
}

void AudioView::pause(bool _pause)
{
	if (playback_active){
		stream->pause(_pause);
		notify(MESSAGE_OUTPUT_STATE_CHANGE);
	}

}
bool AudioView::is_playback_active()
{
	return playback_active;
}

bool AudioView::is_paused()
{
	if (playback_active)
		return stream->is_paused();
	return false;
}

int AudioView::playback_pos()
{
	return stream->get_pos();
}

bool AudioView::has_any_solo_track()
{
	for (auto *t: vtrack)
		if (t->solo)
			return true;
	return false;
}

Set<Track*> AudioView::get_playable_tracks()
{
	Set<Track*> tracks;
	Set<Track*> prevented = mode->prevent_playback();
	bool any_solo = has_any_solo_track();
	for (auto *t: vtrack)
		if (!t->track->muted and (t->solo or !any_solo) and !prevented.contains(t->track))
			tracks.add(t->track);
	return tracks;
}

Set<Track*> AudioView::get_selected_tracks()
{
	Set<Track*> tracks;
	for (Track* t: song->tracks)
		if (sel.has(t))
			tracks.add(t);
	return tracks;
}

bool AudioView::has_any_solo_layer(Track *t)
{
	for (auto *v: vlayer)
		if (v->solo and (v->layer->track == t))
			return true;
	return false;
}

Set<TrackLayer*> AudioView::get_playable_layers()
{
	auto tracks = get_playable_tracks();

	Set<TrackLayer*> layers;
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
