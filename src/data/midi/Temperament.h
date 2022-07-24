/*
 * Temperament.h
 *
 *  Created on: 26 Mar 2022
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_TEMPERAMENT_H_
#define SRC_DATA_MIDI_TEMPERAMENT_H_

#include "MidiData.h"

enum class TemperamentType {
	CUSTOM,
	EQUAL_12,
	MEANTONE_QUARTER_COMMA,
	MEANTONE_THIRD_COMMA,
	PYTHAGOREAN,
	FIVE_LIMIT_DIATONIC_MAJOR,
	NUM
};

class Temperament {
public:
	float freq[MAX_PITCH];

	bool operator==(const Temperament &t) const;
	bool is_default() const;
	bool has_equal_octaves() const;

	static Temperament create_default();
	static Temperament create(TemperamentType type, int ref_pitch, float ref_freq);

	bool guess_parameters(TemperamentType &type, int &ref_pitch, float &ref_freq) const;

private:
	static Temperament create_from_factor(int pitch_ref, float freq_ref, float factor[12]);
	static Temperament create_meantone(int pitch_ref, float freq_ref, float x);
	static Temperament create_equal12(int pitch_ref, float freq_ref);
};

#endif /* SRC_DATA_MIDI_TEMPERAMENT_H_ */
