/*
 * SubDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SubDialog.h"
#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../Helper/Slider.h"

SubDialog::SubDialog(AudioView *v, AudioFile *a):
	SideBarConsole("Sample-Eigenschaften")
{
	FromResource("sub_track_dialog");
	view = v;
	audio = a;
	track = NULL;
	sample = NULL;

	volume_slider = new Slider((HuiPanel*)this, "volume_slider", "volume", 0.0f, 2.0f, 100.0f, (void(HuiEventHandler::*)())&SubDialog::OnVolume, 1.0f);
	EventM("mute", this, &SubDialog::OnMute);
	EventM("level_track", this, &SubDialog::OnLevelTrack);
	EventM("repnum", this, &SubDialog::OnRepNum);
	EventM("repdelay", this, &SubDialog::OnRepDelay);

	Subscribe(view, "CurSampleChange");
	Subscribe(audio);
}

SubDialog::~SubDialog()
{
	Unsubscribe(audio);
	Unsubscribe(view);
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

void SubDialog::LoadData()
{
	Enable("name", sample);
	Enable("mute", sample);
	volume_slider->Enable(sample);
	Enable("repnum", sample);
	Enable("repdelay", sample);

	if (!sample)
		return;
	SetString("name", sample->origin->name);
	SetDecimals(1);
	Check("mute", sample->muted);
	volume_slider->Enable(!sample->muted);
	foreach(Track *t, audio->track)
		AddString("level_track", t->GetNiceName());
	SetInt("level_track", sample->track_no);
//	AddEffectList(LevelDialog, "fx_list", s->fx);
	SetInt("repnum", sample->rep_num + 1);
	SetFloat("repdelay", (float)sample->rep_delay / (float)sample->owner->sample_rate * 1000.0f);
	Enable("repdelay", sample->rep_num > 0);
}

void SubDialog::OnUpdate(Observable *o, const string &message)
{
	msg_write(o->GetName() + " - " + message);
	if (o == view){
		sample = view->cur_sample;
		LoadData();
	}
}
