/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "AudioView.h"
#include "Mode/ViewMode.h"
#include "Mode/ViewModeMidi.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "SideBar/SideBar.h"
#include "../Data/Song.h"
#include "../Rhythm/BarCollection.h"
#include "../Rhythm/Bar.h"
#include "../Rhythm/Beat.h"
#include "../Midi/MidiData.h"
#include "../Midi/Clef.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Source/SongRenderer.h"


const int PITCH_SHOW_COUNT = 30;

string i2s_small(int i); // -> MidiData.cpp

AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track)
{
	view = _view;
	track = _track;
	solo = false;

	pitch_min = 55;
	pitch_max = pitch_min + PITCH_SHOW_COUNT;

	area = rect(0, 0, 0, 0);
	height_min = height_wish = 0;
	clef_dy = 0;
	clef_y0 = 0;
}

AudioViewTrack::~AudioViewTrack()
{
}



color AudioViewTrack::getPitchColor(int pitch)
{
	return SetColorHSB(1, (float)(pitch % 12) / 12.0f, 0.6f, 1);
}

const float AudioViewTrack::MIN_GRID_DIST = 10.0f;

static Array<complex> tt;

inline void draw_line_buffer(Painter *c, int width, double view_pos, double zoom, float hf, float x, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	int i0 = max((double) x          / zoom + view_pos - offset    , 0.0);
	int i1 = min((double)(x + width) / zoom + view_pos - offset + 2, (double)buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);

	for (int i=i0; i<i1; i++){

		double p = x + ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 + buf[i] * hf;
		if (zoom > 5)
			c->drawCircle(p, tt[nl].y, 2);
		nl ++;
	}
	c->drawLines(tt);
}

inline void draw_line_buffer_sel(Painter *c, int width, double view_pos, double zoom, float hf, float x, float y0, const Array<float> &buf, int offset, const Range &r)
{
	int nl = 0;
	int i0 = max((double) x          / zoom + view_pos - offset    , 0.0);
	int i1 = min((double)(x + width) / zoom + view_pos - offset + 2, (double)buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);
	c->setLineWidth(3.0f);

	for (int i=i0; i<i1; i++){
		if (!r.is_inside(i + offset))
			continue;

		double p = x + ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 + buf[i] * hf;
		if (zoom > 5)
			c->drawCircle(p, tt[nl].y, 4);
		nl ++;
	}
	tt.resize(nl);
	c->drawLines(tt);
	c->setLineWidth(1.0f);
}

inline void draw_peak_buffer(Painter *c, int width, int di, double view_pos_rel, double zoom, float f, float hf, float x, float y0, const string &buf, int offset)
{
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize(width/di + 10);
	for (int i=0; i<width+di; i+=di){

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) and (ip < buf.num))
		if (((int)(p) < offset + buf.num*f) and (p >= offset)){
			tt[nl].x = (float)x+i;
			float dy = ((float)(buf[ip])/255.0f) * hf;
			tt[nl].y  = y0 - dy;
			nl ++;
		}
	}
	if (nl == 0)
		return;
	tt.resize(nl * 2);
	for (int i=0; i<nl; i++){
		tt[nl + i].x = tt[nl - i - 1].x;
		tt[nl + i].y = y0 *2 - tt[nl - i - 1].y + 1;
	}
	c->drawPolygon(tt);
}

inline void draw_peak_buffer_sel(Painter *c, int width, int di, double view_pos_rel, double zoom, float f, float hf, float x, float y0, const string &buf, int offset, int xmin, int xmax)
{
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize(width/di + 10);
	for (int i=0; i<width+di; i+=di){

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) and (ip < buf.num))
		if (((int)(p) < offset + buf.num*f) and (p >= offset)){
			if ((((float)x+i) >= xmin) and (((float)x+i)<= xmax)){
				tt[nl].x = (float)x+i;
				float dy = ((float)(buf[ip])/255.0f) * hf;
				tt[nl].y  = y0 - dy - 1;
				nl ++;
			}
		}
	}
	if (nl == 0)
		return;
	tt.resize(nl * 2);
	for (int i=0; i<nl; i++){
		tt[nl + i].x = tt[nl - i - 1].x;
		tt[nl + i].y = y0 *2 - tt[nl - i - 1].y + 1;
	}
	c->drawPolygon(tt);
}

