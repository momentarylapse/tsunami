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
#include "../lib/file/file.h"

class Data;
class Effect;
class Track;

struct Tag
{
	//Tag(){	HistoryStructReset("Tag", this);	}

	string key, value;
};

class AudioFile : public Data
{
public:
	AudioFile();
	virtual ~AudioFile();
	Track *GetCurTrack();
	Track *GetCurSub();
	Range GetRange();

	int screen2sample(int x);
	int sample2screen(int s);
	string get_time_str(int t);
	string get_time_str_fuzzy(int t, float dt);

	void Reset();
	void NewEmpty(int _sample_rate);
	void NewWithOneTrack(int _sample_rate);
	bool Load(const string &filename, bool deep);
	bool Save(const string &filename);
	void UpdateSelection();
	void UnselectAllSubs();
	void AddTag(const string &key, const string &value);

	virtual void PostActionUpdate();
	void UpdatePeaks();

	void SetCurSub(Track *s);
	void SetCurTrack(Track *t);

	int GetNumSelectedSubs();

	// action
	Track *AddEmptyTrack(int index = -1);
	Track *AddTimeTrack(int index = -1);
	void DeleteCurrentTrack();
	void AddLevel();
	void InsertSelectedSubs();
	void DeleteSelection(bool all_levels);

	Track *get_track(int track_no, int sub_no);

// data
	bool used;
	string filename;
	Array<Tag> tag;
	int sample_rate;

	float volume;

	Array<Effect> fx;
	Array<Track> track;

// editing
	// needed for rendering
	int x, y, width, height;

	// selection within the buffer?
	Range selection;
	bool mo_sel_start, mo_sel_end;
	// (T_T)  -> global

	int cur_track;
	int cur_level;

	float view_pos;
	float view_zoom;

	Range sel_raw;

	Array<string> level_name;
};



int get_track_index(Track *t);
int get_sub_index(Track *s);
void get_track_sub_index(Track *t, int &track_no, int &sub_no);

#endif /* AUDIOFILE_H_ */
