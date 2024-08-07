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

const int MaxPitch = 128;
const int MiddleC = 60;
const int MiddleA = 69;

class MidiEffect;
class Instrument;
class Clef;
class Scale;
class SongSelection;
class MidiNoteBuffer;

enum class MidiMode {
	Linear,
	Tab,
	Classical,
	Drum,
	DontCare
};


class MidiEventBuffer : public Array<MidiEvent> {
public:
	MidiEventBuffer();
	void _cdecl __init__();
	void clear();
	Array<MidiEvent> get_events(const Range &r) const;
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


namespace DrumPitch {
	enum {
		BassAccoustic = 35,
		Bass = 36,
		SideStick = 37,
		Snare = 38,
		Clap = 39,
		SnareElectronic = 40,
		TomFloorLow = 41,
		HihatClosed = 42,
		TomFloorHi = 43,
		HihatPedal = 44,
		TomLow = 45,
		HihatOpen = 46,
		TomLowMid = 47,
		TomHiMid = 48,
		Crash1 = 49,
		TomHi = 50,
		Ride1 = 51,
		Chinese = 52,
		BellRide = 53,
		Tambourine = 54,
		Splash = 55,
		Cowbell = 56,
		Crash2 = 57,
		Vibraslash = 58,
		Ride2 = 59,
		BongoHi = 60,
		BongoLow = 61,
	};
}

enum class ChordType {
	Minor,
	Major,
	Diminished,
	Augmented,
	MinorSeventh,
	MajorSeventh,
	MinorMajorSeventh,
	DiminishedSeventh,
	HalfDiminishedSeventh,
	DominantSeventh,
	AugmentedSeventh,
	AugmentedMajorSeventh,
	Count
};

string chord_type_name(ChordType type);
Array<int> chord_notes(ChordType type, int inversion, int pitch);



enum class NoteModifier {
	None,
	Sharp,
	Flat,
	Natural,
	Unknown = -1,
};

string modifier_symbol(NoteModifier mod);

int modifier_shift(NoteModifier mod);
NoteModifier combine_note_modifiers(NoteModifier mod, NoteModifier scale_mod);

int modifier_apply(int pitch, NoteModifier mod);
int modifier_apply(int pitch, NoteModifier mod, NoteModifier scale_mod);

}

#endif /* SRC_DATA_MIDI_MIDIDATA_H_ */
