/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "../Helper/BarList.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "../BottomBar/BottomBar.h"
#include "TrackConsole.h"

TrackConsole::TrackConsole(AudioView *_view) :
	SideBarConsole(_("Spur Eigenschaften")),
	Observer("TrackConsole")
{
	view = _view;
	track = NULL;
	setBorderWidth(5);
	fromResource("track_dialog");
	setOptions("ttd_grid_1", "noexpandx,width=300");
	setDecimals(1);
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar", "edit_bars", view);


	expand("td_t_bars", 0, true);

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
}

TrackConsole::~TrackConsole()
{
	unsubscribe(view);
	if (track)
		unsubscribe(track);
	delete(bar_list);
}

void TrackConsole::loadData()
{
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	bar_list->setTrack(track);
	hideControl("td_t_edit", !track);
	if (track){
		setString("name", track->name);
		setOptions("name", "placeholder=" + track->getNiceName());
		setFloat("volume", amplitude2db(track->volume));
		setFloat("panning", track->panning * 100.0f);
		hideControl("td_t_bars", track->type != Track::TYPE_TIME);
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
	tsunami->win->side_bar->open(SideBar::SONG_CONSOLE);
}

void TrackConsole::onEditFx()
{
	tsunami->win->bottom_bar->open(BottomBar::TRACK_FX_CONSOLE);
}

void TrackConsole::onEditMidi()
{
	tsunami->win->side_bar->open(SideBar::MIDI_EDITOR);
}

void TrackConsole::onEditMidiFx()
{
	tsunami->win->bottom_bar->open(BottomBar::TRACK_MIDI_FX_CONCOLE);
}

void TrackConsole::onEditSynth()
{
	tsunami->win->side_bar->open(SideBar::SYNTH_CONSOLE);
}

/*void TrackConsole::onSynthesizer()
{
	Synthesizer *s = ChooseSynthesizer(tsunami->win, track->synth->name);
	if (s)
		track->setSynthesizer(s);
}*/

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
