/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "AudioFile.h"
#include "Curve.h"
#include "../Plugins/Effect.h"
#include "../Action/AudioFile/ActionAudioDeleteSelection.h"
#include "../Action/AudioFile/Level/ActionAudioAddLevel.h"
#include "../Action/AudioFile/Level/ActionAudioRenameLevel.h"
#include "../Action/AudioFile/Data/ActionAudioSetVolume.h"
#include "../Action/AudioFile/Tag/ActionAudioAddTag.h"
#include "../Action/AudioFile/Tag/ActionAudioEditTag.h"
#include "../Action/AudioFile/Tag/ActionAudioDeleteTag.h"
#include "../Action/AudioFile/Sample/ActionAudioAddSample.h"
#include "../Action/AudioFile/Sample/ActionAudioDeleteSample.h"
#include "../Action/AudioFile/Sample/ActionAudioSampleEditName.h"
#include "../Action/Track/ActionTrackAdd.h"
#include "../Action/Track/ActionTrackDelete.h"
#include "../Action/Track/Sample/ActionTrackInsertSelectedSamples.h"
#include "../Action/Track/Sample/ActionTrackSampleFromSelection.h"
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackDeleteEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Effect/ActionTrackToggleEffectEnabled.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Storage/Storage.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"
#include <assert.h>
#include <math.h>

float amplitude2db(float amp)
{
	return log10(amp) * 20.0f;
}

float db2amplitude(float db)
{
	return pow10(db * 0.05);
}


int get_track_index(Track *t)
{
	if (t)
		return t->get_index();
	return -1;
}

int get_sample_ref_index(SampleRef *s)
{
	if (s)
		return s->get_index();
	return -1;
}

AudioFile::AudioFile() :
	Data("AudioFile")
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	used = false;
	volume = 1;
}


const string AudioFile::MESSAGE_NEW = "New";
const string AudioFile::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string AudioFile::MESSAGE_ADD_TRACK = "AddTrack";
const string AudioFile::MESSAGE_DELETE_TRACK = "DeleteTrack";
const string AudioFile::MESSAGE_ADD_EFFECT = "AddEffect";
const string AudioFile::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string AudioFile::MESSAGE_ADD_CURVE = "AddCurve";
const string AudioFile::MESSAGE_DELETE_CURVE = "DeleteCurve";


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

void AudioFile::AddEffect(Effect *effect)
{
	Execute(new ActionTrackAddEffect(NULL, effect));
}

// execute after editing...
void AudioFile::EditEffect(int index, const string &param_old)
{
	Execute(new ActionTrackEditEffect(NULL, index, param_old, fx[index]));
}

void AudioFile::EnableEffect(int index, bool enabled)
{
	if (fx[index]->enabled != enabled)
		Execute(new ActionTrackToggleEffectEnabled(NULL, index));
}

void AudioFile::DeleteEffect(int index)
{
	Execute(new ActionTrackDeleteEffect(NULL, index));
}

void AudioFile::SetVolume(float volume)
{
	Execute(new ActionAudioSetVolume(this, volume));
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
	Notify();
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
	volume = 1;
	sample_rate = DEFAULT_SAMPLE_RATE;
	foreach(Effect *f, fx)
		delete(f);
	fx.clear();
	foreach(Track *t, track)
		delete(t);
	track.clear();

	foreach(Sample *s, sample)
		delete(s);
	sample.clear();

	foreach(Curve *c, curve)
		delete(c);
	curve.clear();

	level_name.clear();
	level_name.add("");

	action_manager->Reset();

	Notify();
	Notify(MESSAGE_NEW);
}

AudioFile::~AudioFile()
{
	Reset();
}


void AudioFile::UpdateSelection(const Range &range)
{
	msg_db_f("UpdateSelection", 1);

	// subs
	foreach(Track *t, track)
		foreach(SampleRef *s, t->sample)
			s->is_selected = (t->is_selected) && range.overlaps(s->GetRange());
	Notify(MESSAGE_SELECTION_CHANGE);
}


