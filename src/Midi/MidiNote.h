/*
 * MidiNote.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDINOTE_H_
#define SRC_DATA_MIDINOTE_H_

#include "../Data/Range.h"

class Scale;
class Instrument;

class MidiNote
{
public:
	MidiNote();
	MidiNote(const Range &range, float pitch, float volume);
	MidiNote *copy() const;
	float getFrequency();
	Range range;
	float pitch;
	float volume;
	mutable int stringno, clef_position, modifier;
	mutable int y;

	void reset_meta();
	void update_meta(const Instrument &i, const Scale &s, int hand_position) const;
};


#endif /* SRC_DATA_MIDINOTE_H_ */
