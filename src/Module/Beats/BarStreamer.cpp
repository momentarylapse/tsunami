/*
 * BarStreamer.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "BarStreamer.h"

#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Rhythm/Bar.h"

BarStreamer::BarStreamer(BarCollection &_bars)
{
	bars = _bars;
	offset = 0;
}

int BarStreamer::read(Array<Beat> &beats, int samples)
{
	beats = bars.get_beats(Range(offset, samples), false, false);
	for (Beat &b: beats)
		if (b.range.offset >= offset){
			b.range.offset -= offset;
		}
	offset += samples;
	return samples;
}

void BarStreamer::seek(int pos)
{
	offset = pos;
}

void BarStreamer::reset()
{
	offset = 0;
}

int BarStreamer::beats_per_bar()
{
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0)
		return bars[beats.back().bar_index]->num_beats;
	return 4;
}

int BarStreamer::cur_beat()
{
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0)
		return beats.back().beat_no;
	return 0;
}

float BarStreamer::beat_fraction()
{
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0){
		auto &r = beats.back().range;
		return (float)(offset - r.offset) / (float)r.length;
	}
	return 0;//_beat_fraction;
}
