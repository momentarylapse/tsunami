/*
 * MidiEvent.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDIEVENT_H_
#define SRC_DATA_MIDI_MIDIEVENT_H_


class MidiNote;

class MidiEvent
{
public:
	MidiEvent(){}
	MidiEvent(int pos, float pitch, float volume);
	MidiEvent(const MidiNote *n);
	int pos;
	float pitch;
	float volume;
	int flags;
	mutable int stringno, clef_position;
};



#endif /* SRC_DATA_MIDI_MIDIEVENT_H_ */
