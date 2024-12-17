/*
 * ViewModeEditMidi.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewModeEditMidi.h"
#include "ViewModeEdit.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../helper/graph/ScrollBar.h"
#include "../MouseDelayPlanner.h"
#include "../helper/MidiPreview.h"
#include "../painter/GridPainter.h"
#include "../painter/MidiPainter.h"
#include "../sidebar/SideBar.h"
#include "../TsunamiWindow.h"
#include "../ColorScheme.h"
#include "../../module/SignalChain.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../lib/os/time.h"
#include "../../action/Action.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Sample.h"
#include "../../data/midi/Clef.h"
#include "../../data/SongSelection.h"
#include "../../data/midi/Clef.h"
#include "../../device/Device.h"
#include "../../device/DeviceManager.h"
#include "../../module/stream/MidiInput.h"
#include "../../module/midi/MidiAccumulator.h"
#include "../../Session.h"
#include "../../lib/hui/language.h"
#include "../../lib/image/Painter.h"

namespace tsunami {

void align_to_beats(Song *s, Range &r, int beat_partition);

const float EDIT_PITCH_SHOW_COUNT = 30.0f;



Range get_allowed_midi_range(TrackLayer *l, const Array<int> &pitch, int start) {
	Range allowed = Range::ALL;
	for (MidiNote *n: weak(l->midi)) {
		for (int p: pitch)
			if (n->pitch == p) {
				if (n->range.is_inside(start))
					return Range::NONE;
			}
	}

	MidiEventBuffer midi = midi_notes_to_events(l->midi);
	for (MidiEvent &e: midi)
		for (int p: pitch)
			if (e.pitch == p) {
				if ((e.pos >= start) and (e.pos < allowed.end()))
					allowed.set_end(e.pos);
				if ((e.pos < start) and (e.pos >= allowed.start()))
					allowed.set_start(e.pos);
			}
	return allowed;
}

MidiNote *make_note(const Range &r, int pitch, int clef_pos, NoteModifier mod, float volume = 1.0f) {
	auto *n = new MidiNote(r, pitch, volume);
	n->modifier = mod;
	n->clef_position = clef_pos;
	return n;
}

class ActionTrackMoveNotes: public Action {
public:
	ActionTrackMoveNotes(TrackLayer *l, const SongSelection &sel) {
		layer = l;
		for (auto n: weak(l->midi))
			if (sel.has(n))
				notes.add({n, n->range.offset, n->pitch, n->stringno});
	}

	string name() const override { return ":##:move notes"; }

	void *execute(Data *d) override {
		for (auto &d: notes) {
			d.note->range.offset = d.pos_old + doffset;
			d.note->pitch = d.pitch_old + dpitch;
			d.note->stringno = d.string_old;
			if (dstring != 0 and layer->track->instrument.has_strings())
				d.note->stringno = clamp(d.string_old + dstring, 0, layer->track->instrument.string_pitch.num - 1);
			d.note->reset_clef();
		}
		layer->track->out_changed.notify();
		return nullptr;
	}
	void undo(Data *d) override{
		for (auto &d: notes) {
			d.note->range.offset = d.pos_old;
			d.note->pitch = d.pitch_old;
			d.note->stringno = d.string_old;
			d.note->reset_clef();
		}
		layer->track->out_changed.notify();
	}

	// continuous editing
	void abort(Data *d) {
		undo(d);
	}
	void abort_and_notify(Data *d) {
		abort(d);
		d->out_changed.notify();
	}
	void set_param_and_notify(Data *d, int _doffset, float _dpitch, int _dstring) {
		doffset = _doffset;
		dpitch = _dpitch;
		dstring = _dstring;
		execute(d);
		d->out_changed.notify();
	}

	bool is_trivial() override {
		return (doffset == 0) and (dpitch == 0);
	}

private:
	struct NoteSaveData {
		MidiNote *note;
		int pos_old;
		float pitch_old;
		int string_old;
	};
	Array<NoteSaveData> notes;
	TrackLayer *layer;
	int doffset;
	float dpitch;
	int dstring;
};

Range selected_midi_range(TrackLayer *l, const SongSelection &sel) {
	bool first = true;
	Range r = Range(0,0);
	for (auto n: weak(l->midi))
		if (sel.has(n)) {
			if (first)
				r = n->range;
			else
				r = r or n->range;
			first = false;
		}
	return r;
}

class ActionTrackScaleNotes: public Action {
public:
	ActionTrackScaleNotes(TrackLayer *l, const SongSelection &sel) {
		layer = l;
		range0 = selected_midi_range(l, sel);
		for (auto n: weak(l->midi))
			if (sel.has(n))
				notes.add({n, n->range});
	}

	string name() const override { return ":##:scale notes"; }

	void *execute(Data *d) override {
		for (auto &d: notes) {
			d.note->range = d.range_old.scale_rel(range0, range);
		}
		layer->track->out_changed.notify();
		return nullptr;
	}
	void undo(Data *d) override{
		for (auto &d: notes)
			d.note->range = d.range_old;
		layer->track->out_changed.notify();
	}

	// continuous editing
	void abort(Data *d) {
		undo(d);
	}
	void abort_and_notify(Data *d) {
		abort(d);
		d->out_changed.notify();
	}
	void set_param_and_notify(Data *d, const Range &r) {
		range = r;
		execute(d);
		d->out_changed.notify();
	}

	bool is_trivial() override {
		return range == range0;
	}

private:
	struct NoteSaveData {
		MidiNote *note;
		Range range_old;
	};
	Array<NoteSaveData> notes;
	TrackLayer *layer;
	Range range0;
	Range range;
};

class MouseDelayNotesDnD : public scenegraph::MouseDelayAction {
public:
	AudioViewLayer *layer;
	AudioView *view;
	SongSelection sel;
	ViewModeEditMidi *mode_midi;
	ActionTrackMoveNotes *action = nullptr;
	int mouse_pos0;
	int ref_pos;
	int pitch0;
	int string0;
	MouseDelayNotesDnD(AudioViewLayer *l, const SongSelection &s) {
		layer = l;
		view = layer->view;
		mode_midi = view->mode_edit_midi;
		sel = s;
		mouse_pos0 = view->hover().pos;
		ref_pos = hover_reference_pos(view->hover());
		pitch0 = mouse_to_pitch(view->hover().y0);
		string0 = mouse_to_string(view->hover().y0);
	}
	int mouse_to_pitch(float y) {
		// relative to arbitrary point!
		auto mp = layer->midi_context();
		if (layer->midi_mode() == MidiMode::Linear)
			return mp->y2pitch(y, NoteModifier::None);
		if (layer->midi_mode() == MidiMode::Classical)
			return mp->screen_to_clef_pos(y * 12.0f / 7.0f); // quick'n'dirty chromatic hack :D
		if (layer->midi_mode() == MidiMode::Tab)
			return layer->track()->instrument.string_pitch[mouse_to_string(y)];
		return 0;
	}
	int mouse_to_string(float y) {
		auto mp = layer->midi_context();
		if (layer->midi_mode() == MidiMode::Tab)
			return mp->screen_to_string(y);
		return 0;
	}
	void on_start(const vec2 &m) override {
		action = new ActionTrackMoveNotes(layer->layer, sel);
	}
	void on_update(const vec2 &m) override {
		int p = view->get_mouse_pos(m) + (ref_pos - mouse_pos0);
		int pitch = mouse_to_pitch(m.y);
		int stringno = mouse_to_string(m.y);

		view->snap_to_grid(p);
		int dpos = p - mouse_pos0 - (ref_pos - mouse_pos0);
		int dpitch = pitch - pitch0;
		int dstring = stringno - string0;
		action->set_param_and_notify(view->song, dpos, dpitch, dstring);
	}
	void on_finish(const vec2 &m) override {
		view->song->execute(action);
	}
	void on_cancel() override {
		action->undo(view->song);
		delete action;
	}

	int hover_reference_pos(HoverData &s) {
		if (s.note)
			return s.note->range.offset;
		return s.pos;
	}
};

class MouseDelayNotesScaleDnD : public scenegraph::MouseDelayAction {
public:
	AudioViewLayer *layer;
	AudioView *view;
	SongSelection sel;
	ViewModeEditMidi *mode_midi;
	ActionTrackScaleNotes *action = nullptr;
	int mouse_pos0;
	int ref_pos;
	Range range0;
	MouseDelayNotesScaleDnD(AudioViewLayer *l, const SongSelection &s) {
		layer = l;
		view = layer->view;
		mode_midi = view->mode_edit_midi;
		sel = s;
		mouse_pos0 = view->hover().pos;
		ref_pos = hover_reference_pos(view->hover()) + view->hover().note->range.length;
		range0 = selected_midi_range(layer->layer, sel);
	}
	void on_start(const vec2 &m) override {
		action = new ActionTrackScaleNotes(layer->layer, sel);
	}
	void on_update(const vec2 &m) override {
		int p = view->get_mouse_pos(m) + (ref_pos - mouse_pos0);

		view->snap_to_grid(p);
		[[maybe_unused]] int dpos = p - mouse_pos0 - (ref_pos - mouse_pos0);
		range0.set_end(p);
		action->set_param_and_notify(view->song, range0);
	}
	void on_finish(const vec2 &m) override {
		view->song->execute(action);
	}
	void on_cancel() override {
		action->undo(view->song);
		delete action;
	}
	int hover_reference_pos(HoverData &s) {
		if (s.note)
			return s.note->range.offset;
		return s.pos;
	}
};

ViewModeEditMidi::ViewModeEditMidi(AudioView *view) :
	ViewModeDefault(view)
{
	mode_name = "midi";
	sub_beat_partition = 1;
	note_length = 1;
	win->set_int("beat_partition", sub_beat_partition);
	win->set_int("note_length", note_length);
	mode_wanted = MidiMode::Classical;
	creation_mode = CreationMode::Note;
	input_mode = InputMode::Default;
	midi_interval = 3;
	chord_type = ChordType::Minor;
	chord_inversion = 0;
	modifier = NoteModifier::None;

	moving = false;
	string_no = 0;
	octave = 4;

	input_wanted_active = false;
	input_capture = true;
	input_wanted_device = session->device_manager->choose_device(DeviceType::MidiInput);

	rep_key_runner = -1;
	rep_key = -1;
	rep_key_num = 0;

	maximize_input_volume = true;

	mouse_pre_moving_pos = -1;
}


void ViewModeEditMidi::set_modifier(NoteModifier mod) {
	if (mod == modifier) {
		modifier = NoteModifier::None;
	} else {
		modifier = mod;
	}
	if (modifier == NoteModifier::None)
		session->status(_("no modifier"));
	else
		session->status(modifier_symbol(modifier));
	out_changed.notify();
}

void ViewModeEditMidi::set_mode(MidiMode _mode) {
	mode_wanted = _mode;
	view->thm.set_dirty();
	view->force_redraw();
	out_changed.notify();
}

void ViewModeEditMidi::set_creation_mode(CreationMode _mode) {
	creation_mode = _mode;
	view->force_redraw();
	out_changed.notify();
}

void ViewModeEditMidi::set_input_mode(InputMode _mode) {
	input_mode = _mode;
	view->force_redraw();
	out_changed.notify();
}

bool ViewModeEditMidi::editing(AudioViewLayer *l) {
	return view->mode_edit->editing(l);
}

TrackLayer* ViewModeEditMidi::cur_layer() {
	return view->cur_layer();
}

AudioViewLayer* ViewModeEditMidi::cur_vlayer() {
	return view->cur_vlayer();
}


void ViewModeEditMidi::start_midi_preview(const Array<int> &pitch, float ttl) {
	preview->start(pitch, view->cur_track()->volume, ttl);
}

static os::Timer ri_timer;
static MidiEventBuffer ri_keys;

void ViewModeEditMidi::ri_insert() {
	if (ri_keys.num == 0)
		return;
	Range r = get_edit_range();
	for (auto &e: ri_keys) {
		float vol = e.volume;
		if (maximize_input_volume)
			vol = 1;
		view->cur_layer()->add_midi_note(make_note(r, e.pitch, -1, NoteModifier::Unknown, vol));
	}
	ri_keys.clear();
	view->set_cursor_pos(r.end());
	select_in_edit_cursor();
	ri_timer.get();

}


void ViewModeEditMidi::on_midi_input() {
	auto rec = (MidiAccumulator*)preview->accumulator.get();

	if (input_capture) {

		// insert
		for (auto &e: rec->buffer) {
			if (e.is_note_on()) {
				if (ri_keys.num > 0 and ri_timer.peek() > 0.3f)
					ri_insert();
				e.pos = 0;
				ri_keys.add(e);
				ri_timer.get();
			} else if (e.is_note_off()) {
				ri_insert();
			}
		}
	}
	preview->chain->command(ModuleCommand::AccumulationClear, 0);
}

bool ViewModeEditMidi::is_input_active() {
	return input_wanted_active;
}

void ViewModeEditMidi::activate_input(bool active) {
	input_wanted_active = active;
	if (active and !preview->input) {
		_start_input();
	} else if (!active and preview->input) {
		_stop_input();
	}
}

void ViewModeEditMidi::set_input_capture(bool capture) {
	input_capture = capture;
	out_changed.notify();
}

void ViewModeEditMidi::_start_input() {
	preview->_start_input();
	preview->chain->out_tick >> create_sink([this] { on_midi_input(); });
}

void ViewModeEditMidi::_stop_input() {
	preview->chain->unsubscribe(this);
	preview->_stop_input();
}

void ViewModeEditMidi::set_input_device(Device *d) {
	input_wanted_device = d;
	preview->set_input_device(d);
}

Device *ViewModeEditMidi::input_device() {
	if (preview)
		if (preview->input)
			return preview->input->get_device();
	return input_wanted_device;
}

void ViewModeEditMidi::on_start() {
	set_side_bar(SideBar::Index::MidiEditorConsole);
	preview = new MidiPreview(view->session, (Synthesizer*)cur_vlayer()->layer->track->synth->copy());
	preview->set_input_device(input_wanted_device);
	if (input_wanted_active)
		_start_input();

	on_cur_layer_change();

	if (!song->time_track())
		session->q(_("Midi editing is far easier with a metronome track. Do you want to add one?"), {"track-add-beats:" + _("yes")});
}

void ViewModeEditMidi::on_end() {
	preview = nullptr;

	for (auto *v: view->vlayers)
		v->scroll_bar->hidden = true;
}

class MouseDelayAddMidi : public scenegraph::MouseDelayAction {
public:
	AudioViewLayer *vlayer;
	AudioView *view;
	Array<int> pitch;
	int pos0;
	int clef_pos;
	MouseDelayAddMidi(AudioViewLayer *l, const Array<int> &_pitch, int _clef_pos) {
		vlayer = l;
		view = vlayer->view;
		pitch = _pitch;
		clef_pos = _clef_pos;
		pos0 = vlayer->view->hover().pos;
	}
	void on_start(const vec2 &m) override {
		view->mode_edit_midi->start_midi_preview(pitch, 1.0f);
	}
	void on_finish(const vec2 &m) override {
		auto notes = get_creation_notes(m);

		if (notes.num > 0) {
			view->set_cursor_pos(notes[0]->range.end());
			view->mode_edit_midi->select_in_edit_cursor();
			vlayer->layer->add_midi_notes(notes);
			notes.clear(); // all notes owned by track now
		}
	}
	void on_clean_up() override {
		view->mode_edit_midi->preview->end();
	}
	void on_draw_post(Painter *c) override {
		auto *mp = vlayer->midi_context();
		mp->set_force_shadows(true);

		// current creation
		auto notes = get_creation_notes(view->cursor());
		mp->draw(c, notes);
	}
	MidiNoteBuffer get_creation_notes(const vec2 &m) {
		Range r = Range::to(pos0, view->get_mouse_pos(m));
		r = r.canonical();

		// align to beats
		if (view->song->bars.num > 0)
			align_to_beats(view->song, r, view->mode_edit_midi->sub_beat_partition);

		// collision?
		Range allowed = get_allowed_midi_range(vlayer->layer, pitch, pos0);

		// create notes
		MidiNoteBuffer notes;
		if (allowed.is_empty())
			return notes;
		for (int p: pitch)
			notes.add(new MidiNote(r and allowed, p, 1));
		if (notes.num > 0) {
			auto mode = vlayer->midi_mode();
			if (mode == MidiMode::Classical) {
				//s.clef_position = mp->screen_to_clef_pos(m.y);
				notes[0]->clef_position = clef_pos;
				int upos = vlayer->track()->instrument.get_clef().position_to_uniclef(clef_pos);
				notes[0]->modifier = combine_note_modifiers(view->mode_edit_midi->modifier, view->mode_edit_midi->cur_scale().get_modifier(upos));
			}
		}
		return notes;
	}
};

struct MidiHoverMetadata {
	int string_no = -1;
	int clef_pos = -1;
	NoteModifier mod = NoteModifier::None;
	int pitch = -1;
};

MidiHoverMetadata get_midi_hover_meta(ViewModeEditMidi *m) {
	MidiHoverMetadata r;

	auto l = m->view->cur_vlayer();
	auto mode = l->midi_mode();

	if (m->hover().type == HoverData::Type::ClefPosition) {
		if (mode == MidiMode::Tab)
			r.string_no = clamp(m->hover().index, 0, m->view->cur_track()->instrument.string_pitch.num - 1);
		if (mode == MidiMode::Classical) {
			r.clef_pos = m->hover().index;
			int upos = l->track()->instrument.get_clef().position_to_uniclef(r.clef_pos);
			r.mod = combine_note_modifiers(m->modifier, m->cur_scale().get_modifier(upos));
			r.pitch = uniclef_to_pitch(upos, r.mod);
		}
	} else if (m->hover().type == HoverData::Type::MidiPitch) {
		r.pitch = m->hover().index;
	}
	return r;
}

// note clicking already handled by ViewModeDefault!
void ViewModeEditMidi::left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) {

	if (!view->sel.has(vlayer->layer)) {
		view->exclusively_select_layer(vlayer);
		return;
	}

	auto mode = cur_vlayer()->midi_mode();

	auto hmi = get_midi_hover_meta(this);

	if (mode == MidiMode::Tab) {
		string_no = hmi.string_no;
	} else if (mode == MidiMode::Classical) {
		octave = pitch_get_octave(hmi.pitch);
	}

	if (creation_mode == CreationMode::Select ) {
		view->set_cursor_pos(hover().pos_snap);
		select_in_edit_cursor();
		start_selection_rect(m, SelectionMode::Rect);

	} else /* note / chord */ {
		if (mode == MidiMode::Classical) {
			if (hover().type == HoverData::Type::ClefPosition) {
				auto pitch = get_creation_pitch(hmi.pitch);
				view->mdp_run(new MouseDelayAddMidi(vlayer, pitch, hmi.clef_pos), m);
			}
		} else if (mode == MidiMode::Linear) {
			if (hover().type == HoverData::Type::MidiPitch) {
				auto pitch = get_creation_pitch(hmi.pitch);
				view->mdp_run(new MouseDelayAddMidi(vlayer, pitch, hmi.clef_pos), m);
			}
		} else /* TAB */ {
			view->set_cursor_pos(hover().pos_snap);
		}
	}
	view->exclusively_select_layer(vlayer);
}

