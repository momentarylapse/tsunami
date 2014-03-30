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
	SideBarConsole("Sample-Eigenschaften"),
	Observer("SubDialog")
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
}

SubDialog::~SubDialog()
{
	if (sample)
		Unsubscribe(sample);
	Unsubscribe(view);
	delete(volume_slider);
}


void SubDialog::OnName()
{
	//sample->origin->name = GetString("");
}

void SubDialog::OnMute()
{
	if (!sample)
		return;
	Unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, sample->volume, IsChecked(""), sample->rep_num, sample->rep_delay);

	volume_slider->Enable(!sample->muted);
	Subscribe(sample);
}

void SubDialog::OnLevelTrack()
{
	int n = GetInt("");
}

void SubDialog::OnVolume()
{
	if (!sample)
		return;
	Unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, volume_slider->Get(), sample->muted, sample->rep_num, sample->rep_delay);
	Subscribe(sample);
}

void SubDialog::OnRepNum()
{
	if (!sample)
		return;
	Unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, sample->volume, sample->muted, GetInt("repnum") - 1, sample->rep_delay);
	Enable("repdelay", sample->rep_num > 0);
	Subscribe(sample);
}

void SubDialog::OnRepDelay()
{
	if (!sample)
		return;
	Unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, sample->volume, sample->muted, sample->rep_num, (int)(GetFloat("repdelay") * (float)sample->owner->sample_rate / 1000.0f));
	Subscribe(sample);
}

void SubDialog::LoadData()
{
	Enable("name", false);
	Enable("mute", sample);
	volume_slider->Enable(sample);
	Enable("repnum", sample);
	Enable("repdelay", sample);

	if (!sample)
		return;
	SetString("name", sample->origin->name);
	SetDecimals(1);
	Check("mute", sample->muted);
	volume_slider->Set(sample->volume);
	volume_slider->Enable(!sample->muted);
	foreach(Track *t, audio->track)
		AddString("level_track", t->GetNiceName());
	SetInt("level_track", sample->track_no);
	SetInt("repnum", sample->rep_num + 1);
	SetFloat("repdelay", (float)sample->rep_delay / (float)sample->owner->sample_rate * 1000.0f);
	Enable("repdelay", sample->rep_num > 0);
}

void SubDialog::OnUpdate(Observable *o, const string &message)
{
	if (o == view){
		if (sample)
			Unsubscribe(sample);
		track = view->cur_track;
		sample = view->cur_sample;
		if (sample)
			Subscribe(sample);
		LoadData();
	}else{
		LoadData();
	}
}
