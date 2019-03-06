/*
 * BeatMidifier.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "BeatMidifier.h"
#include "../Port/Port.h"
#include "../ModuleFactory.h"
#include "../../Data/Midi/MidiData.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/base.h"

BeatMidifier::BeatMidifier()
{
	module_type = ModuleType::PLUMBING;
	module_subtype = "BeatMidifier";
	beat_source = nullptr;
	port_in.add(InPortDescription(SignalType::BEATS, (Port**)&beat_source, "in"));

	volume = 1.0f;
}

int BeatMidifier::read(MidiEventBuffer &midi)
{
	if (!beat_source)
		return midi.samples;

	Array<Beat> beats;
	int r = beat_source->read_beats(beats, midi.samples);

	for (Beat &b: beats)
		midi.add_metronome_click(b.range.offset, b.level, volume);

	return r;
}
