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

}

