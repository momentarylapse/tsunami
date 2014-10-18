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
	Range _cdecl GetRange();

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

	void Reset();
	void _cdecl NewEmpty(int _sample_rate);
	void _cdecl NewWithOneTrack(int track_type, int _sample_rate);
	bool _cdecl Load(const string &filename, bool deep);
	bool _cdecl Save(const string &filename);
	void _cdecl UpdateSelection(const Range &range);
	void UnselectAllSamples();

	void UpdatePeaks(int mode);
	void InvalidateAllPeaks();

	int GetNumSelectedSamples();

	Track *_cdecl GetTimeTrack();
	int _cdecl GetNextBeat(int pos);
	string _cdecl GetNiceLevelName(int index);

	// action
	void _cdecl AddTag(const string &key, const string &value);
	void _cdecl EditTag(int index, const string &key, const string &value);
	void _cdecl DeleteTag(int index);
	void _cdecl AddEffect(Effect *effect);
	void _cdecl DeleteEffect(int index);
	void _cdecl EditEffect(int index, const string &param_old);
	void _cdecl EnableEffect(int index, bool enabled);
	void _cdecl SetVolume(float volume);
	Track *_cdecl AddTrack(int type, int index = -1);
	void _cdecl DeleteTrack(int index);
	Sample *_cdecl AddSample(const string &name, BufferBox &buf);
	void _cdecl DeleteSample(int index);
	void _cdecl EditSampleName(int index, const string &name);
	void _cdecl AddLevel(const string &name);
	void _cdecl DeleteLevel(int index, bool merge);
	void _cdecl RenameLevel(int index, const string &name);
	void _cdecl InsertSelectedSamples(int level_no);
	void _cdecl DeleteSelectedSamples();
	void _cdecl DeleteSelection(int level_no, const Range &range, bool all_levels);
	void _cdecl CreateSamplesFromSelection(int level_no, const Range &range);

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

// editing
	// needed for rendering
	rect area;

	Array<string> level_name;
};


int get_track_index(Track *t);
int get_sample_index(Sample *s);

float amplitude2db(float amp);
float db2amplitude(float db);

#endif /* AUDIOFILE_H_ */