bool hover_end_of_note(HoverData &h, MidiNote *n) {
	return h.pos >= n->range.end() - n->range.length*0.1f;
}

void ViewModeEditMidi::left_click_handle_object(AudioViewLayer *vlayer, const vec2 &m) {

	view->exclusively_select_layer(vlayer);
	if (!view->hover_selected_object()) {
		view->exclusively_select_object();
		view->set_current(hover());
	}

	// start drag'n'drop?
	if (auto n = hover().note) {
		if (hover_end_of_note(hover(), n))
			view->mdp_prepare(new MouseDelayNotesScaleDnD(vlayer, view->sel.filter({vlayer->layer}).filter(SongSelection::Mask::MidiNotes)), m);
		else
			view->mdp_prepare(new MouseDelayNotesDnD(vlayer, view->sel.filter({vlayer->layer}).filter(SongSelection::Mask::MidiNotes)), m);
	} else {
		ViewModeDefault::left_click_handle_object(vlayer, m);
	}
}

void ViewModeEditMidi::edit_add_pause() {
	Range r = get_edit_range();
	view->set_cursor_pos(r.end());
	select_in_edit_cursor();
}

Array<MidiKeyChange> get_key_changes(const TrackLayer *l);

Scale ViewModeEditMidi::cur_scale() {
	Scale scale = Scale::C_MAJOR;
	for (auto &kc: get_key_changes(view->cur_layer()))
		if (kc.pos < get_edit_range().offset)
			scale = kc.key;
	return scale;
}

