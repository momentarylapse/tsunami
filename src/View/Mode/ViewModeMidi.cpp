/*
 * ViewModeMidi.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeMidi.h"

#include "../../Module/Audio/SongRenderer.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/Port/MidiPort.h"
#include "../../Device/OutputStream.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/Sample.h"
#include "../../Data/Midi/Clef.h"
#include "../../Data/SongSelection.h"
#include "../../Data/SampleRef.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../Helper/MidiPreview.h"

void align_to_beats(Song *s, Range &r, int beat_partition);

const int PITCH_SHOW_COUNT = 30;

ViewModeMidi::ViewModeMidi(AudioView *view) :
	ViewModeDefault(view)
{
	cur_layer = nullptr;
	beat_partition = 4;
	note_length = 1;
	win->setInt("beat_partition", beat_partition);
	win->setInt("note_length", note_length);
	mode_wanted = MidiMode::CLASSICAL;
	creation_mode = CreationMode::NOTE;
	midi_interval = 3;
	chord_type = ChordType::MINOR;
	chord_inversion = 0;
	modifier = NoteModifier::NONE;

	moving = false;
	string_no = 0;
	octave = 3;

	scroll_offset = 0;
	scroll_bar = rect(0, 0, 0, 0);

	preview = new MidiPreview(view->session);

	mouse_pre_moving_pos = -1;
}

ViewModeMidi::~ViewModeMidi()
{
	delete preview;
}

void ViewModeMidi::setMode(MidiMode _mode)
{
	mode_wanted = _mode;
	view->thm.dirty = true;
	view->forceRedraw();
}

void ViewModeMidi::setCreationMode(CreationMode _mode)
{
	creation_mode = _mode;
	//view->forceRedraw();
}


void ViewModeMidi::startMidiPreview(const Array<int> &pitch, float ttl)
{
	preview->start(view->cur_track->synth, pitch, view->cur_track->volume, ttl);
}

void ViewModeMidi::onLeftButtonDown()
{
	ViewModeDefault::onLeftButtonDown();
	auto mode = which_midi_mode(cur_layer->layer->track);

	if (creation_mode == CreationMode::SELECT){
		setCursorPos(hover->pos, true);
		view->msp.start(hover->pos, hover->y0);

	}else{
		//view->msp.start(hover->pos, hover->y0);
		view->hide_selection = true;
	}

	if (hover->type == Selection::Type::MIDI_NOTE){
		view->sel.click(hover->note, win->getKey(hui::KEY_CONTROL));

		dnd_start_soon(view->sel);
	}else if (hover->type == Selection::Type::CLEF_POSITION){
		/*if (creation_mode != CreationMode::SELECT){
			view->msp.stop();
		}*/
		view->msp.start_pos = hover->pos; // TODO ...bad
		if (mode == MidiMode::TAB){
			string_no = clampi(hover->clef_position, 0, cur_layer->layer->track->instrument.string_pitch.num - 1);
		}
	}else if (hover->type == Selection::Type::MIDI_PITCH){
		view->msp.start_pos = hover->pos; // TODO ...bad
		if (mode == MidiMode::TAB){
		}else{ // CLASSICAL/LINEAR
			// create new note
			startMidiPreview(getCreationPitch(hover->pitch), 1.0f);
		}
	}else if (hover->type == Selection::Type::SCROLL){
		scroll_offset = view->my - scroll_bar.y1;
		view->msp.stop();
	}
}

void ViewModeMidi::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();
	view->hide_selection = false;

	auto mode = which_midi_mode(cur_layer->layer->track);
	if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR)){
		if (hover->type == Selection::Type::MIDI_PITCH){
			auto notes = getCreationNotes(hover, view->msp.start_pos);
			setCursorPos(notes[0]->range.end() + 1, true);
			octave = pitch_get_octave(hover->pitch);
			view->cur_layer->addMidiNotes(notes);
			notes.clear(); // all notes owned by track now
			preview->end();
		}
	}

	if (moving){
		moving = false;
	}
}

void ViewModeMidi::onMouseMove()
{
	ViewModeDefault::onMouseMove();
	auto e = hui::GetEvent();

	if (hover->type == Selection::Type::MIDI_PITCH){
		// creating notes
		//view->forceRedraw();
	}else if (hover->type == Selection::Type::SCROLL){
		if (e->lbut){
			int _pitch_max = (cur_layer->area.y2 + scroll_offset - view->my) / cur_layer->area.height() * (MAX_PITCH - 1.0f);
			cur_layer->setPitchMinMax(_pitch_max - PITCH_SHOW_COUNT, _pitch_max);
		}
	}
}