void AudioFile::UnselectAllSamples()
{
	foreach(Track *t, track)
		foreach(SampleRef *s, t->sample)
			s->is_selected = false;
	Notify(MESSAGE_SELECTION_CHANGE);
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

void disect_time(int t, int sample_rate, bool &sign, int &min, int &sec, int &msec)
{
	sign = (t < 0);
	if (sign)
		t = -t;
	min=(t / 60 / sample_rate);
	sec=((t / sample_rate) % 60);
	msec=(((t - sample_rate * (t / sample_rate)) * 1000 / sample_rate) % 1000);
}

string AudioFile::get_time_str(int t)
{
	int _sample_rate = used ? sample_rate : DEFAULT_SAMPLE_RATE;
	bool sign;
	int _min, _sec, _msec;
	disect_time(t, _sample_rate, sign, _min, _sec, _msec);
	if (_min > 0)
		return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
	else
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
}

string AudioFile::get_time_str_fuzzy(int t, float dt)
{
	int _sample_rate = used ? sample_rate : DEFAULT_SAMPLE_RATE;
	bool sign;
	int _min, _sec, _msec;
	disect_time(t, _sample_rate, sign, _min, _sec, _msec);
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

string AudioFile::get_time_str_long(int t)
{
	bool sign;
	int _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _min, _sec, _msec);
	if (_min > 0)
		return format("%s%dm %.2ds %.3dms",sign?"-":"",_min,_sec,_msec);
	else
		return format("%s%ds %.3dms",sign?"-":"",_sec,_msec);
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
	//msg_write(format("up %f", debug_timer.get()));
}


void AudioFile::PostActionUpdate()
{
	UpdatePeaks(tsunami->win->view->peak_mode);
}

int AudioFile::GetNumSelectedSamples()
{
	int n = 0;
	foreach(Track *t, track)
		foreach(SampleRef *s, t->sample)
			if (s->is_selected)
				n ++;
	return n;
}

void AudioFile::InsertSelectedSamples(int level_no)
{
	if (GetNumSelectedSamples() > 0)
		Execute(new ActionTrackInsertSelectedSamples(this, level_no));
}

void AudioFile::DeleteSelectedSamples()
{
	action_manager->BeginActionGroup();
	foreachi(Track *t, track, i){
		for (int j=t->sample.num-1;j>=0;j--)
			if (t->sample[j]->is_selected)
				t->DeleteSample(j);
	}
	action_manager->EndActionGroup();
}

void AudioFile::AddLevel(const string &name)
{
	Execute(new ActionAudioAddLevel(name));
}

void AudioFile::DeleteLevel(int index, bool merge)
{
	tsunami->log->Error(_("Ebene l&oschen: noch nicht implementiert..."));
}

void AudioFile::RenameLevel(int index, const string &name)
{
	Execute(new ActionAudioRenameLevel(index, name));
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

void AudioFile::EditSampleName(int index, const string &name)
{
	Execute(new ActionAudioSampleEditName(this, index, name));
}

void AudioFile::DeleteSelection(int level_no, const Range &range, bool all_levels)
{
	if (!range.empty())
		Execute(new ActionAudioDeleteSelection(this, level_no, range, all_levels));
}

void AudioFile::CreateSamplesFromSelection(int level_no, const Range &range)
{
	if (!range.empty())
		Execute(new ActionTrackSampleFromSelection(this, range, level_no));
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

SampleRef *AudioFile::get_sample_ref(int track_no, int index)
{
	assert((track_no >= 0) && (track_no < track.num) && "AudioFile.get_sample");
	Track *t = track[track_no];

	assert((index >= 0) && "AudioFile.get_sample");
	assert((index < t->sample.num) && "AudioFile.get_sample");
	return t->sample[index];
}

int AudioFile::get_sample_by_uid(int uid)
{
	foreachi(Sample *s, sample, i)
		if (s->uid == uid)
			return i;
	return -1;
}

Effect *AudioFile::get_fx(int track_no, int index)
{
	assert(index >= 0);
	if (track_no >= 0){
		Track *t = get_track(track_no);
		assert(t);
		assert(index < t->fx.num);

		return t->fx[index];
	}else{
		assert(index < fx.num);

		return fx[index];
	}
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

string AudioFile::GetNiceLevelName(int index)
{
	if (level_name[index].num > 0)
		return level_name[index];
	return format(_("Ebene %d"), index + 1);
}