void AudioViewTrack::drawBuffer(Painter *c, AudioBuffer &b, double view_pos_rel, const color &col)
{
	float w = area.width();
	float h = area.height();
	float hf = h / 4;
	float x1 = area.x1;
	float y1 = area.y1;

	// zero heights of both channels
	float y0r = y1 + hf;
	float y0l = y1 + hf * 3;


	int di = view->detail_steps;
	c->setColor(col);

	//int l = min(view->prefered_buffer_layer - 1, b.peaks.num / 4);
	int l = view->prefered_buffer_layer * 4;
	if (l >= 0){
		double bzf = view->buffer_zoom_factor;


		// no peaks yet? -> show dummy
		if (b.peaks.num < l){
			c->setColor(ColorInterpolate(col, Red, 0.3f));
			c->drawRect((b.offset - view_pos_rel) * view->cam.scale, y1, b.length * view->cam.scale, h);
			return;
		}

		// maximum
		double _bzf = bzf;
		int ll = l;
		color cc = col;
		cc.a *= 0.3f;
		c->setColor(cc);
		if (ll + 4 < b.peaks.num){
			ll += 4;
			_bzf *= 2;
		}
		draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, _bzf, hf, x1, y0r, b.peaks[ll], b.offset);
		draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, _bzf, hf, x1, y0l, b.peaks[ll+1], b.offset);

		// mean square
		c->setColor(col);
		draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, bzf, hf, x1, y0r, b.peaks[l+2], b.offset);
		draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, bzf, hf, x1, y0l, b.peaks[l+3], b.offset);


		// invalid peaks...
		int nn = min(b.length / b.PEAK_CHUNK_SIZE, b.peaks[b.PEAK_MAGIC_LEVEL4].num);
		for (int i=0; i<nn; i++){
			if (b.peaks[b.PEAK_MAGIC_LEVEL4][i] == 255){
				c->setColor(ColorInterpolate(col, Red, 0.3f));
				float x1 = max((float)view->cam.sample2screen(b.offset + i*b.PEAK_CHUNK_SIZE), 0.0f);
				float x2 = min((float)view->cam.sample2screen(b.offset + (i+1)*b.PEAK_CHUNK_SIZE), w);
				c->drawRect(x1, y1, x2 - x1, h);
			}
		}
	}else{

		// directly show every sample
		draw_line_buffer(c, w, view_pos_rel, view->cam.scale, hf, x1, y0r, b.c[0], b.offset);
		draw_line_buffer(c, w, view_pos_rel, view->cam.scale, hf, x1, y0l, b.c[1], b.offset);
	}
}

void AudioViewTrack::drawBufferSelection(Painter *c, AudioBuffer &b, double view_pos_rel, const color &col, const Range &r)
{
	float w = area.width();
	float h = area.height();
	float hf = h / 4;
	float x1 = area.x1;
	float y1 = area.y1;

	// zero heights of both channels
	float y0r = y1 + hf;
	float y0l = y1 + hf * 3;


	int di = view->detail_steps;
	color cc = col;
	cc.a = 0.7f;
	c->setColor(cc);

	//int l = min(view->prefered_buffer_layer - 1, b.peaks.num / 4);
	int l = view->prefered_buffer_layer * 4;
	if (l >= 0){
		double bzf = view->buffer_zoom_factor;


		// no peaks yet? -> show dummy
		if (b.peaks.num < l){
			c->setColor(ColorInterpolate(col, Red, 0.3f));
			c->drawRect((b.offset - view_pos_rel) * view->cam.scale, y1, b.length * view->cam.scale, h);
			return;
		}

		// maximum
		double _bzf = bzf;
		int ll = l;
		if (ll + 4 < b.peaks.num){
			ll += 4;
			_bzf *= 2;
		}
		int xmin = view->cam.sample2screen(r.start());
		int xmax = view->cam.sample2screen(r.end());
		draw_peak_buffer_sel(c, w, di, view_pos_rel, view->cam.scale, _bzf, hf, x1, y0r, b.peaks[ll], b.offset, xmin, xmax);
		draw_peak_buffer_sel(c, w, di, view_pos_rel, view->cam.scale, _bzf, hf, x1, y0l, b.peaks[ll+1], b.offset, xmin, xmax);
	}else{

		// directly show every sample
		draw_line_buffer_sel(c, w, view_pos_rel, view->cam.scale, hf, x1, y0r, b.c[0], b.offset, r);
		draw_line_buffer_sel(c, w, view_pos_rel, view->cam.scale, hf, x1, y0l, b.c[1], b.offset, r);
	}
}

