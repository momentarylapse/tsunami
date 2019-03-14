/*
 * ViewModeMidi.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeMidi.h"

#include "../../Module/SignalChain.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Sample.h"
#include "../../Data/Midi/Clef.h"
#include "../../Data/SongSelection.h"
#include "../../Data/Midi/Clef.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"
#include "../AudioView.h"
#include "../AudioViewLayer.h"
#include "../Helper/MidiPreview.h"
#include "../Helper/ScrollBar.h"
#include "../Painter/GridPainter.h"
#include "../Painter/MidiPainter.h"
#include "../SideBar/SideBar.h"

void align_to_beats(Song *s, Range &r, int beat_partition);

const int EDIT_PITCH_SHOW_COUNT = 30;


MidiPainter* midi_context(ViewModeMidi *vm)
{
	auto *mp = vm->view->midi_painter;
	auto *l = vm->view->cur_vlayer;
	mp->set_context(l->area, l->layer->track->instrument, vm->view->midi_scale, l->is_playable(), l->midi_mode);
	mp->set_linear_range(l->edit_pitch_min, l->edit_pitch_max);
	return mp;
}


MidiNote *make_note(const Range &r, int pitch, int clef_pos, NoteModifier mod, float volume = 1.0f)
{
	auto *n = new MidiNote(r, pitch, volume);
	n->modifier = mod;
	n->clef_position = clef_pos;
	return n;
}

ViewModeMidi::ViewModeMidi(AudioView *view) :
	ViewModeDefault(view)
{
	sub_beat_partition = 4;
	note_length = 1;
	win->set_int("beat_partition", sub_beat_partition);
	win->set_int("note_length", note_length);
	mode_wanted = MidiMode::CLASSICAL;
	creation_mode = CreationMode::NOTE;
	input_mode = InputMode::DEFAULT;
	midi_interval = 3;
	chord_type = ChordType::MINOR;
	chord_inversion = 0;
	modifier = NoteModifier::NONE;

	moving = false;
	string_no = 0;
	octave = 3;

	rep_key_runner = -1;
	rep_key = -1;
	rep_key_num = 0;

	scroll = new ScrollBar;
	scroll->update(EDIT_PITCH_SHOW_COUNT, 128);

	preview = new MidiPreview(view->session);
	input_chain = nullptr;
	input = nullptr;

	mouse_pre_moving_pos = -1;

	side_bar_console = SideBar::MIDI_EDITOR_CONSOLE;
}

ViewModeMidi::~ViewModeMidi()
{
	delete preview;
}


void ViewModeMidi::set_modifier(NoteModifier mod)
{
	modifier = mod;
	view->set_message(modifier_symbol(mod), 4);
	notify();
}

void ViewModeMidi::set_mode(MidiMode _mode)
{
	mode_wanted = _mode;
	view->thm.dirty = true;
	view->force_redraw();
	notify();
}

void ViewModeMidi::set_creation_mode(CreationMode _mode)
{
	creation_mode = _mode;
	view->force_redraw();
	notify();
}

void ViewModeMidi::set_input_mode(InputMode _mode)
{
	input_mode = _mode;
	view->force_redraw();
	notify();
}

bool ViewModeMidi::editing(AudioViewLayer *l)
{
	if (view->mode != this)
		return false;
	if (l != view->cur_vlayer)
		return false;
	if (l->layer->type != SignalType::MIDI)
		return false;
	return true;
}

TrackLayer* ViewModeMidi::cur_layer()
{
	return view->cur_layer();
}

AudioViewLayer* ViewModeMidi::cur_vlayer()
{
	return view->cur_vlayer;
}


void ViewModeMidi::start_midi_preview(const Array<int> &pitch, float ttl)
{
	preview->start(view->cur_track()->synth, pitch, view->cur_track()->volume, ttl);
}

static hui::Timer ri_timer;
static MidiEventBuffer ri_keys;

void ri_insert(ViewModeMidi *me)
{
	if (ri_keys.num == 0)
		return;
	Range r = me->get_edit_range();
	for (auto &e: ri_keys){
		me->view->cur_layer()->add_midi_note(make_note(r, e.pitch, -1, NoteModifier::UNKNOWN, e.volume));
		me->start_midi_preview(e.pitch, 0.1f);
	}
	ri_keys.clear();
	me->set_cursor_pos(r.end(), true);
	me->select_in_edit_cursor();
	ri_timer.get();

}

void on_midi_input(ViewModeMidi *me)
{
	MidiEventBuffer buf;
	buf.samples = 1024;
	me->input->out->read_midi(buf);
	for (auto &e: buf){
		if (e.volume > 0){
			if (ri_keys.num > 0 and ri_timer.peek() > 0.3f)
				ri_insert(me);
			ri_keys.add(e);
			ri_timer.get();
		}else{
			ri_insert(me);
		}
	}

}

void ViewModeMidi::on_start()
{
	input_chain = new SignalChain(session, "midi-input");
	input = (InputStreamMidi*)input_chain->add(ModuleType::STREAM, "MidiInput");
	input_chain->subscribe(this, [=]{ on_midi_input(this); }, Module::MESSAGE_TICK);
	input_chain->start();
}

void ViewModeMidi::on_end()
{
	input_chain->unsubscribe(this);
	input_chain->stop();
	delete input_chain;
	input_chain= nullptr;
	input = nullptr;
}

void ViewModeMidi::on_left_button_down()
{
	ViewModeDefault::on_left_button_down();
	auto mode = cur_vlayer()->midi_mode;

	bool over_sel_note = false;
	if (hover->note)
		over_sel_note = view->sel.has(hover->note);


	if (creation_mode == CreationMode::SELECT and !over_sel_note){
		set_cursor_pos(hover->pos, true);
		select_in_edit_cursor();
		view->msp.start(hover->pos, hover->y0);

	}else{
		//view->msp.start(hover->pos, hover->y0);
		view->hide_selection = true;
	}

	if (hover->type == Selection::Type::MIDI_NOTE){
		view->sel.click(hover->note, win->get_key(hui::KEY_CONTROL));

		dnd_start_soon(view->sel);
	}else if (hover->type == Selection::Type::CLEF_POSITION){
		/*if (creation_mode != CreationMode::SELECT){
			view->msp.stop();
		}*/
		view->msp.start_pos = hover->pos; // TODO ...bad
		if (mode == MidiMode::TAB){
			string_no = clampi(hover->clef_position, 0, view->cur_track()->instrument.string_pitch.num - 1);
		}
	}else if (hover->type == Selection::Type::MIDI_PITCH){
		view->msp.start_pos = hover->pos; // TODO ...bad
		if (mode == MidiMode::TAB){
		}else{ // CLASSICAL/LINEAR
			// create new note
			start_midi_preview(get_creation_pitch(hover->pitch), 1.0f);
		}
	}else if (hover->type == Selection::Type::SCROLLBAR_MIDI){
		scroll->drag_start(view->mx, view->my);
		view->msp.stop();
	}
}

