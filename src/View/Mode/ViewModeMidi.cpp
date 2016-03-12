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
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../../Audio/Renderer/SynthesizerRenderer.h"
#include "../../Data/Clef.h"
#include "../../TsunamiWindow.h"

void align_to_beats(Song *s, Range &r, int beat_partition);

const int PITCH_SHOW_COUNT = 30;

ViewModeMidi::ViewModeMidi(AudioView *view) :
	ViewModeDefault(view)
{
	pitch_min = 55;
	pitch_max = pitch_min + PITCH_SHOW_COUNT;
	beat_partition = 4;
	win->setInt("beat_partition", beat_partition);
	midi_mode = MIDI_MODE_NOTE;
	midi_interval = 3;
	chord_type = 0;
	chord_inversion = 0;
	modifier = MODIFIER_NONE;

	deleting = false;

	scroll_offset = 0;
	scroll_bar = rect(0, 0, 0, 0);
	track_rect = rect(0, 0, 0, 0);


	preview_renderer = new SynthesizerRenderer(NULL);
	preview_renderer->setAutoStop(true);
	preview_stream = new AudioStream(preview_renderer);
	preview_stream->setBufferSize(2048);
}

ViewModeMidi::~ViewModeMidi()
{
	delete(preview_stream);
	delete(preview_renderer);
}

void ViewModeMidi::onLeftButtonDown()
{
	ViewModeDefault::onLeftButtonDown();

	if (selection->type == Selection::TYPE_MIDI_NOTE){
		selection->track->deleteMidiNote(selection->index);
		hover->clear();
		deleting = true;
	}else if (selection->type == Selection::TYPE_MIDI_PITCH){
		preview_renderer->resetMidiData();
		preview_renderer->setSynthesizer(view->cur_track->synth);

		Array<int> pitch = getCreationPitch();
		MidiRawData midi;
		foreach(int p, pitch)
			midi.add(MidiEvent(0, p, 1));
		preview_renderer->feed(midi);
		preview_stream->play();
	}else if (selection->type == Selection::TYPE_SCROLL){
		scroll_offset = view->my - scroll_bar.y1;
	}
}

void ViewModeMidi::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();

	if (selection->type == Selection::TYPE_MIDI_PITCH){
		view->cur_track->addMidiNotes(getCreationNotes());

		preview_renderer->endAllNotes();
	}
	deleting = false;
}

void ViewModeMidi::onMouseMove()
{
	ViewModeDefault::onMouseMove();

	if (deleting){
		*hover = getHover();
		if ((hover->type == Selection::TYPE_MIDI_NOTE) and (hover->track == view->cur_track)){
			selection->track->deleteMidiNote(hover->index);
			hover->clear();
		}
	}

	// drag & drop
	if (selection->type == Selection::TYPE_MIDI_PITCH){
		view->forceRedraw();
	}else if (selection->type == Selection::TYPE_SCROLL){
		pitch_max = (track_rect.y2 + scroll_offset - view->my) / track_rect.height() * (MAX_PITCH - 1.0f);
		setPitchMin(pitch_max - PITCH_SHOW_COUNT);
	}
}

void ViewModeMidi::onKeyDown(int k)
{
	if (k == KEY_1){
		modifier = MODIFIER_NONE;
		view->notify(view->MESSAGE_SETTINGS_CHANGE);
	}else if (k == KEY_2){
		modifier = MODIFIER_SHARP;
		view->notify(view->MESSAGE_SETTINGS_CHANGE);
	}else if (k == KEY_3){
		modifier = MODIFIER_FLAT;
		view->notify(view->MESSAGE_SETTINGS_CHANGE);
	}else if (k == KEY_4){
		modifier = MODIFIER_NATURAL;
		view->notify(view->MESSAGE_SETTINGS_CHANGE);
	}else{
		ViewModeDefault::onKeyDown(k);
	}
}

