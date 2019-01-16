/*
 * MidiEditorConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Rhythm/BarCollection.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Action/ActionManager.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ConfigPanel.h"
#include "../AudioView.h"
#include "../AudioViewLayer.h"
#include "../Mode/ViewModeMidi.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../BottomBar/BottomBar.h"
#include "MidiEditorConsole.h"

#include "../../Module/Midi/MidiEffect.h"

int get_track_index_save(Song *song, Track *t);


MidiEditorConsole::MidiEditorConsole(Session *session) :
	SideBarConsole(_("Midi"), session)
{
	from_resource("midi_editor");

	id_inner = "midi_fx_inner_table";

	set_int("interval", view->mode_midi->midi_interval);

	for (int i=0; i<(int)ChordType::NUM; i++)
		add_string("chord_type", chord_type_name((ChordType)i));
	set_int("chord_type", 0);
	add_string("chord_inversion", _("Basic form"));
	add_string("chord_inversion", _("1st inversion"));
	add_string("chord_inversion", _("2nd inversion"));
	set_int("chord_inversion", 0);

	for (int i=0; i<12; i++)
		add_string("scale_root", rel_pitch_name(11 - i));
	for (int i=0; i<(int)Scale::Type::NUM_TYPES; i++)
		add_string("scale_type", Scale::get_type_name((Scale::Type)i));
	set_int("scale_root", 11 - view->midi_scale.root);
	set_int("scale_type", (int)view->midi_scale.type);


	layer = nullptr;
	//Enable("add", false);
	enable("track_name", false);

	event("beat_partition", std::bind(&MidiEditorConsole::on_beat_partition, this));
	event("note_length", std::bind(&MidiEditorConsole::on_note_length, this));
	event("scale_root", std::bind(&MidiEditorConsole::on_scale, this));
	event("scale_type", std::bind(&MidiEditorConsole::on_scale, this));
	event("midi_edit_mode", std::bind(&MidiEditorConsole::on_creation_mode, this));
	event("interval", std::bind(&MidiEditorConsole::on_interval, this));
	event("chord_type", std::bind(&MidiEditorConsole::on_chord_type, this));
	event("chord_inversion", std::bind(&MidiEditorConsole::on_chord_inversion, this));
	event_x("reference_tracks", "hui:select", std::bind(&MidiEditorConsole::on_reference_tracks, this));
	event("modifier:none", std::bind(&MidiEditorConsole::on_modifier_none, this));
	event("modifier:sharp", std::bind(&MidiEditorConsole::on_modifier_sharp, this));
	event("modifier:flat", std::bind(&MidiEditorConsole::on_modifier_flat, this));
	event("modifier:natural", std::bind(&MidiEditorConsole::on_modifier_natural, this));
	event("quantize", std::bind(&MidiEditorConsole::on_quantize, this));
	event("apply_string", std::bind(&MidiEditorConsole::on_apply_string, this));
	event("apply_hand_position", std::bind(&MidiEditorConsole::on_apply_hand_position, this));
	event("edit_track", std::bind(&MidiEditorConsole::on_edit_track, this));
	event("edit_midi_fx", std::bind(&MidiEditorConsole::on_edit_midi_fx, this));
	event("edit_song", std::bind(&MidiEditorConsole::on_edit_song, this));

	view->subscribe(this, std::bind(&MidiEditorConsole::on_view_cur_layer_change, this), view->MESSAGE_CUR_LAYER_CHANGE);
	view->subscribe(this, std::bind(&MidiEditorConsole::on_view_vtrack_change, this), view->MESSAGE_VTRACK_CHANGE);
	view->mode_midi->subscribe(this, std::bind(&MidiEditorConsole::on_settings_change, this));
	update();
}

MidiEditorConsole::~MidiEditorConsole()
{
	clear();
	view->mode_midi->unsubscribe(this);
	view->unsubscribe(this);
}

void MidiEditorConsole::update()
{
	bool allow = false;
	if (layer)
		//if (get_track_index_save(view->song, view->cur_track) >= 0)
			allow = (layer->type == SignalType::MIDI);
	hide_control("me_grid_yes", !allow);
	hide_control("me_grid_no", allow);
	hide_control(id_inner, !allow);

	if (!layer)
		return;

	check("modifier:none", view->mode_midi->modifier == NoteModifier::NONE);
	check("modifier:sharp", view->mode_midi->modifier == NoteModifier::SHARP);
	check("modifier:flat", view->mode_midi->modifier == NoteModifier::FLAT);
	check("modifier:natural", view->mode_midi->modifier == NoteModifier::NATURAL);

	MidiMode mode = view->get_layer(layer)->midi_mode;

	enable("modifier:none", mode == MidiMode::CLASSICAL);
	enable("modifier:sharp", mode == MidiMode::CLASSICAL);
	enable("modifier:flat", mode == MidiMode::CLASSICAL);
	enable("modifier:natural", mode == MidiMode::CLASSICAL);

	set_int("midi_edit_mode", (int)view->mode_midi->creation_mode);

	set_int("beat_partition", view->mode_midi->sub_beat_partition);
	set_int("note_length", view->mode_midi->note_length);


	if (layer->track->instrument.type == Instrument::Type::DRUMS){
		// select a nicer pitch range in linear mode for drums
//		view->get_layer(layer->track)->setPitchMinMax(34, 34 + 30);//PITCH_SHOW_COUNT);
		msg_write("todo");
	}
}

void MidiEditorConsole::on_layer_delete()
{
	set_layer(nullptr);
}

void MidiEditorConsole::on_view_cur_layer_change()
{
	set_layer(view->cur_layer());
}

void MidiEditorConsole::on_view_vtrack_change()
{
	/*update();

	reset("reference_tracks");
	if (song){
		for (Track *t: song->tracks)
			addString("reference_tracks", t->getNiceName());
	}

	if (layer){
		//setSelection("reference_tracks", view->get_layer(layer)->reference_tracks);
	}*/
}

