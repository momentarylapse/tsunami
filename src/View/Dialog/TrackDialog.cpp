/*
 * TrackDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "TrackDialog.h"

TrackDialog::TrackDialog(CHuiWindow *_parent, bool _allow_parent, Track *t):
	CHuiWindow("dummy", -1, -1, 200, 300, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	track = t;
	FromResource("track_dialog");

	SetString("name", t->name);
	SetDecimals(1);
	Check("mute", t->muted);
	volume_slider = new Slider(this, "volume_slider", "volume", 0, 2, 100, (void(HuiEventHandler::*)())&TrackDialog::OnVolume, t->volume);
	volume_slider->Enable(!t->muted);
	fx_list = new FxList(this, "fx_list", "add_effect", "configure_effect", "delete_effect", t->fx);

	EventM("name", this, (void(HuiEventHandler::*)())&TrackDialog::OnName);
	EventM("mute", this, (void(HuiEventHandler::*)())&TrackDialog::OnMute);
	EventM("close", this, (void(HuiEventHandler::*)())&TrackDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&TrackDialog::OnClose);
}

TrackDialog::~TrackDialog()
{
	delete(volume_slider);
	delete(fx_list);
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
	delete(this);
}
