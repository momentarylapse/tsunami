/*
 * MidiNote.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDINOTE_H_
#define SRC_DATA_MIDI_MIDINOTE_H_

#include "../Range.h"

class Scale;
class Instrument;
enum class NoteModifier;

enum{
	NOTE_FLAG_TRILL = 1<<0,
	NOTE_FLAG_LEGATO = 1<<1,
	NOTE_FLAG_STACCATO = 1<<2,
	NOTE_FLAG_TENUTO = 1<<3,
	NOTE_FLAG_HAMMER_ON = 1<<4,
	NOTE_FLAG_PULL_OFF = 1<<5,
	NOTE_FLAG_DEAD = 1<<6,
};

class MidiNote
{
public:
	MidiNote();
	MidiNote(const Range &range, float pitch, float volume);
	MidiNote *copy() const;
	Range range;
	float pitch;
	float volume;
	int flags;
	mutable int stringno, clef_position;
	mutable NoteModifier modifier;
	mutable int y;

	bool is(int mask) const;
	void set(int mask);

	void reset_meta();
	void update_meta(const Instrument &i, const Scale &s) const;
};


#endif /* SRC_DATA_MIDI_MIDINOTE_H_ */
