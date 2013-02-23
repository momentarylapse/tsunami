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

class Beat
{
public:
	Beat(){}
	Beat(int p, int bar, int beat);
	int pos;
	int bar_no;
	int beat_no;
};

class BarCollection : public Array<Bar>
{
public:
	Array<Beat> GetBeats(const Range &r);
	int GetNextBeat(int pos);
	Range GetRange();
};


#endif /* RHYTHM_H_ */
