/*
 * BarCollection.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */


#include "BarCollection.h"

#include "../Range.h"
#include "Bar.h"
#include "Beat.h"


namespace tsunami {


// pos is precise... beat length not...
Array<Beat> BarCollection::get_beats(const Range &r, bool include_hidden, const base::optional<int>& sub_beat_partition) const {
	Array<Beat> beats;

	int pos_bar = 0;
	int bar_index = 0;
	[[maybe_unused]] int bar_no = 0;

	for (Bar *b: weak(*this)) {
		if (b->is_pause()) {
			if (include_hidden)
				beats.add(Beat(Range(pos_bar, b->length), 0, 0, bar_index, -1));
			pos_bar += b->length;
		} else {
			auto _beats = b->get_beats(pos_bar, sub_beat_partition);
			for (Beat &bb: _beats)
				if (r.overlaps(bb.range))
					beats.add(bb);
			pos_bar += b->length;
			bar_no ++;
		}
		bar_index ++;
	}
	if (include_hidden and (this->num > 0))
		beats.add(Beat(Range(pos_bar, 0), 0, 0, bar_index, -1));
	return beats;
}

Array<Bar*> BarCollection::get_bars(const Range &r) const {
	Array<Bar*> bars;

	int pos0 = 0;
	int index = 0;
	int bar_no_text = 0;
	for (Bar *b: weak(*this)) {
		Range rr = Range(pos0, b->length);
		b->index = index;
		b->index_text = bar_no_text;
		b->offset = pos0;
		if (rr.overlaps(r))
			bars.add(b);
		pos0 += b->length;
		if (!b->is_pause())
			bar_no_text ++;
		index ++;
	}
	return bars;
}

int BarCollection::get_bar_no(int pos) const {
	if (pos < 0)
		return -1;
	int pos0 = 0;
	for (int i=0; i<num; i++) {
		pos0 += (weak(*this)[i])->length;
		if (pos < pos0)
			return i;
	}
	return -1;
}

int BarCollection::get_next_beat(int pos) const {
	auto beats = get_beats(Range::ALL, true);
	for (Beat &b: beats)
		if (b.range.offset > pos)
			return b.range.offset;
	return 0;
}

int BarCollection::get_prev_beat(int pos) const {
	auto beats = get_beats(Range::ALL, true);
	int prev = 0;
	for (Beat &b: beats) {
		if (b.range.offset >= pos)
			return prev;
		prev = b.range.offset;
	}
	return 0;
}

int BarCollection::get_next_sub_beat(int pos, int sub_beat_partition) const {
	auto beats = get_beats(Range::ALL, true, sub_beat_partition);
	for (Beat &b: beats)
		if (b.range.offset > pos)
			return b.range.offset;
	return pos;
}

int BarCollection::get_prev_sub_beat(int pos, int sub_beat_partition) const {
	auto beats = get_beats(Range::ALL, true, sub_beat_partition);
	int prev = pos;
	for (Beat &b: beats) {
		if (b.range.offset >= pos)
			return prev;
		prev = b.range.offset;
	}
	return pos;
}

Range BarCollection::get_sub_beats(int pos, int sub_beat_partition, int num_sub_beats) const {
	int a = get_prev_sub_beat(pos+1, sub_beat_partition);
	int b = get_next_sub_beat(pos-1, sub_beat_partition);
	if (num_sub_beats > 0) {
		// >>> forward
		if (a == b) // on a sub beat
			b = get_next_sub_beat(b, sub_beat_partition);
		for (int i=1; i<num_sub_beats; i++)
			b = get_next_sub_beat(b, sub_beat_partition);
	} else {
		// <<< backward
		if (a == b) // on a sub beat
			a = get_prev_sub_beat(a, sub_beat_partition);
		for (int i=1; i<-num_sub_beats; i++)
			a = get_prev_sub_beat(a, sub_beat_partition);
	}
	if (b > a and a >= 0)
		return Range::to(a, b);

	// out of bars...
	return Range::to(pos, pos);
}



Range BarCollection::expand(const Range &r, int beat_partition) const {
	Range o = r;
	o.set_start(get_prev_sub_beat(r.start(), beat_partition));
	o.set_end(get_next_sub_beat(r.end(), beat_partition));
	return r;
}

Range BarCollection::range() const {
	int pos0 = 0;
	for (Bar *b: weak(*this))
		pos0 += b->length;
	return Range(0, pos0);
}

void BarCollection::_update_offsets() {
	int pos = 0;
	for (Bar *b: weak(*this)) {
		b->offset = pos;
		pos += b->length;
	}
}

}

