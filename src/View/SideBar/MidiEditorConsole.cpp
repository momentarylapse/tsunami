/*
 * MidiEditorConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/Rhythm/BarCollection.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Action/ActionManager.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ConfigPanel.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
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
	fromResource("midi_editor");

	id_inner = "midi_fx_inner_table";

	setInt("interval", view->mode_midi->midi_interval);

	for (int i=0; i<(int)ChordType::NUM; i++)
		addString("chord_type", chord_type_name((ChordType)i));
	setInt("chord_type", 0);
	addString("chord_inversion", _("Basic form"));
	addString("chord_inversion", _("1st inversion"));
	addString("chord_inversion", _("2nd inversion"));
	setInt("chord_inversion", 0);

	for (int i=0; i<12; i++)
		addString("scale_root", rel_pitch_name(11 - i));
	for (int i=0; i<(int)Scale::Type::NUM_TYPES; i++)
		addString("scale_type", Scale::get_type_name((Scale::Type)i));
	setInt("scale_root", 11 - view->midi_scale.root);
	setInt("scale_type", (int)view->midi_scale.type);


	layer = nullptr;
	//Enable("add", false);
	enable("track_name", false);

	event("beat_partition", std::bind(&MidiEditorConsole::onBeatPartition, this));
	event("note_length", std::bind(&MidiEditorConsole::onNoteLength, this));
	event("scale_root", std::bind(&MidiEditorConsole::onScale, this));
	event("scale_type", std::bind(&MidiEditorConsole::onScale, this));
	event("midi_edit_mode", std::bind(&MidiEditorConsole::onCreationMode, this));
	event("interval", std::bind(&MidiEditorConsole::onInterval, this));
	event("chord_type", std::bind(&MidiEditorConsole::onChordType, this));
	event("chord_inversion", std::bind(&MidiEditorConsole::onChordInversion, this));
	eventX("reference_tracks", "hui:select", std::bind(&MidiEditorConsole::onReferenceTracks, this));
	event("modifier:none", std::bind(&MidiEditorConsole::onModifierNone, this));
	event("modifier:sharp", std::bind(&MidiEditorConsole::onModifierSharp, this));
	event("modifier:flat", std::bind(&MidiEditorConsole::onModifierFlat, this));
	event("modifier:natural", std::bind(&MidiEditorConsole::onModifierNatural, this));
	event("quantize", std::bind(&MidiEditorConsole::onQuantize, this));
	event("edit_track", std::bind(&MidiEditorConsole::onEditTrack, this));
	event("edit_midi_fx", std::bind(&MidiEditorConsole::onEditMidiFx, this));
	event("edit_song", std::bind(&MidiEditorConsole::onEditSong, this));

	view->subscribe(this, std::bind(&MidiEditorConsole::onViewCurLayerChange, this), view->MESSAGE_CUR_LAYER_CHANGE);
	view->subscribe(this, std::bind(&MidiEditorConsole::onViewVTrackChange, this), view->MESSAGE_VTRACK_CHANGE);
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
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);

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

	setInt("midi_edit_mode", (int)view->mode_midi->creation_mode);


	if (layer->track->instrument.type == Instrument::Type::DRUMS){
		// select a nicer pitch range in linear mode for drums
//		view->get_layer(layer->track)->setPitchMinMax(34, 34 + 30);//PITCH_SHOW_COUNT);
		msg_write("todo");
	}
}

void MidiEditorConsole::onLayerDelete()
{
	setLayer(nullptr);
}

void MidiEditorConsole::onViewCurLayerChange()
{
	setLayer(view->cur_layer());
}

void MidiEditorConsole::onViewVTrackChange()
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

void MidiEditorConsole::onScale()
{
	view->setScale(Scale((Scale::Type)getInt("scale_type"), 11 - getInt("scale_root")));
}

void MidiEditorConsole::onBeatPartition()
{
	view->mode_midi->setBeatPartition(getInt(""));
}

void MidiEditorConsole::onNoteLength()
{
	view->mode_midi->setNoteLength(getInt(""));
}

void MidiEditorConsole::onCreationMode()
{
	int n = getInt("midi_edit_mode");
	if (n == 0){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::SELECT);
	}else if (n == 1){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::NOTE);
	}else if (n == 2){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::INTERVAL);
	}else if (n == 3){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::CHORD);
	}
}

void MidiEditorConsole::onInterval()
{
	view->mode_midi->midi_interval = getInt("");
}

void MidiEditorConsole::onChordType()
{
	view->mode_midi->chord_type = (ChordType)getInt("");
}

void MidiEditorConsole::onChordInversion()
{
	view->mode_midi->chord_inversion = getInt("");
}

void MidiEditorConsole::onReferenceTracks()
{
	/*int tn = track->get_index();
	view->vtrack[tn]->reference_tracks = getSelection("");
	view->forceRedraw();*/
}

void MidiEditorConsole::onEditTrack()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void MidiEditorConsole::onEditMidiFx()
{
	bar()->open(SideBar::MIDI_FX_CONCOLE);
}

void MidiEditorConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void MidiEditorConsole::onModifierNone()
{
	view->mode_midi->modifier = NoteModifier::NONE;
}

void MidiEditorConsole::onModifierSharp()
{
	view->mode_midi->modifier = NoteModifier::SHARP;
}

void MidiEditorConsole::onModifierFlat()
{
	view->mode_midi->modifier = NoteModifier::FLAT;
}

void MidiEditorConsole::onModifierNatural()
{
	view->mode_midi->modifier = NoteModifier::NATURAL;
}

void MidiEditorConsole::clear()
{
	if (layer)
		layer->unsubscribe(this);
	layer = nullptr;
	setSelection("reference_tracks", {});
}

void MidiEditorConsole::on_enter()
{
	view->setMode(view->mode_midi);
}

void MidiEditorConsole::on_leave()
{
	view->setMode(view->mode_default);
}

void MidiEditorConsole::setLayer(TrackLayer *l)
{
	clear();

	layer = l;
	if (layer){
		layer->subscribe(this, std::bind(&MidiEditorConsole::onLayerDelete, this), layer->MESSAGE_DELETE);

		/*auto v = view->get_layer(layer);
		if (v)
			setSelection("reference_tracks", v->reference_tracks);*/

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

void MidiEditorConsole::onQuantize()
{
	auto beats = song->bars.get_beats(Range::ALL, true, true, view->mode_midi->beat_partition);

	song->action_manager->beginActionGroup();
	MidiNoteBufferRef ref = layer->midi.getNotesBySelection(view->sel);
	for (auto *n: ref){
		view->sel.set(n, false);
		MidiNote *nn = n->copy();
		nn->range.set_start(align_to_beats(nn->range.start(), beats));
		nn->range.set_end(align_to_beats(nn->range.end(), beats));
		layer->deleteMidiNote(n);
		layer->addMidiNote(nn);
		view->sel.add(nn);
	}
	song->action_manager->endActionGroup();
}