MidiNote *make_note(ViewModeMidi *m, const Range &r, int pitch, NoteModifier mod, float volume = 1.0f)
{
	auto *n = new MidiNote(r, modifier_apply(pitch, mod), volume);
	n->modifier = mod;

	// dirty hack for clef position...
	const Clef& clef = m->cur_layer->layer->track->instrument.get_clef();
	NoteModifier dummy;
	n->clef_position = clef.pitch_to_position(pitch, m->view->midi_scale, dummy);
	return n;
}

void ViewModeMidi::onKeyDown(int k)
{
	auto mode = which_midi_mode(cur_layer->layer->track);
	if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR)){
		if (k == hui::KEY_0){
			modifier = NoteModifier::NONE;
			view->notify(view->MESSAGE_SETTINGS_CHANGE);
		}else if (k == hui::KEY_FENCE){
			modifier = NoteModifier::SHARP;
			view->notify(view->MESSAGE_SETTINGS_CHANGE);
		}else if (k == hui::KEY_3){
			modifier = NoteModifier::FLAT;
			view->notify(view->MESSAGE_SETTINGS_CHANGE);
		}else if (k == hui::KEY_4){
			modifier = NoteModifier::NATURAL;
			view->notify(view->MESSAGE_SETTINGS_CHANGE);
		}

		// add note
		if ((k >= hui::KEY_A) and (k <= hui::KEY_G)){
			Range r = getMidiEditRange();
			int number = (k - hui::KEY_A);
			int rel[7] = {9,11,0,2,4,5,7};
			int pitch = pitch_from_octave_and_rel(rel[number], octave);
			cur_layer->layer->addMidiNote(make_note(this, r, pitch, modifier));
			setCursorPos(r.end() + 1, true);
			startMidiPreview(pitch, 0.1f);
		}

		// add break
		if (k == hui::KEY_DOT){
			Range r = getMidiEditRange();
			setCursorPos(r.end() + 1, true);
		}

		// select octave
		if (k == hui::KEY_UP){
			octave = min(octave + 1, 7);
			view->forceRedraw();
		}
		if (k == hui::KEY_DOWN){
			octave = max(octave - 1, 0);
			view->forceRedraw();
		}
	}else if (mode == MidiMode::TAB){

		// add note
		if (((k >= hui::KEY_0) and (k <= hui::KEY_9)) or ((k >= hui::KEY_A) and (k <= hui::KEY_F))){
			Range r = getMidiEditRange();
			int number = (k - hui::KEY_0);
			if (k >= hui::KEY_A)
				number = 10 + (k - hui::KEY_A);
			int pitch = cur_layer->layer->track->instrument.string_pitch[string_no] + number;
			MidiNote *n = new MidiNote(r, pitch, 1.0f);
			n->stringno = string_no;
			cur_layer->layer->addMidiNote(n);
			setCursorPos(r.end() + 1, true);
			startMidiPreview(pitch, 0.1f);
		}

		// add break
		if (k == hui::KEY_DOT){
			Range r = getMidiEditRange();
			setCursorPos(r.end() + 1, true);
		}

		// select string
		if (k == hui::KEY_UP){
			string_no = min(string_no + 1, cur_layer->layer->track->instrument.string_pitch.num - 1);
			view->forceRedraw();
		}
		if (k == hui::KEY_DOWN){
			string_no = max(string_no - 1, 0);
			view->forceRedraw();
		}
	}

	//if (k == hui::KEY_ESCAPE)
		//tsunami->side_bar->open(SideBar::MIDI_EDITOR_CONSOLE);
		//view->setMode(view->mode_default);

	ViewModeDefault::onKeyDown(k);
}