void AudioViewTrack::drawTrackBuffers(Painter *c, double view_pos_rel)
{
	// non-current layers
	foreachi(TrackLayer &lev, track->layers, layer_no){
		if (layer_no == view->cur_layer)
			continue;
		for (AudioBuffer &b: lev.buffers)
			drawBuffer(c, b, view_pos_rel, view->colors.text_soft2);
	}

	// current
	for (AudioBuffer &b: track->layers[view->cur_layer].buffers)
		drawBuffer(c, b, view_pos_rel, view->colors.text);

	if (view->sel.has(track)){
		// selection
		for (AudioBuffer &b: track->layers[view->cur_layer].buffers){
			drawBufferSelection(c, b, view_pos_rel, view->colors.selection_boundary, view->sel.range);
		}
	}
}

void AudioViewTrack::drawSampleFrame(Painter *c, SampleRef *s, const color &col, int delay)
{
	// frame
	Range rr = s->range() + delay;
	int asx = clampi(view->cam.sample2screen(rr.start()), area.x1, area.x2);
	int aex = clampi(view->cam.sample2screen(rr.end()), area.x1, area.x2);

	if (delay == 0)
		s->area = rect(asx, aex, area.y1, area.y2);


	color col2 = col;
	col2.a *= 0.2f;
	c->setColor(col2);
	c->drawRect(asx, area.y1,                             aex - asx, view->SAMPLE_FRAME_HEIGHT);
	c->drawRect(asx, area.y2 - view->SAMPLE_FRAME_HEIGHT, aex - asx, view->SAMPLE_FRAME_HEIGHT);

	c->setColor(col);
	c->drawLine(asx, area.y1, asx, area.y2);
	c->drawLine(aex, area.y1, aex, area.y2);
	c->drawLine(asx, area.y1, aex, area.y1);
	c->drawLine(asx, area.y2, aex, area.y2);
}

void drawStrWithShadow(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_shadow)
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

void AudioViewTrack::drawSample(Painter *c, SampleRef *s)
{
	color col = view->colors.sample;
	if (view->sel.has(s))
		col = view->colors.sample_selected;
	if (view->hover.sample == s)
		col = ColorInterpolate(col,  view->colors.hover, 0.2f);

	drawSampleFrame(c, s, col, 0);

	// buffer
	if (s->type() == Track::TYPE_AUDIO)
		drawBuffer(c, *s->buf, view->cam.pos - (double)s->pos, col);
	else if (s->type() == Track::TYPE_MIDI)
		view->mode->drawMidi(c, this, *s->midi, true, s->pos);

	if (view->sel.has(s)){
		int asx = clampi(view->cam.sample2screen(s->pos), area.x1, area.x2);
		drawStrWithShadow(c, asx, area.y2 - view->SAMPLE_FRAME_HEIGHT, s->origin->name, view->colors.text, view->colors.background_track_selected);
	}
}

