/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "AudioFile.h"



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

//void NewEmptyAudioFile(AudioFile *a, int sample_rate)
void AudioFile::New()
{
	msg_db_r("AudioFile.New",1);
	Reset();
	//a->sample_rate = sample_rate;
	AddTag("title", "new audio file");//_("neue Audiodatei"));
	AddTag("album", "tsunami");//AppTitle + " " + AppVersion);
	AddTag("artist", "tsunami");//AppTitle);

	//a->history->Reset(false);
	//force_redraw = true;
	Notify("Change");
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
	fx.clear();

	/*for (int i=0;i<a->Track.num;i++)
		delete(a->Track[i]);*/
	track.clear();
	cur_track = -1;

	action_manager->Reset();

	Notify("Change");

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
