/*
 * SampleRefConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../AudioView.h"
#include "../../Data/Track.h"
#include "../Helper/Slider.h"
#include "SampleRefConsole.h"

SampleRefConsole::SampleRefConsole(AudioView *v, Song *s):
	SideBarConsole("Sample-Eigenschaften"),
	Observer("SampleRefConsole")
{
	fromResource("sample_ref_dialog");
	view = v;
	song = s;
	track = NULL;
	sample = NULL;

	event("volume", this, &SampleRefConsole::onVolume);
	event("mute", this, &SampleRefConsole::onMute);
	event("level_track", this, &SampleRefConsole::onLevelTrack);
	event("repnum", this, &SampleRefConsole::onRepNum);
	event("repdelay", this, &SampleRefConsole::onRepDelay);

	event("edit_song", this, &SampleRefConsole::onEditSong);
	event("edit_track", this, &SampleRefConsole::onEditTrack);

	subscribe(view, view->MESSAGE_CUR_SAMPLE_CHANGE);
}

SampleRefConsole::~SampleRefConsole()
{
	if (sample)
		unsubscribe(sample);
	unsubscribe(view);
}


void SampleRefConsole::onName()
{
	//sample->origin->name = GetString("");
}

void SampleRefConsole::onMute()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSampleRef(index, sample->volume, isChecked(""), sample->rep_num, sample->rep_delay);

	enable("volume", !sample->muted);
	subscribe(sample);
}

void SampleRefConsole::onLevelTrack()
{
	int n = getInt("");
}

void SampleRefConsole::onVolume()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSampleRef(index, db2amplitude(getFloat("")), sample->muted, sample->rep_num, sample->rep_delay);
	subscribe(sample);
}

void SampleRefConsole::onRepNum()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSampleRef(index, sample->volume, sample->muted, getInt("repnum") - 1, sample->rep_delay);
	enable("repdelay", sample->rep_num > 0);
	subscribe(sample);
}

void SampleRefConsole::onRepDelay()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSampleRef(index, sample->volume, sample->muted, sample->rep_num, (int)(getFloat("repdelay") * (float)sample->owner->sample_rate / 1000.0f));
	subscribe(sample);
}

void SampleRefConsole::onEditSong()
{
	((SideBar*)parent)->open(SideBar::SONG_CONSOLE);
}

void SampleRefConsole::onEditTrack()
{
	((SideBar*)parent)->open(SideBar::TRACK_CONSOLE);
}

void SampleRefConsole::loadData()
{
	enable("name", false);
	enable("mute", sample);
	enable("volume", sample);
	enable("repnum", sample);
	enable("repdelay", sample);

	setString("name", _("no sample selected"));

	if (!sample)
		return;
	setString("name", sample->origin->name);
	setDecimals(1);
	check("mute", sample->muted);
	setFloat("volume", amplitude2db(sample->volume));
	enable("volume", !sample->muted);
	reset("level_track");
	for (Track *t : song->tracks)
		addString("level_track", t->getNiceName());
	setInt("level_track", sample->track_no);
	setInt("repnum", sample->rep_num + 1);
	setFloat("repdelay", (float)sample->rep_delay / (float)sample->owner->sample_rate * 1000.0f);
	enable("repdelay", sample->rep_num > 0);
}

void SampleRefConsole::onUpdate(Observable *o, const string &message)
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
