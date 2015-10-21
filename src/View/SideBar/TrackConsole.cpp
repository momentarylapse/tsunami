/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "TrackConsole.h"

TrackConsole::TrackConsole(AudioView *_view) :
	SideBarConsole(_("Spur Eigenschaften")),
	Observer("TrackConsole")
{
	view = _view;
	track = NULL;
	setBorderWidth(5);
	fromResource("track_dialog");
	setDecimals(1);

	loadData();
	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", this, &TrackConsole::onName);
	event("volume", this, &TrackConsole::onVolume);
	event("panning", this, &TrackConsole::onPanning);

	event("edit_song", this, &TrackConsole::onEditSong);
	event("edit_fx", this, &TrackConsole::onEditFx);
	event("edit_midi", this, &TrackConsole::onEditMidi);
	event("edit_midi_fx", this, &TrackConsole::onEditMidiFx);
	event("edit_synth", this, &TrackConsole::onEditSynth);
	event("edit_bars", this, &TrackConsole::onEditBars);
}

TrackConsole::~TrackConsole()
{
	unsubscribe(view);
	if (track)
		unsubscribe(track);
}

void TrackConsole::loadData()
{
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	hideControl("td_t_edit", !track);
	if (track){
		setString("name", track->name);
		setOptions("name", "placeholder=" + track->getNiceName());
		setFloat("volume", amplitude2db(track->volume));
		setFloat("panning", track->panning * 100.0f);
		enable("edit_midi", track->type == Track::TYPE_MIDI);
		enable("edit_midi_fx", track->type == Track::TYPE_MIDI);
		enable("edit_synth", track->type != Track::TYPE_AUDIO);
	}else{
		hideControl("td_t_bars", true);
	}
}

void TrackConsole::setTrack(Track *t)
{
	if (track)
		unsubscribe(track);
	track = t;
	loadData();
	if (track)
		subscribe(track);
}

void TrackConsole::onName()
{
	track->setName(getString(""));
}

void TrackConsole::onVolume()
{
	track->setVolume(db2amplitude(getFloat("volume")));
}

void TrackConsole::onPanning()
{
	track->setPanning(getFloat("panning") / 100.0f);
}

void TrackConsole::applyData()
{
}

void TrackConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void TrackConsole::onEditFx()
{
	((SideBar*)parent)->open(SideBar::FX_CONSOLE);
}

void TrackConsole::onEditMidi()
{
	((SideBar*)parent)->open(SideBar::MIDI_EDITOR);
}

void TrackConsole::onEditMidiFx()
{
	((SideBar*)parent)->open(SideBar::MIDI_FX_CONCOLE);
}

void TrackConsole::onEditSynth()
{
	((SideBar*)parent)->open(SideBar::SYNTH_CONSOLE);
}

void TrackConsole::onEditBars()
{
	((SideBar*)parent)->open(SideBar::BARS_CONSOLE);
}

void TrackConsole::onUpdate(Observable *o, const string &message)
{
	if (o == view){
		setTrack(view->cur_track);
	}else if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else{
		loadData();
	}
}