void AudioViewTrack::drawMarker(Painter *c, const TrackMarker *marker, int index, bool hover)
{
	string text = marker->text;
	if (text.match(":pos *:"))
		text = "🖐 " + text.substr(5, -2);
	if (marker->fx.num > 0)
		text += format(" (%d fx)", marker->fx.num);

	bool sel = view->sel.has(marker);

	if (sel)
		c->setFont("", -1, true, false);

	float w = c->getStrWidth(text) + view->CORNER_RADIUS * 2;
	float x0 = view->cam.sample2screen(marker->range.start());
	float x1 = view->cam.sample2screen(marker->range.end());
	float y0 = area.y1;
	float y1 = area.y1 + 30;

	w = max(w, x1 - x0);

	color col = view->colors.text;
	color col_bg = view->colors.background_track;
	color col2 = getPitchColor(marker->text.hash() % MAX_PITCH);
	if (sel){
		col = view->colors.selection;
		col_bg = ColorInterpolate(view->colors.background_track, view->colors.selection, 0.2f);
		col2 = ColorInterpolate(col2, view->colors.selection, 0.5f);
	}
	if (hover){
		col = ColorInterpolate(col, view->colors.hover, 0.3f);
		col2 = ColorInterpolate(col2, view->colors.hover, 0.3f);
	}

	color col_bg2 = col2;
	col_bg2.a = 0.25f;
	c->setColor(col_bg2);
	c->drawRect(x0, y0, x1-x0, y1-y0);
	c->setColor(col2);
	c->setLineWidth(2.0f);
	c->drawLine(x0, area.y1, x0, area.y2);
	c->drawLine(x1, area.y1, x1, area.y2);
	c->setLineWidth(1.0f);
	view->drawBoxedStr(c,  x0 + view->CORNER_RADIUS, y0 + 10, text, col, col_bg);

	marker_areas[index] = rect(x0, x0 + w, y0, y1);

	c->setFont("", -1, false, false);
}



void AudioViewTrack::setPitchMinMax(int _min, int _max)
{
	int diff = _max - _min;
	pitch_min = clampi(_min, 0, MAX_PITCH - 1 - diff);
	pitch_max = pitch_min + diff;
	view->forceRedraw();
}

float AudioViewTrack::pitch2y_linear(int pitch)
{
	return area.y2 - area.height() * ((float)pitch - pitch_min) / (pitch_max - pitch_min);
}

float AudioViewTrack::pitch2y_classical(int pitch)
{
	int mod;
	const Clef& clef = track->instrument.get_clef();
	int p = clef.pitch_to_position(pitch, view->midi_scale, mod);
	return clef_pos_to_screen(p);
}

int AudioViewTrack::y2pitch_linear(float y)
{
	return pitch_min + ((area.y2 - y) * (pitch_max - pitch_min) / area.height());
}

int AudioViewTrack::y2pitch_classical(float y, int modifier)
{
	const Clef& clef = track->instrument.get_clef();
	int pos = screen_to_clef_pos(y);
	return clef.position_to_pitch(pos, view->midi_scale, modifier);
}

int AudioViewTrack::y2clef_classical(float y, int &mod)
{
	mod = MODIFIER_UNKNOWN;//modifier;
	return screen_to_clef_pos(y);
}

int AudioViewTrack::y2clef_linear(float y, int &mod)
{
	mod = MODIFIER_UNKNOWN;//modifier;

	int pitch = y2pitch_linear(y);
	const Clef& clef = track->instrument.get_clef();
	return clef.pitch_to_position(pitch, view->midi_scale, mod);
}


void AudioViewTrack::drawMidiNoteLinear(Painter *c, const MidiNote &n, int shift, MidiNoteState state)
{
	float x1 = view->cam.sample2screen(n.range.offset + shift);
	float x2 = view->cam.sample2screen(n.range.end() + shift);
	float y1 = pitch2y_linear(n.pitch + 1);
	float y2 = pitch2y_linear(n.pitch);

	float y = (y1 + y2) / 2;
	n.y = y;
	float r = max((y2 - y1) / 2.3f, 2.0f);

	if (state & AudioViewTrack::STATE_SELECTED){
		color col1 = view->colors.selection;
		AudioViewTrack::draw_simple_note(c, x1, x2, y, r, 2, col1, col1, false);
	}

	color col = ColorInterpolate(AudioViewTrack::getPitchColor(n.pitch), view->colors.text, 0.2f);
	if (state & AudioViewTrack::STATE_HOVER){
		col = ColorInterpolate(col, view->colors.hover, 0.333f);
	}else if (state & AudioViewTrack::STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.background_track, 0.65f);
	}

	AudioViewTrack::draw_simple_note(c, x1, x2, y, r, 0, col, ColorInterpolate(col, view->colors.background_track, 0.4f), false);
}

