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

#define MAX_PITCH		128

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

class MidiRawData : public Array<MidiEvent>
{
public:
	MidiRawData();
	void _cdecl __init__();
	MidiRawData getEvents(const Range &r) const;
	int read(MidiRawData &data, const Range &r) const;
	Array<MidiNote> getNotes(const Range &r) const;
	int getNextEvent(int pos) const;

	Range getRange(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);

	void addMetronomeClick(int pos, int level, float volume);
	void append(const MidiRawData &data);
};

class MidiNoteData : public Array<MidiNote>
{
public:
	MidiNoteData();
	void _cdecl __init__();
	MidiRawData getEvents(const Range &r) const;
	MidiNoteData getNotes(const Range &r) const;

	Range getRange(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);

	Array<MidiEffect*> fx;
};

MidiRawData midi_notes_to_events(const MidiNoteData &notes);
MidiNoteData midi_events_to_notes(const MidiRawData &events);



float pitch_to_freq(float pitch);
float freq_to_pitch(float freq);
int pitch_get_octave(int pitch);
int pitch_from_octave_and_rel(int rel, int octave);
int pitch_to_rel(int pitch);
string rel_pitch_name(int pitch_rel);
string pitch_name(int pitch);
string drum_pitch_name(int pitch);

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
