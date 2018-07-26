/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Song.h"
#include "base.h"
#include "Track.h"
#include "Sample.h"
#include "SongSelection.h"
#include "Curve.h"
#include "Rhythm/Bar.h"
#include "../Action/ActionManager.h"
#include "../Action/Bar/ActionBarAdd.h"
#include "../Action/Bar/ActionBarDelete.h"
#include "../Action/Bar/ActionBarEdit.h"
#include "../Action/Curve/ActionCurveAdd.h"
#include "../Action/Curve/ActionCurveAddPoint.h"
#include "../Action/Curve/ActionCurveDelete.h"
#include "../Action/Curve/ActionCurveDeletePoint.h"
#include "../Action/Curve/ActionCurveEdit.h"
#include "../Action/Curve/ActionCurveEditPoint.h"
#include "../Action/Sample/ActionSampleAdd.h"
#include "../Action/Sample/ActionSampleDelete.h"
#include "../Action/Sample/ActionSampleEditName.h"
#include "../Action/Sample/ActionSampleScale.h"
#include "../Action/Song/ActionSongDeleteSelection.h"
#include "../Action/Song/Data/ActionSongChangeAllTrackVolumes.h"
#include "../Action/Song/Data/ActionSongSetDefaultFormat.h"
#include "../Action/Song/Data/ActionSongSetSampleRate.h"
#include "../Action/Song/Data/ActionSongSetVolume.h"
#include "../Action/Tag/ActionTagAdd.h"
#include "../Action/Tag/ActionTagDelete.h"
#include "../Action/Tag/ActionTagEdit.h"
#include "../Action/Track/ActionTrackAdd.h"
#include "../Action/Track/ActionTrackDelete.h"
#include "../Action/Track/Sample/ActionTrackInsertSelectedSamples.h"
#include "../Action/Track/Sample/ActionTrackSampleFromSelection.h"
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Effect/ActionTrackToggleEffectEnabled.h"
#include "../Action/Track/Effect/ActionTrackDeleteAudioEffect.h"
#include "../Module/Synth/DummySynthesizer.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"
#include "../lib/threads/Mutex.h"
#include <assert.h>


int get_track_index(Track *t)
{
	if (t)
		return t->get_index();
	return -1;
}

Song::Exception::Exception(const string &_message)
{
	message = _message;
}

Song::Song(Session *session) :
	Data(session)
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	default_format = SampleFormat::SAMPLE_FORMAT_16;
	compression = 0;
	volume = 1;
}

void Song::__init__(Session *session)
{
	new(this) Song(session);
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
const string Song::MESSAGE_EDIT_CURVE = "EditCurve";
const string Song::MESSAGE_ADD_SAMPLE = "AddSample";
const string Song::MESSAGE_DELETE_SAMPLE = "DeleteSample";
const string Song::MESSAGE_ADD_LAYER = "AddLayer";
const string Song::MESSAGE_EDIT_LAYER = "EditLayer";
const string Song::MESSAGE_DELETE_LAYER = "DeleteLayer";
const string Song::MESSAGE_CHANGE_CHANNELS = "ChangeChannels";
const string Song::MESSAGE_EDIT_BARS = "EditBars";


void Song::addTag(const string &key, const string &value)
{
	if ((key != "") and (value != ""))
		execute(new ActionTagAdd(Tag(key, value)));
}

void Song::editTag(int index, const string &key, const string &value)
{
	execute(new ActionTagEdit(index, Tag(key, value)));
}

void Song::deleteTag(int index)
{
	execute(new ActionTagDelete(index));
}

void Song::addEffect(AudioEffect *effect)
{
	execute(new ActionTrackAddEffect(nullptr, effect));
}

// execute after editing...
void Song::editEffect(AudioEffect *effect, const string &param_old)
{
	execute(new ActionTrackEditEffect(effect, param_old));
}

void Song::enableEffect(AudioEffect *effect, bool enabled)
{
	if (effect->enabled != enabled)
		execute(new ActionTrackToggleEffectEnabled(effect));
}

void Song::deleteEffect(AudioEffect *effect)
{
	foreachi (AudioEffect *f, fx, index)
		if (f == effect)
			execute(new ActionTrackDeleteEffect(nullptr, index));
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
	addTag("artist", hui::Config.getStr("DefaultArtist", AppName));

	action_manager->enable(true);
	notify();
}

void Song::newWithOneTrack(SignalType track_type, int _sample_rate)
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
	default_format = SampleFormat::SAMPLE_FORMAT_16;
	compression = 0;
	sample_rate = DEFAULT_SAMPLE_RATE;

	for (AudioEffect *f: fx)
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

	action_manager->reset();

	notify();
	notify(MESSAGE_NEW);
}

