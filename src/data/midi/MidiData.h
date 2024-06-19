/*
 * MidiData.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_MIDIDATA_H_
#define SRC_DATA_MIDI_MIDIDATA_H_

#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"
#include "../Range.h"
#include "MidiNote.h"
#include "MidiEvent.h"


namespace tsunami {

const int MAX_PITCH = 128;
const int MIDDLE_C = 60;
const int MIDDLE_A = 69;

class MidiEffect;
class Instrument;
class Clef;
class Scale;
class SongSelection;
class MidiNoteBuffer;

enum class MidiMode {
	LINEAR,
	TAB,
	CLASSICAL,
	DRUM,
	DONT_CARE
};


class MidiEventBuffer : public Array<MidiEvent> {
public:
	MidiEventBuffer();
	void _cdecl __init__();
	void clear();
	MidiEventBuffer get_events(const Range &r) const;
	int read(MidiEventBuffer &data, const Range &r) const;
	MidiNoteBuffer get_notes(const Range &r) const;
	int get_next_event(int pos) const;

	Range range(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);

	void add_note(const Range &range, float pitch, float volume);
	void add_metronome_click(int pos, int level, float volume);
	void append(const MidiEventBuffer &data);
};

class MidiNoteBuffer : public shared_array<MidiNote> {
public:
	MidiNoteBuffer();
	MidiNoteBuffer(const MidiNoteBuffer &midi);
	~MidiNoteBuffer();
	void _cdecl __init__();
	void _cdecl __delete__();
	void clear();
	MidiEventBuffer get_events(const Range &r) const;
	MidiNoteBuffer get_notes(const Range &r) const;
	MidiNoteBuffer get_notes_by_selection(const SongSelection &s) const;
	MidiNoteBuffer duplicate(int offset=0) const;
	void append(const MidiNoteBuffer &midi, int offset);

	Range range(int elongation) const;
	int samples;

	void sort();
	void sanify(const Range &r);

	void operator=(const MidiNoteBuffer &midi);

	void update_clef_pos(const Instrument &instrument, const Scale &s) const;
	void reset_clef() const;

	bool has(MidiNote *n) const;
};

MidiEventBuffer midi_notes_to_events(const MidiNoteBuffer &notes);
MidiNoteBuffer midi_events_to_notes(const MidiEventBuffer &events);



float pitch_to_freq(float pitch);
float freq_to_pitch(float freq);
int pitch_get_octave(int pitch);
int pitch_from_octave_and_rel(int rel, int octave);
int pitch_to_rel(int pitch);
string rel_pitch_name(int pitch_rel);
string rel_pitch_name_canonical(int pitch_rel);
int parse_rel_pitch(const string &name);
string pitch_name(int pitch);
string drum_pitch_name(int pitch);


enum DrumPitch {
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

enum class ChordType {
	MINOR,
	MAJOR,
	DIMINISHED,
	AUGMENTED,
	MINOR_SEVENTH,
	MAJOR_SEVENTH,
	MINOR_MAJOR_SEVENTH,
	DIMINISHED_SEVENTH,
	HALF_DIMINISHED_SEVENTH,
	DOMINANT_SEVENTH,
	AUGMENTED_SEVENTH,
	AUGMENTED_MAJOR_SEVENTH,
	NUM
};

string chord_type_name(ChordType type);
Array<int> chord_notes(ChordType type, int inversion, int pitch);



enum class NoteModifier {
	NONE,
	SHARP,
	FLAT,
	NATURAL,
	UNKNOWN = -1,
};

string modifier_symbol(NoteModifier mod);

int modifier_shift(NoteModifier mod);
NoteModifier combine_note_modifiers(NoteModifier mod, NoteModifier scale_mod);

int modifier_apply(int pitch, NoteModifier mod);
int modifier_apply(int pitch, NoteModifier mod, NoteModifier scale_mod);

}

#endif /* SRC_DATA_MIDI_MIDIDATA_H_ */
