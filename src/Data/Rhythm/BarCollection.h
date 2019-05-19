/*
 * BarCollection.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_DATA_RHYTHM_BARCOLLECTION_H_
#define SRC_DATA_RHYTHM_BARCOLLECTION_H_


#include "../../lib/base/base.h"

class Beat;
class Bar;
class Range;


class BarCollection : public Array<Bar*>
{
public:
	Array<Beat> get_beats(const Range &r, bool include_hidden = false, bool include_sub_beats = false, int sub_beat_partition = 1);
	Array<Bar*> get_bars(const Range &r);
	int get_next_beat(int pos);
	int get_prev_beat(int pos);
	int get_next_sub_beat(int pos, int sub_beat_partition);
	int get_prev_sub_beat(int pos, int sub_beat_partition);
	Range get_sub_beats(int pos, int sub_beat_partition, int num_sub_beats);
	Range expand(const Range &r, int sub_beat_partition);
	Range range();
	Range sub_range(const Range &indices);
};



#endif /* SRC_DATA_RHYTHM_BARCOLLECTION_H_ */
