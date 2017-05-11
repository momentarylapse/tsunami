/*
 * MidiEditorConsole.cpp
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../../Midi/MidiData.h"
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

int get_track_index_save(Song *song, Track *t);


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
	addString("chord_inversion", _("Basic form"));
	addString("chord_inversion", _("1st inversion"));
	addString("chord_inversion", _("2nd inversion"));
	setInt("chord_inversion", 0);

	for (int i=0; i<12; i++)
		addString("scale_root", rel_pitch_name(11 - i));
	for (int i=0; i<Scale::NUM_TYPES; i++)
		addString("scale_type", Scale::get_type_name(i));
	setInt("scale_root", 11 - view->midi_scale.root);
	setInt("scale_type", view->midi_scale.type);


	track = NULL;
	//Enable("add", false);
	enable("track_name", false);

	event("mode:linear", std::bind(&MidiEditorConsole::onViewModeLinear, this));
	event("mode:tab", std::bind(&MidiEditorConsole::onViewModeTab, this));
	event("mode:classical", std::bind(&MidiEditorConsole::onViewModeClassical, this));
	event("beat_partition", std::bind(&MidiEditorConsole::onBeatPartition, this));
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
	event("edit_track", std::bind(&MidiEditorConsole::onEditTrack, this));
	event("edit_midi_fx", std::bind(&MidiEditorConsole::onEditMidiFx, this));
	event("edit_song", std::bind(&MidiEditorConsole::onEditSong, this));

	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);
	subscribe(view, view->MESSAGE_VTRACK_CHANGE);
	subscribe(view, view->MESSAGE_SETTINGS_CHANGE);
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
		if (get_track_index_save(view->song, view->cur_track) >= 0)
			allow = (view->cur_track->type == Track::TYPE_MIDI);
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);

	if (!track)
		return;

	check("modifier:none", view->mode_midi->modifier == MODIFIER_NONE);
	check("modifier:sharp", view->mode_midi->modifier == MODIFIER_SHARP);
	check("modifier:flat", view->mode_midi->modifier == MODIFIER_FLAT);
	check("modifier:natural", view->mode_midi->modifier == MODIFIER_NATURAL);

	int mode = view->mode->which_midi_mode(track);
	view->mode_midi->setMode(mode);
	check("mode:linear", mode == view->MIDI_MODE_LINEAR);
	check("mode:classical", mode == view->MIDI_MODE_CLASSICAL);
	check("mode:tab", mode == view->MIDI_MODE_TAB);

	enable("modifier:none", mode == view->MIDI_MODE_CLASSICAL);
	enable("modifier:sharp", mode == view->MIDI_MODE_CLASSICAL);
	enable("modifier:flat", mode == view->MIDI_MODE_CLASSICAL);
	enable("modifier:natural", mode == view->MIDI_MODE_CLASSICAL);


	if (track->instrument.type == Instrument::TYPE_DRUMS){
		// select a nicer pitch range in linear mode for drums
		view->vtrack[track->get_index()]->setPitchMinMax(34, 34 + 30);//PITCH_SHOW_COUNT);
	}
}

void MidiEditorConsole::onUpdate(Observable* o, const string &message)
{
	update();
	if ((o == track) and (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else if ((o == view) and (message == view->MESSAGE_CUR_TRACK_CHANGE)){
		setTrack(view->cur_track);
	}else if ((o == view) and (message == view->MESSAGE_VTRACK_CHANGE)){

		reset("reference_tracks");
		if (song){
			for (Track *t : song->tracks)
				addString("reference_tracks", t->getNiceName());
		}

		if (track){
			int tn = track->get_index();
			if ((tn >= 0) and (tn < view->vtrack.num))
				if (view->vtrack[tn])
					setSelection("reference_tracks", view->vtrack[tn]->reference_tracks);
		}
	}else{
		setTrack(track);
	}
}

void MidiEditorConsole::onScale()
{
	view->setScale(Scale(getInt("scale_type"), 11 - getInt("scale_root")));
}

void MidiEditorConsole::onBeatPartition()
{
	view->mode_midi->setBeatPartition(getInt(""));
}

void MidiEditorConsole::onCreationMode()
{
	int n = getInt("midi_edit_mode");
	if (n == 0){
		view->mode_midi->setCreationMode(ViewModeMidi::CREATION_MODE_NOTE);
	}else if (n == 1){
		view->mode_midi->setCreationMode(ViewModeMidi::CREATION_MODE_INTERVAL);
	}else if (n == 2){
		view->mode_midi->setCreationMode(ViewModeMidi::CREATION_MODE_CHORD);
	}
}

void MidiEditorConsole::onViewModeLinear()
{
	setMode(AudioView::MIDI_MODE_LINEAR);
}

void MidiEditorConsole::onViewModeClassical()
{
	setMode(AudioView::MIDI_MODE_CLASSICAL);
}

void MidiEditorConsole::onViewModeTab()
{
	setMode(AudioView::MIDI_MODE_TAB);
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

void MidiEditorConsole::onReferenceTracks()
{
	int tn = track->get_index();
	view->vtrack[tn]->reference_tracks = getSelection("");
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

void MidiEditorConsole::onModifierNone()
{
	view->mode_midi->modifier = MODIFIER_NONE;
}

void MidiEditorConsole::onModifierSharp()
{
	view->mode_midi->modifier = MODIFIER_SHARP;
}

void MidiEditorConsole::onModifierFlat()
{
	view->mode_midi->modifier = MODIFIER_FLAT;
}

void MidiEditorConsole::onModifierNatural()
{
	view->mode_midi->modifier = MODIFIER_NATURAL;
}

void MidiEditorConsole::clear()
{
	if (track)
		unsubscribe(track);
	track = NULL;
	setSelection("reference_tracks", Array<int>());
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
				setSelection("reference_tracks", view->vtrack[tn]->reference_tracks);

		update();
	}

}

void MidiEditorConsole::setMode(int mode)
{
	view->mode_midi->setMode(mode);
	update();
}

