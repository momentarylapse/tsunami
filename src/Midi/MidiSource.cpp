/*
 * MidiSource.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiSource.h"
#include "../lib/file/msg.h"
#include "../Data/Rhythm.h"


const int MidiSource::NOT_ENOUGH_DATA = 0;
const int MidiSource::END_OF_STREAM = -2;

MidiSource::MidiSource()
{
	beat_source = BeatSource::dummy;
}

void MidiSource::__init__()
{
	new(this) MidiSource;
}

void MidiSource::__delete__()
{
	this->MidiSource::~MidiSource();
}

void MidiSource::setBeatSource(BeatSource *_beat_source)
{
	beat_source = _beat_source;
}



MidiDataStreamer::MidiDataStreamer(const MidiRawData& _midi)
{
	midi = _midi;
	offset = 0;
	ignore_end = false;
}

MidiDataStreamer::~MidiDataStreamer()
{
}

int MidiDataStreamer::read(MidiRawData& _midi)
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

void MidiDataStreamer::setData(const MidiRawData &_midi)
{
	midi = _midi;
	offset = 0;
}

void MidiDataStreamer::seek(int pos)
{
	offset = pos;
}




int BeatMidifier::read(MidiRawData &midi)
{
	Array<Beat> beats;
	beat_source->read(beats, midi.samples);
	return midi.samples;
}
