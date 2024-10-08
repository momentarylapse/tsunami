/*
 * MidiEditorConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "MidiEditorConsole.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../dialog/MarkerDialog.h"
#include "../dialog/QuestionDialog.h"
#include "../dialog/SelectStringDialog.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../mode/ViewModeEditMidi.h"
#include "../module/ConfigPanel.h"
#include "../TsunamiWindow.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/rhythm/BarCollection.h"
#include "../../data/rhythm/Beat.h"
#include "../../data/midi/MidiData.h"
#include "../../device/DeviceManager.h"
#include "../../device/Device.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../plugins/PluginManager.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../lib/hui/language.h"

namespace tsunami {

//int get_track_index_save(Song *song, Track *t);


enum class NoteBaseLength {
	Whole,
	Half,
	Quarter,
	Eigth,
	Sixteenth
};

enum class NoteLengthModifier {
	None,
	Dotted,
	Triplet
};


MidiEditorConsole::MidiEditorConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Midi editor"), "midi-editor", session, bar)
{
	from_resource("midi_editor");

	mode = view->mode_edit_midi;
	set_int("interval", mode->midi_interval);


	layer = nullptr;
	enable("track_name", false);

	event("length-whole", [this] { on_base_length(NoteBaseLength::Whole); });
	event("length-half", [this] { on_base_length(NoteBaseLength::Half); });
	event("length-quarter", [this] { on_base_length(NoteBaseLength::Quarter); });
	event("length-eighth", [this] { on_base_length(NoteBaseLength::Eigth); });
	event("length-sixteenth", [this] { on_base_length(NoteBaseLength::Sixteenth); });
	event("length-dotted", [this] { on_length_dotted(); });
	event("length-triplet", [this] { on_length_triplet(); });
	event("length-custom", [this] { on_length_custom(); });

	event("mode-select", [this] { mode->set_creation_mode(ViewModeEditMidi::CreationMode::Select); });
	event("mode-note", [this] { mode->set_creation_mode(ViewModeEditMidi::CreationMode::Note); });
	event("mode-interval", [this] { mode->set_creation_mode(ViewModeEditMidi::CreationMode::Interval); });
	event("mode-chord", [this] { mode->set_creation_mode(ViewModeEditMidi::CreationMode::Chord); });

	event("mode-classical", [this] { view->cur_vtrack()->set_midi_mode(MidiMode::Classical); });
	event("mode-tab", [this] { view->cur_vtrack()->set_midi_mode(MidiMode::Tab); });
	event("mode-linear", [this] { view->cur_vtrack()->set_midi_mode(MidiMode::Linear); });


	event("interval", [this] { on_interval(); });
	event("chord-major", [this] { on_chord_type(ChordType::Major); });
	event("chord-minor", [this] { on_chord_type(ChordType::Minor); });
	event("chord-diminished", [this] { on_chord_type(ChordType::Diminished); });
	event("chord-augmented", [this] { on_chord_type(ChordType::Augmented); });
	event("chord-major7", [this] { on_chord_type(ChordType::MajorSeventh); });
	event("chord-minor7", [this] { on_chord_type(ChordType::MinorSeventh); });
	event("chord-minor-major7", [this] { on_chord_type(ChordType::MinorMajorSeventh); });
	event("chord-diminished7", [this] { on_chord_type(ChordType::DiminishedSeventh); });
	event("chord-dominant7", [this] { on_chord_type(ChordType::DominantSeventh); });
	event("chord-diminished7", [this] { on_chord_type(ChordType::DiminishedSeventh); });
	event("chord-half-diminished7", [this] { on_chord_type(ChordType::HalfDiminishedSeventh); });
	event("chord-augmented7", [this] { on_chord_type(ChordType::AugmentedSeventh); });
	event("chord-augmented-major7", [this] { on_chord_type(ChordType::AugmentedMajorSeventh); });
	event("chord-inversion-none", [this] { on_chord_inversion(0); });
	event("chord-inversion-1", [this] { on_chord_inversion(1); });
	event("chord-inversion-2", [this] { on_chord_inversion(2); });
	event_x("reference_tracks", "hui:select", [this] { on_reference_tracks(); });
	event("modifier-none", [this] { on_modifier(NoteModifier::None); });
	event("modifier-sharp", [this] { on_modifier(NoteModifier::Sharp); });
	event("modifier-flat", [this] { on_modifier(NoteModifier::Flat); });
	event("modifier-natural", [this] { on_modifier(NoteModifier::Natural); });
	event("input_active", [this] { on_input_active(); });
	event("input_capture", [this] { on_input_capture(); });
	event("input", [this] { on_input_source(); });
	event("input_volume:key", [this] { on_input_volume(0); });
	event("input_volume:max", [this] { on_input_volume(1); });
	event("quantize", [this] { on_quantize(); });
	event("apply_string", [this] { on_apply_string(); });
	event("apply_hand_position", [this] { on_apply_hand_position(); });
	event("apply_pitch_shift", [this] { on_apply_pitch_shift(); });
	event("action-source", [this] { on_apply_source(); });
	event("action-effect", [this] { on_apply_effect(); });
	event("flag-none", [this] { on_apply_flags(0); });
	event("flag-trill", [this] { on_apply_flags(NoteFlag::Trill); });
	event("flag-staccato", [this] { on_apply_flags(NoteFlag::Staccato); });
	event("flag-tenuto", [this] { on_apply_flags(NoteFlag::Tenuto); });
	event("flag-dead", [this] { on_apply_flags(NoteFlag::Dead); });
	event("flag-bend-half", [this] { on_apply_flags(NoteFlag::BendHalf); });
	event("flag-bend-full", [this] { on_apply_flags(NoteFlag::BendFull); });
	event("add_key_change", [this] { on_add_key_change(); });
	event("edit-song", [session] {
		session->set_mode(EditMode::DefaultSong);
	});
	event("edit-track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	/*event("edit-midi-fx", [session] {
		session->set_mode(EditMode::DefaultMidiFx);
	});
	event("edit-synth", [session] {
		session->set_mode(EditMode::DefaultTrackSynth);
	});*/
	event("edit-track-curves", [session] {
		session->set_mode(EditMode::Curves);
	});
}