void ViewModeMidi::updateTrackHeights()
{
	foreach(AudioViewTrack *t, view->vtrack){
		t->height_min = view->TIME_SCALE_HEIGHT;
		if (t->track->type == Track::TYPE_AUDIO){
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
		}else if (t->track->type == Track::TYPE_MIDI){
			if (t->track == view->cur_track){
				if (view->midi_view_mode == view->VIEW_MIDI_SCORE)
					t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * 4;
				else
					t->height_wish = 5000;
			}else{
				t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
			}
		}else{
			t->height_wish = view->TIME_SCALE_HEIGHT * 2;
		}
	}
}

void ViewModeMidi::onCurTrackChange()
{
	view->thm.dirty = true;
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

Array<int> ViewModeMidi::getCreationPitch()
{
	Array<int> pitch;
	if (midi_mode == MIDI_MODE_NOTE){
		pitch.add(selection->pitch);
	}else if (midi_mode == MIDI_MODE_INTERVAL){
		pitch.add(selection->pitch);
		if (midi_interval != 0)
			pitch.add(selection->pitch + midi_interval);
	}else if (midi_mode == MIDI_MODE_CHORD){
		pitch = chord_notes(chord_type, chord_inversion, selection->pitch);
	}
	return pitch;
}

MidiData ViewModeMidi::getCreationNotes()
{
	int start = min(mouse_possibly_selecting_start, selection->pos);
	int end = max(mouse_possibly_selecting_start, selection->pos);
	Range r = Range(start, end - start);

	// align to beats
	if (song->bars.num > 0)
		align_to_beats(song, r, beat_partition);

	Array<int> pitch = getCreationPitch();

	// collision?
	Range allowed = get_allowed_midi_range(view->cur_track, pitch, mouse_possibly_selecting_start);

	// create notes
	MidiData notes;
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
	if (view->midi_view_mode == view->VIEW_MIDI_SCORE){
		const Clef& clef = t->track->instrument.get_clef();
		int pos = t->screen_to_clef_pos(y);
		return clef.position_to_pitch(pos, view->midi_scale, modifier);
	}
	return pitch_min + ((t->area.y2 - y) * (pitch_max - pitch_min) / t->area.height());
}

float ViewModeMidi::pitch2y(int pitch)
{
	int ti = view->cur_track->get_index();
	AudioViewTrack *t = view->vtrack[ti];

	if (view->midi_view_mode == view->VIEW_MIDI_SCORE){
		int mod;
		const Clef& clef = t->track->instrument.get_clef();
		int p = clef.pitch_to_position(pitch, view->midi_scale, mod);
		return t->clef_pos_to_screen(p);
	}

	return t->area.y2 - t->area.height() * ((float)pitch - pitch_min) / (pitch_max - pitch_min);
}

void ViewModeMidi::setPitchMin(int pitch)
{
	pitch_min = clampi(pitch, 0, MAX_PITCH - 1 - PITCH_SHOW_COUNT);
	pitch_max = pitch_min + PITCH_SHOW_COUNT;
	view->forceRedraw();
}

void ViewModeMidi::setBeatPartition(int partition)
{
	beat_partition = partition;
	view->forceRedraw();
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
	if (view->midi_view_mode == view->VIEW_MIDI_SCORE)
		ViewModeDefault::drawTrackBackground(c, t);
	else
		drawTrackBackgroundDefault(c, t);
}

void ViewModeMidi::drawTrackBackgroundDefault(HuiPainter *c, AudioViewTrack *t)
{
	color cc = (view->sel.has(t->track)) ? view->colors.background_track_selected : view->colors.background_track;
	c->setColor(cc);
	c->drawRect(t->area);

	view->drawGridTime(c, t->area, cc, false);
	drawGridBars(c, t->area, cc, (t->track->type == Track::TYPE_TIME));

	if ((t->track == view->cur_track) and (t->track->type == Track::TYPE_MIDI)){
		// pitch grid
		c->setColor(color(0.25f, 0, 0, 0));
		for (int i=pitch_min; i<pitch_max; i++){
			float y0 = pitch2y(i + 1);
			float y1 = pitch2y(i);
			if (!view->midi_scale.contains(i)){
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
	if ((s.track) and (s.track->type == Track::TYPE_MIDI) and (s.track == view->cur_track)){
		if (scroll_bar.inside(view->mx, view->my)){
			s.type = Selection::TYPE_SCROLL;
			return s;
		}
		if (midi_mode != MIDI_MODE_SELECT){
			s.pitch = y2pitch(my);
			s.type = Selection::TYPE_MIDI_PITCH;
			s.index = randi(1000); // quick'n'dirty fix to force view update every time the mouse moves
			Array<MidiNote> notes = s.track->midi;
			foreachi(MidiNote &n, notes, i)
				if ((n.pitch == s.pitch) and (n.range.is_inside(s.pos))){
					s.index = i;
					s.type = Selection::TYPE_MIDI_NOTE;
					return s;
				}
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



void ViewModeMidi::drawMidiNote(HuiPainter *c, const MidiNote &n, int state)
{
	float x1 = cam->sample2screen(n.range.offset);
	float x2 = cam->sample2screen(n.range.end());
	float y1 = pitch2y(n.pitch + 1);
	float y2 = pitch2y(n.pitch);
	color col = AudioViewTrack::getPitchColor(n.pitch);
	if (state == AudioViewTrack::STATE_HOVER){
		col.a = 0.5f;
	}else if (state == AudioViewTrack::STATE_REFERENCE){
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

void ViewModeMidi::drawMidiEditable(HuiPainter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area)
{
	if (view->midi_view_mode == view->VIEW_MIDI_SCORE)
		drawMidiEditableScore(c, t, midi, as_reference, track, area);
	else
		drawMidiEditableDefault(c, t, midi, as_reference, track, area);
}

void ViewModeMidi::drawMidiEditableDefault(HuiPainter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area)
{
	Array<MidiEvent> events = midi.getEvents(view->cam.range());
	Array<MidiNote> notes = midi;//.getNotes(view->cam.range());

	track_rect = area;

	// draw notes
	foreachi(MidiNote &n, notes, i){
		if ((n.pitch < pitch_min) or (n.pitch >= pitch_max))
			continue;
		bool _hover = ((hover->type == Selection::TYPE_MIDI_NOTE) and (i == hover->index));
		if (as_reference){
			drawMidiNote(c, n, AudioViewTrack::STATE_REFERENCE);
		}else{
			drawMidiNote(c, n, _hover ? AudioViewTrack::STATE_HOVER : AudioViewTrack::STATE_DEFAULT);
		}
	}
	if (as_reference)
		return;

	// draw events
	foreach(MidiEvent &e, events)
		if ((e.pitch >= pitch_min) and (e.pitch < pitch_max))
			drawMidiEvent(c, e);
}

void ViewModeMidi::drawMidiEditableScore(HuiPainter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area)
{
	Array<MidiNote> notes = midi;//.getNotes(view->cam.range());

	const Clef& clef = track->instrument.get_clef();

	t->drawMidiScoreClef(c, clef, view->midi_scale);
	//const int *mod = view->midi_scale.get_modifiers_clef();

	c->setAntialiasing(true);

	/*for (int i=0; i<7; i++)
		if (mod[i] != MODIFIER_NONE)
			c->drawStr(100, t->clef_pos_to_screen(i), modifier_symbol(mod[i]));*/

	// draw notes
	foreachi(MidiNote &n, notes, i){
		bool _hover = ((hover->type == Selection::TYPE_MIDI_NOTE) and (i == hover->index));
		if (as_reference){
			t->drawMidiNoteScore(c, n, 0, AudioViewTrack::STATE_REFERENCE, clef);
		}else{
			t->drawMidiNoteScore(c, n, 0, _hover ? AudioViewTrack::STATE_HOVER : AudioViewTrack::STATE_DEFAULT, clef);
		}
	}

	c->setFontSize(view->FONT_SIZE);
	c->setAntialiasing(false);
}

void ViewModeMidi::drawTrackData(HuiPainter *c, AudioViewTrack *t)
{
	// midi
	if ((view->cur_track == t->track) and (t->track->type == Track::TYPE_MIDI)){
		foreach(int n, t->reference_tracks)
			if ((n >= 0) and (n < song->tracks.num) and (n != t->track->get_index()))
				drawMidiEditable(c, t, song->tracks[n]->midi, true, t->track, t->area);
		drawMidiEditable(c, t, t->track->midi, false, t->track, t->area);


		// current creation
		if ((HuiGetEvent()->lbut) and (selection->type == Selection::TYPE_MIDI_PITCH)){
			Array<MidiNote> notes = getCreationNotes();
			foreach(MidiNote &n, notes){
				if (view->midi_view_mode == view->VIEW_MIDI_SCORE)
					t->drawMidiNoteScore(c, n, 0, AudioViewTrack::STATE_HOVER, t->track->instrument.get_clef());
				else
					drawMidiNote(c, n, AudioViewTrack::STATE_HOVER);
			}
			c->setFontSize(view->FONT_SIZE);
		}


		if (view->midi_view_mode == view->VIEW_MIDI_SCORE){
			if ((!HuiGetEvent()->lbut) and (hover->type == Selection::TYPE_MIDI_PITCH)){
				Range r = Range(hover->pos, 0);
				align_to_beats(song, r, beat_partition);
				const Clef &clef = t->track->instrument.get_clef();
				t->drawMidiNoteScore(c, MidiNote(r, hover->pitch, 1), 0, t->STATE_HOVER, clef);

				float x = view->cam.sample2screen(r.offset);
				int mod;
				int p = clef.pitch_to_position(hover->pitch, view->midi_scale, mod);
				float y = t->clef_pos_to_screen(p);
				c->setColor(view->colors.text);
				c->setFontSize(view->FONT_SIZE);
				c->drawStr(x, y + 30, pitch_name(hover->pitch));
			}

		}else{
		// pitch names
		color cc = view->colors.text;
		cc.a = 0.4f;
		Array<SampleRef*> *p = NULL;
		if ((t->track->synth) and (t->track->synth->name == "Sample")){
			PluginData *c = t->track->synth->get_config();
			p = (Array<SampleRef*> *)&c[1];
		}
		bool is_drum = (t->track->instrument.type == Instrument::TYPE_DRUMS);
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
			c->drawStr(20, t->area.y1 + t->area.height() * (pitch_max - i - 1) / PITCH_SHOW_COUNT, name);
		}


		// scrollbar
		if (hover->type == Selection::TYPE_SCROLL)
			c->setColor(view->colors.text);
		else
			c->setColor(view->colors.text_soft1);
		scroll_bar = rect(t->area.x2 - 40, t->area.x2 - 20, t->area.y2 - t->area.height() * pitch_max / (MAX_PITCH - 1.0f), t->area.y2 - t->area.height() * pitch_min / (MAX_PITCH - 1.0f));
		c->drawRect(scroll_bar);
		}
	}else{
		if ((t->track->type == Track::TYPE_MIDI) or (t->track->midi.num > 0))
			t->drawMidi(c, t->track->midi, 0);
	}

	// audio buffer
	t->drawTrackBuffers(c, view->cam.pos);

	// samples
	foreach(SampleRef *s, t->track->samples)
		t->drawSample(c, s);

	// marker
	t->marker_areas.resize(t->track->markers.num);
	foreachi(TrackMarker &m, t->track->markers, i)
		if (!m.text.match(":*:"))
			t->drawMarker(c, m, i, (view->hover.type == Selection::TYPE_MARKER) and (view->hover.track == t->track) and (view->hover.index == i));
}