Song::~Song()
{
	reset();
}

bool Song::is_empty()
{
	if (filename.num > 0)
		return false;
	return action_manager->isSave();
}

Range Song::range()
{
	Range r = Range::EMPTY;

	for (Track *t: tracks)
		r = r or t->range();

	return r;
}

Range Song::range_with_time()
{
	Range r = range();

	if (bars.num > 0)
		r = r or bars.range();

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
		pos += bars[i]->length;
	return pos;
}



Track *Song::addTrack(SignalType type, int index)
{
	if (type == SignalType::BEATS){
		// force single time track
		if (getTimeTrack())
			throw Exception(_("There already is one rhythm track."));
	}
	if (index < 0)
		index = tracks.num;
	return (Track*)execute(new ActionTrackAdd(new Track(type, CreateSynthesizer(session, "")), index));
}

Track *Song::addTrackAfter(SignalType type, Track *ref)
{
	int index = ref->get_index();
	return addTrack(type, index + 1);
}

void Song::insertSelectedSamples(const SongSelection &sel)
{
	if (sel.num_samples() > 0)
		execute(new ActionTrackInsertSelectedSamples(sel, 0));
}

void Song::deleteSelectedSamples(const SongSelection &sel)
{
	action_manager->beginActionGroup();
	for (Track *t: tracks)
	for (TrackLayer *l: t->layers){
		for (int j=l->samples.num-1; j>=0; j--)
			if (sel.has(l->samples[j]))
				l->deleteSampleRef(l->samples[j]);
	}
	action_manager->endActionGroup();
}

/*Song::Layer *Song::addLayer(const string &name, int index)
{
	return (Song::Layer*)execute(new ActionLayerAdd(name, index));
}

void Song::deleteLayer(int index)
{
	if (layers.num < 2)
		throw Exception(_("At least one layer has to exist."));
	execute(new ActionLayerDelete(index));
}

void Song::mergeLayers(int source, int target)
{
	if (layers.num < 2)
		throw Exception(_("At least one layer has to exist."));
	if (source == target)
		throw Exception(_("Can't merge a layer with itself."));
	execute(new ActionLayerMerge(source, target));
}

void Song::moveLayer(int source, int target)
{
	execute(new ActionLayerMove(source, target));
}

void Song::renameLayer(int index, const string &name)
{
	execute(new ActionLayerRename(index, name));
}*/

void Song::deleteTrack(Track *track)
{
	if (tracks.num < 2)
		throw Exception(_("At least one layer has to exist."));
	execute(new ActionTrackDelete(track));
}

Sample *Song::addSample(const string &name, AudioBuffer &buf)
{
	return (Sample*)execute(new ActionSampleAdd(name, buf, false));
}

void Song::deleteSample(Sample *s)
{
	if (s->ref_count > 0)
		throw Exception(_("Can only delete samples which are unused."));
	execute(new ActionSampleDelete(s));
}

void Song::editSampleName(Sample *s, const string &name)
{
	execute(new ActionSampleEditName(s, name));
}

