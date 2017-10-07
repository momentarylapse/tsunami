/*
 * Rhythm.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef RHYTHM_H_
#define RHYTHM_H_

#include "../lib/base/base.h"
#include "Range.h"

class BarPattern
{
public:
	int num_beats;
	int sub_beats;
	int length;
	int type;

	enum{
		TYPE_BAR,
		TYPE_PAUSE
	};
};

class Bar
{
public:
	Bar(){}
	Bar(const Range &r, int num_beats, int index);
	Range range;
	int num_beats;
	int index;
	float bpm(float sample_rate);
};

class Beat
{
public:
	Beat(){}
	Beat(const Range &r, int bar, int beat, int level);
	Range range;
	int bar_no;
	int beat_no;
	int level;
	Range sub(int index, int parts);
};

class BarCollection : public Array<BarPattern>
{
public:
	Array<Beat> getBeats(const Range &r, bool include_hidden = false, bool include_sub_beats = false);
	Array<Bar> getBars(const Range &r);
	int getNextBeat(int pos);
	int getPrevBeat(int pos);
	int getNextSubBeat(int pos, int beat_partition);
	int getPrevSubBeat(int pos, int beat_partition);
	Range getRange();
};

class DummyBeatSource;

class BeatSource : public VirtualBase
{
public:
	virtual ~BeatSource(){}
	virtual int _cdecl read(Array<Beat> &beats, int samples) = 0;

	static DummyBeatSource *dummy;
};

class DummyBeatSource : public BeatSource
{
public:
	virtual int _cdecl read(Array<Beat> &beats, int samples){ return samples; }
};

class BarStreamer : public BeatSource
{
	BarStreamer(BarCollection &bars);
	virtual int _cdecl read(Array<Beat> &beats, int samples);
	void seek(int pos);

	BarCollection bars;
	int offset;
};

// TODO: find better name!
class RhythmHelper
{
public:
	RhythmHelper(BarCollection *bars);
	Array<Beat> getBeats(const Range &r, bool include_hidden = false, bool include_sub_beats = false);
	int getNextBeat();
	int getPrevBeat();
	int getNextSubBeat(int beat_partition);
	int getPrevSubBeat(int beat_partition);
	Range expand(const Range &r, int beat_partition);
	void set_pos(int pos);
	void consume(int samples);
	void reset();
	BarCollection *bars;
	int offset;
};


#endif /* RHYTHM_H_ */