void ViewModeEditMidi::edit_add_note_by_urelative(int urelative, bool shift) {
	Range r = get_edit_range();
	int upos = octave * 7 + urelative;
	const Clef& clef = view->cur_track()->instrument.get_clef();
	int clef_pos = upos - clef.offset;

	auto scale_modifier = cur_scale().get_modifier(upos);
	NoteModifier mod = combine_note_modifiers(modifier, scale_modifier);
	if (modifier == NoteModifier::None and shift) {
		static const NoteModifier qnd_mod[7] = {NoteModifier::Sharp,
		NoteModifier::Sharp,NoteModifier::Flat,
		NoteModifier::Sharp,NoteModifier::Sharp,
		NoteModifier::Flat,NoteModifier::Flat};
		if (scale_modifier == NoteModifier::None)
		 	mod = qnd_mod[urelative];
		else
			mod = NoteModifier::None;
	}

	int pitch = uniclef_to_pitch(upos);
	pitch = modifier_apply(pitch, mod);
	view->cur_layer()->add_midi_note(make_note(r, pitch, clef_pos, mod));
	view->set_cursor_pos(r.end());
	select_in_edit_cursor();
	start_midi_preview({pitch}, 0.1f);
}

void ViewModeEditMidi::edit_add_note_on_string(int hand_pos) {
	Range r = get_edit_range();
	int pitch = cur_layer()->track->instrument.string_pitch[string_no] + hand_pos;
	MidiNote *n = new MidiNote(r, pitch, 1.0f);
	n->stringno = string_no;
	cur_layer()->add_midi_note(n);
	view->set_cursor_pos(r.end());
	select_in_edit_cursor();
	start_midi_preview({pitch}, 0.1f);
}

