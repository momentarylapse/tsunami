/*
 * MidiJoiner.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: michi
 */

#include "MidiJoiner.h"
#include "../../Data/base.h"
#include "../../Data/Midi/MidiData.h"

MidiJoiner::MidiJoiner() : Module(ModuleType::PLUMBING, "MidiJoiner") {
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::MIDI, &a, "a"));
	port_in.add(InPortDescription(SignalType::MIDI, &b, "b"));
	a = nullptr;
	b = nullptr;
}

MidiJoiner::Output::Output(MidiJoiner* j) : Port(SignalType::MIDI, "out") {
	joiner = j;
}

int MidiJoiner::Output::read_midi(MidiEventBuffer& buf) {
	if (joiner->a and joiner->b) {
		int ra = joiner->a->read_midi(buf);
		if (ra <= 0)
			return ra;
		// hmmm needs buffering if a has data, but b has none yet (input stream)
		MidiEventBuffer buf_b;
		buf_b.samples = buf.samples;
		int rb = joiner->b->read_midi(buf_b);
		for (auto &n: buf_b)
			buf.add(n);
		return max(ra, rb);
	} else if (joiner->a) {
		return joiner->a->read_midi(buf);
	} else if (joiner->b) {
		return joiner->b->read_midi(buf);
	}
	return buf.samples;
}
