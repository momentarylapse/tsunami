/*
 * AudioView.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "AudioView.h"
#include "AudioViewTrack.h"
#include "Mode/ViewModeDefault.h"
#include "Mode/ViewModeMidi.h"
#include "Mode/ViewModeCurve.h"
#include "Mode/ViewModeCapture.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Device/InputStreamAny.h"
#include "../Device/InputStreamAudio.h"
#include "../Device/InputStreamMidi.h"
#include "../Device/OutputStream.h"
#include "../Audio/Renderer/MidiRenderer.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Renderer/SongRenderer.h"
#include "../Stuff/Log.h"
#include "../lib/math/math.h"
#include "../lib/threads/Thread.h"
#include "Mode/ViewModeScaleBars.h"

#include "../lib/threads/Mutex.h"

const int AudioView::FONT_SIZE = 10;
const int AudioView::MAX_TRACK_CHANNEL_HEIGHT = 125;
const float AudioView::LINE_WIDTH = 1.0f;
const float AudioView::CORNER_RADIUS = 8.0f;
const int AudioView::SAMPLE_FRAME_HEIGHT = 20;
const int AudioView::TIME_SCALE_HEIGHT = 20;
const int AudioView::TRACK_HANDLE_WIDTH = 120;
const int AudioView::TRACK_HANDLE_HEIGHT = AudioView::TIME_SCALE_HEIGHT * 2;
const int AudioView::TRACK_HANDLE_HEIGHT_SMALL = AudioView::TIME_SCALE_HEIGHT;
const int AudioView::BARRIER_DIST = 8;
ColorSchemeBasic AudioView::basic_colors;
ColorScheme AudioView::_export_colors;

int get_track_index_save(Song *song, Track *t)
{
	if (t){
		foreachi(Track *tt, song->tracks, i)
			if (t == tt)
				return i;
	}
	return -1;
}


const string AudioView::MESSAGE_CUR_TRACK_CHANGE = "CurTrackChange";
const string AudioView::MESSAGE_CUR_SAMPLE_CHANGE = "CurSampleChange";
const string AudioView::MESSAGE_CUR_LAYER_CHANGE = "CurLayerChange";
const string AudioView::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string AudioView::MESSAGE_SETTINGS_CHANGE = "SettingsChange";
const string AudioView::MESSAGE_VIEW_CHANGE = "ViewChange";
const string AudioView::MESSAGE_VTRACK_CHANGE = "VTrackChange";


class PeakThread : public Thread
{
public:
	AudioView *view;
	PeakThread(AudioView *_view)
	{
		view = _view;
	}
	virtual void _cdecl onRun()
	{
		//msg_write("  run");
		//HuiSleep(0.1f);
		view->song->updatePeaks();
		view->is_updating_peaks = false;
	}
	/*virtual void _cdecl onCancel()
	{
		msg_error("onCancel!!!");
	}*/
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

