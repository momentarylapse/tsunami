/*
 * TrackConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "../Helper/BarList.h"
#include "BottomBar.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "../../Action/Track/Synthesizer/ActionTrackEditSynthesizer.h"
#include "TrackConsole.h"

TrackConsole::TrackConsole(AudioView *_view) :
	BottomBarConsole(_("Spur")),
	Observer("TrackConsole")
{
	view = _view;
	track = NULL;
	setBorderWidth(5);
	fromResource("track_dialog");
	setOptions("ttd_grid_1", "noexpandx,width=300");
	setDecimals(1);
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar");


	expand("ld_t_synth", 0, true);
	expand("ld_t_bars", 0, true);
	expand("ld_t_effects", 0, true);

	loadData();
	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", this, &TrackConsole::onName);
	event("volume", this, &TrackConsole::onVolume);
	event("panning", this, &TrackConsole::onPanning);
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
	if (track){
		setString("name", track->name);
		setOptions("name", "placeholder=" + track->getNiceName());
		setFloat("volume", amplitude2db(track->volume));
		setFloat("panning", track->panning * 100.0f);
		hideControl("ld_t_bars", track->type != Track::TYPE_TIME);
	}else{
		hideControl("ld_t_bars", true);
	}
	hideControl("ld_t_synth", true);
	hideControl("ld_t_midi", true);
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
