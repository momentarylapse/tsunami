/*
 * BeatMidifier.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "BeatMidifier.h"
#include "../Port/BeatPort.h"
#include "../ModuleFactory.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/base.h"

BeatMidifier::BeatMidifier()
{
	module_type = ModuleType::BEAT_MIDIFIER;
	port_in.add(InPortDescription(SignalType::BEATS, (Port**)&beat_source, "in"));

	volume = 1.0f;
}

int BeatMidifier::read(MidiEventBuffer &midi)
{
	if (!beat_source)
		return midi.samples;

	Array<Beat> beats;
	int r = beat_source->read(beats, midi.samples);

	for (Beat &b: beats)
		midi.addMetronomeClick(b.range.offset, b.level, volume);

	return r;
}

void BeatMidifier::reset()
{
	if (beat_source)
		beat_source->reset();
}



BeatMidifier *CreateBeatMidifier(Session *session)
{
	return (BeatMidifier*)ModuleFactory::create(session, ModuleType::BEAT_MIDIFIER, "");
}