AudioView::AudioView(TsunamiWindow *parent, const string &_id, Song *_song) :
	Observable("AudioView"),
	Observer("AudioView"),
	midi_scale(Scale::TYPE_MAJOR, 0),
	cam(this)
{
	id = _id;
	win = parent;

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
	dark.selection = color(1, 0.3f, 0.3f, 0.8f);
	dark.hover = White;
	dark.gamma = 0.3f;
	dark.name = "dark";
	basic_schemes.add(dark);

	setColorScheme(hui::Config.getStr("View.ColorScheme", "bright"));

	dummy_vtrack = new AudioViewTrack(this, NULL);

	song = _song;
	input = NULL;

	midi_view_mode = hui::Config.getInt("View.MidiMode", MIDI_MODE_CLASSICAL);

	// modes
	mode = NULL;
	mode_default = new ViewModeDefault(this);
	mode_midi = new ViewModeMidi(this);
	mode_scale_bars = new ViewModeScaleBars(this);
	mode_curve = new ViewModeCurve(this);
	mode_capture = new ViewModeCapture(this);
	setMode(mode_default);

	area = rect(0, 1024, 0, 768);
	enabled = true;

	detail_steps = hui::Config.getInt("View.DetailSteps", 1);
	msp.min_move_to_select = hui::Config.getInt("View.MouseMinMoveToSelect", 5);
	preview_sleep_time = hui::Config.getInt("PreviewSleepTime", 10);
	ScrollSpeed = hui::Config.getInt("View.ScrollSpeed", 300);
	ScrollSpeedFast = hui::Config.getInt("View.ScrollSpeedFast", 3000);
	ZoomSpeed = hui::Config.getFloat("View.ZoomSpeed", 0.1f);
	antialiasing = hui::Config.getBool("View.Antialiasing", false);

	images.speaker = LoadImage(tsunami->directory_static + "Data/volume.tga");
	images.speaker_bg = ExpandImageMask(images.speaker, 1.5f);
	images.x = LoadImage(tsunami->directory_static + "Data/x.tga");
	images.x_bg = ExpandImageMask(images.x, 1.5f);
	images.solo = LoadImage(tsunami->directory_static + "Data/solo.tga");
	images.solo_bg = ExpandImageMask(images.solo, 1.5f);
	images.track_audio = LoadImage(tsunami->directory_static + "Data/track-audio.tga");
	images.track_audio_bg = ExpandImageMask(images.track_audio, 1.5f);
	images.track_time = LoadImage(tsunami->directory_static + "Data/track-time.tga");
	images.track_time_bg = ExpandImageMask(images.track_time, 1.5f);
	images.track_midi = LoadImage(tsunami->directory_static + "Data/track-midi.tga");
	images.track_midi_bg = ExpandImageMask(images.track_midi, 1.5f);

	cur_track = NULL;
	cur_sample = NULL;
	cur_layer = 0;
	capturing_track = NULL;

	bars_edit_data = true;

	peak_thread = new PeakThread(this);
	is_updating_peaks = false;

	renderer = new SongRenderer(song);
	stream = new OutputStream(renderer);

	mx = my = 0;
	msp.stop();
	selection_mode = SELECTION_MODE_NONE;
	hide_selection = false;
	subscribe(song);
	subscribe(stream);



	// events
	parent->eventXP(id, "hui:draw", std::bind(&AudioView::onDraw, this, std::placeholders::_1));
	parent->eventX(id, "hui:mouse-move", std::bind(&AudioView::onMouseMove, this));
	parent->eventX(id, "hui:left-button-down", std::bind(&AudioView::onLeftButtonDown, this));
	parent->eventX(id, "hui:left-double-click", std::bind(&AudioView::onLeftDoubleClick, this));
	parent->eventX(id, "hui:left-button-up", std::bind(&AudioView::onLeftButtonUp, this));
	parent->eventX(id, "hui:middle-button-down", std::bind(&AudioView::onMiddleButtonDown, this));
	parent->eventX(id, "hui:middle-button-up", std::bind(&AudioView::onMiddleButtonUp, this));
	parent->eventX(id, "hui:right-button-down", std::bind(&AudioView::onRightButtonDown, this));
	parent->eventX(id, "hui:right-button-up", std::bind(&AudioView::onRightButtonUp, this));
	parent->eventX(id, "hui:key-down", std::bind(&AudioView::onKeyDown, this));
	parent->eventX(id, "hui:key-up", std::bind(&AudioView::onKeyUp, this));
	parent->eventX(id, "hui:mouse-wheel", std::bind(&AudioView::onMouseWheel, this));

	parent->activate(id);


	menu_song = hui::CreateResourceMenu("popup_song_menu");
	menu_track = hui::CreateResourceMenu("popup_track_menu");
	menu_sample = hui::CreateResourceMenu("popup_sample_menu");
	menu_marker = hui::CreateResourceMenu("popup_marker_menu");

	//ForceRedraw();
	updateMenu();
}