inline AudioViewTrack::MidiNoteState note_state(MidiNote *n, bool as_reference, AudioView *view)
{
	AudioViewTrack::MidiNoteState s = AudioViewTrack::STATE_DEFAULT;
	if (view->sel.has(n))
		s = AudioViewTrack::STATE_SELECTED;
	if (as_reference)
		return (AudioViewTrack::MidiNoteState)(AudioViewTrack::STATE_REFERENCE | s);
	if ((view->hover.type == Selection::TYPE_MIDI_NOTE) and (n == view->hover.note))
		return (AudioViewTrack::MidiNoteState)(AudioViewTrack::STATE_HOVER | s);
	return s;
}

void AudioViewTrack::drawMidiLinear(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift)
{
	Range range = view->cam.range() - shift;
	MidiNoteBufferRef notes = midi.getNotes(range);
	//c->setLineWidth(3.0f);


	// draw notes
	for (MidiNote *n: midi){
		if ((n->pitch < pitch_min) or (n->pitch >= pitch_max))
			continue;
		drawMidiNoteLinear(c, *n, shift, note_state(n, as_reference, view));
	}
}

void AudioViewTrack::draw_simple_note(Painter *c, float x1, float x2, float y, float r, float rx, const color &col, const color &col_shadow, bool force_circle)
{
	float x = x1 + r;

	// "shadow" to indicate length
	if (x2 - x1 > r*2){
		c->setColor(col_shadow);
		c->drawRect(x, y - r - rx, x2 - x + rx, r*2 + rx*2);
	}

	// the note circle
	c->setColor(col);
	if ((x2 - x1 > 6) or force_circle)
		c->drawCircle(x, y, r+rx);
	else
		c->drawRect(x - r*0.8f - rx, y - r*0.8f - rx, r*1.6f + rx*2, r*1.6f + rx*2);
}

void AudioViewTrack::drawMidiClefTab(Painter *c)
{
	c->setColor(view->colors.text);

	// clef lines
	float dy = (area.height() * 0.7f) / track->instrument.string_pitch.num;
	dy = min(dy, 40.0f);
	clef_dy = dy;

	float h = dy * track->instrument.string_pitch.num;
	float y0 = area.y2 - (area.height() - h) / 2 - dy/2;
	clef_y0 = y0;
	for (int i=0; i<track->instrument.string_pitch.num; i++){
		float y = y0 - i*dy;
		c->drawLine(area.x1, y, area.x2, y);
	}
	c->setAntialiasing(true);

	c->setFontSize(h / 6);
	c->drawStr(10, area.y1 + area.height() / 2 - h * 0.37f, "T\nA\nB");
}

void AudioViewTrack::drawMidiNoteTab(Painter *c, const MidiNote *n, int shift, MidiNoteState state)
{
	float r = min(clef_dy/2, 15.0f);

	float x1 = view->cam.sample2screen(n->range.offset + shift);
	float x2 = view->cam.sample2screen(n->range.end() + shift);
	//float x = x1 + r;


	int p = n->stringno;
	float y = string_to_screen(p);
	n->y = y;

	if (state & STATE_SELECTED){
		color col1 = view->colors.selection;
		draw_simple_note(c, x1, x2, y, r, 2, col1, col1, false);
	}

	color col = ColorInterpolate(getPitchColor(n->pitch), view->colors.text, 0.2f);
	if (state & STATE_HOVER){
		col = ColorInterpolate(col, view->colors.hover, 0.333f);
	}else if (state & STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.background_track, 0.65f);
	}

	/*if (n->modifier != MODIFIER_NONE){
		c->setColor(ColorInterpolate(col, view->colors.text, 0.5f));
		float size = r*2.8f;
		c->setFontSize(size);
		c->drawStr(x1 - size*0.7f, y - size*0.8f , modifier_symbol(n->modifier));
	}*/

	//draw_simple_note(c, x1, x2, y, r, 0, col, ColorInterpolate(col, view->colors.background_track, 0.4f), (state == STATE_HOVER));
