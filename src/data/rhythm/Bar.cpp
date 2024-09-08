/*
 * Bar.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Bar.h"
#include "Beat.h"
#include "../../lib/base/iter.h"


namespace tsunami {

// MidiData.cpp
string i2s_small(int i);


BarPattern::BarPattern() {
	length = 0;
	divisor = 1;
	total_sub_beats = 0;
}

BarPattern::BarPattern(int _length, int _num_beats, int _divisor) {
	length = _length;
	divisor = _divisor;

	beats.resize(_num_beats);
	for (int i=0; i<beats.num; i++)
		beats[i] = 1;
	update_total();
}

void BarPattern::set_pattern(const Array<int> &p) {
	beats = p;
	update_total();
}

void BarPattern::update_total() {
	total_sub_beats = 0;
	for (int b: beats)
		total_sub_beats += b;
}

string BarPattern::pat_str() const {
	return str(beats).sub(1, -1).replace(" ", "");
}

bool BarPattern::is_uniform() const {
	if (beats.num == 0)
		return true;
	for (int i=0; i<beats.num; i++)
		if (beats[i] != 1)
			return false;
	return true;
}

// well... quarters per minute!!!
float BarPattern::bpm(float sample_rate) {
	float quarter_length = (float)length / (float)total_sub_beats * (float)divisor;
	return 60.0f * sample_rate / quarter_length;
}

// well... quarters per minute!!!
void BarPattern::set_bpm(float bpm, float sample_rate) {
	float quarter_length = 60.0f * sample_rate / bpm;
	length = (int)(quarter_length * (float)total_sub_beats / (float)divisor);
}

bool BarPattern::operator ==(const BarPattern &o) const {
	if (length != o.length or beats != o.beats or divisor != o.divisor)
		return false;
	return true;
}

bool BarPattern::operator !=(const BarPattern &o) const {
	return !(*this == o);
}

Bar::Bar(const BarPattern &b) /*: Sharable<BarPattern>(b)*/ {
	*(BarPattern*)this = b;
	offset = 0;
	index = -1;
	index_text = -1;
}

Bar::Bar(int _length, int _num_beats, int _num_sub_beats) : Bar(BarPattern(_length, _num_beats, _num_sub_beats))
{}

void Bar::__init__(int _length, int _num_beats, int _num_sub_beats) {
	new(this) Bar(_length, _num_beats, _num_sub_beats);
}

const BarPattern &Bar::pattern() const {
	return *(const BarPattern*)this;
}

Bar *Bar::copy() const {
	return new Bar(pattern());
}

string Bar::format_beats(bool fancy) const {
	string div = fancy ? i2s_small(4*divisor) : i2s(4*divisor);
	if (is_uniform())
		return i2s(beats.num) + "/" + div;
	else
		return pat_str() + "/" + div;
}

Array<Beat> Bar::get_beats(int offset, base::optional<int> sub_beat_partition) const {
	Array<Beat> _beats;
	int sub_beat_length = length / (total_sub_beats * sub_beat_partition.value_or(1));
	int level = 0;
	int pos_beat = offset;
	for (auto&& [beat_index, beat]: enumerate(beats)) {
		int sub_beats = beat * sub_beat_partition.value_or(1);
		int beat_length = sub_beat_length * sub_beats;

		if (sub_beat_partition.has_value()) {

			for (int k=0; k<sub_beats; k++) {
				int pos_sub_beat = pos_beat + k * sub_beat_length;
				// sub beat
				//if (r.is_inside(pos_sub_beat))
					_beats.add(Beat(Range(pos_sub_beat, sub_beat_length), beat_index, level, index, index_text));
				level = 2;
			}

		} else {

			//if (r.is_inside(pos_beat))
				_beats.add(Beat(Range(pos_beat, beat_length), beat_index, level, index, index_text));
		}
		pos_beat += beat_length;
		level = 1;
	}
	return _beats;
}

bool Bar::is_pause() const {
	return (beats.num == 0);
}

Range Bar::range() const {
	return Range(offset, length);
}

}