void ViewModeEditMidi::edit_backspace() {
	Range r = get_backwards_range();
	view->set_cursor_pos(r.offset);
	auto s = get_select_in_edit_cursor();
	view->song->delete_selection(s);
}

void ViewModeEditMidi::jump_string(int delta) {
	string_no = max(min(string_no + delta, cur_layer()->track->instrument.string_pitch.num - 1), 0);
	select_in_edit_cursor();
	view->force_redraw();
}

void ViewModeEditMidi::jump_octave(int delta) {
	octave = max(min(octave + delta, 7), 0);
	select_in_edit_cursor();
	view->force_redraw();
}

void ViewModeEditMidi::set_rep_key(int k) {
	if (rep_key_runner >= 0)
		hui::cancel_runner(rep_key_runner);
	rep_key_runner = hui::run_later(0.8f, [this] {
		rep_key_runner = -1;
		rep_key_num = -1;
		rep_key = -1;
	});

	if (k == rep_key)
		rep_key_num ++;
	else
		rep_key_num = 1;
	rep_key = k;
}

int song_bar_divisor(Song *s, int pos);

void set_note_lengthx(ViewModeEditMidi *m, int l, int p, int n, const string &text) {
	[[maybe_unused]] int div = song_bar_divisor(m->view->song, m->view->cursor_pos());
	//l *= div;

	if ((m->sub_beat_partition % p) == 0) {
		m->set_note_length_and_partition(m->sub_beat_partition / p * n, m->sub_beat_partition);
	} else {
		m->set_note_length_and_partition(l * n, p);
	}
	m->view->set_cursor_pos(m->view->cursor_pos());

	string t;
	if (n > 4) {
		t = text + u8" \u00d7 " + i2s(n);
	} else {
		t = (text + " ").repeat(n).trim();
	}
	m->session->status(t);
}