void ViewModeMidi::on_left_button_up()
{
	ViewModeDefault::on_left_button_up();
	view->hide_selection = false;

	auto mode = cur_vlayer()->midi_mode;
	if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR)){
		if (hover->type == Selection::Type::MIDI_PITCH){
			auto notes = get_creation_notes(hover, view->msp.start_pos);
			if (notes.num > 0){
				set_cursor_pos(notes[0]->range.end(), true);
				octave = pitch_get_octave(hover->pitch);
				select_in_edit_cursor();
				view->cur_layer()->add_midi_notes(notes);
				notes.clear(); // all notes owned by track now
			}
			preview->end();
		}
	}

	if (moving){
		moving = false;
	}
}

void ViewModeMidi::on_mouse_move()
{
	ViewModeDefault::on_mouse_move();
	auto e = hui::GetEvent();

	if (hover->type == Selection::Type::MIDI_PITCH){
		// creating notes
		//view->forceRedraw();
	}else if (hover->type == Selection::Type::SCROLLBAR_MIDI){
		if (e->lbut){
			scroll->drag_update(view->mx, view->my);
			int _pitch_max = 127 - scroll->offset;
			cur_vlayer()->set_edit_pitch_min_max(_pitch_max - EDIT_PITCH_SHOW_COUNT, _pitch_max);
		}
	}
}

void ViewModeMidi::edit_add_pause()
{
	Range r = get_edit_range();
	set_cursor_pos(r.end(), true);
	select_in_edit_cursor();
}

void ViewModeMidi::edit_add_note_by_urelative(int urelative)
{
	Range r = get_edit_range();
	int upos = octave * 7 + urelative;
	const Clef& clef = view->cur_track()->instrument.get_clef();
	int clef_pos = upos - clef.offset;
	NoteModifier mod = combine_note_modifiers(modifier, view->midi_scale.get_modifier(upos));
	int pitch = uniclef_to_pitch(upos);
	pitch = modifier_apply(pitch, mod);
	view->cur_layer()->add_midi_note(make_note(r, pitch, clef_pos, mod));
	set_cursor_pos(r.end(), true);
	select_in_edit_cursor();
	start_midi_preview(pitch, 0.1f);
}