MidiEditorConsole::~MidiEditorConsole() {
	clear();
}

//NoteBaseLength analyse_note_length(MidiEditorConsole *c, NoteLengthModifier &mod) {
//}

bool is_dotted(MidiEditorConsole *c) {
	if (c->mode->sub_beat_partition*6 == c->mode->note_length) // whole
		return true;
	if (c->mode->sub_beat_partition*3 == c->mode->note_length) // half
		return true;
	if (c->mode->sub_beat_partition*3 == c->mode->note_length*2) // quarter
		return true;
	if (c->mode->sub_beat_partition*3 == c->mode->note_length*4) // eighth
		return true;
	if (c->mode->sub_beat_partition*3 == c->mode->note_length*8) // sixteenth
		return true;
	return false;
}

bool is_triplet(MidiEditorConsole *c) {
	if (c->mode->sub_beat_partition*4 == c->mode->note_length*3) // half
		return true;
	if (c->mode->sub_beat_partition*2 == c->mode->note_length*3) // quarter
		return true;
	if (c->mode->sub_beat_partition == c->mode->note_length*3) // eighth
		return true;
	if (c->mode->sub_beat_partition == c->mode->note_length*6) // sixteenth
		return true;
	return false;
}

bool base_is_whole(MidiEditorConsole *c) {
	if (is_dotted(c))
		return c->mode->sub_beat_partition*6 == c->mode->note_length;
	if (is_triplet(c))
		return c->mode->sub_beat_partition*8 == c->mode->note_length*3;
	return c->mode->sub_beat_partition*4 == c->mode->note_length;
}

