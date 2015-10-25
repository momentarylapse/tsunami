/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "AudioView.h"
#include "../Tsunami.h"
#include "../Data/Song.h"
#include "../Audio/Synth/Synthesizer.h"

void DrawStrBg(HuiPainter *c, float x, float y, const string &str, const color &fg, const color &bg);

AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track)
{
	view = _view;
	track = _track;
	reference_track = -1;

	area = rect(0, 0, 0, 0);
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
	if (b.peaks.num <= 2){
		c->drawRect((b.offset - view_pos_rel) * view->cam.scale, y1, b.num * view->cam.scale, h);
		return;
	}

	int l = min(view->prefered_buffer_level - 1, b.peaks.num / 2);
	if (l >= 1){//f < MIN_MAX_FACTOR){
		draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, view->buffer_zoom_factor, hf, x1, y0r, b.peaks[l*2-2], b.offset);
		if (!view->show_mono)
			draw_peak_buffer(c, w, di, view_pos_rel, view->cam.scale, view->buffer_zoom_factor, hf, x1, y0l, b.peaks[l*2-1], b.offset);
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
	drawMidi(c, *s->midi, s->pos);

	int asx = clampi(view->cam.sample2screen(s->pos), area.x1, area.x2);
	if (s->is_selected)//((is_cur) or (a->sub_mouse_over == s))
		c->drawStr(asx, area.y2 - view->SAMPLE_FRAME_HEIGHT, s->origin->name);
}

void AudioViewTrack::drawMarker(HuiPainter *c, const TrackMarker &marker, int index)
{
	float w = c->getStrWidth(marker.text);
	int x = view->cam.sample2screen(marker.pos);

	c->setColor(view->colors.background);
	rect bg = rect(x-2, x+w+2, area.y1, area.y1+18);
	c->drawRect(bg);
	marker_areas[index] = bg;

	c->setColor(view->colors.text);
	c->drawStr(x, area.y1, marker.text);
}


void AudioViewTrack::drawMidi(HuiPainter *c, const MidiNoteData &midi, int shift)
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

void AudioViewTrack::drawMidiNote(HuiPainter *c, const MidiNote &n, MidiNoteState state)
{
	float x1 = view->cam.sample2screen(n.range.offset);
	float x2 = view->cam.sample2screen(n.range.end());
	float y1 = view->pitch2y(n.pitch + 1);
	float y2 = view->pitch2y(n.pitch);
	color col = getPitchColor(n.pitch);
	if (state == STATE_HOVER){
		col.a = 0.5f;
	}else if (state == STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.text_soft2, 0.7f);
		col.a = 0.5f;
	}
	c->setColor(col);
	c->drawRect(rect(x1, x2, y1, y2));
}

void AudioViewTrack::drawMidiEvent(HuiPainter *c, const MidiEvent &e)
{
	float x = view->cam.sample2screen(e.pos);
	float y1 = view->pitch2y(e.pitch + 1);
	float y2 = view->pitch2y(e.pitch);
	color col = getPitchColor(e.pitch);
	col = ColorInterpolate(col, view->colors.text, 0.5f);
	c->setColor(col);
	c->drawRect(rect(x-1.5f, x+1.5f, y1, y2));
}

void AudioViewTrack::drawMidiEditable(HuiPainter *c, const MidiNoteData &midi, bool as_reference)
{
	Array<MidiEvent> events = midi.getEvents(view->cam.range());
	Array<MidiNote> notes = midi.getNotes(view->cam.range());

	// draw notes
	foreachi(MidiNote &n, notes, i){
		if ((n.pitch < view->pitch_min) or (n.pitch >= view->pitch_max))
			continue;
		bool hover = ((view->hover.type == view->SEL_TYPE_MIDI_NOTE) and (n.range.offset == view->hover.note_start) and (n.pitch == view->hover.pitch));
		if (as_reference){
			drawMidiNote(c, n, STATE_REFERENCE);
		}else{
			drawMidiNote(c, n, hover ? STATE_HOVER : STATE_DEFAULT);
		}
	}
	if (as_reference)
		return;

	// draw events
	foreach(MidiEvent &e, events)
		drawMidiEvent(c, e);

	// current creation
	if ((HuiGetEvent()->lbut) and (view->selection.type == view->SEL_TYPE_MIDI_PITCH)){
		Array<MidiNote> notes = view->getCreationNotes();
		foreach(MidiNote &n, notes){
			drawMidiNote(c, n, STATE_HOVER);
		}
	}

	color cc = view->colors.text;
	cc.a = 0.4f;
	Array<SampleRef*> *p = NULL;
	if ((track->synth) and (track->synth->name == "Sample")){
		PluginData *c = track->synth->get_config();
		p = (Array<SampleRef*> *)&c[1];
	}
	bool is_drum = ((track->synth) and (track->synth->name == "Drumset"));
	for (int i=view->pitch_min; i<view->pitch_max; i++){
		c->setColor(cc);
		if (((view->hover.type == view->SEL_TYPE_MIDI_PITCH) or (view->hover.type == view->SEL_TYPE_MIDI_NOTE)) and (i == view->hover.pitch))
			c->setColor(view->colors.text);

		string name = pitch_name(i);
		if (is_drum){
			name = drum_pitch_name(i);
		}else if (p){
			if (i < p->num)
				if ((*p)[i])
					name = (*p)[i]->origin->name;
		}
		c->drawStr(20, area.y1 + area.height() * (view->pitch_max - i - 1) / (view->pitch_max - view->pitch_min), name);
	}
}

void AudioViewTrack::draw(HuiPainter *c, int track_no)
{
	msg_db_f("DrawTrack", 1);

	if ((view->cur_track == track) and (view->editingMidi())){
		if ((reference_track >= 0) and (reference_track < track->song->tracks.num))
			drawMidiEditable(c, track->song->tracks[reference_track]->midi, true);
		drawMidiEditable(c, track->midi, false);
	}else{
		drawMidi(c, track->midi, 0);
	}

	drawTrackBuffers(c, view->cam.pos);

	foreach(SampleRef *s, track->samples)
		drawSample(c, s);

	marker_areas.resize(track->markers.num);
	foreachi(TrackMarker &m, track->markers, i)
		drawMarker(c, m, i);

	if ((view->hover.track == track) and (view->mx < view->TRACK_HANDLE_WIDTH)){
		c->setColor(color(0.4f, 1, 1, 1));
		c->drawRect(0, area.y1, view->TRACK_HANDLE_WIDTH, area.height());
	}

	//c->setColor((track_no == a->CurTrack) ? Black : ColorWaveCur);
//	c->setColor(ColorWaveCur);
	c->setFont("", -1, (track == view->cur_track), false);
	DrawStrBg(c, area.x1 + 23, area.y1 + 3, track->getNiceName(), view->colors.text, view->colors.background_track);
	c->setFont("", -1, false, false);

	if (track->type == track->TYPE_TIME)
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, view->image_track_time);
	else if (track->type == track->TYPE_MIDI)
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, view->image_track_midi);
	else
		c->drawMaskImage(area.x1 + 5, area.y1 + 5, view->image_track_audio);

	if ((track->muted) or (view->hover.show_track_controls == track))
		c->drawMaskImage(area.x1 + 5, area.y1 + 22, track->muted ? view->image_muted : view->image_unmuted);
	if ((view->song->tracks.num > 1) and (view->hover.show_track_controls == track))
		c->drawMaskImage(area.x1 + 22, area.y1 + 22, view->image_solo);
}


