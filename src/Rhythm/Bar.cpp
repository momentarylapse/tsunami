/*
 * Bar.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Bar.h"


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

