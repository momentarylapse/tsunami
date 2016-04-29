/*
 * MidiSource.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiSource.h"

MidiDataSource::MidiDataSource(const MidiData& _midi)
{
	midi = midi_notes_to_events(_midi);
	offset = 0;
}

MidiDataSource::~MidiDataSource()
{
}

int MidiDataSource::read(MidiRawData& _midi)
{
	int n = min(midi.samples - offset, _midi.samples);
	Range r = Range(offset, n);
	foreach(MidiEvent &e, midi)
		if (r.is_inside(e.pos))
			_midi.add(MidiEvent(e.pos - offset, e.pitch, e.volume));
	offset -= n;
	return n;
}

void MidiDataSource::reset()
{
	offset = 0;
}

Range MidiDataSource::range()
{
	return midi.getRange(0);
}

int MidiDataSource::getPos()
{
	return offset;
}

void MidiDataSource::seek(int pos)
{
	offset = pos;
}
