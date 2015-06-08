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
#include "../Action/AudioFile/Data/ActionAudioChangeAllTrackVolumes.h"
#include "../Action/AudioFile/Data/ActionAudioSetDefaultFormat.h"
#include "../Action/AudioFile/Data/ActionAudioSetSampleRate.h"
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
	default_format = SAMPLE_FORMAT_16;
	compression = 0;
	volume = 1;
	level_names.add("");
}

void AudioFile::__init__()
{
	new(this) AudioFile;
}

void AudioFile::__delete__()
{
	reset();
}


const string AudioFile::MESSAGE_NEW = "New";
const string AudioFile::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string AudioFile::MESSAGE_ADD_TRACK = "AddTrack";
const string AudioFile::MESSAGE_DELETE_TRACK = "DeleteTrack";
const string AudioFile::MESSAGE_ADD_EFFECT = "AddEffect";
const string AudioFile::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string AudioFile::MESSAGE_ADD_CURVE = "AddCurve";
const string AudioFile::MESSAGE_DELETE_CURVE = "DeleteCurve";
const string AudioFile::MESSAGE_ADD_SAMPLE = "AddSample";
const string AudioFile::MESSAGE_DELETE_SAMPLE = "DeleteSample";
const string AudioFile::MESSAGE_ADD_LEVEL = "AddLevel";
const string AudioFile::MESSAGE_EDIT_LEVEL = "EditLevel";
const string AudioFile::MESSAGE_DELETE_LEVEL = "DeleteLevel";


void AudioFile::addTag(const string &key, const string &value)
{
	if ((key != "") and (value != ""))
		execute(new ActionAudioAddTag(Tag(key, value)));
}

void AudioFile::editTag(int index, const string &key, const string &value)
{
	execute(new ActionAudioEditTag(index, Tag(key, value)));
}

void AudioFile::deleteTag(int index)
{
	execute(new ActionAudioDeleteTag(index));
}

void AudioFile::addEffect(Effect *effect)
{
	execute(new ActionTrackAddEffect(NULL, effect));
}

// execute after editing...
void AudioFile::editEffect(int index, const string &param_old)
{
	execute(new ActionTrackEditEffect(NULL, index, param_old, fx[index]));
}

void AudioFile::enableEffect(int index, bool enabled)
{
	if (fx[index]->enabled != enabled)
		execute(new ActionTrackToggleEffectEnabled(NULL, index));
}

void AudioFile::deleteEffect(int index)
{
	execute(new ActionTrackDeleteEffect(NULL, index));
}

void AudioFile::setVolume(float volume)
{
	execute(new ActionAudioSetVolume(this, volume));
}

void AudioFile::changeAllTrackVolumes(Track *t, float volume)
{
	execute(new ActionAudioChangeAllTrackVolumes(this, t, volume));
}

void AudioFile::setSampleRate(int _sample_rate)
{
	if (_sample_rate > 0)
		execute(new ActionAudioSetSampleRate(this, _sample_rate));
}

void AudioFile::setDefaultFormat(SampleFormat _format)
{
	execute(new ActionAudioSetDefaultFormat(_format, compression));
}

void AudioFile::setCompression(int _compression)
{
	execute(new ActionAudioSetDefaultFormat(default_format, _compression));
}

void AudioFile::newEmpty(int _sample_rate)
{
	msg_db_f("AudioFile.NewEmpty",1);

	reset();
	action_manager->enable(false);
	sample_rate = _sample_rate;

	// default tags
	addTag("title", "New Audio File");//_("neue Audiodatei"));
	addTag("album", AppName);
	addTag("artist", HuiConfig.getStr("DefaultArtist", AppName));

	action_manager->enable(true);
	notify();
}

void AudioFile::newWithOneTrack(int track_type, int _sample_rate)
{
	msg_db_f("AudioFile.NewWithOneTrack",1);

	notifyBegin();
	newEmpty(_sample_rate);
	action_manager->enable(false);
	addTrack(track_type);
	action_manager->enable(true);
	notifyEnd();
}

