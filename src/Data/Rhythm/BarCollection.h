/*
 * BarCollection.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_DATA_RHYTHM_BARCOLLECTION_H_
#define SRC_DATA_RHYTHM_BARCOLLECTION_H_


#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"
#include "Bar.h"

class Beat;
class Bar;
class Range;


class BarCollection : public shared_array<Bar> {
public:
	Array<Beat> get_beats(const Range &r, bool include_hidden = false, bool include_sub_beats = false, int sub_beat_partition = 1) const;
	Array<Bar*> get_bars(const Range &r) const;
	int get_next_beat(int pos) const;
	int get_prev_beat(int pos) const;
	int get_next_sub_beat(int pos, int sub_beat_partition) const;
	int get_prev_sub_beat(int pos, int sub_beat_partition) const;
	Range get_sub_beats(int pos, int sub_beat_partition, int num_sub_beats) const;
	Range expand(const Range &r, int sub_beat_partition) const;
	Range range() const;
	Range sub_range(const Range &indices) const;
};



#endif /* SRC_DATA_RHYTHM_BARCOLLECTION_H_ */
