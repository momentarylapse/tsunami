/*
 * MidiPattern.cpp
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#include "MidiPattern.h"
#include "MidiData.h"

MidiPattern::MidiPattern()
{
	num_beats = 4;
	beat_partition = 4;
	volume_randomness = 0;
	time_randomness = 0;
	ref_count = 0;
	owner = NULL;
}

MidiPattern::~MidiPattern()
{
}

