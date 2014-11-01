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
	SetBorderWidth(5);
	FromResource("track_dialog");
	SetDecimals(1);
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar");


	Expand("ld_t_synth", 0, true);
	Expand("ld_t_bars", 0, true);
	Expand("ld_t_effects", 0, true);

	loadData();
	subscribe(view, view->MESSAGE_CUR_TRACK_CHANGE);

	EventM("name", this, &TrackDialog::onName);
	EventM("volume", this, &TrackDialog::onVolume);
	EventM("panning", this, &TrackDialog::onPanning);
	EventM("synthesizer", this, &TrackDialog::onSynthesizer);
	EventM("config_synth", this, &TrackDialog::onConfigSynthesizer);
	EventM("edit_midi", this, &TrackDialog::onEditMidi);
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
	Enable("name", track);
	Enable("volume", track);
	Enable("panning", track);
	bar_list->SetTrack(track);
	if (track){
		SetString("name", track->name);
		SetOptions("name", "placeholder=" + track->GetNiceName());
		SetFloat("volume", amplitude2db(track->volume));
		SetFloat("panning", track->panning * 100.0f);
		SetString("synthesizer", track->synth->name);
		HideControl("ld_t_synth", track->type == track->TYPE_AUDIO);
		//Enable("config_synth", track->type != track->TYPE_AUDIO);
		HideControl("ld_t_bars", track->type != Track::TYPE_TIME);
		HideControl("ld_t_midi", track->type != Track::TYPE_MIDI);
	}else{
		HideControl("ld_t_synth", true);
		HideControl("ld_t_bars", true);
		HideControl("ld_t_midi", true);
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
	track->SetName(GetString(""));
}

void TrackDialog::onVolume()
{
	track->SetVolume(db2amplitude(GetFloat("volume")));
}

void TrackDialog::onPanning()
{
	track->SetPanning(GetFloat("panning") / 100.0f);
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