void ViewModeEditMidi::on_key_down(int k) {
	if ((k & hui::KEY_CONTROL) or (k & hui::KEY_ALT))
		return;
	bool shift = (k & hui::KEY_SHIFT);
	int pure_key = k & 0xff;

	set_rep_key(k);
	auto mode = cur_vlayer()->midi_mode();
	if ((mode == MidiMode::Classical) or (mode == MidiMode::Linear)) {
		if (input_mode == InputMode::Default) {
			if (k == hui::KEY_0 or k == hui::KEY_1){
				set_modifier(NoteModifier::None);
			} else if (k == hui::KEY_2) {
				set_modifier(NoteModifier::Sharp);
			} else if (k == hui::KEY_3) {
				set_modifier(NoteModifier::Flat);
			} else if (k == hui::KEY_4) {
				set_modifier(NoteModifier::Natural);
			}

			// add note
			if ((pure_key >= hui::KEY_A) and (pure_key <= hui::KEY_G)) {
				int number = (pure_key - hui::KEY_A);
				static const int urel[7] = {5,6,0,1,2,3,4};
				edit_add_note_by_urelative(urel[number], shift);
			}
		}

		// select octave
		if (k == hui::KEY_UP)
			jump_octave(1);
		if (k == hui::KEY_DOWN)
			jump_octave(-1);
	} else if (mode == MidiMode::Tab) {
		if (input_mode == InputMode::Default) {

			// add note
			if (((pure_key >= hui::KEY_0) and (pure_key <= hui::KEY_9)) or ((pure_key >= hui::KEY_A) and (pure_key <= hui::KEY_F))) {
				int number = (pure_key - hui::KEY_0);
				if (pure_key >= hui::KEY_A)
					number = 10 + (pure_key - hui::KEY_A);
				if (shift)
					number += 10;
				edit_add_note_on_string(number);
			}
		}

		// select string
		if (k == hui::KEY_UP)
			jump_string(1);
		if (k == hui::KEY_DOWN)
			jump_string(-1);
	}
	

	// remove
	if (k == hui::KEY_BACKSPACE)
		edit_backspace();

	if (input_mode == InputMode::NoteLength) {
		if (((k >= hui::KEY_1) and (k <= hui::KEY_9)) or ((k >= hui::KEY_A) and (k <= hui::KEY_F))) {
			int number = (k - hui::KEY_0);
			if (k >= hui::KEY_A)
				number = 10 + (k - hui::KEY_A);
			set_note_length_and_partition(number, sub_beat_partition);
			set_input_mode(InputMode::Default);
		}
	} else if (input_mode == InputMode::BeatPartition) {
		if (((k >= hui::KEY_1) and (k <= hui::KEY_9)) or ((k >= hui::KEY_A) and (k <= hui::KEY_F))) {
			int number = (k - hui::KEY_0);
			if (k >= hui::KEY_A)
				number = 10 + (k - hui::KEY_A);
			set_note_length_and_partition(note_length, number);
			set_input_mode(InputMode::Default);
		}
	}

	if (k == hui::KEY_Q)
		// quarter
		set_note_lengthx(this, 1, 1, rep_key_num, u8"\U0001d15f");
	if (k == hui::KEY_W)
		// 8th
		set_note_lengthx(this, 1, 2, rep_key_num, u8"\U0001d160");
	if (k == hui::KEY_S)
		// 16th
		set_note_lengthx(this, 1, 4, rep_key_num, u8"\U0001d161");

	if (k == hui::KEY_T)
		set_note_lengthx(this, 1, 3, rep_key_num, "⅓");


	if (k == hui::KEY_L)
		set_input_mode(InputMode::NoteLength);
	if (k == hui::KEY_P)
		set_input_mode(InputMode::BeatPartition);
//	if (k == hui::KEY_ESCAPE)
//		set_input_mode(InputMode::DEFAULT);

	//if (k == hui::KEY_ESCAPE)
	//	session->set_mode(EditMode::Default);

	ViewModeDefault::on_key_down(k);
}

