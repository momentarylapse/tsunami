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
	Range GetRange();

	string get_time_str(int t);
	string get_time_str_fuzzy(int t, float dt);

	void Reset();
	void NewEmpty(int _sample_rate);
	void NewWithOneTrack(int track_type, int _sample_rate);
	bool Load(const string &filename, bool deep);
	bool Save(const string &filename);
	void UpdateSelection();
	Range GetPlaybackSelection();
	void UnselectAllSubs();

	virtual void PostActionUpdate();
	void UpdatePeaks(int mode);
	void InvalidateAllPeaks();

	int GetNumSelectedSubs();

	Track *GetTimeTrack();
	int GetNextBeat(int pos);

	// action
	void AddTag(const string &key, const string &value);
	void EditTag(int index, const string &key, const string &value);
	void DeleteTag(int index);
	void SetVolume(float volume);
	Track *AddTrack(int type, int index = -1);
	void DeleteTrack(int index);
	Sample *AddSample(const string &name, BufferBox &buf);
	void DeleteSample(int index);
	void AddLevel();
	void InsertSelectedSubs(int level_no);
	void DeleteSelection(int level_no, bool all_levels);
	void CreateSubsFromSelection(int level_no);

	Track *get_track(int track_no);
	SampleRef *get_sub(int track_no, int sub_no);

// data
	bool used;
	string filename;
	Array<Tag> tag;
	int sample_rate;

	float volume;

	Array<Effect*> fx;
	Array<Track*> track;
	Array<Sample*> sample;

// editing
	// needed for rendering
	rect area;

	// selection within the buffer?
	Range selection;

	Range sel_raw;

	Array<string> level_name;
};



int get_track_index(Track *t);
int get_sample_ref_index(SampleRef *s);
int get_sample_index(Sample *s);

#endif /* AUDIOFILE_H_ */