bool base_is_half(MidiEditorConsole *c) {
	if (is_dotted(c))
		return c->mode->sub_beat_partition*3 == c->mode->note_length;
	if (is_triplet(c))
		return c->mode->sub_beat_partition*4 == c->mode->note_length*3;
	return c->mode->sub_beat_partition*2 == c->mode->note_length;
}

bool base_is_quarter(MidiEditorConsole *c) {
	if (is_dotted(c))
		return c->mode->sub_beat_partition*3 == c->mode->note_length*2;
	if (is_triplet(c))
		return c->mode->sub_beat_partition*2 == c->mode->note_length*3;
	return c->mode->sub_beat_partition == c->mode->note_length;
}

bool base_is_eighth(MidiEditorConsole *c) {
	if (is_dotted(c))
		return c->mode->sub_beat_partition*3 == c->mode->note_length*4;
	if (is_triplet(c))
		return c->mode->sub_beat_partition == c->mode->note_length*3;
	return c->mode->sub_beat_partition == c->mode->note_length*2;
}

bool base_is_sixteenth(MidiEditorConsole *c) {
	if (is_dotted(c))
		return c->mode->sub_beat_partition*3 == c->mode->note_length*8;
	if (is_triplet(c))
		return c->mode->sub_beat_partition == c->mode->note_length*6;
	return c->mode->sub_beat_partition == c->mode->note_length*4;
}

void MidiEditorConsole::update() {
	layer = view->cur_layer();
	if (!layer)
		return;

	[[maybe_unused]] bool allow = false;
	//if (get_track_index_save(view->song, view->cur_track) >= 0)
		allow = (layer->type == SignalType::Midi);

	check("mode-select", mode->creation_mode == ViewModeEditMidi::CreationMode::Select);
	check("mode-note", mode->creation_mode == ViewModeEditMidi::CreationMode::Note);
	check("mode-interval", mode->creation_mode == ViewModeEditMidi::CreationMode::Interval);
	check("mode-chord", mode->creation_mode == ViewModeEditMidi::CreationMode::Chord);

	check("mode-classical", view->cur_vlayer()->midi_mode() == MidiMode::Classical);
	check("mode-tab", view->cur_vlayer()->midi_mode() == MidiMode::Tab);
	check("mode-linear", view->cur_vlayer()->midi_mode() == MidiMode::Linear);

	expand("revealer-interval", mode->creation_mode == ViewModeEditMidi::CreationMode::Interval);
	expand("revealer-chord", mode->creation_mode == ViewModeEditMidi::CreationMode::Chord);

	check("chord-major", mode->chord_type == ChordType::Major);
	check("chord-minor", mode->chord_type == ChordType::Minor);
	check("chord-diminished", mode->chord_type == ChordType::Diminished);
	check("chord-augmented", mode->chord_type == ChordType::Augmented);
	check("chord-major7", mode->chord_type == ChordType::MajorSeventh);
	check("chord-minor7", mode->chord_type == ChordType::MinorSeventh);
	check("chord-minor-major7", mode->chord_type == ChordType::MinorMajorSeventh);
	check("chord-dominant7", mode->chord_type == ChordType::DominantSeventh);
	check("chord-diminished7", mode->chord_type == ChordType::DiminishedSeventh);
	check("chord-half-diminished7", mode->chord_type == ChordType::HalfDiminishedSeventh);
	check("chord-augmented7", mode->chord_type == ChordType::AugmentedSeventh);
	check("chord-augmented-major7", mode->chord_type == ChordType::AugmentedMajorSeventh);
	check("chord-inversion-none", mode->chord_inversion == 0);
	check("chord-inversion-1", mode->chord_inversion == 1);
	check("chord-inversion-2", mode->chord_inversion == 2);

	//check("modifier-none", mode->modifier == NoteModifier::NONE);
	check("modifier-sharp", mode->modifier == NoteModifier::Sharp);
	check("modifier-flat", mode->modifier == NoteModifier::Flat);
	check("modifier-natural", mode->modifier == NoteModifier::Natural);

	MidiMode _mode = view->get_layer(layer)->midi_mode();

	//enable("modifier-none", _mode == MidiMode::CLASSICAL);
	enable("modifier-none", false);
	enable("modifier-sharp", _mode == MidiMode::Classical);
	enable("modifier-flat", _mode == MidiMode::Classical);
	enable("modifier-natural", _mode == MidiMode::Classical);

	set_int("midi_edit_mode", (int)mode->creation_mode);

	set_int("beat_partition", mode->sub_beat_partition);
	set_int("note_length", mode->note_length);
	string length = format(u8"(%d \U0001D15F / %d)", mode->note_length, mode->sub_beat_partition);
	set_string("length-result", length);

	check("length-whole", base_is_whole(this));
	check("length-half", base_is_half(this));
	check("length-quarter", base_is_quarter(this));
	check("length-eighth", base_is_eighth(this));
	check("length-sixteenth", base_is_sixteenth(this));
	check("length-dotted", is_dotted(this));
	check("length-triplet", is_triplet(this));

	check("input_active", mode->is_input_active());
	enable("input_capture", mode->is_input_active());
	check("input_capture", mode->input_capture);
	enable("input", mode->is_input_active());
	update_input_device_list();

	if (mode->maximize_input_volume)
		check("input_volume:max", true);
	else
		check("input_volume:key", true);
	enable("input_volume:key", mode->is_input_active());
	enable("input_volume:max", mode->is_input_active());


	int strings = layer->track->instrument.string_pitch.num;
	enable("mode-tab", strings > 0);
	enable("apply_string", strings > 0);
	enable("apply_hand_position", strings > 0);



	if (layer->track->instrument.type == Instrument::Type::Drums) {
		// select a nicer pitch range in linear mode for drums
//		view->get_layer(layer->track)->setPitchMinMax(34, 34 + 30);//PITCH_SHOW_COUNT);
		// TODO
	}
}