void ViewModeEditMidi::on_command(const string &id) {

	// cursor
	if (id == "cursor-move-left") {
		Range r = get_backwards_range();
		view->set_cursor_pos(r.offset);
		select_in_edit_cursor();
		return;
	}
	if (id == "cursor-move-right") {
		edit_add_pause();
		return;
	}

	ViewModeDefault::on_command(id);
}

float ViewModeEditMidi::layer_suggested_height(AudioViewLayer *l) {
	if (editing(l)) {
		auto mode = l->midi_mode();
		if (mode == MidiMode::Linear)
			return theme.MAX_TRACK_CHANNEL_HEIGHT * 8;
		else if (mode == MidiMode::Classical)
			return theme.MAX_TRACK_CHANNEL_HEIGHT * 3;
		else // TAB
			return theme.MAX_TRACK_CHANNEL_HEIGHT * 3;
	}

	return ViewModeDefault::layer_suggested_height(l);
}

void ViewModeEditMidi::on_cur_layer_change() {
	view->thm.set_dirty();
	auto sb = cur_vlayer()->scroll_bar;
	sb->set_content(0, MaxPitch);
	sb->set_view_size(EDIT_PITCH_SHOW_COUNT);
	sb->set_view_offset(MaxPitch - cur_vlayer()->edit_pitch_max);

	for (auto l: view->vlayers)
		l->scroll_bar->hidden = true;
	sb->hidden = false;
	sb->out_offset >> create_data_sink<float>([this] (float offset) {
		float _pitch_max = MaxPitch - offset;
		cur_vlayer()->set_edit_pitch_min_max(_pitch_max - EDIT_PITCH_SHOW_COUNT, _pitch_max);
	});
}


Array<int> ViewModeEditMidi::get_creation_pitch(int base_pitch) {
	if (creation_mode == CreationMode::Interval) {
		if (midi_interval != 0)
			return {base_pitch, base_pitch + midi_interval};
	} else if (creation_mode == CreationMode::Chord) {
		return chord_notes(chord_type, chord_inversion, base_pitch);
	} else if (creation_mode == CreationMode::Note) {
		return {base_pitch};
	}
	return {};
}

