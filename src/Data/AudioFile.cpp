/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "AudioFile.h"
#include "../Plugins/Effect.h"
#include "../Action/AudioFile/ActionAudioDeleteSelection.h"
#include "../Action/AudioFile/Level/ActionAudioAddLevel.h"
#include "../Action/AudioFile/Tag/ActionAudioAddTag.h"
#include "../Action/AudioFile/Tag/ActionAudioEditTag.h"
#include "../Action/AudioFile/Tag/ActionAudioDeleteTag.h"
#include "../Action/AudioFile/Sample/ActionAudioAddSample.h"
#include "../Action/AudioFile/Sample/ActionAudioDeleteSample.h"
#include "../Action/Track/ActionTrackAdd.h"
#include "../Action/Track/ActionTrackDelete.h"
#include "../Action/Track/Sample/ActionTrackInsertSelectedSamples.h"
#include "../Action/Track/Sample/ActionTrackSampleFromSelection.h"
#include "../Tsunami.h"
#include "../Storage/Storage.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"
#include <assert.h>


int get_track_index(Track *t)
{
	if (t){
		AudioFile *a = t->root;
		if (a){
			foreachi(Track *tt, a->track, i)
				if (t == tt)
					return i;
		}
	}
	return -1;
}

int get_sample_ref_index(SampleRef *s)
{
	if (s){
		Track *t = s->GetParent();
		if (t){
			foreachi(SampleRef *ss, t->sample, i)
				if (s == ss)
					return i;
		}
	}
	return -1;
}

int get_sample_index(Sample *s)
{
	foreachi(Sample *ss, s->owner->sample, i)
		if (s == ss)
			return i;
	return -1;
}

AudioFile::AudioFile() :
	Data("AudioFile")
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	used = false;
	volume = 1;
	selection.clear();
}



void AudioFile::AddTag(const string &key, const string &value)
{
	Execute(new ActionAudioAddTag(Tag(key, value)));
}

void AudioFile::EditTag(int index, const string &key, const string &value)
{
	Execute(new ActionAudioEditTag(index, Tag(key, value)));
}

void AudioFile::DeleteTag(int index)
{
	Execute(new ActionAudioDeleteTag(index));
}

void AudioFile::NewEmpty(int _sample_rate)
{
	msg_db_f("AudioFile.NewEmpty",1);

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
}

void AudioFile::NewWithOneTrack(int track_type, int _sample_rate)
{
	msg_db_f("AudioFile.NewWithOneTrack",1);

	NotifyBegin();
	NewEmpty(_sample_rate);
	action_manager->Enable(false);
	AddTrack(track_type);
	action_manager->Enable(true);
	NotifyEnd();
}

// delete all data
void AudioFile::Reset()
{
	msg_db_f("AudioFile.Reset",1);
	action_manager->Reset();

	used = false;
	filename = "";
	tag.clear();
	area = rect(0, 0, 0, 0);
	sel_raw.clear();
	selection.clear();
	volume = 1;
	sample_rate = DEFAULT_SAMPLE_RATE;
	fx.clear();
	foreach(Track *t, track)
		delete(t);
	track.clear();

	foreach(Sample *s, sample)
		delete(s);
	sample.clear();

	level_name.clear();
	level_name.add("level 1");

	action_manager->Reset();

	Notify("Change");
	Notify("New");
}

AudioFile::~AudioFile()
{
	Reset();
}


void AudioFile::UpdateSelection()
{
	msg_db_f("UpdateSelection", 1);
	selection = sel_raw;
	if (selection.num < 0)
		selection.invert();

	// subs
	foreach(Track *t, track)
		foreach(SampleRef *s, t->sample)
			s->is_selected = (t->is_selected) && selection.overlaps(s->GetRange());
	Notify("SelectionChange");
}

Range AudioFile::GetPlaybackSelection()
{
	if (selection.empty()){
		int num = GetRange().end() - selection.start();
		if (num <= 0)
			num = sample_rate; // 1 second
		return Range(selection.start(), num);
	}
	return selection;
}


void AudioFile::UnselectAllSubs()
{
	foreach(Track *t, track)
		foreach(SampleRef *s, t->sample)
			s->is_selected = false;
	Notify("SelectionChange");
}


bool AudioFile::Load(const string & filename, bool deep)
{
	return tsunami->storage->Load(this, filename);
}

bool AudioFile::Save(const string & filename)
{
	return tsunami->storage->Save(this, filename);
}

