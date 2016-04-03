/*
 * Rhythm.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "Rhythm.h"

Beat::Beat(const Range &r, int bar, int beat)
{
	range = r;
	bar_no = bar;
	beat_no = beat;
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

Array<Beat> BarCollection::getBeats(const Range &r)
{
	Array<Beat> beats;

	int pos0 = 0;
	int bar_no = 0;
	foreach(BarPattern &b, *this)
		if (b.type == b.TYPE_BAR){
			int beat_length = b.length / b.num_beats;
			for (int i=0;i<b.num_beats;i++){
				int pos = pos0 + i * beat_length;
				if (r.is_inside(pos))
					beats.add(Beat(Range(pos, beat_length), bar_no, i));
			}
			pos0 += b.length;
			bar_no ++;
		}else if (b.type == b.TYPE_PAUSE){
			pos0 += b.length;
		}
	return beats;
}

Array<Bar> BarCollection::getBars(const Range &r)
{
	Array<Bar> bars;

	int pos0 = 0;
	int bar_no = 0;
	foreach(BarPattern &b, *this)
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
	foreach(BarPattern &b, *this){
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
	foreach(BarPattern &b, *this)
		pos0 += b.length;
	return Range(0, pos0);
}





