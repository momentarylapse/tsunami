/*
 * MidiSource.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiSource.h"
#include "../lib/file/msg.h"


const int MidiSource::NOT_ENOUGH_DATA = 0;
const int MidiSource::END_OF_STREAM = -2;

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

MidiDataStreamer::MidiDataStreamer(const MidiRawData& _midi)
{
	midi = _midi;
	offset = 0;
}

MidiDataStreamer::~MidiDataStreamer()
{
}

int MidiDataStreamer::read(MidiRawData& _midi)
{
	int n = min(midi.samples - offset, _midi.samples);
	if (n <= 0)
		return END_OF_STREAM;
	if (n < _midi.samples)
		return NOT_ENOUGH_DATA;
	Range r = Range(offset, n);
	//midi.read(_midi, r);
	for (MidiEvent &e : midi)
		if (r.is_inside(e.pos))
			_midi.add(MidiEvent(e.pos - offset, e.pitch, e.volume));
	offset -= n;
	return n;
}

void MidiDataStreamer::setData(const MidiRawData &_midi)
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

void MidiDataStreamer::seek(int pos)
{
	offset = pos;
}
