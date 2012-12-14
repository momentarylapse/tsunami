/*
 * TrackDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "TrackDialog.h"
#include "../../Data/Track.h"
#include "Slider.h"
#include "FxList.h"
#include "BarList.h"
#include "../../Tsunami.h"

TrackDialog::TrackDialog(CHuiWindow *win):
	EmbeddedDialog(win)
{
	track = NULL;
	win->SetTarget("tool_table", 0);
	win->SetBorderWidth(8);
	win->EmbedDialog("track_dialog", 1, 0);
	win->SetDecimals(1);
	/*if (t->type == Track::TYPE_TIME)
		FromResource("track_time_dialog");
	else
		FromResource("track_dialog");*/
	volume_slider = new Slider(win, "volume_slider", "volume", 0, 2, 100, (void(HuiEventHandler::*)())&TrackDialog::OnVolume, 0);
	fx_list = new FxList(win, "fx_list", "add_effect", "configure_effect", "delete_effect", NULL);
	bar_list = NULL;
	/*if (t->type == Track::TYPE_TIME)
		bar_list = new BarList(this, "bar_list", "add_bar", "add_bar_pause", "delete_bar", t->bar, t->root->sample_rate);*/

	LoadData();
	Subscribe(tsunami->audio);

	win->EventM("name", this, (void(HuiEventHandler::*)())&TrackDialog::OnName);
	win->EventM("mute", this, (void(HuiEventHandler::*)())&TrackDialog::OnMute);
	win->EventM("close", this, (void(HuiEventHandler::*)())&TrackDialog::OnClose);
	win->EventM("hui:close", this, (void(HuiEventHandler::*)())&TrackDialog::OnClose);
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
	if (track){
		SetString("name", track->name);
		Check("mute", track->muted);
		volume_slider->Set(track->volume);
		volume_slider->Enable(!track->muted);
		fx_list->SetFxList(&track->fx);
	}else{
		volume_slider->Enable(false);
		fx_list->SetFxList(NULL);
	}
}

void TrackDialog::SetTrack(Track *t)
{
	track = t;
	LoadData();
}

void TrackDialog::OnName()
{
	track->name = GetString("");
}

void TrackDialog::OnVolume()
{
	track->volume = volume_slider->Get();
}

void TrackDialog::OnMute()
{
	track->muted = IsChecked("");
	volume_slider->Enable(!track->muted);
}

void TrackDialog::OnClose()
{
	//delete(this);
	win->HideControl("tool_table", true);
}

void TrackDialog::OnUpdate(Observable *o)
{
	if (!track)
		return;
	bool ok = false;
	foreach(Track *t, tsunami->audio->track)
		if (track == t)
			ok = true;
	if (!ok)
		SetTrack(NULL);
}
