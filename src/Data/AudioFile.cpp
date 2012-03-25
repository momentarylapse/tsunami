/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "AudioFile.h"
#include "../Action/ActionAudioAddTrack.h"
#include "../Tsunami.h"
#include <assert.h>


int get_track_index(Track *t)
{
	if (t){
		AudioFile *a = t->root;
		if (a){
			foreachi(a->track, tt, i)
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
			foreachi(t->sub, ss, i)
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
	//HistoryStructReset("AudioFile", this);

	used = false;
	volume = 1;
	selection = false;
	cur_track = -1;

	/*history = new History();
	history->AddData("AudioFile", this);
	history->Enable(false);
	history->OnChange(&HistoryOnChange);
	history->OnApply(&HistoryOnApply);*/
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
	used = true;
	sample_rate = _sample_rate;

	// default tags
	AddTag("title", "new audio file");//_("neue Audiodatei"));
	AddTag("album", "tsunami");//AppTitle + " " + AppVersion);
	AddTag("artist", "tsunami");//AppTitle);

	//a->history->Reset(false);
	//force_redraw = true;
	Notify("Change");
	msg_db_l(1);
}

void AudioFile::NewWithOneTrack(int _sample_rate)
{
	msg_db_r("AudioFile.NewWithOneTrack",1);

	NotifyBegin();
	NewEmpty(_sample_rate);
	AddEmptyTrack(-1);
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
	selection = false;
	view_pos = 0;
	view_zoom = 1;
	volume = 1;
	sample_rate = DEFAULT_SAMPLE_RATE;
	fx.clear();
	track.clear();
	cur_track = -1;

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
	if (!selection){
		sel_start_raw = GetMin();
		sel_end_raw = GetMax();
	}
	if (sel_start_raw > sel_end_raw){
		selection_start = sel_end_raw;
		selection_end = sel_start_raw;
	}else{
		selection_start = sel_start_raw;
		selection_end = sel_end_raw;
	}
	selection_length = selection_end - selection_start;

	// subs
	foreach(track, t){
		foreach(t.sub, s){
			if (selection){
				s.is_selected = (t.is_selected) && ((selection_start <= s.pos + s.length) && (selection_end >= s.pos));
			}else{
				s.is_selected = false;
			}
		}
	}
	Notify("SelectionChange");
	msg_db_l(1);
}


void AudioFile::UnselectAllSubs()
{
	foreach(track, t){
		foreach(t.sub, s){
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
	foreach(track, t)
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

int AudioFile::GetMin()
{
	int min = 2147483640;
	foreach(track, t)
		if (t.GetMinUnsafe() < min)
			min = t.GetMinUnsafe();
	if (min == 2147483640)
		return 0;
	return min;
}

int AudioFile::GetMax()
{
	int max = -2147483640;
	foreach(track, t)
		if (t.GetMaxUnsafe() > max)
			max = t.GetMaxUnsafe();
	if (max == -2147483640)
		return 0;
	return max;
}

int AudioFile::GetLength()
{
	return max(GetMax() - GetMin(), 0);
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
	return (Track*)Execute(new ActionAudioAddTrack(index));
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



