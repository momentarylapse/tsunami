/*
 * TrackDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "TrackDialog.h"
#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "../Helper/FxList.h"
#include "../Helper/BarList.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../Dialog/SynthesizerDialog.h"
#include "../../Tsunami.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "../../Action/Track/Synthesizer/ActionTrackEditSynthesizer.h"

TrackDialog::TrackDialog() :
	SideBarConsole(_("Spur-Eigenschaften"))
{
	track = NULL;
	SetBorderWidth(5);
	FromResource("track_dialog");
	SetDecimals(1);
	bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar");


	Expand("ld_t_synth", 0, true);
	Expand("ld_t_bars", 0, true);
	Expand("ld_t_effects", 0, true);

	LoadData();
	Subscribe(tsunami->audio);

	EventM("name", this, &TrackDialog::OnName);
	EventM("synthesizer", this, &TrackDialog::OnSynthesizer);
	EventM("config_synth", this, &TrackDialog::OnConfigSynthesizer);
	EventM("edit_midi_track", this, &TrackDialog::OnEditMidiTrack);
}

TrackDialog::~TrackDialog()
{
	Unsubscribe(tsunami->audio);
	if (bar_list)
		delete(bar_list);
}

void TrackDialog::LoadData()
{
	Enable("name", track);
	bar_list->SetTrack(track);
	if (track){
		SetString("name", track->name);
		SetString("synthesizer", track->synth->name);
		HideControl("ld_t_synth", track->type == track->TYPE_AUDIO);
		//Enable("config_synth", track->type != track->TYPE_AUDIO);
		HideControl("ld_t_midi", track->type != Track::TYPE_MIDI);
		HideControl("ld_t_bars", track->type != Track::TYPE_TIME);
	}else{
		HideControl("ld_t_synth", true);
		HideControl("ld_t_midi", true);
		HideControl("ld_t_bars", true);
		//Enable("synthesizer", track);
		//Enable("config_synth", track);
		//Enable("edit_midi_track", false);
	}
}

void TrackDialog::SetTrack(Track *t)
{
	track = t;
	LoadData();
}

void TrackDialog::OnName()
{
	track->SetName(GetString(""));
}

void TrackDialog::OnSynthesizer()
{
	Synthesizer *s = ChooseSynthesizer(tsunami, track->synth->name);
	if (s)
		track->SetSynthesizer(s);
}

void TrackDialog::OnConfigSynthesizer()
{
	string params_old = track->synth->ConfigToString();
	if (tsunami->plugin_manager->ConfigureSynthesizer(track->synth))
		track->root->Execute(new ActionTrackEditSynthesizer(track, params_old));
}

void TrackDialog::OnEditMidiTrack()
{
	tsunami->view->SetEditModeMidi(track);
}

void TrackDialog::OnUpdate(Observable *o, const string &message)
{
	if (!track)
		return;
	bool ok = false;
	foreach(Track *t, tsunami->audio->track)
		if (track == t)
			ok = true;
	if (ok)
		LoadData();
	else
		SetTrack(NULL);
}
