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

class Clef {
public:
	Clef(int type, const string &symbol, int offset);
	enum Type{
		TREBLE,
		TREBLE_8,
		BASS,
		BASS_8,
		DRUMS,
		NUM
	};

	static const Clef _TREBLE;
	static const Clef _TREBLE_8;
	static const Clef _BASS;
	static const Clef _BASS_8;
	static const Clef _DRUMS;

	int type;
	int offset;
	string symbol;

	int pitch_to_position(int pitch, const Scale &s, NoteModifier &modifier) const;
	int position_to_pitch(int position, const Scale &s, NoteModifier modifier) const;

	int position_to_uniclef(int pos) const;
};

}

#endif /* SRC_DATA_MIDI_CLEF_H_ */
