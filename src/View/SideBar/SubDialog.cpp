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
	fromResource("sample_ref_dialog");
	view = v;
	audio = a;
	track = NULL;
	sample = NULL;

	event("volume", this, &SubDialog::onVolume);
	event("mute", this, &SubDialog::onMute);
	event("level_track", this, &SubDialog::onLevelTrack);
	event("repnum", this, &SubDialog::onRepNum);
	event("repdelay", this, &SubDialog::onRepDelay);

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
	track->editSample(index, sample->volume, isChecked(""), sample->rep_num, sample->rep_delay);

	enable("volume", !sample->muted);
	subscribe(sample);
}

void SubDialog::onLevelTrack()
{
	int n = getInt("");
}

void SubDialog::onVolume()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, db2amplitude(getFloat("")), sample->muted, sample->rep_num, sample->rep_delay);
	subscribe(sample);
}

void SubDialog::onRepNum()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, sample->volume, sample->muted, getInt("repnum") - 1, sample->rep_delay);
	enable("repdelay", sample->rep_num > 0);
	subscribe(sample);
}

void SubDialog::onRepDelay()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, sample->volume, sample->muted, sample->rep_num, (int)(getFloat("repdelay") * (float)sample->owner->sample_rate / 1000.0f));
	subscribe(sample);
}

void SubDialog::loadData()
{
	enable("name", false);
	enable("mute", sample);
	enable("volume", sample);
	enable("repnum", sample);
	enable("repdelay", sample);

	setString("name", _("keine Sample gew&ahlt"));

	if (!sample)
		return;
	setString("name", sample->origin->name);
	setDecimals(1);
	check("mute", sample->muted);
	setFloat("volume", amplitude2db(sample->volume));
	enable("volume", !sample->muted);
	reset("level_track");
	foreach(Track *t, audio->tracks)
		addString("level_track", t->getNiceName());
	setInt("level_track", sample->track_no);
	setInt("repnum", sample->rep_num + 1);
	setFloat("repdelay", (float)sample->rep_delay / (float)sample->owner->sample_rate * 1000.0f);
	enable("repdelay", sample->rep_num > 0);
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
