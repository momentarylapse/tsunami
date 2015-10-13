/*
 * MidiEditor.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "MidiEditor.h"
#include "../../Data/Track.h"
#include "../../Data/MidiData.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../BottomBar/BottomBar.h"



MidiEditor::MidiEditor(AudioView *_view, Song *_song) :
	SideBarConsole(_("Midi")),
	Observer("MidiEditor")
{
	view = _view;
	song = _song;

	fromResource("midi_editor");

	id_inner = "midi_fx_inner_table";

	Array<string> chord_types = GetChordTypeNames();
	foreach(string &ct, chord_types)
		addString("chord_type", ct);
	setInt("chord_type", 0);
	enable("chord_type", false);
	addString("chord_inversion", _("Grundform"));
	addString("chord_inversion", _("1. Umkehrung"));
	addString("chord_inversion", _("2. Umkehrung"));
	setInt("chord_inversion", 0);
	enable("chord_inversion", false);

	for (int i=0; i<12; i++)
		addString("scale", rel_pitch_name(i) + " " + GetChordTypeName(CHORD_TYPE_MAJOR) + " / " + rel_pitch_name(pitch_to_rel(i + 9)) + " " + GetChordTypeName(CHORD_TYPE_MINOR));
	setInt("scale", view->midi_scale);

	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("pitch_offset", this, &MidiEditor::onPitch);
	event("beat_partition", this, &MidiEditor::onBeatPartition);
	event("scale", this, &MidiEditor::onScale);
	event("midi_mode:select", this, &MidiEditor::onMidiModeSelect);
	event("midi_mode:note", this, &MidiEditor::onMidiModeNote);
	event("midi_mode:chord", this, &MidiEditor::onMidiModeChord);
	event("chord_type", this, &MidiEditor::onChordType);
	event("chord_inversion", this, &MidiEditor::onChordInversion);
	event("reference_track", this, &MidiEditor::onReferenceTrack);
	event("edit_track", this, &MidiEditor::onEditTrack);
	event("edit_midi_fx", this, &MidiEditor::onEditMidiFx);
	event("edit_song", this, &MidiEditor::onEditSong);

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	subscribe(view, view->MESSAGE_VTRACK_CHANGE);
	update();
}

MidiEditor::~MidiEditor()
{
	clear();
	unsubscribe(view);
	unsubscribe(song);
}

void MidiEditor::update()
{
	bool allow = false;
	if (view->cur_track)
		allow = (view->cur_track->type == Track::TYPE_MIDI);
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);
}

void MidiEditor::onUpdate(Observable* o, const string &message)
{
	update();
	if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) && (message == view->MESSAGE_CUR_TRACK_CHANGE)){
		setTrack(view->cur_track);
	}else if ((o == view) && (message == view->MESSAGE_VTRACK_CHANGE)){

		reset("reference_track");
		addString("reference_track", _("   - keine Referenz -"));
		if (song){
			foreach(Track *t, song->tracks)
				addString("reference_track", t->getNiceName());
		}
		setInt("reference_track", 0);

		if (track){
			int tn = track->get_index();
			if ((tn >= 0) and (tn < view->vtrack.num))
				if (view->vtrack[tn])
					setInt("reference_track", view->vtrack[tn]->reference_track + 1);
		}
	}else{
		setTrack(track);
	}
}


void MidiEditor::onPitch()
{
	view->pitch_min = getInt("");
	view->pitch_max = getInt("") + 30;
	view->forceRedraw();
}

void MidiEditor::onScale()
{
	view->midi_scale = getInt("");
	view->forceRedraw();
}

void MidiEditor::onBeatPartition()
{
	view->beat_partition = getInt("");
	view->forceRedraw();
}

void MidiEditor::onMidiModeSelect()
{
	view->midi_mode = view->MIDI_MODE_SELECT;
	enable("chord_type", false);
	enable("chord_inversion", false);
}

void MidiEditor::onMidiModeNote()
{
	view->midi_mode = view->MIDI_MODE_NOTE;
	enable("chord_type", false);
	enable("chord_inversion", false);
}

void MidiEditor::onMidiModeChord()
{
	view->midi_mode = view->MIDI_MODE_CHORD;
	enable("chord_type", true);
	enable("chord_inversion", true);
}

void MidiEditor::onChordType()
{
	view->chord_type = getInt("");
}

void MidiEditor::onChordInversion()
{
	view->chord_inversion = getInt("");
}

void MidiEditor::onReferenceTrack()
{
	int n = getInt("") - 1;
	int tn = track->get_index();
	view->vtrack[tn]->reference_track = n;
	view->forceRedraw();
}

void MidiEditor::onEditTrack()
{
	((SideBar*)parent)->open(SideBar::TRACK_CONSOLE);
}

void MidiEditor::onEditMidiFx()
{
	tsunami->win->bottom_bar->open(BottomBar::TRACK_MIDI_FX_CONCOLE);
}

void MidiEditor::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void MidiEditor::clear()
{
	if (track)
		unsubscribe(track);
	track = NULL;
	setInt("reference_track", 0);
	//Enable("add", false);
}

void MidiEditor::setTrack(Track *t)
{
	clear();

	track = t;
	if (track){
		subscribe(track, track->MESSAGE_DELETE);
		subscribe(track, track->MESSAGE_ADD_MIDI_EFFECT);
		subscribe(track, track->MESSAGE_DELETE_MIDI_EFFECT);

		int tn = track->get_index();
		if ((tn >= 0) and (tn < view->vtrack.num))
			if (view->vtrack[tn])
				setInt("reference_track", view->vtrack[tn]->reference_track + 1);
	}

}

