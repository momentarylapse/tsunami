/*
 * BarCollection.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BARCOLLECTION_H_
#define SRC_RHYTHM_BARCOLLECTION_H_


#include "../lib/base/base.h"

class Beat;
class Bar;
class Range;

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


class BarCollection : public Array<BarPattern>
{
public:
	Array<Beat> getBeats(const Range &r, bool include_hidden = false, bool include_sub_beats = false);
	Array<Bar> getBars(const Range &r);
	int getNextBeat(int pos);
	int getPrevBeat(int pos);
	int getNextSubBeat(int pos, int beat_partition);
	int getPrevSubBeat(int pos, int beat_partition);
	Range expand(const Range &r, int beat_partition);
	Range getRange();
};



#endif /* SRC_RHYTHM_BARCOLLECTION_H_ */