void MidiEditorConsole::update_input_device_list() {
	input_sources = session->device_manager->good_device_list(DeviceType::MidiInput);

	reset("input");
	for (auto *d: input_sources)
		set_string("input", d->get_name());

	foreachi(auto *d, input_sources, i)
		if (d == mode->input_device())
			set_int("input", i);
}

void simplify_fraction(int &a, int &b) {
	for (int i=2; i<20; i++)
		if ((a % i) == 0 and (b % i) == 0) {
			a /= i;
			b /= i;
		}
}

void MidiEditorConsole::on_base_length(NoteBaseLength l) {
	int partition = 1, length = 1;
	if (l == NoteBaseLength::Whole)
		length = 4;
	if (l == NoteBaseLength::Half)
		length = 2;
	if (l == NoteBaseLength::Quarter)
		length = 1;
	if (l == NoteBaseLength::Eigth)
		partition = 2;
	if (l == NoteBaseLength::Sixteenth)
		partition = 4;

	if (is_checked("length-dotted")) {
		partition *= 2;
		length *= 3;
	}
	if (is_checked("length-triplet")) {
		partition *= 3;
		length *= 2;
	}

	simplify_fraction(length, partition);
	mode->set_note_length_and_partition(length, partition);
}

NoteBaseLength MidiEditorConsole::get_base_length() {
	if (is_checked("length-whole"))
		return NoteBaseLength::Whole;
	if (is_checked("length-half"))
		return NoteBaseLength::Half;
	if (is_checked("length-quarter"))
		return NoteBaseLength::Quarter;
	if (is_checked("length-eighth"))
		return NoteBaseLength::Eigth;
	if (is_checked("length-sixteenth"))
		return NoteBaseLength::Sixteenth;
	return NoteBaseLength::Quarter; // ...
}

void MidiEditorConsole::on_length_dotted() {
	auto l = get_base_length();
	on_base_length(l);
}

void MidiEditorConsole::on_length_triplet() {
	auto l = get_base_length();
	on_base_length(l);
}

