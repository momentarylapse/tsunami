/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Curve.h"
#include "../Plugins/Effect.h"
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
#include "../lib/threads/Mutex.h"
#include <assert.h>
#include <math.h>
#include "Song.h"

#include "../Action/Song/ActionSongDeleteSelection.h"
#include "../Action/Song/Bar/ActionSongAddBar.h"
#include "../Action/Song/Bar/ActionSongEditBar.h"
#include "../Action/Song/Bar/ActionSongDeleteBar.h"
#include "../Action/Song/Data/ActionSongChangeAllTrackVolumes.h"
#include "../Action/Song/Data/ActionSongSetDefaultFormat.h"
#include "../Action/Song/Data/ActionSongSetSampleRate.h"
#include "../Action/Song/Data/ActionSongSetVolume.h"
#include "../Action/Song/Level/ActionSongAddLevel.h"
#include "../Action/Song/Level/ActionSongDeleteLevel.h"
#include "../Action/Song/Level/ActionSongRenameLevel.h"
#include "../Action/Song/Level/ActionSongMergeLevels.h"
#include "../Action/Song/Sample/ActionSongAddSample.h"
#include "../Action/Song/Sample/ActionSongDeleteSample.h"
#include "../Action/Song/Sample/ActionSongSampleEditName.h"
#include "../Action/Song/Sample/ActionSampleScale.h"
#include "../Action/Song/Tag/ActionSongEditTag.h"
#include "../Action/Song/Tag/ActionSongAddTag.h"
#include "../Action/Song/Tag/ActionSongDeleteTag.h"

float amplitude2db(float amp)
{
	return log10(amp) * 20.0f;
}

float db2amplitude(float db)
{
	return pow(10, db * 0.05);
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

SongException::SongException(const string &_message)
{
	message = _message;
}

Song::Song() :
	Data("AudioFile")
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	default_format = SAMPLE_FORMAT_16;
	compression = 0;
	volume = 1;
	level_names.add("");
}

void Song::__init__()
{
	new(this) Song;
}

void Song::__delete__()
{
	this->Song::~Song();
}


const string Song::MESSAGE_NEW = "New";
const string Song::MESSAGE_ADD_TRACK = "AddTrack";
const string Song::MESSAGE_DELETE_TRACK = "DeleteTrack";
const string Song::MESSAGE_ADD_EFFECT = "AddEffect";
const string Song::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Song::MESSAGE_ADD_CURVE = "AddCurve";
const string Song::MESSAGE_DELETE_CURVE = "DeleteCurve";
const string Song::MESSAGE_ADD_SAMPLE = "AddSample";
const string Song::MESSAGE_DELETE_SAMPLE = "DeleteSample";
const string Song::MESSAGE_ADD_LEVEL = "AddLevel";
const string Song::MESSAGE_EDIT_LEVEL = "EditLevel";
const string Song::MESSAGE_DELETE_LEVEL = "DeleteLevel";
const string Song::MESSAGE_EDIT_BARS = "EditBars";


void Song::addTag(const string &key, const string &value)
{
	if ((key != "") and (value != ""))
		execute(new ActionSongAddTag(Tag(key, value)));
}

void Song::editTag(int index, const string &key, const string &value)
{
	execute(new ActionSongEditTag(index, Tag(key, value)));
}

void Song::deleteTag(int index)
{
	execute(new ActionSongDeleteTag(index));
}

void Song::addEffect(Effect *effect)
{
	execute(new ActionTrackAddEffect(NULL, effect));
}

// execute after editing...
void Song::editEffect(int index, const string &param_old)
{
	execute(new ActionTrackEditEffect(NULL, index, param_old, fx[index]));
}

void Song::enableEffect(int index, bool enabled)
{
	if (fx[index]->enabled != enabled)
		execute(new ActionTrackToggleEffectEnabled(NULL, index));
}

void Song::deleteEffect(int index)
{
	execute(new ActionTrackDeleteEffect(NULL, index));
}

void Song::setVolume(float volume)
{
	execute(new ActionSongSetVolume(this, volume));
}

void Song::changeAllTrackVolumes(Track *t, float volume)
{
	execute(new ActionSongChangeAllTrackVolumes(this, t, volume));
}

void Song::setSampleRate(int _sample_rate)
{
	if (_sample_rate > 0)
		execute(new ActionSongSetSampleRate(this, _sample_rate));
}

