/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "AudioView.h"
#include "Mode/ViewMode.h"
#include "../Tsunami.h"
#include "../Data/Song.h"
#include "../Audio/Synth/Synthesizer.h"

AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track)
{
	view = _view;
	track = _track;
	reference_track = -1;

	area = rect(0, 0, 0, 0);
	height_min = height_wish = 0;
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

inline void draw_line_buffer(HuiPainter *c, int width, double view_pos, double zoom, float hf, float x, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	int i0 = max((double) x          / zoom + view_pos - offset    , 0);
	int i1 = min((double)(x + width) / zoom + view_pos - offset + 2, buf.num);
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

inline void draw_peak_buffer(HuiPainter *c, int width, int di, double view_pos_rel, double zoom, float f, float hf, float x, float y0, const string &buf, int offset)
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

void AudioViewTrack::drawBuffer(HuiPainter *c, BufferBox &b, double view_pos_rel, const color &col)
{
	msg_db_f("DrawBuffer", 1);

	float w = area.width();
	float h = area.height();
	float hf = h / 4;
	float x1 = area.x1;
	float y1 = area.y1;

	// zero heights of both channels
	float y0r = y1 + hf;
	float y0l = y1 + hf * 3;

	if (view->show_mono){
		y0r = y1 + h / 2;
		hf *= 2;
	}


	int di = view->detail_steps;
	c->setColor(col);

	// no peaks yet? -> show dummy
	if (b.peaks.num <= 4){
		c->drawRect((b.offset - view_pos_rel) * view->cam.scale, y1, b.num * view->cam.scale, h);
		return;
	}

	int l = min(view->prefered_buffer_level - 1, b.peaks.num / 4);
	if (l >= 1){//f < MIN_MAX_FACTOR){


		if ((view->peak_mode == BufferBox::PEAK_MAXIMUM) or (view->peak_mode == BufferBox::PEAK_BOTH)){
			double bzf = view->buffer_zoom_factor;
			int ll = l;
			if (view->peak_mode == BufferBox::PEAK_BOTH){
				color cc = col;
				cc.a *= 0.3f;
				c->setColor(cc);
				if (ll < b.peaks.num / 4){
					ll ++;
					bzf *= 2;
				}
			}else{
				c->setColor(col);
			}
			draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, bzf, hf, x1, y0r, b.peaks[ll*4-4], b.offset);
			if (!view->show_mono)
				draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, bzf, hf, x1, y0l, b.peaks[ll*4-3], b.offset);
		}

		if ((view->peak_mode == BufferBox::PEAK_SQUAREMEAN) or (view->peak_mode == BufferBox::PEAK_BOTH)){
			c->setColor(col);
			draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, view->buffer_zoom_factor, hf, x1, y0r, b.peaks[l*4-2], b.offset);
			if (!view->show_mono)
				draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, view->buffer_zoom_factor, hf, x1, y0l, b.peaks[l*4-1], b.offset);
		}
	}else{
		draw_line_buffer(c, w, view_pos_rel, view->cam.scale, hf, x1, y0r, b.r, b.offset);
		if (!view->show_mono)
			draw_line_buffer(c, w, view_pos_rel, view->cam.scale, hf, x1, y0l, b.l, b.offset);
	}
}

void AudioViewTrack::drawTrackBuffers(HuiPainter *c, double view_pos_rel)
{
	msg_db_f("DrawTrackBuffers", 1);

	// non-current levels
	foreachi(TrackLevel &lev, track->levels, level_no){
		if (level_no == view->cur_level)
			continue;
		foreach(BufferBox &b, lev.buffers)
			drawBuffer(c, b, view_pos_rel, view->colors.text_soft2);
	}

	// current
	foreach(BufferBox &b, track->levels[view->cur_level].buffers)
		drawBuffer(c, b, view_pos_rel, view->colors.text);
}

void AudioViewTrack::drawSampleFrame(HuiPainter *c, SampleRef *s, const color &col, int delay)
{
	// frame
	Range rr = s->getRange() + delay;
	int asx = clampi(view->cam.sample2screen(rr.start()), area.x1, area.x2);
	int aex = clampi(view->cam.sample2screen(rr.end()), area.x1, area.x2);

	if (delay == 0)
		s->area = rect(asx, aex, area.y1, area.y2);


	color col2 = col;
	col2.a *= 0.2f;
	c->setColor(col2);
	c->drawRect(asx, area.y1,                          aex - asx, view->SAMPLE_FRAME_HEIGHT);
	c->drawRect(asx, area.y2 - view->SAMPLE_FRAME_HEIGHT, aex - asx, view->SAMPLE_FRAME_HEIGHT);

	c->setColor(col);
	c->drawLine(asx, area.y1, asx, area.y2);
	c->drawLine(aex, area.y1, aex, area.y2);
	c->drawLine(asx, area.y1, aex, area.y1);
	c->drawLine(asx, area.y2, aex, area.y2);
}

