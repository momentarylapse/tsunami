/*
 * ViewModeMidi.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeMidi.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Device/OutputStream.h"
#include "../../Audio/Renderer/MidiRenderer.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../../Midi/Clef.h"
#include "../../Midi/MidiSource.h"
#include "../../Data/SongSelection.h"
#include "../../TsunamiWindow.h"

void align_to_beats(Song *s, Range &r, int beat_partition);

const int PITCH_SHOW_COUNT = 30;

class MidiPreviewSource : public MidiSource
{
public:
	MidiPreviewSource()
	{
		started = ended = false;
		end_of_stream = false;
	}
	virtual int _cdecl read(MidiRawData &midi)
	{
		if (end_of_stream)
			return 0;
		if (started){
			for (int p: pitch)
				midi.add(MidiEvent(0, p, 1));
			started = false;
		}
		if (ended){
			for (int p: pitch)
				midi.add(MidiEvent(0, p, 0));
			ended = false;
			end_of_stream = true;
		}
		return midi.samples;
	}

	void start(const Array<int> &_pitch)
	{
		if (ended)
			return;
		pitch = _pitch;
		started = true;
		end_of_stream = false;
	}
	void end()
	{
		ended = true;
	}

	bool started, ended;
	bool end_of_stream;

	Array<int> pitch;
};

static MidiPreviewSource *preview_source;

ViewModeMidi::ViewModeMidi(AudioView *view) :
	ViewModeDefault(view)
{
	pitch_min = 55;
	pitch_max = pitch_min + PITCH_SHOW_COUNT;
	beat_partition = 4;
	win->setInt("beat_partition", beat_partition);
	mode_wanted = AudioView::MIDI_MODE_CLASSICAL;
	creation_mode = CREATION_MODE_NOTE;
	midi_interval = 3;
	chord_type = 0;
	chord_inversion = 0;
	modifier = MODIFIER_NONE;

	deleting = false;

	scroll_offset = 0;
	scroll_bar = rect(0, 0, 0, 0);
	track_rect = rect(0, 0, 0, 0);

	preview_source = new MidiPreviewSource;

	preview_synth = NULL;
	preview_renderer = new MidiRenderer(preview_synth, preview_source);
	preview_stream = new OutputStream(preview_renderer);
	preview_stream->setBufferSize(2048);
}

ViewModeMidi::~ViewModeMidi()
{
	delete preview_stream;
	delete preview_renderer;
	if (preview_synth)
		delete preview_synth;
}

void ViewModeMidi::setMode(int _mode)
{
	mode_wanted = _mode;
	view->forceRedraw();
}

void ViewModeMidi::setCreationMode(int _mode)
{
	creation_mode = _mode;
	//view->forceRedraw();
}

void ViewModeMidi::onLeftButtonDown()
{
	ViewModeDefault::onLeftButtonDown();

	if (selection->type == Selection::TYPE_MIDI_NOTE){
		selection->track->deleteMidiNote(selection->index);
		hover->clear();
		deleting = true;
	}else if (selection->type == Selection::TYPE_MIDI_PITCH){
		if (preview_synth)
			delete preview_synth;
		preview_synth = (Synthesizer*)view->cur_track->synth->copy();
		preview_renderer->setSynthesizer(preview_synth);

		preview_source->start(getCreationPitch());
		if (!preview_stream->isPlaying())
			preview_stream->play();
	}else if (selection->type == Selection::TYPE_SCROLL){
		scroll_offset = view->my - scroll_bar.y1;
	}
}

void ViewModeMidi::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();

	if (selection->type == Selection::TYPE_MIDI_PITCH){
		view->song->action_manager->beginActionGroup();
		Array<MidiNote> notes = getCreationNotes();
		for (MidiNote &n: notes)
			view->cur_track->addMidiNote(n);
		view->song->action_manager->endActionGroup();

		preview_source->end();
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
	for (AudioViewTrack *t: view->vtrack){
		t->height_min = view->TIME_SCALE_HEIGHT;
		if (t->track->type == Track::TYPE_AUDIO){
			t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
		}else if (t->track->type == Track::TYPE_MIDI){
			if (t->track == view->cur_track){
				if (view->midi_view_mode == view->MIDI_MODE_CLASSICAL)
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
	for (MidiNote *n: t->midi){
		for (int p: pitch)
			if (n->pitch == p){
				if (n->range.is_inside(start))
					return Range::EMPTY;
			}
	}

	MidiRawData midi = midi_notes_to_events(t->midi);
	for (MidiEvent &e: midi)
		for (int p: pitch)
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
	if (creation_mode == CREATION_MODE_NOTE){
		pitch.add(selection->pitch);
	}else if (creation_mode == CREATION_MODE_INTERVAL){
		pitch.add(selection->pitch);
		if (midi_interval != 0)
			pitch.add(selection->pitch + midi_interval);
	}else if (creation_mode == CREATION_MODE_CHORD){
		pitch = chord_notes(chord_type, chord_inversion, selection->pitch);
	}
	return pitch;
}

Array<MidiNote> ViewModeMidi::getCreationNotes()
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
	Array<MidiNote> notes;
	if (allowed.empty())
		return notes;
	for (int p: pitch)
		notes.add(MidiNote(r and allowed, p, 1));
	notes[0].clef_position = selection->clef_position;
	notes[0].modifier = selection->modifier;
	return notes;
}

int ViewModeMidi::y2clef(int y, int &mod)
{
	int ti = view->cur_track->get_index();
	AudioViewTrack *t = view->vtrack[ti];
	if (view->midi_view_mode == view->MIDI_MODE_CLASSICAL){
		mod = modifier;
		return t->screen_to_clef_pos(y);
	}
	int pitch = pitch_min + ((t->area.y2 - y) * (pitch_max - pitch_min) / t->area.height());
	const Clef& clef = t->track->instrument.get_clef();
	return clef.pitch_to_position(pitch, view->midi_scale, mod);
}

int ViewModeMidi::y2pitch(int y)
{
	int ti = view->cur_track->get_index();
	AudioViewTrack *t = view->vtrack[ti];
	if (view->midi_view_mode == view->MIDI_MODE_CLASSICAL){
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

	if (view->midi_view_mode == view->MIDI_MODE_CLASSICAL){
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

void ViewModeMidi::drawGridBars(Painter *c, const rect &r, const color &bg, bool show_time)
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
	for (Bar &b: bars){
		int xx = cam->sample2screen(b.range.offset);

		float dx_bar = cam->dsample2screen(b.range.length);
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
			float beat_length = (float)b.range.length / (float)b.num_beats;
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

void ViewModeMidi::drawTrackBackground(Painter *c, AudioViewTrack *t)
{
	int mode = which_midi_mode(t->track);
	ViewModeDefault::drawTrackBackground(c, t);
	if ((t->track == view->cur_track) and (mode == AudioView::MIDI_MODE_LINEAR) and (t->track->type == Track::TYPE_MIDI))
		drawTrackPitchGrid(c, t);
}

void ViewModeMidi::drawTrackPitchGrid(Painter *c, AudioViewTrack *t)
{
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

inline bool hover_note_classical(const MidiNote &n, Selection &s, ViewModeMidi *vmm)
{
	if (n.clef_position != s.clef_position)
			return false;
	return n.range.is_inside(s.pos);
}

inline bool hover_note_midi(const MidiNote &n, Selection &s, ViewModeMidi *vmm)
{
	if (n.pitch != s.pitch)
		return false;
	return n.range.is_inside(s.pos);
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
		for (SampleRef *ss: s.track->samples){
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
		if (creation_mode != CREATION_MODE_SELECT){
			s.pitch = y2pitch(my);
			s.clef_position = y2clef(my, s.modifier);
			//s.modifier = modifier;
			s.type = Selection::TYPE_MIDI_PITCH;
			s.index = randi(100000); // quick'n'dirty fix to force view update every time the mouse moves
			int mode = which_midi_mode(s.track);
			if (mode == AudioView::MIDI_MODE_CLASSICAL){
				foreachi(MidiNote *n, s.track->midi, i)
					if (hover_note_classical(*n, s, this)){
						s.index = i;
						s.type = Selection::TYPE_MIDI_NOTE;
						return s;
					}
			}else if (mode == AudioView::MIDI_MODE_LINEAR){
				foreachi(MidiNote *n, s.track->midi, i)
					if (hover_note_midi(*n, s, this)){
						s.index = i;
						s.type = Selection::TYPE_MIDI_NOTE;
						return s;
					}
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



void ViewModeMidi::drawMidiNote(Painter *c, const MidiNote &n, int state)
{
	float x1 = cam->sample2screen(n.range.offset);
	float x2 = cam->sample2screen(n.range.end());
	float y1 = pitch2y(n.pitch + 1);
	float y2 = pitch2y(n.pitch);

	float y = (y1 + y2) / 2;
	float r = (y2 - y1) / 2.5f;

	color col = ColorInterpolate(AudioViewTrack::getPitchColor(n.pitch), view->colors.text, 0.2f);
	if (state == AudioViewTrack::STATE_HOVER){
		col = ColorInterpolate(col, view->colors.hover, 0.333f);
	}else if (state == AudioViewTrack::STATE_REFERENCE){
		col = ColorInterpolate(col, view->colors.background_track, 0.65f);
	}
	if (view->sel.has(&n)){
		color col1 = view->colors.selection;
		AudioViewTrack::draw_classical_note(c, x1, x2, y, r, 2, col1, col1, false);
	}

	AudioViewTrack::draw_classical_note(c, x1, x2, y, r, 0, col, ColorInterpolate(col, view->colors.background_track, 0.4f), false);
}

void ViewModeMidi::drawMidiEditable(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area)
{
	int mode = which_midi_mode(t->track);
	if (mode == view->MIDI_MODE_CLASSICAL)
		t->drawMidiClassical(c, midi, as_reference, 0);
	else if (mode == view->MIDI_MODE_TAB)
		t->drawMidiTab(c, midi, as_reference, 0);
	else // midi
		drawMidiEditableLinear(c, t, midi, as_reference, track, area);
}

void ViewModeMidi::drawMidiEditableLinear(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area)
{
	track_rect = area;

	// draw notes
	foreachi(MidiNote *n, midi, i){
		if ((n->pitch < pitch_min) or (n->pitch >= pitch_max))
			continue;
		bool _hover = ((hover->type == Selection::TYPE_MIDI_NOTE) and (i == hover->index));
		if (as_reference){
			drawMidiNote(c, *n, AudioViewTrack::STATE_REFERENCE);
		}else{
			drawMidiNote(c, *n, _hover ? AudioViewTrack::STATE_HOVER : AudioViewTrack::STATE_DEFAULT);
		}
	}
}

void ViewModeMidi::drawMidiEditableClassical(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area)
{
	const Clef& clef = track->instrument.get_clef();

	t->drawMidiClassicalClef(c, clef, view->midi_scale);
	//const int *mod = view->midi_scale.get_modifiers_clef();

	c->setAntialiasing(true);

	/*for (int i=0; i<7; i++)
		if (mod[i] != MODIFIER_NONE)
			c->drawStr(100, t->clef_pos_to_screen(i), modifier_symbol(mod[i]));*/

	// draw notes
	foreachi(MidiNote *n, midi, i){
		bool _hover = ((hover->type == Selection::TYPE_MIDI_NOTE) and (i == hover->index));
		if (as_reference){
			t->drawMidiNoteClassical(c, n, 0, AudioViewTrack::STATE_REFERENCE, clef);
		}else{
			t->drawMidiNoteClassical(c, n, 0, _hover ? AudioViewTrack::STATE_HOVER : AudioViewTrack::STATE_DEFAULT, clef);
		}
	}

	c->setFontSize(view->FONT_SIZE);
	c->setAntialiasing(false);
}

