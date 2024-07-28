/*
 * Clef.h
 *
 *  Created on: 11.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_CLEF_H_
#define SRC_DATA_MIDI_CLEF_H_

#include "../../lib/base/base.h"


namespace tsunami {

class Scale;
enum class NoteModifier;

enum class ClefType {
	Treble,
	Treble8,
	Bass,
	Bass8,
	Drums,
	Count
};

class Clef {
public:
	Clef(ClefType type, const string &symbol, int offset);

	static const Clef Treble;
	static const Clef Treble8;
	static const Clef Bass;
	static const Clef Bass8;
	static const Clef Drums;

	ClefType type;
	int offset;
	string symbol;

	int pitch_to_position(int pitch, const Scale &s, NoteModifier &modifier) const;
	int position_to_pitch(int position, const Scale &s, NoteModifier modifier) const;

	int position_to_uniclef(int pos) const;
};

}

#endif /* SRC_DATA_MIDI_CLEF_H_ */
