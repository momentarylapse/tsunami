/*
 * MidiPattern.h
 *
 *  Created on: 14.08.2013
 *      Author: michi
 */

#ifndef MIDIPATTERN_H_
#define MIDIPATTERN_H_

#include "../lib/base/base.h"

class MidiNote;
class AudioFile;

class MidiPattern
{
public:
	MidiPattern();
	virtual ~MidiPattern();

	string name;
	int ref_count;
	AudioFile *owner;
	int num_beats, beat_partition;
	float volume_randomness, time_randomness;

	Array<MidiNote> notes;
};

#endif /* MIDIPATTERN_H_ */
