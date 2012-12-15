/*
 * Track.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef TRACK_H_
#define TRACK_H_

#include "Range.h"
#include "BufferBox.h"
#include "AudioFile.h"
#include "../Plugins/Effect.h"
#include "../lib/types/rect.h"


#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2
#define MIN_MAX_FACTOR		10

#define DEFAULT_SAMPLE_RATE		44100

class BufferBox;
class AudioFile;

class Bar
{
public:
	int num_beats;
	int length;
	int type;
	int count;

	enum{
		TYPE_BAR,
		TYPE_PAUSE
	};

	// editing
	bool is_selected;
	int x, width;
};


class TrackLevel
{
public:
	Array<BufferBox> buffer;
};


class Track
{
public:
	Track();
	virtual ~Track();
	Track *GetParent();
	Range GetRange();
	Range GetRangeUnsafe();

	void Reset();
	void UpdatePeaks(int mode);
	void InvalidateAllPeaks();
	BufferBox ReadBuffers(int level_no, const Range &r);
	BufferBox ReadBuffersCol(const Range &r);

	string GetNiceName();

	// actions
	BufferBox GetBuffers(int level_no, const Range &r);
	Track *AddEmptySubTrack(const Range &r, const string &name);


	enum
	{
		TYPE_AUDIO,
		TYPE_TIME
	};

// data
	int type;
	string name;

	Array<TrackLevel> level;
	int length, pos; // sub

	float volume;
	bool muted;

	// repetition
	int rep_num;
	int rep_delay;

	Array<Effect> fx;
	Array<Track*> sub;

	// time track
	Array<Bar> bar;

// editing
	rect area;
	int parent;
	AudioFile *root;

	bool is_selected;
};

#endif /* TRACK_H_ */