void ViewModeMidi::edit_add_note_on_string(int hand_pos)
{
	Range r = get_edit_range();
	int pitch = cur_layer()->track->instrument.string_pitch[string_no] + hand_pos;
	MidiNote *n = new MidiNote(r, pitch, 1.0f);
	n->stringno = string_no;
	cur_layer()->add_midi_note(n);
	set_cursor_pos(r.end(), true);
	select_in_edit_cursor();
	start_midi_preview(pitch, 0.1f);
}

void ViewModeMidi::edit_backspace()
{
	Range r = get_backwards_range();
	set_cursor_pos(r.offset, true);
	SongSelection s = get_select_in_edit_cursor();
	view->song->delete_selection(s);
}

void ViewModeMidi::jump_string(int delta)
{
	string_no = max(min(string_no + delta, cur_layer()->track->instrument.string_pitch.num - 1), 0);
	select_in_edit_cursor();
	view->force_redraw();
}

void ViewModeMidi::jump_octave(int delta)
{
	octave = max(min(octave + delta, 7), 0);
	select_in_edit_cursor();
	view->force_redraw();
}

void ViewModeMidi::set_rep_key(int k)
{
	if (rep_key_runner >= 0)
		hui::CancelRunner(rep_key_runner);
	rep_key_runner = hui::RunLater(0.8f, [=]{ rep_key_runner = -1; rep_key_num = -1; rep_key = -1; });

	if (k == rep_key)
		rep_key_num ++;
	else
		rep_key_num = 1;
	rep_key = k;
}

void set_note_lengthx(ViewModeMidi *m, int l, int p, int n, const string &text)
{
	if ((m->sub_beat_partition % p) == 0){
		m->set_note_length(m->sub_beat_partition / p * n);
	}else{
		m->set_note_length(l * n);
		m->set_sub_beat_partition(p);
	}
	m->set_cursor_pos(m->view->sel.range.offset, true);

	string t;
	if (n > 4){
		t = text + " x " + i2s(n);
	}else{
		for (int i=0; i<n; i++)
			t += text;
	}
	m->view->set_message(t, 6);
}

void ViewModeMidi::on_key_down(int k)
{
	set_rep_key(k);
	auto mode = cur_vlayer()->midi_mode;
	if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR)){
		if (input_mode == InputMode::DEFAULT){
			if (k == hui::KEY_0){
				set_modifier(NoteModifier::NONE);
			}else if (k == hui::KEY_FENCE){
				set_modifier(NoteModifier::SHARP);
			}else if (k == hui::KEY_3){
				set_modifier(NoteModifier::FLAT);
			}else if (k == hui::KEY_4){
				set_modifier(NoteModifier::NATURAL);
			}

			// add note
			if ((k >= hui::KEY_A) and (k <= hui::KEY_G)){
				int number = (k - hui::KEY_A);
				int urel[7] = {5,6,0,1,2,3,4};
				edit_add_note_by_urelative(urel[number]);
			}
		}

		// select octave
		if (k == hui::KEY_UP)
			jump_octave(1);
		if (k == hui::KEY_DOWN)
			jump_octave(-1);
	}else if (mode == MidiMode::TAB){
		if (input_mode == InputMode::DEFAULT){

			// add note
			if (((k >= hui::KEY_0) and (k <= hui::KEY_9)) or ((k >= hui::KEY_A) and (k <= hui::KEY_F))){
				int number = (k - hui::KEY_0);
				if (k >= hui::KEY_A)
					number = 10 + (k - hui::KEY_A);
				edit_add_note_on_string(number);
			}
		}

		// select string
		if (k == hui::KEY_UP)
			jump_string(1);
		if (k == hui::KEY_DOWN)
			jump_string(-1);
	}
	
	
	// cursor
	if (k == hui::KEY_LEFT){
		Range r = get_backwards_range();
		set_cursor_pos(r.offset, true);
		select_in_edit_cursor();
		return;
	}
	if (k == hui::KEY_RIGHT){
		edit_add_pause();
		return;
	}

	// remove
	if (k == hui::KEY_BACKSPACE)
		edit_backspace();

	if (input_mode == InputMode::NOTE_LENGTH){
		if (((k >= hui::KEY_1) and (k <= hui::KEY_9)) or ((k >= hui::KEY_A) and (k <= hui::KEY_F))){
			int number = (k - hui::KEY_0);
			if (k >= hui::KEY_A)
				number = 10 + (k - hui::KEY_A);
			set_note_length(number);
			set_input_mode(InputMode::DEFAULT);
		}
	}else if (input_mode == InputMode::BEAT_PARTITION){
		if (((k >= hui::KEY_1) and (k <= hui::KEY_9)) or ((k >= hui::KEY_A) and (k <= hui::KEY_F))){
			int number = (k - hui::KEY_0);
			if (k >= hui::KEY_A)
				number = 10 + (k - hui::KEY_A);
			set_sub_beat_partition(number);
			set_input_mode(InputMode::DEFAULT);
		}
	}

	if (k == hui::KEY_Q){
		// quarter
		set_note_lengthx(this, 1, 1, rep_key_num, "𝅘𝅥  ");
	}
	if (k == hui::KEY_W){
		// 8th
		set_note_lengthx(this, 1, 2, rep_key_num, "𝅘𝅥𝅮  ");
	}
	if (k == hui::KEY_S){
		// 16th
		set_note_lengthx(this, 1, 4, rep_key_num, "𝅘𝅥𝅯  ");
	}

	if (k == hui::KEY_T){
		set_note_lengthx(this, 1, 3, rep_key_num, "⅓");
	}


	if (k == hui::KEY_L){
		set_input_mode(InputMode::NOTE_LENGTH);
	}
	if (k == hui::KEY_P){
		set_input_mode(InputMode::BEAT_PARTITION);
	}
	if (k == hui::KEY_ESCAPE){
		set_input_mode(InputMode::DEFAULT);
	}

	//if (k == hui::KEY_ESCAPE)
	//	session->set_mode("default");

	ViewModeDefault::on_key_down(k);
}