void MidiEditorConsole::on_length_custom() {
	QuestionDialogIntInt::ask(win, _("Custom note length and beat sub-partitions (of quarter notes):"), {_("Length"), _("Partition")}, {"range=1:20", "range=1:20"}, [this] (int a, int b) {
		if (!QuestionDialogIntInt::aborted)
			mode->set_note_length_and_partition(a, b);
	});
}

void MidiEditorConsole::on_layer_delete() {
	set_layer(nullptr);
}

void MidiEditorConsole::on_view_cur_layer_change() {
	set_layer(view->cur_layer());
}

void MidiEditorConsole::on_view_vtrack_change() {
	/*update();

	reset("reference_tracks");
	if (song) {
		for (Track *t: song->tracks)
			addString("reference_tracks", t->getNiceName());
	}

	if (layer) {
		//setSelection("reference_tracks", view->get_layer(layer)->reference_tracks);
	}*/
}

void MidiEditorConsole::on_settings_change() {
	update();
}

void MidiEditorConsole::on_creation_mode() {
	int n = get_int("midi_edit_mode");
	if (n == 0) {
		mode->set_creation_mode(ViewModeEditMidi::CreationMode::Select);
	} else if (n == 1) {
		mode->set_creation_mode(ViewModeEditMidi::CreationMode::Note);
	} else if (n == 2) {
		mode->set_creation_mode(ViewModeEditMidi::CreationMode::Interval);
	} else if (n == 3) {
		mode->set_creation_mode(ViewModeEditMidi::CreationMode::Chord);
	}
}

void MidiEditorConsole::on_interval() {
	mode->midi_interval = get_int("");
	mode->out_changed.notify();
}

void MidiEditorConsole::on_chord_type(ChordType t) {
	mode->chord_type = t;
	mode->out_changed.notify();
}

void MidiEditorConsole::on_chord_inversion(int i) {
	mode->chord_inversion = i;
	mode->out_changed.notify();
}

void MidiEditorConsole::on_reference_tracks() {
	/*int tn = track->get_index();
	view->vtrack[tn]->reference_tracks = getSelection("");
	view->forceRedraw();*/
}

void MidiEditorConsole::on_modifier(NoteModifier m) {
	if (m == mode->modifier)
		mode->set_modifier(NoteModifier::None);
	else
		mode->set_modifier(m);
}

void MidiEditorConsole::on_input_active() {
	bool a = is_checked("");
	mode->activate_input(a);
	enable("input", a);
	enable("input_volume:key", a);
	enable("input_volume:max", a);
	enable("input_capture", a);
}

void MidiEditorConsole::on_input_capture() {
	bool a = is_checked("");
	mode->set_input_capture(a);
}

void MidiEditorConsole::on_input_source() {
	int n = get_int("");
	if (n >= 0 and n < input_sources.num)
		mode->set_input_device(input_sources[n]);
}

void MidiEditorConsole::on_input_volume(int _mode) {
	mode->maximize_input_volume = (_mode == 1);
}

void MidiEditorConsole::clear() {
	if (layer)
		layer->unsubscribe(this);
	view->cur_vtrack()->unsubscribe(this);
	layer = nullptr;
	set_selection("reference_tracks", {});
}

void MidiEditorConsole::on_enter() {
	session->device_manager->out_add_device >> create_data_sink<Device*>([this] (Device*) { update_input_device_list(); });
	session->device_manager->out_remove_device >> create_data_sink<Device*>([this] (Device*) { update_input_device_list(); });
	view->out_cur_layer_changed >> create_sink([this] { on_view_cur_layer_change(); });
	view->out_vtrack_changed >> create_sink([this] { on_view_vtrack_change(); });
	mode->out_changed >> create_sink([this] { on_settings_change(); });
	set_layer(view->cur_layer());
}

void MidiEditorConsole::on_leave() {
	clear();
	session->device_manager->unsubscribe(this);
	mode->unsubscribe(this);
	view->unsubscribe(this);
}

