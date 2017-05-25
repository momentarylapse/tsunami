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
#include "../Storage/Storage.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"
#include "../lib/threads/Mutex.h"
#include <assert.h>
#include <math.h>
#include "Song.h"

#include "../Action/Bar/ActionBarAdd.h"
#include "../Action/Bar/ActionBarDelete.h"
#include "../Action/Bar/ActionBarEdit.h"
#include "../Action/Layer/ActionLayerAdd.h"
#include "../Action/Layer/ActionLayerDelete.h"
#include "../Action/Layer/ActionLayerMerge.h"
#include "../Action/Layer/ActionLayerMove.h"
#include "../Action/Layer/ActionLayerRename.h"
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

Song::Exception::Exception(const string &_message)
{
	message = _message;
}

Song::Layer::Layer(const string &_name)
{
	name = _name;
	active = true;
}

Song::Song() :
	Data("AudioFile")
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	default_format = SAMPLE_FORMAT_16;
	compression = 0;
	volume = 1;
	layers.add(new Layer(""));
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
const string Song::MESSAGE_ADD_LAYER = "AddLayer";
const string Song::MESSAGE_EDIT_LAYER = "EditLayer";
const string Song::MESSAGE_DELETE_LAYER = "DeleteLayer";
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
	addTag("artist", hui::Config.getStr("DefaultArtist", AppName));

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

	for (Layer *l: layers)
		delete(l);
	layers.clear();
	layers.add(new Layer(""));

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
	Range r = Range::EMPTY;

	for (Track *t: tracks)
		r = r or t->getRange();

	return r;
}

Range Song::getRangeWithTime()
{
	Range r = getRange();

	if (bars.num > 0)
		r = r or bars.getRange();

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
			throw Exception(_("There already is one rhythm track."));
	}
	if (index < 0)
		index = tracks.num;
	return (Track*)execute(new ActionTrackAdd(type, index));
}

Track *Song::addTrackAfter(int type, Track *ref)
{
	int index = ref->get_index();
	return addTrack(type, index + 1);
}

extern hui::Timer debug_timer;

void Song::updatePeaks()
{
	debug_timer.reset();
	for (Track *t: tracks)
		t->updatePeaks();
	for (Sample *s: samples)
		s->buf.update_peaks();
	//msg_write(format("up %f", debug_timer.get()));
}

void Song::insertSelectedSamples(const SongSelection &sel, int layer_no)
{
	if (sel.getNumSamples() > 0)
		execute(new ActionTrackInsertSelectedSamples(sel, layer_no));
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

Song::Layer *Song::addLayer(const string &name, int index)
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
}

void Song::deleteTrack(Track *track)
{
	if (tracks.num < 2)
		throw Exception(_("At least one layer has to exist."));
	execute(new ActionTrackDelete(track));
}

Sample *Song::addSample(const string &name, BufferBox &buf)
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

void Song::deleteSelection(const SongSelection &sel, int layer_no, bool all_layers)
{
	if (!sel.range.empty())
		execute(new ActionSongDeleteSelection(layer_no, sel, all_layers));
}

void Song::createSamplesFromSelection(const SongSelection &sel, int layer_no)
{
	if (!sel.range.empty())
		execute(new ActionTrackSampleFromSelection(sel, layer_no));
}

void Song::addBar(int index, float bpm, int beats, int sub_beats, bool affect_midi)
{
	BarPattern b;
	b.num_beats = beats;
	b.sub_beats = sub_beats;
	b.type = b.TYPE_BAR;
	b.length = (int)((float)b.num_beats * (float)sample_rate * 60.0f / bpm);
	if (index >= 0)
		execute(new ActionBarAdd(index, b, affect_midi));
	else
		execute(new ActionBarAdd(bars.num, b, affect_midi));
}

void Song::addPause(int index, float time, bool affect_midi)
{
	BarPattern b;
	b.num_beats = 0;
	b.sub_beats = 1;
	b.type = b.TYPE_PAUSE;
	b.length = (int)((float)sample_rate * time);
	if (index >= 0)
		execute(new ActionBarAdd(index, b, affect_midi));
	else
		execute(new ActionBarAdd(bars.num, b, affect_midi));
}

void Song::editBar(int index, BarPattern &p, bool affect_midi)
{
	execute(new ActionBarEdit(index, p, affect_midi));
}

void Song::deleteBar(int index, bool affect_midi)
{
	execute(new ActionBarDelete(index, affect_midi));
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

string Song::getNiceLayerName(int index)
{
	if (layers[index]->name.num > 0)
		return layers[index]->name;
	return format(_("Layer %d"), index + 1);
}

string Song::getTag(const string &key)
{
	for (Tag &t: tags)
		if (t.key == key)
			return t.value;
	return "";
}