void AudioViewTrack::drawSample(HuiPainter *c, SampleRef *s)
{
	color col = view->colors.sample;
	//bool is_cur = ((s == cur_sub) and (t->IsSelected));
	if (!s->is_selected)
		col = view->colors.sample_selected;
	if (view->hover.sample == s)
		col = view->colors.sample_hover;
	//col.a = 0.2f;

	drawSampleFrame(c, s, col, 0);

	color col2 = col;
	col2.a *= 0.5f;
	for (int i=0;i<s->rep_num;i++)
		drawSampleFrame(c, s, col2, (i + 1) * s->rep_delay);

	// buffer
	drawBuffer(c, *s->buf, view->cam.pos - (double)s->pos, col);
	if (s->midi)
		drawMidi(c, *s->midi, s->pos);

	int asx = clampi(view->cam.sample2screen(s->pos), area.x1, area.x2);
	if (s->is_selected)//((is_cur) or (a->sub_mouse_over == s))
		c->drawStr(asx, area.y2 - view->SAMPLE_FRAME_HEIGHT, s->origin->name);
}

void AudioViewTrack::drawMarker(HuiPainter *c, const TrackMarker &marker, int index, bool hover)
{
	float w = c->getStrWidth(marker.text);
	int x = view->cam.sample2screen(marker.pos);

	c->setColor(view->colors.background_track);
	rect bg = rect(x-2, x+w+2, area.y1, area.y1+18);
	c->drawRect(bg);
	marker_areas[index] = bg;

	c->setColor(view->colors.text);
	if (hover)
		c->setColor(view->colors.text_soft2);
	c->drawStr(x, area.y1, marker.text);
}


void AudioViewTrack::drawMidi(HuiPainter *c, const MidiNoteData &midi, int shift)
{
	if (view->midi_view_mode == view->VIEW_MIDI_DEFAULT)
		drawMidiDefault(c, midi, shift);
	else if ((view->midi_view_mode == view->VIEW_MIDI_TAB) and (track->instrument.tuning.num > 0))
		drawMidiTab(c, midi, shift);
	else // if (view->midi_view_mode == view->VIEW_MIDI_SCORE)
		drawMidiScore(c, midi, shift);
}

void AudioViewTrack::drawMidiDefault(HuiPainter *c, const MidiNoteData &midi, int shift)
{
	Range range = view->cam.range() - shift;
	Array<MidiNote> notes = midi.getNotes(range);
	c->setLineWidth(3.0f);
	foreach(MidiNote &n, notes){
		c->setColor(getPitchColor(n.pitch));
		float x1 = view->cam.sample2screen(n.range.offset + shift);
		float x2 = view->cam.sample2screen(n.range.end() + shift);
		x2 = max(x2, x1 + 4);
		float h = area.y2 - clampf((float)n.pitch / 80.0f - 0.3f, 0, 1) * area.height();
		//c->drawRect(rect(x1, x2, r.y1, r.y2));
		c->drawLine(x1, h, x2, h);
	}
	c->setLineWidth(view->LINE_WIDTH);
}

void get_tab(int &stringno, int &position, int pitch, Array<int> &tuning)
{
	stringno = 0;
	position = 0;
	for (int i=0; i<tuning.num; i++)
		if (pitch >= tuning[i]){
			stringno = i;
			position = pitch - tuning[i];
		}
}