MidiNoteBuffer ViewModeEditMidi::get_creation_notes(HoverData *sel, int pos0) {
	int start = min(pos0, sel->pos);
	int end = max(pos0, sel->pos);
	Range r = Range::to(start, end);

	auto *l = cur_vlayer();
	if (!l)
		return MidiNoteBuffer();
	[[maybe_unused]] auto mode = l->midi_mode();

	// align to beats
	if (song->bars.num > 0)
		align_to_beats(song, r, sub_beat_partition);

	auto hmi = get_midi_hover_meta(this);

	Array<int> pitch = get_creation_pitch(hmi.pitch);
	auto vlayer = view->cur_vlayer();

	// collision?
	Range allowed = get_allowed_midi_range(vlayer->layer, pitch, pos0);

	// create notes
	MidiNoteBuffer notes;
	if (allowed.is_empty())
		return notes;
	for (int p: pitch)
		notes.add(new MidiNote(r and allowed, p, 1));
	if (notes.num > 0) {
		// FIXME: clef/modifier for root in inversions...
		notes[0]->clef_position = hmi.clef_pos;
		notes[0]->modifier = hmi.mod;
	}
	return notes;
}

void ViewModeEditMidi::set_note_length_and_partition(int length, int partition) {
	note_length = max(length, 1);
	sub_beat_partition = max(partition, 1);
	select_in_edit_cursor();
	view->force_redraw();
	out_changed.notify();
}

void ViewModeEditMidi::draw_layer_background(Painter *c, AudioViewLayer *l) {
	if (editing(l)) {
		view->grid_painter->set_context(l->area, l->grid_colors());
		view->grid_painter->draw_empty_background(c);
		view->grid_painter->draw_whatever(c, sub_beat_partition);

		if (l->layer->type == SignalType::Midi) {
			auto *mp = l->midi_context();
			mp->set_force_shadows(true);
			mp->set_synthesizer(l->layer->track->synth.get());
			mp->draw_background(c, true);
		}
	} else {
		ViewModeDefault::draw_layer_background(c, l);
	}
}

inline bool hover_note_classical(const MidiNote &n, int clef_position, int pos) {
	if (n.clef_position != clef_position)
		return false;
	return n.range.is_inside(pos);
}

inline bool hover_note_tab(const MidiNote &n, int string_no, int pos) {
	if (n.stringno != string_no)
		return false;
	return n.range.is_inside(pos);
}

inline bool hover_note_linear(const MidiNote &n, int pitch, int pos) {
	if (n.pitch != pitch)
		return false;
	return n.range.is_inside(pos);
}

HoverData ViewModeEditMidi::get_hover_data(AudioViewLayer *vlayer, const vec2 &m) {
	auto s = ViewModeDefault::get_hover_data(vlayer, m);
	if (s.type != HoverData::Type::Layer)
		return s;
	if (!editing(vlayer))
		return s;
	auto *l = vlayer->layer;

	// midi
	auto mode = vlayer->midi_mode();

	auto *mp = vlayer->midi_context();

	/*if (creation_mode != CreationMode::SELECT)*/{
		if (mode == MidiMode::Classical) {
			s.index = mp->screen_to_clef_pos(m.y);
			s.type = HoverData::Type::ClefPosition;

			for (auto *n: weak(l->midi))
				if (hover_note_classical(*n, s.index, s.pos)) {
					s.note = n;
					s.type = HoverData::Type::MidiNote;
					return s;
				}
		} else if (mode == MidiMode::Tab) {
			s.index = mp->screen_to_string(m.y);
			s.type = HoverData::Type::ClefPosition;

			for (auto *n: weak(l->midi))
				if (hover_note_tab(*n, s.index, s.pos)) {
					s.note = n;
					s.type = HoverData::Type::MidiNote;
					return s;
				}
		} else if (mode == MidiMode::Linear) {
			s.index = mp->y2pitch(m.y, NoteModifier::None);
			s.type = HoverData::Type::MidiPitch;

			for (auto *n: weak(l->midi))
				if (hover_note_linear(*n, s.index, s.pos)) {
					s.note = n;
					s.type = HoverData::Type::MidiNote;
					return s;
				}
		}
	}

	return s;
}



