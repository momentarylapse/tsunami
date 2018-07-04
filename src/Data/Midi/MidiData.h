/*
 * MidiData.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDIDATA_H_
#define SRC_DATA_MIDI_MIDIDATA_H_

#include "../../lib/base/base.h"
#include "../Range.h"
#include "MidiNote.h"
#include "MidiEvent.h"

#define MAX_PITCH		128

class MidiNoteBufferRef;
class MidiEffect;
class Instrument;
class Clef;
class Scale;
class Track;
class SongSelection;


class MidiEventBuffer : public Array<MidiEvent>
{
public:
	MidiEventBuffer();
	void _cdecl __init__();
	MidiEventBuffer getEvents(const Range &r) const;
	int read(MidiEventBuffer &data, const Range &r) const;
	Array<MidiNote> getNotes(const Range &r) const;
	int getNextEvent(int pos) const;

	Range getRange(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);

	void addMetronomeClick(int pos, int level, float volume);
	void append(const MidiEventBuffer &data);
};

class MidiNoteBuffer : public Array<MidiNote*>
{
public:
	MidiNoteBuffer();
	MidiNoteBuffer(const MidiNoteBuffer &midi);
	~MidiNoteBuffer();
	void _cdecl __init__();
	void _cdecl __delete__();
	void deep_clear();
	MidiEventBuffer getEvents(const Range &r) const;
	MidiNoteBufferRef getNotes(const Range &r) const;
	MidiNoteBufferRef getNotesBySelection(const SongSelection &s) const;
	MidiNoteBuffer duplicate() const;
	void append(const MidiNoteBuffer &midi, int offset);

	Range range(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);

	void operator=(const MidiNoteBuffer &midi);
	void operator=(const MidiNoteBufferRef &midi);

	//void update_meta(const Instrument &i, const Scale &s) const;
	void update_meta(Track *t, const Scale &s) const;
	void clear_meta() const;
};

class MidiNoteBufferRef : public MidiNoteBuffer
{
public:
	MidiNoteBufferRef();
	~MidiNoteBufferRef();
};

MidiEventBuffer midi_notes_to_events(const MidiNoteBuffer &notes);
MidiNoteBuffer midi_events_to_notes(const MidiEventBuffer &events);



float pitch_to_freq(float pitch);
float freq_to_pitch(float freq);
int pitch_get_octave(int pitch);
int pitch_from_octave_and_rel(int rel, int octave);
int pitch_to_rel(int pitch);
string rel_pitch_name(int pitch_rel);
string pitch_name(int pitch);
string drum_pitch_name(int pitch);


enum DrumPitch
{
	BASS_ACCOUSTIC = 35,
	BASS = 36,
	SIDE_STICK = 37,
	SNARE = 38,
	CLAP = 39,
	SNARE_ELECTRONIC = 40,
	TOM_FLOOR_LOW = 41,
	HIHAT_CLOSED = 42,
	TOM_FLOOR_HI = 43,
	HIHAT_PEDAL = 44,
	TOM_LOW = 45,
	HIHAT_OPEN = 46,
	TOM_LOW_MID = 47,
	TOM_HI_MID = 48,
	CRASH_1 = 49,
	TOM_HI = 50,
	RIDE_1 = 51,
	CHINESE = 52,
	BELL_RIDE = 53,
	TAMBOURINE = 54,
	SPLASH = 55,
	COWBELL = 56,
	CRASH_2 = 57,
	VIBRASLASH = 58,
	RIDE_2 = 59,
	BONGO_HI = 60,
	BONGO_LOW = 61,
};

enum ChordType
{
	MINOR,
	MAJOR,
	DIMINISHED,
	AUGMENTED,
	NUM
};

string chord_type_name(int type);
Array<int> chord_notes(int type, int inversion, int pitch);



enum Modifier
{
	NONE,
	SHARP,
	FLAT,
	NATURAL,
	UNKNOWN = -1,
};

inline string modifier_symbol(int mod)
{
	if (mod == Modifier::NONE)
		return "";
	if (mod == Modifier::SHARP)
		return "\u266f";
	if (mod == Modifier::FLAT)
		return "\u266d";
	if (mod == Modifier::NATURAL)
		return "\u266e";
	return "?";
}


#endif /* SRC_DATA_MIDI_MIDIDATA_H_ */