void AudioViewTrack::drawMidiTab(HuiPainter *c, const MidiNoteData &midi, int shift)
{
	Range range = view->cam.range() - shift;
	Array<MidiNote> notes = midi.getNotes(range);



	c->setColor(view->colors.text);

	// clef lines
	float h = area.height() * 0.7f;
	float dy = h / track->instrument.tuning.num;
	float y0 = area.y2 - (area.height() - h) / 2 - dy/2;
	for (int i=0; i<track->instrument.tuning.num; i++){
		float y = y0 - i*dy;
		c->drawLine(area.x1, y, area.x2, y);
	}
	c->setAntialiasing(true);

	c->setFontSize(h / 6);
	c->drawStr(10, area.y1 + area.height() * 0.22f, "T\nA\nB");
	float r = min(dy/2, 8);
	c->setFontSize(r * 1.6f);

	foreach(MidiNote &n, notes){
		float x1 = view->cam.sample2screen(n.range.offset + shift);
		float x2 = view->cam.sample2screen(n.range.end() + shift);
		float x = x1 + r;

		int stringno, position;
		get_tab(stringno, position, n.pitch, track->instrument.tuning);

		float y = y0 - stringno * dy;

		color col = ColorInterpolate(getPitchColor(n.pitch), view->colors.text, 0.3f);

		// "shadow" to indicate length
		if (x2 - x1 > r*2){
			color col2 = col;
			col2.a *= 0.4f;
			c->setColor(col2);
			c->drawRect(x, y - r, x2 - x, r*2);
		}

		// the note circle
		col = ColorInterpolate(col, view->colors.background_track, 0.5f);
		//col.a *= 0.4f;
		c->setColor(col);
		if (x2 - x1 > 6)
			c->drawCircle(x, y, r);
		else
			c->drawRect(x - r, y - r, r*2, r*2);
		if (x2 - x1 > r/2){
			c->setColor(view->colors.text);
			c->drawStr(x1 + r/3, y - r * 1.20f, i2s(position));
		}

	}
	c->setAntialiasing(false);
	c->setFontSize(view->FONT_SIZE);
}

int get_score_position(int pitch, int clef, bool &sharp)
{
	if (clef == CLEF_TYPE_DRUMS){
		sharp = false;
		/*if (pitch == 35)	return "bass      (akk)";
		if (pitch == 36)	return "bass";
		if (pitch == 37)	return "side stick";
		if (pitch == 38)	return "snare";
		if (pitch == 39)	return "clap";
		if (pitch == 40)	return "snare     (electronic)";
		if (pitch == 41)	return "tom - floor low";
		if (pitch == 42)	return "hihat - closed";
		if (pitch == 43)	return "tom - floor hi";
		if (pitch == 44)	return "hihat - pedal";
		if (pitch == 45)	return "tom - low";
		if (pitch == 46)	return "hihat - open";
		if (pitch == 47)	return "tom - low mid";
		if (pitch == 48)	return "tom - hi mid";
		if (pitch == 49)	return "crash 1";
		if (pitch == 50)	return "tom - hi";
		if (pitch == 51)	return "ride 1";
		if (pitch == 52)	return "chinese";
		if (pitch == 53)	return "bell ride";
		if (pitch == 54)	return "tambourine";
		if (pitch == 55)	return "splash";
		if (pitch == 56)	return "cowbell";
		if (pitch == 57)	return "crash 2";
		if (pitch == 58)	return "vibraslash?";
		if (pitch == 59)	return "ride 2";
		if (pitch == 60)	return "bongo - hi";
		if (pitch == 61)	return "bongo - low";*/
		if ((pitch == 35) or (pitch == 36)) // bass
			return 1;
		if ((pitch == 38) or (pitch == 40)) // snare
			return 5;
		if ((pitch == 48) or (pitch == 50)) // tom hi
			return 9;
		if ((pitch == 45) or (pitch == 47)) // tom mid
			return 8;
		if ((pitch == 41) or (pitch == 43)) // tom floor
			return 3;
		sharp = true;
		return 0;
	}
	int octave = pitch_get_octave(pitch);
	int rel = pitch_to_rel(pitch);

	const int pp[12] = {0,0,1,1,2,3,3,4,4,5,5,6};
	const bool ss[12] = {false,true,false,true,false,false,true,false,true,false,true,false};
	const int clef_offset[4] = {5*7, 4*7, 3*7+2, 2*7+2};

	sharp = ss[rel];
	return pp[rel] + 7 * octave + 5 - clef_offset[clef];
}

float clef_pos_to_screen(int pos, rect &area, float dy)
{
	return area.y2 - area.height() / 2 - (pos - 4) * dy / 2;
}

