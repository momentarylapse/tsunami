/*
 * Bar.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Bar.h"

BarPattern::BarPattern()
{
	length = 0;
	num_beats = 0;
	num_sub_beats = 0;
	total_sub_beats = 0;
}

BarPattern::BarPattern(int _length, int _num_beats, int _num_sub_beats)
{
	length = _length;
	num_beats = _num_beats;
	num_sub_beats = _num_sub_beats;

	for (int i=0; i<num_beats; i++)
		pattern.add(num_sub_beats);
	update_total();
}

void BarPattern::update_total()
{
	total_sub_beats = 0;
	for (int b: pattern)
		total_sub_beats += b;
}

string BarPattern::pat_str() const
{
	return ia2s(pattern).substr(1, -2).replace(", ", " ");
}

bool BarPattern::is_uniform() const
{
	if (pattern.num == 0)
		return true;
	for (int i=1; i<pattern.num; i++)
		if (pattern[i] != pattern[0])
			return false;
	return true;
}

Bar::Bar(const BarPattern &b) : BarPattern(b)
{
	offset = 0;
	index = -1;
	index_text = -1;
}

Bar::Bar(int _length, int _num_beats, int _num_sub_beats) : Bar(BarPattern(_length, _num_beats, _num_sub_beats))
{}

float Bar::bpm(float sample_rate)
{
	return 60.0f * ((float)total_sub_beats / (float)length) * sample_rate / pattern[0];
}

bool Bar::is_pause()
{
	return (num_beats == 0);
}

Range Bar::range()
{
	return Range(offset, length);
}

