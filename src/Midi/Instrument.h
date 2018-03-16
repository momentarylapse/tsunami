/*
 * Instrument.h
 *
 *  Created on: 06.12.2015
 *      Author: michi
 */

#ifndef SRC_DATA_INSTRUMENT_H_
#define SRC_DATA_INSTRUMENT_H_

class Clef;

class Instrument
{
public:
	Instrument();
	Instrument(int type);

	enum Type{
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

	int type;
	Array<int> string_pitch;

	Array<int> default_tuning() const;
	string name() const;
	int midi_no() const;
	bool has_default_tuning() const;

	bool operator==(const Instrument &i) const;

	void set_midi_no(int no);

	const Clef& get_clef() const;

	static Array<Instrument> enumerate();
};

#endif /* SRC_DATA_INSTRUMENT_H_ */