AudioView::~AudioView()
{
	unsubscribe(song);
	unsubscribe(stream);
	setInput(NULL);

	delete(mode_curve);
	delete(mode_scale_bars);
	delete(mode_midi);
	delete(mode_capture);
	delete(mode_default);

	delete(peak_thread);
	delete(stream);
	delete(renderer);

	delete(dummy_vtrack);

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
	hui::Config.setInt("View.MidiMode", midi_view_mode);
}

void AudioView::setColorScheme(const string &name)
{
	hui::Config.setStr("View.ColorScheme", name);
	basic_colors = basic_schemes[0];
	for (ColorSchemeBasic &b: basic_schemes)
		if (b.name == name)
			basic_colors = b;

	colors = basic_colors.create(true);
	_export_colors = colors;
	forceRedraw();
}

void AudioView::setMode(ViewMode *m)
{
	mode = m;
	thm.dirty = true;
	forceRedraw();
}

void AudioView::setScale(const Scale &s)
{
	midi_scale = s;
	notify(MESSAGE_SETTINGS_CHANGE);
	forceRedraw();
}

void AudioView::setMouse()
{
	mx = hui::GetEvent()->mx;
	my = hui::GetEvent()->my;
}

bool AudioView::mouseOverTrack(AudioViewTrack *t)
{
	return t->area.inside(mx, my);
}

int AudioView::mouseOverSample(SampleRef *s)
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

void AudioView::selectionUpdatePos(Selection &s)
{
	s.pos = cam.screen2sample(mx);
}

void AudioView::updateSelection()
{
	if (sel.range.length < 0)
		sel.range.invert();

	sel.update_bars(song);


	renderer->setRange(getPlaybackSelection());
	if (stream->isPlaying()){
		if (renderer->range().is_inside(stream->getPos()))
			renderer->setRange(getPlaybackSelection());
		else
			stream->stop();
	}
	forceRedraw();

	notify(MESSAGE_SELECTION_CHANGE);
}


void AudioView::setSelection(const SongSelection &s)
{
	sel = s;
	updateSelection();
}


bool AudioView::mouse_over_time(int pos)
{
	int ssx = cam.sample2screen(pos);
	return ((mx >= ssx - 5) and (mx <= ssx + 5));
}


Range AudioView::getPlaybackSelection()
{
	if (sel.range.empty()){
		int num = song->getRangeWithTime().end() - sel.range.start();
		if (num <= 0)
			num = song->sample_rate; // 1 second
		return Range(sel.range.start(), num);
	}
	return sel.range;
}

void AudioView::applyBarriers(int &pos)
{
	int dmin = BARRIER_DIST;
	bool found = false;
	int new_pos;
	for (int b: hover.barrier){
		int dist = fabs(cam.sample2screen(b) - cam.sample2screen(pos));
		if (dist < dmin){
			//msg_write(format("barrier:  %d  ->  %d", pos, b));
			new_pos = b;
			found = true;
			dmin = dist;
		}
	}
	if (found)
		pos = new_pos;
}

void AudioView::onMouseMove()
{
	setMouse();
	mode->onMouseMove();


	if (selection_mode != SELECTION_MODE_NONE){

		applyBarriers(hover.pos);
		hover.range.set_end(hover.pos);
		hover.y1 = my;
		if (win->getKey(hui::KEY_CONTROL))
			setSelection(sel_temp or mode->getSelection(hover.range));
		else
			setSelection(mode->getSelection(hover.range));
	}else{

		// selection:
		if (msp.step()){
			mode->startSelection();
			msp.stop();
		}
	}
	forceRedraw();
}

void AudioView::onLeftButtonDown()
{
	setMouse();
	mode->onLeftButtonDown();

	forceRedraw();
	updateMenu();
}

