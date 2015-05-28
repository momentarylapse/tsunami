/*
 * AudioViewTrack.cpp
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#include "AudioViewTrack.h"
#include "AudioView.h"
#include "../Tsunami.h"
#include "../Data/AudioFile.h"
#include "../Audio/Synth/Synthesizer.h"

void DrawStrBg(HuiPainter *c, float x, float y, const string &str, const color &fg, const color &bg);

AudioViewTrack::AudioViewTrack(AudioView *_view, Track *_track)
{
	view = _view;
	track = _track;

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

void AudioViewTrack::drawBuffer(HuiPainter *c, const rect &r, BufferBox &b, double view_pos_rel, const color &col)
{
	msg_db_f("DrawBuffer", 1);

	// zero heights of both channels
	float y0r = r.y1 + r.height() / 4;
	float y0l = r.y1 + r.height() * 3 / 4;

	float hf = r.height() / 4;

	if (view->show_mono){
		y0r = r.y1 + r.height() / 2;
		hf *= 2;
	}


	int di = view->detail_steps;
	c->setColor(col);

	if (b.peaks.num <= 2){
		// no peaks yet...
		c->drawRect((b.offset - view_pos_rel) * view->view_zoom, r.y1, b.num * view->view_zoom, r.height());
		return;
	}

	int l = min(view->prefered_buffer_level - 1, b.peaks.num / 2);
	if (l >= 1){//f < MIN_MAX_FACTOR){
		draw_peak_buffer(c, r.width(), di, view_pos_rel, view->view_zoom, view->buffer_zoom_factor, hf, r.x1, y0r, b.peaks[l*2-2], b.offset);
		if (!view->show_mono)
			draw_peak_buffer(c, r.width(), di, view_pos_rel, view->view_zoom, view->buffer_zoom_factor, hf, r.x1, y0l, b.peaks[l*2-1], b.offset);
	}else{
		draw_line_buffer(c, r.width(), view_pos_rel, view->view_zoom, hf, r.x1, y0r, b.r, b.offset);
		if (!view->show_mono)
			draw_line_buffer(c, r.width(), view_pos_rel, view->view_zoom, hf, r.x1, y0l, b.l, b.offset);
	}
}

void AudioViewTrack::drawTrackBuffers(HuiPainter *c, const rect &r, double view_pos_rel)
{
	msg_db_f("DrawTrackBuffers", 1);

	// non-current levels
	foreachi(TrackLevel &lev, track->levels, level_no){
		if (level_no == view->cur_level)
			continue;
		foreach(BufferBox &b, lev.buffers)
			drawBuffer(c, r, b, view_pos_rel, view->colors.text_soft2);
	}

	// current
	foreach(BufferBox &b, track->levels[view->cur_level].buffers)
		drawBuffer(c, r, b, view_pos_rel, view->colors.text);
}

void AudioViewTrack::drawSampleFrame(HuiPainter *c, const rect &r, SampleRef *s, const color &col, int delay)
{
	// frame
	Range rr = s->getRange() + delay;
	int asx = clampi(view->sample2screen(rr.start()), r.x1, r.x2);
	int aex = clampi(view->sample2screen(rr.end()), r.x1, r.x2);

	if (delay == 0)
		s->area = rect(asx, aex, r.y1, r.y2);


	color col2 = col;
	col2.a *= 0.2f;
	c->setColor(col2);
	c->drawRect(asx, r.y1,                          aex - asx, view->SUB_FRAME_HEIGHT);
	c->drawRect(asx, r.y2 - view->SUB_FRAME_HEIGHT, aex - asx, view->SUB_FRAME_HEIGHT);

	c->setColor(col);
	c->drawLine(asx, r.y1, asx, r.y2);
	c->drawLine(aex, r.y1, aex, r.y2);
	c->drawLine(asx, r.y1, aex, r.y1);
	c->drawLine(asx, r.y2, aex, r.y2);
}

void AudioViewTrack::drawSample(HuiPainter *c, const rect &r, SampleRef *s)
{
	color col = view->colors.sample;
	//bool is_cur = ((s == cur_sub) and (t->IsSelected));
	if (!s->is_selected)
		col = view->colors.sample_selected;
	if (view->hover.sample == s)
		col = view->colors.sample_hover;
	//col.a = 0.2f;

	drawSampleFrame(c, r, s, col, 0);

	color col2 = col;
	col2.a *= 0.5f;
	for (int i=0;i<s->rep_num;i++)
		drawSampleFrame(c, r, s, col2, (i + 1) * s->rep_delay);

	// buffer
	drawBuffer(c, r, *s->buf, view->view_pos - (double)s->pos, col);
	drawMidi(c, r, *s->midi, s->pos);

	int asx = clampi(view->sample2screen(s->pos), r.x1, r.x2);
	if (s->is_selected)//((is_cur) or (a->sub_mouse_over == s))
		c->drawStr(asx, r.y2 - view->SUB_FRAME_HEIGHT, s->origin->name);
}

void AudioViewTrack::drawMarker(HuiPainter *c, const rect &r, TrackMarker &marker)
{
	int x = view->sample2screen(marker.pos);
	c->setColor(view->colors.text);
	c->drawStr(x, r.y1, marker.text);
}


void AudioViewTrack::drawMidi(HuiPainter *c, const rect &r, MidiData &midi, int shift)
{
	Range range = Range(view->screen2sample(r.x1) - shift, view->screen2sample(r.x2) - view->screen2sample(r.x1));
	Array<MidiNote> notes = midi.getNotes(range);
	c->setLineWidth(3.0f);
	foreach(MidiNote &n, notes){
		c->setColor(getPitchColor(n.pitch));
		float x1 = view->sample2screen(n.range.offset + shift);
		float x2 = view->sample2screen(n.range.end() + shift);
		x2 = max(x2, x1 + 4);
		float h = r.y2 - clampf((float)n.pitch / 80.0f - 0.3f, 0, 1) * r.height();
		//c->drawRect(rect(x1, x2, r.y1, r.y2));
		c->drawLine(x1, h, x2, h);
	}
	c->setLineWidth(view->LINE_WIDTH);
}

void draw_note(HuiPainter *c, const MidiNote &n, color &col, AudioView *v)
{
	float x1 = v->sample2screen(n.range.offset);
	float x2 = v->sample2screen(n.range.end());
	float xm = x1 * 0.8f + x2 * 0.2f;
	float y1 = v->pitch2y(n.pitch + 1);
	float y2 = v->pitch2y(n.pitch);
	c->setColor(col);
	c->drawRect(rect(x1, xm, y1, y2));
	color col2 = ColorInterpolate(col, Black, 0.1f);
	c->setColor(col2);
	c->drawRect(rect(xm, x2, y1, y2));
}

void AudioViewTrack::drawMidiEditable(HuiPainter *c, const rect &r, MidiData &midi)
{
	Array<MidiNote> notes = midi.getNotes(view->viewRange());
	foreachi(MidiNote &n, notes, i){
		if ((n.pitch < view->pitch_min) or (n.pitch >= view->pitch_max))
			continue;
		color col = getPitchColor(n.pitch);
		if ((view->hover.type == view->SEL_TYPE_MIDI_NOTE) and (n.range.offset == view->hover.note_start))
			col.a = 0.5f;
		draw_note(c, n, col, view);
	}
	if ((HuiGetEvent()->lbut) and (view->selection.type == view->SEL_TYPE_MIDI_PITCH)){
		Array<MidiNote> notes = view->getCreationNotes();
		foreach(MidiNote &n, notes){
			color col = getPitchColor(n.pitch);
			col.a = 0.5f;
			draw_note(c, n, col, view);
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
		c->drawStr(20, r.y1 + r.height() * (view->pitch_max - i - 1) / (view->pitch_max - view->pitch_min), name);
	}
}

void AudioViewTrack::drawTrack(HuiPainter *c, const rect &r, int track_no)
{
	msg_db_f("DrawTrack", 1);

	if ((view->cur_track == track) and (view->editingMidi()))
		drawMidiEditable(c, r, track->midi);
	else
		drawMidi(c, r, track->midi, 0);

	drawTrackBuffers(c, r, view->view_pos);

	foreach(SampleRef *s, track->samples)
		drawSample(c, r, s);

	foreach(TrackMarker &m, track->markers)
		drawMarker(c, r, m);

	if ((view->hover.track == track) and (view->mx < view->TRACK_HANDLE_WIDTH)){
		c->setColor(color(0.4f, 1, 1, 1));
		c->drawRect(0, r.y1, view->TRACK_HANDLE_WIDTH, r.height());
	}

	//c->setColor((track_no == a->CurTrack) ? Black : ColorWaveCur);
//	c->setColor(ColorWaveCur);
	c->setFont("", -1, (track == view->cur_track), false);
	DrawStrBg(c, r.x1 + 23, r.y1 + 3, track->getNiceName(), view->colors.text, view->colors.background_track);
	c->setFont("", -1, false, false);

	if (track->type == track->TYPE_TIME)
		c->drawImage(r.x1 + 5, r.y1 + 5, view->image_track_time);
	else if (track->type == track->TYPE_MIDI)
		c->drawImage(r.x1 + 5, r.y1 + 5, view->image_track_midi);
	else
		c->drawImage(r.x1 + 5, r.y1 + 5, view->image_track_audio);

	if ((track->muted) or (view->hover.show_track_controls == track))
		c->drawImage(r.x1 + 5, r.y1 + 22, track->muted ? view->image_muted : view->image_unmuted);
	if ((view->audio->tracks.num > 1) and (view->hover.show_track_controls == track))
		c->drawImage(r.x1 + 22, r.y1 + 22, view->image_solo);
}