void ViewModeMidi::updateTrackHeights()
{
	int n_ch = 2;
	for (AudioViewLayer *t: view->vlayer){
		t->height_min = view->TIME_SCALE_HEIGHT * 2;
		if (t->layer->is_main){
			if (t->layer->type == SignalType::AUDIO){
				t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
			}else if (t->layer->type == SignalType::MIDI){
				if (t->layer == view->cur_layer){
					auto mode = which_midi_mode(t->layer->track);
					if (mode == MidiMode::LINEAR)
						t->height_wish = 5000;
					else if (mode == MidiMode::CLASSICAL)
						t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * 6;
					else // TAB
						t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT * 4;
				}else{
					t->height_wish = view->MAX_TRACK_CHANNEL_HEIGHT;
				}
			}else{
				t->height_wish = view->TIME_SCALE_HEIGHT * 2;
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


Range get_allowed_midi_range(TrackLayer *l, Array<int> pitch, int start)
{
	Range allowed = Range::ALL;
	for (MidiNote *n: l->midi){
		for (int p: pitch)
			if (n->pitch == p){
				if (n->range.is_inside(start))
					return Range::EMPTY;
			}
	}

	MidiEventBuffer midi = midi_notes_to_events(l->midi);
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

Array<int> ViewModeMidi::getCreationPitch(int base_pitch)
{
	Array<int> pitch;
	if (creation_mode == CreationMode::NOTE){
		pitch.add(base_pitch);
	}else if (creation_mode == CreationMode::INTERVAL){
		pitch.add(base_pitch);
		if (midi_interval != 0)
			pitch.add(base_pitch + midi_interval);
	}else if (creation_mode == CreationMode::CHORD){
		pitch = chord_notes(chord_type, chord_inversion, base_pitch);
	}
	return pitch;
}

MidiNoteBuffer ViewModeMidi::getCreationNotes(Selection *sel, int pos0)
{
	int start = min(pos0, sel->pos);
	int end = max(pos0, sel->pos);
	Range r = Range(start, end - start);

	// align to beats
	if (song->bars.num > 0)
		align_to_beats(song, r, beat_partition);

	Array<int> pitch = getCreationPitch(sel->pitch);

	// collision?
	Range allowed = get_allowed_midi_range(view->cur_layer, pitch, pos0);

	// create notes
	MidiNoteBuffer notes;
	if (allowed.empty())
		return notes;
	for (int p: pitch)
		notes.add(new MidiNote(r and allowed, p, 1));
	if (notes.num > 0){
		notes[0]->clef_position = sel->clef_position;
		notes[0]->modifier = sel->modifier;
	}
	return notes;
}

void ViewModeMidi::setBeatPartition(int partition)
{
	beat_partition = partition;
	view->forceRedraw();
}

void ViewModeMidi::setNoteLength(int length)
{
	note_length = length;
	view->forceRedraw();
}

void ViewModeMidi::drawLayerBackground(Painter *c, AudioViewLayer *l)
{
	l->drawBlankBackground(c);

	color cc = l->getBackgroundColor();
	if (song->bars.num > 0)
		l->drawGridBars(c, cc, (l->layer->type == SignalType::BEATS), beat_partition);
	else
		view->drawGridTime(c, l->area, cc, false);

	if (l->layer->type == SignalType::MIDI){
		auto mode = which_midi_mode(l->layer->track);
		if (l->layer== view->cur_layer){
			if (mode == MidiMode::LINEAR)
				drawLayerPitchGrid(c, l);
		}

		if (mode == MidiMode::CLASSICAL){
			const Clef& clef = l->layer->track->instrument.get_clef();
			l->drawMidiClefClassical(c, clef, view->midi_scale);
		}else if (mode == MidiMode::TAB){
			l->drawMidiClefTab(c);
		}
	}
}

void ViewModeMidi::drawLayerPitchGrid(Painter *c, AudioViewLayer *l)
{
	cur_layer = l;

	// pitch grid
	c->setColor(color(0.25f, 0, 0, 0));
	for (int i=l->pitch_min; i<l->pitch_max; i++){
		float y0 = l->pitch2y_linear(i + 1);
		float y1 = l->pitch2y_linear(i);
		if (!view->midi_scale.contains(i)){
			c->setColor(color(0.2f, 0, 0, 0));
			c->drawRect(l->area.x1, y0, l->area.width(), y1 - y0);
		}
	}


	// pitch names
	color cc = view->colors.text;
	cc.a = 0.4f;
	Array<SampleRef*> *p = nullptr;
	if ((l->layer->track->synth) and (l->layer->track->synth->module_subtype == "Sample")){
		auto *c = l->layer->track->synth->get_config();
		p = (Array<SampleRef*> *)&c[1];
	}
	bool is_drum = (l->layer->track->instrument.type == Instrument::Type::DRUMS);
	for (int i=l->pitch_min; i<l->pitch_max; i++){
		c->setColor(cc);
		if (((hover->type == Selection::Type::MIDI_PITCH) or (hover->type == Selection::Type::MIDI_NOTE)) and (i == hover->pitch))
			c->setColor(view->colors.text);

		string name = pitch_name(i);
		if (is_drum){
			name = drum_pitch_name(i);
		}else if (p){
			if (i < p->num)
				if ((*p)[i])
					name = (*p)[i]->origin->name;
		}
		c->drawStr(20, l->area.y1 + l->area.height() * (l->pitch_max - i - 1) / PITCH_SHOW_COUNT, name);
	}
}

inline bool hover_note_classical(const MidiNote &n, Selection &s, ViewModeMidi *vmm)
{
	if (n.clef_position != s.clef_position)
		return false;
	return n.range.is_inside(s.pos);
}

inline bool hover_note_tab(const MidiNote &n, Selection &s, ViewModeMidi *vmm)
{
	if (n.stringno != s.clef_position)
		return false;
	return n.range.is_inside(s.pos);
}

inline bool hover_note_linear(const MidiNote &n, Selection &s, ViewModeMidi *vmm)
{
	if (n.pitch != s.pitch)
		return false;
	return n.range.is_inside(s.pos);
}

Selection ViewModeMidi::getHover()
{
	Selection s = ViewModeDefault::getHover();
	if (s.type != s.Type::LAYER)
		return s;

	int mx = view->mx;
	int my = view->my;

	// midi
	if ((s.layer) and (s.layer->type == SignalType::MIDI) and (s.layer == view->cur_layer)){
		auto mode = which_midi_mode(s.track);

		// scroll bar
		if ((mode == MidiMode::LINEAR) and (scroll_bar.inside(view->mx, view->my))){
			s.type = Selection::Type::SCROLL;
			return s;
		}

		/*if (creation_mode != CreationMode::SELECT)*/{
			if ((mode == MidiMode::CLASSICAL)){
				s.pitch = cur_layer->y2pitch_classical(my, modifier);
				s.clef_position = cur_layer->screen_to_clef_pos(my);
				s.modifier = modifier;
				s.type = Selection::Type::MIDI_PITCH;
				s.index = randi(100000); // quick'n'dirty fix to force view update every time the mouse moves

				foreachi(MidiNote *n, s.layer->midi, i)
					if (hover_note_classical(*n, s, this)){
						s.note = n;
						s.index = i;
						s.type = Selection::Type::MIDI_NOTE;
						return s;
					}
			}else if ((mode == MidiMode::TAB)){
				//s.pitch = cur_track->y2pitch_classical(my, modifier);
				s.clef_position = cur_layer->screen_to_string(my);
				s.modifier = modifier;
				s.type = Selection::Type::CLEF_POSITION;
				s.index = randi(100000); // quick'n'dirty fix to force view update every time the mouse moves

				foreachi(MidiNote *n, s.layer->midi, i)
					if (hover_note_tab(*n, s, this)){
						s.note = n;
						s.index = i;
						s.type = Selection::Type::MIDI_NOTE;
						return s;
					}
			}else if (mode == MidiMode::LINEAR){
				s.pitch = cur_layer->y2pitch_linear(my);
				s.clef_position = cur_layer->y2clef_linear(my, s.modifier);
				//s.modifier = modifier;
				s.type = Selection::Type::MIDI_PITCH;
				s.index = randi(100000); // quick'n'dirty fix to force view update every time the mouse moves

				foreachi(MidiNote *n, s.layer->midi, i)
					if (hover_note_linear(*n, s, this)){
						s.note = n;
						s.index = i;
						s.type = Selection::Type::MIDI_NOTE;
						return s;
					}
			}
		}
		/*if (creation_mode == CreationMode::SELECT){
			if ((s.type == Selection::Type::MIDI_PITCH) or (s.type == Selection::Type::CLEF_POSITION)){
				s.type = Selection::Type::TRACK;
			}
		}*/
	}

	return s;
}

void ViewModeMidi::drawLayerData(Painter *c, AudioViewLayer *l)
{
	// midi
	if ((view->cur_layer == l->layer) and (l->layer->type == SignalType::MIDI)){
		// we're editing this track...
		cur_layer = l;

		/*for (int n: t->reference_tracks)
			if ((n >= 0) and (n < song->tracks.num) and (song->tracks[n] != t->track))
				drawMidi(c, t, song->tracks[n]->midi, true, 0);*/

		drawMidi(c, l, l->layer->midi, false, 0);

		auto mode = which_midi_mode(l->layer->track);

		if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR)){

			// current creation
			if ((hui::GetEvent()->lbut) and (hover->type == Selection::Type::MIDI_PITCH)){
				auto notes = getCreationNotes(hover, view->msp.start_pos);
				drawMidi(c, l, notes, false, 0);
				//c->setFontSize(view->FONT_SIZE);
			}


			// creation preview
			if ((!hui::GetEvent()->lbut) and (hover->type == Selection::Type::MIDI_PITCH)){
				auto notes = getCreationNotes(hover, hover->pos);
				drawMidi(c, l, notes, false, 0);
			}
		}


		if (mode == MidiMode::CLASSICAL){

		}else if (mode == MidiMode::LINEAR){

			// scrollbar
			if (hover->type == Selection::Type::SCROLL)
				c->setColor(view->colors.text);
			else
				c->setColor(view->colors.text_soft1);
			scroll_bar = rect(l->area.x2 - 40, l->area.x2 - 20, l->area.y2 - l->area.height() * cur_layer->pitch_max / (MAX_PITCH - 1.0f), l->area.y2 - l->area.height() * cur_layer->pitch_min / (MAX_PITCH - 1.0f));
			c->drawRect(scroll_bar);
		}
	}else{

		// not editing -> just draw
		if (l->layer->type == SignalType::MIDI)
			drawMidi(c, l, l->layer->midi, false, 0);
	}


	// samples
	for (SampleRef *s: l->layer->samples)
		l->drawSample(c, s);


	if (l->layer->is_main){

		Track *t = l->layer->track;

		// marker
		l->marker_areas.resize(t->markers.num);
		l->marker_label_areas.resize(t->markers.num);
		foreachi(TrackMarker *m, t->markers, i)
			l->drawMarker(c, m, i, (view->hover.type == Selection::Type::MARKER) and (view->hover.track == t) and (view->hover.index == i));
	}

}

void ViewModeMidi::drawTrackData(Painter *c, AudioViewTrack *t)
{
}

MidiMode ViewModeMidi::which_midi_mode(Track *t)
{
	if ((view->cur_track == t) and (t->type == SignalType::MIDI)){
		if (mode_wanted == MidiMode::TAB){
			if (t->instrument.string_pitch.num > 0)
				return MidiMode::TAB;
			return MidiMode::CLASSICAL;
		}
		return mode_wanted;
	}
	return ViewModeDefault::which_midi_mode(t);
}

void ViewModeMidi::drawPost(Painter *c)
{
	ViewModeDefault::drawPost(c);

	auto mode = which_midi_mode(cur_layer->layer->track);
	Range r = getMidiEditRange();
	int x1 = view->cam.sample2screen(r.start());
	int x2 = view->cam.sample2screen(r.end());

	c->setColor(view->colors.selection_internal);
	if (mode == MidiMode::TAB){
		int y = cur_layer->string_to_screen(string_no);
		int y1 = y - cur_layer->clef_dy/2;
		int y2 = y + cur_layer->clef_dy/2;
		c->drawRect(x1,  y1,  x2 - x1,  y2 - y1);
	}else if (mode == MidiMode::CLASSICAL){
		int p1 = pitch_from_octave_and_rel(0, octave);
		int p2 = pitch_from_octave_and_rel(0, octave+1);
		int y1 = cur_layer->pitch2y_classical(p2);
		int y2 = cur_layer->pitch2y_classical(p1);
		c->drawRect(x1,  y1,  x2 - x1,  y2 - y1);
	}else if (mode == MidiMode::LINEAR){
		int p1 = pitch_from_octave_and_rel(0, octave);
		int p2 = pitch_from_octave_and_rel(0, octave+1);
		int y1 = cur_layer->pitch2y_linear(p2);
		int y2 = cur_layer->pitch2y_linear(p1);
		c->drawRect(x1,  y1,  x2 - x1,  y2 - y1);
	}
}

Range ViewModeMidi::getMidiEditRange()
{
	int a = song->bars.getPrevSubBeat(view->sel.range.offset+1, beat_partition);
	int b = song->bars.getNextSubBeat(view->sel.range.end()-1, beat_partition);
	if (a == b)
		b = song->bars.getNextSubBeat(b, beat_partition);
	for (int i=1; i<note_length; i++)
		b = song->bars.getNextSubBeat(b, beat_partition);
	return Range(a, b - a);
}

void ViewModeMidi::startSelection()
{
	hover->range.set_start(view->msp.start_pos);
	hover->range.set_end(hover->pos);
	if (hover->type == Selection::Type::TIME){
		hover->type = Selection::Type::SELECTION_END;
		view->selection_mode = view->SelectionMode::TIME;
	}else{
		hover->y0 = view->msp.start_y;
		hover->y1 = view->my;
		view->selection_mode = view->SelectionMode::RECT;
	}
	view->setSelection(getSelection(hover->range));
}