Range AudioFile::GetRange()
{
	int min =  1073741824;
	int max = -1073741824;
	Range r = Range(min, max - min);
	foreach(Track *t, track)
		r = r || t->GetRangeUnsafe();

	if (r.length() < 0)
		return Range(0, 0);
	return r;
}


string AudioFile::get_time_str(int t)
{
	int _sample_rate = used ? sample_rate : DEFAULT_SAMPLE_RATE;
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _min=(t/60/_sample_rate);
	int _sec=((t/_sample_rate) %60);
	int _msec=(( (t-_sample_rate*(t/_sample_rate))*1000/_sample_rate) %1000);
	if (_min > 0)
		return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
	else
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
}

string AudioFile::get_time_str_fuzzy(int t, float dt)
{
	int _sample_rate = used ? sample_rate : DEFAULT_SAMPLE_RATE;
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _min=(t/60/_sample_rate);
	int _sec=((t/_sample_rate) %60);
	int _msec=(( (t-_sample_rate*(t/_sample_rate))*1000/_sample_rate) %1000);
	if (dt < 1.0){
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
		else
			return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
	}else{
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		else
			return format("%s%.2d",sign?"-":"",_sec);
	}
}



Track *AudioFile::AddTrack(int type, int index)
{
	if (type == Track::TYPE_TIME){
		// force single time track
		if (GetTimeTrack()){
			tsunami->log->Error(_("Es existiert schon eine Rhythmus-Spur."));
			return NULL;
		}
	}
	if (index < 0)
		index = track.num;
	return (Track*)Execute(new ActionTrackAdd(index, type));
}

extern HuiTimer debug_timer;

void AudioFile::UpdatePeaks(int mode)
{
	msg_db_f("Audio.UpdatePeaks", 2);
	debug_timer.reset();
	foreach(Track *t, track)
		t->UpdatePeaks(mode);
	foreach(Sample *s, sample)
		s->buf.update_peaks(mode);
	msg_write(format("up %f", debug_timer.get()));
}


void AudioFile::PostActionUpdate()
{
	UpdatePeaks(tsunami->view->PeakMode);
}

int AudioFile::GetNumSelectedSubs()
{
	int n = 0;
	foreach(Track *t, track)
		foreach(SampleRef *s, t->sample)
			if (s->is_selected)
				n ++;
	return n;
}

void AudioFile::InsertSelectedSubs(int level_no)
{
	if (GetNumSelectedSubs() > 0)
		Execute(new ActionTrackInsertSelectedSamples(this, level_no));
}

void AudioFile::AddLevel()
{
	Execute(new ActionAudioAddLevel());
}

void AudioFile::DeleteTrack(int index)
{
	Execute(new ActionTrackDelete(this, index));
}

Sample *AudioFile::AddSample(const string &name, BufferBox &buf)
{
	return (Sample*)Execute(new ActionAudioAddSample(name, buf));
}

void AudioFile::DeleteSample(int index)
{
	if (sample[index]->ref_count == 0)
		Execute(new ActionAudioDeleteSample(index));
	else
		tsunami->log->Error(_("Kann nur Samples l&oschen, die nicht benutzt werden!"));
}

void AudioFile::DeleteSelection(int level_no, bool all_levels)
{
	if (!selection.empty())
		Execute(new ActionAudioDeleteSelection(this, level_no, all_levels));
}

void AudioFile::CreateSubsFromSelection(int level_no)
{
	if (!selection.empty())
		Execute(new ActionTrackSampleFromSelection(this, level_no));
}

void AudioFile::InvalidateAllPeaks()
{
	foreach(Track *t, track)
		t->InvalidateAllPeaks();
	foreach(Sample *s, sample)
		s->buf.invalidate_peaks(s->buf.range());
}

Track *AudioFile::get_track(int track_no)
{
	assert((track_no >= 0) && (track_no < track.num) && "AudioFile.get_track");
	return track[track_no];
}

SampleRef *AudioFile::get_sub(int track_no, int sub_no)
{
	assert((track_no >= 0) && (track_no < track.num) && "AudioFile.get_sub");
	Track *t = track[track_no];

	assert((sub_no >= 0) && "AudioFile.get_sub");
	assert((sub_no < t->sample.num) && "AudioFile.get_sub");
	return t->sample[sub_no];
}

Track *AudioFile::GetTimeTrack()
{
	foreach(Track *t, track)
		if (t->type == t->TYPE_TIME)
			return t;
	return NULL;
}

int AudioFile::GetNextBeat(int pos)
{
	Track *t = GetTimeTrack();
	if (!t)
		return pos;
	return t->bar.GetNextBeat(pos);
}

