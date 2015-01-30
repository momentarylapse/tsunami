/*
 * MidiData.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "MidiData.h"
#include <math.h>
#include "../lib/hui/hui.h"


float pitch_to_freq(float pitch)
{
	return 440.0f * pow(2, (pitch - 69.0f) / 12.0f);
}


// "scientific" notation
//   naive MIDI octave is off by 1
int pitch_get_octave(int pitch)
{
	return (pitch / 12) - 1;
}

int pitch_from_octave_and_rel(int rel, int octave)
{
	return rel + octave * 12 + 12;
}

int pitch_to_rel(int pitch)
{
	return pitch % 12;
}

string rel_pitch_name(int pitch_rel)
{
	if (pitch_rel == 0)
		return "C";
	if (pitch_rel == 1)
		return "C#";
	if (pitch_rel == 2)
		return "D";
	if (pitch_rel == 3)
		return "D#";
	if (pitch_rel == 4)
		return "E";
	if (pitch_rel == 5)
		return "F";
	if (pitch_rel == 6)
		return "F#";
	if (pitch_rel == 7)
		return "G";
	if (pitch_rel == 8)
		return "G#";
	if (pitch_rel == 9)
		return "A";
	if (pitch_rel == 10)
		return "A#";
	if (pitch_rel == 11)
		return "H";
	return "???";
}

string pitch_name(int pitch)
{
	return rel_pitch_name(pitch_to_rel(pitch)) + " " + i2s(pitch_get_octave(pitch));
}

MidiNote::MidiNote(const Range &_range, float _pitch, float _volume)
{
	range = _range;
	pitch = _pitch;
	volume = _volume;
}

float MidiNote::getFrequency()
{
	return 440.0f * pow(2.0f, (float)(pitch - 69) / 12.0f);
}

Array<MidiNote> MidiData::getNotes(const Range &r)
{
	Array<MidiNote> a;
	for (int i=0;i<num;i++)
		if ((*this)[i].range.overlaps(r))
			a.add((*this)[i]);
	return a;
}

int MidiData::getNextNote(int pos)
{
	return 0;
}

Range MidiData::getRange(int elongation)
{
	if (num == 0)
		return Range(0, 0);
	int i0 = (*this)[0].range.offset;
	int i1 = back().range.end();
	return Range(i0, i1 - i0 + elongation);
}

void MidiData::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i].range.offset > (*this)[j].range.offset)
				swap(i, j);
}

MidiEvent::MidiEvent(int _pos, float _pitch, float _volume)
{
	pos = _pos;
	pitch = _pitch;
	volume = _volume;
}


string GetChordTypeName(int type)
{
	if (type == CHORD_TYPE_MINOR)
		return _("Moll");
	if (type == CHORD_TYPE_MAJOR)
		return _("Dur");
	if (type == CHORD_TYPE_DIMINISHED)
		return _("Vermindert");
	if (type == CHORD_TYPE_AUGMENTED)
		return _("&Uberm&a&sig");
	return "???";
}

Array<string> GetChordTypeNames()
{
	Array<string> r;
	for (int i=0; i<NUM_CHORD_TYPES; i++)
		r.add(GetChordTypeName(i));
	return r;
}

Array<int> GetChordNotes(int type, int inversion, int pitch)
{
	Array<int> r;
	r.add(pitch);
	if (type == CHORD_TYPE_MINOR){
		r.add(pitch + 3);
		r.add(pitch + 7);
	}else if (type == CHORD_TYPE_MAJOR){
		r.add(pitch + 4);
		r.add(pitch + 7);
	}else if (type == CHORD_TYPE_DIMINISHED){
		r.add(pitch + 3);
		r.add(pitch + 6);
	}else if (type == CHORD_TYPE_AUGMENTED){
		r.add(pitch + 4);
		r.add(pitch + 8);
	}
	if (inversion == 2)
		r.insert(r.pop() - 12, 0);
	if (inversion == 1){
		r.insert(r.pop() - 12, 0);
		r.insert(r.pop() - 12, 0);
	}
	return r;
}

