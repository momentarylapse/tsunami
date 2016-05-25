/*
 * MidiSource.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiSource.h"
#include "../lib/file/msg.h"


MidiSource::MidiSource()
{
}

void MidiSource::__init__()
{
	new(this) MidiSource;
}

void MidiSource::__delete__()
{
	this->MidiSource::~MidiSource();
}

MidiDataSource::MidiDataSource(const MidiRawData& _midi)
{
	midi = _midi;
	offset = 0;
}

MidiDataSource::~MidiDataSource()
{
}

int MidiDataSource::read(MidiRawData& _midi)
{
	int n = min(midi.samples - offset, _midi.samples);
	Range r = Range(offset, n);
	//midi.read(_midi, r);
	foreach(MidiEvent &e, midi)
		if (r.is_inside(e.pos))
			_midi.add(MidiEvent(e.pos - offset, e.pitch, e.volume));
	offset -= n;
	return n;
}

void MidiDataSource::setData(const MidiRawData &_midi)
{
	midi = _midi;
	offset = 0;
}

/*Range MidiDataSource::range()
{
	return midi.getRange(0);
}

int MidiDataSource::getPos()
{
	return offset;
}*/

void MidiDataSource::seek(int pos)
{
	offset = pos;
}
