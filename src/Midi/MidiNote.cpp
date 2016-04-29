/*
 * MidiNote.cpp
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#include "MidiNote.h"
#include "MidiData.h"
#include "Clef.h"
#include "Instrument.h"


MidiNote::MidiNote()
{
	range = Range::EMPTY;
	pitch = 0;
	volume = 0;
	stringno = -1;
	clef_position = -1;
	modifier = MODIFIER_NONE;
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume)
{
	range = _range;
	pitch = _pitch;
	volume = _volume;
	stringno = -1;
	clef_position = -1;
	modifier = MODIFIER_NONE;
}

float MidiNote::getFrequency()
{
	return pitch_to_freq(pitch);//440.0f * pow(2.0f, (float)(pitch - 69) / 12.0f);
}


void MidiNote::update_meta(const Instrument &instrument, const Scale& scale) const
{
	if (clef_position < 0){
		const Clef& clef = instrument.get_clef();
		clef_position = clef.pitch_to_position(pitch, scale, modifier);
	}

	if (stringno < 0){
		stringno = 0;
		for (int i=0; i<instrument.tuning.num; i++)
			if (pitch >= instrument.tuning[i]){
				stringno = i;
			}
	}
}
