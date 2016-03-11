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
#include "MidiNote.h"
#include "MidiEvent.h"
#include "Scale.h"

#define MAX_PITCH		128

class MidiDataRef;
class MidiEffect;
class Instrument;


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

class MidiData : public Array<MidiNote>
{
public:
	MidiData();
	void _cdecl __init__();
	MidiRawData getEvents(const Range &r) const;
	MidiDataRef getNotes(const Range &r) const;
	MidiDataRef getNotesSafe(const Range &r) const;

	Range getRange(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);
	void update_meta(const Instrument &i, const Scale &s) const;

	Array<MidiEffect*> fx;
};

class MidiDataRef : public MidiData
{
public:
	MidiDataRef(const MidiData &m);
};

MidiRawData midi_notes_to_events(const MidiData &notes);
MidiData midi_events_to_notes(const MidiRawData &events);



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


enum{
	CLEF_TYPE_TREBLE,
	CLEF_TYPE_TREBLE_8,
	CLEF_TYPE_BASS,
	CLEF_TYPE_BASS_8,
	CLEF_TYPE_DRUMS,
	NUM_CLEF_TYPES
};

string clef_symbol(int clef);

int pitch_to_clef_position(int pitch, int clef, const Scale &s, int &modifier);
int clef_position_to_pitch(int position, int clef, const Scale &s, int modifier);

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
