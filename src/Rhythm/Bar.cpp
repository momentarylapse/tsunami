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
}

BarPattern::BarPattern(int _length, int _num_beats, int _num_sub_beats)
{
	length = _length;
	num_beats = _num_beats;
	num_sub_beats = _num_sub_beats;
}

Bar::Bar(int _length, int _num_beats, int _num_sub_beats)
{
	length = _length;
	num_beats = _num_beats;
	num_sub_beats = _num_sub_beats;
	offset = 0;
	index = -1;
	index_text = -1;
}

float Bar::bpm(float sample_rate)
{
	return 60.0f / (float)length * (float)num_beats * sample_rate;
}

bool Bar::is_pause()
{
	return (num_beats == 0);
}

Range Bar::range()
{
	return Range(offset, length);
}

