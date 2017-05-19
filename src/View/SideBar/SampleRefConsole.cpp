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
	SideBarConsole("Sample-Eigenschaften"),
	Observer("SampleRefConsole")
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
	track->editSampleRef(index, sample->volume, isChecked(""));

	enable("volume", !sample->muted);
	subscribe(sample);
}

void SampleRefConsole::onTrack()
{
	//int n = getInt("");
}

void SampleRefConsole::onVolume()
{
	if (!sample)
		return;
	unsubscribe(sample);
	int index = sample->get_index();
	track->editSampleRef(index, db2amplitude(getFloat("")), sample->muted);
	subscribe(sample);
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
	}else if ((o == sample) and (message == o->MESSAGE_DELETE)){
		unsubscribe(sample);
		sample = NULL;
		loadData();
	}else{
		loadData();
	}
}
