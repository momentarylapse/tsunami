/*
 * BarMidifier.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "BarMidifier.h"
#include "Beat.h"
#include "../Midi/MidiData.h"
#include "../Module/Port/BeatPort.h"

int BeatMidifier::read(MidiEventBuffer &midi)
{
	if (!beat_source)
		return midi.samples;

	Array<Beat> beats;
	beat_source->read(beats, midi.samples);

	for (Beat &b: beats)
		midi.addMetronomeClick(b.range.offset, b.level, 0.8f);

	return midi.samples;
}

void BeatMidifier::reset()
{
	if (beat_source)
		beat_source->reset();
}
