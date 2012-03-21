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
	int GetMin();
	int GetMax();
	int GetLength();

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
	bool selection;
	int selection_start, selection_end, selection_length;
	bool mo_sel_start, mo_sel_end;
	// (T_T)  -> global

	int cur_track;

	float view_pos;
	float view_zoom;

	int sel_start_raw, sel_end_raw;

	//History *history;
};

#endif /* AUDIOFILE_H_ */