/*

	color col = ColorInterpolate(getPitchColor(n->pitch), view->colors.text, 0.2f);
	col = ColorInterpolate(col, view->colors.background_track, 0.3f);*/

	draw_simple_note(c, x1, x2, y, r, 0, col, ColorInterpolate(col, view->colors.background_track, 0.3f), false);

	if (x2 - x1 > r/4){
		float font_size = r * 1.4f;
		c->setFontSize(font_size);
		c->setColor(view->colors.text);
		c->drawStr(x1 + font_size*0.2f, y - font_size*0.75f, i2s(n->pitch - track->instrument.string_pitch[n->stringno]));
	}
}

void AudioViewTrack::drawMidiTab(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift)
{
	Range range = view->cam.range() - shift;
	midi.update_meta(track, view->midi_scale);
	MidiNoteBufferRef notes = midi.getNotes(range);

	for (MidiNote *n: notes)
		drawMidiNoteTab(c,  n,  shift,  note_state(n, as_reference, view));

	c->setAntialiasing(false);
	c->setFontSize(view->FONT_SIZE);
}

float AudioViewTrack::clef_pos_to_screen(int pos)
{
	return area.y2 - area.height() / 2 - (pos - 4) * clef_dy / 2.0f;
}

int AudioViewTrack::screen_to_clef_pos(float y)
{
	return (int)floor((area.y2 - y - area.height() / 2) * 2.0f / clef_dy + 0.5f) + 4;
}

float AudioViewTrack::string_to_screen(int string_no)
{
	return clef_y0 - string_no * clef_dy;
}

int AudioViewTrack::screen_to_string(float y)
{
	return (int)floor((clef_y0 - y) / clef_dy + 0.5f);
}

void AudioViewTrack::drawMidiNoteClassical(Painter *c, const MidiNote *n, int shift, MidiNoteState state, const Clef &clef)
{
	float r = clef_dy/2;

	float x1 = view->cam.sample2screen(n->range.offset + shift);
	float x2 = view->cam.sample2screen(n->range.end() + shift);
	float x = x1 + r;

	// checked before...
//	if (n.clef_position < 0)
//		n.update_meta(track->instrument, view->midi_scale, 0);

	int p = n->clef_position;
	float y = clef_pos_to_screen(p);
	n->y = y;

	// auxiliary lines
	for (int i=10; i<=p; i+=2){
		c->setColor(view->colors.text_soft2);
		float y = clef_pos_to_screen(i);
		c->drawLine(x - clef_dy, y, x + clef_dy, y);
	}
	for (int i=-2; i>=p; i-=2){
		c->setColor(view->colors.text_soft2);
		float y = clef_pos_to_screen(i);
		c->drawLine(x - clef_dy, y, x + clef_dy, y);
	}

	if (state & STATE_SELECTED){
		color col1 = view->colors.selection;
		draw_simple_note(c, x1, x2, y, r, 2, col1, col1, false);
	}

	color col = ColorInterpolate(getPitchColor(n->pitch), view->colors.text, 0.2f);
	if (state & STATE_HOVER){
		col = ColorInterpolate(col, view->colors.hover, 0.333f);
	}else if (state & STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.background_track, 0.65f);
	}

	if (n->modifier != MODIFIER_NONE){
		c->setColor(view->colors.text);
		//c->setColor(ColorInterpolate(col, view->colors.text, 0.5f));
		float size = r*2.8f;
		c->setFontSize(size);
		c->drawStr(x1 - size*0.7f, y - size*0.8f , modifier_symbol(n->modifier));
	}

	draw_simple_note(c, x1, x2, y, r, 0, col, ColorInterpolate(col, view->colors.background_track, 0.4f), (state == STATE_HOVER));
}

void AudioViewTrack::drawMidiClefClassical(Painter *c, const Clef &clef, const Scale &scale)
{
	// clef lines
	float dy = min(area.height() / 13, 30.0f);
	clef_dy = dy;
	c->setColor(view->colors.text);
	for (int i=0; i<10; i+=2){
		float y = clef_pos_to_screen(i);
		c->drawLine(area.x1, y, area.x2, y);
	}

	// clef symbol
	c->setFontSize(dy*4);
	c->drawStr(10, clef_pos_to_screen(10), clef.symbol);
	c->setFontSize(dy);

	for (int i=0; i<7; i++)
		if (scale.modifiers[i] != MODIFIER_NONE)
			c->drawStr(18 + dy*3.0f + dy*0.6f*(i % 3), clef_pos_to_screen((i - clef.offset + 7*20) % 7) - dy*0.8f, modifier_symbol(scale.modifiers[i]));
	c->setFontSize(view->FONT_SIZE);
}

