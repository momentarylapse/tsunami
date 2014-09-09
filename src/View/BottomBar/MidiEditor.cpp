/*
 * MidiEditor.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "MidiEditor.h"
#include "../../Data/Track.h"
#include "../AudioView.h"

MidiEditor::MidiEditor(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Midi Editor")),
	Observer("MidiEditor")
{
	view = _view;
	audio = _audio;

	FromResource("midi_editor");

	Array<string> chord_types = GetChordTypeNames();
	foreach(string &ct, chord_types)
		AddString("chord_type", ct);
	SetInt("chord_type", 0);
	Enable("chord_type", false);
	AddString("chord_inversion", _("Grundform"));
	AddString("chord_inversion", _("1. Umkehrung"));
	AddString("chord_inversion", _("2. Umkehrung"));
	SetInt("chord_inversion", 0);
	Enable("chord_inversion", false);

	EventM("pitch_offset", this, &MidiEditor::OnPitch);
	EventM("beat_partition", this, &MidiEditor::OnBeatPartition);
	EventM("insert_chord", this, &MidiEditor::OnInsertChord);
	EventM("chord_type", this, &MidiEditor::OnChordType);
	EventM("chord_inversion", this, &MidiEditor::OnChordInversion);

	Subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	update();
}

MidiEditor::~MidiEditor()
{
}

void MidiEditor::update()
{
	bool allow = false;
	if (view->cur_track)
		allow = (view->cur_track->type == Track::TYPE_MIDI);
	HideControl("me_grid_yes", !allow);
	HideControl("me_grid_no", allow);
}

void MidiEditor::OnUpdate(Observable* o, const string &message)
{
	update();
}


void MidiEditor::OnPitch()
{
	view->pitch_min = GetInt("");
	view->pitch_max = GetInt("") + 30;
	view->ForceRedraw();
}

void MidiEditor::OnBeatPartition()
{
	view->beat_partition = GetInt("");
	view->ForceRedraw();
}

void MidiEditor::OnInsertChord()
{
	if (IsChecked(""))
		view->chord_mode = GetInt("chord_type");
	else
		view->chord_mode = CHORD_TYPE_NONE;
	Enable("chord_type", IsChecked(""));
	Enable("chord_inversion", IsChecked(""));
}

void MidiEditor::OnChordType()
{
	view->chord_mode = GetInt("");
}

void MidiEditor::OnChordInversion()
{
	view->chord_inversion = GetInt("");
}