// delete all data
void AudioFile::reset()
{
	msg_db_f("AudioFile.Reset",1);
	action_manager->reset();

	filename = "";
	tags.clear();
	volume = 1;
	default_format = SAMPLE_FORMAT_16;
	compression = 0;
	sample_rate = DEFAULT_SAMPLE_RATE;
	foreach(Effect *f, fx)
		delete(f);
	fx.clear();
	foreach(Track *t, tracks)
		delete(t);
	tracks.clear();

	foreach(Sample *s, samples)
		delete(s);
	samples.clear();

	foreach(Curve *c, curves)
		delete(c);
	curves.clear();

	level_names.clear();
	level_names.add("");

	action_manager->reset();

	notify();
	notify(MESSAGE_NEW);
}

AudioFile::~AudioFile()
{
	reset();
}


void AudioFile::updateSelection(const Range &range)
{
	msg_db_f("UpdateSelection", 1);

	// subs
	foreach(Track *t, tracks)
		foreach(SampleRef *s, t->samples)
			s->is_selected = (t->is_selected) and range.overlaps(s->getRange());
	notify(MESSAGE_SELECTION_CHANGE);
}


void AudioFile::unselectAllSamples()
{
	foreach(Track *t, tracks)
		foreach(SampleRef *s, t->samples)
			s->is_selected = false;
	notify(MESSAGE_SELECTION_CHANGE);
}


bool AudioFile::load(const string & filename, bool deep)
{
	return tsunami->storage->load(this, filename);
}

bool AudioFile::save(const string & filename)
{
	return tsunami->storage->save(this, filename);
}

Range AudioFile::getRange()
{
	int min =  1073741824;
	int max = -1073741824;
	Range r = Range(min, max - min);
	foreach(Track *t, tracks)
		r = r || t->getRangeUnsafe();

	if (r.length() < 0)
		return Range(0, 0);
	return r;
}

void disect_time(int t, int sample_rate, bool &sign, int &hours, int &min, int &sec, int &msec)
{
	sign = (t < 0);
	if (sign)
		t = -t;
	hours = (t / 3600 / sample_rate);
	min = ((t / 60 / sample_rate) % 60);
	sec = ((t / sample_rate) % 60);
	msec = (((t - sample_rate * (t / sample_rate)) * 1000 / sample_rate) % 1000);
}

string AudioFile::get_time_str(int t)
{
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (_hours > 0)
		return format("%s%d:%.2d:%.2d,%.3d",sign?"-":"",_hours,_min,_sec,_msec);
	if (_min > 0)
		return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
	return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
}

string AudioFile::get_time_str_fuzzy(int t, float dt)
{
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (dt < 1.0){
		if (_hours > 0)
			return format("%s%d:%.2d:%.2d,%.3d",sign?"-":"",_hours,_min,_sec,_msec);
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
	}else{
		if (_hours > 0)
			return format("%s%d:%.2d:%.2d",sign?"-":"",_hours,_min,_sec);
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		return format("%s%.2d",sign?"-":"",_sec);
	}
}

string AudioFile::get_time_str_long(int t)
{
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (_hours > 0)
		return format("%s%dh %.2dm %.2ds %.3dms",sign?"-":"",_hours,_min,_sec,_msec);
	if (_min > 0)
		return format("%s%dm %.2ds %.3dms",sign?"-":"",_min,_sec,_msec);
	return format("%s%ds %.3dms",sign?"-":"",_sec,_msec);
}



Track *AudioFile::addTrack(int type, int index)
{
	if (type == Track::TYPE_TIME){
		// force single time track
		if (getTimeTrack()){
			tsunami->log->error(_("Es existiert schon eine Rhythmus-Spur."));
			return NULL;
		}
	}
	if (index < 0)
		index = tracks.num;
	return (Track*)execute(new ActionTrackAdd(index, type));
}

extern HuiTimer debug_timer;

void AudioFile::updatePeaks(int mode)
{
	msg_db_f("Audio.UpdatePeaks", 2);
	debug_timer.reset();
	foreach(Track *t, tracks)
		t->updatePeaks(mode);
	foreach(Sample *s, samples)
		s->buf.update_peaks(mode);
	//msg_write(format("up %f", debug_timer.get()));
}

