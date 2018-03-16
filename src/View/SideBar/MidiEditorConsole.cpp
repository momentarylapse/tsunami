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


MidiEditorConsole::MidiEditorConsole(Session *session) :
	SideBarConsole(_("Midi"), session)
{
	fromResource("midi_editor");

	id_inner = "midi_fx_inner_table";

	setInt("interval", view->mode_midi->midi_interval);

	for (int i=0; i<ChordType::NUM; i++)
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

	view->subscribe(this, std::bind(&MidiEditorConsole::onViewCurTrackChange, this), view->MESSAGE_CUR_TRACK_CHANGE);
	view->subscribe(this, std::bind(&MidiEditorConsole::onViewVTrackChange, this), view->MESSAGE_VTRACK_CHANGE);
	view->subscribe(this, std::bind(&MidiEditorConsole::onUpdate, this), view->MESSAGE_SETTINGS_CHANGE);
	update();
}

MidiEditorConsole::~MidiEditorConsole()
{
	clear();
	view->unsubscribe(this);
	song->unsubscribe(this);
}

void MidiEditorConsole::update()
{
	bool allow = false;
	if (view->cur_track)
		if (get_track_index_save(view->song, view->cur_track) >= 0)
			allow = (view->cur_track->type == Track::Type::MIDI);
	hideControl("me_grid_yes", !allow);
	hideControl("me_grid_no", allow);
	hideControl(id_inner, !allow);

	if (!track)
		return;

	check("modifier:none", view->mode_midi->modifier == Modifier::NONE);
	check("modifier:sharp", view->mode_midi->modifier == Modifier::SHARP);
	check("modifier:flat", view->mode_midi->modifier == Modifier::FLAT);
	check("modifier:natural", view->mode_midi->modifier == Modifier::NATURAL);

	int mode = view->mode->which_midi_mode(track);
	view->mode_midi->setMode(mode);
	check("mode:linear", mode == view->MidiMode::LINEAR);
	check("mode:classical", mode == view->MidiMode::CLASSICAL);
	check("mode:tab", mode == view->MidiMode::TAB);

	enable("modifier:none", mode == view->MidiMode::CLASSICAL);
	enable("modifier:sharp", mode == view->MidiMode::CLASSICAL);
	enable("modifier:flat", mode == view->MidiMode::CLASSICAL);
	enable("modifier:natural", mode == view->MidiMode::CLASSICAL);

	setInt("midi_edit_mode", view->mode_midi->creation_mode);


	if (track->instrument.type == Instrument::Type::DRUMS){
		// select a nicer pitch range in linear mode for drums
		view->get_track(track)->setPitchMinMax(34, 34 + 30);//PITCH_SHOW_COUNT);
	}
}

void MidiEditorConsole::onTrackDelete()
{
	update();
	setTrack(NULL);
}

void MidiEditorConsole::onViewCurTrackChange()
{
	update();
	setTrack(view->cur_track);
}

void MidiEditorConsole::onViewVTrackChange()
{
	update();

	reset("reference_tracks");
	if (song){
		for (Track *t: song->tracks)
			addString("reference_tracks", t->getNiceName());
	}

	if (track){
		setSelection("reference_tracks", view->get_track(track)->reference_tracks);
	}
}

void MidiEditorConsole::onUpdate()
{
	update();
	setTrack(track);
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
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::SELECT);
	}else if (n == 1){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::NOTE);
	}else if (n == 2){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::INTERVAL);
	}else if (n == 3){
		view->mode_midi->setCreationMode(ViewModeMidi::CreationMode::CHORD);
	}
}

void MidiEditorConsole::onViewModeLinear()
{
	setMode(AudioView::MidiMode::LINEAR);
}

void MidiEditorConsole::onViewModeClassical()
{
	setMode(AudioView::MidiMode::CLASSICAL);
}

void MidiEditorConsole::onViewModeTab()
{
	setMode(AudioView::MidiMode::TAB);
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
	view->mode_midi->modifier = Modifier::NONE;
}

void MidiEditorConsole::onModifierSharp()
{
	view->mode_midi->modifier = Modifier::SHARP;
}

void MidiEditorConsole::onModifierFlat()
{
	view->mode_midi->modifier = Modifier::FLAT;
}

void MidiEditorConsole::onModifierNatural()
{
	view->mode_midi->modifier = Modifier::NATURAL;
}

void MidiEditorConsole::clear()
{
	if (track)
		track->unsubscribe(this);
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
		track->subscribe(this, std::bind(&MidiEditorConsole::onTrackDelete, this), track->MESSAGE_DELETE);
		track->subscribe(this, std::bind(&MidiEditorConsole::onUpdate, this), track->MESSAGE_ADD_MIDI_EFFECT);
		track->subscribe(this, std::bind(&MidiEditorConsole::onUpdate, this), track->MESSAGE_DELETE_MIDI_EFFECT);

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

