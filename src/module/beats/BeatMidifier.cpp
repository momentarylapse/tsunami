/*
 * BeatMidifier.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "BeatMidifier.h"
#include "../port/Port.h"
#include "../ModuleFactory.h"
#include "../../data/midi/MidiData.h"
#include "../../data/rhythm/Beat.h"
#include "../../data/base.h"

namespace tsunami {

BeatMidifier::BeatMidifier() {
	module_category = ModuleCategory::Plumbing;
	module_class = "BeatMidifier";

	volume = 1.0f;
}

int BeatMidifier::read(MidiEventBuffer &midi) {
	if (!in.source)
		return midi.samples;

	Array<Beat> beats;
	int r = in.source->read_beats(beats, midi.samples);

	for (Beat &b: beats)
		midi.add_metronome_click(b.range.offset, b.level, volume);

	return r;
}

}
