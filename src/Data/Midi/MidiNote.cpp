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
	flags = 0;
	reset_meta();
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume)
{
	range = _range;
	pitch = _pitch;
	volume = _volume;
	flags = 0;
	reset_meta();
}

MidiNote *MidiNote::copy() const
{
	MidiNote *n = new MidiNote;
	*n = *this;
	return n;
}

void MidiNote::reset_meta()
{
	stringno = -1;
	clef_position = -1;
	modifier = NoteModifier::UNKNOWN;
}

void MidiNote::update_meta(const Instrument &instrument, const Scale& scale) const
{
	if ((clef_position < 0) or (modifier == NoteModifier::UNKNOWN)){
		const Clef& clef = instrument.get_clef();
		clef_position = clef.pitch_to_position(pitch, scale, modifier);
	}

	int highest_possible_string = 0;
	for (int i=0; i<instrument.string_pitch.num; i++)
		if (pitch >= instrument.string_pitch[i])
			highest_possible_string = i;

	if (stringno < 0 or stringno > highest_possible_string)
		stringno = highest_possible_string;
}

bool MidiNote::is(int mask) const
{
	return (flags & mask) > 0;
}

void MidiNote::set(int mask)
{
	flags |= mask;
}
