/*
 * MidiEditorConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../../Data/MidiData.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../Mode/ViewModeMidi.h"
#include "../../Plugins/ConfigPanel.h"
#include "../../Plugins/MidiEffect.h"
#include "../../Plugins/PluginManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../BottomBar/BottomBar.h"
#include "MidiEditorConsole.h"



MidiEditorConsole::MidiEditorConsole(AudioView *_view, Song *_song) :
	SideBarConsole(_("Midi")),
	Observer("MidiEditorConsole")
{
	view = _view;
	song = _song;

	fromResource("midi_editor");

	id_inner = "midi_fx_inner_table";

	setInt("interval", view->mode_midi->midi_interval);

	for (int i=0; i<NUM_CHORD_TYPES; i++)
		addString("chord_type", chord_type_name(i));
	setInt("chord_type", 0);
	addString("chord_inversion", _("Grundform"));
	addString("chord_inversion", _("1. Umkehrung"));
	addString("chord_inversion", _("2. Umkehrung"));
	setInt("chord_inversion", 0);

	for (int i=0; i<12; i++)
		addString("scale_root", rel_pitch_name(11 - i));
	for (int i=0; i<NUM_SCALE_TYPES; i++)
		addString("scale_type", scale_type_name(i));
	setInt("scale_root", 11 - view->mode_midi->midi_scale_root);
	setInt("scale_type", view->mode_midi->midi_scale_type);

	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("beat_partition", this, &MidiEditorConsole::onBeatPartition);
	event("scale_root", this, &MidiEditorConsole::onScale);
	event("scale_type", this, &MidiEditorConsole::onScale);
	event("midi_edit_mode", this, &MidiEditorConsole::onMidiEditMode);
	event("interval", this, &MidiEditorConsole::onInterval);
	event("chord_type", this, &MidiEditorConsole::onChordType);
	event("chord_inversion", this, &MidiEditorConsole::onChordInversion);
	event("reference_track", this, &MidiEditorConsole::onReferenceTrack);
	event("edit_track", this, &MidiEditorConsole::onEditTrack);
	event("edit_midi_fx", this, &MidiEditorConsole::onEditMidiFx);
	event("edit_song", this, &MidiEditorConsole::onEditSong);

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	subscribe(view, view->MESSAGE_VTRACK_CHANGE);
	update();
}

MidiEditorConsole::~MidiEditorConsole()
{
	clear();
	unsubscribe(view);
	unsubscribe(song);
}

void MidiEditorConsole::update()
{
	bool allow = false;
	if (view->cur_track)
		allow = (view->cur_track->type == Track::TYPE_MIDI);
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);
}

void MidiEditorConsole::onUpdate(Observable* o, const string &message)
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

void MidiEditorConsole::onScale()
{
	view->mode_midi->setScale(getInt("scale_type"), 11 - getInt("scale_root"));
}

void MidiEditorConsole::onBeatPartition()
{
	view->mode_midi->setBeatPartition(getInt(""));
}

void MidiEditorConsole::onMidiEditMode()
{
	int n = getInt("midi_edit_mode");
	if (n == 0){
		view->mode_midi->midi_mode = ViewModeMidi::MIDI_MODE_NOTE;
	}else if (n == 1){
		view->mode_midi->midi_mode = ViewModeMidi::MIDI_MODE_INTERVAL;
	}else if (n == 2){
		view->mode_midi->midi_mode = ViewModeMidi::MIDI_MODE_CHORD;
	}
}

void MidiEditorConsole::onInterval()
{
	view->mode_midi->midi_interval = getInt("");
}

void MidiEditorConsole::onChordType()
{
	view->mode_midi->chord_type = getInt("");
}

void MidiEditorConsole::onChordInversion()
{
	view->mode_midi->chord_inversion = getInt("");
}

void MidiEditorConsole::onReferenceTrack()
{
	int n = getInt("") - 1;
	int tn = track->get_index();
	view->vtrack[tn]->reference_track = n;
	view->forceRedraw();
}

void MidiEditorConsole::onEditTrack()
{
	((SideBar*)parent)->open(SideBar::TRACK_CONSOLE);
}

void MidiEditorConsole::onEditMidiFx()
{
	((SideBar*)parent)->open(SideBar::MIDI_FX_CONCOLE);
}

void MidiEditorConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void MidiEditorConsole::clear()
{
	if (track)
		unsubscribe(track);
	track = NULL;
	setInt("reference_track", 0);
	//Enable("add", false);
}

void MidiEditorConsole::onEnter()
{
	view->setMode(view->mode_midi);
}

void MidiEditorConsole::onLeave()
{
	view->setMode(view->mode_default);
}

void MidiEditorConsole::setTrack(Track *t)
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

