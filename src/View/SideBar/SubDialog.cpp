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

	EventM("volume", this, &SubDialog::onVolume);
	EventM("mute", this, &SubDialog::onMute);
	EventM("level_track", this, &SubDialog::onLevelTrack);
	EventM("repnum", this, &SubDialog::onRepNum);
	EventM("repdelay", this, &SubDialog::onRepDelay);

	subscribe(view, view->MESSAGE_CUR_SAMPLE_CHANGE);
}

SubDialog::~SubDialog()
{
	if (sample)
		unsubscribe(sample);
	unsubscribe(view);
}


void SubDialog::onName()
{
	//sample->origin->name = GetString("");
}

void SubDialog::onMute()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, sample->volume, IsChecked(""), sample->rep_num, sample->rep_delay);

	Enable("volume", !sample->muted);
	subscribe(sample);
}

void SubDialog::onLevelTrack()
{
	int n = GetInt("");
}

void SubDialog::onVolume()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, db2amplitude(GetFloat("")), sample->muted, sample->rep_num, sample->rep_delay);
	subscribe(sample);
}

void SubDialog::onRepNum()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, sample->volume, sample->muted, GetInt("repnum") - 1, sample->rep_delay);
	Enable("repdelay", sample->rep_num > 0);
	subscribe(sample);
}

void SubDialog::onRepDelay()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->EditSample(index, sample->volume, sample->muted, sample->rep_num, (int)(GetFloat("repdelay") * (float)sample->owner->sample_rate / 1000.0f));
	subscribe(sample);
}

void SubDialog::loadData()
{
	Enable("name", false);
	Enable("mute", sample);
	Enable("volume", sample);
	Enable("repnum", sample);
	Enable("repdelay", sample);

	SetString("name", _("keine Sample gew&ahlt"));

	if (!sample)
		return;
	SetString("name", sample->origin->name);
	SetDecimals(1);
	Check("mute", sample->muted);
	SetFloat("volume", amplitude2db(sample->volume));
	Enable("volume", !sample->muted);
	Reset("level_track");
	foreach(Track *t, audio->track)
		AddString("level_track", t->GetNiceName());
	SetInt("level_track", sample->track_no);
	SetInt("repnum", sample->rep_num + 1);
	SetFloat("repdelay", (float)sample->rep_delay / (float)sample->owner->sample_rate * 1000.0f);
	Enable("repdelay", sample->rep_num > 0);
}

void SubDialog::onUpdate(Observable *o, const string &message)
{
	if (o == view){
		if (sample)
			unsubscribe(sample);
		track = view->cur_track;
		sample = view->cur_sample;
		if (sample)
			subscribe(sample);
		loadData();
	}else if ((o == sample) && (message == o->MESSAGE_DELETE)){
		unsubscribe(sample);
		sample = NULL;
		loadData();
	}else{
		loadData();
	}
}
