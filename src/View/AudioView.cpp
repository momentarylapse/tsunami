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
#include "../Device/OutputStream.h"
#include "../Audio/Renderer/MidiRenderer.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Renderer/SongRenderer.h"
#include "../Stuff/Log.h"
#include "../lib/math/math.h"
#include "../lib/threads/Thread.h"
#include "Mode/ViewModeScaleBars.h"

const int AudioView::FONT_SIZE = 10;
const int AudioView::MAX_TRACK_CHANNEL_HEIGHT = 125;
const float AudioView::LINE_WIDTH = 1.0f;
const int AudioView::SAMPLE_FRAME_HEIGHT = 20;
const int AudioView::TIME_SCALE_HEIGHT = 20;
const int AudioView::TRACK_HANDLE_WIDTH = 60;
const int AudioView::BARRIER_DIST = 8;
ColorScheme AudioView::colors;

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
const string AudioView::MESSAGE_CUR_LEVEL_CHANGE = "CurLevelChange";
const string AudioView::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string AudioView::MESSAGE_SETTINGS_CHANGE = "SettingsChange";
const string AudioView::MESSAGE_VIEW_CHANGE = "ViewChange";
const string AudioView::MESSAGE_VTRACK_CHANGE = "VTrackChange";


class PeakThread : public Thread
{
public:
	AudioView *view;
	bool recheck;
	PeakThread(AudioView *_view)
	{
		view = _view;
		recheck = false;
	}
	virtual void _cdecl onRun()
	{
		view->song->updatePeaks();
		while(recheck){
			printf("----recheck!!!!!\n");
			recheck = false;
			view->song->updatePeaks();
		}
		view->is_updating_peaks = false;
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

AudioView::AudioView(TsunamiWindow *parent, Song *_song, DeviceManager *_output) :
	Observable("AudioView"),
	Observer("AudioView"),
	midi_scale(Scale::TYPE_MAJOR, 0),
	cam(this)
{
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

	setColorScheme(HuiConfig.getStr("View.ColorScheme", "bright"));

	song = _song;
	input = NULL;

	edit_multi = false;
	midi_view_mode = HuiConfig.getInt("View.MidiMode", MIDI_MODE_SCORE);

	// modes
	mode = NULL;
	mode_default = new ViewModeDefault(this);
	mode_midi = new ViewModeMidi(this);
	mode_scale_bars = new ViewModeScaleBars(this);
	mode_curve = new ViewModeCurve(this);
	mode_capture = new ViewModeCapture(this);
	setMode(mode_default);

	drawing_rect = rect(0, 1024, 0, 768);
	enabled = true;

	show_mono = HuiConfig.getBool("View.Mono", true);
	detail_steps = HuiConfig.getInt("View.DetailSteps", 1);
	mouse_min_move_to_select = HuiConfig.getInt("View.MouseMinMoveToSelect", 5);
	preview_sleep_time = HuiConfig.getInt("PreviewSleepTime", 10);
	ScrollSpeed = HuiConfig.getInt("View.ScrollSpeed", 300);
	ScrollSpeedFast = HuiConfig.getInt("View.ScrollSpeedFast", 3000);
	ZoomSpeed = HuiConfig.getFloat("View.ZoomSpeed", 0.1f);
	peak_mode = HuiConfig.getInt("View.PeakMode", BufferBox::PEAK_BOTH);
	antialiasing = HuiConfig.getBool("View.Antialiasing", false);

	images.speaker = LoadImage(HuiAppDirectoryStatic + "Data/volume.tga");
	images.speaker_bg = ExpandImageMask(images.speaker, 1.5f);
	images.x = LoadImage(HuiAppDirectoryStatic + "Data/x.tga");
	images.x_bg = ExpandImageMask(images.x, 1.5f);
	images.solo = LoadImage(HuiAppDirectoryStatic + "Data/solo.tga");
	images.solo_bg = ExpandImageMask(images.solo, 1.5f);
	images.track_audio = LoadImage(HuiAppDirectoryStatic + "Data/track-audio.tga");
	images.track_audio_bg = ExpandImageMask(images.track_audio, 1.5f);
	images.track_time = LoadImage(HuiAppDirectoryStatic + "Data/track-time.tga");
	images.track_time_bg = ExpandImageMask(images.track_time, 1.5f);
	images.track_midi = LoadImage(HuiAppDirectoryStatic + "Data/track-midi.tga");
	images.track_midi_bg = ExpandImageMask(images.track_midi, 1.5f);

	cur_track = NULL;
	cur_sample = NULL;
	cur_level = 0;
	capturing_track = 0;

	bars_edit_data = true;

	peak_thread = new PeakThread(this);
	is_updating_peaks = false;

	renderer = new SongRenderer(song, &sel);
	stream = new OutputStream(renderer);

	area = rect(0, 0, 0, 0);
	mx = my = 0;
	subscribe(song);
	subscribe(stream);



	// events
	parent->eventXP("area", "hui:draw", this, &AudioView::onDraw);
	parent->eventX("area", "hui:mouse-move", this, &AudioView::onMouseMove);
	parent->eventX("area", "hui:left-button-down", this, &AudioView::onLeftButtonDown);
	parent->eventX("area", "hui:left-double-click", this, &AudioView::onLeftDoubleClick);
	parent->eventX("area", "hui:left-button-up", this, &AudioView::onLeftButtonUp);
	parent->eventX("area", "hui:middle-button-down", this, &AudioView::onMiddleButtonDown);
	parent->eventX("area", "hui:middle-button-up", this, &AudioView::onMiddleButtonUp);
	parent->eventX("area", "hui:right-button-down", this, &AudioView::onRightButtonDown);
	parent->eventX("area", "hui:right-button-up", this, &AudioView::onRightButtonUp);
	parent->eventX("area", "hui:key-down", this, &AudioView::onKeyDown);
	parent->eventX("area", "hui:key-up", this, &AudioView::onKeyUp);
	parent->eventX("area", "hui:mouse-wheel", this, &AudioView::onMouseWheel);

	parent->activate("area");


	menu_song = HuiCreateResourceMenu("popup_song_menu");
	menu_track = HuiCreateResourceMenu("popup_track_menu");
	menu_sample = HuiCreateResourceMenu("popup_sample_menu");
	menu_marker = HuiCreateResourceMenu("popup_marker_menu");

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

	HuiConfig.setBool("View.Mono", show_mono);
	HuiConfig.setInt("View.DetailSteps", detail_steps);
	HuiConfig.setInt("View.MouseMinMoveToSelect", mouse_min_move_to_select);
	HuiConfig.setInt("View.ScrollSpeed", ScrollSpeed);
	HuiConfig.setInt("View.ScrollSpeedFast", ScrollSpeedFast);
	HuiConfig.setFloat("View.ZoomSpeed", ZoomSpeed);
	HuiConfig.setBool("View.Antialiasing", antialiasing);
	HuiConfig.setInt("View.MidiMode", midi_view_mode);
}

void AudioView::setColorScheme(const string &name)
{
	HuiConfig.setStr("View.ColorScheme", name);
	colors.create(basic_schemes[0]);
	for (ColorSchemeBasic &b : basic_schemes)
		if (b.name == name)
			colors.create(b);
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
	mx = HuiGetEvent()->mx;
	my = HuiGetEvent()->my;
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
	sel.fromRange(song, sel_raw);

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

void AudioView::onMouseMove()
{
	setMouse();
	mode->onMouseMove();
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

	// TODO !!!!!!!!
	selection.clear();

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
	int k = HuiGetEvent()->key_code;
	mode->onKeyDown(k);
}

void AudioView::onKeyUp()
{
	int k = HuiGetEvent()->key_code;
	mode->onKeyUp(k);
}

void AudioView::onMouseWheel()
{
	mode->onMouseWheel();
}


void AudioView::forceRedraw()
{
	force_redraw = true;
	win->redraw("area");
}

void AudioView::unselectAllSamples()
{
	sel.samples.clear();
}

void AudioView::updateBufferZoom()
{
	prefered_buffer_level = 0;
	buffer_zoom_factor = 1.0;

	// which level of detail?
	if (cam.scale < 0.8f)
		for (int i=24-1;i>=0;i--){
			double _f = pow(2, (double)i);
			if (_f > 1.0 / cam.scale){
				prefered_buffer_level = i;
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

	// check cur_level consistency
	if ((cur_level < 0) or (cur_level >= song->level_names.num)){
		cur_level = 0;
		forceRedraw();
	}
}

void AudioView::onUpdate(Observable *o, const string &message)
{
	checkConsistency();

	if (o == song){
		if (message == song->MESSAGE_NEW){
			updateTracks();
			sel.range = sel_raw = Range(0, 0);
			setCurTrack(NULL);
			if (song->tracks.num > 0)
				setCurTrack(song->tracks[0]);
			optimizeView();
		}else{
			if ((message == song->MESSAGE_ADD_TRACK) or (message == song->MESSAGE_DELETE_TRACK))
				updateTracks();
			forceRedraw();
			updateMenu();
		}

		if (message == song->MESSAGE_CHANGE)
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

void AudioView::updateTracks()
{
	bool changed = false;

	Array<AudioViewTrack*> vtrack2;
	vtrack2.resize(song->tracks.num);
	foreachi(Track *t, song->tracks, ti){
		bool found = false;
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
	for (AudioViewTrack *v : vtrack)
		if (v){
			delete(v);
			changed = true;
		}
	vtrack = vtrack2;
	thm.dirty = true;
	foreachi(AudioViewTrack *v, vtrack, i){
		if (i > 0){
			if (v->area.y1 < vtrack[i-1]->area.y2){
				v->area.y1 = vtrack[i-1]->area.y2;
				v->area.y2 = vtrack[i-1]->area.y2;
			}
		}
	}

	checkConsistency();

	if (changed)
		notify(MESSAGE_VTRACK_CHANGE);
}

void AudioView::drawTimeLine(Painter *c, int pos, int type, color &col, bool show_time)
{
	int p = cam.sample2screen(pos);
	if ((p >= area.x1) and (p <= area.x2)){
		c->setColor((type == hover.type) ? colors.selection_boundary_hover : col);
		c->drawLine(p, area.y1, p, area.y2);
		if (show_time)
			c->drawStr(p, (area.y1 + area.y2) / 2, song->get_time_str_long(pos));
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
	foreachi(AudioViewTrack *t, vtrack, i)
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
	foreachi(AudioViewTrack *t, vtrack, i)
		c->drawLine(0, t->area.y1, r.width(), t->area.y1);
	if (yy < r.y2)
		c->drawLine(0, yy, r.width(), yy);

	//DrawGrid(c, r, ColorBackgroundCurWave, true);
}

void AudioView::drawSelection(Painter *c, const rect &r)
{
	int sx1 = cam.sample2screen(sel.range.start());
	int sx2 = cam.sample2screen(sel.range.end());
	int sxx1 = clampi(sx1, r.x1, r.x2);
	int sxx2 = clampi(sx2, r.x1, r.x2);
	c->setColor(colors.selection_internal);
	foreachi(AudioViewTrack *t, vtrack, i)
		if (sel.has(t->track))
			c->drawRect(rect(sxx1, sxx2, t->area.y1, t->area.y2));
	drawTimeLine(c, sel_raw.start(), Selection::TYPE_SELECTION_START, colors.selection_boundary);
	drawTimeLine(c, sel_raw.end(), Selection::TYPE_SELECTION_END, colors.selection_boundary);



	sx1 = cam.sample2screen(sel.bar_range.start());
	sx2 = cam.sample2screen(sel.bar_range.end());
	sxx1 = clampi(sx1, r.x1, r.x2);
	sxx2 = clampi(sx2, r.x1, r.x2);
	c->setColor(colors.selection_internal);
	foreachi(AudioViewTrack *t, vtrack, i)
		if (t->track->type == Track::TYPE_TIME){
			c->drawRect(rect(sxx1, sxx2, t->area.y1, t->area.y2));
			c->drawRect(rect(sxx2 - 5, sxx2 + 5, t->area.y1, t->area.y2));
		}
	/*drawTimeLine(c, sel_raw.start(), Selection::TYPE_SELECTION_START, colors.selection_boundary);
	drawTimeLine(c, sel_raw.end(), Selection::TYPE_SELECTION_END, colors.selection_boundary);*/
}

void AudioView::drawAudioFile(Painter *c, const rect &r)
{
	area = r;

	bool repeat = thm.update(this, song, r);
	updateBufferZoom();

	// background
	drawBackground(c, r);

	// tracks
	for (AudioViewTrack *t : vtrack)
		t->draw(c);

	// capturing preview
	if (input and input->isCapturing()){
		if (input->type == Track::TYPE_AUDIO)
			input->buffer->update_peaks();
		if ((capturing_track >= 0) and (capturing_track < vtrack.num)){
			if (input->type == Track::TYPE_AUDIO)
				vtrack[capturing_track]->drawBuffer(c, *input->buffer, cam.pos - sel.range.offset, colors.capture_marker);
			if (input->type == Track::TYPE_MIDI)
				vtrack[capturing_track]->drawMidi(c, midi_events_to_notes(*input->midi), true, sel.range.start());
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
		HuiRunLaterM(0.03f, this, &AudioView::forceRedraw);
}

int frame=0;

void AudioView::onDraw(Painter *c)
{
	force_redraw = false;

	drawing_rect = rect(0, c->width, 0, c->height);

	c->setFontSize(FONT_SIZE);
	c->setLineWidth(LINE_WIDTH);
	c->setAntialiasing(antialiasing);
	//c->setColor(ColorWaveCur);

	if (enabled)
		drawAudioFile(c, drawing_rect);

	//c->DrawStr(100, 100, i2s(frame++));
}

void AudioView::optimizeView()
{
	if (area.x2 <= 0)
		area.x2 = drawing_rect.x2;

	Range r = song->getRangeWithTime();

	if (r.length == 0)
		r.length = 10 * song->sample_rate;

	cam.show(r);
}

void AudioView::updateMenu()
{
	// edit
	win->check("edit_multi", edit_multi);
	// view
	win->check("view_mono", show_mono);
	win->check("view_stereo", !show_mono);
	win->check("view_peaks_max", peak_mode == BufferBox::PEAK_MAXIMUM);
	win->check("view_peaks_mean", peak_mode == BufferBox::PEAK_SQUAREMEAN);
	win->check("view_peaks_both", peak_mode == BufferBox::PEAK_BOTH);
	win->check("view_midi_default", midi_view_mode == MIDI_MODE_MIDI);
	win->check("view_midi_tab", midi_view_mode == MIDI_MODE_TAB);
	win->check("view_midi_score", midi_view_mode == MIDI_MODE_SCORE);
	win->enable("view_samples", false);
}

void AudioView::updatePeaks()
{
	if (is_updating_peaks){
		peak_thread->recheck = true;
		return;
	}
	is_updating_peaks = true;
	peak_thread->run();
	for (int i=0; i<10; i++){
		if (peak_thread->isDone())
			break;
		else
			HuiSleep(0.001f);
	}

	forceRedraw();
}

void AudioView::setPeaksMode(int mode)
{
	peak_mode = mode;
	forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	updateMenu();
}

void AudioView::setShowMono(bool mono)
{
	show_mono = mono;
	forceRedraw();
	notify(MESSAGE_SETTINGS_CHANGE);
	updateMenu();
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
	sel_raw = song->getRange();
	updateSelection();
}

void AudioView::selectNone()
{
	// select all/none
	sel_raw.clear();
	updateSelection();
	unselectAllSamples();
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
	bool update = true;
	while(update){
		update = false;
		for (Track *t: song->tracks){
			if (!sel.has(t))
				continue;

			// midi
			for (MidiNote *n: t->midi)
				test_range(n->range, sel_raw, update);

			// buffers
			for (TrackLevel &l: t->levels)
				for (BufferBox &b: l.buffers)
					test_range(b.range(), sel_raw, update);

			// samples
			for (SampleRef *s: t->samples)
				test_range(s->range(), sel_raw, update);
		}
	}

	updateSelection();
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
	updateSelection();
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

void AudioView::setCurLevel(int l)
{
	if (cur_level == l)
		return;
	if ((l < 0) or (l >= song->level_names.num))
		return;
	cur_level = l;
	forceRedraw();
	notify(MESSAGE_CUR_LEVEL_CHANGE);
}

void AudioView::setInput(InputStreamAny *_input)
{
	if (input)
		unsubscribe(input);

	input = _input;

	if (input)
		subscribe(input);
}

void AudioView::enable(bool _enabled)
{
	if (enabled and !_enabled)
		unsubscribe(song);
	else if (!enabled and _enabled)
		subscribe(song);
	enabled = _enabled;
}

void AudioView::setEditMulti(bool enabled)
{
	edit_multi = enabled;
	updateMenu();
}

SongSelection AudioView::getEditSeletion()
{
	if (edit_multi)
		return sel;

	return sel.restrict_to_track(cur_track);
}
