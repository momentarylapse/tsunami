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
#include "MidiData.h"
#include "Rhythm.h"
#include "AudioFile.h"
#include "../Plugins/Effect.h"
#include "../lib/math/rect.h"


#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2
#define MIN_MAX_FACTOR		10

#define DEFAULT_SAMPLE_RATE		44100

class BufferBox;
class AudioFile;


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
	void SetName(const string &name);
	void SetMuted(bool muted);
	void SetVolume(float volume);
	void SetPanning(float panning);
	BufferBox GetBuffers(int level_no, const Range &r);
	Track *AddEmptySubTrack(const Range &r, const string &name);
	void InsertMidiData(int offset, MidiData &midi);


	enum
	{
		TYPE_AUDIO,
		TYPE_TIME,
		TYPE_MIDI
	};

// data
	int type;
	string name;

	Array<TrackLevel> level;
	int length, pos; // sub

	float volume, panning;
	bool muted;

	// repetition
	int rep_num;
	int rep_delay;

	Array<Effect> fx;
	Array<Track*> sub;

	// time track
	BarCollection bar;

	// midi track
	MidiData midi;

// editing
	rect area;
	int parent;
	AudioFile *root;

	bool is_selected;
};

#endif /* TRACK_H_ */