void AudioViewTrack::drawMidiClassical(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift)
{
	Range range = view->cam.range() - shift;
	midi.update_meta(track, view->midi_scale);
	MidiNoteBufferRef notes = midi.getNotes(range);

	const Clef& clef = track->instrument.get_clef();

	c->setAntialiasing(true);

	for (MidiNote *n: notes)
		drawMidiNoteClassical(c, n, shift, note_state(n, as_reference, view), clef);

	c->setFontSize(view->FONT_SIZE);
	c->setAntialiasing(false);
}

void AudioViewTrack::drawGridBars(Painter *c, const color &bg, bool show_time, int beat_partition)
{
	if (view->song->bars.num == 0)
		return;
	int prev_num_beats = 0;
	float prev_bpm = 0;
	int s0 = view->cam.screen2sample(area.x1 - 1);
	int s1 = view->cam.screen2sample(area.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash, no_dash;
	dash.add(6);
	dash.add(4);
	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar> bars = view->song->bars.getBars(Range(s0, s1 - s0));
	for (Bar &b: bars){
		int xx = view->cam.sample2screen(b.range.offset);

		float dx_bar = view->cam.dsample2screen(b.range.length);
		float dx_beat = dx_bar / b.num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b.index % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			c->setColor(ColorInterpolate(bg, view->colors.text_soft1, f1));
			c->setLineDash(no_dash, area.y1);
			c->drawLine(xx, area.y1, xx, area.y2);
		}

		if (f2 >= 0.1f){
			color c1 = ColorInterpolate(bg, view->colors.text_soft1, f2);
			float beat_length = (float)b.range.length / (float)b.num_beats;
			c->setLineDash(dash, area.y1);
			for (int i=0; i<b.num_beats; i++){
				float beat_offset = b.range.offset + (float)i * beat_length;
				color c2 = ColorInterpolate(bg, c1, 0.6f);
				c->setColor(c2);
				for (int j=1; j<beat_partition; j++){
					int x = view->cam.sample2screen(beat_offset + beat_length * j / beat_partition);
					c->drawLine(x, area.y1, x, area.y2);
				}
				if (i == 0)
					continue;
				c->setColor(c1);
				int x = view->cam.sample2screen(beat_offset);
				c->drawLine(x, area.y1, x, area.y2);
			}
		}

		if (show_time){
			if (f1 > 0.9f){
				c->setColor(view->colors.text_soft1);
				c->drawStr(xx + 2, area.y1, i2s(b.index + 1));
			}
			float bpm = b.bpm(view->song->sample_rate);
			string s;
			if (prev_num_beats != b.num_beats)
				s = i2s(b.num_beats) + "/" + i2s_small(4);
			if (fabs(prev_bpm - bpm) > 0.5f)
				s += format(" \u2669=%.0f", bpm);
			if (s.num > 0){
				c->setColor(view->colors.text_soft1);
				c->setFont("", view->FONT_SIZE, true, false);
				c->drawStr(max(xx + 4, 20), area.y2 - 16, s);
				c->setFont("", view->FONT_SIZE, false, false);
			}
			prev_num_beats = b.num_beats;
			prev_bpm = bpm;
		}
	}
	c->setLineDash(no_dash, 0);
	c->setLineWidth(view->LINE_WIDTH);
}

color AudioViewTrack::getBackgroundColor()
{
	return (view->sel.has(track)) ? view->colors.background_track_selected : view->colors.background_track;
}

void AudioViewTrack::drawBlankBackground(Painter *c)
{
	color cc = getBackgroundColor();
	c->setColor(cc);
	c->drawRect(area);
}

void AudioViewTrack::draw(Painter *c)
{
	view->mode->drawTrackData(c, this);

	drawHeader(c);
}

