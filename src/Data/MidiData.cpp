/*
 * MidiData.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "MidiData.h"
#include <math.h>
#include "../lib/hui/hui.h"


MidiNote::MidiNote(const Range &_range, float _pitch, float _volume)
{
	range = _range;
	pitch = _pitch;
	volume = _volume;
}

float MidiNote::GetFrequency()
{
	return 440.0f * pow(2.0f, (float)(pitch - 69) / 12.0f);
}

Array<MidiNote> MidiData::GetNotes(const Range &r)
{
	Array<MidiNote> a;
	for (int i=0;i<num;i++)
		if ((*this)[i].range.overlaps(r))
			a.add((*this)[i]);
	return a;
}

int MidiData::GetNextNote(int pos)
{
	return 0;
}

Range MidiData::GetRange()
{
	if (num == 0)
		return Range(0, 0);
	int i0 = (*this)[0].range.offset;
	int i1 = back().range.end();
	return Range(i0, i1 - i0);
}

void MidiData::sort()
{
	for (int i=0;i<num;i++)
		for (int j=i+1;j<num;j++)
			if ((*this)[i].range.offset > (*this)[j].range.offset)
				swap(i, j);
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