void ViewModeMidi::drawTrackData(Painter *c, AudioViewTrack *t)
{
	// midi
	if ((view->cur_track == t->track) and (t->track->type == Track::TYPE_MIDI)){
		// we're editing this track...

		for (int n: t->reference_tracks)
			if ((n >= 0) and (n < song->tracks.num) and (n != t->track->get_index()))
				drawMidiEditable(c, t, song->tracks[n]->midi, true, t->track, t->area);

		drawMidiEditable(c, t, t->track->midi, false, t->track, t->area);

		int mode = which_midi_mode(t->track);

		// current creation
		if ((HuiGetEvent()->lbut) and (selection->type == Selection::TYPE_MIDI_PITCH)){
			Array<MidiNote> notes = getCreationNotes();
			for (MidiNote &n: notes){
				if (mode == view->MIDI_MODE_CLASSICAL)
					t->drawMidiNoteClassical(c, &n, 0, AudioViewTrack::STATE_HOVER, t->track->instrument.get_clef());
				else if (mode == view->MIDI_MODE_TAB)
					{}
				else
					drawMidiNote(c, n, AudioViewTrack::STATE_HOVER);
			}
			c->setFontSize(view->FONT_SIZE);
		}


		if (mode == view->MIDI_MODE_CLASSICAL){
			if ((!HuiGetEvent()->lbut) and (hover->type == Selection::TYPE_MIDI_PITCH)){
				Range r = Range(hover->pos, 0);
				align_to_beats(song, r, beat_partition);
				const Clef &clef = t->track->instrument.get_clef();
				MidiNote n = MidiNote(r, hover->pitch, 1);
				n.clef_position = hover->clef_position;
				n.modifier = hover->modifier;
				t->drawMidiNoteClassical(c, &n, 0, t->STATE_HOVER, clef);

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
		if (t->track->type == Track::TYPE_MIDI)
			drawMidi(c, t, t->track->midi, false, 0);
	}

	// audio buffer
	t->drawTrackBuffers(c, view->cam.pos);

	// samples
	for (SampleRef *s: t->track->samples)
		t->drawSample(c, s);

	// marker
	t->marker_areas.resize(t->track->markers.num);
	foreachi(TrackMarker *m, t->track->markers, i)
		t->drawMarker(c, m, i, (view->hover.type == Selection::TYPE_MARKER) and (view->hover.track == t->track) and (view->hover.index == i));
}

int ViewModeMidi::which_midi_mode(Track *t)
{
	if ((view->cur_track == t) and (t->type == Track::TYPE_MIDI)){
		if (mode_wanted == view->MIDI_MODE_TAB){
			if (t->instrument.string_pitch.num > 0)
				return view->MIDI_MODE_TAB;
			return view->MIDI_MODE_CLASSICAL;
		}
		return mode_wanted;
	}
	return ViewModeDefault::which_midi_mode(t);
}