bool AudioView::editingTrack(Track *t)
{
	if (cur_track != t)
		return false;
	if (win->side_bar->isActive(SideBar::TRACK_CONSOLE))
		return true;
	if (win->side_bar->isActive(SideBar::FX_CONSOLE))
		return true;
	if (win->side_bar->isActive(SideBar::MIDI_FX_CONCOLE))
		return true;
	if (win->side_bar->isActive(SideBar::SYNTH_CONSOLE))
		return true;
	if (win->side_bar->isActive(SideBar::MIDI_EDITOR_CONSOLE))
		return true;
	if (win->side_bar->isActive(SideBar::CAPTURE_CONSOLE))
		return true;
	return false;
}

void AudioViewTrack::drawHeader(Painter *c)
{
	bool hover = (view->hover.track == track) and view->hover.is_in(Selection::TYPE_TRACK_HEADER);
	bool visible = hover or view->editingTrack(track);
	bool playable = (view->get_playable_tracks().find(track) >= 0);

	color col = view->colors.background_track_selected;
	if (view->sel.has(track))
		col = ColorInterpolate(col, view->colors.selection, 0.4f);
	if (hover)
		col = ColorInterpolate(col, view->colors.hover, 0.2f);
	c->setColor(col);
	float h = visible ? view->TRACK_HANDLE_HEIGHT : view->TRACK_HANDLE_HEIGHT_SMALL;
	c->setRoundness(view->CORNER_RADIUS);
	c->drawRect(area.x1,  area.y1,  view->TRACK_HANDLE_WIDTH, h);
	c->setRoundness(0);

	// track title
	c->setFont("", view->FONT_SIZE, view->sel.has(track) and playable, false);
	if (playable)
		c->setColor(view->colors.text);
	else
		c->setColor(view->colors.text_soft2);
	c->drawStr(area.x1 + 23, area.y1 + 3, track->getNiceName() + (solo ? " (solo)" : ""));

	c->setFont("", -1, false, false);

	// icons
	if (track->type == track->TYPE_TIME){
		c->setColor(view->colors.text);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_time); // "⏱"
	}else if (track->type == track->TYPE_MIDI){
		c->setColor(view->colors.text);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_midi); // "♫"
	}else{
		c->setColor(view->colors.text);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_audio); // "∿"
	}
	if (track->muted and !visible)
		c->drawImage(area.x1 + 5, area.y1 + 5, *view->images.x);

	color col_but = ColorInterpolate(view->colors.text, view->colors.hover, 0.3f);
	color col_but_hover = view->colors.text;

	if (visible){
		c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::TYPE_TRACK_BUTTON_MUTE))
			c->setColor(col_but_hover);
		//c->drawStr(area.x1 + 5, area.y1 + 22-2, "\U0001f50a"); // U+1F50A "🔊"
		c->drawMaskImage(area.x1 + 5, area.y1 + 22, *view->images.speaker);
		if (track->muted)
			c->drawImage(area.x1 + 5, area.y1 + 22, *view->images.x);
	}
	if ((view->song->tracks.num > 1) and visible){
		c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::TYPE_TRACK_BUTTON_SOLO))
			c->setColor(col_but_hover);
		//c->drawStr(area.x1 + 5 + 17, area.y1 + 22-2, "S");
		c->drawMaskImage(area.x1 + 22, area.y1 + 22, *view->images.solo);
	}
	if (visible){
		c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::TYPE_TRACK_BUTTON_EDIT))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*2, area.y1 + 22-2, "\U0001f527"); // U+1F527 "🔧"

		c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::TYPE_TRACK_BUTTON_FX))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*3, area.y1 + 22-2, "⚡"); // ...

		/*c->setColor(col_but);
		if ((view->hover.track == track) and (view->hover.type == Selection::TYPE_TRACK_BUTTON_CURVE))
			c->setColor(col_but_hover);
		c->drawStr(area.x1 + 5 + 17*4, area.y1 + 22-2, "☊"); // ... */
	}
}

void AudioViewTrack::setSolo(bool _solo)
{
	solo = _solo;
	view->renderer->allowTracks(view->get_playable_tracks());
}

