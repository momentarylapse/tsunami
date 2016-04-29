/*
 * MidiData.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef MIDIDATA_H_
#define MIDIDATA_H_

#include "../lib/base/base.h"
#include "../Data/Range.h"
#include "MidiNote.h"
#include "MidiEvent.h"

#define MAX_PITCH		128

class MidiDataRef;
class MidiEffect;
class Instrument;
class Clef;
class Scale;


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
	void clear_meta() const;

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
	DRUM_PITCH_BASS_ACCOUSTIC = 35,
	DRUM_PITCH_BASS = 36,
	DRUM_PITCH_SIDE_STICK = 37,
	DRUM_PITCH_SNARE = 38,
	DRUM_PITCH_CLAP = 39,
	DRUM_PITCH_SNARE_ELECTRONIC = 40,
	DRUM_PITCH_TOM_FLOOR_LOW = 41,
	DRUM_PITCH_HIHAT_CLOSED = 42,
	DRUM_PITCH_TOM_FLOOR_HI = 43,
	DRUM_PITCH_HIHAT_PEDAL = 44,
	DRUM_PITCH_TOM_LOW = 45,
	DRUM_PITCH_HIHAT_OPEN = 46,
	DRUM_PITCH_TOM_LOW_MID = 47,
	DRUM_PITCH_TOM_HI_MID = 48,
	DRUM_PITCH_CRASH_1 = 49,
	DRUM_PITCH_TOM_HI = 50,
	DRUM_PITCH_RIDE_1 = 51,
	DRUM_PITCH_CHINESE = 52,
	DRUM_PITCH_BELL_RIDE = 53,
	DRUM_PITCH_TAMBOURINE = 54,
	DRUM_PITCH_SPLASH = 55,
	DRUM_PITCH_COWBELL = 56,
	DRUM_PITCH_CRASH_2 = 57,
	DRUM_PITCH_VIBRASLASH = 58,
	DRUM_PITCH_RIDE_2 = 59,
	DRUM_PITCH_BONGO_HI = 60,
	DRUM_PITCH_BONGO_LOW = 61,
};

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
