/*
 * Instrument.h
 *
 *  Created on: 06.12.2015
 *      Author: michi
 */

#ifndef SRC_DATA_INSTRUMENT_H_
#define SRC_DATA_INSTRUMENT_H_

class Instrument
{
public:
	Instrument();
	Instrument(int type);

	enum
	{
		TYPE_NONE,
		TYPE_PIANO,
		TYPE_ORGAN,
		TYPE_HAPSICHORD,
		TYPE_KEYBOARD,
		TYPE_GUITAR,
		TYPE_ELECTRIC_GUITAR,
		TYPE_ELECTRIC_BASS,
		TYPE_DRUMS,
		TYPE_VOCALS,
		TYPE_VIOLIN,
		TYPE_CELLO,
		TYPE_DOUBLE_BASS,
		TYPE_FLUTE,
		TYPE_TRUMPET,
		TYPE_TROMBONE,
		TYPE_TUBA,
		TYPE_HORN,
		TYPE_SAXOPHONE,
		TYPE_CLARINET,
		NUM_TYPES,
	};

	int type;

	Array<int> default_tuning() const;
	string name() const;
	int midi_no() const;
	bool is_default_tuning(const Array<int> &tuning) const;

	bool operator==(const Instrument &i) const;

	void set_midi_no(int no);

	int get_clef();

	static Array<Instrument> enumerate();
};

#endif /* SRC_DATA_INSTRUMENT_H_ */