void align_to_beats(Song *s, Range &r, int beat_partition)
{
	Array<Beat> beats = s->bars.getBeats(Range::ALL);//audio->getRange());
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


void AudioView::onLeftButtonUp()
{
	mode->onLeftButtonUp();

	forceRedraw();
	updateMenu();
}



void AudioView::onMiddleButtonDown()
{
}



void AudioView::onMiddleButtonUp()
{
}



void AudioView::onRightButtonDown()
{
	mode->onRightButtonDown();
}

void AudioView::onRightButtonUp()
{
	mode->onRightButtonUp();
}



void AudioView::onLeftDoubleClick()
{
	mode->onLeftDoubleClick();
	forceRedraw();
	updateMenu();
}



void AudioView::onCommand(const string & id)
{
}



void AudioView::onKeyDown()
{
	int k = hui::GetEvent()->key_code;
	mode->onKeyDown(k);
}

void AudioView::onKeyUp()
{
	int k = hui::GetEvent()->key_code;
	mode->onKeyUp(k);
}

void AudioView::onMouseWheel()
{
	mode->onMouseWheel();
}


void AudioView::forceRedraw()
{
	//msg_write("force redraw");
	force_redraw = true;
	win->redraw(id);
}

void AudioView::unselectAllSamples()
{
	sel.samples.clear();
}

void AudioView::updateBufferZoom()
{
	prefered_buffer_layer = -1;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (cam.scale < 0.2f)
		for (int i=24-1;i>=0;i--){
			double _f = (double)(1 << (i + BufferBox::PEAK_OFFSET_EXP));
			if (_f > 1.0 / cam.scale){
				prefered_buffer_layer = i;
				buffer_zoom_factor = _f;
			}
		}
}

void AudioView::drawGridTime(Painter *c, const rect &r, const color &bg, bool show_time)
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
	int nx0 = floor(cam.screen2sample(r.x1 - 1) / dl);
	int nx1 = ceil(cam.screen2sample(r.x2) / dl);
	color c1 = ColorInterpolate(bg, colors.grid, exp_s_mod);
	color c2 = colors.grid;
	for (int n=nx0; n<nx1; n++){
		c->setColor(((n % 10) == 0) ? c2 : c1);
		int xx = cam.sample2screen(n * dl);
		c->drawLine(xx, r.y1, xx, r.y2);
	}
	if (show_time){
		if (stream->isPlaying()){
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

void AudioView::checkConsistency()
{
	// check cur_track consistency
	int n = get_track_index_save(song, cur_track);
	if (cur_track and (n < 0))
		if (song->tracks.num > 0)
			setCurTrack(song->tracks[0]);

	// check cur_layer consistency
	if ((cur_layer < 0) or (cur_layer >= song->layers.num)){
		cur_layer = 0;
		forceRedraw();
	}
}

void AudioView::onUpdate(Observable *o, const string &message)
{
	//msg_write("AudioView: " + o->getName() + " / " + message);
	checkConsistency();

	if (o == song){

		if (message == song->MESSAGE_NEW){
			updateTracks();
			sel.range = Range(0, 0);
			setCurTrack(NULL);
			if (song->tracks.num > 0){
				if ((song->tracks[0]->type == Track::TYPE_TIME) and song->tracks.num > 1)
					setCurTrack(song->tracks[1]);
				else
					setCurTrack(song->tracks[0]);
			}
			optimizeView();
		}else if (message == song->MESSAGE_FINISHED_LOADING){
			optimizeView();
			hui::RunLater(0.5f, std::bind(&AudioView::optimizeView, this));
		}else{
			if ((message == song->MESSAGE_ADD_TRACK) or (message == song->MESSAGE_DELETE_TRACK))
				updateTracks();
			forceRedraw();
			updateMenu();
		}

		if (message == song->MESSAGE_CHANGE)
			if (song->action_manager->isEnabled())
				updatePeaks();
	}else if (o == stream){
		if (stream->isPlaying())
			cam.makeSampleVisible(stream->getPos());
		forceRedraw();
	}else if (input and o == input){
		if (input->isCapturing())
			cam.makeSampleVisible(sel.range.start() + input->getSampleCount());
		forceRedraw();
	}else{
		forceRedraw();
	}
}

AudioViewTrack *AudioView::get_track(Track *track)
{
	for (auto t: vtrack){
		if (t->track == track)
			return t;
	}
	return dummy_vtrack;
}

void AudioView::updateTracks()
{
	bool changed = false;

	Array<AudioViewTrack*> vtrack2;
	vtrack2.resize(song->tracks.num);
	foreachi(Track *t, song->tracks, ti){
		bool found = false;

		// find existing
		foreachi(AudioViewTrack *v, vtrack, vi)
			if (v){
				if (v->track == t){
					vtrack2[ti] = v;
					vtrack[vi] = NULL;
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

	// delete deleted
	for (AudioViewTrack *v: vtrack)
		if (v){
			delete(v);
			changed = true;
		}
	vtrack = vtrack2;
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

	checkConsistency();

	if (changed)
		notify(MESSAGE_VTRACK_CHANGE);
}

void AudioView::drawBoxedStr(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg)
{
	float w = c->getStrWidth(str);
	c->setColor(col_bg);
	c->setRoundness(CORNER_RADIUS);
	c->drawRect(x-CORNER_RADIUS, y-CORNER_RADIUS, w + 2*CORNER_RADIUS, FONT_SIZE + 2*CORNER_RADIUS);
	c->setRoundness(0);
	c->setColor(col_text);
	c->drawStr(x, y - FONT_SIZE/3, str);
}

void AudioView::drawTimeLine(Painter *c, int pos, int type, const color &col, bool show_time)
{
	int p = cam.sample2screen(pos);
	if ((p >= area.x1) and (p <= area.x2)){
		color cc = (type == hover.type) ? colors.selection_boundary_hover : col;
		c->setColor(cc);
		c->setLineWidth(2.0f);
		c->drawLine(p, area.y1, p, area.y2);
		if (show_time)
			drawBoxedStr(c,  p, (area.y1 + area.y2) / 2, song->get_time_str_long(pos), cc, colors.background);
		c->setLineWidth(1.0f);
	}
}

void AudioView::drawBackground(Painter *c, const rect &r)
{
	int yy = 0;
	if (vtrack.num > 0)
		yy = vtrack.back()->area.y2;

	// time scale
	c->setColor(colors.background_track);
	c->drawRect(r.x1, r.y1, r.width(), TIME_SCALE_HEIGHT);
	drawGridTime(c, rect(r.x1, r.x2, r.y1, r.y1 + TIME_SCALE_HEIGHT), colors.background_track, true);

	// tracks
	for (AudioViewTrack *t: vtrack)
		mode->drawTrackBackground(c, t);

	// free space below tracks
	if (yy < r.y2){
		c->setColor(colors.background);
		rect rr = rect(r.x1, r.x2, yy, r.y2);
		c->drawRect(rr);
		drawGridTime(c, rr, colors.background, false);
	}

	// lines between tracks
	c->setColor(colors.grid);
	for (AudioViewTrack *t: vtrack)
		c->drawLine(0, t->area.y1, r.width(), t->area.y1);
	if (yy < r.y2)
		c->drawLine(0, yy, r.width(), yy);

	//DrawGrid(c, r, ColorBackgroundCurWave, true);
}

void AudioView::drawSelection(Painter *c, const rect &r)
{
	// time selection
	int sx1 = cam.sample2screen(sel.range.start());
	int sx2 = cam.sample2screen(sel.range.end());
	int sxx1 = clampi(sx1, r.x1, r.x2);
	int sxx2 = clampi(sx2, r.x1, r.x2);
	c->setColor(colors.selection_internal);
	c->drawRect(rect(sxx1, sxx2, r.y1, r.y1 + TIME_SCALE_HEIGHT));
	drawTimeLine(c, sel.range.start(), Selection::TYPE_SELECTION_START, colors.selection_boundary);
	drawTimeLine(c, sel.range.end(), Selection::TYPE_SELECTION_END, colors.selection_boundary);

	if (!hide_selection){
	if ((selection_mode == SELECTION_MODE_TIME) or (selection_mode == SELECTION_MODE_TRACK_RECT)){
		c->setColor(colors.selection_internal);
		for (AudioViewTrack *t: vtrack)
			if (sel.has(t->track))
				c->drawRect(rect(sxx1, sxx2, t->area.y1, t->area.y2));
	}else if (selection_mode == SELECTION_MODE_RECT){
		int sx1 = cam.sample2screen(hover.range.start());
		int sx2 = cam.sample2screen(hover.range.end());
		int sxx1 = clampi(sx1, r.x1, r.x2);
		int sxx2 = clampi(sx2, r.x1, r.x2);
		c->setColor(colors.selection_internal);
		c->setFill(false);
		c->drawRect(rect(sxx1, sxx2, hover.y0, hover.y1));
		c->setFill(true);
		c->drawRect(rect(sxx1, sxx2, hover.y0, hover.y1));
	}
	}

	// bar selection
	sx1 = cam.sample2screen(sel.bar_range.start());
	sx2 = cam.sample2screen(sel.bar_range.end());
	c->setAntialiasing(true);
	c->setColor(colors.text_soft1);
	c->setLineWidth(2.5f);
	for (AudioViewTrack *t: vtrack)
		if (t->track->type == Track::TYPE_TIME){
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

void AudioView::drawAudioFile(Painter *c, const rect &r)
{
	area = r;
	//msg_write("draw");

	bool repeat = thm.update(this, song, r);
	bool repeat_fast = repeat;
	updateBufferZoom();

	// background
	drawBackground(c, r);

	// tracks
	for (AudioViewTrack *t: vtrack)
		t->draw(c);

	// capturing preview
	if (input and input->isCapturing()){
		int type = input->getType();
		if (type == Track::TYPE_AUDIO)
			((InputStreamAudio*)input)->buffer.update_peaks();
		if (capturing_track){
			if (type == Track::TYPE_AUDIO)
				get_track(capturing_track)->drawBuffer(c, dynamic_cast<InputStreamAudio*>(input)->buffer, cam.pos - sel.range.offset, colors.capture_marker);
			if (type == Track::TYPE_MIDI)
				mode->drawMidi(c, get_track(capturing_track), midi_events_to_notes(((InputStreamMidi*)input)->midi), true, sel.range.start());
		}
	}


	// selection
	drawSelection(c, r);


	// playing/capturing position
	if (input and input->isCapturing())
		drawTimeLine(c, sel.range.start() + input->getSampleCount(), Selection::TYPE_PLAYBACK, colors.capture_marker, true);
	else if (stream->isPlaying())
		drawTimeLine(c, stream->getPos(), Selection::TYPE_PLAYBACK, colors.preview_marker, true);

	mode->drawPost(c);

	repeat |= is_updating_peaks;

	if (repeat)
		hui::RunLater(repeat_fast ? 0.03f : 0.2f, std::bind(&AudioView::forceRedraw, this));
}

int frame=0;

void AudioView::onDraw(Painter *c)
{
	colors = basic_colors.create(win->isActive(id));
	force_redraw = false;

	area = rect(0, c->width, 0, c->height);

	c->setFontSize(FONT_SIZE);
	c->setLineWidth(LINE_WIDTH);
	c->setAntialiasing(antialiasing);
	//c->setColor(ColorWaveCur);

	if (enabled)
		drawAudioFile(c, area);

	//c->DrawStr(100, 100, i2s(frame++));

	colors = basic_colors.create(true);
}

void AudioView::optimizeView()
{
	if (area.x2 <= 0)
		area.x2 = 1024;

	Range r = song->getRangeWithTime();

	if (r.length == 0)
		r.length = 10 * song->sample_rate;

	cam.show(r);
}

void AudioView::updateMenu()
{
	// view
	win->check("view_midi_default", midi_view_mode == MIDI_MODE_LINEAR);
	win->check("view_midi_tab", midi_view_mode == MIDI_MODE_TAB);
	win->check("view_midi_score", midi_view_mode == MIDI_MODE_CLASSICAL);
	win->enable("view_samples", false);
}

void AudioView::updatePeaks()
{
	//msg_write("-------------------- view update peaks");
	if (is_updating_peaks){
		//msg_error("   already updating peaks...");
		delete(peak_thread);
		peak_thread = new PeakThread(this);
	}
	is_updating_peaks = true;
	peak_thread->run();
	for (int i=0; i<5; i++){
		if (peak_thread->isDone()){
			forceRedraw();
			break;
		}else
			hui::Sleep(0.001f);
	}
}

void AudioView::setMidiViewMode(int mode)
{
	midi_view_mode = mode;
	forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	updateMenu();
}

void AudioView::zoomIn()
{
	cam.zoom(2.0f);
}

void AudioView::zoomOut()
{
	cam.zoom(0.5f);
}

void AudioView::selectAll()
{
	setSelection(mode->getSelectionForRange(song->getRange()));
}

void AudioView::selectNone()
{
	// select all/none
	setSelection(SongSelection());
	setCurSample(NULL);
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

void AudioView::selectExpand()
{
	Range r = sel.range;
	bool update = true;
	while(update){
		update = false;
		for (Track *t: song->tracks){
			if (!sel.has(t))
				continue;

			// midi
			for (MidiNote *n: t->midi)
				test_range(n->range, r, update);

			// buffers
			for (TrackLayer &l: t->layers)
				for (BufferBox &b: l.buffers)
					test_range(b.range(), r, update);

			// samples
			for (SampleRef *s: t->samples)
				test_range(s->range(), r, update);
		}
	}

	setSelection(mode->getSelectionForRange(r));
}



void AudioView::selectSample(SampleRef *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		sel.set(s, !sel.has(s));
	}else{
		if (!sel.has(s))
			unselectAllSamples();

		// select this sub
		sel.add(s);
	}
}

void AudioView::selectTrack(Track *t, bool diff)
{
	if (!t)
		return;
	if (diff){
		bool is_only_selected = true;
		for (Track *tt : t->song->tracks)
			if (sel.has(tt) and (tt != t))
				is_only_selected = false;
		sel.set(t, !sel.has(t) or is_only_selected);
	}else{
		if (!sel.has(t)){
			// unselect all tracks
			for (Track *tt : t->song->tracks)
				sel.set(tt, false);
		}

		// select this track
		sel.add(t);
	}
	// TODO: what to do???
	SongSelection ss = mode->getSelectionForRange(sel.range);
	setSelection(ss);
}

void AudioView::setCurSample(SampleRef *s)
{
	if (cur_sample == s)
		return;
	cur_sample = s;
	forceRedraw();
	notify(MESSAGE_CUR_SAMPLE_CHANGE);
}


void AudioView::setCurTrack(Track *t)
{
	if (cur_track == t)
		return;
	cur_track = t;
	mode->onCurTrackChange();
	forceRedraw();
	notify(MESSAGE_CUR_TRACK_CHANGE);
}

void AudioView::setCurLayer(int l)
{
	if (cur_layer == l)
		return;
	if ((l < 0) or (l >= song->layers.num))
		return;
	cur_layer = l;
	forceRedraw();
	notify(MESSAGE_CUR_LAYER_CHANGE);
}


void AudioView::setInput(InputStreamAny *_input)
{
	if (input)
		unsubscribe(input);

	input = _input;
	notify("SetInput");

	if (input)
		subscribe(input);
}

// unused?!?
void AudioView::enable(bool _enabled)
{
	if (enabled and !_enabled)
		unsubscribe(song);
	else if (!enabled and _enabled)
		subscribe(song);
	enabled = _enabled;
}
