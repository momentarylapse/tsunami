/*
 * Bar.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Bar.h"


Bar::Bar(const Range &r, int _num_beats, int _index, int _index_text)
{
	range = r;
	num_beats = _num_beats;
	index = _index;
	index_text = _index_text;
}

float Bar::bpm(float sample_rate)
{
	return 60.0f / (float)range.length * (float)num_beats * sample_rate;
}