void ViewModeEditMidi::draw_post(Painter *c) {
	ViewModeDefault::draw_post(c);

	auto *l = cur_vlayer();
	if (!l)
		return;
	auto mode = l->midi_mode();
	Range r = get_edit_range();
	float x1, x2;
	view->cam.range2screen(r, x1, x2);

	auto *mp = l->midi_context();
	mp->set_force_shadows(true);

	// creation preview
	if (mode == MidiMode::Classical) {
		if (!hui::get_event()->lbut and (hover().type == HoverData::Type::ClefPosition)) {
			auto notes = get_creation_notes(&hover(), hover().pos);
			mp->draw(c, notes);
		}
	} else if (mode == MidiMode::Linear) {
		if (!hui::get_event()->lbut and (hover().type == HoverData::Type::MidiPitch)) {
			auto notes = get_creation_notes(&hover(), hover().pos);
			mp->draw(c, notes);
		}
	}


	l->scroll_bar->hidden = true;

	if (mode == MidiMode::Classical) {

	} else if (mode == MidiMode::Linear) {
		l->scroll_bar->hidden = false;
	}


	// editing rect
	auto xxx = c->clip();
	c->set_clip(l->area and view->song_area());
	c->set_color(theme.text_soft1);
	c->set_fill(false);
	c->set_line_dash({5,5},0);
	if (mode == MidiMode::Tab) {
		int y = mp->string_to_screen(string_no);
		int y1 = y - mp->get_clef_dy() / 2;
		int y2 = y + mp->get_clef_dy() / 2;
		c->draw_rect(rect(x1, x2, y1, y2));
	} else if (mode == MidiMode::Classical) {
		int p1 = pitch_from_octave_and_rel(0, octave);
		int p2 = pitch_from_octave_and_rel(0, octave+1);
		int y1 = mp->pitch2y(p2);
		int y2 = mp->pitch2y(p1);
		c->draw_rect(rect(x1, x2, y1, y2));
	} else if (mode == MidiMode::Linear) {
		int p1 = pitch_from_octave_and_rel(0, octave);
		int p2 = pitch_from_octave_and_rel(0, octave+1);
		int y1 = mp->pitch2y(p2);
		int y2 = mp->pitch2y(p1);
		c->draw_rect(rect(x1, x2, y1, y2));
	}
	c->set_clip(xxx);
	c->set_line_dash({},0);
	c->set_fill(true);
}

string ViewModeEditMidi::get_tip() {
	if (input_mode == InputMode::NoteLength)
		return _("enter note length [1-9], [A-F]    cancel [Esc]");
	if (input_mode == InputMode::BeatPartition)
		return _("enter beat partition [1-9], [A-F]    cancel [Esc]");
	string message = _("cursor [←,→]");
	string message2 = _("    track [Alt+↑,↓]    delete [⟵]    note length,partition [L,P]");
	message2 += u8"    \U0001d15f  ,\U0001d160  ,\U0001d161  ,\U0001d160/₃  [Q,W,S,T]";
	if (!cur_vlayer())
		return message + message2;
	auto mode = cur_vlayer()->midi_mode();
	if (mode == MidiMode::Tab) {
		message += _("    string [↑,↓]");
		message2 += _("    add note [0-9], [A-F]");
	} else if ((mode == MidiMode::Classical) or (mode == MidiMode::Linear)) {
		message += _("    octave [↑,↓]");
		message2 += _("    modifiers [1-4]    add note [A-G]");
	}
	return message + message2;
}

int ViewModeEditMidi::suggest_move_cursor(const Range &cursor, bool forward) {
	int pos = cursor.start();
	if (forward)
		pos = cursor.end();

	if (cursor.length > 0)
		return pos;

	if (forward) {
		Range rr = song->bars.get_sub_beats(pos, sub_beat_partition, note_length);
		if (rr.length > 0)
			return rr.end();
	} else {
		Range rr = song->bars.get_sub_beats(pos, sub_beat_partition, -note_length);
		if (rr.length > 0)
			return rr.start();
	}

	// in case we ran out of bars
	return pos + (forward ? 1 : -1) * note_length * session->sample_rate() / sub_beat_partition;
}

// seems fine
Range ViewModeEditMidi::get_edit_range() {
	// manual selection has priority
	if (view->sel.range().length > 0)
		return view->sel.range();

	int pos = view->cursor_pos();
	return Range::to(pos, suggest_move_cursor(Range(pos, 0), true));
}


Range ViewModeEditMidi::get_backwards_range() {
	// manual selection has priority
	if (view->sel.range().length > 0)
		return view->sel.range();

	int pos = view->cursor_pos();
	return Range::to(suggest_move_cursor(view->sel.range(), false), pos);
}

SongSelection ViewModeEditMidi::get_select_in_edit_cursor() {
	Range r = get_edit_range();
	auto s = SongSelection::from_range(view->song, r).filter({view->cur_layer()}).filter(SongSelection::Mask::MidiNotes);
	auto mode = cur_vlayer()->midi_mode();
	auto notes = s._notes;
	if (mode == MidiMode::Tab) {
		for (auto *n: notes)
			if (n->stringno != string_no)
				s.set(n, false);
	} else if (mode == MidiMode::Classical or mode == MidiMode::Linear) {
		for (auto *n: notes) {
			if (pitch_get_octave(n->pitch) != octave)
				s.set(n, false);
		}
	}
	return s;
}

void ViewModeEditMidi::select_in_edit_cursor() {
	view->sel._notes = get_select_in_edit_cursor()._notes;
	view->update_selection();
}

SongSelection ViewModeEditMidi::get_selection_for_rect(const Range &r, int y0, int y1) {
	return ViewModeDefault::get_selection_for_rect(r, y0, y1).filter(SongSelection::Mask::MidiNotes);
}

SongSelection ViewModeEditMidi::get_selection_for_range(const Range &r) {
	return ViewModeDefault::get_selection_for_range(r).filter(SongSelection::Mask::MidiNotes);
}

}
