/*
 * MidiJoiner.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: michi
 */

#include "MidiJoiner.h"
#include "../../data/base.h"
#include "../../data/midi/MidiData.h"

MidiJoiner::MidiJoiner() : Module(ModuleCategory::PLUMBING, "MidiJoiner") {
	port_out.add(new Output(this));
	port_in.add({SignalType::MIDI, &a, "a"});
	port_in.add({SignalType::MIDI, &b, "b"});
	a = nullptr;
	b = nullptr;
}

MidiJoiner::Output::Output(MidiJoiner* j) : Port(SignalType::MIDI, "out") {
	joiner = j;
}

// if A or B is end-of-stream... just ignore and suck the other!
int MidiJoiner::Output::read_midi(MidiEventBuffer& buf) {
	if (joiner->a and joiner->b) {
		int ra = joiner->a->read_midi(buf);
		if (ra == NOT_ENOUGH_DATA) {
			// better to wait...
			return ra;
		}
		// hmmm needs buffering if A has data, but B has none yet (e.g. input stream)
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

	// no sources present
	return NO_SOURCE;
}
