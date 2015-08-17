/*
 * SampleRefDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "SampleRefDialog.h"

SampleRefDialog::SampleRefDialog(AudioView *v, Song *s):
	SideBarConsole("Sample-Eigenschaften"),
	Observer("SampleRefDialog")
{
	fromResource("sample_ref_dialog");
	view = v;
	song = s;
	track = NULL;
	sample = NULL;

	event("volume", this, &SampleRefDialog::onVolume);
	event("mute", this, &SampleRefDialog::onMute);
	event("level_track", this, &SampleRefDialog::onLevelTrack);
	event("repnum", this, &SampleRefDialog::onRepNum);
	event("repdelay", this, &SampleRefDialog::onRepDelay);

	event("edit_song", this, &SampleRefDialog::onEditSong);
	event("edit_track", this, &SampleRefDialog::onEditTrack);

	subscribe(view, view->MESSAGE_CUR_SAMPLE_CHANGE);
}

SampleRefDialog::~SampleRefDialog()
{
	if (sample)
		unsubscribe(sample);
	unsubscribe(view);
}


void SampleRefDialog::onName()
{
	//sample->origin->name = GetString("");
}

void SampleRefDialog::onMute()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, sample->volume, isChecked(""), sample->rep_num, sample->rep_delay);

	enable("volume", !sample->muted);
	subscribe(sample);
}

void SampleRefDialog::onLevelTrack()
{
	int n = getInt("");
}

void SampleRefDialog::onVolume()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, db2amplitude(getFloat("")), sample->muted, sample->rep_num, sample->rep_delay);
	subscribe(sample);
}

void SampleRefDialog::onRepNum()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, sample->volume, sample->muted, getInt("repnum") - 1, sample->rep_delay);
	enable("repdelay", sample->rep_num > 0);
	subscribe(sample);
}

void SampleRefDialog::onRepDelay()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSample(index, sample->volume, sample->muted, sample->rep_num, (int)(getFloat("repdelay") * (float)sample->owner->sample_rate / 1000.0f));
	subscribe(sample);
}

void SampleRefDialog::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void SampleRefDialog::onEditTrack()
{
	((SideBar*)parent)->open(SideBar::TRACK_CONSOLE);
}

void SampleRefDialog::loadData()
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
	foreach(Track *t, song->tracks)
		addString("level_track", t->getNiceName());
	setInt("level_track", sample->track_no);
	setInt("repnum", sample->rep_num + 1);
	setFloat("repdelay", (float)sample->rep_delay / (float)sample->owner->sample_rate * 1000.0f);
	enable("repdelay", sample->rep_num > 0);
}

void SampleRefDialog::onUpdate(Observable *o, const string &message)
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
