/*
 * SubDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SubDialog.h"

SubDialog::SubDialog(CHuiWindow *_parent, bool _allow_parent, Track *s):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	sub = s;
	FromResource("level_dialog");

	SetString("name", s->name);
	SetDecimals(1);
	volume_slider = new Slider(this, "volume_slider", "volume", 0, 2, 100, (void(HuiEventHandler::*)())&SubDialog::OnVolume, s->volume);
	Check("mute", s->muted);
	Enable("volume", !s->muted);
	Enable("volume_slider", !s->muted);
	foreach(Track *t, s->root->track)
		AddString("level_track", t->GetNiceName());
	SetInt("level_track", s->parent);
//	AddEffectList(LevelDialog, "fx_list", s->fx);
	SetInt("repnum", s->rep_num + 1);
	SetFloat("repdelay", (float)s->rep_delay / (float)s->root->sample_rate * 1000.0f);
	Enable("repdelay", s->rep_num > 0);

	EventM("mute", this, (void(HuiEventHandler::*)())&SubDialog::OnMute);
	EventM("name", this, (void(HuiEventHandler::*)())&SubDialog::OnName);
	EventM("level_track", this, (void(HuiEventHandler::*)())&SubDialog::OnLevelTrack);
	EventM("repnum", this, (void(HuiEventHandler::*)())&SubDialog::OnRepNum);
	EventM("repdelay", this, (void(HuiEventHandler::*)())&SubDialog::OnRepDelay);
	EventM("close", this, (void(HuiEventHandler::*)())&SubDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&SubDialog::OnClose);
}

SubDialog::~SubDialog()
{
	delete(volume_slider);
}


void SubDialog::OnName()
{
	sub->name = GetString("");
}

void SubDialog::OnMute()
{
	sub->muted = IsChecked("");
	volume_slider->Enable(!sub->muted);
}

void SubDialog::OnLevelTrack()
{
	int n = GetInt("");
}

void SubDialog::OnVolume()
{
	sub->volume = volume_slider->Get();
}

void SubDialog::OnRepNum()
{
	sub->rep_num = GetInt("repnum") - 1;
	Enable("repdelay", sub->rep_num > 0);
}

void SubDialog::OnRepDelay()
{
	sub->rep_delay = (int)(GetFloat("repdelay") * (float)sub->root->sample_rate / 1000.0f);
}

void SubDialog::OnClose()
{
	delete(this);
}