void MidiEditorConsole::set_layer(TrackLayer *l) {
	clear();

	layer = l;
	if (layer) {

		layer->out_death >> create_sink([this] { on_layer_delete(); });
		view->cur_vtrack()->out_changed >> create_sink([this] { update(); });

		/*auto v = view->get_layer(layer);
		if (v)
			setSelection("reference_tracks", v->reference_tracks);*/

		update();
	}

}

int align_to_beats(int pos, Array<Beat> &beats) {
	int best = pos;
	int best_diff = 100000000;
	for (auto &b: beats) {
		int d = abs(b.range.offset - pos);
		if (d < best_diff) {
			best_diff = d;
			best = b.range.offset;
		}
	}
	return best;
}

void MidiEditorConsole::on_quantize() {
	auto beats = song->bars.get_beats(Range::ALL, true, mode->sub_beat_partition);

	song->begin_action_group("quantize midi");
	auto notes = layer->midi.get_notes_by_selection(view->sel);
	for (auto *n: weak(notes)) {
		view->sel.set(n, false);
		auto nn = n->copy();
		nn->range.set_start(align_to_beats(nn->range.start(), beats));
		nn->range.set_end(align_to_beats(nn->range.end(), beats));
		layer->delete_midi_note(n);
		layer->add_midi_note(nn);
		view->sel.add(nn);
	}
	song->end_action_group();
}

void MidiEditorConsole::on_apply_string() {
	auto dlg = new SelectStringDialog(win, layer->track->instrument.string_pitch);
	hui::fly(dlg).then([this, dlg] {
		if (dlg->result) {
			song->begin_action_group("midi apply string");
			auto notes = layer->midi.get_notes_by_selection(view->sel);
			for (auto *n: weak(notes))
				layer->midi_note_set_string(n, *dlg->result);
			song->end_action_group();
		}
	});
}

void MidiEditorConsole::on_apply_hand_position() {
	QuestionDialogInt::ask(win, _("Move selected notes to which hand position?"), "range=0:99").then([this] (int hand_position) {
		auto &string_pitch = layer->track->instrument.string_pitch;

		song->begin_action_group("midi apply hand position");
		auto notes = layer->midi.get_notes_by_selection(view->sel);
		for (auto *n: weak(notes)) {
			int stringno = 0;
			for (int i=0; i<string_pitch.num; i++)
				if (n->pitch >= string_pitch[i] + hand_position) {
					stringno = i;
				}
			layer->midi_note_set_string(n, stringno);
		}
		song->end_action_group();
	});
}

void MidiEditorConsole::on_apply_pitch_shift() {
	QuestionDialogInt::ask(win, _("Move selected notes up by how many semi tones?"), "range=-99:99").then([this] (int delta) {
		song->begin_action_group("midi pitch shift");
		auto notes = layer->midi.get_notes_by_selection(view->sel);
		for (auto *n: weak(notes))
			layer->edit_midi_note(n, n->range, n->pitch + delta, n->volume);
		song->end_action_group();
	});
}

void MidiEditorConsole::on_apply_flags(int mask) {
	song->begin_action_group("midi apply flags");
	auto notes = layer->midi.get_notes_by_selection(view->sel);
	if (mask == 0) {
		for (auto *n: weak(notes))
			layer->midi_note_set_flags(n, 0);
	} else {
		for (auto *n: weak(notes))
			layer->midi_note_set_flags(n, n->flags | mask);

	}
	song->end_action_group();
}

void MidiEditorConsole::on_apply_effect() {
	ModuleSelectorDialog::choose(win, session, ModuleCategory::MidiEffect).then([this] (const string &name) {
		session->win->on_menu_execute_midi_effect(name);
	});
}

void MidiEditorConsole::on_apply_source() {
	ModuleSelectorDialog::choose(win, session, ModuleCategory::MidiSource).then([this] (const string &name) {
		session->win->on_menu_execute_midi_source(name);
	});
}

void MidiEditorConsole::on_add_key_change() {
	hui::fly(new MarkerDialog(win, layer, Range(view->cursor_pos(), 0), "::key=c-major::"));
}

}
