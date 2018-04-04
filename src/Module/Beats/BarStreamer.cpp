/*
 * BarStreamer.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "BarStreamer.h"

#include "../../Data/Rhythm/Beat.h"

BarStreamer::BarStreamer(BarCollection &_bars)
{
	bars = _bars;
	offset = 0;
}

int BarStreamer::read(Array<Beat> &beats, int samples)
{
	beats = bars.getBeats(Range(offset, samples), false, false);
	for (Beat &b: beats)
		if (b.range.offset >= offset)
			b.range.offset -= offset;
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
