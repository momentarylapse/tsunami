/*
 * MidiSource.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiSource.h"
#include "../lib/file/msg.h"
#include "../Rhythm/Beat.h"
#include "../Rhythm/BeatSource.h"


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




int BeatMidifier::read(MidiEventBuffer &midi)
{
	Array<Beat> beats;
	beat_source->read(beats, midi.samples);


	for (Beat &b: beats)
		midi.addMetronomeClick(b.range.offset, b.level, 0.8f);

	return midi.samples;
}
