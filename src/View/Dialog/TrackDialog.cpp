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
#include "SynthesizerDialog.h"
#include "../../Tsunami.h"
#include "../../Plugins/PluginManager.h"
#include "../AudioView.h"
#include "../../Action/Track/Synthesizer/ActionTrackEditSynthesizer.h"

TrackDialog::TrackDialog(HuiWindow *win):
	EmbeddedDialog(win)
{
	track = NULL;
	win->SetTarget("track_dialog_table", 0);
	win->SetBorderWidth(5);
	win->EmbedDialog("track_time_dialog", 0, 0);
	win->SetDecimals(1);
	volume_slider = new Slider(win, "volume_slider", "volume", 0, 2, 100, (void(HuiEventHandler::*)())&TrackDialog::OnVolume, 0, this);
	panning_slider = new Slider(win, "panning_slider", "panning", -1, 1, 100, (void(HuiEventHandler::*)())&TrackDialog::OnPanning, 0, this);
	fx_list = new FxList(win, "fx_list", "add_effect", "configure_effect", "delete_effect");
	bar_list = new BarList(win, "bar_list", "add_bar", "add_bar_pause", "delete_bar");

	LoadData();
	Subscribe(tsunami->audio);

	win->EventM("name", this, &TrackDialog::OnName);
	win->EventM("mute", this, &TrackDialog::OnMute);
	win->EventM("synthesizer", this, &TrackDialog::OnSynthesizer);
	win->EventM("config_synth", this, &TrackDialog::OnConfigSynthesizer);
	win->EventM("edit_midi_track", this, &TrackDialog::OnEditMidiTrack);
	win->EventM("close", this, &TrackDialog::OnClose);
}

TrackDialog::~TrackDialog()
{
	Unsubscribe(tsunami->audio);
	delete(volume_slider);
	delete(fx_list);
	if (bar_list)
		delete(bar_list);
}

void TrackDialog::LoadData()
{
	Enable("name", track);
	Enable("mute", track);
	panning_slider->Enable(track);
	fx_list->SetTrack(track);
	bar_list->SetTrack(track);
	if (track){
		SetString("name", track->name);
		Check("mute", track->muted);
		volume_slider->Set(track->volume);
		volume_slider->Enable(!track->muted);
		panning_slider->Set(track->panning);
		SetString("synthesizer", track->synth->name);
		Enable("synthesizer", track->type != track->TYPE_AUDIO);
		Enable("config_synth", track->type != track->TYPE_AUDIO);
		Enable("edit_midi_track", track->type == Track::TYPE_MIDI);
	}else{
		volume_slider->Enable(false);
		Enable("synthesizer", track);
		Enable("config_synth", track);
		Enable("edit_midi_track", false);
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

void TrackDialog::OnVolume()
{
	track->SetVolume(volume_slider->Get());
}

void TrackDialog::OnMute()
{
	track->SetMuted(IsChecked(""));
}

void TrackDialog::OnPanning()
{
	track->SetPanning(panning_slider->Get());
}

void TrackDialog::OnSynthesizer()
{
	Synthesizer *s = ChooseSynthesizer(tsunami, track->synth->name);
	if (s)
		track->SetSynthesizer(s);
}

void TrackDialog::OnConfigSynthesizer()
{
	string params_old = track->synth->options_to_string();
	if (tsunami->plugin_manager->ConfigureSynthesizer(track->synth))
		track->root->Execute(new ActionTrackEditSynthesizer(track, params_old));
}

void TrackDialog::OnEditMidiTrack()
{
	tsunami->view->SetEditModeMidi(track);
}

void TrackDialog::OnClose()
{
	win->HideControl("track_dialog_table", true);
}

void TrackDialog::OnUpdate(Observable *o)
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
