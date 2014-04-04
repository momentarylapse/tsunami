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

class MidiNote
{
public:
	MidiNote(){}
	MidiNote(const Range &range, float pitch, float volume);
	float GetFrequency();
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
	Array<MidiNote> GetNotes(const Range &r);
	int GetNextNote(int pos);
	Range GetRange();

	void sort();
};

enum
{
	CHORD_TYPE_NONE = -1
};

string GetChordTypeName(int type);
Array<string> GetChordTypeNames();
Array<int> GetChordNotes(int type, int pitch);


#endif /* MIDIDATA_H_ */
