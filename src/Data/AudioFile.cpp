/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "AudioFile.h"
#include "../Action/AudioFile/ActionAudioAddTrack.h"
#include "../Action/AudioFile/ActionAudioDeleteTrack.h"
#include "../Action/AudioFile/ActionAudioAddLevel.h"
#include "../Action/AudioFile/ActionAudioDeleteSelection.h"
#include "../Action/SubTrack/ActionSubTrackInsertSelected.h"
#include "../Action/SubTrack/ActionSubTrackFromSelection.h"
#include "../Tsunami.h"
#include <assert.h>


int get_track_index(Track *t)
{
	if (t){
		AudioFile *a = t->root;
		if (a){
			foreachi(Track &tt, a->track, i)
				if (t == &tt)
					return i;
		}
	}
	return -1;
}

int get_sub_index(Track *s)
{
	if (s){
		Track *t = s->GetParent();
		if (t){
			foreachi(Track &ss, t->sub, i)
				if (s == &ss)
					return i;
		}
	}
	return -1;
}

void get_track_sub_index(Track *t, int &track_no, int &sub_no)
{
	sub_no = get_sub_index(t);
	if (sub_no >= 0)
		track_no = get_track_index(t->GetParent());
	else
		track_no = get_track_index(t);
}

AudioFile::AudioFile()
{
	used = false;
	volume = 1;
	selection.clear();
	cur_track = -1;
}



void AudioFile::AddTag(const string &key, const string &value)
{
	Tag t;
	t.key = key;
	t.value = value;
	tag.add(t);
}

void AudioFile::NewEmpty(int _sample_rate)
{
	msg_db_r("AudioFile.NewEmpty",1);

	Reset();
	action_manager->Enable(false);
	used = true;
	sample_rate = _sample_rate;

	// default tags
	AddTag("title", "new audio file");//_("neue Audiodatei"));
	AddTag("album", "tsunami");//AppTitle + " " + AppVersion);
	AddTag("artist", "tsunami");//AppTitle);

	action_manager->Enable(true);
	Notify("Change");
	msg_db_l(1);
}

void AudioFile::NewWithOneTrack(int _sample_rate)
{
	msg_db_r("AudioFile.NewWithOneTrack",1);

	NotifyBegin();
	NewEmpty(_sample_rate);
	action_manager->Enable(false);
	AddEmptyTrack();
	action_manager->Enable(true);
	NotifyEnd();

	msg_db_l(1);
}

// delete all data
void AudioFile::Reset()
{
	msg_db_r("AudioFile.Reset",1);
	used = false;
	filename = "";
	tag.clear();
	width = -1;
	selection.clear();
	view_pos = 0;
	view_zoom = 1;
	volume = 1;
	sample_rate = DEFAULT_SAMPLE_RATE;
	fx.clear();
	track.clear();
	cur_track = -1;

	level_name.clear();
	level_name.add("level 1");
	cur_level = 0;

	action_manager->Reset();

	Notify("Change");
	Notify("New");

	msg_db_l(1);
}

AudioFile::~AudioFile()
{
	Reset();
}


void AudioFile::UpdateSelection()
{
	msg_db_r("UpdateSelection", 1);
	selection = sel_raw;
	if (selection.num < 0)
		selection.invert();

	// subs
	foreach(Track &t, track)
		foreach(Track &s, t.sub)
			s.is_selected = (t.is_selected) && selection.overlaps(s.GetRange());
	Notify("SelectionChange");
	msg_db_l(1);
}


void AudioFile::UnselectAllSubs()
{
	foreach(Track &t, track){
		foreach(Track &s, t.sub){
			s.is_selected = false;
		}
		t.cur_sub = -1;
	}
	Notify("SelectionChange");
}


void AudioFile::SetCurSub(Track *s)
{
	msg_db_r("SetCurSub", 2);
	// unset
	foreach(Track &t, track)
		t.cur_sub = -1;

	if (s){
		// set
		Track *t = s->GetParent();
		if (t)
			t->cur_sub = get_sub_index(s);
	}
	msg_db_l(2);
}


bool AudioFile::Load(const string & filename, bool deep)
{
	return tsunami->storage->Load(this, filename);
}

bool AudioFile::Save(const string & filename)
{
	return tsunami->storage->Save(this, filename);
}

void AudioFile::SetCurTrack(Track *t)
{
	cur_track = get_track_index(t);
}


