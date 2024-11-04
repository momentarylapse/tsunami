/*
 * MidiEvent.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDIEVENT_H_
#define SRC_DATA_MIDI_MIDIEVENT_H_

#include "../../lib/base/base.h"

namespace tsunami {

class MidiNote;

class MidiEvent {
public:
	MidiEvent(){}
	MidiEvent(int pos, float pitch, float volume);
	explicit MidiEvent(const MidiNote *n);
	int pos;
	float pitch;
	float volume;
	int flags;
	mutable int stringno, clef_position;
	int64 raw = 0;

	bool is_note_on() const;
	bool is_note_off() const;
	bool is_special() const;
	MidiEvent with_raw(const bytes& raw) const;
	MidiEvent shifted(int offset) const;
	static MidiEvent special(const bytes& raw);
};

}


#endif /* SRC_DATA_MIDI_MIDIEVENT_H_ */
