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


MidiNote::MidiNote() {
	range = Range::EMPTY;
	pitch = 0;
	volume = 0;
	flags = 0;
	stringno = -1;
	reset_clef();
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume) {
	range = _range;
	pitch = _pitch;
	volume = _volume;
	flags = 0;
	stringno = -1;
	reset_clef();
}

MidiNote *MidiNote::copy() const {
	MidiNote *n = new MidiNote(range, pitch, volume);
	n->flags = flags;
	n->stringno = stringno;

	n->clef_position = clef_position;
	n->modifier = modifier;
	n->y = y;
	return n;
}

void MidiNote::reset_clef() {
	clef_position = -1;
	modifier = NoteModifier::UNKNOWN;
}

void MidiNote::update_clef_pos(const Clef &clef, const Instrument &instrument, const Scale& scale) const {
	if ((clef_position < 0) or (modifier == NoteModifier::UNKNOWN)) {
		clef_position = clef.pitch_to_position(pitch, scale, modifier);
	}
}

bool MidiNote::is(int mask) const {
	return (flags & mask) > 0;
}

void MidiNote::set(int mask) {
	flags |= mask;
}