void Song::setDefaultFormat(SampleFormat _format)
{
	execute(new ActionSongSetDefaultFormat(_format, compression));
}

void Song::setCompression(int _compression)
{
	execute(new ActionSongSetDefaultFormat(default_format, _compression));
}

void Song::newEmpty(int _sample_rate)
{
	reset();
	action_manager->enable(false);
	sample_rate = _sample_rate;

	// default tags
	addTag("title", "New Audio File");//_("New Audio File"));
	addTag("album", AppName);
	addTag("artist", HuiConfig.getStr("DefaultArtist", AppName));

	action_manager->enable(true);
	notify();
}

void Song::newWithOneTrack(int track_type, int _sample_rate)
{
	notifyBegin();
	newEmpty(_sample_rate);
	action_manager->enable(false);
	addTrack(track_type);
	action_manager->enable(true);
	notifyEnd();
}

// delete all data
void Song::reset()
{
	action_manager->reset();

	filename = "";
	tags.clear();
	bars.clear();
	volume = 1;
	default_format = SAMPLE_FORMAT_16;
	compression = 0;
	sample_rate = DEFAULT_SAMPLE_RATE;

	for (Effect *f: fx)
		delete(f);
	fx.clear();

	for (Track *t: tracks)
		delete(t);
	tracks.clear();

	for (Sample *s: samples)
		delete(s);
	samples.clear();

	for (Curve *c: curves)
		delete(c);
	curves.clear();

	level_names.clear();
	level_names.add("");

	action_manager->reset();

	notify();
	notify(MESSAGE_NEW);
}

Song::~Song()
{
	reset();
}


bool Song::load(const string & filename, bool deep)
{
	return tsunami->storage->load(this, filename);
}

bool Song::save(const string & filename)
{
	return tsunami->storage->save(this, filename);
}

Range Song::getRange()
{
	int min =  1073741824;
	int max = -1073741824;
	Range r = Range(min, max - min);

	for (Track *t: tracks)
		r = r or t->getRangeUnsafe();

	if (r.length < 0)
		return Range::EMPTY;
	return r;
}

