/*
 * BarStreamer.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "BarStreamer.h"

#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Rhythm/Bar.h"

BarStreamer::BarStreamer(BarCollection &_bars) {
	module_subtype = "BarStreamer";
	bars = _bars;
	offset = 0;
}

int BarStreamer::read(Array<Beat> &beats, int samples) {
	perf_start();
	beats = bars.get_beats(Range(offset, samples), false, false);
	for (Beat &b: beats)
		if (b.range.offset >= offset)
			b.range.offset -= offset;
	offset += samples;
	perf_end();
	return samples;
}

void BarStreamer::set_pos(int pos) {
	offset = pos;
}

int BarStreamer::get_pos() {
	return offset;
}

void BarStreamer::reset() {
	offset = 0;
}

int BarStreamer::beats_per_bar() {
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0)
		return bars[beats.back().bar_index]->beats.num;
	return 4;
}

int BarStreamer::cur_beat() {
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0)
		return beats.back().beat_no;
	return 0;
}

int BarStreamer::cur_bar() {
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0)
		return beats.back().bar_no;
	return 0;
}

float BarStreamer::beat_fraction() {
	auto beats = bars.get_beats(Range(0, offset), false, false);
	if (beats.num > 0) {
		auto &r = beats.back().range;
		return (float)(offset - r.offset) / (float)r.length;
	}
	return 0;//_beat_fraction;
}