void MidiEditorConsole::on_settings_change()
{
	update();
}

void MidiEditorConsole::on_scale()
{
	view->set_scale(Scale((Scale::Type)get_int("scale_type"), 11 - get_int("scale_root")));
}

void MidiEditorConsole::on_beat_partition()
{
	view->mode_midi->set_sub_beat_partition(get_int(""));
}

void MidiEditorConsole::on_note_length()
{
	view->mode_midi->set_note_length(get_int(""));
}

void MidiEditorConsole::on_creation_mode()
{
	int n = get_int("midi_edit_mode");
	if (n == 0){
		view->mode_midi->set_creation_mode(ViewModeMidi::CreationMode::SELECT);
	}else if (n == 1){
		view->mode_midi->set_creation_mode(ViewModeMidi::CreationMode::NOTE);
	}else if (n == 2){
		view->mode_midi->set_creation_mode(ViewModeMidi::CreationMode::INTERVAL);
	}else if (n == 3){
		view->mode_midi->set_creation_mode(ViewModeMidi::CreationMode::CHORD);
	}
}

void MidiEditorConsole::on_interval()
{
	view->mode_midi->midi_interval = get_int("");
}

void MidiEditorConsole::on_chord_type()
{
	view->mode_midi->chord_type = (ChordType)get_int("");
}

void MidiEditorConsole::on_chord_inversion()
{
	view->mode_midi->chord_inversion = get_int("");
}

void MidiEditorConsole::on_reference_tracks()
{
	/*int tn = track->get_index();
	view->vtrack[tn]->reference_tracks = getSelection("");
	view->forceRedraw();*/
}

void MidiEditorConsole::on_edit_track()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void MidiEditorConsole::on_edit_midi_fx()
{
	bar()->open(SideBar::MIDI_FX_CONCOLE);
}

void MidiEditorConsole::on_edit_song()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void MidiEditorConsole::on_modifier_none()
{
	view->mode_midi->modifier = NoteModifier::NONE;
}

void MidiEditorConsole::on_modifier_sharp()
{
	view->mode_midi->modifier = NoteModifier::SHARP;
}

void MidiEditorConsole::on_modifier_flat()
{
	view->mode_midi->modifier = NoteModifier::FLAT;
}

void MidiEditorConsole::on_modifier_natural()
{
	view->mode_midi->modifier = NoteModifier::NATURAL;
}

void MidiEditorConsole::clear()
{
	if (layer)
		layer->unsubscribe(this);
	layer = nullptr;
	set_selection("reference_tracks", {});
}

void MidiEditorConsole::on_enter()
{
	view->set_mode(view->mode_midi);
}

void MidiEditorConsole::on_leave()
{
	view->set_mode(view->mode_default);
}

void MidiEditorConsole::set_layer(TrackLayer *l)
{
	clear();

	layer = l;
	if (layer){
		layer->subscribe(this, std::bind(&MidiEditorConsole::on_layer_delete, this), layer->MESSAGE_DELETE);

		/*auto v = view->get_layer(layer);
		if (v)
			setSelection("reference_tracks", v->reference_tracks);*/

		enable("apply_string", layer->track->instrument.string_pitch.num);
		enable("string_no", layer->track->instrument.string_pitch.num);
		set_options("string_no", format("range=1:%d", layer->track->instrument.string_pitch.num));

		update();
	}

}

int align_to_beats(int pos, Array<Beat> &beats)
{
	int best = pos;
	int best_diff = 100000000;
	for (auto &b: beats){
		int d = abs(b.range.offset - pos);
		if (d < best_diff){
			best_diff = d;
			best = b.range.offset;
		}
	}
	return best;
}

void MidiEditorConsole::on_quantize()
{
	auto beats = song->bars.get_beats(Range::ALL, true, true, view->mode_midi->sub_beat_partition);

	song->action_manager->group_begin();
	MidiNoteBufferRef ref = layer->midi.get_notes_by_selection(view->sel);
	for (auto *n: ref){
		view->sel.set(n, false);
		MidiNote *nn = n->copy();
		nn->range.set_start(align_to_beats(nn->range.start(), beats));
		nn->range.set_end(align_to_beats(nn->range.end(), beats));
		layer->delete_midi_note(n);
		layer->add_midi_note(nn);
		view->sel.add(nn);
	}
	song->action_manager->group_end();
}

void MidiEditorConsole::on_apply_string()
{
	int string_no = get_int("string_no") - 1;

	song->action_manager->group_begin();
	MidiNoteBufferRef ref = layer->midi.get_notes_by_selection(view->sel);
	for (auto *n: ref)
		layer->midi_note_set_string(n, string_no);
	song->action_manager->group_end();
}

void MidiEditorConsole::on_apply_hand_position()
{
	int hand_position = get_int("fret_no");
	auto &string_pitch = layer->track->instrument.string_pitch;

	song->action_manager->group_begin();
	MidiNoteBufferRef ref = layer->midi.get_notes_by_selection(view->sel);
	for (auto *n: ref){
 		int stringno = 0;
 		for (int i=0; i<string_pitch.num; i++)
			if (n->pitch >= string_pitch[i] + hand_position){
 				stringno = i;
 			}
		layer->midi_note_set_string(n, stringno);
	}
	song->action_manager->group_end();
}
