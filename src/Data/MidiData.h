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
	MidiNoteData getNotesSafe(const Range &r) const;

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


enum
{
	CHORD_TYPE_MINOR,
	CHORD_TYPE_MAJOR,
	CHORD_TYPE_DIMINISHED,
	CHORD_TYPE_AUGMENTED,
	NUM_CHORD_TYPES
};

string chord_type_name(int type);
Array<int> chord_notes(int type, int inversion, int pitch);

class Scale
{
public:

	enum
	{
		TYPE_MAJOR,
		TYPE_DORIAN,
		TYPE_PHRYGIAN,
		TYPE_LYDIAN,
		TYPE_MIXOLYDIAN,
		TYPE_MINOR,
		TYPE_LOCRIAN,
		NUM_TYPES
	};
	int type, root;

	Scale(int type, int root);
	bool contains(int pitch) const;
	const int* get_modifiers_clef();
	//int* get_modifiers_pitch();
	int transform_out(int x, int modifier) const;

	static string type_name(int type);
};


enum{
	CLEF_TYPE_TREBLE,
	CLEF_TYPE_TREBLE_8,
	CLEF_TYPE_BASS,
	CLEF_TYPE_BASS_8,
	CLEF_TYPE_DRUMS,
	NUM_CLEF_TYPES
};

string clef_symbol(int clef);

int pitch_to_clef_position(int pitch, int clef, Scale &s, int &modifier);
int clef_position_to_pitch(int position, int clef, Scale &s, int modifier);

enum{
	MODIFIER_NONE,
	MODIFIER_SHARP,
	MODIFIER_FLAT,
	MODIFIER_NATURAL,
};

inline string modifier_symbol(int mod)
{
	if (mod == MODIFIER_SHARP)
		return "\u266f";
	if (mod == MODIFIER_FLAT)
		return "\u266d";
	if (mod == MODIFIER_NATURAL)
		return "\u266e";
	return "";
}


#endif /* MIDIDATA_H_ */