int AudioFile::getNumSelectedSamples()
{
	int n = 0;
	foreach(Track *t, tracks)
		foreach(SampleRef *s, t->samples)
			if (s->is_selected)
				n ++;
	return n;
}

void AudioFile::insertSelectedSamples(int level_no)
{
	if (getNumSelectedSamples() > 0)
		execute(new ActionTrackInsertSelectedSamples(this, level_no));
}

void AudioFile::deleteSelectedSamples()
{
	action_manager->beginActionGroup();
	foreachi(Track *t, tracks, i){
		for (int j=t->samples.num-1;j>=0;j--)
			if (t->samples[j]->is_selected)
				t->deleteSample(j);
	}
	action_manager->endActionGroup();
}

void AudioFile::addLevel(const string &name)
{
	execute(new ActionAudioAddLevel(name));
}

void AudioFile::deleteLevel(int index, bool merge)
{
	tsunami->log->error(_("Ebene l&oschen: noch nicht implementiert..."));
}

void AudioFile::renameLevel(int index, const string &name)
{
	execute(new ActionAudioRenameLevel(index, name));
}

void AudioFile::deleteTrack(int index)
{
	execute(new ActionTrackDelete(this, index));
}

Sample *AudioFile::addSample(const string &name, BufferBox &buf)
{
	return (Sample*)execute(new ActionAudioAddSample(name, buf));
}

void AudioFile::deleteSample(int index)
{
	if (samples[index]->ref_count == 0)
		execute(new ActionAudioDeleteSample(index));
	else
		tsunami->log->error(_("Kann nur Samples l&oschen, die nicht benutzt werden!"));
}

void AudioFile::editSampleName(int index, const string &name)
{
	execute(new ActionAudioSampleEditName(this, index, name));
}

void AudioFile::deleteSelection(int level_no, const Range &range, bool all_levels)
{
	if (!range.empty())
		execute(new ActionAudioDeleteSelection(this, level_no, range, all_levels));
}

void AudioFile::createSamplesFromSelection(int level_no, const Range &range)
{
	if (!range.empty())
		execute(new ActionTrackSampleFromSelection(this, range, level_no));
}

void AudioFile::invalidateAllPeaks()
{
	foreach(Track *t, tracks)
		t->invalidateAllPeaks();
	foreach(Sample *s, samples)
		s->buf.invalidate_peaks(s->buf.range());
}

Track *AudioFile::get_track(int track_no)
{
	assert((track_no >= 0) && (track_no < tracks.num) && "AudioFile.get_track");
	return tracks[track_no];
}

SampleRef *AudioFile::get_sample_ref(int track_no, int index)
{
	assert((track_no >= 0) && (track_no < tracks.num) && "AudioFile.get_sample");
	Track *t = tracks[track_no];

	assert((index >= 0) && "AudioFile.get_sample");
	assert((index < t->samples.num) && "AudioFile.get_sample");
	return t->samples[index];
}

int AudioFile::get_sample_by_uid(int uid)
{
	foreachi(Sample *s, samples, i)
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

MidiEffect *AudioFile::get_midi_fx(int track_no, int index)
{
	assert(index >= 0);
	Track *t = get_track(track_no);
	assert(t);
	assert(index < t->midi.fx.num);

	return t->midi.fx[index];
}

Track *AudioFile::getTimeTrack()
{
	foreach(Track *t, tracks)
		if (t->type == t->TYPE_TIME)
			return t;
	return NULL;
}

int AudioFile::getNextBeat(int pos)
{
	Track *t = getTimeTrack();
	if (!t)
		return pos;
	return t->bars.getNextBeat(pos);
}

string AudioFile::getNiceLevelName(int index)
{
	if (level_names[index].num > 0)
		return level_names[index];
	return format(_("Ebene %d"), index + 1);
}

string AudioFile::getTag(const string &key)
{
	foreach(Tag &t, tags)
		if (t.key == key)
			return t.value;
	return "";
}

