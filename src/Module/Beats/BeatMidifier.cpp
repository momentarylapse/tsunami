/*
 * BeatMidifier.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "BeatMidifier.h"
#include "../Port/BeatPort.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/base.h"

BeatMidifier::BeatMidifier()
{
	module_type = ModuleType::BEAT_MIDIFIER;
	port_in.add(PortDescription(SignalType::BEATS, (Port**)&beat_source, "in"));

	volume = 0.8f;
}

int BeatMidifier::read(MidiEventBuffer &midi)
{
	if (!beat_source)
		return midi.samples;

	Array<Beat> beats;
	beat_source->read(beats, midi.samples);

	for (Beat &b: beats)
		midi.addMetronomeClick(b.range.offset, b.level, volume);

	return midi.samples;
}

void BeatMidifier::reset()
{
	if (beat_source)
		beat_source->reset();
}
