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


namespace tsunami {

const int MidiNote::UndefinedString = -1;
const int MidiNote::UndefinedClef = -123;


MidiNote::MidiNote() : MidiNote(Range::NONE, 0, 0) {
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume) {
	range = _range;
	pitch = _pitch;
	volume = _volume;
	flags = 0;
	stringno = UndefinedString;
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
	clef_position = UndefinedClef;
	modifier = NoteModifier::Unknown;
}

void MidiNote::update_clef_pos(const Clef &clef, const Instrument &instrument, const Scale& scale) const {
	if ((clef_position <= UndefinedClef) or (modifier == NoteModifier::Unknown)) {
		clef_position = clef.pitch_to_position(pitch, scale, modifier);
	}
}

bool MidiNote::is(int mask) const {
	return (flags & mask) > 0;
}

void MidiNote::set(int mask) {
	flags |= mask;
}

}
