/*
 * MidiEvent.cpp
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#include "MidiEvent.h"
#include "MidiNote.h"


namespace tsunami {

MidiEvent::MidiEvent(int _pos, float _pitch, float _volume) {
	pos = _pos;
	pitch = _pitch;
	volume = _volume;
	flags = 0;
	stringno = MidiNote::UndefinedString;
	clef_position = MidiNote::UndefinedClef;
}

MidiEvent::MidiEvent(const MidiNote *n) {
	pos = n->range.offset;
	pitch = n->pitch;
	volume = n->volume;
	flags = n->flags;
	stringno = n->stringno;
	clef_position = n->clef_position;
}

bool MidiEvent::is_note_on() const {
	return volume > 0;
}

bool MidiEvent::is_note_off() const {
	return volume == 0;
}

bool MidiEvent::is_special() const {
	return volume < 0;
}


MidiEvent MidiEvent::with_raw(const bytes& raw) const {
	auto e = *this;
	int size = min(raw.num, 8);
	memcpy(&e.raw, raw.data, size);
	return e;
}

MidiEvent MidiEvent::shifted(int offset) const {
	auto e = *this;
	e.pos += offset;
	return e;
}

MidiEvent MidiEvent::special(const bytes& raw) {
	return MidiEvent(-1, -1, -1).with_raw(raw);
}

}