void Song::scaleSample(Sample *s, int new_size, int method)
{
	execute(new ActionSampleScale(s, new_size, method));
}

void Song::deleteSelection(const SongSelection &sel)
{
	execute(new ActionSongDeleteSelection(sel));
}

void Song::createSamplesFromSelection(const SongSelection &sel)
{
	if (!sel.range.empty())
		execute(new ActionTrackSampleFromSelection(sel));
}

void Song::addBar(int index, float bpm, int beats, int sub_beats, int mode)
{
	int length = (int)((float)beats * (float)sample_rate * 60.0f / bpm);
	if (index >= 0)
		execute(new ActionBarAdd(index, length, beats, sub_beats, mode));
	else
		execute(new ActionBarAdd(bars.num, length, beats, sub_beats, mode));
}

void Song::addPause(int index, float time, int mode)
{
	int length = (int)((float)sample_rate * time);
	if (index >= 0)
		execute(new ActionBarAdd(index, length, 0, 0, mode));
	else
		execute(new ActionBarAdd(bars.num, length, 0, 0, mode));
}

void Song::editBar(int index, int length, int beats, int sub_beats, int mode)
{
	execute(new ActionBarEdit(index, length, beats, sub_beats, mode));
}

void Song::deleteBar(int index, bool affect_midi)
{
	execute(new ActionBarDelete(index, affect_midi));
}

void Song::deleteTimeInterval(int index, const Range &range)
{
	//execute(new ActionBarDelete(index, affect_midi));
}

Curve *Song::addCurve(const string &name, Array<Curve::Target> &targets)
{
	Curve *c = new Curve;
	c->name = name;
	c->targets = targets;
	execute(new ActionCurveAdd(c, curves.num));
	return c;
}
void Song::deleteCurve(Curve *curve)
{
	foreachi(auto c, curves, i)
		if (c == curve)
			execute(new ActionCurveDelete(i));
}

void Song::editCurve(Curve *curve, const string &name, float min, float max)
{
	execute(new ActionCurveEdit(curve, name, min, max, curve->targets));
}

void Song::curveSetTargets(Curve *curve, Array<Curve::Target> &targets)
{
	execute(new ActionCurveEdit(curve, curve->name, curve->min, curve->max, targets));
}

void Song::curveAddPoint(Curve *curve, int pos, float value)
{
	execute(new ActionCurveAddPoint(curve, pos, value));
}

void Song::curveDeletePoint(Curve *curve, int index)
{
	execute(new ActionCurveDeletePoint(curve, index));
}

void Song::curveEditPoint(Curve *curve, int index, int pos, float value)
{
	execute(new ActionCurveEditPoint(curve, index, pos, value));
}

void Song::invalidateAllPeaks()
{
	for (Track *t: tracks)
		t->invalidateAllPeaks();
	for (Sample *s: samples)
		s->buf.peaks.clear();
}


Sample* Song::get_sample_by_uid(int uid)
{
	for (Sample *s: samples)
		if (s->uid == uid)
			return s;
	return nullptr;
}

AudioEffect *Song::get_fx(Track *track, int index)
{
	assert(index >= 0);
	if (track >= 0){
		assert(index < track->fx.num);

		return track->fx[index];
	}else{
		assert(index < fx.num);

		return fx[index];
	}
}

// unused...
MidiEffect *Song::get_midi_fx(Track *track, int index)
{
	assert(index >= 0);
	assert(index < track->midi_fx.num);

	return track->midi_fx[index];
}

Track *Song::getTimeTrack()
{
	for (Track *t: tracks)
		if (t->type == SignalType::BEATS)
			return t;
	return nullptr;
}

string Song::getTag(const string &key)
{
	for (Tag &t: tags)
		if (t.key == key)
			return t.value;
	return "";
}

Array<TrackLayer*> Song::layers() const
{
	Array<TrackLayer*> layers;
	for (Track *t: tracks)
		layers.append(t->layers);
	return layers;

}