void AudioViewTrack::drawMidiScore(HuiPainter *c, const MidiNoteData &midi, int shift)
{
	Range range = view->cam.range() - shift;
	Array<MidiNote> notes = midi.getNotes(range);

	int clef = track->instrument.get_clef();

	c->setColor(view->colors.text);

	// clef lines
	float dy = area.height() / 13;
	for (int i=0; i<10; i+=2){
		float y = clef_pos_to_screen(i, area, dy);
		c->drawLine(area.x1, y, area.x2, y);
	}
	c->setAntialiasing(true);

	c->setFontSize(dy*4);
	c->drawStr(10, clef_pos_to_screen(10, area, dy), clef_symbol(clef));
	c->setFontSize(dy);

	float r = dy/2;

	foreach(MidiNote &n, notes){
		float x1 = view->cam.sample2screen(n.range.offset + shift);
		float x2 = view->cam.sample2screen(n.range.end() + shift);
		float x = x1 + r;


		bool sharp;
		int p = get_score_position(n.pitch, clef, sharp);

		float y = clef_pos_to_screen(p, area, dy);

		// auxiliary lines
		for (int i=10; i<=p; i+=2){
			c->setColor(view->colors.text_soft1);
			float y = clef_pos_to_screen(i, area, dy);
			c->drawLine(x - dy, y, x + dy, y);
		}
		for (int i=-2; i>=p; i-=2){
			c->setColor(view->colors.text_soft1);
			float y = clef_pos_to_screen(i, area, dy);
			c->drawLine(x - dy, y, x + dy, y);
		}


		color col = ColorInterpolate(getPitchColor(n.pitch), view->colors.text, 0.3f);

		// "shadow" to indicate length
		if (x2 - x1 > r*2){
			color col2 = col;
			col2.a *= 0.4f;
			c->setColor(col2);
			c->drawRect(x, y - r, x2 - x, r*2);
		}

		// the note circle
		c->setColor(col);
		if (sharp)
			c->drawStr(x - 15, y - 8, "\u266F");
		if (x2 - x1 > 6)
			c->drawCircle(x, y, r);
		else
			c->drawRect(x - r, y - r, r*2, r*2);
	}
	c->setFontSize(view->FONT_SIZE);
	c->setAntialiasing(false);
}

void AudioViewTrack::draw(HuiPainter *c)
{
	view->mode->drawTrackData(c, this);

	drawHeader(c);
}

void AudioViewTrack::drawHeader(HuiPainter *c)
{
	if (view->hover.show_track_controls == track){
		c->setColor(color(0.4f, 1, 1, 1));
		c->drawRect(0, area.y1, view->TRACK_HANDLE_WIDTH, area.height());
	}

	// track title
	c->setFont("", -1, (track == view->cur_track), false);
	c->setFill(false);
	c->setLineWidth(3);
	c->setColor(view->colors.background_track);
	c->drawStr(area.x1 + 23, area.y1 + 3, track->getNiceName());
	c->setFill(true);
	c->setLineWidth(1);
	c->setColor(view->colors.text);
	c->drawStr(area.x1 + 23, area.y1 + 3, track->getNiceName());
	c->setFont("", -1, false, false);

	// icons
	if (track->type == track->TYPE_TIME){
		c->setColor(view->colors.background_track);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_time_bg);
		c->setColor(view->colors.text);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_time);
	}else if (track->type == track->TYPE_MIDI){
		c->setColor(view->colors.background_track);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_midi_bg);
		c->setColor(view->colors.text);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_midi);
	}else{
		c->setColor(view->colors.background_track);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_audio_bg);
		c->setColor(view->colors.text);
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, *view->images.track_audio);
	}

	c->setColor(view->colors.text);
	if ((track->muted) or (view->hover.show_track_controls == track)){
		c->setColor(view->colors.background_track);
		c->drawMaskImage(area.x1 + 5, area.y1 + 22, *view->images.speaker_bg);
		if (track->muted)
			c->drawMaskImage(area.x1 + 5, area.y1 + 22, *view->images.x_bg);
		c->setColor(view->colors.text);
		if (view->hover.type == Selection::TYPE_MUTE)
			c->setColor(view->colors.text_soft2);
		c->drawMaskImage(area.x1 + 5, area.y1 + 22, *view->images.speaker);
		if (track->muted)
			c->drawImage(area.x1 + 5, area.y1 + 22, *view->images.x);
	}
	if ((view->song->tracks.num > 1) and (view->hover.show_track_controls == track)){
		c->setColor(view->colors.background_track);
		c->drawMaskImage(area.x1 + 22, area.y1 + 22, *view->images.solo_bg);
		c->setColor(view->colors.text);
		if (view->hover.type == Selection::TYPE_SOLO)
			c->setColor(view->colors.text_soft2);
		c->drawMaskImage(area.x1 + 22, area.y1 + 22, *view->images.solo);
	}
}


