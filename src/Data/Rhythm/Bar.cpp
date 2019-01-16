/*
 * Bar.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Bar.h"
#include "Beat.h"

// MidiData.cpp
string i2s_small(int i);

BarPattern::BarPattern()
{
	length = 0;
	divisor = 1;
	total_sub_beats = 0;
}

BarPattern::BarPattern(int _length, int _num_beats, int _divisor)
{
	length = _length;
	divisor = _divisor;

	beats.resize(_num_beats);
	for (int i=0; i<beats.num; i++)
		beats[i] = 1;
	update_total();
}

void BarPattern::set_pattern(const Array<int> &p)
{
	beats = p;
	update_total();
}

void BarPattern::update_total()
{
	total_sub_beats = 0;
	for (int b: beats)
		total_sub_beats += b;
}

string BarPattern::pat_str() const
{
	return ia2s(beats).substr(1, -2).replace(" ", "");
}

bool BarPattern::is_uniform() const
{
	if (beats.num == 0)
		return true;
	for (int i=0; i<beats.num; i++)
		if (beats[i] != 1)
			return false;
	return true;
}

bool BarPattern::operator ==(const BarPattern &o) const
{
	if (length != o.length or beats != o.beats or divisor != o.divisor)
		return false;
	return true;
}

bool BarPattern::operator !=(const BarPattern &o) const
{
	return !(*this == o);
}

Bar::Bar(const BarPattern &b) : BarPattern(b)
{
	offset = 0;
	index = -1;
	index_text = -1;
}

Bar::Bar(int _length, int _num_beats, int _num_sub_beats) : Bar(BarPattern(_length, _num_beats, _num_sub_beats))
{}

// well... quarters per minute!!!
float Bar::bpm(float sample_rate)
{
	float quarter_length = (float)length / (float)total_sub_beats * (float)divisor;
	return 60.0f * sample_rate / quarter_length;
}

string Bar::format_beats(bool fancy) const
{
	string div = fancy ? i2s_small(4*divisor) : i2s(4*divisor);
	if (is_uniform())
		return i2s(beats.num) + "/" + div;
	else
		return "(" + pat_str() + ")/" + div;
}

Array<Beat> Bar::get_beats(int offset, bool include_sub_beats, int sub_beat_partition) const
{
	sub_beat_partition = max(sub_beat_partition, 1);
	Array<Beat> _beats;
	int sub_beat_length = length / (total_sub_beats * sub_beat_partition);
	int level = 0;
	int pos_beat = offset;
	foreachi (int bb, beats, beat_index){
		int sub_beats = bb * sub_beat_partition;
		int beat_length = sub_beat_length * sub_beats;

		if (include_sub_beats){

			for (int k=0; k<sub_beats; k++){
				int pos_sub_beat = pos_beat + k * sub_beat_length;
				// sub beat
				//if (r.is_inside(pos_sub_beat))
					_beats.add(Beat(Range(pos_sub_beat, sub_beat_length), beat_index, level, index, index_text));
				level = 2;
			}

		}else{

			//if (r.is_inside(pos_beat))
				_beats.add(Beat(Range(pos_beat, beat_length), beat_index, level, index, index_text));
		}
		pos_beat += beat_length;
		level = 1;
	}
	return _beats;
}

bool Bar::is_pause()
{
	return (beats.num == 0);
}

Range Bar::range()
{
	return Range(offset, length);
}

