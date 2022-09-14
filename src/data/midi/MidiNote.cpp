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

const int MidiNote::UNDEFINED_STRING = -1;
const int MidiNote::UNDEFINED_CLEF = -123;


MidiNote::MidiNote() : MidiNote(Range::NONE, 0, 0) {
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume) {
	range = _range;
	pitch = _pitch;
	volume = _volume;
	flags = 0;
	stringno = UNDEFINED_STRING;
	reset_clef();
}

MidiNote *MidiNote::copy(int offset) const {
	MidiNote *n = new MidiNote(range + offset, pitch, volume);
	n->flags = flags;
	n->stringno = stringno;

	n->clef_position = clef_position;
	n->modifier = modifier;
	n->y = y;
	return n;
}

void MidiNote::reset_clef() {
	clef_position = UNDEFINED_CLEF;
	modifier = NoteModifier::UNKNOWN;
}

void MidiNote::update_clef_pos(const Clef &clef, const Instrument &instrument, const Scale& scale) const {
	if ((clef_position <= UNDEFINED_CLEF) or (modifier == NoteModifier::UNKNOWN)) {
		clef_position = clef.pitch_to_position(pitch, scale, modifier);
	}
}

bool MidiNote::is(int mask) const {
	return (flags & mask) > 0;
}

void MidiNote::set(int mask) {
	flags |= mask;
}
