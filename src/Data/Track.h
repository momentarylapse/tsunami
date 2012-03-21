/*
 * Track.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef TRACK_H_
#define TRACK_H_

#include "BufferBox.h"
#include "AudioFile.h"

class BufferBox;
class AudioFile;

class EffectParam
{
public:
//	EffectParam(){	HistoryStructReset("EffectParam", this);	}

	string name, type;
	string value;
};

class Effect
{
public:
//	Effect(){	HistoryStructReset("Effect", this);	}

	string filename;
	Array<EffectParam> param;
	bool only_on_selection;
	int start, end;
};

class TimeBar
{
public:
	//TimeBar(){	HistoryStructReset("TimeBar", this);	}

	int num_beats;
	int length;
	int type; // reserved

	// editing
	bool is_selected;
	bool is_mouse_over;
	int x, width;
};

class BarCollection
{
public:
	//BarCollection(){	HistoryStructReset("BarCollection", this);	}

	int pos;
	int length;
	Array<TimeBar> bar;

	// editing
	bool is_selected;
	bool is_mouse_over;
	int x, width;
};


class Track
{
public:
	Track();
	virtual ~Track();
	Track *GetCurSub();
	Track *GetParent();
	int GetMin();
	int GetMax();
	int GetMinUnsafe();
	int GetMaxUnsafe();

// data
	int type;
	string name;

	Array<BufferBox> buffer;
	int length, pos; // sub

	float volume;
	bool muted;

	// repetition
	int rep_num;
	int rep_delay;

	Array<Effect> fx;
	Array<Track> sub;
	int cur_sub;

	// time track
	Array<BarCollection> bar_col;

// editing
	int x, y, width, height;
	int parent;
	AudioFile *root;

	bool is_selected;
	bool is_mouse_over;

//	TrackRenderBuffer render_r[NUM_PEAK_LEVELS], render_l[NUM_PEAK_LEVELS];
};

#endif /* TRACK_H_ */
