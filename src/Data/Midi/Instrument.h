/*
 * Instrument.h
 *
 *  Created on: 06.12.2015
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_INSTRUMENT_H_
#define SRC_DATA_MIDI_INSTRUMENT_H_

#include "../../lib/base/base.h"

class Clef;

class Instrument
{
public:
	enum class Type{
		NONE,
		PIANO,
		ORGAN,
		HAPSICHORD,
		KEYBOARD,
		GUITAR,
		ELECTRIC_GUITAR,
		ELECTRIC_BASS,
		DRUMS,
		VOCALS,
		VIOLIN,
		CELLO,
		DOUBLE_BASS,
		FLUTE,
		TRUMPET,
		TROMBONE,
		TUBA,
		HORN,
		SAXOPHONE,
		CLARINET,
		NUM_TYPES,
	};

	Instrument();
	Instrument(Type type);

	Type type;
	Array<int> string_pitch;

	Array<int> default_tuning() const;
	string name() const;
	int midi_no() const;
	bool has_default_tuning() const;

	bool operator==(const Instrument &i) const;

	void set_midi_no(int no);

	const Clef& get_clef() const;
	bool has_strings() const;

	static Array<Instrument> enumerate();

	int highest_usable_string(int pitch) const;
	int make_string_valid(int pitch, int string) const;
};

#endif /* SRC_DATA_MIDI_INSTRUMENT_H_ */
