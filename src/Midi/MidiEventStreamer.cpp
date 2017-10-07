/*
 * MidiEventStreamer.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "MidiEventStreamer.h"


MidiEventStreamer::MidiEventStreamer(const MidiEventBuffer& _midi)
{
	midi = _midi;
	offset = 0;
	ignore_end = false;
}

MidiEventStreamer::~MidiEventStreamer()
{
}

int MidiEventStreamer::read(MidiEventBuffer& _midi)
{
	int n = min(midi.samples - offset, _midi.samples);
	if (ignore_end)
		n = _midi.samples;
	if (n <= 0)
		return END_OF_STREAM;
	if ((n < _midi.samples) and !ignore_end)
		return NOT_ENOUGH_DATA;
	Range r = Range(offset, n);
	//midi.read(_midi, r);
	for (MidiEvent &e : midi)
		if (r.is_inside(e.pos))
			_midi.add(MidiEvent(e.pos - offset, e.pitch, e.volume));
	offset += n;
	return n;
}

void MidiEventStreamer::setData(const MidiEventBuffer &_midi)
{
	midi = _midi;
	offset = 0;
}

void MidiEventStreamer::seek(int pos)
{
	offset = pos;
}




