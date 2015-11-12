/*
 * ViewModeMidi.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeMidi.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Audio/AudioStream.h"
#include "../../Action/Track/Sample/ActionTrackMoveSample.h"
#include "../../Audio/AudioStream.h"
#include "../../Audio/SongRenderer.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Audio/Synth/SynthesizerRenderer.h"
#include "../../TsunamiWindow.h"

void align_to_beats(Song *s, Range &r, int beat_partition);

ViewModeMidi::ViewModeMidi(AudioView *view, ViewMode *parent) :
	ViewModeDefault(view, parent)
{
	pitch_min = 60;
	pitch_max = 90;
	beat_partition = 4;
	win->setInt("beat_partition", beat_partition);
	midi_scale = 0;
	midi_mode = MIDI_MODE_NOTE;
	chord_type = 0;
	chord_inversion = 0;
}

ViewModeMidi::~ViewModeMidi()
{
}


void deleteMidiNote(Track *t, int pitch, int start)
{
	foreachi(MidiNote &n, t->midi, i)
		if ((n.pitch == pitch) and (n.range.offset == start))
			t->deleteMidiNote(i);
}

void ViewModeMidi::onLeftButtonDown()
{
	selectUnderMouse();
	view->updateMenu();

	setBarriers(selection);

	applyBarriers(selection->pos);
	mouse_possibly_selecting_start = selection->pos;

	// selection:
	//   start after lb down and moving
	if ((selection->type == Selection::TYPE_TRACK) or (selection->type == Selection::TYPE_TIME)){
		setCursorPos(selection->pos);
	}else if (selection->type == Selection::TYPE_SELECTION_START){
		// swap end / start
		selection->type = Selection::TYPE_SELECTION_END;
		hover->type = Selection::TYPE_SELECTION_END;
		view->sel_raw.invert();
	}else if (selection->type == Selection::TYPE_MUTE){
		selection->track->setMuted(!selection->track->muted);
	}else if (selection->type == Selection::TYPE_SOLO){
		foreach(Track *t, song->tracks)
			t->is_selected = (t == selection->track);
		if (selection->track->muted)
			selection->track->setMuted(false);
	}else if (selection->type == Selection::TYPE_SAMPLE){
		cur_action = new ActionTrackMoveSample(view->song);
	}else if (selection->type == Selection::TYPE_MIDI_NOTE){
		deleteMidiNote(view->cur_track, selection->pitch, selection->note_start);
	}else if (selection->type == Selection::TYPE_MIDI_PITCH){
		view->midi_preview_renderer->resetMidiData();
		view->midi_preview_renderer->setSynthesizer(view->cur_track->synth);

		Array<int> pitch = GetChordNotes((midi_mode == MIDI_MODE_CHORD) ? chord_type : -1, chord_inversion, selection->pitch);
		MidiRawData midi;
		foreach(int p, pitch)
			midi.add(MidiEvent(0, p, 1));
		view->midi_preview_renderer->feed(midi);
		view->midi_preview_stream->play();
	}

	view->forceRedraw();
	view->updateMenu();
}

void ViewModeMidi::onLeftButtonUp()
{
	if (selection->type == Selection::TYPE_SAMPLE){
		if (cur_action)
			song->execute(cur_action);
	}else if (selection->type == Selection::TYPE_MIDI_PITCH){
		view->cur_track->addMidiNotes(getCreationNotes());

		view->midi_preview_renderer->endAllNotes();
	}
	cur_action = NULL;

	view->notify(view->MESSAGE_MOUSE_UP);

	// TODO !!!!!!!!
	selection->type = Selection::TYPE_NONE;
	view->forceRedraw();
	view->updateMenu();
}


Range get_allowed_midi_range(Track *t, Array<int> pitch, int start)
{
	Range allowed = Range::ALL;
	foreach(MidiNote &n, t->midi){
		foreach(int p, pitch)
			if (n.pitch == p){
				if (n.range.is_inside(start))
					return Range::EMPTY;
			}
	}

	MidiRawData midi = midi_notes_to_events(t->midi);
	foreach(MidiEvent &e, midi)
		foreach(int p, pitch)
			if (e.pitch == p){
				if ((e.pos >= start) and (e.pos < allowed.end()))
					allowed.set_end(e.pos);
				if ((e.pos < start) and (e.pos >= allowed.start()))
					allowed.set_start(e.pos);
			}
	return allowed;
}

bool ViewModeMidi::is_sharp(int pitch)
{
	int r = (pitch - midi_scale + 12) % 12;
	// 69 = 9 = a
	return ((r == 10) or (r == 1) or (r == 3) or (r == 6) or (r == 8));
}

MidiNoteData ViewModeMidi::getCreationNotes()
{
	int start = min(mouse_possibly_selecting_start, selection->pos);
	int end = max(mouse_possibly_selecting_start, selection->pos);
	Range r = Range(start, end - start);

	// align to beats
	if (song->bars.num > 0)
		align_to_beats(song, r, beat_partition);

	Array<int> pitch = GetChordNotes((midi_mode == MIDI_MODE_CHORD) ? chord_type : -1, chord_inversion, selection->pitch);

	// collision?
	Range allowed = get_allowed_midi_range(view->cur_track, pitch, mouse_possibly_selecting_start);

	// create notes
	MidiNoteData notes;
	if (allowed.empty())
		return notes;
	foreach(int p, pitch)
		notes.add(MidiNote(r and allowed, p, 1));
	return notes;
}

int ViewModeMidi::y2pitch(int y)
{
	int ti = view->cur_track->get_index();
	AudioViewTrack *t = view->vtrack[ti];
	return pitch_min + ((t->area.y2 - y) * (pitch_max - pitch_min) / t->area.height());
}

float ViewModeMidi::pitch2y(int p)
{
	int ti = view->cur_track->get_index();
	AudioViewTrack *t = view->vtrack[ti];
	return t->area.y2 - t->area.height() * ((float)p - pitch_min) / (pitch_max - pitch_min);
}

void ViewModeMidi::drawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time)
{
	if (song->bars.num == 0)
		return;
	int s0 = cam->screen2sample(r.x1 - 1);
	int s1 = cam->screen2sample(r.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash, no_dash;
	dash.add(6);
	dash.add(4);
	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar> bars = song->bars.getBars(Range(s0, s1 - s0));
	foreach(Bar &b, bars){
		int xx = cam->sample2screen(b.range.offset);

		float dx_bar = cam->dsample2screen(b.range.num);
		float dx_beat = dx_bar / b.num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b.index % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			c->setColor(ColorInterpolate(bg, view->colors.text_soft1, f1));
			c->setLineDash(no_dash, r.y1);
			c->drawLine(xx, r.y1, xx, r.y2);
		}

		if (f2 >= 0.1f){
			color c1 = ColorInterpolate(bg, view->colors.text_soft1, f2);
			float beat_length = (float)b.range.num / (float)b.num_beats;
			c->setLineDash(dash, r.y1);
			for (int i=0; i<b.num_beats; i++){
				float beat_offset = b.range.offset + (float)i * beat_length;
				color c2 = ColorInterpolate(bg, c1, 0.6f);
				c->setColor(c2);
				for (int j=1; j<beat_partition; j++){
					int x = cam->sample2screen(beat_offset + beat_length * j / beat_partition);
					c->drawLine(x, r.y1, x, r.y2);
				}
				if (i == 0)
					continue;
				c->setColor(c1);
				int x = cam->sample2screen(beat_offset);
				c->drawLine(x, r.y1, x, r.y2);
			}
		}

		if ((show_time) and (f1 > 0.9f)){
			c->setColor(view->colors.text_soft1);
			c->drawStr(xx + 2, r.y1, i2s(b.index + 1));
		}
	}
	c->setLineDash(no_dash, 0);
	c->setLineWidth(view->LINE_WIDTH);
}

void ViewModeMidi::drawTrackBackground(HuiPainter *c, AudioViewTrack *t)
{
	color cc = (t->track->is_selected) ? view->colors.background_track_selected : view->colors.background_track;
	c->setColor(cc);
	c->drawRect(t->area);

	view->drawGridTime(c, t->area, cc, false);
	drawGridBars(c, t->area, cc, (t->track->type == Track::TYPE_TIME));

	if (t->track == view->cur_track){
		// pitch grid
		c->setColor(color(0.25f, 0, 0, 0));
		for (int i=pitch_min; i<pitch_max; i++){
			float y0 = pitch2y(i + 1);
			float y1 = pitch2y(i);
			if (is_sharp(i)){
				c->setColor(color(0.2f, 0, 0, 0));
				c->drawRect(t->area.x1, y0, t->area.width(), y1 - y0);
			}
		}
	}
}

Selection ViewModeMidi::getHover()
{
	Selection s;
	int mx = view->mx;
	int my = view->my;

	// track?
	foreachi(AudioViewTrack *t, view->vtrack, i){
		if (view->mouseOverTrack(t)){
			s.vtrack = t;
			s.index = i;
			s.track = t->track;
			s.type = Selection::TYPE_TRACK;
			if (view->mx < t->area.x1 + view->TRACK_HANDLE_WIDTH)
				s.show_track_controls = t->track;
		}
	}

	// selection boundaries?
	view->selectionUpdatePos(s);
	if (view->mouse_over_time(view->sel_raw.end())){
		s.type = Selection::TYPE_SELECTION_END;
		return s;
	}
	if (view->mouse_over_time(view->sel_raw.start())){
		s.type = Selection::TYPE_SELECTION_START;
		return s;
	}
	if (view->stream->isPlaying()){
		if (view->mouse_over_time(view->stream->getPos())){
			s.type = Selection::TYPE_PLAYBACK;
			return s;
		}
	}

	// mute button?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		if ((mx >= t->area.x1 + 5) and (mx < t->area.x1 + 17) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_MUTE;
			return s;
		}
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){

		// markers
		for (int i=0; i<min(s.track->markers.num, view->vtrack[s.index]->marker_areas.num); i++){
			if (view->vtrack[s.index]->marker_areas[i].inside(mx, my)){
				s.type = Selection::TYPE_MARKER;
				s.index = i;
				return s;
			}
		}

		// TODO: prefer selected subs
		foreach(SampleRef *ss, s.track->samples){
			int offset = view->mouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = Selection::TYPE_SAMPLE;
				s.sample_offset = offset;
				return s;
			}
		}
	}

	// midi
	if ((s.track) and (s.track == view->cur_track) and (midi_mode != MIDI_MODE_SELECT)){
		s.pitch = y2pitch(my);
		s.type = Selection::TYPE_MIDI_PITCH;
		Array<MidiNote> notes = s.track->midi.getNotes(cam->range());
		foreach(MidiNote &n, notes)
			if ((n.pitch == s.pitch) and (n.range.is_inside(s.pos))){
				s.note_start = n.range.offset;
				s.type = Selection::TYPE_MIDI_NOTE;
				return s;
			}
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::TYPE_TIME;
		return s;
	}

	// track handle
	if ((s.track) and (mx < view->area.x1 + view->TRACK_HANDLE_WIDTH)){
		s.type = Selection::TYPE_TRACK_HANDLE;
		return s;
	}

	return s;
}



void ViewModeMidi::drawMidiNote(HuiPainter *c, const MidiNote &n, MidiNoteState state)
{
	float x1 = cam->sample2screen(n.range.offset);
	float x2 = cam->sample2screen(n.range.end());
	float y1 = pitch2y(n.pitch + 1);
	float y2 = pitch2y(n.pitch);
	color col = AudioViewTrack::getPitchColor(n.pitch);
	if (state == STATE_HOVER){
		col.a = 0.5f;
	}else if (state == STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.text_soft2, 0.7f);
		col.a = 0.5f;
	}
	c->setColor(col);
	c->drawRect(rect(x1, x2, y1, y2));
}

void ViewModeMidi::drawMidiEvent(HuiPainter *c, const MidiEvent &e)
{
	float x = cam->sample2screen(e.pos);
	float y1 = pitch2y(e.pitch + 1);
	float y2 = pitch2y(e.pitch);
	color col = AudioViewTrack::getPitchColor(e.pitch);
	col = ColorInterpolate(col, view->colors.text, 0.5f);
	c->setColor(col);
	c->drawRect(rect(x-1.5f, x+1.5f, y1, y2));
}

void ViewModeMidi::drawMidiEditable(HuiPainter *c, const MidiNoteData &midi, bool as_reference, Track *track, const rect &area)
{
	Array<MidiEvent> events = midi.getEvents(view->cam.range());
	Array<MidiNote> notes = midi.getNotes(view->cam.range());

	// draw notes
	foreachi(MidiNote &n, notes, i){
		if ((n.pitch < pitch_min) or (n.pitch >= pitch_max))
			continue;
		bool _hover = ((hover->type == Selection::TYPE_MIDI_NOTE) and (n.range.offset == hover->note_start) and (n.pitch == hover->pitch));
		if (as_reference){
			drawMidiNote(c, n, STATE_REFERENCE);
		}else{
			drawMidiNote(c, n, _hover ? STATE_HOVER : STATE_DEFAULT);
		}
	}
	if (as_reference)
		return;

	// draw events
	foreach(MidiEvent &e, events)
		drawMidiEvent(c, e);

	// current creation
	if ((HuiGetEvent()->lbut) and (selection->type == Selection::TYPE_MIDI_PITCH)){
		Array<MidiNote> notes = view->mode_midi->getCreationNotes();
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
	for (int i=pitch_min; i<pitch_max; i++){
		c->setColor(cc);
		if (((hover->type == Selection::TYPE_MIDI_PITCH) or (hover->type == Selection::TYPE_MIDI_NOTE)) and (i == hover->pitch))
			c->setColor(view->colors.text);

		string name = pitch_name(i);
		if (is_drum){
			name = drum_pitch_name(i);
		}else if (p){
			if (i < p->num)
				if ((*p)[i])
					name = (*p)[i]->origin->name;
		}
		c->drawStr(20, area.y1 + area.height() * (pitch_max - i - 1) / (pitch_max - pitch_min), name);
	}
}

void ViewModeMidi::drawTrackData(HuiPainter *c, AudioViewTrack *t)
{
	// midi
	if (view->cur_track == t->track){
		if ((t->reference_track >= 0) and (t->reference_track < song->tracks.num))
			drawMidiEditable(c, song->tracks[t->reference_track]->midi, true, t->track, t->area);
		drawMidiEditable(c, t->track->midi, false, t->track, t->area);
	}else{
		t->drawMidi(c, t->track->midi, 0);
	}

	// audio buffer
	t->drawTrackBuffers(c, view->cam.pos);

	foreach(SampleRef *s, t->track->samples)
		t->drawSample(c, s);

	// marker
	t->marker_areas.resize(t->track->markers.num);
	foreachi(TrackMarker &m, t->track->markers, i)
		if (!m.text.match(":*:"))
			t->drawMarker(c, m, i, (view->hover.type == Selection::TYPE_MARKER) and (view->hover.track == t->track) and (view->hover.index == i));
}