Range Song::getRangeWithTime()
{
	int min =  1073741824;
	int max = -1073741824;
	Range r = Range(min, max - min);

	if (bars.num > 0)
		r = r or bars.getRange();

	for (Track *t: tracks)
		r = r or t->getRangeUnsafe();

	if (r.length < 0)
		return Range::EMPTY;
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

string Song::get_time_str(int t)
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

string Song::get_time_str_fuzzy(int t, float dt)
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

string Song::get_time_str_long(int t)
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

int Song::barOffset(int index)
{
	int pos = 0;
	for (int i=0; i<min(index, bars.num); i++)
		pos += bars[i].length;
	return pos;
}



Track *Song::addTrack(int type, int index)
{
	if (type == Track::TYPE_TIME){
		// force single time track
		if (getTimeTrack())
			throw SongException(_("There already is one rhythm track."));
	}
	if (index < 0)
		index = tracks.num;
	return (Track*)execute(new ActionTrackAdd(index, type));
}

extern HuiTimer debug_timer;

void Song::updatePeaks()
{
	debug_timer.reset();
	for (Track *t: tracks)
		t->updatePeaks();
	for (Sample *s: samples)
		s->buf.update_peaks();
	//msg_write(format("up %f", debug_timer.get()));
}

void Song::insertSelectedSamples(const SongSelection &sel, int level_no)
{
	if (sel.getNumSamples() > 0)
		execute(new ActionTrackInsertSelectedSamples(sel, level_no));
}

void Song::deleteSelectedSamples(const SongSelection &sel)
{
	action_manager->beginActionGroup();
	for (Track *t: tracks){
		for (int j=t->samples.num-1; j>=0; j--)
			if (sel.has(t->samples[j]))
				t->deleteSampleRef(j);
	}
	action_manager->endActionGroup();
}

void Song::addLevel(const string &name)
{
	execute(new ActionSongAddLevel(name));
}

void Song::deleteLevel(int index)
{
	if (level_names.num < 2)
		throw SongException(_("At least one level has to exist."));
	execute(new ActionSongDeleteLevel(index));
}

void Song::mergeLevels(int source, int target)
{
	if (level_names.num < 2)
		throw SongException(_("At least one level has to exist."));
	if (source == target)
		throw SongException(_("Can't merge a level with itself."));
	execute(new ActionSongMergeLevels(source, target));
}

void Song::renameLevel(int index, const string &name)
{
	execute(new ActionSongRenameLevel(index, name));
}

void Song::deleteTrack(int index)
{
	if (tracks.num < 2)
		throw SongException(_("At least one level has to exist."));
	execute(new ActionTrackDelete(index));
}

Sample *Song::addSample(const string &name, BufferBox &buf)
{
	return (Sample*)execute(new ActionSongAddSample(name, buf));
}

void Song::deleteSample(Sample *s)
{
	if (s->ref_count > 0)
		throw SongException(_("Can only delete samples which are unused."));
	execute(new ActionSongDeleteSample(s));
}

void Song::editSampleName(Sample *s, const string &name)
{
	execute(new ActionSongSampleEditName(s, name));
}

void Song::scaleSample(Sample *s, int new_size, int method)
{
	execute(new ActionSampleScale(s, new_size, method));
}

void Song::deleteSelection(const SongSelection &sel, int level_no, bool all_levels)
{
	if (!sel.range.empty())
		execute(new ActionSongDeleteSelection(level_no, sel, all_levels));
}

void Song::createSamplesFromSelection(const SongSelection &sel, int level_no)
{
	if (!sel.range.empty())
		execute(new ActionTrackSampleFromSelection(sel, level_no));
}

void Song::addBar(int index, float bpm, int beats, int sub_beats, bool affect_midi)
{
	BarPattern b;
	b.num_beats = beats;
	b.sub_beats = sub_beats;
	b.type = b.TYPE_BAR;
	b.length = (int)((float)b.num_beats * (float)sample_rate * 60.0f / bpm);
	if (index >= 0)
		execute(new ActionSongAddBar(index, b, affect_midi));
	else
		execute(new ActionSongAddBar(bars.num, b, affect_midi));
}

void Song::addPause(int index, float time, bool affect_midi)
{
	BarPattern b;
	b.num_beats = 0;
	b.sub_beats = 1;
	b.type = b.TYPE_PAUSE;
	b.length = (int)((float)sample_rate * time);
	if (index >= 0)
		execute(new ActionSongAddBar(index, b, affect_midi));
	else
		execute(new ActionSongAddBar(bars.num, b, affect_midi));
}

void Song::editBar(int index, BarPattern &p, bool affect_midi)
{
	execute(new ActionSongEditBar(index, p, affect_midi));
}

void Song::deleteBar(int index, bool affect_midi)
{
	execute(new ActionSongDeleteBar(index, affect_midi));
}

void Song::invalidateAllPeaks()
{
	for (Track *t: tracks)
		t->invalidateAllPeaks();
	for (Sample *s: samples)
		s->buf.peaks.clear();
}

Track *Song::get_track(int track_no)
{
	assert((track_no >= 0) and (track_no < tracks.num) and "AudioFile.get_track");
	return tracks[track_no];
}

SampleRef *Song::get_sample_ref(int track_no, int index)
{
	assert((track_no >= 0) and (track_no < tracks.num) and "AudioFile.get_sample");
	Track *t = tracks[track_no];

	assert((index >= 0) and "AudioFile.get_sample");
	assert((index < t->samples.num) and "AudioFile.get_sample");
	return t->samples[index];
}

Sample* Song::get_sample_by_uid(int uid)
{
	for (Sample *s: samples)
		if (s->uid == uid)
			return s;
	return NULL;
}

Effect *Song::get_fx(int track_no, int index)
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

MidiEffect *Song::get_midi_fx(int track_no, int index)
{
	assert(index >= 0);
	Track *t = get_track(track_no);
	assert(t);
	assert(index < t->midi.fx.num);

	return t->midi.fx[index];
}

Track *Song::getTimeTrack()
{
	for (Track *t: tracks)
		if (t->type == t->TYPE_TIME)
			return t;
	return NULL;
}

int Song::getNextBeat(int pos)
{
	return bars.getNextBeat(pos);
}

string Song::getNiceLevelName(int index)
{
	if (level_names[index].num > 0)
		return level_names[index];
	return format(_("Level %d"), index + 1);
}

string Song::getTag(const string &key)
{
	for (Tag &t: tags)
		if (t.key == key)
			return t.value;
	return "";
}

