/*
 * Song.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SONG_H_
#define SONG_H_

#include "Data.h"
#include "Track.h"
#include "Sample.h"
#include "SampleRef.h"
#include "Rhythm.h"
#include "../lib/base/base.h"
#include "../lib/math/rect.h"

class Data;
class Effect;
class Track;
class Sample;
class Synthesizer;
class Curve;
class SongSelection;

struct Tag
{
	string key, value;
	Tag(){}
	Tag(const string &_key, const string &_value)
	{
		key = _key;
		value = _value;
	}
};

class Song : public Data
{
public:
	Song();
	virtual ~Song();

	void _cdecl __init__();
	void _cdecl __delete__();

	Range _cdecl getRange();
	Range _cdecl getRangeWithTime();

	static const string MESSAGE_NEW;
	static const string MESSAGE_ADD_TRACK;
	static const string MESSAGE_DELETE_TRACK;
	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_CURVE;
	static const string MESSAGE_DELETE_CURVE;
	static const string MESSAGE_ADD_SAMPLE;
	static const string MESSAGE_DELETE_SAMPLE;
	static const string MESSAGE_ADD_LEVEL;
	static const string MESSAGE_EDIT_LEVEL;
	static const string MESSAGE_DELETE_LEVEL;
	static const string MESSAGE_EDIT_BARS;

	string _cdecl get_time_str(int t);
	string _cdecl get_time_str_fuzzy(int t, float dt);
	string _cdecl get_time_str_long(int t);

	virtual void _cdecl reset();
	void _cdecl newEmpty(int _sample_rate);
	void _cdecl newWithOneTrack(int track_type, int _sample_rate);
	virtual bool _cdecl load(const string &filename, bool deep);
	virtual bool _cdecl save(const string &filename);

	void _cdecl updatePeaks();
	void _cdecl invalidateAllPeaks();

	Track *_cdecl getTimeTrack();
	int _cdecl getNextBeat(int pos);
	int _cdecl barOffset(int index);
	string _cdecl getNiceLevelName(int index);

	string _cdecl getTag(const string &key);

	// action
	void _cdecl addTag(const string &key, const string &value);
	void _cdecl editTag(int index, const string &key, const string &value);
	void _cdecl deleteTag(int index);
	void _cdecl addEffect(Effect *effect);
	void _cdecl deleteEffect(int index);
	void _cdecl editEffect(int index, const string &param_old);
	void _cdecl enableEffect(int index, bool enabled);
	void _cdecl setVolume(float volume);
	void _cdecl changeAllTrackVolumes(Track *t, float volume);
	void _cdecl setSampleRate(int sample_rate);
	void _cdecl setDefaultFormat(SampleFormat format);
	void _cdecl setCompression(int compression);
	Track *_cdecl addTrack(int type, int index = -1);
	void _cdecl deleteTrack(int index);
	Sample *_cdecl addSample(const string &name, BufferBox &buf);
	void _cdecl deleteSample(int index);
	void _cdecl editSampleName(int index, const string &name);
	void _cdecl addLevel(const string &name);
	void _cdecl deleteLevel(int index, bool merge);
	void _cdecl renameLevel(int index, const string &name);
	void _cdecl addBar(int index, float bpm, int beats, int sub_beats, bool affect_midi);
	void _cdecl addPause(int index, float time, bool affect_midi);
	void _cdecl editBar(int index, BarPattern &p, bool affect_midi);
	void _cdecl deleteBar(int index, bool affect_midi);
	void _cdecl insertSelectedSamples(const SongSelection &sel, int level_no);
	void _cdecl deleteSelectedSamples(const SongSelection &sel);
	void _cdecl deleteSelection(const SongSelection &sel, int level_no, bool all_levels);
	void _cdecl createSamplesFromSelection(const SongSelection &sel, int level_no);

	// helper
	Track *_cdecl get_track(int track_no);
	SampleRef *_cdecl get_sample_ref(int track_no, int index);
	int _cdecl get_sample_by_uid(int uid);
	Effect *_cdecl get_fx(int track_no, int index);
	MidiEffect *_cdecl get_midi_fx(int track_no, int index);

// data
	string filename;
	Array<Tag> tags;
	int sample_rate;
	SampleFormat default_format;
	int compression;

	float volume;

	Array<Effect*> fx;
	Array<Track*> tracks;
	Array<Sample*> samples;
	Array<Curve*> curves;
	BarCollection bars;

	Array<string> level_names;
};


int get_track_index(Track *t);
int get_sample_index(Sample *s);

float amplitude2db(float amp);
float db2amplitude(float db);

#endif /* SONG_H_ */
