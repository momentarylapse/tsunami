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

// pos is precise... beat length not...
Array<Beat> get_beats(BarCollection &collection, const Range &r, bool include_hidden, bool include_sub_beats, int overwrite_sub_beats = -1)
{
	Array<Beat> beats;

	int pos_bar = 0;
	int bar_no = 0;

	for (BarPattern &b: collection)
		if (b.type == b.TYPE_BAR){
			// bar
			int beat_length = b.length / b.num_beats;
			int level = 0;
			for (int i=0; i<b.num_beats; i++){
				// beat
				int pos_beat = pos_bar + i * beat_length;

				if (include_sub_beats){

					int sub_beats = (overwrite_sub_beats > 0) ? overwrite_sub_beats : b.sub_beats;
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
			pos_bar += b.length;
			bar_no ++;
		}else if (b.type == b.TYPE_PAUSE){
			if (include_hidden)
				beats.add(Beat(Range(pos_bar, b.length), -1, 0, -1));
			pos_bar += b.length;
		}
	if (include_hidden and (collection.num > 0))
		beats.add(Beat(Range(pos_bar, 0), -1, 0, -1));
	return beats;
}

Array<Beat> BarCollection::getBeats(const Range &r, bool include_hidden, bool include_sub_beats)
{
	return get_beats(*this, r, include_hidden, include_sub_beats, -1);
}

Array<Bar> BarCollection::getBars(const Range &r)
{
	Array<Bar> bars;

	int pos0 = 0;
	int bar_no = 0;
	for (BarPattern &b: *this)
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

Range BarCollection::getRange()
{
	int pos0 = 0;
	for (BarPattern &b: *this)
		pos0 += b.length;
	return Range(0, pos0);
}



RhythmHelper::RhythmHelper(BarCollection *_bars)
{
	bars = _bars;
	offset = 0;
}

Array<Beat> RhythmHelper::getBeats(const Range &r, bool include_hidden, bool include_sub_beats)
{
	return bars->getBeats(r, include_hidden, include_sub_beats);
}

int RhythmHelper::getNextBeat()
{
	return bars->getNextBeat(offset);
}

int RhythmHelper::getPrevBeat()
{
	return bars->getPrevBeat(offset);
}

int RhythmHelper::getNextSubBeat(int beat_partition)
{
	return bars->getNextSubBeat(offset, beat_partition);
}

int RhythmHelper::getPrevSubBeat(int beat_partition)
{
	return bars->getPrevSubBeat(offset, beat_partition);
}

Range RhythmHelper::expand(const Range &r, int beat_partition)
{
	Range o = r;
	offset = r.start();
	o.set_start(getPrevSubBeat(beat_partition));
	offset = r.end();
	o.set_end(getNextSubBeat(beat_partition));
	return r;
}

void RhythmHelper::set_pos(int pos)
{
	offset = pos;
}

void RhythmHelper::consume(int samples)
{
	offset += samples;
}

void RhythmHelper::reset()
{
	offset = 0;
}




DummyBeatSource* BeatSource::dummy = new DummyBeatSource;

BarStreamer::BarStreamer(BarCollection &_bars)
{
	bars = _bars;
	offset = 0;
}

int BarStreamer::read(Array<Beat> &beats, int samples)
{
	beats = bars.getBeats(Range(offset, samples), false, false);
	return samples;
}

void BarStreamer::seek(int pos)
{
	offset = pos;
}


