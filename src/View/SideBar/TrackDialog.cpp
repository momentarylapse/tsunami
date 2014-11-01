/*
 * TrackDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "TrackDialog.h"
#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "../Helper/BarList.h"
#include "../BottomBar/BottomBar.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "../../Action/Track/Synthesizer/ActionTrackEditSynthesizer.h"

TrackDialog::TrackDialog(AudioView *_view) :
	SideBarConsole(_("Spur-Eigenschaften")),
	Observer("TrackDialog")
{
	view = _view;
	track = NULL;
	setBorderWidth(5);
	fromResource("track_dialog");
	setDecimals(1);
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar");


	expand("ld_t_synth", 0, true);
	expand("ld_t_bars", 0, true);
	expand("ld_t_effects", 0, true);

	loadData();
	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);

	event("name", this, &TrackDialog::onName);
	event("volume", this, &TrackDialog::onVolume);
	event("panning", this, &TrackDialog::onPanning);
	event("synthesizer", this, &TrackDialog::onSynthesizer);
	event("config_synth", this, &TrackDialog::onConfigSynthesizer);
	event("edit_midi", this, &TrackDialog::onEditMidi);
}

TrackDialog::~TrackDialog()
{
	unsubscribe(view);
	if (track)
		unsubscribe(track);
	delete(bar_list);
}

void TrackDialog::loadData()
{
	enable("name", track);
	enable("volume", track);
	enable("panning", track);
	bar_list->setTrack(track);
	if (track){
		setString("name", track->name);
		setOptions("name", "placeholder=" + track->GetNiceName());
		setFloat("volume", amplitude2db(track->volume));
		setFloat("panning", track->panning * 100.0f);
		setString("synthesizer", track->synth->name);
		hideControl("ld_t_synth", track->type == track->TYPE_AUDIO);
		//Enable("config_synth", track->type != track->TYPE_AUDIO);
		hideControl("ld_t_bars", track->type != Track::TYPE_TIME);
		hideControl("ld_t_midi", track->type != Track::TYPE_MIDI);
	}else{
		hideControl("ld_t_synth", true);
		hideControl("ld_t_bars", true);
		hideControl("ld_t_midi", true);
		//Enable("synthesizer", track);
		//Enable("config_synth", track);
	}
}

void TrackDialog::setTrack(Track *t)
{
	if (track)
		unsubscribe(track);
	track = t;
	loadData();
	if (track)
		subscribe(track);
}

void TrackDialog::onName()
{
	track->SetName(getString(""));
}

void TrackDialog::onVolume()
{
	track->SetVolume(db2amplitude(getFloat("volume")));
}

void TrackDialog::onPanning()
{
	track->SetPanning(getFloat("panning") / 100.0f);
}

void TrackDialog::onSynthesizer()
{
	Synthesizer *s = ChooseSynthesizer(tsunami->win, track->synth->name);
	if (s)
		track->SetSynthesizer(s);
}

void TrackDialog::onConfigSynthesizer()
{
	tsunami->win->bottom_bar->choose(BottomBar::SYNTH_CONSOLE);
}

void TrackDialog::onEditMidi()
{
	tsunami->win->bottom_bar->choose(BottomBar::MIDI_EDITOR);
}

void TrackDialog::onUpdate(Observable *o, const string &message)
{
	if (o == view){
		setTrack(view->cur_track);
	}else if ((o == track) && (message == track->MESSAGE_DELETE)){
		setTrack(NULL);
	}else{
		loadData();
	}
}