float ViewModeMidi::layer_min_height(AudioViewLayer *l)
{
	if (editing(l)){
		auto mode = l->midi_mode;
		if (mode == MidiMode::LINEAR)
			return 500;
		else if (mode == MidiMode::CLASSICAL)
			return view->MAX_TRACK_CHANNEL_HEIGHT * 4;
		else // TAB
			return view->MAX_TRACK_CHANNEL_HEIGHT * 4;
	}

	return ViewModeDefault::layer_min_height(l);
}

float ViewModeMidi::layer_suggested_height(AudioViewLayer *l)
{
	if (editing(l)){
		auto mode = l->midi_mode;
		if (mode == MidiMode::LINEAR)
			return 5000;
		else if (mode == MidiMode::CLASSICAL)
			return view->MAX_TRACK_CHANNEL_HEIGHT * 4;
		else // TAB
			return view->MAX_TRACK_CHANNEL_HEIGHT * 4;
	}

	return ViewModeDefault::layer_suggested_height(l);
}

void ViewModeMidi::on_cur_layer_change()
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

Array<int> ViewModeMidi::get_creation_pitch(int base_pitch)
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

MidiNoteBuffer ViewModeMidi::get_creation_notes(Selection *sel, int pos0)
{
	int start = min(pos0, sel->pos);
	int end = max(pos0, sel->pos);
	Range r = RangeTo(start, end);

	// align to beats
	if (song->bars.num > 0)
		align_to_beats(song, r, sub_beat_partition);

	Array<int> pitch = get_creation_pitch(sel->pitch);

	// collision?
	Range allowed = get_allowed_midi_range(view->cur_layer(), pitch, pos0);

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

void ViewModeMidi::set_sub_beat_partition(int partition)
{
	sub_beat_partition = partition;
	select_in_edit_cursor();
	view->force_redraw();
	notify();
}

void ViewModeMidi::set_note_length(int length)
{
	note_length = length;
	select_in_edit_cursor();
	view->force_redraw();
	notify();
}

void ViewModeMidi::draw_layer_background(Painter *c, AudioViewLayer *l)
{
	if (editing(l)){
		view->grid_painter->set_context(l->area, l->grid_colors());
		view->grid_painter->draw_empty_background(c);
		view->grid_painter->draw_whatever(c, sub_beat_partition);

		if (l->layer->type == SignalType::MIDI){
			auto *mp = midi_context(this);
			auto mode = l->midi_mode;
			if (mode == MidiMode::LINEAR)
				mp->draw_pitch_grid(c, l->layer->track->synth);

			if (mode == MidiMode::CLASSICAL){
				mp->draw_clef_classical(c);
			}else if (mode == MidiMode::TAB){
				mp->draw_clef_tab(c);
			}
		}
	}else
		ViewModeDefault::draw_layer_background(c, l);
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

Selection ViewModeMidi::get_hover()
{
	Selection s = ViewModeDefault::get_hover();
	if (s.type != s.Type::LAYER)
		return s;

	int mx = view->mx;
	int my = view->my;

	// midi
	if (s.layer and editing(s.vlayer)){
		auto mode = s.vlayer->midi_mode;

		auto *mp = midi_context(this);

		// scroll bar
		if ((mode == MidiMode::LINEAR) and (scroll->area.inside(view->mx, view->my))){
			s.type = Selection::Type::SCROLLBAR_MIDI;
			return s;
		}

		/*if (creation_mode != CreationMode::SELECT)*/{
			if ((mode == MidiMode::CLASSICAL)){
				s.clef_position = mp->screen_to_clef_pos(my);
				int upos = s.track->instrument.get_clef().position_to_uniclef(s.clef_position);
				s.modifier = combine_note_modifiers(modifier, view->midi_scale.get_modifier(upos));
				s.pitch = uniclef_to_pitch(upos, s.modifier);
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
				s.clef_position = mp->screen_to_string(my);
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
				s.pitch = mp->y2pitch_linear(my);
				s.clef_position = mp->y2clef_linear(my, s.modifier);
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

void ViewModeMidi::draw_layer_data(Painter *c, AudioViewLayer *l)
{
	if (!editing(l)){
		// not editing -> just draw
		ViewModeDefault::draw_layer_data(c, l);
		return;
	}


	auto mode = l->midi_mode;

	/*for (int n: t->reference_tracks)
		if ((n >= 0) and (n < song->tracks.num) and (song->tracks[n] != t->track))
			drawMidi(c, t, song->tracks[n]->midi, true, 0);*/

	auto *mp = midi_context(this);
	mp->draw(c, l->layer->midi);

	if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR)){

		// current creation
		if ((hui::GetEvent()->lbut) and (hover->type == Selection::Type::MIDI_PITCH)){
			auto notes = get_creation_notes(hover, view->msp.start_pos);
			mp->draw(c, notes);
			//c->setFontSize(view->FONT_SIZE);
		}


		// creation preview
		if ((!hui::GetEvent()->lbut) and (hover->type == Selection::Type::MIDI_PITCH)){
			auto notes = get_creation_notes(hover, hover->pos);
			mp->draw(c, notes);
		}
	}


	if (mode == MidiMode::CLASSICAL){

	}else if (mode == MidiMode::LINEAR){

		// scrollbar
		scroll->offset = 127 - cur_vlayer()->edit_pitch_max;
		scroll->set_area(rect(l->area.x2 - view->SCROLLBAR_WIDTH, l->area.x2, l->area.y1, l->area.y2));
		scroll->draw(c, hover->type == Selection::Type::SCROLLBAR_MIDI);
	}




	// samples
	for (auto *s: l->layer->samples)
		l->draw_sample(c, s);


	if (l->layer->is_main())
		l->draw_markers(c, l->layer->track->markers_sorted(), *hover);
}

void ViewModeMidi::draw_track_data(Painter *c, AudioViewTrack *t)
{
	ViewModeDefault::draw_track_data(c, t);
}


void ViewModeMidi::draw_imploded_track_data(Painter *c, AudioViewTrack *t)
{
	ViewModeDefault::draw_imploded_track_data(c, t);
}

void ViewModeMidi::draw_post(Painter *c)
{
	ViewModeDefault::draw_post(c);

	auto *l = cur_vlayer();
	auto mode = l->midi_mode;
	Range r = get_edit_range();
	float x1, x2;
	view->cam.range2screen(r, x1, x2);

	auto *mp = midi_context(this);

	c->set_color(view->colors.text_soft1);
	c->set_fill(false);
	if (mode == MidiMode::TAB){
		int y = mp->string_to_screen(string_no);
		int y1 = y - mp->clef_dy/2;
		int y2 = y + mp->clef_dy/2;
		c->draw_rect(x1,  y1,  x2 - x1,  y2 - y1);
	}else if (mode == MidiMode::CLASSICAL){
		int p1 = pitch_from_octave_and_rel(0, octave);
		int p2 = pitch_from_octave_and_rel(0, octave+1);
		int y1 = mp->pitch2y_classical(p2);
		int y2 = mp->pitch2y_classical(p1);
		c->draw_rect(x1,  y1,  x2 - x1,  y2 - y1);
	}else if (mode == MidiMode::LINEAR){
		int p1 = pitch_from_octave_and_rel(0, octave);
		int p2 = pitch_from_octave_and_rel(0, octave+1);
		int y1 = mp->pitch2y_linear(p2);
		int y2 = mp->pitch2y_linear(p1);
		c->draw_rect(x1,  y1,  x2 - x1,  y2 - y1);
	}
	c->set_fill(true);

	string message = _("cursor (←,→)    delete (⟵)    note length (L)    beat partition (P)");
	if (mode == MidiMode::TAB)
		message += "    " + _("string (↑,↓)    add note (0-9, A-F)");
	else if ((mode == MidiMode::CLASSICAL) or (mode == MidiMode::LINEAR))
		message += "    " + _("octave (↑,↓)    modifiers (#,3,0)    add note (A-G)");
	message += "    " + _("𝅘𝅥  ,𝅘𝅥𝅮  ,𝅘𝅥𝅯  ,𝅘𝅥𝅮 ₃    (Q,W,S,T)");
	if (input_mode == InputMode::NOTE_LENGTH)
		message = _("enter note length (1-9, A-F)    cancel (Esc)");
	else if (input_mode == InputMode::BEAT_PARTITION)
		message = _("enter beat partition (1-9, A-F)    cancel (Esc)");
	view->draw_boxed_str(c, (view->song_area.x1 + view->song_area.x2)/2, view->area.y2 - 30, message, view->colors.text_soft1, view->colors.background_track_selected, 0);

}

// seems fine
Range ViewModeMidi::get_edit_range()
{
	// manual selection has priority
	if (view->sel.range.length > 0)
		return view->sel.range;

	int pos = view->sel.range.offset;
	int a = song->bars.get_prev_sub_beat(pos+1, sub_beat_partition);
	int b = song->bars.get_next_sub_beat(pos-1, sub_beat_partition);
	if (a == b) // on a sub beat
		b = song->bars.get_next_sub_beat(b, sub_beat_partition);
	for (int i=1; i<note_length; i++)
		b = song->bars.get_next_sub_beat(b, sub_beat_partition);
	return RangeTo(a, b);
}


Range ViewModeMidi::get_backwards_range()
{
	// manual selection has priority
	if (view->sel.range.length > 0)
		return view->sel.range;

	int pos = view->sel.range.offset;
	int a = song->bars.get_prev_sub_beat(pos+1, sub_beat_partition);
	int b = song->bars.get_next_sub_beat(pos-1, sub_beat_partition);
	if (a == b) // on a sub beat
		a = song->bars.get_prev_sub_beat(a, sub_beat_partition);
	for (int i=1; i<note_length; i++)
		a = song->bars.get_prev_sub_beat(a, sub_beat_partition);
	return RangeTo(a, b);
}

SongSelection ViewModeMidi::get_select_in_edit_cursor()
{
	Range r = get_edit_range();
	SongSelection s = SongSelection::from_range(view->song, r, view->cur_layer()->track, view->cur_layer()).filter(SongSelection::Mask::MIDI_NOTES);
	auto mode = cur_vlayer()->midi_mode;
	auto notes = s.notes;
	if (mode == MidiMode::TAB){
		for (auto *n: notes)
			if (n->stringno != string_no)
				s.set(n, false);
	}else if (mode == MidiMode::CLASSICAL or mode == MidiMode::LINEAR){
		for (auto *n: notes){
			if (pitch_get_octave(n->pitch) != octave)
				s.set(n, false);
		}
	}
	return s;
}

void ViewModeMidi::select_in_edit_cursor()
{
	view->sel.notes = get_select_in_edit_cursor().notes;
	view->update_selection();
}

void ViewModeMidi::start_selection()
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
	view->set_selection(get_selection(hover->range));
}

SongSelection ViewModeMidi::get_selection_for_rect(const Range &r, int y0, int y1)
{
	auto sel = ViewModeDefault::get_selection_for_rect(r, y0, y1);
	sel.markers = {};
	return sel;
}

SongSelection ViewModeMidi::get_selection_for_range(const Range &r)
{
	auto sel = ViewModeDefault::get_selection_for_range(r);
	sel.markers = {};
	return sel;
}
