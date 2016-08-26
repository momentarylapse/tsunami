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
	Range getRange();
};


#endif /* RHYTHM_H_ */
