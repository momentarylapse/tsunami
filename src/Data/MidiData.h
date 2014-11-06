/*
 * MidiData.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef MIDIDATA_H_
#define MIDIDATA_H_

#include "../lib/base/base.h"
#include "Range.h"

class MidiEffect;

class MidiNote
{
public:
	MidiNote(){}
	MidiNote(const Range &range, float pitch, float volume);
	float getFrequency();
	Range range;
	float pitch;
	float volume;
};

class MidiEvent
{
public:
	MidiEvent(){}
	MidiEvent(int pos, float pitch, float volume);
	int pos;
	float pitch;
	float volume;
};

class MidiData : public Array<MidiNote>
{
public:
	Array<MidiNote> getNotes(const Range &r);
	int getNextNote(int pos);
	Range getRange(int elongation);

	void sort();

	Array<MidiEffect*> fx;
};

string GetChordTypeName(int type);
Array<string> GetChordTypeNames();
Array<int> GetChordNotes(int type, int inversion, int pitch);


enum
{
	CHORD_TYPE_MINOR,
	CHORD_TYPE_MAJOR,
	CHORD_TYPE_DIMINISHED,
	CHORD_TYPE_AUGMENTED,
	NUM_CHORD_TYPES
};


#endif /* MIDIDATA_H_ */
