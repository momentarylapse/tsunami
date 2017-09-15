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
#include "SampleManagerConsole.h"

SampleRefConsole::SampleRefConsole(AudioView *v, Song *s):
	SideBarConsole("Sample-Eigenschaften")
{
	fromResource("sample_ref_dialog");
	view = v;
	song = s;
	track = NULL;
	sample = NULL;

	event("volume", std::bind(&SampleRefConsole::onVolume, this));
	event("mute", std::bind(&SampleRefConsole::onMute, this));
	event("track", std::bind(&SampleRefConsole::onTrack, this));

	event("edit_song", std::bind(&SampleRefConsole::onEditSong, this));
	event("edit_track", std::bind(&SampleRefConsole::onEditTrack, this));
	event("edit_sample", std::bind(&SampleRefConsole::onEditSample, this));

	view->subscribe_old2(this, SampleRefConsole, view->MESSAGE_CUR_SAMPLE_CHANGE);
}

SampleRefConsole::~SampleRefConsole()
{
	if (sample)
		sample->unsubscribe(this);
	view->unsubscribe(this);
}


void SampleRefConsole::onName()
{
	//sample->origin->name = GetString("");
}

void SampleRefConsole::onMute()
{
	if (!sample)
		return;
	sample->unsubscribe(this);
	track->editSampleRef(sample, sample->volume, isChecked(""));

	enable("volume", !sample->muted);
	sample->subscribe_old(this, SampleRefConsole);
}

void SampleRefConsole::onTrack()
{
	//int n = getInt("");
}

void SampleRefConsole::onVolume()
{
	if (!sample)
		return;
	sample->unsubscribe(this);
	track->editSampleRef(sample, db2amplitude(getFloat("")), sample->muted);
	sample->subscribe_old(this, SampleRefConsole);
}

void SampleRefConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void SampleRefConsole::onEditTrack()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void SampleRefConsole::onEditSample()
{
	bar()->sample_manager->setSelection(sample->origin);
	bar()->open(SideBar::SAMPLE_CONSOLE);
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
	reset("track");
	for (Track *t: song->tracks)
		addString("track", t->getNiceName());
	setInt("track", sample->track_no);
}

void SampleRefConsole::onUpdate(Observable *o)
{
	if (o == view){
		if (sample)
			sample->unsubscribe(this);
		track = view->cur_track;
		sample = view->cur_sample;
		if (sample)
			sample->subscribe_old(this, SampleRefConsole);
		loadData();
	}else if ((o == sample) and (o->cur_message() == o->MESSAGE_DELETE)){
		sample->unsubscribe(this);
		sample = NULL;
		loadData();
	}else{
		loadData();
	}
}
