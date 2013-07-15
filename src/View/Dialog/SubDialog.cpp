/*
 * SubDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SubDialog.h"
#include "../../Data/Track.h"
#include "../Helper/Slider.h"

SubDialog::SubDialog(HuiWindow *_parent, bool _allow_parent, SampleRef *s):
	HuiWindow("level_dialog", _parent, _allow_parent)
{
	sample = s;

	SetString("name", s->origin->name);
	SetDecimals(1);
	volume_slider = new Slider(this, "volume_slider", "volume", 0, 2, 100, (void(HuiEventHandler::*)())&SubDialog::OnVolume, s->volume);
	Check("mute", s->muted);
	Enable("volume", !s->muted);
	Enable("volume_slider", !s->muted);
	foreach(Track *t, s->owner->track)
		AddString("level_track", t->GetNiceName());
	SetInt("level_track", s->track_no);
//	AddEffectList(LevelDialog, "fx_list", s->fx);
	SetInt("repnum", s->rep_num + 1);
	SetFloat("repdelay", (float)s->rep_delay / (float)s->owner->sample_rate * 1000.0f);
	Enable("repdelay", s->rep_num > 0);

	EventM("mute", this, &SubDialog::OnMute);
	EventM("level_track", this, &SubDialog::OnLevelTrack);
	EventM("repnum", this, &SubDialog::OnRepNum);
	EventM("repdelay", this, &SubDialog::OnRepDelay);
	EventM("close", this, &SubDialog::OnClose);
	EventM("hui:close", this, &SubDialog::OnClose);
}

SubDialog::~SubDialog()
{
	delete(volume_slider);
}


void SubDialog::OnName()
{
	sample->origin->name = GetString("");
}

void SubDialog::OnMute()
{
	sample->muted = IsChecked("");
	volume_slider->Enable(!sample->muted);
}

void SubDialog::OnLevelTrack()
{
	int n = GetInt("");
}

void SubDialog::OnVolume()
{
	sample->volume = volume_slider->Get();
}

void SubDialog::OnRepNum()
{
	sample->rep_num = GetInt("repnum") - 1;
	Enable("repdelay", sample->rep_num > 0);
}

void SubDialog::OnRepDelay()
{
	sample->rep_delay = (int)(GetFloat("repdelay") * (float)sample->owner->sample_rate / 1000.0f);
}

void SubDialog::OnClose()
{
	delete(this);
}
