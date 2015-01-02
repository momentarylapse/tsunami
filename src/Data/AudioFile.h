/*
 * AudioFile.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef AUDIOFILE_H_
#define AUDIOFILE_H_

#include "Data.h"
#include "Track.h"
#include "Sample.h"
#include "../lib/base/base.h"
#include "../lib/math/rect.h"

class Data;
class Effect;
class Track;
class Sample;
class Synthesizer;
class Curve;

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

class AudioFile : public Data
{
public:
	AudioFile();
	virtual ~AudioFile();
	Range _cdecl getRange();

	static const string MESSAGE_NEW;
	static const string MESSAGE_SELECTION_CHANGE;
	static const string MESSAGE_ADD_TRACK;
	static const string MESSAGE_DELETE_TRACK;
	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_CURVE;
	static const string MESSAGE_DELETE_CURVE;

	string get_time_str(int t);
	string get_time_str_fuzzy(int t, float dt);
	string get_time_str_long(int t);

	void reset();
	void _cdecl newEmpty(int _sample_rate);
	void _cdecl newWithOneTrack(int track_type, int _sample_rate);
	bool _cdecl load(const string &filename, bool deep);
	bool _cdecl save(const string &filename);
	void _cdecl updateSelection(const Range &range);
	void unselectAllSamples();

	void updatePeaks(int mode);
	void invalidateAllPeaks();

	int getNumSelectedSamples();

	Track *_cdecl getTimeTrack();
	int _cdecl getNextBeat(int pos);
	string _cdecl getNiceLevelName(int index);

	// action
	void _cdecl addTag(const string &key, const string &value);
	void _cdecl editTag(int index, const string &key, const string &value);
	void _cdecl deleteTag(int index);
	void _cdecl addEffect(Effect *effect);
	void _cdecl deleteEffect(int index);
	void _cdecl editEffect(int index, const string &param_old);
	void _cdecl enableEffect(int index, bool enabled);
	void _cdecl setVolume(float volume);
	Track *_cdecl addTrack(int type, int index = -1);
	void _cdecl deleteTrack(int index);
	Sample *_cdecl addSample(const string &name, BufferBox &buf);
	void _cdecl deleteSample(int index);
	void _cdecl editSampleName(int index, const string &name);
	void _cdecl addLevel(const string &name);
	void _cdecl deleteLevel(int index, bool merge);
	void _cdecl renameLevel(int index, const string &name);
	void _cdecl insertSelectedSamples(int level_no);
	void _cdecl deleteSelectedSamples();
	void _cdecl deleteSelection(int level_no, const Range &range, bool all_levels);
	void _cdecl createSamplesFromSelection(int level_no, const Range &range);

	Track *get_track(int track_no);
	SampleRef *get_sample_ref(int track_no, int index);
	int get_sample_by_uid(int uid);
	Effect *get_fx(int track_no, int index);
	MidiEffect *get_midi_fx(int track_no, int index);

// data
	string filename;
	Array<Tag> tag;
	int sample_rate;

	float volume;

	Array<Effect*> fx;
	Array<Track*> track;
	Array<Sample*> sample;
	Array<Curve*> curve;

	Array<string> level_name;
};


int get_track_index(Track *t);
int get_sample_index(Sample *s);

float amplitude2db(float amp);
float db2amplitude(float db);

#endif /* AUDIOFILE_H_ */
