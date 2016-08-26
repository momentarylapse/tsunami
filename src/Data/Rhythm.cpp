/*
 * Rhythm.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "Rhythm.h"

Beat::Beat(const Range &r, int _bar_no, int _beat_no, int _level)
{
	range = r;
	bar_no = _bar_no;
	beat_no = _beat_no;
	level = _level;
};

Range Beat::sub(int index, int parts)
{
	int length = range.length / parts;
	int start = range.offset + index * length;
	if (index == parts - 1)
		return Range(start, range.length - start);
	return Range(start, length);
}

Bar::Bar(const Range &r, int _num_beats, int _index)
{
	range = r;
	num_beats = _num_beats;
	index = _index;
}

float Bar::bpm(float sample_rate)
{
	return 60.0f / (float)range.length * (float)num_beats * sample_rate;
}

Array<Beat> BarCollection::getBeats(const Range &r, bool include_hidden, bool include_sub_beats)
{
	Array<Beat> beats;

	int pos_bar = 0;
	int bar_no = 0;
	for (BarPattern &b : *this)
		if (b.type == b.TYPE_BAR){
			int beat_length = b.length / b.num_beats;
			int sub_beat_length = beat_length / b.sub_beats;
			for (int i=0;i<b.num_beats;i++){
				int pos_beat = pos_bar + i * beat_length;
				if (r.is_inside(pos_beat))
					beats.add(Beat(Range(pos_beat, beat_length), bar_no, i, (i == 0) ? 0 : 1));

				if (!include_sub_beats)
					continue;

				for (int k=1;k<b.sub_beats;k++){
					int pos_sub_beat = pos_beat + k * sub_beat_length;
					if (r.is_inside(pos_sub_beat))
						beats.add(Beat(Range(pos_sub_beat, sub_beat_length), bar_no, i, 2));
				}
			}
			pos_bar += b.length;
			bar_no ++;
		}else if (b.type == b.TYPE_PAUSE){
			if (include_hidden)
				beats.add(Beat(Range(pos_bar, b.length), -1, 0, -1));
			pos_bar += b.length;
		}
	if (include_hidden and (num > 0))
		beats.add(Beat(Range(pos_bar, 0), -1, 0, -1));
	return beats;
}

Array<Bar> BarCollection::getBars(const Range &r)
{
	Array<Bar> bars;

	int pos0 = 0;
	int bar_no = 0;
	for (BarPattern &b : *this)
		if (b.type == b.TYPE_BAR){
			Range rr = Range(pos0, b.length);
			if (rr.overlaps(r))
				bars.add(Bar(rr, b.num_beats, bar_no));
			pos0 += b.length;
			bar_no ++;
		}else if (b.type == b.TYPE_PAUSE){
			pos0 += b.length;
		}
	return bars;
}

int BarCollection::getNextBeat(int pos)
{
	int p0 = 0;
	if (p0 > pos)
		return p0;
	for (BarPattern &b : *this){
		if (b.type == b.TYPE_BAR){
			int pp = p0;
			for (int j=0;j<b.num_beats;j++){
				pp += b.length / b.num_beats;
				if (pp > pos)
					return pp;
			}
			p0 += b.length;
		}else if (b.type == b.TYPE_PAUSE){
			p0 += b.length;
			if (p0 > pos)
				return p0;
		}
	}
	return pos;
}

Range BarCollection::getRange()
{
	int pos0 = 0;
	for (BarPattern &b : *this)
		pos0 += b.length;
	return Range(0, pos0);
}





