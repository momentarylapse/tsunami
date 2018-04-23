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


// pos is precise... beat length not...
Array<Beat> get_beats(BarCollection &collection, const Range &r, bool include_hidden, bool include_sub_beats, int overwrite_sub_beats = -1)
{
	Array<Beat> beats;

	int pos_bar = 0;
	int bar_no = 0;

	for (Bar *b: collection)
		if (!b->is_pause()){
			// bar
			int beat_length = b->length / b->num_beats;
			int level = 0;
			for (int i=0; i<b->num_beats; i++){
				// beat
				int pos_beat = pos_bar + i * beat_length;

				if (include_sub_beats){

					int sub_beats = (overwrite_sub_beats > 0) ? overwrite_sub_beats : b->num_sub_beats;
					int sub_beat_length = beat_length / sub_beats;

					for (int k=0; k<sub_beats; k++){
						// sub beat
						int pos_sub_beat = pos_beat + k * sub_beat_length;
						if (r.is_inside(pos_sub_beat))
							beats.add(Beat(Range(pos_sub_beat, sub_beat_length), bar_no, i, level));
						level = 2;
					}

				}else{

					if (r.is_inside(pos_beat))
						beats.add(Beat(Range(pos_beat, beat_length), bar_no, i, level));
				}
				level = 1;
			}
			pos_bar += b->length;
			bar_no ++;
		}else{
			if (include_hidden)
				beats.add(Beat(Range(pos_bar, b->length), -1, 0, 0));
			pos_bar += b->length;
		}
	if (include_hidden and (collection.num > 0))
		beats.add(Beat(Range(pos_bar, 0), -1, 0, 0));
	return beats;
}

Array<Beat> BarCollection::getBeats(const Range &r, bool include_hidden, bool include_sub_beats)
{
	return get_beats(*this, r, include_hidden, include_sub_beats, -1);
}

Array<Bar*> BarCollection::getBars(const Range &r)
{
	Array<Bar*> bars;

	int pos0 = 0;
	int index = 0;
	int bar_no_text = 0;
	for (Bar *b: *this){
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

int BarCollection::getNextBeat(int pos)
{
	Array<Beat> beats = get_beats(*this, Range::ALL, true, false);
	for (Beat &b: beats)
		if (b.range.offset > pos)
			return b.range.offset;
	return 0;
}

int BarCollection::getPrevBeat(int pos)
{
	Array<Beat> beats = get_beats(*this, Range::ALL, true, false);
	int prev = 0;
	for (Beat &b: beats){
		if (b.range.offset >= pos)
			return prev;
		prev = b.range.offset;
	}
	return 0;
}

int BarCollection::getNextSubBeat(int pos, int beat_partition)
{
	Array<Beat> beats = get_beats(*this, Range::ALL, true, true, beat_partition);
	for (Beat &b: beats)
		if (b.range.offset > pos)
			return b.range.offset;
	return 0;
}

int BarCollection::getPrevSubBeat(int pos, int beat_partition)
{
	Array<Beat> beats = get_beats(*this, Range::ALL, true, true, beat_partition);
	int prev = 0;
	for (Beat &b: beats){
		if (b.range.offset >= pos)
			return prev;
		prev = b.range.offset;
	}
	return 0;
}



Range BarCollection::expand(const Range &r, int beat_partition)
{
	Range o = r;
	o.set_start(getPrevSubBeat(r.start(), beat_partition));
	o.set_end(getNextSubBeat(r.end(), beat_partition));
	return r;
}

Range BarCollection::range()
{
	int pos0 = 0;
	for (Bar *b: *this)
		pos0 += b->length;
	return Range(0, pos0);
}

Range BarCollection::sub_range(const Range &indices)
{
	int offset = 0;
	for (int i=0; i<indices.offset; i++)
		offset += (*this)[i]->length;
	int pos0 = offset;
	for (int i=indices.offset; i<indices.end(); i++)
		pos0 += (*this)[i]->length;
	return Range(offset, pos0 - offset);
}