Track *AudioFile::GetCurTrack()
{
	if ((cur_track >= 0) && (cur_track < track.num))
		return &track[cur_track];
	return NULL;
}

Track *AudioFile::GetCurSub()
{
	Track *t = GetCurTrack();
	if (t)
		return t->GetCurSub();
	return NULL;
}

Range AudioFile::GetRange()
{
	int min = 2147483640;
	int max = -2147483640;
	foreach(Track &t, track){
		Range r = t.GetRangeUnsafe();
		if (r.start() < min)
			min = r.start();
		if (r.end() > max)
			max = r.end();
	}
	if (min > max)
		return Range(0, 0);
	return Range(min, max - min);
}


int AudioFile::screen2sample(int _x)
{
	return (int)( (_x - x) / view_zoom + view_pos );
}

int AudioFile::sample2screen(int s)
{
	return (int)( x + (s - view_pos) * view_zoom );
}


string AudioFile::get_time_str(int t)
{
	int _sample_rate = used ? sample_rate : DEFAULT_SAMPLE_RATE;
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _min=(t/60/_sample_rate);
	int _sec=((t/_sample_rate) %60);
	int _usec=(( (t-_sample_rate*(t/_sample_rate))*1000/_sample_rate) %1000);
	if (_min > 0)
		return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_usec);
	else
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_usec);
}

string AudioFile::get_time_str_fuzzy(int t, float dt)
{
	int _sample_rate = used ? sample_rate : DEFAULT_SAMPLE_RATE;
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _min=(t/60/_sample_rate);
	int _sec=((t/_sample_rate) %60);
	int _usec=(( (t-_sample_rate*(t/_sample_rate))*1000/_sample_rate) %1000);
	if (dt < 1.0){
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_usec);
		else
			return format("%s%.2d,%.3d",sign?"-":"",_sec,_usec);
	}else{
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		else
			return format("%s%.2d",sign?"-":"",_sec);
	}
}



Track *AudioFile::AddEmptyTrack(int index)
{
	if (index < 0)
		index = cur_track + 1;
	return (Track*)Execute(new ActionAudioAddTrack(index, Track::TYPE_AUDIO));
}



Track *AudioFile::AddTimeTrack(int index)
{
	// force single time track
	foreach(Track &tt, track)
		if (tt.type == Track::TYPE_TIME){
			tsunami->log->Error(_("Es existiert schon eine Rhythmus-Spur."));
			return NULL;
		}

	if (index < 0)
		index = cur_track + 1;
	return (Track*)Execute(new ActionAudioAddTrack(index, Track::TYPE_TIME));
}

extern int debug_timer;

void AudioFile::UpdatePeaks()
{
	msg_db_r("Audio.UpdatePeaks", 2);
	HuiGetTime(debug_timer);
	foreach(Track &t, track)
		t.UpdatePeaks();
	msg_write(format("up %f", HuiGetTime(debug_timer)));
	msg_db_l(2);
}


void AudioFile::PostActionUpdate()
{
	UpdatePeaks();
}

int AudioFile::GetNumSelectedSubs()
{
	int n = 0;
	foreach(Track &t, track)
		foreach(Track &s, t.sub)
			if (s.is_selected)
				n ++;
	return n;
}

void AudioFile::InsertSelectedSubs()
{
	if (GetNumSelectedSubs() > 0)
		Execute(new ActionSubTrackInsertSelected(this));
}

void AudioFile::AddLevel()
{
	Execute(new ActionAudioAddLevel());
}

void AudioFile::DeleteCurrentTrack()
{
	if (cur_track >= 0)
		Execute(new ActionAudioDeleteTrack(this, cur_track));
}

void AudioFile::DeleteSelection(bool all_levels)
{
	if (!selection.empty())
		Execute(new ActionAudioDeleteSelection(this, all_levels));
}

void AudioFile::CreateSubsFromSelection()
{
	if (!selection.empty())
		Execute(new ActionSubTrackFromSelection(this));
}

void AudioFile::InvalidateAllPeaks()
{
	foreach(Track &t, track)
		t.InvalidateAllPeaks();
}

Track *AudioFile::get_track(int track_no, int sub_no)
{
	assert((track_no >= 0) && (track_no < track.num) && "AudioFile.get_track");
	Track *t = &track[track_no];
	if (sub_no < 0)
		return t;

	assert((sub_no < t->sub.num) && "AudioFile.get_track");
	return &t->sub[sub_no];
}



