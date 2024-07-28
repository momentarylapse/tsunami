/*
 * MidiNote.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDINOTE_H_
#define SRC_DATA_MIDI_MIDINOTE_H_

#include "../../lib/base/pointer.h"
#include "../Range.h"


namespace tsunami {

class Scale;
class Clef;
class Instrument;
enum class NoteModifier;

namespace NoteFlag {
	enum {
		Trill = 1<<0,
		Legato = 1<<1,
		Staccato = 1<<2,
		Tenuto = 1<<3,
		HammerOn = 1<<4,
		PullOff = 1<<5,
		Dead = 1<<6,
		BendHalf = 1<<7,
		BendFull = 1<<8,
	};
}


class MidiNote : public Sharable<base::Empty> {
public:
	MidiNote();
	MidiNote(const Range &range, float pitch, float volume);
	MidiNote *copy(int offset=0) const;
	Range range;
	float pitch;
	float volume;
	int flags;
	int stringno;

	// temporary meta data
	mutable int clef_position;
	mutable NoteModifier modifier;
	mutable int y;

	bool is(int mask) const;
	void set(int mask);

	void reset_clef();
	void update_clef_pos(const Clef &clef, const Instrument &instrument, const Scale &s) const;


	static const int UndefinedClef;
	static const int UndefinedString;
};

}


#endif /* SRC_DATA_MIDI_MIDINOTE_H_ */
